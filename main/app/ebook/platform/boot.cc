/**
 * [初学者导读] 文件：main/app/ebook/platform/boot.cc
 * 作用：按依赖顺序初始化 NVS、显示、触摸、音频、数据、电池和任务。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：阻塞工作放独立任务，周期小检查放 Housekeeper。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "platform/boot.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_app_desc.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_mac.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <nvs_flash.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app_registry.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/auto_lock.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/power_save.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/lock_password.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "config/config.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/clock_provider.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/clock/clock_store.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/music/music_store.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/reading_store.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/persist.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/system_state.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "display/display_port.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ft6336u.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/font.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "i2c/i2c.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ime/ime_engine.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/input_router.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/key_bindings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/physical_input.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ota/ota.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "platform/audio_codec.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "platform/battery_sampler.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "platform/housekeeper.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "platform/music_player.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "platform/wooden_fish_sound.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "presenter/presenter.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "network/wifi/wifi.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "protocol/http/http.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "protocol/ntp/ntp.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "storage/storage.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/event/event.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_bus.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_loop.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "Boot";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::platform {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 执行语句：::app::bsp::i2c::I2C g_i2c;；它会更新一次变量、对象或程序状态。
::app::bsp::i2c::I2C g_i2c;
// 执行语句：::app::bsp::driver::ft6336u::Ft6336u g_touch;；它会更新一次变量、对象或程序状态。
::app::bsp::driver::ft6336u::Ft6336u g_touch;

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using StepFn = core::Status (*)();

// struct BootStep：把相关数据字段组合在一起，表示项目中的一个概念。
struct BootStep
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* name;
    // 执行语句：StepFn run;；它会更新一次变量、对象或程序状态。
    StepFn run;
    // 执行语句：bool required;；它会更新一次变量、对象或程序状态。
    bool required;
};

// [初学者导读] init_nvs_partition()：准备 NVS 闪存分区。详见 docs/代码详解-初学者版.md。
bool init_nvs_partition()
{
    // 定义变量 esp_err_t r 并初始化为 nvs_flash_init()。
    esp_err_t r = nvs_flash_init();
    // 判断条件：r == ESP_ERR_NVS_NO_FREE_PAGES || r == ESP_ERR_NVS_NEW_VERSION_FOUND 是否成立。
    if (r == ESP_ERR_NVS_NO_FREE_PAGES || r == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // 调用 ESP_ERROR_CHECK(nvs_flash_erase());，执行这个函数并处理它的返回值或状态变化。
        ESP_ERROR_CHECK(nvs_flash_erase());
        // 赋值：把右侧 nvs_flash_init() 的结果保存到左侧 r。
        r = nvs_flash_init();
    }
    // 返回 r == ESP_OK 给调用者，表示当前函数的执行结果。
    return r == ESP_OK;
}

// [初学者导读] step_system_services()：系统服务初始化。详见 docs/代码详解-初学者版.md。
core::Status step_system_services()
{
    // 判断条件：!init_nvs_partition() 是否成立。
    if (!init_nvs_partition())
        // 调用 EBOOK_LOGW(TAG, "nvs partial");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "nvs partial");
    // 判断条件：!::app::sys::event::EventMgr::get_instance().init() 是否成立。
    if (!::app::sys::event::EventMgr::get_instance().init())
        // 调用 EBOOK_LOGW(TAG, "EventMgr");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "EventMgr");
    // 判断条件：!::app::protocol::http::HttpMgr::get_instance().init() 是否成立。
    if (!::app::protocol::http::HttpMgr::get_instance().init())
        // 调用 EBOOK_LOGW(TAG, "HttpMgr");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "HttpMgr");
    // 判断条件：!::app::protocol::ntp::NtpMgr::get_instance().init() 是否成立。
    if (!::app::protocol::ntp::NtpMgr::get_instance().init())
        // 调用 EBOOK_LOGW(TAG, "NtpMgr");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "NtpMgr");

    // 函数调用：::app::protocol::ntp::NtpMgr::get_instance().set_sync_callback(；执行该函数并处理它的返回值或副作用。
    ::app::protocol::ntp::NtpMgr::get_instance().set_sync_callback(
        // 开始代码块：下面是这个函数、分支或循环的主体。
        [](::app::protocol::ntp::SyncStatus st) {
            // 函数调用：(void)::app::ebook::ui::UiBus::get_instance().post_ntp_sync_done(；执行该函数并处理它的返回值或副作用。
            (void)::app::ebook::ui::UiBus::get_instance().post_ntp_sync_done(
                // 继续上一行的表达式或参数列表：static_cast<uint8_t>(st));。
                static_cast<uint8_t>(st));
        // 执行语句：});；它会更新一次变量、对象或程序状态。
        });

    // 赋值：把右侧 {} 的结果保存到左侧 char device_id[18]。
    char device_id[18] = {};
    // 赋值：把右侧 {} 的结果保存到左侧 uint8_t mac[6]。
    uint8_t mac[6] = {};
    // 判断条件：esp_read_mac(mac, ESP_MAC_WIFI_STA) == ESP_OK 是否成立。
    if (esp_read_mac(mac, ESP_MAC_WIFI_STA) == ESP_OK)
    {
        // 函数调用：(void)std::snprintf(device_id, sizeof(device_id), "%02x%02x%02x%02x%02x%02x", mac[0],；执行该函数并处理它的返回值或副作用。
        (void)std::snprintf(device_id, sizeof(device_id), "%02x%02x%02x%02x%02x%02x", mac[0],
                            // 继续上一行的表达式或参数列表：mac[1], mac[2], mac[3], mac[4], mac[5]);。
                            mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    // 只读约束：这个值或对象在当前位置不能被修改。
    const esp_app_desc_t* desc = esp_app_get_description();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* ver = (desc != nullptr) ? desc->version : "0.0.0";
    // 判断条件：!::app::common::ota::OtaMgr::get_instance().init(device_id, ver) 是否成立。
    if (!::app::common::ota::OtaMgr::get_instance().init(device_id, ver))
        // 调用 EBOOK_LOGW(TAG, "OtaMgr");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "OtaMgr");

    // 判断条件：::app::common::storage::StorageMgr::get_instance().init_defaults() != ESP_OK 是否成立。
    if (::app::common::storage::StorageMgr::get_instance().init_defaults() != ESP_OK)
        // 调用 EBOOK_LOGW(TAG, "storage defaults");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "storage defaults");

    // 返回 core::Status::Ok 给调用者，表示当前函数的执行结果。
    return core::Status::Ok;
}

// [初学者导读] step_persist()：打开 NVS 命名空间。详见 docs/代码详解-初学者版.md。
core::Status step_persist()
{
    // 返回 data::Persist::init() 给调用者，表示当前函数的执行结果。
    return data::Persist::init();
}

// [初学者导读] step_display()：初始化显示与字体。详见 docs/代码详解-初学者版.md。
core::Status step_display()
{
    // 判断条件：!display::DisplayPort::instance().init() 是否成立。
    if (!display::DisplayPort::instance().init())
        // 返回 core::Status::IoError 给调用者，表示当前函数的执行结果。
        return core::Status::IoError;
    // 判断条件：!gfx::Font::get_instance().init() 是否成立。
    if (!gfx::Font::get_instance().init())
        // 调用 EBOOK_LOGW(TAG, "font partial");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "font partial");
    // 拼音候选表非关键路径：加载失败仅退化为软键盘无中文候选，不阻断启动
    if (!ime::ImeEngine::get_instance().init())
        // 调用 EBOOK_LOGW(TAG, "ime engine unavailable");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "ime engine unavailable");
    // 返回 core::Status::Ok 给调用者，表示当前函数的执行结果。
    return core::Status::Ok;
}

// [初学者导读] step_touch()：初始化 I2C 与 FT6336U。详见 docs/代码详解-初学者版.md。
core::Status step_touch()
{
    // 判断条件：!g_i2c.init() 是否成立。
    if (!g_i2c.init())
        // 返回 core::Status::IoError 给调用者，表示当前函数的执行结果。
        return core::Status::IoError;

    // 声明并默认初始化对象：::app::bsp::driver::ft6336u::Config tc{}。
    ::app::bsp::driver::ft6336u::Config tc{};
    // 赋值：把右侧 core::kScreenW 的结果保存到左侧 tc.panel_width。
    tc.panel_width = core::kScreenW;
    // 赋值：把右侧 core::kScreenH 的结果保存到左侧 tc.panel_height。
    tc.panel_height = core::kScreenH;
    // 赋值：把右侧 ::app::config::TOUCH_SWAP_XY 的结果保存到左侧 tc.swap_xy。
    tc.swap_xy = ::app::config::TOUCH_SWAP_XY;
    // 赋值：把右侧 ::app::config::TOUCH_MIRROR_X 的结果保存到左侧 tc.mirror_x。
    tc.mirror_x = ::app::config::TOUCH_MIRROR_X;
    // 赋值：把右侧 ::app::config::TOUCH_MIRROR_Y 的结果保存到左侧 tc.mirror_y。
    tc.mirror_y = ::app::config::TOUCH_MIRROR_Y;

    // 判断条件：!g_touch.init(g_i2c.get_bus_handle(), &tc) 是否成立。
    if (!g_touch.init(g_i2c.get_bus_handle(), &tc))
        // 返回 core::Status::IoError 给调用者，表示当前函数的执行结果。
        return core::Status::IoError;
    // 调用 enable_interrupt()：它们配置 FT6336U 的中断输出模式。启用后，芯片有触摸状态变化时拉低中断引脚。。
    (void)g_touch.enable_interrupt();

    // 判断条件：!input::InputRouter::get_instance().init(&g_touch) 是否成立。
    if (!input::InputRouter::get_instance().init(&g_touch))
        // 返回 core::Status::IoError 给调用者，表示当前函数的执行结果。
        return core::Status::IoError;
    // 返回 core::Status::Ok 给调用者，表示当前函数的执行结果。
    return core::Status::Ok;
}

// [初学者导读] step_audio()：初始化音频链路。详见 docs/代码详解-初学者版.md。
core::Status step_audio()
{
    // 判断条件：!AudioCodec::get_instance().init(g_i2c.get_bus_handle()) 是否成立。
    if (!AudioCodec::get_instance().init(g_i2c.get_bus_handle()))
    {
        // 调用 EBOOK_LOGW(TAG, "audio init fail");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "audio init fail");
        // 返回 core::Status::Ok 给调用者，表示当前函数的执行结果。
        return core::Status::Ok;
    }
    // 判断条件：!MusicPlayer::get_instance().init() 是否成立。
    if (!MusicPlayer::get_instance().init())
        // 调用 EBOOK_LOGW(TAG, "music player init fail");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "music player init fail");
    // 判断条件：!WoodenFishSound::get_instance().init() 是否成立。
    if (!WoodenFishSound::get_instance().init())
        // 调用 EBOOK_LOGW(TAG, "wooden fish sound init fail");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "wooden fish sound init fail");
    // 返回 core::Status::Ok 给调用者，表示当前函数的执行结果。
    return core::Status::Ok;
}

// [初学者导读] wire_system_state_observers()：把状态变化接到硬件。详见 docs/代码详解-初学者版.md。
void wire_system_state_observers()
{
    // 调用 data::SystemState::get_instance().set_brightness_observer(，执行这个函数并处理它的返回值或状态变化。
    data::SystemState::get_instance().set_brightness_observer(
        // 继续上一行的表达式或参数列表：[](uint8_t v) { display::DisplayPort::instance().set_brightness(v); });。
        [](uint8_t v) { display::DisplayPort::instance().set_brightness(v); });
    // 调用 data::SystemState::get_instance().set_volume_observer(，执行这个函数并处理它的返回值或状态变化。
    data::SystemState::get_instance().set_volume_observer(
        // 继续上一行的表达式或参数列表：[](uint8_t /*v*/) { AudioCodec::get_instance().apply_system_volume(); });。
        [](uint8_t /*v*/) { AudioCodec::get_instance().apply_system_volume(); });
}

