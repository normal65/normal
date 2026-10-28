/**
 * [初学者导读] 文件：main/app/ebook/apps/clock/clock_app.cc
 * 作用：实现时钟界面、闹钟显示和闹钟编辑。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/clock/clock_app.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdlib>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/clock_provider.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/icon.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/keyboard.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/toast.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::clock {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;
// 固定参数 kTabBarH = 24：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kTabBarH = 24;
// 固定参数 kContentTopY = Theme::kListStartY + kTabBarH：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kContentTopY = Theme::kListStartY + kTabBarH;
// 固定参数 kAlarmAddBtnH = 30：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kAlarmAddBtnH = 30;
// 固定参数 kEditActionBarH = 30：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kEditActionBarH = 30;
// 固定参数 kEditRowH = 32：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kEditRowH = 32;
// 固定参数 kNavIconSize = 14：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kNavIconSize = 14;
// 固定参数 kTimeStepBtnW = 32：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kTimeStepBtnW = 32;
// 固定参数 kTimeLblW = 18：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kTimeLblW = 18;

// struct TimeStepperGeom：把相关数据字段组合在一起，表示项目中的一个概念。
struct TimeStepperGeom
{
    // 执行语句：core::Rect row;；它会更新一次变量、对象或程序状态。
    core::Rect row;
    // 执行语句：core::Rect label;；它会更新一次变量、对象或程序状态。
    core::Rect label;
    // 执行语句：core::Rect dec;；它会更新一次变量、对象或程序状态。
    core::Rect dec;
    // 执行语句：core::Rect val;；它会更新一次变量、对象或程序状态。
    core::Rect val;
    // 执行语句：core::Rect inc;；它会更新一次变量、对象或程序状态。
    core::Rect inc;
};

// 定义或声明函数 draw_nav_btn：参数在括号里，返回类型写在函数名前面。
void draw_nav_btn(gfx::Canvas& c, const core::Rect& btn, uint32_t icon_cp)
{
    // 调用 rect(btn,1)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    c.rect(btn, 1);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t ax = static_cast<int16_t>(btn.x + (btn.w - kNavIconSize) / 2);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t ay = static_cast<int16_t>(btn.y + (btn.h - kNavIconSize) / 2);
    // 访问对象成员：c.glyph(ax, ay, icon_cp, kNavIconSize, gfx::FontFace::Icon);；读取或修改这个对象里的成员状态。
    c.glyph(ax, ay, icon_cp, kNavIconSize, gfx::FontFace::Icon);
}

/** 编辑页从工具栏下开始（隐藏 Tab），行高略压缩以腾出底栏 */
// 定义或声明函数 alarm_edit_hour_y：参数在括号里，返回类型写在函数名前面。
int16_t alarm_edit_hour_y()
{
    // 返回 static_cast<int16_t>(Theme::kListStartY + 4) 给调用者，表示当前函数的执行结果。
    return static_cast<int16_t>(Theme::kListStartY + 4);
}

// 定义或声明函数 alarm_edit_minute_y：参数在括号里，返回类型写在函数名前面。
int16_t alarm_edit_minute_y()
{
    // 返回 static_cast<int16_t>(alarm_edit_hour_y() + kEditRowH) 给调用者，表示当前函数的执行结果。
    return static_cast<int16_t>(alarm_edit_hour_y() + kEditRowH);
}

// 定义或声明函数 alarm_edit_repeat_y：参数在括号里，返回类型写在函数名前面。
int16_t alarm_edit_repeat_y()
{
    // 返回 static_cast<int16_t>(alarm_edit_minute_y() + kEditRowH + 2) 给调用者，表示当前函数的执行结果。
    return static_cast<int16_t>(alarm_edit_minute_y() + kEditRowH + 2);
}

// 定义或声明函数 time_stepper_geom：参数在括号里，返回类型写在函数名前面。
TimeStepperGeom time_stepper_geom(int16_t y)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect row{Theme::kPadLg, y,
                         // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kScreenW - Theme::kPadLg * 2), kEditRowH};。
                         static_cast<uint16_t>(Theme::kScreenW - Theme::kPadLg * 2), kEditRowH};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect label{row.x, row.y, kTimeLblW, row.h};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect dec{static_cast<int16_t>(label.right()), row.y, kTimeStepBtnW, row.h};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect inc{static_cast<int16_t>(row.right() - kTimeStepBtnW), row.y, kTimeStepBtnW,
                         // 继续上一行的表达式或参数列表：row.h};。
                         row.h};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect val{static_cast<int16_t>(dec.right()), row.y,
                         // 继续上一行的表达式或参数列表：static_cast<uint16_t>(inc.x - dec.right()), row.h};。
                         static_cast<uint16_t>(inc.x - dec.right()), row.h};
    // 返回 {row, label, dec, val, inc} 给调用者，表示当前函数的执行结果。
    return {row, label, dec, val, inc};
}

// 定义或声明函数 paint_time_stepper：参数在括号里，返回类型写在函数名前面。
void paint_time_stepper(gfx::Canvas& c, const TimeStepperGeom& g, const char* label, uint8_t value)
{
    // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
    gfx::Canvas::TextStyle ts{};
    // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontBody;
    // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
    ts.v = gfx::VAlign::Middle;
    // 调用 text_in(g.label,label,ts)：文本对齐。
    c.text_in(g.label, label, ts);

    // 调用 draw_nav_btn(c, g.dec, gfx::icon::kFaMinus);，执行这个函数并处理它的返回值或状态变化。
    draw_nav_btn(c, g.dec, gfx::icon::kFaMinus);
    // 调用 draw_nav_btn(c, g.inc, gfx::icon::kFaPlus);，执行这个函数并处理它的返回值或状态变化。
    draw_nav_btn(c, g.inc, gfx::icon::kFaPlus);

    // 执行语句：char buf[8];；它会更新一次变量、对象或程序状态。
    char buf[8];
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    (void)std::snprintf(buf, sizeof(buf), "%02u", static_cast<unsigned>(value));
    // 调用 rect(g.val,1)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    c.rect(g.val, 1);
    // 赋值：把右侧 Theme::kFontTitle 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontTitle;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
    ts.h = gfx::HAlign::Center;
    // 调用 text_in(g.val,buf,ts)：文本对齐。
    c.text_in(g.val, buf, ts);
}

