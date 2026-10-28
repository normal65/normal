/**
 * 文件：main/bsp/driver/gdey027t91/gdey027t91.cc
 *
 * GDEY027T91 墨水屏驱动（SSD1680 兼容控制器），通过 4 线 SPI 通信。
 *
 * 面板 FPC 引脚 -> ESP32 GPIO：
 *   FPC 13 SCL  -> GPIO46 SCLK
 *   FPC 14 SDA  -> GPIO3  MOSI
 *   FPC 12 CS#  -> GPIO9  CS
 *   FPC 11 D/C# -> GPIO10 DC
 *   FPC 10 RES# -> GPIO11 RST
 *   FPC 9  BUSY -> GPIO12 BUSY
 *
 * 启动顺序：
 *   hw_reset()
 *   -> sw_reset()          发 0x12
 *   -> configure_panel()   0x01 / 0x18 / 0x3C / 0x11 / 0x44 / 0x45 / 0x4E / 0x4F
 *   -> power_on()          0x22 + 0xE0, 0x20
 *   -> 写 0x24 / 0x26
 *   -> trigger_update()    0x22 + 0xF7 (Full), 0x20
 *   -> power_off()         0x22 + 0x83, 0x20
 *
 * 并发约束：所有 SPI 访问都要先拿 BusGuard（总线互斥量），
 * 避免刷新、休眠和初始化互相打断。
 */

#include "gdey027t91.hpp"

#include <algorithm>
#include <cstring>

#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_heap_caps.h>
#include <esp_log.h>

#include "config/config.hpp"
#include "ssd1680.hpp"
#include "system/task/task.hpp"

static const char* const TAG = "GDEY027T91";

namespace app::bsp::driver::gdey027t91 {

using Cmd = ssd1680::Cmd;
using Seq = ssd1680::Seq;
using Param = ssd1680::Param;

// SPI 设备句柄的简单封装，避免头文件里暴露 ESP-IDF 类型。
struct Gdey027t91::Spi
{
    spi_device_handle_t handle = nullptr;
};

namespace {

// 屏幕接在 SPI2 主机上。
constexpr spi_host_device_t kSpiHost = SPI2_HOST;
// 20MHz 是 SSD1680 手册写模式的上限。
constexpr int kSpiClockHz = 20'000'000;
// 单次 DMA 传输的最大字节数，整屏 5808 字节会拆成两块发送。
constexpr uint32_t kDmaChunkMax = 4096;

} // namespace

// BusGuard 是一个 RAII 锁：
// 构造时尝试拿总线互斥量，析构时自动释放 CS 和互斥量。
// 用 try_lock 而不是阻塞 lock，避免刷新任务和休眠任务互相卡死。
Gdey027t91::BusGuard::BusGuard(Gdey027t91& dev)
    : dev_(dev)
    , acquired_(dev.bus_mutex_.try_lock())
{
    if (!acquired_)
        ESP_LOGW(TAG, "总线占用，跳过本次操作");
}

Gdey027t91::BusGuard::~BusGuard()
{
    if (!acquired_)
        return;
    // 如果函数中途退出时 CS 还拉着，这里兜底释放。
    if (dev_.cs_active_)
        dev_.cs_release();
    dev_.bus_mutex_.unlock();
}

Gdey027t91::Gdey027t91()
    : spi_(nullptr)
    , framebuffer_(nullptr)
    , initialized_(false)
    , powered_(false)
    , base_ok_(false)
    , cs_active_(false)
{
}

Gdey027t91::~Gdey027t91()
{
    deinit();
}

// 初始化整个屏幕驱动：
//   1. 配置 DC / RST / CS / BUSY
//   2. 初始化 SPI
//   3. 分配整屏 framebuffer
//   4. 硬件复位 + 软件复位
//   5. 写基础寄存器
bool Gdey027t91::init()
{
    if (initialized_)
        return true;

    // DC / RST / CS 是 ESP32 输出给屏幕的控制脚。
    gpio_config_t out_cfg = {};
    out_cfg.pin_bit_mask = (1ULL << config::GDEY027T91_DC) |
                           (1ULL << config::GDEY027T91_RST) |
                           (1ULL << config::GDEY027T91_CS);
    out_cfg.mode = GPIO_MODE_OUTPUT;
    if (gpio_config(&out_cfg) != ESP_OK)
    {
        ESP_LOGE(TAG, "GPIO 输出配置失败");
        return false;
    }

    // BUSY 是屏幕输出给 ESP32 的状态脚。
    gpio_config_t in_cfg = {};
    in_cfg.pin_bit_mask = (1ULL << config::GDEY027T91_BUSY);
    in_cfg.mode = GPIO_MODE_INPUT;
    if (gpio_config(&in_cfg) != ESP_OK)
    {
        ESP_LOGE(TAG, "BUSY GPIO 配置失败");
        return false;
    }
    // 先取消片选，表示“还没开始和屏幕通信”。
    gpio_set_level(config::GDEY027T91_CS, 1);

    // 初始化 SPI2：只写不读，所以 MISO 设为 -1。
    spi_bus_config_t bus_cfg = {};
    bus_cfg.mosi_io_num = config::GDEY027T91_MOSI;
    bus_cfg.miso_io_num = -1;
    bus_cfg.sclk_io_num = config::GDEY027T91_SCLK;
    bus_cfg.quadwp_io_num = -1;
    bus_cfg.quadhd_io_num = -1;
    bus_cfg.max_transfer_sz = static_cast<int>(kDmaChunkMax);

    const esp_err_t err = spi_bus_initialize(kSpiHost, &bus_cfg, SPI_DMA_CH_AUTO);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE(TAG, "SPI bus init 失败: %s", esp_err_to_name(err));
        return false;
    }

