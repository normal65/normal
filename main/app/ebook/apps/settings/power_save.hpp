// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/settings/power_save.hpp
 * 作用：实现省电开关、背光限制和无线功能关闭/恢复。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::settings {

/** @brief L1 省电模式（背光上限 + 关无线）；低电量阈值提示 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class PowerSave
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
    enum class Level : uint8_t
    {
        // 执行这条代码：Normal = 0,。它会更新变量、对象状态或调用链。
        Normal = 0,
        // 继续上一行的表达式或参数列表：Strong,。
        Strong,
    };

    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static PowerSave& get_instance();

    // 函数调用：void load();；执行该函数并处理它的返回值或副作用。
    void load();

    // 定义或声明函数 enabled：参数在括号里，返回类型写在函数名前面。
    bool enabled() const
    {
        // 返回 enabled_ 给调用者，表示当前函数的执行结果。
        return enabled_;
    }
    // 定义或声明函数 auto_enabled：参数在括号里，返回类型写在函数名前面。
    bool auto_enabled() const
    {
        // 返回 auto_enabled_ 给调用者，表示当前函数的执行结果。
        return auto_enabled_;
    }
    // 定义或声明函数 level：参数在括号里，返回类型写在函数名前面。
    Level level() const
    {
        // 返回 level_ 给调用者，表示当前函数的执行结果。
        return level_;
    }

    // 调用 toggle_manual()：用户点击开关时调用，通常先弹出确认提示。。
    void toggle_manual();
    // 函数调用：void set_enabled(bool on, Level level, bool from_auto);；执行该函数并处理它的返回值或副作用。
    void set_enabled(bool on, Level level, bool from_auto);
    // 调用 on_battery_pct(uint8_tpct)：根据电量应用自动策略： 低于 20%：可以进入普通省电。 低于 10%：可以进入强省电。 高于 80%：解除自动省电。。
    void on_battery_pct(uint8_t pct);

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 PowerSave() = default;，执行这个函数并处理它的返回值或状态变化。
    PowerSave() = default;

    // 调用 apply_policy()：应用当前级别的策略： 1. 限制背光。 2. 关闭 WiFi。 3. 关闭蓝牙。 4. 关闭热点。。
    void apply_policy();
    // 调用 restore_snapshot()：关闭省电时恢复快照。它恢复的是进入省电前的设置，而不是硬编码值。。
    void restore_snapshot();
    // 调用 capture_snapshot()：保存当前： 亮度。 WiFi 状态。 蓝牙状态。 热点状态。。
    void capture_snapshot();

    // 调用 prompt_enable(Levellevel)：显示开启或关闭确认。省电会关闭 WiFi 等能力，用户需要知道影响。。
    void prompt_enable(Level level);
    // 调用 prompt_disable()：显示开启或关闭确认。省电会关闭 WiFi 等能力，用户需要知道影响。。
    void prompt_disable();

    // 静态函数 on_enable_choice：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void on_enable_choice(bool accepted, void* user);
    // 静态函数 on_disable_choice：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void on_disable_choice(bool accepted, void* user);

    // 固定参数 kThreshLow20 = 20：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kThreshLow20 = 20;
    // 固定参数 kThreshLow10 = 10：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kThreshLow10 = 10;
    // 固定参数 kThreshHigh80 = 80：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kThreshHigh80 = 80;
    // 固定参数 kCapNormal = 30：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kCapNormal = 30;
    // 固定参数 kCapStrong = 15：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kCapStrong = 15;

    // 执行语句：bool enabled_{false};；它会更新一次变量、对象或程序状态。
    bool enabled_{false};
    // 执行语句：bool auto_enabled_{false};；它会更新一次变量、对象或程序状态。
    bool auto_enabled_{false};
    // 执行语句：Level level_{Level::Normal};；它会更新一次变量、对象或程序状态。
    Level level_{Level::Normal};
    // 执行语句：uint8_t last_pct_{0xFF};；它会更新一次变量、对象或程序状态。
    uint8_t last_pct_{0xFF};
    // 执行语句：bool snapshot_valid_{false};；它会更新一次变量、对象或程序状态。
    bool snapshot_valid_{false};
    // 执行语句：uint8_t snap_brightness_{50};；它会更新一次变量、对象或程序状态。
    uint8_t snap_brightness_{50};
    // 执行语句：bool snap_wifi_{false};；它会更新一次变量、对象或程序状态。
    bool snap_wifi_{false};
    // 执行语句：bool snap_bt_{false};；它会更新一次变量、对象或程序状态。
    bool snap_bt_{false};
    // 执行语句：bool snap_hotspot_{false};；它会更新一次变量、对象或程序状态。
    bool snap_hotspot_{false};
    // 执行语句：Level pending_level_{Level::Normal};；它会更新一次变量、对象或程序状态。
    Level pending_level_{Level::Normal};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::settings