// 定义或声明函数 parse_time_field：参数在括号里，返回类型写在函数名前面。
bool parse_time_field(const char* text, uint8_t max_val, uint8_t& out)
{
    // 判断条件：text == nullptr || text[0] == '\0' 是否成立。
    if (text == nullptr || text[0] == '\0')
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (const char* p = text; *p != '\0'; ++p)
    {
        // 判断条件：*p < '0' || *p > '9' 是否成立。
        if (*p < '0' || *p > '9')
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
    }
    // 赋值：把右侧 nullptr 的结果保存到左侧 char* end。
    char* end = nullptr;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const long v = std::strtol(text, &end, 10);
    // 判断条件：end == text || *end != '\0' || v < 0 || v > static_cast<long>(max_val) 是否成立。
    if (end == text || *end != '\0' || v < 0 || v > static_cast<long>(max_val))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 赋值：把右侧 static_cast<uint8_t>(v) 的结果保存到左侧 out。
    out = static_cast<uint8_t>(v);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 ClockApp::instance：参数在括号里，返回类型写在函数名前面。
ClockApp& ClockApp::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static ClockApp s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义只读对象或变量：初始化后不能在当前位置修改。
const char* ClockApp::title() const
{
    // 判断条件：sub_ == SubPage::AlarmEdit 是否成立。
    if (sub_ == SubPage::AlarmEdit)
        // 返回 ui::strings::kClkEditAlarm 给调用者，表示当前函数的执行结果。
        return ui::strings::kClkEditAlarm;
    // 返回 ui::strings::kAppClock 给调用者，表示当前函数的执行结果。
    return ui::strings::kAppClock;
}
// 函数定义/声明：调用 ClockApp::icon_cp()：返回图标 codepoint。图标通常来自字体图标表，所以这里返回的是 uint32t，不是图片路径。。
uint32_t ClockApp::icon_cp() const
{
    // 返回 gfx::icon::kFaClock 给调用者，表示当前函数的执行结果。
    return gfx::icon::kFaClock;
}

// 定义或声明函数 ClockApp::tabs_rect：参数在括号里，返回类型写在函数名前面。
core::Rect ClockApp::tabs_rect()
{
    // 返回 core::Rect{0, static_cast<int16_t>(Theme::kListStartY), Theme::kScreenW, kTabBarH} 给调用者，表示当前函数的执行结果。
    return core::Rect{0, static_cast<int16_t>(Theme::kListStartY), Theme::kScreenW, kTabBarH};
}

// 定义或声明函数 ClockApp::content_rect：参数在括号里，返回类型写在函数名前面。
core::Rect ClockApp::content_rect()
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y = static_cast<int16_t>(kContentTopY);
    // 返回 core::Rect{0, y, Theme::kScreenW, static_cast<uint16_t>(Theme::kScreenH - y)} 给调用者，表示当前函数的执行结果。
    return core::Rect{0, y, Theme::kScreenW, static_cast<uint16_t>(Theme::kScreenH - y)};
}

// 定义或声明函数 ClockApp::alarm_add_btn_rect：参数在括号里，返回类型写在函数名前面。
core::Rect ClockApp::alarm_add_btn_rect()
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect c = content_rect();
    // 返回 core::Rect{c.x, static_cast<int16_t>(c.bottom() - kAlarmAddBtnH), c.w, kAlarmAddBtnH} 给调用者，表示当前函数的执行结果。
    return core::Rect{c.x, static_cast<int16_t>(c.bottom() - kAlarmAddBtnH), c.w, kAlarmAddBtnH};
}

// 定义或声明函数 ClockApp::alarm_edit_action_bar_rect：参数在括号里，返回类型写在函数名前面。
core::Rect ClockApp::alarm_edit_action_bar_rect()
{
    // 返回 core::Rect{0, static_cast<int16_t>(Theme::kScreenH - kEditActionBarH), Theme::kScreenW, 给调用者，表示当前函数的执行结果。
    return core::Rect{0, static_cast<int16_t>(Theme::kScreenH - kEditActionBarH), Theme::kScreenW,
                      // 继续上一行的表达式或参数列表：kEditActionBarH};。
                      kEditActionBarH};
}

// 定义或声明函数 ClockApp::alarm_edit_ok_rect：参数在括号里，返回类型写在函数名前面。
core::Rect ClockApp::alarm_edit_ok_rect() const
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect bar = alarm_edit_action_bar_rect();
    // 判断条件：edit_is_new_ 是否成立。
    if (edit_is_new_)
        // 返回 bar 给调用者，表示当前函数的执行结果。
        return bar;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t half = static_cast<uint16_t>(bar.w / 2);
    // 返回 core::Rect{static_cast<int16_t>(bar.x + half), bar.y, half, bar.h} 给调用者，表示当前函数的执行结果。
    return core::Rect{static_cast<int16_t>(bar.x + half), bar.y, half, bar.h};
}

// 定义或声明函数 ClockApp::alarm_edit_delete_rect：参数在括号里，返回类型写在函数名前面。
core::Rect ClockApp::alarm_edit_delete_rect() const
{
    // 判断条件：edit_is_new_ 是否成立。
    if (edit_is_new_)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect bar = alarm_edit_action_bar_rect();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t half = static_cast<uint16_t>(bar.w / 2);
    // 返回 core::Rect{bar.x, bar.y, half, bar.h} 给调用者，表示当前函数的执行结果。
    return core::Rect{bar.x, bar.y, half, bar.h};
}

