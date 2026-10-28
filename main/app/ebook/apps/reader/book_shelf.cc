/**
 * [初学者导读] 文件：main/app/ebook/apps/reader/book_shelf.cc
 * 作用：扫描书架目录并管理书籍列表。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/book_shelf.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <dirent.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <sys/stat.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "memory/psram_alloc.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "storage/storage.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "text/path_encoding.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "BookShelf";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::reader {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using Mount = ::app::common::storage::MountKind;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using app::common::memory::alloc_array_psram;

// 定义或声明函数 ends_with_txt_ci：参数在括号里，返回类型写在函数名前面。
bool ends_with_txt_ci(const char* name)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t n = (name != nullptr) ? std::strlen(name) : 0;
    // 判断条件：n < 4 是否成立。
    if (n < 4)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* t = name + n - 4;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (int i = 0; i < 4; ++i)
    {
        // 定义变量 char c 并初始化为 t[i]。
        char c = t[i];
        // 判断条件：c >= 'A' && c <= 'Z' 是否成立。
        if (c >= 'A' && c <= 'Z')
            // 赋值：把右侧 static_cast<char>(c + ('a' - 'A')) 的结果保存到左侧 c。
            c = static_cast<char>(c + ('a' - 'A'));
        // 判断条件：".txt"[i] != c 是否成立。
        if (".txt"[i] != c)
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
    }
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 title_from_filename：参数在括号里，返回类型写在函数名前面。
void title_from_filename(const char* file_name, char* out, size_t out_cap)
{
    // 判断条件：file_name == nullptr || out == nullptr || out_cap == 0 是否成立。
    if (file_name == nullptr || out == nullptr || out_cap == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 执行语句：char raw[48];；它会更新一次变量、对象或程序状态。
    char raw[48];
    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t flen = std::strlen(file_name);
    // 定义变量 size_t copy_len 并初始化为 flen。
    size_t copy_len = flen;
    // 判断条件：flen >= 4 && ends_with_txt_ci(file_name) 是否成立。
    if (flen >= 4 && ends_with_txt_ci(file_name))
        // 赋值：把右侧 flen - 4U 的结果保存到左侧 copy_len。
        copy_len = flen - 4U;
    // 判断条件：copy_len >= sizeof(raw) 是否成立。
    if (copy_len >= sizeof(raw))
        // 赋值：把右侧 sizeof(raw) - 1U 的结果保存到左侧 copy_len。
        copy_len = sizeof(raw) - 1U;
    // 调用 std::memcpy(raw, file_name, copy_len);，执行这个函数并处理它的返回值或状态变化。
    std::memcpy(raw, file_name, copy_len);
    // 赋值：把右侧 '\0' 的结果保存到左侧 raw[copy_len]。
    raw[copy_len] = '\0';

    // 函数调用：(void)text::normalize_path_segment(raw, out, out_cap);；执行该函数并处理它的返回值或副作用。
    (void)text::normalize_path_segment(raw, out, out_cap);
    // 判断条件：out[0] == '\0' 是否成立。
    if (out[0] == '\0')
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const size_t n = (copy_len < out_cap - 1) ? copy_len : (out_cap - 1);
        // 调用 std::memcpy(out, raw, n);，执行这个函数并处理它的返回值或状态变化。
        std::memcpy(out, raw, n);
        // 赋值：把右侧 '\0' 的结果保存到左侧 out[n]。
        out[n] = '\0';
    }
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 BookShelf::ensure_items：参数在括号里，返回类型写在函数名前面。
bool BookShelf::ensure_items()
{
    // 判断条件：items_ != nullptr 是否成立。
    if (items_ != nullptr)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    // 赋值：把右侧 alloc_array_psram<BookItem>(kMaxBooks) 的结果保存到左侧 items_。
    items_ = alloc_array_psram<BookItem>(kMaxBooks);
    // 返回 items_ != nullptr 给调用者，表示当前函数的执行结果。
    return items_ != nullptr;
}

// 定义或声明函数 BookShelf::clear：参数在括号里，返回类型写在函数名前面。
void BookShelf::clear()
{
    // 赋值：把右侧 0 的结果保存到左侧 count_。
    count_ = 0;
}

// 只读约束：这个值或对象在当前位置不能被修改。
const BookItem& BookShelf::item(uint8_t idx) const
{
    // 静态空对象 kEmpty：作为默认返回值或占位值，避免返回悬空引用。
    static const BookItem kEmpty{};
    // 判断条件：items_ == nullptr || idx >= count_ 是否成立。
    if (items_ == nullptr || idx >= count_)
        // 返回 kEmpty 给调用者，表示当前函数的执行结果。
        return kEmpty;
    // 返回 items_[idx] 给调用者，表示当前函数的执行结果。
    return items_[idx];
}

// 定义或声明函数 BookShelf::add_unique：参数在括号里，返回类型写在函数名前面。
bool BookShelf::add_unique(const char* full_path, const char* file_name, uint32_t size_bytes)
{
    // 判断条件：!ensure_items() || count_ >= kMaxBooks || full_path == nullptr || file_name == nullptr 是否成立。
    if (!ensure_items() || count_ >= kMaxBooks || full_path == nullptr || file_name == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：!ends_with_txt_ci(file_name) 是否成立。
    if (!ends_with_txt_ci(file_name))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < count_; ++i)
    {
        // 判断条件：std::strcmp(items_[i].path, full_path) == 0 是否成立。
        if (std::strcmp(items_[i].path, full_path) == 0)
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
    }

    // 赋值：把右侧 items_[count_] 的结果保存到左侧 BookItem& b。
    BookItem& b = items_[count_];
    // 访问对象成员：(void)std::strncpy(b.path, full_path, sizeof(b.path) - 1);；读取或修改这个对象里的成员状态。
    (void)std::strncpy(b.path, full_path, sizeof(b.path) - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 b.path[sizeof(b.path) - 1]。
    b.path[sizeof(b.path) - 1] = '\0';
    // 调用 title_from_filename(file_name, b.title, sizeof(b.title));，执行这个函数并处理它的返回值或状态变化。
    title_from_filename(file_name, b.title, sizeof(b.title));
    // 赋值：把右侧 size_bytes 的结果保存到左侧 b.size_bytes。
    b.size_bytes = size_bytes;
    // 执行语句：++count_;；它会更新一次变量、对象或程序状态。
    ++count_;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 BookShelf::scan_mount：参数在括号里，返回类型写在函数名前面。
bool BookShelf::scan_mount(const char* base_mount)
{
    // 判断条件：base_mount == nullptr 是否成立。
    if (base_mount == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 ::app::common::storage::StorageMgr::get_instance() 的结果保存到左侧 auto& sto。
    auto& sto = ::app::common::storage::StorageMgr::get_instance();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const Mount kind = ::app::common::storage::mount_kind_from_path_prefix(base_mount);
    // 判断条件：kind == Mount::Invalid || !sto.is_mounted(kind) 是否成立。
    if (kind == Mount::Invalid || !sto.is_mounted(kind))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 执行语句：char dir_path[96];；它会更新一次变量、对象或程序状态。
    char dir_path[96];
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int n = std::snprintf(dir_path, sizeof(dir_path), "%s%s", base_mount, kBookDirTxt);
    // 判断条件：n <= 0 || static_cast<size_t>(n) >= sizeof(dir_path) 是否成立。
    if (n <= 0 || static_cast<size_t>(n) >= sizeof(dir_path))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 ::opendir(dir_path) 的结果保存到左侧 DIR* dp。
    DIR* dp = ::opendir(dir_path);
    // 判断条件：dp == nullptr 是否成立。
    if (dp == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // struct dirent：把相关数据字段组合在一起，表示项目中的一个概念。
    struct dirent* ent = nullptr;
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while ((ent = ::readdir(dp)) != nullptr && count_ < kMaxBooks)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* name = ent->d_name;
        // 判断条件：name[0] == '\0' || name[0] == '.' 是否成立。
        if (name[0] == '\0' || name[0] == '.')
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        // 判断条件：!ends_with_txt_ci(name) 是否成立。
        if (!ends_with_txt_ci(name))
            // 继续下一轮：跳过本次循环剩余代码。
            continue;

        // 执行语句：char full[96];；它会更新一次变量、对象或程序状态。
        char full[96];
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int pn = std::snprintf(full, sizeof(full), "%s/%s", dir_path, name);
        // 判断条件：pn <= 0 || static_cast<size_t>(pn) >= sizeof(full) 是否成立。
        if (pn <= 0 || static_cast<size_t>(pn) >= sizeof(full))
            // 继续下一轮：跳过本次循环剩余代码。
            continue;

        // struct stat：把相关数据字段组合在一起，表示项目中的一个概念。
        struct stat st{};
        // 判断条件：::stat(full, &st) != 0 || S_ISDIR(st.st_mode) 是否成立。
        if (::stat(full, &st) != 0 || S_ISDIR(st.st_mode))
            // 继续下一轮：跳过本次循环剩余代码。
            continue;

        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        (void)add_unique(full, name, static_cast<uint32_t>(st.st_size));
    }
    // 函数调用：::closedir(dp);；执行该函数并处理它的返回值或副作用。
    ::closedir(dp);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 BookShelf::sort_by_title：参数在括号里，返回类型写在函数名前面。
void BookShelf::sort_by_title()
{
    // 判断条件：items_ == nullptr || count_ < 2 是否成立。
    if (items_ == nullptr || count_ < 2)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 1; i < count_; ++i)
    {
        // 定义变量 BookItem tmp 并初始化为 items_[i]。
        BookItem tmp = items_[i];
        // 定义变量 uint8_t j 并初始化为 i。
        uint8_t j = i;
        // 循环执行：条件为真时重复运行，条件变化后退出。
        while (j > 0 && std::strcmp(items_[j - 1].title, tmp.title) > 0)
        {
            // 赋值：把右侧 items_[j - 1] 的结果保存到左侧 items_[j]。
            items_[j] = items_[j - 1];
            // 执行语句：--j;；它会更新一次变量、对象或程序状态。
            --j;
        }
        // 赋值：把右侧 tmp 的结果保存到左侧 items_[j]。
        items_[j] = tmp;
    }
}

// 定义或声明函数 BookShelf::scan：参数在括号里，返回类型写在函数名前面。
void BookShelf::scan()
{
    // 调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    clear();
    // 赋值：把右侧 ::app::common::storage::StorageMgr::get_instance() 的结果保存到左侧 auto& sto。
    auto& sto = ::app::common::storage::StorageMgr::get_instance();
    // 判断条件：sto.is_mounted(Mount::Internal) 是否成立。
    if (sto.is_mounted(Mount::Internal))
    {
        // 函数调用：(void)::mkdir("/int/Ebook", 0775);；执行该函数并处理它的返回值或副作用。
        (void)::mkdir("/int/Ebook", 0775);
        // 函数调用：(void)::mkdir(kScanPathInt, 0775);；执行该函数并处理它的返回值或副作用。
        (void)::mkdir(kScanPathInt, 0775);
    }
    // 判断条件：sto.is_mounted(Mount::Sd) 是否成立。
    if (sto.is_mounted(Mount::Sd))
    {
        // 函数调用：(void)::mkdir("/sd/Ebook", 0775);；执行该函数并处理它的返回值或副作用。
        (void)::mkdir("/sd/Ebook", 0775);
        // 函数调用：(void)::mkdir(kScanPathSd, 0775);；执行该函数并处理它的返回值或副作用。
        (void)::mkdir(kScanPathSd, 0775);
    }
    // 函数调用：(void)scan_mount(::app::common::storage::k_path_internal);；执行该函数并处理它的返回值或副作用。
    (void)scan_mount(::app::common::storage::k_path_internal);
    // 函数调用：(void)scan_mount(::app::common::storage::k_path_sd);；执行该函数并处理它的返回值或副作用。
    (void)scan_mount(::app::common::storage::k_path_sd);
    // 判断条件：count_ > 1 是否成立。
    if (count_ > 1)
        // 调用 sort_by_title();，执行这个函数并处理它的返回值或状态变化。
        sort_by_title();
    // 调用 EBOOK_LOGD(TAG, "found %u books", static_cast<unsigned>(count_));，执行这个函数并处理它的返回值或状态变化。
    EBOOK_LOGD(TAG, "found %u books", static_cast<unsigned>(count_));
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::reader