    // 添加 SPI 从设备：Mode 0、20MHz、半双工。
    // CS 不用硬件 SPI 片选，而是由驱动手动控制 GPIO。
    auto* spi = new Spi();
    spi_device_interface_config_t dev_cfg = {};
    dev_cfg.clock_speed_hz = kSpiClockHz;
    dev_cfg.mode = 0;
    dev_cfg.spics_io_num = -1;
    dev_cfg.queue_size = 1;
    dev_cfg.flags = SPI_DEVICE_HALFDUPLEX;
    if (spi_bus_add_device(kSpiHost, &dev_cfg, &spi->handle) != ESP_OK)
    {
        ESP_LOGE(TAG, "SPI add_device 失败");
        delete spi;
        return false;
    }
    spi_ = spi;

    // 分配整屏 framebuffer：176×264 黑白，1 位一个像素，共 5808 字节。
    framebuffer_ = static_cast<uint8_t*>(heap_caps_malloc(BUFFER_SIZE, MALLOC_CAP_DMA));
    if (framebuffer_ == nullptr)
    {
        ESP_LOGE(TAG, "帧缓冲分配失败");
        deinit();
        return false;
    }
    // 0xFF 表示全白，避免上电时出现随机雪花。
    std::memset(framebuffer_, 0xFF, BUFFER_SIZE);

    hw_reset();
    if (!sw_reset())
    {
        deinit();
        return false;
    }
    configure_panel();
    drop_session();

    initialized_ = true;
    ESP_LOGD(TAG, "OK %ux%u SPI=%dMHz", WIDTH, HEIGHT, kSpiClockHz / 1000000);
    return true;
}

// 按相反顺序释放资源：先让屏幕进深睡眠，再释放 framebuffer 和 SPI。
void Gdey027t91::deinit()
{
    if (initialized_)
        sleep();

    if (framebuffer_ != nullptr)
    {
        heap_caps_free(framebuffer_);
        framebuffer_ = nullptr;
    }
    if (spi_ != nullptr)
    {
        if (spi_->handle != nullptr)
            spi_bus_remove_device(spi_->handle);
        delete spi_;
        spi_ = nullptr;
    }

    initialized_ = false;
    drop_session();
    cs_active_ = false;
}

// 只改内存里的 framebuffer，不立即上屏。
// 真正的上屏由 present_* 负责。
void Gdey027t91::fill(uint8_t color)
{
    if (framebuffer_ != nullptr)
        std::memset(framebuffer_, color, BUFFER_SIZE);
}

// 统一检查初始化状态，避免空指针进入 SPI 和内存操作。
bool Gdey027t91::ensure_hw() const
{
    if (!initialized_ || spi_ == nullptr || framebuffer_ == nullptr)
    {
        ESP_LOGE(TAG, "设备未初始化");
        return false;
    }
    return true;
}

// 让电源状态和局刷基线全部失效。
// 下次刷新前必须重新上电 / 重建基线。
void Gdey027t91::drop_session()
{
    powered_ = false;
    base_ok_ = false;
}

