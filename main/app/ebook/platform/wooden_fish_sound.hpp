// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/platform/wooden_fish_sound.hpp
 * 作用：加载并播放电子木鱼音效。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：阻塞工作放独立任务，周期小检查放 Housekeeper。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstddef>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <memory>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/task/task.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::platform {

/** @brief 木鱼敲击短音（启动期预加载 PCM；play_knock 仅 write） */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class WoodenFishSound
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static WoodenFishSound& get_instance();

    // 函数调用：bool init();；执行该函数并处理它的返回值或副作用。
    bool init();
    // 定义或声明函数 ready：参数在括号里，返回类型写在函数名前面。
    bool ready() const
    {
        // 返回 ready_ 给调用者，表示当前函数的执行结果。
        return ready_;
    }

    /** 静音/音乐播放中/上一段未播完时跳过 */
    // 函数调用：bool play_knock();；执行该函数并处理它的返回值或副作用。
    bool play_knock();

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 WoodenFishSound() = default;，执行这个函数并处理它的返回值或状态变化。
    WoodenFishSound() = default;

    // 函数调用：void run_play();；执行该函数并处理它的返回值或副作用。
    void run_play();
    // 函数调用：void clear_play_task();；执行该函数并处理它的返回值或副作用。
    void clear_play_task();

    // 执行语句：bool ready_{false};；它会更新一次变量、对象或程序状态。
    bool ready_{false};
    // 执行语句：uint32_t dst_rate_{16000};；它会更新一次变量、对象或程序状态。
    uint32_t dst_rate_{16000};
    // 执行语句：int16_t* pcm_{nullptr};；它会更新一次变量、对象或程序状态。
    int16_t* pcm_{nullptr};
    // 执行语句：size_t pcm_count_{0};；它会更新一次变量、对象或程序状态。
    size_t pcm_count_{0};

    // 易变变量：告诉编译器该值可能被中断或硬件改变，不能随意优化读取。
    volatile bool play_task_alive_{false};
    // 执行语句：std::unique_ptr<::app::sys::task::Task> play_task_;；它会更新一次变量、对象或程序状态。
    std::unique_ptr<::app::sys::task::Task> play_task_;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::platform
