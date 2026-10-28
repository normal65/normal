/**
 * [初学者导读] 文件：main/app/ebook/text/path_encoding.cc
 * 作用：处理 UTF-8、GBK 和文件路径显示编码。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：上层通过 Router、UiBus 和 SystemState 协作，不直接操作硬件。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "text/path_encoding.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/utf8.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::text {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// kGbkLeadMin / kGbkLeadMax / kGbkTrailCount / kGbkGrid[]
#include "text/gbk_table.inc"

// 定义或声明函数 gbk_trail_index：参数在括号里，返回类型写在函数名前面。
bool gbk_trail_index(uint8_t trail, uint8_t& idx)
{
    // 判断条件：trail < 0x40 || trail == 0x7F || trail > 0xFE 是否成立。
    if (trail < 0x40 || trail == 0x7F || trail > 0xFE)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 赋值：把右侧 (trail < 0x7F) ? static_cast<uint8_t>(trail - 0x40) : static_cast<uint8_t>(trail - 0x41) 的结果保存到左侧 idx。
    idx = (trail < 0x7F) ? static_cast<uint8_t>(trail - 0x40) : static_cast<uint8_t>(trail - 0x41);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 gbk_pair_to_unicode：参数在括号里，返回类型写在函数名前面。
bool gbk_pair_to_unicode(uint8_t lead, uint8_t trail, uint16_t& uni)
{
    // 判断条件：lead < kGbkLeadMin || lead > kGbkLeadMax 是否成立。
    if (lead < kGbkLeadMin || lead > kGbkLeadMax)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 定义变量 uint8_t ti 并初始化为 0。
    uint8_t ti = 0;
    // 判断条件：!gbk_trail_index(trail, ti) 是否成立。
    if (!gbk_trail_index(trail, ti))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t li = static_cast<size_t>(lead - kGbkLeadMin);
    // 赋值：把右侧 kGbkGrid[li * kGbkTrailCount + ti] 的结果保存到左侧 uni。
    uni = kGbkGrid[li * kGbkTrailCount + ti];
    // 返回 uni != 0xFFFF 给调用者，表示当前函数的执行结果。
    return uni != 0xFFFF;
}

// 定义或声明函数 append_utf8：参数在括号里，返回类型写在函数名前面。
size_t append_utf8(uint32_t cp, char* dst, size_t dst_cap, size_t di)
{
    // 判断条件：cp < 0x80 是否成立。
    if (cp < 0x80)
    {
        // 判断条件：di + 1 >= dst_cap 是否成立。
        if (di + 1 >= dst_cap)
            // 返回 di 给调用者，表示当前函数的执行结果。
            return di;
        // 赋值：把右侧 static_cast<char>(cp) 的结果保存到左侧 dst[di++]。
        dst[di++] = static_cast<char>(cp);
        // 返回 di 给调用者，表示当前函数的执行结果。
        return di;
    }
    // 判断条件：cp < 0x800 是否成立。
    if (cp < 0x800)
    {
        // 判断条件：di + 2 >= dst_cap 是否成立。
        if (di + 2 >= dst_cap)
            // 返回 di 给调用者，表示当前函数的执行结果。
            return di;
        // 赋值：把右侧 static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)) 的结果保存到左侧 dst[di++]。
        dst[di++] = static_cast<char>(0xC0 | ((cp >> 6) & 0x1F));
        // 赋值：把右侧 static_cast<char>(0x80 | (cp & 0x3F)) 的结果保存到左侧 dst[di++]。
        dst[di++] = static_cast<char>(0x80 | (cp & 0x3F));
        // 返回 di 给调用者，表示当前函数的执行结果。
        return di;
    }
    // 判断条件：cp < 0x10000 是否成立。
    if (cp < 0x10000)
    {
        // 判断条件：di + 3 >= dst_cap 是否成立。
        if (di + 3 >= dst_cap)
            // 返回 di 给调用者，表示当前函数的执行结果。
            return di;
        // 赋值：把右侧 static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)) 的结果保存到左侧 dst[di++]。
        dst[di++] = static_cast<char>(0xE0 | ((cp >> 12) & 0x0F));
        // 赋值：把右侧 static_cast<char>(0x80 | ((cp >> 6) & 0x3F)) 的结果保存到左侧 dst[di++]。
        dst[di++] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        // 赋值：把右侧 static_cast<char>(0x80 | (cp & 0x3F)) 的结果保存到左侧 dst[di++]。
        dst[di++] = static_cast<char>(0x80 | (cp & 0x3F));
        // 返回 di 给调用者，表示当前函数的执行结果。
        return di;
    }
    // 返回 di 给调用者，表示当前函数的执行结果。
    return di;
}

// 定义或声明函数 gbk_to_utf8：参数在括号里，返回类型写在函数名前面。
size_t gbk_to_utf8(const char* src, size_t src_len, char* dst, size_t dst_cap)
{
    // 判断条件：src == nullptr || dst == nullptr || dst_cap == 0 是否成立。
    if (src == nullptr || dst == nullptr || dst_cap == 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 定义变量 size_t si 并初始化为 0, di = 0。
    size_t si = 0, di = 0;
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (si < src_len && src[si] != '\0')
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const auto c = static_cast<uint8_t>(src[si]);
        // 判断条件：c < 0x80 是否成立。
        if (c < 0x80)
        {
            // 判断条件：di + 1 >= dst_cap 是否成立。
            if (di + 1 >= dst_cap)
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 赋值：把右侧 static_cast<char>(c) 的结果保存到左侧 dst[di++]。
            dst[di++] = static_cast<char>(c);
            // 执行语句：++si;；它会更新一次变量、对象或程序状态。
            ++si;
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        }
        // 判断条件：si + 1 >= src_len 是否成立。
        if (si + 1 >= src_len)
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 定义变量 uint16_t uni 并初始化为 0。
        uint16_t uni = 0;
        // 判断条件：!gbk_pair_to_unicode(c, static_cast<uint8_t>(src[si + 1]), uni) 是否成立。
        if (!gbk_pair_to_unicode(c, static_cast<uint8_t>(src[si + 1]), uni))
        {
            // 判断条件：di + 1 >= dst_cap 是否成立。
            if (di + 1 >= dst_cap)
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 赋值：把右侧 '?' 的结果保存到左侧 dst[di++]。
            dst[di++] = '?';
            // 执行语句：++si;；它会更新一次变量、对象或程序状态。
            ++si;
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        }

        // 只读约束：这个值或对象在当前位置不能被修改。
        const size_t next = append_utf8(uni, dst, dst_cap, di);
        // 判断条件：next == di 是否成立。
        if (next == di)
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 赋值：把右侧 next 的结果保存到左侧 di。
        di = next;
        // 赋值：把右侧 2 的结果保存到左侧 si +。
        si += 2;
    }

    // 赋值：把右侧 '\0' 的结果保存到左侧 dst[di < dst_cap ? di : (dst_cap - 1)]。
    dst[di < dst_cap ? di : (dst_cap - 1)] = '\0';
    // 返回 di 给调用者，表示当前函数的执行结果。
    return di;
}

// struct EncodeScore：把相关数据字段组合在一起，表示项目中的一个概念。
struct EncodeScore
{
    // 执行语句：uint32_t utf8_cjk3{0};；它会更新一次变量、对象或程序状态。
    uint32_t utf8_cjk3{0};
    // 执行语句：uint32_t gbk_cjk{0};；它会更新一次变量、对象或程序状态。
    uint32_t gbk_cjk{0};
    // 执行语句：uint32_t high_bytes{0};；它会更新一次变量、对象或程序状态。
    uint32_t high_bytes{0};
};

// 定义或声明函数 is_cjk：参数在括号里，返回类型写在函数名前面。
bool is_cjk(uint32_t cp)
{
    // 返回 cp >= 0x4E00U && cp <= 0x9FFFU 给调用者，表示当前函数的执行结果。
    return cp >= 0x4E00U && cp <= 0x9FFFU;
}

// 定义或声明函数 score_sample：参数在括号里，返回类型写在函数名前面。
void score_sample(const char* data, size_t len, EncodeScore& s)
{
    // 判断条件：data == nullptr || len == 0 是否成立。
    if (data == nullptr || len == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 定义变量 size_t i 并初始化为 0。
    size_t i = 0;
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (i < len)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const auto c0 = static_cast<uint8_t>(data[i]);
        // 判断条件：c0 < 0x80 是否成立。
        if (c0 < 0x80)
        {
            // 执行语句：++i;；它会更新一次变量、对象或程序状态。
            ++i;
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        }
        // 访问对象成员：++s.high_bytes;；读取或修改这个对象里的成员状态。
        ++s.high_bytes;

        // 尝试 UTF-8 三字节 CJK
        if ((c0 & 0xF0) == 0xE0 && i + 2 < len)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const auto c1 = static_cast<uint8_t>(data[i + 1]);
            // 只读约束：这个值或对象在当前位置不能被修改。
            const auto c2 = static_cast<uint8_t>(data[i + 2]);
            // 判断条件：(c1 & 0xC0) == 0x80 && (c2 & 0xC0) == 0x80 && !(c0 == 0xE0 && c1 < 0xA0) 是否成立。
            if ((c1 & 0xC0) == 0x80 && (c2 & 0xC0) == 0x80 && !(c0 == 0xE0 && c1 < 0xA0))
            {
                // 只读约束：这个值或对象在当前位置不能被修改。
                const uint32_t cp = (static_cast<uint32_t>(c0 & 0x0F) << 12) |
                                    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
                                    (static_cast<uint32_t>(c1 & 0x3F) << 6) |
                                    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
                                    static_cast<uint32_t>(c2 & 0x3F);
                // 判断条件：is_cjk(cp) 是否成立。
                if (is_cjk(cp))
                {
                    // 访问对象成员：++s.utf8_cjk3;；读取或修改这个对象里的成员状态。
                    ++s.utf8_cjk3;
                    // 赋值：把右侧 3 的结果保存到左侧 i +。
                    i += 3;
                    // 继续下一轮：跳过本次循环剩余代码。
                    continue;
                }
            }
        }

        // 尝试 GBK 双字节 CJK
        if (i + 1 < len)
        {
            // 定义变量 uint16_t uni 并初始化为 0。
            uint16_t uni = 0;
            // 判断条件：gbk_pair_to_unicode(c0, static_cast<uint8_t>(data[i + 1]), uni) && is_cjk(uni) 是否成立。
            if (gbk_pair_to_unicode(c0, static_cast<uint8_t>(data[i + 1]), uni) && is_cjk(uni))
            {
                // 访问对象成员：++s.gbk_cjk;；读取或修改这个对象里的成员状态。
                ++s.gbk_cjk;
                // 赋值：把右侧 2 的结果保存到左侧 i +。
                i += 2;
                // 继续下一轮：跳过本次循环剩余代码。
                continue;
            }
        }
        // 执行语句：++i;；它会更新一次变量、对象或程序状态。
        ++i;
    }
}

// 定义或声明函数 decide_encoding：参数在括号里，返回类型写在函数名前面。
TextEncoding decide_encoding(const EncodeScore& s)
{
    // 判断条件：s.high_bytes == 0 是否成立。
    if (s.high_bytes == 0)
        // 返回 TextEncoding::Utf8 给调用者，表示当前函数的执行结果。
        return TextEncoding::Utf8;
    // 判断条件：s.utf8_cjk3 > s.gbk_cjk 是否成立。
    if (s.utf8_cjk3 > s.gbk_cjk)
        // 返回 TextEncoding::Utf8 给调用者，表示当前函数的执行结果。
        return TextEncoding::Utf8;
    // 返回 TextEncoding::Gbk 给调用者，表示当前函数的执行结果。
    return TextEncoding::Gbk;
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 is_valid_utf8：参数在括号里，返回类型写在函数名前面。
bool is_valid_utf8(const char* s, size_t len)
{
    // 判断条件：s == nullptr 是否成立。
    if (s == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：len == 0 是否成立。
    if (len == 0)
        // 赋值：把右侧 std::strlen(s) 的结果保存到左侧 len。
        len = std::strlen(s);

    // 定义变量 size_t i 并初始化为 0。
    size_t i = 0;
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (i < len)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const auto c0 = static_cast<uint8_t>(s[i]);
        // 判断条件：c0 < 0x80 是否成立。
        if (c0 < 0x80)
        {
            // 执行语句：++i;；它会更新一次变量、对象或程序状态。
            ++i;
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        }

        // 2 字节序列
        if ((c0 & 0xE0) == 0xC0)
        {
            // 判断条件：i + 1 >= len 是否成立。
            if (i + 1 >= len)
                // 返回 false 给调用者，表示当前函数的执行结果。
                return false;
            // 只读约束：这个值或对象在当前位置不能被修改。
            const auto c1 = static_cast<uint8_t>(s[i + 1]);
            // 判断条件：(c1 & 0xC0) != 0x80 || c0 < 0xC2 是否成立。
            if ((c1 & 0xC0) != 0x80 || c0 < 0xC2)
                // 返回 false 给调用者，表示当前函数的执行结果。
                return false;
            // 赋值：把右侧 2 的结果保存到左侧 i +。
            i += 2;
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        }
        // 3 字节序列
        if ((c0 & 0xF0) == 0xE0)
        {
            // 判断条件：i + 2 >= len 是否成立。
            if (i + 2 >= len)
                // 返回 false 给调用者，表示当前函数的执行结果。
                return false;
            // 只读约束：这个值或对象在当前位置不能被修改。
            const auto c1 = static_cast<uint8_t>(s[i + 1]);
            // 只读约束：这个值或对象在当前位置不能被修改。
            const auto c2 = static_cast<uint8_t>(s[i + 2]);
            // 判断条件：(c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80 是否成立。
            if ((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80)
                // 返回 false 给调用者，表示当前函数的执行结果。
                return false;
            // 判断条件：c0 == 0xE0 && c1 < 0xA0 是否成立。
            if (c0 == 0xE0 && c1 < 0xA0)
                // 返回 false 给调用者，表示当前函数的执行结果。
                return false; // 拒绝过长编码
            // 赋值：把右侧 3 的结果保存到左侧 i +。
            i += 3;
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        }
        // 4 字节序列
        if ((c0 & 0xF8) == 0xF0)
        {
            // 判断条件：i + 3 >= len 是否成立。
            if (i + 3 >= len)
                // 返回 false 给调用者，表示当前函数的执行结果。
                return false;
            // 只读约束：这个值或对象在当前位置不能被修改。
            const auto c1 = static_cast<uint8_t>(s[i + 1]);
            // 只读约束：这个值或对象在当前位置不能被修改。
            const auto c2 = static_cast<uint8_t>(s[i + 2]);
            // 只读约束：这个值或对象在当前位置不能被修改。
            const auto c3 = static_cast<uint8_t>(s[i + 3]);
            // 判断条件：(c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80 是否成立。
            if ((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80)
                // 返回 false 给调用者，表示当前函数的执行结果。
                return false;
            // 判断条件：c0 == 0xF0 && c1 < 0x90 是否成立。
            if (c0 == 0xF0 && c1 < 0x90)
                // 返回 false 给调用者，表示当前函数的执行结果。
                return false;
            // 赋值：把右侧 4 的结果保存到左侧 i +。
            i += 4;
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        }
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 normalize_path_segment：参数在括号里，返回类型写在函数名前面。
size_t normalize_path_segment(const char* src, char* dst, size_t dst_cap)
{
    // 判断条件：src == nullptr || dst == nullptr || dst_cap == 0 是否成立。
    if (src == nullptr || dst == nullptr || dst_cap == 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t src_len = std::strlen(src);
    // 判断条件：is_valid_utf8(src, src_len) 是否成立。
    if (is_valid_utf8(src, src_len))
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const size_t copy_len = (src_len < dst_cap - 1) ? src_len : (dst_cap - 1);
        // 调用 std::memcpy(dst, src, copy_len);，执行这个函数并处理它的返回值或状态变化。
        std::memcpy(dst, src, copy_len);
        // 赋值：把右侧 '\0' 的结果保存到左侧 dst[copy_len]。
        dst[copy_len] = '\0';
        // 返回 copy_len 给调用者，表示当前函数的执行结果。
        return copy_len;
    }
    // 返回 gbk_to_utf8(src, src_len, dst, dst_cap) 给调用者，表示当前函数的执行结果。
    return gbk_to_utf8(src, src_len, dst, dst_cap);
}

// 定义或声明函数 detect_text_encoding：参数在括号里，返回类型写在函数名前面。
TextEncoding detect_text_encoding(const char* data, size_t len)
{
    // 判断条件：data == nullptr || len == 0 是否成立。
    if (data == nullptr || len == 0)
        // 返回 TextEncoding::Utf8 给调用者，表示当前函数的执行结果。
        return TextEncoding::Utf8;

    // UTF-8 BOM
    if (len >= 3 && static_cast<uint8_t>(data[0]) == 0xEF &&
        // 定义或声明函数 static_cast<uint8_t>：参数在括号里，返回类型写在函数名前面。
        static_cast<uint8_t>(data[1]) == 0xBB && static_cast<uint8_t>(data[2]) == 0xBF)
        // 返回 TextEncoding::Utf8 给调用者，表示当前函数的执行结果。
        return TextEncoding::Utf8;

    // 声明并默认初始化对象：EncodeScore s{}。
    EncodeScore s{};
    // 调用 score_sample(data, len, s);，执行这个函数并处理它的返回值或状态变化。
    score_sample(data, len, s);
    // 返回 decide_encoding(s) 给调用者，表示当前函数的执行结果。
    return decide_encoding(s);
}

// 定义或声明函数 detect_file_encoding：参数在括号里，返回类型写在函数名前面。
TextEncoding detect_file_encoding(const char* path)
{
    // 判断条件：path == nullptr || path[0] == '\0' 是否成立。
    if (path == nullptr || path[0] == '\0')
        // 返回 TextEncoding::Utf8 给调用者，表示当前函数的执行结果。
        return TextEncoding::Utf8;

    // 赋值：把右侧 std::fopen(path, "rb") 的结果保存到左侧 FILE* f。
    FILE* f = std::fopen(path, "rb");
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
        // 返回 TextEncoding::Utf8 给调用者，表示当前函数的执行结果。
        return TextEncoding::Utf8;

    // 判断条件：std::fseek(f, 0, SEEK_END) != 0 是否成立。
    if (std::fseek(f, 0, SEEK_END) != 0)
    {
        // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(f);
        // 返回 TextEncoding::Utf8 给调用者，表示当前函数的执行结果。
        return TextEncoding::Utf8;
    }
    // 只读约束：这个值或对象在当前位置不能被修改。
    const long sz = std::ftell(f);
    // 判断条件：sz <= 0 是否成立。
    if (sz <= 0)
    {
        // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(f);
        // 返回 TextEncoding::Utf8 给调用者，表示当前函数的执行结果。
        return TextEncoding::Utf8;
    }

    // 执行语句：uint8_t buf[4096];；它会更新一次变量、对象或程序状态。
    uint8_t buf[4096];
    // 只读约束：这个值或对象在当前位置不能被修改。
    const long offsets[] = {0, sz / 4, sz / 2, (sz * 3) / 4};
    // 声明并默认初始化对象：EncodeScore total{}。
    EncodeScore total{};

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (long off : offsets)
    {
        // 判断条件：off < 0 || off >= sz 是否成立。
        if (off < 0 || off >= sz)
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        // 判断条件：std::fseek(f, off, SEEK_SET) != 0 是否成立。
        if (std::fseek(f, off, SEEK_SET) != 0)
            // 继续下一轮：跳过本次循环剩余代码。
            continue;

        // 定义变量 size_t to_read 并初始化为 sizeof(buf)。
        size_t to_read = sizeof(buf);
        // 判断条件：off + static_cast<long>(to_read) > sz 是否成立。
        if (off + static_cast<long>(to_read) > sz)
            // 赋值：把右侧 static_cast<size_t>(sz - off) 的结果保存到左侧 to_read。
            to_read = static_cast<size_t>(sz - off);

        // 只读约束：这个值或对象在当前位置不能被修改。
        const size_t got = std::fread(buf, 1, to_read, f);
        // 判断条件：got == 0 是否成立。
        if (got == 0)
            // 继续下一轮：跳过本次循环剩余代码。
            continue;

        // 文件头 BOM
        if (off == 0 && got >= 3 && buf[0] == 0xEF && buf[1] == 0xBB && buf[2] == 0xBF)
        {
            // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
            std::fclose(f);
            // 返回 TextEncoding::Utf8 给调用者，表示当前函数的执行结果。
            return TextEncoding::Utf8;
        }
        // 调用 score_sample(reinterpret_cast<const char*>(buf), got, total);，执行这个函数并处理它的返回值或状态变化。
        score_sample(reinterpret_cast<const char*>(buf), got, total);
    }

    // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
    std::fclose(f);
    // 返回 decide_encoding(total) 给调用者，表示当前函数的执行结果。
    return decide_encoding(total);
}

// 函数调用：size_t convert_text_to_utf8(TextEncoding enc, const char* src, size_t src_len, char* dst,；执行该函数并处理它的返回值或副作用。
size_t convert_text_to_utf8(TextEncoding enc, const char* src, size_t src_len, char* dst,
                            // 继续上一行的表达式或参数列表：size_t dst_cap)。
                            size_t dst_cap)
{
    // 判断条件：src == nullptr || dst == nullptr || dst_cap == 0 是否成立。
    if (src == nullptr || dst == nullptr || dst_cap == 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 判断条件：enc == TextEncoding::Utf8 是否成立。
    if (enc == TextEncoding::Utf8)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const size_t copy_len = (src_len < dst_cap - 1) ? src_len : (dst_cap - 1);
        // 调用 std::memcpy(dst, src, copy_len);，执行这个函数并处理它的返回值或状态变化。
        std::memcpy(dst, src, copy_len);
        // 赋值：把右侧 '\0' 的结果保存到左侧 dst[copy_len]。
        dst[copy_len] = '\0';
        // 返回 copy_len 给调用者，表示当前函数的执行结果。
        return copy_len;
    }
    // 返回 gbk_to_utf8(src, src_len, dst, dst_cap) 给调用者，表示当前函数的执行结果。
    return gbk_to_utf8(src, src_len, dst, dst_cap);
}

// 定义或声明函数 next_text_codepoint：参数在括号里，返回类型写在函数名前面。
bool next_text_codepoint(TextEncoding enc, const char*& p, const char* end, uint32_t& cp)
{
    // 判断条件：p == nullptr || end == nullptr || p >= end 是否成立。
    if (p == nullptr || end == nullptr || p >= end)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 判断条件：enc == TextEncoding::Utf8 是否成立。
    if (enc == TextEncoding::Utf8)
    {
        // 判断条件：*p == '\0' 是否成立。
        if (*p == '\0')
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t n = gfx::utf8_decode(p, cp);
        // 判断条件：n == 0 是否成立。
        if (n == 0)
        {
            // 赋值：把右侧 static_cast<uint8_t>(*p) 的结果保存到左侧 cp。
            cp = static_cast<uint8_t>(*p);
            // 执行语句：++p;；它会更新一次变量、对象或程序状态。
            ++p;
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 赋值：把右侧 n 的结果保存到左侧 p +。
            p += n;
        }
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // GBK
    const auto c0 = static_cast<uint8_t>(*p);
    // 判断条件：c0 < 0x80 是否成立。
    if (c0 < 0x80)
    {
        // 赋值：把右侧 c0 的结果保存到左侧 cp。
        cp = c0;
        // 执行语句：++p;；它会更新一次变量、对象或程序状态。
        ++p;
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 判断条件：p + 1 >= end 是否成立。
    if (p + 1 >= end)
    {
        // 赋值：把右侧 '?' 的结果保存到左侧 cp。
        cp = '?';
        // 执行语句：++p;；它会更新一次变量、对象或程序状态。
        ++p;
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 定义变量 uint16_t uni 并初始化为 0。
    uint16_t uni = 0;
    // 判断条件：!gbk_pair_to_unicode(c0, static_cast<uint8_t>(p[1]), uni) 是否成立。
    if (!gbk_pair_to_unicode(c0, static_cast<uint8_t>(p[1]), uni))
    {
        // 赋值：把右侧 '?' 的结果保存到左侧 cp。
        cp = '?';
        // 执行语句：++p;；它会更新一次变量、对象或程序状态。
        ++p;
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 赋值：把右侧 uni 的结果保存到左侧 cp。
    cp = uni;
    // 赋值：把右侧 2 的结果保存到左侧 p +。
    p += 2;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::text
