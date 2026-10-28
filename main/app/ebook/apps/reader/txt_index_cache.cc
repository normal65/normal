/**
 * [初学者导读] 文件：main/app/ebook/apps/reader/txt_index_cache.cc
 * 作用：建立和缓存 TXT 分页索引。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/txt_index_cache.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cerrno>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <sys/stat.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "storage/storage.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "TxtIndexCache";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::reader {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 固定参数 kMagicIndex = 0x54585449U：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t kMagicIndex = 0x54585449U;
// 固定参数 kMagicProgress = 0x50545854U：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t kMagicProgress = 0x50545854U;
// 固定参数 kIndexVersion = 4：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kIndexVersion = 4;
// 固定参数 kProgressVersion = 1：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kProgressVersion = 1;
// 固定参数 kCacheRoot = "/.ebook/cache"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kCacheRoot = "/.ebook/cache";

// 定义或声明函数 pack：参数在括号里，返回类型写在函数名前面。
#pragma pack(push, 1)
// struct IndexHeader：把相关数据字段组合在一起，表示项目中的一个概念。
struct IndexHeader
{
    // 执行语句：uint32_t magic;；它会更新一次变量、对象或程序状态。
    uint32_t magic;
    // 执行语句：uint8_t version;；它会更新一次变量、对象或程序状态。
    uint8_t version;
    // 执行语句：uint8_t reserved0;；它会更新一次变量、对象或程序状态。
    uint8_t reserved0;
    // 执行语句：uint16_t viewport_w;；它会更新一次变量、对象或程序状态。
    uint16_t viewport_w;
    // 执行语句：uint16_t viewport_h;；它会更新一次变量、对象或程序状态。
    uint16_t viewport_h;
    // 执行语句：uint8_t lines_per_page;；它会更新一次变量、对象或程序状态。
    uint8_t lines_per_page;
    // 执行语句：uint8_t font_size;；它会更新一次变量、对象或程序状态。
    uint8_t font_size;
    // 执行语句：uint16_t line_height;；它会更新一次变量、对象或程序状态。
    uint16_t line_height;
    // 执行语句：uint16_t screen_w;；它会更新一次变量、对象或程序状态。
    uint16_t screen_w;
    // 执行语句：uint16_t screen_h;；它会更新一次变量、对象或程序状态。
    uint16_t screen_h;
    // 执行语句：uint32_t file_size;；它会更新一次变量、对象或程序状态。
    uint32_t file_size;
    // 执行语句：uint32_t page_count;；它会更新一次变量、对象或程序状态。
    uint32_t page_count;
};

// struct ProgressRecord：把相关数据字段组合在一起，表示项目中的一个概念。
struct ProgressRecord
{
    // 执行语句：uint32_t magic;；它会更新一次变量、对象或程序状态。
    uint32_t magic;
    // 执行语句：uint8_t version;；它会更新一次变量、对象或程序状态。
    uint8_t version;
    // 执行语句：uint16_t current_page;；它会更新一次变量、对象或程序状态。
    uint16_t current_page;
    // 执行语句：uint16_t total_pages;；它会更新一次变量、对象或程序状态。
    uint16_t total_pages;
};
// 定义或声明函数 pack：参数在括号里，返回类型写在函数名前面。
#pragma pack(pop)

// 定义或声明函数 fnv1a32：参数在括号里，返回类型写在函数名前面。
uint32_t fnv1a32(const char* s)
{
    // 定义变量 uint32_t h 并初始化为 2166136261U。
    uint32_t h = 2166136261U;
    // 判断条件：s == nullptr 是否成立。
    if (s == nullptr)
        // 返回 h 给调用者，表示当前函数的执行结果。
        return h;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (; *s != '\0'; ++s)
    {
        // 赋值：把右侧 static_cast<uint8_t>(*s) 的结果保存到左侧 h ^。
        h ^= static_cast<uint8_t>(*s);
        // 赋值：把右侧 16777619U 的结果保存到左侧 h *。
        h *= 16777619U;
    }
    // 返回 h 给调用者，表示当前函数的执行结果。
    return h;
}

// 只读约束：这个值或对象在当前位置不能被修改。
const char* mount_for(const char* book_path)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using namespace ::app::common::storage;
    // 判断条件：book_path == nullptr 是否成立。
    if (book_path == nullptr)
        // 返回 k_path_internal 给调用者，表示当前函数的执行结果。
        return k_path_internal;
    // 判断条件：std::strncmp(book_path, k_path_sd, std::strlen(k_path_sd)) == 0 是否成立。
    if (std::strncmp(book_path, k_path_sd, std::strlen(k_path_sd)) == 0)
        // 返回 k_path_sd 给调用者，表示当前函数的执行结果。
        return k_path_sd;
    // 返回 k_path_internal 给调用者，表示当前函数的执行结果。
    return k_path_internal;
}

// 定义或声明函数 mkdir_p：参数在括号里，返回类型写在函数名前面。
bool mkdir_p(const char* path)
{
    // 判断条件：path == nullptr || path[0] == '\0' 是否成立。
    if (path == nullptr || path[0] == '\0')
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 执行语句：char tmp[128];；它会更新一次变量、对象或程序状态。
    char tmp[128];
    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t len = std::strlen(path);
    // 判断条件：len >= sizeof(tmp) 是否成立。
    if (len >= sizeof(tmp))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 调用 std::memcpy(tmp, path, len + 1);，执行这个函数并处理它的返回值或状态变化。
    std::memcpy(tmp, path, len + 1);

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (size_t i = 1; i < len; ++i)
    {
        // 判断条件：tmp[i] != '/' 是否成立。
        if (tmp[i] != '/')
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        // 赋值：把右侧 '\0' 的结果保存到左侧 tmp[i]。
        tmp[i] = '\0';
        // 判断条件：::mkdir(tmp, 0755) != 0 && errno != EEXIST 是否成立。
        if (::mkdir(tmp, 0755) != 0 && errno != EEXIST)
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        // 赋值：把右侧 '/' 的结果保存到左侧 tmp[i]。
        tmp[i] = '/';
    }
    // 判断条件：::mkdir(tmp, 0755) != 0 && errno != EEXIST 是否成立。
    if (::mkdir(tmp, 0755) != 0 && errno != EEXIST)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 header_matches：参数在括号里，返回类型写在函数名前面。
bool header_matches(const IndexHeader& h, const TxtIndexLayout& l)
{
    // 返回 h.viewport_w == l.viewport_w && h.viewport_h == l.viewport_h && 给调用者，表示当前函数的执行结果。
    return h.viewport_w == l.viewport_w && h.viewport_h == l.viewport_h &&
           // 继续上一行的表达式或参数列表：h.lines_per_page == l.lines_per_page && h.font_size == l.font_size &&。
           h.lines_per_page == l.lines_per_page && h.font_size == l.font_size &&
           // 继续上一行的表达式或参数列表：h.line_height == l.line_height && h.screen_w == l.screen_w && h.screen_h == l.screen_h &&。
           h.line_height == l.line_height && h.screen_w == l.screen_w && h.screen_h == l.screen_h &&
           // 继续上一行的表达式或参数列表：h.file_size == l.file_size && h.reserved0 == l.text_encoding;。
           h.file_size == l.file_size && h.reserved0 == l.text_encoding;
}

// 定义或声明函数 read_exact：参数在括号里，返回类型写在函数名前面。
bool read_exact(FILE* f, void* p, size_t n)
{
    // 返回 std::fread(p, 1, n, f) == n 给调用者，表示当前函数的执行结果。
    return std::fread(p, 1, n, f) == n;
}

// 定义或声明函数 write_exact：参数在括号里，返回类型写在函数名前面。
bool write_exact(FILE* f, const void* p, size_t n)
{
    // 返回 std::fwrite(p, 1, n, f) == n 给调用者，表示当前函数的执行结果。
    return std::fwrite(p, 1, n, f) == n;
}

// 定义或声明函数 progress_path：参数在括号里，返回类型写在函数名前面。
bool progress_path(const char* book_path, char* out, size_t out_len)
{
    // 执行语句：char dir[128], idx[144];；它会更新一次变量、对象或程序状态。
    char dir[128], idx[144];
    // 判断条件：!TxtIndexCache::make_paths(book_path, dir, sizeof(dir), idx, sizeof(idx)) 是否成立。
    if (!TxtIndexCache::make_paths(book_path, dir, sizeof(dir), idx, sizeof(idx)))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int n = std::snprintf(out, out_len, "%s/progress.bin", dir);
    // 返回 n > 0 && static_cast<size_t>(n) < out_len 给调用者，表示当前函数的执行结果。
    return n > 0 && static_cast<size_t>(n) < out_len;
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 函数调用：bool TxtIndexCache::make_paths(const char* book_path, char* dir_out, size_t dir_len,；执行该函数并处理它的返回值或副作用。
bool TxtIndexCache::make_paths(const char* book_path, char* dir_out, size_t dir_len,
                               // 继续上一行的表达式或参数列表：char* index_out, size_t index_len)。
                               char* index_out, size_t index_len)
{
    // 判断条件：book_path == nullptr || dir_out == nullptr || index_out == nullptr 是否成立。
    if (book_path == nullptr || dir_out == nullptr || index_out == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint32_t hash = fnv1a32(book_path);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int dn = std::snprintf(dir_out, dir_len, "%s%s/txt_%08lx", mount_for(book_path),
                                 // 继续上一行的表达式或参数列表：kCacheRoot, static_cast<unsigned long>(hash));。
                                 kCacheRoot, static_cast<unsigned long>(hash));
    // 判断条件：dn <= 0 || static_cast<size_t>(dn) >= dir_len 是否成立。
    if (dn <= 0 || static_cast<size_t>(dn) >= dir_len)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int in = std::snprintf(index_out, index_len, "%s/index.bin", dir_out);
    // 返回 in > 0 && static_cast<size_t>(in) < index_len 给调用者，表示当前函数的执行结果。
    return in > 0 && static_cast<size_t>(in) < index_len;
}

// 定义或声明函数 TxtIndexCache::ensure_dir：参数在括号里，返回类型写在函数名前面。
bool TxtIndexCache::ensure_dir(const char* dir_path)
{
    // 返回 mkdir_p(dir_path) 给调用者，表示当前函数的执行结果。
    return mkdir_p(dir_path);
}

// 函数调用：bool TxtIndexCache::load(const char* book_path, const TxtIndexLayout& layout, uint32_t* offsets,；执行该函数并处理它的返回值或副作用。
bool TxtIndexCache::load(const char* book_path, const TxtIndexLayout& layout, uint32_t* offsets,
                         // 继续上一行的表达式或参数列表：uint16_t* total_pages)。
                         uint16_t* total_pages)
{
    // 判断条件：book_path == nullptr || offsets == nullptr || total_pages == nullptr 是否成立。
    if (book_path == nullptr || offsets == nullptr || total_pages == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 执行语句：char dir[128], index_path[144];；它会更新一次变量、对象或程序状态。
    char dir[128], index_path[144];
    // 判断条件：!make_paths(book_path, dir, sizeof(dir), index_path, sizeof(index_path)) 是否成立。
    if (!make_paths(book_path, dir, sizeof(dir), index_path, sizeof(index_path)))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 std::fopen(index_path, "rb") 的结果保存到左侧 FILE* f。
    FILE* f = std::fopen(index_path, "rb");
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 声明并默认初始化对象：IndexHeader hdr{}。
    IndexHeader hdr{};
    // 判断条件：!read_exact(f, &hdr, sizeof(hdr)) 是否成立。
    if (!read_exact(f, &hdr, sizeof(hdr)))
    {
        // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(f);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 判断条件：hdr.magic != kMagicIndex || hdr.version != kIndexVersion || !header_matches(hdr, layout) 是否成立。
    if (hdr.magic != kMagicIndex || hdr.version != kIndexVersion || !header_matches(hdr, layout))
    {
        // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(f);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 判断条件：hdr.page_count == 0 || hdr.page_count > kMaxPages 是否成立。
    if (hdr.page_count == 0 || hdr.page_count > kMaxPages)
    {
        // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(f);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint32_t i = 0; i < hdr.page_count; ++i)
    {
        // 定义变量 uint32_t off 并初始化为 0。
        uint32_t off = 0;
        // 判断条件：!read_exact(f, &off, sizeof(off)) 是否成立。
        if (!read_exact(f, &off, sizeof(off)))
        {
            // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
            std::fclose(f);
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }
        // 赋值：把右侧 off 的结果保存到左侧 offsets[i]。
        offsets[i] = off;
        // 判断条件：i > 0 && off <= offsets[i - 1] 是否成立。
        if (i > 0 && off <= offsets[i - 1])
        {
            // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
            std::fclose(f);
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }
    }
    // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
    std::fclose(f);

    *total_pages = static_cast<uint16_t>(hdr.page_count);
    // 调用 EBOOK_LOGD(TAG, "cache hit %u pages", static_cast<unsigned>(*total_pages));，执行这个函数并处理它的返回值或状态变化。
    EBOOK_LOGD(TAG, "cache hit %u pages", static_cast<unsigned>(*total_pages));
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数调用：bool TxtIndexCache::save(const char* book_path, const TxtIndexLayout& layout,；执行该函数并处理它的返回值或副作用。
bool TxtIndexCache::save(const char* book_path, const TxtIndexLayout& layout,
                         // 只读约束：这个值或对象在当前位置不能被修改。
                         const uint32_t* offsets, uint16_t total_pages)
{
    // 判断条件：book_path == nullptr || offsets == nullptr || total_pages == 0 是否成立。
    if (book_path == nullptr || offsets == nullptr || total_pages == 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 执行语句：char dir[128], index_path[144];；它会更新一次变量、对象或程序状态。
    char dir[128], index_path[144];
    // 判断条件：!make_paths(book_path, dir, sizeof(dir), index_path, sizeof(index_path)) 是否成立。
    if (!make_paths(book_path, dir, sizeof(dir), index_path, sizeof(index_path)))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：!ensure_dir(dir) 是否成立。
    if (!ensure_dir(dir))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 std::fopen(index_path, "wb") 的结果保存到左侧 FILE* f。
    FILE* f = std::fopen(index_path, "wb");
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 声明并默认初始化对象：IndexHeader hdr{}。
    IndexHeader hdr{};
    // 赋值：把右侧 kMagicIndex 的结果保存到左侧 hdr.magic。
    hdr.magic = kMagicIndex;
    // 赋值：把右侧 kIndexVersion 的结果保存到左侧 hdr.version。
    hdr.version = kIndexVersion;
    // 赋值：把右侧 layout.text_encoding 的结果保存到左侧 hdr.reserved0。
    hdr.reserved0 = layout.text_encoding;
    // 赋值：把右侧 layout.viewport_w 的结果保存到左侧 hdr.viewport_w。
    hdr.viewport_w = layout.viewport_w;
    // 赋值：把右侧 layout.viewport_h 的结果保存到左侧 hdr.viewport_h。
    hdr.viewport_h = layout.viewport_h;
    // 赋值：把右侧 layout.lines_per_page 的结果保存到左侧 hdr.lines_per_page。
    hdr.lines_per_page = layout.lines_per_page;
    // 赋值：把右侧 layout.font_size 的结果保存到左侧 hdr.font_size。
    hdr.font_size = layout.font_size;
    // 赋值：把右侧 layout.line_height 的结果保存到左侧 hdr.line_height。
    hdr.line_height = layout.line_height;
    // 赋值：把右侧 layout.screen_w 的结果保存到左侧 hdr.screen_w。
    hdr.screen_w = layout.screen_w;
    // 赋值：把右侧 layout.screen_h 的结果保存到左侧 hdr.screen_h。
    hdr.screen_h = layout.screen_h;
    // 赋值：把右侧 layout.file_size 的结果保存到左侧 hdr.file_size。
    hdr.file_size = layout.file_size;
    // 赋值：把右侧 total_pages 的结果保存到左侧 hdr.page_count。
    hdr.page_count = total_pages;

    // 定义变量 bool ok 并初始化为 write_exact(f, &hdr, sizeof(hdr))。
    bool ok = write_exact(f, &hdr, sizeof(hdr));
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint16_t i = 0; ok && i < total_pages; ++i)
        // 赋值：把右侧 write_exact(f, &offsets[i], sizeof(uint32_t)) 的结果保存到左侧 ok。
        ok = write_exact(f, &offsets[i], sizeof(uint32_t));
    // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
    std::fclose(f);

    // 判断条件：!ok 是否成立。
    if (!ok)
    {
        // 调用 std::remove(index_path)：VFS remove()。
        (void)std::remove(index_path);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 TxtIndexCache::load_progress：参数在括号里，返回类型写在函数名前面。
bool TxtIndexCache::load_progress(const char* book_path, uint16_t* current_page)
{
    // 判断条件：book_path == nullptr || current_page == nullptr 是否成立。
    if (book_path == nullptr || current_page == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 执行语句：char path[160];；它会更新一次变量、对象或程序状态。
    char path[160];
    // 判断条件：!progress_path(book_path, path, sizeof(path)) 是否成立。
    if (!progress_path(book_path, path, sizeof(path)))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 std::fopen(path, "rb") 的结果保存到左侧 FILE* f。
    FILE* f = std::fopen(path, "rb");
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 声明并默认初始化对象：ProgressRecord pf{}。
    ProgressRecord pf{};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool ok = read_exact(f, &pf, sizeof(pf));
    // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
    std::fclose(f);
    // 判断条件：!ok || pf.magic != kMagicProgress || pf.version != kProgressVersion || pf.total_pages == 0 是否成立。
    if (!ok || pf.magic != kMagicProgress || pf.version != kProgressVersion || pf.total_pages == 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    *current_page = pf.current_page;
    // 判断条件：*current_page >= pf.total_pages 是否成立。
    if (*current_page >= pf.total_pages)
        *current_page = static_cast<uint16_t>(pf.total_pages - 1);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数调用：bool TxtIndexCache::save_progress(const char* book_path, uint16_t current_page,；执行该函数并处理它的返回值或副作用。
bool TxtIndexCache::save_progress(const char* book_path, uint16_t current_page,
                                  // 继续上一行的表达式或参数列表：uint16_t total_pages)。
                                  uint16_t total_pages)
{
    // 判断条件：book_path == nullptr || total_pages == 0 是否成立。
    if (book_path == nullptr || total_pages == 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 执行语句：char path[160], dir[128], index_path[144];；它会更新一次变量、对象或程序状态。
    char path[160], dir[128], index_path[144];
    // 判断条件：!progress_path(book_path, path, sizeof(path)) 是否成立。
    if (!progress_path(book_path, path, sizeof(path)))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：!make_paths(book_path, dir, sizeof(dir), index_path, sizeof(index_path)) 是否成立。
    if (!make_paths(book_path, dir, sizeof(dir), index_path, sizeof(index_path)))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：!ensure_dir(dir) 是否成立。
    if (!ensure_dir(dir))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 判断条件：current_page >= total_pages 是否成立。
    if (current_page >= total_pages)
        // 赋值：把右侧 static_cast<uint16_t>(total_pages - 1) 的结果保存到左侧 current_page。
        current_page = static_cast<uint16_t>(total_pages - 1);

    // 声明并默认初始化对象：ProgressRecord pf{}。
    ProgressRecord pf{};
    // 赋值：把右侧 kMagicProgress 的结果保存到左侧 pf.magic。
    pf.magic = kMagicProgress;
    // 赋值：把右侧 kProgressVersion 的结果保存到左侧 pf.version。
    pf.version = kProgressVersion;
    // 赋值：把右侧 current_page 的结果保存到左侧 pf.current_page。
    pf.current_page = current_page;
    // 赋值：把右侧 total_pages 的结果保存到左侧 pf.total_pages。
    pf.total_pages = total_pages;

    // 赋值：把右侧 std::fopen(path, "wb") 的结果保存到左侧 FILE* f。
    FILE* f = std::fopen(path, "wb");
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool ok = write_exact(f, &pf, sizeof(pf));
    // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
    std::fclose(f);
    // 返回 ok 给调用者，表示当前函数的执行结果。
    return ok;
}

// 定义或声明函数 TxtIndexCache::query_read_percent：参数在括号里，返回类型写在函数名前面。
bool TxtIndexCache::query_read_percent(const char* book_path, uint8_t* percent_out)
{
    // 判断条件：book_path == nullptr || percent_out == nullptr 是否成立。
    if (book_path == nullptr || percent_out == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 执行语句：char path[160];；它会更新一次变量、对象或程序状态。
    char path[160];
    // 判断条件：!progress_path(book_path, path, sizeof(path)) 是否成立。
    if (!progress_path(book_path, path, sizeof(path)))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 std::fopen(path, "rb") 的结果保存到左侧 FILE* f。
    FILE* f = std::fopen(path, "rb");
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 声明并默认初始化对象：ProgressRecord pf{}。
    ProgressRecord pf{};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool ok = read_exact(f, &pf, sizeof(pf));
    // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
    std::fclose(f);
    // 判断条件：!ok || pf.magic != kMagicProgress || pf.version != kProgressVersion || pf.total_pages == 0 是否成立。
    if (!ok || pf.magic != kMagicProgress || pf.version != kProgressVersion || pf.total_pages == 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint32_t pct = (static_cast<uint32_t>(pf.current_page + 1U) * 100U) /
                         // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
                         static_cast<uint32_t>(pf.total_pages);
    *percent_out = static_cast<uint8_t>(pct > 100U ? 100U : pct);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::reader
