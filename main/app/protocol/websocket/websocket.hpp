// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/protocol/websocket/websocket.hpp
 * 作用：实现 WebSocket 连接、发送、接收和事件回调。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：网络请求可能阻塞，必须在后台任务或专门任务中执行。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <functional>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <mutex>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <string>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_err.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_websocket_client.h"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::protocol::websocket {

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class State
{
    // 执行这条代码：IDLE,。它会更新变量、对象状态或调用链。
    IDLE,
    // 继续上一行的表达式或参数列表：INITIALIZED,。
    INITIALIZED,
    // 继续上一行的表达式或参数列表：CONNECTING,。
    CONNECTING,
    // 继续上一行的表达式或参数列表：CONNECTED,。
    CONNECTED,
    // 继续上一行的表达式或参数列表：DISCONNECTED,。
    DISCONNECTED,
};

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class EventType
{
    // 执行这条代码：CONNECTED,。它会更新变量、对象状态或调用链。
    CONNECTED,
    // 继续上一行的表达式或参数列表：DISCONNECTED,。
    DISCONNECTED,
    // 继续上一行的表达式或参数列表：DATA,。
    DATA,
    // 继续上一行的表达式或参数列表：ERROR,。
    ERROR,
};

// struct DataEvent：把相关数据字段组合在一起，表示项目中的一个概念。
struct DataEvent
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t* data;
    // 执行语句：size_t length;；它会更新一次变量、对象或程序状态。
    size_t length;
    // 执行语句：bool is_text;；它会更新一次变量、对象或程序状态。
    bool is_text;
};

// struct ErrorEvent：把相关数据字段组合在一起，表示项目中的一个概念。
struct ErrorEvent
{
    // 执行语句：esp_err_t esp_err_code;；它会更新一次变量、对象或程序状态。
    esp_err_t esp_err_code;
    // 执行语句：esp_websocket_error_type_t error_type;；它会更新一次变量、对象或程序状态。
    esp_websocket_error_type_t error_type;
    // 执行语句：int handshake_status;；它会更新一次变量、对象或程序状态。
    int handshake_status;
    // 执行语句：std::string message;；它会更新一次变量、对象或程序状态。
    std::string message;
};

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ConnectedCallback = std::function<void()>;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using DisconnectedCallback = std::function<void()>;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using DataCallback = std::function<void(const DataEvent& event)>;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ErrorCallback = std::function<void(const ErrorEvent& event)>;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using StateCallback = std::function<void(State state)>;

// struct Config：把相关数据字段组合在一起，表示项目中的一个概念。
struct Config
{
    // 执行语句：std::string uri;；它会更新一次变量、对象或程序状态。
    std::string uri;
    // 执行语句：std::string host;；它会更新一次变量、对象或程序状态。
    std::string host;
    // 定义变量 int port 并初始化为 0。
    int port = 0;
    // 执行语句：std::string path;；它会更新一次变量、对象或程序状态。
    std::string path;
    // 执行语句：std::string subprotocol;；它会更新一次变量、对象或程序状态。
    std::string subprotocol;
    // 执行语句：std::string headers;；它会更新一次变量、对象或程序状态。
    std::string headers;
    // 定义变量 int ping_interval_sec 并初始化为 10。
    int ping_interval_sec = 10;
    // 定义变量 int pingpong_timeout_sec 并初始化为 10。
    int pingpong_timeout_sec = 10;
    // 定义变量 int reconnect_timeout_ms 并初始化为 10000。
    int reconnect_timeout_ms = 10000;
    // 定义变量 int network_timeout_ms 并初始化为 10000。
    int network_timeout_ms = 10000;
    // 定义变量 size_t buffer_size 并初始化为 16384。
    size_t buffer_size = 16384;
    // 定义变量 bool disable_auto_reconnect 并初始化为 false。
    bool disable_auto_reconnect = false;
    // 定义变量 bool disable_pingpong_discon 并初始化为 false。
    bool disable_pingpong_discon = false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* cert_pem = nullptr;
    // 定义变量 size_t cert_len 并初始化为 0。
    size_t cert_len = 0;
    // 定义变量 bool skip_cert_common_name_check 并初始化为 false。
    bool skip_cert_common_name_check = false;
    // 定义变量 bool use_global_ca_store 并初始化为 false。
    bool use_global_ca_store = false;
    // 定义变量 bool skip_cert_verification 并初始化为 true。
    bool skip_cert_verification = true;
};

