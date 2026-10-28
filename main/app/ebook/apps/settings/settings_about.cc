/**
 * [初学者导读] 文件：main/app/ebook/apps/settings/settings_about.cc
 * 作用：显示设备名、MAC、IP、固件版本和内存信息。
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
#include <esp_mac.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_internal.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "network/wifi/wifi.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ota/ota.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/info/info.hpp"
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
using detail::scroll_rows;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 SettingsApp::paint_about：参数在括号里，返回类型写在函数名前面。
void SettingsApp::paint_about(gfx::Canvas& c)
{
    // 赋值：把右侧 {} 的结果保存到左侧 char fw[24]。
    char fw[24] = {};
    // 调用 get_current_version()：返回当前固件版本，供关于页面显示。。
    std::strncpy(fw, ::app::common::ota::OtaMgr::get_instance().get_current_version().c_str(),
                 // 继续上一行的表达式或参数列表：sizeof(fw) - 1);。
                 sizeof(fw) - 1);

    // 赋值：把右侧 {} 的结果保存到左侧 char cpu[16]。
    char cpu[16] = {};
    // 函数调用：(void)std::snprintf(；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(
        // 继续上一行的表达式或参数列表：cpu, sizeof(cpu), "%u MHz",。
        cpu, sizeof(cpu), "%u MHz",
        // 继续上一行的表达式或参数列表：static_cast<unsigned>(::app::sys::info::CpuInfo::get_cpu_info().get_cpu_freq() / 1000000U));。
        static_cast<unsigned>(::app::sys::info::CpuInfo::get_cpu_info().get_cpu_freq() / 1000000U));

    // 赋值：把右侧 "-" 的结果保存到左侧 char mac[20]。
    char mac[20] = "-";
    // 赋值：把右侧 {} 的结果保存到左侧 uint8_t m[6]。
    uint8_t m[6] = {};
    // 判断条件：esp_read_mac(m, ESP_MAC_WIFI_STA) == ESP_OK 是否成立。
    if (esp_read_mac(m, ESP_MAC_WIFI_STA) == ESP_OK)
    {
        // 函数调用：(void)std::snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2],；执行该函数并处理它的返回值或副作用。
        (void)std::snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2],
                            // 继续上一行的表达式或参数列表：m[3], m[4], m[5]);。
                            m[3], m[4], m[5]);
    }

    // 执行语句：char ip[20];；它会更新一次变量、对象或程序状态。
    char ip[20];
    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto& info = ::app::network::wifi::WiFiMgr::get_instance().get_info();
    // 判断条件：info.state == ::app::network::wifi::State::CONNECTED & 是否成立。
    if (info.state == ::app::network::wifi::State::CONNECTED &&
        // 继续上一行的表达式或参数列表：(info.ip[0] | info.ip[1] | info.ip[2] | info.ip[3]) != 0)。
        (info.ip[0] | info.ip[1] | info.ip[2] | info.ip[3]) != 0)
    {
        // 函数调用：(void)std::snprintf(ip, sizeof(ip), "%u.%u.%u.%u", info.ip[0], info.ip[1], info.ip[2],；执行该函数并处理它的返回值或副作用。
        (void)std::snprintf(ip, sizeof(ip), "%u.%u.%u.%u", info.ip[0], info.ip[1], info.ip[2],
                            // 继续上一行的表达式或参数列表：info.ip[3]);。
                            info.ip[3]);
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 调用 std::strncpy(ip, ui::strings::kIpNone, sizeof(ip) - 1);，执行这个函数并处理它的返回值或状态变化。
        std::strncpy(ip, ui::strings::kIpNone, sizeof(ip) - 1);
        // 赋值：把右侧 '\0' 的结果保存到左侧 ip[sizeof(ip) - 1]。
        ip[sizeof(ip) - 1] = '\0';
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto mi = ::app::sys::info::MemoryInfo::get_memory_info();
    // 执行语句：char sram[24];；它会更新一次变量、对象或程序状态。
    char sram[24];
    // 执行语句：char psram[24];；它会更新一次变量、对象或程序状态。
    char psram[24];
    // 函数调用：(void)std::snprintf(sram, sizeof(sram), "%u/%u KB",；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(sram, sizeof(sram), "%u/%u KB",
                        // 继续上一行的表达式或参数列表：static_cast<unsigned>(mi.get_sram_free() / 1024),。
                        static_cast<unsigned>(mi.get_sram_free() / 1024),
                        // 继续上一行的表达式或参数列表：static_cast<unsigned>(mi.get_sram_total() / 1024));。
                        static_cast<unsigned>(mi.get_sram_total() / 1024));
    // 函数调用：(void)std::snprintf(psram, sizeof(psram), "%u/%u KB",；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(psram, sizeof(psram), "%u/%u KB",
                        // 继续上一行的表达式或参数列表：static_cast<unsigned>(mi.get_psram_free() / 1024),。
                        static_cast<unsigned>(mi.get_psram_free() / 1024),
                        // 继续上一行的表达式或参数列表：static_cast<unsigned>(mi.get_psram_total() / 1024));。
                        static_cast<unsigned>(mi.get_psram_total() / 1024));

    // struct RowDef：把相关数据字段组合在一起，表示项目中的一个概念。
    struct RowDef
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* lbl;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* val;
    };
    // 只读约束：这个值或对象在当前位置不能被修改。
    const RowDef rows[] = {
        // 执行这条代码：{ui::strings::kAboutDeviceName, ui::strings::kSetDeviceModel},。它会更新变量、对象状态或调用链。
        {ui::strings::kAboutDeviceName, ui::strings::kSetDeviceModel},
        // 继续上一行的表达式或参数列表：{ui::strings::kSetCpu, cpu},。
        {ui::strings::kSetCpu, cpu},
        // 继续上一行的表达式或参数列表：{ui::strings::kAboutMac, mac},。
        {ui::strings::kAboutMac, mac},
        // 继续上一行的表达式或参数列表：{ui::strings::kAboutIp, ip},。
        {ui::strings::kAboutIp, ip},
        // 继续上一行的表达式或参数列表：{ui::strings::kSetFirmware, fw},。
        {ui::strings::kSetFirmware, fw},
        // 继续上一行的表达式或参数列表：{ui::strings::kAboutSram, sram},。
        {ui::strings::kAboutSram, sram},
        // 继续上一行的表达式或参数列表：{ui::strings::kAboutPsram, psram},。
        {ui::strings::kAboutPsram, psram},
    };

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t total = sizeof(rows) / sizeof(rows[0]);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t visible = Theme::kListVisibleRows;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t max_off = (total > visible) ? static_cast<uint8_t>(total - visible) : 0;
    // 判断条件：about_scroll_row_ > max_off 是否成立。
    if (about_scroll_row_ > max_off)
        // 赋值：把右侧 max_off 的结果保存到左侧 about_scroll_row_。
        about_scroll_row_ = max_off;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool has_scroll = total > visible;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t row_w = static_cast<uint16_t>(Theme::kScreenW - (has_scroll ? 6 : 0));

    // 定义变量 int16_t y 并初始化为 Theme::kListStartY。
    int16_t y = Theme::kListStartY;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t end = static_cast<uint8_t>(about_scroll_row_ + visible);
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = about_scroll_row_; i < end && i < total; ++i)
    {
        // 声明并默认初始化对象：ui::widgets::RowStyle rs{}。
        ui::widgets::RowStyle rs{};
        // 赋值：把右侧 rows[i].lbl 的结果保存到左侧 rs.label。
        rs.label = rows[i].lbl;
        // 赋值：把右侧 rows[i].val 的结果保存到左侧 rs.value。
        rs.value = rows[i].val;
        // 赋值：把右侧 false 的结果保存到左侧 rs.show_chevron。
        rs.show_chevron = false;
        // 调用 ui::widgets::list_row(c, core::Rect{0, y, row_w, Theme::kListRowH}, rs);，执行这个函数并处理它的返回值或状态变化。
        ui::widgets::list_row(c, core::Rect{0, y, row_w, Theme::kListRowH}, rs);
        // 赋值：把右侧 static_cast<int16_t>(y + Theme::kListRowH) 的结果保存到左侧 y。
        y = static_cast<int16_t>(y + Theme::kListRowH);
    }

    // 判断条件：has_scroll 是否成立。
    if (has_scroll)
    {
        // 调用 ui::widgets::scrollbar(c, Theme::kListStartY, Theme::kListRegionH, about_scroll_row_, total,，执行这个函数并处理它的返回值或状态变化。
        ui::widgets::scrollbar(c, Theme::kListStartY, Theme::kListRegionH, about_scroll_row_, total,
                               // 继续上一行的表达式或参数列表：visible);。
                               visible);
    }
}

// 定义或声明函数 SettingsApp::handle_about：参数在括号里，返回类型写在函数名前面。
shell::InputResult SettingsApp::handle_about(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;
    // 固定参数 kAboutTotal = 7：由当前模块定义，用于控制布局、时序或硬件行为。
    constexpr uint8_t kAboutTotal = 7;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t visible = Theme::kListVisibleRows;

    // 判断条件：ev.start_y > Theme::kStatusBarH + Theme::kToolbarH 是否成立。
    if (ev.start_y > Theme::kStatusBarH + Theme::kToolbarH)
    {
        // 判断条件：scroll_rows(about_scroll_row_, kAboutTotal, visible, ev.type) 是否成立。
        if (scroll_rows(about_scroll_row_, kAboutTotal, visible, ev.type))
        {
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            request_repaint();
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        }
    }
    // 返回 {} 给调用者，表示当前函数的执行结果。
    return {};
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::settings
