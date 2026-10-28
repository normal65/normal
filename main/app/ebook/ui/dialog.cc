/**
 * [初学者导读] 文件：main/app/ebook/ui/dialog.cc
 * 作用：实现通用对话框、按钮布局和关闭按钮绘制。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：UI 状态只能在 UiLoop/UI 线程修改，跨任务结果通过 UiBus 回 UI。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/dialog.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/icon.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::ui::dialog {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using Theme = ui::Theme;

// 定义或声明函数 body_center_style：参数在括号里，返回类型写在函数名前面。
gfx::Canvas::TextStyle body_center_style()
{
    // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
    gfx::Canvas::TextStyle ts{};
    // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontBody;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
    ts.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
    ts.v = gfx::VAlign::Middle;
    // 赋值：把右侧 2 的结果保存到左侧 ts.padding。
    ts.padding = 2;
    // 返回 ts 给调用者，表示当前函数的执行结果。
    return ts;
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 centered：参数在括号里，返回类型写在函数名前面。
core::Rect centered(uint16_t w, uint16_t h)
{
    // 返回 core::Rect{static_cast<int16_t>((Theme::kScreenW - w) / 2), 给调用者，表示当前函数的执行结果。
    return core::Rect{static_cast<int16_t>((Theme::kScreenW - w) / 2),
                      // 继续上一行的表达式或参数列表：static_cast<int16_t>((Theme::kScreenH - h) / 2), w, h};。
                      static_cast<int16_t>((Theme::kScreenH - h) / 2), w, h};
}

// 定义或声明函数 close_draw_rect：参数在括号里，返回类型写在函数名前面。
core::Rect close_draw_rect(const core::Rect& dlg)
{
    // 返回 core::Rect{static_cast<int16_t>(dlg.right() - kCloseBtnSize - kCloseBtnPad), 给调用者，表示当前函数的执行结果。
    return core::Rect{static_cast<int16_t>(dlg.right() - kCloseBtnSize - kCloseBtnPad),
                      // 继续上一行的表达式或参数列表：static_cast<int16_t>(dlg.y + kCloseBtnPad), kCloseBtnSize, kCloseBtnSize};。
                      static_cast<int16_t>(dlg.y + kCloseBtnPad), kCloseBtnSize, kCloseBtnSize};
}

// 定义或声明函数 close_hit_rect：参数在括号里，返回类型写在函数名前面。
core::Rect close_hit_rect(const core::Rect& dlg)
{
    // 固定参数 hit_w = 40：由当前模块定义，用于控制布局、时序或硬件行为。
    constexpr uint16_t hit_w = 40;
    // 固定参数 hit_h = 24：由当前模块定义，用于控制布局、时序或硬件行为。
    constexpr uint16_t hit_h = 24;
    // 返回 core::Rect{static_cast<int16_t>(dlg.right() - static_cast<int16_t>(hit_w)), dlg.y, hit_w, 给调用者，表示当前函数的执行结果。
    return core::Rect{static_cast<int16_t>(dlg.right() - static_cast<int16_t>(hit_w)), dlg.y, hit_w,
                      // 继续上一行的表达式或参数列表：hit_h};。
                      hit_h};
}

// 定义或声明函数 yes_rect：参数在括号里，返回类型写在函数名前面。
core::Rect yes_rect(const core::Rect& dlg, uint16_t btn_h)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t half = static_cast<uint16_t>(dlg.w / 2);
    // 返回 core::Rect{dlg.x, static_cast<int16_t>(dlg.bottom() - btn_h), half, btn_h} 给调用者，表示当前函数的执行结果。
    return core::Rect{dlg.x, static_cast<int16_t>(dlg.bottom() - btn_h), half, btn_h};
}

// 定义或声明函数 no_rect：参数在括号里，返回类型写在函数名前面。
core::Rect no_rect(const core::Rect& dlg, uint16_t btn_h)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t half = static_cast<uint16_t>(dlg.w / 2);
    // 返回 core::Rect{static_cast<int16_t>(dlg.x + half), 给调用者，表示当前函数的执行结果。
    return core::Rect{static_cast<int16_t>(dlg.x + half),
                      // 继续上一行的表达式或参数列表：static_cast<int16_t>(dlg.bottom() - btn_h), half, btn_h};。
                      static_cast<int16_t>(dlg.bottom() - btn_h), half, btn_h};
}

// 定义或声明函数 paint_frame：参数在括号里，返回类型写在函数名前面。
void paint_frame(gfx::Canvas& c, const core::Rect& dlg)
{
    // 访问对象成员：c.fill(dlg, gfx::Ink::White);；读取或修改这个对象里的成员状态。
    c.fill(dlg, gfx::Ink::White);
    // 调用 rect(dlg,2)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    c.rect(dlg, 2);
}

// 定义或声明函数 paint_close_btn：参数在括号里，返回类型写在函数名前面。
void paint_close_btn(gfx::Canvas& c, const core::Rect& dlg)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect btn = close_draw_rect(dlg);
    // 调用 rect(btn,1)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    c.rect(btn, 1);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t ix = static_cast<int16_t>(btn.x + ((btn.w - kCloseIconSize) / 2));
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t iy = static_cast<int16_t>(btn.y + ((btn.h - kCloseIconSize) / 2));
    // 访问对象成员：c.glyph(ix, iy, gfx::icon::kFaTimes, kCloseIconSize, gfx::FontFace::Icon);；读取或修改这个对象里的成员状态。
    c.glyph(ix, iy, gfx::icon::kFaTimes, kCloseIconSize, gfx::FontFace::Icon);
}

// 定义或声明函数 paint_yes_no_row：参数在括号里，返回类型写在函数名前面。
void paint_yes_no_row(gfx::Canvas& c, const core::Rect& dlg, uint16_t btn_h)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect yes = yes_rect(dlg, btn_h);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect no = no_rect(dlg, btn_h);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto ts = body_center_style();
    // 调用 rect(yes,1)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    c.rect(yes, 1);
    // 调用 rect(no,1)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    c.rect(no, 1);
    // 访问对象成员：c.text_in(yes, ui::strings::kDrawYes, ts);；读取或修改这个对象里的成员状态。
    c.text_in(yes, ui::strings::kDrawYes, ts);
    // 访问对象成员：c.text_in(no, ui::strings::kDrawNo, ts);；读取或修改这个对象里的成员状态。
    c.text_in(no, ui::strings::kDrawNo, ts);
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    c.vline(static_cast<int16_t>(yes.right()), yes.y, yes.h);
}

// 函数调用：void paint_confirm(gfx::Canvas& c, const core::Rect& dlg, const char* message, bool with_close,；执行该函数并处理它的返回值或副作用。
void paint_confirm(gfx::Canvas& c, const core::Rect& dlg, const char* message, bool with_close,
                   // 继续上一行的表达式或参数列表：uint16_t btn_h)。
                   uint16_t btn_h)
{
    // 调用 paint_frame(c, dlg);，执行这个函数并处理它的返回值或状态变化。
    paint_frame(c, dlg);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto ts = body_center_style();

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t msg_w =
        // 继续上一行的表达式或参数列表：with_close ? static_cast<uint16_t>(dlg.w - kCloseBtnSize - (kCloseBtnPad * 2)) : dlg.w;。
        with_close ? static_cast<uint16_t>(dlg.w - kCloseBtnSize - (kCloseBtnPad * 2)) : dlg.w;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect msg{dlg.x, static_cast<int16_t>(dlg.y + 4), msg_w,
                         // 继续上一行的表达式或参数列表：static_cast<uint16_t>(dlg.h - btn_h - 6)};。
                         static_cast<uint16_t>(dlg.h - btn_h - 6)};
    // 判断条件：message != nullptr 是否成立。
    if (message != nullptr)
        // 调用 text_in(msg,message,ts)：文本对齐。
        c.text_in(msg, message, ts);

    // 判断条件：with_close 是否成立。
    if (with_close)
        // 调用 paint_close_btn(c, dlg);，执行这个函数并处理它的返回值或状态变化。
        paint_close_btn(c, dlg);
    // 调用 paint_yes_no_row(c, dlg, btn_h);，执行这个函数并处理它的返回值或状态变化。
    paint_yes_no_row(c, dlg, btn_h);
}

// 定义或声明函数 hit_confirm：参数在括号里，返回类型写在函数名前面。
ConfirmHit hit_confirm(const core::Rect& dlg, int16_t x, int16_t y, bool with_close, uint16_t btn_h)
{
    // 判断条件：with_close && close_hit_rect(dlg).contains(x, y) 是否成立。
    if (with_close && close_hit_rect(dlg).contains(x, y))
        // 返回 ConfirmHit::Close 给调用者，表示当前函数的执行结果。
        return ConfirmHit::Close;
    // 判断条件：yes_rect(dlg, btn_h).contains(x, y) 是否成立。
    if (yes_rect(dlg, btn_h).contains(x, y))
        // 返回 ConfirmHit::Yes 给调用者，表示当前函数的执行结果。
        return ConfirmHit::Yes;
    // 判断条件：no_rect(dlg, btn_h).contains(x, y) 是否成立。
    if (no_rect(dlg, btn_h).contains(x, y))
        // 返回 ConfirmHit::No 给调用者，表示当前函数的执行结果。
        return ConfirmHit::No;
    // 返回 ConfirmHit::None 给调用者，表示当前函数的执行结果。
    return ConfirmHit::None;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::ui::dialog
