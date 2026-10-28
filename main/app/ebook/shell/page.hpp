// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/shell/page.hpp
 * 作用：实现 shell 模块中的 page 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：Shell 是 App 之外的外壳页面，负责统一返回和输入兜底。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/canvas.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/input_event.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_event.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::shell {

// struct InputResult：把相关数据字段组合在一起，表示项目中的一个概念。
struct InputResult
{
    // 执行语句：bool consumed{false};；它会更新一次变量、对象或程序状态。
    bool consumed{false};
};

/** Shell / App 页面基类 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class Page
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 赋值：把右侧 default 的结果保存到左侧 virtual ~Page()。
    virtual ~Page() = default;

    // 调用 on_enter()：进入文件 App 时： 1. 重置浏览状态或恢复根卷状态。 2. 清空弹窗。 3. 设置加载状态。 4. 调度目录扫描。 5. 请求重绘。。
    virtual void on_enter() {}
    // 调用 on_exit()：退出时取消未完成的扫描、关闭弹窗，并确保后台任务不会在对象状态失效后继续写数据。。
    virtual void on_exit() {}

    // 定义或声明函数 wants_status_bar：参数在括号里，返回类型写在函数名前面。
    virtual bool wants_status_bar() const
    {
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 赋值：把右侧 0 的结果保存到左侧 virtual void paint(gfx::Canvas& canvas)。
    virtual void paint(gfx::Canvas& canvas) = 0;
    // 定义或声明函数 paint_overlay：参数在括号里，返回类型写在函数名前面。
    virtual void paint_overlay(gfx::Canvas& canvas)
    {
        // 执行语句：(void)canvas;；它会更新一次变量、对象或程序状态。
        (void)canvas;
    }

    // 函数定义/声明：调用 on_input()：1. 检查工具栏：退出、切数字、切字母、切符号、切拼音、完成。 2. 拼音模式下检查候选条。 3. 检查字母区域。 4. 处理 Shift、空格、退格或普通字符。。
    virtual InputResult on_input(const ::app::ebook::input::Event& ev)
    {
        // 执行语句：(void)ev;；它会更新一次变量、对象或程序状态。
        (void)ev;
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};
    }

    // 定义或声明函数 on_ui_event：参数在括号里，返回类型写在函数名前面。
    virtual void on_ui_event(const ui::UiEvent& ev)
    {
        // 执行语句：(void)ev;；它会更新一次变量、对象或程序状态。
        (void)ev;
    }
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::shell
