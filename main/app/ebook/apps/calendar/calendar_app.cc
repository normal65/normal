/**
 * [初学者导读] 文件：main/app/ebook/apps/calendar/calendar_app.cc
 * 作用：实现日历月视图、日期选择和月份切换。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/calendar/calendar_app.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <ctime>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/clock_provider.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/icon.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/toast.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/refresh_intent.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::calendar {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;

// 固定参数 kMonthBarH = 22：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kMonthBarH = 22;
// 固定参数 kWeekdayH = 14：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kWeekdayH = 14;
// 固定参数 kGridTopY = Theme::kListStartY + kMonthBarH + kWeekdayH：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kGridTopY = Theme::kListStartY + kMonthBarH + kWeekdayH;
// 固定参数 kNavIconSize = 14：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kNavIconSize = 14;
// 固定参数 kPickerYearMin = 1970：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kPickerYearMin = 1970;
// 固定参数 kPickerYearMax = 2099：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kPickerYearMax = 2099;

// 定义或声明函数 is_leap：参数在括号里，返回类型写在函数名前面。
bool is_leap(uint16_t y)
{
    // 返回 (y % 4U == 0U && y % 100U != 0U) || (y % 400U == 0U) 给调用者，表示当前函数的执行结果。
    return (y % 4U == 0U && y % 100U != 0U) || (y % 400U == 0U);
}

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

// 定义或声明函数 fill_dim_cell：参数在括号里，返回类型写在函数名前面。
void fill_dim_cell(gfx::Canvas& c, const core::Rect& cell)
{
    // 调用 rect(cell,1)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    c.rect(cell, 1);
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (int16_t py = cell.y; py < cell.bottom(); py += 2)
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (int16_t px = cell.x; px < cell.right(); px += 2)
            // 访问对象成员：c.pixel(px, py);；读取或修改这个对象里的成员状态。
            c.pixel(px, py);
}

// 函数调用：void draw_day_num(gfx::Canvas& c, const core::Rect& cell, uint8_t day, bool dim,；执行该函数并处理它的返回值或副作用。
void draw_day_num(gfx::Canvas& c, const core::Rect& cell, uint8_t day, bool dim,
                  // 继续上一行的表达式或参数列表：gfx::Ink ink = gfx::Ink::Black)。
                  gfx::Ink ink = gfx::Ink::Black)
{
    // 执行语句：char buf[4];；它会更新一次变量、对象或程序状态。
    char buf[4];
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    (void)std::snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(day));
    // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
    gfx::Canvas::TextStyle ts{};
    // 赋值：把右侧 dim ? Theme::kFontSmall : Theme::kFontBody 的结果保存到左侧 ts.size_px。
    ts.size_px = dim ? Theme::kFontSmall : Theme::kFontBody;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
    ts.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
    ts.v = gfx::VAlign::Middle;
    // 赋值：把右侧 ink 的结果保存到左侧 ts.ink。
    ts.ink = ink;
    // 调用 text_in(cell,buf,ts)：文本对齐。
    c.text_in(cell, buf, ts);
}

// 定义或声明函数 pk_year_row：参数在括号里，返回类型写在函数名前面。
core::Rect pk_year_row(const core::Rect& panel)
{
    // 返回 core::Rect{panel.x, static_cast<int16_t>(panel.y + 22), panel.w, 28} 给调用者，表示当前函数的执行结果。
    return core::Rect{panel.x, static_cast<int16_t>(panel.y + 22), panel.w, 28};
}
// 定义或声明函数 pk_year_prev：参数在括号里，返回类型写在函数名前面。
core::Rect pk_year_prev(const core::Rect& panel)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect r = pk_year_row(panel);
    // 返回 core::Rect{static_cast<int16_t>(r.x + 8), r.y, 36, r.h} 给调用者，表示当前函数的执行结果。
    return core::Rect{static_cast<int16_t>(r.x + 8), r.y, 36, r.h};
}
// 定义或声明函数 pk_year_next：参数在括号里，返回类型写在函数名前面。
core::Rect pk_year_next(const core::Rect& panel)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect r = pk_year_row(panel);
    // 返回 core::Rect{static_cast<int16_t>(r.right() - 44), r.y, 36, r.h} 给调用者，表示当前函数的执行结果。
    return core::Rect{static_cast<int16_t>(r.right() - 44), r.y, 36, r.h};
}
// 定义或声明函数 pk_grid：参数在括号里，返回类型写在函数名前面。
core::Rect pk_grid(const core::Rect& panel)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect y_row = pk_year_row(panel);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t top = static_cast<int16_t>(y_row.bottom() + 4);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t foot = static_cast<int16_t>(panel.bottom() - 32);
    // 返回 core::Rect{static_cast<int16_t>(panel.x + 8), top, static_cast<uint16_t>(panel.w - 16), 给调用者，表示当前函数的执行结果。
    return core::Rect{static_cast<int16_t>(panel.x + 8), top, static_cast<uint16_t>(panel.w - 16),
                      // 继续上一行的表达式或参数列表：static_cast<uint16_t>(foot - top)};。
                      static_cast<uint16_t>(foot - top)};
}
// 定义或声明函数 pk_month_cell：参数在括号里，返回类型写在函数名前面。
core::Rect pk_month_cell(const core::Rect& grid, uint8_t month)
{
    // 判断条件：month < 1 || month > 12 是否成立。
    if (month < 1 || month > 12)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t idx = static_cast<uint8_t>(month - 1);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t row = static_cast<uint8_t>(idx / 3);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t col = static_cast<uint8_t>(idx % 3);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t cw = static_cast<uint16_t>(grid.w / 3);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t ch = static_cast<uint16_t>(grid.h / 4);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x = static_cast<int16_t>(grid.x + col * cw);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y = static_cast<int16_t>(grid.y + row * ch);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t w = (col == 2) ? static_cast<uint16_t>(grid.right() - x) : cw;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t h = (row == 3) ? static_cast<uint16_t>(grid.bottom() - y) : ch;
    // 返回 core::Rect{x, y, w, h} 给调用者，表示当前函数的执行结果。
    return core::Rect{x, y, w, h};
}
// 定义或声明函数 pk_footer：参数在括号里，返回类型写在函数名前面。
core::Rect pk_footer(const core::Rect& panel)
{
    // 返回 core::Rect{panel.x, static_cast<int16_t>(panel.bottom() - 32), panel.w, 32} 给调用者，表示当前函数的执行结果。
    return core::Rect{panel.x, static_cast<int16_t>(panel.bottom() - 32), panel.w, 32};
}
// 定义或声明函数 pk_ok：参数在括号里，返回类型写在函数名前面。
core::Rect pk_ok(const core::Rect& panel)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect f = pk_footer(panel);
    // 返回 core::Rect{static_cast<int16_t>(f.x + f.w / 2), f.y, static_cast<uint16_t>(f.w / 2), 给调用者，表示当前函数的执行结果。
    return core::Rect{static_cast<int16_t>(f.x + f.w / 2), f.y, static_cast<uint16_t>(f.w / 2),
                      // 继续上一行的表达式或参数列表：f.h};。
                      f.h};
}
// 定义或声明函数 pk_cancel：参数在括号里，返回类型写在函数名前面。
core::Rect pk_cancel(const core::Rect& panel)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect f = pk_footer(panel);
    // 返回 core::Rect{f.x, f.y, static_cast<uint16_t>(f.w / 2), f.h} 给调用者，表示当前函数的执行结果。
    return core::Rect{f.x, f.y, static_cast<uint16_t>(f.w / 2), f.h};
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 CalendarApp::instance：参数在括号里，返回类型写在函数名前面。
CalendarApp& CalendarApp::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static CalendarApp s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义只读对象或变量：初始化后不能在当前位置修改。
const char* CalendarApp::title() const
{
    // 返回 ui::strings::kAppCalendar 给调用者，表示当前函数的执行结果。
    return ui::strings::kAppCalendar;
}
// 函数定义/声明：调用 CalendarApp::icon_cp()：返回图标 codepoint。图标通常来自字体图标表，所以这里返回的是 uint32t，不是图片路径。。
uint32_t CalendarApp::icon_cp() const
{
    // 返回 gfx::icon::kFaCalendarAlt 给调用者，表示当前函数的执行结果。
    return gfx::icon::kFaCalendarAlt;
}

// 定义或声明函数 CalendarApp::on_enter：参数在括号里，返回类型写在函数名前面。
void CalendarApp::on_enter()
{
    // 赋值：把右侧 false 的结果保存到左侧 picker_open_。
    picker_open_ = false;
    // 调用 sync_to_clock();，执行这个函数并处理它的返回值或状态变化。
    sync_to_clock();
    // 调用 rebuild_grid();，执行这个函数并处理它的返回值或状态变化。
    rebuild_grid();
    // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
    request_repaint(router::intent_partial_full());
}

// 函数定义/声明：调用 CalendarApp::on_exit()：退出时取消未完成的扫描、关闭弹窗，并确保后台任务不会在对象状态失效后继续写数据。。
void CalendarApp::on_exit()
{
    // 赋值：把右侧 false 的结果保存到左侧 picker_open_。
    picker_open_ = false;
}

// 定义或声明函数 CalendarApp::sync_to_clock：参数在括号里，返回类型写在函数名前面。
void CalendarApp::sync_to_clock()
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto clk = data::Clock::now();
    // 赋值：把右侧 clk.year 的结果保存到左侧 view_year_。
    view_year_ = clk.year;
    // 赋值：把右侧 clk.month 的结果保存到左侧 view_month_。
    view_month_ = clk.month;
    // 赋值：把右侧 clk.year 的结果保存到左侧 sel_year_。
    sel_year_ = clk.year;
    // 赋值：把右侧 clk.month 的结果保存到左侧 sel_month_。
    sel_month_ = clk.month;
    // 赋值：把右侧 clk.day 的结果保存到左侧 sel_day_。
    sel_day_ = clk.day;
}

// 定义或声明函数 CalendarApp::days_in_month：参数在括号里，返回类型写在函数名前面。
uint8_t CalendarApp::days_in_month(uint16_t year, uint8_t month)
{
    // 静态查表数组 kDays：运行时复用固定数据，避免每次重新计算。
    static const uint8_t kDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    // 判断条件：month < 1 || month > 12 是否成立。
    if (month < 1 || month > 12)
        // 返回 30 给调用者，表示当前函数的执行结果。
        return 30;
    // 判断条件：month == 2 && is_leap(year) 是否成立。
    if (month == 2 && is_leap(year))
        // 返回 29 给调用者，表示当前函数的执行结果。
        return 29;
    // 返回 kDays[month - 1] 给调用者，表示当前函数的执行结果。
    return kDays[month - 1];
}

// 定义或声明函数 CalendarApp::weekday_sun0：参数在括号里，返回类型写在函数名前面。
uint8_t CalendarApp::weekday_sun0(uint16_t year, uint8_t month, uint8_t day)
{
    // struct tm：把相关数据字段组合在一起，表示项目中的一个概念。
    struct tm t{};
    // 赋值：把右侧 static_cast<int>(year) - 1900 的结果保存到左侧 t.tm_year。
    t.tm_year = static_cast<int>(year) - 1900;
    // 赋值：把右侧 static_cast<int>(month) - 1 的结果保存到左侧 t.tm_mon。
    t.tm_mon = static_cast<int>(month) - 1;
    // 赋值：把右侧 static_cast<int>(day) 的结果保存到左侧 t.tm_mday。
    t.tm_mday = static_cast<int>(day);
    // 赋值：把右侧 -1 的结果保存到左侧 t.tm_isdst。
    t.tm_isdst = -1;
    // 函数调用：(void)mktime(&t);；执行该函数并处理它的返回值或副作用。
    (void)mktime(&t);
    // 返回 static_cast<uint8_t>(t.tm_wday) 给调用者，表示当前函数的执行结果。
    return static_cast<uint8_t>(t.tm_wday);
}

// 定义或声明函数 CalendarApp::shift_month：参数在括号里，返回类型写在函数名前面。
void CalendarApp::shift_month(int delta)
{
    // 定义变量 int y 并初始化为 static_cast<int>(view_year_)。
    int y = static_cast<int>(view_year_);
    // 定义变量 int m 并初始化为 static_cast<int>(view_month_) + delta。
    int m = static_cast<int>(view_month_) + delta;
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (m < 1)
    {
        // 执行语句：--y;；它会更新一次变量、对象或程序状态。
        --y;
        // 赋值：把右侧 12 的结果保存到左侧 m +。
        m += 12;
    }
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (m > 12)
    {
        // 执行语句：++y;；它会更新一次变量、对象或程序状态。
        ++y;
        // 赋值：把右侧 12 的结果保存到左侧 m -。
        m -= 12;
    }
    // 赋值：把右侧 static_cast<uint16_t>(y) 的结果保存到左侧 view_year_。
    view_year_ = static_cast<uint16_t>(y);
    // 赋值：把右侧 static_cast<uint8_t>(m) 的结果保存到左侧 view_month_。
    view_month_ = static_cast<uint8_t>(m);

    // 判断条件：sel_year_ == view_year_ && sel_month_ == view_month_ 是否成立。
    if (sel_year_ == view_year_ && sel_month_ == view_month_)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t dim = days_in_month(view_year_, view_month_);
        // 判断条件：sel_day_ > dim 是否成立。
        if (sel_day_ > dim)
            // 赋值：把右侧 dim 的结果保存到左侧 sel_day_。
            sel_day_ = dim;
    }

    // 调用 rebuild_grid();，执行这个函数并处理它的返回值或状态变化。
    rebuild_grid();
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 定义或声明函数 CalendarApp::rebuild_grid：参数在括号里，返回类型写在函数名前面。
void CalendarApp::rebuild_grid()
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto today = data::Clock::now();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t dim = days_in_month(view_year_, view_month_);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t w0 = weekday_sun0(view_year_, view_month_, 1);

    // 定义变量 uint16_t py 并初始化为 view_year_。
    uint16_t py = view_year_;
    // 定义变量 uint8_t pm 并初始化为 view_month_。
    uint8_t pm = view_month_;
    // 判断条件：pm == 1 是否成立。
    if (pm == 1)
    {
        // 赋值：把右侧 12 的结果保存到左侧 pm。
        pm = 12;
        // 执行语句：--py;；它会更新一次变量、对象或程序状态。
        --py;
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 执行语句：--pm;；它会更新一次变量、对象或程序状态。
        --pm;
    }
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t prev_dim = days_in_month(py, pm);

    // 定义变量 uint16_t ny 并初始化为 view_year_。
    uint16_t ny = view_year_;
    // 定义变量 uint8_t nm 并初始化为 view_month_。
    uint8_t nm = view_month_;
    // 判断条件：nm == 12 是否成立。
    if (nm == 12)
    {
        // 赋值：把右侧 1 的结果保存到左侧 nm。
        nm = 1;
        // 执行语句：++ny;；它会更新一次变量、对象或程序状态。
        ++ny;
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 执行语句：++nm;；它会更新一次变量、对象或程序状态。
        ++nm;
    }

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < kGridCols * kGridRows; ++i)
    {
        // 赋值：把右侧 cells_[i] 的结果保存到左侧 DayCell& c。
        DayCell& c = cells_[i];
        // 赋值：把右侧 {} 的结果保存到左侧 c。
        c = {};

        // 只读约束：这个值或对象在当前位置不能被修改。
        const int slot = static_cast<int>(i) - static_cast<int>(w0) + 1;
        // 判断条件：slot < 1 是否成立。
        if (slot < 1)
        {
            // 赋值：把右侧 py 的结果保存到左侧 c.year。
            c.year = py;
            // 赋值：把右侧 pm 的结果保存到左侧 c.month。
            c.month = pm;
            // 赋值：把右侧 static_cast<uint8_t>(slot + static_cast<int>(prev_dim)) 的结果保存到左侧 c.day。
            c.day = static_cast<uint8_t>(slot + static_cast<int>(prev_dim));
            // 赋值：把右侧 false 的结果保存到左侧 c.in_month。
            c.in_month = false;
        }
        // 备用分支：前面的条件不成立时执行这里。
        else if (slot > static_cast<int>(dim))
        {
            // 赋值：把右侧 ny 的结果保存到左侧 c.year。
            c.year = ny;
            // 赋值：把右侧 nm 的结果保存到左侧 c.month。
            c.month = nm;
            // 赋值：把右侧 static_cast<uint8_t>(slot - static_cast<int>(dim)) 的结果保存到左侧 c.day。
            c.day = static_cast<uint8_t>(slot - static_cast<int>(dim));
            // 赋值：把右侧 false 的结果保存到左侧 c.in_month。
            c.in_month = false;
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 赋值：把右侧 view_year_ 的结果保存到左侧 c.year。
            c.year = view_year_;
            // 赋值：把右侧 view_month_ 的结果保存到左侧 c.month。
            c.month = view_month_;
            // 赋值：把右侧 static_cast<uint8_t>(slot) 的结果保存到左侧 c.day。
            c.day = static_cast<uint8_t>(slot);
            // 赋值：把右侧 true 的结果保存到左侧 c.in_month。
            c.in_month = true;
        }

        // 判断条件：c.day > 0 是否成立。
        if (c.day > 0)
            // 赋值：把右侧 (c.year == today.year && c.month == today.month && c.day == today.day) 的结果保存到左侧 c.is_today。
            c.is_today = (c.year == today.year && c.month == today.month && c.day == today.day);
    }
}

// 定义或声明函数 CalendarApp::month_bar_rect：参数在括号里，返回类型写在函数名前面。
core::Rect CalendarApp::month_bar_rect()
{
    // 返回 core::Rect{0, static_cast<int16_t>(Theme::kListStartY), Theme::kScreenW, kMonthBarH} 给调用者，表示当前函数的执行结果。
    return core::Rect{0, static_cast<int16_t>(Theme::kListStartY), Theme::kScreenW, kMonthBarH};
}

// 定义或声明函数 CalendarApp::month_title_rect：参数在括号里，返回类型写在函数名前面。
core::Rect CalendarApp::month_title_rect()
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect bar = month_bar_rect();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t btn_w = static_cast<uint16_t>(Theme::kScreenW / 4);
    // 返回 core::Rect{static_cast<int16_t>(btn_w), bar.y, 给调用者，表示当前函数的执行结果。
    return core::Rect{static_cast<int16_t>(btn_w), bar.y,
                      // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kScreenW - btn_w * 2), bar.h};。
                      static_cast<uint16_t>(Theme::kScreenW - btn_w * 2), bar.h};
}

// 定义或声明函数 CalendarApp::weekday_row_rect：参数在括号里，返回类型写在函数名前面。
core::Rect CalendarApp::weekday_row_rect()
{
    // 返回 core::Rect{0, static_cast<int16_t>(Theme::kListStartY + kMonthBarH), Theme::kScreenW, 给调用者，表示当前函数的执行结果。
    return core::Rect{0, static_cast<int16_t>(Theme::kListStartY + kMonthBarH), Theme::kScreenW,
                      // 继续上一行的表达式或参数列表：kWeekdayH};。
                      kWeekdayH};
}

// 定义或声明函数 CalendarApp::grid_region_rect：参数在括号里，返回类型写在函数名前面。
core::Rect CalendarApp::grid_region_rect()
{
    // 返回 core::Rect{0, static_cast<int16_t>(kGridTopY), Theme::kScreenW, 给调用者，表示当前函数的执行结果。
    return core::Rect{0, static_cast<int16_t>(kGridTopY), Theme::kScreenW,
                      // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kScreenH - kGridTopY)};。
                      static_cast<uint16_t>(Theme::kScreenH - kGridTopY)};
}

// 定义或声明函数 CalendarApp::cell_rect：参数在括号里，返回类型写在函数名前面。
core::Rect CalendarApp::cell_rect(uint8_t row, uint8_t col)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t cell_w = static_cast<uint16_t>(Theme::kScreenW / kGridCols);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t grid_h = static_cast<uint16_t>(Theme::kScreenH - kGridTopY);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t cell_h = static_cast<uint16_t>(grid_h / kGridRows);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x = static_cast<int16_t>(col * cell_w);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y = static_cast<int16_t>(kGridTopY + row * cell_h);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t w =
        // 继续上一行的表达式或参数列表：(col + 1 == kGridCols) ? static_cast<uint16_t>(Theme::kScreenW - col * cell_w) : cell_w;。
        (col + 1 == kGridCols) ? static_cast<uint16_t>(Theme::kScreenW - col * cell_w) : cell_w;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t hh =
        // 继续上一行的表达式或参数列表：(row + 1 == kGridRows) ? static_cast<uint16_t>(Theme::kScreenH - y) : cell_h;。
        (row + 1 == kGridRows) ? static_cast<uint16_t>(Theme::kScreenH - y) : cell_h;
    // 返回 core::Rect{x, y, w, hh} 给调用者，表示当前函数的执行结果。
    return core::Rect{x, y, w, hh};
}

// 定义或声明函数 CalendarApp::picker_panel_rect：参数在括号里，返回类型写在函数名前面。
core::Rect CalendarApp::picker_panel_rect()
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y = static_cast<int16_t>(Theme::kListStartY + kMonthBarH + 2);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t h = static_cast<uint16_t>(Theme::kScreenH - y - 4);
    // 返回 core::Rect{8, y, static_cast<uint16_t>(Theme::kScreenW - 16), h} 给调用者，表示当前函数的执行结果。
    return core::Rect{8, y, static_cast<uint16_t>(Theme::kScreenW - 16), h};
}

// 定义或声明函数 CalendarApp::open_picker：参数在括号里，返回类型写在函数名前面。
void CalendarApp::open_picker()
{
    // 赋值：把右侧 true 的结果保存到左侧 picker_open_。
    picker_open_ = true;
    // 赋值：把右侧 view_year_ 的结果保存到左侧 picker_year_。
    picker_year_ = view_year_;
    // 赋值：把右侧 view_month_ 的结果保存到左侧 picker_month_。
    picker_month_ = view_month_;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 定义或声明函数 CalendarApp::close_picker：参数在括号里，返回类型写在函数名前面。
void CalendarApp::close_picker()
{
    // 判断条件：!picker_open_ 是否成立。
    if (!picker_open_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 false 的结果保存到左侧 picker_open_。
    picker_open_ = false;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 定义或声明函数 CalendarApp::apply_picker：参数在括号里，返回类型写在函数名前面。
void CalendarApp::apply_picker()
{
    // 赋值：把右侧 picker_year_ 的结果保存到左侧 view_year_。
    view_year_ = picker_year_;
    // 赋值：把右侧 picker_month_ 的结果保存到左侧 view_month_。
    view_month_ = picker_month_;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t dim = days_in_month(view_year_, view_month_);
    // 赋值：把右侧 view_year_ 的结果保存到左侧 sel_year_。
    sel_year_ = view_year_;
    // 赋值：把右侧 view_month_ 的结果保存到左侧 sel_month_。
    sel_month_ = view_month_;
    // 判断条件：sel_day_ > dim 是否成立。
    if (sel_day_ > dim)
        // 赋值：把右侧 dim 的结果保存到左侧 sel_day_。
        sel_day_ = dim;
    // 判断条件：sel_day_ < 1 是否成立。
    if (sel_day_ < 1)
        // 赋值：把右侧 1 的结果保存到左侧 sel_day_。
        sel_day_ = 1;

    // 调用 rebuild_grid();，执行这个函数并处理它的返回值或状态变化。
    rebuild_grid();
    // 调用 close_picker();，执行这个函数并处理它的返回值或状态变化。
    close_picker();
}

// 定义或声明函数 CalendarApp::paint_picker：参数在括号里，返回类型写在函数名前面。
void CalendarApp::paint_picker(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect panel = picker_panel_rect();
    // 访问对象成员：c.fill(panel, gfx::Ink::White);；读取或修改这个对象里的成员状态。
    c.fill(panel, gfx::Ink::White);
    // 调用 rect(panel,2)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    c.rect(panel, 2);

    // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
    gfx::Canvas::TextStyle ts{};
    // 赋值：把右侧 Theme::kFontTitle 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontTitle;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
    ts.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
    ts.v = gfx::VAlign::Middle;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect hdr{panel.x, panel.y, panel.w, 22};
    // 调用 text_in(hdr,"\u9009\u62E9\u5E74\u6708",ts)：文本对齐。
    c.text_in(hdr, "\u9009\u62E9\u5E74\u6708", ts);

    // 调用 draw_nav_btn(c, pk_year_prev(panel), gfx::icon::kFaChevronLeft);，执行这个函数并处理它的返回值或状态变化。
    draw_nav_btn(c, pk_year_prev(panel), gfx::icon::kFaChevronLeft);
    // 调用 draw_nav_btn(c, pk_year_next(panel), gfx::icon::kFaChevronRight);，执行这个函数并处理它的返回值或状态变化。
    draw_nav_btn(c, pk_year_next(panel), gfx::icon::kFaChevronRight);

    // 执行语句：char yr_buf[16];；它会更新一次变量、对象或程序状态。
    char yr_buf[16];
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    (void)std::snprintf(yr_buf, sizeof(yr_buf), "%u\u5E74", static_cast<unsigned>(picker_year_));
    // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontBody;
    // 访问对象成员：c.text_in(pk_year_row(panel), yr_buf, ts);；读取或修改这个对象里的成员状态。
    c.text_in(pk_year_row(panel), yr_buf, ts);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect grid = pk_grid(panel);
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t m = 1; m <= 12; ++m)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect cell = pk_month_cell(grid, m);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const bool picked = (m == picker_month_);

        // 判断条件：picked 是否成立。
        if (picked)
            // 访问对象成员：c.fill(cell, gfx::Ink::Black);；读取或修改这个对象里的成员状态。
            c.fill(cell, gfx::Ink::Black);
        // 备用分支：前面的条件不成立时执行这里。
        else
            // 调用 rect(cell,1)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
            c.rect(cell, 1);

        // 执行语句：char buf[8];；它会更新一次变量、对象或程序状态。
        char buf[8];
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        (void)std::snprintf(buf, sizeof(buf), "%u\u6708", static_cast<unsigned>(m));
        // 赋值：把右侧 picked ? gfx::Ink::White : gfx::Ink::Black 的结果保存到左侧 ts.ink。
        ts.ink = picked ? gfx::Ink::White : gfx::Ink::Black;
        // 调用 text_in(cell,buf,ts)：文本对齐。
        c.text_in(cell, buf, ts);
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect foot = pk_footer(panel);
    // 访问对象成员：c.hline(foot.x, foot.y, foot.w);；读取或修改这个对象里的成员状态。
    c.hline(foot.x, foot.y, foot.w);
    // 赋值：把右侧 gfx::Ink::Black 的结果保存到左侧 ts.ink。
    ts.ink = gfx::Ink::Black;
    // 访问对象成员：c.text_in(pk_cancel(panel), ui::strings::kCancel, ts);；读取或修改这个对象里的成员状态。
    c.text_in(pk_cancel(panel), ui::strings::kCancel, ts);
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    c.vline(static_cast<int16_t>(foot.x + foot.w / 2), foot.y, foot.h);
    // 访问对象成员：c.text_in(pk_ok(panel), ui::strings::kOk, ts);；读取或修改这个对象里的成员状态。
    c.text_in(pk_ok(panel), ui::strings::kOk, ts);
}

// 定义或声明函数 CalendarApp::handle_picker_input：参数在括号里，返回类型写在函数名前面。
shell::InputResult CalendarApp::handle_picker_input(int16_t x, int16_t y)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect panel = picker_panel_rect();
    // 判断条件：!panel.contains(x, y) 是否成立。
    if (!panel.contains(x, y))
    {
        // 调用 close_picker();，执行这个函数并处理它的返回值或状态变化。
        close_picker();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 判断条件：pk_cancel(panel).contains(x, y) 是否成立。
    if (pk_cancel(panel).contains(x, y))
    {
        // 调用 close_picker();，执行这个函数并处理它的返回值或状态变化。
        close_picker();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：pk_ok(panel).contains(x, y) 是否成立。
    if (pk_ok(panel).contains(x, y))
    {
        // 调用 apply_picker();，执行这个函数并处理它的返回值或状态变化。
        apply_picker();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 判断条件：pk_year_prev(panel).contains(x, y) 是否成立。
    if (pk_year_prev(panel).contains(x, y))
    {
        // 判断条件：picker_year_ > kPickerYearMin 是否成立。
        if (picker_year_ > kPickerYearMin)
            // 执行语句：--picker_year_;；它会更新一次变量、对象或程序状态。
            --picker_year_;
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：pk_year_next(panel).contains(x, y) 是否成立。
    if (pk_year_next(panel).contains(x, y))
    {
        // 判断条件：picker_year_ < kPickerYearMax 是否成立。
        if (picker_year_ < kPickerYearMax)
            // 执行语句：++picker_year_;；它会更新一次变量、对象或程序状态。
            ++picker_year_;
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect grid = pk_grid(panel);
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t m = 1; m <= 12; ++m)
    {
        // 判断条件：pk_month_cell(grid, m).contains(x, y) 是否成立。
        if (pk_month_cell(grid, m).contains(x, y))
        {
            // 赋值：把右侧 m 的结果保存到左侧 picker_month_。
            picker_month_ = m;
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            request_repaint();
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        }
    }
    // 返回 {true} 给调用者，表示当前函数的执行结果。
    return {true};
}

// 定义或声明函数 CalendarApp::select_cell：参数在括号里，返回类型写在函数名前面。
void CalendarApp::select_cell(uint8_t index)
{
    // 判断条件：index >= kGridCols * kGridRows 是否成立。
    if (index >= kGridCols * kGridRows)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const DayCell& cell = cells_[index];
    // 判断条件：cell.day == 0 是否成立。
    if (cell.day == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 cell.year 的结果保存到左侧 sel_year_。
    sel_year_ = cell.year;
    // 赋值：把右侧 cell.month 的结果保存到左侧 sel_month_。
    sel_month_ = cell.month;
    // 赋值：把右侧 cell.day 的结果保存到左侧 sel_day_。
    sel_day_ = cell.day;

    // 判断条件：!cell.in_month 是否成立。
    if (!cell.in_month)
    {
        // 赋值：把右侧 cell.year 的结果保存到左侧 view_year_。
        view_year_ = cell.year;
        // 赋值：把右侧 cell.month 的结果保存到左侧 view_month_。
        view_month_ = cell.month;
        // 调用 rebuild_grid();，执行这个函数并处理它的返回值或状态变化。
        rebuild_grid();
    }

    // 声明并默认初始化对象：data::Clock clk{}。
    data::Clock clk{};
    // 赋值：把右侧 cell.year 的结果保存到左侧 clk.year。
    clk.year = cell.year;
    // 赋值：把右侧 cell.month 的结果保存到左侧 clk.month。
    clk.month = cell.month;
    // 赋值：把右侧 cell.day 的结果保存到左侧 clk.day。
    clk.day = cell.day;
    // 赋值：把右侧 weekday_sun0(cell.year, cell.month, cell.day) 的结果保存到左侧 clk.weekday。
    clk.weekday = weekday_sun0(cell.year, cell.month, cell.day);

    // 执行语句：char date_buf[24];；它会更新一次变量、对象或程序状态。
    char date_buf[24];
    // 访问对象成员：clk.format_date_cn(date_buf, sizeof(date_buf));；读取或修改这个对象里的成员状态。
    clk.format_date_cn(date_buf, sizeof(date_buf));
    // 执行语句：char msg[40];；它会更新一次变量、对象或程序状态。
    char msg[40];
    // 访问对象成员：(void)std::snprintf(msg, sizeof(msg), "%s %s", date_buf, clk.weekday_name_cn());；读取或修改这个对象里的成员状态。
    (void)std::snprintf(msg, sizeof(msg), "%s %s", date_buf, clk.weekday_name_cn());
    // 调用 show(msg,1800)：ensuretimer()：第一次调用创建一次性软件定时器，后续修改周期。。
    overlays::Toast::instance().show(msg, 1800);

    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 定义或声明函数 CalendarApp::paint：参数在括号里，返回类型写在函数名前面。
void CalendarApp::paint(gfx::Canvas& c)
{
    // 调用 title()：返回 App 标题。在文件浏览器内，标题有时会动态反映当前目录，因此可能存在一个 buffer，而不是只返回一个常量字符串。。
    ui::widgets::toolbar(c, title());

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect bar = month_bar_rect();
    // 访问对象成员：c.hline(bar.x, bar.y, bar.w);；读取或修改这个对象里的成员状态。
    c.hline(bar.x, bar.y, bar.w);
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    c.hline(bar.x, static_cast<int16_t>(bar.bottom() - 1), bar.w);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t btn_w = static_cast<uint16_t>(Theme::kScreenW / 4);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect prev_btn{bar.x, bar.y, btn_w, bar.h};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect next_btn{static_cast<int16_t>(bar.right() - btn_w), bar.y, btn_w, bar.h};
    // 调用 draw_nav_btn(c, prev_btn, gfx::icon::kFaChevronLeft);，执行这个函数并处理它的返回值或状态变化。
    draw_nav_btn(c, prev_btn, gfx::icon::kFaChevronLeft);
    // 调用 draw_nav_btn(c, next_btn, gfx::icon::kFaChevronRight);，执行这个函数并处理它的返回值或状态变化。
    draw_nav_btn(c, next_btn, gfx::icon::kFaChevronRight);

    // 执行语句：char title_buf[20];；它会更新一次变量、对象或程序状态。
    char title_buf[20];
    // 函数调用：(void)std::snprintf(title_buf, sizeof(title_buf), "%u\u5E74%u\u6708",；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(title_buf, sizeof(title_buf), "%u\u5E74%u\u6708",
                        // 继续上一行的表达式或参数列表：static_cast<unsigned>(view_year_), static_cast<unsigned>(view_month_));。
                        static_cast<unsigned>(view_year_), static_cast<unsigned>(view_month_));
    // 声明并默认初始化对象：gfx::Canvas::TextStyle title_ts{}。
    gfx::Canvas::TextStyle title_ts{};
    // 赋值：把右侧 Theme::kFontTitle 的结果保存到左侧 title_ts.size_px。
    title_ts.size_px = Theme::kFontTitle;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 title_ts.h。
    title_ts.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 title_ts.v。
    title_ts.v = gfx::VAlign::Middle;
    // 访问对象成员：c.text_in(month_title_rect(), title_buf, title_ts);；读取或修改这个对象里的成员状态。
    c.text_in(month_title_rect(), title_buf, title_ts);

    // 判断条件：picker_open_ 是否成立。
    if (picker_open_)
    {
        // 调用 paint_picker(c);，执行这个函数并处理它的返回值或状态变化。
        paint_picker(c);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect wd_row = weekday_row_rect();
    // 静态查表数组 kWd：运行时复用固定数据，避免每次重新计算。
    static const char* const kWd[] = {"\u65E5", "\u4E00", "\u4E8C", "\u4E09",
                                      // 继续上一行的表达式或参数列表："\u56DB", "\u4E94", "\u516D"};。
                                      "\u56DB", "\u4E94", "\u516D"};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t wd_cw = static_cast<uint16_t>(Theme::kScreenW / kGridCols);
    // 声明并默认初始化对象：gfx::Canvas::TextStyle wd_ts{}。
    gfx::Canvas::TextStyle wd_ts{};
    // 赋值：把右侧 Theme::kFontSmall 的结果保存到左侧 wd_ts.size_px。
    wd_ts.size_px = Theme::kFontSmall;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 wd_ts.h。
    wd_ts.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 wd_ts.v。
    wd_ts.v = gfx::VAlign::Middle;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t col = 0; col < kGridCols; ++col)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t x = static_cast<int16_t>(col * wd_cw);
        // 访问对象成员：c.text_in(core::Rect{x, wd_row.y, wd_cw, wd_row.h}, kWd[col], wd_ts);；读取或修改这个对象里的成员状态。
        c.text_in(core::Rect{x, wd_row.y, wd_cw, wd_row.h}, kWd[col], wd_ts);
    }
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    c.hline(0, static_cast<int16_t>(wd_row.bottom() - 1), Theme::kScreenW);

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t row = 0; row < kGridRows; ++row)
    {
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint8_t col = 0; col < kGridCols; ++col)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint8_t idx = static_cast<uint8_t>(row * kGridCols + col);
            // 只读约束：这个值或对象在当前位置不能被修改。
            const DayCell& dc = cells_[idx];
            // 判断条件：dc.day == 0 是否成立。
            if (dc.day == 0)
                // 继续下一轮：跳过本次循环剩余代码。
                continue;

            // 只读约束：这个值或对象在当前位置不能被修改。
            const core::Rect cell = cell_rect(row, col);
            // 只读约束：这个值或对象在当前位置不能被修改。
            const bool selected =
                // 继续上一行的表达式或参数列表：(dc.year == sel_year_ && dc.month == sel_month_ && dc.day == sel_day_);。
                (dc.year == sel_year_ && dc.month == sel_month_ && dc.day == sel_day_);

            // 判断条件：selected 是否成立。
            if (selected)
                // 访问对象成员：c.fill(cell, gfx::Ink::Black);；读取或修改这个对象里的成员状态。
                c.fill(cell, gfx::Ink::Black);
            // 备用分支：前面的条件不成立时执行这里。
            else if (!dc.in_month)
                // 调用 fill_dim_cell(c, cell);，执行这个函数并处理它的返回值或状态变化。
                fill_dim_cell(c, cell);
            // 备用分支：前面的条件不成立时执行这里。
            else
                // 调用 rect(cell,1)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
                c.rect(cell, 1);

            // 判断条件：dc.is_today && !selected 是否成立。
            if (dc.is_today && !selected)
            {
                // 只读约束：这个值或对象在当前位置不能被修改。
                const int16_t cx = static_cast<int16_t>(cell.x + cell.w / 2);
                // 只读约束：这个值或对象在当前位置不能被修改。
                const int16_t cy = static_cast<int16_t>(cell.y + 2);
                // 访问对象成员：c.vline(cx, cy, 4);；读取或修改这个对象里的成员状态。
                c.vline(cx, cy, 4);
            }

            // 调用 draw_day_num(c, cell, dc.day, !dc.in_month,，执行这个函数并处理它的返回值或状态变化。
            draw_day_num(c, cell, dc.day, !dc.in_month,
                         // 继续上一行的表达式或参数列表：selected ? gfx::Ink::White : gfx::Ink::Black);。
                         selected ? gfx::Ink::White : gfx::Ink::Black);
        }
    }
}

// 定义或声明函数 CalendarApp::on_input：参数在括号里，返回类型写在函数名前面。
shell::InputResult CalendarApp::on_input(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;
    // 判断条件：ev.type != EventType::Tap 是否成立。
    if (ev.type != EventType::Tap)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x = static_cast<int16_t>(ev.x);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y = static_cast<int16_t>(ev.y);

    // 判断条件：ui::widgets::hit_toolbar_back(x, y) 是否成立。
    if (ui::widgets::hit_toolbar_back(x, y))
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};
    // 判断条件：picker_open_ 是否成立。
    if (picker_open_)
        // 返回 handle_picker_input(x, y) 给调用者，表示当前函数的执行结果。
        return handle_picker_input(x, y);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect bar = month_bar_rect();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t btn_w = static_cast<uint16_t>(Theme::kScreenW / 4);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect prev_btn{bar.x, bar.y, btn_w, bar.h};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect next_btn{static_cast<int16_t>(bar.right() - btn_w), bar.y, btn_w, bar.h};

    // 判断条件：prev_btn.contains(x, y) 是否成立。
    if (prev_btn.contains(x, y))
    {
        // 调用 shift_month(-1);，执行这个函数并处理它的返回值或状态变化。
        shift_month(-1);
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：next_btn.contains(x, y) 是否成立。
    if (next_btn.contains(x, y))
    {
        // 调用 shift_month(1);，执行这个函数并处理它的返回值或状态变化。
        shift_month(1);
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect title_hit = month_title_rect();
    // 判断条件：title_hit.contains(x, y) 是否成立。
    if (title_hit.contains(x, y))
    {
        // 调用 open_picker();，执行这个函数并处理它的返回值或状态变化。
        open_picker();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t row = 0; row < kGridRows; ++row)
    {
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint8_t col = 0; col < kGridCols; ++col)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const core::Rect cell = cell_rect(row, col);
            // 判断条件：!cell.contains(x, y) 是否成立。
            if (!cell.contains(x, y))
                // 继续下一轮：跳过本次循环剩余代码。
                continue;
            // 调用 select_cell(static_cast<uint8_t>(row * kGridCols + col));，执行这个函数并处理它的返回值或状态变化。
            select_cell(static_cast<uint8_t>(row * kGridCols + col));
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        }
    }

    // 返回 {} 给调用者，表示当前函数的执行结果。
    return {};
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::calendar
