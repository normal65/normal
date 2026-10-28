/**
 * [初学者导读] 文件：main/app/ebook/ui/ui_loop.cc
 * 作用：运行 UI 主循环，消费 UiBus 事件并分发给当前页面。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：UI 状态只能在 UiLoop/UI 线程修改，跨任务结果通过 UiBus 回 UI。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_loop.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app_registry.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/input_dispatch.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/physical_types.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/auto_lock.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/system_state.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/power_save.hpp"
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
#include "router/page_id.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/refresh_intent.hpp"
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
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_bus.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "UiLoop";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::ui {

// 定义或声明函数 UiLoop::instance：参数在括号里，返回类型写在函数名前面。
UiLoop& UiLoop::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static UiLoop s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// [初学者导读] UiLoop::init()：注册夜间模式重绘。详见 docs/代码详解-初学者版.md。
bool UiLoop::init()
{
    // 调用 data::SystemState::get_instance().set_night_mode_observer([](bool /*enabled*/) {，执行这个函数并处理它的返回值或状态变化。
    data::SystemState::get_instance().set_night_mode_observer([](bool /*enabled*/) {
        // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
        (void)router::Router::instance().repaint(router::intent_partial_full());
    // 执行语句：});；它会更新一次变量、对象或程序状态。
    });
    // 返回 UiBus::get_instance().init() 给调用者，表示当前函数的执行结果。
    return UiBus::get_instance().init();
}

// 定义或声明函数 UiLoop::deinit：参数在括号里，返回类型写在函数名前面。
void UiLoop::deinit()
{
    // 调用 stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
    stop();
}

