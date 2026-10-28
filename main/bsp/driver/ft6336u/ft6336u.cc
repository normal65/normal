/**
 * 文件：main/bsp/driver/ft6336u/ft6336u.cc
 *
 * FT6336U 电容触摸控制器的底层驱动，走 I2C。
 *
 * 这个文件最重要的地方是"中断"：
 *   手指按下 / 抬起时，芯片会把 INT 引脚拉低，触发 ESP32 的 GPIO 中断。
 *   中断服务函数 int_isr_handler 只做一件事：给一个信号量发通知。
 *   真正的 I2C 读坐标交给输入任务（input_router.cc）去做。
 *
 * 为什么不直接在中断里读 I2C？因为 I2C 传输要等总线、要几百微秒到几毫秒，
 * 中断里等这么久会拖垮整个系统。所以中断必须"秒退"，只留一个信号给任务。
 *
 * 对外接口：
 *   init()             初始化 I2C 设备、复位、探测、写基础寄存器
 *   enable_interrupt() 注册 GPIO 中断并创建信号量
 *   wait_interrupt()   任务里等待中断（带超时）
 *   poll_interrupt()   不等待，只清一下攒下的中断标记
 *   read() / is_touched()  通过 I2C 读坐标
 */

#include "ft6336u.hpp"

#include <algorithm>
#include <cstring>

#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "config/config.hpp"
#include "system/task/task.hpp"

static const char* const TAG = "FT6336U";

namespace {

// 中断里用的二进制信号量。中断 give，任务 take。
SemaphoreHandle_t g_int_sem = nullptr;
// GPIO 中断服务是否已经安装（全局只需要装一次）。
bool g_isr_service = false;
// 中断处理函数的引用计数。多个触摸对象时保证只注册/注销一次。
int g_int_isr_ref = 0;

// GPIO 中断服务函数。
//
// 约束：
//   - IRAM_ATTR：放进内部 RAM 执行。因为中断可能在 Flash 操作期间触发，
//     那时不能从 Flash 取指令，否则会崩。
//   - 必须极快返回：这里只给信号量发通知，不读 I2C、不打印日志。
void IRAM_ATTR int_isr_handler(void* /*arg*/)
{
    BaseType_t wake = pdFALSE;
    if (g_int_sem != nullptr)
        xSemaphoreGiveFromISR(g_int_sem, &wake);

    // 如果这次 give 唤醒了更高优先级的任务，退出中断时立刻切过去，
    // 让输入任务尽快开始读坐标。
    if (wake == pdTRUE)
        portYIELD_FROM_ISR();
}

} // namespace