// [初学者导读] step_data_load()：加载持久化数据。详见 docs/代码详解-初学者版.md。
core::Status step_data_load()
{
    // 调用 data::SystemState::get_instance().load();，执行这个函数并处理它的返回值或状态变化。
    data::SystemState::get_instance().load();
    // 调用 apps::settings::LockPassword::get_instance().load();，执行这个函数并处理它的返回值或状态变化。
    apps::settings::LockPassword::get_instance().load();
    // 调用 apps::settings::AutoLock::get_instance().load();，执行这个函数并处理它的返回值或状态变化。
    apps::settings::AutoLock::get_instance().load();
    // 调用 apps::settings::PowerSave::get_instance().load();，执行这个函数并处理它的返回值或状态变化。
    apps::settings::PowerSave::get_instance().load();
    // 调用 input::KeyBindings::instance().load();，执行这个函数并处理它的返回值或状态变化。
    input::KeyBindings::instance().load();
    // 调用 apps::reader::ReadingStore::get_instance().load();，执行这个函数并处理它的返回值或状态变化。
    apps::reader::ReadingStore::get_instance().load();
    // 调用 apps::clock::ClockStore::get_instance().load();，执行这个函数并处理它的返回值或状态变化。
    apps::clock::ClockStore::get_instance().load();
    // 调用 apps::music::MusicStore::get_instance().load();，执行这个函数并处理它的返回值或状态变化。
    apps::music::MusicStore::get_instance().load();
    // 返回 core::Status::Ok 给调用者，表示当前函数的执行结果。
    return core::Status::Ok;
}