void Gdey027t91::invalidate_session()
{
    drop_session();
}

// 手动拉低 CS，选中屏幕。
// 重复调用是安全的：已经拉低就不重复操作。
void Gdey027t91::cs_assert()
{
    if (cs_active_)
        return;
    gpio_set_level(config::GDEY027T91_CS, 0);
    cs_active_ = true;
}

// 手动拉高 CS，结束本次传输。
void Gdey027t91::cs_release()
{
    if (!cs_active_)
        return;
    gpio_set_level(config::GDEY027T91_CS, 1);
    cs_active_ = false;
}

// 发一个命令字节：
//   CS 拉低 -> D/C# 拉低（表示命令）-> 发 8 位 -> 保持 CS，等后续 dat()
void Gdey027t91::cmd(uint8_t c)
{
    if (spi_ == nullptr || spi_->handle == nullptr)
        return;
    cs_assert();
    gpio_set_level(config::GDEY027T91_DC, 0);

    spi_transaction_t tx = {};
    tx.length = 8;
    tx.tx_buffer = &c;
    spi_device_polling_transmit(spi_->handle, &tx);
}

// 发一个数据字节：
//   CS 已经拉低 -> D/C# 拉高（表示数据）-> 发 8 位
void Gdey027t91::dat(uint8_t d)
{
    if (spi_ == nullptr || spi_->handle == nullptr)
        return;
    cs_assert();
    gpio_set_level(config::GDEY027T91_DC, 1);

    spi_transaction_t tx = {};
    tx.length = 8;
    tx.tx_buffer = &d;
    spi_device_polling_transmit(spi_->handle, &tx);
}

// 连续发一大块数据（例如整屏 5808 字节）。
// 按 kDmaChunkMax 分块，避免单次 DMA 传输过大。
void Gdey027t91::dat_bulk(const uint8_t* buf, uint32_t len)
{
    if (spi_ == nullptr || spi_->handle == nullptr || buf == nullptr || len == 0)
        return;
    cs_assert();
    gpio_set_level(config::GDEY027T91_DC, 1);

    uint32_t off = 0;
    while (off < len)
    {
        const uint32_t chunk = std::min(len - off, kDmaChunkMax);
        spi_transaction_t tx = {};
        tx.length = chunk * 8;
        tx.tx_buffer = buf + off;
        spi_device_polling_transmit(spi_->handle, &tx);
        off += chunk;
    }
}

// 硬件复位：RES# 拉低 10ms，再拉高 10ms。
// 面板 FPC 10 脚 RES# -> GPIO11。
// 手册只明确写了“复位后 Wait 10ms”，低电平 10ms 是工程保守值。
void Gdey027t91::hw_reset()
{
    gpio_set_level(config::GDEY027T91_RST, 0);
    ::app::sys::task::TaskMgr::delay_ms(Panel::RESET_SETTLE_MS);
    gpio_set_level(config::GDEY027T91_RST, 1);
    ::app::sys::task::TaskMgr::delay_ms(Panel::RESET_SETTLE_MS);
}

// BUSY 高电平表示屏幕还在忙。
// 面板 FPC 9 脚 BUSY -> GPIO12。
bool Gdey027t91::is_busy() const
{
    return gpio_get_level(config::GDEY027T91_BUSY) == 1;
}

// 等 BUSY 变低，最多等 timeout_ms。
// 前 50ms 用 1ms 轮询，之后每 10ms 轮询一次，减少 CPU 占用。
bool Gdey027t91::wait_busy(uint32_t timeout_ms) const
{
    uint32_t elapsed = 0;
    while (is_busy())
    {
        if (elapsed >= timeout_ms)
        {
            ESP_LOGW(TAG, "BUSY 超时 %u ms", static_cast<unsigned>(timeout_ms));
            return false;
        }
        const uint32_t step = (elapsed < 50) ? 1 : 10;
        ::app::sys::task::TaskMgr::delay_ms(step);
        elapsed += step;
    }
    return true;
}

// 软件复位：发 0x12，等 BUSY，再等 10ms。
// 0x12 定义在 ssd1680.hpp 的 Cmd::kSwReset。
bool Gdey027t91::sw_reset()
{
    cmd(Cmd::kSwReset);
    cs_release();

    if (!wait_busy(Panel::BUSY_TIMEOUT_CMD_MS))
        return false;

    ::app::sys::task::TaskMgr::delay_ms(Panel::RESET_SETTLE_MS);
    return true;
}

