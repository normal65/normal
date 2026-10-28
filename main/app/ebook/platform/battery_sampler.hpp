// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/platform/battery_sampler.hpp
 * 作用：周期采样电池电压，并对电量进行平滑和状态广播。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：阻塞工作放独立任务，周期小检查放 Housekeeper。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::platform {

/**
 * @brief 电池采样（EMA K=5；下降 ≥2% / 充电上升 ≥5% 才更新；写 SystemState）
 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class BatterySampler
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static BatterySampler& get_instance();

    // 函数调用：bool init();；执行该函数并处理它的返回值或副作用。
    bool init();
    // 定义或声明函数 ready：参数在括号里，返回类型写在函数名前面。
    bool ready() const
    {
        // 返回 ready_ 给调用者，表示当前函数的执行结果。
        return ready_;
    }

    // 函数调用：uint8_t percent();；执行该函数并处理它的返回值或副作用。
    uint8_t percent();
    // 函数调用：uint32_t voltage_mv();；执行该函数并处理它的返回值或副作用。
    uint32_t voltage_mv();

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 BatterySampler() = default;，执行这个函数并处理它的返回值或状态变化。
    BatterySampler() = default;
    // 赋值：把右侧 default 的结果保存到左侧 ~BatterySampler()。
    ~BatterySampler() = default;

    // 固定参数 kEmaWeight = 5：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kEmaWeight = 5;
    // 固定参数 kDropDeltaPct = 2：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kDropDeltaPct = 2;
    // 固定参数 kChargeJumpPct = 5：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kChargeJumpPct = 5;

    // 执行语句：bool ready_{false};；它会更新一次变量、对象或程序状态。
    bool ready_{false};
    // 执行语句：bool ema_init_{false};；它会更新一次变量、对象或程序状态。
    bool ema_init_{false};
    // 执行语句：bool pct_init_{false};；它会更新一次变量、对象或程序状态。
    bool pct_init_{false};
    // 执行语句：uint32_t voltage_ema_mv_{0};；它会更新一次变量、对象或程序状态。
    uint32_t voltage_ema_mv_{0};
    // 执行语句：uint8_t stable_pct_{0};；它会更新一次变量、对象或程序状态。
    uint8_t stable_pct_{0};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::platform
