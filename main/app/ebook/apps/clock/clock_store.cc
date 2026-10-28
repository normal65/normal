/**
 * [初学者导读] 文件：main/app/ebook/apps/clock/clock_store.cc
 * 作用：保存和读取闹钟配置。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/clock/clock_store.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/persist.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_bus.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_event.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::clock {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 定义或声明函数 repeat_from_u8：参数在括号里，返回类型写在函数名前面。
AlarmRepeat repeat_from_u8(uint8_t v)
{
    // 返回 (v <= 2) ? static_cast<AlarmRepeat>(v) : AlarmRepeat::Daily 给调用者，表示当前函数的执行结果。
    return (v <= 2) ? static_cast<AlarmRepeat>(v) : AlarmRepeat::Daily;
}

// 定义或声明函数 make_key：参数在括号里，返回类型写在函数名前面。
void make_key(char* buf, size_t cap, uint8_t idx, char suffix)
{
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    (void)std::snprintf(buf, cap, "clk.a%u%c", static_cast<unsigned>(idx), suffix);
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 ClockStore::get_instance：参数在括号里，返回类型写在函数名前面。
ClockStore& ClockStore::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static ClockStore s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义或声明函数 ClockStore::load：参数在括号里，返回类型写在函数名前面。
void ClockStore::load()
{
    // 赋值：把右侧 0 的结果保存到左侧 alarm_count_。
    alarm_count_ = 0;
    // 定义变量 uint8_t n 并初始化为 0。
    uint8_t n = 0;
    // 判断条件：data::Persist::get_u8("clk.n", n) && n <= kMaxAlarms 是否成立。
    if (data::Persist::get_u8("clk.n", n) && n <= kMaxAlarms)
        // 赋值：把右侧 n 的结果保存到左侧 alarm_count_。
        alarm_count_ = n;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < alarm_count_; ++i)
        // 调用 load_alarm(i);，执行这个函数并处理它的返回值或状态变化。
        load_alarm(i);
}

// 定义或声明函数 ClockStore::save：参数在括号里，返回类型写在函数名前面。
void ClockStore::save()
{
    // 访问对象成员：(void)data::Persist::set_u8("clk.n", alarm_count_);；读取或修改这个对象里的成员状态。
    (void)data::Persist::set_u8("clk.n", alarm_count_);
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < alarm_count_; ++i)
        // 调用 persist_alarm(i);，执行这个函数并处理它的返回值或状态变化。
        persist_alarm(i);
    // 调用 data::Persist::commit();，执行这个函数并处理它的返回值或状态变化。
    data::Persist::commit();
}

// 定义或声明函数 ClockStore::load_alarm：参数在括号里，返回类型写在函数名前面。
void ClockStore::load_alarm(uint8_t idx)
{
    // 判断条件：idx >= kMaxAlarms 是否成立。
    if (idx >= kMaxAlarms)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 执行语句：char key[12];；它会更新一次变量、对象或程序状态。
    char key[12];
    // 赋值：把右侧 alarms_[idx] 的结果保存到左侧 AlarmEntry& a。
    AlarmEntry& a = alarms_[idx];

    // 调用 make_key(key, sizeof(key), idx, 'h');，执行这个函数并处理它的返回值或状态变化。
    make_key(key, sizeof(key), idx, 'h');
    // 定义变量 uint8_t v 并初始化为 0。
    uint8_t v = 0;
    // 判断条件：data::Persist::get_u8(key, v) 是否成立。
    if (data::Persist::get_u8(key, v))
        // 赋值：把右侧 v 的结果保存到左侧 a.hour。
        a.hour = v;

    // 调用 make_key(key, sizeof(key), idx, 'm');，执行这个函数并处理它的返回值或状态变化。
    make_key(key, sizeof(key), idx, 'm');
    // 判断条件：data::Persist::get_u8(key, v) 是否成立。
    if (data::Persist::get_u8(key, v))
        // 赋值：把右侧 v 的结果保存到左侧 a.minute。
        a.minute = v;

    // 调用 make_key(key, sizeof(key), idx, 'e');，执行这个函数并处理它的返回值或状态变化。
    make_key(key, sizeof(key), idx, 'e');
    // 定义变量 bool en 并初始化为 false。
    bool en = false;
    // 判断条件：data::Persist::get_bool(key, en) 是否成立。
    if (data::Persist::get_bool(key, en))
        // 赋值：把右侧 en 的结果保存到左侧 a.enabled。
        a.enabled = en;

    // 调用 make_key(key, sizeof(key), idx, 'r');，执行这个函数并处理它的返回值或状态变化。
    make_key(key, sizeof(key), idx, 'r');
    // 判断条件：data::Persist::get_u8(key, v) 是否成立。
    if (data::Persist::get_u8(key, v))
        // 赋值：把右侧 repeat_from_u8(v) 的结果保存到左侧 a.repeat。
        a.repeat = repeat_from_u8(v);
}

// 定义或声明函数 ClockStore::persist_alarm：参数在括号里，返回类型写在函数名前面。
void ClockStore::persist_alarm(uint8_t idx) const
{
    // 判断条件：idx >= kMaxAlarms 是否成立。
    if (idx >= kMaxAlarms)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 执行语句：char key[12];；它会更新一次变量、对象或程序状态。
    char key[12];
    // 只读约束：这个值或对象在当前位置不能被修改。
    const AlarmEntry& a = alarms_[idx];

    // 调用 make_key(key, sizeof(key), idx, 'h');，执行这个函数并处理它的返回值或状态变化。
    make_key(key, sizeof(key), idx, 'h');
    // 访问对象成员：(void)data::Persist::set_u8(key, a.hour);；读取或修改这个对象里的成员状态。
    (void)data::Persist::set_u8(key, a.hour);
    // 调用 make_key(key, sizeof(key), idx, 'm');，执行这个函数并处理它的返回值或状态变化。
    make_key(key, sizeof(key), idx, 'm');
    // 访问对象成员：(void)data::Persist::set_u8(key, a.minute);；读取或修改这个对象里的成员状态。
    (void)data::Persist::set_u8(key, a.minute);
    // 调用 make_key(key, sizeof(key), idx, 'e');，执行这个函数并处理它的返回值或状态变化。
    make_key(key, sizeof(key), idx, 'e');
    // 访问对象成员：(void)data::Persist::set_bool(key, a.enabled);；读取或修改这个对象里的成员状态。
    (void)data::Persist::set_bool(key, a.enabled);
    // 调用 make_key(key, sizeof(key), idx, 'r');，执行这个函数并处理它的返回值或状态变化。
    make_key(key, sizeof(key), idx, 'r');
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    (void)data::Persist::set_u8(key, static_cast<uint8_t>(a.repeat));
}

// 只读约束：这个值或对象在当前位置不能被修改。
const AlarmEntry& ClockStore::alarm(uint8_t idx) const
{
    // 静态空对象 kEmpty：作为默认返回值或占位值，避免返回悬空引用。
    static const AlarmEntry kEmpty{};
    // 返回 (idx < alarm_count_) ? alarms_[idx] : kEmpty 给调用者，表示当前函数的执行结果。
    return (idx < alarm_count_) ? alarms_[idx] : kEmpty;
}

// 定义或声明函数 ClockStore::alarm_mut：参数在括号里，返回类型写在函数名前面。
AlarmEntry& ClockStore::alarm_mut(uint8_t idx)
{
    // 静态空对象 kEmpty：作为默认返回值或占位值，避免返回悬空引用。
    static AlarmEntry kEmpty{};
    // 返回 (idx < alarm_count_) ? alarms_[idx] : kEmpty 给调用者，表示当前函数的执行结果。
    return (idx < alarm_count_) ? alarms_[idx] : kEmpty;
}

// 定义或声明函数 ClockStore::add_alarm：参数在括号里，返回类型写在函数名前面。
bool ClockStore::add_alarm()
{
    // 判断条件：alarm_count_ >= kMaxAlarms 是否成立。
    if (alarm_count_ >= kMaxAlarms)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 赋值：把右侧 alarms_[alarm_count_] 的结果保存到左侧 AlarmEntry& a。
    AlarmEntry& a = alarms_[alarm_count_];
    // 赋值：把右侧 {} 的结果保存到左侧 a。
    a = {};
    // 赋值：把右侧 7 的结果保存到左侧 a.hour。
    a.hour = 7;
    // 赋值：把右侧 0 的结果保存到左侧 a.minute。
    a.minute = 0;
    // 赋值：把右侧 true 的结果保存到左侧 a.enabled。
    a.enabled = true;
    // 赋值：把右侧 AlarmRepeat::Daily 的结果保存到左侧 a.repeat。
    a.repeat = AlarmRepeat::Daily;
    // 执行语句：++alarm_count_;；它会更新一次变量、对象或程序状态。
    ++alarm_count_;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 ClockStore::remove_alarm：参数在括号里，返回类型写在函数名前面。
void ClockStore::remove_alarm(uint8_t idx)
{
    // 判断条件：idx >= alarm_count_ 是否成立。
    if (idx >= alarm_count_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = idx; i + 1 < alarm_count_; ++i)
        // 赋值：把右侧 alarms_[i + 1] 的结果保存到左侧 alarms_[i]。
        alarms_[i] = alarms_[i + 1];
    // 执行语句：--alarm_count_;；它会更新一次变量、对象或程序状态。
    --alarm_count_;
    // 调用 save()：这是拨码映射的持久化表： load() 先设默认值，再用 NVS 覆盖合法值。 save() 把所有槽位写入 NVS 并 commit。 lookup() 按 Context 和 Key 找动作。 cycle() 在设置页循环切换到下一个可选动作。。
    save();
}

// 只读约束：这个值或对象在当前位置不能被修改。
const char* ClockStore::repeat_label(AlarmRepeat r) const
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (r)
    {
        // 分支匹配：case AlarmRepeat::Daily 成立时执行下面的代码。
        case AlarmRepeat::Daily:
            // 返回 "\u6BCF\u5929" 给调用者，表示当前函数的执行结果。
            return "\u6BCF\u5929";
        // 分支匹配：case AlarmRepeat::Weekdays 成立时执行下面的代码。
        case AlarmRepeat::Weekdays:
            // 返回 "\u5DE5\u4F5C\u65E5" 给调用者，表示当前函数的执行结果。
            return "\u5DE5\u4F5C\u65E5";
        // 分支匹配：case AlarmRepeat::Once 成立时执行下面的代码。
        case AlarmRepeat::Once:
            // 返回 "\u5355\u6B21" 给调用者，表示当前函数的执行结果。
            return "\u5355\u6B21";
    }
    // 返回 "" 给调用者，表示当前函数的执行结果。
    return "";
}

// 定义或声明函数 ClockStore::weekday_matches：参数在括号里，返回类型写在函数名前面。
bool ClockStore::weekday_matches(const AlarmEntry& a, uint8_t weekday) const
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (a.repeat)
    {
        // 分支匹配：case AlarmRepeat::Daily 成立时执行下面的代码。
        case AlarmRepeat::Daily:
            // 返回 true 给调用者，表示当前函数的执行结果。
            return true;
        // 分支匹配：case AlarmRepeat::Weekdays 成立时执行下面的代码。
        case AlarmRepeat::Weekdays:
            // 返回 weekday >= 1 && weekday <= 5 给调用者，表示当前函数的执行结果。
            return weekday >= 1 && weekday <= 5;
        // 分支匹配：case AlarmRepeat::Once 成立时执行下面的代码。
        case AlarmRepeat::Once:
            // 返回 true 给调用者，表示当前函数的执行结果。
            return true;
    }
    // 返回 false 给调用者，表示当前函数的执行结果。
    return false;
}

// 定义或声明函数 ClockStore::try_fire_alarms：参数在括号里，返回类型写在函数名前面。
void ClockStore::try_fire_alarms(uint8_t hour, uint8_t minute, uint8_t weekday)
{
    // 判断条件：last_fire_hour_ != hour || last_fire_minute_ != minute 是否成立。
    if (last_fire_hour_ != hour || last_fire_minute_ != minute)
    {
        // 赋值：把右侧 hour 的结果保存到左侧 last_fire_hour_。
        last_fire_hour_ = hour;
        // 赋值：把右侧 minute 的结果保存到左侧 last_fire_minute_。
        last_fire_minute_ = minute;
        // 赋值：把右侧 false 的结果保存到左侧 fired_this_minute_。
        fired_this_minute_ = false;
    }
    // 判断条件：fired_this_minute_ 是否成立。
    if (fired_this_minute_)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < alarm_count_; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const AlarmEntry& a = alarms_[i];
        // 判断条件：!a.enabled 是否成立。
        if (!a.enabled)
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        // 判断条件：a.hour != hour || a.minute != minute 是否成立。
        if (a.hour != hour || a.minute != minute)
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        // 判断条件：!weekday_matches(a, weekday) 是否成立。
        if (!weekday_matches(a, weekday))
            // 继续下一轮：跳过本次循环剩余代码。
            continue;

        // 赋值：把右侧 true 的结果保存到左侧 fired_this_minute_。
        fired_this_minute_ = true;
        // 判断条件：a.repeat == AlarmRepeat::Once 是否成立。
        if (a.repeat == AlarmRepeat::Once)
        {
            // 赋值：把右侧 false 的结果保存到左侧 alarms_[i].enabled。
            alarms_[i].enabled = false;
            // 调用 save()：这是拨码映射的持久化表： load() 先设默认值，再用 NVS 覆盖合法值。 save() 把所有槽位写入 NVS 并 commit。 lookup() 按 Context 和 Key 找动作。 cycle() 在设置页循环切换到下一个可选动作。。
            save();
        }
        // 访问对象成员：(void)ui::UiBus::get_instance().post_system_hint(ui::SystemHintKind::ClockAlarm, i);；读取或修改这个对象里的成员状态。
        (void)ui::UiBus::get_instance().post_system_hint(ui::SystemHintKind::ClockAlarm, i);
        // 跳出当前循环或 switch，不再执行剩余分支。
        break;
    }
}

// 定义或声明函数 ClockStore::tick：参数在括号里，返回类型写在函数名前面。
void ClockStore::tick(uint8_t hour, uint8_t minute, uint8_t second, uint8_t weekday)
{
    // 判断条件：second == 0 是否成立。
    if (second == 0)
        // 调用 try_fire_alarms(hour, minute, weekday);，执行这个函数并处理它的返回值或状态变化。
        try_fire_alarms(hour, minute, weekday);
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::clock
