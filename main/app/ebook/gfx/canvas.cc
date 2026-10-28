/**
 * [初学者导读] 文件：main/app/ebook/gfx/canvas.cc
 * 作用：向 framebuffer 绘制文字、线条、矩形和位图。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：绘制只改 framebuffer，不直接操作 SPI。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/canvas.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "bsp/driver/gdey027t91/framebuffer.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::gfx {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using Fb = ::app::bsp::driver::gdey027t91::Framebuffer;

// 定义或声明函数 is_black：参数在括号里，返回类型写在函数名前面。
inline bool is_black(Ink i)
{
    // 返回 i == Ink::Black 给调用者，表示当前函数的执行结果。
    return i == Ink::Black;
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 Canvas::clear：参数在括号里，返回类型写在函数名前面。
void Canvas::clear(Ink ink)
{
    // 判断条件：empty() 是否成立。
    if (empty())
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const Ink phys = physical(ink);
    // 调用 Fb::fill_rect(fb_, static_cast<uint16_t>(clip_.x), static_cast<uint16_t>(clip_.y), clip_.w,，执行这个函数并处理它的返回值或状态变化。
    Fb::fill_rect(fb_, static_cast<uint16_t>(clip_.x), static_cast<uint16_t>(clip_.y), clip_.w,
                  // 继续上一行的表达式或参数列表：clip_.h, is_black(phys));。
                  clip_.h, is_black(phys));
}

// 函数定义/声明：调用 Canvas::fill()：fill()： 只把 framebuffer 填成一个字节颜色，不改屏幕。。
void Canvas::fill(const core::Rect& r, Ink ink)
{
    // 判断条件：empty() 是否成立。
    if (empty())
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect cr = clip_of(r);
    // 判断条件：cr.empty() 是否成立。
    if (cr.empty())
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const Ink phys = physical(ink);
    // 调用 Fb::fill_rect(fb_, static_cast<uint16_t>(cr.x), static_cast<uint16_t>(cr.y), cr.w, cr.h,，执行这个函数并处理它的返回值或状态变化。
    Fb::fill_rect(fb_, static_cast<uint16_t>(cr.x), static_cast<uint16_t>(cr.y), cr.w, cr.h,
                  // 继续上一行的表达式或参数列表：is_black(phys));。
                  is_black(phys));
}

// 定义或声明函数 Canvas::invert：参数在括号里，返回类型写在函数名前面。
void Canvas::invert(const core::Rect& r)
{
    // 判断条件：empty() 是否成立。
    if (empty())
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect cr = clip_of(r);
    // 判断条件：cr.empty() 是否成立。
    if (cr.empty())
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 Fb::invert_rect(fb_, static_cast<uint16_t>(cr.x), static_cast<uint16_t>(cr.y), cr.w, cr.h);，执行这个函数并处理它的返回值或状态变化。
    Fb::invert_rect(fb_, static_cast<uint16_t>(cr.x), static_cast<uint16_t>(cr.y), cr.w, cr.h);
}

// 函数定义/声明：调用 Canvas::rect()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
void Canvas::rect(const core::Rect& r, uint8_t thickness, Ink ink)
{
    // 判断条件：empty() || thickness == 0 || r.w == 0 || r.h == 0 是否成立。
    if (empty() || thickness == 0 || r.w == 0 || r.h == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t t = thickness;

    // 调用 fill(core::Rect{r.x, r.y, r.w, t}, ink);，执行这个函数并处理它的返回值或状态变化。
    fill(core::Rect{r.x, r.y, r.w, t}, ink);
    // 判断条件：r.h > t 是否成立。
    if (r.h > t)
        // 调用 fill(core::Rect{r.x, static_cast<int16_t>(r.y + r.h - t), r.w, t}, ink);，执行这个函数并处理它的返回值或状态变化。
        fill(core::Rect{r.x, static_cast<int16_t>(r.y + r.h - t), r.w, t}, ink);

    // 判断条件：r.h > 2 * t 是否成立。
    if (r.h > 2 * t)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint16_t mid_h = static_cast<uint16_t>(r.h - 2 * t);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t mid_y = static_cast<int16_t>(r.y + t);
        // 调用 fill(core::Rect{r.x, mid_y, t, mid_h}, ink);，执行这个函数并处理它的返回值或状态变化。
        fill(core::Rect{r.x, mid_y, t, mid_h}, ink);
        // 判断条件：r.w > t 是否成立。
        if (r.w > t)
            // 调用 fill(core::Rect{static_cast<int16_t>(r.x + r.w - t), mid_y, t, mid_h}, ink);，执行这个函数并处理它的返回值或状态变化。
            fill(core::Rect{static_cast<int16_t>(r.x + r.w - t), mid_y, t, mid_h}, ink);
    }
}

// 定义或声明函数 Canvas::hline：参数在括号里，返回类型写在函数名前面。
void Canvas::hline(int16_t x, int16_t y, uint16_t w, Ink ink)
{
    // 调用 fill(core::Rect{x, y, w, 1}, ink);，执行这个函数并处理它的返回值或状态变化。
    fill(core::Rect{x, y, w, 1}, ink);
}

// 定义或声明函数 Canvas::vline：参数在括号里，返回类型写在函数名前面。
void Canvas::vline(int16_t x, int16_t y, uint16_t h, Ink ink)
{
    // 调用 fill(core::Rect{x, y, 1, h}, ink);，执行这个函数并处理它的返回值或状态变化。
    fill(core::Rect{x, y, 1, h}, ink);
}

// 定义或声明函数 Canvas::pixel：参数在括号里，返回类型写在函数名前面。
void Canvas::pixel(int16_t x, int16_t y, Ink ink)
{
    // 判断条件：empty() || !clip_.contains(x, y) 是否成立。
    if (empty() || !clip_.contains(x, y))
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 physical(ink)：逻辑颜色转物理颜色。
    Fb::set_pixel(fb_, x, y, is_black(physical(ink)));
}

// 函数调用：uint16_t Canvas::glyph(int16_t x, int16_t y_top, uint32_t cp, uint8_t size_px, FontFace face,；执行该函数并处理它的返回值或副作用。
uint16_t Canvas::glyph(int16_t x, int16_t y_top, uint32_t cp, uint8_t size_px, FontFace face,
                       // 继续上一行的表达式或参数列表：Ink ink)。
                       Ink ink)
{
    // 判断条件：empty() 是否成立。
    if (empty())
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    // 返回结果：调用 physical(ink)：逻辑颜色转物理颜色。
    return Font::get_instance().draw_glyph(fb_, x, y_top, cp, size_px, face, physical(ink), clip_);
}

// 函数调用：uint16_t Canvas::text(int16_t x, int16_t y_top, const char* utf8, uint8_t size_px, FontFace face,；执行该函数并处理它的返回值或副作用。
uint16_t Canvas::text(int16_t x, int16_t y_top, const char* utf8, uint8_t size_px, FontFace face,
                      // 继续上一行的表达式或参数列表：Ink ink)。
                      Ink ink)
{
    // 判断条件：empty() || utf8 == nullptr 是否成立。
    if (empty() || utf8 == nullptr)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    // 返回结果：调用 physical(ink)：逻辑颜色转物理颜色。
    return Font::get_instance().draw_text(fb_, x, y_top, utf8, size_px, face, physical(ink), clip_);
}

// 函数定义/声明：调用 Canvas::text_in()：文本对齐。
void Canvas::text_in(const core::Rect& box, const char* utf8, const TextStyle& style)
{
    // 判断条件：empty() || utf8 == nullptr || box.empty() 是否成立。
    if (empty() || utf8 == nullptr || box.empty())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 Font::get_instance() 的结果保存到左侧 auto& font。
    auto& font = Font::get_instance();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t pad = style.padding;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect inner = box.inflated(static_cast<int16_t>(-pad), static_cast<int16_t>(-pad));
    // 判断条件：inner.empty() 是否成立。
    if (inner.empty())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t tw = font.measure(utf8, style.size_px, style.face);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t lh = font.line_height(style.size_px, style.face);

    // 定义变量 int16_t x 并初始化为 inner.x。
    int16_t x = inner.x;
    // 多分支判断：根据括号里的值选择不同 case。
    switch (style.h)
    {
        // 分支匹配：case HAlign::Center 成立时执行下面的代码。
        case HAlign::Center:
            // 赋值：把右侧 static_cast<int16_t>(inner.x + (inner.w > tw ? (inner.w - tw) / 2 : 0)) 的结果保存到左侧 x。
            x = static_cast<int16_t>(inner.x + (inner.w > tw ? (inner.w - tw) / 2 : 0));
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case HAlign::Right 成立时执行下面的代码。
        case HAlign::Right:
            // 赋值：把右侧 static_cast<int16_t>(inner.x + (inner.w > tw ? (inner.w - tw) : 0)) 的结果保存到左侧 x。
            x = static_cast<int16_t>(inner.x + (inner.w > tw ? (inner.w - tw) : 0));
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case HAlign::Left 成立时执行下面的代码。
        case HAlign::Left:
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }

    // 定义变量 int16_t y_top 并初始化为 inner.y。
    int16_t y_top = inner.y;
    // 多分支判断：根据括号里的值选择不同 case。
    switch (style.v)
    {
        // 分支匹配：case VAlign::Middle 成立时执行下面的代码。
        case VAlign::Middle:
            // 赋值：把右侧 static_cast<int16_t>(inner.y + (inner.h > lh ? (inner.h - lh) / 2 : 0)) 的结果保存到左侧 y_top。
            y_top = static_cast<int16_t>(inner.y + (inner.h > lh ? (inner.h - lh) / 2 : 0));
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case VAlign::Bottom 成立时执行下面的代码。
        case VAlign::Bottom:
            // 赋值：把右侧 static_cast<int16_t>(inner.y + (inner.h > lh ? (inner.h - lh) : 0)) 的结果保存到左侧 y_top。
            y_top = static_cast<int16_t>(inner.y + (inner.h > lh ? (inner.h - lh) : 0));
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case VAlign::Top 成立时执行下面的代码。
        case VAlign::Top:
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect sub_clip = core::Rect::intersect(clip_, box);
    // 调用 physical(style.ink)：逻辑颜色转物理颜色。
    font.draw_text(fb_, x, y_top, utf8, style.size_px, style.face, physical(style.ink), sub_clip);
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::gfx
