// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/overlays/toast.hpp
 * 作用：实现 overlays 模块中的 toast 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：覆盖层优先接收输入，打开和关闭会改变路由栈。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <freertos/FreeRTOS.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <freertos/timers.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/canvas.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::overlays {

/** @brief 底部短提示；xTimer 到期经 UiBus ToastExpire 自隐 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class Toast
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static Toast& instance();

    // 调用 show(constchar*text,uint16_tduration_ms=1500)：ensuretimer()：第一次调用创建一次性软件定时器，后续修改周期。。
    void show(const char* text, uint16_t duration_ms = 1500);
    // 调用 hide()：ensuretimer()：第一次调用创建一次性软件定时器，后续修改周期。。
    void hide();

    // 函数定义/声明：调用 visible()：状态栏是否显示由两件事决定： 当前 Shell 页面。 如果当前是 AppHost，还要问活动 App 的 wantsstatusbar()。。
    bool visible() const
    {
        // 返回 visible_ 给调用者，表示当前函数的执行结果。
        return visible_;
    }
    // 函数调用：void paint(gfx::Canvas& canvas);；执行该函数并处理它的返回值或副作用。
    void paint(gfx::Canvas& canvas);

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 Toast() = default;，执行这个函数并处理它的返回值或状态变化。
    Toast() = default;

    // 固定参数 kMaxLen = 60：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr size_t kMaxLen = 60;

    // 静态函数 timer_cb：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void timer_cb(TimerHandle_t t);
    // 调用 ensure_timer(uint32_tduration_ms)：ensuretimer()：第一次调用创建一次性软件定时器，后续修改周期。。
    void ensure_timer(uint32_t duration_ms);

    // 执行语句：char text_[kMaxLen + 1]{};；它会更新一次变量、对象或程序状态。
    char text_[kMaxLen + 1]{};
    // 执行语句：bool visible_{false};；它会更新一次变量、对象或程序状态。
    bool visible_{false};
    // 执行语句：TimerHandle_t timer_{nullptr};；它会更新一次变量、对象或程序状态。
    TimerHandle_t timer_{nullptr};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::overlays