// 定义或声明函数 ClockApp::invalidate_body：参数在括号里，返回类型写在函数名前面。
void ClockApp::invalidate_body()
{
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 定义或声明函数 ClockApp::set_tab：参数在括号里，返回类型写在函数名前面。
void ClockApp::set_tab(Tab t)
{
    // 赋值：把右侧 t 的结果保存到左侧 tab_。
    tab_ = t;
    // 赋值：把右侧 SubPage::Main 的结果保存到左侧 sub_。
    sub_ = SubPage::Main;
    // 赋值：把右侧 false 的结果保存到左侧 edit_is_new_。
    edit_is_new_ = false;
    // 调用 invalidate_body();，执行这个函数并处理它的返回值或状态变化。
    invalidate_body();
}

// 定义或声明函数 ClockApp::cancel_alarm_edit：参数在括号里，返回类型写在函数名前面。
void ClockApp::cancel_alarm_edit()
{
    // 判断条件：sub_ != SubPage::AlarmEdit 是否成立。
    if (sub_ != SubPage::AlarmEdit)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 判断条件：overlays::Keyboard::instance().is_open() 是否成立。
    if (overlays::Keyboard::instance().is_open())
        // 调用 close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
        overlays::Keyboard::instance().close();

    // 赋值：把右侧 ClockStore::get_instance() 的结果保存到左侧 auto& st。
    auto& st = ClockStore::get_instance();
    // 判断条件：edit_is_new_ 是否成立。
    if (edit_is_new_)
    {
        // 判断条件：edit_alarm_idx_ < st.alarm_count() 是否成立。
        if (edit_alarm_idx_ < st.alarm_count())
            // 访问对象成员：st.remove_alarm(edit_alarm_idx_);；读取或修改这个对象里的成员状态。
            st.remove_alarm(edit_alarm_idx_);
    }
    // 备用分支：前面的条件不成立时执行这里。
    else if (edit_alarm_idx_ < st.alarm_count())
    {
        // 赋值：把右侧 edit_snapshot_ 的结果保存到左侧 st.alarm_mut(edit_alarm_idx_)。
        st.alarm_mut(edit_alarm_idx_) = edit_snapshot_;
    }

    // 赋值：把右侧 false 的结果保存到左侧 edit_is_new_。
    edit_is_new_ = false;
    // 赋值：把右侧 SubPage::Main 的结果保存到左侧 sub_。
    sub_ = SubPage::Main;
    // 赋值：把右侧 Tab::Alarm 的结果保存到左侧 tab_。
    tab_ = Tab::Alarm;
    // 调用 invalidate_body();，执行这个函数并处理它的返回值或状态变化。
    invalidate_body();
}

// 定义或声明函数 ClockApp::on_enter：参数在括号里，返回类型写在函数名前面。
void ClockApp::on_enter()
{
    // 赋值：把右侧 Tab::Clock 的结果保存到左侧 tab_。
    tab_ = Tab::Clock;
    // 赋值：把右侧 SubPage::Main 的结果保存到左侧 sub_。
    sub_ = SubPage::Main;
    // 赋值：把右侧 false 的结果保存到左侧 edit_is_new_。
    edit_is_new_ = false;

    // 访问对象成员：alarm_list_.set_area(content_rect());；读取或修改这个对象里的成员状态。
    alarm_list_.set_area(content_rect());
    // 开始代码块：下面是这个函数、分支或循环的主体。
    alarm_list_.set_provider([](uint8_t i, ui::widgets::RowStyle& s) {
        // 赋值：把右侧 ClockStore::get_instance() 的结果保存到左侧 auto& st。
        auto& st = ClockStore::get_instance();
        // 判断条件：i >= st.alarm_count() 是否成立。
        if (i >= st.alarm_count())
        {
            // 赋值：把右侧 ui::strings::kEmpty 的结果保存到左侧 s.label。
            s.label = ui::strings::kEmpty;
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }
        // 只读约束：这个值或对象在当前位置不能被修改。
        const auto& a = st.alarm(i);
        // 静态字符缓冲区 buf：保存临时文本，跨函数调用复用。
        static char buf[32];
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        (void)std::snprintf(buf, sizeof(buf), "%02u:%02u %s", static_cast<unsigned>(a.hour),
                            // 继续上一行的表达式或参数列表：static_cast<unsigned>(a.minute), st.repeat_label(a.repeat));。
                            static_cast<unsigned>(a.minute), st.repeat_label(a.repeat));
        // 赋值：把右侧 buf 的结果保存到左侧 s.label。
        s.label = buf;
        // 赋值：把右侧 gfx::icon::toggle(a.enabled) 的结果保存到左侧 s.value_icon。
        s.value_icon = gfx::icon::toggle(a.enabled);
        // 赋值：把右侧 20 的结果保存到左侧 s.value_icon_size。
        s.value_icon_size = 20;
        // 赋值：把右侧 true 的结果保存到左侧 s.show_chevron。
        s.show_chevron = true;
    // 执行语句：});；它会更新一次变量、对象或程序状态。
    });

    // 调用 invalidate_body();，执行这个函数并处理它的返回值或状态变化。
    invalidate_body();
}

// 函数定义/声明：调用 ClockApp::on_exit()：退出时取消未完成的扫描、关闭弹窗，并确保后台任务不会在对象状态失效后继续写数据。。
void ClockApp::on_exit()
{
    // 判断条件：overlays::Keyboard::instance().is_open() 是否成立。
    if (overlays::Keyboard::instance().is_open())
        // 调用 close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
        overlays::Keyboard::instance().close();
    // 赋值：把右侧 SubPage::Main 的结果保存到左侧 sub_。
    sub_ = SubPage::Main;
}

// 函数定义/声明：调用 ClockApp::on_semantic_back()：处理语义返回，顺序通常是： 1. 如果弹窗打开，先关闭弹窗。 2. 如果列表还在浏览目录，先返回上一级。 3. 如果已经到根卷，返回 false，让 Shell 退出 App。。
bool ClockApp::on_semantic_back()
{
    // 判断条件：overlays::Keyboard::instance().is_open() 是否成立。
    if (overlays::Keyboard::instance().is_open())
    {
        // 调用 close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
        overlays::Keyboard::instance().close();
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 判断条件：sub_ == SubPage::AlarmEdit 是否成立。
    if (sub_ == SubPage::AlarmEdit)
    {
        // 调用 cancel_alarm_edit();，执行这个函数并处理它的返回值或状态变化。
        cancel_alarm_edit();
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 返回 false 给调用者，表示当前函数的执行结果。
    return false;
}

// 定义或声明函数 ClockApp::open_hour_keyboard：参数在括号里，返回类型写在函数名前面。
void ClockApp::open_hour_keyboard()
{
    // 赋值：把右侧 ClockStore::get_instance() 的结果保存到左侧 auto& st。
    auto& st = ClockStore::get_instance();
    // 判断条件：edit_alarm_idx_ >= st.alarm_count() 是否成立。
    if (edit_alarm_idx_ >= st.alarm_count())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto& a = st.alarm(edit_alarm_idx_);
    // 执行语句：char init[4];；它会更新一次变量、对象或程序状态。
    char init[4];
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    (void)std::snprintf(init, sizeof(init), "%02u", static_cast<unsigned>(a.hour));

    // 声明并默认初始化对象：overlays::KeyboardConfig kc{}。
    overlays::KeyboardConfig kc{};
    // 赋值：把右侧 overlays::KeyboardLayer::Numbers 的结果保存到左侧 kc.default_layer。
    kc.default_layer = overlays::KeyboardLayer::Numbers;
    // 赋值：把右侧 2 的结果保存到左侧 kc.max_len。
    kc.max_len = 2;
    // 赋值：把右侧 init 的结果保存到左侧 kc.initial_text。
    kc.initial_text = init;
    // 赋值：把右侧 ui::strings::kClkHourHint 的结果保存到左侧 kc.title。
    kc.title = ui::strings::kClkHourHint;
    // 调用 overlays::Keyboard::instance().open(kc, on_hour_kb_done, this);，执行这个函数并处理它的返回值或状态变化。
    overlays::Keyboard::instance().open(kc, on_hour_kb_done, this);
}

// 定义或声明函数 ClockApp::open_minute_keyboard：参数在括号里，返回类型写在函数名前面。
void ClockApp::open_minute_keyboard()
{
    // 赋值：把右侧 ClockStore::get_instance() 的结果保存到左侧 auto& st。
    auto& st = ClockStore::get_instance();
    // 判断条件：edit_alarm_idx_ >= st.alarm_count() 是否成立。
    if (edit_alarm_idx_ >= st.alarm_count())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto& a = st.alarm(edit_alarm_idx_);
    // 执行语句：char init[4];；它会更新一次变量、对象或程序状态。
    char init[4];
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    (void)std::snprintf(init, sizeof(init), "%02u", static_cast<unsigned>(a.minute));

    // 声明并默认初始化对象：overlays::KeyboardConfig kc{}。
    overlays::KeyboardConfig kc{};
    // 赋值：把右侧 overlays::KeyboardLayer::Numbers 的结果保存到左侧 kc.default_layer。
    kc.default_layer = overlays::KeyboardLayer::Numbers;
    // 赋值：把右侧 2 的结果保存到左侧 kc.max_len。
    kc.max_len = 2;
    // 赋值：把右侧 init 的结果保存到左侧 kc.initial_text。
    kc.initial_text = init;
    // 赋值：把右侧 ui::strings::kClkMinuteHint 的结果保存到左侧 kc.title。
    kc.title = ui::strings::kClkMinuteHint;
    // 调用 overlays::Keyboard::instance().open(kc, on_minute_kb_done, this);，执行这个函数并处理它的返回值或状态变化。
    overlays::Keyboard::instance().open(kc, on_minute_kb_done, this);
}

// 定义或声明函数 ClockApp::on_hour_kb_done：参数在括号里，返回类型写在函数名前面。
void ClockApp::on_hour_kb_done(const char* text, void* user)
{
    // 赋值：把右侧 static_cast<ClockApp*>(user) 的结果保存到左侧 auto* self。
    auto* self = static_cast<ClockApp*>(user);
    // 判断条件：self == nullptr 是否成立。
    if (self == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 ClockStore::get_instance() 的结果保存到左侧 auto& st。
    auto& st = ClockStore::get_instance();
    // 判断条件：self->edit_alarm_idx_ >= st.alarm_count() 是否成立。
    if (self->edit_alarm_idx_ >= st.alarm_count())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 判断条件：text == nullptr || text[0] == '\0' 是否成立。
    if (text == nullptr || text[0] == '\0')
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kClkTimeEmpty, 1500);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kClkTimeEmpty, 1500);
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        self->request_repaint();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 定义变量 uint8_t hour 并初始化为 0。
    uint8_t hour = 0;
    // 判断条件：!parse_time_field(text, 23, hour) 是否成立。
    if (!parse_time_field(text, 23, hour))
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kClkTimeInvalid, 1500);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kClkTimeInvalid, 1500);
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        self->request_repaint();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 赋值：把右侧 hour 的结果保存到左侧 st.alarm_mut(self->edit_alarm_idx_).hour。
    st.alarm_mut(self->edit_alarm_idx_).hour = hour;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    self->request_repaint();
}