// 设置 RAM 窗口：
//   0x44 设 X 起止（单位是 8 像素）
//   0x45 设 Y 起止（单位是 1 行）
void Gdey027t91::set_ram_area(uint8_t x_start, uint8_t x_end, uint8_t y_start, uint8_t y_start_hi,
                              uint8_t y_end, uint8_t y_end_hi)
{
    cmd(Cmd::kSetRamX);
    dat(x_start);
    dat(x_end);
    cmd(Cmd::kSetRamY);
    dat(y_start);
    dat(y_start_hi);
    dat(y_end);
    dat(y_end_hi);
}

// 把 RAM 写指针移到指定位置：
//   0x4E 设 X 地址
//   0x4F 设 Y 地址（9 位，分两次发）
void Gdey027t91::set_ram_cursor(uint16_t x_byte, uint16_t y)
{
    cmd(Cmd::kSetRamXCounter);
    dat(static_cast<uint8_t>(x_byte & 0xFF));
    cmd(Cmd::kSetRamYCounter);
    dat(static_cast<uint8_t>(y & 0xFF));
    dat(static_cast<uint8_t>((y >> 8) & 0x01));
}

// 把画布窗口设定为“全屏”，并把画笔归位到左上角 (0,0)。
//   0x11 数据入口模式：X 递增、Y 递增
//   0x44 X 范围 0 ~ 21（22 个字节）
//   0x45 Y 范围 0 ~ 263
//   0x4E/0x4F 光标回到 (0,0)
void Gdey027t91::set_full_ram_entry()
{
    const uint16_t y_end = static_cast<uint16_t>(HEIGHT - 1U);

    cmd(Cmd::kDataEntryMode);
    dat(Param::kDataEntryNormal);

    set_ram_area(0, Panel::RAM_X_END, 0, 0, static_cast<uint8_t>(y_end & 0xFF),
                 static_cast<uint8_t>((y_end >> 8) & 0x01));
    set_ram_cursor(0, 0);
}

// 设置边框波形（0x3C）。0x05 是常规边框设置。
void Gdey027t91::set_border_waveform(uint8_t value)
{
    cmd(Cmd::kBorderWaveform);
    dat(value);
    cs_release();
}

// 一次性配置控制器：
//   0x01 设驱动输出行数（264 行）
//   0x18 使用内部温度传感器
//   0x3C 设边框波形
//   0x11/0x44/0x45/0x4E/0x4F 设整屏 RAM 区域
void Gdey027t91::configure_panel()
{
    // 0x01：MUX = HEIGHT - 1 = 263，表示 264 行。
    cmd(Cmd::kDriverOutputControl);
    dat(static_cast<uint8_t>((HEIGHT - 1U) & 0xFF));
    dat(static_cast<uint8_t>(((HEIGHT - 1U) >> 8) & 0x01));
    dat(0x00);

    // 0x18：0x80 表示使用内部温度传感器。
    cmd(Cmd::kTempSensor);
    dat(Param::kTempInternal);

    set_border_waveform(Param::kBorderNormal);
    // 把画布窗口设成全屏（176×264），并把画笔归位到左上角 (0,0)。
    set_full_ram_entry();
    cs_release();
}

// 把局刷窗口换算成 RAM 地址并设置光标。
// X 地址以 8 像素为单位，所以先除以 8。
void Gdey027t91::apply_window(const Window& win)
{
    const uint16_t x_byte_start = static_cast<uint16_t>(win.x / 8U);
    const uint16_t x_byte_end = static_cast<uint16_t>(((win.x + win.w) / 8U) - 1U);
    const uint16_t y_end = static_cast<uint16_t>(win.y + win.h - 1U);

    set_ram_area(static_cast<uint8_t>(x_byte_start & 0xFF), static_cast<uint8_t>(x_byte_end & 0xFF),
                 static_cast<uint8_t>(win.y & 0xFF), static_cast<uint8_t>((win.y >> 8) & 0x01),
                 static_cast<uint8_t>(y_end & 0xFF), static_cast<uint8_t>((y_end >> 8) & 0x01));
    set_ram_cursor(x_byte_start, win.y);
}

