// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/shell/lock_page.hpp
 * 作用：实现 shell 模块中的 lock page 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：Shell 是 App 之外的外壳页面，负责统一返回和输入兜底。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "shell/page.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::shell {

/** 锁屏：时钟、金句、上滑解锁 / PIN */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class LockPage : public Page
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static LockPage& instance();

    // 定义或声明函数 wants_status_bar：参数在括号里，返回类型写在函数名前面。
    bool wants_status_bar() const override
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 函数调用：void paint(gfx::Canvas& canvas) override;；执行该函数并处理它的返回值或副作用。
    void paint(gfx::Canvas& canvas) override;
    // 调用 on_input(const::app::ebook::input::Event&ev)：文件 App 的输入总入口。它处理： 触摸按下、移动、抬起和滑动。 工具栏返回。 列表滚动节流。 弹窗点击优先于底层列表点击。。
    InputResult on_input(const ::app::ebook::input::Event& ev) override;
    // 调用 on_ui_event(constui::UiEvent&ev)：处理 UiBus 消息。文件模块最关心的是 FilesReloadDone：收到它以后，在 UI 线程调用 onbrowseready()。。
    void on_ui_event(const ui::UiEvent& ev) override;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 LockPage() = default;，执行这个函数并处理它的返回值或状态变化。
    LockPage() = default;
    // 执行语句：uint8_t hour_{0};；它会更新一次变量、对象或程序状态。
    uint8_t hour_{0};
    // 执行语句：uint8_t minute_{0};；它会更新一次变量、对象或程序状态。
    uint8_t minute_{0};
    // 执行语句：uint8_t battery_pct_{0};；它会更新一次变量、对象或程序状态。
    uint8_t battery_pct_{0};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::shell
