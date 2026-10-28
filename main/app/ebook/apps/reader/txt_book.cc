/**
 * [初学者导读] 文件：main/app/ebook/apps/reader/txt_book.cc
 * 作用：读取和解析 TXT 文本，处理编码、分页和内容缓存。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/txt_book.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_heap_caps.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/txt_index_cache.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/font.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/text_layout.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/utf8.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "memory/psram_alloc.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "text/path_encoding.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "TxtBook";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::reader {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using app::common::memory::alloc_psram_prefer;

// 定义或声明函数 glyph_advance：参数在括号里，返回类型写在函数名前面。
uint16_t glyph_advance(uint32_t cp, uint8_t font_px)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t a = gfx::Font::get_instance().advance(cp, font_px, gfx::FontFace::Text);
    // 返回 (a == 0) ? font_px : a 给调用者，表示当前函数的执行结果。
    return (a == 0) ? font_px : a;
}

// 定义或声明函数 strip_cr：参数在括号里，返回类型写在函数名前面。
void strip_cr(char* buf, uint32_t& len)
{
    // 判断条件：buf == nullptr || len == 0 是否成立。
    if (buf == nullptr || len == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 定义变量 uint32_t w 并初始化为 0。
    uint32_t w = 0;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint32_t r = 0; r < len; ++r)
    {
        // 判断条件：buf[r] == '\r' 是否成立。
        if (buf[r] == '\r')
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        // 赋值：把右侧 buf[r] 的结果保存到左侧 buf[w++]。
        buf[w++] = buf[r];
    }
    // 赋值：把右侧 w 的结果保存到左侧 len。
    len = w;
    // 赋值：把右侧 '\0' 的结果保存到左侧 buf[len]。
    buf[len] = '\0';
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 函数定义/声明：调用 TxtBook::close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
void TxtBook::close()
{
    // 判断条件：page_offsets_ != nullptr 是否成立。
    if (page_offsets_ != nullptr)
    {
        // 调用 heap_caps_free(page_offsets_);，执行这个函数并处理它的返回值或状态变化。
        heap_caps_free(page_offsets_);
        // 赋值：把右侧 nullptr 的结果保存到左侧 page_offsets_。
        page_offsets_ = nullptr;
    }
    // 判断条件：text_ != nullptr 是否成立。
    if (text_ != nullptr)
    {
        // 调用 heap_caps_free(text_);，执行这个函数并处理它的返回值或状态变化。
        heap_caps_free(text_);
        // 赋值：把右侧 nullptr 的结果保存到左侧 text_。
        text_ = nullptr;
    }
    // 赋值：把右侧 0 的结果保存到左侧 text_len_。
    text_len_ = 0;
    // 赋值：把右侧 0 的结果保存到左侧 page_count_。
    page_count_ = 0;
    // 赋值：把右侧 0 的结果保存到左侧 page_。
    page_ = 0;
    // 赋值：把右侧 0 的结果保存到左侧 file_size_。
    file_size_ = 0;
    // 赋值：把右侧 '\0' 的结果保存到左侧 path_[0]。
    path_[0] = '\0';
    // 赋值：把右侧 0 的结果保存到左侧 text_encoding_。
    text_encoding_ = 0;
}

// 定义或声明函数 TxtBook::load_file：参数在括号里，返回类型写在函数名前面。
bool TxtBook::load_file(const char* path)
{
    // 赋值：把右侧 std::fopen(path, "rb") 的结果保存到左侧 FILE* fp。
    FILE* fp = std::fopen(path, "rb");
    // 判断条件：fp == nullptr 是否成立。
    if (fp == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 判断条件：std::fseek(fp, 0, SEEK_END) != 0 是否成立。
    if (std::fseek(fp, 0, SEEK_END) != 0)
    {
        // 调用 std::fclose(fp);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(fp);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 只读约束：这个值或对象在当前位置不能被修改。
    const long sz = std::ftell(fp);
    // 判断条件：sz < 0 || static_cast<uint32_t>(sz) > kMaxFileBytes 是否成立。
    if (sz < 0 || static_cast<uint32_t>(sz) > kMaxFileBytes)
    {
        // 调用 std::fclose(fp);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(fp);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 判断条件：std::fseek(fp, 0, SEEK_SET) != 0 是否成立。
    if (std::fseek(fp, 0, SEEK_SET) != 0)
    {
        // 调用 std::fclose(fp);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(fp);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 static_cast<uint32_t>(sz) 的结果保存到左侧 file_size_。
    file_size_ = static_cast<uint32_t>(sz);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t raw_sz = (sz == 0) ? 1U : static_cast<size_t>(sz);
    // 赋值：把右侧 static_cast<char*>(alloc_psram_prefer(raw_sz + 1U)) 的结果保存到左侧 char* raw。
    char* raw = static_cast<char*>(alloc_psram_prefer(raw_sz + 1U));
    // 判断条件：raw == nullptr 是否成立。
    if (raw == nullptr)
    {
        // 调用 std::fclose(fp);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(fp);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 判断条件：sz > 0 是否成立。
    if (sz > 0)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const size_t rd = std::fread(raw, 1, static_cast<size_t>(sz), fp);
        // 判断条件：rd != static_cast<size_t>(sz) 是否成立。
        if (rd != static_cast<size_t>(sz))
        {
            // 调用 heap_caps_free(raw);，执行这个函数并处理它的返回值或状态变化。
            heap_caps_free(raw);
            // 调用 std::fclose(fp);，执行这个函数并处理它的返回值或状态变化。
            std::fclose(fp);
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }
    }
    // 调用 std::fclose(fp);，执行这个函数并处理它的返回值或状态变化。
    std::fclose(fp);
    // 赋值：把右侧 '\0' 的结果保存到左侧 raw[sz]。
    raw[sz] = '\0';

    // 只读约束：这个值或对象在当前位置不能被修改。
    const text::TextEncoding enc = text::detect_file_encoding(path);
    // 赋值：把右侧 static_cast<uint8_t>(enc) 的结果保存到左侧 text_encoding_。
    text_encoding_ = static_cast<uint8_t>(enc);

    // 判断条件：enc == text::TextEncoding::Utf8 是否成立。
    if (enc == text::TextEncoding::Utf8)
    {
        // 赋值：把右侧 raw 的结果保存到左侧 text_。
        text_ = raw;
        // 赋值：把右侧 static_cast<uint32_t>(sz) 的结果保存到左侧 text_len_。
        text_len_ = static_cast<uint32_t>(sz);
        // 调用 strip_cr(text_, text_len_);，执行这个函数并处理它的返回值或状态变化。
        strip_cr(text_, text_len_);
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t out_cap = static_cast<size_t>(sz) * 2U + 8U;
    // 赋值：把右侧 static_cast<char*>(alloc_psram_prefer(out_cap)) 的结果保存到左侧 char* utf8。
    char* utf8 = static_cast<char*>(alloc_psram_prefer(out_cap));
    // 判断条件：utf8 == nullptr 是否成立。
    if (utf8 == nullptr)
    {
        // 调用 heap_caps_free(raw);，执行这个函数并处理它的返回值或状态变化。
        heap_caps_free(raw);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t out_len =
        // 继续上一行的表达式或参数列表：text::convert_text_to_utf8(enc, raw, static_cast<size_t>(sz), utf8, out_cap);。
        text::convert_text_to_utf8(enc, raw, static_cast<size_t>(sz), utf8, out_cap);
    // 调用 heap_caps_free(raw);，执行这个函数并处理它的返回值或状态变化。
    heap_caps_free(raw);
    // 判断条件：out_len == 0 是否成立。
    if (out_len == 0)
    {
        // 调用 heap_caps_free(utf8);，执行这个函数并处理它的返回值或状态变化。
        heap_caps_free(utf8);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 赋值：把右侧 utf8 的结果保存到左侧 text_。
    text_ = utf8;
    // 赋值：把右侧 static_cast<uint32_t>(out_len) 的结果保存到左侧 text_len_。
    text_len_ = static_cast<uint32_t>(out_len);
    // 调用 strip_cr(text_, text_len_);，执行这个函数并处理它的返回值或状态变化。
    strip_cr(text_, text_len_);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 TxtBook::skip_line：参数在括号里，返回类型写在函数名前面。
uint32_t TxtBook::skip_line(uint32_t offset, const ReaderLayout& layout) const
{
    // 判断条件：text_ == nullptr || offset >= text_len_ 是否成立。
    if (text_ == nullptr || offset >= text_len_)
        // 返回 text_len_ 给调用者，表示当前函数的执行结果。
        return text_len_;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* p = text_ + offset;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* end = text_ + text_len_;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t max_w = layout.text_width();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t fpx = layout.font_px();

    // 判断条件：*p == '\n' 是否成立。
    if (*p == '\n')
        // 返回 offset + 1U 给调用者，表示当前函数的执行结果。
        return offset + 1U;

    // 定义变量 uint16_t line_w 并初始化为 0。
    uint16_t line_w = 0;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* line_end = p;

    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (p < end && *p != '\n')
    {
        // 定义变量 uint32_t cp 并初始化为 0。
        uint32_t cp = 0;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t n = gfx::utf8_decode(p, cp);
        // 判断条件：n == 0 是否成立。
        if (n == 0)
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint16_t adv = glyph_advance(cp, fpx);
        // 判断条件：line_w + adv > max_w 是否成立。
        if (line_w + adv > max_w)
        {
            // 判断条件：line_end == text_ + offset 是否成立。
            if (line_end == text_ + offset)
                // 赋值：把右侧 p + n 的结果保存到左侧 line_end。
                line_end = p + n;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        }
        // 赋值：把右侧 adv 的结果保存到左侧 line_w +。
        line_w += adv;
        // 赋值：把右侧 n 的结果保存到左侧 p +。
        p += n;
        // 赋值：把右侧 p 的结果保存到左侧 line_end。
        line_end = p;
    }

    // 判断条件：p < end && *p == '\n' 是否成立。
    if (p < end && *p == '\n')
        // 返回 static_cast<uint32_t>((p + 1) - text_) 给调用者，表示当前函数的执行结果。
        return static_cast<uint32_t>((p + 1) - text_);
    // 返回 static_cast<uint32_t>(line_end - text_) 给调用者，表示当前函数的执行结果。
    return static_cast<uint32_t>(line_end - text_);
}

// 定义或声明函数 TxtBook::try_load_cache：参数在括号里，返回类型写在函数名前面。
bool TxtBook::try_load_cache(const ReaderLayout& layout)
{
    // 判断条件：path_[0] == '\0' || page_offsets_ == nullptr 是否成立。
    if (path_[0] == '\0' || page_offsets_ == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const TxtIndexLayout idx = layout.make_index_layout(text_encoding_, file_size_);
    // 定义变量 uint16_t total 并初始化为 0。
    uint16_t total = 0;
    // 判断条件：!TxtIndexCache::load(path_, idx, page_offsets_, &total) 是否成立。
    if (!TxtIndexCache::load(path_, idx, page_offsets_, &total))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 total 的结果保存到左侧 page_count_。
    page_count_ = total;
    // 返回 page_count_ > 0 给调用者，表示当前函数的执行结果。
    return page_count_ > 0;
}

// 定义或声明函数 TxtBook::save_cache：参数在括号里，返回类型写在函数名前面。
void TxtBook::save_cache(const ReaderLayout& layout)
{
    // 判断条件：path_[0] == '\0' || page_offsets_ == nullptr || page_count_ == 0 是否成立。
    if (path_[0] == '\0' || page_offsets_ == nullptr || page_count_ == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const TxtIndexLayout idx = layout.make_index_layout(text_encoding_, file_size_);
    // 函数调用：(void)TxtIndexCache::save(path_, idx, page_offsets_, page_count_);；执行该函数并处理它的返回值或副作用。
    (void)TxtIndexCache::save(path_, idx, page_offsets_, page_count_);
}

// 定义或声明函数 TxtBook::build_pages：参数在括号里，返回类型写在函数名前面。
bool TxtBook::build_pages(const ReaderLayout& layout)
{
    // 判断条件：text_ == nullptr 是否成立。
    if (text_ == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 判断条件：page_offsets_ == nullptr 是否成立。
    if (page_offsets_ == nullptr)
    {
        // 赋值：把右侧 static_cast<uint32_t*>(alloc_psram_prefer(sizeof(uint32_t) * kMaxPages)) 的结果保存到左侧 page_offsets_。
        page_offsets_ = static_cast<uint32_t*>(alloc_psram_prefer(sizeof(uint32_t) * kMaxPages));
        // 判断条件：page_offsets_ == nullptr 是否成立。
        if (page_offsets_ == nullptr)
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
    }

    // 判断条件：try_load_cache(layout) 是否成立。
    if (try_load_cache(layout))
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t lines_per = layout.lines_per_page();
    // 定义变量 uint32_t pos 并初始化为 0。
    uint32_t pos = 0;
    // 赋值：把右侧 0 的结果保存到左侧 page_count_。
    page_count_ = 0;

    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (pos <= text_len_ && page_count_ < kMaxPages)
    {
        // 赋值：把右侧 pos 的结果保存到左侧 page_offsets_[page_count_++]。
        page_offsets_[page_count_++] = pos;
        // 判断条件：pos >= text_len_ 是否成立。
        if (pos >= text_len_)
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 定义变量 uint8_t lines 并初始化为 0。
        uint8_t lines = 0;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint32_t page_start = pos;
        // 循环执行：条件为真时重复运行，条件变化后退出。
        while (lines < lines_per && pos < text_len_)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint32_t next = skip_line(pos, layout);
            // 判断条件：next <= pos 是否成立。
            if (next <= pos)
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 赋值：把右侧 next 的结果保存到左侧 pos。
            pos = next;
            // 执行语句：++lines;；它会更新一次变量、对象或程序状态。
            ++lines;
        }
        // 判断条件：pos == page_start 是否成立。
        if (pos == page_start)
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }

    // 判断条件：page_count_ == 0 是否成立。
    if (page_count_ == 0)
    {
        // 赋值：把右侧 0 的结果保存到左侧 page_offsets_[0]。
        page_offsets_[0] = 0;
        // 赋值：把右侧 1 的结果保存到左侧 page_count_。
        page_count_ = 1;
    }

    // 调用 save_cache(layout);，执行这个函数并处理它的返回值或状态变化。
    save_cache(layout);
    // 调用 EBOOK_LOGD(TAG, "built %u pages", static_cast<unsigned>(page_count_));，执行这个函数并处理它的返回值或状态变化。
    EBOOK_LOGD(TAG, "built %u pages", static_cast<unsigned>(page_count_));
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 TxtBook::open：参数在括号里，返回类型写在函数名前面。
bool TxtBook::open(const char* path, const ReaderLayout& layout)
{
    // 调用 close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
    close();
    // 判断条件：path == nullptr || path[0] == '\0' 是否成立。
    if (path == nullptr || path[0] == '\0')
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 函数调用：(void)std::strncpy(path_, path, sizeof(path_) - 1);；执行该函数并处理它的返回值或副作用。
    (void)std::strncpy(path_, path, sizeof(path_) - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 path_[sizeof(path_) - 1]。
    path_[sizeof(path_) - 1] = '\0';

    // 赋值：把右侧 layout 的结果保存到左侧 layout_。
    layout_ = layout;

    // 判断条件：!load_file(path) 是否成立。
    if (!load_file(path))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：!build_pages(layout) 是否成立。
    if (!build_pages(layout))
    {
        // 调用 close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
        close();
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 赋值：把右侧 0 的结果保存到左侧 page_。
    page_ = 0;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 TxtBook::set_layout：参数在括号里，返回类型写在函数名前面。
bool TxtBook::set_layout(const ReaderLayout& layout)
{
    // 判断条件：!is_open() 是否成立。
    if (!is_open())
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：layout.font_px() == layout_.font_px() 是否成立。
    if (layout.font_px() == layout_.font_px())
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t old_page = page_;
    // 赋值：把右侧 layout 的结果保存到左侧 layout_。
    layout_ = layout;
    // 判断条件：!build_pages(layout) 是否成立。
    if (!build_pages(layout))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：old_page < page_count_ 是否成立。
    if (old_page < page_count_)
        // 赋值：把右侧 old_page 的结果保存到左侧 page_。
        page_ = old_page;
    // 备用分支：前面的条件不成立时执行这里。
    else if (page_count_ > 0)
        // 赋值：把右侧 static_cast<uint16_t>(page_count_ - 1) 的结果保存到左侧 page_。
        page_ = static_cast<uint16_t>(page_count_ - 1);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 TxtBook::set_page：参数在括号里，返回类型写在函数名前面。
bool TxtBook::set_page(uint16_t p)
{
    // 判断条件：page_count_ == 0 是否成立。
    if (page_count_ == 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：p >= page_count_ 是否成立。
    if (p >= page_count_)
        // 赋值：把右侧 static_cast<uint16_t>(page_count_ - 1) 的结果保存到左侧 p。
        p = static_cast<uint16_t>(page_count_ - 1);
    // 判断条件：p == page_ 是否成立。
    if (p == page_)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 赋值：把右侧 p 的结果保存到左侧 page_。
    page_ = p;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 TxtBook::next_page：参数在括号里，返回类型写在函数名前面。
bool TxtBook::next_page()
{
    // 判断条件：page_count_ == 0 || page_ + 1U >= page_count_ 是否成立。
    if (page_count_ == 0 || page_ + 1U >= page_count_)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 执行语句：++page_;；它会更新一次变量、对象或程序状态。
    ++page_;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 TxtBook::prev_page：参数在括号里，返回类型写在函数名前面。
bool TxtBook::prev_page()
{
    // 判断条件：page_ == 0 是否成立。
    if (page_ == 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 执行语句：--page_;；它会更新一次变量、对象或程序状态。
    --page_;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 TxtBook::paint_page：参数在括号里，返回类型写在函数名前面。
void TxtBook::paint_page(gfx::Canvas& canvas, const core::Rect& area) const
{
    // 判断条件：text_ == nullptr || page_count_ == 0 || page_ >= page_count_ 是否成立。
    if (text_ == nullptr || page_count_ == 0 || page_ >= page_count_)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint32_t start = page_offsets_[page_];
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint32_t end = (page_ + 1U < page_count_) ? page_offsets_[page_ + 1U] : text_len_;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* p = text_ + start;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* lim = text_ + end;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t fpx = layout_.font_px();
    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const uint16_t lh = layout_.line_height();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t max_lines = layout_.lines_per_page();

    // 定义变量 int16_t y 并初始化为 static_cast<int16_t>(area.y + ReaderLayout::kContentPadY)。
    int16_t y = static_cast<int16_t>(area.y + ReaderLayout::kContentPadY);

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < max_lines && p < lim && y + lh <= area.bottom(); ++i)
    {
        // 判断条件：*p == '\n' 是否成立。
        if (*p == '\n')
        {
            // 执行语句：++p;；它会更新一次变量、对象或程序状态。
            ++p;
            // 赋值：把右侧 static_cast<int16_t>(y + lh) 的结果保存到左侧 y。
            y = static_cast<int16_t>(y + lh);
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        }

        // 声明并默认初始化对象：gfx::LineSlice slice{}。
        gfx::LineSlice slice{};
        // 赋值：把右侧 p 的结果保存到左侧 slice.begin。
        slice.begin = p;
        // 定义变量 uint16_t line_w 并初始化为 0。
        uint16_t line_w = 0;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* line_end = p;

        // 循环执行：条件为真时重复运行，条件变化后退出。
        while (p < lim && *p != '\n')
        {
            // 定义变量 uint32_t cp 并初始化为 0。
            uint32_t cp = 0;
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint8_t n = gfx::utf8_decode(p, cp);
            // 判断条件：n == 0 是否成立。
            if (n == 0)
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint16_t adv = glyph_advance(cp, fpx);
            // 判断条件：line_w + adv > area.w 是否成立。
            if (line_w + adv > area.w)
            {
                // 判断条件：line_end == slice.begin 是否成立。
                if (line_end == slice.begin)
                    // 赋值：把右侧 p + n 的结果保存到左侧 line_end。
                    line_end = p + n;
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            }
            // 赋值：把右侧 adv 的结果保存到左侧 line_w +。
            line_w += adv;
            // 赋值：把右侧 n 的结果保存到左侧 p +。
            p += n;
            // 赋值：把右侧 p 的结果保存到左侧 line_end。
            line_end = p;
        }

        // 赋值：把右侧 line_end 的结果保存到左侧 slice.end。
        slice.end = line_end;
        // 赋值：把右侧 line_w 的结果保存到左侧 slice.width_px。
        slice.width_px = line_w;

        // 执行语句：char line_buf[128];；它会更新一次变量、对象或程序状态。
        char line_buf[128];
        // 只读约束：这个值或对象在当前位置不能被修改。
        const size_t ln = static_cast<size_t>(slice.end - slice.begin);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const size_t cp_len = (ln < sizeof(line_buf) - 1) ? ln : (sizeof(line_buf) - 1);
        // 判断条件：cp_len > 0 是否成立。
        if (cp_len > 0)
            // 调用 std::memcpy(line_buf, slice.begin, cp_len);，执行这个函数并处理它的返回值或状态变化。
            std::memcpy(line_buf, slice.begin, cp_len);
        // 赋值：把右侧 '\0' 的结果保存到左侧 line_buf[cp_len]。
        line_buf[cp_len] = '\0';

        // 调用 text(area.x,y,line_buf,fpx)：glyph()： 接收一个 Unicode 码点，委托 Font::drawglyph() 画一个字形，并返回该字形的推进宽度。。
        (void)canvas.text(area.x, y, line_buf, fpx);

        // 判断条件：p < lim && *p == '\n' 是否成立。
        if (p < lim && *p == '\n')
            // 执行语句：++p;；它会更新一次变量、对象或程序状态。
            ++p;
        // 赋值：把右侧 static_cast<int16_t>(y + lh) 的结果保存到左侧 y。
        y = static_cast<int16_t>(y + lh);
    }
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::reader
