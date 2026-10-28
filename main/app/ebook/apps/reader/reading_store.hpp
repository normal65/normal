// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/reader/reading_store.hpp
 * 作用：保存阅读进度、最近阅读和阅读器偏好。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::reader {

/**
 * @brief 阅读进度（NVS: bk.*；每书页码见 TxtIndexCache::progress.bin）
 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class ReadingStore
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static ReadingStore& get_instance();

    // 函数调用：void load();；执行该函数并处理它的返回值或副作用。
    void load();

    // 定义或声明函数 has_book：参数在括号里，返回类型写在函数名前面。
    bool has_book() const
    {
        // 返回 has_book_ 给调用者，表示当前函数的执行结果。
        return has_book_;
    }
    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const char* title() const
    {
        // 返回 title_ 给调用者，表示当前函数的执行结果。
        return title_;
    }
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* book_path() const
    {
        // 返回 path_ 给调用者，表示当前函数的执行结果。
        return path_;
    }
    // 定义或声明函数 current_page：参数在括号里，返回类型写在函数名前面。
    uint16_t current_page() const
    {
        // 返回 current_page_ 给调用者，表示当前函数的执行结果。
        return current_page_;
    }
    // 定义或声明函数 total_pages：参数在括号里，返回类型写在函数名前面。
    uint16_t total_pages() const
    {
        // 返回 total_pages_ 给调用者，表示当前函数的执行结果。
        return total_pages_;
    }

    /** 已读百分比 1..100 */
    // 定义或声明函数 percent：参数在括号里，返回类型写在函数名前面。
    uint8_t percent() const
    {
        // 判断条件：!has_book_ || total_pages_ == 0 是否成立。
        if (!has_book_ || total_pages_ == 0)
            // 返回 0 给调用者，表示当前函数的执行结果。
            return 0;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint32_t pct = (static_cast<uint32_t>(current_page_ + 1U) * 100U) /
                             // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
                             static_cast<uint32_t>(total_pages_);
        // 返回 static_cast<uint8_t>(pct > 100U ? 100U : pct) 给调用者，表示当前函数的执行结果。
        return static_cast<uint8_t>(pct > 100U ? 100U : pct);
    }

    // 函数调用：void set_book(const char* title, const char* path, uint16_t current, uint16_t total);；执行该函数并处理它的返回值或副作用。
    void set_book(const char* title, const char* path, uint16_t current, uint16_t total);
    // 调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    void clear();

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 ReadingStore() = default;，执行这个函数并处理它的返回值或状态变化。
    ReadingStore() = default;

    // 固定参数 kTitleMax = 48：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr size_t kTitleMax = 48;
    // 固定参数 kPathMax = 96：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr size_t kPathMax = 96;

    // 执行语句：bool has_book_{false};；它会更新一次变量、对象或程序状态。
    bool has_book_{false};
    // 执行语句：char title_[kTitleMax + 1]{};；它会更新一次变量、对象或程序状态。
    char title_[kTitleMax + 1]{};
    // 执行语句：char path_[kPathMax + 1]{};；它会更新一次变量、对象或程序状态。
    char path_[kPathMax + 1]{};
    // 执行语句：uint16_t current_page_{0};；它会更新一次变量、对象或程序状态。
    uint16_t current_page_{0};
    // 执行语句：uint16_t total_pages_{0};；它会更新一次变量、对象或程序状态。
    uint16_t total_pages_{0};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::reader
