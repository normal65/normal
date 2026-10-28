/**
 * [初学者导读] 文件：main/app/ebook/apps/files/vfs_browser.cc
 * 作用：实现受控目录扫描、排序、路径校验和文件删除。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/files/vfs_browser.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <dirent.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <sys/stat.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <unistd.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "memory/psram_alloc.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "VfsBrowser";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::files {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using Mount = ::app::common::storage::MountKind;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using app::common::memory::alloc_array_psram;

// 固定参数 kSdLabel = "SD Card"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kSdLabel = "SD Card";

// 编译期数组 kRoots：运行时只读的固定查表数据。
constexpr VfsBrowser::RootVol kRoots[] = {
    // 执行这条代码：{::app::common::storage::k_path_internal, ::app::common::storage::k_label_userdata,。它会更新变量、对象状态或调用链。
    {::app::common::storage::k_path_internal, ::app::common::storage::k_label_userdata,
     // 继续上一行的表达式或参数列表：Mount::Internal},。
     Mount::Internal},
    // 继续上一行的表达式或参数列表：{::app::common::storage::k_path_sd, kSdLabel, Mount::Sd},。
    {::app::common::storage::k_path_sd, kSdLabel, Mount::Sd},
    // 继续上一行的表达式或参数列表：{::app::common::storage::k_path_assets, ::app::common::storage::k_label_assets, Mount::Assets},。
    {::app::common::storage::k_path_assets, ::app::common::storage::k_label_assets, Mount::Assets},
};
// 固定参数 kRootSlotCount = sizeof(kRoots) / sizeof(kRoots[0])：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kRootSlotCount = sizeof(kRoots) / sizeof(kRoots[0]);

// 定义或声明函数 sort_entries：参数在括号里，返回类型写在函数名前面。
void sort_entries(VfsBrowser::Entry* entries, uint8_t count)
{
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 1; i < count; ++i)
    {
        // 定义变量 VfsBrowser::Entry tmp 并初始化为 entries[i]。
        VfsBrowser::Entry tmp = entries[i];
        // 定义变量 uint8_t j 并初始化为 i。
        uint8_t j = i;
        // 循环执行：条件为真时重复运行，条件变化后退出。
        while (j > 0)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const VfsBrowser::Entry& prev = entries[j - 1];
            // 定义变量 bool before 并初始化为 false。
            bool before = false;
            // 判断条件：prev.is_dir != tmp.is_dir 是否成立。
            if (prev.is_dir != tmp.is_dir)
                // 赋值：把右侧 (!prev.is_dir && tmp.is_dir) 的结果保存到左侧 before。
                before = (!prev.is_dir && tmp.is_dir);
            // 备用分支：前面的条件不成立时执行这里。
            else if (std::strcmp(prev.name, tmp.name) > 0)
                // 赋值：把右侧 true 的结果保存到左侧 before。
                before = true;
            // 判断条件：!before 是否成立。
            if (!before)
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 赋值：把右侧 prev 的结果保存到左侧 entries[j]。
            entries[j] = prev;
            // 执行语句：--j;；它会更新一次变量、对象或程序状态。
            --j;
        }
        // 赋值：把右侧 tmp 的结果保存到左侧 entries[j]。
        entries[j] = tmp;
    }
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 函数定义/声明：调用 VfsBrowser::path_allowed()：这是第一层安全边界。它判断路径是否属于允许浏览的根目录，例如 /int、/sd、/assets。任何上层误传的 /、系统特殊路径或者拼错路径，都应在这里被拒绝。。
bool VfsBrowser::path_allowed(const char* path)
{
    // 文件管理只允许访问受控根卷，防止路径越过到系统挂载点。
    if (path == nullptr || path[0] != '/')
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const Mount kind = ::app::common::storage::mount_kind_from_path_prefix(path);
    // 返回 kind == Mount::Internal || kind == Mount::Sd || kind == Mount::Assets 给调用者，表示当前函数的执行结果。
    return kind == Mount::Internal || kind == Mount::Sd || kind == Mount::Assets;
}

// 函数定义/声明：调用 VfsBrowser::is_storage_root()：判断当前路径是否正好等于一个存储根卷。它常用于决定返回键行为：在根卷内部按返回应该回到根卷列表；在某个子目录按返回应该回到父目录。。
bool VfsBrowser::is_storage_root(const char* path)
{
    // 判断是否正好位于 /int、/sd 或 /assets 根目录。
    if (path == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 返回 std::strcmp(path, ::app::common::storage::k_path_internal) == 0 || 给调用者，表示当前函数的执行结果。
    return std::strcmp(path, ::app::common::storage::k_path_internal) == 0 ||
           // 继续上一行的表达式或参数列表：std::strcmp(path, ::app::common::storage::k_path_sd) == 0 ||。
           std::strcmp(path, ::app::common::storage::k_path_sd) == 0 ||
           // 继续上一行的表达式或参数列表：std::strcmp(path, ::app::common::storage::k_path_assets) == 0;。
           std::strcmp(path, ::app::common::storage::k_path_assets) == 0;
}

// 函数定义/声明：调用 VfsBrowser::visible_root_count()：返回当前真正可见的根卷数量。例如 SD 卡没有插入、没有挂载成功时，/sd 不应该出现在列表里。所以不能直接返回常量 3，而要根据 Storage 状态决定。。
uint8_t VfsBrowser::visible_root_count()
{
    // 只统计当前实际挂载成功的根卷。
    auto& sto = ::app::common::storage::StorageMgr::get_instance();
    // 定义变量 uint8_t n 并初始化为 0。
    uint8_t n = 0;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < kRootSlotCount; ++i)
        // 判断条件：sto.is_mounted(kRoots[i].kind) 是否成立。
        if (sto.is_mounted(kRoots[i].kind))
            // 执行语句：++n;；它会更新一次变量、对象或程序状态。
            ++n;
    // 返回 n 给调用者，表示当前函数的执行结果。
    return n;
}

// 定义只读对象或变量：初始化后不能在当前位置修改。
const VfsBrowser::RootVol* VfsBrowser::root_at_visible(uint8_t vis_idx)
{
    // 把界面可见索引映射到真实根卷，跳过未挂载卷。
    auto& sto = ::app::common::storage::StorageMgr::get_instance();
    // 定义变量 uint8_t seen 并初始化为 0。
    uint8_t seen = 0;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < kRootSlotCount; ++i)
    {
        // 判断条件：!sto.is_mounted(kRoots[i].kind) 是否成立。
        if (!sto.is_mounted(kRoots[i].kind))
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        // 判断条件：seen == vis_idx 是否成立。
        if (seen == vis_idx)
            // 返回 &kRoots[i] 给调用者，表示当前函数的执行结果。
            return &kRoots[i];
        // 执行语句：++seen;；它会更新一次变量、对象或程序状态。
        ++seen;
    }
    // 返回 nullptr 给调用者，表示当前函数的执行结果。
    return nullptr;
}

// 函数定义/声明：调用 VfsBrowser::entry_hidden()：判断某个名字是否应该隐藏。项目明确处理过 System Volume Information 和 $RECYCLE.BIN 这类 Windows 在可移动存储上产生的目录。。
bool VfsBrowser::entry_hidden(const char* cwd, const char* name)
{
    // 隐藏 Windows 可移动存储产生的系统目录，避免用户误操作。
    if (name == nullptr)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;

    // 静态查表：保存需要在文件列表中隐藏的系统文件名。
    static const char* const kHidden[] = {
        // 执行这条代码："System Volume Information",。它会更新变量、对象状态或调用链。
        "System Volume Information",
        // 继续上一行的表达式或参数列表："$RECYCLE.BIN",。
        "$RECYCLE.BIN",
    };
    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool on_sd =
        // 继续上一行的表达式或参数列表：::app::common::storage::path_has_prefix(cwd, ::app::common::storage::k_path_sd);。
        ::app::common::storage::path_has_prefix(cwd, ::app::common::storage::k_path_sd);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool on_int =
        // 继续上一行的表达式或参数列表：::app::common::storage::path_has_prefix(cwd, ::app::common::storage::k_path_internal);。
        ::app::common::storage::path_has_prefix(cwd, ::app::common::storage::k_path_internal);
    // 判断条件：!on_sd && !on_int 是否成立。
    if (!on_sd && !on_int)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (const char* h : kHidden)
        // 判断条件：std::strcmp(name, h) == 0 是否成立。
        if (std::strcmp(name, h) == 0)
            // 返回 true 给调用者，表示当前函数的执行结果。
            return true;
    // 返回 false 给调用者，表示当前函数的执行结果。
    return false;
}

// 定义只读对象或变量：初始化后不能在当前位置修改。
const VfsBrowser::Entry& VfsBrowser::entry_at(uint8_t idx) const
{
    // 静态空对象 kEmpty：作为默认返回值或占位值，避免返回悬空引用。
    static const Entry kEmpty{};
    // 判断条件：entries_ == nullptr || idx >= entry_count_ 是否成立。
    if (entries_ == nullptr || idx >= entry_count_)
        // 返回 kEmpty 给调用者，表示当前函数的执行结果。
        return kEmpty;
    // 返回 entries_[idx] 给调用者，表示当前函数的执行结果。
    return entries_[idx];
}

// 函数定义/声明：调用 VfsBrowser::ensure_entries()：惰性打开当前目录。它只有在 entries、计数或者当前目录状态需要重新扫描时才调用底层 opendir/readdir。这样做的目的是避免每次刷新界面都重复扫盘。。
bool VfsBrowser::ensure_entries()
{
    // 判断条件：entries_ != nullptr 是否成立。
    if (entries_ != nullptr)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    // 赋值：把右侧 alloc_array_psram<Entry>(kMaxEntries) 的结果保存到左侧 entries_。
    entries_ = alloc_array_psram<Entry>(kMaxEntries);
    // 返回 entries_ != nullptr 给调用者，表示当前函数的执行结果。
    return entries_ != nullptr;
}

// 函数定义/声明：调用 VfsBrowser::row_count()：返回列表应该显示多少行。它不是简单返回 entrycount，还要考虑： 是否显示“返回上一级”行。 当前是 Roots 还是 Browse。 根卷数量在不同 SD 状态下是否变化。。
uint8_t VfsBrowser::row_count() const
{
    // 判断条件：mode_ == Mode::Roots 是否成立。
    if (mode_ == Mode::Roots)
        // 返回结果：调用 visible_root_count()：返回当前真正可见的根卷数量。例如 SD 卡没有插入、没有挂载成功时，/sd 不应该出现在列表里。所以不能直接返回常量 3，而要根据 Storage 状态决定。。
        return visible_root_count();
    // 返回 static_cast<uint8_t>(entry_count_ + (has_parent_row_ ? 1U : 0U)) 给调用者，表示当前函数的执行结果。
    return static_cast<uint8_t>(entry_count_ + (has_parent_row_ ? 1U : 0U));
}

// 函数定义/声明：调用 VfsBrowser::show_roots()：把浏览器切回根卷模式。它把 mode 设为 Roots，清空当前目录相关状态，并让后续 rowcount() 显示可见根卷。。
void VfsBrowser::show_roots()
{
    // 赋值：把右侧 Mode::Roots 的结果保存到左侧 mode_。
    mode_ = Mode::Roots;
    // 赋值：把右侧 '\0' 的结果保存到左侧 cwd_[0]。
    cwd_[0] = '\0';
    // 赋值：把右侧 '\0' 的结果保存到左侧 browse_title_[0]。
    browse_title_[0] = '\0';
    // 调用 clear_entries()：清空当前条目计数、父目录标记和截断标记。它不释放 entries，因为底层缓冲区可能是复用存储。这个区别很重要：清空的是“有效数据视图”，不是底层内存所有权。。
    clear_entries();
}

// 函数定义/声明：调用 VfsBrowser::clear_entries()：清空当前条目计数、父目录标记和截断标记。它不释放 entries，因为底层缓冲区可能是复用存储。这个区别很重要：清空的是“有效数据视图”，不是底层内存所有权。。
void VfsBrowser::clear_entries()
{
    // 赋值：把右侧 0 的结果保存到左侧 entry_count_。
    entry_count_ = 0;
    // 赋值：把右侧 false 的结果保存到左侧 has_parent_row_。
    has_parent_row_ = false;
    // 赋值：把右侧 false 的结果保存到左侧 truncated_。
    truncated_ = false;
}

// 函数定义/声明：调用 VfsBrowser::sync_parent_row()：根据当前目录决定是否显示“返回上一级”。通常在根卷目录不显示，在子目录显示。。
void VfsBrowser::sync_parent_row()
{
    // 调用 is_storage_root(cwd_)：判断当前路径是否正好等于一个存储根卷。它常用于决定返回键行为：在根卷内部按返回应该回到根卷列表；在某个子目录按返回应该回到父目录。。
    has_parent_row_ = !is_storage_root(cwd_);
}

// 函数定义/声明：调用 VfsBrowser::sync_title_from_cwd()：从当前目录路径生成一个适合显示在工具栏的短标题。因为路径可能很长，标题缓冲区只有 32 字节，所以要避免直接无检查地 strcpy。。
void VfsBrowser::sync_title_from_cwd()
{
    // 判断条件：is_storage_root(cwd_) 是否成立。
    if (is_storage_root(cwd_))
    {
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint8_t i = 0; i < kRootSlotCount; ++i)
        {
            // 判断条件：std::strcmp(cwd_, kRoots[i].path) == 0 是否成立。
            if (std::strcmp(cwd_, kRoots[i].path) == 0)
            {
                // 调用 std::strncpy(browse_title_, kRoots[i].label, sizeof(browse_title_) - 1);，执行这个函数并处理它的返回值或状态变化。
                std::strncpy(browse_title_, kRoots[i].label, sizeof(browse_title_) - 1);
                // 赋值：把右侧 '\0' 的结果保存到左侧 browse_title_[sizeof(browse_title_) - 1]。
                browse_title_[sizeof(browse_title_) - 1] = '\0';
                // 提前返回：结束当前函数，不返回具体值。
                return;
            }
        }
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* name = std::strrchr(cwd_, '/');
    // 赋值：把右侧 (name != nullptr) ? (name + 1) : cwd_ 的结果保存到左侧 name。
    name = (name != nullptr) ? (name + 1) : cwd_;
    // 调用 std::strncpy(browse_title_, name, sizeof(browse_title_) - 1);，执行这个函数并处理它的返回值或状态变化。
    std::strncpy(browse_title_, name, sizeof(browse_title_) - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 browse_title_[sizeof(browse_title_) - 1]。
    browse_title_[sizeof(browse_title_) - 1] = '\0';
}

// 定义或声明函数 VfsBrowser::scan_entries：参数在括号里，返回类型写在函数名前面。
bool VfsBrowser::scan_entries(const char* dir, Entry* out, uint8_t& out_count, bool& truncated_out)
{
    // 后台任务在这里做阻塞扫描；结果最多 48 条，目录排在文件前，名称升序。
    out_count = 0;
    // 赋值：把右侧 false 的结果保存到左侧 truncated_out。
    truncated_out = false;
    // 判断条件：dir == nullptr || out == nullptr 是否成立。
    if (dir == nullptr || out == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // struct stat：把相关数据字段组合在一起，表示项目中的一个概念。
    struct stat dst{};
    // 判断条件：stat(dir, &dst) != 0 || !S_ISDIR(dst.st_mode) 是否成立。
    if (stat(dir, &dst) != 0 || !S_ISDIR(dst.st_mode))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 ::opendir(dir) 的结果保存到左侧 DIR* d。
    DIR* d = ::opendir(dir);
    // 判断条件：d == nullptr 是否成立。
    if (d == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // struct dirent：把相关数据字段组合在一起，表示项目中的一个概念。
    struct dirent* ent = nullptr;
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while ((ent = ::readdir(d)) != nullptr && out_count < kMaxEntries)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* name = ent->d_name;
        // 判断条件：name == nullptr || name[0] == '\0' || std::strcmp(name, "." 是否成立。
        if (name == nullptr || name[0] == '\0' || std::strcmp(name, ".") == 0 ||
            // 定义或声明函数 std::strcmp：参数在括号里，返回类型写在函数名前面。
            std::strcmp(name, "..") == 0 || entry_hidden(dir, name))
            // 继续下一轮：跳过本次循环剩余代码。
            continue;

        // 执行语句：char full[kPathCap];；它会更新一次变量、对象或程序状态。
        char full[kPathCap];
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int n = std::snprintf(full, sizeof(full), "%s/%s", dir, name);
        // 判断条件：n <= 0 || static_cast<size_t>(n) >= sizeof(full) 是否成立。
        if (n <= 0 || static_cast<size_t>(n) >= sizeof(full))
            // 继续下一轮：跳过本次循环剩余代码。
            continue;

        // 定义变量 bool is_dir 并初始化为 false。
        bool is_dir = false;
        // 定义变量 uint32_t size 并初始化为 0。
        uint32_t size = 0;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const unsigned char dtype = static_cast<unsigned char>(ent->d_type);

        // 判断条件：dtype == DT_DIR 是否成立。
        if (dtype == DT_DIR)
        {
            // 赋值：把右侧 true 的结果保存到左侧 is_dir。
            is_dir = true;
        }
        // 备用分支：前面的条件不成立时执行这里。
        else if (dtype == DT_REG)
        {
            // struct stat：把相关数据字段组合在一起，表示项目中的一个概念。
            struct stat st{};
            // 判断条件：::stat(full, &st) != 0 || !S_ISREG(st.st_mode) 是否成立。
            if (::stat(full, &st) != 0 || !S_ISREG(st.st_mode))
                // 继续下一轮：跳过本次循环剩余代码。
                continue;
            // 赋值：把右侧 static_cast<uint32_t>(st.st_size) 的结果保存到左侧 size。
            size = static_cast<uint32_t>(st.st_size);
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // struct stat：把相关数据字段组合在一起，表示项目中的一个概念。
            struct stat st{};
            // 判断条件：::stat(full, &st) != 0 是否成立。
            if (::stat(full, &st) != 0)
                // 继续下一轮：跳过本次循环剩余代码。
                continue;
            // 赋值：把右侧 S_ISDIR(st.st_mode) 的结果保存到左侧 is_dir。
            is_dir = S_ISDIR(st.st_mode);
            // 判断条件：!is_dir && !S_ISREG(st.st_mode) 是否成立。
            if (!is_dir && !S_ISREG(st.st_mode))
                // 继续下一轮：跳过本次循环剩余代码。
                continue;
            // 赋值：把右侧 is_dir ? 0U : static_cast<uint32_t>(st.st_size) 的结果保存到左侧 size。
            size = is_dir ? 0U : static_cast<uint32_t>(st.st_size);
        }

        // 赋值：把右侧 out[out_count] 的结果保存到左侧 Entry& e。
        Entry& e = out[out_count];
        // 调用 std::strncpy(e.name, name, sizeof(e.name) - 1);，执行这个函数并处理它的返回值或状态变化。
        std::strncpy(e.name, name, sizeof(e.name) - 1);
        // 赋值：把右侧 '\0' 的结果保存到左侧 e.name[sizeof(e.name) - 1]。
        e.name[sizeof(e.name) - 1] = '\0';
        // 赋值：把右侧 is_dir 的结果保存到左侧 e.is_dir。
        e.is_dir = is_dir;
        // 赋值：把右侧 size 的结果保存到左侧 e.size_bytes。
        e.size_bytes = size;
        // 执行语句：++out_count;；它会更新一次变量、对象或程序状态。
        ++out_count;
    }

    // 判断条件：ent != nullptr 是否成立。
    if (ent != nullptr)
        // 赋值：把右侧 true 的结果保存到左侧 truncated_out。
        truncated_out = true;

    // 函数调用：::closedir(d);；执行该函数并处理它的返回值或副作用。
    ::closedir(d);

    // 判断条件：out_count > 1 是否成立。
    if (out_count > 1)
        // 调用 sort_entries(out, out_count);，执行这个函数并处理它的返回值或状态变化。
        sort_entries(out, out_count);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数调用：void VfsBrowser::adopt_scan(const Entry* entries, uint8_t count, bool truncated,；执行该函数并处理它的返回值或副作用。
void VfsBrowser::adopt_scan(const Entry* entries, uint8_t count, bool truncated,
                            // 继续上一行的表达式或参数列表：bool has_parent_row)。
                            bool has_parent_row)
{
    // 把后台扫描结果交给 UI 侧浏览器，这是跨线程结果的落地步骤。
    clear_entries();
    // 判断条件：!ensure_entries() 是否成立。
    if (!ensure_entries())
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：entries != nullptr && count > 0 是否成立。
    if (entries != nullptr && count > 0)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t n = (count > kMaxEntries) ? kMaxEntries : count;
        // 调用 std::memcpy(entries_, entries, sizeof(Entry) * n);，执行这个函数并处理它的返回值或状态变化。
        std::memcpy(entries_, entries, sizeof(Entry) * n);
        // 赋值：把右侧 n 的结果保存到左侧 entry_count_。
        entry_count_ = n;
    }
    // 赋值：把右侧 truncated 的结果保存到左侧 truncated_。
    truncated_ = truncated;
    // 赋值：把右侧 has_parent_row 的结果保存到左侧 has_parent_row_。
    has_parent_row_ = has_parent_row;
}

// 函数定义/声明：调用 VfsBrowser::open_root()：进入一个根卷： 1. 检查路径是否允许。 2. 复制挂载路径到 cwd。 3. 设置界面标题。 4. 切换到 Browse。 5. 准备父目录行和目录条目状态。。
bool VfsBrowser::open_root(const char* mount_path, const char* vol_label)
{
    // 判断条件：mount_path == nullptr || vol_label == nullptr || !path_allowed(mount_path) 是否成立。
    if (mount_path == nullptr || vol_label == nullptr || !path_allowed(mount_path))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 ::app::common::storage::StorageMgr::get_instance() 的结果保存到左侧 auto& sto。
    auto& sto = ::app::common::storage::StorageMgr::get_instance();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const Mount kind = ::app::common::storage::mount_kind_from_path_prefix(mount_path);
    // 判断条件：kind == Mount::Invalid || !sto.is_mounted(kind) 是否成立。
    if (kind == Mount::Invalid || !sto.is_mounted(kind))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 Mode::Browse 的结果保存到左侧 mode_。
    mode_ = Mode::Browse;
    // 调用 std::strncpy(cwd_, mount_path, sizeof(cwd_) - 1);，执行这个函数并处理它的返回值或状态变化。
    std::strncpy(cwd_, mount_path, sizeof(cwd_) - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 cwd_[sizeof(cwd_) - 1]。
    cwd_[sizeof(cwd_) - 1] = '\0';
    // 调用 std::strncpy(browse_title_, vol_label, sizeof(browse_title_) - 1);，执行这个函数并处理它的返回值或状态变化。
    std::strncpy(browse_title_, vol_label, sizeof(browse_title_) - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 browse_title_[sizeof(browse_title_) - 1]。
    browse_title_[sizeof(browse_title_) - 1] = '\0';
    // 调用 clear_entries()：清空当前条目计数、父目录标记和截断标记。它不释放 entries，因为底层缓冲区可能是复用存储。这个区别很重要：清空的是“有效数据视图”，不是底层内存所有权。。
    clear_entries();
    // 调用 sync_parent_row()：根据当前目录决定是否显示“返回上一级”。通常在根卷目录不显示，在子目录显示。。
    sync_parent_row();
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数定义/声明：调用 VfsBrowser::reload()：重新加载当前模式： 在根卷模式时重新计算可见根卷。 在浏览模式时重新扫描当前目录。。
void VfsBrowser::reload()
{
    // 调用 clear_entries()：清空当前条目计数、父目录标记和截断标记。它不释放 entries，因为底层缓冲区可能是复用存储。这个区别很重要：清空的是“有效数据视图”，不是底层内存所有权。。
    clear_entries();
    // 判断条件：mode_ != Mode::Browse || cwd_[0] == '\0' 是否成立。
    if (mode_ != Mode::Browse || cwd_[0] == '\0')
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：!ensure_entries() 是否成立。
    if (!ensure_entries())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 ::app::common::storage::StorageMgr::get_instance() 的结果保存到左侧 auto& sto。
    auto& sto = ::app::common::storage::StorageMgr::get_instance();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const Mount kind = ::app::common::storage::mount_kind_from_path_prefix(cwd_);
    // 判断条件：kind != Mount::Invalid && !sto.is_mounted(kind) 是否成立。
    if (kind != Mount::Invalid && !sto.is_mounted(kind))
    {
        // 调用 EBOOK_LOGW(TAG, "volume unmounted: %s", cwd_);，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "volume unmounted: %s", cwd_);
        // 调用 show_roots()：把浏览器切回根卷模式。它把 mode 设为 Roots，清空当前目录相关状态，并让后续 rowcount() 显示可见根卷。。
        show_roots();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 定义变量 bool truncated 并初始化为 false。
    bool truncated = false;
    // 判断条件：!scan_entries(cwd_, entries_, entry_count_, truncated) 是否成立。
    if (!scan_entries(cwd_, entries_, entry_count_, truncated))
    {
        // 调用 EBOOK_LOGW(TAG, "scan failed: %s", cwd_);，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "scan failed: %s", cwd_);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 赋值：把右侧 truncated 的结果保存到左侧 truncated_。
    truncated_ = truncated;
    // 调用 sync_parent_row()：根据当前目录决定是否显示“返回上一级”。通常在根卷目录不显示，在子目录显示。。
    sync_parent_row();
}

// 函数定义/声明：调用 VfsBrowser::enter_dir()：尝试进入指定条目。只有满足这些条件才成功： 下标有效。 当前是浏览模式。 条目确实是目录。 新路径仍然在允许范围内。。
bool VfsBrowser::enter_dir(uint8_t entry_idx)
{
    // 判断条件：entries_ == nullptr || mode_ != Mode::Browse || entry_idx >= entry_count_ 是否成立。
    if (entries_ == nullptr || mode_ != Mode::Browse || entry_idx >= entry_count_)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const Entry& e = entries_[entry_idx];
    // 判断条件：!e.is_dir 是否成立。
    if (!e.is_dir)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 执行语句：char next[kPathCap];；它会更新一次变量、对象或程序状态。
    char next[kPathCap];
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int n = std::snprintf(next, sizeof(next), "%s/%s", cwd_, e.name);
    // 判断条件：n <= 0 || static_cast<size_t>(n) >= sizeof(next) || !path_allowed(next) 是否成立。
    if (n <= 0 || static_cast<size_t>(n) >= sizeof(next) || !path_allowed(next))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 调用 std::strncpy(cwd_, next, sizeof(cwd_) - 1);，执行这个函数并处理它的返回值或状态变化。
    std::strncpy(cwd_, next, sizeof(cwd_) - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 cwd_[sizeof(cwd_) - 1]。
    cwd_[sizeof(cwd_) - 1] = '\0';
    // 调用 std::strncpy(browse_title_, e.name, sizeof(browse_title_) - 1);，执行这个函数并处理它的返回值或状态变化。
    std::strncpy(browse_title_, e.name, sizeof(browse_title_) - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 browse_title_[sizeof(browse_title_) - 1]。
    browse_title_[sizeof(browse_title_) - 1] = '\0';
    // 调用 clear_entries()：清空当前条目计数、父目录标记和截断标记。它不释放 entries，因为底层缓冲区可能是复用存储。这个区别很重要：清空的是“有效数据视图”，不是底层内存所有权。。
    clear_entries();
    // 调用 sync_parent_row()：根据当前目录决定是否显示“返回上一级”。通常在根卷目录不显示，在子目录显示。。
    sync_parent_row();
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数定义/声明：调用 VfsBrowser::entry_path()：把“当前目录 + 条目名称”拼成一个完整路径。它只对普通文件返回成功；目录路径通常由 enterdir() 自己处理。。
bool VfsBrowser::entry_path(uint8_t entry_idx, char* out, size_t cap) const
{
    // 判断条件：entries_ == nullptr || out == nullptr || cap == 0 || entry_idx >= entry_count_ 是否成立。
    if (entries_ == nullptr || out == nullptr || cap == 0 || entry_idx >= entry_count_)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const Entry& e = entries_[entry_idx];
    // 判断条件：e.name[0] == '\0' || e.is_dir 是否成立。
    if (e.name[0] == '\0' || e.is_dir)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int n = std::snprintf(out, cap, "%s/%s", cwd_, e.name);
    // 返回 n > 0 && static_cast<size_t>(n) < cap 给调用者，表示当前函数的执行结果。
    return n > 0 && static_cast<size_t>(n) < cap;
}

// 函数定义/声明：调用 VfsBrowser::delete_file()：VfsBrowser::deletefile()。
bool VfsBrowser::delete_file(uint8_t entry_idx)
{
    // 删除前再次检查路径、目录和系统文件保护，最后调用 VFS remove()。
    if (entries_ == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 执行语句：char path[kPathCap];；它会更新一次变量、对象或程序状态。
    char path[kPathCap];
    // 判断条件：!entry_path(entry_idx, path, sizeof(path)) 是否成立。
    if (!entry_path(entry_idx, path, sizeof(path)))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 判断条件：::app::common::storage::path_is_system(path) 是否成立。
    if (::app::common::storage::path_is_system(path))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 返回结果：调用 remove(path)：VFS remove()。
    return ::remove(path) == 0;
}

// 函数定义/声明：调用 VfsBrowser::go_up()：返回上一级目录： 1. 如果当前是子目录，把 cwd 缩短到父目录。 2. 如果在根卷目录，回到 Roots。 3. 更新标题、父目录行和条目视图。。
bool VfsBrowser::go_up()
{
    // 判断条件：mode_ != Mode::Browse 是否成立。
    if (mode_ != Mode::Browse)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 判断条件：is_storage_root(cwd_) 是否成立。
    if (is_storage_root(cwd_))
    {
        // 调用 show_roots()：把浏览器切回根卷模式。它把 mode 设为 Roots，清空当前目录相关状态，并让后续 rowcount() 显示可见根卷。。
        show_roots();
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 赋值：把右侧 std::strrchr(cwd_, '/') 的结果保存到左侧 char* slash。
    char* slash = std::strrchr(cwd_, '/');
    // 判断条件：slash == nullptr || slash == cwd_ 是否成立。
    if (slash == nullptr || slash == cwd_)
    {
        // 调用 show_roots()：把浏览器切回根卷模式。它把 mode 设为 Roots，清空当前目录相关状态，并让后续 rowcount() 显示可见根卷。。
        show_roots();
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    *slash = '\0';
    // 判断条件：!path_allowed(cwd_) 是否成立。
    if (!path_allowed(cwd_))
    {
        // 调用 show_roots()：把浏览器切回根卷模式。它把 mode 设为 Roots，清空当前目录相关状态，并让后续 rowcount() 显示可见根卷。。
        show_roots();
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 调用 sync_title_from_cwd()：从当前目录路径生成一个适合显示在工具栏的短标题。因为路径可能很长，标题缓冲区只有 32 字节，所以要避免直接无检查地 strcpy。。
    sync_title_from_cwd();
    // 调用 clear_entries()：清空当前条目计数、父目录标记和截断标记。它不释放 entries，因为底层缓冲区可能是复用存储。这个区别很重要：清空的是“有效数据视图”，不是底层内存所有权。。
    clear_entries();
    // 调用 sync_parent_row()：根据当前目录决定是否显示“返回上一级”。通常在根卷目录不显示，在子目录显示。。
    sync_parent_row();
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::files
