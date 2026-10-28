/**
 * [初学者导读] 文件：main/app/ebook/composer/composer.cc
 * 作用：按 Shell、页面、覆盖层顺序合成一帧画面。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：合成顺序决定覆盖层和页面谁先画。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "composer/composer.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app_registry.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/system_state.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/canvas.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/confirm_prompt.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/control_center.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/keyboard.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/status_bar.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/toast.hpp"
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

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::composer {

// 定义或声明函数 Composer::instance：参数在括号里，返回类型写在函数名前面。
Composer& Composer::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static Composer s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 函数定义/声明：调用 Composer::paint()：页面合成的总顺序。
void Composer::paint(uint8_t* back_fb)
{
    // 判断条件：back_fb == nullptr 是否成立。
    if (back_fb == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool dark = data::SystemState::get_instance().night_mode();
    // 函数调用：gfx::Canvas canvas{back_fb, core::Rect::full(), dark};；执行该函数并处理它的返回值或副作用。
    gfx::Canvas canvas{back_fb, core::Rect::full(), dark};
    // 访问对象成员：canvas.clear(gfx::Ink::White);；读取或修改这个对象里的成员状态。
    canvas.clear(gfx::Ink::White);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto& stack = router::Router::instance().stack();

    // 开始代码块：下面是这个函数、分支或循环的主体。
    auto paint_app_host = [&]() {
        // 调用 shell::AppHostPage::instance().paint(canvas);，执行这个函数并处理它的返回值或状态变化。
        shell::AppHostPage::instance().paint(canvas);
        // 判断条件：apps::App* app = apps::AppRegistry::instance().active() 是否成立。
        if (apps::App* app = apps::AppRegistry::instance().active())
        {
            // 判断条件：app->wants_status_bar() 是否成立。
            if (app->wants_status_bar())
                // 调用 overlays::StatusBar::instance().paint(canvas);，执行这个函数并处理它的返回值或状态变化。
                overlays::StatusBar::instance().paint(canvas);
            // 访问对象成员：app->paint_overlay(canvas);；读取或修改这个对象里的成员状态。
            app->paint_overlay(canvas);
        }
    };

    // 多分支判断：根据括号里的值选择不同 case。
    switch (stack.shell_top())
    {
        // 分支匹配：case router::ShellPage::Lock 成立时执行下面的代码。
        case router::ShellPage::Lock:
            // 调用 shell::LockPage::instance().paint(canvas);，执行这个函数并处理它的返回值或状态变化。
            shell::LockPage::instance().paint(canvas);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case router::ShellPage::Home 成立时执行下面的代码。
        case router::ShellPage::Home:
            // 调用 shell::HomePage::instance().paint(canvas);，执行这个函数并处理它的返回值或状态变化。
            shell::HomePage::instance().paint(canvas);
            // 调用 overlays::StatusBar::instance().paint(canvas);，执行这个函数并处理它的返回值或状态变化。
            overlays::StatusBar::instance().paint(canvas);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case router::ShellPage::AppGrid 成立时执行下面的代码。
        case router::ShellPage::AppGrid:
            // 调用 shell::AppGridPage::instance().paint(canvas);，执行这个函数并处理它的返回值或状态变化。
            shell::AppGridPage::instance().paint(canvas);
            // 调用 overlays::StatusBar::instance().paint(canvas);，执行这个函数并处理它的返回值或状态变化。
            overlays::StatusBar::instance().paint(canvas);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case router::ShellPage::AppHost 成立时执行下面的代码。
        case router::ShellPage::AppHost:
            // 调用 paint_app_host();，执行这个函数并处理它的返回值或状态变化。
            paint_app_host();
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }

    // 判断条件：stack.overlay_top() == router::OverlayId::ControlCenter 是否成立。
    if (stack.overlay_top() == router::OverlayId::ControlCenter)
        // 调用 overlays::ControlCenter::instance().paint(canvas);，执行这个函数并处理它的返回值或状态变化。
        overlays::ControlCenter::instance().paint(canvas);
    // 备用分支：前面的条件不成立时执行这里。
    else if (overlays::Keyboard::instance().is_open())
        // 调用 overlays::Keyboard::instance().paint(canvas);，执行这个函数并处理它的返回值或状态变化。
        overlays::Keyboard::instance().paint(canvas);

    // 调用 overlays::Toast::instance().paint(canvas);，执行这个函数并处理它的返回值或状态变化。
    overlays::Toast::instance().paint(canvas);
    // 判断条件：overlays::ConfirmPrompt::instance().is_open() 是否成立。
    if (overlays::ConfirmPrompt::instance().is_open())
        // 调用 overlays::ConfirmPrompt::instance().paint(canvas);，执行这个函数并处理它的返回值或状态变化。
        overlays::ConfirmPrompt::instance().paint(canvas);
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::composer
