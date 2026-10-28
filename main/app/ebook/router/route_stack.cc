/**
 * [初学者导读] 文件：main/app/ebook/router/route_stack.cc
 * 作用：保存 Shell、App 和 overlay 的页面历史，决定返回目标。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：路由先改页面栈，再向 Presenter 提交刷新请求；提交是异步的。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/route_stack.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::router {

// 函数定义/声明：调用 RouteStack::reset_shell()：路由栈保存页面历史。resetshell() 清空 Shell 和 overlay，并把指定 shell 设为栈底；pushshell() 返回 false 表示深度超过 8；popshell() 不允许弹掉最后一个页面。。
void RouteStack::reset_shell(ShellPage page)
{
    // 赋值：把右侧 0 的结果保存到左侧 shell_depth_。
    shell_depth_ = 0;
    // 赋值：把右侧 0 的结果保存到左侧 overlay_depth_。
    overlay_depth_ = 0;
    // 赋值：把右侧 page 的结果保存到左侧 shell_[shell_depth_++]。
    shell_[shell_depth_++] = page;
    // 调用 clear_app()：setapp() 记录当前 App 和 App 内页面。clearapp() 在回到 shell 时清理。current() 优先返回 overlay，其次返回 AppHost 里的 App，最后返回 shell。。
    clear_app();
}

// 函数定义/声明：调用 RouteStack::push_shell()：路由栈保存页面历史。resetshell() 清空 Shell 和 overlay，并把指定 shell 设为栈底；pushshell() 返回 false 表示深度超过 8；popshell() 不允许弹掉最后一个页面。。
bool RouteStack::push_shell(ShellPage page)
{
    // 判断条件：shell_depth_ >= kMaxShellDepth 是否成立。
    if (shell_depth_ >= kMaxShellDepth)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 赋值：把右侧 page 的结果保存到左侧 shell_[shell_depth_++]。
    shell_[shell_depth_++] = page;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数定义/声明：调用 RouteStack::pop_shell()：路由栈保存页面历史。resetshell() 清空 Shell 和 overlay，并把指定 shell 设为栈底；pushshell() 返回 false 表示深度超过 8；popshell() 不允许弹掉最后一个页面。。
bool RouteStack::pop_shell()
{
    // 判断条件：shell_depth_ <= 1 是否成立。
    if (shell_depth_ <= 1)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 执行语句：--shell_depth_;；它会更新一次变量、对象或程序状态。
    --shell_depth_;
    // 判断条件：shell_top() != ShellPage::AppHost 是否成立。
    if (shell_top() != ShellPage::AppHost)
        // 调用 clear_app()：setapp() 记录当前 App 和 App 内页面。clearapp() 在回到 shell 时清理。current() 优先返回 overlay，其次返回 AppHost 里的 App，最后返回 shell。。
        clear_app();
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 RouteStack::shell_top：参数在括号里，返回类型写在函数名前面。
ShellPage RouteStack::shell_top() const
{
    // 返回 (shell_depth_ > 0) ? shell_[shell_depth_ - 1] : ShellPage::Lock 给调用者，表示当前函数的执行结果。
    return (shell_depth_ > 0) ? shell_[shell_depth_ - 1] : ShellPage::Lock;
}

// 定义或声明函数 RouteStack::shell_below：参数在括号里，返回类型写在函数名前面。
ShellPage RouteStack::shell_below() const
{
    // 返回 (shell_depth_ >= 2) ? shell_[shell_depth_ - 2] : ShellPage::Lock 给调用者，表示当前函数的执行结果。
    return (shell_depth_ >= 2) ? shell_[shell_depth_ - 2] : ShellPage::Lock;
}

// 函数定义/声明：调用 RouteStack::push_overlay()：overlay 是覆盖在页面上的临时层，例如控制中心。最多四层，overlaytop() 返回当前最上面一层，没有时返回 None。。
bool RouteStack::push_overlay(OverlayId id)
{
    // 判断条件：overlay_depth_ >= kMaxOverlayDepth 是否成立。
    if (overlay_depth_ >= kMaxOverlayDepth)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 赋值：把右侧 id 的结果保存到左侧 overlay_[overlay_depth_++]。
    overlay_[overlay_depth_++] = id;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数定义/声明：调用 RouteStack::pop_overlay()：overlay 是覆盖在页面上的临时层，例如控制中心。最多四层，overlaytop() 返回当前最上面一层，没有时返回 None。。
bool RouteStack::pop_overlay()
{
    // 判断条件：overlay_depth_ == 0 是否成立。
    if (overlay_depth_ == 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 执行语句：--overlay_depth_;；它会更新一次变量、对象或程序状态。
    --overlay_depth_;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数定义/声明：调用 RouteStack::overlay_top()：overlay 是覆盖在页面上的临时层，例如控制中心。最多四层，overlaytop() 返回当前最上面一层，没有时返回 None。。
OverlayId RouteStack::overlay_top() const
{
    // 返回 (overlay_depth_ > 0) ? overlay_[overlay_depth_ - 1] : OverlayId::None 给调用者，表示当前函数的执行结果。
    return (overlay_depth_ > 0) ? overlay_[overlay_depth_ - 1] : OverlayId::None;
}

// 函数定义/声明：调用 RouteStack::set_app()：setapp() 记录当前 App 和 App 内页面。clearapp() 在回到 shell 时清理。current() 优先返回 overlay，其次返回 AppHost 里的 App，最后返回 shell。。
void RouteStack::set_app(apps::AppId app, uint16_t page)
{
    // 赋值：把右侧 app 的结果保存到左侧 active_app_。
    active_app_ = app;
    // 赋值：把右侧 page 的结果保存到左侧 active_app_page_。
    active_app_page_ = page;
}

// 函数定义/声明：调用 RouteStack::clear_app()：setapp() 记录当前 App 和 App 内页面。clearapp() 在回到 shell 时清理。current() 优先返回 overlay，其次返回 AppHost 里的 App，最后返回 shell。。
void RouteStack::clear_app()
{
    // 赋值：把右侧 apps::AppId::None 的结果保存到左侧 active_app_。
    active_app_ = apps::AppId::None;
    // 赋值：把右侧 0 的结果保存到左侧 active_app_page_。
    active_app_page_ = 0;
}

// 函数定义/声明：调用 RouteStack::current()：setapp() 记录当前 App 和 App 内页面。clearapp() 在回到 shell 时清理。current() 优先返回 overlay，其次返回 AppHost 里的 App，最后返回 shell。。
PageId RouteStack::current() const
{
    // 判断条件：overlay_top() != OverlayId::None 是否成立。
    if (overlay_top() != OverlayId::None)
        // 返回结果：调用 overlay_top()：overlay 是覆盖在页面上的临时层，例如控制中心。最多四层，overlaytop() 返回当前最上面一层，没有时返回 None。。
        return page_overlay(overlay_top());

    // 判断条件：shell_top() == ShellPage::AppHost && active_app_ != apps::AppId::None 是否成立。
    if (shell_top() == ShellPage::AppHost && active_app_ != apps::AppId::None)
        // 返回 page_app(active_app_, active_app_page_) 给调用者，表示当前函数的执行结果。
        return page_app(active_app_, active_app_page_);

    // 返回 page_shell(shell_top()) 给调用者，表示当前函数的执行结果。
    return page_shell(shell_top());
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::router
