// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/presenter/presenter.hpp
 * 作用：调度墨水屏刷新请求、合成 framebuffer 并等待显示任务完成。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：Presenter 队列有容量限制，提交失败要调用方处理。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <memory>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <freertos/FreeRTOS.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <freertos/event_groups.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <freertos/queue.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <freertos/semphr.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/result.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "presenter/frame_request.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "presenter/present_plan.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/task/task.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::presenter {

/** @brief 合成 back_fb 并异步上屏（队列合并；合成在 submit 调用方任务） */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class Presenter
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static Presenter& instance();

    // 函数调用：bool init();；执行该函数并处理它的返回值或副作用。
    bool init();
    // 调用 deinit()：创建和销毁队列。
    void deinit();
    // 调用 start()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
    bool start();
    // 调用 stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
    void stop();

    // 函数调用：uint8_t* back_fb();；执行该函数并处理它的返回值或副作用。
    uint8_t* back_fb();
    // 调用 submit(constFrameRequest&req)：从 UI 线程提交一帧。
    core::Status submit(const FrameRequest& req);
    // 调用 wait_idle(uint32_ttimeout_ms=portMAX_DELAY)：等待 idle bit。它的语义是“上一次执行是否已经完成”。App 在请求高频重绘前用它避免堆积请求。。
    bool wait_idle(uint32_t timeout_ms = portMAX_DELAY);

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 Presenter() = default;，执行这个函数并处理它的返回值或状态变化。
    Presenter() = default;

    // 调用 task_main()：init() 初始化 ADC 拨码驱动并清空消抖状态。pollonce() 读取当前档位，连续三次一致才改变稳定状态；稳定状态变成非 NONE 时发 Press，变成 NONE 时发 Release。taskmain() 每 25 ms 轮询一次。。
    void task_main();
    // 调用 execute(constFrameRequest&req)：按计划刷新。
    core::Status execute(const FrameRequest& req);
    // 调用 copy_fb()：把后备 framebuffer 复制到 DisplayPort::framebuffer()，即屏幕驱动的 framebuffer。复制发生在锁内，避免 UI 合成和刷新任务同时改同一块内存。。
    void copy_fb();
    // 函数调用：core::Status run_plan(const PresentPlan& plan);；执行该函数并处理它的返回值或副作用。
    core::Status run_plan(const PresentPlan& plan);

    // 队列深度：最多缓存 4 个事件。
    static constexpr uint16_t kQueueDepth = 4;
    // 任务栈大小：6 * 1024，栈太小会崩溃，太大会浪费 RAM。
    static constexpr uint32_t kStackBytes = 6 * 1024;
    // 固定参数 kSubmitTmoMs = 5000：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint32_t kSubmitTmoMs = 5000;
    // 任务优先级：数值或枚举决定它相对其他任务的重要程度。
    static constexpr ::app::sys::task::Priority kPriority = ::app::sys::task::Priority::HIGH;
    // 固定参数 kBitIdle = 1U << 0：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr EventBits_t kBitIdle = 1U << 0;

    // 执行语句：QueueHandle_t queue_{nullptr};；它会更新一次变量、对象或程序状态。
    QueueHandle_t queue_{nullptr};
    // 执行语句：SemaphoreHandle_t fb_mutex_{nullptr};；它会更新一次变量、对象或程序状态。
    SemaphoreHandle_t fb_mutex_{nullptr};
    // 执行语句：EventGroupHandle_t state_evt_{nullptr};；它会更新一次变量、对象或程序状态。
    EventGroupHandle_t state_evt_{nullptr};
    // 执行语句：std::unique_ptr<::app::sys::task::Task> task_;；它会更新一次变量、对象或程序状态。
    std::unique_ptr<::app::sys::task::Task> task_;

    // 执行语句：uint8_t* back_fb_{nullptr};；它会更新一次变量、对象或程序状态。
    uint8_t* back_fb_{nullptr};
    // 执行语句：bool running_{false};；它会更新一次变量、对象或程序状态。
    bool running_{false};
    // 执行语句：bool started_{false};；它会更新一次变量、对象或程序状态。
    bool started_{false};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::presenter
