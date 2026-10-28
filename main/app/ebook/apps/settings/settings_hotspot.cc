/**
 * [初学者导读] 文件：main/app/ebook/apps/settings/settings_hotspot.cc
 * 作用：实现热点设置页的名称、密码、AP 模式和文件服务。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_app.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_netif.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_internal.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/persist.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/system_state.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/icon.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "network/wifi/wifi.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/keyboard.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/toast.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "protocol/file_server/file_server.hpp"
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
using detail::copy_str;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using detail::TAG;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using detail::tap_row_index;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 SettingsApp::paint_hotspot：参数在括号里，返回类型写在函数名前面。
void SettingsApp::paint_hotspot(gfx::Canvas& c)
{
    // 定义变量 int16_t y 并初始化为 Theme::kListStartY。
    int16_t y = Theme::kListStartY;
    // 赋值：把右侧 data::SystemState::get_instance() 的结果保存到左侧 auto& sys。
    auto& sys = data::SystemState::get_instance();

    // struct RowDef：把相关数据字段组合在一起，表示项目中的一个概念。
    struct RowDef
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* label;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* value;
        // 执行语句：bool chevron;；它会更新一次变量、对象或程序状态。
        bool chevron;
        // 执行语句：uint32_t icon;；它会更新一次变量、对象或程序状态。
        uint32_t icon;
    };
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint32_t toggle = gfx::icon::toggle(sys.hotspot());
    // 只读约束：这个值或对象在当前位置不能被修改。
    const RowDef rows[] = {
        // 执行这条代码：{ui::strings::kSetHotspotSwitch, nullptr, false, toggle},。它会更新变量、对象状态或调用链。
        {ui::strings::kSetHotspotSwitch, nullptr, false, toggle},
        // 继续上一行的表达式或参数列表：{ui::strings::kSetHotspotName, hotspot_name_, true, 0},。
        {ui::strings::kSetHotspotName, hotspot_name_, true, 0},
        // 继续上一行的表达式或参数列表：{ui::strings::kSetHotspotPwd, hotspot_pwd_, true, 0},。
        {ui::strings::kSetHotspotPwd, hotspot_pwd_, true, 0},
    };
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (const RowDef& r : rows)
    {
        // 声明并默认初始化对象：ui::widgets::RowStyle rs{}。
        ui::widgets::RowStyle rs{};
        // 赋值：把右侧 r.label 的结果保存到左侧 rs.label。
        rs.label = r.label;
        // 赋值：把右侧 r.value 的结果保存到左侧 rs.value。
        rs.value = r.value;
        // 赋值：把右侧 r.chevron 的结果保存到左侧 rs.show_chevron。
        rs.show_chevron = r.chevron;
        // 判断条件：r.icon != 0 是否成立。
        if (r.icon != 0)
        {
            // 赋值：把右侧 r.icon 的结果保存到左侧 rs.value_icon。
            rs.value_icon = r.icon;
            // 赋值：把右侧 20 的结果保存到左侧 rs.value_icon_size。
            rs.value_icon_size = 20;
        }
        // 调用 ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);，执行这个函数并处理它的返回值或状态变化。
        ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);
        // 赋值：把右侧 static_cast<int16_t>(y + Theme::kListRowH) 的结果保存到左侧 y。
        y = static_cast<int16_t>(y + Theme::kListRowH);
    }

    // 执行语句：char cnt[8];；它会更新一次变量、对象或程序状态。
    char cnt[8];
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t n =
        // 继续上一行的表达式或参数列表：sys.hotspot() ? ::app::network::wifi::WiFiMgr::get_instance().get_ap_sta_count() : 0;。
        sys.hotspot() ? ::app::network::wifi::WiFiMgr::get_instance().get_ap_sta_count() : 0;
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    (void)std::snprintf(cnt, sizeof(cnt), ui::strings::kSetConnCountFmt, static_cast<unsigned>(n));
    // 声明并默认初始化对象：ui::widgets::RowStyle rs{}。
    ui::widgets::RowStyle rs{};
    // 赋值：把右侧 ui::strings::kSetHotspotClients 的结果保存到左侧 rs.label。
    rs.label = ui::strings::kSetHotspotClients;
    // 赋值：把右侧 cnt 的结果保存到左侧 rs.value。
    rs.value = cnt;
    // 赋值：把右侧 false 的结果保存到左侧 rs.show_chevron。
    rs.show_chevron = false;
    // 调用 ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);，执行这个函数并处理它的返回值或状态变化。
    ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);
}

// 定义或声明函数 SettingsApp::handle_hotspot：参数在括号里，返回类型写在函数名前面。
shell::InputResult SettingsApp::handle_hotspot(const ::app::ebook::input::Event& ev)
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
        // 访问对象成员：sys.set_hotspot(!sys.hotspot());；读取或修改这个对象里的成员状态。
        sys.set_hotspot(!sys.hotspot());
        // 调用 hotspot_apply_state()：打开时： 1. 启动 WiFi AP。 2. 启动文件服务器。 3. 显示 AP IP，通常为 192.168.4.1。。
        hotspot_apply_state();
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 声明并默认初始化对象：overlays::KeyboardConfig kc{}。
    overlays::KeyboardConfig kc{};
    // 赋值：把右侧 overlays::KeyboardLayer::Letters 的结果保存到左侧 kc.default_layer。
    kc.default_layer = overlays::KeyboardLayer::Letters;
    // 赋值：把右侧 32 的结果保存到左侧 kc.max_len。
    kc.max_len = 32;

    // 判断条件：row == 1 是否成立。
    if (row == 1)
    {
        // 赋值：把右侧 hotspot_name_ 的结果保存到左侧 kc.initial_text。
        kc.initial_text = hotspot_name_;
        // 赋值：把右侧 ui::strings::kSetHotspotName 的结果保存到左侧 kc.title。
        kc.title = ui::strings::kSetHotspotName;
        // 调用 overlays::Keyboard::instance().open(kc, on_hotspot_name_done, this);，执行这个函数并处理它的返回值或状态变化。
        overlays::Keyboard::instance().open(kc, on_hotspot_name_done, this);
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：row == 2 是否成立。
    if (row == 2)
    {
        // 赋值：把右侧 hotspot_pwd_ 的结果保存到左侧 kc.initial_text。
        kc.initial_text = hotspot_pwd_;
        // 赋值：把右侧 ui::strings::kSetHotspotPwd 的结果保存到左侧 kc.title。
        kc.title = ui::strings::kSetHotspotPwd;
        // 调用 overlays::Keyboard::instance().open(kc, on_hotspot_pwd_done, this);，执行这个函数并处理它的返回值或状态变化。
        overlays::Keyboard::instance().open(kc, on_hotspot_pwd_done, this);
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 返回 {} 给调用者，表示当前函数的执行结果。
    return {};
}

// 函数定义/声明：调用 SettingsApp::hotspot_apply_state()：打开时： 1. 启动 WiFi AP。 2. 启动文件服务器。 3. 显示 AP IP，通常为 192.168.4.1。。
void SettingsApp::hotspot_apply_state()
{
    // 赋值：把右侧 ::app::network::wifi::WiFiMgr::get_instance() 的结果保存到左侧 auto& mgr。
    auto& mgr = ::app::network::wifi::WiFiMgr::get_instance();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool on = data::SystemState::get_instance().hotspot();
    // 判断条件：on 是否成立。
    if (on)
    {
        // 判断条件：!mgr.init() 是否成立。
        if (!mgr.init())
        {
            // 调用 EBOOK_LOGW(TAG, "wifi init failed; cannot enable hotspot");，执行这个函数并处理它的返回值或状态变化。
            EBOOK_LOGW(TAG, "wifi init failed; cannot enable hotspot");
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }
        // 判断条件：!mgr.enable_ap(hotspot_name_, hotspot_pwd_) 是否成立。
        if (!mgr.enable_ap(hotspot_name_, hotspot_pwd_))
        {
            // 调用 EBOOK_LOGW(TAG, "enable_ap failed");，执行这个函数并处理它的返回值或状态变化。
            EBOOK_LOGW(TAG, "enable_ap failed");
            // 调用 data::SystemState::get_instance().set_hotspot(false);，执行这个函数并处理它的返回值或状态变化。
            data::SystemState::get_instance().set_hotspot(false);
            // 调用 overlays::Toast::instance().show(ui::strings::kWifiFailed, 2000);，执行这个函数并处理它的返回值或状态变化。
            overlays::Toast::instance().show(ui::strings::kWifiFailed, 2000);
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }
        // 判断条件：::app::protocol::file_server::start() == ESP_OK 是否成立。
        if (::app::protocol::file_server::start() == ESP_OK)
        {
            // 赋值：把右侧 "192.168.4.1" 的结果保存到左侧 char ip[16]。
            char ip[16] = "192.168.4.1";
            // 赋值：把右侧 esp_netif_get_handle_from_ifkey("WIFI_AP_DEF") 的结果保存到左侧 esp_netif_t* ap。
            esp_netif_t* ap = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
            // 声明并默认初始化对象：esp_netif_ip_info_t info{}。
            esp_netif_ip_info_t info{};
            // 判断条件：ap != nullptr && esp_netif_get_ip_info(ap, &info) == ESP_OK && info.ip.addr != 0 是否成立。
            if (ap != nullptr && esp_netif_get_ip_info(ap, &info) == ESP_OK && info.ip.addr != 0)
            {
                // 访问对象成员：(void)std::snprintf(ip, sizeof(ip), IPSTR, IP2STR(&info.ip));；读取或修改这个对象里的成员状态。
                (void)std::snprintf(ip, sizeof(ip), IPSTR, IP2STR(&info.ip));
            }
            // 调用 show(ip,3000)：ensuretimer()：第一次调用创建一次性软件定时器，后续修改周期。。
            overlays::Toast::instance().show(ip, 3000);
        }
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 调用 app::protocol::file_server::stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
        ::app::protocol::file_server::stop();
        // 访问对象成员：mgr.disable_ap();；读取或修改这个对象里的成员状态。
        mgr.disable_ap();
    }
}

// 函数定义/声明：调用 SettingsApp::load_hotspot_prefs()：从 NVS 读取或保存名称和密码。加载失败时保留默认值。。
void SettingsApp::load_hotspot_prefs()
{
    // 赋值：把右侧 {} 的结果保存到左侧 char buf[33]。
    char buf[33] = {};
    // 判断条件：data::Persist::get_str("hs_name", buf, sizeof(buf)) && buf[0] != '\0' 是否成立。
    if (data::Persist::get_str("hs_name", buf, sizeof(buf)) && buf[0] != '\0')
        // 调用 copy_str(hotspot_name_, sizeof(hotspot_name_), buf);，执行这个函数并处理它的返回值或状态变化。
        copy_str(hotspot_name_, sizeof(hotspot_name_), buf);
    // 赋值：把右侧 '\0' 的结果保存到左侧 buf[0]。
    buf[0] = '\0';
    // 判断条件：data::Persist::get_str("hs_pwd", buf, sizeof(buf)) && buf[0] != '\0' 是否成立。
    if (data::Persist::get_str("hs_pwd", buf, sizeof(buf)) && buf[0] != '\0')
        // 调用 copy_str(hotspot_pwd_, sizeof(hotspot_pwd_), buf);，执行这个函数并处理它的返回值或状态变化。
        copy_str(hotspot_pwd_, sizeof(hotspot_pwd_), buf);
}

// 函数定义/声明：调用 SettingsApp::save_hotspot_prefs()：从 NVS 读取或保存名称和密码。加载失败时保留默认值。。
void SettingsApp::save_hotspot_prefs()
{
    // 调用 data::Persist::set_str("hs_name", hotspot_name_);，执行这个函数并处理它的返回值或状态变化。
    data::Persist::set_str("hs_name", hotspot_name_);
    // 调用 data::Persist::set_str("hs_pwd", hotspot_pwd_);，执行这个函数并处理它的返回值或状态变化。
    data::Persist::set_str("hs_pwd", hotspot_pwd_);
    // 调用 data::Persist::commit();，执行这个函数并处理它的返回值或状态变化。
    data::Persist::commit();
}

// 定义或声明函数 SettingsApp::on_hotspot_name_done：参数在括号里，返回类型写在函数名前面。
void SettingsApp::on_hotspot_name_done(const char* text, void* user)
{
    // 赋值：把右侧 static_cast<SettingsApp*>(user) 的结果保存到左侧 auto* self。
    auto* self = static_cast<SettingsApp*>(user);
    // 判断条件：self == nullptr 是否成立。
    if (self == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 copy_str(self->hotspot_name_, sizeof(self->hotspot_name_), text);，执行这个函数并处理它的返回值或状态变化。
    copy_str(self->hotspot_name_, sizeof(self->hotspot_name_), text);
    // 调用 save_hotspot_prefs()：从 NVS 读取或保存名称和密码。加载失败时保留默认值。。
    self->save_hotspot_prefs();
    // 判断条件：data::SystemState::get_instance().hotspot() 是否成立。
    if (data::SystemState::get_instance().hotspot())
        // 调用 hotspot_apply_state()：打开时： 1. 启动 WiFi AP。 2. 启动文件服务器。 3. 显示 AP IP，通常为 192.168.4.1。。
        self->hotspot_apply_state();
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    self->request_repaint();
}

// 定义或声明函数 SettingsApp::on_hotspot_pwd_done：参数在括号里，返回类型写在函数名前面。
void SettingsApp::on_hotspot_pwd_done(const char* text, void* user)
{
    // 赋值：把右侧 static_cast<SettingsApp*>(user) 的结果保存到左侧 auto* self。
    auto* self = static_cast<SettingsApp*>(user);
    // 判断条件：self == nullptr 是否成立。
    if (self == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 copy_str(self->hotspot_pwd_, sizeof(self->hotspot_pwd_), text);，执行这个函数并处理它的返回值或状态变化。
    copy_str(self->hotspot_pwd_, sizeof(self->hotspot_pwd_), text);
    // 调用 save_hotspot_prefs()：从 NVS 读取或保存名称和密码。加载失败时保留默认值。。
    self->save_hotspot_prefs();
    // 判断条件：data::SystemState::get_instance().hotspot() 是否成立。
    if (data::SystemState::get_instance().hotspot())
        // 调用 hotspot_apply_state()：打开时： 1. 启动 WiFi AP。 2. 启动文件服务器。 3. 显示 AP IP，通常为 192.168.4.1。。
        self->hotspot_apply_state();
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    self->request_repaint();
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::settings
