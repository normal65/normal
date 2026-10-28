// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/common/time/time.hpp
 * 作用：提供系统时间、时间戳和格式化函数。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：本地时间依赖时区和同步状态，不要把它当成单调计时器。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <string>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::common::time {

/** @brief Unix 时间戳（秒），需 NTP 同步后才有意义 */
// 函数调用：int64_t unix_timestamp_sec();；执行该函数并处理它的返回值或副作用。
int64_t unix_timestamp_sec();

/** @brief Unix 时间戳（毫秒） */
// 函数调用：int64_t unix_timestamp_ms();；执行该函数并处理它的返回值或副作用。
int64_t unix_timestamp_ms();

/** @brief 自启动以来的毫秒数（重启归零） */
// 函数调用：int64_t uptime_ms();；执行该函数并处理它的返回值或副作用。
int64_t uptime_ms();

/** @brief 自启动以来的微秒数 */
// 函数调用：int64_t uptime_us();；执行该函数并处理它的返回值或副作用。
int64_t uptime_us();

/** @brief 自启动以来的秒数 */
// 函数调用：int64_t uptime_sec();；执行该函数并处理它的返回值或副作用。
int64_t uptime_sec();

/** @brief ISO 8601 UTC 字符串，如 2025-03-12T19:00:00Z */
// 函数调用：std::string iso8601_timestamp();；执行该函数并处理它的返回值或副作用。
std::string iso8601_timestamp();

// 命名空间结束：离开当前模块作用域。
} // namespace app::common::time