// 定义或声明函数 ClockApp::on_minute_kb_done：参数在括号里，返回类型写在函数名前面。
void ClockApp::on_minute_kb_done(const char* text, void* user)
{
    // 赋值：把右侧 static_cast<ClockApp*>(user) 的结果保存到左侧 auto* self。
    auto* self = static_cast<ClockApp*>(user);
    // 判断条件：self == nullptr 是否成立。
    if (self == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 ClockStore::get_instance() 的结果保存到左侧 auto& st。
    auto& st = ClockStore::get_instance();
    // 判断条件：self->edit_alarm_idx_ >= st.alarm_count() 是否成立。
    if (self->edit_alarm_idx_ >= st.alarm_count())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 判断条件：text == nullptr || text[0] == '\0' 是否成立。
    if (text == nullptr || text[0] == '\0')
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kClkTimeEmpty, 1500);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kClkTimeEmpty, 1500);
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        self->request_repaint();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 定义变量 uint8_t minute 并初始化为 0。
    uint8_t minute = 0;
    // 判断条件：!parse_time_field(text, 59, minute) 是否成立。
    if (!parse_time_field(text, 59, minute))
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kClkTimeInvalid, 1500);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kClkTimeInvalid, 1500);
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        self->request_repaint();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 赋值：把右侧 minute 的结果保存到左侧 st.alarm_mut(self->edit_alarm_idx_).minute。
    st.alarm_mut(self->edit_alarm_idx_).minute = minute;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    self->request_repaint();
}

// 定义或声明函数 ClockApp::on_ui_event：参数在括号里，返回类型写在函数名前面。
void ClockApp::on_ui_event(const ui::UiEvent& ev)
{
    // 判断条件：ev.kind == ui::UiEventKind::SystemHint & 是否成立。
    if (ev.kind == ui::UiEventKind::SystemHint &&
        // 继续上一行的表达式或参数列表：ev.payload.system.hint == ui::SystemHintKind::ClockAlarm)。
        ev.payload.system.hint == ui::SystemHintKind::ClockAlarm)
    {
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
    }
}

