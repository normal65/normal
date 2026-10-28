// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/clock/clock_store.hpp
 * 作用：保存和读取闹钟配置。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::clock {

/** 闹钟重复方式 */
// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class AlarmRepeat : uint8_t
{
    // 执行这条代码：Daily = 0,。它会更新变量、对象状态或调用链。
    Daily = 0,    ///< 每天
    // 继续上一行的表达式或参数列表：Weekdays = 1,。
    Weekdays = 1, ///< 工作日（周一至周五）
    // 继续上一行的表达式或参数列表：Once = 2,。
    Once = 2,     ///< 单次，响铃后自动关闭
};

// struct AlarmEntry：把相关数据字段组合在一起，表示项目中的一个概念。
struct AlarmEntry
{
    // 执行语句：uint8_t hour{7};；它会更新一次变量、对象或程序状态。
    uint8_t hour{7};
    // 执行语句：uint8_t minute{0};；它会更新一次变量、对象或程序状态。
    uint8_t minute{0};
    // 执行语句：bool enabled{false};；它会更新一次变量、对象或程序状态。
    bool enabled{false};
    // 执行语句：AlarmRepeat repeat{AlarmRepeat::Daily};；它会更新一次变量、对象或程序状态。
    AlarmRepeat repeat{AlarmRepeat::Daily};
};

/**
 * @brief 闹钟状态（NVS 持久化；housekeeper 整分 tick 后投递 ClockAlarm）
 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class ClockStore
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 固定参数 kMaxAlarms = 6：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kMaxAlarms = 6;

    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static ClockStore& get_instance();

    // 函数调用：void load();；执行该函数并处理它的返回值或副作用。
    void load();
    // 调用 save()：这是拨码映射的持久化表： load() 先设默认值，再用 NVS 覆盖合法值。 save() 把所有槽位写入 NVS 并 commit。 lookup() 按 Context 和 Key 找动作。 cycle() 在设置页循环切换到下一个可选动作。。
    void save();

    // 定义或声明函数 alarm_count：参数在括号里，返回类型写在函数名前面。
    uint8_t alarm_count() const
    {
        // 返回 alarm_count_ 给调用者，表示当前函数的执行结果。
        return alarm_count_;
    }
    // 只读约束：这个值或对象在当前位置不能被修改。
    const AlarmEntry& alarm(uint8_t idx) const;
    // 函数调用：AlarmEntry& alarm_mut(uint8_t idx);；执行该函数并处理它的返回值或副作用。
    AlarmEntry& alarm_mut(uint8_t idx);

    // 函数调用：bool add_alarm();；执行该函数并处理它的返回值或副作用。
    bool add_alarm();
    // 函数调用：void remove_alarm(uint8_t idx);；执行该函数并处理它的返回值或副作用。
    void remove_alarm(uint8_t idx);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* repeat_label(AlarmRepeat r) const;

    /** 每秒调用；在整分时刻检查闹钟 */
    // 函数调用：void tick(uint8_t hour, uint8_t minute, uint8_t second, uint8_t weekday);；执行该函数并处理它的返回值或副作用。
    void tick(uint8_t hour, uint8_t minute, uint8_t second, uint8_t weekday);

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 ClockStore() = default;，执行这个函数并处理它的返回值或状态变化。
    ClockStore() = default;

    // 函数调用：bool weekday_matches(const AlarmEntry& a, uint8_t weekday) const;；执行该函数并处理它的返回值或副作用。
    bool weekday_matches(const AlarmEntry& a, uint8_t weekday) const;
    // 函数调用：void try_fire_alarms(uint8_t hour, uint8_t minute, uint8_t weekday);；执行该函数并处理它的返回值或副作用。
    void try_fire_alarms(uint8_t hour, uint8_t minute, uint8_t weekday);
    // 函数调用：void persist_alarm(uint8_t idx) const;；执行该函数并处理它的返回值或副作用。
    void persist_alarm(uint8_t idx) const;
    // 函数调用：void load_alarm(uint8_t idx);；执行该函数并处理它的返回值或副作用。
    void load_alarm(uint8_t idx);

    // 执行语句：AlarmEntry alarms_[kMaxAlarms]{};；它会更新一次变量、对象或程序状态。
    AlarmEntry alarms_[kMaxAlarms]{};
    // 执行语句：uint8_t alarm_count_{0};；它会更新一次变量、对象或程序状态。
    uint8_t alarm_count_{0};

    /** 每分钟最多触发一次的去重状态 */
    // 执行语句：uint8_t last_fire_hour_{0xFF};；它会更新一次变量、对象或程序状态。
    uint8_t last_fire_hour_{0xFF};
    // 执行语句：uint8_t last_fire_minute_{0xFF};；它会更新一次变量、对象或程序状态。
    uint8_t last_fire_minute_{0xFF};
    // 执行语句：bool fired_this_minute_{false};；它会更新一次变量、对象或程序状态。
    bool fired_this_minute_{false};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::clock
