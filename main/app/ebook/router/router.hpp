// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/router/router.hpp
 * 作用：管理 Shell、App、覆盖层之间的跳转、返回和页面栈变化。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：路由先改页面栈，再向 Presenter 提交刷新请求；提交是异步的。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/result.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/refresh_intent.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/route_stack.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/transition.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::router {

/** @brief 页面路由：栈变更 + RefreshEdges 解析 + 提交 Presenter */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class Router
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static Router& instance();

    // 函数定义/声明：调用 current()：setapp() 记录当前 App 和 App 内页面。clearapp() 在回到 shell 时清理。current() 优先返回 overlay，其次返回 AppHost 里的 App，最后返回 shell。。
    PageId current() const
    {
        // 返回结果：调用 current()：setapp() 记录当前 App 和 App 内页面。clearapp() 在回到 shell 时清理。current() 优先返回 overlay，其次返回 AppHost 里的 App，最后返回 shell。。
        return stack_.current();
    }
    // 只读约束：这个值或对象在当前位置不能被修改。
    const RouteStack& stack() const
    {
        // 返回 stack_ 给调用者，表示当前函数的执行结果。
        return stack_;
    }

    // 函数调用：core::Status navigate(PageId to, NavAction action = NavAction::Forward,；执行该函数并处理它的返回值或副作用。
    core::Status navigate(PageId to, NavAction action = NavAction::Forward,
                          // 只读约束：这个值或对象在当前位置不能被修改。
                          const RefreshIntent* override_intent = nullptr);

    // 调用 back(constRefreshIntent*override_intent=nullptr)：如果最上面是 overlay，优先关闭 overlay。否则计算上一层 shell；如果当前在 AppHost，会先关闭活动 App，再提交返回转换。。
    core::Status back(const RefreshIntent* override_intent = nullptr);
    // 调用 replace_shell(ShellPageshell,constRefreshIntent*override_intent=nullptr)：这些函数只是用明确的参数调用统一导航逻辑，避免每个页面自己拼 Transition。。
    core::Status replace_shell(ShellPage shell, const RefreshIntent* override_intent = nullptr);
    // 函数调用：core::Status repaint(RefreshIntent intent);；执行该函数并处理它的返回值或副作用。
    core::Status repaint(RefreshIntent intent);

    // 调用 open_overlay(OverlayIdid,constRefreshIntent*override_intent=nullptr)：这些函数只是用明确的参数调用统一导航逻辑，避免每个页面自己拼 Transition。。
    core::Status open_overlay(OverlayId id, const RefreshIntent* override_intent = nullptr);
    // 调用 close_overlay(constRefreshIntent*override_intent=nullptr)：这些函数只是用明确的参数调用统一导航逻辑，避免每个页面自己拼 Transition。。
    core::Status close_overlay(const RefreshIntent* override_intent = nullptr);

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 Router() = default;，执行这个函数并处理它的返回值或状态变化。
    Router() = default;

    // 函数调用：core::Status submit_transition(const Transition& t, RefreshIntent intent);；执行该函数并处理它的返回值或副作用。
    core::Status submit_transition(const Transition& t, RefreshIntent intent);
    // 调用 apply_route_change(PageIdto,NavActionaction)：真正改变栈。
    core::Status apply_route_change(PageId to, NavAction action);

    // 声明并默认初始化对象：RouteStack stack_{}。
    RouteStack stack_{};
    // 执行语句：uint32_t seq_{0};；它会更新一次变量、对象或程序状态。
    uint32_t seq_{0};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::router