// 定义或声明函数 ClockApp::paint_tabs：参数在括号里，返回类型写在函数名前面。
void ClockApp::paint_tabs(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect bar = tabs_rect();
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    c.hline(bar.x, static_cast<int16_t>(bar.bottom() - 1), bar.w);

    // 静态查表：保存界面显示用的固定文字标签。
    static const char* const kLabels[] = {
        // 执行这条代码：ui::strings::kClkTabClock,。它会更新变量、对象状态或调用链。
        ui::strings::kClkTabClock,
        // 继续上一行的表达式或参数列表：ui::strings::kClkTabAlarm,。
        ui::strings::kClkTabAlarm,
    };

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t tw = static_cast<uint16_t>(Theme::kScreenW / 2);
    // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
    gfx::Canvas::TextStyle ts{};
    // 赋值：把右侧 Theme::kFontSmall 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontSmall;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
    ts.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
    ts.v = gfx::VAlign::Middle;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < 2; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t x = static_cast<int16_t>(i * tw);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect cell{x, bar.y, tw, bar.h};
        // 只读约束：这个值或对象在当前位置不能被修改。
        const bool active = (static_cast<uint8_t>(tab_) == i);
        // 判断条件：active 是否成立。
        if (active)
            // 访问对象成员：c.fill(cell, gfx::Ink::Black);；读取或修改这个对象里的成员状态。
            c.fill(cell, gfx::Ink::Black);
        // 赋值：把右侧 active ? gfx::Ink::White : gfx::Ink::Black 的结果保存到左侧 ts.ink。
        ts.ink = active ? gfx::Ink::White : gfx::Ink::Black;
        // 调用 text_in(cell,kLabels[i],ts)：文本对齐。
        c.text_in(cell, kLabels[i], ts);
        // 判断条件：i > 0 是否成立。
        if (i > 0)
            // 访问对象成员：c.vline(x, bar.y, bar.h);；读取或修改这个对象里的成员状态。
            c.vline(x, bar.y, bar.h);
    }
}

// 定义或声明函数 ClockApp::paint_clock_face：参数在括号里，返回类型写在函数名前面。
void ClockApp::paint_clock_face(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect area = content_rect();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto clk = data::Clock::now();

    // 执行语句：char hm[8];；它会更新一次变量、对象或程序状态。
    char hm[8];
    // 访问对象成员：clk.format_time_hm(hm, sizeof(hm));；读取或修改这个对象里的成员状态。
    clk.format_time_hm(hm, sizeof(hm));

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect time_box{area.x, static_cast<int16_t>(area.y + 24), area.w, 40};
    // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
    gfx::Canvas::TextStyle ts{};
    // 赋值：把右侧 Theme::kFontHuge 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontHuge;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
    ts.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
    ts.v = gfx::VAlign::Middle;
    // 调用 text_in(time_box,hm,ts)：文本对齐。
    c.text_in(time_box, hm, ts);

    // 执行语句：char date_buf[32];；它会更新一次变量、对象或程序状态。
    char date_buf[32];
    // 访问对象成员：clk.format_date_cn(date_buf, sizeof(date_buf));；读取或修改这个对象里的成员状态。
    clk.format_date_cn(date_buf, sizeof(date_buf));
    // 执行语句：char line[40];；它会更新一次变量、对象或程序状态。
    char line[40];
    // 访问对象成员：(void)std::snprintf(line, sizeof(line), "%s  %s", date_buf, clk.weekday_name_cn());；读取或修改这个对象里的成员状态。
    (void)std::snprintf(line, sizeof(line), "%s  %s", date_buf, clk.weekday_name_cn());

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect date_box{area.x, static_cast<int16_t>(time_box.bottom() + 4), area.w, 20};
    // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontBody;
    // 调用 text_in(date_box,line,ts)：文本对齐。
    c.text_in(date_box, line, ts);
}

// 定义或声明函数 ClockApp::paint_alarm_list：参数在括号里，返回类型写在函数名前面。
void ClockApp::paint_alarm_list(gfx::Canvas& c)
{
    // 赋值：把右侧 ClockStore::get_instance() 的结果保存到左侧 auto& st。
    auto& st = ClockStore::get_instance();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect add_btn = alarm_add_btn_rect();
    // 定义变量 core::Rect list_area 并初始化为 content_rect()。
    core::Rect list_area = content_rect();
    // 赋值：把右侧 static_cast<uint16_t>(list_area.h - kAlarmAddBtnH) 的结果保存到左侧 list_area.h。
    list_area.h = static_cast<uint16_t>(list_area.h - kAlarmAddBtnH);

    // 访问对象成员：alarm_list_.set_area(list_area);；读取或修改这个对象里的成员状态。
    alarm_list_.set_area(list_area);
    // 访问对象成员：alarm_list_.set_total(st.alarm_count());；读取或修改这个对象里的成员状态。
    alarm_list_.set_total(st.alarm_count());

    // 判断条件：st.alarm_count() == 0 是否成立。
    if (st.alarm_count() == 0)
    {
        // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
        gfx::Canvas::TextStyle ts{};
        // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 ts.size_px。
        ts.size_px = Theme::kFontBody;
        // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
        ts.h = gfx::HAlign::Center;
        // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
        ts.v = gfx::VAlign::Middle;
        // 访问对象成员：c.text_in(list_area, ui::strings::kClkNoAlarm, ts);；读取或修改这个对象里的成员状态。
        c.text_in(list_area, ui::strings::kClkNoAlarm, ts);
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 访问对象成员：alarm_list_.paint(c);；读取或修改这个对象里的成员状态。
        alarm_list_.paint(c);
    }

    // 访问对象成员：c.hline(add_btn.x, add_btn.y, add_btn.w);；读取或修改这个对象里的成员状态。
    c.hline(add_btn.x, add_btn.y, add_btn.w);
    // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
    gfx::Canvas::TextStyle ts{};
    // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontBody;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
    ts.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
    ts.v = gfx::VAlign::Middle;
    // 访问对象成员：c.text_in(add_btn, ui::strings::kClkAddAlarm, ts);；读取或修改这个对象里的成员状态。
    c.text_in(add_btn, ui::strings::kClkAddAlarm, ts);
}

// 定义或声明函数 ClockApp::paint_alarm_edit_action_bar：参数在括号里，返回类型写在函数名前面。
void ClockApp::paint_alarm_edit_action_bar(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect bar = alarm_edit_action_bar_rect();
    // 访问对象成员：c.hline(bar.x, bar.y, bar.w);；读取或修改这个对象里的成员状态。
    c.hline(bar.x, bar.y, bar.w);

    // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
    gfx::Canvas::TextStyle ts{};
    // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontBody;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
    ts.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
    ts.v = gfx::VAlign::Middle;

    // 判断条件：!edit_is_new_ 是否成立。
    if (!edit_is_new_)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect del = alarm_edit_delete_rect();
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect ok = alarm_edit_ok_rect();
        // 访问对象成员：c.vline(ok.x, bar.y, bar.h);；读取或修改这个对象里的成员状态。
        c.vline(ok.x, bar.y, bar.h);
        // 访问对象成员：c.text_in(del, ui::strings::kClkDelete, ts);；读取或修改这个对象里的成员状态。
        c.text_in(del, ui::strings::kClkDelete, ts);
        // 访问对象成员：c.text_in(ok, ui::strings::kOk, ts);；读取或修改这个对象里的成员状态。
        c.text_in(ok, ui::strings::kOk, ts);
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 访问对象成员：c.text_in(alarm_edit_ok_rect(), ui::strings::kOk, ts);；读取或修改这个对象里的成员状态。
        c.text_in(alarm_edit_ok_rect(), ui::strings::kOk, ts);
    }
}

