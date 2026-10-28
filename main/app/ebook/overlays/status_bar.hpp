// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/overlays/status_bar.hpp
 * 作用：实现 overlays 模块中的 status bar 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：覆盖层优先接收输入，打开和关闭会改变路由栈。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/geometry.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/canvas.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::overlays {

/** @brief 全局状态栏（时间 + 电量） */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class StatusBar
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static StatusBar& instance();

    // 函数调用：core::Rect bounds() const;；执行该函数并处理它的返回值或副作用。
    core::Rect bounds() const;
    // 调用 visible()：状态栏是否显示由两件事决定： 当前 Shell 页面。 如果当前是 AppHost，还要问活动 App 的 wantsstatusbar()。。
    bool visible() const;
    // 函数调用：void paint(gfx::Canvas& canvas);；执行该函数并处理它的返回值或副作用。
    void paint(gfx::Canvas& canvas);

    // 调用 on_tick_clock(uint8_th,uint8_tm)：两者只在数值变化且状态栏可见时重绘，避免后台页面推动刷新。。
    void on_tick_clock(uint8_t h, uint8_t m);
    // 调用 on_tick_battery(uint8_tpct)：两者只在数值变化且状态栏可见时重绘，避免后台页面推动刷新。。
    void on_tick_battery(uint8_t pct);
    // 调用 on_ntp_sync_done(uint8_tstatus)：处理同步完成事件，更新页面状态。它由 UI 线程消费。。
    void on_ntp_sync_done(uint8_t status);

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 StatusBar() = default;，执行这个函数并处理它的返回值或状态变化。
    StatusBar() = default;

    // 执行语句：uint8_t hour_{0};；它会更新一次变量、对象或程序状态。
    uint8_t hour_{0};
    // 执行语句：uint8_t minute_{0};；它会更新一次变量、对象或程序状态。
    uint8_t minute_{0};
    // 执行语句：uint8_t battery_pct_{0};；它会更新一次变量、对象或程序状态。
    uint8_t battery_pct_{0};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::overlays
