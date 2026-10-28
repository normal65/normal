// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/input/physical_input.hpp
 * 作用：运行物理按键任务并产生按键事件。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：中断和输入任务不能直接修改页面，只能投递结果。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <memory>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "button.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/task/task.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::input {

/**
 * @brief 拨码采集任务：ADC 消抖 + 边沿检测 → UiBus::post_physical
 *
 * 禁止在 App / UiLoop 外直接读 DipSwitch。
 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class PhysicalInput
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static PhysicalInput& get_instance();

    // 函数调用：bool init();；执行该函数并处理它的返回值或副作用。
    bool init();
    // 调用 deinit()：创建和销毁队列。
    void deinit();
    // 调用 start()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
    bool start();
    // 调用 stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
    void stop();

    // 定义或声明函数 is_ready：参数在括号里，返回类型写在函数名前面。
    bool is_ready() const
    {
        // 返回 dip_.is_init() 给调用者，表示当前函数的执行结果。
        return dip_.is_init();
    }

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 PhysicalInput() = default;，执行这个函数并处理它的返回值或状态变化。
    PhysicalInput() = default;

    // 调用 task_main()：init() 初始化 ADC 拨码驱动并清空消抖状态。pollonce() 读取当前档位，连续三次一致才改变稳定状态；稳定状态变成非 NONE 时发 Press，变成 NONE 时发 Release。taskmain() 每 25 ms 轮询一次。。
    void task_main();
    // 调用 poll_once()：init() 初始化 ADC 拨码驱动并清空消抖状态。pollonce() 读取当前档位，连续三次一致才改变稳定状态；稳定状态变成非 NONE 时发 Press，变成 NONE 时发 Release。taskmain() 每 25 ms 轮询一次。。
    void poll_once();

    // 固定参数 kPollMs = 25：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint32_t kPollMs = 25;
    // 采样次数或采样参数：3，用于降低测量抖动。
    static constexpr uint8_t kDebounceSamples = 3;
    // 固定参数 kCooldownMs = 180：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint32_t kCooldownMs = 180;
    // 任务栈大小：3072，栈太小会崩溃，太大会浪费 RAM。
    static constexpr uint32_t kStackBytes = 3072;

    // 声明并默认初始化对象：::app::bsp::driver::button::DipSwitch dip_{}。
    ::app::bsp::driver::button::DipSwitch dip_{};
    // 执行语句：std::unique_ptr<::app::sys::task::Task> task_;；它会更新一次变量、对象或程序状态。
    std::unique_ptr<::app::sys::task::Task> task_;
    // 执行语句：bool running_{false};；它会更新一次变量、对象或程序状态。
    bool running_{false};

    // 执行语句：::app::bsp::driver::button::DipId stable_{::app::bsp::driver::button::DipId::NONE};；它会更新一次变量、对象或程序状态。
    ::app::bsp::driver::button::DipId stable_{::app::bsp::driver::button::DipId::NONE};
    // 执行语句：::app::bsp::driver::button::DipId pending_{::app::bsp::driver::button::DipId::NONE};；它会更新一次变量、对象或程序状态。
    ::app::bsp::driver::button::DipId pending_{::app::bsp::driver::button::DipId::NONE};
    // 执行语句：uint8_t pending_count_{0};；它会更新一次变量、对象或程序状态。
    uint8_t pending_count_{0};
    // 执行语句：uint32_t last_emit_ms_{0};；它会更新一次变量、对象或程序状态。
    uint32_t last_emit_ms_{0};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::input
