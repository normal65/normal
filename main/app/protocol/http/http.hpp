// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/protocol/http/http.hpp
 * 作用：实现 HTTP 请求、响应缓冲和流式下载。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：网络请求可能阻塞，必须在后台任务或专门任务中执行。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <functional>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <map>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <mutex>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <string>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <vector>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_err.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_http_client.h"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::protocol::http {

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class HttpMethod
{
    // 执行这条代码：GET,。它会更新变量、对象状态或调用链。
    GET,
    // 继续上一行的表达式或参数列表：POST,。
    POST,
    // 继续上一行的表达式或参数列表：PUT,。
    PUT,
    // 继续上一行的表达式或参数列表：DELETE,。
    DELETE,
    // 继续上一行的表达式或参数列表：PATCH,。
    PATCH,
    // 继续上一行的表达式或参数列表：HEAD,。
    HEAD,
    // 继续上一行的表达式或参数列表：OPTIONS。
    OPTIONS
};

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class HttpStatus
{
    // 执行这条代码：OK = 200,。它会更新变量、对象状态或调用链。
    OK = 200,
    // 继续上一行的表达式或参数列表：CREATED = 201,。
    CREATED = 201,
    // 继续上一行的表达式或参数列表：NO_CONTENT = 204,。
    NO_CONTENT = 204,
    // 继续上一行的表达式或参数列表：BAD_REQUEST = 400,。
    BAD_REQUEST = 400,
    // 继续上一行的表达式或参数列表：UNAUTHORIZED = 401,。
    UNAUTHORIZED = 401,
    // 继续上一行的表达式或参数列表：FORBIDDEN = 403,。
    FORBIDDEN = 403,
    // 继续上一行的表达式或参数列表：NOT_FOUND = 404,。
    NOT_FOUND = 404,
    // 继续上一行的表达式或参数列表：INTERNAL_ERROR = 500,。
    INTERNAL_ERROR = 500,
    // 继续上一行的表达式或参数列表：BAD_GATEWAY = 502,。
    BAD_GATEWAY = 502,
    // 继续上一行的表达式或参数列表：SERVICE_UNAVAILABLE = 503,。
    SERVICE_UNAVAILABLE = 503,
    // 继续上一行的表达式或参数列表：UNKNOWN = 0。
    UNKNOWN = 0
};

// struct HttpRequest：把相关数据字段组合在一起，表示项目中的一个概念。
struct HttpRequest
{
    // 执行语句：std::string url;；它会更新一次变量、对象或程序状态。
    std::string url;
    // 定义变量 HttpMethod method 并初始化为 HttpMethod::GET。
    HttpMethod method = HttpMethod::GET;
    // 执行语句：std::map<std::string, std::string> headers;；它会更新一次变量、对象或程序状态。
    std::map<std::string, std::string> headers;
    // 执行语句：std::vector<uint8_t> body;；它会更新一次变量、对象或程序状态。
    std::vector<uint8_t> body;
    // 定义变量 int32_t timeout_ms 并初始化为 5000。
    int32_t timeout_ms = 5000;
    // 定义变量 bool skip_cert_common_name_check 并初始化为 false。
    bool skip_cert_common_name_check = false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* cert_pem = nullptr;
};

// struct HttpResponse：把相关数据字段组合在一起，表示项目中的一个概念。
struct HttpResponse
{
    // 定义变量 HttpStatus status_code 并初始化为 HttpStatus::UNKNOWN。
    HttpStatus status_code = HttpStatus::UNKNOWN;
    // 定义变量 int32_t status_code_int 并初始化为 0。
    int32_t status_code_int = 0;
    // 执行语句：std::map<std::string, std::string> headers;；它会更新一次变量、对象或程序状态。
    std::map<std::string, std::string> headers;
    // 执行语句：std::vector<uint8_t> body;；它会更新一次变量、对象或程序状态。
    std::vector<uint8_t> body;
    // 定义变量 int32_t content_length 并初始化为 0。
    int32_t content_length = 0;
};

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ResponseCallback = std::function<bool(const HttpResponse& response)>;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using DataCallback = std::function<bool(const uint8_t* data, size_t len)>;

