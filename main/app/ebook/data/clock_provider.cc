/**
 * [初学者导读] 文件：main/app/ebook/data/clock_provider.cc
 * 作用：提供当前时间和时间字符串。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：小配置写 NVS，运行时共享状态放 SystemState。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/clock_provider.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <ctime>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "common/time/time.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::data {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 静态查表数组 kCnWeekdays：运行时复用固定数据，避免每次重新计算。
constexpr const char* kCnWeekdays[7] = {
    // 执行这条代码："\u5468\u65E5", "\u5468\u4E00", "\u5468\u4E8C", "\u5468\u4E09",。它会更新变量、对象状态或调用链。
    "\u5468\u65E5", "\u5468\u4E00", "\u5468\u4E8C", "\u5468\u4E09",
    // 继续上一行的表达式或参数列表："\u5468\u56DB", "\u5468\u4E94", "\u5468\u516D",。
    "\u5468\u56DB", "\u5468\u4E94", "\u5468\u516D",
};

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 Clock::now：参数在括号里，返回类型写在函数名前面。
Clock Clock::now()
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto ts = static_cast<time_t>(::app::common::time::unix_timestamp_sec());
    // struct tm：把相关数据字段组合在一起，表示项目中的一个概念。
    struct tm lt = {};
    // 调用 localtime_r(&ts, &lt);，执行这个函数并处理它的返回值或状态变化。
    localtime_r(&ts, &lt);

    // 声明并默认初始化对象：Clock c{}。
    Clock c{};
    // 赋值：把右侧 static_cast<uint8_t>(lt.tm_hour) 的结果保存到左侧 c.hour。
    c.hour = static_cast<uint8_t>(lt.tm_hour);
    // 赋值：把右侧 static_cast<uint8_t>(lt.tm_min) 的结果保存到左侧 c.minute。
    c.minute = static_cast<uint8_t>(lt.tm_min);
    // 赋值：把右侧 static_cast<uint8_t>(lt.tm_sec) 的结果保存到左侧 c.second。
    c.second = static_cast<uint8_t>(lt.tm_sec);
    // 赋值：把右侧 static_cast<uint16_t>(lt.tm_year + 1900) 的结果保存到左侧 c.year。
    c.year = static_cast<uint16_t>(lt.tm_year + 1900);
    // 赋值：把右侧 static_cast<uint8_t>(lt.tm_mon + 1) 的结果保存到左侧 c.month。
    c.month = static_cast<uint8_t>(lt.tm_mon + 1);
    // 赋值：把右侧 static_cast<uint8_t>(lt.tm_mday) 的结果保存到左侧 c.day。
    c.day = static_cast<uint8_t>(lt.tm_mday);
    // 赋值：把右侧 static_cast<uint8_t>(lt.tm_wday) 的结果保存到左侧 c.weekday。
    c.weekday = static_cast<uint8_t>(lt.tm_wday);
    // 赋值：把右侧 static_cast<uint16_t>(lt.tm_yday) 的结果保存到左侧 c.day_of_year。
    c.day_of_year = static_cast<uint16_t>(lt.tm_yday);
    // 返回 c 给调用者，表示当前函数的执行结果。
    return c;
}

// 定义或声明函数 Clock::format_time_hm：参数在括号里，返回类型写在函数名前面。
void Clock::format_time_hm(char* buf, size_t buf_size) const
{
    // 判断条件：buf == nullptr || buf_size == 0 是否成立。
    if (buf == nullptr || buf_size == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    (void)std::snprintf(buf, buf_size, "%02u:%02u", static_cast<unsigned>(hour),
                        // 继续上一行的表达式或参数列表：static_cast<unsigned>(minute));。
                        static_cast<unsigned>(minute));
}

// 定义或声明函数 Clock::format_date_cn：参数在括号里，返回类型写在函数名前面。
void Clock::format_date_cn(char* buf, size_t buf_size) const
{
    // 判断条件：buf == nullptr || buf_size == 0 是否成立。
    if (buf == nullptr || buf_size == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    (void)std::snprintf(buf, buf_size, "%u\u5E74%u\u6708%u\u65E5", static_cast<unsigned>(year),
                        // 继续上一行的表达式或参数列表：static_cast<unsigned>(month), static_cast<unsigned>(day));。
                        static_cast<unsigned>(month), static_cast<unsigned>(day));
}

// 只读约束：这个值或对象在当前位置不能被修改。
const char* Clock::weekday_name_cn() const
{
    // 返回 (weekday <= 6) ? kCnWeekdays[weekday] : "" 给调用者，表示当前函数的执行结果。
    return (weekday <= 6) ? kCnWeekdays[weekday] : "";
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::data
