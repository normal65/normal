// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/files/vfs_browser.hpp
 * 作用：实现受控目录扫描、排序、路径校验和文件删除。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "storage/storage.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::files {

/** 受控路径目录枚举（/int、/sd、/assets） */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class VfsBrowser
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 固定参数 kMaxEntries = 48：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kMaxEntries = 48;
    // 固定参数 kPathCap = 96：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kPathCap = 96;
    // 固定参数 kNameCap = 40：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kNameCap = 40;

    // 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
    enum class Mode : uint8_t
    {
        // 执行这条代码：Roots = 0,。它会更新变量、对象状态或调用链。
        Roots = 0,
        // 继续上一行的表达式或参数列表：Browse,。
        Browse,
    };

    // struct Entry：把相关数据字段组合在一起，表示项目中的一个概念。
    struct Entry
    {
        // 执行语句：char name[kNameCap]{};；它会更新一次变量、对象或程序状态。
        char name[kNameCap]{};
        // 执行语句：bool is_dir{false};；它会更新一次变量、对象或程序状态。
        bool is_dir{false};
        // 执行语句：uint32_t size_bytes{0};；它会更新一次变量、对象或程序状态。
        uint32_t size_bytes{0};
    };

    // struct RootVol：把相关数据字段组合在一起，表示项目中的一个概念。
    struct RootVol
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* path;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* label;
        // 执行语句：::app::common::storage::MountKind kind;；它会更新一次变量、对象或程序状态。
        ::app::common::storage::MountKind kind;
    };

    // 静态函数 path_allowed：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool path_allowed(const char* path);
    // 静态函数 is_storage_root：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool is_storage_root(const char* path);
    // 静态函数 visible_root_count：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static uint8_t visible_root_count();
    // 静态函数 root_at_visible：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static const RootVol* root_at_visible(uint8_t vis_idx);

    // 定义或声明函数 mode：参数在括号里，返回类型写在函数名前面。
    Mode mode() const
    {
        // 返回 mode_ 给调用者，表示当前函数的执行结果。
        return mode_;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* cwd() const
    {
        // 返回 cwd_ 给调用者，表示当前函数的执行结果。
        return cwd_;
    }
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* browse_title() const
    {
        // 返回 browse_title_ 给调用者，表示当前函数的执行结果。
        return browse_title_;
    }

    // 定义或声明函数 has_parent_row：参数在括号里，返回类型写在函数名前面。
    bool has_parent_row() const
    {
        // 返回 has_parent_row_ 给调用者，表示当前函数的执行结果。
        return has_parent_row_;
    }
    // 定义或声明函数 dir_entry_count：参数在括号里，返回类型写在函数名前面。
    uint8_t dir_entry_count() const
    {
        // 返回 entry_count_ 给调用者，表示当前函数的执行结果。
        return entry_count_;
    }
    // 定义或声明函数 truncated：参数在括号里，返回类型写在函数名前面。
    bool truncated() const
    {
        // 返回 truncated_ 给调用者，表示当前函数的执行结果。
        return truncated_;
    }

    // 调用 row_count()：返回列表应该显示多少行。它不是简单返回 entrycount，还要考虑： 是否显示“返回上一级”行。 当前是 Roots 还是 Browse。 根卷数量在不同 SD 状态下是否变化。。
    uint8_t row_count() const;

    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const Entry& entry_at(uint8_t idx) const;

    // 调用 show_roots()：把浏览器切回根卷模式。它把 mode 设为 Roots，清空当前目录相关状态，并让后续 rowcount() 显示可见根卷。。
    void show_roots();
    // 调用 open_root(constchar*mount_path,constchar*vol_label)：进入一个根卷： 1. 检查路径是否允许。 2. 复制挂载路径到 cwd。 3. 设置界面标题。 4. 切换到 Browse。 5. 准备父目录行和目录条目状态。。
    bool open_root(const char* mount_path, const char* vol_label);
    // 调用 enter_dir(uint8_tentry_idx)：尝试进入指定条目。只有满足这些条件才成功： 下标有效。 当前是浏览模式。 条目确实是目录。 新路径仍然在允许范围内。。
    bool enter_dir(uint8_t entry_idx);
    // 调用 go_up()：返回上一级目录： 1. 如果当前是子目录，把 cwd 缩短到父目录。 2. 如果在根卷目录，回到 Roots。 3. 更新标题、父目录行和条目视图。。
    bool go_up();
    // 调用 reload()：重新加载当前模式： 在根卷模式时重新计算可见根卷。 在浏览模式时重新扫描当前目录。。
    void reload();

    // 静态函数 scan_entries：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool scan_entries(const char* dir, Entry* out, uint8_t& out_count, bool& truncated_out);
    // 调用 adopt_scan(constEntry*entries,uint8_tcount,booltruncated,boolhas_parent_row)：把后台扫描好的结果转交给 UI 侧的 VfsBrowser。它不执行 readdir，只复制或接收已经完成的数组，并更新计数、截断标记和父目录标记。。
    void adopt_scan(const Entry* entries, uint8_t count, bool truncated, bool has_parent_row);

    // 调用 entry_path(uint8_tentry_idx,char*out,size_tcap)：把“当前目录 + 条目名称”拼成一个完整路径。它只对普通文件返回成功；目录路径通常由 enterdir() 自己处理。。
    bool entry_path(uint8_t entry_idx, char* out, size_t cap) const;
    // 调用 delete_file(uint8_tentry_idx)：删除普通文件，拒绝这些情况： 目录。 系统文件。 路径不合法。 entrypath() 失败。。
    bool delete_file(uint8_t entry_idx);

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 clear_entries()：清空当前条目计数、父目录标记和截断标记。它不释放 entries，因为底层缓冲区可能是复用存储。这个区别很重要：清空的是“有效数据视图”，不是底层内存所有权。。
    void clear_entries();
    // 调用 sync_parent_row()：根据当前目录决定是否显示“返回上一级”。通常在根卷目录不显示，在子目录显示。。
    void sync_parent_row();
    // 调用 sync_title_from_cwd()：从当前目录路径生成一个适合显示在工具栏的短标题。因为路径可能很长，标题缓冲区只有 32 字节，所以要避免直接无检查地 strcpy。。
    void sync_title_from_cwd();
    // 静态函数 entry_hidden：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool entry_hidden(const char* cwd, const char* name);

    // 执行语句：Mode mode_{Mode::Roots};；它会更新一次变量、对象或程序状态。
    Mode mode_{Mode::Roots};
    // 执行语句：char cwd_[kPathCap]{};；它会更新一次变量、对象或程序状态。
    char cwd_[kPathCap]{};
    // 执行语句：char browse_title_[32]{};；它会更新一次变量、对象或程序状态。
    char browse_title_[32]{};
    // 执行语句：Entry* entries_{nullptr};；它会更新一次变量、对象或程序状态。
    Entry* entries_{nullptr};
    // 执行语句：uint8_t entry_count_{0};；它会更新一次变量、对象或程序状态。
    uint8_t entry_count_{0};
    // 执行语句：bool has_parent_row_{false};；它会更新一次变量、对象或程序状态。
    bool has_parent_row_{false};
    // 执行语句：bool truncated_{false};；它会更新一次变量、对象或程序状态。
    bool truncated_{false};

    // 调用 ensure_entries()：惰性打开当前目录。它只有在 entries、计数或者当前目录状态需要重新扫描时才调用底层 opendir/readdir。这样做的目的是避免每次刷新界面都重复扫盘。。
    bool ensure_entries();
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::files