// [初学者导读] step_battery()：初始化电池采样器。详见 docs/代码详解-初学者版.md。
core::Status step_battery()
{
    // 调用 BatterySampler::get_instance().init();，执行这个函数并处理它的返回值或状态变化。
    BatterySampler::get_instance().init();
    // 返回 core::Status::Ok 给调用者，表示当前函数的执行结果。
    return core::Status::Ok;
}

// [初学者导读] step_dip_switch()：初始化拨码输入。详见 docs/代码详解-初学者版.md。
core::Status step_dip_switch()
{
    // 判断条件：!input::PhysicalInput::get_instance().init() 是否成立。
    if (!input::PhysicalInput::get_instance().init())
        // 调用 EBOOK_LOGW(TAG, "dip switch unavailable");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "dip switch unavailable");
    // 返回 core::Status::Ok 给调用者，表示当前函数的执行结果。
    return core::Status::Ok;
}

// [初学者导读] step_tasks_start()：启动所有常驻任务。详见 docs/代码详解-初学者版.md。
core::Status step_tasks_start()
{
    // 判断条件：!presenter::Presenter::instance().init() 是否成立。
    if (!presenter::Presenter::instance().init())
        // 返回 core::Status::OutOfResource 给调用者，表示当前函数的执行结果。
        return core::Status::OutOfResource;
    // 判断条件：!ui::UiBus::get_instance().init() 是否成立。
    if (!ui::UiBus::get_instance().init())
        // 返回 core::Status::OutOfResource 给调用者，表示当前函数的执行结果。
        return core::Status::OutOfResource;

    // 调用 register_defaults()：这里按固定顺序注册 Reader、Notepad、Gallery、Drawing、Music、Weather、Clock、Calendar、WoodenFish、Files、Update Stub、Settings。。
    apps::AppRegistry::instance().register_defaults();

    // 判断条件：!presenter::Presenter::instance().start() 是否成立。
    if (!presenter::Presenter::instance().start())
        // 返回 core::Status::IoError 给调用者，表示当前函数的执行结果。
        return core::Status::IoError;
    // 判断条件：!ui::UiLoop::instance().init() 是否成立。
    if (!ui::UiLoop::instance().init())
        // 返回 core::Status::Internal 给调用者，表示当前函数的执行结果。
        return core::Status::Internal;
    // 判断条件：!ui::UiLoop::instance().start() 是否成立。
    if (!ui::UiLoop::instance().start())
        // 返回 core::Status::Internal 给调用者，表示当前函数的执行结果。
        return core::Status::Internal;

    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const auto clk = data::Clock::now();
        // 调用 post_tick_clock(clk.hour,clk.minute)：这些函数的共同模式是： 1. 用 makeevent(kind) 创建零初始化事件。 2. 填写 payload。 3. 调用 post(e, 0)。。
        (void)ui::UiBus::get_instance().post_tick_clock(clk.hour, clk.minute);
    }

    // 判断条件：BatterySampler::get_instance().ready() 是否成立。
    if (BatterySampler::get_instance().ready())
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const auto& st = data::SystemState::get_instance();
        // 访问对象成员：(void)ui::UiBus::get_instance().post_tick_battery(st.battery_pct(), st.battery_mv());；读取或修改这个对象里的成员状态。
        (void)ui::UiBus::get_instance().post_tick_battery(st.battery_pct(), st.battery_mv());
    }

    // 判断条件：!input::InputRouter::get_instance().start() 是否成立。
    if (!input::InputRouter::get_instance().start())
        // 返回 core::Status::Internal 给调用者，表示当前函数的执行结果。
        return core::Status::Internal;

    // 判断条件：input::PhysicalInput::get_instance().is_ready() 是否成立。
    if (input::PhysicalInput::get_instance().is_ready())
    {
        // 判断条件：!input::PhysicalInput::get_instance().start() 是否成立。
        if (!input::PhysicalInput::get_instance().start())
            // 调用 EBOOK_LOGW(TAG, "dip input task start failed");，执行这个函数并处理它的返回值或状态变化。
            EBOOK_LOGW(TAG, "dip input task start failed");
    }

    // 调用 brightness()：把 0 到 100 的亮度限制在合法范围后交给背光驱动。墨水屏本身不依赖背光，背光是前光/照明系统，不能把亮度当成画面灰度。。
    display::DisplayPort::instance().set_brightness(data::SystemState::get_instance().brightness());
    // 调用 AudioCodec::get_instance().apply_system_volume();，执行这个函数并处理它的返回值或状态变化。
    AudioCodec::get_instance().apply_system_volume();

    // 调用 hotspot_apply_state()：打开时： 1. 启动 WiFi AP。 2. 启动文件服务器。 3. 显示 AP IP，通常为 192.168.4.1。。
    apps::settings::SettingsApp::instance().hotspot_apply_state();
    // 调用 bt_apply_state()：根据 SystemState.bluetooth： 打开时调用 BleMgr.init()，并按需要开始广播。 关闭时停止广播。。
    apps::settings::SettingsApp::instance().bt_apply_state();

    // 函数调用：::app::network::wifi::WiFiMgr::get_instance().set_state_cb(；执行该函数并处理它的返回值或副作用。
    ::app::network::wifi::WiFiMgr::get_instance().set_state_cb(
        // 开始代码块：下面是这个函数、分支或循环的主体。
        [](::app::network::wifi::State st, ::app::network::wifi::FailureReason r) {
            // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
            (void)ui::UiBus::get_instance().post_wifi_state(static_cast<uint8_t>(st),
                                                            // 继续上一行的表达式或参数列表：static_cast<uint8_t>(r));。
                                                            static_cast<uint8_t>(r));
        // 执行语句：});；它会更新一次变量、对象或程序状态。
        });

    // 调用 apps::settings::SettingsApp::instance().wifi_apply_state(2000);，执行这个函数并处理它的返回值或状态变化。
    apps::settings::SettingsApp::instance().wifi_apply_state(2000);

    // 调用 start()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
    Housekeeper::get_instance().start();
    // 返回 core::Status::Ok 给调用者，表示当前函数的执行结果。
    return core::Status::Ok;
}

