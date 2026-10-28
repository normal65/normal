/**
 * [初学者导读] 文件：main/app/ebook/input/physical_input.cc
 * 作用：运行物理按键任务并产生按键事件。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：中断和输入任务不能直接修改页面，只能投递结果。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/physical_input.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <freertos/FreeRTOS.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <freertos/task.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/physical_types.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_bus.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "PhysicalInput";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::input {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ::app::bsp::driver::button::DipId;

// 函数定义/声明：调用 now_ms()：返回单调递增毫秒时间。它用于计算“距离上次活动多久”，不应依赖会跳变的墙上时钟。。
uint32_t now_ms()
{
    // 返回 static_cast<uint32_t>(xTaskGetTickCount() * portTICK_PERIOD_MS) 给调用者，表示当前函数的执行结果。
    return static_cast<uint32_t>(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

// 定义或声明函数 map_key：参数在括号里，返回类型写在函数名前面。
PhysicalKey map_key(DipId id)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (id)
    {
        // 分支匹配：case DipId::UP 成立时执行下面的代码。
        case DipId::UP:
            // 返回 PhysicalKey::Up 给调用者，表示当前函数的执行结果。
            return PhysicalKey::Up;
        // 分支匹配：case DipId::MID 成立时执行下面的代码。
        case DipId::MID:
            // 返回 PhysicalKey::Mid 给调用者，表示当前函数的执行结果。
            return PhysicalKey::Mid;
        // 分支匹配：case DipId::DOWN 成立时执行下面的代码。
        case DipId::DOWN:
            // 返回 PhysicalKey::Down 给调用者，表示当前函数的执行结果。
            return PhysicalKey::Down;
        // 分支匹配：case DipId::NONE 成立时执行下面的代码。
        case DipId::NONE:
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
    // 返回 PhysicalKey::Mid 给调用者，表示当前函数的执行结果。
    return PhysicalKey::Mid;
}

// 函数定义/声明：调用 post_physical()：这些函数的共同模式是： 1. 用 makeevent(kind) 创建零初始化事件。 2. 填写 payload。 3. 调用 post(e, 0)。。
void post_physical(PhysicalKey key, PhysicalAction action)
{
    // 调用 post_physical(key,action)：这些函数的共同模式是： 1. 用 makeevent(kind) 创建零初始化事件。 2. 填写 payload。 3. 调用 post(e, 0)。。
    (void)ui::UiBus::get_instance().post_physical(key, action);
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 PhysicalInput::get_instance：参数在括号里，返回类型写在函数名前面。
PhysicalInput& PhysicalInput::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static PhysicalInput s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// [初学者导读] PhysicalInput::init()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
bool PhysicalInput::init()
{
    // 判断条件：dip_.is_init() 是否成立。
    if (dip_.is_init())
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    // 判断条件：!dip_.init() 是否成立。
    if (!dip_.init())
    {
        // 调用 EBOOK_LOGW(TAG, "dip init failed");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "dip init failed");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 赋值：把右侧 DipId::NONE 的结果保存到左侧 stable_。
    stable_ = DipId::NONE;
    // 赋值：把右侧 DipId::NONE 的结果保存到左侧 pending_。
    pending_ = DipId::NONE;
    // 赋值：把右侧 0 的结果保存到左侧 pending_count_。
    pending_count_ = 0;
    // 调用 now_ms()：返回单调递增毫秒时间。它用于计算“距离上次活动多久”，不应依赖会跳变的墙上时钟。。
    last_emit_ms_ = now_ms();
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 PhysicalInput::deinit：参数在括号里，返回类型写在函数名前面。
void PhysicalInput::deinit()
{
    // 调用 stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
    stop();
    // 调用 deinit()：创建和销毁队列。
    dip_.deinit();
}

// 定义或声明函数 PhysicalInput::start：参数在括号里，返回类型写在函数名前面。
bool PhysicalInput::start()
{
    // 判断条件：running_ 是否成立。
    if (running_)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    // 判断条件：!dip_.is_init() 是否成立。
    if (!dip_.is_init())
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 执行语句：::app::sys::task::Cfg cfg;；它会更新一次变量、对象或程序状态。
    ::app::sys::task::Cfg cfg;
    // 赋值：把右侧 "ebook_dip" 的结果保存到左侧 cfg.name。
    cfg.name = "ebook_dip";
    // 赋值：把右侧 kStackBytes 的结果保存到左侧 cfg.stack_size。
    cfg.stack_size = kStackBytes;
    // 赋值：把右侧 ::app::sys::task::Priority::NORMAL 的结果保存到左侧 cfg.priority。
    cfg.priority = ::app::sys::task::Priority::NORMAL;
    // 赋值：把右侧 tskNO_AFFINITY 的结果保存到左侧 cfg.core_id。
    cfg.core_id = tskNO_AFFINITY;

    // 赋值：把右侧 true 的结果保存到左侧 running_。
    running_ = true;
    // 执行这条代码：task_ = std::make_unique<::app::sys::task::Task>(。它会更新变量、对象状态或调用链。
    task_ = std::make_unique<::app::sys::task::Task>(
        // 继续上一行的表达式或参数列表：[](void* arg) { static_cast<PhysicalInput*>(arg)->task_main(); }, cfg, this);。
        [](void* arg) { static_cast<PhysicalInput*>(arg)->task_main(); }, cfg, this);
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
    // 调用 EBOOK_LOGI(TAG, "started");，执行这个函数并处理它的返回值或状态变化。
    EBOOK_LOGI(TAG, "started");
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数定义/声明：调用 PhysicalInput::stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
void PhysicalInput::stop()
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

// 函数定义/声明：调用 PhysicalInput::poll_once()：init() 初始化 ADC 拨码驱动并清空消抖状态。pollonce() 读取当前档位，连续三次一致才改变稳定状态；稳定状态变成非 NONE 时发 Press，变成 NONE 时发 Release。taskmain() 每 25 ms 轮询一次。。
void PhysicalInput::poll_once()
{
    // 定义变量 DipId raw 并初始化为 DipId::NONE。
    DipId raw = DipId::NONE;
    // 判断条件：dip_.read(DipId::UP) 是否成立。
    if (dip_.read(DipId::UP))
        // 赋值：把右侧 DipId::UP 的结果保存到左侧 raw。
        raw = DipId::UP;
    // 备用分支：前面的条件不成立时执行这里。
    else if (dip_.read(DipId::MID))
        // 赋值：把右侧 DipId::MID 的结果保存到左侧 raw。
        raw = DipId::MID;
    // 备用分支：前面的条件不成立时执行这里。
    else if (dip_.read(DipId::DOWN))
        // 赋值：把右侧 DipId::DOWN 的结果保存到左侧 raw。
        raw = DipId::DOWN;

    // 判断条件：raw == stable_ 是否成立。
    if (raw == stable_)
    {
        // 赋值：把右侧 DipId::NONE 的结果保存到左侧 pending_。
        pending_ = DipId::NONE;
        // 赋值：把右侧 0 的结果保存到左侧 pending_count_。
        pending_count_ = 0;
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：raw == pending_ 是否成立。
    if (raw == pending_)
    {
        // 判断条件：pending_count_ < 255 是否成立。
        if (pending_count_ < 255)
            // 执行语句：++pending_count_;；它会更新一次变量、对象或程序状态。
            ++pending_count_;
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 赋值：把右侧 raw 的结果保存到左侧 pending_。
        pending_ = raw;
        // 赋值：把右侧 1 的结果保存到左侧 pending_count_。
        pending_count_ = 1;
    }

    // 判断条件：pending_count_ < kDebounceSamples 是否成立。
    if (pending_count_ < kDebounceSamples)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const DipId prev = stable_;
    // 赋值：把右侧 pending_ 的结果保存到左侧 stable_。
    stable_ = pending_;
    // 赋值：把右侧 0 的结果保存到左侧 pending_count_。
    pending_count_ = 0;

    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const uint32_t t = now_ms();
    // 判断条件：t - last_emit_ms_ < kCooldownMs 是否成立。
    if (t - last_emit_ms_ < kCooldownMs)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 判断条件：stable_ != DipId::NONE 是否成立。
    if (stable_ != DipId::NONE)
    {
        // 调用 EBOOK_LOGD(TAG, "press %s", physical_key_name(map_key(stable_)));，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGD(TAG, "press %s", physical_key_name(map_key(stable_)));
        // 调用 post_physical(map_key(stable_), PhysicalAction::Press);，执行这个函数并处理它的返回值或状态变化。
        post_physical(map_key(stable_), PhysicalAction::Press);
        // 赋值：把右侧 t 的结果保存到左侧 last_emit_ms_。
        last_emit_ms_ = t;
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：prev != DipId::NONE 是否成立。
    if (prev != DipId::NONE)
    {
        // 调用 EBOOK_LOGD(TAG, "release %s", physical_key_name(map_key(prev)));，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGD(TAG, "release %s", physical_key_name(map_key(prev)));
        // 调用 post_physical(map_key(prev), PhysicalAction::Release);，执行这个函数并处理它的返回值或状态变化。
        post_physical(map_key(prev), PhysicalAction::Release);
        // 赋值：把右侧 t 的结果保存到左侧 last_emit_ms_。
        last_emit_ms_ = t;
    }
}

// 定义或声明函数 PhysicalInput::task_main：参数在括号里，返回类型写在函数名前面。
void PhysicalInput::task_main()
{
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (running_)
    {
        // 调用 poll_once()：init() 初始化 ADC 拨码驱动并清空消抖状态。pollonce() 读取当前档位，连续三次一致才改变稳定状态；稳定状态变成非 NONE 时发 Press，变成 NONE 时发 Release。taskmain() 每 25 ms 轮询一次。。
        poll_once();
        // 调用 app::sys::task::TaskMgr::delay_ms(kPollMs)：调用 vTaskDelay()，这是让出 CPU 的阻塞延时。一千毫秒延时期间，任务不占 CPU，调度器可以运行别人。。
        ::app::sys::task::TaskMgr::delay_ms(kPollMs);
    }
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::input