// 定义或声明函数 ClockApp::paint_alarm_edit：参数在括号里，返回类型写在函数名前面。
void ClockApp::paint_alarm_edit(gfx::Canvas& c)
{
    // 赋值：把右侧 ClockStore::get_instance() 的结果保存到左侧 auto& st。
    auto& st = ClockStore::get_instance();
    // 判断条件：edit_alarm_idx_ >= st.alarm_count() 是否成立。
    if (edit_alarm_idx_ >= st.alarm_count())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto& a = st.alarm(edit_alarm_idx_);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const TimeStepperGeom hour_g = time_stepper_geom(alarm_edit_hour_y());
    // 只读约束：这个值或对象在当前位置不能被修改。
    const TimeStepperGeom min_g = time_stepper_geom(alarm_edit_minute_y());
    // 调用 paint_time_stepper(c, hour_g, ui::strings::kClkHourLabel, a.hour);，执行这个函数并处理它的返回值或状态变化。
    paint_time_stepper(c, hour_g, ui::strings::kClkHourLabel, a.hour);
    // 调用 paint_time_stepper(c, min_g, ui::strings::kClkMinuteLabel, a.minute);，执行这个函数并处理它的返回值或状态变化。
    paint_time_stepper(c, min_g, ui::strings::kClkMinuteLabel, a.minute);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect row = hour_g.row;
    // 定义变量 int16_t y 并初始化为 alarm_edit_repeat_y()。
    int16_t y = alarm_edit_repeat_y();

    // 声明并默认初始化对象：ui::widgets::RowStyle rs{}。
    ui::widgets::RowStyle rs{};
    // 赋值：把右侧 ui::strings::kClkRepeat 的结果保存到左侧 rs.label。
    rs.label = ui::strings::kClkRepeat;
    // 赋值：把右侧 st.repeat_label(a.repeat) 的结果保存到左侧 rs.value。
    rs.value = st.repeat_label(a.repeat);
    // 赋值：把右侧 true 的结果保存到左侧 rs.show_chevron。
    rs.show_chevron = true;
    // 调用 ui::widgets::list_row(c, core::Rect{row.x, y, row.w, kEditRowH}, rs);，执行这个函数并处理它的返回值或状态变化。
    ui::widgets::list_row(c, core::Rect{row.x, y, row.w, kEditRowH}, rs);
    // 赋值：把右侧 static_cast<int16_t>(y + kEditRowH) 的结果保存到左侧 y。
    y = static_cast<int16_t>(y + kEditRowH);

    // 赋值：把右侧 {} 的结果保存到左侧 rs。
    rs = {};
    // 赋值：把右侧 ui::strings::kOn 的结果保存到左侧 rs.label。
    rs.label = ui::strings::kOn;
    // 赋值：把右侧 gfx::icon::toggle(a.enabled) 的结果保存到左侧 rs.value_icon。
    rs.value_icon = gfx::icon::toggle(a.enabled);
    // 赋值：把右侧 20 的结果保存到左侧 rs.value_icon_size。
    rs.value_icon_size = 20;
    // 赋值：把右侧 false 的结果保存到左侧 rs.show_chevron。
    rs.show_chevron = false;
    // 调用 ui::widgets::list_row(c, core::Rect{row.x, y, row.w, kEditRowH}, rs);，执行这个函数并处理它的返回值或状态变化。
    ui::widgets::list_row(c, core::Rect{row.x, y, row.w, kEditRowH}, rs);

    // 调用 paint_alarm_edit_action_bar(c);，执行这个函数并处理它的返回值或状态变化。
    paint_alarm_edit_action_bar(c);
}

// 定义或声明函数 ClockApp::paint：参数在括号里，返回类型写在函数名前面。
void ClockApp::paint(gfx::Canvas& c)
{
    // 调用 title()：返回 App 标题。在文件浏览器内，标题有时会动态反映当前目录，因此可能存在一个 buffer，而不是只返回一个常量字符串。。
    ui::widgets::toolbar(c, title());

    // 判断条件：sub_ == SubPage::AlarmEdit 是否成立。
    if (sub_ == SubPage::AlarmEdit)
    {
        // 调用 paint_alarm_edit(c);，执行这个函数并处理它的返回值或状态变化。
        paint_alarm_edit(c);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 调用 paint_tabs(c);，执行这个函数并处理它的返回值或状态变化。
    paint_tabs(c);
    // 判断条件：tab_ == Tab::Clock 是否成立。
    if (tab_ == Tab::Clock)
        // 调用 paint_clock_face(c);，执行这个函数并处理它的返回值或状态变化。
        paint_clock_face(c);
    // 备用分支：前面的条件不成立时执行这里。
    else
        // 调用 paint_alarm_list(c);，执行这个函数并处理它的返回值或状态变化。
        paint_alarm_list(c);
}

// 定义或声明函数 ClockApp::handle_tabs：参数在括号里，返回类型写在函数名前面。
shell::InputResult ClockApp::handle_tabs(int16_t x, int16_t y)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect bar = tabs_rect();
    // 判断条件：!bar.contains(x, y) 是否成立。
    if (!bar.contains(x, y))
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t tw = static_cast<uint16_t>(Theme::kScreenW / 2);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t idx = static_cast<uint8_t>(x / tw);
    // 判断条件：idx > 1 是否成立。
    if (idx > 1)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};
    // 调用 set_tab(static_cast<Tab>(idx));，执行这个函数并处理它的返回值或状态变化。
    set_tab(static_cast<Tab>(idx));
    // 返回 {true} 给调用者，表示当前函数的执行结果。
    return {true};
}

