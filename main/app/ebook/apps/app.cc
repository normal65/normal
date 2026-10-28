/**
 * [初学者导读] 文件：main/app/ebook/apps/app.cc
 * 作用：定义所有 App 的公共生命周期、绘制、输入和刷新接口。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app_registry.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "common/time/time.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "presenter/presenter.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/router.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps {

// 函数定义/声明：调用 App::navigate_page()：App 内部切换子页面。
void App::navigate_page(uint16_t page, const router::RefreshIntent* override_intent)
{
    // 函数调用：(void)router::Router::instance().navigate(router::page_app(id(), page),；执行该函数并处理它的返回值或副作用。
    (void)router::Router::instance().navigate(router::page_app(id(), page),
                                              // 继续上一行的表达式或参数列表：router::NavAction::Forward, override_intent);。
                                              router::NavAction::Forward, override_intent);
}

// 函数定义/声明：调用 App::repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
void App::repaint(const router::RefreshIntent& intent)
{
    // 访问对象成员：(void)router::Router::instance().repaint(intent);；读取或修改这个对象里的成员状态。
    (void)router::Router::instance().repaint(intent);
}

// 函数定义/声明：调用 App::request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
void App::request_repaint()
{
    // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
    request_repaint(router::intent_partial_full());
}

// 函数定义/声明：调用 App::request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
void App::request_repaint(const router::RefreshIntent& intent)
{
    // 调用 repaint(intent);，执行这个函数并处理它的返回值或状态变化。
    repaint(intent);
}

// 函数定义/声明：调用 App::exit_to_parent()：它是应用内“返回上一层”的统一入口，最终通常走 Router::back()。子页面或编辑器应该优先调用它，而不是自己 pop 路由栈。。
void App::exit_to_parent(const router::RefreshIntent* override_intent)
{
    // Router::back 在 AppHost 下会 close_active；路由变更后再合成上屏
    (void)router::Router::instance().back(override_intent);
}

// 函数定义/声明：调用 App::request_repaint_if_ready()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
bool App::request_repaint_if_ready(int64_t& last_ms, int64_t min_interval_ms, bool force)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int64_t now = ::app::common::time::uptime_ms();
    // 判断条件：!force 是否成立。
    if (!force)
    {
        // 判断条件：(now - last_ms) < min_interval_ms 是否成立。
        if ((now - last_ms) < min_interval_ms)
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        // 判断条件：!presenter::Presenter::instance().wait_idle(0) 是否成立。
        if (!presenter::Presenter::instance().wait_idle(0))
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
    }

    // 赋值：把右侧 now 的结果保存到左侧 last_ms。
    last_ms = now;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps
