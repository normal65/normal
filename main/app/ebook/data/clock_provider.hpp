// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/data/clock_provider.hpp
 * 作用：提供当前时间和时间字符串。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：小配置写 NVS，运行时共享状态放 SystemState。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstddef>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::data {

/** @brief 时钟字段（包装 common/time，供 widget 复用格式化） */
// struct Clock：把相关数据字段组合在一起，表示项目中的一个概念。
struct Clock
{
    // 执行语句：uint8_t hour{0};；它会更新一次变量、对象或程序状态。
    uint8_t hour{0};
    // 执行语句：uint8_t minute{0};；它会更新一次变量、对象或程序状态。
    uint8_t minute{0};
    // 执行语句：uint8_t second{0};；它会更新一次变量、对象或程序状态。
    uint8_t second{0};
    // 执行语句：uint16_t year{0};；它会更新一次变量、对象或程序状态。
    uint16_t year{0};
    // 执行语句：uint8_t month{0};；它会更新一次变量、对象或程序状态。
    uint8_t month{0};
    // 执行语句：uint8_t day{0};；它会更新一次变量、对象或程序状态。
    uint8_t day{0};
    // 执行语句：uint8_t weekday{0};；它会更新一次变量、对象或程序状态。
    uint8_t weekday{0}; ///< 0=Sunday
    // 执行语句：uint16_t day_of_year{0};；它会更新一次变量、对象或程序状态。
    uint16_t day_of_year{0};

    // 静态函数 now：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static Clock now();

    // 函数调用：void format_time_hm(char* buf, size_t buf_size) const;；执行该函数并处理它的返回值或副作用。
    void format_time_hm(char* buf, size_t buf_size) const;
    // 函数调用：void format_date_cn(char* buf, size_t buf_size) const;；执行该函数并处理它的返回值或副作用。
    void format_date_cn(char* buf, size_t buf_size) const;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* weekday_name_cn() const;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::data
