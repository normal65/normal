// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/common/uuid/uuid.hpp
 * 作用：生成和处理 UUID。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：UUID 只表示唯一标识，不承担认证或安全职责。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstddef>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::common::uuid {

// 固定参数 UUID_BYTE_SIZE = 16：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr size_t UUID_BYTE_SIZE = 16;
// 固定参数 UUID_STRING_SIZE = 37：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr size_t UUID_STRING_SIZE = 37;

/** @brief 128 位 UUID */
// struct Uuid：把相关数据字段组合在一起，表示项目中的一个概念。
struct Uuid
{
    // 执行语句：uint8_t data[UUID_BYTE_SIZE];；它会更新一次变量、对象或程序状态。
    uint8_t data[UUID_BYTE_SIZE];

    // 定义变量 bool operator 并初始化为 =(const Uuid& other) cons。
    bool operator==(const Uuid& other) const
    {
        // 返回 memcmp(data, other.data, UUID_BYTE_SIZE) == 0 给调用者，表示当前函数的执行结果。
        return memcmp(data, other.data, UUID_BYTE_SIZE) == 0;
    }

    // 比较表达式：比较左右两边的值，结果会用于条件判断或返回。
    bool operator!=(const Uuid& other) const
    {
        // 返回 !(*this == other) 给调用者，表示当前函数的执行结果。
        return !(*this == other);
    }
};

// 函数调用：Uuid generate();；执行该函数并处理它的返回值或副作用。
Uuid generate();
// 函数调用：bool to_string(const Uuid& uuid, char* buffer, size_t buffer_size);；执行该函数并处理它的返回值或副作用。
bool to_string(const Uuid& uuid, char* buffer, size_t buffer_size);
// 函数调用：bool from_string(const char* str, Uuid& uuid);；执行该函数并处理它的返回值或副作用。
bool from_string(const char* str, Uuid& uuid);
// 函数调用：bool is_empty(const Uuid& uuid);；执行该函数并处理它的返回值或副作用。
bool is_empty(const Uuid& uuid);

// 命名空间结束：离开当前模块作用域。
} // namespace app::common::uuid
