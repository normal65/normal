/**
 * [初学者导读] 文件：main/app/ebook/platform/housekeeper.cc
 * 作用：周期更新时钟、电池、自动锁屏和存储热插拔状态。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：阻塞工作放独立任务，周期小检查放 Housekeeper。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "platform/housekeeper.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/auto_lock.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/clock_provider.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/clock/clock_store.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/system_state.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "platform/battery_sampler.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "storage/storage.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_bus.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "Housekeeper";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::platform {

// 定义或声明函数 Housekeeper::get_instance：参数在括号里，返回类型写在函数名前面。
Housekeeper& Housekeeper::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static Housekeeper s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// [初学者导读] Housekeeper::start()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
bool Housekeeper::start()
{
    // 判断条件：running_ 是否成立。
    if (running_)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;

    // 执行语句：::app::sys::task::Cfg cfg;；它会更新一次变量、对象或程序状态。
    ::app::sys::task::Cfg cfg;
    // 赋值：把右侧 "ebook_house" 的结果保存到左侧 cfg.name。
    cfg.name = "ebook_house";
    // 赋值：把右侧 kStackBytes 的结果保存到左侧 cfg.stack_size。
    cfg.stack_size = kStackBytes;
    // 赋值：把右侧 ::app::sys::task::Priority::LOW 的结果保存到左侧 cfg.priority。
    cfg.priority = ::app::sys::task::Priority::LOW;
    // 赋值：把右侧 tskNO_AFFINITY 的结果保存到左侧 cfg.core_id。
    cfg.core_id = tskNO_AFFINITY;

    // 赋值：把右侧 true 的结果保存到左侧 running_。
    running_ = true;
    // 执行这条代码：task_ = std::make_unique<::app::sys::task::Task>(。它会更新变量、对象状态或调用链。
    task_ = std::make_unique<::app::sys::task::Task>(
        // 继续上一行的表达式或参数列表：[](void* arg) { static_cast<Housekeeper*>(arg)->run(); }, cfg, this);。
        [](void* arg) { static_cast<Housekeeper*>(arg)->run(); }, cfg, this);
    // 判断条件：!task_->start() 是否成立。
    if (!task_->start())
    {
        // 赋值：把右侧 false 的结果保存到左侧 running_。
        running_ = false;
        // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
        task_.reset();
        // 调用 EBOOK_LOGE(TAG, "task start failed");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGE(TAG, "task start failed");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数定义/声明：调用 Housekeeper::stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
void Housekeeper::stop()
{
    // 判断条件：!running_ 是否成立。
    if (!running_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 false 的结果保存到左侧 running_。
    running_ = false;
    // 判断条件：task_ 是否成立。
    if (task_)
    {
        // 调用 destroy()：直接删除，不是优雅停止。
        task_->destroy();
        // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
        task_.reset();
    }
}

// [初学者导读] Housekeeper::run()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void Housekeeper::run()
{
    // 定义变量 uint8_t last_hour 并初始化为 0xFF。
    uint8_t last_hour = 0xFF;
    // 定义变量 uint8_t last_min 并初始化为 0xFF。
    uint8_t last_min = 0xFF;
    // 定义变量 uint8_t last_pct 并初始化为 data::SystemState::get_instance().battery_pct()。
    uint8_t last_pct = data::SystemState::get_instance().battery_pct();
    // 定义变量 uint32_t tick 并初始化为 0。
    uint32_t tick = 0;

    // 开始代码块：下面是这个函数、分支或循环的主体。
    auto broadcast_clock_if_changed = [&](const data::Clock& clk) {
        // 调用 apps::clock::ClockStore::get_instance().tick(clk.hour, clk.minute, clk.second, clk.weekday);，执行这个函数并处理它的返回值或状态变化。
        apps::clock::ClockStore::get_instance().tick(clk.hour, clk.minute, clk.second, clk.weekday);
        // 判断条件：clk.hour != last_hour || clk.minute != last_min 是否成立。
        if (clk.hour != last_hour || clk.minute != last_min)
        {
            // 赋值：把右侧 clk.hour 的结果保存到左侧 last_hour。
            last_hour = clk.hour;
            // 赋值：把右侧 clk.minute 的结果保存到左侧 last_min。
            last_min = clk.minute;
            // 调用 post_tick_clock(clk.hour,clk.minute,255)：这些函数的共同模式是： 1. 用 makeevent(kind) 创建零初始化事件。 2. 填写 payload。 3. 调用 post(e, 0)。。
            (void)ui::UiBus::get_instance().post_tick_clock(clk.hour, clk.minute, 255);
        }
    };

    // 调用 broadcast_clock_if_changed(data::Clock::now());，执行这个函数并处理它的返回值或状态变化。
    broadcast_clock_if_changed(data::Clock::now());

    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (running_)
    {
        // 调用 app::sys::task::TaskMgr::delay_ms(kPeriodMs)：调用 vTaskDelay()，这是让出 CPU 的阻塞延时。一千毫秒延时期间，任务不占 CPU，调度器可以运行别人。。
        ::app::sys::task::TaskMgr::delay_ms(kPeriodMs);
        // 执行语句：++tick;；它会更新一次变量、对象或程序状态。
        ++tick;

        // 调用 broadcast_clock_if_changed(data::Clock::now());，执行这个函数并处理它的返回值或状态变化。
        broadcast_clock_if_changed(data::Clock::now());

        // 判断条件：(tick % kBatteryEveryTicks) == 0 && BatterySampler::get_instance().ready() 是否成立。
        if ((tick % kBatteryEveryTicks) == 0 && BatterySampler::get_instance().ready())
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint8_t pct = BatterySampler::get_instance().percent();
            // 判断条件：pct != last_pct 是否成立。
            if (pct != last_pct)
            {
                // 赋值：把右侧 pct 的结果保存到左侧 last_pct。
                last_pct = pct;
                // 函数调用：(void)ui::UiBus::get_instance().post_tick_battery(；执行该函数并处理它的返回值或副作用。
                (void)ui::UiBus::get_instance().post_tick_battery(
                    // 继续上一行的表达式或参数列表：pct, BatterySampler::get_instance().voltage_mv());。
                    pct, BatterySampler::get_instance().voltage_mv());
            }
        }

        // 调用 apps::settings::AutoLock::get_instance().tick();，执行这个函数并处理它的返回值或状态变化。
        apps::settings::AutoLock::get_instance().tick();

        // 调用 service_sd_hotplug()：这个函数被 Housekeeper 周期性调用。它先做时间节流，默认每 500 ms 最多执行一次，避免在 UI 循环里频繁访问 SD 总线。。
        ::app::common::storage::StorageMgr::get_instance().service_sd_hotplug();
        // 访问对象成员：::app::common::storage::StorageMgr::get_instance().service_internal_recovery();；读取或修改这个对象里的成员状态。
        ::app::common::storage::StorageMgr::get_instance().service_internal_recovery();
    }
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::platform
