// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/core/result.hpp
 * 作用：实现 core 模块中的 result 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：上层通过 Router、UiBus 和 SystemState 协作，不直接操作硬件。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::core {

/** @brief ebook 返回码（与 esp_err_t 不互通，进 app 层前须显式映射） */
// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class Status : uint8_t
{
    // 执行这条代码：Ok = 0,。它会更新变量、对象状态或调用链。
    Ok = 0,
    // 继续上一行的表达式或参数列表：InvalidArg,。
    InvalidArg,
    // 继续上一行的表达式或参数列表：NotInit,。
    NotInit,
    // 继续上一行的表达式或参数列表：AlreadyInit,。
    AlreadyInit,
    // 继续上一行的表达式或参数列表：Busy,。
    Busy,
    // 继续上一行的表达式或参数列表：Timeout,。
    Timeout,
    // 继续上一行的表达式或参数列表：OutOfResource,。
    OutOfResource,
    // 继续上一行的表达式或参数列表：IoError,。
    IoError,
    // 继续上一行的表达式或参数列表：NotSupported,。
    NotSupported,
    // 继续上一行的表达式或参数列表：NotFound,。
    NotFound,
    // 继续上一行的表达式或参数列表：Cancelled,。
    Cancelled,
    // 继续上一行的表达式或参数列表：Internal,。
    Internal,
};

// 编译期函数 ok：常用于固定规则计算，能参与编译期优化。
constexpr bool ok(Status s)
{
    // 返回 s == Status::Ok 给调用者，表示当前函数的执行结果。
    return s == Status::Ok;
}
// 编译期函数 fail：常用于固定规则计算，能参与编译期优化。
constexpr bool fail(Status s)
{
    // 返回 s != Status::Ok 给调用者，表示当前函数的执行结果。
    return s != Status::Ok;
}

// 编译期函数 to_str：常用于固定规则计算，能参与编译期优化。
constexpr const char* to_str(Status s)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (s)
    {
        // 分支匹配：case Status::Ok 成立时执行下面的代码。
        case Status::Ok:
            // 返回 "OK" 给调用者，表示当前函数的执行结果。
            return "OK";
        // 分支匹配：case Status::InvalidArg 成立时执行下面的代码。
        case Status::InvalidArg:
            // 返回 "INVALID_ARG" 给调用者，表示当前函数的执行结果。
            return "INVALID_ARG";
        // 分支匹配：case Status::NotInit 成立时执行下面的代码。
        case Status::NotInit:
            // 返回 "NOT_INIT" 给调用者，表示当前函数的执行结果。
            return "NOT_INIT";
        // 分支匹配：case Status::AlreadyInit 成立时执行下面的代码。
        case Status::AlreadyInit:
            // 返回 "ALREADY_INIT" 给调用者，表示当前函数的执行结果。
            return "ALREADY_INIT";
        // 分支匹配：case Status::Busy 成立时执行下面的代码。
        case Status::Busy:
            // 返回 "BUSY" 给调用者，表示当前函数的执行结果。
            return "BUSY";
        // 分支匹配：case Status::Timeout 成立时执行下面的代码。
        case Status::Timeout:
            // 返回 "TIMEOUT" 给调用者，表示当前函数的执行结果。
            return "TIMEOUT";
        // 分支匹配：case Status::OutOfResource 成立时执行下面的代码。
        case Status::OutOfResource:
            // 返回 "OUT_OF_RESOURCE" 给调用者，表示当前函数的执行结果。
            return "OUT_OF_RESOURCE";
        // 分支匹配：case Status::IoError 成立时执行下面的代码。
        case Status::IoError:
            // 返回 "IO_ERROR" 给调用者，表示当前函数的执行结果。
            return "IO_ERROR";
        // 分支匹配：case Status::NotSupported 成立时执行下面的代码。
        case Status::NotSupported:
            // 返回 "NOT_SUPPORTED" 给调用者，表示当前函数的执行结果。
            return "NOT_SUPPORTED";
        // 分支匹配：case Status::NotFound 成立时执行下面的代码。
        case Status::NotFound:
            // 返回 "NOT_FOUND" 给调用者，表示当前函数的执行结果。
            return "NOT_FOUND";
        // 分支匹配：case Status::Cancelled 成立时执行下面的代码。
        case Status::Cancelled:
            // 返回 "CANCELLED" 给调用者，表示当前函数的执行结果。
            return "CANCELLED";
        // 分支匹配：case Status::Internal 成立时执行下面的代码。
        case Status::Internal:
            // 返回 "INTERNAL" 给调用者，表示当前函数的执行结果。
            return "INTERNAL";
    }
    // 返回 "?" 给调用者，表示当前函数的执行结果。
    return "?";
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::core
