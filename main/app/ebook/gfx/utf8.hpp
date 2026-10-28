// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/gfx/utf8.hpp
 * 作用：实现 UTF-8 编码判断、解码和遍历。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：绘制只改 framebuffer，不直接操作 SPI。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::gfx {

/** UTF-8 解码；非法字节替换为 '?' 并前进 1 字节，保证循环可终止 */

// 编译期函数 utf8_seq_len：常用于固定规则计算，能参与编译期优化。
constexpr uint8_t utf8_seq_len(uint8_t c0)
{
    // 判断条件：c0 < 0x80 是否成立。
    if (c0 < 0x80)
        // 返回 1 给调用者，表示当前函数的执行结果。
        return 1;
    // 判断条件：(c0 & 0xE0) == 0xC0 是否成立。
    if ((c0 & 0xE0) == 0xC0)
        // 返回 2 给调用者，表示当前函数的执行结果。
        return 2;
    // 判断条件：(c0 & 0xF0) == 0xE0 是否成立。
    if ((c0 & 0xF0) == 0xE0)
        // 返回 3 给调用者，表示当前函数的执行结果。
        return 3;
    // 判断条件：(c0 & 0xF8) == 0xF0 是否成立。
    if ((c0 & 0xF8) == 0xF0)
        // 返回 4 给调用者，表示当前函数的执行结果。
        return 4;
    // 返回 0 给调用者，表示当前函数的执行结果。
    return 0;
}

// 函数定义/声明：调用 utf8_decode()：UTF8 是变长编码： ASCII 字母数字是 1 字节。 大部分拉丁扩展字符是 2 字节。 常见中文是 3 字节。 Unicode 较大范围字符是 4 字节。。
inline uint8_t utf8_decode(const char* p, uint32_t& cp)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t c0 = static_cast<uint8_t>(*p);
    // 判断条件：c0 == 0 是否成立。
    if (c0 == 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t len = utf8_seq_len(c0);
    // 判断条件：len == 0 是否成立。
    if (len == 0)
    {
        // 赋值：把右侧 '?' 的结果保存到左侧 cp。
        cp = '?';
        // 返回 1 给调用者，表示当前函数的执行结果。
        return 1;
    }

    // 静态查表数组 kFirstByteMask：运行时复用固定数据，避免每次重新计算。
    static constexpr uint8_t kFirstByteMask[5] = {0, 0x7F, 0x1F, 0x0F, 0x07};
    // 定义变量 uint32_t value 并初始化为 static_cast<uint32_t>(c0 & kFirstByteMask[len])。
    uint32_t value = static_cast<uint32_t>(c0 & kFirstByteMask[len]);

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 1; i < len; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t c = static_cast<uint8_t>(p[i]);
        // 判断条件：(c & 0xC0) != 0x80 是否成立。
        if ((c & 0xC0) != 0x80)
        {
            // 赋值：把右侧 '?' 的结果保存到左侧 cp。
            cp = '?';
            // 返回 1 给调用者，表示当前函数的执行结果。
            return 1;
        }
        // 赋值：把右侧 (value << 6) | static_cast<uint32_t>(c & 0x3F) 的结果保存到左侧 value。
        value = (value << 6) | static_cast<uint32_t>(c & 0x3F);
    }
    // 赋值：把右侧 value 的结果保存到左侧 cp。
    cp = value;
    // 返回 len 给调用者，表示当前函数的执行结果。
    return len;
}

// 定义或声明函数 utf8_next：参数在括号里，返回类型写在函数名前面。
inline bool utf8_next(const char*& p, uint32_t& cp)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t n = utf8_decode(p, cp);
    // 判断条件：n == 0 是否成立。
    if (n == 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 赋值：把右侧 n 的结果保存到左侧 p +。
    p += n;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::gfx
