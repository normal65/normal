// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/reader/book_shelf.hpp
 * 作用：扫描书架目录并管理书籍列表。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstddef>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::reader {

/** 书架条目 */
// struct BookItem：把相关数据字段组合在一起，表示项目中的一个概念。
struct BookItem
{
    // 执行语句：char path[96]{};；它会更新一次变量、对象或程序状态。
    char path[96]{};
    // 执行语句：char title[48]{};；它会更新一次变量、对象或程序状态。
    char title[48]{};
    // 执行语句：uint32_t size_bytes{0};；它会更新一次变量、对象或程序状态。
    uint32_t size_bytes{0};
};

/**
 * @brief 书架扫描（仅 /int/Ebook/txt 与 /sd/Ebook/txt，非全盘检索）
 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class BookShelf
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 固定参数 kMaxBooks = 48：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kMaxBooks = 48;
    // 固定参数 kBookDirTxt = "/Ebook/txt"：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr const char* kBookDirTxt = "/Ebook/txt";

    /** 与 ends_with_txt_ci() / scan_mount() 一致，供空状态提示 */
    // 固定参数 kScanFmtHint = "TXT"：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr const char* kScanFmtHint = "TXT";
    // 固定参数 kScanPathInt = "/int/Ebook/txt"：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr const char* kScanPathInt = "/int/Ebook/txt";
    // 固定参数 kScanPathSd = "/sd/Ebook/txt"：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr const char* kScanPathSd = "/sd/Ebook/txt";

    // 调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    void clear();
    // 函数调用：void scan();；执行该函数并处理它的返回值或副作用。
    void scan();

    // 定义或声明函数 count：参数在括号里，返回类型写在函数名前面。
    uint8_t count() const
    {
        // 返回 count_ 给调用者，表示当前函数的执行结果。
        return count_;
    }
    // 只读约束：这个值或对象在当前位置不能被修改。
    const BookItem& item(uint8_t idx) const;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 函数调用：bool scan_mount(const char* base_mount);；执行该函数并处理它的返回值或副作用。
    bool scan_mount(const char* base_mount);
    // 函数调用：bool add_unique(const char* full_path, const char* file_name, uint32_t size_bytes);；执行该函数并处理它的返回值或副作用。
    bool add_unique(const char* full_path, const char* file_name, uint32_t size_bytes);
    // 函数调用：void sort_by_title();；执行该函数并处理它的返回值或副作用。
    void sort_by_title();

    // 执行语句：BookItem* items_{nullptr};；它会更新一次变量、对象或程序状态。
    BookItem* items_{nullptr};
    // 执行语句：uint8_t count_{0};；它会更新一次变量、对象或程序状态。
    uint8_t count_{0};

    // 函数调用：bool ensure_items();；执行该函数并处理它的返回值或副作用。
    bool ensure_items();
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::reader
