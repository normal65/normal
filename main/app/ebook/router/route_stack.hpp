// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/router/route_stack.hpp
 * 作用：保存 Shell、App 和 overlay 的页面历史，决定返回目标。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：路由先改页面栈，再向 Presenter 提交刷新请求；提交是异步的。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app_id.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/page_id.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::router {

/** Shell / App / Overlay 路由栈 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class RouteStack
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 固定参数 kMaxShellDepth = 8：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kMaxShellDepth = 8;
    // 固定参数 kMaxOverlayDepth = 4：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kMaxOverlayDepth = 4;

    // 调用 reset_shell(ShellPagepage)：路由栈保存页面历史。resetshell() 清空 Shell 和 overlay，并把指定 shell 设为栈底；pushshell() 返回 false 表示深度超过 8；popshell() 不允许弹掉最后一个页面。。
    void reset_shell(ShellPage page);
    // 调用 push_shell(ShellPagepage)：路由栈保存页面历史。resetshell() 清空 Shell 和 overlay，并把指定 shell 设为栈底；pushshell() 返回 false 表示深度超过 8；popshell() 不允许弹掉最后一个页面。。
    bool push_shell(ShellPage page);
    // 调用 pop_shell()：路由栈保存页面历史。resetshell() 清空 Shell 和 overlay，并把指定 shell 设为栈底；pushshell() 返回 false 表示深度超过 8；popshell() 不允许弹掉最后一个页面。。
    bool pop_shell();
    // 函数调用：ShellPage shell_top() const;；执行该函数并处理它的返回值或副作用。
    ShellPage shell_top() const;
    // 函数调用：ShellPage shell_below() const;；执行该函数并处理它的返回值或副作用。
    ShellPage shell_below() const;

    // 调用 push_overlay(OverlayIdid)：overlay 是覆盖在页面上的临时层，例如控制中心。最多四层，overlaytop() 返回当前最上面一层，没有时返回 None。。
    bool push_overlay(OverlayId id);
    // 调用 pop_overlay()：overlay 是覆盖在页面上的临时层，例如控制中心。最多四层，overlaytop() 返回当前最上面一层，没有时返回 None。。
    bool pop_overlay();
    // 调用 overlay_top()：overlay 是覆盖在页面上的临时层，例如控制中心。最多四层，overlaytop() 返回当前最上面一层，没有时返回 None。。
    OverlayId overlay_top() const;

    // 函数调用：void set_app(apps::AppId app, uint16_t page);；执行该函数并处理它的返回值或副作用。
    void set_app(apps::AppId app, uint16_t page);
    // 定义或声明函数 active_app：参数在括号里，返回类型写在函数名前面。
    apps::AppId active_app() const
    {
        // 返回 active_app_ 给调用者，表示当前函数的执行结果。
        return active_app_;
    }
    // 定义或声明函数 active_app_page：参数在括号里，返回类型写在函数名前面。
    uint16_t active_app_page() const
    {
        // 返回 active_app_page_ 给调用者，表示当前函数的执行结果。
        return active_app_page_;
    }
    // 调用 clear_app()：setapp() 记录当前 App 和 App 内页面。clearapp() 在回到 shell 时清理。current() 优先返回 overlay，其次返回 AppHost 里的 App，最后返回 shell。。
    void clear_app();

    // 调用 current()：setapp() 记录当前 App 和 App 内页面。clearapp() 在回到 shell 时清理。current() 优先返回 overlay，其次返回 AppHost 里的 App，最后返回 shell。。
    PageId current() const;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 执行语句：ShellPage shell_[kMaxShellDepth]{};；它会更新一次变量、对象或程序状态。
    ShellPage shell_[kMaxShellDepth]{};
    // 执行语句：uint8_t shell_depth_{0};；它会更新一次变量、对象或程序状态。
    uint8_t shell_depth_{0};

    // 执行语句：OverlayId overlay_[kMaxOverlayDepth]{};；它会更新一次变量、对象或程序状态。
    OverlayId overlay_[kMaxOverlayDepth]{};
    // 执行语句：uint8_t overlay_depth_{0};；它会更新一次变量、对象或程序状态。
    uint8_t overlay_depth_{0};

    // 执行语句：apps::AppId active_app_{apps::AppId::None};；它会更新一次变量、对象或程序状态。
    apps::AppId active_app_{apps::AppId::None};
    // 执行语句：uint16_t active_app_page_{0};；它会更新一次变量、对象或程序状态。
    uint16_t active_app_page_{0};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::router