// 把整块 framebuffer 写进控制器 RAM。
// cmd_id 用 0x24（当前帧）或 0x26（上一帧）。
void Gdey027t91::write_ram_full(uint8_t cmd_id, const uint8_t* src)
{
    if (src == nullptr || spi_ == nullptr)
        return;

    set_ram_cursor(0, 0);
    cmd(cmd_id);
    dat_bulk(src, BUFFER_SIZE);
    cs_release();
}

// 只写窗口区域。
// region_tl 指向窗口左上角在整屏 framebuffer 里的位置。
void Gdey027t91::write_ram_window(uint8_t cmd_id, const Window& win, const uint8_t* region_tl)
{
    if (region_tl == nullptr)
        return;

    const uint32_t row_bytes = static_cast<uint32_t>(win.w / 8U);
    cmd(cmd_id);
    for (uint16_t row = 0; row < win.h; row++)
        dat_bulk(region_tl + (static_cast<size_t>(row) * STRIDE), row_bytes);
    cs_release();
}

// 设置快刷用的 booster 时序（0x1A + 0x64）。
void Gdey027t91::set_fast_booster_timing()
{
    cmd(0x1A);
    dat(Param::kBoosterTiming);
    cs_release();
}

// 给屏幕内部升压电路上电：0x22 + 0xE0，再发 0x20 触发。
// 上电后才能写 RAM 和刷新。
bool Gdey027t91::power_on()
{
    if (powered_)
        return true;

    cmd(Cmd::kDisplayUpdateCtrl1);
    dat(Seq::kPowerOn);
    cmd(Cmd::kDisplayUpdateCtrl2);
    cs_release();
    if (!wait_busy(Panel::BUSY_TIMEOUT_CMD_MS))
        return false;

    powered_ = true;
    return true;
}

// 关闭屏幕内部升压电路：0x22 + 0x83，再发 0x20 触发。
bool Gdey027t91::power_off()
{
    if (!powered_)
        return true;

    cmd(Cmd::kDisplayUpdateCtrl1);
    dat(Seq::kPowerOff);
    cmd(Cmd::kDisplayUpdateCtrl2);
    cs_release();
    if (!wait_busy(Panel::BUSY_TIMEOUT_CMD_MS))
        return false;

    powered_ = false;
    return true;
}

// 触发一次刷新：
//   0x22 + sequence 选择波形（F7=Full / D7=Fast / FC=Partial）
//   0x20 让控制器开始执行
//   然后等 BUSY 变低，刷新完成。
bool Gdey027t91::trigger_update(uint8_t sequence, uint32_t busy_timeout_ms)
{
    cmd(Cmd::kDisplayUpdateCtrl1);
    dat(sequence);
    cmd(Cmd::kDisplayUpdateCtrl2);
    cs_release();
    return wait_busy(busy_timeout_ms);
}

bool Gdey027t91::ensure_power()
{
    return power_on();
}

// 冷启动：复位 + 重新配置 + 上电。
// 用在屏幕状态不确定、或者从深睡眠唤醒后。
bool Gdey027t91::cold_boot()
{
    hw_reset();
    if (!sw_reset())
        return false;
    configure_panel();
    drop_session();
    return ensure_power();
}

// 建立第一张局刷基线：
//   1. 上电
//   2. 写当前帧 0x24 和上一帧 0x26（内容相同）
//   3. 用 Fast 波形刷一次
bool Gdey027t91::present_base()
{
    if (!ensure_power())
        return false;

    set_border_waveform(Param::kBorderNormal);
    // 把画布窗口设成全屏，并把画笔归位到左上角 (0,0)。
    set_full_ram_entry();
    write_ram_full(Cmd::kWriteRamBw, framebuffer_);
    write_ram_full(Cmd::kWriteRamBwPrev, framebuffer_);

    set_fast_booster_timing();
    if (!trigger_update(Seq::kFast, Panel::BUSY_TIMEOUT_FAST_MS))
        return false;

    base_ok_ = true;
    return true;
}

// 最稳妥的全屏刷新：
//   上电 -> 写当前帧/上一帧 -> 用 Full 波形刷 -> 断电
bool Gdey027t91::present_full()
{
    if (!wait_busy(Panel::BUSY_TIMEOUT_CMD_MS))
        return false;
    if (!ensure_power())
        return false;

    set_border_waveform(Param::kBorderNormal);
    // 把画布窗口设成全屏，并把画笔归位到左上角 (0,0)。
    set_full_ram_entry();
    write_ram_full(Cmd::kWriteRamBw, framebuffer_);
    write_ram_full(Cmd::kWriteRamBwPrev, framebuffer_);

    if (!trigger_update(Seq::kFull, Panel::BUSY_TIMEOUT_FULL_MS))
        return false;

    (void)power_off();
    base_ok_ = true;
    return true;
}

