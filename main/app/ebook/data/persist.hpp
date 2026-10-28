// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/data/persist.hpp
 * 作用：封装 NVS 的初始化、读写、提交和命名空间管理。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：小配置写 NVS，运行时共享状态放 SystemState。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstddef>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/result.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::data {

/**
 * @brief NVS 键值（namespace "ebook"；UI 任务内调用；set_* 不自动 commit）
 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class Persist
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 init：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Status init();
    // 静态函数 deinit：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void deinit();

    // 静态函数 get_u8：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool get_u8(const char* key, uint8_t& out);
    // 静态函数 set_u8：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool set_u8(const char* key, uint8_t v);

    // 静态函数 get_bool：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool get_bool(const char* key, bool& out);
    // 静态函数 set_bool：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool set_bool(const char* key, bool v);

    // 静态函数 get_str：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool get_str(const char* key, char* out, size_t out_size);
    // 静态函数 set_str：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool set_str(const char* key, const char* v);

    // 静态函数 commit：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void commit();
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::data