namespace app::bsp::driver::ft6336u {

Ft6336u::Ft6336u()
    : bus_handle_(nullptr)
    , dev_handle_(nullptr)
    , initialized_(false)
    , int_enabled_(false)
{
}

Ft6336u::~Ft6336u()
{
    deinit();
}

// 硬件复位：RST 拉低 10ms，再拉高 10ms。
// 具体时间来自屏幕手册；如果换屏，改这里的常量而不是散在各处写魔数。
void Ft6336u::hw_reset()
{
    gpio_config_t cfg = {};
    cfg.pin_bit_mask = (1ULL << config::FT6336_RST);
    cfg.mode = GPIO_MODE_OUTPUT;
    gpio_config(&cfg);

    gpio_set_level(config::FT6336_RST, 0);
    app::sys::task::TaskMgr::delay_ms(10);
    gpio_set_level(config::FT6336_RST, 1);
    app::sys::task::TaskMgr::delay_ms(300);
}

// 往 FT6336U 的一个寄存器写一个字节：先发寄存器地址，再发数据。
bool Ft6336u::write_reg(uint8_t reg, uint8_t val)
{
    if (dev_handle_ == nullptr)
        return false;
    const uint8_t buf[2] = {reg, val};
    return i2c_master_transmit(dev_handle_, buf, 2, 1000) == ESP_OK;
}

// 读寄存器：先写寄存器地址（告诉芯片从哪读），再连续读 len 个字节。
bool Ft6336u::read_reg(uint8_t reg, uint8_t* buf, size_t len) const
{
    if (dev_handle_ == nullptr || buf == nullptr || len == 0)
        return false;
    return i2c_master_transmit_receive(dev_handle_, &reg, 1, buf, len, 1000) == ESP_OK;
}

// 读芯片 ID 并打日志。排查"触摸不工作"时看这条日志最直接。
void Ft6336u::log_chip_info()
{
    if (dev_handle_ == nullptr)
        return;

    uint8_t cipher_lo = 0;
    uint8_t cipher_hi = 0;
    uint8_t fw = 0;
    uint8_t vendor = 0;
    (void)read_reg(REG_CIPHER_LOW, &cipher_lo, 1);
    (void)read_reg(REG_CIPHER_HIGH, &cipher_hi, 1);
    (void)read_reg(REG_FIRMWARE_ID, &fw, 1);
    (void)read_reg(REG_FOCALTECH_ID, &vendor, 1);

    const char* variant = "unknown";
    if (cipher_lo == 0x02)
        variant = "Ft6336U";
    else if (cipher_lo == 0x01)
        variant = "Ft6336G";

    ESP_LOGD(TAG, "CIPHER=0x%02X%02X (%s) FW=0x%02X FOCALTECH_ID=0x%02X", cipher_hi, cipher_lo,
             variant, fw, vendor);
}

// 探测芯片是否在位。
// 读 ID 寄存器，比对厂商规定的值。偶发 I2C 失败会重试 5 次。
// 如果 ID 不是典型值，但能读到触摸状态寄存器，也认为芯片可用（兼容一批山寨屏）。
bool Ft6336u::probe_chip()
{
    uint8_t cipher_lo = 0;
    uint8_t cipher_hi = 0;
    uint8_t vendor = 0;

    for (int attempt = 0; attempt < 5; attempt++)
    {
        if (attempt > 0)
            app::sys::task::TaskMgr::delay_ms(20);

        if (!read_reg(REG_CIPHER_HIGH, &cipher_hi, 1) || !read_reg(REG_CIPHER_LOW, &cipher_lo, 1) ||
            !read_reg(REG_FOCALTECH_ID, &vendor, 1))
        {
            continue;
        }

        // 只要不是全零，说明 I2C 通了，可以退出重试。
        if (cipher_hi != 0 || cipher_lo != 0 || vendor != 0)
            break;
    }

    const bool id_ok = (cipher_hi == kCipherHighExpected && vendor == kFocalTechId &&
                        (cipher_lo == 0x01 || cipher_lo == 0x02));
    if (id_ok)
        return true;

    uint8_t td = 0;
    if (read_reg(REG_TD_STATUS, &td, 1))
    {
        ESP_LOGW(TAG, "ID 非典型 CIPHER=0x%02X%02X VENDOR=0x%02X TD=0x%02X", cipher_hi, cipher_lo,
                 vendor, td);
        return true;
    }

    ESP_LOGE(TAG, "探测失败 CIPHER=0x%02X%02X VENDOR=0x%02X", cipher_hi, cipher_lo, vendor);
    return false;
}

// 写一组基础寄存器：工作模式、触摸阈值、上报方式等。
// 任何一项写失败就返回 false，让 init 能及时报错。
bool Ft6336u::setup_base_regs()
{
    return write_reg(REG_MODE, 0x00) && write_reg(REG_THGROUP, cfg_.threshold) &&
           write_reg(REG_CTRL, 0x00) && write_reg(REG_PERIODACTIVE, cfg_.active_period) &&
           write_reg(REG_REPORT_INT_MODE, cfg_.report_int_mode) &&
           write_reg(REG_WORK_STATE, kWorkStateNormal);
}

// 运行时改坐标方向，不用重新 init。
void Ft6336u::apply_transform(bool swap_xy, bool mirror_x, bool mirror_y)
{
    cfg_.swap_xy = swap_xy;
    cfg_.mirror_x = mirror_x;
    cfg_.mirror_y = mirror_y;
}

// 把原始坐标按配置做交换 / 翻转 / 裁剪。
// 这是"硬件层"的坐标校正；上层手势状态机只再做偏移和裁剪，不重复翻转。
void Ft6336u::map_coord(uint16_t& x, uint16_t& y) const
{
    if (cfg_.panel_width == 0 || cfg_.panel_height == 0)
        return;

    int32_t ix = static_cast<int32_t>(x);
    int32_t iy = static_cast<int32_t>(y);

    if (cfg_.swap_xy)
    {
        const int32_t t = ix;
        ix = iy;
        iy = t;
    }

    if (cfg_.mirror_x)
        ix = static_cast<int32_t>(cfg_.panel_width - 1) - ix;
    if (cfg_.mirror_y)
        iy = static_cast<int32_t>(cfg_.panel_height - 1) - iy;

    // 双保险：翻转后可能越界，裁回屏幕内。
    ix = std::max<int32_t>(0, std::min<int32_t>(ix, cfg_.panel_width - 1));
    iy = std::max<int32_t>(0, std::min<int32_t>(iy, cfg_.panel_height - 1));

    x = static_cast<uint16_t>(ix);
    y = static_cast<uint16_t>(iy);
}

// ─────────────────────────────────────────────────────────
// 中断相关：这一段是理解"触摸怎么触发到任务"的关键。
// ─────────────────────────────────────────────────────────

// 打开触摸中断。做的事：
//   1. 创建二进制信号量（中断 give，任务 take）。
//   2. 安装 GPIO 中断服务（全局一次）。
//   3. 把 int_isr_handler 挂到 INT 引脚。
//   4. 设置触发方式为"任意边沿"——按下和抬起都要通知。
bool Ft6336u::enable_interrupt()
{
    if (!initialized_ || int_enabled_)
        return true;

    if (g_int_sem == nullptr)
    {
        g_int_sem = xSemaphoreCreateBinary();
        if (g_int_sem == nullptr)
        {
            ESP_LOGE(TAG, "INT 信号量创建失败");
            return false;
        }
    }

    if (!g_isr_service)
    {
        if (gpio_install_isr_service(0) != ESP_OK)
        {
            ESP_LOGE(TAG, "gpio_install_isr_service 失败");
            return false;
        }
        g_isr_service = true;
    }

    if (g_int_isr_ref == 0)
    {
        if (gpio_isr_handler_add(config::FT6336_INT, int_isr_handler, nullptr) != ESP_OK)
        {
            ESP_LOGE(TAG, "INT ISR 注册失败");
            return false;
        }
    }
    g_int_isr_ref++;

    if (gpio_set_intr_type(config::FT6336_INT, GPIO_INTR_ANYEDGE) != ESP_OK)
    {
        ESP_LOGE(TAG, "INT 触发类型配置失败");
        disable_interrupt();
        return false;
    }

    int_enabled_ = true;
    ESP_LOGD(TAG, "INT 已启用 GPIO=%d", static_cast<int>(config::FT6336_INT));
    return true;
}

// 关掉中断。引用计数减到 0 时才真正移除处理函数，
// 避免多个使用者在用的时候被别人关掉。
void Ft6336u::disable_interrupt()
{
    if (!int_enabled_)
        return;

    gpio_set_intr_type(config::FT6336_INT, GPIO_INTR_DISABLE);

    if (g_int_isr_ref > 0)
    {
        g_int_isr_ref--;
        if (g_int_isr_ref == 0)
            gpio_isr_handler_remove(config::FT6336_INT);
    }

    int_enabled_ = false;
}

// 任务里等待中断。返回 true 表示等到了，false 表示超时。
//
// 先 take(0) 清一次旧标记，再正式 take(timeout)：
// 这样不会因为信号量里攒着上一次的旧通知而立刻返回。
bool Ft6336u::wait_interrupt(uint32_t timeout_ms)
{
    if (!int_enabled_ || g_int_sem == nullptr)
        return false;

    xSemaphoreTake(g_int_sem, 0);
    return xSemaphoreTake(g_int_sem, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

// 不等待，只把当前攒下的中断标记全部清掉。
// 输入任务处理完一批采样后调用它，避免下一轮被旧标记立刻唤醒。
bool Ft6336u::poll_interrupt()
{
    if (!int_enabled_ || g_int_sem == nullptr)
        return false;

    bool got = false;
    while (xSemaphoreTake(g_int_sem, 0) == pdTRUE)
        got = true;
    return got;
}

// 直接读 INT 引脚电平。FT6336U 有触摸时会把它拉低，所以 ==0 表示有手指。
bool Ft6336u::touch_active() const
{
    return gpio_get_level(config::FT6336_INT) == 0;
}

// ─────────────────────────────────────────────────────────
// 初始化与销毁
// ─────────────────────────────────────────────────────────

// 完整初始化：配 GPIO -> 复位 -> 挂 I2C 设备 -> 探测 -> 写寄存器。
// 任何一步失败都返回 false，并且把已经建好的资源清掉（deinit）。
bool Ft6336u::init(i2c_master_bus_handle_t bus_handle, const Config* config)
{
    if (bus_handle == nullptr)
    {
        ESP_LOGE(TAG, "bus_handle 为空");
        return false;
    }
    if (initialized_)
        return true;

    if (config != nullptr)
        cfg_ = *config;

    bus_handle_ = bus_handle;

    // INT 引脚配成输入 + 上拉；此时先不打开中断，等 enable_interrupt 再开。
    gpio_config_t int_cfg = {};
    int_cfg.pin_bit_mask = (1ULL << config::FT6336_INT);
    int_cfg.mode = GPIO_MODE_INPUT;
    int_cfg.pull_up_en = GPIO_PULLUP_ENABLE;
    int_cfg.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&int_cfg);

    hw_reset();

    i2c_device_config_t dev_cfg = {};
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address = kI2cAddr;
    dev_cfg.scl_speed_hz = cfg_.i2c_speed_hz;

    if (i2c_master_bus_add_device(bus_handle_, &dev_cfg, &dev_handle_) != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C 设备添加失败");
        return false;
    }

    if (cfg_.verify_chip && !probe_chip())
    {
        deinit();
        return false;
    }

    log_chip_info();

    if (!setup_base_regs())
    {
        ESP_LOGE(TAG, "基础寄存器配置失败");
        deinit();
        return false;
    }

    initialized_ = true;
    ESP_LOGD(TAG, "OK addr=0x%02X mirror=%d/%d swap=%d", kI2cAddr, cfg_.mirror_x ? 1 : 0,
             cfg_.mirror_y ? 1 : 0, cfg_.swap_xy ? 1 : 0);
    return true;
}

void Ft6336u::deinit()
{
    disable_interrupt();

    if (dev_handle_ != nullptr)
    {
        i2c_master_bus_rm_device(dev_handle_);
        dev_handle_ = nullptr;
    }
    bus_handle_ = nullptr;
    initialized_ = false;
}

// ─────────────────────────────────────────────────────────
// 读点：把寄存器内容解析成坐标
// ─────────────────────────────────────────────────────────

// 解析一个触点。寄存器里 x/y 各 12bit，跨两个字节。
// 返回 false 表示这个点是"抬起"事件，没有有效坐标。
bool Ft6336u::parse_point(const uint8_t* buf, TouchPoint& pt)
{
    const uint8_t event = (buf[0] >> 6) & 0x03;
    if (event == TOUCH_RELEASE)
        return false;

    pt.x = static_cast<uint16_t>(((buf[0] & 0x0F) << 8) | buf[1]);
    pt.y = static_cast<uint16_t>(((buf[2] & 0x0F) << 8) | buf[3]);
    pt.event = event;
    return true;
}

// 解析整包触摸数据：先看有几个触点，再逐个解析。
// FT6336U 最多支持 2 点，但我们只取第 0 个点给手势用。
bool Ft6336u::parse_touch_body(const uint8_t* body, TouchSample& sample)
{
    sample.count = 0;

    const uint8_t td_status = body[0] & 0x0F;
    if (td_status > 2)
        return true;  // 数据异常，当作没有触点

    for (uint8_t i = 0; i < td_status; i++)
    {
        const uint8_t* p = body + 1 + (static_cast<size_t>(i) * kTouchRegStride);
        TouchPoint pt = {};
        if (!parse_point(p, pt))
            continue;

        pt.id = sample.count;
        sample.points[sample.count++] = pt;
    }

    return true;
}

// 便捷查询：现在有没有手指。内部就是读一次再数有几个点。
bool Ft6336u::is_touched()
{
    if (!initialized_)
        return false;

    TouchSample sample = {};
    if (!read(sample))
        return false;
    return sample.count > 0;
}

// 正式读取触摸数据。
// 有意思的一点：这里读了两次，如果两次不一致，以第二次为准。
// 原因是触摸芯片在更新的瞬间读出来的数据可能是半新半旧的，
// 再读一次能大幅降低读到错误坐标的概率。
bool Ft6336u::read(TouchSample& sample)
{
    if (!initialized_)
        return false;

    sample = {};

    uint8_t body[kTouchBodyLen] = {};
    uint8_t body2[kTouchBodyLen] = {};

    if (!read_reg(REG_TD_STATUS, body, sizeof(body)))
        return false;

    if (read_reg(REG_TD_STATUS, body2, sizeof(body2)) &&
        std::memcmp(body, body2, sizeof(body)) != 0)
    {
        std::memcpy(body, body2, sizeof(body));
    }

    if (!parse_touch_body(body, sample))
        return false;

    // 解析出的原始坐标再做一次硬件层校正。
    for (uint8_t i = 0; i < sample.count; i++)
        map_coord(sample.points[i].x, sample.points[i].y);

    return true;
}

} // namespace app::bsp::driver::ft6336u
