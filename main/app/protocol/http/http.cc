/**
 * [初学者导读] 文件：main/app/protocol/http/http.cc
 * 作用：实现 HTTP 请求、响应缓冲和流式下载。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：网络请求可能阻塞，必须在后台任务或专门任务中执行。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "http.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_crt_bundle.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_err.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_http_client.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_log.h"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "HTTP";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::protocol::http {

// 定义或声明函数 HttpMgr::get_instance：参数在括号里，返回类型写在函数名前面。
HttpMgr& HttpMgr::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static HttpMgr instance;
    // 返回 instance 给调用者，表示当前函数的执行结果。
    return instance;
}

// 定义或声明函数 HttpMgr::init：参数在括号里，返回类型写在函数名前面。
bool HttpMgr::init()
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 判断条件：initialized_ 是否成立。
    if (initialized_)
    {
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 赋值：把右侧 nullptr 的结果保存到左侧 current_client_handle_。
    current_client_handle_ = nullptr;
    // 赋值：把右侧 nullptr 的结果保存到左侧 current_response_。
    current_response_ = nullptr;
    // 赋值：把右侧 nullptr 的结果保存到左侧 current_response_callback_。
    current_response_callback_ = nullptr;
    // 赋值：把右侧 nullptr 的结果保存到左侧 current_data_callback_。
    current_data_callback_ = nullptr;
    // 赋值：把右侧 true 的结果保存到左侧 initialized_。
    initialized_ = true;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 HttpMgr::~HttpMgr：参数在括号里，返回类型写在函数名前面。
HttpMgr::~HttpMgr()
{
    // 判断条件：initialized_ 是否成立。
    if (initialized_)
    {
        // 调用 deinit()：创建和销毁队列。
        deinit();
    }
}

// 定义或声明函数 HttpMgr::deinit：参数在括号里，返回类型写在函数名前面。
void HttpMgr::deinit()
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 判断条件：!initialized_ 是否成立。
    if (!initialized_)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 赋值：把右侧 false 的结果保存到左侧 initialized_。
    initialized_ = false;
    // 赋值：把右侧 nullptr 的结果保存到左侧 current_client_handle_。
    current_client_handle_ = nullptr;
    // 赋值：把右侧 nullptr 的结果保存到左侧 current_response_。
    current_response_ = nullptr;
    // 赋值：把右侧 nullptr 的结果保存到左侧 current_response_callback_。
    current_response_callback_ = nullptr;
    // 赋值：把右侧 nullptr 的结果保存到左侧 current_data_callback_。
    current_data_callback_ = nullptr;
}

// 定义或声明函数 HttpMgr::http_event_handler：参数在括号里，返回类型写在函数名前面。
esp_err_t HttpMgr::http_event_handler(esp_http_client_event_t* evt)
{
    // 赋值：把右侧 get_instance() 的结果保存到左侧 auto& instance。
    auto& instance = get_instance();

    // 多分支判断：根据括号里的值选择不同 case。
    switch (evt->event_id)
    {
        // 分支匹配：case HTTP_EVENT_ERROR 成立时执行下面的代码。
        case HTTP_EVENT_ERROR:
            // 调用 ESP_LOGE(TAG, "HTTP 事件错误");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "HTTP 事件错误");
            // 判断条件：instance.current_response_ 是否成立。
            if (instance.current_response_)
            {
                // 赋值：把右侧 HttpStatus::UNKNOWN 的结果保存到左侧 instance.current_response_->status_code。
                instance.current_response_->status_code = HttpStatus::UNKNOWN;
            }
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 分支匹配：case HTTP_EVENT_ON_HEADER 成立时执行下面的代码。
        case HTTP_EVENT_ON_HEADER:
            // 判断条件：instance.current_response_ && evt->header_key && evt->header_value 是否成立。
            if (instance.current_response_ && evt->header_key && evt->header_value)
            {
                // 赋值：把右侧 evt->header_value 的结果保存到左侧 instance.current_response_->headers[evt->header_key]。
                instance.current_response_->headers[evt->header_key] = evt->header_value;
            }
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 分支匹配：case HTTP_EVENT_ON_DATA 成立时执行下面的代码。
        case HTTP_EVENT_ON_DATA:
            // 判断条件：instance.current_data_callback_ 是否成立。
            if (instance.current_data_callback_)
            {
                // 判断条件：!instance.current_data_callback_(static_cast<const uint8_t*>(evt->data 是否成立。
                if (!instance.current_data_callback_(static_cast<const uint8_t*>(evt->data),
                                                     // 定义或声明函数 static_cast<size_t>：参数在括号里，返回类型写在函数名前面。
                                                     static_cast<size_t>(evt->data_len)))
                {
                    // 返回 ESP_FAIL 给调用者，表示当前函数的执行结果。
                    return ESP_FAIL;
                }
            }

            // 判断条件：instance.current_response_ 是否成立。
            if (instance.current_response_)
            {
                // 定义变量 size_t data_len 并初始化为 static_cast<size_t>(evt->data_len)。
                size_t data_len = static_cast<size_t>(evt->data_len);
                // 函数调用：instance.current_response_->body.insert(；执行该函数并处理它的返回值或副作用。
                instance.current_response_->body.insert(
                    // 继续上一行的表达式或参数列表：instance.current_response_->body.end(), static_cast<const uint8_t*>(evt->data),。
                    instance.current_response_->body.end(), static_cast<const uint8_t*>(evt->data),
                    // 继续上一行的表达式或参数列表：static_cast<const uint8_t*>(evt->data) + data_len);。
                    static_cast<const uint8_t*>(evt->data) + data_len);
            }
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }

    // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
    return ESP_OK;
}

// 定义或声明函数 HttpMgr::convert_method：参数在括号里，返回类型写在函数名前面。
esp_http_client_method_t HttpMgr::convert_method(HttpMethod method)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (method)
    {
        // 分支匹配：case HttpMethod::GET 成立时执行下面的代码。
        case HttpMethod::GET:
            // 返回 HTTP_METHOD_GET 给调用者，表示当前函数的执行结果。
            return HTTP_METHOD_GET;
        // 分支匹配：case HttpMethod::POST 成立时执行下面的代码。
        case HttpMethod::POST:
            // 返回 HTTP_METHOD_POST 给调用者，表示当前函数的执行结果。
            return HTTP_METHOD_POST;
        // 分支匹配：case HttpMethod::PUT 成立时执行下面的代码。
        case HttpMethod::PUT:
            // 返回 HTTP_METHOD_PUT 给调用者，表示当前函数的执行结果。
            return HTTP_METHOD_PUT;
        // 分支匹配：case HttpMethod::DELETE 成立时执行下面的代码。
        case HttpMethod::DELETE:
            // 返回 HTTP_METHOD_DELETE 给调用者，表示当前函数的执行结果。
            return HTTP_METHOD_DELETE;
        // 分支匹配：case HttpMethod::PATCH 成立时执行下面的代码。
        case HttpMethod::PATCH:
            // 返回 HTTP_METHOD_PATCH 给调用者，表示当前函数的执行结果。
            return HTTP_METHOD_PATCH;
        // 分支匹配：case HttpMethod::HEAD 成立时执行下面的代码。
        case HttpMethod::HEAD:
            // 返回 HTTP_METHOD_HEAD 给调用者，表示当前函数的执行结果。
            return HTTP_METHOD_HEAD;
        // 分支匹配：case HttpMethod::OPTIONS 成立时执行下面的代码。
        case HttpMethod::OPTIONS:
            // 返回 HTTP_METHOD_OPTIONS 给调用者，表示当前函数的执行结果。
            return HTTP_METHOD_OPTIONS;
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 返回 HTTP_METHOD_GET 给调用者，表示当前函数的执行结果。
            return HTTP_METHOD_GET;
    }
}

// 定义或声明函数 HttpMgr::convert_status_code：参数在括号里，返回类型写在函数名前面。
HttpStatus HttpMgr::convert_status_code(int32_t status_code_int)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (status_code_int)
    {
        // 分支匹配：case 200 成立时执行下面的代码。
        case 200:
            // 返回 HttpStatus::OK 给调用者，表示当前函数的执行结果。
            return HttpStatus::OK;
        // 分支匹配：case 201 成立时执行下面的代码。
        case 201:
            // 返回 HttpStatus::CREATED 给调用者，表示当前函数的执行结果。
            return HttpStatus::CREATED;
        // 分支匹配：case 204 成立时执行下面的代码。
        case 204:
            // 返回 HttpStatus::NO_CONTENT 给调用者，表示当前函数的执行结果。
            return HttpStatus::NO_CONTENT;
        // 分支匹配：case 400 成立时执行下面的代码。
        case 400:
            // 返回 HttpStatus::BAD_REQUEST 给调用者，表示当前函数的执行结果。
            return HttpStatus::BAD_REQUEST;
        // 分支匹配：case 401 成立时执行下面的代码。
        case 401:
            // 返回 HttpStatus::UNAUTHORIZED 给调用者，表示当前函数的执行结果。
            return HttpStatus::UNAUTHORIZED;
        // 分支匹配：case 403 成立时执行下面的代码。
        case 403:
            // 返回 HttpStatus::FORBIDDEN 给调用者，表示当前函数的执行结果。
            return HttpStatus::FORBIDDEN;
        // 分支匹配：case 404 成立时执行下面的代码。
        case 404:
            // 返回 HttpStatus::NOT_FOUND 给调用者，表示当前函数的执行结果。
            return HttpStatus::NOT_FOUND;
        // 分支匹配：case 500 成立时执行下面的代码。
        case 500:
            // 返回 HttpStatus::INTERNAL_ERROR 给调用者，表示当前函数的执行结果。
            return HttpStatus::INTERNAL_ERROR;
        // 分支匹配：case 502 成立时执行下面的代码。
        case 502:
            // 返回 HttpStatus::BAD_GATEWAY 给调用者，表示当前函数的执行结果。
            return HttpStatus::BAD_GATEWAY;
        // 分支匹配：case 503 成立时执行下面的代码。
        case 503:
            // 返回 HttpStatus::SERVICE_UNAVAILABLE 给调用者，表示当前函数的执行结果。
            return HttpStatus::SERVICE_UNAVAILABLE;
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 判断条件：status_code_int >= 200 && status_code_int < 300 是否成立。
            if (status_code_int >= 200 && status_code_int < 300)
            {
                // 返回 HttpStatus::OK 给调用者，表示当前函数的执行结果。
                return HttpStatus::OK;
            }
            // 返回 HttpStatus::UNKNOWN 给调用者，表示当前函数的执行结果。
            return HttpStatus::UNKNOWN;
    }
}

// 定义或声明函数 HttpMgr::configure_client：参数在括号里，返回类型写在函数名前面。
void HttpMgr::configure_client(const HttpRequest& request, esp_http_client_config_t& config)
{
    // 赋值：把右侧 request.url.c_str() 的结果保存到左侧 config.url。
    config.url = request.url.c_str();
    // 赋值：把右侧 http_event_handler 的结果保存到左侧 config.event_handler。
    config.event_handler = http_event_handler;
    // 赋值：把右侧 request.timeout_ms 的结果保存到左侧 config.timeout_ms。
    config.timeout_ms = request.timeout_ms;
    // 赋值：把右侧 request.skip_cert_common_name_check 的结果保存到左侧 config.skip_cert_common_name_check。
    config.skip_cert_common_name_check = request.skip_cert_common_name_check;
    // 赋值：把右侧 request.cert_pem 的结果保存到左侧 config.cert_pem。
    config.cert_pem = request.cert_pem;

    // HTTPS 必须配置服务端校验，否则 esp-tls 报 ESP_ERR_MBEDTLS_SSL_SETUP_FAILED
    if (request.url.size() >= 8 && request.url.compare(0, 8, "https://") == 0)
    {
        // 判断条件：request.cert_pem != nullptr 是否成立。
        if (request.cert_pem != nullptr)
        {
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }
        // 赋值：把右侧 esp_crt_bundle_attach 的结果保存到左侧 config.crt_bundle_attach。
        config.crt_bundle_attach = esp_crt_bundle_attach;
    }
}

// 定义或声明函数 HttpMgr::setup_client：参数在括号里，返回类型写在函数名前面。
esp_err_t HttpMgr::setup_client(esp_http_client_handle_t client, const HttpRequest& request)
{
    // 定义变量 esp_http_client_method_t method 并初始化为 convert_method(request.method)。
    esp_http_client_method_t method = convert_method(request.method);
    // 定义变量 esp_err_t ret 并初始化为 esp_http_client_set_method(client, method)。
    esp_err_t ret = esp_http_client_set_method(client, method);
    // 判断条件：ret != ESP_OK 是否成立。
    if (ret != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "设置 HTTP 方法失败: %s", esp_err_to_name(ret));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "设置 HTTP 方法失败: %s", esp_err_to_name(ret));
        // 返回 ret 给调用者，表示当前函数的执行结果。
        return ret;
    }

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (const auto& header : request.headers)
    {
        // 调用 esp_http_client_set_header(client, header.first.c_str(), header.second.c_str());，执行这个函数并处理它的返回值或状态变化。
        esp_http_client_set_header(client, header.first.c_str(), header.second.c_str());
    }

    // 判断条件：!request.body.empty() 是否成立。
    if (!request.body.empty())
    {
        // 函数调用：ret = esp_http_client_set_post_field(client, (const char*)request.body.data(),；执行该函数并处理它的返回值或副作用。
        ret = esp_http_client_set_post_field(client, (const char*)request.body.data(),
                                             // 继续上一行的表达式或参数列表：request.body.size());。
                                             request.body.size());
        // 判断条件：ret != ESP_OK 是否成立。
        if (ret != ESP_OK)
        {
            // 调用 ESP_LOGE(TAG, "设置请求体失败: %s", esp_err_to_name(ret));，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "设置请求体失败: %s", esp_err_to_name(ret));
            // 返回 ret 给调用者，表示当前函数的执行结果。
            return ret;
        }
    }

    // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
    return ESP_OK;
}

// 函数调用：bool HttpMgr::perform_internal(const HttpRequest& request, HttpResponse* response,；执行该函数并处理它的返回值或副作用。
bool HttpMgr::perform_internal(const HttpRequest& request, HttpResponse* response,
                               // 只读约束：这个值或对象在当前位置不能被修改。
                               const ResponseCallback& response_callback,
                               // 只读约束：这个值或对象在当前位置不能被修改。
                               const DataCallback& data_callback)
{
    // 判断条件：request.url.empty() 是否成立。
    if (request.url.empty())
    {
        // 调用 ESP_LOGE(TAG, "URL 不能为空");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "URL 不能为空");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 判断条件：response == nullptr && !response_callback 是否成立。
    if (response == nullptr && !response_callback)
    {
        // 调用 ESP_LOGE(TAG, "必须提供 response 或 response_callback");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "必须提供 response 或 response_callback");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);
        // 判断条件：!initialized_ 是否成立。
        if (!initialized_)
        {
            // 调用 ESP_LOGE(TAG, "HTTP 客户端未初始化");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "HTTP 客户端未初始化");
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }
    }

    // 判断条件：response 是否成立。
    if (response)
    {
        // 赋值：把右侧 HttpStatus::UNKNOWN 的结果保存到左侧 response->status_code。
        response->status_code = HttpStatus::UNKNOWN;
        // 赋值：把右侧 0 的结果保存到左侧 response->status_code_int。
        response->status_code_int = 0;
        // 调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
        response->headers.clear();
        // 调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
        response->body.clear();
        // 赋值：把右侧 0 的结果保存到左侧 response->content_length。
        response->content_length = 0;
    }

    // 定义变量 esp_http_client_config_t config 并初始化为 {}。
    esp_http_client_config_t config = {};
    // 调用 configure_client(request, config);，执行这个函数并处理它的返回值或状态变化。
    configure_client(request, config);

    // 定义变量 esp_http_client_handle_t client 并初始化为 esp_http_client_init(&config)。
    esp_http_client_handle_t client = esp_http_client_init(&config);
    // 判断条件：client == nullptr 是否成立。
    if (client == nullptr)
    {
        // 调用 ESP_LOGE(TAG, "HTTP 客户端初始化失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "HTTP 客户端初始化失败");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 定义变量 esp_err_t ret 并初始化为 setup_client(client, request)。
    esp_err_t ret = setup_client(client, request);
    // 判断条件：ret != ESP_OK 是否成立。
    if (ret != ESP_OK)
    {
        // 调用 esp_http_client_cleanup(client);，执行这个函数并处理它的返回值或状态变化。
        esp_http_client_cleanup(client);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);
        // 赋值：把右侧 client 的结果保存到左侧 current_client_handle_。
        current_client_handle_ = client;
        // 赋值：把右侧 response 的结果保存到左侧 current_response_。
        current_response_ = response;
        // 赋值：把右侧 response_callback 的结果保存到左侧 current_response_callback_。
        current_response_callback_ = response_callback;
        // 赋值：把右侧 data_callback 的结果保存到左侧 current_data_callback_。
        current_data_callback_ = data_callback;
    }

    // 赋值：把右侧 esp_http_client_perform(client) 的结果保存到左侧 ret。
    ret = esp_http_client_perform(client);
    // 定义变量 bool success 并初始化为 (ret == ESP_OK)。
    bool success = (ret == ESP_OK);

    // 定义变量 int32_t status_code_int 并初始化为 0。
    int32_t status_code_int = 0;
    // 定义变量 int32_t content_length 并初始化为 0。
    int32_t content_length = 0;
    // 判断条件：success 是否成立。
    if (success)
    {
        // 赋值：把右侧 esp_http_client_get_status_code(client) 的结果保存到左侧 status_code_int。
        status_code_int = esp_http_client_get_status_code(client);
        // 赋值：把右侧 esp_http_client_get_content_length(client) 的结果保存到左侧 content_length。
        content_length = esp_http_client_get_content_length(client);
    }

    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);
        // 赋值：把右侧 nullptr 的结果保存到左侧 current_client_handle_。
        current_client_handle_ = nullptr;
        // 赋值：把右侧 nullptr 的结果保存到左侧 current_response_。
        current_response_ = nullptr;
        // 赋值：把右侧 nullptr 的结果保存到左侧 current_response_callback_。
        current_response_callback_ = nullptr;
        // 赋值：把右侧 nullptr 的结果保存到左侧 current_data_callback_。
        current_data_callback_ = nullptr;
    }

    // 判断条件：success 是否成立。
    if (success)
    {
        // 定义变量 HttpStatus status_code 并初始化为 convert_status_code(status_code_int)。
        HttpStatus status_code = convert_status_code(status_code_int);

        // 判断条件：response 是否成立。
        if (response)
        {
            // 赋值：把右侧 status_code 的结果保存到左侧 response->status_code。
            response->status_code = status_code;
            // 赋值：把右侧 status_code_int 的结果保存到左侧 response->status_code_int。
            response->status_code_int = status_code_int;
            // 赋值：把右侧 content_length 的结果保存到左侧 response->content_length。
            response->content_length = content_length;
        }

        // 判断条件：response_callback 是否成立。
        if (response_callback)
        {
            // 执行语句：HttpResponse callback_response;；它会更新一次变量、对象或程序状态。
            HttpResponse callback_response;
            // 赋值：把右侧 status_code 的结果保存到左侧 callback_response.status_code。
            callback_response.status_code = status_code;
            // 赋值：把右侧 status_code_int 的结果保存到左侧 callback_response.status_code_int。
            callback_response.status_code_int = status_code_int;
            // 赋值：把右侧 content_length 的结果保存到左侧 callback_response.content_length。
            callback_response.content_length = content_length;

            // 判断条件：!response_callback(callback_response) 是否成立。
            if (!response_callback(callback_response))
            {
                // 赋值：把右侧 false 的结果保存到左侧 success。
                success = false;
            }
        }
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 调用 ESP_LOGE(TAG, "HTTP 请求失败: %s, 错误: %s", request.url.c_str(), esp_err_to_name(ret));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "HTTP 请求失败: %s, 错误: %s", request.url.c_str(), esp_err_to_name(ret));
    }

    // 调用 esp_http_client_cleanup(client);，执行这个函数并处理它的返回值或状态变化。
    esp_http_client_cleanup(client);
    // 返回 success 给调用者，表示当前函数的执行结果。
    return success;
}

// 定义或声明函数 HttpMgr::perform：参数在括号里，返回类型写在函数名前面。
bool HttpMgr::perform(const HttpRequest& request, HttpResponse& response)
{
    // 返回 perform_internal(request, &response, nullptr, nullptr) 给调用者，表示当前函数的执行结果。
    return perform_internal(request, &response, nullptr, nullptr);
}

// 函数调用：bool HttpMgr::perform(const HttpRequest& request, const ResponseCallback& response_callback,；执行该函数并处理它的返回值或副作用。
bool HttpMgr::perform(const HttpRequest& request, const ResponseCallback& response_callback,
                      // 只读约束：这个值或对象在当前位置不能被修改。
                      const DataCallback& data_callback)
{
    // 返回 perform_internal(request, nullptr, response_callback, data_callback) 给调用者，表示当前函数的执行结果。
    return perform_internal(request, nullptr, response_callback, data_callback);
}

// 函数定义/声明：调用 HttpMgr::get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
bool HttpMgr::get(const std::string& url, HttpResponse& response, int32_t timeout_ms)
{
    // 执行语句：HttpRequest request;；它会更新一次变量、对象或程序状态。
    HttpRequest request;
    // 赋值：把右侧 url 的结果保存到左侧 request.url。
    request.url = url;
    // 赋值：把右侧 HttpMethod::GET 的结果保存到左侧 request.method。
    request.method = HttpMethod::GET;
    // 赋值：把右侧 timeout_ms 的结果保存到左侧 request.timeout_ms。
    request.timeout_ms = timeout_ms;

    // 返回 perform(request, response) 给调用者，表示当前函数的执行结果。
    return perform(request, response);
}

// 函数调用：bool HttpMgr::post(const std::string& url, const std::vector<uint8_t>& body, HttpResponse& response,；执行该函数并处理它的返回值或副作用。
bool HttpMgr::post(const std::string& url, const std::vector<uint8_t>& body, HttpResponse& response,
                   // 继续上一行的表达式或参数列表：int32_t timeout_ms)。
                   int32_t timeout_ms)
{
    // 执行语句：HttpRequest request;；它会更新一次变量、对象或程序状态。
    HttpRequest request;
    // 赋值：把右侧 url 的结果保存到左侧 request.url。
    request.url = url;
    // 赋值：把右侧 HttpMethod::POST 的结果保存到左侧 request.method。
    request.method = HttpMethod::POST;
    // 赋值：把右侧 body 的结果保存到左侧 request.body。
    request.body = body;
    // 赋值：把右侧 timeout_ms 的结果保存到左侧 request.timeout_ms。
    request.timeout_ms = timeout_ms;
    // 赋值：把右侧 "application/octet-stream" 的结果保存到左侧 request.headers["Content-Type"]。
    request.headers["Content-Type"] = "application/octet-stream";
    // 返回 perform(request, response) 给调用者，表示当前函数的执行结果。
    return perform(request, response);
}

// 函数调用：bool HttpMgr::post(const std::string& url, const std::string& body, HttpResponse& response,；执行该函数并处理它的返回值或副作用。
bool HttpMgr::post(const std::string& url, const std::string& body, HttpResponse& response,
                   // 继续上一行的表达式或参数列表：int32_t timeout_ms)。
                   int32_t timeout_ms)
{
    // 执行语句：HttpRequest request;；它会更新一次变量、对象或程序状态。
    HttpRequest request;
    // 赋值：把右侧 url 的结果保存到左侧 request.url。
    request.url = url;
    // 赋值：把右侧 HttpMethod::POST 的结果保存到左侧 request.method。
    request.method = HttpMethod::POST;
    // 赋值：把右侧 timeout_ms 的结果保存到左侧 request.timeout_ms。
    request.timeout_ms = timeout_ms;
    // 访问对象成员：request.body.assign(body.begin(), body.end());；读取或修改这个对象里的成员状态。
    request.body.assign(body.begin(), body.end());
    // 赋值：把右侧 "application/json" 的结果保存到左侧 request.headers["Content-Type"]。
    request.headers["Content-Type"] = "application/json";
    // 返回 perform(request, response) 给调用者，表示当前函数的执行结果。
    return perform(request, response);
}

// 定义或声明函数 HttpMgr::is_init：参数在括号里，返回类型写在函数名前面。
bool HttpMgr::is_init() const
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 返回 initialized_ 给调用者，表示当前函数的执行结果。
    return initialized_;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::protocol::http
