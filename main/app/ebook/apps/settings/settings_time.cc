/**
 * [初学者导读] 文件：main/app/ebook/apps/settings/settings_time.cc
 * 作用：实现时区、NTP 自动同步和手动同步设置。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_app.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <vector>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_internal.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/clock_provider.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/persist.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/system_state.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/icon.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "network/wifi/wifi.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/toast.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "protocol/ntp/ntp.hpp"
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
using detail::ensure_wifi;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using detail::kTzCount;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using detail::kTzOffsets;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using detail::tap_row_index;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 SettingsApp::paint_time：参数在括号里，返回类型写在函数名前面。
void SettingsApp::paint_time(gfx::Canvas& c)
{
    // 定义变量 int16_t y 并初始化为 Theme::kListStartY。
    int16_t y = Theme::kListStartY;

    // 执行语句：char now_buf[24];；它会更新一次变量、对象或程序状态。
    char now_buf[24];
    // 定义变量 auto clk 并初始化为 data::Clock::now()。
    auto clk = data::Clock::now();
    // 函数调用：(void)std::snprintf(now_buf, sizeof(now_buf), "%04u-%02u-%02u %02u:%02u",；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(now_buf, sizeof(now_buf), "%04u-%02u-%02u %02u:%02u",
                        // 继续上一行的表达式或参数列表：static_cast<unsigned>(clk.year), static_cast<unsigned>(clk.month),。
                        static_cast<unsigned>(clk.year), static_cast<unsigned>(clk.month),
                        // 继续上一行的表达式或参数列表：static_cast<unsigned>(clk.day), static_cast<unsigned>(clk.hour),。
                        static_cast<unsigned>(clk.day), static_cast<unsigned>(clk.hour),
                        // 继续上一行的表达式或参数列表：static_cast<unsigned>(clk.minute));。
                        static_cast<unsigned>(clk.minute));

    // 执行语句：char tz_buf[12];；它会更新一次变量、对象或程序状态。
    char tz_buf[12];
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    (void)std::snprintf(tz_buf, sizeof(tz_buf), "UTC%+d", static_cast<int>(tz_offset_h_));

    // struct RowDef：把相关数据字段组合在一起，表示项目中的一个概念。
    struct RowDef
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* label;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* value;
        // 执行语句：uint32_t value_icon;；它会更新一次变量、对象或程序状态。
        uint32_t value_icon;
        // 执行语句：bool chevron;；它会更新一次变量、对象或程序状态。
        bool chevron;
    };
    // 只读约束：这个值或对象在当前位置不能被修改。
    const RowDef rows[] = {
        // 执行这条代码：{ui::strings::kSetCurTime, now_buf, 0, false},。它会更新变量、对象状态或调用链。
        {ui::strings::kSetCurTime, now_buf, 0, false},
        // 继续上一行的表达式或参数列表：{ui::strings::kSetTimezone, tz_buf, 0, true},。
        {ui::strings::kSetTimezone, tz_buf, 0, true},
        // 继续上一行的表达式或参数列表：{ui::strings::kSetAutoSync, nullptr, gfx::icon::toggle(ntp_auto_sync_), false},。
        {ui::strings::kSetAutoSync, nullptr, gfx::icon::toggle(ntp_auto_sync_), false},
        // 继续上一行的表达式或参数列表：{ui::strings::kSetSyncNow, ui::strings::kSetClickSync, 0, true},。
        {ui::strings::kSetSyncNow, ui::strings::kSetClickSync, 0, true},
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
        // 赋值：把右侧 r.value_icon 的结果保存到左侧 rs.value_icon。
        rs.value_icon = r.value_icon;
        // 赋值：把右侧 20 的结果保存到左侧 rs.value_icon_size。
        rs.value_icon_size = 20;
        // 赋值：把右侧 r.chevron 的结果保存到左侧 rs.show_chevron。
        rs.show_chevron = r.chevron;
        // 调用 ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);，执行这个函数并处理它的返回值或状态变化。
        ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);
        // 赋值：把右侧 static_cast<int16_t>(y + Theme::kListRowH) 的结果保存到左侧 y。
        y = static_cast<int16_t>(y + Theme::kListRowH);
    }
}

// 定义或声明函数 SettingsApp::handle_time：参数在括号里，返回类型写在函数名前面。
shell::InputResult SettingsApp::handle_time(const ::app::ebook::input::Event& ev)
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

    // 判断条件：row == 1 是否成立。
    if (row == 1)
    {
        // 赋值：把右侧 static_cast<uint8_t>((tz_idx_ + 1) % kTzCount) 的结果保存到左侧 tz_idx_。
        tz_idx_ = static_cast<uint8_t>((tz_idx_ + 1) % kTzCount);
        // 调用 apply_tz_from_index()：把 NVS 索引转换成 POSIX 时区字符串，格式为 CST%+d。例如东八区是 CST8 这种 TZ 形式，具体符号规则由 C 库时区约定决定。。
        apply_tz_from_index();
        // 调用 save_time_prefs()：读写时区索引和自动同步开关。。
        save_time_prefs();
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：row == 2 是否成立。
    if (row == 2)
    {
        // 赋值：把右侧 !ntp_auto_sync_ 的结果保存到左侧 ntp_auto_sync_。
        ntp_auto_sync_ = !ntp_auto_sync_;
        // 调用 save_time_prefs()：读写时区索引和自动同步开关。。
        save_time_prefs();
        // 判断条件：ntp_auto_sync_ 是否成立。
        if (ntp_auto_sync_)
            // 调用 try_ntp_auto_sync()：自动同步前必须确认 WiFi 已连接。NTP 需要网络，离线状态下只能读取本地时间或显示未同步。。
            try_ntp_auto_sync();
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：row == 3 是否成立。
    if (row == 3)
    {
        // 判断条件：!data::SystemState::get_instance().wifi( 是否成立。
        if (!data::SystemState::get_instance().wifi() ||
            // 定义或声明函数 app::network::wifi::WiFiMgr::get_instance：参数在括号里，返回类型写在函数名前面。
            !::app::network::wifi::WiFiMgr::get_instance().is_connected())
        {
            // 调用 overlays::Toast::instance().show(ui::strings::kNeedWifiFirst, 2000);，执行这个函数并处理它的返回值或状态变化。
            overlays::Toast::instance().show(ui::strings::kNeedWifiFirst, 2000);
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        }
        // 判断条件：!ensure_ntp_started() 是否成立。
        if (!ensure_ntp_started())
        {
            // 调用 overlays::Toast::instance().show(ui::strings::kTimeSyncFail, 2500);，执行这个函数并处理它的返回值或状态变化。
            overlays::Toast::instance().show(ui::strings::kTimeSyncFail, 2500);
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        }
        // 调用 overlays::Toast::instance().show(ui::strings::kWifiConnecting, 1500);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kWifiConnecting, 1500);
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 返回 {} 给调用者，表示当前函数的执行结果。
    return {};
}

// 函数定义/声明：调用 SettingsApp::on_ntp_sync_done()：处理同步完成事件，更新页面状态。它由 UI 线程消费。。
void SettingsApp::on_ntp_sync_done(uint8_t status)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using NtpStatus = ::app::protocol::ntp::SyncStatus;
    // 赋值：把右侧 overlays::Toast::instance() 的结果保存到左侧 auto& toast。
    auto& toast = overlays::Toast::instance();
    // 多分支判断：根据括号里的值选择不同 case。
    switch (static_cast<NtpStatus>(status))
    {
        // 分支匹配：case NtpStatus::COMPLETED 成立时执行下面的代码。
        case NtpStatus::COMPLETED:
            // 访问对象成员：toast.show(ui::strings::kTimeSyncOk, 2000);；读取或修改这个对象里的成员状态。
            toast.show(ui::strings::kTimeSyncOk, 2000);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case NtpStatus::FAILED 成立时执行下面的代码。
        case NtpStatus::FAILED:
            // 访问对象成员：toast.show(ui::strings::kTimeSyncFail, 2500);；读取或修改这个对象里的成员状态。
            toast.show(ui::strings::kTimeSyncFail, 2500);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 提前返回：结束当前函数，不返回具体值。
            return;
    }
    // 判断条件：page_ == SettingsPage::Time 是否成立。
    if (page_ == SettingsPage::Time)
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
}

// 函数定义/声明：调用 SettingsApp::ensure_ntp_started()：保证 NTP 服务只初始化一次。它不做同步本身，不同服务器和网络状态的组合由同步流程负责。。
bool SettingsApp::ensure_ntp_started()
{
    // 赋值：把右侧 ::app::protocol::ntp::NtpMgr::get_instance() 的结果保存到左侧 auto& ntp。
    auto& ntp = ::app::protocol::ntp::NtpMgr::get_instance();
    // 判断条件：!ntp.is_init() && !ntp.init() 是否成立。
    if (!ntp.is_init() && !ntp.init())
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：ntp.is_started() 是否成立。
    if (ntp.is_started())
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;

    // 静态值 kServers：只属于当前模块或类，并保持固定含义。
    static const std::vector<std::string> kServers = {
        // 执行这条代码："ntp.aliyun.com",。它会更新变量、对象状态或调用链。
        "ntp.aliyun.com",
        // 继续上一行的表达式或参数列表："cn.pool.ntp.org",。
        "cn.pool.ntp.org",
        // 继续上一行的表达式或参数列表："pool.ntp.org",。
        "pool.ntp.org",
    };
    // 判断条件：!ntp.configure(kServers) 是否成立。
    if (!ntp.configure(kServers))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 返回结果：调用 start()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
    return ntp.start();
}

// 函数定义/声明：调用 SettingsApp::try_ntp_auto_sync()：自动同步前必须确认 WiFi 已连接。NTP 需要网络，离线状态下只能读取本地时间或显示未同步。。
void SettingsApp::try_ntp_auto_sync()
{
    // 判断条件：!ntp_auto_sync_ 是否成立。
    if (!ntp_auto_sync_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：!data::SystemState::get_instance().wifi() 是否成立。
    if (!data::SystemState::get_instance().wifi())
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：!::app::network::wifi::WiFiMgr::get_instance().is_connected() 是否成立。
    if (!::app::network::wifi::WiFiMgr::get_instance().is_connected())
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 ensure_ntp_started()：保证 NTP 服务只初始化一次。它不做同步本身，不同服务器和网络状态的组合由同步流程负责。。
    (void)ensure_ntp_started();
}

// 函数定义/声明：调用 SettingsApp::try_ntp_auto_sync_on_connect()：WiFi 连接状态变化时触发自动同步。只有连接成功才继续，其他状态不应反复发 NTP 请求。。
void SettingsApp::try_ntp_auto_sync_on_connect(uint8_t wifi_state)
{
    // 判断条件：static_cast<::app::network::wifi::State>(wifi_state 是否成立。
    if (static_cast<::app::network::wifi::State>(wifi_state) !=
        // 继续上一行的表达式或参数列表：::app::network::wifi::State::CONNECTED)。
        ::app::network::wifi::State::CONNECTED)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：!ntp_auto_sync_ 是否成立。
    if (!ntp_auto_sync_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 ensure_ntp_started()：保证 NTP 服务只初始化一次。它不做同步本身，不同服务器和网络状态的组合由同步流程负责。。
    (void)ensure_ntp_started();
}

// 函数定义/声明：调用 SettingsApp::load_time_prefs()：读写时区索引和自动同步开关。。
void SettingsApp::load_time_prefs()
{
    // 定义变量 uint8_t v 并初始化为 0。
    uint8_t v = 0;
    // 判断条件：data::Persist::get_u8("tz_idx", v) && v < kTzCount 是否成立。
    if (data::Persist::get_u8("tz_idx", v) && v < kTzCount)
        // 赋值：把右侧 v 的结果保存到左侧 tz_idx_。
        tz_idx_ = v;
    // 定义变量 bool b 并初始化为 true。
    bool b = true;
    // 判断条件：data::Persist::get_bool("ntp_auto", b) 是否成立。
    if (data::Persist::get_bool("ntp_auto", b))
        // 赋值：把右侧 b 的结果保存到左侧 ntp_auto_sync_。
        ntp_auto_sync_ = b;
}

// 函数定义/声明：调用 SettingsApp::save_time_prefs()：读写时区索引和自动同步开关。。
void SettingsApp::save_time_prefs()
{
    // 调用 data::Persist::set_u8("tz_idx", tz_idx_);，执行这个函数并处理它的返回值或状态变化。
    data::Persist::set_u8("tz_idx", tz_idx_);
    // 调用 data::Persist::set_bool("ntp_auto", ntp_auto_sync_);，执行这个函数并处理它的返回值或状态变化。
    data::Persist::set_bool("ntp_auto", ntp_auto_sync_);
    // 调用 data::Persist::commit();，执行这个函数并处理它的返回值或状态变化。
    data::Persist::commit();
}

// 函数定义/声明：调用 SettingsApp::apply_tz_from_index()：把 NVS 索引转换成 POSIX 时区字符串，格式为 CST%+d。例如东八区是 CST8 这种 TZ 形式，具体符号规则由 C 库时区约定决定。。
void SettingsApp::apply_tz_from_index()
{
    // 判断条件：tz_idx_ >= kTzCount 是否成立。
    if (tz_idx_ >= kTzCount)
        // 赋值：把右侧 0 的结果保存到左侧 tz_idx_。
        tz_idx_ = 0;
    // 赋值：把右侧 kTzOffsets[tz_idx_] 的结果保存到左侧 tz_offset_h_。
    tz_offset_h_ = kTzOffsets[tz_idx_];
    // 执行语句：char tz[12];；它会更新一次变量、对象或程序状态。
    char tz[12];
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    (void)std::snprintf(tz, sizeof(tz), "CST%+d", -static_cast<int>(tz_offset_h_));
    // 访问对象成员：(void)::app::protocol::ntp::NtpMgr::get_instance().set_timezone(tz);；读取或修改这个对象里的成员状态。
    (void)::app::protocol::ntp::NtpMgr::get_instance().set_timezone(tz);
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::settings
