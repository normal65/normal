// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/system/power/power.hpp
 * 作用：管理轻睡眠、深睡眠和唤醒后的恢复。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：进入低功耗前要确认外设和内存状态，唤醒后需要重新校准。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <functional>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_sleep.h"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::sys::power {

/** Light / Deep Sleep 封装 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class PowerMgr
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ExitLowPowerCallback = std::function<void()>;

    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static PowerMgr& get_instance();

    // 调用 set_exit_low_power_cb(ExitLowPowerCallbackcallback)：保存一个退出轻睡眠后的回调。。
    void set_exit_low_power_cb(ExitLowPowerCallback callback);

    // 调用 enter_light_sleep()：调用 esplightsleepstart()。这个函数会返回，返回后执行 exitcallback()。。
    void enter_light_sleep();

    /** 不返回，芯片重启 */
    // 调用 enter_deep_sleep()：调用 espdeepsleepstart()。这个函数不返回，芯片会复位重启。。
    void enter_deep_sleep();

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 PowerMgr() = default;，执行这个函数并处理它的返回值或状态变化。
    PowerMgr() = default;
    // 赋值：把右侧 default 的结果保存到左侧 ~PowerMgr()。
    ~PowerMgr() = default;
    // 调用 PowerMgr(const PowerMgr&) = delete;，执行这个函数并处理它的返回值或状态变化。
    PowerMgr(const PowerMgr&) = delete;
    // 赋值：把右侧 (const PowerMgr&) = delete 的结果保存到左侧 PowerMgr& operator。
    PowerMgr& operator=(const PowerMgr&) = delete;

    // 执行语句：ExitLowPowerCallback exit_callback_;；它会更新一次变量、对象或程序状态。
    ExitLowPowerCallback exit_callback_;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::sys::power
