/**
 * [初学者导读] 文件：main/app/ebook/apps/reader/reader_menu.cc
 * 作用：绘制和处理阅读器菜单。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/reader_menu.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/text_layout.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::reader {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;

// 定义或声明函数 draw_cell：参数在括号里，返回类型写在函数名前面。
void draw_cell(gfx::Canvas& c, const core::Rect& r, const char* label)
{
    // 判断条件：r.empty() 是否成立。
    if (r.empty())
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 访问对象成员：c.fill(r, gfx::Ink::White);；读取或修改这个对象里的成员状态。
    c.fill(r, gfx::Ink::White);
    // 判断条件：r.w >= 2 && r.h >= 2 是否成立。
    if (r.w >= 2 && r.h >= 2)
    {
        // 访问对象成员：c.hline(r.x, r.y, r.w);；读取或修改这个对象里的成员状态。
        c.hline(r.x, r.y, r.w);
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        c.hline(r.x, static_cast<int16_t>(r.y + r.h - 1), r.w);
        // 访问对象成员：c.vline(r.x, r.y, r.h);；读取或修改这个对象里的成员状态。
        c.vline(r.x, r.y, r.h);
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        c.vline(static_cast<int16_t>(r.x + r.w - 1), r.y, r.h);
    }
    // 判断条件：label == nullptr || label[0] == '\0' 是否成立。
    if (label == nullptr || label[0] == '\0')
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 执行语句：char show[32];；它会更新一次变量、对象或程序状态。
    char show[32];
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t max_w = (r.w > 4) ? static_cast<uint16_t>(r.w - 4) : r.w;
    // 函数调用：(void)gfx::truncate_text(label, Theme::kFontSmall, max_w, show, sizeof(show));；执行该函数并处理它的返回值或副作用。
    (void)gfx::truncate_text(label, Theme::kFontSmall, max_w, show, sizeof(show));

    // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
    gfx::Canvas::TextStyle ts{};
    // 赋值：把右侧 Theme::kFontSmall 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontSmall;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
    ts.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
    ts.v = gfx::VAlign::Middle;
    // 调用 text_in(r,show,ts)：文本对齐。
    c.text_in(r, show, ts);
}

// 定义或声明函数 row_rect：参数在括号里，返回类型写在函数名前面。
core::Rect row_rect(const ReaderLayout& layout, uint8_t row, uint8_t col, uint8_t col_count)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t base_y = layout.menu_panel_y();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y =
        // 继续上一行的表达式或参数列表：static_cast<int16_t>(base_y + row * (ReaderLayout::kMenuRowH + ReaderLayout::kMenuRowGap));。
        static_cast<int16_t>(base_y + row * (ReaderLayout::kMenuRowH + ReaderLayout::kMenuRowGap));
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t cell_w = static_cast<uint16_t>(Theme::kScreenW / col_count);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x = static_cast<int16_t>(col * cell_w);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t w =
        // 继续上一行的表达式或参数列表：(col + 1 == col_count) ? static_cast<uint16_t>(Theme::kScreenW - col * cell_w) : cell_w;。
        (col + 1 == col_count) ? static_cast<uint16_t>(Theme::kScreenW - col * cell_w) : cell_w;
    // 返回 core::Rect{x, y, w, ReaderLayout::kMenuRowH} 给调用者，表示当前函数的执行结果。
    return core::Rect{x, y, w, ReaderLayout::kMenuRowH};
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 paint_menu_panel：参数在括号里，返回类型写在函数名前面。
void paint_menu_panel(gfx::Canvas& c, const ReaderLayout& layout, const MenuPaintInfo& info)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect panel = layout.menu_panel_rect();
    // 访问对象成员：c.fill(panel, gfx::Ink::White);；读取或修改这个对象里的成员状态。
    c.fill(panel, gfx::Ink::White);
    // 访问对象成员：c.hline(panel.x, panel.y, panel.w);；读取或修改这个对象里的成员状态。
    c.hline(panel.x, panel.y, panel.w);

    // 调用 draw_cell(c, row_rect(layout, 0, 0, 3), "<");，执行这个函数并处理它的返回值或状态变化。
    draw_cell(c, row_rect(layout, 0, 0, 3), "<");
    // 调用 draw_cell(c, row_rect(layout, 0, 1, 3), info.font_label);，执行这个函数并处理它的返回值或状态变化。
    draw_cell(c, row_rect(layout, 0, 1, 3), info.font_label);
    // 调用 draw_cell(c, row_rect(layout, 0, 2, 3), ">");，执行这个函数并处理它的返回值或状态变化。
    draw_cell(c, row_rect(layout, 0, 2, 3), ">");

    // 调用 draw_cell(c, row_rect(layout, 1, 0, 3), "<");，执行这个函数并处理它的返回值或状态变化。
    draw_cell(c, row_rect(layout, 1, 0, 3), "<");
    // 调用 draw_cell(c, row_rect(layout, 1, 1, 3), info.refresh_label);，执行这个函数并处理它的返回值或状态变化。
    draw_cell(c, row_rect(layout, 1, 1, 3), info.refresh_label);
    // 调用 draw_cell(c, row_rect(layout, 1, 2, 3), ">");，执行这个函数并处理它的返回值或状态变化。
    draw_cell(c, row_rect(layout, 1, 2, 3), ">");

    // 调用 draw_cell(c, row_rect(layout, 2, 0, 5), "<<");，执行这个函数并处理它的返回值或状态变化。
    draw_cell(c, row_rect(layout, 2, 0, 5), "<<");
    // 调用 draw_cell(c, row_rect(layout, 2, 1, 5), "<");，执行这个函数并处理它的返回值或状态变化。
    draw_cell(c, row_rect(layout, 2, 1, 5), "<");

    // 执行语句：char pct_str[8];；它会更新一次变量、对象或程序状态。
    char pct_str[8];
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    (void)std::snprintf(pct_str, sizeof(pct_str), "%u%%", static_cast<unsigned>(info.read_percent));
    // 调用 draw_cell(c, row_rect(layout, 2, 2, 5), pct_str);，执行这个函数并处理它的返回值或状态变化。
    draw_cell(c, row_rect(layout, 2, 2, 5), pct_str);

    // 调用 draw_cell(c, row_rect(layout, 2, 3, 5), ">");，执行这个函数并处理它的返回值或状态变化。
    draw_cell(c, row_rect(layout, 2, 3, 5), ">");
    // 调用 draw_cell(c, row_rect(layout, 2, 4, 5), ">>");，执行这个函数并处理它的返回值或状态变化。
    draw_cell(c, row_rect(layout, 2, 4, 5), ">>");

    // 调用 draw_cell(c, row_rect(layout, 3, 0, 3), "\u4E66\u5E93");，执行这个函数并处理它的返回值或状态变化。
    draw_cell(c, row_rect(layout, 3, 0, 3), "\u4E66\u5E93");
    // 调用 draw_cell(c, row_rect(layout, 3, 1, 3), "\u8DF3\u8F6C");，执行这个函数并处理它的返回值或状态变化。
    draw_cell(c, row_rect(layout, 3, 1, 3), "\u8DF3\u8F6C");
    // 调用 draw_cell(c, row_rect(layout, 3, 2, 3), "\u9501\u5C4F");，执行这个函数并处理它的返回值或状态变化。
    draw_cell(c, row_rect(layout, 3, 2, 3), "\u9501\u5C4F");
}

// 定义或声明函数 hit_menu：参数在括号里，返回类型写在函数名前面。
MenuHit hit_menu(int16_t x, int16_t y, const ReaderLayout& layout)
{
    // 判断条件：y < layout.menu_panel_y() 是否成立。
    if (y < layout.menu_panel_y())
        // 返回 MenuHit::None 给调用者，表示当前函数的执行结果。
        return MenuHit::None;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto hit = [&](uint8_t row, uint8_t col, uint8_t cols, MenuHit h) {
        // 返回 row_rect(layout, row, col, cols).contains(x, y) ? h : MenuHit::None 给调用者，表示当前函数的执行结果。
        return row_rect(layout, row, col, cols).contains(x, y) ? h : MenuHit::None;
    };

    // 判断条件：auto h = hit(0, 0, 3, MenuHit::FontPrev); h != MenuHit::None 是否成立。
    if (auto h = hit(0, 0, 3, MenuHit::FontPrev); h != MenuHit::None)
        // 返回 h 给调用者，表示当前函数的执行结果。
        return h;
    // 判断条件：auto h = hit(0, 2, 3, MenuHit::FontNext); h != MenuHit::None 是否成立。
    if (auto h = hit(0, 2, 3, MenuHit::FontNext); h != MenuHit::None)
        // 返回 h 给调用者，表示当前函数的执行结果。
        return h;
    // 判断条件：auto h = hit(1, 0, 3, MenuHit::RefreshPrev); h != MenuHit::None 是否成立。
    if (auto h = hit(1, 0, 3, MenuHit::RefreshPrev); h != MenuHit::None)
        // 返回 h 给调用者，表示当前函数的执行结果。
        return h;
    // 判断条件：auto h = hit(1, 2, 3, MenuHit::RefreshNext); h != MenuHit::None 是否成立。
    if (auto h = hit(1, 2, 3, MenuHit::RefreshNext); h != MenuHit::None)
        // 返回 h 给调用者，表示当前函数的执行结果。
        return h;
    // 判断条件：auto h = hit(2, 0, 5, MenuHit::JumpPrev10); h != MenuHit::None 是否成立。
    if (auto h = hit(2, 0, 5, MenuHit::JumpPrev10); h != MenuHit::None)
        // 返回 h 给调用者，表示当前函数的执行结果。
        return h;
    // 判断条件：auto h = hit(2, 1, 5, MenuHit::PagePrev); h != MenuHit::None 是否成立。
    if (auto h = hit(2, 1, 5, MenuHit::PagePrev); h != MenuHit::None)
        // 返回 h 给调用者，表示当前函数的执行结果。
        return h;
    // 判断条件：auto h = hit(2, 3, 5, MenuHit::PageNext); h != MenuHit::None 是否成立。
    if (auto h = hit(2, 3, 5, MenuHit::PageNext); h != MenuHit::None)
        // 返回 h 给调用者，表示当前函数的执行结果。
        return h;
    // 判断条件：auto h = hit(2, 4, 5, MenuHit::JumpNext10); h != MenuHit::None 是否成立。
    if (auto h = hit(2, 4, 5, MenuHit::JumpNext10); h != MenuHit::None)
        // 返回 h 给调用者，表示当前函数的执行结果。
        return h;
    // 判断条件：auto h = hit(3, 0, 3, MenuHit::Shelf); h != MenuHit::None 是否成立。
    if (auto h = hit(3, 0, 3, MenuHit::Shelf); h != MenuHit::None)
        // 返回 h 给调用者，表示当前函数的执行结果。
        return h;
    // 判断条件：auto h = hit(3, 1, 3, MenuHit::JumpPage); h != MenuHit::None 是否成立。
    if (auto h = hit(3, 1, 3, MenuHit::JumpPage); h != MenuHit::None)
        // 返回 h 给调用者，表示当前函数的执行结果。
        return h;
    // 判断条件：auto h = hit(3, 2, 3, MenuHit::Lock); h != MenuHit::None 是否成立。
    if (auto h = hit(3, 2, 3, MenuHit::Lock); h != MenuHit::None)
        // 返回 h 给调用者，表示当前函数的执行结果。
        return h;
    // 返回 MenuHit::None 给调用者，表示当前函数的执行结果。
    return MenuHit::None;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::reader
