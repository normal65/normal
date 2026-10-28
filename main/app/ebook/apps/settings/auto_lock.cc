/**
 * [初学者导读] 文件：main/app/ebook/apps/settings/auto_lock.cc
 * 作用：根据空闲时间判断何时发出自动锁屏提示。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/auto_lock.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <freertos/FreeRTOS.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <freertos/task.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/persist.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/page_id.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/router.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_bus.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::settings {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 固定参数 kKOn = "disp.autolock.on"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kKOn = "disp.autolock.on";
// 固定参数 kKMin = "disp.autolock.min"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kKMin = "disp.autolock.min";

// 编译期数组 kPresets：运行时只读的固定查表数据。
constexpr uint8_t kPresets[] = {1, 3, 5, 10, 30};
// 固定参数 kPresetCount = sizeof(kPresets) / sizeof(kPresets[0])：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kPresetCount = sizeof(kPresets) / sizeof(kPresets[0]);

// 定义或声明函数 index_of_min：参数在括号里，返回类型写在函数名前面。
uint8_t index_of_min(uint8_t min)
{
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < kPresetCount; ++i)
    {
        // 判断条件：kPresets[i] == min 是否成立。
        if (kPresets[i] == min)
            // 返回 i 给调用者，表示当前函数的执行结果。
            return i;
    }
    // 返回 2 给调用者，表示当前函数的执行结果。
    return 2;
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// [初学者导读] AutoLock::get_instance()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
AutoLock& AutoLock::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static AutoLock s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// [初学者导读] AutoLock::now_ms()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
uint32_t AutoLock::now_ms()
{
    // 返回 static_cast<uint32_t>(xTaskGetTickCount() * portTICK_PERIOD_MS) 给调用者，表示当前函数的执行结果。
    return static_cast<uint32_t>(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

// [初学者导读] AutoLock::load()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void AutoLock::load()
{
    // 赋值：把右侧 false 的结果保存到左侧 enabled_。
    enabled_ = false;
    // 赋值：把右侧 2 的结果保存到左侧 preset_idx_。
    preset_idx_ = 2;
    // 调用 now_ms()：返回单调递增毫秒时间。它用于计算“距离上次活动多久”，不应依赖会跳变的墙上时钟。。
    last_activity_ms_ = now_ms();
    // 赋值：把右侧 false 的结果保存到左侧 lock_hint_sent_。
    lock_hint_sent_ = false;

    // 定义变量 bool on 并初始化为 false。
    bool on = false;
    // 判断条件：data::Persist::get_bool(kKOn, on) 是否成立。
    if (data::Persist::get_bool(kKOn, on))
        // 赋值：把右侧 on 的结果保存到左侧 enabled_。
        enabled_ = on;

    // 定义变量 uint8_t min 并初始化为 kPresets[preset_idx_]。
    uint8_t min = kPresets[preset_idx_];
    // 判断条件：data::Persist::get_u8(kKMin, min) 是否成立。
    if (data::Persist::get_u8(kKMin, min))
        // 赋值：把右侧 index_of_min(min) 的结果保存到左侧 preset_idx_。
        preset_idx_ = index_of_min(min);
}

// [初学者导读] AutoLock::timeout_min()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
uint8_t AutoLock::timeout_min() const
{
    // 返回 kPresets[preset_idx_ < kPresetCount ? preset_idx_ : 0] 给调用者，表示当前函数的执行结果。
    return kPresets[preset_idx_ < kPresetCount ? preset_idx_ : 0];
}

// [初学者导读] AutoLock::preset_count()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
uint8_t AutoLock::preset_count() const
{
    // 返回 kPresetCount 给调用者，表示当前函数的执行结果。
    return kPresetCount;
}

// [初学者导读] AutoLock::preset_index()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
uint8_t AutoLock::preset_index() const
{
    // 返回 preset_idx_ 给调用者，表示当前函数的执行结果。
    return preset_idx_;
}

// 定义或声明函数 AutoLock::set_enabled：参数在括号里，返回类型写在函数名前面。
void AutoLock::set_enabled(bool on)
{
    // 判断条件：on == enabled_ 是否成立。
    if (on == enabled_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 on 的结果保存到左侧 enabled_。
    enabled_ = on;
    // 函数调用：(void)data::Persist::set_bool(kKOn, on);；执行该函数并处理它的返回值或副作用。
    (void)data::Persist::set_bool(kKOn, on);
    // 调用 data::Persist::commit();，执行这个函数并处理它的返回值或状态变化。
    data::Persist::commit();
    // 调用 notify_activity()：记录用户活动时间。触摸、按键或页面交互后应该调用它，否则用户刚操作完也可能被锁屏。。
    notify_activity();
}

// [初学者导读] AutoLock::cycle_timeout_preset()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void AutoLock::cycle_timeout_preset()
{
    // 赋值：把右侧 static_cast<uint8_t>((preset_idx_ + 1U) % kPresetCount) 的结果保存到左侧 preset_idx_。
    preset_idx_ = static_cast<uint8_t>((preset_idx_ + 1U) % kPresetCount);
    // 调用 timeout_min()：返回当前预设对应的分钟数。。
    (void)data::Persist::set_u8(kKMin, timeout_min());
    // 调用 data::Persist::commit();，执行这个函数并处理它的返回值或状态变化。
    data::Persist::commit();
    // 调用 notify_activity()：记录用户活动时间。触摸、按键或页面交互后应该调用它，否则用户刚操作完也可能被锁屏。。
    notify_activity();
}

// [初学者导读] AutoLock::notify_activity()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void AutoLock::notify_activity()
{
    // 调用 now_ms()：返回单调递增毫秒时间。它用于计算“距离上次活动多久”，不应依赖会跳变的墙上时钟。。
    last_activity_ms_ = now_ms();
    // 赋值：把右侧 false 的结果保存到左侧 lock_hint_sent_。
    lock_hint_sent_ = false;
}

// 函数定义/声明：调用 AutoLock::format_timeout()：把分钟数格式化成界面文字。必须有长度上限，不能直接拼接未知长度字符串。。
void AutoLock::format_timeout(char* buf, size_t cap) const
{
    // 判断条件：buf == nullptr || cap == 0U 是否成立。
    if (buf == nullptr || cap == 0U)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const unsigned min = static_cast<unsigned>(timeout_min());
    // 函数调用：(void)std::snprintf(buf, cap, "%u\u5206\u949F", min);；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(buf, cap, "%u\u5206\u949F", min);
}

// [初学者导读] AutoLock::tick()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void AutoLock::tick()
{
    // 判断条件：!enabled_ 是否成立。
    if (!enabled_)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 判断条件：router::Router::instance().stack().shell_top() == router::ShellPage::Lock 是否成立。
    if (router::Router::instance().stack().shell_top() == router::ShellPage::Lock)
    {
        // 赋值：把右侧 false 的结果保存到左侧 lock_hint_sent_。
        lock_hint_sent_ = false;
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const uint32_t limit_ms = static_cast<uint32_t>(timeout_min()) * 60U * 1000U;
    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const uint32_t idle_ms = now_ms() - last_activity_ms_;
    // 判断条件：idle_ms < limit_ms || lock_hint_sent_ 是否成立。
    if (idle_ms < limit_ms || lock_hint_sent_)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 true 的结果保存到左侧 lock_hint_sent_。
    lock_hint_sent_ = true;
    // 访问对象成员：(void)ui::UiBus::get_instance().post_system_hint(ui::SystemHintKind::AutoLock);；读取或修改这个对象里的成员状态。
    (void)ui::UiBus::get_instance().post_system_hint(ui::SystemHintKind::AutoLock);
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::settings
