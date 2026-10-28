/**
 * [初学者导读] 文件：main/app/ebook/apps/settings/power_save.cc
 * 作用：实现省电开关、背光限制和无线功能关闭/恢复。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/power_save.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/persist.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/system_state.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "network/wifi/wifi.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/confirm_prompt.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/refresh_intent.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/router.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::settings {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 固定参数 kKOn = "pwr.on"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kKOn = "pwr.on";
// 固定参数 kKAuto = "pwr.auto"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kKAuto = "pwr.auto";
// 固定参数 kKLvl = "pwr.lvl"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kKLvl = "pwr.lvl";

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 PowerSave::get_instance：参数在括号里，返回类型写在函数名前面。
PowerSave& PowerSave::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static PowerSave s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// [初学者导读] PowerSave::load()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void PowerSave::load()
{
    // 定义变量 bool on 并初始化为 false。
    bool on = false;
    // 判断条件：data::Persist::get_bool(kKOn, on) && on 是否成立。
    if (data::Persist::get_bool(kKOn, on) && on)
    {
        // 定义变量 bool auto_on 并初始化为 false。
        bool auto_on = false;
        // 函数调用：(void)data::Persist::get_bool(kKAuto, auto_on);；执行该函数并处理它的返回值或副作用。
        (void)data::Persist::get_bool(kKAuto, auto_on);
        // 定义变量 uint8_t lvl 并初始化为 0。
        uint8_t lvl = 0;
        // 函数调用：(void)data::Persist::get_u8(kKLvl, lvl);；执行该函数并处理它的返回值或副作用。
        (void)data::Persist::get_u8(kKLvl, lvl);

        // 赋值：把右侧 auto_on 的结果保存到左侧 auto_enabled_。
        auto_enabled_ = auto_on;
        // 赋值：把右侧 (lvl == static_cast<uint8_t>(Level::Strong)) ? Level::Strong : Level::Normal 的结果保存到左侧 level_。
        level_ = (lvl == static_cast<uint8_t>(Level::Strong)) ? Level::Strong : Level::Normal;
        // 调用 capture_snapshot()：保存当前： 亮度。 WiFi 状态。 蓝牙状态。 热点状态。。
        capture_snapshot();
        // 赋值：把右侧 true 的结果保存到左侧 enabled_。
        enabled_ = true;
        // 调用 apply_policy()：应用当前级别的策略： 1. 限制背光。 2. 关闭 WiFi。 3. 关闭蓝牙。 4. 关闭热点。。
        apply_policy();
    }

    // 赋值：把右侧 data::SystemState::get_instance().battery_pct() 的结果保存到左侧 last_pct_。
    last_pct_ = data::SystemState::get_instance().battery_pct();
}

// [初学者导读] PowerSave::capture_snapshot()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void PowerSave::capture_snapshot()
{
    // 判断条件：snapshot_valid_ 是否成立。
    if (snapshot_valid_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 data::SystemState::get_instance() 的结果保存到左侧 auto& sys。
    auto& sys = data::SystemState::get_instance();
    // 调用 brightness()：把 0 到 100 的亮度限制在合法范围后交给背光驱动。墨水屏本身不依赖背光，背光是前光/照明系统，不能把亮度当成画面灰度。。
    snap_brightness_ = sys.brightness();
    // 赋值：把右侧 sys.wifi() 的结果保存到左侧 snap_wifi_。
    snap_wifi_ = sys.wifi();
    // 赋值：把右侧 sys.bluetooth() 的结果保存到左侧 snap_bt_。
    snap_bt_ = sys.bluetooth();
    // 赋值：把右侧 sys.hotspot() 的结果保存到左侧 snap_hotspot_。
    snap_hotspot_ = sys.hotspot();
    // 赋值：把右侧 true 的结果保存到左侧 snapshot_valid_。
    snapshot_valid_ = true;
}

// [初学者导读] PowerSave::restore_snapshot()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void PowerSave::restore_snapshot()
{
    // 判断条件：!snapshot_valid_ 是否成立。
    if (!snapshot_valid_)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 data::SystemState::get_instance() 的结果保存到左侧 auto& sys。
    auto& sys = data::SystemState::get_instance();
    // 调用 set_brightness(snap_brightness_)：把 0 到 100 的亮度限制在合法范围后交给背光驱动。墨水屏本身不依赖背光，背光是前光/照明系统，不能把亮度当成画面灰度。。
    sys.set_brightness(snap_brightness_);
    // 访问对象成员：sys.set_wifi(snap_wifi_);；读取或修改这个对象里的成员状态。
    sys.set_wifi(snap_wifi_);
    // 调用 SettingsApp::instance()：返回设置页单例。所有设置页面共享同一份 page 和输入状态。。
    SettingsApp::instance().wifi_apply_state();

    // 访问对象成员：sys.set_bluetooth(snap_bt_);；读取或修改这个对象里的成员状态。
    sys.set_bluetooth(snap_bt_);
    // 调用 SettingsApp::instance()：返回设置页单例。所有设置页面共享同一份 page 和输入状态。。
    SettingsApp::instance().bt_apply_state();

    // 访问对象成员：sys.set_hotspot(snap_hotspot_);；读取或修改这个对象里的成员状态。
    sys.set_hotspot(snap_hotspot_);
    // 调用 SettingsApp::instance()：返回设置页单例。所有设置页面共享同一份 page 和输入状态。。
    SettingsApp::instance().hotspot_apply_state();

    // 赋值：把右侧 false 的结果保存到左侧 snapshot_valid_。
    snapshot_valid_ = false;
}

// [初学者导读] PowerSave::apply_policy()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void PowerSave::apply_policy()
{
    // 赋值：把右侧 data::SystemState::get_instance() 的结果保存到左侧 auto& sys。
    auto& sys = data::SystemState::get_instance();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t cap = (level_ == Level::Strong) ? kCapStrong : kCapNormal;
    // 判断条件：sys.brightness() > cap 是否成立。
    if (sys.brightness() > cap)
        // 调用 set_brightness(cap)：把 0 到 100 的亮度限制在合法范围后交给背光驱动。墨水屏本身不依赖背光，背光是前光/照明系统，不能把亮度当成画面灰度。。
        sys.set_brightness(cap);

    // 判断条件：sys.wifi() 是否成立。
    if (sys.wifi())
    {
        // 访问对象成员：sys.set_wifi(false);；读取或修改这个对象里的成员状态。
        sys.set_wifi(false);
        // 访问对象成员：::app::network::wifi::WiFiMgr::get_instance().disconnect();；读取或修改这个对象里的成员状态。
        ::app::network::wifi::WiFiMgr::get_instance().disconnect();
    }
    // 判断条件：sys.bluetooth() 是否成立。
    if (sys.bluetooth())
    {
        // 访问对象成员：sys.set_bluetooth(false);；读取或修改这个对象里的成员状态。
        sys.set_bluetooth(false);
        // 调用 SettingsApp::instance()：返回设置页单例。所有设置页面共享同一份 page 和输入状态。。
        SettingsApp::instance().bt_apply_state();
    }
    // 判断条件：sys.hotspot() 是否成立。
    if (sys.hotspot())
    {
        // 访问对象成员：sys.set_hotspot(false);；读取或修改这个对象里的成员状态。
        sys.set_hotspot(false);
        // 调用 SettingsApp::instance()：返回设置页单例。所有设置页面共享同一份 page 和输入状态。。
        SettingsApp::instance().hotspot_apply_state();
    }
}

// 定义或声明函数 PowerSave::set_enabled：参数在括号里，返回类型写在函数名前面。
void PowerSave::set_enabled(bool on, Level level, bool from_auto)
{
    // 判断条件：on 是否成立。
    if (on)
    {
        // 判断条件：!enabled_ 是否成立。
        if (!enabled_)
            // 调用 capture_snapshot()：保存当前： 亮度。 WiFi 状态。 蓝牙状态。 热点状态。。
            capture_snapshot();
        // 赋值：把右侧 true 的结果保存到左侧 enabled_。
        enabled_ = true;
        // 赋值：把右侧 level 的结果保存到左侧 level_。
        level_ = level;
        // 赋值：把右侧 from_auto 的结果保存到左侧 auto_enabled_。
        auto_enabled_ = from_auto;
        // 调用 apply_policy()：应用当前级别的策略： 1. 限制背光。 2. 关闭 WiFi。 3. 关闭蓝牙。 4. 关闭热点。。
        apply_policy();

        // 函数调用：(void)data::Persist::set_bool(kKOn, true);；执行该函数并处理它的返回值或副作用。
        (void)data::Persist::set_bool(kKOn, true);
        // 函数调用：(void)data::Persist::set_bool(kKAuto, auto_enabled_);；执行该函数并处理它的返回值或副作用。
        (void)data::Persist::set_bool(kKAuto, auto_enabled_);
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        (void)data::Persist::set_u8(kKLvl, static_cast<uint8_t>(level_));
        // 调用 data::Persist::commit();，执行这个函数并处理它的返回值或状态变化。
        data::Persist::commit();
        // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
        (void)router::Router::instance().repaint(router::intent_partial_full());
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：!enabled_ 是否成立。
    if (!enabled_)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 false 的结果保存到左侧 enabled_。
    enabled_ = false;
    // 赋值：把右侧 false 的结果保存到左侧 auto_enabled_。
    auto_enabled_ = false;
    // 调用 restore_snapshot()：关闭省电时恢复快照。它恢复的是进入省电前的设置，而不是硬编码值。。
    restore_snapshot();

    // 函数调用：(void)data::Persist::set_bool(kKOn, false);；执行该函数并处理它的返回值或副作用。
    (void)data::Persist::set_bool(kKOn, false);
    // 函数调用：(void)data::Persist::set_bool(kKAuto, false);；执行该函数并处理它的返回值或副作用。
    (void)data::Persist::set_bool(kKAuto, false);
    // 调用 data::Persist::commit();，执行这个函数并处理它的返回值或状态变化。
    data::Persist::commit();
    // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
    (void)router::Router::instance().repaint(router::intent_partial_full());
}

// [初学者导读] PowerSave::toggle_manual()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void PowerSave::toggle_manual()
{
    // 判断条件：enabled_ 是否成立。
    if (enabled_)
        // 调用 set_enabled(false, level_, false);，执行这个函数并处理它的返回值或状态变化。
        set_enabled(false, level_, false);
    // 备用分支：前面的条件不成立时执行这里。
    else
        // 调用 set_enabled(true, Level::Normal, false);，执行这个函数并处理它的返回值或状态变化。
        set_enabled(true, Level::Normal, false);
}

// 函数定义/声明：调用 PowerSave::prompt_enable()：显示开启或关闭确认。省电会关闭 WiFi 等能力，用户需要知道影响。。
void PowerSave::prompt_enable(Level level)
{
    // 判断条件：overlays::ConfirmPrompt::instance().is_open() 是否成立。
    if (overlays::ConfirmPrompt::instance().is_open())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 level 的结果保存到左侧 pending_level_。
    pending_level_ = level;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* msg = (level == Level::Strong) ? ui::strings::kPwrPromptEnable10
                                               // 执行语句：: ui::strings::kPwrPromptEnable20;；它会更新一次变量、对象或程序状态。
                                               : ui::strings::kPwrPromptEnable20;
    // 调用 show(msg,on_enable_choice,this)：ensuretimer()：第一次调用创建一次性软件定时器，后续修改周期。。
    overlays::ConfirmPrompt::instance().show(msg, on_enable_choice, this);
}

// 函数定义/声明：调用 PowerSave::prompt_disable()：显示开启或关闭确认。省电会关闭 WiFi 等能力，用户需要知道影响。。
void PowerSave::prompt_disable()
{
    // 判断条件：overlays::ConfirmPrompt::instance().is_open() 是否成立。
    if (overlays::ConfirmPrompt::instance().is_open())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 调用 overlays::ConfirmPrompt::instance().show(ui::strings::kPwrPromptDisable80, on_disable_choice,，执行这个函数并处理它的返回值或状态变化。
    overlays::ConfirmPrompt::instance().show(ui::strings::kPwrPromptDisable80, on_disable_choice,
                                             // 继续上一行的表达式或参数列表：this);。
                                             this);
}

// 函数定义/声明：调用 PowerSave::on_enable_choice()：对话框回调，确认后执行真正的状态改变。。
void PowerSave::on_enable_choice(bool accepted, void* user)
{
    // 赋值：把右侧 static_cast<PowerSave*>(user) 的结果保存到左侧 auto* self。
    auto* self = static_cast<PowerSave*>(user);
    // 判断条件：self == nullptr || !accepted 是否成立。
    if (self == nullptr || !accepted)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 访问对象成员：self->set_enabled(true, self->pending_level_, true);；读取或修改这个对象里的成员状态。
    self->set_enabled(true, self->pending_level_, true);
}

// 函数定义/声明：调用 PowerSave::on_disable_choice()：对话框回调，确认后执行真正的状态改变。。
void PowerSave::on_disable_choice(bool accepted, void* user)
{
    // 赋值：把右侧 static_cast<PowerSave*>(user) 的结果保存到左侧 auto* self。
    auto* self = static_cast<PowerSave*>(user);
    // 判断条件：self == nullptr || !accepted 是否成立。
    if (self == nullptr || !accepted)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 访问对象成员：self->set_enabled(false, self->level_, false);；读取或修改这个对象里的成员状态。
    self->set_enabled(false, self->level_, false);
}

// 函数定义/声明：调用 PowerSave::on_battery_pct()：根据电量应用自动策略： 低于 20%：可以进入普通省电。 低于 10%：可以进入强省电。 高于 80%：解除自动省电。。
void PowerSave::on_battery_pct(uint8_t pct)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t prev = last_pct_;
    // 赋值：把右侧 pct 的结果保存到左侧 last_pct_。
    last_pct_ = pct;

    // 判断条件：prev == 0xFF 是否成立。
    if (prev == 0xFF)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：overlays::ConfirmPrompt::instance().is_open() 是否成立。
    if (overlays::ConfirmPrompt::instance().is_open())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 判断条件：prev > kThreshLow10 && pct <= kThreshLow10 是否成立。
    if (prev > kThreshLow10 && pct <= kThreshLow10)
    {
        // 判断条件：!enabled_ 是否成立。
        if (!enabled_)
            // 调用 prompt_enable(Level::Strong);，执行这个函数并处理它的返回值或状态变化。
            prompt_enable(Level::Strong);
        // 备用分支：前面的条件不成立时执行这里。
        else if (level_ == Level::Normal)
            // 调用 prompt_enable(Level::Strong);，执行这个函数并处理它的返回值或状态变化。
            prompt_enable(Level::Strong);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：prev > kThreshLow20 && pct <= kThreshLow20 是否成立。
    if (prev > kThreshLow20 && pct <= kThreshLow20)
    {
        // 判断条件：!enabled_ 是否成立。
        if (!enabled_)
            // 调用 prompt_enable(Level::Normal);，执行这个函数并处理它的返回值或状态变化。
            prompt_enable(Level::Normal);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：prev <= kThreshHigh80 && pct > kThreshHigh80 是否成立。
    if (prev <= kThreshHigh80 && pct > kThreshHigh80)
    {
        // 判断条件：enabled_ && auto_enabled_ 是否成立。
        if (enabled_ && auto_enabled_)
            // 调用 prompt_disable()：显示开启或关闭确认。省电会关闭 WiFi 等能力，用户需要知道影响。。
            prompt_disable();
    }
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::settings
