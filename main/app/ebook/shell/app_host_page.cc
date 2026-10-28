/**
 * [初学者导读] 文件：main/app/ebook/shell/app_host_page.cc
 * 作用：实现 shell 模块中的 app host page 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：Shell 是 App 之外的外壳页面，负责统一返回和输入兜底。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "shell/app_host_page.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app_registry.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/control_center.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/router.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::shell {

// 定义或声明函数 AppHostPage::instance：参数在括号里，返回类型写在函数名前面。
AppHostPage& AppHostPage::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static AppHostPage s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义或声明函数 AppHostPage::paint：参数在括号里，返回类型写在函数名前面。
void AppHostPage::paint(gfx::Canvas& c)
{
    // 判断条件：apps::App* a = apps::AppRegistry::instance().active() 是否成立。
    if (apps::App* a = apps::AppRegistry::instance().active())
        // 访问对象成员：a->paint(c);；读取或修改这个对象里的成员状态。
        a->paint(c);
}

// 定义或声明函数 AppHostPage::on_input：参数在括号里，返回类型写在函数名前面。
InputResult AppHostPage::on_input(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;
    // 赋值：把右侧 apps::AppRegistry::instance() 的结果保存到左侧 auto& reg。
    auto& reg = apps::AppRegistry::instance();
    // 赋值：把右侧 router::Router::instance() 的结果保存到左侧 auto& r。
    auto& r = router::Router::instance();

    // 判断条件：ev.type == EventType::SwipeDown && ev.start_y <= ui::Theme::kStatusBarH 是否成立。
    if (ev.type == EventType::SwipeDown && ev.start_y <= ui::Theme::kStatusBarH)
    {
        // 调用 open()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
        overlays::ControlCenter::instance().open();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 判断条件：apps::App* a = reg.active() 是否成立。
    if (apps::App* a = reg.active())
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const auto out = a->on_input(ev);
        // 判断条件：out.consumed 是否成立。
        if (out.consumed)
            // 返回 out 给调用者，表示当前函数的执行结果。
            return out;
    }

    // 判断条件：ev.type == EventType::Tap & 是否成立。
    if (ev.type == EventType::Tap &&
        // 函数定义/声明：调用 ui::widgets::hit_toolbar_back()：toolbar()： 在固定工具栏矩形中画返回图标、标题和底部横线。返回图标使用 Font Awesome 的码点，而不是手绘 SVG。。
        ui::widgets::hit_toolbar_back(static_cast<int16_t>(ev.x), static_cast<int16_t>(ev.y)))
    {
        // 调用 close_active()：关闭当前 App 时： 1. 调用活动 App 的 onexit()。 2. 清空 active。 3. 把页面编号清回 0。。
        reg.close_active();
        // 调用 back()：如果最上面是 overlay，优先关闭 overlay。否则计算上一层 shell；如果当前在 AppHost，会先关闭活动 App，再提交返回转换。。
        (void)r.back();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 返回 {} 给调用者，表示当前函数的执行结果。
    return {};
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::shell
