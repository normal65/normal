/**
 * [初学者导读] 文件：main/app/ebook/apps/settings/settings_bluetooth.cc
 * 作用：实现蓝牙设置页的开关、名称修改和连接数显示。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_app.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_internal.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/system_state.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/icon.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "network/bluetooth/bluetooth.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/keyboard.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/toast.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::settings {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using detail::tap_row_index;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 SettingsApp::paint_bluetooth：参数在括号里，返回类型写在函数名前面。
void SettingsApp::paint_bluetooth(gfx::Canvas& c)
{
    // 定义变量 int16_t y 并初始化为 Theme::kListStartY。
    int16_t y = Theme::kListStartY;
    // 赋值：把右侧 data::SystemState::get_instance() 的结果保存到左侧 auto& sys。
    auto& sys = data::SystemState::get_instance();

    {
        // 声明并默认初始化对象：ui::widgets::RowStyle rs{}。
        ui::widgets::RowStyle rs{};
        // 赋值：把右侧 ui::strings::kCtlBt 的结果保存到左侧 rs.label。
        rs.label = ui::strings::kCtlBt;
        // 赋值：把右侧 gfx::icon::toggle(sys.bluetooth()) 的结果保存到左侧 rs.value_icon。
        rs.value_icon = gfx::icon::toggle(sys.bluetooth());
        // 赋值：把右侧 20 的结果保存到左侧 rs.value_icon_size。
        rs.value_icon_size = 20;
        // 赋值：把右侧 false 的结果保存到左侧 rs.show_chevron。
        rs.show_chevron = false;
        // 调用 ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);，执行这个函数并处理它的返回值或状态变化。
        ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);
    }
    // 赋值：把右侧 static_cast<int16_t>(y + Theme::kListRowH) 的结果保存到左侧 y。
    y = static_cast<int16_t>(y + Theme::kListRowH);

    {
        // 声明并默认初始化对象：ui::widgets::RowStyle rs{}。
        ui::widgets::RowStyle rs{};
        // 赋值：把右侧 ui::strings::kSetBtName 的结果保存到左侧 rs.label。
        rs.label = ui::strings::kSetBtName;
        // 赋值：把右侧 bt_name_ 的结果保存到左侧 rs.value。
        rs.value = bt_name_;
        // 赋值：把右侧 true 的结果保存到左侧 rs.show_chevron。
        rs.show_chevron = true;
        // 调用 ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);，执行这个函数并处理它的返回值或状态变化。
        ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);
    }
    // 赋值：把右侧 static_cast<int16_t>(y + Theme::kListRowH) 的结果保存到左侧 y。
    y = static_cast<int16_t>(y + Theme::kListRowH);

    {
        // 执行语句：char cnt[8];；它会更新一次变量、对象或程序状态。
        char cnt[8];
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t n = sys.bluetooth()
                              // 定义或声明函数 app::network::bluetooth::BleMgr::get_instance：参数在括号里，返回类型写在函数名前面。
                              ? ::app::network::bluetooth::BleMgr::get_instance().get_conn_count()
                              // 执行语句：: 0;；它会更新一次变量、对象或程序状态。
                              : 0;
        // 函数调用：(void)std::snprintf(cnt, sizeof(cnt), ui::strings::kSetConnCountFmt,；执行该函数并处理它的返回值或副作用。
        (void)std::snprintf(cnt, sizeof(cnt), ui::strings::kSetConnCountFmt,
                            // 继续上一行的表达式或参数列表：static_cast<unsigned>(n));。
                            static_cast<unsigned>(n));
        // 声明并默认初始化对象：ui::widgets::RowStyle rs{}。
        ui::widgets::RowStyle rs{};
        // 赋值：把右侧 ui::strings::kSetBtClients 的结果保存到左侧 rs.label。
        rs.label = ui::strings::kSetBtClients;
        // 赋值：把右侧 cnt 的结果保存到左侧 rs.value。
        rs.value = cnt;
        // 赋值：把右侧 false 的结果保存到左侧 rs.show_chevron。
        rs.show_chevron = false;
        // 调用 ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);，执行这个函数并处理它的返回值或状态变化。
        ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);
    }
}

// 定义或声明函数 SettingsApp::handle_bluetooth：参数在括号里，返回类型写在函数名前面。
shell::InputResult SettingsApp::handle_bluetooth(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;
    // 判断条件：ev.type != EventType::Tap 是否成立。
    if (ev.type != EventType::Tap)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t row = tap_row_index(static_cast<int16_t>(ev.y));
    // 判断条件：row == 0xFF 是否成立。
    if (row == 0xFF)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 赋值：把右侧 data::SystemState::get_instance() 的结果保存到左侧 auto& sys。
    auto& sys = data::SystemState::get_instance();
    // 判断条件：row == 0 是否成立。
    if (row == 0)
    {
        // 访问对象成员：sys.set_bluetooth(!sys.bluetooth());；读取或修改这个对象里的成员状态。
        sys.set_bluetooth(!sys.bluetooth());
        // 调用 bt_apply_state()：根据 SystemState.bluetooth： 打开时调用 BleMgr.init()，并按需要开始广播。 关闭时停止广播。。
        bt_apply_state();
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：row == 1 是否成立。
    if (row == 1)
    {
        // 声明并默认初始化对象：overlays::KeyboardConfig kc{}。
        overlays::KeyboardConfig kc{};
        // 赋值：把右侧 overlays::KeyboardLayer::Letters 的结果保存到左侧 kc.default_layer。
        kc.default_layer = overlays::KeyboardLayer::Letters;
        // 赋值：把右侧 32 的结果保存到左侧 kc.max_len。
        kc.max_len = 32;
        // 赋值：把右侧 bt_name_ 的结果保存到左侧 kc.initial_text。
        kc.initial_text = bt_name_;
        // 赋值：把右侧 ui::strings::kSetBtName 的结果保存到左侧 kc.title。
        kc.title = ui::strings::kSetBtName;
        // 调用 overlays::Keyboard::instance().open(kc, on_bt_name_done, this);，执行这个函数并处理它的返回值或状态变化。
        overlays::Keyboard::instance().open(kc, on_bt_name_done, this);
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 返回 {} 给调用者，表示当前函数的执行结果。
    return {};
}

// 函数定义/声明：调用 SettingsApp::bt_enter()：进入页面时同步 BLE 当前状态，必要时刷新连接数。。
void SettingsApp::bt_enter()
{
    // 调用 bt_apply_state()：根据 SystemState.bluetooth： 打开时调用 BleMgr.init()，并按需要开始广播。 关闭时停止广播。。
    bt_apply_state();
}

// 调用 SettingsApp::bt_leave()：目前为空，因为离开页面不应自动关闭 BLE；BLE 状态是系统级设置，不是页面级资源。。
void SettingsApp::bt_leave() {}

// 函数定义/声明：调用 SettingsApp::bt_apply_state()：根据 SystemState.bluetooth： 打开时调用 BleMgr.init()，并按需要开始广播。 关闭时停止广播。。
void SettingsApp::bt_apply_state()
{
    // 赋值：把右侧 ::app::network::bluetooth::BleMgr::get_instance() 的结果保存到左侧 auto& mgr。
    auto& mgr = ::app::network::bluetooth::BleMgr::get_instance();
    // 赋值：把右侧 data::SystemState::get_instance() 的结果保存到左侧 auto& sys。
    auto& sys = data::SystemState::get_instance();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool on = sys.bluetooth();
    // 判断条件：on 是否成立。
    if (on)
    {
        // 判断条件：!mgr.is_init() && !mgr.init(bt_name_) 是否成立。
        if (!mgr.is_init() && !mgr.init(bt_name_))
        {
            // 访问对象成员：sys.set_bluetooth(false);；读取或修改这个对象里的成员状态。
            sys.set_bluetooth(false);
            // 调用 overlays::Toast::instance().show(ui::strings::kSetBtInitFail, 2000);，执行这个函数并处理它的返回值或状态变化。
            overlays::Toast::instance().show(ui::strings::kSetBtInitFail, 2000);
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            request_repaint();
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }
        // 判断条件：!mgr.is_advertising() 是否成立。
        if (!mgr.is_advertising())
        {
            // 执行语句：::app::network::bluetooth::AdvCfg cfg;；它会更新一次变量、对象或程序状态。
            ::app::network::bluetooth::AdvCfg cfg;
            // 赋值：把右侧 bt_name_ 的结果保存到左侧 cfg.device_name。
            cfg.device_name = bt_name_;
            // 判断条件：!mgr.start_advertising(cfg) 是否成立。
            if (!mgr.start_advertising(cfg))
            {
                // 调用 overlays::Toast::instance().show(ui::strings::kSetBtInitFail, 2000);，执行这个函数并处理它的返回值或状态变化。
                overlays::Toast::instance().show(ui::strings::kSetBtInitFail, 2000);
                // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
                request_repaint();
            }
        }
    }
    // 备用分支：前面的条件不成立时执行这里。
    else if (mgr.is_advertising())
    {
        // 访问对象成员：mgr.stop_advertising();；读取或修改这个对象里的成员状态。
        mgr.stop_advertising();
    }
}

// 定义或声明函数 SettingsApp::on_bt_name_done：参数在括号里，返回类型写在函数名前面。
void SettingsApp::on_bt_name_done(const char* text, void* user)
{
    // 赋值：把右侧 static_cast<SettingsApp*>(user) 的结果保存到左侧 auto* self。
    auto* self = static_cast<SettingsApp*>(user);
    // 判断条件：self == nullptr || text == nullptr 是否成立。
    if (self == nullptr || text == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 std::strncpy(self->bt_name_, text, sizeof(self->bt_name_) - 1);，执行这个函数并处理它的返回值或状态变化。
    std::strncpy(self->bt_name_, text, sizeof(self->bt_name_) - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 self->bt_name_[sizeof(self->bt_name_) - 1]。
    self->bt_name_[sizeof(self->bt_name_) - 1] = '\0';
    // 调用 bt_apply_state()：根据 SystemState.bluetooth： 打开时调用 BleMgr.init()，并按需要开始广播。 关闭时停止广播。。
    self->bt_apply_state();
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    self->request_repaint();
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::settings
