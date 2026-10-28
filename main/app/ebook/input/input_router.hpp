#pragma once

/**
 * 文件：main/app/ebook/input/input_router.hpp
 *
 * InputRouter 是触摸链路的调度器：它把"硬件中断 / 触摸驱动"和
 * "手势状态机 / UI 队列"粘在一起。
 *
 * 分工：
 *   - Ft6336u          ：驱动，负责 I2C 读寄存器、开关中断。
 *   - TouchGesture     ：状态机，负责把一串坐标变成 Tap / Swipe 等手势。
 *   - InputRouter      ：任务，负责"等中断 -> 取点 -> 收事件 -> 投给 UI"。
 *   - UiBus            ：队列，负责把事件安全地交给 UI 线程。
 *
 * 想改"触摸灵敏度"看 input/gesture_tuner.*，
 * 想改"什么算点击 / 滑动"看 bsp/driver/ft6336u/touch_gesture.*，
 * 想改"采集节奏"看本文件 task_main。
 */

#include <memory>

#include "ft6336u.hpp"
#include "system/task/task.hpp"
#include "touch_gesture.hpp"

#include "input/gesture_tuner.hpp"
#include "input/input_event.hpp"

namespace app::ebook::input {

/**
 * 触摸采集任务。
 *
 * 一句话：等中断 -> 读触摸 -> 识别手势 -> 投递到 UiBus。
 * 它不直接改页面，页面由 UI 线程处理 UiBus 里的消息时再改。
 */
class InputRouter
{
  public:
    // 单例：整个程序只有一个触摸源。
    static InputRouter& get_instance();

    // 绑定触摸驱动并初始化手势状态机。必须在 start() 之前调用。
    bool init(::app::bsp::driver::ft6336u::Ft6336u* touch);
    void deinit();

    // 创建 / 销毁采集任务。重复 start() 是安全的。
    bool start();
    void stop();

    // 切换手势灵敏度档位（Normal / Reader 等，见 gesture_tuner.hpp）。
    // 注意：切换会重置手势状态，不要在手指按住屏幕时调用。
    void set_profile(Profile p);

    // 主要给调试面板用，可以直接看/改底层的状态机。
    ::app::bsp::driver::ft6336u::TouchGesture& gesture()
    {
        return gesture_;
    }

  private:
    InputRouter() = default;
    ~InputRouter() = default;
    InputRouter(const InputRouter&) = delete;
    InputRouter& operator=(const InputRouter&) = delete;

    // 任务主体，见 .cc 里的说明。
    void task_main();

    // 把状态机里攒的事件全部投到 UiBus。
    void drain_events();

    // 芯片已报告无触点、但状态机还认为按着时，补一次空采样完成释放。
    void process_release();

    // 没有触摸时的最长等待时间（毫秒）。太大会让首次触摸感觉迟钝，
    // 太小会白白唤醒 CPU 耗电。
    static constexpr uint32_t kIdlePollMs = 20;

    // 手指按住时的采样合并窗口（毫秒）。触摸芯片速度远快于屏幕，
    // 不需要每个中断都完整处理一遍。
    static constexpr uint32_t kCoalesceMs = 12;

    // 任务栈大小（字节）。这个任务调用了 I2C 和手势计算，
    // 栈给小了会在运行时崩溃。
    static constexpr uint32_t kStackBytes = 4096;

    ::app::bsp::driver::ft6336u::Ft6336u* dev_{nullptr};  // 不拥有，只借用
    ::app::bsp::driver::ft6336u::TouchGesture gesture_;
    GestureConfig cfg_{};
    std::unique_ptr<::app::sys::task::Task> task_;
    bool running_{false};
};

} // namespace app::ebook::input
