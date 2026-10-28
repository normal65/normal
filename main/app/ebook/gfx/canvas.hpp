// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/gfx/canvas.hpp
 * 作用：向 framebuffer 绘制文字、线条、矩形和位图。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：绘制只改 framebuffer，不直接操作 SPI。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/geometry.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/font.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/text_layout.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::gfx {

/**
 * @brief 1-bit 帧缓冲绘制（clip + dark 翻转；不持有 fb 生命周期）
 *
 * dark_=true 时 Ink 黑白对调，实现夜间模式无需事后 invert。
 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class Canvas
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态值 dark：只属于当前模块或类，并保持固定含义。
    static Canvas full(uint8_t* fb, bool dark = false)
    {
        // 返回 Canvas{fb, core::Rect::full(), dark} 给调用者，表示当前函数的执行结果。
        return Canvas{fb, core::Rect::full(), dark};
    }

    // 调用 Canvas() = default;，执行这个函数并处理它的返回值或状态变化。
    Canvas() = default;
    // 定义或声明函数 Canvas：参数在括号里，返回类型写在函数名前面。
    Canvas(uint8_t* fb, const core::Rect& clip, bool dark = false)
        // 定义或声明函数 fb_：参数在括号里，返回类型写在函数名前面。
        : fb_(fb)
        // 定义或声明函数 clip_：参数在括号里，返回类型写在函数名前面。
        , clip_(clip.clamped())
        // 定义或声明函数 dark_：参数在括号里，返回类型写在函数名前面。
        , dark_(dark)
    {
    }

    // 定义或声明函数 with_clip：参数在括号里，返回类型写在函数名前面。
    Canvas with_clip(const core::Rect& r) const
    {
        // 返回 Canvas{fb_, core::Rect::intersect(clip_, r), dark_} 给调用者，表示当前函数的执行结果。
        return Canvas{fb_, core::Rect::intersect(clip_, r), dark_};
    }

    // 定义或声明函数 with_dark：参数在括号里，返回类型写在函数名前面。
    Canvas with_dark(bool d) const
    {
        // 返回 Canvas{fb_, clip_, d} 给调用者，表示当前函数的执行结果。
        return Canvas{fb_, clip_, d};
    }

    // 定义或声明函数 empty：参数在括号里，返回类型写在函数名前面。
    bool empty() const
    {
        // 返回 fb_ == nullptr || clip_.empty() 给调用者，表示当前函数的执行结果。
        return fb_ == nullptr || clip_.empty();
    }
    // 定义或声明函数 dark：参数在括号里，返回类型写在函数名前面。
    bool dark() const
    {
        // 返回 dark_ 给调用者，表示当前函数的执行结果。
        return dark_;
    }
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect& clip() const
    {
        // 返回 clip_ 给调用者，表示当前函数的执行结果。
        return clip_;
    }
    // 定义或声明函数 fb：参数在括号里，返回类型写在函数名前面。
    uint8_t* fb() const
    {
        // 返回 fb_ 给调用者，表示当前函数的执行结果。
        return fb_;
    }

    // 赋值：把右侧 Ink::White) 的结果保存到左侧 void clear(Ink ink。
    void clear(Ink ink = Ink::White);
    // 赋值：把右侧 Ink::Black) 的结果保存到左侧 void fill(const core::Rect& r, Ink ink。
    void fill(const core::Rect& r, Ink ink = Ink::Black);
    // 赋值：把右侧 1, Ink ink = Ink::Black) 的结果保存到左侧 void rect(const core::Rect& r, uint8_t thickness。
    void rect(const core::Rect& r, uint8_t thickness = 1, Ink ink = Ink::Black);
    // 赋值：把右侧 Ink::Black) 的结果保存到左侧 void hline(int16_t x, int16_t y, uint16_t w, Ink ink。
    void hline(int16_t x, int16_t y, uint16_t w, Ink ink = Ink::Black);
    // 赋值：把右侧 Ink::Black) 的结果保存到左侧 void vline(int16_t x, int16_t y, uint16_t h, Ink ink。
    void vline(int16_t x, int16_t y, uint16_t h, Ink ink = Ink::Black);
    // 赋值：把右侧 Ink::Black) 的结果保存到左侧 void pixel(int16_t x, int16_t y, Ink ink。
    void pixel(int16_t x, int16_t y, Ink ink = Ink::Black);

    /** 物理 XOR，不受 dark_ 影响 */
    // 函数调用：void invert(const core::Rect& r);；执行该函数并处理它的返回值或副作用。
    void invert(const core::Rect& r);

    // 函数调用：uint16_t glyph(int16_t x, int16_t y_top, uint32_t cp, uint8_t size_px,；执行该函数并处理它的返回值或副作用。
    uint16_t glyph(int16_t x, int16_t y_top, uint32_t cp, uint8_t size_px,
                   // 继续上一行的表达式或参数列表：FontFace face = FontFace::Text, Ink ink = Ink::Black);。
                   FontFace face = FontFace::Text, Ink ink = Ink::Black);

    // 函数调用：uint16_t text(int16_t x, int16_t y_top, const char* utf8, uint8_t size_px,；执行该函数并处理它的返回值或副作用。
    uint16_t text(int16_t x, int16_t y_top, const char* utf8, uint8_t size_px,
                  // 继续上一行的表达式或参数列表：FontFace face = FontFace::Text, Ink ink = Ink::Black);。
                  FontFace face = FontFace::Text, Ink ink = Ink::Black);

    // struct TextStyle：把相关数据字段组合在一起，表示项目中的一个概念。
    struct TextStyle
    {
        // 执行语句：uint8_t size_px{12};；它会更新一次变量、对象或程序状态。
        uint8_t size_px{12};
        // 执行语句：FontFace face{FontFace::Text};；它会更新一次变量、对象或程序状态。
        FontFace face{FontFace::Text};
        // 执行语句：Ink ink{Ink::Black};；它会更新一次变量、对象或程序状态。
        Ink ink{Ink::Black};
        // 执行语句：HAlign h{HAlign::Left};；它会更新一次变量、对象或程序状态。
        HAlign h{HAlign::Left};
        // 执行语句：VAlign v{VAlign::Top};；它会更新一次变量、对象或程序状态。
        VAlign v{VAlign::Top};
        // 执行语句：uint8_t padding{0};；它会更新一次变量、对象或程序状态。
        uint8_t padding{0};
    };

    // 函数调用：void text_in(const core::Rect& box, const char* utf8, const TextStyle& style);；执行该函数并处理它的返回值或副作用。
    void text_in(const core::Rect& box, const char* utf8, const TextStyle& style);

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 函数定义/声明：调用 physical()：逻辑颜色转物理颜色。
    Ink physical(Ink logical) const
    {
        // 判断条件：!dark_ 是否成立。
        if (!dark_)
            // 返回 logical 给调用者，表示当前函数的执行结果。
            return logical;
        // 返回 (logical == Ink::Black) ? Ink::White : Ink::Black 给调用者，表示当前函数的执行结果。
        return (logical == Ink::Black) ? Ink::White : Ink::Black;
    }

    // 定义或声明函数 clip_of：参数在括号里，返回类型写在函数名前面。
    core::Rect clip_of(const core::Rect& r) const
    {
        // 返回 core::Rect::intersect(clip_, r) 给调用者，表示当前函数的执行结果。
        return core::Rect::intersect(clip_, r);
    }

    // 执行语句：uint8_t* fb_{nullptr};；它会更新一次变量、对象或程序状态。
    uint8_t* fb_{nullptr};
    // 声明并默认初始化对象：core::Rect clip_{}。
    core::Rect clip_{};
    // 执行语句：bool dark_{false};；它会更新一次变量、对象或程序状态。
    bool dark_{false};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::gfx
