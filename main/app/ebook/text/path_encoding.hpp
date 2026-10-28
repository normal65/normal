// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/text/path_encoding.hpp
 * 作用：处理 UTF-8、GBK 和文件路径显示编码。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：上层通过 Router、UiBus 和 SystemState 协作，不直接操作硬件。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstddef>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::text {

/** 文本编码 */
// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class TextEncoding : uint8_t
{
    // 执行这条代码：Utf8 = 0,。它会更新变量、对象状态或调用链。
    Utf8 = 0,
    // 继续上一行的表达式或参数列表：Gbk = 1,。
    Gbk = 1,
};

/** 严格 UTF-8 校验（拒绝过长编码） */
// 赋值：把右侧 0) 的结果保存到左侧 bool is_valid_utf8(const char* s, size_t len。
bool is_valid_utf8(const char* s, size_t len = 0);

/** 检测一段字节的编码（合法 UTF-8 视作 UTF-8，否则 GBK） */
// 函数调用：TextEncoding detect_text_encoding(const char* data, size_t len);；执行该函数并处理它的返回值或副作用。
TextEncoding detect_text_encoding(const char* data, size_t len);

/** 多点采样检测整文件编码 */
// 函数调用：TextEncoding detect_file_encoding(const char* path);；执行该函数并处理它的返回值或副作用。
TextEncoding detect_file_encoding(const char* path);

/**
 * @brief 把文件名 / 目录名规范为 UTF-8 供 UI 显示
 *
 * - 已是 UTF-8 → 原样拷贝
 * - 否则按 GBK 解码并转 UTF-8
 * - 用 readdir/stat 时仍应保留原始字节路径打开文件
 *
 * @return 写入字节数（不含 '\0'）
 */
// 函数调用：size_t normalize_path_segment(const char* src, char* dst, size_t dst_cap);；执行该函数并处理它的返回值或副作用。
size_t normalize_path_segment(const char* src, char* dst, size_t dst_cap);

/** 按指定编码把字节流转 UTF-8 */
// 函数调用：size_t convert_text_to_utf8(TextEncoding enc, const char* src, size_t src_len, char* dst,；执行该函数并处理它的返回值或副作用。
size_t convert_text_to_utf8(TextEncoding enc, const char* src, size_t src_len, char* dst,
                            // 继续上一行的表达式或参数列表：size_t dst_cap);。
                            size_t dst_cap);

/**
 * @brief 按编码读下一个码点（用于分页 / 折行）
 *
 * @param[in]  p    当前指针，成功后前移
 * @param[in]  end  缓冲末尾（不含）
 * @param[out] cp   输出码点
 * @return 是否成功读到一个码点
 */
// 函数调用：bool next_text_codepoint(TextEncoding enc, const char*& p, const char* end, uint32_t& cp);；执行该函数并处理它的返回值或副作用。
bool next_text_codepoint(TextEncoding enc, const char*& p, const char* end, uint32_t& cp);

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::text
