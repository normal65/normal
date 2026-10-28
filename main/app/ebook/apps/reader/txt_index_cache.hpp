// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/reader/txt_index_cache.hpp
 * 作用：建立和缓存 TXT 分页索引。
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

/**
 * @brief TXT 分页索引磁盘缓存
 *
 * 路径：{mount}/.ebook/cache/txt_{fnv1a32(path):08x}/
 *   - index.bin    分页偏移（"TXTI" v4）
 *   - progress.bin 阅读进度（"TXTP" v1）
 */
// struct TxtIndexLayout：把相关数据字段组合在一起，表示项目中的一个概念。
struct TxtIndexLayout
{
    // 执行语句：uint16_t viewport_w{0};；它会更新一次变量、对象或程序状态。
    uint16_t viewport_w{0};
    // 执行语句：uint16_t viewport_h{0};；它会更新一次变量、对象或程序状态。
    uint16_t viewport_h{0};
    // 执行语句：uint8_t lines_per_page{0};；它会更新一次变量、对象或程序状态。
    uint8_t lines_per_page{0};
    // 执行语句：uint8_t font_size{0};；它会更新一次变量、对象或程序状态。
    uint8_t font_size{0};
    // 执行语句：uint16_t line_height{0};；它会更新一次变量、对象或程序状态。
    uint16_t line_height{0};
    // 执行语句：uint16_t screen_w{0};；它会更新一次变量、对象或程序状态。
    uint16_t screen_w{0};
    // 执行语句：uint16_t screen_h{0};；它会更新一次变量、对象或程序状态。
    uint16_t screen_h{0};
    // 执行语句：uint32_t file_size{0};；它会更新一次变量、对象或程序状态。
    uint32_t file_size{0};
    // 执行语句：uint8_t text_encoding{0};；它会更新一次变量、对象或程序状态。
    uint8_t text_encoding{0};
};

// 类定义：把数据和操作这些数据的方法绑定在一起。
class TxtIndexCache
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 固定参数 kMaxPages = 8000：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kMaxPages = 8000;

    // 静态函数 make_paths：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool make_paths(const char* book_path, char* dir_out, size_t dir_len, char* index_out,
                           // 继续上一行的表达式或参数列表：size_t index_len);。
                           size_t index_len);

    // 静态函数 ensure_dir：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool ensure_dir(const char* dir_path);

    // 静态函数 load：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool load(const char* book_path, const TxtIndexLayout& layout, uint32_t* offsets,
                     // 继续上一行的表达式或参数列表：uint16_t* total_pages);。
                     uint16_t* total_pages);

    // 静态函数 save：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool save(const char* book_path, const TxtIndexLayout& layout, const uint32_t* offsets,
                     // 继续上一行的表达式或参数列表：uint16_t total_pages);。
                     uint16_t total_pages);

    // 静态函数 load_progress：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool load_progress(const char* book_path, uint16_t* current_page);
    // 静态函数 save_progress：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool save_progress(const char* book_path, uint16_t current_page, uint16_t total_pages);

    // 静态函数 query_read_percent：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool query_read_percent(const char* book_path, uint8_t* percent_out);
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::reader