/** @brief esp_websocket_client 单例封装 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class WebSocketMgr
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static WebSocketMgr& get_instance();

    // 函数调用：bool init(const Config& config);；执行该函数并处理它的返回值或副作用。
    bool init(const Config& config);
    // 调用 deinit()：创建和销毁队列。
    void deinit();

    // 函数调用：bool connect();；执行该函数并处理它的返回值或副作用。
    bool connect();
    // 函数调用：bool disconnect();；执行该函数并处理它的返回值或副作用。
    bool disconnect();

    // 赋值：把右侧 5000) 的结果保存到左侧 int send_text(const std::string& text, int timeout_ms。
    int send_text(const std::string& text, int timeout_ms = 5000);
    // 赋值：把右侧 5000) 的结果保存到左侧 int send_binary(const uint8_t* data, size_t len, int timeout_ms。
    int send_binary(const uint8_t* data, size_t len, int timeout_ms = 5000);

    // 函数调用：State get_state() const;；执行该函数并处理它的返回值或副作用。
    State get_state() const;
    // 函数调用：bool is_connected() const;；执行该函数并处理它的返回值或副作用。
    bool is_connected() const;
    // 函数调用：bool is_init() const;；执行该函数并处理它的返回值或副作用。
    bool is_init() const;

    // 函数调用：void set_connected_callback(ConnectedCallback callback);；执行该函数并处理它的返回值或副作用。
    void set_connected_callback(ConnectedCallback callback);
    // 函数调用：void set_disconnected_callback(DisconnectedCallback callback);；执行该函数并处理它的返回值或副作用。
    void set_disconnected_callback(DisconnectedCallback callback);
    // 函数调用：void set_data_callback(DataCallback callback);；执行该函数并处理它的返回值或副作用。
    void set_data_callback(DataCallback callback);
    // 函数调用：void set_error_callback(ErrorCallback callback);；执行该函数并处理它的返回值或副作用。
    void set_error_callback(ErrorCallback callback);
    // 函数调用：void set_state_callback(StateCallback callback);；执行该函数并处理它的返回值或副作用。
    void set_state_callback(StateCallback callback);

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 静态函数 websocket_event_handler：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void websocket_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id,
                                        // 继续上一行的表达式或参数列表：void* event_data);。
                                        void* event_data);

    // 函数调用：void handle_connected();；执行该函数并处理它的返回值或副作用。
    void handle_connected();
    // 函数调用：void handle_disconnected();；执行该函数并处理它的返回值或副作用。
    void handle_disconnected();
    // 函数调用：void handle_data(esp_websocket_event_data_t* event_data);；执行该函数并处理它的返回值或副作用。
    void handle_data(esp_websocket_event_data_t* event_data);
    // 调用 handle_error(esp_websocket_event_data_t*event_data)：统一把状态置为失败并触发最终回调。集中处理错误可以让下载、校验和网络失败行为一致。。
    void handle_error(esp_websocket_event_data_t* event_data);
    // 函数调用：void set_state(State new_state);；执行该函数并处理它的返回值或副作用。
    void set_state(State new_state);

    // 调用 WebSocketMgr() = default;，执行这个函数并处理它的返回值或状态变化。
    WebSocketMgr() = default;
    // 函数调用：~WebSocketMgr();；执行该函数并处理它的返回值或副作用。
    ~WebSocketMgr();
    // 调用 WebSocketMgr(const WebSocketMgr&) = delete;，执行这个函数并处理它的返回值或状态变化。
    WebSocketMgr(const WebSocketMgr&) = delete;
    // 赋值：把右侧 (const WebSocketMgr&) = delete 的结果保存到左侧 WebSocketMgr& operator。
    WebSocketMgr& operator=(const WebSocketMgr&) = delete;

    // 执行语句：mutable std::mutex mutex_;；它会更新一次变量、对象或程序状态。
    mutable std::mutex mutex_;
    // 定义变量 State state_ 并初始化为 State::IDLE。
    State state_ = State::IDLE;
    // 定义变量 bool initialized_ 并初始化为 false。
    bool initialized_ = false;

    // 定义变量 esp_websocket_client_handle_t client_handle_ 并初始化为 nullptr。
    esp_websocket_client_handle_t client_handle_ = nullptr;
    // 执行语句：Config config_;；它会更新一次变量、对象或程序状态。
    Config config_;

    // 执行语句：ConnectedCallback connected_callback_;；它会更新一次变量、对象或程序状态。
    ConnectedCallback connected_callback_;
    // 执行语句：DisconnectedCallback disconnected_callback_;；它会更新一次变量、对象或程序状态。
    DisconnectedCallback disconnected_callback_;
    // 执行语句：DataCallback data_callback_;；它会更新一次变量、对象或程序状态。
    DataCallback data_callback_;
    // 执行语句：ErrorCallback error_callback_;；它会更新一次变量、对象或程序状态。
    ErrorCallback error_callback_;
    // 执行语句：StateCallback state_callback_;；它会更新一次变量、对象或程序状态。
    StateCallback state_callback_;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::protocol::websocket
