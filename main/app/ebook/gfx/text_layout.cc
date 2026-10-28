/**
 * [初学者导读] 文件：main/app/ebook/gfx/text_layout.cc
 * 作用：实现文本换行、截断和排版计算。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：绘制只改 framebuffer，不直接操作 SPI。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/text_layout.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/utf8.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::gfx {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 固定参数 kEllipsisCp = 0x2026：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t kEllipsisCp = 0x2026;
// 静态查表数组 kEllipsisUtf8：运行时复用固定数据，避免每次重新计算。
constexpr char kEllipsisUtf8[3] = {static_cast<char>(0xE2), static_cast<char>(0x80),
                                   // 继续上一行的表达式或参数列表：static_cast<char>(0xA6)};。
                                   static_cast<char>(0xA6)};

// 定义或声明函数 safe_advance：参数在括号里，返回类型写在函数名前面。
uint16_t safe_advance(uint32_t cp, uint8_t size_px, FontFace face)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t a = Font::get_instance().advance(cp, size_px, face);
    // 返回 (a == 0) ? size_px : a 给调用者，表示当前函数的执行结果。
    return (a == 0) ? size_px : a;
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 ellipsis_width：参数在括号里，返回类型写在函数名前面。
uint16_t ellipsis_width(uint8_t size_px, FontFace face)
{
    // 返回结果：调用 advance(kEllipsisCp,size_px,face)：advance(cp)： 查一个字符的前进宽度。点阵优先，FreeType 兜底。它不会绘制任何像素，适合排版测量。。
    return Font::get_instance().advance(kEllipsisCp, size_px, face);
}

// 函数调用：uint8_t wrap_text(const char* utf8, uint8_t size_px, uint16_t max_width, uint8_t max_lines,；执行该函数并处理它的返回值或副作用。
uint8_t wrap_text(const char* utf8, uint8_t size_px, uint16_t max_width, uint8_t max_lines,
                  // 继续上一行的表达式或参数列表：LineSlice* out, FontFace face)。
                  LineSlice* out, FontFace face)
{
    // 判断条件：utf8 == nullptr || out == nullptr || max_lines == 0 || max_width == 0 是否成立。
    if (utf8 == nullptr || out == nullptr || max_lines == 0 || max_width == 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t ell_w = ellipsis_width(size_px, face);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* cursor = utf8;
    // 定义变量 uint8_t emitted 并初始化为 0。
    uint8_t emitted = 0;

    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (*cursor != '\0' && emitted < max_lines)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const bool last_line = (emitted + 1 == max_lines);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint16_t budget =
            // 定义或声明函数 static_cast<uint16_t>：参数在括号里，返回类型写在函数名前面。
            last_line ? static_cast<uint16_t>((max_width > ell_w) ? (max_width - ell_w) : 0)
                      // 执行语句：: max_width;；它会更新一次变量、对象或程序状态。
                      : max_width;

        // 声明并默认初始化对象：LineSlice slice{}。
        LineSlice slice{};
        // 赋值：把右侧 cursor 的结果保存到左侧 slice.begin。
        slice.begin = cursor;

        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* line_end = cursor; // 当前已确认能放下的尾指针
        // 定义变量 uint32_t line_w 并初始化为 0。
        uint32_t line_w = 0;

        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* probe = cursor;
        // 循环执行：条件为真时重复运行，条件变化后退出。
        while (*probe != '\0')
        {
            // 定义变量 uint32_t cp 并初始化为 0。
            uint32_t cp = 0;
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint8_t n = utf8_decode(probe, cp);
            // 判断条件：n == 0 是否成立。
            if (n == 0)
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;

            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint16_t adv = safe_advance(cp, size_px, face);

            // 判断条件：line_w + adv > budget 是否成立。
            if (line_w + adv > budget)
            {
                // 判断条件：line_end == slice.begin 是否成立。
                if (line_end == slice.begin)
                {
                    // 赋值：把右侧 probe + n 的结果保存到左侧 line_end。
                    line_end = probe + n;
                    // 赋值：把右侧 adv 的结果保存到左侧 line_w。
                    line_w = adv;
                }
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            }

            // 赋值：把右侧 adv 的结果保存到左侧 line_w +。
            line_w += adv;
            // 赋值：把右侧 n 的结果保存到左侧 probe +。
            probe += n;
            // 赋值：把右侧 probe 的结果保存到左侧 line_end。
            line_end = probe;
        }

        // 判断条件：line_end == slice.begin 是否成立。
        if (line_end == slice.begin)
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 赋值：把右侧 line_end 的结果保存到左侧 slice.end。
        slice.end = line_end;
        // 赋值：把右侧 static_cast<uint16_t>(line_w) 的结果保存到左侧 slice.width_px。
        slice.width_px = static_cast<uint16_t>(line_w);
        // 判断条件：last_line && *line_end != '\0' 是否成立。
        if (last_line && *line_end != '\0')
            // 赋值：把右侧 static_cast<uint16_t>(slice.width_px + ell_w) 的结果保存到左侧 slice.width_px。
            slice.width_px = static_cast<uint16_t>(slice.width_px + ell_w);

        // 赋值：把右侧 slice 的结果保存到左侧 out[emitted++]。
        out[emitted++] = slice;
        // 判断条件：last_line 是否成立。
        if (last_line)
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 赋值：把右侧 line_end 的结果保存到左侧 cursor。
        cursor = line_end;
    }

    // 返回 emitted 给调用者，表示当前函数的执行结果。
    return emitted;
}

// 函数调用：size_t truncate_text(const char* utf8, uint8_t size_px, uint16_t max_width, char* out,；执行该函数并处理它的返回值或副作用。
size_t truncate_text(const char* utf8, uint8_t size_px, uint16_t max_width, char* out,
                     // 继续上一行的表达式或参数列表：size_t out_size, FontFace face)。
                     size_t out_size, FontFace face)
{
    // 判断条件：out == nullptr || out_size == 0 是否成立。
    if (out == nullptr || out_size == 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    // 赋值：把右侧 '\0' 的结果保存到左侧 out[0]。
    out[0] = '\0';
    // 判断条件：utf8 == nullptr || max_width == 0 是否成立。
    if (utf8 == nullptr || max_width == 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t total = Font::get_instance().measure(utf8, size_px, face);
    // 判断条件：total <= max_width 是否成立。
    if (total <= max_width)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const size_t need = std::strlen(utf8);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const size_t copy = (need < out_size - 1) ? need : (out_size - 1);
        // 调用 std::memcpy(out, utf8, copy);，执行这个函数并处理它的返回值或状态变化。
        std::memcpy(out, utf8, copy);
        // 赋值：把右侧 '\0' 的结果保存到左侧 out[copy]。
        out[copy] = '\0';
        // 返回 copy 给调用者，表示当前函数的执行结果。
        return copy;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t ell_w = ellipsis_width(size_px, face);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t budget = static_cast<uint16_t>((max_width > ell_w) ? (max_width - ell_w) : 0);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* p = utf8;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* cut = utf8;
    // 定义变量 uint32_t acc 并初始化为 0。
    uint32_t acc = 0;
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (*p != '\0')
    {
        // 定义变量 uint32_t cp 并初始化为 0。
        uint32_t cp = 0;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t n = utf8_decode(p, cp);
        // 判断条件：n == 0 是否成立。
        if (n == 0)
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint16_t adv = safe_advance(cp, size_px, face);
        // 判断条件：acc + adv > budget 是否成立。
        if (acc + adv > budget)
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 赋值：把右侧 adv 的结果保存到左侧 acc +。
        acc += adv;
        // 赋值：把右侧 n 的结果保存到左侧 p +。
        p += n;
        // 赋值：把右侧 p 的结果保存到左侧 cut。
        cut = p;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t prefix = static_cast<size_t>(cut - utf8);
    // 定义变量 size_t pos 并初始化为 (prefix < out_size - 1) ? prefix : (out_size - 1)。
    size_t pos = (prefix < out_size - 1) ? prefix : (out_size - 1);
    // 调用 std::memcpy(out, utf8, pos);，执行这个函数并处理它的返回值或状态变化。
    std::memcpy(out, utf8, pos);

    // 判断条件：pos + sizeof(kEllipsisUtf8) < out_size 是否成立。
    if (pos + sizeof(kEllipsisUtf8) < out_size)
    {
        // 调用 std::memcpy(out + pos, kEllipsisUtf8, sizeof(kEllipsisUtf8));，执行这个函数并处理它的返回值或状态变化。
        std::memcpy(out + pos, kEllipsisUtf8, sizeof(kEllipsisUtf8));
        // 赋值：把右侧 sizeof(kEllipsisUtf8) 的结果保存到左侧 pos +。
        pos += sizeof(kEllipsisUtf8);
    }
    // 赋值：把右侧 '\0' 的结果保存到左侧 out[pos]。
    out[pos] = '\0';
    // 返回 pos 给调用者，表示当前函数的执行结果。
    return pos;
}

// 定义或声明函数 format_file_size：参数在括号里，返回类型写在函数名前面。
void format_file_size(uint32_t bytes, char* out, size_t out_size)
{
    // 判断条件：out == nullptr || out_size == 0 是否成立。
    if (out == nullptr || out_size == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：bytes < 1024U 是否成立。
    if (bytes < 1024U)
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        (void)std::snprintf(out, out_size, "%lu B", static_cast<unsigned long>(bytes));
    // 备用分支：前面的条件不成立时执行这里。
    else if (bytes < 1024U * 1024U)
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        (void)std::snprintf(out, out_size, "%lu KB", static_cast<unsigned long>(bytes / 1024U));
    // 备用分支：前面的条件不成立时执行这里。
    else
        // 函数调用：(void)std::snprintf(out, out_size, "%lu MB",；执行该函数并处理它的返回值或副作用。
        (void)std::snprintf(out, out_size, "%lu MB",
                            // 继续上一行的表达式或参数列表：static_cast<unsigned long>(bytes / (1024U * 1024U)));。
                            static_cast<unsigned long>(bytes / (1024U * 1024U)));
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::gfx
