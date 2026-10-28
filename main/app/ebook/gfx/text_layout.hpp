// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/gfx/text_layout.hpp
 * 作用：实现文本换行、截断和排版计算。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：绘制只改 framebuffer，不直接操作 SPI。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstddef>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/font.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::gfx {

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class HAlign : uint8_t
{
    // 执行这条代码：Left,。它会更新变量、对象状态或调用链。
    Left,
    // 继续上一行的表达式或参数列表：Center,。
    Center,
    // 继续上一行的表达式或参数列表：Right。
    Right
};
// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class VAlign : uint8_t
{
    // 执行这条代码：Top,。它会更新变量、对象状态或调用链。
    Top,
    // 继续上一行的表达式或参数列表：Middle,。
    Middle,
    // 继续上一行的表达式或参数列表：Bottom。
    Bottom
};

// struct LineSlice：把相关数据字段组合在一起，表示项目中的一个概念。
struct LineSlice
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* begin{nullptr};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* end{nullptr};
    // 执行语句：uint16_t width_px{0};；它会更新一次变量、对象或程序状态。
    uint16_t width_px{0};
};

/** CJK 字符级换行；末行溢出时追加 U+2026 */
// 函数调用：uint8_t wrap_text(const char* utf8, uint8_t size_px, uint16_t max_width, uint8_t max_lines,；执行该函数并处理它的返回值或副作用。
uint8_t wrap_text(const char* utf8, uint8_t size_px, uint16_t max_width, uint8_t max_lines,
                  // 继续上一行的表达式或参数列表：LineSlice* out, FontFace face = FontFace::Text);。
                  LineSlice* out, FontFace face = FontFace::Text);

/** 单行截断，必要时追加 U+2026 */
// 函数调用：size_t truncate_text(const char* utf8, uint8_t size_px, uint16_t max_width, char* out,；执行该函数并处理它的返回值或副作用。
size_t truncate_text(const char* utf8, uint8_t size_px, uint16_t max_width, char* out,
                     // 继续上一行的表达式或参数列表：size_t out_size, FontFace face = FontFace::Text);。
                     size_t out_size, FontFace face = FontFace::Text);

// 赋值：把右侧 FontFace::Text) 的结果保存到左侧 uint16_t ellipsis_width(uint8_t size_px, FontFace face。
uint16_t ellipsis_width(uint8_t size_px, FontFace face = FontFace::Text);

// 函数调用：void format_file_size(uint32_t bytes, char* out, size_t out_size);；执行该函数并处理它的返回值或副作用。
void format_file_size(uint32_t bytes, char* out, size_t out_size);

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::gfx