// 快速刷新整屏。
// 如果没有基线，先自动建立基线。
bool Gdey027t91::present_fast()
{
    if (!base_ok_)
        return present_base();

    if (!wait_busy(Panel::BUSY_TIMEOUT_CMD_MS))
        return false;
    if (!ensure_power())
        return false;

    set_border_waveform(Param::kBorderNormal);
    // 把画布窗口设成全屏，并把画笔归位到左上角 (0,0)。
    set_full_ram_entry();
    write_ram_full(Cmd::kWriteRamBw, framebuffer_);

    set_fast_booster_timing();
    if (!trigger_update(Seq::kFast, Panel::BUSY_TIMEOUT_FAST_MS))
        return false;

    // 刷新完成后把当前帧存成“上一帧”，供下次局刷比较。
    write_ram_full(Cmd::kWriteRamBwPrev, framebuffer_);

    base_ok_ = true;
    return true;
}

// 局部刷新指定窗口。
// 注意：当前实现仍然依赖整屏基线 base_ok_。
bool Gdey027t91::present_partial(const Window& win)
{
    if (!base_ok_)
        return false;

    if (!wait_busy(Panel::BUSY_TIMEOUT_CMD_MS))
        return false;
    if (!ensure_power())
        return false;

    set_border_waveform(Param::kBorderNormal);
    apply_window(win);

    // 找到窗口左上角在整屏 framebuffer 里的地址。
    const uint8_t* region_tl =
        framebuffer_ + (static_cast<uint32_t>(win.y) * STRIDE) + (win.x / 8U);

    write_ram_window(Cmd::kWriteRamBw, win, region_tl);
    if (!trigger_update(Seq::kPartial, Panel::BUSY_TIMEOUT_PARTIAL_MS))
        return false;

    // 局刷完成后同步“上一帧”，供下一次局刷比较。
    apply_window(win);
    write_ram_window(Cmd::kWriteRamBwPrev, win, region_tl);

    // 局刷结束后把窗口恢复成全屏，光标也归零。
    set_full_ram_entry();
    cs_release();
    return true;
}

// 无窗口版本：只允许 Base / Fast / Full。
// Partial 必须有矩形窗口，所以这里直接报错。
bool Gdey027t91::present(Present kind)
{
    if (!ensure_hw())
        return false;

    BusGuard lock(*this);
    if (!lock.ok())
        return false;

    switch (kind)
    {
        case Present::Base:
            return present_base();
        case Present::Fast:
            return present_fast();
        case Present::Full:
            return present_full();
        case Present::Partial:
            ESP_LOGE(TAG, "Partial 需要窗口参数");
            return false;
    }
    return false;
}

// 带窗口版本：先把窗口裁剪并对齐到 8 像素，再走局刷。
bool Gdey027t91::present(Present kind, Window win)
{
    if (!ensure_hw())
        return false;

    if (!win.normalize())
        return false;

    BusGuard lock(*this);
    if (!lock.ok())
        return false;

    if (kind != Present::Partial)
    {
        ESP_LOGE(TAG, "present(%d) 不接受窗口", static_cast<int>(kind));
        return false;
    }
    return present_partial(win);
}

// 进入深睡眠：
//   先断电 -> 发 0x10 + 0x01 -> 等 100ms -> 丢弃 session。
// 深睡眠后不能再发普通命令，必须重新 wake()。
void Gdey027t91::sleep()
{
    if (!initialized_)
        return;

    BusGuard lock(*this);
    if (!lock.ok())
        return;

    power_off();
    cmd(Cmd::kDeepSleep);
    dat(Param::kDeepSleepMode);
    cs_release();
    ::app::sys::task::TaskMgr::delay_ms(100);
    drop_session();
}

// 从深睡眠唤醒：重新复位 + 重新配置 + 丢弃旧基线。
void Gdey027t91::wake()
{
    if (!initialized_)
        return;

    BusGuard lock(*this);
    if (!lock.ok())
        return;

    hw_reset();
    if (sw_reset())
        configure_panel();
    drop_session();
}

} // namespace app::bsp::driver::gdey027t91