// 编译期数组 kSteps：运行时只读的固定查表数据。
constexpr BootStep kSteps[] = {
    // 执行这条代码：{"system", step_system_services, false},。它会更新变量、对象状态或调用链。
    {"system", step_system_services, false},
    // 继续上一行的表达式或参数列表：{"persist", step_persist, false},。
    {"persist", step_persist, false},
    // 继续上一行的表达式或参数列表：{"display", step_display, true},。
    {"display", step_display, true},
    // 继续上一行的表达式或参数列表：{"touch", step_touch, true},。
    {"touch", step_touch, true},
    // 继续上一行的表达式或参数列表：{"audio", step_audio, false},。
    {"audio", step_audio, false},
    // 继续上一行的表达式或参数列表：{"data", step_data_load, false},。
    {"data", step_data_load, false},
    // 继续上一行的表达式或参数列表：{"battery", step_battery, false},。
    {"battery", step_battery, false},
    // 继续上一行的表达式或参数列表：{"dip", step_dip_switch, false},。
    {"dip", step_dip_switch, false},
    // 继续上一行的表达式或参数列表：{"tasks", step_tasks_start, true},。
    {"tasks", step_tasks_start, true},
};

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 函数定义/声明：调用 boot()：有顺序的启动流水线。
core::Status boot()
{
    // 调用 wire_system_state_observers()：把状态变化接到硬件。
    wire_system_state_observers();

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (const BootStep& step : kSteps)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Status st = step.run();
        // 判断条件：st != core::Status::Ok 是否成立。
        if (st != core::Status::Ok)
        {
            // 调用 EBOOK_LOGE(TAG, "step %s failed: %s", step.name, core::to_str(st));，执行这个函数并处理它的返回值或状态变化。
            EBOOK_LOGE(TAG, "step %s failed: %s", step.name, core::to_str(st));
            // 判断条件：step.required 是否成立。
            if (step.required)
                // 返回 st 给调用者，表示当前函数的执行结果。
                return st;
        }
    }

    // 调用 EBOOK_LOGD(TAG, "boot ok");，执行这个函数并处理它的返回值或状态变化。
    EBOOK_LOGD(TAG, "boot ok");
    // 返回 core::Status::Ok 给调用者，表示当前函数的执行结果。
    return core::Status::Ok;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::platform
