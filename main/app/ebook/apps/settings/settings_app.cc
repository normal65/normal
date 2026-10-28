/**
 * [初学者导读] 文件：main/app/ebook/apps/settings/settings_app.cc
 * 作用：实现设置菜单、页面切换和设置页生命周期。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_app.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_internal.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/icon.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/key_bindings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_event.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::settings {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using detail::page_title;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;

// 固定参数 kMenuItems = 11：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kMenuItems = 11;

// 只读约束：这个值或对象在当前位置不能被修改。
const char* const kMenuLabels[kMenuItems] = {
    // 执行这条代码：ui::strings::kSetWifi,     ui::strings::kSetBt,    ui::strings::kSetHotspot,。它会更新变量、对象状态或调用链。
    ui::strings::kSetWifi,     ui::strings::kSetBt,    ui::strings::kSetHotspot,
    // 继续上一行的表达式或参数列表：ui::strings::kSetDisplay,  ui::strings::kSetKeys,  ui::strings::kSetBattery,。
    ui::strings::kSetDisplay,  ui::strings::kSetKeys,  ui::strings::kSetBattery,
    // 继续上一行的表达式或参数列表：ui::strings::kSetSound,    ui::strings::kSetTime,  ui::strings::kSetStorage,。
    ui::strings::kSetSound,    ui::strings::kSetTime,  ui::strings::kSetStorage,
    // 继续上一行的表达式或参数列表：ui::strings::kSetSecurity, ui::strings::kSetAbout,。
    ui::strings::kSetSecurity, ui::strings::kSetAbout,
};
// 静态查表数组 kMenuIcons：运行时复用固定数据，避免每次重新计算。
constexpr uint32_t kMenuIcons[kMenuItems] = {
    // 执行这条代码：gfx::icon::kFaWifi,     gfx::icon::kEbBluetooth,  gfx::icon::kFaHotspot,。它会更新变量、对象状态或调用链。
    gfx::icon::kFaWifi,     gfx::icon::kEbBluetooth,  gfx::icon::kFaHotspot,
    // 继续上一行的表达式或参数列表：gfx::icon::kFaTv,       gfx::icon::kFaKeyboard,   gfx::icon::kFaBatteryFull,。
    gfx::icon::kFaTv,       gfx::icon::kFaKeyboard,   gfx::icon::kFaBatteryFull,
    // 继续上一行的表达式或参数列表：gfx::icon::kFaVolumeUp, gfx::icon::kFaClock,      gfx::icon::kFaHdd,。
    gfx::icon::kFaVolumeUp, gfx::icon::kFaClock,      gfx::icon::kFaHdd,
    // 继续上一行的表达式或参数列表：gfx::icon::kFaShield,   gfx::icon::kFaInfoCircle,。
    gfx::icon::kFaShield,   gfx::icon::kFaInfoCircle,
};
// 静态查表数组 kPageOfMenuRow：运行时复用固定数据，避免每次重新计算。
constexpr SettingsPage kPageOfMenuRow[kMenuItems] = {
    // 执行这条代码：SettingsPage::Wifi,    SettingsPage::Bluetooth, SettingsPage::Hotspot, SettingsPage::Display,。它会更新变量、对象状态或调用链。
    SettingsPage::Wifi,    SettingsPage::Bluetooth, SettingsPage::Hotspot, SettingsPage::Display,
    // 继续上一行的表达式或参数列表：SettingsPage::Keys,    SettingsPage::Battery,   SettingsPage::Sound,   SettingsPage::Time,。
    SettingsPage::Keys,    SettingsPage::Battery,   SettingsPage::Sound,   SettingsPage::Time,
    // 继续上一行的表达式或参数列表：SettingsPage::Storage, SettingsPage::Security,  SettingsPage::About,。
    SettingsPage::Storage, SettingsPage::Security,  SettingsPage::About,
};

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// [初学者导读] SettingsApp::instance()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
SettingsApp& SettingsApp::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static SettingsApp s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// [初学者导读] SettingsApp::SettingsApp()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
SettingsApp::SettingsApp()
{
    // 访问对象成员：menu_list_.set_total(kMenuItems);；读取或修改这个对象里的成员状态。
    menu_list_.set_total(kMenuItems);
    // 开始代码块：下面是这个函数、分支或循环的主体。
    menu_list_.set_provider([](uint8_t i, ui::widgets::RowStyle& s) {
        // 赋值：把右侧 (i < kMenuItems) ? kMenuLabels[i] : "" 的结果保存到左侧 s.label。
        s.label = (i < kMenuItems) ? kMenuLabels[i] : "";
        // 赋值：把右侧 (i < kMenuItems) ? kMenuIcons[i] : 0U 的结果保存到左侧 s.icon_cp。
        s.icon_cp = (i < kMenuItems) ? kMenuIcons[i] : 0U;
        // 赋值：把右侧 true 的结果保存到左侧 s.show_chevron。
        s.show_chevron = true;
    // 执行语句：});；它会更新一次变量、对象或程序状态。
    });
    // 开始代码块：下面是这个函数、分支或循环的主体。
    menu_list_.set_tap_handler([this](uint8_t i) {
        // 判断条件：i < kMenuItems 是否成立。
        if (i < kMenuItems)
            // 调用 enter_page(kPageOfMenuRow[i])：切到新设置页。它通常先调用当前页的 leave 逻辑，再设置 page，最后调用新页的 enter 逻辑。。
            enter_page(kPageOfMenuRow[i]);
    // 执行语句：});；它会更新一次变量、对象或程序状态。
    });

    // 调用 load_time_prefs()：读写时区索引和自动同步开关。。
    load_time_prefs();
    // 调用 apply_tz_from_index()：把 NVS 索引转换成 POSIX 时区字符串，格式为 CST%+d。例如东八区是 CST8 这种 TZ 形式，具体符号规则由 C 库时区约定决定。。
    apply_tz_from_index();
}

// 定义只读对象或变量：初始化后不能在当前位置修改。
const char* SettingsApp::title() const
{
    // 返回 ui::strings::kAppSettings 给调用者，表示当前函数的执行结果。
    return ui::strings::kAppSettings;
}

// 函数定义/声明：调用 SettingsApp::icon_cp()：返回图标 codepoint。图标通常来自字体图标表，所以这里返回的是 uint32t，不是图片路径。。
uint32_t SettingsApp::icon_cp() const
{
    // 返回 gfx::icon::kFaCog 给调用者，表示当前函数的执行结果。
    return gfx::icon::kFaCog;
}

// 定义或声明函数 SettingsApp::on_enter：参数在括号里，返回类型写在函数名前面。
void SettingsApp::on_enter()
{
    // 赋值：把右侧 SettingsPage::Menu 的结果保存到左侧 page_。
    page_ = SettingsPage::Menu;
    // 访问对象成员：menu_list_.set_scroll(0);；读取或修改这个对象里的成员状态。
    menu_list_.set_scroll(0);
    // 调用 load_hotspot_prefs()：从 NVS 读取或保存名称和密码。加载失败时保留默认值。。
    load_hotspot_prefs();
}

// 函数定义/声明：调用 SettingsApp::on_exit()：退出时取消未完成的扫描、关闭弹窗，并确保后台任务不会在对象状态失效后继续写数据。。
void SettingsApp::on_exit()
{
    // 调用 exit_current_page()：执行当前页面离开时的清理。它是集中管理页面资源释放的地方。。
    exit_current_page();
    // 赋值：把右侧 SettingsPage::Menu 的结果保存到左侧 page_。
    page_ = SettingsPage::Menu;
}

// 函数定义/声明：调用 SettingsApp::enter_page()：切到新设置页。它通常先调用当前页的 leave 逻辑，再设置 page，最后调用新页的 enter 逻辑。。
void SettingsApp::enter_page(SettingsPage p)
{
    // 调用 exit_current_page()：执行当前页面离开时的清理。它是集中管理页面资源释放的地方。。
    exit_current_page();
    // 赋值：把右侧 p 的结果保存到左侧 page_。
    page_ = p;
    // 多分支判断：根据括号里的值选择不同 case。
    switch (p)
    {
        // 分支匹配：case SettingsPage::Wifi 成立时执行下面的代码。
        case SettingsPage::Wifi:
            // 调用 wifi_enter()：进入 WiFi 页时刷新已保存网络，必要时触发扫描，更新页面状态。。
            wifi_enter();
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SettingsPage::Bluetooth 成立时执行下面的代码。
        case SettingsPage::Bluetooth:
            // 调用 bt_enter()：进入页面时同步 BLE 当前状态，必要时刷新连接数。。
            bt_enter();
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SettingsPage::Time 成立时执行下面的代码。
        case SettingsPage::Time:
            // 调用 apply_tz_from_index()：把 NVS 索引转换成 POSIX 时区字符串，格式为 CST%+d。例如东八区是 CST8 这种 TZ 形式，具体符号规则由 C 库时区约定决定。。
            apply_tz_from_index();
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SettingsPage::About 成立时执行下面的代码。
        case SettingsPage::About:
            // 赋值：把右侧 0 的结果保存到左侧 about_scroll_row_。
            about_scroll_row_ = 0;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 函数定义/声明：调用 SettingsApp::exit_current_page()：执行当前页面离开时的清理。它是集中管理页面资源释放的地方。。
void SettingsApp::exit_current_page()
{
    // 判断条件：page_ == SettingsPage::Wifi 是否成立。
    if (page_ == SettingsPage::Wifi)
        // 调用 wifi_leave()：离开页面时停止扫描相关的 UI 状态。它不关闭整个 WiFi 网络，也不删除保存的 SSID。。
        wifi_leave();
    // 备用分支：前面的条件不成立时执行这里。
    else if (page_ == SettingsPage::Bluetooth)
        // 调用 bt_leave()：目前为空，因为离开页面不应自动关闭 BLE；BLE 状态是系统级设置，不是页面级资源。。
        bt_leave();
    // 备用分支：前面的条件不成立时执行这里。
    else if (page_ == SettingsPage::Security)
    {
        // 赋值：把右侧 SecPinPurpose::None 的结果保存到左侧 sec_pin_purpose_。
        sec_pin_purpose_ = SecPinPurpose::None;
        // 赋值：把右侧 '\0' 的结果保存到左侧 sec_pending_pin_[0]。
        sec_pending_pin_[0] = '\0';
    }
}

// 函数定义/声明：调用 SettingsApp::back_to_menu()：从二级页面返回设置菜单。它不等同于退出 SettingsApp，只有菜单页按返回才会退出 App。。
bool SettingsApp::back_to_menu()
{
    // 判断条件：page_ == SettingsPage::Menu 是否成立。
    if (page_ == SettingsPage::Menu)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：page_ == SettingsPage::KeysEdit 是否成立。
    if (page_ == SettingsPage::KeysEdit)
    {
        // 赋值：把右侧 SettingsPage::Keys 的结果保存到左侧 page_。
        page_ = SettingsPage::Keys;
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 调用 exit_current_page()：执行当前页面离开时的清理。它是集中管理页面资源释放的地方。。
    exit_current_page();
    // 赋值：把右侧 SettingsPage::Menu 的结果保存到左侧 page_。
    page_ = SettingsPage::Menu;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数定义/声明：调用 SettingsApp::on_semantic_back()：处理语义返回，顺序通常是： 1. 如果弹窗打开，先关闭弹窗。 2. 如果列表还在浏览目录，先返回上一级。 3. 如果已经到根卷，返回 false，让 Shell 退出 App。。
bool SettingsApp::on_semantic_back()
{
    // 返回结果：调用 back_to_menu()：从二级页面返回设置菜单。它不等同于退出 SettingsApp，只有菜单页按返回才会退出 App。。
    return back_to_menu();
}

// 定义或声明函数 SettingsApp::paint：参数在括号里，返回类型写在函数名前面。
void SettingsApp::paint(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* title = page_title(page_);
    // 判断条件：page_ == SettingsPage::KeysEdit 是否成立。
    if (page_ == SettingsPage::KeysEdit)
        // 调用 app::ebook::input::KeyBindings::context_label(keys_edit_ctx_)：把枚举转换成界面文字。新增动作时这三类标签必须一起更新，否则页面会显示空文字或错误名称。。
        title = ::app::ebook::input::KeyBindings::context_label(keys_edit_ctx_);
    // 调用 ui::widgets::toolbar(c,title)：toolbar()： 在固定工具栏矩形中画返回图标、标题和底部横线。返回图标使用 Font Awesome 的码点，而不是手绘 SVG。。
    ui::widgets::toolbar(c, title);
    // 多分支判断：根据括号里的值选择不同 case。
    switch (page_)
    {
        // 分支匹配：case SettingsPage::Menu 成立时执行下面的代码。
        case SettingsPage::Menu:
            // 调用 paint_menu(c);，执行这个函数并处理它的返回值或状态变化。
            paint_menu(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SettingsPage::Wifi 成立时执行下面的代码。
        case SettingsPage::Wifi:
            // 调用 paint_wifi(c);，执行这个函数并处理它的返回值或状态变化。
            paint_wifi(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SettingsPage::Bluetooth 成立时执行下面的代码。
        case SettingsPage::Bluetooth:
            // 调用 paint_bluetooth(c);，执行这个函数并处理它的返回值或状态变化。
            paint_bluetooth(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SettingsPage::Hotspot 成立时执行下面的代码。
        case SettingsPage::Hotspot:
            // 调用 paint_hotspot(c);，执行这个函数并处理它的返回值或状态变化。
            paint_hotspot(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SettingsPage::Display 成立时执行下面的代码。
        case SettingsPage::Display:
            // 调用 paint_display(c);，执行这个函数并处理它的返回值或状态变化。
            paint_display(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SettingsPage::Keys 成立时执行下面的代码。
        case SettingsPage::Keys:
            // 调用 paint_keys(c);，执行这个函数并处理它的返回值或状态变化。
            paint_keys(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SettingsPage::KeysEdit 成立时执行下面的代码。
        case SettingsPage::KeysEdit:
            // 调用 paint_keys_edit(c);，执行这个函数并处理它的返回值或状态变化。
            paint_keys_edit(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SettingsPage::Battery 成立时执行下面的代码。
        case SettingsPage::Battery:
            // 调用 paint_placeholder(c);，执行这个函数并处理它的返回值或状态变化。
            paint_placeholder(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SettingsPage::Sound 成立时执行下面的代码。
        case SettingsPage::Sound:
            // 调用 paint_sound(c);，执行这个函数并处理它的返回值或状态变化。
            paint_sound(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SettingsPage::Time 成立时执行下面的代码。
        case SettingsPage::Time:
            // 调用 paint_time(c);，执行这个函数并处理它的返回值或状态变化。
            paint_time(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SettingsPage::Storage 成立时执行下面的代码。
        case SettingsPage::Storage:
            // 调用 paint_storage(c);，执行这个函数并处理它的返回值或状态变化。
            paint_storage(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SettingsPage::Security 成立时执行下面的代码。
        case SettingsPage::Security:
            // 调用 paint_security(c);，执行这个函数并处理它的返回值或状态变化。
            paint_security(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SettingsPage::About 成立时执行下面的代码。
        case SettingsPage::About:
            // 调用 paint_about(c);，执行这个函数并处理它的返回值或状态变化。
            paint_about(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 调用 paint_placeholder(c);，执行这个函数并处理它的返回值或状态变化。
            paint_placeholder(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
}

// 定义或声明函数 SettingsApp::paint_menu：参数在括号里，返回类型写在函数名前面。
void SettingsApp::paint_menu(gfx::Canvas& c)
{
    // 访问对象成员：menu_list_.paint(c);；读取或修改这个对象里的成员状态。
    menu_list_.paint(c);
}

// 定义或声明函数 SettingsApp::paint_placeholder：参数在括号里，返回类型写在函数名前面。
void SettingsApp::paint_placeholder(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect box{0, static_cast<int16_t>(Theme::kListStartY + 20), Theme::kScreenW,
                         // 继续上一行的表达式或参数列表：Theme::kFontBody};。
                         Theme::kFontBody};
    // 声明并默认初始化对象：gfx::Canvas::TextStyle s{}。
    gfx::Canvas::TextStyle s{};
    // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 s.size_px。
    s.size_px = Theme::kFontBody;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 s.h。
    s.h = gfx::HAlign::Center;
    // 访问对象成员：c.text_in(box, ui::strings::kWip, s);；读取或修改这个对象里的成员状态。
    c.text_in(box, ui::strings::kWip, s);
}

// 定义或声明函数 SettingsApp::on_input：参数在括号里，返回类型写在函数名前面。
shell::InputResult SettingsApp::on_input(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;

    // 判断条件：ev.type == EventType::Tap && page_ != SettingsPage::Menu & 是否成立。
    if (ev.type == EventType::Tap && page_ != SettingsPage::Menu &&
        // 函数定义/声明：调用 ui::widgets::hit_toolbar_back()：toolbar()： 在固定工具栏矩形中画返回图标、标题和底部横线。返回图标使用 Font Awesome 的码点，而不是手绘 SVG。。
        ui::widgets::hit_toolbar_back(static_cast<int16_t>(ev.x), static_cast<int16_t>(ev.y)))
    {
        // 判断条件：back_to_menu() 是否成立。
        if (back_to_menu())
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
    }

    // 多分支判断：根据括号里的值选择不同 case。
    switch (page_)
    {
        // 分支匹配：case SettingsPage::Menu 成立时执行下面的代码。
        case SettingsPage::Menu:
            // 返回 handle_menu(ev) 给调用者，表示当前函数的执行结果。
            return handle_menu(ev);
        // 分支匹配：case SettingsPage::Wifi 成立时执行下面的代码。
        case SettingsPage::Wifi:
            // 返回结果：调用 handle_wifi(ev)：SettingsApp::handlewifi()。
            return handle_wifi(ev);
        // 分支匹配：case SettingsPage::Hotspot 成立时执行下面的代码。
        case SettingsPage::Hotspot:
            // 返回 handle_hotspot(ev) 给调用者，表示当前函数的执行结果。
            return handle_hotspot(ev);
        // 分支匹配：case SettingsPage::Bluetooth 成立时执行下面的代码。
        case SettingsPage::Bluetooth:
            // 返回 handle_bluetooth(ev) 给调用者，表示当前函数的执行结果。
            return handle_bluetooth(ev);
        // 分支匹配：case SettingsPage::Display 成立时执行下面的代码。
        case SettingsPage::Display:
            // 返回 handle_display(ev) 给调用者，表示当前函数的执行结果。
            return handle_display(ev);
        // 分支匹配：case SettingsPage::Keys 成立时执行下面的代码。
        case SettingsPage::Keys:
            // 返回 handle_keys(ev) 给调用者，表示当前函数的执行结果。
            return handle_keys(ev);
        // 分支匹配：case SettingsPage::KeysEdit 成立时执行下面的代码。
        case SettingsPage::KeysEdit:
            // 返回 handle_keys_edit(ev) 给调用者，表示当前函数的执行结果。
            return handle_keys_edit(ev);
        // 分支匹配：case SettingsPage::Sound 成立时执行下面的代码。
        case SettingsPage::Sound:
            // 返回 handle_sound(ev) 给调用者，表示当前函数的执行结果。
            return handle_sound(ev);
        // 分支匹配：case SettingsPage::Time 成立时执行下面的代码。
        case SettingsPage::Time:
            // 返回 handle_time(ev) 给调用者，表示当前函数的执行结果。
            return handle_time(ev);
        // 分支匹配：case SettingsPage::Security 成立时执行下面的代码。
        case SettingsPage::Security:
            // 返回 handle_security(ev) 给调用者，表示当前函数的执行结果。
            return handle_security(ev);
        // 分支匹配：case SettingsPage::About 成立时执行下面的代码。
        case SettingsPage::About:
            // 返回 handle_about(ev) 给调用者，表示当前函数的执行结果。
            return handle_about(ev);
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 返回 {} 给调用者，表示当前函数的执行结果。
            return {};
    }
}

// 定义或声明函数 SettingsApp::handle_menu：参数在括号里，返回类型写在函数名前面。
shell::InputResult SettingsApp::handle_menu(const ::app::ebook::input::Event& ev)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto out = menu_list_.handle_input(ev);
    // 判断条件：out.scroll_changed 是否成立。
    if (out.scroll_changed)
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
    // 返回 {out.consumed} 给调用者，表示当前函数的执行结果。
    return {out.consumed};
}

// 定义或声明函数 SettingsApp::on_ui_event：参数在括号里，返回类型写在函数名前面。
void SettingsApp::on_ui_event(const ui::UiEvent& ev)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (ev.kind)
    {
        // 分支匹配：case ui::UiEventKind::WifiState 成立时执行下面的代码。
        case ui::UiEventKind::WifiState:
            // 调用 on_wifi_state(ev.payload.wifi.state, ev.payload.wifi.fail);，执行这个函数并处理它的返回值或状态变化。
            on_wifi_state(ev.payload.wifi.state, ev.payload.wifi.fail);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case ui::UiEventKind::WifiScanDone 成立时执行下面的代码。
        case ui::UiEventKind::WifiScanDone:
            // 调用 on_wifi_scan_done()：扫描回调先把结果复制到 gpending，并用 gpendingmtx 保护；完成后投递 WifiScanDone。UI 线程收到事件后再读取 gpending 更新 wifiscan。。
            on_wifi_scan_done();
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case ui::UiEventKind::NtpSyncDone 成立时执行下面的代码。
        case ui::UiEventKind::NtpSyncDone:
            // 调用 on_ntp_sync_done(static_cast<uint8_t>(ev.payload.system.value));，执行这个函数并处理它的返回值或状态变化。
            on_ntp_sync_done(static_cast<uint8_t>(ev.payload.system.value));
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::settings
