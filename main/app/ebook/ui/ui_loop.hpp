// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/ui/ui_loop.hpp
 * 作用：运行 UI 主循环，消费 UiBus 事件并分发给当前页面。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：UI 状态只能在 UiLoop/UI 线程修改，跨任务结果通过 UiBus 回 UI。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <memory>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/input_event.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/task/task.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_event.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::ui {

/** @brief UI 线程：UiBus 消费、输入分发、Tick/WiFi/NTP 路由 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class UiLoop
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static UiLoop& instance();

    // 函数调用：bool init();；执行该函数并处理它的返回值或副作用。
    bool init();
    // 调用 deinit()：创建和销毁队列。
    void deinit();
    // 调用 start()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
    bool start();
    // 调用 stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
    void stop();

    // 定义或声明函数 frame_count：参数在括号里，返回类型写在函数名前面。
    uint32_t frame_count() const
    {
        // 返回 frames_ 给调用者，表示当前函数的执行结果。
        return frames_;
    }

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 UiLoop() = default;，执行这个函数并处理它的返回值或状态变化。
    UiLoop() = default;

    // 函数调用：void run();；执行该函数并处理它的返回值或副作用。
    void run();
    // 调用 handle_event(constUiEvent&ev)：按事件类型分流。
    void handle_event(const UiEvent& ev);
    // 函数调用：void dispatch_input(const ::app::ebook::input::Event& ev);；执行该函数并处理它的返回值或副作用。
    void dispatch_input(const ::app::ebook::input::Event& ev);

    // 任务栈大小：24 * 1024，栈太小会崩溃，太大会浪费 RAM。
    static constexpr uint32_t kStackBytes = 24 * 1024;

    // 执行语句：std::unique_ptr<::app::sys::task::Task> task_;；它会更新一次变量、对象或程序状态。
    std::unique_ptr<::app::sys::task::Task> task_;
    // 执行语句：bool running_{false};；它会更新一次变量、对象或程序状态。
    bool running_{false};
    // 执行语句：uint32_t frames_{0};；它会更新一次变量、对象或程序状态。
    uint32_t frames_{0};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::ui
