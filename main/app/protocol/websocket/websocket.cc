/**
 * [初学者导读] 文件：main/app/protocol/websocket/websocket.cc
 * 作用：实现 WebSocket 连接、发送、接收和事件回调。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：网络请求可能阻塞，必须在后台任务或专门任务中执行。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "websocket.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_log.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_websocket_client.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_crt_bundle.h"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "WebSocket";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::protocol::websocket {

// 定义或声明函数 WebSocketMgr::get_instance：参数在括号里，返回类型写在函数名前面。
WebSocketMgr& WebSocketMgr::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static WebSocketMgr instance;
    // 返回 instance 给调用者，表示当前函数的执行结果。
    return instance;
}

// 定义或声明函数 WebSocketMgr::init：参数在括号里，返回类型写在函数名前面。
bool WebSocketMgr::init(const Config& config)
{
    // 执行语句：StateCallback state_cb;；它会更新一次变量、对象或程序状态。
    StateCallback state_cb;
    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);

        // 判断条件：initialized_ 是否成立。
        if (initialized_)
        {
            // 调用 ESP_LOGW(TAG, "WebSocket 客户端已初始化");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGW(TAG, "WebSocket 客户端已初始化");
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }

        // 赋值：把右侧 config 的结果保存到左侧 config_。
        config_ = config;

        // 配置 WebSocket 客户端
        esp_websocket_client_config_t ws_cfg = {};

        // 判断条件：!config.uri.empty() 是否成立。
        if (!config.uri.empty())
        {
            // 赋值：把右侧 config.uri.c_str() 的结果保存到左侧 ws_cfg.uri。
            ws_cfg.uri = config.uri.c_str();
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 判断条件：!config.host.empty() 是否成立。
            if (!config.host.empty())
            {
                // 赋值：把右侧 config.host.c_str() 的结果保存到左侧 ws_cfg.host。
                ws_cfg.host = config.host.c_str();
            }
            // 赋值：把右侧 config.port 的结果保存到左侧 ws_cfg.port。
            ws_cfg.port = config.port;
            // 判断条件：!config.path.empty() 是否成立。
            if (!config.path.empty())
            {
                // 赋值：把右侧 config.path.c_str() 的结果保存到左侧 ws_cfg.path。
                ws_cfg.path = config.path.c_str();
            }
        }

        // 判断条件：!config.subprotocol.empty() 是否成立。
        if (!config.subprotocol.empty())
        {
            // 赋值：把右侧 config.subprotocol.c_str() 的结果保存到左侧 ws_cfg.subprotocol。
            ws_cfg.subprotocol = config.subprotocol.c_str();
        }

        // 判断条件：!config.headers.empty() 是否成立。
        if (!config.headers.empty())
        {
            // 赋值：把右侧 config.headers.c_str() 的结果保存到左侧 ws_cfg.headers。
            ws_cfg.headers = config.headers.c_str();
        }

        // 赋值：把右侧 config.ping_interval_sec 的结果保存到左侧 ws_cfg.ping_interval_sec。
        ws_cfg.ping_interval_sec = config.ping_interval_sec;
        // 赋值：把右侧 config.pingpong_timeout_sec 的结果保存到左侧 ws_cfg.pingpong_timeout_sec。
        ws_cfg.pingpong_timeout_sec = config.pingpong_timeout_sec;
        // 赋值：把右侧 config.reconnect_timeout_ms 的结果保存到左侧 ws_cfg.reconnect_timeout_ms。
        ws_cfg.reconnect_timeout_ms = config.reconnect_timeout_ms;
        // 赋值：把右侧 config.network_timeout_ms 的结果保存到左侧 ws_cfg.network_timeout_ms。
        ws_cfg.network_timeout_ms = config.network_timeout_ms;
        // 赋值：把右侧 config.buffer_size 的结果保存到左侧 ws_cfg.buffer_size。
        ws_cfg.buffer_size = config.buffer_size;
        // 赋值：把右侧 config.disable_auto_reconnect 的结果保存到左侧 ws_cfg.disable_auto_reconnect。
        ws_cfg.disable_auto_reconnect = config.disable_auto_reconnect;
        // 赋值：把右侧 config.disable_pingpong_discon 的结果保存到左侧 ws_cfg.disable_pingpong_discon。
        ws_cfg.disable_pingpong_discon = config.disable_pingpong_discon;
        // 赋值：把右侧 config.skip_cert_common_name_check 的结果保存到左侧 ws_cfg.skip_cert_common_name_check。
        ws_cfg.skip_cert_common_name_check = config.skip_cert_common_name_check;
        // 赋值：把右侧 config.use_global_ca_store 的结果保存到左侧 ws_cfg.use_global_ca_store。
        ws_cfg.use_global_ca_store = config.use_global_ca_store;

        // 配置 WSS/TLS 验证选项
        if (!config.uri.empty() && (config.uri.find("wss://") == 0))
        {
            // 判断条件：config.cert_pem != nullptr && config.cert_len > 0 是否成立。
            if (config.cert_pem != nullptr && config.cert_len > 0)
            {
                // 使用自定义证书
                ws_cfg.cert_pem = config.cert_pem;
                // 赋值：把右侧 config.cert_len 的结果保存到左侧 ws_cfg.cert_len。
                ws_cfg.cert_len = config.cert_len;
                // 调用 ESP_LOGD(TAG, "WSS 连接：使用自定义证书");，执行这个函数并处理它的返回值或状态变化。
                ESP_LOGD(TAG, "WSS 连接：使用自定义证书");
            }
            // 备用分支：前面的条件不成立时执行这里。
            else if (config.skip_cert_verification)
            {
                // 跳过验证模式：挂载 bundle 但跳过名称检查
                // 这样可以绕过 "No server verification option set" 检查
                ws_cfg.crt_bundle_attach = esp_crt_bundle_attach;
                // 赋值：把右侧 true 的结果保存到左侧 ws_cfg.skip_cert_common_name_check。
                ws_cfg.skip_cert_common_name_check = true;
                // 调用 ESP_LOGW(TAG, "WSS 连接：跳过证书验证（仅用于测试）");，执行这个函数并处理它的返回值或状态变化。
                ESP_LOGW(TAG, "WSS 连接：跳过证书验证（仅用于测试）");
            }
            // 备用分支：前面的条件不成立时执行这里。
            else
            {
                // 默认模式：使用 ESP32 自带的 CA 证书包进行标准验证
                ws_cfg.crt_bundle_attach = esp_crt_bundle_attach;
                // 调用 ESP_LOGD(TAG, "WSS 连接：使用系统证书包验证");，执行这个函数并处理它的返回值或状态变化。
                ESP_LOGD(TAG, "WSS 连接：使用系统证书包验证");
            }
        }

        // 自动判断传输类型（根据 URI 协议或端口）
        if (!config.uri.empty())
        {
            // 根据 URI 协议自动选择传输层
            ws_cfg.transport = (config.uri.find("wss://") == 0 || config.uri.find("https://") == 0)
                                   // 执行这条代码：? WEBSOCKET_TRANSPORT_OVER_SSL。它会更新变量、对象状态或调用链。
                                   ? WEBSOCKET_TRANSPORT_OVER_SSL
                                   // 执行语句：: WEBSOCKET_TRANSPORT_OVER_TCP;；它会更新一次变量、对象或程序状态。
                                   : WEBSOCKET_TRANSPORT_OVER_TCP;
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // URI 为空时，根据端口判断（443 通常为 SSL）
            ws_cfg.transport =
                // 继续上一行的表达式或参数列表：(config.port == 443) ? WEBSOCKET_TRANSPORT_OVER_SSL : WEBSOCKET_TRANSPORT_OVER_TCP;。
                (config.port == 443) ? WEBSOCKET_TRANSPORT_OVER_SSL : WEBSOCKET_TRANSPORT_OVER_TCP;
        }

        // 设置事件处理器
        ws_cfg.user_context = this;

        // 初始化客户端
        client_handle_ = esp_websocket_client_init(&ws_cfg);
        // 判断条件：client_handle_ == nullptr 是否成立。
        if (client_handle_ == nullptr)
        {
            // 调用 ESP_LOGE(TAG, "WebSocket 客户端初始化失败");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "WebSocket 客户端初始化失败");
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }

        // 注册事件处理器
        esp_err_t ret = esp_websocket_register_events(client_handle_, WEBSOCKET_EVENT_ANY,
                                                      // 继续上一行的表达式或参数列表：websocket_event_handler, this);。
                                                      websocket_event_handler, this);
        // 判断条件：ret != ESP_OK 是否成立。
        if (ret != ESP_OK)
        {
            // 调用 ESP_LOGE(TAG, "注册事件处理器失败: %s", esp_err_to_name(ret));，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "注册事件处理器失败: %s", esp_err_to_name(ret));
            // 调用 esp_websocket_client_destroy(client_handle_);，执行这个函数并处理它的返回值或状态变化。
            esp_websocket_client_destroy(client_handle_);
            // 赋值：把右侧 nullptr 的结果保存到左侧 client_handle_。
            client_handle_ = nullptr;
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }

        // 赋值：把右侧 true 的结果保存到左侧 initialized_。
        initialized_ = true;
        // 赋值：把右侧 State::INITIALIZED 的结果保存到左侧 state_。
        state_ = State::INITIALIZED;
        // 赋值：把右侧 state_callback_ 的结果保存到左侧 state_cb。
        state_cb = state_callback_;
    }

    // 在锁外调用回调
    if (state_cb)
    {
        // 调用 state_cb(State::INITIALIZED);，执行这个函数并处理它的返回值或状态变化。
        state_cb(State::INITIALIZED);
    }

    // 调用 ESP_LOGD(TAG, "WebSocket 客户端初始化成功");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "WebSocket 客户端初始化成功");
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 WebSocketMgr::~WebSocketMgr：参数在括号里，返回类型写在函数名前面。
WebSocketMgr::~WebSocketMgr()
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 判断条件：initialized_ 是否成立。
    if (initialized_)
    {
        // 在析构函数中不调用 deinit()，因为它需要 unlock mutex
        // 直接清理资源
        if (state_ == State::CONNECTED || state_ == State::CONNECTING)
        {
            // 判断条件：client_handle_ != nullptr 是否成立。
            if (client_handle_ != nullptr)
            {
                // 调用 esp_websocket_client_stop(client_handle_);，执行这个函数并处理它的返回值或状态变化。
                esp_websocket_client_stop(client_handle_);
            }
        }

        // 判断条件：client_handle_ != nullptr 是否成立。
        if (client_handle_ != nullptr)
        {
            // 调用 esp_websocket_client_destroy(client_handle_);，执行这个函数并处理它的返回值或状态变化。
            esp_websocket_client_destroy(client_handle_);
            // 赋值：把右侧 nullptr 的结果保存到左侧 client_handle_。
            client_handle_ = nullptr;
        }

        // 赋值：把右侧 false 的结果保存到左侧 initialized_。
        initialized_ = false;
        // 赋值：把右侧 State::IDLE 的结果保存到左侧 state_。
        state_ = State::IDLE;
    }
}

// 定义或声明函数 WebSocketMgr::deinit：参数在括号里，返回类型写在函数名前面。
void WebSocketMgr::deinit()
{
    // 定义变量 bool should_stop 并初始化为 false。
    bool should_stop = false;

    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);

        // 判断条件：!initialized_ 是否成立。
        if (!initialized_)
        {
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }

        // 标记需要停止连接
        if (state_ == State::CONNECTED || state_ == State::CONNECTING)
        {
            // 赋值：把右侧 true 的结果保存到左侧 should_stop。
            should_stop = true;
        }
    }

    // 在锁外停止连接（避免死锁）
    if (should_stop && client_handle_ != nullptr)
    {
        // 调用 esp_websocket_client_stop(client_handle_);，执行这个函数并处理它的返回值或状态变化。
        esp_websocket_client_stop(client_handle_);
    }

    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);

        // 判断条件：client_handle_ != nullptr 是否成立。
        if (client_handle_ != nullptr)
        {
            // 调用 esp_websocket_client_destroy(client_handle_);，执行这个函数并处理它的返回值或状态变化。
            esp_websocket_client_destroy(client_handle_);
            // 赋值：把右侧 nullptr 的结果保存到左侧 client_handle_。
            client_handle_ = nullptr;
        }

        // 赋值：把右侧 false 的结果保存到左侧 initialized_。
        initialized_ = false;
        // 赋值：把右侧 State::IDLE 的结果保存到左侧 state_。
        state_ = State::IDLE;

        // 清空回调
        connected_callback_ = nullptr;
        // 赋值：把右侧 nullptr 的结果保存到左侧 disconnected_callback_。
        disconnected_callback_ = nullptr;
        // 赋值：把右侧 nullptr 的结果保存到左侧 data_callback_。
        data_callback_ = nullptr;
        // 赋值：把右侧 nullptr 的结果保存到左侧 error_callback_。
        error_callback_ = nullptr;
        // 赋值：把右侧 nullptr 的结果保存到左侧 state_callback_。
        state_callback_ = nullptr;
    }

    // 调用 ESP_LOGD(TAG, "WebSocket 客户端已反初始化");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "WebSocket 客户端已反初始化");
}

// 定义或声明函数 WebSocketMgr::connect：参数在括号里，返回类型写在函数名前面。
bool WebSocketMgr::connect()
{
    // 执行语句：StateCallback state_cb;；它会更新一次变量、对象或程序状态。
    StateCallback state_cb;
    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);

        // 判断条件：!initialized_ 是否成立。
        if (!initialized_)
        {
            // 调用 ESP_LOGE(TAG, "WebSocket 客户端未初始化");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "WebSocket 客户端未初始化");
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }

        // 判断条件：state_ == State::CONNECTED || state_ == State::CONNECTING 是否成立。
        if (state_ == State::CONNECTED || state_ == State::CONNECTING)
        {
            // 调用 ESP_LOGW(TAG, "WebSocket 已连接或正在连接中");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGW(TAG, "WebSocket 已连接或正在连接中");
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }

        // 判断条件：client_handle_ == nullptr 是否成立。
        if (client_handle_ == nullptr)
        {
            // 调用 ESP_LOGE(TAG, "WebSocket 客户端句柄无效");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "WebSocket 客户端句柄无效");
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }

        // 设置状态（在锁内）
        if (state_ != State::CONNECTING)
        {
            // 赋值：把右侧 State::CONNECTING 的结果保存到左侧 state_。
            state_ = State::CONNECTING;
            // 赋值：把右侧 state_callback_ 的结果保存到左侧 state_cb。
            state_cb = state_callback_;
        }
    }

    // 在锁外调用状态回调
    if (state_cb)
    {
        // 调用 state_cb(State::CONNECTING);，执行这个函数并处理它的返回值或状态变化。
        state_cb(State::CONNECTING);
    }

    // 定义变量 esp_err_t ret 并初始化为 esp_websocket_client_start(client_handle_)。
    esp_err_t ret = esp_websocket_client_start(client_handle_);
    // 判断条件：ret != ESP_OK 是否成立。
    if (ret != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "启动 WebSocket 连接失败: %s", esp_err_to_name(ret));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "启动 WebSocket 连接失败: %s", esp_err_to_name(ret));
        // 设置失败状态
        {
            // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
            std::lock_guard<std::mutex> lock(mutex_);
            // 判断条件：state_ == State::CONNECTING 是否成立。
            if (state_ == State::CONNECTING)
            {
                // 赋值：把右侧 State::DISCONNECTED 的结果保存到左侧 state_。
                state_ = State::DISCONNECTED;
                // 赋值：把右侧 state_callback_ 的结果保存到左侧 state_cb。
                state_cb = state_callback_;
            }
            // 备用分支：前面的条件不成立时执行这里。
            else
            {
                // 赋值：把右侧 nullptr 的结果保存到左侧 state_cb。
                state_cb = nullptr;
            }
        }
        // 判断条件：state_cb 是否成立。
        if (state_cb)
        {
            // 调用 state_cb(State::DISCONNECTED);，执行这个函数并处理它的返回值或状态变化。
            state_cb(State::DISCONNECTED);
        }
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 调用 ESP_LOGD(TAG, "正在连接 WebSocket 服务器...");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "正在连接 WebSocket 服务器...");
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 WebSocketMgr::disconnect：参数在括号里，返回类型写在函数名前面。
bool WebSocketMgr::disconnect()
{
    // 执行语句：StateCallback state_cb;；它会更新一次变量、对象或程序状态。
    StateCallback state_cb;
    // 定义变量 bool should_stop 并初始化为 false。
    bool should_stop = false;
    // 定义变量 esp_websocket_client_handle_t handle_to_stop 并初始化为 nullptr。
    esp_websocket_client_handle_t handle_to_stop = nullptr;

    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);

        // 判断条件：!initialized_ || client_handle_ == nullptr 是否成立。
        if (!initialized_ || client_handle_ == nullptr)
        {
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }

        // 判断条件：state_ != State::CONNECTED && state_ != State::CONNECTING 是否成立。
        if (state_ != State::CONNECTED && state_ != State::CONNECTING)
        {
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }

        // 赋值：把右侧 true 的结果保存到左侧 should_stop。
        should_stop = true;
        // 赋值：把右侧 client_handle_ 的结果保存到左侧 handle_to_stop。
        handle_to_stop = client_handle_;
        // 赋值：把右侧 state_callback_ 的结果保存到左侧 state_cb。
        state_cb = state_callback_;
    }

    // 在锁外调用 esp_websocket_client_stop（避免死锁）
    if (should_stop && handle_to_stop != nullptr)
    {
        // 定义变量 esp_err_t ret 并初始化为 esp_websocket_client_stop(handle_to_stop)。
        esp_err_t ret = esp_websocket_client_stop(handle_to_stop);
        // 判断条件：ret != ESP_OK 是否成立。
        if (ret != ESP_OK)
        {
            // 调用 ESP_LOGE(TAG, "断开 WebSocket 连接失败: %s", esp_err_to_name(ret));，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "断开 WebSocket 连接失败: %s", esp_err_to_name(ret));
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }

        // 更新状态（在锁内）
        {
            // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
            std::lock_guard<std::mutex> lock(mutex_);
            // 判断条件：state_ != State::DISCONNECTED 是否成立。
            if (state_ != State::DISCONNECTED)
            {
                // 赋值：把右侧 State::DISCONNECTED 的结果保存到左侧 state_。
                state_ = State::DISCONNECTED;
                // 赋值：把右侧 state_callback_ 的结果保存到左侧 state_cb。
                state_cb = state_callback_;
            }
            // 备用分支：前面的条件不成立时执行这里。
            else
            {
                // 赋值：把右侧 nullptr 的结果保存到左侧 state_cb。
                state_cb = nullptr;
            }
        }

        // 在锁外调用状态回调
        if (state_cb)
        {
            // 调用 state_cb(State::DISCONNECTED);，执行这个函数并处理它的返回值或状态变化。
            state_cb(State::DISCONNECTED);
        }

        // 调用 ESP_LOGD(TAG, "WebSocket 已断开连接");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGD(TAG, "WebSocket 已断开连接");
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 返回 false 给调用者，表示当前函数的执行结果。
    return false;
}

// 定义或声明函数 WebSocketMgr::send_text：参数在括号里，返回类型写在函数名前面。
int WebSocketMgr::send_text(const std::string& text, int timeout_ms)
{
    // 定义变量 esp_websocket_client_handle_t handle 并初始化为 nullptr。
    esp_websocket_client_handle_t handle = nullptr;

    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);

        // 判断条件：!initialized_ || client_handle_ == nullptr 是否成立。
        if (!initialized_ || client_handle_ == nullptr)
        {
            // 调用 ESP_LOGE(TAG, "WebSocket 客户端未初始化");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "WebSocket 客户端未初始化");
            // 返回 -1 给调用者，表示当前函数的执行结果。
            return -1;
        }

        // 判断条件：state_ != State::CONNECTED 是否成立。
        if (state_ != State::CONNECTED)
        {
            // 调用 ESP_LOGE(TAG, "WebSocket 未连接，无法发送数据");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "WebSocket 未连接，无法发送数据");
            // 返回 -1 给调用者，表示当前函数的执行结果。
            return -1;
        }

        // 赋值：把右侧 client_handle_ 的结果保存到左侧 handle。
        handle = client_handle_;
    }

    // 在锁外发送数据（避免长时间持锁）
    TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);
    // 定义变量 int sent 并初始化为 esp_websocket_client_send_text(handle, text.c_str(), static_cast<int>(text.length())。
    int sent = esp_websocket_client_send_text(handle, text.c_str(), static_cast<int>(text.length()),
                                              // 继续上一行的表达式或参数列表：timeout_ticks);。
                                              timeout_ticks);

    // 判断条件：sent < 0 是否成立。
    if (sent < 0)
    {
        // 调用 ESP_LOGE(TAG, "发送文本消息失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "发送文本消息失败");
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 调用 ESP_LOGD(TAG, "已发送文本消息，长度: %d", sent);，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGD(TAG, "已发送文本消息，长度: %d", sent);
    }

    // 返回 sent 给调用者，表示当前函数的执行结果。
    return sent;
}

// 定义或声明函数 WebSocketMgr::send_binary：参数在括号里，返回类型写在函数名前面。
int WebSocketMgr::send_binary(const uint8_t* data, size_t len, int timeout_ms)
{
    // 定义变量 esp_websocket_client_handle_t handle 并初始化为 nullptr。
    esp_websocket_client_handle_t handle = nullptr;

    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);

        // 判断条件：!initialized_ || client_handle_ == nullptr 是否成立。
        if (!initialized_ || client_handle_ == nullptr)
        {
            // 调用 ESP_LOGE(TAG, "WebSocket 客户端未初始化");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "WebSocket 客户端未初始化");
            // 返回 -1 给调用者，表示当前函数的执行结果。
            return -1;
        }

        // 判断条件：state_ != State::CONNECTED 是否成立。
        if (state_ != State::CONNECTED)
        {
            // 调用 ESP_LOGE(TAG, "WebSocket 未连接，无法发送数据");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "WebSocket 未连接，无法发送数据");
            // 返回 -1 给调用者，表示当前函数的执行结果。
            return -1;
        }

        // 赋值：把右侧 client_handle_ 的结果保存到左侧 handle。
        handle = client_handle_;
    }

    // 在锁外发送数据（避免长时间持锁，特别是大数据包）
    TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);
    // 定义变量 int sent 并初始化为 esp_websocket_client_send_bin(handle, reinterpret_cast<const char*>(data)。
    int sent = esp_websocket_client_send_bin(handle, reinterpret_cast<const char*>(data),
                                             // 继续上一行的表达式或参数列表：static_cast<int>(len), timeout_ticks);。
                                             static_cast<int>(len), timeout_ticks);

    // 判断条件：sent < 0 是否成立。
    if (sent < 0)
    {
        // 调用 ESP_LOGE(TAG, "发送二进制数据失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "发送二进制数据失败");
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 调用 ESP_LOGD(TAG, "已发送二进制数据，长度: %d", sent);，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGD(TAG, "已发送二进制数据，长度: %d", sent);
    }

    // 返回 sent 给调用者，表示当前函数的执行结果。
    return sent;
}

// 定义或声明函数 WebSocketMgr::get_state：参数在括号里，返回类型写在函数名前面。
State WebSocketMgr::get_state() const
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 返回 state_ 给调用者，表示当前函数的执行结果。
    return state_;
}

// 定义或声明函数 WebSocketMgr::is_connected：参数在括号里，返回类型写在函数名前面。
bool WebSocketMgr::is_connected() const
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 返回 state_ == State::CONNECTED 给调用者，表示当前函数的执行结果。
    return state_ == State::CONNECTED;
}

// 定义或声明函数 WebSocketMgr::is_init：参数在括号里，返回类型写在函数名前面。
bool WebSocketMgr::is_init() const
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 返回 initialized_ 给调用者，表示当前函数的执行结果。
    return initialized_;
}

// 定义或声明函数 WebSocketMgr::set_connected_callback：参数在括号里，返回类型写在函数名前面。
void WebSocketMgr::set_connected_callback(ConnectedCallback callback)
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 赋值：把右侧 callback 的结果保存到左侧 connected_callback_。
    connected_callback_ = callback;
}

// 定义或声明函数 WebSocketMgr::set_disconnected_callback：参数在括号里，返回类型写在函数名前面。
void WebSocketMgr::set_disconnected_callback(DisconnectedCallback callback)
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 赋值：把右侧 callback 的结果保存到左侧 disconnected_callback_。
    disconnected_callback_ = callback;
}

// 定义或声明函数 WebSocketMgr::set_data_callback：参数在括号里，返回类型写在函数名前面。
void WebSocketMgr::set_data_callback(DataCallback callback)
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 赋值：把右侧 callback 的结果保存到左侧 data_callback_。
    data_callback_ = callback;
}

// 定义或声明函数 WebSocketMgr::set_error_callback：参数在括号里，返回类型写在函数名前面。
void WebSocketMgr::set_error_callback(ErrorCallback callback)
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 赋值：把右侧 callback 的结果保存到左侧 error_callback_。
    error_callback_ = callback;
}

// 定义或声明函数 WebSocketMgr::set_state_callback：参数在括号里，返回类型写在函数名前面。
void WebSocketMgr::set_state_callback(StateCallback callback)
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 赋值：把右侧 callback 的结果保存到左侧 state_callback_。
    state_callback_ = callback;
}

// 函数调用：void WebSocketMgr::websocket_event_handler(void* handler_args, esp_event_base_t base,；执行该函数并处理它的返回值或副作用。
void WebSocketMgr::websocket_event_handler(void* handler_args, esp_event_base_t base,
                                           // 继续上一行的表达式或参数列表：int32_t event_id, void* event_data)。
                                           int32_t event_id, void* event_data)
{
    // 赋值：把右侧 static_cast<WebSocketMgr*>(handler_args) 的结果保存到左侧 auto* client。
    auto* client = static_cast<WebSocketMgr*>(handler_args);
    // 判断条件：client == nullptr 是否成立。
    if (client == nullptr)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 定义变量 esp_websocket_event_id_t ws_event_id 并初始化为 static_cast<esp_websocket_event_id_t>(event_id)。
    esp_websocket_event_id_t ws_event_id = static_cast<esp_websocket_event_id_t>(event_id);
    // 赋值：把右侧 static_cast<esp_websocket_event_data_t*>(event_data) 的结果保存到左侧 auto* event_data_ptr。
    auto* event_data_ptr = static_cast<esp_websocket_event_data_t*>(event_data);

    // 多分支判断：根据括号里的值选择不同 case。
    switch (ws_event_id)
    {
        // 分支匹配：case WEBSOCKET_EVENT_CONNECTED 成立时执行下面的代码。
        case WEBSOCKET_EVENT_CONNECTED:
            // 访问对象成员：client->handle_connected();；读取或修改这个对象里的成员状态。
            client->handle_connected();
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 分支匹配：case WEBSOCKET_EVENT_DISCONNECTED 成立时执行下面的代码。
        case WEBSOCKET_EVENT_DISCONNECTED:
            // 访问对象成员：client->handle_disconnected();；读取或修改这个对象里的成员状态。
            client->handle_disconnected();
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 分支匹配：case WEBSOCKET_EVENT_DATA 成立时执行下面的代码。
        case WEBSOCKET_EVENT_DATA:
            // 访问对象成员：client->handle_data(event_data_ptr);；读取或修改这个对象里的成员状态。
            client->handle_data(event_data_ptr);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 分支匹配：case WEBSOCKET_EVENT_ERROR 成立时执行下面的代码。
        case WEBSOCKET_EVENT_ERROR:
            // 调用 handle_error(event_data_ptr)：统一把状态置为失败并触发最终回调。集中处理错误可以让下载、校验和网络失败行为一致。。
            client->handle_error(event_data_ptr);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 分支匹配：case WEBSOCKET_EVENT_CLOSED 成立时执行下面的代码。
        case WEBSOCKET_EVENT_CLOSED:
            // 访问对象成员：client->handle_disconnected();；读取或修改这个对象里的成员状态。
            client->handle_disconnected();
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
}

// 定义或声明函数 WebSocketMgr::handle_connected：参数在括号里，返回类型写在函数名前面。
void WebSocketMgr::handle_connected()
{
    // 执行语句：ConnectedCallback cb;；它会更新一次变量、对象或程序状态。
    ConnectedCallback cb;
    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);
        // 赋值：把右侧 connected_callback_ 的结果保存到左侧 cb。
        cb = connected_callback_;
    }

    // 调用 ESP_LOGD(TAG, "WebSocket 已连接");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "WebSocket 已连接");

    // 调用 set_state(State::CONNECTED);，执行这个函数并处理它的返回值或状态变化。
    set_state(State::CONNECTED);

    // 判断条件：cb 是否成立。
    if (cb)
    {
        // 调用 cb();，执行这个函数并处理它的返回值或状态变化。
        cb();
    }
}

// 定义或声明函数 WebSocketMgr::handle_disconnected：参数在括号里，返回类型写在函数名前面。
void WebSocketMgr::handle_disconnected()
{
    // 执行语句：DisconnectedCallback cb;；它会更新一次变量、对象或程序状态。
    DisconnectedCallback cb;
    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);
        // 赋值：把右侧 disconnected_callback_ 的结果保存到左侧 cb。
        cb = disconnected_callback_;
    }

    // 调用 ESP_LOGD(TAG, "WebSocket 已断开连接");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "WebSocket 已断开连接");

    // 调用 set_state(State::DISCONNECTED);，执行这个函数并处理它的返回值或状态变化。
    set_state(State::DISCONNECTED);

    // 判断条件：cb 是否成立。
    if (cb)
    {
        // 调用 cb();，执行这个函数并处理它的返回值或状态变化。
        cb();
    }
}

// 定义或声明函数 WebSocketMgr::handle_data：参数在括号里，返回类型写在函数名前面。
void WebSocketMgr::handle_data(esp_websocket_event_data_t* event_data)
{
    // 判断条件：event_data == nullptr 是否成立。
    if (event_data == nullptr)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 跳过长度为0的数据
    if (event_data->data_len == 0)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 执行语句：DataCallback cb;；它会更新一次变量、对象或程序状态。
    DataCallback cb;
    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);
        // 赋值：把右侧 data_callback_ 的结果保存到左侧 cb。
        cb = data_callback_;
    }

    // 判断条件：cb 是否成立。
    if (cb)
    {
        // 执行语句：DataEvent event;；它会更新一次变量、对象或程序状态。
        DataEvent event;
        // 赋值：把右侧 reinterpret_cast<const uint8_t*>(event_data->data_ptr) 的结果保存到左侧 event.data。
        event.data = reinterpret_cast<const uint8_t*>(event_data->data_ptr);
        // 赋值：把右侧 static_cast<size_t>(event_data->data_len) 的结果保存到左侧 event.length。
        event.length = static_cast<size_t>(event_data->data_len);
        // 赋值：把右侧 (event_data->op_code == 0x1) 的结果保存到左侧 event.is_text。
        event.is_text = (event_data->op_code == 0x1); // 文本帧 opcode = 0x1

        // 调用 ESP_LOGD(TAG, "recv len=%u %s", static_cast<unsigned>(event.length),，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGD(TAG, "recv len=%u %s", static_cast<unsigned>(event.length),
                 // 继续上一行的表达式或参数列表：event.is_text ? "text" : "bin");。
                 event.is_text ? "text" : "bin");

        // 调用 cb(event);，执行这个函数并处理它的返回值或状态变化。
        cb(event);
    }
}

// 函数定义/声明：调用 WebSocketMgr::handle_error()：统一把状态置为失败并触发最终回调。集中处理错误可以让下载、校验和网络失败行为一致。。
void WebSocketMgr::handle_error(esp_websocket_event_data_t* event_data)
{
    // 判断条件：event_data == nullptr 是否成立。
    if (event_data == nullptr)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 执行语句：ErrorCallback cb;；它会更新一次变量、对象或程序状态。
    ErrorCallback cb;
    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);
        // 赋值：把右侧 error_callback_ 的结果保存到左侧 cb。
        cb = error_callback_;
    }

    // 判断条件：cb 是否成立。
    if (cb)
    {
        // 执行语句：ErrorEvent event;；它会更新一次变量、对象或程序状态。
        ErrorEvent event;
        // 赋值：把右侧 event_data->error_handle.esp_tls_last_esp_err 的结果保存到左侧 event.esp_err_code。
        event.esp_err_code = event_data->error_handle.esp_tls_last_esp_err;
        // 赋值：把右侧 event_data->error_handle.error_type 的结果保存到左侧 event.error_type。
        event.error_type = event_data->error_handle.error_type;
        // 赋值：把右侧 event_data->error_handle.esp_ws_handshake_status_code 的结果保存到左侧 event.handshake_status。
        event.handshake_status = event_data->error_handle.esp_ws_handshake_status_code;

        // 生成错误消息
        char msg[128];
        // 调用 snprintf(msg, sizeof(msg), "WebSocket 错误: type=%d, esp_err=0x%x, handshake=%d",，执行这个函数并处理它的返回值或状态变化。
        snprintf(msg, sizeof(msg), "WebSocket 错误: type=%d, esp_err=0x%x, handshake=%d",
                 // 继续上一行的表达式或参数列表：event.error_type, event.esp_err_code, event.handshake_status);。
                 event.error_type, event.esp_err_code, event.handshake_status);
        // 赋值：把右侧 msg 的结果保存到左侧 event.message。
        event.message = msg;

        // 调用 ESP_LOGE(TAG, "%s", event.message.c_str());，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "%s", event.message.c_str());

        // 调用 cb(event);，执行这个函数并处理它的返回值或状态变化。
        cb(event);
    }
}

// 定义或声明函数 WebSocketMgr::set_state：参数在括号里，返回类型写在函数名前面。
void WebSocketMgr::set_state(State new_state)
{
    // 执行语句：StateCallback state_cb;；它会更新一次变量、对象或程序状态。
    StateCallback state_cb;
    // 定义变量 bool state_changed 并初始化为 false。
    bool state_changed = false;

    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);
        // 判断条件：state_ != new_state 是否成立。
        if (state_ != new_state)
        {
            // 赋值：把右侧 new_state 的结果保存到左侧 state_。
            state_ = new_state;
            // 赋值：把右侧 true 的结果保存到左侧 state_changed。
            state_changed = true;
            // 赋值：把右侧 state_callback_ 的结果保存到左侧 state_cb。
            state_cb = state_callback_;
        }
    }

    // 判断条件：state_changed && state_cb 是否成立。
    if (state_changed && state_cb)
    {
        // 调用 state_cb(new_state);，执行这个函数并处理它的返回值或状态变化。
        state_cb(new_state);
    }
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::protocol::websocket
