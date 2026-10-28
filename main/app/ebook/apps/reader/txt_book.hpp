// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/reader/txt_book.hpp
 * 作用：读取和解析 TXT 文本，处理编码、分页和内容缓存。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/reader_layout.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/txt_index_cache.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/canvas.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::reader {

/** @brief 单本 TXT：加载 → 分页（可缓存）→ 按页绘制 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class TxtBook
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 固定参数 kMaxFileBytes = 384U * 1024U：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint32_t kMaxFileBytes = 384U * 1024U;
    // 固定参数 kMaxPages = static_cast<uint16_t>(TxtIndexCache::kMaxPages)：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kMaxPages = static_cast<uint16_t>(TxtIndexCache::kMaxPages);

    // 函数调用：bool open(const char* path, const ReaderLayout& layout);；执行该函数并处理它的返回值或副作用。
    bool open(const char* path, const ReaderLayout& layout);
    // 调用 close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
    void close();

    // 函数调用：bool set_layout(const ReaderLayout& layout);；执行该函数并处理它的返回值或副作用。
    bool set_layout(const ReaderLayout& layout);

    // 函数定义/声明：调用 is_open()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
    bool is_open() const
    {
        // 返回 text_ != nullptr 给调用者，表示当前函数的执行结果。
        return text_ != nullptr;
    }
    // 定义或声明函数 page_count：参数在括号里，返回类型写在函数名前面。
    uint16_t page_count() const
    {
        // 返回 page_count_ 给调用者，表示当前函数的执行结果。
        return page_count_;
    }
    // 定义或声明函数 page：参数在括号里，返回类型写在函数名前面。
    uint16_t page() const
    {
        // 返回 page_ 给调用者，表示当前函数的执行结果。
        return page_;
    }
    // 定义或声明函数 text_encoding：参数在括号里，返回类型写在函数名前面。
    uint8_t text_encoding() const
    {
        // 返回 text_encoding_ 给调用者，表示当前函数的执行结果。
        return text_encoding_;
    }
    // 定义或声明函数 file_size：参数在括号里，返回类型写在函数名前面。
    uint32_t file_size() const
    {
        // 返回 file_size_ 给调用者，表示当前函数的执行结果。
        return file_size_;
    }

    // 函数调用：bool set_page(uint16_t p);；执行该函数并处理它的返回值或副作用。
    bool set_page(uint16_t p);
    // 函数调用：bool next_page();；执行该函数并处理它的返回值或副作用。
    bool next_page();
    // 函数调用：bool prev_page();；执行该函数并处理它的返回值或副作用。
    bool prev_page();

    // 函数调用：void paint_page(gfx::Canvas& canvas, const core::Rect& area) const;；执行该函数并处理它的返回值或副作用。
    void paint_page(gfx::Canvas& canvas, const core::Rect& area) const;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 函数调用：bool load_file(const char* path);；执行该函数并处理它的返回值或副作用。
    bool load_file(const char* path);
    // 函数调用：bool build_pages(const ReaderLayout& layout);；执行该函数并处理它的返回值或副作用。
    bool build_pages(const ReaderLayout& layout);
    // 函数调用：bool try_load_cache(const ReaderLayout& layout);；执行该函数并处理它的返回值或副作用。
    bool try_load_cache(const ReaderLayout& layout);
    // 函数调用：void save_cache(const ReaderLayout& layout);；执行该函数并处理它的返回值或副作用。
    void save_cache(const ReaderLayout& layout);
    // 函数调用：uint32_t skip_line(uint32_t offset, const ReaderLayout& layout) const;；执行该函数并处理它的返回值或副作用。
    uint32_t skip_line(uint32_t offset, const ReaderLayout& layout) const;

    // 执行语句：char* text_{nullptr};；它会更新一次变量、对象或程序状态。
    char* text_{nullptr};
    // 执行语句：uint32_t text_len_{0};；它会更新一次变量、对象或程序状态。
    uint32_t text_len_{0};
    // 执行语句：uint32_t* page_offsets_{nullptr};；它会更新一次变量、对象或程序状态。
    uint32_t* page_offsets_{nullptr};
    // 执行语句：uint16_t page_count_{0};；它会更新一次变量、对象或程序状态。
    uint16_t page_count_{0};
    // 执行语句：uint16_t page_{0};；它会更新一次变量、对象或程序状态。
    uint16_t page_{0};
    // 执行语句：uint8_t text_encoding_{0};；它会更新一次变量、对象或程序状态。
    uint8_t text_encoding_{0};
    // 执行语句：uint32_t file_size_{0};；它会更新一次变量、对象或程序状态。
    uint32_t file_size_{0};
    // 执行语句：char path_[96]{};；它会更新一次变量、对象或程序状态。
    char path_[96]{};
    // 执行语句：ReaderLayout layout_{12};；它会更新一次变量、对象或程序状态。
    ReaderLayout layout_{12};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::reader
