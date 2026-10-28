/**
 * [初学者导读] 文件：main/app/ebook/router/refresh_edges.cc
 * 作用：根据页面跳转类型决定使用哪种刷新波形。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：路由先改页面栈，再向 Presenter 提交刷新请求；提交是异步的。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/refresh_edges.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "RefreshEdges";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::router {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 类型别名引入：把 apps::AppId 导入当前命名空间，后续判断页面时直接写 AppId。
using apps::AppId;

// 定义编译期常量：值在编译阶段确定，运行时不能修改。
constexpr RefreshIntent kPartialFull = intent_partial_full();

// 定义或声明函数 edge_match：参数在括号里，返回类型写在函数名前面。
bool edge_match(const RefreshEdge& e, const Transition& t)
{
    // 返回 e.from == t.from && e.to == t.to && e.action == t.action 给调用者，表示当前函数的执行结果。
    return e.from == t.from && e.to == t.to && e.action == t.action;
}

// kEdges[]：Shell / Overlay / App 跳转默认 intent_partial_full()
constexpr RefreshEdge kEdges[] = {
    // 函数调用：{page_shell(ShellPage::Lock), page_shell(ShellPage::Lock), NavAction::Replace, kPartialFull},；执行该函数并处理它的返回值或副作用。
    {page_shell(ShellPage::Lock), page_shell(ShellPage::Lock), NavAction::Replace, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::Lock), page_shell(ShellPage::Home), NavAction::Forward, kPartialFull},。
    {page_shell(ShellPage::Lock), page_shell(ShellPage::Home), NavAction::Forward, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::Home), page_shell(ShellPage::Lock), NavAction::Replace, kPartialFull},。
    {page_shell(ShellPage::Home), page_shell(ShellPage::Lock), NavAction::Replace, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_overlay(OverlayId::ControlCenter), page_shell(ShellPage::Lock), NavAction::Replace,。
    {page_overlay(OverlayId::ControlCenter), page_shell(ShellPage::Lock), NavAction::Replace,
     // 继续上一行的表达式或参数列表：kPartialFull},。
     kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::Reader), page_shell(ShellPage::Lock), NavAction::Replace, kPartialFull},。
    {page_app(AppId::Reader), page_shell(ShellPage::Lock), NavAction::Replace, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::Home), page_shell(ShellPage::AppGrid), NavAction::Forward, kPartialFull},。
    {page_shell(ShellPage::Home), page_shell(ShellPage::AppGrid), NavAction::Forward, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::AppGrid), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},。
    {page_shell(ShellPage::AppGrid), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},

    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::Home), page_overlay(OverlayId::ControlCenter), NavAction::Forward,。
    {page_shell(ShellPage::Home), page_overlay(OverlayId::ControlCenter), NavAction::Forward,
     // 继续上一行的表达式或参数列表：kPartialFull},。
     kPartialFull},
    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::AppGrid), page_overlay(OverlayId::ControlCenter), NavAction::Forward,。
    {page_shell(ShellPage::AppGrid), page_overlay(OverlayId::ControlCenter), NavAction::Forward,
     // 继续上一行的表达式或参数列表：kPartialFull},。
     kPartialFull},
    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::AppHost), page_overlay(OverlayId::ControlCenter), NavAction::Forward,。
    {page_shell(ShellPage::AppHost), page_overlay(OverlayId::ControlCenter), NavAction::Forward,
     // 继续上一行的表达式或参数列表：kPartialFull},。
     kPartialFull},
    // 继续上一行的表达式或参数列表：{page_overlay(OverlayId::ControlCenter), page_shell(ShellPage::Home), NavAction::Back,。
    {page_overlay(OverlayId::ControlCenter), page_shell(ShellPage::Home), NavAction::Back,
     // 继续上一行的表达式或参数列表：kPartialFull},。
     kPartialFull},
    // 继续上一行的表达式或参数列表：{page_overlay(OverlayId::ControlCenter), page_shell(ShellPage::AppGrid), NavAction::Back,。
    {page_overlay(OverlayId::ControlCenter), page_shell(ShellPage::AppGrid), NavAction::Back,
     // 继续上一行的表达式或参数列表：kPartialFull},。
     kPartialFull},
    // 继续上一行的表达式或参数列表：{page_overlay(OverlayId::ControlCenter), page_shell(ShellPage::AppHost), NavAction::Back,。
    {page_overlay(OverlayId::ControlCenter), page_shell(ShellPage::AppHost), NavAction::Back,
     // 继续上一行的表达式或参数列表：kPartialFull},。
     kPartialFull},

    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::Home), page_app(AppId::Reader), NavAction::Forward, kPartialFull},。
    {page_shell(ShellPage::Home), page_app(AppId::Reader), NavAction::Forward, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::Home), page_app(AppId::Notepad), NavAction::Forward, kPartialFull},。
    {page_shell(ShellPage::Home), page_app(AppId::Notepad), NavAction::Forward, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::Home), page_app(AppId::WoodenFish), NavAction::Forward, kPartialFull},。
    {page_shell(ShellPage::Home), page_app(AppId::WoodenFish), NavAction::Forward, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::Home), page_app(AppId::Settings), NavAction::Forward, kPartialFull},。
    {page_shell(ShellPage::Home), page_app(AppId::Settings), NavAction::Forward, kPartialFull},

    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::AppGrid), page_app(AppId::Reader), NavAction::Forward, kPartialFull},。
    {page_shell(ShellPage::AppGrid), page_app(AppId::Reader), NavAction::Forward, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::AppGrid), page_app(AppId::Notepad), NavAction::Forward, kPartialFull},。
    {page_shell(ShellPage::AppGrid), page_app(AppId::Notepad), NavAction::Forward, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::AppGrid), page_app(AppId::Gallery), NavAction::Forward, kPartialFull},。
    {page_shell(ShellPage::AppGrid), page_app(AppId::Gallery), NavAction::Forward, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::AppGrid), page_app(AppId::Drawing), NavAction::Forward, kPartialFull},。
    {page_shell(ShellPage::AppGrid), page_app(AppId::Drawing), NavAction::Forward, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::AppGrid), page_app(AppId::Music), NavAction::Forward, kPartialFull},。
    {page_shell(ShellPage::AppGrid), page_app(AppId::Music), NavAction::Forward, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::AppGrid), page_app(AppId::Weather), NavAction::Forward, kPartialFull},。
    {page_shell(ShellPage::AppGrid), page_app(AppId::Weather), NavAction::Forward, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::AppGrid), page_app(AppId::Clock), NavAction::Forward, kPartialFull},。
    {page_shell(ShellPage::AppGrid), page_app(AppId::Clock), NavAction::Forward, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::AppGrid), page_app(AppId::Calendar), NavAction::Forward, kPartialFull},。
    {page_shell(ShellPage::AppGrid), page_app(AppId::Calendar), NavAction::Forward, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::AppGrid), page_app(AppId::Files), NavAction::Forward, kPartialFull},。
    {page_shell(ShellPage::AppGrid), page_app(AppId::Files), NavAction::Forward, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::AppGrid), page_app(AppId::WoodenFish), NavAction::Forward, kPartialFull},。
    {page_shell(ShellPage::AppGrid), page_app(AppId::WoodenFish), NavAction::Forward, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::AppGrid), page_app(AppId::Update), NavAction::Forward, kPartialFull},。
    {page_shell(ShellPage::AppGrid), page_app(AppId::Update), NavAction::Forward, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_shell(ShellPage::AppGrid), page_app(AppId::Settings), NavAction::Forward, kPartialFull},。
    {page_shell(ShellPage::AppGrid), page_app(AppId::Settings), NavAction::Forward, kPartialFull},

    // 继续上一行的表达式或参数列表：{page_app(AppId::Reader), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},。
    {page_app(AppId::Reader), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::Notepad), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},。
    {page_app(AppId::Notepad), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::Gallery), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},。
    {page_app(AppId::Gallery), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::Drawing), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},。
    {page_app(AppId::Drawing), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::WoodenFish), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},。
    {page_app(AppId::WoodenFish), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::Music), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},。
    {page_app(AppId::Music), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::Weather), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},。
    {page_app(AppId::Weather), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::Clock), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},。
    {page_app(AppId::Clock), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::Calendar), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},。
    {page_app(AppId::Calendar), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::Files), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},。
    {page_app(AppId::Files), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::Update), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},。
    {page_app(AppId::Update), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::Settings), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},。
    {page_app(AppId::Settings), page_shell(ShellPage::Home), NavAction::Back, kPartialFull},

    // 继续上一行的表达式或参数列表：{page_app(AppId::Reader), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},。
    {page_app(AppId::Reader), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::Notepad), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},。
    {page_app(AppId::Notepad), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::Gallery), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},。
    {page_app(AppId::Gallery), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::Drawing), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},。
    {page_app(AppId::Drawing), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::WoodenFish), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},。
    {page_app(AppId::WoodenFish), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::Music), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},。
    {page_app(AppId::Music), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::Weather), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},。
    {page_app(AppId::Weather), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::Clock), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},。
    {page_app(AppId::Clock), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::Calendar), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},。
    {page_app(AppId::Calendar), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::Files), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},。
    {page_app(AppId::Files), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::Update), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},。
    {page_app(AppId::Update), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},
    // 继续上一行的表达式或参数列表：{page_app(AppId::Settings), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},。
    {page_app(AppId::Settings), page_shell(ShellPage::AppGrid), NavAction::Back, kPartialFull},
};

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 RefreshEdges::fallback：参数在括号里，返回类型写在函数名前面。
RefreshIntent RefreshEdges::fallback()
{
    // 返回 kPartialFull 给调用者，表示当前函数的执行结果。
    return kPartialFull;
}

// 函数定义/声明：调用 RefreshEdges::resolve()：它遍历 kEdges[]，匹配 from、to 和动作，返回刷新波形。匹配不到时打警告并使用 Partial。。
RefreshIntent RefreshEdges::resolve(const Transition& t)
{
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (const RefreshEdge& e : kEdges)
    {
        // 判断条件：edge_match(e, t) 是否成立。
        if (edge_match(e, t))
            // 返回 e.intent 给调用者，表示当前函数的执行结果。
            return e.intent;
    }
    // 调用 EBOOK_LOGW(TAG, "missing edge %s->%s %s, fallback Partial", page_name(t.from), page_name(t.to),，执行这个函数并处理它的返回值或状态变化。
    EBOOK_LOGW(TAG, "missing edge %s->%s %s, fallback Partial", page_name(t.from), page_name(t.to),
               // 继续上一行的表达式或参数列表：action_name(t.action));。
               action_name(t.action));
    // 返回 fallback() 给调用者，表示当前函数的执行结果。
    return fallback();
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::router
