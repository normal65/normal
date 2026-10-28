#pragma once

/**
 * 文件：main/bsp/driver/ft6336u/ft6336u.hpp
 *
 * FT6336U 触摸控制器驱动（I2C 接口）。
 * 负责：初始化、读坐标、硬件坐标校正、开关 GPIO 中断。
 *
 * 这里定义了两类东西：
 *   - 数据结构：TouchPoint / TouchSample / Config
 *   - 驱动类：  Ft6336u
 *
 * 上层手势识别在 touch_gesture.hpp；
 * 中断 -> 任务 -> UI 的完整链路在 input_router.cc 里。
 *
 * 中断设计要点：中断只给信号量发通知，不在这里读 I2C。
 */

#include <cstddef>
#include <cstdint>

#include <driver/i2c_master.h>

namespace app::bsp::driver::ft6336u {

// FT6336U 的固定 I2C 地址（7 位）。
constexpr uint8_t kI2cAddr = 0x38;

// 触摸事件类型，来自 FT6336U 寄存器。
enum TouchEvent : uint8_t
{
    TOUCH_PRESS = 0x00,    // 按下
    TOUCH_RELEASE = 0x01,  // 抬起
    TOUCH_CONTACT = 0x02,  // 持续接触（按住）
};

// 一个触点。
struct TouchPoint
{
    uint16_t x = 0;
    uint16_t y = 0;
    uint8_t event = 0;  // TouchEvent
    uint8_t id = 0;     // 手指编号
};

// 一次触摸读取的结果：count 是有效触点数，最多 2 个。
// 本项目的手势只用 points[0]，但结构保留 2 个位置以备扩展。
struct TouchSample
{
    uint8_t count = 0;
    TouchPoint points[2] = {};
};

// 驱动配置。
struct Config
{
    uint32_t i2c_speed_hz = 400'000;  // I2C 速率，400kHz 是 FT6336U 的常用值
    uint8_t threshold = 30;           // 触摸灵敏度阈值：越大越不灵敏（防误触）
    uint8_t active_period = 0x08;     // 活跃模式下的上报周期
    bool verify_chip = false;         // 是否在 init 时做芯片 ID 探测

    // 屏幕尺寸，用于坐标裁剪。
    uint16_t panel_width = 176;
    uint16_t panel_height = 264;

    // 坐标方向校正。屏幕装反或排线折了导致坐标颠倒时改这里。
    bool swap_xy = false;
    bool mirror_x = false;
    bool mirror_y = false;

    // 中断上报方式（写 REG_REPORT_INT_MODE）。
    uint8_t report_int_mode = 0x01;
};

/**
 * FT6336U 驱动。
 *
 * 典型使用顺序：
 *   Ft6336u touch;
 *   touch.init(i2c_bus, &config);   // 建好 I2C 设备
 *   touch.enable_interrupt();       // 打开 GPIO 中断
 *   // 任务里：wait_interrupt() -> read() -> 交给手势状态机
 */
class Ft6336u
{
  public:
    Ft6336u();
    ~Ft6336u();

    Ft6336u(const Ft6336u&) = delete;
    Ft6336u& operator=(const Ft6336u&) = delete;

    // 绑定 I2C 总线并完成复位、探测、寄存器配置。config 可为空（用默认值）。
    bool init(i2c_master_bus_handle_t bus_handle, const Config* config = nullptr);
    void deinit();

    // 运行时修改坐标方向，不需要重新 init。
    void apply_transform(bool swap_xy, bool mirror_x, bool mirror_y);

    // 读一次触摸，结果放进 sample。返回 false 表示 I2C 通信失败。
    bool read(TouchSample& sample);
    // 便捷查询：现在有没有手指。
    bool is_touched();

    // ---- 中断 ----
    // 打开 GPIO 中断：芯片有触摸变化时拉低 INT，触发我们的中断服务函数。
    bool enable_interrupt();
    void disable_interrupt();

    // 任务里等待中断，最多等 timeout_ms。返回 true=等到了。
    bool wait_interrupt(uint32_t timeout_ms);
    // 不等待，只清掉已经攒下的中断标记。返回 true=清过至少一次。
    bool poll_interrupt();
    // 直接读 INT 引脚电平：有手指时是低电平。
    bool touch_active() const;

    bool is_init() const
    {
        return initialized_;
    }

    bool interrupt_enabled() const
    {
        return int_enabled_;
    }

  private:
    // FT6336U 寄存器地址。
    enum Reg : uint8_t
    {
        REG_MODE = 0x00,
        REG_TD_STATUS = 0x02,         // 触摸状态 + 触点数
        REG_THGROUP = 0x80,           // 触摸阈值
        REG_CTRL = 0x86,
        REG_PERIODACTIVE = 0x88,      // 活跃周期
        REG_CIPHER_LOW = 0xA0,        // 芯片版本（低字节）
        REG_CIPHER_HIGH = 0xA3,       // 芯片版本（高字节）
        REG_REPORT_INT_MODE = 0xA4,   // 中断上报方式
        REG_FIRMWARE_ID = 0xA6,
        REG_FOCALTECH_ID = 0xA8,      // 厂商 ID
        REG_WORK_STATE = 0xBC,
    };

    // 校验用的期望值。
    static constexpr uint8_t kCipherHighExpected = 0x64;
    static constexpr uint8_t kFocalTechId = 0x11;
    static constexpr uint8_t kWorkStateNormal = 0x01;

    // 触点在寄存器里的字节步长，以及一次读取的总长度。
    static constexpr uint8_t kTouchRegStride = 6;
    static constexpr size_t kTouchBodyLen = 13;

    void hw_reset();
    void log_chip_info();
    bool probe_chip();
    bool setup_base_regs();
    void map_coord(uint16_t& x, uint16_t& y) const;
    bool write_reg(uint8_t reg, uint8_t val);
    bool read_reg(uint8_t reg, uint8_t* buf, size_t len) const;
    static bool parse_point(const uint8_t* buf, TouchPoint& pt);
    static bool parse_touch_body(const uint8_t* body, TouchSample& sample);

    Config cfg_;
    i2c_master_bus_handle_t bus_handle_;
    i2c_master_dev_handle_t dev_handle_;
    bool initialized_;
    bool int_enabled_;
};

} // namespace app::bsp::driver::ft6336u