// 定义或声明函数 ClockApp::handle_alarm_list：参数在括号里，返回类型写在函数名前面。
shell::InputResult ClockApp::handle_alarm_list(int16_t x, int16_t y)
{
    // 赋值：把右侧 ClockStore::get_instance() 的结果保存到左侧 auto& st。
    auto& st = ClockStore::get_instance();

    // 判断条件：alarm_add_btn_rect().contains(x, y) 是否成立。
    if (alarm_add_btn_rect().contains(x, y))
    {
        // 判断条件：st.add_alarm() 是否成立。
        if (st.add_alarm())
        {
            // 赋值：把右侧 static_cast<uint8_t>(st.alarm_count() - 1) 的结果保存到左侧 edit_alarm_idx_。
            edit_alarm_idx_ = static_cast<uint8_t>(st.alarm_count() - 1);
            // 赋值：把右侧 true 的结果保存到左侧 edit_is_new_。
            edit_is_new_ = true;
            // 赋值：把右侧 SubPage::AlarmEdit 的结果保存到左侧 sub_。
            sub_ = SubPage::AlarmEdit;
            // 调用 invalidate_body();，执行这个函数并处理它的返回值或状态变化。
            invalidate_body();
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 调用 show("\u6700\u591A6\u4E2A\u95F9\u949F",1500)：ensuretimer()：第一次调用创建一次性软件定时器，后续修改周期。。
            overlays::Toast::instance().show("\u6700\u591A 6 \u4E2A\u95F9\u949F", 1500);
        }
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 定义变量 core::Rect list_area 并初始化为 content_rect()。
    core::Rect list_area = content_rect();
    // 赋值：把右侧 static_cast<uint16_t>(list_area.h - kAlarmAddBtnH) 的结果保存到左侧 list_area.h。
    list_area.h = static_cast<uint16_t>(list_area.h - kAlarmAddBtnH);
    // 判断条件：!list_area.contains(x, y) 是否成立。
    if (!list_area.contains(x, y))
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t visible = static_cast<uint8_t>(list_area.h / Theme::kListRowH);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t local = static_cast<uint8_t>((y - list_area.y) / Theme::kListRowH);
    // 判断条件：local >= visible 是否成立。
    if (local >= visible)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t idx = static_cast<uint8_t>(alarm_list_.scroll() + local);
    // 判断条件：idx >= st.alarm_count() 是否成立。
    if (idx >= st.alarm_count())
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect row{list_area.x, static_cast<int16_t>(list_area.y + local * Theme::kListRowH),
                         // 继续上一行的表达式或参数列表：static_cast<uint16_t>(list_area.w - Theme::kScrollbarW - 2),。
                         static_cast<uint16_t>(list_area.w - Theme::kScrollbarW - 2),
                         // 继续上一行的表达式或参数列表：Theme::kListRowH};。
                         Theme::kListRowH};

    // 判断条件：ui::widgets::hit_row_action(row, x, y) 是否成立。
    if (ui::widgets::hit_row_action(row, x, y))
    {
        // 赋值：把右侧 st.alarm_mut(idx) 的结果保存到左侧 auto& a。
        auto& a = st.alarm_mut(idx);
        // 赋值：把右侧 !a.enabled 的结果保存到左侧 a.enabled。
        a.enabled = !a.enabled;
        // 调用 save()：这是拨码映射的持久化表： load() 先设默认值，再用 NVS 覆盖合法值。 save() 把所有槽位写入 NVS 并 commit。 lookup() 按 Context 和 Key 找动作。 cycle() 在设置页循环切换到下一个可选动作。。
        st.save();
        // 调用 invalidate_body();，执行这个函数并处理它的返回值或状态变化。
        invalidate_body();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 赋值：把右侧 idx 的结果保存到左侧 edit_alarm_idx_。
    edit_alarm_idx_ = idx;
    // 赋值：把右侧 false 的结果保存到左侧 edit_is_new_。
    edit_is_new_ = false;
    // 赋值：把右侧 st.alarm(idx) 的结果保存到左侧 edit_snapshot_。
    edit_snapshot_ = st.alarm(idx);
    // 赋值：把右侧 SubPage::AlarmEdit 的结果保存到左侧 sub_。
    sub_ = SubPage::AlarmEdit;
    // 调用 invalidate_body();，执行这个函数并处理它的返回值或状态变化。
    invalidate_body();
    // 返回 {true} 给调用者，表示当前函数的执行结果。
    return {true};
}

// 定义或声明函数 ClockApp::handle_alarm_edit：参数在括号里，返回类型写在函数名前面。
shell::InputResult ClockApp::handle_alarm_edit(int16_t x, int16_t y)
{
    // 赋值：把右侧 ClockStore::get_instance() 的结果保存到左侧 auto& st。
    auto& st = ClockStore::get_instance();
    // 判断条件：edit_alarm_idx_ >= st.alarm_count() 是否成立。
    if (edit_alarm_idx_ >= st.alarm_count())
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};

    // 赋值：把右侧 st.alarm_mut(edit_alarm_idx_) 的结果保存到左侧 auto& a。
    auto& a = st.alarm_mut(edit_alarm_idx_);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const TimeStepperGeom hour_g = time_stepper_geom(alarm_edit_hour_y());
    // 只读约束：这个值或对象在当前位置不能被修改。
    const TimeStepperGeom min_g = time_stepper_geom(alarm_edit_minute_y());
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect row = hour_g.row;

    // 判断条件：!edit_is_new_ && alarm_edit_delete_rect().contains(x, y) 是否成立。
    if (!edit_is_new_ && alarm_edit_delete_rect().contains(x, y))
    {
        // 访问对象成员：st.remove_alarm(edit_alarm_idx_);；读取或修改这个对象里的成员状态。
        st.remove_alarm(edit_alarm_idx_);
        // 赋值：把右侧 false 的结果保存到左侧 edit_is_new_。
        edit_is_new_ = false;
        // 赋值：把右侧 SubPage::Main 的结果保存到左侧 sub_。
        sub_ = SubPage::Main;
        // 赋值：把右侧 Tab::Alarm 的结果保存到左侧 tab_。
        tab_ = Tab::Alarm;
        // 调用 invalidate_body();，执行这个函数并处理它的返回值或状态变化。
        invalidate_body();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：alarm_edit_ok_rect().contains(x, y) 是否成立。
    if (alarm_edit_ok_rect().contains(x, y))
    {
        // 调用 save()：这是拨码映射的持久化表： load() 先设默认值，再用 NVS 覆盖合法值。 save() 把所有槽位写入 NVS 并 commit。 lookup() 按 Context 和 Key 找动作。 cycle() 在设置页循环切换到下一个可选动作。。
        st.save();
        // 赋值：把右侧 false 的结果保存到左侧 edit_is_new_。
        edit_is_new_ = false;
        // 赋值：把右侧 SubPage::Main 的结果保存到左侧 sub_。
        sub_ = SubPage::Main;
        // 赋值：把右侧 Tab::Alarm 的结果保存到左侧 tab_。
        tab_ = Tab::Alarm;
        // 调用 invalidate_body();，执行这个函数并处理它的返回值或状态变化。
        invalidate_body();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 判断条件：hour_g.dec.contains(x, y) 是否成立。
    if (hour_g.dec.contains(x, y))
    {
        // 赋值：把右侧 (a.hour > 0) ? static_cast<uint8_t>(a.hour - 1U) : 23U 的结果保存到左侧 a.hour。
        a.hour = (a.hour > 0) ? static_cast<uint8_t>(a.hour - 1U) : 23U;
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：hour_g.inc.contains(x, y) 是否成立。
    if (hour_g.inc.contains(x, y))
    {
        // 赋值：把右侧 (a.hour < 23) ? static_cast<uint8_t>(a.hour + 1U) : 0U 的结果保存到左侧 a.hour。
        a.hour = (a.hour < 23) ? static_cast<uint8_t>(a.hour + 1U) : 0U;
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：hour_g.val.contains(x, y) 是否成立。
    if (hour_g.val.contains(x, y))
    {
        // 调用 open_hour_keyboard();，执行这个函数并处理它的返回值或状态变化。
        open_hour_keyboard();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 判断条件：min_g.dec.contains(x, y) 是否成立。
    if (min_g.dec.contains(x, y))
    {
        // 赋值：把右侧 (a.minute > 0) ? static_cast<uint8_t>(a.minute - 1U) : 59U 的结果保存到左侧 a.minute。
        a.minute = (a.minute > 0) ? static_cast<uint8_t>(a.minute - 1U) : 59U;
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：min_g.inc.contains(x, y) 是否成立。
    if (min_g.inc.contains(x, y))
    {
        // 赋值：把右侧 (a.minute < 59) ? static_cast<uint8_t>(a.minute + 1U) : 0U 的结果保存到左侧 a.minute。
        a.minute = (a.minute < 59) ? static_cast<uint8_t>(a.minute + 1U) : 0U;
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：min_g.val.contains(x, y) 是否成立。
    if (min_g.val.contains(x, y))
    {
        // 调用 open_minute_keyboard();，执行这个函数并处理它的返回值或状态变化。
        open_minute_keyboard();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 定义变量 int16_t ry 并初始化为 alarm_edit_repeat_y()。
    int16_t ry = alarm_edit_repeat_y();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect rep_row{row.x, ry, row.w, kEditRowH};
    // 判断条件：rep_row.contains(x, y) 是否成立。
    if (rep_row.contains(x, y))
    {
        // 赋值：把右侧 static_cast<AlarmRepeat>((static_cast<uint8_t>(a.repeat) + 1U) % 3U) 的结果保存到左侧 a.repeat。
        a.repeat = static_cast<AlarmRepeat>((static_cast<uint8_t>(a.repeat) + 1U) % 3U);
        // 调用 invalidate_body();，执行这个函数并处理它的返回值或状态变化。
        invalidate_body();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 赋值：把右侧 static_cast<int16_t>(ry + kEditRowH) 的结果保存到左侧 ry。
    ry = static_cast<int16_t>(ry + kEditRowH);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect en_row{row.x, ry, row.w, kEditRowH};
    // 判断条件：en_row.contains(x, y) 是否成立。
    if (en_row.contains(x, y))
    {
        // 赋值：把右侧 !a.enabled 的结果保存到左侧 a.enabled。
        a.enabled = !a.enabled;
        // 调用 invalidate_body();，执行这个函数并处理它的返回值或状态变化。
        invalidate_body();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 返回 {true} 给调用者，表示当前函数的执行结果。
    return {true};
}

// 定义或声明函数 ClockApp::on_input：参数在括号里，返回类型写在函数名前面。
shell::InputResult ClockApp::on_input(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;

    // 判断条件：ev.type == EventType::Tap 是否成立。
    if (ev.type == EventType::Tap)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t x = static_cast<int16_t>(ev.x);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t y = static_cast<int16_t>(ev.y);

        // 判断条件：ui::widgets::hit_toolbar_back(x, y) 是否成立。
        if (ui::widgets::hit_toolbar_back(x, y))
        {
            // 判断条件：sub_ == SubPage::AlarmEdit 是否成立。
            if (sub_ == SubPage::AlarmEdit)
            {
                // 调用 cancel_alarm_edit();，执行这个函数并处理它的返回值或状态变化。
                cancel_alarm_edit();
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            }
            // 返回 {} 给调用者，表示当前函数的执行结果。
            return {};
        }

        // 判断条件：sub_ == SubPage::AlarmEdit 是否成立。
        if (sub_ == SubPage::AlarmEdit)
            // 返回 handle_alarm_edit(x, y) 给调用者，表示当前函数的执行结果。
            return handle_alarm_edit(x, y);

        // 判断条件：auto r = handle_tabs(x, y); r.consumed 是否成立。
        if (auto r = handle_tabs(x, y); r.consumed)
            // 返回 r 给调用者，表示当前函数的执行结果。
            return r;

        // 判断条件：tab_ == Tab::Alarm 是否成立。
        if (tab_ == Tab::Alarm)
        {
            // 定义变量 auto out 并初始化为 alarm_list_.handle_input(ev)。
            auto out = alarm_list_.handle_input(ev);
            // 判断条件：out.scroll_changed 是否成立。
            if (out.scroll_changed)
                // 调用 invalidate_body();，执行这个函数并处理它的返回值或状态变化。
                invalidate_body();
            // 判断条件：out.consumed && !out.tap_consumed 是否成立。
            if (out.consumed && !out.tap_consumed)
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            // 返回 handle_alarm_list(x, y) 给调用者，表示当前函数的执行结果。
            return handle_alarm_list(x, y);
        }
    }

    // 判断条件：sub_ == SubPage::Main && tab_ == Tab::Alarm 是否成立。
    if (sub_ == SubPage::Main && tab_ == Tab::Alarm)
    {
        // 定义变量 auto out 并初始化为 alarm_list_.handle_input(ev)。
        auto out = alarm_list_.handle_input(ev);
        // 判断条件：out.scroll_changed 是否成立。
        if (out.scroll_changed)
            // 调用 invalidate_body();，执行这个函数并处理它的返回值或状态变化。
            invalidate_body();
        // 判断条件：out.consumed 是否成立。
        if (out.consumed)
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
    }
    // 返回 {} 给调用者，表示当前函数的执行结果。
    return {};
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::clock
