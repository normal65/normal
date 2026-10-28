/**
 * [初学者导读] 文件：main/app/ebook/input/input_dispatch.cc
 * 作用：把物理按键和语义动作分发到 Router、App 或全局功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：中断和输入任务不能直接修改页面，只能投递结果。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/input_dispatch.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app_registry.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/reader_app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/auto_lock.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/system_state.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/input_event.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/control_center.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/keyboard.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/router.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "shell/app_grid_page.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "shell/app_host_page.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "shell/home_page.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "shell/lock_page.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::input {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;

// 定义或声明函数 synthetic_swipe：参数在括号里，返回类型写在函数名前面。
Event synthetic_swipe(EventType type)
{
    // 声明并默认初始化对象：Event ev{}。
    Event ev{};
    // 赋值：把右侧 type 的结果保存到左侧 ev.type。
    ev.type = type;
    // 赋值：把右侧 static_cast<uint16_t>(Theme::kScreenW / 2) 的结果保存到左侧 ev.x。
    ev.x = static_cast<uint16_t>(Theme::kScreenW / 2);
    // 赋值：把右侧 static_cast<uint16_t>(Theme::kListStartY + Theme::kListRowH) 的结果保存到左侧 ev.y。
    ev.y = static_cast<uint16_t>(Theme::kListStartY + Theme::kListRowH);
    // 赋值：把右侧 ev.x 的结果保存到左侧 ev.start_x。
    ev.start_x = ev.x;
    // 赋值：把右侧 ev.y 的结果保存到左侧 ev.start_y。
    ev.start_y = ev.y;
    // 返回 ev 给调用者，表示当前函数的执行结果。
    return ev;
}

// 定义或声明函数 route_gesture：参数在括号里，返回类型写在函数名前面。
void route_gesture(const Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using router::OverlayId;
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using router::Router;
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using router::ShellPage;

    // 判断条件：overlays::Keyboard::instance().is_open() 是否成立。
    if (overlays::Keyboard::instance().is_open())
    {
        // 访问对象成员：(void)overlays::Keyboard::instance().on_input(ev);；读取或修改这个对象里的成员状态。
        (void)overlays::Keyboard::instance().on_input(ev);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto& stack = Router::instance().stack();
    // 判断条件：stack.overlay_top() == OverlayId::ControlCenter 是否成立。
    if (stack.overlay_top() == OverlayId::ControlCenter)
    {
        // 访问对象成员：(void)overlays::ControlCenter::instance().on_input(ev);；读取或修改这个对象里的成员状态。
        (void)overlays::ControlCenter::instance().on_input(ev);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 判断条件：stack.overlay_top() != OverlayId::None 是否成立。
    if (stack.overlay_top() != OverlayId::None)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 多分支判断：根据括号里的值选择不同 case。
    switch (stack.shell_top())
    {
        // 分支匹配：case ShellPage::Lock 成立时执行下面的代码。
        case ShellPage::Lock:
            // 访问对象成员：(void)shell::LockPage::instance().on_input(ev);；读取或修改这个对象里的成员状态。
            (void)shell::LockPage::instance().on_input(ev);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case ShellPage::Home 成立时执行下面的代码。
        case ShellPage::Home:
            // 访问对象成员：(void)shell::HomePage::instance().on_input(ev);；读取或修改这个对象里的成员状态。
            (void)shell::HomePage::instance().on_input(ev);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case ShellPage::AppGrid 成立时执行下面的代码。
        case ShellPage::AppGrid:
            // 访问对象成员：(void)shell::AppGridPage::instance().on_input(ev);；读取或修改这个对象里的成员状态。
            (void)shell::AppGridPage::instance().on_input(ev);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case ShellPage::AppHost 成立时执行下面的代码。
        case ShellPage::AppHost:
            // 访问对象成员：(void)shell::AppHostPage::instance().on_input(ev);；读取或修改这个对象里的成员状态。
            (void)shell::AppHostPage::instance().on_input(ev);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
}

// 定义或声明函数 adjust_brightness：参数在括号里，返回类型写在函数名前面。
void adjust_brightness(int delta)
{
    // 赋值：把右侧 data::SystemState::get_instance() 的结果保存到左侧 auto& sys。
    auto& sys = data::SystemState::get_instance();
    // 定义变量并调用函数初始化：调用 brightness()：把 0 到 100 的亮度限制在合法范围后交给背光驱动。墨水屏本身不依赖背光，背光是前光/照明系统，不能把亮度当成画面灰度。，结果保存到 int v。
    int v = static_cast<int>(sys.brightness()) + delta;
    // 判断条件：v < 0 是否成立。
    if (v < 0)
        // 赋值：把右侧 0 的结果保存到左侧 v。
        v = 0;
    // 判断条件：v > 100 是否成立。
    if (v > 100)
        // 赋值：把右侧 100 的结果保存到左侧 v。
        v = 100;
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    sys.set_brightness(static_cast<uint8_t>(v));
    // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
    (void)router::Router::instance().repaint(router::intent_partial_full());
}

// 定义或声明函数 adjust_volume：参数在括号里，返回类型写在函数名前面。
void adjust_volume(int delta)
{
    // 赋值：把右侧 data::SystemState::get_instance() 的结果保存到左侧 auto& sys。
    auto& sys = data::SystemState::get_instance();
    // 定义变量 int v 并初始化为 static_cast<int>(sys.volume()) + delta。
    int v = static_cast<int>(sys.volume()) + delta;
    // 判断条件：v < 0 是否成立。
    if (v < 0)
        // 赋值：把右侧 0 的结果保存到左侧 v。
        v = 0;
    // 判断条件：v > 100 是否成立。
    if (v > 100)
        // 赋值：把右侧 100 的结果保存到左侧 v。
        v = 100;
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    sys.set_volume(static_cast<uint8_t>(v));
    // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
    (void)router::Router::instance().repaint(router::intent_partial_full());
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 函数定义/声明：调用 dispatch_semantic()：语义动作不包含“哪个按键”，只描述用户意图，例如 PageNext、Back、Home、VolumeUp。dispatchsemantic() 把意图变成真实的 Router 导航、音量/亮度修改或阅读菜单操作。。
void dispatch_semantic(SemanticAction action)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (action)
    {
        // 分支匹配：case SemanticAction::PageNext 成立时执行下面的代码。
        case SemanticAction::PageNext:
            // 调用 route_gesture(synthetic_swipe(EventType::SwipeLeft));，执行这个函数并处理它的返回值或状态变化。
            route_gesture(synthetic_swipe(EventType::SwipeLeft));
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SemanticAction::PagePrev 成立时执行下面的代码。
        case SemanticAction::PagePrev:
            // 调用 route_gesture(synthetic_swipe(EventType::SwipeRight));，执行这个函数并处理它的返回值或状态变化。
            route_gesture(synthetic_swipe(EventType::SwipeRight));
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SemanticAction::ListUp 成立时执行下面的代码。
        case SemanticAction::ListUp:
            // 调用 route_gesture(synthetic_swipe(EventType::SwipeUp));，执行这个函数并处理它的返回值或状态变化。
            route_gesture(synthetic_swipe(EventType::SwipeUp));
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SemanticAction::ListDown 成立时执行下面的代码。
        case SemanticAction::ListDown:
            // 调用 route_gesture(synthetic_swipe(EventType::SwipeDown));，执行这个函数并处理它的返回值或状态变化。
            route_gesture(synthetic_swipe(EventType::SwipeDown));
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SemanticAction::Back 成立时执行下面的代码。
        case SemanticAction::Back:
            // 判断条件：router::Router::instance().stack().shell_top() == router::ShellPage::Lock 是否成立。
            if (router::Router::instance().stack().shell_top() == router::ShellPage::Lock)
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 判断条件：router::Router::instance().stack().shell_top() == router::ShellPage::AppHost 是否成立。
            if (router::Router::instance().stack().shell_top() == router::ShellPage::AppHost)
            {
                // 判断条件：apps::App* a = apps::AppRegistry::instance().active() 是否成立。
                if (apps::App* a = apps::AppRegistry::instance().active())
                {
                    // 判断条件：a->on_semantic_back() 是否成立。
                    if (a->on_semantic_back())
                        // 跳出当前循环或 switch，不再执行剩余分支。
                        break;
                }
                // 调用 close_active()：关闭当前 App 时： 1. 调用活动 App 的 onexit()。 2. 清空 active。 3. 把页面编号清回 0。。
                apps::AppRegistry::instance().close_active();
                // 调用 back()：如果最上面是 overlay，优先关闭 overlay。否则计算上一层 shell；如果当前在 AppHost，会先关闭活动 App，再提交返回转换。。
                (void)router::Router::instance().back();
            }
            // 备用分支：前面的条件不成立时执行这里。
            else
            {
                // 调用 back()：如果最上面是 overlay，优先关闭 overlay。否则计算上一层 shell；如果当前在 AppHost，会先关闭活动 App，再提交返回转换。。
                (void)router::Router::instance().back();
            }
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SemanticAction::Menu 成立时执行下面的代码。
        case SemanticAction::Menu:
            // 判断条件：router::Router::instance().stack().shell_top( 是否成立。
            if (router::Router::instance().stack().shell_top() == router::ShellPage::AppHost &&
                // 定义或声明函数 apps::AppRegistry::instance：参数在括号里，返回类型写在函数名前面。
                apps::AppRegistry::instance().active_id() == apps::AppId::Reader)
            {
                // 调用 apps::reader::ReaderApp::instance().toggle_reading_menu();，执行这个函数并处理它的返回值或状态变化。
                apps::reader::ReaderApp::instance().toggle_reading_menu();
            }
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SemanticAction::Home 成立时执行下面的代码。
        case SemanticAction::Home:
            // 判断条件：router::Router::instance().stack().shell_top() != router::ShellPage::Home 是否成立。
            if (router::Router::instance().stack().shell_top() != router::ShellPage::Home)
                // 访问对象成员：(void)router::Router::instance().replace_shell(router::ShellPage::Home);；读取或修改这个对象里的成员状态。
                (void)router::Router::instance().replace_shell(router::ShellPage::Home);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SemanticAction::BrightnessUp 成立时执行下面的代码。
        case SemanticAction::BrightnessUp:
            // 调用 adjust_brightness(10);，执行这个函数并处理它的返回值或状态变化。
            adjust_brightness(10);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SemanticAction::BrightnessDown 成立时执行下面的代码。
        case SemanticAction::BrightnessDown:
            // 调用 adjust_brightness(-10);，执行这个函数并处理它的返回值或状态变化。
            adjust_brightness(-10);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SemanticAction::VolumeUp 成立时执行下面的代码。
        case SemanticAction::VolumeUp:
            // 调用 adjust_volume(10);，执行这个函数并处理它的返回值或状态变化。
            adjust_volume(10);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SemanticAction::VolumeDown 成立时执行下面的代码。
        case SemanticAction::VolumeDown:
            // 调用 adjust_volume(-10);，执行这个函数并处理它的返回值或状态变化。
            adjust_volume(-10);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SemanticAction::None 成立时执行下面的代码。
        case SemanticAction::None:
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
}

// 函数定义/声明：调用 dispatch_physical()：resolvephysical() 只处理 Press，然后查 KeyBindings。dispatchphysical() 负责处理键盘覆盖层和上下文判定，最后调用 dispatchsemantic()。。
void dispatch_physical(const PhysicalEvent& ev)
{
    // 判断条件：overlays::Keyboard::instance().is_open() 是否成立。
    if (overlays::Keyboard::instance().is_open())
    {
        // 键盘弹出期间不走 InputContext/SemanticAction 这条常规链路（那条链路
        // 只认 App/Shell 上下文，键盘是覆盖层，会被判成 Global 从而被吞掉）。
        // 拨码 Up/Down 直接转给键盘用于翻页浏览拼音候选字，其余档位保持原样忽略。
        if (ev.action == PhysicalAction::Press)
        {
            // 判断条件：ev.key == PhysicalKey::Down 是否成立。
            if (ev.key == PhysicalKey::Down)
                // 访问对象成员：(void)overlays::Keyboard::instance().scroll_candidates(true);；读取或修改这个对象里的成员状态。
                (void)overlays::Keyboard::instance().scroll_candidates(true);
            // 备用分支：前面的条件不成立时执行这里。
            else if (ev.key == PhysicalKey::Up)
                // 访问对象成员：(void)overlays::Keyboard::instance().scroll_candidates(false);；读取或修改这个对象里的成员状态。
                (void)overlays::Keyboard::instance().scroll_candidates(false);
        }
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：router::Router::instance().stack().overlay_top() == router::OverlayId::ControlCenter 是否成立。
    if (router::Router::instance().stack().overlay_top() == router::OverlayId::ControlCenter)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const InputContext ctx = detect_context();
    // 判断条件：ctx == InputContext::Global 是否成立。
    if (ctx == InputContext::Global)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 调用 notify_activity()：记录用户活动时间。触摸、按键或页面交互后应该调用它，否则用户刚操作完也可能被锁屏。。
    apps::settings::AutoLock::get_instance().notify_activity();

    // 只读约束：这个值或对象在当前位置不能被修改。
    const SemanticAction act = resolve_physical(ev.key, ev.action, ctx);
    // 判断条件：act != SemanticAction::None 是否成立。
    if (act != SemanticAction::None)
        // 调用 dispatch_semantic(act)：语义动作不包含“哪个按键”，只描述用户意图，例如 PageNext、Back、Home、VolumeUp。dispatchsemantic() 把意图变成真实的 Router 导航、音量/亮度修改或阅读菜单操作。。
        dispatch_semantic(act);
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::input
