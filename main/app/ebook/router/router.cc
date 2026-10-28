/**
 * [初学者导读] 文件：main/app/ebook/router/router.cc
 * 作用：管理 Shell、App、覆盖层之间的跳转、返回和页面栈变化。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：路由先改页面栈，再向 Presenter 提交刷新请求；提交是异步的。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/router.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app_registry.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "presenter/frame_request.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "presenter/presenter.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/refresh_edges.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "shell/app_grid_page.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "Router"; // 日志标签：串口日志里用 Router 区分路由模块。

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::router {

// 获取全局唯一的路由器对象。
Router& Router::instance()
{
    // 函数内静态变量只初始化一次，生命周期持续到程序结束。
    static Router s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 把一次页面变化提交给 Presenter，等待它异步刷新屏幕。
core::Status Router::submit_transition(const Transition& t, RefreshIntent intent)
{
    // 把一次路由变化包装成 FrameRequest，交给 Presenter 异步刷新。
    // 这里提交成功不等于墨水屏已经刷新完成。
    presenter::FrameRequest req{}; // 构造一个清空的刷新请求。
    // 赋值：把右侧 t 的结果保存到左侧 req.transition。
    req.transition = t;            // 记录从哪个页面到哪里、执行什么动作。
    // 赋值：把右侧 intent 的结果保存到左侧 req.intent。
    req.intent = intent;           // 记录使用哪种刷新波形。
    // 赋值：把右侧 ++seq_ 的结果保存到左侧 req.seq。
    req.seq = ++seq_;              // 递增请求序号，便于 Presenter 丢弃旧请求。

    // 调用 EBOOK_LOGD(TAG, "nav %s->%s %s intent=%s", page_name(t.from), page_name(t.to),，执行这个函数并处理它的返回值或状态变化。
    EBOOK_LOGD(TAG, "nav %s->%s %s intent=%s", page_name(t.from), page_name(t.to),
               // 继续上一行的表达式或参数列表：action_name(t.action), waveform_name(intent.waveform));。
               action_name(t.action), waveform_name(intent.waveform));

    // 提交是异步的：这里返回成功只表示请求入队，不表示屏幕已经刷完。
    return presenter::Presenter::instance().submit(req);
}

// 根据导航动作真正修改 RouteStack 和活动 App。
core::Status Router::apply_route_change(PageId to, NavAction action)
{
    // 先修改路由栈，再提交刷新。失败时返回错误，但已经完成的部分状态
    // 不一定会自动回滚，因此调用者要处理“页面状态变了、刷新没提交”的风险。
    // 类型别名引入：把 apps::AppRegistry 导入当前作用域，后面直接写 AppRegistry。
    using apps::AppRegistry;

    // 根据动作类型分别处理重绘、替换、返回和前进。
    switch (action)
    {
        // 分支匹配：case NavAction::Repaint 成立时执行下面的代码。
        case NavAction::Repaint:
            // 只重绘当前页，不需要修改路由栈。
            return core::Status::Ok;

        // 分支匹配：case NavAction::Replace 成立时执行下面的代码。
        case NavAction::Replace:
            // 替换 Shell：先关闭活动 App，再重建 Shell 栈底。
            if (to.zone == PageId::Zone::Shell)
            {
                // 调用 close_active()：关闭当前 App 时： 1. 调用活动 App 的 onexit()。 2. 清空 active。 3. 把页面编号清回 0。。
                AppRegistry::instance().close_active();
                // 调用 reset_shell(to.shell)：路由栈保存页面历史。resetshell() 清空 Shell 和 overlay，并把指定 shell 设为栈底；pushshell() 返回 false 表示深度超过 8；popshell() 不允许弹掉最后一个页面。。
                stack_.reset_shell(to.shell);
            }
            // 返回 core::Status::Ok 给调用者，表示当前函数的执行结果。
            return core::Status::Ok;

        // 分支匹配：case NavAction::Back 成立时执行下面的代码。
        case NavAction::Back:
            // 返回时如果当前在 AppHost，要先结束活动 App。
            if (stack_.shell_top() == ShellPage::AppHost)
                // 调用 close_active()：关闭当前 App 时： 1. 调用活动 App 的 onexit()。 2. 清空 active。 3. 把页面编号清回 0。。
                AppRegistry::instance().close_active();
            // 弹出上一层 Shell；栈为空说明返回非法。
            if (!stack_.pop_shell())
                // 返回 core::Status::InvalidArg 给调用者，表示当前函数的执行结果。
                return core::Status::InvalidArg;
            // 已经离开 AppHost 时，清掉 App 记录。
            if (stack_.shell_top() != ShellPage::AppHost)
                // 调用 clear_app()：setapp() 记录当前 App 和 App 内页面。clearapp() 在回到 shell 时清理。current() 优先返回 overlay，其次返回 AppHost 里的 App，最后返回 shell。。
                stack_.clear_app();
            // 返回 core::Status::Ok 给调用者，表示当前函数的执行结果。
            return core::Status::Ok;

        // 分支匹配：case NavAction::Forward 成立时执行下面的代码。
        case NavAction::Forward:
            // 前进到 Overlay：只压入覆盖层，不打开普通 App。
            if (to.zone == PageId::Zone::Overlay)
                // 返回结果：调用 push_overlay(to.overlay)：overlay 是覆盖在页面上的临时层，例如控制中心。最多四层，overlaytop() 返回当前最上面一层，没有时返回 None。。
                return stack_.push_overlay(to.overlay) ? core::Status::Ok
                                                       // 执行语句：: core::Status::OutOfResource;；它会更新一次变量、对象或程序状态。
                                                       : core::Status::OutOfResource;

            // 前进到 App：先打开 App，再确保 Shell 进入 AppHost。
            if (to.zone == PageId::Zone::App)
            {
                // 判断条件：!AppRegistry::instance().open(to.app, to.app_page) 是否成立。
                if (!AppRegistry::instance().open(to.app, to.app_page))
                    // 返回 core::Status::NotFound 给调用者，表示当前函数的执行结果。
                    return core::Status::NotFound;
                // 判断条件：stack_.shell_top( 是否成立。
                if (stack_.shell_top() != ShellPage::AppHost &&
                    // 函数定义/声明：调用 push_shell()：路由栈保存页面历史。resetshell() 清空 Shell 和 overlay，并把指定 shell 设为栈底；pushshell() 返回 false 表示深度超过 8；popshell() 不允许弹掉最后一个页面。。
                    !stack_.push_shell(ShellPage::AppHost))
                    // 返回 core::Status::OutOfResource 给调用者，表示当前函数的执行结果。
                    return core::Status::OutOfResource;
                // 调用 set_app(to.app,to.app_page)：setapp() 记录当前 App 和 App 内页面。clearapp() 在回到 shell 时清理。current() 优先返回 overlay，其次返回 AppHost 里的 App，最后返回 shell。。
                stack_.set_app(to.app, to.app_page);
                // 返回 core::Status::Ok 给调用者，表示当前函数的执行结果。
                return core::Status::Ok;
            }

            // 判断条件：to.zone == PageId::Zone::Shell 是否成立。
            if (to.zone == PageId::Zone::Shell)
            {
                // AppHost 不能通过普通 Shell 前进被动创建。
                if (to.shell == ShellPage::AppHost)
                    // 返回 core::Status::InvalidArg 给调用者，表示当前函数的执行结果。
                    return core::Status::InvalidArg;
                // 判断条件：!stack_.push_shell(to.shell) 是否成立。
                if (!stack_.push_shell(to.shell))
                    // 返回 core::Status::OutOfResource 给调用者，表示当前函数的执行结果。
                    return core::Status::OutOfResource;
                // 判断条件：to.shell == ShellPage::AppGrid 是否成立。
                if (to.shell == ShellPage::AppGrid)
                    // 调用 on_enter()：进入文件 App 时： 1. 重置浏览状态或恢复根卷状态。 2. 清空弹窗。 3. 设置加载状态。 4. 调度目录扫描。 5. 请求重绘。。
                    shell::AppGridPage::instance().on_enter();
                // 返回 core::Status::Ok 给调用者，表示当前函数的执行结果。
                return core::Status::Ok;
            }
            // 返回 core::Status::InvalidArg 给调用者，表示当前函数的执行结果。
            return core::Status::InvalidArg;

        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 返回 core::Status::InvalidArg 给调用者，表示当前函数的执行结果。
            return core::Status::InvalidArg;
    }
}

// 函数定义/声明：调用 Router::navigate()：统一导航入口。
core::Status Router::navigate(PageId to, NavAction action, const RefreshIntent* override_intent)
{
    // 统一导航入口：先根据 RefreshEdges 决定波形，再应用路由变化，
    // 最后提交 FrameRequest。override_intent 用于页面临时覆盖刷新策略。
    const PageId from = stack_.current(); // 记录跳转前页面。
    // 执行语句：Transition t{from, to, action};；它会更新一次变量、对象或程序状态。
    Transition t{from, to, action};       // 组装本次页面变化描述。

    // 页面没有指定波形时，用 RefreshEdges 的集中决策表。
    RefreshIntent intent = override_intent != nullptr ? *override_intent : RefreshEdges::resolve(t);

    // 先改路由状态；失败则不再提交刷新。
    const core::Status route_st = apply_route_change(to, action);
    // 判断条件：route_st != core::Status::Ok 是否成立。
    if (route_st != core::Status::Ok)
        // 返回 route_st 给调用者，表示当前函数的执行结果。
        return route_st;

    // 返回 submit_transition(t, intent) 给调用者，表示当前函数的执行结果。
    return submit_transition(t, intent);
}

// 函数定义/声明：调用 Router::back()：如果最上面是 overlay，优先关闭 overlay。否则计算上一层 shell；如果当前在 AppHost，会先关闭活动 App，再提交返回转换。。
core::Status Router::back(const RefreshIntent* override_intent)
{
    // 返回优先级：先关闭 overlay；没有 overlay 时再退 Shell/App 页。
    // 有覆盖层时，返回优先关闭覆盖层。
    if (stack_.overlay_top() != OverlayId::None)
        // 返回结果：调用 close_overlay(override_intent)：这些函数只是用明确的参数调用统一导航逻辑，避免每个页面自己拼 Transition。。
        return close_overlay(override_intent);

    // 计算返回目标和刷新意图。
    const PageId from = stack_.current();
    // 定义变量 PageId to 并初始化为 page_shell(stack_.shell_below())。
    PageId to = page_shell(stack_.shell_below());

    // 页面变化后再提交一次异步刷新。
    Transition t{from, to, NavAction::Back};
    // 定义变量并调用函数初始化：调用 RefreshEdges::resolve(t)：它遍历 kEdges[]，匹配 from、to 和动作，返回刷新波形。匹配不到时打警告并使用 Partial。，结果保存到 RefreshIntent intent。
    RefreshIntent intent = override_intent != nullptr ? *override_intent : RefreshEdges::resolve(t);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Status route_st = apply_route_change(to, NavAction::Back);
    // 判断条件：route_st != core::Status::Ok 是否成立。
    if (route_st != core::Status::Ok)
        // 返回 route_st 给调用者，表示当前函数的执行结果。
        return route_st;

    // 返回 submit_transition(t, intent) 给调用者，表示当前函数的执行结果。
    return submit_transition(t, intent);
}

// 函数定义/声明：调用 Router::replace_shell()：这些函数只是用明确的参数调用统一导航逻辑，避免每个页面自己拼 Transition。。
core::Status Router::replace_shell(ShellPage shell, const RefreshIntent* override_intent)
{
    // 替换当前 Shell，不会在历史栈里保留旧 Shell。
    return navigate(page_shell(shell), NavAction::Replace, override_intent);
}

// 定义或声明函数 Router::repaint：参数在括号里，返回类型写在函数名前面。
core::Status Router::repaint(RefreshIntent intent)
{
    // 页面没有跳转，只要求重新合成和刷新当前页。
    const PageId p = stack_.current();
    // 执行语句：Transition t{p, p, NavAction::Repaint};；它会更新一次变量、对象或程序状态。
    Transition t{p, p, NavAction::Repaint};
    // 返回 submit_transition(t, intent) 给调用者，表示当前函数的执行结果。
    return submit_transition(t, intent);
}

// 函数定义/声明：调用 Router::open_overlay()：这些函数只是用明确的参数调用统一导航逻辑，避免每个页面自己拼 Transition。。
core::Status Router::open_overlay(OverlayId id, const RefreshIntent* override_intent)
{
    // overlay 是压在页面上的临时层，例如控制中心。
    return navigate(page_overlay(id), NavAction::Forward, override_intent);
}

// 函数定义/声明：调用 Router::close_overlay()：这些函数只是用明确的参数调用统一导航逻辑，避免每个页面自己拼 Transition。。
core::Status Router::close_overlay(const RefreshIntent* override_intent)
{
    // 关闭 overlay 后才重新计算当前页面，并提交返回刷新。
    if (stack_.overlay_top() == OverlayId::None)
        // 返回 core::Status::InvalidArg 给调用者，表示当前函数的执行结果。
        return core::Status::InvalidArg;

    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const PageId from = stack_.current(); // 关闭前页面，用于刷新边描述。
    // 调用 pop_overlay()：overlay 是覆盖在页面上的临时层，例如控制中心。最多四层，overlaytop() 返回当前最上面一层，没有时返回 None。。
    stack_.pop_overlay();                 // 先弹出最上面的覆盖层。
    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const PageId to = stack_.current();   // 弹出后真正显示页面。

    // 执行语句：Transition t{from, to, NavAction::Back};；它会更新一次变量、对象或程序状态。
    Transition t{from, to, NavAction::Back};
    // 定义变量并调用函数初始化：调用 RefreshEdges::resolve(t)：它遍历 kEdges[]，匹配 from、to 和动作，返回刷新波形。匹配不到时打警告并使用 Partial。，结果保存到 RefreshIntent intent。
    RefreshIntent intent = override_intent != nullptr ? *override_intent : RefreshEdges::resolve(t);
    // 返回 submit_transition(t, intent) 给调用者，表示当前函数的执行结果。
    return submit_transition(t, intent);
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::router
