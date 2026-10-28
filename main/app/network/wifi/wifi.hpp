// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/network/wifi/wifi.hpp
 * 作用：初始化 Wi-Fi、扫描 AP、连接网络、保存凭据并广播状态。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：网络回调不在 UI 线程，状态变化通常通过 UiBus 回到页面。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <functional>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <memory>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <mutex>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <vector>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/event/event.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/task/task.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::network::wifi {

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class State
{
    // 执行这条代码：DISCONNECTED,。它会更新变量、对象状态或调用链。
    DISCONNECTED,
    // 继续上一行的表达式或参数列表：CONNECTING,。
    CONNECTING,
    // 继续上一行的表达式或参数列表：CONNECTED,。
    CONNECTED,
    // 继续上一行的表达式或参数列表：FAILED,。
    FAILED,
};

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class FailureReason
{
    // 执行这条代码：NONE,。它会更新变量、对象状态或调用链。
    NONE,
    // 继续上一行的表达式或参数列表：TIMEOUT,。
    TIMEOUT,
    // 继续上一行的表达式或参数列表：WRONG_PASSWORD,。
    WRONG_PASSWORD,
    // 继续上一行的表达式或参数列表：NETWORK_NOT_FOUND,。
    NETWORK_NOT_FOUND,
    // 继续上一行的表达式或参数列表：CONNECTION_FAILED,。
    CONNECTION_FAILED,
    // 继续上一行的表达式或参数列表：UNKNOWN,。
    UNKNOWN,
};

// struct Credentials：把相关数据字段组合在一起，表示项目中的一个概念。
struct Credentials
{
    // 执行语句：char ssid[32]{};；它会更新一次变量、对象或程序状态。
    char ssid[32]{};
    // 执行语句：char password[64]{};；它会更新一次变量、对象或程序状态。
    char password[64]{};

    // 调用 Credentials() = default;，执行这个函数并处理它的返回值或状态变化。
    Credentials() = default;

    // 定义或声明函数 Credentials：参数在括号里，返回类型写在函数名前面。
    Credentials(const char* s, const char* p)
    {
        // 调用 set_ssid(s);，执行这个函数并处理它的返回值或状态变化。
        set_ssid(s);
        // 调用 set_password(p);，执行这个函数并处理它的返回值或状态变化。
        set_password(p);
    }

    // 定义或声明函数 set_ssid：参数在括号里，返回类型写在函数名前面。
    void set_ssid(const char* s)
    {
        // 判断条件：s != nullptr 是否成立。
        if (s != nullptr)
        {
            // 调用 strncpy(ssid, s, sizeof(ssid) - 1);，执行这个函数并处理它的返回值或状态变化。
            strncpy(ssid, s, sizeof(ssid) - 1);
            // 赋值：把右侧 '\0' 的结果保存到左侧 ssid[sizeof(ssid) - 1]。
            ssid[sizeof(ssid) - 1] = '\0';
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 赋值：把右侧 '\0' 的结果保存到左侧 ssid[0]。
            ssid[0] = '\0';
        }
    }

    // 定义或声明函数 set_password：参数在括号里，返回类型写在函数名前面。
    void set_password(const char* p)
    {
        // 判断条件：p != nullptr 是否成立。
        if (p != nullptr)
        {
            // 调用 strncpy(password, p, sizeof(password) - 1);，执行这个函数并处理它的返回值或状态变化。
            strncpy(password, p, sizeof(password) - 1);
            // 赋值：把右侧 '\0' 的结果保存到左侧 password[sizeof(password) - 1]。
            password[sizeof(password) - 1] = '\0';
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 赋值：把右侧 '\0' 的结果保存到左侧 password[0]。
            password[0] = '\0';
        }
    }

    // 定义或声明函数 valid：参数在括号里，返回类型写在函数名前面。
    bool valid() const
    {
        // 返回 ssid[0] != '\0' 给调用者，表示当前函数的执行结果。
        return ssid[0] != '\0';
    }
};

// struct ApInfo：把相关数据字段组合在一起，表示项目中的一个概念。
struct ApInfo
{
    // 执行语句：char ssid[32]{};；它会更新一次变量、对象或程序状态。
    char ssid[32]{};
    // 执行语句：uint8_t bssid[6]{};；它会更新一次变量、对象或程序状态。
    uint8_t bssid[6]{};
    // 声明并默认初始化对象：int8_t rssi{}。
    int8_t rssi{};
    // 声明并默认初始化对象：uint8_t authmode{}。
    uint8_t authmode{};
    // 声明并默认初始化对象：bool encrypted{}。
    bool encrypted{};

    // 调用 ApInfo() = default;，执行这个函数并处理它的返回值或状态变化。
    ApInfo() = default;
};

// struct Info：把相关数据字段组合在一起，表示项目中的一个概念。
struct Info
{
    // 执行语句：State state{State::DISCONNECTED};；它会更新一次变量、对象或程序状态。
    State state{State::DISCONNECTED};
    // 执行语句：FailureReason fail_reason{FailureReason::NONE};；它会更新一次变量、对象或程序状态。
    FailureReason fail_reason{FailureReason::NONE};
    // 执行语句：char ssid[32]{};；它会更新一次变量、对象或程序状态。
    char ssid[32]{};
    // 执行语句：uint8_t ip[4]{};；它会更新一次变量、对象或程序状态。
    uint8_t ip[4]{};
    // 声明并默认初始化对象：int8_t rssi{}。
    int8_t rssi{};

    // 调用 Info() = default;，执行这个函数并处理它的返回值或状态变化。
    Info() = default;
};

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using StateCb = std::function<void(State state, FailureReason reason)>;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ScanCb = std::function<void(const std::vector<ApInfo>& aps)>;

/** @brief WiFi STA + SoftAP；凭证存 NVS 命名空间 wifi */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class WiFiMgr
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static WiFiMgr& get_instance();

    // 函数调用：bool init();；执行该函数并处理它的返回值或副作用。
    bool init();
    // 调用 deinit()：创建和销毁队列。
    void deinit();

    // 赋值：把右侧 nullptr) 的结果保存到左侧 bool scan(ScanCb cb。
    bool scan(ScanCb cb = nullptr);
    // 函数调用：bool connect(const char* ssid = nullptr, const char* password = nullptr,；执行该函数并处理它的返回值或副作用。
    bool connect(const char* ssid = nullptr, const char* password = nullptr,
                 // 继续上一行的表达式或参数列表：uint32_t timeout_ms = 30000);。
                 uint32_t timeout_ms = 30000);
    // 函数调用：void disconnect();；执行该函数并处理它的返回值或副作用。
    void disconnect();

    // 只读约束：这个值或对象在当前位置不能被修改。
    const Info& get_info() const;
    // 函数调用：State get_state() const;；执行该函数并处理它的返回值或副作用。
    State get_state() const;
    // 函数调用：bool is_connected() const;；执行该函数并处理它的返回值或副作用。
    bool is_connected() const;

    // 函数调用：void set_state_cb(StateCb cb);；执行该函数并处理它的返回值或副作用。
    void set_state_cb(StateCb cb);

    // 函数调用：bool save_creds(const Credentials& creds);；执行该函数并处理它的返回值或副作用。
    bool save_creds(const Credentials& creds);
    // 函数调用：bool get_creds(std::vector<Credentials>& creds);；执行该函数并处理它的返回值或副作用。
    bool get_creds(std::vector<Credentials>& creds);
    // 函数调用：bool clear_creds();；执行该函数并处理它的返回值或副作用。
    bool clear_creds();
    // 函数调用：bool remove_creds(const char* ssid);；执行该函数并处理它的返回值或副作用。
    bool remove_creds(const char* ssid);
    // 函数调用：bool has_saved_creds();；执行该函数并处理它的返回值或副作用。
    bool has_saved_creds();

    /** 将当前 STA 配置（SSID/密码）写入 NVS，供已保存列表展示与重连 */
    // 函数调用：bool persist_sta_creds();；执行该函数并处理它的返回值或副作用。
    bool persist_sta_creds();

    /** APSTA；password≥8 为 WPA2，否则开放 */
    // 赋值：把右侧 nullptr, uint8_t channel = 6) 的结果保存到左侧 bool enable_ap(const char* ssid, const char* password。
    bool enable_ap(const char* ssid, const char* password = nullptr, uint8_t channel = 6);

    // 函数调用：bool disable_ap();；执行该函数并处理它的返回值或副作用。
    bool disable_ap();
    // 函数调用：bool is_ap_active() const;；执行该函数并处理它的返回值或副作用。
    bool is_ap_active() const;
    // 函数调用：uint8_t get_ap_sta_count() const;；执行该函数并处理它的返回值或副作用。
    uint8_t get_ap_sta_count() const;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 WiFiMgr();，执行这个函数并处理它的返回值或状态变化。
    WiFiMgr();
    // 函数调用：~WiFiMgr();；执行该函数并处理它的返回值或副作用。
    ~WiFiMgr();
    // 调用 WiFiMgr(const WiFiMgr&) = delete;，执行这个函数并处理它的返回值或状态变化。
    WiFiMgr(const WiFiMgr&) = delete;
    // 赋值：把右侧 (const WiFiMgr&) = delete 的结果保存到左侧 WiFiMgr& operator。
    WiFiMgr& operator=(const WiFiMgr&) = delete;

    // 函数调用：void handle_wifi_event(esp_event_base_t base, app::sys::event::EventId id,；执行该函数并处理它的返回值或副作用。
    void handle_wifi_event(esp_event_base_t base, app::sys::event::EventId id,
                           // 只读约束：这个值或对象在当前位置不能被修改。
                           const app::sys::event::EventData& data);
    // 函数调用：void handle_ip_event(esp_event_base_t base, app::sys::event::EventId id,；执行该函数并处理它的返回值或副作用。
    void handle_ip_event(esp_event_base_t base, app::sys::event::EventId id,
                         // 只读约束：这个值或对象在当前位置不能被修改。
                         const app::sys::event::EventData& data);

    // 赋值：把右侧 FailureReason::NONE) 的结果保存到左侧 void update_state(State state, FailureReason reason。
    void update_state(State state, FailureReason reason = FailureReason::NONE);
    // 函数调用：void clear_timeout_task();；执行该函数并处理它的返回值或副作用。
    void clear_timeout_task();
    // 函数调用：void check_timeout();；执行该函数并处理它的返回值或副作用。
    void check_timeout();

    // 静态函数 timeout_task_fn：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void timeout_task_fn(void* arg);

    // 执行语句：mutable std::mutex mtx_;；它会更新一次变量、对象或程序状态。
    mutable std::mutex mtx_;
    // 执行语句：bool init_;；它会更新一次变量、对象或程序状态。
    bool init_;
    // 执行语句：bool ap_active_{false};；它会更新一次变量、对象或程序状态。
    bool ap_active_{false};
    // 执行语句：Info info_;；它会更新一次变量、对象或程序状态。
    Info info_;
    // 执行语句：StateCb state_cb_;；它会更新一次变量、对象或程序状态。
    StateCb state_cb_;
    // 执行语句：ScanCb scan_cb_;；它会更新一次变量、对象或程序状态。
    ScanCb scan_cb_;
    // 执行语句：uint32_t conn_timeout_ms_;；它会更新一次变量、对象或程序状态。
    uint32_t conn_timeout_ms_;
    // 执行语句：bool conn_timeout_set_;；它会更新一次变量、对象或程序状态。
    bool conn_timeout_set_;
    // 执行语句：uint32_t conn_start_tick_;；它会更新一次变量、对象或程序状态。
    uint32_t conn_start_tick_;
    // 执行语句：bool disconnecting_;；它会更新一次变量、对象或程序状态。
    bool disconnecting_;
    // 执行语句：bool scanning_;；它会更新一次变量、对象或程序状态。
    bool scanning_;
    // 执行语句：std::unique_ptr<app::sys::task::Task> timeout_task_;；它会更新一次变量、对象或程序状态。
    std::unique_ptr<app::sys::task::Task> timeout_task_;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::network::wifi
