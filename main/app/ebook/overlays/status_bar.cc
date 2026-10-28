/**
 * [初学者导读] 文件：main/app/ebook/overlays/status_bar.cc
 * 作用：实现 overlays 模块中的 status bar 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：覆盖层优先接收输入，打开和关闭会改变路由栈。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/status_bar.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app_registry.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/clock_provider.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/system_state.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "protocol/ntp/ntp.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/refresh_intent.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/router.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::overlays {

// 定义或声明函数 StatusBar::instance：参数在括号里，返回类型写在函数名前面。
StatusBar& StatusBar::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static StatusBar s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义或声明函数 StatusBar::bounds：参数在括号里，返回类型写在函数名前面。
core::Rect StatusBar::bounds() const
{
    // 返回 ui::Theme::status_bar_rect() 给调用者，表示当前函数的执行结果。
    return ui::Theme::status_bar_rect();
}

// [初学者导读] StatusBar::visible()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
bool StatusBar::visible() const
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto& stack = router::Router::instance().stack();
    // 判断条件：stack.shell_top() == router::ShellPage::Lock 是否成立。
    if (stack.shell_top() == router::ShellPage::Lock)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：stack.shell_top() == router::ShellPage::AppHost 是否成立。
    if (stack.shell_top() == router::ShellPage::AppHost)
    {
        // 判断条件：apps::App* a = apps::AppRegistry::instance().active() 是否成立。
        if (apps::App* a = apps::AppRegistry::instance().active())
            // 返回 a->wants_status_bar() 给调用者，表示当前函数的执行结果。
            return a->wants_status_bar();
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 返回 stack.shell_top() == router::ShellPage::Home || 给调用者，表示当前函数的执行结果。
    return stack.shell_top() == router::ShellPage::Home ||
           // 继续上一行的表达式或参数列表：stack.shell_top() == router::ShellPage::AppGrid;。
           stack.shell_top() == router::ShellPage::AppGrid;
}

// 函数定义/声明：调用 StatusBar::on_tick_clock()：两者只在数值变化且状态栏可见时重绘，避免后台页面推动刷新。。
void StatusBar::on_tick_clock(uint8_t h, uint8_t m)
{
    // 判断条件：h == hour_ && m == minute_ 是否成立。
    if (h == hour_ && m == minute_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 h 的结果保存到左侧 hour_。
    hour_ = h;
    // 赋值：把右侧 m 的结果保存到左侧 minute_。
    minute_ = m;
    // 判断条件：visible() 是否成立。
    if (visible())
        // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
        router::Router::instance().repaint(router::intent_partial_full());
}

// 函数定义/声明：调用 StatusBar::on_tick_battery()：两者只在数值变化且状态栏可见时重绘，避免后台页面推动刷新。。
void StatusBar::on_tick_battery(uint8_t pct)
{
    // 判断条件：pct == battery_pct_ 是否成立。
    if (pct == battery_pct_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 pct 的结果保存到左侧 battery_pct_。
    battery_pct_ = pct;
    // 判断条件：visible() 是否成立。
    if (visible())
        // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
        router::Router::instance().repaint(router::intent_partial_full());
}

// 函数定义/声明：调用 StatusBar::on_ntp_sync_done()：处理同步完成事件，更新页面状态。它由 UI 线程消费。。
void StatusBar::on_ntp_sync_done(uint8_t status)
{
    // 判断条件：static_cast<::app::protocol::ntp::SyncStatus>(status 是否成立。
    if (static_cast<::app::protocol::ntp::SyncStatus>(status) !=
        // 继续上一行的表达式或参数列表：::app::protocol::ntp::SyncStatus::COMPLETED)。
        ::app::protocol::ntp::SyncStatus::COMPLETED)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto clk = data::Clock::now();
    // 赋值：把右侧 clk.hour 的结果保存到左侧 hour_。
    hour_ = clk.hour;
    // 赋值：把右侧 clk.minute 的结果保存到左侧 minute_。
    minute_ = clk.minute;
    // 判断条件：visible() 是否成立。
    if (visible())
        // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
        router::Router::instance().repaint(router::intent_partial_full());
}

// 函数定义/声明：调用 StatusBar::paint()：绘制时直接读取 Clock::now() 和 SystemState 的电池值，并顺手更新缓存。这是对锁屏期间可能没有收到 TickClock 的防御。。
void StatusBar::paint(gfx::Canvas& c)
{
    // 判断条件：!visible() 是否成立。
    if (!visible())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 绘制时读系统时间，避免锁屏期间 TickClock 未更新导致显示 00:00
    const auto clk = data::Clock::now();
    // 赋值：把右侧 clk.hour 的结果保存到左侧 hour_。
    hour_ = clk.hour;
    // 赋值：把右侧 clk.minute 的结果保存到左侧 minute_。
    minute_ = clk.minute;

    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ui::Theme;
    // 执行语句：char time_buf[8];；它会更新一次变量、对象或程序状态。
    char time_buf[8];
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    (void)std::snprintf(time_buf, sizeof(time_buf), "%02u:%02u", static_cast<unsigned>(hour_),
                        // 继续上一行的表达式或参数列表：static_cast<unsigned>(minute_));。
                        static_cast<unsigned>(minute_));

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y_top = static_cast<int16_t>((Theme::kStatusBarH - Theme::kFontSmall) / 2);
    // 访问对象成员：c.text(Theme::kPad, y_top, time_buf, Theme::kFontSmall);；读取或修改这个对象里的成员状态。
    c.text(Theme::kPad, y_top, time_buf, Theme::kFontSmall);

    // 赋值：把右侧 data::SystemState::get_instance().battery_pct() 的结果保存到左侧 battery_pct_。
    battery_pct_ = data::SystemState::get_instance().battery_pct();
    // 调用 ui::widgets::battery_indicator(c, static_cast<int16_t>(Theme::kScreenW - Theme::kPad), 0,，执行这个函数并处理它的返回值或状态变化。
    ui::widgets::battery_indicator(c, static_cast<int16_t>(Theme::kScreenW - Theme::kPad), 0,
                                   // 继续上一行的表达式或参数列表：battery_pct_);。
                                   battery_pct_);
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    c.hline(0, static_cast<int16_t>(Theme::kStatusBarH - 1), Theme::kScreenW);
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::overlays
