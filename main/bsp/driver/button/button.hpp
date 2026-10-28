// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/bsp/driver/button/button.hpp
 * 作用：读取拨码/物理按键的 ADC 电压并产生按键事件。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：ADC 采样有抖动和区间误差，需要去抖和边界判断。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "config/config.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::bsp::driver::button {

/** 拨码位置（NONE = 未按下） */
// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class DipId : uint8_t
{
    // 执行这条代码：UP = 0,。它会更新变量、对象状态或调用链。
    UP = 0,
    // 继续上一行的表达式或参数列表：MID = 1,。
    MID = 1,
    // 继续上一行的表达式或参数列表：DOWN = 2,。
    DOWN = 2,
    // 继续上一行的表达式或参数列表：NONE = 3,。
    NONE = 3,
};

// 固定参数 kDipCount = 3：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kDipCount = 3;

/**
 * @brief 三路 DIP 拨码状态（ADC 三档互斥；未按下时全为 false）
 */
// struct DipState：把相关数据字段组合在一起，表示项目中的一个概念。
struct DipState
{
    // 执行语句：bool up;；它会更新一次变量、对象或程序状态。
    bool up;
    // 执行语句：bool mid;；它会更新一次变量、对象或程序状态。
    bool mid;
    // 执行语句：bool down;；它会更新一次变量、对象或程序状态。
    bool down;
};

/**
 * @brief ADC 拨码开关驱动
 *
 * 单 ADC 引脚（config::DIP_SWITCH）通过分压电阻区分三档按键；
 * 未按下时引脚被拉高至约 3.3V。
 */
// struct Config：把相关数据字段组合在一起，表示项目中的一个概念。
struct Config
{
    // 定义变量 uint32_t threshold_up_mid_mv 并初始化为 config::DIP_SWITCH_THRESHOLD_UP_MID_MV。
    uint32_t threshold_up_mid_mv = config::DIP_SWITCH_THRESHOLD_UP_MID_MV;
    // 定义变量 uint32_t threshold_mid_down_mv 并初始化为 config::DIP_SWITCH_THRESHOLD_MID_DOWN_MV。
    uint32_t threshold_mid_down_mv = config::DIP_SWITCH_THRESHOLD_MID_DOWN_MV;
    // 定义变量 uint32_t threshold_released_mv 并初始化为 config::DIP_SWITCH_THRESHOLD_RELEASED_MV。
    uint32_t threshold_released_mv = config::DIP_SWITCH_THRESHOLD_RELEASED_MV;
    // 定义变量 uint32_t sample_count 并初始化为 8。
    uint32_t sample_count = 8;
};

// 类定义：把数据和操作这些数据的方法绑定在一起。
class DipSwitch
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 调用 DipSwitch();，执行这个函数并处理它的返回值或状态变化。
    DipSwitch();
    // 函数调用：~DipSwitch();；执行该函数并处理它的返回值或副作用。
    ~DipSwitch();

    // 赋值：把右侧 nullptr) 的结果保存到左侧 bool init(const Config* config。
    bool init(const Config* config = nullptr);
    // 调用 deinit()：创建和销毁队列。
    void deinit();

    /** 读取当前档位 */
    // 调用 read()：read() 从寄存器读出触摸状态和最多两个触点，解析成 TouchSample。istouched() 是更简单的查询，用于判断当前是否仍有手指接触。。
    DipState read() const;

    /** 当前是否处于指定档位 */
    // 函数调用：bool read(DipId id) const;；执行该函数并处理它的返回值或副作用。
    bool read(DipId id) const;

    /**
     * @brief 读取 3-bit 掩码
     * @return bit0=up, bit1=mid, bit2=down；位为 1 表示当前档位
     */
    // 函数调用：uint8_t read_mask() const;；执行该函数并处理它的返回值或副作用。
    uint8_t read_mask() const;

    /** 读取 ADC 电压 (mV)，便于标定阈值 */
    // 函数调用：uint32_t read_voltage_mv() const;；执行该函数并处理它的返回值或副作用。
    uint32_t read_voltage_mv() const;

    // 定义或声明函数 is_init：参数在括号里，返回类型写在函数名前面。
    bool is_init() const
    {
        // 返回 initialized_ 给调用者，表示当前函数的执行结果。
        return initialized_;
    }

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 函数调用：DipId read_position() const;；执行该函数并处理它的返回值或副作用。
    DipId read_position() const;

    // 执行语句：Config cfg_;；它会更新一次变量、对象或程序状态。
    Config cfg_;
    // 执行语句：void* adc_handle_;；它会更新一次变量、对象或程序状态。
    void* adc_handle_;
    // 执行语句：void* cali_handle_;；它会更新一次变量、对象或程序状态。
    void* cali_handle_;
    // 执行语句：int adc_unit_;；它会更新一次变量、对象或程序状态。
    int adc_unit_;
    // 执行语句：int adc_channel_;；它会更新一次变量、对象或程序状态。
    int adc_channel_;
    // 执行语句：bool initialized_;；它会更新一次变量、对象或程序状态。
    bool initialized_;
    // 执行语句：bool cali_ok_;；它会更新一次变量、对象或程序状态。
    bool cali_ok_;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::bsp::driver::button
