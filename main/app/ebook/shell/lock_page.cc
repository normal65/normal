/**
 * [初学者导读] 文件：main/app/ebook/shell/lock_page.cc
 * 作用：实现 shell 模块中的 lock page 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：Shell 是 App 之外的外壳页面，负责统一返回和输入兜底。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "shell/lock_page.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/lock_password.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/clock_provider.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/system_state.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/font.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/text_layout.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/keyboard.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/toast.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/page_id.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/refresh_intent.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/router.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::shell {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;

// struct Quote：把相关数据字段组合在一起，表示项目中的一个概念。
struct Quote
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* text;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* source;
};

// 编译期数组 kQuotes：运行时只读的固定查表数据。
constexpr Quote kQuotes[] = {
    // 执行这条代码：{"\u8BFB\u4E66\u662F\u5728\u522B\u4EBA\u601D\u60F3\u7684\u5E2E\u52A9\u4E0B\uFF0C\u5EFA\u7ACB"。它会更新变量、对象状态或调用链。
    {"\u8BFB\u4E66\u662F\u5728\u522B\u4EBA\u601D\u60F3\u7684\u5E2E\u52A9\u4E0B\uFF0C\u5EFA\u7ACB"
     // 执行这条代码："\u8D77\u81EA\u5DF1\u7684\u601D\u60F3\u3002",。它会更新变量、对象状态或调用链。
     "\u8D77\u81EA\u5DF1\u7684\u601D\u60F3\u3002",
     // 继续上一行的表达式或参数列表："\u9C81\u5DF4\u91D1"},。
     "\u9C81\u5DF4\u91D1"},
    // 继续上一行的表达式或参数列表：{"\u4E66\u7C4D\u662F\u4EBA\u7C7B\u8FDB\u6B65\u7684\u9636\u68AF\u3002", "\u9AD8\u5C14\u57FA"},。
    {"\u4E66\u7C4D\u662F\u4EBA\u7C7B\u8FDB\u6B65\u7684\u9636\u68AF\u3002", "\u9AD8\u5C14\u57FA"},
    // 继续上一行的表达式或参数列表：{"\u8BFB\u4E00\u672C\u597D\u4E66\uFF0C\u5C31\u662F\u548C\u8BB8\u591A\u9AD8\u5C1A\u7684\u4EBA"。
    {"\u8BFB\u4E00\u672C\u597D\u4E66\uFF0C\u5C31\u662F\u548C\u8BB8\u591A\u9AD8\u5C1A\u7684\u4EBA"
     // 执行这条代码："\u8C08\u8BDD\u3002",。它会更新变量、对象状态或调用链。
     "\u8C08\u8BDD\u3002",
     // 继续上一行的表达式或参数列表："\u6B4C\u5FB7"},。
     "\u6B4C\u5FB7"},
    // 继续上一行的表达式或参数列表：{"\u9ED1\u53D1\u4E0D\u77E5\u52E4\u5B66\u65E9\uFF0C\u767D\u9996\u65B9\u6094\u8BFB\u4E66\u8FDF"。
    {"\u9ED1\u53D1\u4E0D\u77E5\u52E4\u5B66\u65E9\uFF0C\u767D\u9996\u65B9\u6094\u8BFB\u4E66\u8FDF"
     // 执行这条代码："\u3002",。它会更新变量、对象状态或调用链。
     "\u3002",
     // 继续上一行的表达式或参数列表："\u989C\u771F\u537F"},。
     "\u989C\u771F\u537F"},
    // 继续上一行的表达式或参数列表：{"\u4E66\u5230\u7528\u65F6\u65B9\u6068\u5C11\u3002", "\u9646\u6E38"},。
    {"\u4E66\u5230\u7528\u65F6\u65B9\u6068\u5C11\u3002", "\u9646\u6E38"},
    // 继续上一行的表达式或参数列表：{"\u95EE\u6E20\u90A3\u5F97\u6E05\u5982\u8BB8\uFF0C\u4E3A\u6709\u6E90\u5934\u6D3B\u6C34\u6765"。
    {"\u95EE\u6E20\u90A3\u5F97\u6E05\u5982\u8BB8\uFF0C\u4E3A\u6709\u6E90\u5934\u6D3B\u6C34\u6765"
     // 执行这条代码："\u3002",。它会更新变量、对象状态或调用链。
     "\u3002",
     // 继续上一行的表达式或参数列表："\u6731\u7199"},。
     "\u6731\u7199"},
    // 继续上一行的表达式或参数列表：{"\u8179\u6709\u8BD7\u4E66\u6C14\u81EA\u534E\u3002", "\u82CF\u8F7C"},。
    {"\u8179\u6709\u8BD7\u4E66\u6C14\u81EA\u534E\u3002", "\u82CF\u8F7C"},
    // 继续上一行的表达式或参数列表：{"\u8BFB\u4E66\u7834\u4E07\u5377\uFF0C\u4E0B\u7B14\u5982\u6709\u795E\u3002", "\u675C\u752B"},。
    {"\u8BFB\u4E66\u7834\u4E07\u5377\uFF0C\u4E0B\u7B14\u5982\u6709\u795E\u3002", "\u675C\u752B"},
};
// 固定参数 kQuoteCount = sizeof(kQuotes) / sizeof(kQuotes[0])：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr size_t kQuoteCount = sizeof(kQuotes) / sizeof(kQuotes[0]);

// 定义或声明函数 time_box_rect：参数在括号里，返回类型写在函数名前面。
core::Rect time_box_rect()
{
    // 返回 core::Rect{0, static_cast<int16_t>(Theme::kLockTimeY), Theme::kScreenW, 给调用者，表示当前函数的执行结果。
    return core::Rect{0, static_cast<int16_t>(Theme::kLockTimeY), Theme::kScreenW,
                      // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kFontHuge + 8)};。
                      static_cast<uint16_t>(Theme::kFontHuge + 8)};
}

// 定义或声明函数 on_lock_pin_done：参数在括号里，返回类型写在函数名前面。
void on_lock_pin_done(const char* text, void* /*user*/)
{
    // 判断条件：apps::settings::LockPassword::get_instance().verify(text) 是否成立。
    if (apps::settings::LockPassword::get_instance().verify(text))
    {
        // 访问对象成员：(void)router::Router::instance().navigate(router::page_shell(router::ShellPage::Home));；读取或修改这个对象里的成员状态。
        (void)router::Router::instance().navigate(router::page_shell(router::ShellPage::Home));
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 调用 overlays::Toast::instance().show(ui::strings::kSetSecWrong, 2000);，执行这个函数并处理它的返回值或状态变化。
    overlays::Toast::instance().show(ui::strings::kSetSecWrong, 2000);
}

// 定义或声明函数 open_lock_pin_keyboard：参数在括号里，返回类型写在函数名前面。
void open_lock_pin_keyboard()
{
    // 声明并默认初始化对象：overlays::KeyboardConfig kc{}。
    overlays::KeyboardConfig kc{};
    // 赋值：把右侧 overlays::KeyboardLayer::Numbers 的结果保存到左侧 kc.default_layer。
    kc.default_layer = overlays::KeyboardLayer::Numbers;
    // 赋值：把右侧 apps::settings::LockPassword::kPinLen 的结果保存到左侧 kc.max_len。
    kc.max_len = apps::settings::LockPassword::kPinLen;
    // 赋值：把右侧 ui::strings::kLockEnterPin 的结果保存到左侧 kc.title。
    kc.title = ui::strings::kLockEnterPin;
    // 调用 overlays::Keyboard::instance().open(kc, on_lock_pin_done, nullptr);，执行这个函数并处理它的返回值或状态变化。
    overlays::Keyboard::instance().open(kc, on_lock_pin_done, nullptr);
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 LockPage::instance：参数在括号里，返回类型写在函数名前面。
LockPage& LockPage::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static LockPage s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 函数定义/声明：调用 LockPage::paint()：锁屏页隐藏普通状态栏，自己绘制： 日期和星期。 电池图标。 大号时分。 每日格言和出处。 “上滑解锁”提示。。
void LockPage::paint(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto clk = data::Clock::now();

    // 执行语句：char date_buf[48];；它会更新一次变量、对象或程序状态。
    char date_buf[48];
    // 执行语句：char wd_buf[16];；它会更新一次变量、对象或程序状态。
    char wd_buf[16];
    // 访问对象成员：clk.format_date_cn(date_buf, sizeof(date_buf));；读取或修改这个对象里的成员状态。
    clk.format_date_cn(date_buf, sizeof(date_buf));
    // 访问对象成员：(void)std::snprintf(wd_buf, sizeof(wd_buf), "  %s", clk.weekday_name_cn());；读取或修改这个对象里的成员状态。
    (void)std::snprintf(wd_buf, sizeof(wd_buf), "  %s", clk.weekday_name_cn());

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t chrome_y = static_cast<int16_t>((Theme::kLockChromeH - Theme::kFontSmall) / 2);

    // 访问对象成员：c.text(Theme::kPad, chrome_y, date_buf, Theme::kFontSmall);；读取或修改这个对象里的成员状态。
    c.text(Theme::kPad, chrome_y, date_buf, Theme::kFontSmall);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t dw = gfx::Font::get_instance().measure(date_buf, Theme::kFontSmall);
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    c.text(static_cast<int16_t>(Theme::kPad + dw), chrome_y, wd_buf, Theme::kFontSmall);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t batt = data::SystemState::get_instance().battery_pct();
    // 调用 ui::widgets::battery_indicator(c, static_cast<int16_t>(Theme::kScreenW - Theme::kPad), 0, batt);，执行这个函数并处理它的返回值或状态变化。
    ui::widgets::battery_indicator(c, static_cast<int16_t>(Theme::kScreenW - Theme::kPad), 0, batt);
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    c.hline(0, static_cast<int16_t>(Theme::kLockChromeH - 1), Theme::kScreenW);

    // 执行语句：char hm[8];；它会更新一次变量、对象或程序状态。
    char hm[8];
    // 访问对象成员：clk.format_time_hm(hm, sizeof(hm));；读取或修改这个对象里的成员状态。
    clk.format_time_hm(hm, sizeof(hm));
    // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
    gfx::Canvas::TextStyle ts{};
    // 赋值：把右侧 Theme::kFontHuge 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontHuge;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
    ts.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Top 的结果保存到左侧 ts.v。
    ts.v = gfx::VAlign::Top;
    // 访问对象成员：c.text_in(time_box_rect(), hm, ts);；读取或修改这个对象里的成员状态。
    c.text_in(time_box_rect(), hm, ts);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const Quote& q = kQuotes[clk.day_of_year % kQuoteCount];
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t qx = Theme::kPadLg;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t qw = static_cast<uint16_t>(Theme::kScreenW - qx * 2);

    // 访问对象成员：c.text(qx, Theme::kLockQuoteY, ui::strings::kDailyQuote, Theme::kFontSmall);；读取或修改这个对象里的成员状态。
    c.text(qx, Theme::kLockQuoteY, ui::strings::kDailyQuote, Theme::kFontSmall);

    // 执行语句：gfx::LineSlice lines[Theme::kLockQuoteMaxLines];；它会更新一次变量、对象或程序状态。
    gfx::LineSlice lines[Theme::kLockQuoteMaxLines];
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t n =
        // 继续上一行的表达式或参数列表：gfx::wrap_text(q.text, Theme::kFontBody, qw, Theme::kLockQuoteMaxLines, lines);。
        gfx::wrap_text(q.text, Theme::kFontBody, qw, Theme::kLockQuoteMaxLines, lines);

    // 定义变量 int16_t y 并初始化为 static_cast<int16_t>(Theme::kLockQuoteY + 14)。
    int16_t y = static_cast<int16_t>(Theme::kLockQuoteY + 14);
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < n; ++i)
    {
        // 执行语句：char buf[96];；它会更新一次变量、对象或程序状态。
        char buf[96];
        // 只读约束：这个值或对象在当前位置不能被修改。
        const size_t len = static_cast<size_t>(lines[i].end - lines[i].begin);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const size_t cap = (len < sizeof(buf) - 1) ? len : sizeof(buf) - 1;
        // 调用 std::memcpy(buf, lines[i].begin, cap);，执行这个函数并处理它的返回值或状态变化。
        std::memcpy(buf, lines[i].begin, cap);
        // 赋值：把右侧 '\0' 的结果保存到左侧 buf[cap]。
        buf[cap] = '\0';
        // 访问对象成员：c.text(qx, y, buf, Theme::kFontBody);；读取或修改这个对象里的成员状态。
        c.text(qx, y, buf, Theme::kFontBody);
        // 赋值：把右侧 static_cast<int16_t>(y + Theme::kLockQuoteStep) 的结果保存到左侧 y。
        y = static_cast<int16_t>(y + Theme::kLockQuoteStep);
    }

    // 执行语句：char src_buf[48];；它会更新一次变量、对象或程序状态。
    char src_buf[48];
    // 访问对象成员：(void)std::snprintf(src_buf, sizeof(src_buf), "\u2014\u2014 %s", q.source);；读取或修改这个对象里的成员状态。
    (void)std::snprintf(src_buf, sizeof(src_buf), "\u2014\u2014 %s", q.source);
    // 访问对象成员：c.text(qx, y, src_buf, Theme::kFontSmall);；读取或修改这个对象里的成员状态。
    c.text(qx, y, src_buf, Theme::kFontSmall);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect hint_box{0, static_cast<int16_t>(Theme::kLockHintY), Theme::kScreenW,
                              // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kFontSmall + 4)};。
                              static_cast<uint16_t>(Theme::kFontSmall + 4)};
    // 声明并默认初始化对象：gfx::Canvas::TextStyle hs{}。
    gfx::Canvas::TextStyle hs{};
    // 赋值：把右侧 Theme::kFontSmall 的结果保存到左侧 hs.size_px。
    hs.size_px = Theme::kFontSmall;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 hs.h。
    hs.h = gfx::HAlign::Center;
    // 访问对象成员：c.text_in(hint_box, ui::strings::kSwipeToUnlock, hs);；读取或修改这个对象里的成员状态。
    c.text_in(hint_box, ui::strings::kSwipeToUnlock, hs);
}

// 函数定义/声明：调用 LockPage::on_ui_event()：只处理相关事件。
void LockPage::on_ui_event(const ui::UiEvent& ev)
{
    // 判断条件：router::Router::instance().stack().shell_top() != router::ShellPage::Lock 是否成立。
    if (router::Router::instance().stack().shell_top() != router::ShellPage::Lock)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 判断条件：ev.kind == ui::UiEventKind::TickClock 是否成立。
    if (ev.kind == ui::UiEventKind::TickClock)
    {
        // 判断条件：ev.payload.clock.hour == hour_ && ev.payload.clock.minute == minute_ 是否成立。
        if (ev.payload.clock.hour == hour_ && ev.payload.clock.minute == minute_)
            // 提前返回：结束当前函数，不返回具体值。
            return;
        // 赋值：把右侧 ev.payload.clock.hour 的结果保存到左侧 hour_。
        hour_ = ev.payload.clock.hour;
        // 赋值：把右侧 ev.payload.clock.minute 的结果保存到左侧 minute_。
        minute_ = ev.payload.clock.minute;
        // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
        router::Router::instance().repaint(router::intent_partial_full());
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：ev.kind == ui::UiEventKind::TickBattery 是否成立。
    if (ev.kind == ui::UiEventKind::TickBattery)
    {
        // 判断条件：ev.payload.battery.pct == battery_pct_ 是否成立。
        if (ev.payload.battery.pct == battery_pct_)
            // 提前返回：结束当前函数，不返回具体值。
            return;
        // 赋值：把右侧 ev.payload.battery.pct 的结果保存到左侧 battery_pct_。
        battery_pct_ = ev.payload.battery.pct;
        // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
        router::Router::instance().repaint(router::intent_partial_full());
    }
}

// 函数定义/声明：调用 LockPage::on_input()：只有上滑有意义： 如果启用了 PIN，打开数字键盘。 如果没启用 PIN，直接进入 Home。。
InputResult LockPage::on_input(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;
    // 判断条件：ev.type == EventType::SwipeUp 是否成立。
    if (ev.type == EventType::SwipeUp)
    {
        // 判断条件：apps::settings::LockPassword::get_instance().enabled() 是否成立。
        if (apps::settings::LockPassword::get_instance().enabled())
            // 调用 open_lock_pin_keyboard();，执行这个函数并处理它的返回值或状态变化。
            open_lock_pin_keyboard();
        // 备用分支：前面的条件不成立时执行这里。
        else
            // 访问对象成员：(void)router::Router::instance().navigate(router::page_shell(router::ShellPage::Home));；读取或修改这个对象里的成员状态。
            (void)router::Router::instance().navigate(router::page_shell(router::ShellPage::Home));
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 返回 {} 给调用者，表示当前函数的执行结果。
    return {};
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::shell
