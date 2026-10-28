// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/overlays/confirm_prompt.hpp
 * 作用：实现 overlays 模块中的 confirm prompt 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：覆盖层优先接收输入，打开和关闭会改变路由栈。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/canvas.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/input_event.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "shell/page.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::overlays {

/** @brief 是/否确认框（省电等系统提示） */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class ConfirmPrompt
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ChoiceHandler = void (*)(bool accepted, void* user);

    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static ConfirmPrompt& instance();

    // 函数定义/声明：调用 is_open()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
    bool is_open() const
    {
        // 返回 open_ 给调用者，表示当前函数的执行结果。
        return open_;
    }

    // 调用 show(constchar*message,ChoiceHandlerhandler,void*user)：ensuretimer()：第一次调用创建一次性软件定时器，后续修改周期。。
    void show(const char* message, ChoiceHandler handler, void* user);
    // 调用 close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
    void close();

    // 函数调用：void paint(gfx::Canvas& canvas);；执行该函数并处理它的返回值或副作用。
    void paint(gfx::Canvas& canvas);
    // 调用 on_input(const::app::ebook::input::Event&ev)：文件 App 的输入总入口。它处理： 触摸按下、移动、抬起和滑动。 工具栏返回。 列表滚动节流。 弹窗点击优先于底层列表点击。。
    shell::InputResult on_input(const ::app::ebook::input::Event& ev);

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 ConfirmPrompt() = default;，执行这个函数并处理它的返回值或状态变化。
    ConfirmPrompt() = default;

    // 固定参数 kMsgMax = 80：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr size_t kMsgMax = 80;

    // 执行语句：char message_[kMsgMax + 1]{};；它会更新一次变量、对象或程序状态。
    char message_[kMsgMax + 1]{};
    // 执行语句：ChoiceHandler handler_{nullptr};；它会更新一次变量、对象或程序状态。
    ChoiceHandler handler_{nullptr};
    // 执行语句：void* user_{nullptr};；它会更新一次变量、对象或程序状态。
    void* user_{nullptr};
    // 执行语句：bool open_{false};；它会更新一次变量、对象或程序状态。
    bool open_{false};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::overlays
