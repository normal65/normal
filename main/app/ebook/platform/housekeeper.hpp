// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/platform/housekeeper.hpp
 * 作用：周期更新时钟、电池、自动锁屏和存储热插拔状态。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：阻塞工作放独立任务，周期小检查放 Housekeeper。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <memory>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/task/task.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::platform {

/** @brief 1s 周期任务：时钟/电池 tick、SD 热插拔、闹钟、AutoLock（经 UiBus，不直接改 UI） */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class Housekeeper
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static Housekeeper& get_instance();

    // 调用 start()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
    bool start();
    // 调用 stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
    void stop();

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 Housekeeper() = default;，执行这个函数并处理它的返回值或状态变化。
    Housekeeper() = default;
    // 赋值：把右侧 default 的结果保存到左侧 ~Housekeeper()。
    ~Housekeeper() = default;

    // 函数调用：void run();；执行该函数并处理它的返回值或副作用。
    void run();

    // 固定参数 kPeriodMs = 1000：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint32_t kPeriodMs = 1000;
    // 固定参数 kBatteryEveryTicks = 5：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint32_t kBatteryEveryTicks = 5;
    // 任务栈大小：4096，栈太小会崩溃，太大会浪费 RAM。
    static constexpr uint32_t kStackBytes = 4096;

    // 执行语句：std::unique_ptr<::app::sys::task::Task> task_;；它会更新一次变量、对象或程序状态。
    std::unique_ptr<::app::sys::task::Task> task_;
    // 执行语句：bool running_{false};；它会更新一次变量、对象或程序状态。
    bool running_{false};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::platform
