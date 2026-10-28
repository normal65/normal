// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/bsp/battery/battery.hpp
 * 作用：读取电池 ADC 电压，并按分压和放电曲线换算电量百分比。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：电压需要按分压和标定换算，不能直接当百分比。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::bsp::battery {

/**
 * @brief 锂电池 ADC 采样（TP4057 + 分压）
 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class Battery
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 调用 Battery();，执行这个函数并处理它的返回值或状态变化。
    Battery();
    // 函数调用：~Battery();；执行该函数并处理它的返回值或副作用。
    ~Battery();

    // 函数调用：bool init();；执行该函数并处理它的返回值或副作用。
    bool init();
    // 调用 deinit()：创建和销毁队列。
    void deinit();

    /** 读取电池电压 (mV)。内部 ADC 多次平均一次。 */
    // 函数调用：uint32_t read_voltage_mv();；执行该函数并处理它的返回值或副作用。
    uint32_t read_voltage_mv();

    /** 读取电量百分比 (0~100)。等价于 mv_to_percent(read_voltage_mv()) */
    // 函数调用：uint8_t read_percent();；执行该函数并处理它的返回值或副作用。
    uint8_t read_percent();

    /**
     * @brief 纯电压→百分比转换（无 ADC IO，可用于已平滑过的电压）
     *
     * 上层在做 EMA / 迟滞滤波时复用本函数避免重复读 ADC。
     */
    // 不访问 ADC，只把已经读到的电压值按电池放电曲线换算成百分比。
    static uint8_t mv_to_percent(uint32_t mv);

    // 定义或声明函数 is_init：参数在括号里，返回类型写在函数名前面。
    bool is_init() const
    {
        // 返回 initialized_ 给调用者，表示当前函数的执行结果。
        return initialized_;
    }

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 每次电压读取做 32 次 ADC 平均，降低电池电压抖动。
    static constexpr uint32_t kSampleCount = 32;  // ADC 单次读取的内部平均次数
    // 100k/100k 分压，所以 ADC 读到的电压需要乘以 2 才是电池真实电压。
    static constexpr uint32_t kDividerFactor = 2; // 100k/100k 分压比

    // 执行语句：void* adc_handle_;；它会更新一次变量、对象或程序状态。
    void* adc_handle_;
    // 执行语句：void* cali_handle_;；它会更新一次变量、对象或程序状态。
    void* cali_handle_;
    // 执行语句：bool initialized_;；它会更新一次变量、对象或程序状态。
    bool initialized_;
    // 执行语句：bool cali_ok_;；它会更新一次变量、对象或程序状态。
    bool cali_ok_;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::bsp::battery