/** @brief esp_http_client 封装（同步 perform + HTTPS crt_bundle） */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class HttpMgr
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static HttpMgr& get_instance();

    // 函数调用：bool init();；执行该函数并处理它的返回值或副作用。
    bool init();
    // 调用 deinit()：创建和销毁队列。
    void deinit();

    // 函数调用：bool perform(const HttpRequest& request, HttpResponse& response);；执行该函数并处理它的返回值或副作用。
    bool perform(const HttpRequest& request, HttpResponse& response);
    // 函数调用：bool perform(const HttpRequest& request, const ResponseCallback& response_callback,；执行该函数并处理它的返回值或副作用。
    bool perform(const HttpRequest& request, const ResponseCallback& response_callback,
                 // 只读约束：这个值或对象在当前位置不能被修改。
                 const DataCallback& data_callback = nullptr);

    // 赋值：把右侧 5000) 的结果保存到左侧 bool get(const std::string& url, HttpResponse& response, int32_t timeout_ms。
    bool get(const std::string& url, HttpResponse& response, int32_t timeout_ms = 5000);
    // 函数调用：bool post(const std::string& url, const std::vector<uint8_t>& body, HttpResponse& response,；执行该函数并处理它的返回值或副作用。
    bool post(const std::string& url, const std::vector<uint8_t>& body, HttpResponse& response,
              // 继续上一行的表达式或参数列表：int32_t timeout_ms = 5000);。
              int32_t timeout_ms = 5000);
    // 函数调用：bool post(const std::string& url, const std::string& body, HttpResponse& response,；执行该函数并处理它的返回值或副作用。
    bool post(const std::string& url, const std::string& body, HttpResponse& response,
              // 继续上一行的表达式或参数列表：int32_t timeout_ms = 5000);。
              int32_t timeout_ms = 5000);

    // 函数调用：bool is_init() const;；执行该函数并处理它的返回值或副作用。
    bool is_init() const;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 函数调用：bool perform_internal(const HttpRequest& request, HttpResponse* response,；执行该函数并处理它的返回值或副作用。
    bool perform_internal(const HttpRequest& request, HttpResponse* response,
                          // 只读约束：这个值或对象在当前位置不能被修改。
                          const ResponseCallback& response_callback,
                          // 只读约束：这个值或对象在当前位置不能被修改。
                          const DataCallback& data_callback);

    // 静态函数 convert_method：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static esp_http_client_method_t convert_method(HttpMethod method);
    // 静态函数 convert_status_code：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static HttpStatus convert_status_code(int32_t status_code_int);
    // 静态函数 configure_client：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void configure_client(const HttpRequest& request, esp_http_client_config_t& config);
    // 静态函数 setup_client：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static esp_err_t setup_client(esp_http_client_handle_t client, const HttpRequest& request);
    // 静态函数 http_event_handler：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static esp_err_t http_event_handler(esp_http_client_event_t* evt);

    // 调用 HttpMgr() = default;，执行这个函数并处理它的返回值或状态变化。
    HttpMgr() = default;
    // 函数调用：~HttpMgr();；执行该函数并处理它的返回值或副作用。
    ~HttpMgr();
    // 调用 HttpMgr(const HttpMgr&) = delete;，执行这个函数并处理它的返回值或状态变化。
    HttpMgr(const HttpMgr&) = delete;
    // 赋值：把右侧 (const HttpMgr&) = delete 的结果保存到左侧 HttpMgr& operator。
    HttpMgr& operator=(const HttpMgr&) = delete;

    // 执行语句：mutable std::mutex mutex_;；它会更新一次变量、对象或程序状态。
    mutable std::mutex mutex_;
    // 执行语句：bool initialized_{false};；它会更新一次变量、对象或程序状态。
    bool initialized_{false};
    // 执行语句：void* current_client_handle_{nullptr};；它会更新一次变量、对象或程序状态。
    void* current_client_handle_{nullptr};
    // 执行语句：HttpResponse* current_response_{nullptr};；它会更新一次变量、对象或程序状态。
    HttpResponse* current_response_{nullptr};
    // 执行语句：ResponseCallback current_response_callback_;；它会更新一次变量、对象或程序状态。
    ResponseCallback current_response_callback_;
    // 执行语句：DataCallback current_data_callback_;；它会更新一次变量、对象或程序状态。
    DataCallback current_data_callback_;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::protocol::http
