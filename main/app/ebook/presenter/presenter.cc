/**
 * [初学者导读] 文件：main/app/ebook/presenter/presenter.cc
 * 作用：调度墨水屏刷新请求、合成 framebuffer 并等待显示任务完成。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：Presenter 队列有容量限制，提交失败要调用方处理。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "presenter/presenter.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_heap_caps.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "bsp/driver/gdey027t91/framebuffer.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "composer/composer.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/geometry.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "display/display_port.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "presenter/present_plan.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/refresh_intent.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/transition.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "Presenter";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::presenter {

// 定义或声明函数 Presenter::instance：参数在括号里，返回类型写在函数名前面。
Presenter& Presenter::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static Presenter s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// [初学者导读] Presenter::init()：分配后备帧和同步对象。详见 docs/代码详解-初学者版.md。
bool Presenter::init()
{
    // 判断条件：back_fb_ != nullptr 是否成立。
    if (back_fb_ != nullptr)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;

    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    back_fb_ = static_cast<uint8_t*>(
        // 继续上一行的表达式或参数列表：heap_caps_malloc(core::kFbBytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));。
        heap_caps_malloc(core::kFbBytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    // 判断条件：back_fb_ == nullptr 是否成立。
    if (back_fb_ == nullptr)
        // 赋值：把右侧 static_cast<uint8_t*>(heap_caps_malloc(core::kFbBytes, MALLOC_CAP_8BIT)) 的结果保存到左侧 back_fb_。
        back_fb_ = static_cast<uint8_t*>(heap_caps_malloc(core::kFbBytes, MALLOC_CAP_8BIT));
    // 判断条件：back_fb_ == nullptr 是否成立。
    if (back_fb_ == nullptr)
    {
        // 调用 EBOOK_LOGE(TAG, "alloc back_fb failed");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGE(TAG, "alloc back_fb failed");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 调用 std::memset(back_fb_, 0xFF, core::kFbBytes);，执行这个函数并处理它的返回值或状态变化。
    std::memset(back_fb_, 0xFF, core::kFbBytes);

    // 赋值：把右侧 xQueueCreate(kQueueDepth, sizeof(FrameRequest)) 的结果保存到左侧 queue_。
    queue_ = xQueueCreate(kQueueDepth, sizeof(FrameRequest));
    // 赋值：把右侧 xSemaphoreCreateMutex() 的结果保存到左侧 fb_mutex_。
    fb_mutex_ = xSemaphoreCreateMutex();
    // 赋值：把右侧 xEventGroupCreate() 的结果保存到左侧 state_evt_。
    state_evt_ = xEventGroupCreate();
    // 判断条件：queue_ == nullptr || fb_mutex_ == nullptr || state_evt_ == nullptr 是否成立。
    if (queue_ == nullptr || fb_mutex_ == nullptr || state_evt_ == nullptr)
    {
        // 调用 deinit()：创建和销毁队列。
        deinit();
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 调用 xEventGroupSetBits(state_evt_, kBitIdle);，执行这个函数并处理它的返回值或状态变化。
    xEventGroupSetBits(state_evt_, kBitIdle);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 Presenter::deinit：参数在括号里，返回类型写在函数名前面。
void Presenter::deinit()
{
    // 调用 stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
    stop();
    // 判断条件：queue_ != nullptr 是否成立。
    if (queue_ != nullptr)
    {
        // 调用 vQueueDelete(queue_);，执行这个函数并处理它的返回值或状态变化。
        vQueueDelete(queue_);
        // 赋值：把右侧 nullptr 的结果保存到左侧 queue_。
        queue_ = nullptr;
    }
    // 判断条件：fb_mutex_ != nullptr 是否成立。
    if (fb_mutex_ != nullptr)
    {
        // 调用 vSemaphoreDelete(fb_mutex_);，执行这个函数并处理它的返回值或状态变化。
        vSemaphoreDelete(fb_mutex_);
        // 赋值：把右侧 nullptr 的结果保存到左侧 fb_mutex_。
        fb_mutex_ = nullptr;
    }
    // 判断条件：state_evt_ != nullptr 是否成立。
    if (state_evt_ != nullptr)
    {
        // 调用 vEventGroupDelete(state_evt_);，执行这个函数并处理它的返回值或状态变化。
        vEventGroupDelete(state_evt_);
        // 赋值：把右侧 nullptr 的结果保存到左侧 state_evt_。
        state_evt_ = nullptr;
    }
    // 判断条件：back_fb_ != nullptr 是否成立。
    if (back_fb_ != nullptr)
    {
        // 调用 heap_caps_free(back_fb_);，执行这个函数并处理它的返回值或状态变化。
        heap_caps_free(back_fb_);
        // 赋值：把右侧 nullptr 的结果保存到左侧 back_fb_。
        back_fb_ = nullptr;
    }
}

// [初学者导读] Presenter::start()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
bool Presenter::start()
{
    // 判断条件：started_ 是否成立。
    if (started_)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    // 判断条件：back_fb_ == nullptr || queue_ == nullptr 是否成立。
    if (back_fb_ == nullptr || queue_ == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 true 的结果保存到左侧 running_。
    running_ = true;
    // 执行语句：::app::sys::task::Cfg cfg;；它会更新一次变量、对象或程序状态。
    ::app::sys::task::Cfg cfg;
    // 赋值：把右侧 "ebook_present" 的结果保存到左侧 cfg.name。
    cfg.name = "ebook_present";
    // 赋值：把右侧 kStackBytes 的结果保存到左侧 cfg.stack_size。
    cfg.stack_size = kStackBytes;
    // 赋值：把右侧 kPriority 的结果保存到左侧 cfg.priority。
    cfg.priority = kPriority;

    // 执行这条代码：task_ = std::make_unique<::app::sys::task::Task>(。它会更新变量、对象状态或调用链。
    task_ = std::make_unique<::app::sys::task::Task>(
        // 继续上一行的表达式或参数列表：[](void* arg) { static_cast<Presenter*>(arg)->task_main(); }, cfg, this);。
        [](void* arg) { static_cast<Presenter*>(arg)->task_main(); }, cfg, this);
    // 判断条件：!task_->start() 是否成立。
    if (!task_->start())
    {
        // 赋值：把右侧 false 的结果保存到左侧 running_。
        running_ = false;
        // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
        task_.reset();
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 赋值：把右侧 true 的结果保存到左侧 started_。
    started_ = true;
    // 调用 EBOOK_LOGI(TAG, "started");，执行这个函数并处理它的返回值或状态变化。
    EBOOK_LOGI(TAG, "started");
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数定义/声明：调用 Presenter::stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
void Presenter::stop()
{
    // 判断条件：!started_ 是否成立。
    if (!started_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 false 的结果保存到左侧 running_。
    running_ = false;
    // 声明并默认初始化对象：FrameRequest sentinel{}。
    FrameRequest sentinel{};
    // 调用 xQueueSend(queue_, &sentinel, pdMS_TO_TICKS(50));，执行这个函数并处理它的返回值或状态变化。
    xQueueSend(queue_, &sentinel, pdMS_TO_TICKS(50));
    // 判断条件：task_ 是否成立。
    if (task_)
    {
        // 调用 destroy()：直接删除，不是优雅停止。
        task_->destroy();
        // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
        task_.reset();
    }
    // 赋值：把右侧 false 的结果保存到左侧 started_。
    started_ = false;
}

// 定义或声明函数 Presenter::back_fb：参数在括号里，返回类型写在函数名前面。
uint8_t* Presenter::back_fb()
{
    // 返回 back_fb_ 给调用者，表示当前函数的执行结果。
    return back_fb_;
}

// 函数定义/声明：调用 Presenter::wait_idle()：等待 idle bit。它的语义是“上一次执行是否已经完成”。App 在请求高频重绘前用它避免堆积请求。。
bool Presenter::wait_idle(uint32_t timeout_ms)
{
    // 判断条件：state_evt_ == nullptr 是否成立。
    if (state_evt_ == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const TickType_t to = (timeout_ms == portMAX_DELAY) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const EventBits_t bits = xEventGroupWaitBits(state_evt_, kBitIdle, pdFALSE, pdTRUE, to);
    // 返回 (bits & kBitIdle) != 0 给调用者，表示当前函数的执行结果。
    return (bits & kBitIdle) != 0;
}

// [初学者导读] Presenter::copy_fb()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void Presenter::copy_fb()
{
    // 赋值：把右侧 display::DisplayPort::instance().framebuffer() 的结果保存到左侧 uint8_t* front。
    uint8_t* front = display::DisplayPort::instance().framebuffer();
    // 判断条件：front == nullptr || back_fb_ == nullptr 是否成立。
    if (front == nullptr || back_fb_ == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 调用 std::memcpy(front, back_fb_, core::kFbBytes);，执行这个函数并处理它的返回值或状态变化。
    std::memcpy(front, back_fb_, core::kFbBytes);
}

// 定义或声明函数 Presenter::run_plan：参数在括号里，返回类型写在函数名前面。
core::Status Presenter::run_plan(const PresentPlan& plan)
{
    // 赋值：把右侧 display::DisplayPort::instance() 的结果保存到左侧 auto& port。
    auto& port = display::DisplayPort::instance();
    // 定义变量 core::Status last 并初始化为 core::Status::Ok。
    core::Status last = core::Status::Ok;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < plan.count; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const PresentStep& s = plan.steps[i];
        // 多分支判断：根据括号里的值选择不同 case。
        switch (s.op)
        {
            // 分支匹配：case PresentOp::Bootstrap 成立时执行下面的代码。
            case PresentOp::Bootstrap:
                // 调用 bootstrap()：DisplayPort 是 Presenter 和具体屏幕驱动之间的适配层。它把核心层的 Rect 转成驱动的 Window，把驱动的 bool 结果转成 core::Status，并管理 session 状态。。
                last = port.bootstrap();
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 分支匹配：case PresentOp::Partial 成立时执行下面的代码。
            case PresentOp::Partial:
                // 调用 partial(s.rect)：DisplayPort 是 Presenter 和具体屏幕驱动之间的适配层。它把核心层的 Rect 转成驱动的 Window，把驱动的 bool 结果转成 core::Status，并管理 session 状态。。
                last = port.partial(s.rect);
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 分支匹配：case PresentOp::Fast 成立时执行下面的代码。
            case PresentOp::Fast:
                // 调用 fast()：DisplayPort 是 Presenter 和具体屏幕驱动之间的适配层。它把核心层的 Rect 转成驱动的 Window，把驱动的 bool 结果转成 core::Status，并管理 session 状态。。
                last = port.fast();
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 分支匹配：case PresentOp::Full 成立时执行下面的代码。
            case PresentOp::Full:
                // 调用 full()：DisplayPort 是 Presenter 和具体屏幕驱动之间的适配层。它把核心层的 Rect 转成驱动的 Window，把驱动的 bool 结果转成 core::Status，并管理 session 状态。。
                last = port.full();
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
        }
        // 判断条件：last != core::Status::Ok 是否成立。
        if (last != core::Status::Ok)
            // 返回 last 给调用者，表示当前函数的执行结果。
            return last;
    }
    // 返回 last 给调用者，表示当前函数的执行结果。
    return last;
}

// 函数定义/声明：调用 Presenter::execute()：按计划刷新。
core::Status Presenter::execute(const FrameRequest& req)
{
    // 赋值：把右侧 display::DisplayPort::instance() 的结果保存到左侧 auto& port。
    auto& port = display::DisplayPort::instance();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool can_partial = (port.session() == display::SessionState::PartialReady);

    // 定义变量并调用函数初始化：调用 make_plan(req.intent,can_partial)：波形到步骤，结果保存到 PresentPlan plan。
    PresentPlan plan = make_plan(req.intent, can_partial);

    // 调用 copy_fb()：把后备 framebuffer 复制到 DisplayPort::framebuffer()，即屏幕驱动的 framebuffer。复制发生在锁内，避免 UI 合成和刷新任务同时改同一块内存。。
    copy_fb();
    // 定义变量 core::Status st 并初始化为 run_plan(plan)。
    core::Status st = run_plan(plan);
    // 判断条件：st == core::Status::Ok 是否成立。
    if (st == core::Status::Ok)
        // 返回 st 给调用者，表示当前函数的执行结果。
        return st;

    // 调用 EBOOK_LOGW(TAG, "seq=%u present failed intent=%s", static_cast<unsigned>(req.seq),，执行这个函数并处理它的返回值或状态变化。
    EBOOK_LOGW(TAG, "seq=%u present failed intent=%s", static_cast<unsigned>(req.seq),
               // 继续上一行的表达式或参数列表：router::waveform_name(req.intent.waveform));。
               router::waveform_name(req.intent.waveform));

    // 判断条件：req.intent.waveform != router::Waveform::Partial 是否成立。
    if (req.intent.waveform != router::Waveform::Partial)
        // 返回 st 给调用者，表示当前函数的执行结果。
        return st;

    // 访问对象成员：port.invalidate_session();；读取或修改这个对象里的成员状态。
    port.invalidate_session();
    // 调用 make_plan(req.intent,false)：波形到步骤。
    plan = make_plan(req.intent, false);
    // 调用 EBOOK_LOGW(TAG, "seq=%u partial failed, recover bootstrap", static_cast<unsigned>(req.seq));，执行这个函数并处理它的返回值或状态变化。
    EBOOK_LOGW(TAG, "seq=%u partial failed, recover bootstrap", static_cast<unsigned>(req.seq));
    // 赋值：把右侧 run_plan(plan) 的结果保存到左侧 st。
    st = run_plan(plan);
    // 返回 st 给调用者，表示当前函数的执行结果。
    return st;
}

// 函数定义/声明：调用 Presenter::submit()：从 UI 线程提交一帧。
core::Status Presenter::submit(const FrameRequest& req)
{
    // 判断条件：!started_ 是否成立。
    if (!started_)
        // 返回 core::Status::NotInit 给调用者，表示当前函数的执行结果。
        return core::Status::NotInit;

    // 调用 wait_idle(kSubmitTmoMs)：等待 idle bit。它的语义是“上一次执行是否已经完成”。App 在请求高频重绘前用它避免堆积请求。。
    (void)wait_idle(kSubmitTmoMs);

    // 判断条件：xSemaphoreTake(fb_mutex_, pdMS_TO_TICKS(kSubmitTmoMs)) != pdTRUE 是否成立。
    if (xSemaphoreTake(fb_mutex_, pdMS_TO_TICKS(kSubmitTmoMs)) != pdTRUE)
        // 返回 core::Status::Timeout 给调用者，表示当前函数的执行结果。
        return core::Status::Timeout;

    // 调用 composer::Composer::instance().paint(back_fb_);，执行这个函数并处理它的返回值或状态变化。
    composer::Composer::instance().paint(back_fb_);

    // 判断条件：xQueueSend(queue_, &req, 0) != pdTRUE 是否成立。
    if (xQueueSend(queue_, &req, 0) != pdTRUE)
    {
        // 调用 xSemaphoreGive(fb_mutex_);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGive(fb_mutex_);
        // 返回 core::Status::Timeout 给调用者，表示当前函数的执行结果。
        return core::Status::Timeout;
    }

    // 调用 xEventGroupClearBits(state_evt_, kBitIdle);，执行这个函数并处理它的返回值或状态变化。
    xEventGroupClearBits(state_evt_, kBitIdle);
    // 调用 xSemaphoreGive(fb_mutex_);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGive(fb_mutex_);
    // 返回 core::Status::Ok 给调用者，表示当前函数的执行结果。
    return core::Status::Ok;
}

// [初学者导读] Presenter::task_main()：只执行最新请求。详见 docs/代码详解-初学者版.md。
void Presenter::task_main()
{
    // 声明并默认初始化对象：FrameRequest req{}。
    FrameRequest req{};
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (running_)
    {
        // 判断条件：xQueueReceive(queue_, &req, portMAX_DELAY) != pdTRUE 是否成立。
        if (xQueueReceive(queue_, &req, portMAX_DELAY) != pdTRUE)
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        // 判断条件：!running_ 是否成立。
        if (!running_)
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 声明并默认初始化对象：FrameRequest extra{}。
        FrameRequest extra{};
        // 循环执行：条件为真时重复运行，条件变化后退出。
        while (xQueueReceive(queue_, &extra, 0) == pdTRUE)
            // 赋值：把右侧 extra 的结果保存到左侧 req。
            req = extra;

        // 调用 xSemaphoreTake(fb_mutex_, portMAX_DELAY);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreTake(fb_mutex_, portMAX_DELAY);
        // 调用 execute(req)：按计划刷新。
        (void)execute(req);
        // 判断条件：uxQueueMessagesWaiting(queue_) == 0 是否成立。
        if (uxQueueMessagesWaiting(queue_) == 0)
            // 调用 xEventGroupSetBits(state_evt_, kBitIdle);，执行这个函数并处理它的返回值或状态变化。
            xEventGroupSetBits(state_evt_, kBitIdle);
        // 调用 xSemaphoreGive(fb_mutex_);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGive(fb_mutex_);
    }
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::presenter