// [初学者导读] UiLoop::start()：创建 UI 任务。详见 docs/代码详解-初学者版.md。
bool UiLoop::start()
{
    // 判断条件：running_ 是否成立。
    if (running_)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;

    // 执行语句：::app::sys::task::Cfg cfg;；它会更新一次变量、对象或程序状态。
    ::app::sys::task::Cfg cfg;
    // 赋值：把右侧 "ebook_ui" 的结果保存到左侧 cfg.name。
    cfg.name = "ebook_ui";
    // 赋值：把右侧 kStackBytes 的结果保存到左侧 cfg.stack_size。
    cfg.stack_size = kStackBytes;
    // 赋值：把右侧 ::app::sys::task::Priority::NORMAL 的结果保存到左侧 cfg.priority。
    cfg.priority = ::app::sys::task::Priority::NORMAL;
    // 赋值：把右侧 true 的结果保存到左侧 cfg.use_psram。
    cfg.use_psram = true;

    // 赋值：把右侧 true 的结果保存到左侧 running_。
    running_ = true;
    // 执行这条代码：task_ = std::make_unique<::app::sys::task::Task>(。它会更新变量、对象状态或调用链。
    task_ = std::make_unique<::app::sys::task::Task>(
        // 继续上一行的表达式或参数列表：[](void* arg) { static_cast<UiLoop*>(arg)->run(); }, cfg, this);。
        [](void* arg) { static_cast<UiLoop*>(arg)->run(); }, cfg, this);
    // 判断条件：!task_->start() 是否成立。
    if (!task_->start())
    {
        // 赋值：把右侧 false 的结果保存到左侧 running_。
        running_ = false;
        // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
        task_.reset();
        // 调用 EBOOK_LOGE(TAG, "task start failed");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGE(TAG, "task start failed");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 访问对象成员：(void)router::Router::instance().replace_shell(router::ShellPage::Lock);；读取或修改这个对象里的成员状态。
    (void)router::Router::instance().replace_shell(router::ShellPage::Lock);
    // 调用 EBOOK_LOGI(TAG, "started");，执行这个函数并处理它的返回值或状态变化。
    EBOOK_LOGI(TAG, "started");
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数定义/声明：调用 UiLoop::stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
void UiLoop::stop()
{
    // 判断条件：!running_ 是否成立。
    if (!running_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 false 的结果保存到左侧 running_。
    running_ = false;
    // 判断条件：task_ 是否成立。
    if (task_)
    {
        // 调用 destroy()：直接删除，不是优雅停止。
        task_->destroy();
        // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
        task_.reset();
    }
}

// 函数定义/声明：调用 UiLoop::dispatch_input()：输入优先级。
void UiLoop::dispatch_input(const ::app::ebook::input::Event& ev)
{
    // 调用 notify_activity()：记录用户活动时间。触摸、按键或页面交互后应该调用它，否则用户刚操作完也可能被锁屏。。
    apps::settings::AutoLock::get_instance().notify_activity();

    // 判断条件：overlays::ConfirmPrompt::instance().is_open() 是否成立。
    if (overlays::ConfirmPrompt::instance().is_open())
    {
        // 访问对象成员：(void)overlays::ConfirmPrompt::instance().on_input(ev);；读取或修改这个对象里的成员状态。
        (void)overlays::ConfirmPrompt::instance().on_input(ev);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：overlays::Keyboard::instance().is_open() 是否成立。
    if (overlays::Keyboard::instance().is_open())
    {
        // 访问对象成员：(void)overlays::Keyboard::instance().on_input(ev);；读取或修改这个对象里的成员状态。
        (void)overlays::Keyboard::instance().on_input(ev);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto& stack = router::Router::instance().stack();
    // 判断条件：stack.overlay_top() == router::OverlayId::ControlCenter 是否成立。
    if (stack.overlay_top() == router::OverlayId::ControlCenter)
    {
        // 访问对象成员：(void)overlays::ControlCenter::instance().on_input(ev);；读取或修改这个对象里的成员状态。
        (void)overlays::ControlCenter::instance().on_input(ev);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 判断条件：stack.overlay_top() != router::OverlayId::None 是否成立。
    if (stack.overlay_top() != router::OverlayId::None)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 多分支判断：根据括号里的值选择不同 case。
    switch (stack.shell_top())
    {
        // 分支匹配：case router::ShellPage::Lock 成立时执行下面的代码。
        case router::ShellPage::Lock:
            // 访问对象成员：(void)shell::LockPage::instance().on_input(ev);；读取或修改这个对象里的成员状态。
            (void)shell::LockPage::instance().on_input(ev);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case router::ShellPage::Home 成立时执行下面的代码。
        case router::ShellPage::Home:
            // 访问对象成员：(void)shell::HomePage::instance().on_input(ev);；读取或修改这个对象里的成员状态。
            (void)shell::HomePage::instance().on_input(ev);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case router::ShellPage::AppGrid 成立时执行下面的代码。
        case router::ShellPage::AppGrid:
            // 访问对象成员：(void)shell::AppGridPage::instance().on_input(ev);；读取或修改这个对象里的成员状态。
            (void)shell::AppGridPage::instance().on_input(ev);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case router::ShellPage::AppHost 成立时执行下面的代码。
        case router::ShellPage::AppHost:
            // 访问对象成员：(void)shell::AppHostPage::instance().on_input(ev);；读取或修改这个对象里的成员状态。
            (void)shell::AppHostPage::instance().on_input(ev);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
}

// 函数定义/声明：调用 UiLoop::handle_event()：按事件类型分流。
void UiLoop::handle_event(const UiEvent& ev)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (ev.kind)
    {
        // 分支匹配：case UiEventKind::Input 成立时执行下面的代码。
        case UiEventKind::Input:
            // 调用 dispatch_input(ev.payload.input)：输入优先级。
            dispatch_input(ev.payload.input);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 分支匹配：case UiEventKind::PhysicalInput:  成立时执行下面的代码。
        case UiEventKind::PhysicalInput: {
            // 声明并默认初始化对象：::app::ebook::input::PhysicalEvent pe{}。
            ::app::ebook::input::PhysicalEvent pe{};
            // 赋值：把右侧 static_cast<::app::ebook::input::PhysicalKey>(ev.payload.physical.key) 的结果保存到左侧 pe.key。
            pe.key = static_cast<::app::ebook::input::PhysicalKey>(ev.payload.physical.key);
            // 执行这条代码：pe.action =。它会更新变量、对象状态或调用链。
            pe.action =
                // 继续上一行的表达式或参数列表：static_cast<::app::ebook::input::PhysicalAction>(ev.payload.physical.action);。
                static_cast<::app::ebook::input::PhysicalAction>(ev.payload.physical.action);
            // 调用 app::ebook::input::dispatch_physical(pe)：resolvephysical() 只处理 Press，然后查 KeyBindings。dispatchphysical() 负责处理键盘覆盖层和上下文判定，最后调用 dispatchsemantic()。。
            ::app::ebook::input::dispatch_physical(pe);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        }

        // 分支匹配：case UiEventKind::TickClock 成立时执行下面的代码。
        case UiEventKind::TickClock:
            // 判断条件：router::Router::instance().stack().shell_top() == router::ShellPage::Lock 是否成立。
            if (router::Router::instance().stack().shell_top() == router::ShellPage::Lock)
            {
                // 调用 shell::LockPage::instance().on_ui_event(ev);，执行这个函数并处理它的返回值或状态变化。
                shell::LockPage::instance().on_ui_event(ev);
            }
            // 备用分支：前面的条件不成立时执行这里。
            else if (router::Router::instance().stack().shell_top() == router::ShellPage::AppHost)
            {
                // 判断条件：apps::App* a = apps::AppRegistry::instance().active() 是否成立。
                if (apps::App* a = apps::AppRegistry::instance().active())
                {
                    // 判断条件：!a->wants_status_bar() 是否成立。
                    if (!a->wants_status_bar())
                        // 访问对象成员：a->on_ui_event(ev);；读取或修改这个对象里的成员状态。
                        a->on_ui_event(ev);
                    // 备用分支：前面的条件不成立时执行这里。
                    else
                        // 调用 overlays::StatusBar::instance().on_tick_clock(ev.payload.clock.hour,，执行这个函数并处理它的返回值或状态变化。
                        overlays::StatusBar::instance().on_tick_clock(ev.payload.clock.hour,
                                                                      // 继续上一行的表达式或参数列表：ev.payload.clock.minute);。
                                                                      ev.payload.clock.minute);
                }
            }
            // 备用分支：前面的条件不成立时执行这里。
            else
            {
                // 调用 overlays::StatusBar::instance().on_tick_clock(ev.payload.clock.hour,，执行这个函数并处理它的返回值或状态变化。
                overlays::StatusBar::instance().on_tick_clock(ev.payload.clock.hour,
                                                              // 继续上一行的表达式或参数列表：ev.payload.clock.minute);。
                                                              ev.payload.clock.minute);
            }
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case UiEventKind::TickBattery 成立时执行下面的代码。
        case UiEventKind::TickBattery:
            // 调用 on_battery_pct(ev.payload.battery.pct)：根据电量应用自动策略： 低于 20%：可以进入普通省电。 低于 10%：可以进入强省电。 高于 80%：解除自动省电。。
            apps::settings::PowerSave::get_instance().on_battery_pct(ev.payload.battery.pct);
            // 判断条件：router::Router::instance().stack().shell_top() == router::ShellPage::Lock 是否成立。
            if (router::Router::instance().stack().shell_top() == router::ShellPage::Lock)
            {
                // 调用 shell::LockPage::instance().on_ui_event(ev);，执行这个函数并处理它的返回值或状态变化。
                shell::LockPage::instance().on_ui_event(ev);
            }
            // 备用分支：前面的条件不成立时执行这里。
            else if (router::Router::instance().stack().shell_top() == router::ShellPage::AppHost)
            {
                // 判断条件：apps::App* a = apps::AppRegistry::instance().active() 是否成立。
                if (apps::App* a = apps::AppRegistry::instance().active())
                {
                    // 判断条件：!a->wants_status_bar() 是否成立。
                    if (!a->wants_status_bar())
                        // 访问对象成员：a->on_ui_event(ev);；读取或修改这个对象里的成员状态。
                        a->on_ui_event(ev);
                    // 备用分支：前面的条件不成立时执行这里。
                    else
                        // 调用 on_tick_battery(ev.payload.battery.pct)：两者只在数值变化且状态栏可见时重绘，避免后台页面推动刷新。。
                        overlays::StatusBar::instance().on_tick_battery(ev.payload.battery.pct);
                }
            }
            // 备用分支：前面的条件不成立时执行这里。
            else
            {
                // 调用 on_tick_battery(ev.payload.battery.pct)：两者只在数值变化且状态栏可见时重绘，避免后台页面推动刷新。。
                overlays::StatusBar::instance().on_tick_battery(ev.payload.battery.pct);
            }
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case UiEventKind::WifiState 成立时执行下面的代码。
        case UiEventKind::WifiState:
        // 分支匹配：case UiEventKind::WifiScanDone 成立时执行下面的代码。
        case UiEventKind::WifiScanDone:
            // 调用 apps::settings::SettingsApp::instance().on_ui_event(ev);，执行这个函数并处理它的返回值或状态变化。
            apps::settings::SettingsApp::instance().on_ui_event(ev);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case UiEventKind::NtpSyncDone 成立时执行下面的代码。
        case UiEventKind::NtpSyncDone:
            // 调用 apps::settings::SettingsApp::instance().on_ui_event(ev);，执行这个函数并处理它的返回值或状态变化。
            apps::settings::SettingsApp::instance().on_ui_event(ev);
            // 调用 overlays::StatusBar::instance().on_ntp_sync_done(，执行这个函数并处理它的返回值或状态变化。
            overlays::StatusBar::instance().on_ntp_sync_done(
                // 继续上一行的表达式或参数列表：static_cast<uint8_t>(ev.payload.system.value));。
                static_cast<uint8_t>(ev.payload.system.value));
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case UiEventKind::SystemHint:  成立时执行下面的代码。
        case UiEventKind::SystemHint: {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const auto hint = ev.payload.system.hint;
            // 判断条件：hint == SystemHintKind::ToastExpire && overlays::Toast::instance().visible() 是否成立。
            if (hint == SystemHintKind::ToastExpire && overlays::Toast::instance().visible())
            {
                // 调用 hide()：ensuretimer()：第一次调用创建一次性软件定时器，后续修改周期。。
                overlays::Toast::instance().hide();
                // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
                (void)router::Router::instance().repaint(router::intent_partial_full());
            }
            // 备用分支：前面的条件不成立时执行这里。
            else if (hint == SystemHintKind::ClockAlarm)
            {
                // 调用 overlays::Toast::instance().show(strings::kClkAlarmRing, 5000);，执行这个函数并处理它的返回值或状态变化。
                overlays::Toast::instance().show(strings::kClkAlarmRing, 5000);
                // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
                (void)router::Router::instance().repaint(router::intent_partial_full());
            }
            // 备用分支：前面的条件不成立时执行这里。
            else if (hint == SystemHintKind::AutoLock)
            {
                // 赋值：把右侧 router::Router::instance() 的结果保存到左侧 auto& r。
                auto& r = router::Router::instance();
                // 判断条件：r.stack().shell_top() != router::ShellPage::Lock 是否成立。
                if (r.stack().shell_top() != router::ShellPage::Lock)
                    // 访问对象成员：(void)r.replace_shell(router::ShellPage::Lock);；读取或修改这个对象里的成员状态。
                    (void)r.replace_shell(router::ShellPage::Lock);
                // 调用 notify_activity()：记录用户活动时间。触摸、按键或页面交互后应该调用它，否则用户刚操作完也可能被锁屏。。
                apps::settings::AutoLock::get_instance().notify_activity();
            }

            // 判断条件：router::Router::instance().stack().shell_top() == router::ShellPage::AppHost 是否成立。
            if (router::Router::instance().stack().shell_top() == router::ShellPage::AppHost)
            {
                // 判断条件：apps::App* a = apps::AppRegistry::instance().active() 是否成立。
                if (apps::App* a = apps::AppRegistry::instance().active())
                    // 访问对象成员：a->on_ui_event(ev);；读取或修改这个对象里的成员状态。
                    a->on_ui_event(ev);
            }
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        }

        // 分支匹配：case UiEventKind::None 成立时执行下面的代码。
        case UiEventKind::None:
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
}

// [初学者导读] UiLoop::run()：取出一个事件并清空积压。详见 docs/代码详解-初学者版.md。
void UiLoop::run()
{
    // 赋值：把右侧 UiBus::get_instance() 的结果保存到左侧 auto& bus。
    auto& bus = UiBus::get_instance();
    // 声明并默认初始化对象：UiEvent ev{}。
    UiEvent ev{};
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (running_)
    {
        // 判断条件：!bus.recv(ev, portMAX_DELAY) 是否成立。
        if (!bus.recv(ev, portMAX_DELAY))
            // 继续下一轮：跳过本次循环剩余代码。
            continue;

        // 调用 handle_event(ev)：按事件类型分流。
        handle_event(ev);
        // 循环执行：条件为真时重复运行，条件变化后退出。
        while (running_ && bus.recv(ev, 0))
            // 调用 handle_event(ev)：按事件类型分流。
            handle_event(ev);

        // 执行语句：++frames_;；它会更新一次变量、对象或程序状态。
        ++frames_;
    }
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::ui
