/**
 * [初学者导读] 文件：main/app/ebook/apps/settings/settings_wifi.cc
 * 作用：实现 Wi-Fi 设置页的开关、扫描、连接、保存网络和状态显示。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_app.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <algorithm>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <memory>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <mutex>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <vector>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/power_save.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_internal.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/task/task.hpp"
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
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_bus.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::settings {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using detail::copy_str;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using detail::ensure_wifi;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using detail::scroll_rows;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using detail::tap_row_index;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;

// 固定参数 kPendingCap = 16：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kPendingCap = 16;

// struct PendingScan：把相关数据字段组合在一起，表示项目中的一个概念。
struct PendingScan
{
    // 执行语句：SettingsApp::WifiAp items[kPendingCap]{};；它会更新一次变量、对象或程序状态。
    SettingsApp::WifiAp items[kPendingCap]{};
    // 执行语句：uint8_t count{0};；它会更新一次变量、对象或程序状态。
    uint8_t count{0};
    // 执行语句：bool ready{false};；它会更新一次变量、对象或程序状态。
    bool ready{false};
};

// 声明并默认初始化对象：PendingScan g_pending{}。
PendingScan g_pending{};
// 执行语句：std::mutex g_pending_mtx;；它会更新一次变量、对象或程序状态。
std::mutex g_pending_mtx;

// 执行语句：std::unique_ptr<::app::sys::task::Task> g_wifi_defer_task;；它会更新一次变量、对象或程序状态。
std::unique_ptr<::app::sys::task::Task> g_wifi_defer_task;
// 执行语句：uint32_t g_wifi_defer_ms{0};；它会更新一次变量、对象或程序状态。
uint32_t g_wifi_defer_ms{0};

// 任务栈大小：8192，栈太小会崩溃，太大会浪费 RAM。
constexpr uint32_t kWifiDeferStack = 8192;

// [初学者导读] wifi_apply_state_impl()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void wifi_apply_state_impl()
{
    // 赋值：把右侧 data::SystemState::get_instance() 的结果保存到左侧 auto& sys。
    auto& sys = data::SystemState::get_instance();
    // 赋值：把右侧 ::app::network::wifi::WiFiMgr::get_instance() 的结果保存到左侧 auto& mgr。
    auto& mgr = ::app::network::wifi::WiFiMgr::get_instance();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool on = sys.wifi();

    // 判断条件：!on 是否成立。
    if (!on)
    {
        // 访问对象成员：mgr.disconnect();；读取或修改这个对象里的成员状态。
        mgr.disconnect();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：!ensure_wifi() 是否成立。
    if (!ensure_wifi())
    {
        // 访问对象成员：sys.set_wifi(false);；读取或修改这个对象里的成员状态。
        sys.set_wifi(false);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto st = mgr.get_state();
    // 判断条件：st == ::app::network::wifi::State::CONNECTED | 是否成立。
    if (st == ::app::network::wifi::State::CONNECTED ||
        // 继续上一行的表达式或参数列表：st == ::app::network::wifi::State::CONNECTING)。
        st == ::app::network::wifi::State::CONNECTING)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：mgr.has_saved_creds() 是否成立。
    if (mgr.has_saved_creds())
        // 访问对象成员：(void)mgr.connect(nullptr, nullptr, 45000);；读取或修改这个对象里的成员状态。
        (void)mgr.connect(nullptr, nullptr, 45000);
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 函数定义/声明：调用 SettingsApp::wifi_apply_state()：SettingsApp::wifiapplystate()。
void SettingsApp::wifi_apply_state(uint32_t defer_ms)
{
    // 判断条件：PowerSave::get_instance().enabled() 是否成立。
    if (PowerSave::get_instance().enabled())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 判断条件：defer_ms == 0 是否成立。
    if (defer_ms == 0)
    {
        // 调用 wifi_apply_state_impl()：文件级内部函数，根据 SystemState.wifi 决定： 打开 WiFi 时调用初始化。 可用时自动尝试保存的凭据。 关闭时断开或进入停用状态。。
        wifi_apply_state_impl();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：g_wifi_defer_task && g_wifi_defer_task->valid() 是否成立。
    if (g_wifi_defer_task && g_wifi_defer_task->valid())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 defer_ms 的结果保存到左侧 g_wifi_defer_ms。
    g_wifi_defer_ms = defer_ms;

    // 定义变量：::app::sys::task::Cfg cfg =
    ::app::sys::task::Cfg cfg =
        // 继续上一行的表达式或参数列表：::app::sys::task::Cfg::light("wifi_dly", ::app::sys::task::Priority::LOW);。
        ::app::sys::task::Cfg::light("wifi_dly", ::app::sys::task::Priority::LOW);
    // 赋值：把右侧 kWifiDeferStack 的结果保存到左侧 cfg.stack_size。
    cfg.stack_size = kWifiDeferStack;

    // 执行这条代码：g_wifi_defer_task = std::make_unique<::app::sys::task::Task>(。它会更新变量、对象状态或调用链。
    g_wifi_defer_task = std::make_unique<::app::sys::task::Task>(
        // 开始代码块：下面是这个函数、分支或循环的主体。
        [](void* arg) {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint32_t ms = arg != nullptr ? *static_cast<uint32_t*>(arg) : 0U;
            // 判断条件：ms > 0 是否成立。
            if (ms > 0)
                // 调用 app::sys::task::TaskMgr::delay_ms(ms)：调用 vTaskDelay()，这是让出 CPU 的阻塞延时。一千毫秒延时期间，任务不占 CPU，调度器可以运行别人。。
                ::app::sys::task::TaskMgr::delay_ms(ms);
            // 调用 wifi_apply_state_impl()：文件级内部函数，根据 SystemState.wifi 决定： 打开 WiFi 时调用初始化。 可用时自动尝试保存的凭据。 关闭时断开或进入停用状态。。
            wifi_apply_state_impl();
            // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
            g_wifi_defer_task.reset();
        // 结束上一行的 lambda 或初始化列表：把这段匿名代码/数据交给调用者。
        },
        // 继续上一行的表达式或参数列表：cfg, &g_wifi_defer_ms);。
        cfg, &g_wifi_defer_ms);

    // 判断条件：!g_wifi_defer_task->start() 是否成立。
    if (!g_wifi_defer_task->start())
        // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
        g_wifi_defer_task.reset();
}

// 函数定义/声明：调用 SettingsApp::build_wifi_rows()：把“开关行、已保存网络、扫描按钮、扫描结果”组合成逻辑行描述。绘制和点击共用这张表，避免它们对行顺序理解不一致。。
uint8_t SettingsApp::build_wifi_rows(WifiRowDesc* out, uint8_t cap) const
{
    // 判断条件：out == nullptr || cap == 0 是否成立。
    if (out == nullptr || cap == 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 定义变量 uint8_t n 并初始化为 0。
    uint8_t n = 0;
    // 赋值：把右侧 {WifiRowKind::Toggle, 0} 的结果保存到左侧 out[n++]。
    out[n++] = {WifiRowKind::Toggle, 0};
    // 判断条件：!data::SystemState::get_instance().wifi() || n >= cap 是否成立。
    if (!data::SystemState::get_instance().wifi() || n >= cap)
        // 返回 n 给调用者，表示当前函数的执行结果。
        return n;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < wifi_saved_count_ && n < cap; ++i)
        // 赋值：把右侧 {WifiRowKind::SavedAp, i} 的结果保存到左侧 out[n++]。
        out[n++] = {WifiRowKind::SavedAp, i};

    // 判断条件：n >= cap 是否成立。
    if (n >= cap)
        // 返回 n 给调用者，表示当前函数的执行结果。
        return n;
    // 赋值：把右侧 {WifiRowKind::ScanTrigger, 0} 的结果保存到左侧 out[n++]。
    out[n++] = {WifiRowKind::ScanTrigger, 0};

    // 判断条件：!wifi_scanning_ 是否成立。
    if (!wifi_scanning_)
    {
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint8_t i = 0; i < wifi_scan_count_ && n < cap; ++i)
            // 赋值：把右侧 {WifiRowKind::ScannedAp, i} 的结果保存到左侧 out[n++]。
            out[n++] = {WifiRowKind::ScannedAp, i};
    }
    // 返回 n 给调用者，表示当前函数的执行结果。
    return n;
}

// 定义或声明函数 SettingsApp::paint_wifi：参数在括号里，返回类型写在函数名前面。
void SettingsApp::paint_wifi(gfx::Canvas& c)
{
    // 执行语句：WifiRowDesc rows[32];；它会更新一次变量、对象或程序状态。
    WifiRowDesc rows[32];
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t total = build_wifi_rows(rows, 32);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t visible = Theme::kListVisibleRows;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t max_off = (total > visible) ? static_cast<uint8_t>(total - visible) : 0;
    // 判断条件：wifi_scroll_row_ > max_off 是否成立。
    if (wifi_scroll_row_ > max_off)
        // 赋值：把右侧 max_off 的结果保存到左侧 wifi_scroll_row_。
        wifi_scroll_row_ = max_off;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool has_scroll = total > visible;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t row_w = static_cast<uint16_t>(Theme::kScreenW - (has_scroll ? 6 : 0));
    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto& sys = data::SystemState::get_instance();

    // 定义变量 int16_t y 并初始化为 Theme::kListStartY。
    int16_t y = Theme::kListStartY;
    // 执行语句：char val_buf[24];；它会更新一次变量、对象或程序状态。
    char val_buf[24];

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t end = static_cast<uint8_t>(wifi_scroll_row_ + visible);
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t r = wifi_scroll_row_; r < end && r < total; ++r)
    {
        // 声明并默认初始化对象：ui::widgets::RowStyle rs{}。
        ui::widgets::RowStyle rs{};
        // 只读约束：这个值或对象在当前位置不能被修改。
        const WifiRowDesc& row = rows[r];

        // 多分支判断：根据括号里的值选择不同 case。
        switch (row.kind)
        {
            // 分支匹配：case WifiRowKind::Toggle 成立时执行下面的代码。
            case WifiRowKind::Toggle:
                // 赋值：把右侧 ui::strings::kCtlWifi 的结果保存到左侧 rs.label。
                rs.label = ui::strings::kCtlWifi;
                // 赋值：把右侧 gfx::icon::toggle(sys.wifi()) 的结果保存到左侧 rs.value_icon。
                rs.value_icon = gfx::icon::toggle(sys.wifi());
                // 赋值：把右侧 20 的结果保存到左侧 rs.value_icon_size。
                rs.value_icon_size = 20;
                // 赋值：把右侧 false 的结果保存到左侧 rs.show_chevron。
                rs.show_chevron = false;
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 分支匹配：case WifiRowKind::SavedAp 成立时执行下面的代码。
            case WifiRowKind::SavedAp:
                // 判断条件：row.index < wifi_saved_count_ 是否成立。
                if (row.index < wifi_saved_count_)
                {
                    // 赋值：把右侧 wifi_saved_[row.index].ssid 的结果保存到左侧 rs.label。
                    rs.label = wifi_saved_[row.index].ssid;
                    // 赋值：把右侧 false 的结果保存到左侧 rs.show_chevron。
                    rs.show_chevron = false;
                    // 赋值：把右侧 gfx::icon::kFaTimes 的结果保存到左侧 rs.action_icon。
                    rs.action_icon = gfx::icon::kFaTimes;
                    // 赋值：把右侧 16 的结果保存到左侧 rs.action_icon_size。
                    rs.action_icon_size = 16;
                    // 判断条件：wifi_connected_ssid_[0] != '\0' & 是否成立。
                    if (wifi_connected_ssid_[0] != '\0' &&
                        // 定义或声明函数 std::strcmp：参数在括号里，返回类型写在函数名前面。
                        std::strcmp(wifi_saved_[row.index].ssid, wifi_connected_ssid_) == 0)
                        // 赋值：把右侧 gfx::icon::kFaCheck 的结果保存到左侧 rs.icon_cp。
                        rs.icon_cp = gfx::icon::kFaCheck;
                }
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 分支匹配：case WifiRowKind::ScanTrigger 成立时执行下面的代码。
            case WifiRowKind::ScanTrigger:
                // 赋值：把右侧 ui::strings::kSetWifiScan 的结果保存到左侧 rs.label。
                rs.label = ui::strings::kSetWifiScan;
                // 执行这条代码：rs.value =。它会更新变量、对象状态或调用链。
                rs.value =
                    // 继续上一行的表达式或参数列表：wifi_scanning_ ? ui::strings::kSetWifiScanning : ui::strings::kSetWifiClickScan;。
                    wifi_scanning_ ? ui::strings::kSetWifiScanning : ui::strings::kSetWifiClickScan;
                // 赋值：把右侧 !wifi_scanning_ 的结果保存到左侧 rs.show_chevron。
                rs.show_chevron = !wifi_scanning_;
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 分支匹配：case WifiRowKind::ScannedAp 成立时执行下面的代码。
            case WifiRowKind::ScannedAp:
                // 判断条件：row.index < wifi_scan_count_ 是否成立。
                if (row.index < wifi_scan_count_)
                {
                    // 函数调用：(void)std::snprintf(val_buf, sizeof(val_buf), "%s%d",；执行该函数并处理它的返回值或副作用。
                    (void)std::snprintf(val_buf, sizeof(val_buf), "%s%d",
                                        // 继续上一行的表达式或参数列表：wifi_scan_[row.index].encrypted ? "* " : "  ",。
                                        wifi_scan_[row.index].encrypted ? "* " : "  ",
                                        // 继续上一行的表达式或参数列表：static_cast<int>(wifi_scan_[row.index].rssi));。
                                        static_cast<int>(wifi_scan_[row.index].rssi));
                    // 赋值：把右侧 wifi_scan_[row.index].ssid 的结果保存到左侧 rs.label。
                    rs.label = wifi_scan_[row.index].ssid;
                    // 赋值：把右侧 val_buf 的结果保存到左侧 rs.value。
                    rs.value = val_buf;
                    // 赋值：把右侧 true 的结果保存到左侧 rs.show_chevron。
                    rs.show_chevron = true;
                    // 判断条件：wifi_connected_ssid_[0] != '\0' & 是否成立。
                    if (wifi_connected_ssid_[0] != '\0' &&
                        // 定义或声明函数 std::strcmp：参数在括号里，返回类型写在函数名前面。
                        std::strcmp(wifi_scan_[row.index].ssid, wifi_connected_ssid_) == 0)
                        // 赋值：把右侧 gfx::icon::kFaCheck 的结果保存到左侧 rs.icon_cp。
                        rs.icon_cp = gfx::icon::kFaCheck;
                }
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
        }
        // 调用 ui::widgets::list_row(c, core::Rect{0, y, row_w, Theme::kListRowH}, rs);，执行这个函数并处理它的返回值或状态变化。
        ui::widgets::list_row(c, core::Rect{0, y, row_w, Theme::kListRowH}, rs);
        // 赋值：把右侧 static_cast<int16_t>(y + Theme::kListRowH) 的结果保存到左侧 y。
        y = static_cast<int16_t>(y + Theme::kListRowH);
    }

    // 判断条件：has_scroll 是否成立。
    if (has_scroll)
    {
        // 调用 ui::widgets::scrollbar(c, Theme::kListStartY, Theme::kListRegionH, wifi_scroll_row_, total,，执行这个函数并处理它的返回值或状态变化。
        ui::widgets::scrollbar(c, Theme::kListStartY, Theme::kListRegionH, wifi_scroll_row_, total,
                               // 继续上一行的表达式或参数列表：visible);。
                               visible);
    }
}

// 函数定义/声明：调用 SettingsApp::handle_wifi()：SettingsApp::handlewifi()。
shell::InputResult SettingsApp::handle_wifi(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;
    // 赋值：把右侧 data::SystemState::get_instance() 的结果保存到左侧 auto& sys。
    auto& sys = data::SystemState::get_instance();

    // 执行语句：WifiRowDesc rows[32];；它会更新一次变量、对象或程序状态。
    WifiRowDesc rows[32];
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t total = build_wifi_rows(rows, 32);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t visible = Theme::kListVisibleRows;

    // 判断条件：ev.start_y > Theme::kStatusBarH + Theme::kToolbarH 是否成立。
    if (ev.start_y > Theme::kStatusBarH + Theme::kToolbarH)
    {
        // 判断条件：scroll_rows(wifi_scroll_row_, total, visible, ev.type) 是否成立。
        if (scroll_rows(wifi_scroll_row_, total, visible, ev.type))
        {
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            request_repaint();
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        }
    }
    // 判断条件：ev.type != EventType::Tap 是否成立。
    if (ev.type != EventType::Tap)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x = static_cast<int16_t>(ev.x);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y = static_cast<int16_t>(ev.y);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t local = tap_row_index(y);
    // 判断条件：local >= visible 是否成立。
    if (local >= visible)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t idx = static_cast<uint8_t>(wifi_scroll_row_ + local);
    // 判断条件：idx >= total 是否成立。
    if (idx >= total)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool has_scroll = total > visible;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t row_w = static_cast<uint16_t>(Theme::kScreenW - (has_scroll ? 6 : 0));
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect row_rect{0,
                              // 继续上一行的表达式或参数列表：static_cast<int16_t>(Theme::kListStartY + local * Theme::kListRowH),。
                              static_cast<int16_t>(Theme::kListStartY + local * Theme::kListRowH),
                              // 继续上一行的表达式或参数列表：row_w, Theme::kListRowH};。
                              row_w, Theme::kListRowH};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const WifiRowDesc& row = rows[idx];
    // 多分支判断：根据括号里的值选择不同 case。
    switch (row.kind)
    {
        // 分支匹配：case WifiRowKind::Toggle 成立时执行下面的代码。
        case WifiRowKind::Toggle:
            // 访问对象成员：sys.set_wifi(!sys.wifi());；读取或修改这个对象里的成员状态。
            sys.set_wifi(!sys.wifi());
            // 调用 wifi_apply_state();，执行这个函数并处理它的返回值或状态变化。
            wifi_apply_state();
            // 判断条件：sys.wifi() 是否成立。
            if (sys.wifi())
                // 调用 wifi_refresh_saved()：从网络管理器读取保存的凭据列表，更新最多 8 个 WifiAp。。
                wifi_refresh_saved();
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            request_repaint();
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};

        // 分支匹配：case WifiRowKind::SavedAp 成立时执行下面的代码。
        case WifiRowKind::SavedAp:
            // 判断条件：row.index >= wifi_saved_count_ 是否成立。
            if (row.index >= wifi_saved_count_)
                // 返回 {} 给调用者，表示当前函数的执行结果。
                return {};
            // 判断条件：ui::widgets::hit_row_action(row_rect, x, y) 是否成立。
            if (ui::widgets::hit_row_action(row_rect, x, y))
                // 调用 wifi_forget(wifi_saved_[row.index].ssid)：删除保存的凭据并刷新列表。它只影响保存记录，不一定立即断开当前连接；具体行为取决于网络管理接口。。
                wifi_forget(wifi_saved_[row.index].ssid);
            // 备用分支：前面的条件不成立时执行这里。
            else
                // 调用 wifi_reconnect_saved(wifi_saved_[row.index].ssid)：使用已保存凭据重新连接。SSID 是用户可控字符串，复制时必须限制长度。。
                wifi_reconnect_saved(wifi_saved_[row.index].ssid);
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            request_repaint();
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};

        // 分支匹配：case WifiRowKind::ScanTrigger 成立时执行下面的代码。
        case WifiRowKind::ScanTrigger:
            // 判断条件：!wifi_scanning_ 是否成立。
            if (!wifi_scanning_)
                // 调用 wifi_start_scan()：启动扫描。要防止重复扫描和扫描尚未结束时再次点击导致状态混乱。。
                wifi_start_scan();
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            request_repaint();
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};

        // 分支匹配：case WifiRowKind::ScannedAp 成立时执行下面的代码。
        case WifiRowKind::ScannedAp:
            // 判断条件：row.index >= wifi_scan_count_ 是否成立。
            if (row.index >= wifi_scan_count_)
                // 返回 {} 给调用者，表示当前函数的执行结果。
                return {};
            {
                // 只读约束：这个值或对象在当前位置不能被修改。
                const WifiAp& ap = wifi_scan_[row.index];
                // 判断条件：ap.encrypted 是否成立。
                if (ap.encrypted)
                {
                    // 调用 copy_str(wifi_pending_ssid_, sizeof(wifi_pending_ssid_), ap.ssid);，执行这个函数并处理它的返回值或状态变化。
                    copy_str(wifi_pending_ssid_, sizeof(wifi_pending_ssid_), ap.ssid);
                    // 赋值：把右侧 row.index 的结果保存到左侧 wifi_pending_idx_。
                    wifi_pending_idx_ = row.index;
                    // 声明并默认初始化对象：overlays::KeyboardConfig kc{}。
                    overlays::KeyboardConfig kc{};
                    // 赋值：把右侧 overlays::KeyboardLayer::Letters 的结果保存到左侧 kc.default_layer。
                    kc.default_layer = overlays::KeyboardLayer::Letters;
                    // 赋值：把右侧 63 的结果保存到左侧 kc.max_len。
                    kc.max_len = 63;
                    // 赋值：把右侧 ui::strings::kSetWifiPwd 的结果保存到左侧 kc.title。
                    kc.title = ui::strings::kSetWifiPwd;
                    // 调用 overlays::Keyboard::instance().open(kc, on_wifi_pwd_done, this);，执行这个函数并处理它的返回值或状态变化。
                    overlays::Keyboard::instance().open(kc, on_wifi_pwd_done, this);
                }
                // 备用分支：前面的条件不成立时执行这里。
                else
                {
                    // 调用 wifi_connect_to(row.index,"")：根据扫描结果索引连接。密码可以为空，表示开放网络或使用已保存密码。连接前要检查索引范围。。
                    wifi_connect_to(row.index, "");
                }
            }
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
    }
    // 返回 {} 给调用者，表示当前函数的执行结果。
    return {};
}

// 定义或声明函数 SettingsApp::on_wifi_state：参数在括号里，返回类型写在函数名前面。
void SettingsApp::on_wifi_state(uint8_t state, uint8_t fail)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using NetState = ::app::network::wifi::State;
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using NetFail = ::app::network::wifi::FailureReason;

    // 调用 try_ntp_auto_sync_on_connect(state)：WiFi 连接状态变化时触发自动同步。只有连接成功才继续，其他状态不应反复发 NTP 请求。。
    try_ntp_auto_sync_on_connect(state);
    // 判断条件：page_ != SettingsPage::Wifi 是否成立。
    if (page_ != SettingsPage::Wifi)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const NetState st = static_cast<NetState>(state);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const NetFail fl = static_cast<NetFail>(fail);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t prev_state = wifi_state_;
    // 赋值：把右侧 state 的结果保存到左侧 wifi_state_。
    wifi_state_ = state;

    // 赋值：把右侧 overlays::Toast::instance() 的结果保存到左侧 auto& toast。
    auto& toast = overlays::Toast::instance();
    // 执行语句：char msg[64];；它会更新一次变量、对象或程序状态。
    char msg[64];

    // 多分支判断：根据括号里的值选择不同 case。
    switch (st)
    {
        // 分支匹配：case NetState::CONNECTED:  成立时执行下面的代码。
        case NetState::CONNECTED: {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const auto& info = ::app::network::wifi::WiFiMgr::get_instance().get_info();
            // 调用 copy_str(wifi_connected_ssid_, sizeof(wifi_connected_ssid_), info.ssid);，执行这个函数并处理它的返回值或状态变化。
            copy_str(wifi_connected_ssid_, sizeof(wifi_connected_ssid_), info.ssid);
            // 函数调用：(void)std::snprintf(msg, sizeof(msg), ui::strings::kWifiConnectedFmt,；执行该函数并处理它的返回值或副作用。
            (void)std::snprintf(msg, sizeof(msg), ui::strings::kWifiConnectedFmt,
                                // 继续上一行的表达式或参数列表：wifi_connected_ssid_);。
                                wifi_connected_ssid_);
            // 调用 show(msg,2500)：ensuretimer()：第一次调用创建一次性软件定时器，后续修改周期。。
            toast.show(msg, 2500);
            // 调用 wifi_refresh_saved()：从网络管理器读取保存的凭据列表，更新最多 8 个 WifiAp。。
            wifi_refresh_saved();
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        }
        // 分支匹配：case NetState::FAILED:  成立时执行下面的代码。
        case NetState::FAILED: {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const char* reason = ui::strings::kWifiFailed;
            // 多分支判断：根据括号里的值选择不同 case。
            switch (fl)
            {
                // 分支匹配：case NetFail::WRONG_PASSWORD 成立时执行下面的代码。
                case NetFail::WRONG_PASSWORD:
                    // 赋值：把右侧 ui::strings::kWifiWrongPwd 的结果保存到左侧 reason。
                    reason = ui::strings::kWifiWrongPwd;
                    // 跳出当前循环或 switch，不再执行剩余分支。
                    break;
                // 分支匹配：case NetFail::TIMEOUT 成立时执行下面的代码。
                case NetFail::TIMEOUT:
                    // 赋值：把右侧 ui::strings::kWifiTimeout 的结果保存到左侧 reason。
                    reason = ui::strings::kWifiTimeout;
                    // 跳出当前循环或 switch，不再执行剩余分支。
                    break;
                // 分支匹配：case NetFail::NETWORK_NOT_FOUND 成立时执行下面的代码。
                case NetFail::NETWORK_NOT_FOUND:
                    // 赋值：把右侧 ui::strings::kWifiNotFound 的结果保存到左侧 reason。
                    reason = ui::strings::kWifiNotFound;
                    // 跳出当前循环或 switch，不再执行剩余分支。
                    break;
                // 执行这条代码：default:。它会更新变量、对象状态或调用链。
                default:
                    // 跳出当前循环或 switch，不再执行剩余分支。
                    break;
            }
            // 调用 show(reason,2500)：ensuretimer()：第一次调用创建一次性软件定时器，后续修改周期。。
            toast.show(reason, 2500);
            // 赋值：把右侧 '\0' 的结果保存到左侧 wifi_connected_ssid_[0]。
            wifi_connected_ssid_[0] = '\0';
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        }
        // 分支匹配：case NetState::CONNECTING 成立时执行下面的代码。
        case NetState::CONNECTING:
            // 判断条件：prev_state != state 是否成立。
            if (prev_state != state)
                // 访问对象成员：toast.show(ui::strings::kWifiConnecting, 2000);；读取或修改这个对象里的成员状态。
                toast.show(ui::strings::kWifiConnecting, 2000);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case NetState::DISCONNECTED 成立时执行下面的代码。
        case NetState::DISCONNECTED:
            // 赋值：把右侧 '\0' 的结果保存到左侧 wifi_connected_ssid_[0]。
            wifi_connected_ssid_[0] = '\0';
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 函数定义/声明：调用 SettingsApp::on_wifi_scan_done()：扫描回调先把结果复制到 gpending，并用 gpendingmtx 保护；完成后投递 WifiScanDone。UI 线程收到事件后再读取 gpending 更新 wifiscan。。
void SettingsApp::on_wifi_scan_done()
{
    // 赋值：把右侧 false 的结果保存到左侧 wifi_scanning_。
    wifi_scanning_ = false;
    // 赋值：把右侧 0 的结果保存到左侧 wifi_scan_count_。
    wifi_scan_count_ = 0;

    {
        // 函数调用：std::lock_guard<std::mutex> lk(g_pending_mtx);；执行该函数并处理它的返回值或副作用。
        std::lock_guard<std::mutex> lk(g_pending_mtx);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t n = (g_pending.count < kMaxScan) ? g_pending.count : kMaxScan;
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint8_t i = 0; i < n; ++i)
            // 赋值：把右侧 g_pending.items[i] 的结果保存到左侧 wifi_scan_[i]。
            wifi_scan_[i] = g_pending.items[i];
        // 赋值：把右侧 n 的结果保存到左侧 wifi_scan_count_。
        wifi_scan_count_ = n;
        // 调用 std::sort(wifi_scan_, wifi_scan_ + wifi_scan_count_,，执行这个函数并处理它的返回值或状态变化。
        std::sort(wifi_scan_, wifi_scan_ + wifi_scan_count_,
                  // 继续上一行的表达式或参数列表：[](const WifiAp& a, const WifiAp& b) { return a.rssi > b.rssi; });。
                  [](const WifiAp& a, const WifiAp& b) { return a.rssi > b.rssi; });
        // 赋值：把右侧 false 的结果保存到左侧 g_pending.ready。
        g_pending.ready = false;
    }

    // 判断条件：page_ == SettingsPage::Wifi 是否成立。
    if (page_ == SettingsPage::Wifi)
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
}

// 函数定义/声明：调用 SettingsApp::wifi_enter()：进入 WiFi 页时刷新已保存网络，必要时触发扫描，更新页面状态。。
void SettingsApp::wifi_enter()
{
    // 赋值：把右侧 false 的结果保存到左侧 wifi_scanning_。
    wifi_scanning_ = false;
    // 赋值：把右侧 0 的结果保存到左侧 wifi_scan_count_。
    wifi_scan_count_ = 0;
    // 赋值：把右侧 0 的结果保存到左侧 wifi_saved_count_。
    wifi_saved_count_ = 0;
    // 赋值：把右侧 0xFF 的结果保存到左侧 wifi_pending_idx_。
    wifi_pending_idx_ = 0xFF;
    // 赋值：把右侧 '\0' 的结果保存到左侧 wifi_pending_ssid_[0]。
    wifi_pending_ssid_[0] = '\0';
    // 赋值：把右侧 '\0' 的结果保存到左侧 wifi_connected_ssid_[0]。
    wifi_connected_ssid_[0] = '\0';
    // 赋值：把右侧 0 的结果保存到左侧 wifi_state_。
    wifi_state_ = 0;
    // 赋值：把右侧 0 的结果保存到左侧 wifi_scroll_row_。
    wifi_scroll_row_ = 0;

    // 判断条件：!data::SystemState::get_instance().wifi() 是否成立。
    if (!data::SystemState::get_instance().wifi())
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：!ensure_wifi() 是否成立。
    if (!ensure_wifi())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto& info = ::app::network::wifi::WiFiMgr::get_instance().get_info();
    // 赋值：把右侧 static_cast<uint8_t>(info.state) 的结果保存到左侧 wifi_state_。
    wifi_state_ = static_cast<uint8_t>(info.state);
    // 判断条件：info.state == ::app::network::wifi::State::CONNECTED 是否成立。
    if (info.state == ::app::network::wifi::State::CONNECTED)
    {
        // 调用 copy_str(wifi_connected_ssid_, sizeof(wifi_connected_ssid_), info.ssid);，执行这个函数并处理它的返回值或状态变化。
        copy_str(wifi_connected_ssid_, sizeof(wifi_connected_ssid_), info.ssid);
        // 访问对象成员：(void)::app::network::wifi::WiFiMgr::get_instance().persist_sta_creds();；读取或修改这个对象里的成员状态。
        (void)::app::network::wifi::WiFiMgr::get_instance().persist_sta_creds();
    }

    // 调用 wifi_refresh_saved()：从网络管理器读取保存的凭据列表，更新最多 8 个 WifiAp。。
    wifi_refresh_saved();
}

// 函数定义/声明：调用 SettingsApp::wifi_leave()：离开页面时停止扫描相关的 UI 状态。它不关闭整个 WiFi 网络，也不删除保存的 SSID。。
void SettingsApp::wifi_leave()
{
    // 赋值：把右侧 false 的结果保存到左侧 wifi_scanning_。
    wifi_scanning_ = false;
}

// 函数定义/声明：调用 SettingsApp::wifi_start_scan()：启动扫描。要防止重复扫描和扫描尚未结束时再次点击导致状态混乱。。
void SettingsApp::wifi_start_scan()
{
    // 判断条件：wifi_scanning_ 是否成立。
    if (wifi_scanning_)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 true 的结果保存到左侧 wifi_scanning_。
    wifi_scanning_ = true;
    // 赋值：把右侧 0 的结果保存到左侧 wifi_scan_count_。
    wifi_scan_count_ = 0;

    // 赋值：把右侧 ::app::network::wifi::WiFiMgr::get_instance() 的结果保存到左侧 auto& mgr。
    auto& mgr = ::app::network::wifi::WiFiMgr::get_instance();
    // 判断条件：!mgr.scan([](const std::vector<::app::network::wifi::ApInfo>& aps 是否成立。
    if (!mgr.scan([](const std::vector<::app::network::wifi::ApInfo>& aps) {
            // 函数调用：std::lock_guard<std::mutex> lk(g_pending_mtx);；执行该函数并处理它的返回值或副作用。
            std::lock_guard<std::mutex> lk(g_pending_mtx);
            // 赋值：把右侧 0 的结果保存到左侧 g_pending.count。
            g_pending.count = 0;
            // 循环执行：按括号里的初始化、条件和步进规则重复运行。
            for (const auto& ap : aps)
            {
                // 判断条件：g_pending.count >= kPendingCap 是否成立。
                if (g_pending.count >= kPendingCap)
                    // 跳出当前循环或 switch，不再执行剩余分支。
                    break;
                // 判断条件：ap.ssid[0] == '\0' 是否成立。
                if (ap.ssid[0] == '\0')
                    // 继续下一轮：跳过本次循环剩余代码。
                    continue;
                // 赋值：把右侧 g_pending.items[g_pending.count++] 的结果保存到左侧 SettingsApp::WifiAp& w。
                SettingsApp::WifiAp& w = g_pending.items[g_pending.count++];
                // 调用 std::strncpy(w.ssid, ap.ssid, sizeof(w.ssid) - 1);，执行这个函数并处理它的返回值或状态变化。
                std::strncpy(w.ssid, ap.ssid, sizeof(w.ssid) - 1);
                // 赋值：把右侧 '\0' 的结果保存到左侧 w.ssid[sizeof(w.ssid) - 1]。
                w.ssid[sizeof(w.ssid) - 1] = '\0';
                // 赋值：把右侧 ap.rssi 的结果保存到左侧 w.rssi。
                w.rssi = ap.rssi;
                // 赋值：把右侧 ap.encrypted 的结果保存到左侧 w.encrypted。
                w.encrypted = ap.encrypted;
            }
            // 赋值：把右侧 true 的结果保存到左侧 g_pending.ready。
            g_pending.ready = true;
            // 访问对象成员：(void)ui::UiBus::get_instance().post_wifi_scan_done();；读取或修改这个对象里的成员状态。
            (void)ui::UiBus::get_instance().post_wifi_scan_done();
        // 结束上一行的 lambda 或初始化列表：把这段匿名代码/数据交给调用者。
        }))
    {
        // 赋值：把右侧 false 的结果保存到左侧 wifi_scanning_。
        wifi_scanning_ = false;
    }
}

// 函数定义/声明：调用 SettingsApp::wifi_refresh_saved()：从网络管理器读取保存的凭据列表，更新最多 8 个 WifiAp。。
void SettingsApp::wifi_refresh_saved()
{
    // 赋值：把右侧 0 的结果保存到左侧 wifi_saved_count_。
    wifi_saved_count_ = 0;
    // 执行语句：std::vector<::app::network::wifi::Credentials> creds;；它会更新一次变量、对象或程序状态。
    std::vector<::app::network::wifi::Credentials> creds;
    // 判断条件：!::app::network::wifi::WiFiMgr::get_instance().get_creds(creds) 是否成立。
    if (!::app::network::wifi::WiFiMgr::get_instance().get_creds(creds))
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (const auto& c : creds)
    {
        // 判断条件：wifi_saved_count_ >= kMaxSaved 是否成立。
        if (wifi_saved_count_ >= kMaxSaved)
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 判断条件：c.ssid[0] == '\0' 是否成立。
        if (c.ssid[0] == '\0')
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        // 调用 copy_str(wifi_saved_[wifi_saved_count_].ssid, sizeof(wifi_saved_[wifi_saved_count_].ssid),，执行这个函数并处理它的返回值或状态变化。
        copy_str(wifi_saved_[wifi_saved_count_].ssid, sizeof(wifi_saved_[wifi_saved_count_].ssid),
                 // 继续上一行的表达式或参数列表：c.ssid);。
                 c.ssid);
        // 执行语句：++wifi_saved_count_;；它会更新一次变量、对象或程序状态。
        ++wifi_saved_count_;
    }
}

// 函数定义/声明：调用 SettingsApp::wifi_connect_to()：根据扫描结果索引连接。密码可以为空，表示开放网络或使用已保存密码。连接前要检查索引范围。。
void SettingsApp::wifi_connect_to(uint8_t scan_idx, const char* password)
{
    // 判断条件：scan_idx >= wifi_scan_count_ 是否成立。
    if (scan_idx >= wifi_scan_count_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const WifiAp& ap = wifi_scan_[scan_idx];
    // 访问对象成员：(void)::app::network::wifi::WiFiMgr::get_instance().connect(ap.ssid, password, 45000);；读取或修改这个对象里的成员状态。
    (void)::app::network::wifi::WiFiMgr::get_instance().connect(ap.ssid, password, 45000);
}

// 函数定义/声明：调用 SettingsApp::wifi_reconnect_saved()：使用已保存凭据重新连接。SSID 是用户可控字符串，复制时必须限制长度。。
void SettingsApp::wifi_reconnect_saved(const char* ssid)
{
    // 判断条件：ssid == nullptr || ssid[0] == '\0' 是否成立。
    if (ssid == nullptr || ssid[0] == '\0')
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 ::app::network::wifi::WiFiMgr::get_instance() 的结果保存到左侧 auto& mgr。
    auto& mgr = ::app::network::wifi::WiFiMgr::get_instance();
    // 执行语句：std::vector<::app::network::wifi::Credentials> creds;；它会更新一次变量、对象或程序状态。
    std::vector<::app::network::wifi::Credentials> creds;
    // 判断条件：!mgr.get_creds(creds) 是否成立。
    if (!mgr.get_creds(creds))
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (const auto& c : creds)
    {
        // 判断条件：std::strcmp(c.ssid, ssid) == 0 是否成立。
        if (std::strcmp(c.ssid, ssid) == 0)
        {
            // 访问对象成员：(void)mgr.connect(ssid, c.password, 45000);；读取或修改这个对象里的成员状态。
            (void)mgr.connect(ssid, c.password, 45000);
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }
    }
    // 访问对象成员：(void)mgr.connect(ssid, "", 45000);；读取或修改这个对象里的成员状态。
    (void)mgr.connect(ssid, "", 45000);
}

// 函数定义/声明：调用 SettingsApp::wifi_forget()：删除保存的凭据并刷新列表。它只影响保存记录，不一定立即断开当前连接；具体行为取决于网络管理接口。。
void SettingsApp::wifi_forget(const char* ssid)
{
    // 判断条件：ssid == nullptr || ssid[0] == '\0' 是否成立。
    if (ssid == nullptr || ssid[0] == '\0')
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 ::app::network::wifi::WiFiMgr::get_instance() 的结果保存到左侧 auto& mgr。
    auto& mgr = ::app::network::wifi::WiFiMgr::get_instance();
    // 访问对象成员：(void)mgr.remove_creds(ssid);；读取或修改这个对象里的成员状态。
    (void)mgr.remove_creds(ssid);

    // 判断条件：wifi_connected_ssid_[0] != '\0' && std::strcmp(ssid, wifi_connected_ssid_) == 0 是否成立。
    if (wifi_connected_ssid_[0] != '\0' && std::strcmp(ssid, wifi_connected_ssid_) == 0)
    {
        // 访问对象成员：mgr.disconnect();；读取或修改这个对象里的成员状态。
        mgr.disconnect();
        // 赋值：把右侧 '\0' 的结果保存到左侧 wifi_connected_ssid_[0]。
        wifi_connected_ssid_[0] = '\0';
    }
    // 调用 wifi_refresh_saved()：从网络管理器读取保存的凭据列表，更新最多 8 个 WifiAp。。
    wifi_refresh_saved();

    // 执行语句：char msg[64];；它会更新一次变量、对象或程序状态。
    char msg[64];
    // 函数调用：(void)std::snprintf(msg, sizeof(msg), "\u5DF2\u5FD8\u8BB0 %s", ssid);；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(msg, sizeof(msg), "\u5DF2\u5FD8\u8BB0 %s", ssid);
    // 调用 show(msg,2000)：ensuretimer()：第一次调用创建一次性软件定时器，后续修改周期。。
    overlays::Toast::instance().show(msg, 2000);
}

// 函数定义/声明：调用 SettingsApp::on_wifi_pwd_done()：键盘提交回调。它把密码保存到待连接状态，然后回到 UI 线程执行真正连接。。
void SettingsApp::on_wifi_pwd_done(const char* text, void* user)
{
    // 赋值：把右侧 static_cast<SettingsApp*>(user) 的结果保存到左侧 auto* self。
    auto* self = static_cast<SettingsApp*>(user);
    // 判断条件：self == nullptr 是否成立。
    if (self == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：self->wifi_pending_idx_ >= self->wifi_scan_count_ 是否成立。
    if (self->wifi_pending_idx_ >= self->wifi_scan_count_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 wifi_connect_to(self->wifi_pending_idx_,text?text:"")：根据扫描结果索引连接。密码可以为空，表示开放网络或使用已保存密码。连接前要检查索引范围。。
    self->wifi_connect_to(self->wifi_pending_idx_, text ? text : "");
    // 赋值：把右侧 0xFF 的结果保存到左侧 self->wifi_pending_idx_。
    self->wifi_pending_idx_ = 0xFF;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::settings
