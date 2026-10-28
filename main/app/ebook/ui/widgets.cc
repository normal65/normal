/**
 * [初学者导读] 文件：main/app/ebook/ui/widgets.cc
 * 作用：提供工具栏、列表行、电池、滚动条、网格等通用绘制控件。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：UI 状态只能在 UiLoop/UI 线程修改，跨任务结果通过 UiBus 回 UI。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/icon.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/text_layout.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::ui::widgets {

// 函数定义/声明：调用 label()：label()： 直接调用 Canvas 文本绘制，不负责裁剪，所以调用者传入的位置必须已经合适。。
void label(gfx::Canvas& c, int16_t x, int16_t y_top, const char* text, uint8_t size_px)
{
    // 判断条件：text == nullptr 是否成立。
    if (text == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 text(x,y_top,text,size_px)：glyph()： 接收一个 Unicode 码点，委托 Font::drawglyph() 画一个字形，并返回该字形的推进宽度。。
    c.text(x, y_top, text, size_px);
}

// 函数调用：void label_in(gfx::Canvas& c, const core::Rect& box, const char* text, gfx::HAlign h, gfx::VAlign v,；执行该函数并处理它的返回值或副作用。
void label_in(gfx::Canvas& c, const core::Rect& box, const char* text, gfx::HAlign h, gfx::VAlign v,
              // 继续上一行的表达式或参数列表：uint8_t size_px)。
              uint8_t size_px)
{
    // 判断条件：text == nullptr || box.empty() 是否成立。
    if (text == nullptr || box.empty())
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 声明并默认初始化对象：gfx::Canvas::TextStyle s{}。
    gfx::Canvas::TextStyle s{};
    // 赋值：把右侧 size_px 的结果保存到左侧 s.size_px。
    s.size_px = size_px;
    // 赋值：把右侧 h 的结果保存到左侧 s.h。
    s.h = h;
    // 赋值：把右侧 v 的结果保存到左侧 s.v。
    s.v = v;
    // 调用 text_in(box,text,s)：文本对齐。
    c.text_in(box, text, s);
}

// 函数调用：void empty_state(gfx::Canvas& c, const core::Rect& box, const char* title,；执行该函数并处理它的返回值或副作用。
void empty_state(gfx::Canvas& c, const core::Rect& box, const char* title,
                 // 只读约束：这个值或对象在当前位置不能被修改。
                 const char* const* hint_lines, uint8_t hint_line_count)
{
    // 判断条件：box.empty() || title == nullptr 是否成立。
    if (box.empty() || title == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 固定参数 kTitleSize = Theme::kFontBody：由当前模块定义，用于控制布局、时序或硬件行为。
    constexpr uint8_t kTitleSize = Theme::kFontBody;
    // 固定参数 kHintSize = Theme::kFontSmall：由当前模块定义，用于控制布局、时序或硬件行为。
    constexpr uint8_t kHintSize = Theme::kFontSmall;
    // 界面间距：6 像素。
    constexpr uint8_t kGap = 6;
    // 固定参数 kMaxHints = 4：由当前模块定义，用于控制布局、时序或硬件行为。
    constexpr uint8_t kMaxHints = 4;

    // 判断条件：hint_line_count > kMaxHints 是否成立。
    if (hint_line_count > kMaxHints)
        // 赋值：把右侧 kMaxHints 的结果保存到左侧 hint_line_count。
        hint_line_count = kMaxHints;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t title_lh = static_cast<uint8_t>(kTitleSize + 4);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t hint_lh = static_cast<uint8_t>(kHintSize + 2);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t block_h = static_cast<uint16_t>(
        // 继续上一行的表达式或参数列表：title_lh +。
        title_lh +
        // 继续上一行的表达式或参数列表：(hint_line_count > 0 ? kGap + static_cast<uint16_t>(hint_line_count) * hint_lh : 0));。
        (hint_line_count > 0 ? kGap + static_cast<uint16_t>(hint_line_count) * hint_lh : 0));
    // 定义变量 int16_t y 并初始化为 static_cast<int16_t>(box.y + (box.h > block_h ? (box.h - block_h) / 2 : 0))。
    int16_t y = static_cast<int16_t>(box.y + (box.h > block_h ? (box.h - block_h) / 2 : 0));

    // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
    gfx::Canvas::TextStyle ts{};
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
    ts.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
    ts.v = gfx::VAlign::Middle;

    // 执行语句：char line_buf[40];；它会更新一次变量、对象或程序状态。
    char line_buf[40];

    // 赋值：把右侧 kTitleSize 的结果保存到左侧 ts.size_px。
    ts.size_px = kTitleSize;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect title_row{box.x, y, box.w, title_lh};
    // 访问对象成员：(void)gfx::truncate_text(title, kTitleSize, box.w, line_buf, sizeof(line_buf));；读取或修改这个对象里的成员状态。
    (void)gfx::truncate_text(title, kTitleSize, box.w, line_buf, sizeof(line_buf));
    // 调用 text_in(title_row,line_buf,ts)：文本对齐。
    c.text_in(title_row, line_buf, ts);
    // 赋值：把右侧 static_cast<int16_t>(y + title_lh) 的结果保存到左侧 y。
    y = static_cast<int16_t>(y + title_lh);

    // 判断条件：hint_line_count == 0 || hint_lines == nullptr 是否成立。
    if (hint_line_count == 0 || hint_lines == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 static_cast<int16_t>(y + kGap) 的结果保存到左侧 y。
    y = static_cast<int16_t>(y + kGap);
    // 赋值：把右侧 kHintSize 的结果保存到左侧 ts.size_px。
    ts.size_px = kHintSize;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < hint_line_count; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* hint = hint_lines[i];
        // 判断条件：hint == nullptr || hint[0] == '\0' 是否成立。
        if (hint == nullptr || hint[0] == '\0')
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect row{box.x, y, box.w, hint_lh};
        // 访问对象成员：(void)gfx::truncate_text(hint, kHintSize, box.w, line_buf, sizeof(line_buf));；读取或修改这个对象里的成员状态。
        (void)gfx::truncate_text(hint, kHintSize, box.w, line_buf, sizeof(line_buf));
        // 调用 text_in(row,line_buf,ts)：文本对齐。
        c.text_in(row, line_buf, ts);
        // 赋值：把右侧 static_cast<int16_t>(y + hint_lh) 的结果保存到左侧 y。
        y = static_cast<int16_t>(y + hint_lh);
    }
}

// 函数定义/声明：调用 toolbar()：toolbar()： 在固定工具栏矩形中画返回图标、标题和底部横线。返回图标使用 Font Awesome 的码点，而不是手绘 SVG。。
void toolbar(gfx::Canvas& c, const char* title)
{
    // 固定参数 kBackIconSize = 18：由当前模块定义，用于控制布局、时序或硬件行为。
    constexpr uint8_t kBackIconSize = 18;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect bar = Theme::toolbar_rect();

    // 左：返回图标（命中区 kBackTapW）
    const int16_t back_x = static_cast<int16_t>(Theme::kPadLg);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t back_y = static_cast<int16_t>(bar.y + (bar.h - kBackIconSize) / 2);
    // 访问对象成员：c.glyph(back_x, back_y, ::app::ebook::gfx::icon::kFaReply, kBackIconSize, gfx::FontFace::Icon);；读取或修改这个对象里的成员状态。
    c.glyph(back_x, back_y, ::app::ebook::gfx::icon::kFaReply, kBackIconSize, gfx::FontFace::Icon);

    // 中：标题
    if (title != nullptr && title[0] != '\0')
    {
        // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
        gfx::Canvas::TextStyle ts{};
        // 赋值：把右侧 Theme::kFontTitle 的结果保存到左侧 ts.size_px。
        ts.size_px = Theme::kFontTitle;
        // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
        ts.h = gfx::HAlign::Center;
        // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
        ts.v = gfx::VAlign::Middle;
        // 调用 text_in(bar,title,ts)：文本对齐。
        c.text_in(bar, title, ts);
    }

    // 底部分割线
    c.hline(0, static_cast<int16_t>(bar.y + bar.h - 1), Theme::kScreenW);
}

// 函数定义/声明：调用 hit_toolbar_back()：toolbar()： 在固定工具栏矩形中画返回图标、标题和底部横线。返回图标使用 Font Awesome 的码点，而不是手绘 SVG。。
bool hit_toolbar_back(int16_t x, int16_t y)
{
    // 判断条件：y < Theme::kToolbarY || y >= Theme::kListStartY 是否成立。
    if (y < Theme::kToolbarY || y >= Theme::kListStartY)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 返回 x >= 0 && x < Theme::kBackTapW 给调用者，表示当前函数的执行结果。
    return x >= 0 && x < Theme::kBackTapW;
}

// 函数定义/声明：调用 hit_row_action()：RowStyle 描述一行可能有的元素： 左侧图标。 主标签。 右侧文本或值图标。 最右侧动作图标。 是否显示 chevron。 是否反色。。
bool hit_row_action(const core::Rect& row_rect, int16_t x, int16_t y)
{
    // 判断条件：row_rect.empty() 是否成立。
    if (row_rect.empty())
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect area{static_cast<int16_t>(row_rect.x + row_rect.w - kRowActionWidth),
                          // 继续上一行的表达式或参数列表：row_rect.y, kRowActionWidth, row_rect.h};。
                          row_rect.y, kRowActionWidth, row_rect.h};
    // 返回 area.contains(x, y) 给调用者，表示当前函数的执行结果。
    return area.contains(x, y);
}

// 函数定义/声明：调用 list_row()：RowStyle 描述一行可能有的元素： 左侧图标。 主标签。 右侧文本或值图标。 最右侧动作图标。 是否显示 chevron。 是否反色。。
void list_row(gfx::Canvas& c, const core::Rect& r, const RowStyle& s)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const gfx::Ink ink_fg = s.inverted ? gfx::Ink::White : gfx::Ink::Black;

    // 判断条件：s.inverted 是否成立。
    if (s.inverted)
        // 访问对象成员：c.fill(r, gfx::Ink::Black);；读取或修改这个对象里的成员状态。
        c.fill(r, gfx::Ink::Black);

    // 定义变量 int16_t text_x 并初始化为 static_cast<int16_t>(r.x + Theme::kPadLg)。
    int16_t text_x = static_cast<int16_t>(r.x + Theme::kPadLg);

    // 左侧主图标
    if (s.icon_cp != 0)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t iy = static_cast<int16_t>(r.y + (r.h - Theme::kFontIconSm) / 2);
        // 访问对象成员：c.glyph(text_x, iy, s.icon_cp, Theme::kFontIconSm, gfx::FontFace::Icon, ink_fg);；读取或修改这个对象里的成员状态。
        c.glyph(text_x, iy, s.icon_cp, Theme::kFontIconSm, gfx::FontFace::Icon, ink_fg);
        // 赋值：把右侧 static_cast<int16_t>(text_x + Theme::kFontIconSm + 6) 的结果保存到左侧 text_x。
        text_x = static_cast<int16_t>(text_x + Theme::kFontIconSm + 6);
    }

    // 标签文本
    if (s.label != nullptr)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t ty = static_cast<int16_t>(r.y + (r.h - Theme::kFontBody) / 2);
        // 访问对象成员：c.text(text_x, ty, s.label, Theme::kFontBody, gfx::FontFace::Text, ink_fg);；读取或修改这个对象里的成员状态。
        c.text(text_x, ty, s.label, Theme::kFontBody, gfx::FontFace::Text, ink_fg);
    }

    // 右端布局：[ value/value_icon ] [ chevron 或 action_icon ]
    const bool has_action = (s.action_icon != 0);
    // 定义变量 int16_t right_edge 并初始化为 static_cast<int16_t>(r.x + r.w - Theme::kPadLg)。
    int16_t right_edge = static_cast<int16_t>(r.x + r.w - Theme::kPadLg);

    // 判断条件：has_action 是否成立。
    if (has_action)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t area_x = static_cast<int16_t>(r.x + r.w - kRowActionWidth);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t isz = s.action_icon_size;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t ax = static_cast<int16_t>(area_x + (kRowActionWidth - isz) / 2);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t ay = static_cast<int16_t>(r.y + (r.h - isz) / 2);
        // 访问对象成员：c.glyph(ax, ay, s.action_icon, isz, gfx::FontFace::Icon, ink_fg);；读取或修改这个对象里的成员状态。
        c.glyph(ax, ay, s.action_icon, isz, gfx::FontFace::Icon, ink_fg);
        // 赋值：把右侧 static_cast<int16_t>(area_x - 4) 的结果保存到左侧 right_edge。
        right_edge = static_cast<int16_t>(area_x - 4);
    }
    // 备用分支：前面的条件不成立时执行这里。
    else if (s.show_chevron)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t cx = static_cast<int16_t>(r.x + r.w - Theme::kPadLg - 10);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t cy = static_cast<int16_t>(r.y + (r.h - Theme::kFontIconSm) / 2);
        // 函数调用：c.glyph(cx, cy, ::app::ebook::gfx::icon::kFaChevronRight, Theme::kFontIconSm,；执行该函数并处理它的返回值或副作用。
        c.glyph(cx, cy, ::app::ebook::gfx::icon::kFaChevronRight, Theme::kFontIconSm,
                // 继续上一行的表达式或参数列表：gfx::FontFace::Icon, ink_fg);。
                gfx::FontFace::Icon, ink_fg);
        // 赋值：把右侧 static_cast<int16_t>(cx - 4) 的结果保存到左侧 right_edge。
        right_edge = static_cast<int16_t>(cx - 4);
    }

    // 右侧值：图标优先（开关 / 状态），否则文本
    if (s.value_icon != 0)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t isz = s.value_icon_size;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t vx = static_cast<int16_t>(right_edge - isz);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t vy = static_cast<int16_t>(r.y + (r.h - isz) / 2);
        // 访问对象成员：c.glyph(vx, vy, s.value_icon, isz, gfx::FontFace::Icon, ink_fg);；读取或修改这个对象里的成员状态。
        c.glyph(vx, vy, s.value_icon, isz, gfx::FontFace::Icon, ink_fg);
    }
    // 备用分支：前面的条件不成立时执行这里。
    else if (s.value != nullptr && s.value[0] != '\0')
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint16_t vw = gfx::Font::get_instance().measure(s.value, Theme::kFontBody);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t vx = static_cast<int16_t>(right_edge - vw);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t vy = static_cast<int16_t>(r.y + (r.h - Theme::kFontBody) / 2);
        // 访问对象成员：c.text(vx, vy, s.value, Theme::kFontBody, gfx::FontFace::Text, ink_fg);；读取或修改这个对象里的成员状态。
        c.text(vx, vy, s.value, Theme::kFontBody, gfx::FontFace::Text, ink_fg);
    }

    // 底部分割线（反色行不画）
    if (!s.inverted)
    {
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        c.hline(static_cast<int16_t>(r.x + Theme::kPadLg), static_cast<int16_t>(r.y + r.h - 1),
                // 继续上一行的表达式或参数列表：static_cast<uint16_t>(r.w - Theme::kPadLg * 2));。
                static_cast<uint16_t>(r.w - Theme::kPadLg * 2));
    }
}

// 函数调用：void scrollbar(gfx::Canvas& c, uint16_t track_y, uint16_t track_h, uint16_t offset, uint16_t total,；执行该函数并处理它的返回值或副作用。
void scrollbar(gfx::Canvas& c, uint16_t track_y, uint16_t track_h, uint16_t offset, uint16_t total,
               // 继续上一行的表达式或参数列表：uint16_t visible)。
               uint16_t visible)
{
    // 固定参数 kMinThumb = 14：由当前模块定义，用于控制布局、时序或硬件行为。
    constexpr uint16_t kMinThumb = 14;

    // 判断条件：total <= visible || track_h == 0 是否成立。
    if (total <= visible || track_h == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x = static_cast<int16_t>(Theme::kScreenW - 4);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t bar_w = Theme::kScrollbarW;

    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    c.vline(static_cast<int16_t>(x + 1), static_cast<int16_t>(track_y), track_h);

    // 定义变量 uint16_t thumb 并初始化为 static_cast<uint16_t>((static_cast<uint32_t>(visible) * track_h) / total)。
    uint16_t thumb = static_cast<uint16_t>((static_cast<uint32_t>(visible) * track_h) / total);
    // 判断条件：thumb < kMinThumb 是否成立。
    if (thumb < kMinThumb)
        // 赋值：把右侧 kMinThumb 的结果保存到左侧 thumb。
        thumb = kMinThumb;
    // 判断条件：thumb > track_h 是否成立。
    if (thumb > track_h)
        // 赋值：把右侧 track_h 的结果保存到左侧 thumb。
        thumb = track_h;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t travel = static_cast<uint16_t>(track_h - thumb);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t denom = static_cast<uint16_t>(total - visible);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t thumb_y = static_cast<uint16_t>(
        // 继续上一行的表达式或参数列表：track_y + (static_cast<uint32_t>(offset) * travel) / (denom == 0 ? 1 : denom));。
        track_y + (static_cast<uint32_t>(offset) * travel) / (denom == 0 ? 1 : denom));

    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    c.fill(core::Rect{x, static_cast<int16_t>(thumb_y), bar_w, thumb}, gfx::Ink::Black);
}

// 函数定义/声明：调用 progress_bar()：先画矩形边框，再在内部按百分比填黑。百分比 0 或尺寸过小时不填充。它不做动画，因为墨水屏刷新慢，动画通常不划算。。
void progress_bar(gfx::Canvas& c, const core::Rect& bar, uint8_t pct)
{
    // 调用 rect(bar)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    c.rect(bar);
    // 判断条件：pct == 0 || bar.w <= 4 || bar.h <= 4 是否成立。
    if (pct == 0 || bar.w <= 4 || bar.h <= 4)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t inner_w = static_cast<uint16_t>(bar.w - 4);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t fill_w = static_cast<uint16_t>((static_cast<uint32_t>(inner_w) * pct) / 100U);
    // 判断条件：fill_w > 0 是否成立。
    if (fill_w > 0)
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        c.fill(core::Rect{static_cast<int16_t>(bar.x + 2), static_cast<int16_t>(bar.y + 2), fill_w,
                          // 继续上一行的表达式或参数列表：static_cast<uint16_t>(bar.h - 4)},。
                          static_cast<uint16_t>(bar.h - 4)},
               // 继续上一行的表达式或参数列表：gfx::Ink::Black);。
               gfx::Ink::Black);
}

// 函数调用：uint16_t battery_indicator(gfx::Canvas& c, int16_t right_edge, int16_t y_top, uint8_t pct,；执行该函数并处理它的返回值或副作用。
uint16_t battery_indicator(gfx::Canvas& c, int16_t right_edge, int16_t y_top, uint8_t pct,
                           // 继续上一行的表达式或参数列表：uint16_t bar_h)。
                           uint16_t bar_h)
{
    // 固定参数 icon_w = 16, icon_h = 9, knob_w = 2, gap = 3：由当前模块定义，用于控制布局、时序或硬件行为。
    constexpr uint16_t icon_w = 16, icon_h = 9, knob_w = 2, gap = 3;

    // 执行语句：char buf[8];；它会更新一次变量、对象或程序状态。
    char buf[8];
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    (void)std::snprintf(buf, sizeof(buf), "%u%%", static_cast<unsigned>(pct));
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t tw = gfx::Font::get_instance().measure(buf, Theme::kFontSmall);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t total_w = icon_w + knob_w + gap + tw;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t bx = static_cast<int16_t>(right_edge - total_w);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t by = static_cast<int16_t>(y_top + (bar_h - icon_h) / 2);

    // 外框
    c.rect(core::Rect{bx, by, icon_w, icon_h});
    // 凸起
    c.fill(core::Rect{static_cast<int16_t>(bx + icon_w), static_cast<int16_t>(by + 2), knob_w,
                      // 继续上一行的表达式或参数列表：static_cast<uint16_t>(icon_h - 4)});。
                      static_cast<uint16_t>(icon_h - 4)});
    // 内部填充
    if (pct > 0)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint16_t inner = static_cast<uint16_t>(icon_w - 4);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint16_t fw = static_cast<uint16_t>((static_cast<uint32_t>(inner) * pct) / 100U);
        // 判断条件：fw > 0 是否成立。
        if (fw > 0)
            // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
            c.fill(core::Rect{static_cast<int16_t>(bx + 2), static_cast<int16_t>(by + 2), fw,
                              // 继续上一行的表达式或参数列表：static_cast<uint16_t>(icon_h - 4)});。
                              static_cast<uint16_t>(icon_h - 4)});
    }

    // 百分比文本
    const int16_t tx = static_cast<int16_t>(bx + icon_w + knob_w + gap);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t ty = static_cast<int16_t>(y_top + (bar_h - Theme::kFontSmall) / 2);
    // 访问对象成员：c.text(tx, ty, buf, Theme::kFontSmall);；读取或修改这个对象里的成员状态。
    c.text(tx, ty, buf, Theme::kFontSmall);

    // 返回 total_w 给调用者，表示当前函数的执行结果。
    return total_w;
}

// 函数定义/声明：调用 card_border()：cardborder() 是矩形边框的语义包装。coverplaceholder() 在没有真实封面时画边框和居中文字，避免书籍列表出现大片空白。。
void card_border(gfx::Canvas& c, const core::Rect& r)
{
    // 调用 rect(r)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    c.rect(r);
}

// 函数定义/声明：调用 cover_placeholder()：cardborder() 是矩形边框的语义包装。coverplaceholder() 在没有真实封面时画边框和居中文字，避免书籍列表出现大片空白。。
void cover_placeholder(gfx::Canvas& c, const core::Rect& r, const char* label)
{
    // 调用 rect(r)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    c.rect(r);
    // 判断条件：label != nullptr 是否成立。
    if (label != nullptr)
    {
        // 声明并默认初始化对象：gfx::Canvas::TextStyle s{}。
        gfx::Canvas::TextStyle s{};
        // 赋值：把右侧 Theme::kFontSmall 的结果保存到左侧 s.size_px。
        s.size_px = Theme::kFontSmall;
        // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 s.h。
        s.h = gfx::HAlign::Center;
        // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 s.v。
        s.v = gfx::VAlign::Middle;
        // 赋值：把右侧 2 的结果保存到左侧 s.padding。
        s.padding = 2;
        // 调用 text_in(r,label,s)：文本对齐。
        c.text_in(r, label, s);
    }
}

// 函数调用：core::Rect grid_cell(const core::Rect& area, uint8_t cols, uint8_t rows, uint8_t index,；执行该函数并处理它的返回值或副作用。
core::Rect grid_cell(const core::Rect& area, uint8_t cols, uint8_t rows, uint8_t index,
                     // 继续上一行的表达式或参数列表：uint16_t gap)。
                     uint16_t gap)
{
    // 判断条件：cols == 0 || rows == 0 || index >= cols * rows 是否成立。
    if (cols == 0 || rows == 0 || index >= cols * rows)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t total_gap_w = static_cast<uint16_t>(gap * (cols + 1));
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t total_gap_h = static_cast<uint16_t>(gap * (rows + 1));
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t cw =
        // 继续上一行的表达式或参数列表：(area.w > total_gap_w) ? static_cast<uint16_t>((area.w - total_gap_w) / cols) : 0;。
        (area.w > total_gap_w) ? static_cast<uint16_t>((area.w - total_gap_w) / cols) : 0;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t ch =
        // 继续上一行的表达式或参数列表：(area.h > total_gap_h) ? static_cast<uint16_t>((area.h - total_gap_h) / rows) : 0;。
        (area.h > total_gap_h) ? static_cast<uint16_t>((area.h - total_gap_h) / rows) : 0;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t col = index % cols;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t row = index / cols;

    // 返回 core::Rect{static_cast<int16_t>(area.x + gap + col * (cw + gap)), 给调用者，表示当前函数的执行结果。
    return core::Rect{static_cast<int16_t>(area.x + gap + col * (cw + gap)),
                      // 继续上一行的表达式或参数列表：static_cast<int16_t>(area.y + gap + row * (ch + gap)), cw, ch};。
                      static_cast<int16_t>(area.y + gap + row * (ch + gap)), cw, ch};
}

// 函数调用：int8_t grid_hit_test(const core::Rect& area, uint8_t cols, uint8_t rows, int16_t x, int16_t y,；执行该函数并处理它的返回值或副作用。
int8_t grid_hit_test(const core::Rect& area, uint8_t cols, uint8_t rows, int16_t x, int16_t y,
                     // 继续上一行的表达式或参数列表：uint16_t gap)。
                     uint16_t gap)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t total = static_cast<uint8_t>(cols * rows);
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < total; ++i)
    {
        // 判断条件：grid_cell(area, cols, rows, i, gap).contains(x, y) 是否成立。
        if (grid_cell(area, cols, rows, i, gap).contains(x, y))
            // 返回 static_cast<int8_t>(i) 给调用者，表示当前函数的执行结果。
            return static_cast<int8_t>(i);
    }
    // 返回 -1 给调用者，表示当前函数的执行结果。
    return -1;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::ui::widgets
