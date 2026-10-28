/**
 * [初学者导读] 文件：main/app/ebook/data/system_state.cc
 * 作用：保存亮度、音量、Wi-Fi、蓝牙、热点等全局运行时状态。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：小配置写 NVS，运行时共享状态放 SystemState。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/system_state.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/persist.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::data {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// NVS key
constexpr const char* kKWifi = "wifi";
// 固定参数 kKBt = "bt"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kKBt = "bt";
// 固定参数 kKHotspot = "hotspot"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kKHotspot = "hotspot";
// 固定参数 kKNight = "night"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kKNight = "night";
// 固定参数 kKMute = "mute"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kKMute = "mute";
// 固定参数 kKBrightness = "bri"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kKBrightness = "bri";
// 固定参数 kKVolume = "vol"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kKVolume = "vol";

// 定义或声明函数 update_bool：参数在括号里，返回类型写在函数名前面。
void update_bool(bool& field, bool v, const char* key)
{
    // 判断条件：v == field 是否成立。
    if (v == field)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 v 的结果保存到左侧 field。
    field = v;
    // 调用 Persist::set_bool(key, v);，执行这个函数并处理它的返回值或状态变化。
    Persist::set_bool(key, v);
    // 调用 Persist::commit();，执行这个函数并处理它的返回值或状态变化。
    Persist::commit();
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 SystemState::get_instance：参数在括号里，返回类型写在函数名前面。
SystemState& SystemState::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static SystemState s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义或声明函数 SystemState::load：参数在括号里，返回类型写在函数名前面。
void SystemState::load()
{
    // 定义变量 bool b 并初始化为 false。
    bool b = false;
    // 判断条件：Persist::get_bool(kKWifi, b) 是否成立。
    if (Persist::get_bool(kKWifi, b))
        // 赋值：把右侧 b 的结果保存到左侧 wifi_。
        wifi_ = b;
    // 判断条件：Persist::get_bool(kKBt, b) 是否成立。
    if (Persist::get_bool(kKBt, b))
        // 赋值：把右侧 b 的结果保存到左侧 bluetooth_。
        bluetooth_ = b;
    // 判断条件：Persist::get_bool(kKHotspot, b) 是否成立。
    if (Persist::get_bool(kKHotspot, b))
        // 赋值：把右侧 b 的结果保存到左侧 hotspot_。
        hotspot_ = b;
    // 判断条件：Persist::get_bool(kKNight, b) 是否成立。
    if (Persist::get_bool(kKNight, b))
        // 赋值：把右侧 b 的结果保存到左侧 night_mode_。
        night_mode_ = b;
    // 判断条件：Persist::get_bool(kKMute, b) 是否成立。
    if (Persist::get_bool(kKMute, b))
        // 赋值：把右侧 b 的结果保存到左侧 mute_。
        mute_ = b;

    // 定义变量 uint8_t u 并初始化为 0。
    uint8_t u = 0;
    // 判断条件：Persist::get_u8(kKBrightness, u) 是否成立。
    if (Persist::get_u8(kKBrightness, u))
        // 赋值：把右侧 u 的结果保存到左侧 brightness_。
        brightness_ = u;
    // 判断条件：Persist::get_u8(kKVolume, u) 是否成立。
    if (Persist::get_u8(kKVolume, u))
        // 赋值：把右侧 u 的结果保存到左侧 volume_。
        volume_ = u;
}

// 定义或声明函数 SystemState::save：参数在括号里，返回类型写在函数名前面。
void SystemState::save()
{
    // 调用 Persist::set_bool(kKWifi, wifi_);，执行这个函数并处理它的返回值或状态变化。
    Persist::set_bool(kKWifi, wifi_);
    // 调用 Persist::set_bool(kKBt, bluetooth_);，执行这个函数并处理它的返回值或状态变化。
    Persist::set_bool(kKBt, bluetooth_);
    // 调用 Persist::set_bool(kKHotspot, hotspot_);，执行这个函数并处理它的返回值或状态变化。
    Persist::set_bool(kKHotspot, hotspot_);
    // 调用 Persist::set_bool(kKNight, night_mode_);，执行这个函数并处理它的返回值或状态变化。
    Persist::set_bool(kKNight, night_mode_);
    // 调用 Persist::set_bool(kKMute, mute_);，执行这个函数并处理它的返回值或状态变化。
    Persist::set_bool(kKMute, mute_);
    // 调用 Persist::set_u8(kKBrightness, brightness_);，执行这个函数并处理它的返回值或状态变化。
    Persist::set_u8(kKBrightness, brightness_);
    // 调用 Persist::set_u8(kKVolume, volume_);，执行这个函数并处理它的返回值或状态变化。
    Persist::set_u8(kKVolume, volume_);
    // 调用 Persist::commit();，执行这个函数并处理它的返回值或状态变化。
    Persist::commit();
}

// 定义或声明函数 SystemState::set_wifi：参数在括号里，返回类型写在函数名前面。
void SystemState::set_wifi(bool v)
{
    // 调用 update_bool(wifi_, v, kKWifi);，执行这个函数并处理它的返回值或状态变化。
    update_bool(wifi_, v, kKWifi);
}
// 定义或声明函数 SystemState::set_bluetooth：参数在括号里，返回类型写在函数名前面。
void SystemState::set_bluetooth(bool v)
{
    // 调用 update_bool(bluetooth_, v, kKBt);，执行这个函数并处理它的返回值或状态变化。
    update_bool(bluetooth_, v, kKBt);
}
// 定义或声明函数 SystemState::set_hotspot：参数在括号里，返回类型写在函数名前面。
void SystemState::set_hotspot(bool v)
{
    // 调用 update_bool(hotspot_, v, kKHotspot);，执行这个函数并处理它的返回值或状态变化。
    update_bool(hotspot_, v, kKHotspot);
}

// 定义或声明函数 SystemState::set_night_mode：参数在括号里，返回类型写在函数名前面。
void SystemState::set_night_mode(bool v)
{
    // 判断条件：v == night_mode_ 是否成立。
    if (v == night_mode_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 update_bool(night_mode_, v, kKNight);，执行这个函数并处理它的返回值或状态变化。
    update_bool(night_mode_, v, kKNight);
    // 判断条件：night_cb_ 是否成立。
    if (night_cb_)
        // 调用 night_cb_(v);，执行这个函数并处理它的返回值或状态变化。
        night_cb_(v);
}

// 定义或声明函数 SystemState::set_mute：参数在括号里，返回类型写在函数名前面。
void SystemState::set_mute(bool v)
{
    // 调用 update_bool(mute_, v, kKMute);，执行这个函数并处理它的返回值或状态变化。
    update_bool(mute_, v, kKMute);
    // 判断条件：volume_cb_ 是否成立。
    if (volume_cb_)
        // 调用 volume_cb_(volume_);，执行这个函数并处理它的返回值或状态变化。
        volume_cb_(volume_);
}

// 函数定义/声明：调用 SystemState::set_brightness()：把 0 到 100 的亮度限制在合法范围后交给背光驱动。墨水屏本身不依赖背光，背光是前光/照明系统，不能把亮度当成画面灰度。。
void SystemState::set_brightness(uint8_t v)
{
    // 判断条件：v == brightness_ 是否成立。
    if (v == brightness_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 v 的结果保存到左侧 brightness_。
    brightness_ = v;
    // 调用 Persist::set_u8(kKBrightness, v);，执行这个函数并处理它的返回值或状态变化。
    Persist::set_u8(kKBrightness, v);
    // 调用 Persist::commit();，执行这个函数并处理它的返回值或状态变化。
    Persist::commit();
    // 判断条件：bright_cb_ 是否成立。
    if (bright_cb_)
        // 调用 bright_cb_(v);，执行这个函数并处理它的返回值或状态变化。
        bright_cb_(v);
}

// 定义或声明函数 SystemState::set_volume：参数在括号里，返回类型写在函数名前面。
void SystemState::set_volume(uint8_t v)
{
    // 判断条件：v == volume_ 是否成立。
    if (v == volume_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 v 的结果保存到左侧 volume_。
    volume_ = v;
    // 调用 Persist::set_u8(kKVolume, v);，执行这个函数并处理它的返回值或状态变化。
    Persist::set_u8(kKVolume, v);
    // 调用 Persist::commit();，执行这个函数并处理它的返回值或状态变化。
    Persist::commit();
    // 判断条件：volume_cb_ 是否成立。
    if (volume_cb_)
        // 调用 volume_cb_(v);，执行这个函数并处理它的返回值或状态变化。
        volume_cb_(v);
}

// 定义或声明函数 SystemState::set_battery：参数在括号里，返回类型写在函数名前面。
void SystemState::set_battery(uint8_t pct, uint32_t mv)
{
    // 赋值：把右侧 pct 的结果保存到左侧 battery_pct_。
    battery_pct_ = pct;
    // 赋值：把右侧 mv 的结果保存到左侧 battery_mv_。
    battery_mv_ = mv;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::data
