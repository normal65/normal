/**
 * [初学者导读] 文件：main/common/time/time.cc
 * 作用：提供系统时间、时间戳和格式化函数。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：本地时间依赖时区和同步状态，不要把它当成单调计时器。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "time.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <sys/time.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <ctime>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_timer.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/task/task.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::common::time {

// 定义或声明函数 unix_timestamp_sec：参数在括号里，返回类型写在函数名前面。
int64_t unix_timestamp_sec()
{
    // struct timeval：把相关数据字段组合在一起，表示项目中的一个概念。
    struct timeval tv;
    // 调用 gettimeofday(&tv, nullptr);，执行这个函数并处理它的返回值或状态变化。
    gettimeofday(&tv, nullptr);
    // 返回 static_cast<int64_t>(tv.tv_sec) 给调用者，表示当前函数的执行结果。
    return static_cast<int64_t>(tv.tv_sec);
}

// 定义或声明函数 unix_timestamp_ms：参数在括号里，返回类型写在函数名前面。
int64_t unix_timestamp_ms()
{
    // struct timeval：把相关数据字段组合在一起，表示项目中的一个概念。
    struct timeval tv;
    // 调用 gettimeofday(&tv, nullptr);，执行这个函数并处理它的返回值或状态变化。
    gettimeofday(&tv, nullptr);
    // 返回 (static_cast<int64_t>(tv.tv_sec) * 1000LL) + (static_cast<int64_t>(tv.tv_usec) / 1000LL) 给调用者，表示当前函数的执行结果。
    return (static_cast<int64_t>(tv.tv_sec) * 1000LL) + (static_cast<int64_t>(tv.tv_usec) / 1000LL);
}

// 定义或声明函数 uptime_ms：参数在括号里，返回类型写在函数名前面。
int64_t uptime_ms()
{
    // 定义变量 uint32_t ticks 并初始化为 app::sys::task::TaskMgr::tick_count()。
    uint32_t ticks = app::sys::task::TaskMgr::tick_count();
    // 返回 static_cast<int64_t>(ticks) * 1000LL / configTICK_RATE_HZ 给调用者，表示当前函数的执行结果。
    return static_cast<int64_t>(ticks) * 1000LL / configTICK_RATE_HZ;
}

// 定义或声明函数 uptime_us：参数在括号里，返回类型写在函数名前面。
int64_t uptime_us()
{
    // 返回 esp_timer_get_time() 给调用者，表示当前函数的执行结果。
    return esp_timer_get_time();
}

// 定义或声明函数 uptime_sec：参数在括号里，返回类型写在函数名前面。
int64_t uptime_sec()
{
    // 返回 esp_timer_get_time() / 1000000LL 给调用者，表示当前函数的执行结果。
    return esp_timer_get_time() / 1000000LL;
}

// 定义或声明函数 iso8601_timestamp：参数在括号里，返回类型写在函数名前面。
std::string iso8601_timestamp()
{
    // 定义变量 int64_t timestamp_sec 并初始化为 unix_timestamp_sec()。
    int64_t timestamp_sec = unix_timestamp_sec();

    // struct tm：把相关数据字段组合在一起，表示项目中的一个概念。
    struct tm tm_info;
    // 定义变量 time_t time_val 并初始化为 static_cast<time_t>(timestamp_sec)。
    time_t time_val = static_cast<time_t>(timestamp_sec);
    // 调用 gmtime_r(&time_val, &tm_info);，执行这个函数并处理它的返回值或状态变化。
    gmtime_r(&time_val, &tm_info);

    // 执行语句：char buffer[32];；它会更新一次变量、对象或程序状态。
    char buffer[32];
    // 调用 strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &tm_info);，执行这个函数并处理它的返回值或状态变化。
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &tm_info);
    // 返回 std::string(buffer) 给调用者，表示当前函数的执行结果。
    return std::string(buffer);
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::common::time
