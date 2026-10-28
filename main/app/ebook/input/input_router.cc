/**
 * 文件：main/app/ebook/input/input_router.cc
 *
 * 这个文件负责"从触摸芯片读点，判断成手势，再把手势事件递给 UI"。
 *
 * 完整链路长这样：
 *   手指按下
 *     -> FT6336U 拉低 INT 引脚（硬件中断）
 *     -> 中断服务函数 int_isr_handler 给信号量发一个通知
 *     -> 本文件的高优先级任务 task_main 被唤醒
 *     -> gesture_.poll() 通过 I2C 读出坐标，喂给手势状态机
 *     -> 状态机判断这是 Tap / Swipe / LongPress ...
 *     -> drain_events() 把事件投进 UiBus 队列
 *     -> UI 线程（另一个任务）从队列取走事件，改页面、触发刷新
 *
 * 为什么分成两半？因为中断必须极快返回，不能在里面做 I2C 读写。
 * 中断只"敲门"，真正的重活在任务里做。
 *
 * 为什么要经过 UiBus 而不是直接改界面？因为本文件跑在输入任务里，
 * 而界面归 UI 线程管。两个任务同时改同一份数据会出乱子，
 * 所以要"发消息"而不是"直接动手"。
 */

#include "input/input_router.hpp"

#include "core/log.hpp"
#include "ui/ui_bus.hpp"

static const char* const TAG = "InputRouter";

namespace app::ebook::input {

// 全局唯一的输入路由器。整个程序只有一个触摸源，所以用单例。
InputRouter& InputRouter::get_instance()
{
    static InputRouter s;
    return s;
}

// 初始化：拿到触摸驱动对象，准备手势识别配置，并尽量打开硬件中断。
// 返回 false 表示触摸驱动没准备好，后面 start() 就没必要跑了。
bool InputRouter::init(::app::bsp::driver::ft6336u::Ft6336u* touch)
{
    if (touch == nullptr || !touch->is_init())
    {
        EBOOK_LOGE(TAG, "touch device not ready");
        return false;
    }

    dev_ = touch;

    // 默认用 Normal 档：手势阈值按"日常点击"调，不灵敏也不迟钝。
    cfg_ = make_config(Profile::Normal);

    // 打开触摸芯片的中断输出。如果驱动里已经开了就跳过。
    // 即使这里失败也不致命：没有中断就退化成定时轮询，只是更耗电。
    if (!dev_->interrupt_enabled() && !dev_->enable_interrupt())
        EBOOK_LOGW(TAG, "INT not enabled");

    if (!gesture_.init(dev_, &cfg_))
    {
        EBOOK_LOGE(TAG, "gesture init failed");
        return false;
    }
    return true;
}

void InputRouter::deinit()
{
    stop();
    dev_ = nullptr;
}

// 创建并启动采集任务。这个任务优先级设成 REALTIME，
// 因为触摸延迟用户能直接感觉到，不能让给别的任务太久。
bool InputRouter::start()
{
    if (running_)
        return true;

    ::app::sys::task::Cfg cfg;
    cfg.name = "ebook_touch";
    cfg.stack_size = kStackBytes;
    cfg.priority = ::app::sys::task::Priority::REALTIME;
    cfg.core_id = tskNO_AFFINITY;  // 两个核都行，交给调度器决定

    running_ = true;

    // 注意：先设 running_ = true 再创建任务，避免任务一起跑就看到 false 直接退出。
    task_ = std::make_unique<::app::sys::task::Task>(
        [](void* arg) { static_cast<InputRouter*>(arg)->task_main(); }, cfg, this);

    if (!task_->start())
    {
        running_ = false;
        task_.reset();
        EBOOK_LOGE(TAG, "task start failed");
        return false;
    }
    EBOOK_LOGI(TAG, "started");
    return true;
}

void InputRouter::stop()
{
    if (!running_)
        return;

    running_ = false;

    if (task_)
    {
        // destroy() 是直接删任务，不是等它自己收尾。
        // 这里能这么做，是因为 task_main 里没有需要在退出时释放的资源。
        task_->destroy();
        task_.reset();
    }
}

// 切换手势灵敏度档位（例如进入阅读器想更省电/更保守）。
// 换配置后手势状态机会被重置，避免用旧阈值判断到一半。
void InputRouter::set_profile(Profile p)
{
    cfg_ = make_config(p);

    // dev_ 为空说明还没 init，此时只记住配置即可。
    if (dev_ != nullptr)
        gesture_.init(dev_, &cfg_);
}

// 把状态机里攒下的所有手势事件，一个个投递到 UI 队列。
// 用 while 是因为一次采样理论上可能连续产生多条事件（例如先 Press 再 Move）。
void InputRouter::drain_events()
{
    while (gesture_.has_event())
    {
        const Event ev = gesture_.take_event();
        (void)ui::UiBus::get_instance().post_input(ev);
    }
}

// 补一次"手指已离开"的采样。
//
// 场景：状态机以为手指还按着，但触摸芯片已经报告没有触点了。
// 这通常发生在抬手瞬间最后几个中断丢失的时候。
// 这时主动喂一个空采样给状态机，让它走完 finish_contact 流程，
// 把该发的 Tap / Swipe 补发出去。
void InputRouter::process_release()
{
    if (!gesture_.is_pressed() || dev_->is_touched())
        return;

    gesture_.process_sample({});
    drain_events();
}

// 采集任务的主体。
//
// 两种运行状态：
//   情况一：没有触摸时，wait_interrupt 等待，最多等 kIdlePollMs 毫秒。
//           超时了也要 poll 一次，因为可能在等中断的时候刚好错过了一次触发。
//   情况二：有触摸时，不停地读、判断，直到手指离开，
//           中间用 kCoalesceMs 做合并，避免采样太密把 CPU 占满。
void InputRouter::task_main()
{
    while (running_)
    {
        // ── 空闲阶段：等中断，等不到就超时自己醒 ──────────────
        if (!dev_->wait_interrupt(kIdlePollMs))
        {
            gesture_.poll();     // 超时也读一次，防止漏掉事件
            gesture_.tick();     // 推进时间，用来判断长按
            drain_events();
            process_release();   // 补发抬手事件

            // 把这段时间内攒下的中断标记清干净，避免下一轮立刻又醒。
            while (dev_->poll_interrupt())
            {
            }
            continue;
        }

        // ── 触摸阶段：手指还在屏幕上，连续采集 ────────────────
        // do...while 会让同一批中断被合并处理：如果触摸很密，
        // 只消费一次 wait_interrupt，剩下的用 poll_interrupt 快速拿走，
        // 避免每个中断都走一遍完整循环。
        do
        {
            gesture_.poll();
            drain_events();

            while (dev_->poll_interrupt())
            {
                gesture_.poll();
                drain_events();
            }
        } while (dev_->wait_interrupt(kCoalesceMs));

        // 一批采集结束后再做一次收尾，处理抬起或长按。
        gesture_.poll();
        drain_events();
        process_release();

        while (dev_->poll_interrupt())
        {
        }
    }
}

} // namespace app::ebook::input
