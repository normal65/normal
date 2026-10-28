/**
 * [初学者导读] 文件：main/common/uuid/uuid.cc
 * 作用：生成和处理 UUID。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：UUID 只表示唯一标识，不承担认证或安全职责。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "uuid.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_random.h"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::common::uuid {

// 定义或声明函数 generate：参数在括号里，返回类型写在函数名前面。
Uuid generate()
{
    // 执行语句：Uuid uuid;；它会更新一次变量、对象或程序状态。
    Uuid uuid;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (size_t i = 0; i < UUID_BYTE_SIZE; i++)
    {
        // 赋值：把右侧 static_cast<uint8_t>(esp_random() & 0xFF) 的结果保存到左侧 uuid.data[i]。
        uuid.data[i] = static_cast<uint8_t>(esp_random() & 0xFF);
    }

    // 赋值：把右侧 (uuid.data[6] & 0x0F) | 0x40 的结果保存到左侧 uuid.data[6]。
    uuid.data[6] = (uuid.data[6] & 0x0F) | 0x40; // 版本号 4
    // 赋值：把右侧 (uuid.data[8] & 0x3F) | 0x80 的结果保存到左侧 uuid.data[8]。
    uuid.data[8] = (uuid.data[8] & 0x3F) | 0x80; // 变体标识

    // 返回 uuid 给调用者，表示当前函数的执行结果。
    return uuid;
}

// 定义或声明函数 to_string：参数在括号里，返回类型写在函数名前面。
bool to_string(const Uuid& uuid, char* buffer, size_t buffer_size)
{
    // 判断条件：buffer == nullptr || buffer_size < UUID_STRING_SIZE 是否成立。
    if (buffer == nullptr || buffer_size < UUID_STRING_SIZE)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 固定参数 UUID_STRING_LENGTH = 36：由当前模块定义，用于控制布局、时序或硬件行为。
    constexpr size_t UUID_STRING_LENGTH = 36; // 不包括 null 终止符
    // 定义变量 int result 并初始化为 snprintf。
    int result = snprintf(
        // 继续上一行的表达式或参数列表：buffer, buffer_size, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",。
        buffer, buffer_size, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        // 继续上一行的表达式或参数列表：uuid.data[0], uuid.data[1], uuid.data[2], uuid.data[3], uuid.data[4], uuid.data[5],。
        uuid.data[0], uuid.data[1], uuid.data[2], uuid.data[3], uuid.data[4], uuid.data[5],
        // 继续上一行的表达式或参数列表：uuid.data[6], uuid.data[7], uuid.data[8], uuid.data[9], uuid.data[10], uuid.data[11],。
        uuid.data[6], uuid.data[7], uuid.data[8], uuid.data[9], uuid.data[10], uuid.data[11],
        // 继续上一行的表达式或参数列表：uuid.data[12], uuid.data[13], uuid.data[14], uuid.data[15]);。
        uuid.data[12], uuid.data[13], uuid.data[14], uuid.data[15]);

    // 返回 result == UUID_STRING_LENGTH 给调用者，表示当前函数的执行结果。
    return result == UUID_STRING_LENGTH;
}

// 定义或声明函数 from_string：参数在括号里，返回类型写在函数名前面。
bool from_string(const char* str, Uuid& uuid)
{
    // 判断条件：str == nullptr 是否成立。
    if (str == nullptr)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 执行语句：unsigned int bytes[UUID_BYTE_SIZE];；它会更新一次变量、对象或程序状态。
    unsigned int bytes[UUID_BYTE_SIZE];
    // 定义变量：int result =
    int result =
        // 继续上一行的表达式或参数列表：sscanf(str, "%2x%2x%2x%2x-%2x%2x-%2x%2x-%2x%2x-%2x%2x%2x%2x%2x%2x", &bytes[0], &bytes[1],。
        sscanf(str, "%2x%2x%2x%2x-%2x%2x-%2x%2x-%2x%2x-%2x%2x%2x%2x%2x%2x", &bytes[0], &bytes[1],
               // 继续上一行的表达式或参数列表：&bytes[2], &bytes[3], &bytes[4], &bytes[5], &bytes[6], &bytes[7], &bytes[8],。
               &bytes[2], &bytes[3], &bytes[4], &bytes[5], &bytes[6], &bytes[7], &bytes[8],
               // 继续上一行的表达式或参数列表：&bytes[9], &bytes[10], &bytes[11], &bytes[12], &bytes[13], &bytes[14], &bytes[15]);。
               &bytes[9], &bytes[10], &bytes[11], &bytes[12], &bytes[13], &bytes[14], &bytes[15]);

    // 判断条件：result != static_cast<int>(UUID_BYTE_SIZE) 是否成立。
    if (result != static_cast<int>(UUID_BYTE_SIZE))
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (size_t i = 0; i < UUID_BYTE_SIZE; i++)
    {
        // 赋值：把右侧 static_cast<uint8_t>(bytes[i]) 的结果保存到左侧 uuid.data[i]。
        uuid.data[i] = static_cast<uint8_t>(bytes[i]);
    }

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 is_empty：参数在括号里，返回类型写在函数名前面。
bool is_empty(const Uuid& uuid)
{
    // 判断条件：reinterpret_cast<uintptr_t>(uuid.data) % 8 == 0 是否成立。
    if (reinterpret_cast<uintptr_t>(uuid.data) % 8 == 0)
    {
        // 固定参数 zero64 = 0：由当前模块定义，用于控制布局、时序或硬件行为。
        constexpr uint64_t zero64 = 0;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint64_t* data64 = reinterpret_cast<const uint64_t*>(uuid.data);
        // 返回 data64[0] == zero64 && data64[1] == zero64 给调用者，表示当前函数的执行结果。
        return data64[0] == zero64 && data64[1] == zero64;
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (size_t i = 0; i < UUID_BYTE_SIZE; i++)
        {
            // 判断条件：uuid.data[i] != 0 是否成立。
            if (uuid.data[i] != 0)
            {
                // 返回 false 给调用者，表示当前函数的执行结果。
                return false;
            }
        }
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::common::uuid
