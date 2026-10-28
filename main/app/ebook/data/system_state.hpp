// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/data/system_state.hpp
 * 作用：保存亮度、音量、Wi-Fi、蓝牙、热点等全局运行时状态。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：小配置写 NVS，运行时共享状态放 SystemState。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::data {

/**
 * @brief 系统运行时状态（UI 任务持有；setter 写 NVS；亮度/音量/夜间模式可注册观察者）
 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class SystemState
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ValueObserver = void (*)(uint8_t v);
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using BoolObserver = void (*)(bool v);

    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static SystemState& get_instance();

    // 函数调用：void load();；执行该函数并处理它的返回值或副作用。
    void load();
    // 调用 save()：这是拨码映射的持久化表： load() 先设默认值，再用 NVS 覆盖合法值。 save() 把所有槽位写入 NVS 并 commit。 lookup() 按 Context 和 Key 找动作。 cycle() 在设置页循环切换到下一个可选动作。。
    void save();

    // 定义或声明函数 set_brightness_observer：参数在括号里，返回类型写在函数名前面。
    void set_brightness_observer(ValueObserver cb)
    {
        // 赋值：把右侧 cb 的结果保存到左侧 bright_cb_。
        bright_cb_ = cb;
    }
    // 定义或声明函数 set_volume_observer：参数在括号里，返回类型写在函数名前面。
    void set_volume_observer(ValueObserver cb)
    {
        // 赋值：把右侧 cb 的结果保存到左侧 volume_cb_。
        volume_cb_ = cb;
    }
    // 定义或声明函数 set_night_mode_observer：参数在括号里，返回类型写在函数名前面。
    void set_night_mode_observer(BoolObserver cb)
    {
        // 赋值：把右侧 cb 的结果保存到左侧 night_cb_。
        night_cb_ = cb;
    }

    // 定义或声明函数 wifi：参数在括号里，返回类型写在函数名前面。
    bool wifi() const
    {
        // 返回 wifi_ 给调用者，表示当前函数的执行结果。
        return wifi_;
    }
    // 定义或声明函数 bluetooth：参数在括号里，返回类型写在函数名前面。
    bool bluetooth() const
    {
        // 返回 bluetooth_ 给调用者，表示当前函数的执行结果。
        return bluetooth_;
    }
    // 定义或声明函数 hotspot：参数在括号里，返回类型写在函数名前面。
    bool hotspot() const
    {
        // 返回 hotspot_ 给调用者，表示当前函数的执行结果。
        return hotspot_;
    }
    // 定义或声明函数 night_mode：参数在括号里，返回类型写在函数名前面。
    bool night_mode() const
    {
        // 返回 night_mode_ 给调用者，表示当前函数的执行结果。
        return night_mode_;
    }
    // 定义或声明函数 mute：参数在括号里，返回类型写在函数名前面。
    bool mute() const
    {
        // 返回 mute_ 给调用者，表示当前函数的执行结果。
        return mute_;
    }

    // 函数调用：void set_wifi(bool v);；执行该函数并处理它的返回值或副作用。
    void set_wifi(bool v);
    // 函数调用：void set_bluetooth(bool v);；执行该函数并处理它的返回值或副作用。
    void set_bluetooth(bool v);
    // 函数调用：void set_hotspot(bool v);；执行该函数并处理它的返回值或副作用。
    void set_hotspot(bool v);
    // 函数调用：void set_night_mode(bool v);；执行该函数并处理它的返回值或副作用。
    void set_night_mode(bool v);
    // 函数调用：void set_mute(bool v);；执行该函数并处理它的返回值或副作用。
    void set_mute(bool v);

    // 函数定义/声明：调用 brightness()：把 0 到 100 的亮度限制在合法范围后交给背光驱动。墨水屏本身不依赖背光，背光是前光/照明系统，不能把亮度当成画面灰度。。
    uint8_t brightness() const
    {
        // 返回 brightness_ 给调用者，表示当前函数的执行结果。
        return brightness_;
    }
    // 定义或声明函数 volume：参数在括号里，返回类型写在函数名前面。
    uint8_t volume() const
    {
        // 返回 volume_ 给调用者，表示当前函数的执行结果。
        return volume_;
    }

    // 调用 set_brightness(uint8_tv)：把 0 到 100 的亮度限制在合法范围后交给背光驱动。墨水屏本身不依赖背光，背光是前光/照明系统，不能把亮度当成画面灰度。。
    void set_brightness(uint8_t v);
    // 函数调用：void set_volume(uint8_t v);；执行该函数并处理它的返回值或副作用。
    void set_volume(uint8_t v);

    // 定义或声明函数 battery_pct：参数在括号里，返回类型写在函数名前面。
    uint8_t battery_pct() const
    {
        // 返回 battery_pct_ 给调用者，表示当前函数的执行结果。
        return battery_pct_;
    }
    // 定义或声明函数 battery_mv：参数在括号里，返回类型写在函数名前面。
    uint32_t battery_mv() const
    {
        // 返回 battery_mv_ 给调用者，表示当前函数的执行结果。
        return battery_mv_;
    }
    // 函数调用：void set_battery(uint8_t pct, uint32_t mv);；执行该函数并处理它的返回值或副作用。
    void set_battery(uint8_t pct, uint32_t mv);

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 SystemState() = default;，执行这个函数并处理它的返回值或状态变化。
    SystemState() = default;

    // 执行语句：ValueObserver bright_cb_{nullptr};；它会更新一次变量、对象或程序状态。
    ValueObserver bright_cb_{nullptr};
    // 执行语句：ValueObserver volume_cb_{nullptr};；它会更新一次变量、对象或程序状态。
    ValueObserver volume_cb_{nullptr};
    // 执行语句：BoolObserver night_cb_{nullptr};；它会更新一次变量、对象或程序状态。
    BoolObserver night_cb_{nullptr};

    // 执行语句：bool wifi_{false};；它会更新一次变量、对象或程序状态。
    bool wifi_{false};
    // 执行语句：bool bluetooth_{false};；它会更新一次变量、对象或程序状态。
    bool bluetooth_{false};
    // 执行语句：bool hotspot_{false};；它会更新一次变量、对象或程序状态。
    bool hotspot_{false};
    // 执行语句：bool night_mode_{false};；它会更新一次变量、对象或程序状态。
    bool night_mode_{false};
    // 执行语句：bool mute_{false};；它会更新一次变量、对象或程序状态。
    bool mute_{false};
    // 执行语句：uint8_t brightness_{50};；它会更新一次变量、对象或程序状态。
    uint8_t brightness_{50};
    // 执行语句：uint8_t volume_{50};；它会更新一次变量、对象或程序状态。
    uint8_t volume_{50};
    // 执行语句：uint8_t battery_pct_{0};；它会更新一次变量、对象或程序状态。
    uint8_t battery_pct_{0};
    // 执行语句：uint32_t battery_mv_{0};；它会更新一次变量、对象或程序状态。
    uint32_t battery_mv_{0};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::data
