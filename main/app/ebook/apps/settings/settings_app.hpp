// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/settings/settings_app.hpp
 * 作用：实现设置菜单、页面切换和设置页生命周期。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/input_bindings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/list_view.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_event.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::settings {

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class SettingsPage : uint8_t
{
    // 执行这条代码：Menu = 0,。它会更新变量、对象状态或调用链。
    Menu = 0,
    // 继续上一行的表达式或参数列表：Wifi,。
    Wifi,
    // 继续上一行的表达式或参数列表：Bluetooth,。
    Bluetooth,
    // 继续上一行的表达式或参数列表：Hotspot,。
    Hotspot,
    // 继续上一行的表达式或参数列表：Display,。
    Display,
    // 继续上一行的表达式或参数列表：Keys,。
    Keys,
    // 继续上一行的表达式或参数列表：KeysEdit,。
    KeysEdit,
    // 继续上一行的表达式或参数列表：Battery,。
    Battery,
    // 继续上一行的表达式或参数列表：Sound,。
    Sound,
    // 继续上一行的表达式或参数列表：Time,。
    Time,
    // 继续上一行的表达式或参数列表：Storage,。
    Storage,
    // 继续上一行的表达式或参数列表：Security,。
    Security,
    // 继续上一行的表达式或参数列表：About,。
    About,
};

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class SecPinPurpose : uint8_t
{
    // 执行这条代码：None = 0,。它会更新变量、对象状态或调用链。
    None = 0,
    // 继续上一行的表达式或参数列表：SetNew,。
    SetNew,
    // 继续上一行的表达式或参数列表：ConfirmNew,。
    ConfirmNew,
    // 继续上一行的表达式或参数列表：VerifyForChange,。
    VerifyForChange,
    // 继续上一行的表达式或参数列表：VerifyForClear,。
    VerifyForClear,
};

// 类定义：把数据和操作这些数据的方法绑定在一起。
class SettingsApp : public App
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static SettingsApp& instance();

    // 定义或声明函数 id：参数在括号里，返回类型写在函数名前面。
    AppId id() const override
    {
        // 返回 AppId::Settings 给调用者，表示当前函数的执行结果。
        return AppId::Settings;
    }
    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const char* title() const override;
    // 调用 icon_cp()：返回图标 codepoint。图标通常来自字体图标表，所以这里返回的是 uint32t，不是图片路径。。
    uint32_t icon_cp() const override;

    // 调用 on_enter()：进入文件 App 时： 1. 重置浏览状态或恢复根卷状态。 2. 清空弹窗。 3. 设置加载状态。 4. 调度目录扫描。 5. 请求重绘。。
    void on_enter() override;
    // 调用 on_exit()：退出时取消未完成的扫描、关闭弹窗，并确保后台任务不会在对象状态失效后继续写数据。。
    void on_exit() override;
    // 函数调用：void paint(gfx::Canvas& canvas) override;；执行该函数并处理它的返回值或副作用。
    void paint(gfx::Canvas& canvas) override;
    // 调用 on_input(const::app::ebook::input::Event&ev)：文件 App 的输入总入口。它处理： 触摸按下、移动、抬起和滑动。 工具栏返回。 列表滚动节流。 弹窗点击优先于底层列表点击。。
    shell::InputResult on_input(const ::app::ebook::input::Event& ev) override;
    // 调用 on_ui_event(constui::UiEvent&ev)：处理 UiBus 消息。文件模块最关心的是 FilesReloadDone：收到它以后，在 UI 线程调用 onbrowseready()。。
    void on_ui_event(const ui::UiEvent& ev) override;
    // 调用 on_semantic_back()：处理语义返回，顺序通常是： 1. 如果弹窗打开，先关闭弹窗。 2. 如果列表还在浏览目录，先返回上一级。 3. 如果已经到根卷，返回 false，让 Shell 退出 App。。
    bool on_semantic_back() override;

    // 调用 bt_apply_state()：根据 SystemState.bluetooth： 打开时调用 BleMgr.init()，并按需要开始广播。 关闭时停止广播。。
    void bt_apply_state();
    // 调用 hotspot_apply_state()：打开时： 1. 启动 WiFi AP。 2. 启动文件服务器。 3. 显示 AP IP，通常为 192.168.4.1。。
    void hotspot_apply_state();
    /** @brief 按 SystemState.wifi 开关 init/disconnect/自动连已保存网络；defer_ms>0 时延迟执行 */
    // 赋值：把右侧 0) 的结果保存到左侧 void wifi_apply_state(uint32_t defer_ms。
    void wifi_apply_state(uint32_t defer_ms = 0);

    // struct WifiAp：把相关数据字段组合在一起，表示项目中的一个概念。
    struct WifiAp
    {
        // 执行语句：char ssid[33]{};；它会更新一次变量、对象或程序状态。
        char ssid[33]{};
        // 执行语句：int8_t rssi{0};；它会更新一次变量、对象或程序状态。
        int8_t rssi{0};
        // 执行语句：bool encrypted{false};；它会更新一次变量、对象或程序状态。
        bool encrypted{false};
    };

    // 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
    enum class WifiRowKind : uint8_t
    {
        // 执行这条代码：Toggle = 0,。它会更新变量、对象状态或调用链。
        Toggle = 0,
        // 继续上一行的表达式或参数列表：SavedAp,。
        SavedAp,
        // 继续上一行的表达式或参数列表：ScanTrigger,。
        ScanTrigger,
        // 继续上一行的表达式或参数列表：ScannedAp,。
        ScannedAp,
    };

    // struct WifiRowDesc：把相关数据字段组合在一起，表示项目中的一个概念。
    struct WifiRowDesc
    {
        // 执行语句：WifiRowKind kind;；它会更新一次变量、对象或程序状态。
        WifiRowKind kind;
        // 执行语句：uint8_t index;；它会更新一次变量、对象或程序状态。
        uint8_t index;
    };

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 SettingsApp()：初始化菜单列表、默认值和成员状态。构造函数不做耗时网络连接。。
    SettingsApp();

    // 调用 enter_page(SettingsPagep)：切到新设置页。它通常先调用当前页的 leave 逻辑，再设置 page，最后调用新页的 enter 逻辑。。
    void enter_page(SettingsPage p);
    // 函数调用：void enter_keys_edit(::app::ebook::input::InputContext ctx);；执行该函数并处理它的返回值或副作用。
    void enter_keys_edit(::app::ebook::input::InputContext ctx);
    // 调用 exit_current_page()：执行当前页面离开时的清理。它是集中管理页面资源释放的地方。。
    void exit_current_page();
    // 调用 back_to_menu()：从二级页面返回设置菜单。它不等同于退出 SettingsApp，只有菜单页按返回才会退出 App。。
    bool back_to_menu();

    // 函数调用：void paint_menu(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_menu(gfx::Canvas& c);
    // 调用 paint_wifi(gfx::Canvas&c)：画 WiFi 页： 总开关。 当前连接状态。 已保存网络。 扫描按钮。 扫描结果和信号强度。 密码输入状态。。
    void paint_wifi(gfx::Canvas& c);
    // 函数调用：void paint_bluetooth(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_bluetooth(gfx::Canvas& c);
    // 函数调用：void paint_hotspot(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_hotspot(gfx::Canvas& c);
    // 函数调用：void paint_display(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_display(gfx::Canvas& c);
    // 函数调用：void paint_keys(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_keys(gfx::Canvas& c);
    // 函数调用：void paint_keys_edit(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_keys_edit(gfx::Canvas& c);
    // 函数调用：void paint_sound(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_sound(gfx::Canvas& c);
    // 函数调用：void paint_time(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_time(gfx::Canvas& c);
    // 函数调用：void paint_storage(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_storage(gfx::Canvas& c);
    // 函数调用：void paint_security(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_security(gfx::Canvas& c);
    // 函数调用：void paint_about(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_about(gfx::Canvas& c);
    // 函数调用：void paint_placeholder(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_placeholder(gfx::Canvas& c);

    // 函数调用：shell::InputResult handle_menu(const ::app::ebook::input::Event& ev);；执行该函数并处理它的返回值或副作用。
    shell::InputResult handle_menu(const ::app::ebook::input::Event& ev);
    // 调用 handle_wifi(const::app::ebook::input::Event&ev)：处理 WiFi 页点击： 开关。 刷新或扫描。 连接已保存网络。 忘记网络。 选择扫描结果。 需要密码时打开键盘。。
    shell::InputResult handle_wifi(const ::app::ebook::input::Event& ev);
    // 函数调用：shell::InputResult handle_hotspot(const ::app::ebook::input::Event& ev);；执行该函数并处理它的返回值或副作用。
    shell::InputResult handle_hotspot(const ::app::ebook::input::Event& ev);
    // 函数调用：shell::InputResult handle_bluetooth(const ::app::ebook::input::Event& ev);；执行该函数并处理它的返回值或副作用。
    shell::InputResult handle_bluetooth(const ::app::ebook::input::Event& ev);
    // 函数调用：shell::InputResult handle_display(const ::app::ebook::input::Event& ev);；执行该函数并处理它的返回值或副作用。
    shell::InputResult handle_display(const ::app::ebook::input::Event& ev);
    // 函数调用：shell::InputResult handle_keys(const ::app::ebook::input::Event& ev);；执行该函数并处理它的返回值或副作用。
    shell::InputResult handle_keys(const ::app::ebook::input::Event& ev);
    // 函数调用：shell::InputResult handle_keys_edit(const ::app::ebook::input::Event& ev);；执行该函数并处理它的返回值或副作用。
    shell::InputResult handle_keys_edit(const ::app::ebook::input::Event& ev);
    // 函数调用：shell::InputResult handle_sound(const ::app::ebook::input::Event& ev);；执行该函数并处理它的返回值或副作用。
    shell::InputResult handle_sound(const ::app::ebook::input::Event& ev);
    // 函数调用：shell::InputResult handle_time(const ::app::ebook::input::Event& ev);；执行该函数并处理它的返回值或副作用。
    shell::InputResult handle_time(const ::app::ebook::input::Event& ev);
    // 函数调用：shell::InputResult handle_security(const ::app::ebook::input::Event& ev);；执行该函数并处理它的返回值或副作用。
    shell::InputResult handle_security(const ::app::ebook::input::Event& ev);
    // 函数调用：shell::InputResult handle_about(const ::app::ebook::input::Event& ev);；执行该函数并处理它的返回值或副作用。
    shell::InputResult handle_about(const ::app::ebook::input::Event& ev);

    // 调用 on_wifi_state(uint8_tstate,uint8_tfail)：接收 WiFi 驱动状态变化，并转换为用户可理解的显示。状态包括： 正在连接。 已连接。 连接失败。 已断开。。
    void on_wifi_state(uint8_t state, uint8_t fail);
    // 调用 on_wifi_scan_done()：扫描回调先把结果复制到 gpending，并用 gpendingmtx 保护；完成后投递 WifiScanDone。UI 线程收到事件后再读取 gpending 更新 wifiscan。。
    void on_wifi_scan_done();
    // 调用 on_ntp_sync_done(uint8_tstatus)：处理同步完成事件，更新页面状态。它由 UI 线程消费。。
    void on_ntp_sync_done(uint8_t status);

    // 调用 bt_enter()：进入页面时同步 BLE 当前状态，必要时刷新连接数。。
    void bt_enter();
    // 调用 bt_leave()：目前为空，因为离开页面不应自动关闭 BLE；BLE 状态是系统级设置，不是页面级资源。。
    void bt_leave();

    // 调用 wifi_enter()：进入 WiFi 页时刷新已保存网络，必要时触发扫描，更新页面状态。。
    void wifi_enter();
    // 调用 wifi_leave()：离开页面时停止扫描相关的 UI 状态。它不关闭整个 WiFi 网络，也不删除保存的 SSID。。
    void wifi_leave();
    // 调用 wifi_start_scan()：启动扫描。要防止重复扫描和扫描尚未结束时再次点击导致状态混乱。。
    void wifi_start_scan();
    // 调用 wifi_refresh_saved()：从网络管理器读取保存的凭据列表，更新最多 8 个 WifiAp。。
    void wifi_refresh_saved();
    // 调用 wifi_connect_to(uint8_tscan_idx,constchar*password)：根据扫描结果索引连接。密码可以为空，表示开放网络或使用已保存密码。连接前要检查索引范围。。
    void wifi_connect_to(uint8_t scan_idx, const char* password);
    // 调用 wifi_forget(constchar*ssid)：删除保存的凭据并刷新列表。它只影响保存记录，不一定立即断开当前连接；具体行为取决于网络管理接口。。
    void wifi_forget(const char* ssid);
    // 调用 wifi_reconnect_saved(constchar*ssid)：使用已保存凭据重新连接。SSID 是用户可控字符串，复制时必须限制长度。。
    void wifi_reconnect_saved(const char* ssid);
    // 调用 build_wifi_rows(WifiRowDesc*out,uint8_tcap)：把“开关行、已保存网络、扫描按钮、扫描结果”组合成逻辑行描述。绘制和点击共用这张表，避免它们对行顺序理解不一致。。
    uint8_t build_wifi_rows(WifiRowDesc* out, uint8_t cap) const;

    // 调用 try_ntp_auto_sync()：自动同步前必须确认 WiFi 已连接。NTP 需要网络，离线状态下只能读取本地时间或显示未同步。。
    void try_ntp_auto_sync();
    // 调用 try_ntp_auto_sync_on_connect(uint8_twifi_state)：WiFi 连接状态变化时触发自动同步。只有连接成功才继续，其他状态不应反复发 NTP 请求。。
    void try_ntp_auto_sync_on_connect(uint8_t wifi_state);
    // 静态函数 ensure_ntp_started：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool ensure_ntp_started();

    // 静态函数 on_hotspot_name_done：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void on_hotspot_name_done(const char* text, void* user);
    // 静态函数 on_hotspot_pwd_done：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void on_hotspot_pwd_done(const char* text, void* user);
    // 静态函数 on_wifi_pwd_done：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void on_wifi_pwd_done(const char* text, void* user);
    // 静态函数 on_bt_name_done：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void on_bt_name_done(const char* text, void* user);
    // 静态函数 on_sec_pin_done：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void on_sec_pin_done(const char* text, void* user);

    // 调用 open_pin_keyboard(constchar*title)：打开数字键盘。标题说明当前是设置、确认还是验证。。
    void open_pin_keyboard(const char* title);
    // 调用 begin_set_pin_flow(boolrequire_old)：初始化流水线状态。requireold 为真时先验证旧 PIN，否则直接输入新 PIN。。
    void begin_set_pin_flow(bool require_old);
    // 调用 on_sec_pin_done_impl(constchar*text)：键盘静态回调先把 user 转回 SettingsApp，再由 impl 根据 secpinpurpose 执行当前步骤。这样可以避免静态键盘回调直接乱改页面状态。。
    void on_sec_pin_done_impl(const char* text);

    // 调用 load_hotspot_prefs()：从 NVS 读取或保存名称和密码。加载失败时保留默认值。。
    void load_hotspot_prefs();
    // 调用 save_hotspot_prefs()：从 NVS 读取或保存名称和密码。加载失败时保留默认值。。
    void save_hotspot_prefs();
    // 调用 load_time_prefs()：读写时区索引和自动同步开关。。
    void load_time_prefs();
    // 调用 save_time_prefs()：读写时区索引和自动同步开关。。
    void save_time_prefs();
    // 调用 apply_tz_from_index()：把 NVS 索引转换成 POSIX 时区字符串，格式为 CST%+d。例如东八区是 CST8 这种 TZ 形式，具体符号规则由 C 库时区约定决定。。
    void apply_tz_from_index();

    // 固定参数 kMaxScan = 16：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kMaxScan = 16;
    // 固定参数 kMaxSaved = 8：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kMaxSaved = 8;

    // 执行语句：SettingsPage page_{SettingsPage::Menu};；它会更新一次变量、对象或程序状态。
    SettingsPage page_{SettingsPage::Menu};
    // 执行语句：::app::ebook::input::InputContext keys_edit_ctx_{::app::ebook::input::InputContext::Reader};；它会更新一次变量、对象或程序状态。
    ::app::ebook::input::InputContext keys_edit_ctx_{::app::ebook::input::InputContext::Reader};
    // 声明并默认初始化对象：ui::ListView menu_list_{}。
    ui::ListView menu_list_{};

    // 执行语句：bool wifi_scanning_{false};；它会更新一次变量、对象或程序状态。
    bool wifi_scanning_{false};
    // 执行语句：uint8_t wifi_scan_count_{0};；它会更新一次变量、对象或程序状态。
    uint8_t wifi_scan_count_{0};
    // 执行语句：uint8_t wifi_saved_count_{0};；它会更新一次变量、对象或程序状态。
    uint8_t wifi_saved_count_{0};
    // 执行语句：uint8_t wifi_pending_idx_{0xFF};；它会更新一次变量、对象或程序状态。
    uint8_t wifi_pending_idx_{0xFF};
    // 执行语句：uint8_t wifi_state_{0};；它会更新一次变量、对象或程序状态。
    uint8_t wifi_state_{0};
    // 执行语句：uint8_t wifi_scroll_row_{0};；它会更新一次变量、对象或程序状态。
    uint8_t wifi_scroll_row_{0};
    // 执行语句：char wifi_pending_ssid_[33]{};；它会更新一次变量、对象或程序状态。
    char wifi_pending_ssid_[33]{};
    // 执行语句：char wifi_connected_ssid_[33]{};；它会更新一次变量、对象或程序状态。
    char wifi_connected_ssid_[33]{};
    // 执行语句：WifiAp wifi_scan_[kMaxScan]{};；它会更新一次变量、对象或程序状态。
    WifiAp wifi_scan_[kMaxScan]{};
    // 执行语句：WifiAp wifi_saved_[kMaxSaved]{};；它会更新一次变量、对象或程序状态。
    WifiAp wifi_saved_[kMaxSaved]{};

    // 执行语句：char hotspot_name_[33]{"Ebook_AP"};；它会更新一次变量、对象或程序状态。
    char hotspot_name_[33]{"Ebook_AP"};
    // 执行语句：char hotspot_pwd_[33]{"12345678"};；它会更新一次变量、对象或程序状态。
    char hotspot_pwd_[33]{"12345678"};
    // 执行语句：char bt_name_[33]{"Ebook-BLE"};；它会更新一次变量、对象或程序状态。
    char bt_name_[33]{"Ebook-BLE"};

    // 执行语句：uint8_t tz_idx_{20};；它会更新一次变量、对象或程序状态。
    uint8_t tz_idx_{20};
    // 执行语句：int8_t tz_offset_h_{8};；它会更新一次变量、对象或程序状态。
    int8_t tz_offset_h_{8};
    // 执行语句：bool ntp_auto_sync_{true};；它会更新一次变量、对象或程序状态。
    bool ntp_auto_sync_{true};

    // 执行语句：uint8_t about_scroll_row_{0};；它会更新一次变量、对象或程序状态。
    uint8_t about_scroll_row_{0};

    // 执行语句：SecPinPurpose sec_pin_purpose_{SecPinPurpose::None};；它会更新一次变量、对象或程序状态。
    SecPinPurpose sec_pin_purpose_{SecPinPurpose::None};
    // 执行语句：char sec_pending_pin_[5]{};；它会更新一次变量、对象或程序状态。
    char sec_pending_pin_[5]{};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::settings
