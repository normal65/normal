/**
 * [初学者导读] 文件：main/app/protocol/file_server/file_server.cc
 * 作用：启动局域网文件服务，让电脑或手机访问设备文件。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：网络请求可能阻塞，必须在后台任务或专门任务中执行。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "file_server.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <dirent.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <strings.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <sys/stat.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <unistd.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <string>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_http_server.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_heap_caps.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_log.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_netif.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "freertos/FreeRTOS.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "freertos/task.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "lwip/sockets.h"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "storage/storage.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::protocol::file_server {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "FileServer";

// 固定参数 kIndexPath = "/assets/html/index.html"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kIndexPath = "/assets/html/index.html";
// 缓冲区或分块大小：4096，影响单次读写占用和性能。
constexpr size_t kChunkSize = 4096;
// 固定参数 kMaxEntries = 256：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr size_t kMaxEntries = 256;

// 定义变量 httpd_handle_t s_httpd 并初始化为 nullptr。
httpd_handle_t s_httpd = nullptr;
// 定义变量 int s_dns_sock 并初始化为 -1。
int s_dns_sock = -1;
// 易变变量：告诉编译器该值可能被中断或硬件改变，不能随意优化读取。
volatile bool s_dns_run = false;

// 赋值：把右侧 nullptr 的结果保存到左侧 char* s_chunk。
char* s_chunk = nullptr;

// 定义或声明函数 ensure_chunk：参数在括号里，返回类型写在函数名前面。
bool ensure_chunk()
{
    // 判断条件：s_chunk != nullptr 是否成立。
    if (s_chunk != nullptr)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    // 赋值：把右侧 static_cast<char*>(heap_caps_malloc(kChunkSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)) 的结果保存到左侧 s_chunk。
    s_chunk = static_cast<char*>(heap_caps_malloc(kChunkSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    // 判断条件：s_chunk == nullptr 是否成立。
    if (s_chunk == nullptr)
        // 赋值：把右侧 static_cast<char*>(heap_caps_malloc(kChunkSize, MALLOC_CAP_8BIT)) 的结果保存到左侧 s_chunk。
        s_chunk = static_cast<char*>(heap_caps_malloc(kChunkSize, MALLOC_CAP_8BIT));
    // 返回 s_chunk != nullptr 给调用者，表示当前函数的执行结果。
    return s_chunk != nullptr;
}

// 定义或声明函数 free_chunk：参数在括号里，返回类型写在函数名前面。
void free_chunk()
{
    // 判断条件：s_chunk == nullptr 是否成立。
    if (s_chunk == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 heap_caps_free(s_chunk);，执行这个函数并处理它的返回值或状态变化。
    heap_caps_free(s_chunk);
    // 赋值：把右侧 nullptr 的结果保存到左侧 s_chunk。
    s_chunk = nullptr;
}

/** 取 AP 的 IPv4（点分文本）；失败回退 192.168.4.1 */
// 定义或声明函数 get_ap_ip_str：参数在括号里，返回类型写在函数名前面。
void get_ap_ip_str(char* out, size_t len)
{
    // 赋值：把右侧 esp_netif_get_handle_from_ifkey("WIFI_AP_DEF") 的结果保存到左侧 esp_netif_t* ap。
    esp_netif_t* ap = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    // 声明并默认初始化对象：esp_netif_ip_info_t info{}。
    esp_netif_ip_info_t info{};
    // 判断条件：ap != nullptr && esp_netif_get_ip_info(ap, &info) == ESP_OK && info.ip.addr != 0 是否成立。
    if (ap != nullptr && esp_netif_get_ip_info(ap, &info) == ESP_OK && info.ip.addr != 0)
    {
        // 调用 snprintf(out, len, IPSTR, IP2STR(&info.ip));，执行这个函数并处理它的返回值或状态变化。
        snprintf(out, len, IPSTR, IP2STR(&info.ip));
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 调用 snprintf(out, len, "192.168.4.1");，执行这个函数并处理它的返回值或状态变化。
    snprintf(out, len, "192.168.4.1");
}

// 定义或声明函数 get_ap_ip_u32：参数在括号里，返回类型写在函数名前面。
uint32_t get_ap_ip_u32()
{
    // 赋值：把右侧 esp_netif_get_handle_from_ifkey("WIFI_AP_DEF") 的结果保存到左侧 esp_netif_t* ap。
    esp_netif_t* ap = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    // 声明并默认初始化对象：esp_netif_ip_info_t info{}。
    esp_netif_ip_info_t info{};
    // 判断条件：ap != nullptr && esp_netif_get_ip_info(ap, &info) == ESP_OK && info.ip.addr != 0 是否成立。
    if (ap != nullptr && esp_netif_get_ip_info(ap, &info) == ESP_OK && info.ip.addr != 0)
    {
        // 返回 info.ip.addr 给调用者，表示当前函数的执行结果。
        return info.ip.addr; // 网络字节序
    }
    // 返回 esp_netif_htonl(0xC0A80401) 给调用者，表示当前函数的执行结果。
    return esp_netif_htonl(0xC0A80401); // 192.168.4.1
}

/** 原地 percent 解码 */
// 定义或声明函数 url_decode：参数在括号里，返回类型写在函数名前面。
void url_decode(char* s)
{
    // 赋值：把右侧 s 的结果保存到左侧 char* w。
    char* w = s;
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (*s != '\0')
    {
        // 判断条件：*s == '%' && s[1] != '\0' && s[2] != '\0' 是否成立。
        if (*s == '%' && s[1] != '\0' && s[2] != '\0')
        {
            // 开始代码块：下面是这个函数、分支或循环的主体。
            auto hex = [](char c) -> int {
                // 判断条件：c >= '0' && c <= '9' 是否成立。
                if (c >= '0' && c <= '9')
                    // 返回 c - '0' 给调用者，表示当前函数的执行结果。
                    return c - '0';
                // 判断条件：c >= 'a' && c <= 'f' 是否成立。
                if (c >= 'a' && c <= 'f')
                    // 返回 c - 'a' + 10 给调用者，表示当前函数的执行结果。
                    return c - 'a' + 10;
                // 判断条件：c >= 'A' && c <= 'F' 是否成立。
                if (c >= 'A' && c <= 'F')
                    // 返回 c - 'A' + 10 给调用者，表示当前函数的执行结果。
                    return c - 'A' + 10;
                // 返回 -1 给调用者，表示当前函数的执行结果。
                return -1;
            };
            // 只读约束：这个值或对象在当前位置不能被修改。
            const int hi = hex(s[1]);
            // 只读约束：这个值或对象在当前位置不能被修改。
            const int lo = hex(s[2]);
            // 判断条件：hi >= 0 && lo >= 0 是否成立。
            if (hi >= 0 && lo >= 0)
            {
                *w++ = static_cast<char>((hi << 4) | lo);
                // 赋值：把右侧 3 的结果保存到左侧 s +。
                s += 3;
                // 继续下一轮：跳过本次循环剩余代码。
                continue;
            }
        }
        *w++ = (*s == '+') ? ' ' : *s;
        // 执行语句：++s;；它会更新一次变量、对象或程序状态。
        ++s;
    }
    *w = '\0';
}

/** 读取并解码 query 参数；成功返回 true */
// 定义或声明函数 query_param：参数在括号里，返回类型写在函数名前面。
bool query_param(httpd_req_t* req, const char* key, char* out, size_t out_len)
{
    // 执行语句：char query[512];；它会更新一次变量、对象或程序状态。
    char query[512];
    // 判断条件：httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK 是否成立。
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 判断条件：httpd_query_key_value(query, key, out, out_len) != ESP_OK 是否成立。
    if (httpd_query_key_value(query, key, out, out_len) != ESP_OK)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 调用 url_decode(out);，执行这个函数并处理它的返回值或状态变化。
    url_decode(out);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

/**
 * 把页面里的虚拟路径（以 / 开头，根即用户分区）映射到 /int 下的实际路径。
 * 拒绝 ".." 与空路径。
 */
// 定义或声明函数 build_real_path：参数在括号里，返回类型写在函数名前面。
bool build_real_path(const char* vpath, char* out, size_t out_len)
{
    // 判断条件：vpath == nullptr || vpath[0] != '/' 是否成立。
    if (vpath == nullptr || vpath[0] != '/')
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 判断条件：std::strstr(vpath, "..") != nullptr 是否成立。
    if (std::strstr(vpath, "..") != nullptr)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 判断条件：vpath[1] == '\0' 是否成立。
    if (vpath[1] == '\0') // "/"
    {
        // 调用 snprintf(out, out_len, "%s", common::storage::k_path_internal);，执行这个函数并处理它的返回值或状态变化。
        snprintf(out, out_len, "%s", common::storage::k_path_internal);
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int n = snprintf(out, out_len, "%s%s", common::storage::k_path_internal, vpath);
    // 返回 n > 0 && static_cast<size_t>(n) < out_len 给调用者，表示当前函数的执行结果。
    return n > 0 && static_cast<size_t>(n) < out_len;
}

// 定义或声明函数 internal_ready：参数在括号里，返回类型写在函数名前面。
bool internal_ready()
{
    // 返回 common::storage::StorageMgr::get_instance().is_mounted( 给调用者，表示当前函数的执行结果。
    return common::storage::StorageMgr::get_instance().is_mounted(
        // 继续上一行的表达式或参数列表：common::storage::MountKind::Internal);。
        common::storage::MountKind::Internal);
}

/** JSON 字符串转义（仅 " \ 与控制字符） */
// 定义或声明函数 json_escape：参数在括号里，返回类型写在函数名前面。
void json_escape(const char* in, std::string& out)
{
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (const char* p = in; *p != '\0'; ++p)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const unsigned char c = static_cast<unsigned char>(*p);
        // 判断条件：c == '"' || c == '\\' 是否成立。
        if (c == '"' || c == '\\')
        {
            // 访问对象成员：out.push_back('\\');；读取或修改这个对象里的成员状态。
            out.push_back('\\');
            // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
            out.push_back(static_cast<char>(c));
        }
        // 备用分支：前面的条件不成立时执行这里。
        else if (c < 0x20)
        {
            // 执行语句：char buf[8];；它会更新一次变量、对象或程序状态。
            char buf[8];
            // 调用 snprintf(buf, sizeof(buf), "\\u%04x", c);，执行这个函数并处理它的返回值或状态变化。
            snprintf(buf, sizeof(buf), "\\u%04x", c);
            // 赋值：把右侧 buf 的结果保存到左侧 out +。
            out += buf;
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
            out.push_back(static_cast<char>(c));
        }
    }
}

// 定义或声明函数 send_plain：参数在括号里，返回类型写在函数名前面。
esp_err_t send_plain(httpd_req_t* req, const char* status, const char* msg)
{
    // 调用 httpd_resp_set_status(req, status);，执行这个函数并处理它的返回值或状态变化。
    httpd_resp_set_status(req, status);
    // 调用 httpd_resp_set_type(req, "text/plain; charset=utf-8");，执行这个函数并处理它的返回值或状态变化。
    httpd_resp_set_type(req, "text/plain; charset=utf-8");
    // 返回 httpd_resp_send(req, msg, HTTPD_RESP_USE_STRLEN) 给调用者，表示当前函数的执行结果。
    return httpd_resp_send(req, msg, HTTPD_RESP_USE_STRLEN);
}

// 定义或声明函数 index_handler：参数在括号里，返回类型写在函数名前面。
esp_err_t index_handler(httpd_req_t* req)
{
    // 判断条件：!ensure_chunk() 是否成立。
    if (!ensure_chunk())
        // 返回 send_plain(req, "500 Internal Server Error", "内存不足") 给调用者，表示当前函数的执行结果。
        return send_plain(req, "500 Internal Server Error", "内存不足");

    // 赋值：把右侧 fopen(kIndexPath, "rb") 的结果保存到左侧 FILE* f。
    FILE* f = fopen(kIndexPath, "rb");
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
    {
        // 返回 send_plain(req, "500 Internal Server Error", "页面缺失: /assets/html/index.html") 给调用者，表示当前函数的执行结果。
        return send_plain(req, "500 Internal Server Error", "页面缺失: /assets/html/index.html");
    }

    // 调用 httpd_resp_set_type(req, "text/html; charset=utf-8");，执行这个函数并处理它的返回值或状态变化。
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    // 定义变量 size_t n 并初始化为 0。
    size_t n = 0;
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while ((n = fread(s_chunk, 1, kChunkSize, f)) > 0)
    {
        // 判断条件：httpd_resp_send_chunk(req, s_chunk, n) != ESP_OK 是否成立。
        if (httpd_resp_send_chunk(req, s_chunk, n) != ESP_OK)
        {
            // 调用 fclose(f);，执行这个函数并处理它的返回值或状态变化。
            fclose(f);
            // 返回 ESP_FAIL 给调用者，表示当前函数的执行结果。
            return ESP_FAIL;
        }
    }
    // 调用 fclose(f);，执行这个函数并处理它的返回值或状态变化。
    fclose(f);
    // 返回 httpd_resp_send_chunk(req, nullptr, 0) 给调用者，表示当前函数的执行结果。
    return httpd_resp_send_chunk(req, nullptr, 0);
}

/** Captive Portal：未知路径一律 302 到首页（触发系统弹窗） */
// 定义或声明函数 redirect_handler：参数在括号里，返回类型写在函数名前面。
esp_err_t redirect_handler(httpd_req_t* req)
{
    // 执行语句：char ip[16];；它会更新一次变量、对象或程序状态。
    char ip[16];
    // 调用 get_ap_ip_str(ip, sizeof(ip));，执行这个函数并处理它的返回值或状态变化。
    get_ap_ip_str(ip, sizeof(ip));
    // 执行语句：char loc[40];；它会更新一次变量、对象或程序状态。
    char loc[40];
    // 调用 snprintf(loc, sizeof(loc), "http:，执行这个函数并处理它的返回值或状态变化。
    snprintf(loc, sizeof(loc), "http://%s/", ip);

    // 调用 httpd_resp_set_status(req, "302 Found");，执行这个函数并处理它的返回值或状态变化。
    httpd_resp_set_status(req, "302 Found");
    // 调用 httpd_resp_set_hdr(req, "Location", loc);，执行这个函数并处理它的返回值或状态变化。
    httpd_resp_set_hdr(req, "Location", loc);
    // 返回 httpd_resp_send(req, nullptr, 0) 给调用者，表示当前函数的执行结果。
    return httpd_resp_send(req, nullptr, 0);
}

/**
 * 静默丢弃非本服务的请求。
 * 连热点后手机后台 App（微信 /mmtls、各类 PATCH 探测等）会被 DNS 劫持到本机，
 * 若无对应 method 处理器，httpd 会打 405 警告；此处统一回 404 并吞掉 body。
 */
// 定义或声明函数 discard_handler：参数在括号里，返回类型写在函数名前面。
esp_err_t discard_handler(httpd_req_t* req)
{
    // 判断条件：!ensure_chunk() 是否成立。
    if (!ensure_chunk())
    {
        // 调用 httpd_resp_set_status(req, "404 Not Found");，执行这个函数并处理它的返回值或状态变化。
        httpd_resp_set_status(req, "404 Not Found");
        // 返回 httpd_resp_send(req, nullptr, 0) 给调用者，表示当前函数的执行结果。
        return httpd_resp_send(req, nullptr, 0);
    }

    // 定义变量 int remaining 并初始化为 static_cast<int>(req->content_len)。
    int remaining = static_cast<int>(req->content_len);
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (remaining > 0)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int chunk = remaining > 64 ? 64 : remaining;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int got = httpd_req_recv(req, s_chunk, static_cast<size_t>(chunk));
        // 判断条件：got <= 0 是否成立。
        if (got <= 0)
        {
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        }
        // 赋值：把右侧 got 的结果保存到左侧 remaining -。
        remaining -= got;
    }

    // 调用 httpd_resp_set_status(req, "404 Not Found");，执行这个函数并处理它的返回值或状态变化。
    httpd_resp_set_status(req, "404 Not Found");
    // 返回 httpd_resp_send(req, nullptr, 0) 给调用者，表示当前函数的执行结果。
    return httpd_resp_send(req, nullptr, 0);
}

// 定义或声明函数 list_handler：参数在括号里，返回类型写在函数名前面。
esp_err_t list_handler(httpd_req_t* req)
{
    // 判断条件：!internal_ready() 是否成立。
    if (!internal_ready())
    {
        // 返回 send_plain(req, "503 Service Unavailable", "存储被 USB 占用") 给调用者，表示当前函数的执行结果。
        return send_plain(req, "503 Service Unavailable", "存储被 USB 占用");
    }

    // 执行语句：char vpath[256];；它会更新一次变量、对象或程序状态。
    char vpath[256];
    // 执行语句：char real[288];；它会更新一次变量、对象或程序状态。
    char real[288];
    // 判断条件：!query_param(req, "path", vpath, sizeof(vpath) 是否成立。
    if (!query_param(req, "path", vpath, sizeof(vpath)) ||
        // 定义或声明函数 build_real_path：参数在括号里，返回类型写在函数名前面。
        !build_real_path(vpath, real, sizeof(real)))
    {
        // 返回 send_plain(req, "400 Bad Request", "路径无效") 给调用者，表示当前函数的执行结果。
        return send_plain(req, "400 Bad Request", "路径无效");
    }

    // 赋值：把右侧 opendir(real) 的结果保存到左侧 DIR* dir。
    DIR* dir = opendir(real);
    // 判断条件：dir == nullptr 是否成立。
    if (dir == nullptr)
    {
        // 返回 send_plain(req, "404 Not Found", "目录不存在") 给调用者，表示当前函数的执行结果。
        return send_plain(req, "404 Not Found", "目录不存在");
    }

    // 执行语句：std::string json;；它会更新一次变量、对象或程序状态。
    std::string json;
    // 访问对象成员：json.reserve(2048);；读取或修改这个对象里的成员状态。
    json.reserve(2048);
    // 赋值：把右侧 "{\"path\":\"" 的结果保存到左侧 json +。
    json += "{\"path\":\"";
    // 调用 json_escape(vpath, json);，执行这个函数并处理它的返回值或状态变化。
    json_escape(vpath, json);
    // 赋值：把右侧 "\",\"entries\":[" 的结果保存到左侧 json +。
    json += "\",\"entries\":[";

    // 定义变量 size_t count 并初始化为 0。
    size_t count = 0;
    // struct dirent：把相关数据字段组合在一起，表示项目中的一个概念。
    struct dirent* ent = nullptr;
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while ((ent = readdir(dir)) != nullptr && count < kMaxEntries)
    {
        // 判断条件：ent->d_name[0] == '.' 是否成立。
        if (ent->d_name[0] == '.')
        {
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        }
        // 隐藏 Windows 自动生成目录
        if (strcasecmp(ent->d_name, "System Volume Information") == 0 ||
            // 定义或声明函数 strcasecmp：参数在括号里，返回类型写在函数名前面。
            strcasecmp(ent->d_name, "$RECYCLE.BIN") == 0)
        {
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        }

        // 执行语句：char child[560];；它会更新一次变量、对象或程序状态。
        char child[560];
        // 调用 snprintf(child, sizeof(child), "%s/%s", real, ent->d_name);，执行这个函数并处理它的返回值或状态变化。
        snprintf(child, sizeof(child), "%s/%s", real, ent->d_name);
        // struct stat：把相关数据字段组合在一起，表示项目中的一个概念。
        struct stat st{};
        // 只读约束：这个值或对象在当前位置不能被修改。
        const bool ok = (stat(child, &st) == 0);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const bool is_dir = ok && S_ISDIR(st.st_mode);

        // 判断条件：count > 0 是否成立。
        if (count > 0)
        {
            // 赋值：把右侧 ',' 的结果保存到左侧 json +。
            json += ',';
        }
        // 赋值：把右侧 "{\"n\":\"" 的结果保存到左侧 json +。
        json += "{\"n\":\"";
        // 调用 json_escape(ent->d_name, json);，执行这个函数并处理它的返回值或状态变化。
        json_escape(ent->d_name, json);
        // 赋值：把右侧 "\",\"d\":" 的结果保存到左侧 json +。
        json += "\",\"d\":";
        // 赋值：把右侧 is_dir ? "1" : "0" 的结果保存到左侧 json +。
        json += is_dir ? "1" : "0";
        // 赋值：把右侧 ",\"s\":" 的结果保存到左侧 json +。
        json += ",\"s\":";
        // 执行语句：char num[24];；它会更新一次变量、对象或程序状态。
        char num[24];
        // 调用 snprintf(num, sizeof(num), "%u", static_cast<unsigned>(ok && !is_dir ? st.st_size : 0));，执行这个函数并处理它的返回值或状态变化。
        snprintf(num, sizeof(num), "%u", static_cast<unsigned>(ok && !is_dir ? st.st_size : 0));
        // 赋值：把右侧 num 的结果保存到左侧 json +。
        json += num;
        // 赋值：把右侧 '}' 的结果保存到左侧 json +。
        json += '}';
        // 执行语句：++count;；它会更新一次变量、对象或程序状态。
        ++count;
    }
    // 调用 closedir(dir);，执行这个函数并处理它的返回值或状态变化。
    closedir(dir);
    // 赋值：把右侧 "]}" 的结果保存到左侧 json +。
    json += "]}";

    // 调用 httpd_resp_set_type(req, "application/json; charset=utf-8");，执行这个函数并处理它的返回值或状态变化。
    httpd_resp_set_type(req, "application/json; charset=utf-8");
    // 返回 httpd_resp_send(req, json.c_str(), json.size()) 给调用者，表示当前函数的执行结果。
    return httpd_resp_send(req, json.c_str(), json.size());
}

// 定义或声明函数 download_handler：参数在括号里，返回类型写在函数名前面。
esp_err_t download_handler(httpd_req_t* req)
{
    // 判断条件：!internal_ready() 是否成立。
    if (!internal_ready())
    {
        // 返回 send_plain(req, "503 Service Unavailable", "存储被 USB 占用") 给调用者，表示当前函数的执行结果。
        return send_plain(req, "503 Service Unavailable", "存储被 USB 占用");
    }
    // 判断条件：!ensure_chunk() 是否成立。
    if (!ensure_chunk())
        // 返回 send_plain(req, "500 Internal Server Error", "内存不足") 给调用者，表示当前函数的执行结果。
        return send_plain(req, "500 Internal Server Error", "内存不足");

    // 执行语句：char vpath[256];；它会更新一次变量、对象或程序状态。
    char vpath[256];
    // 执行语句：char real[288];；它会更新一次变量、对象或程序状态。
    char real[288];
    // 判断条件：!query_param(req, "path", vpath, sizeof(vpath) 是否成立。
    if (!query_param(req, "path", vpath, sizeof(vpath)) ||
        // 定义或声明函数 build_real_path：参数在括号里，返回类型写在函数名前面。
        !build_real_path(vpath, real, sizeof(real)))
    {
        // 返回 send_plain(req, "400 Bad Request", "路径无效") 给调用者，表示当前函数的执行结果。
        return send_plain(req, "400 Bad Request", "路径无效");
    }

    // 赋值：把右侧 fopen(real, "rb") 的结果保存到左侧 FILE* f。
    FILE* f = fopen(real, "rb");
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
    {
        // 返回 send_plain(req, "404 Not Found", "文件不存在") 给调用者，表示当前函数的执行结果。
        return send_plain(req, "404 Not Found", "文件不存在");
    }

    // 调用 httpd_resp_set_type(req, "application/octet-stream");，执行这个函数并处理它的返回值或状态变化。
    httpd_resp_set_type(req, "application/octet-stream");
    // 调用 httpd_resp_set_hdr(req, "Content-Disposition", "attachment");，执行这个函数并处理它的返回值或状态变化。
    httpd_resp_set_hdr(req, "Content-Disposition", "attachment");

    // 定义变量 size_t n 并初始化为 0。
    size_t n = 0;
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while ((n = fread(s_chunk, 1, kChunkSize, f)) > 0)
    {
        // 判断条件：httpd_resp_send_chunk(req, s_chunk, n) != ESP_OK 是否成立。
        if (httpd_resp_send_chunk(req, s_chunk, n) != ESP_OK)
        {
            // 调用 fclose(f);，执行这个函数并处理它的返回值或状态变化。
            fclose(f);
            // 返回 ESP_FAIL 给调用者，表示当前函数的执行结果。
            return ESP_FAIL;
        }
    }
    // 调用 fclose(f);，执行这个函数并处理它的返回值或状态变化。
    fclose(f);
    // 返回 httpd_resp_send_chunk(req, nullptr, 0) 给调用者，表示当前函数的执行结果。
    return httpd_resp_send_chunk(req, nullptr, 0);
}

// 定义或声明函数 upload_handler：参数在括号里，返回类型写在函数名前面。
esp_err_t upload_handler(httpd_req_t* req)
{
    // 判断条件：!internal_ready() 是否成立。
    if (!internal_ready())
    {
        // 返回 send_plain(req, "503 Service Unavailable", "存储被 USB 占用") 给调用者，表示当前函数的执行结果。
        return send_plain(req, "503 Service Unavailable", "存储被 USB 占用");
    }
    // 判断条件：!ensure_chunk() 是否成立。
    if (!ensure_chunk())
        // 返回 send_plain(req, "500 Internal Server Error", "内存不足") 给调用者，表示当前函数的执行结果。
        return send_plain(req, "500 Internal Server Error", "内存不足");

    // 执行语句：char vdir[224];；它会更新一次变量、对象或程序状态。
    char vdir[224];
    // 执行语句：char name[128];；它会更新一次变量、对象或程序状态。
    char name[128];
    // 判断条件：!query_param(req, "path", vdir, sizeof(vdir) 是否成立。
    if (!query_param(req, "path", vdir, sizeof(vdir)) ||
        // 定义或声明函数 query_param：参数在括号里，返回类型写在函数名前面。
        !query_param(req, "name", name, sizeof(name)))
    {
        // 返回 send_plain(req, "400 Bad Request", "参数缺失") 给调用者，表示当前函数的执行结果。
        return send_plain(req, "400 Bad Request", "参数缺失");
    }
    // 判断条件：name[0] == '\0' || std::strchr(name, '/') != nullptr || std::strstr(name, "..") != nullptr 是否成立。
    if (name[0] == '\0' || std::strchr(name, '/') != nullptr || std::strstr(name, "..") != nullptr)
    {
        // 返回 send_plain(req, "400 Bad Request", "文件名无效") 给调用者，表示当前函数的执行结果。
        return send_plain(req, "400 Bad Request", "文件名无效");
    }

    // 执行语句：char vpath[368];；它会更新一次变量、对象或程序状态。
    char vpath[368];
    // 调用 snprintf(vpath, sizeof(vpath), "%s%s%s", vdir, (vdir[1] == '\0') ? "" : "/", name);，执行这个函数并处理它的返回值或状态变化。
    snprintf(vpath, sizeof(vpath), "%s%s%s", vdir, (vdir[1] == '\0') ? "" : "/", name);
    // 执行语句：char real[400];；它会更新一次变量、对象或程序状态。
    char real[400];
    // 判断条件：!build_real_path(vpath, real, sizeof(real)) 是否成立。
    if (!build_real_path(vpath, real, sizeof(real)))
    {
        // 返回 send_plain(req, "400 Bad Request", "路径无效") 给调用者，表示当前函数的执行结果。
        return send_plain(req, "400 Bad Request", "路径无效");
    }

    // 赋值：把右侧 fopen(real, "wb") 的结果保存到左侧 FILE* f。
    FILE* f = fopen(real, "wb");
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
    {
        // 返回 send_plain(req, "500 Internal Server Error", "无法创建文件") 给调用者，表示当前函数的执行结果。
        return send_plain(req, "500 Internal Server Error", "无法创建文件");
    }

    // 定义变量 size_t remaining 并初始化为 req->content_len。
    size_t remaining = req->content_len;
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (remaining > 0)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const size_t want = remaining < kChunkSize ? remaining : kChunkSize;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int got = httpd_req_recv(req, s_chunk, want);
        // 判断条件：got <= 0 是否成立。
        if (got <= 0)
        {
            // 判断条件：got == HTTPD_SOCK_ERR_TIMEOUT 是否成立。
            if (got == HTTPD_SOCK_ERR_TIMEOUT)
            {
                // 继续下一轮：跳过本次循环剩余代码。
                continue;
            }
            // 调用 fclose(f);，执行这个函数并处理它的返回值或状态变化。
            fclose(f);
            // 调用 unlink(real);，执行这个函数并处理它的返回值或状态变化。
            unlink(real);
            // 返回 ESP_FAIL 给调用者，表示当前函数的执行结果。
            return ESP_FAIL;
        }
        // 判断条件：fwrite(s_chunk, 1, static_cast<size_t>(got), f) != static_cast<size_t>(got) 是否成立。
        if (fwrite(s_chunk, 1, static_cast<size_t>(got), f) != static_cast<size_t>(got))
        {
            // 调用 fclose(f);，执行这个函数并处理它的返回值或状态变化。
            fclose(f);
            // 调用 unlink(real);，执行这个函数并处理它的返回值或状态变化。
            unlink(real);
            // 返回 send_plain(req, "500 Internal Server Error", "写入失败(空间不足?)") 给调用者，表示当前函数的执行结果。
            return send_plain(req, "500 Internal Server Error", "写入失败(空间不足?)");
        }
        // 赋值：把右侧 static_cast<size_t>(got) 的结果保存到左侧 remaining -。
        remaining -= static_cast<size_t>(got);
    }
    // 调用 fclose(f);，执行这个函数并处理它的返回值或状态变化。
    fclose(f);

    // 调用 ESP_LOGD(TAG, "上传完成: %s (%u 字节)", real, static_cast<unsigned>(req->content_len));，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "上传完成: %s (%u 字节)", real, static_cast<unsigned>(req->content_len));
    // 返回 send_plain(req, "200 OK", "ok") 给调用者，表示当前函数的执行结果。
    return send_plain(req, "200 OK", "ok");
}

// 定义或声明函数 delete_handler：参数在括号里，返回类型写在函数名前面。
esp_err_t delete_handler(httpd_req_t* req)
{
    // 判断条件：!internal_ready() 是否成立。
    if (!internal_ready())
    {
        // 返回 send_plain(req, "503 Service Unavailable", "存储被 USB 占用") 给调用者，表示当前函数的执行结果。
        return send_plain(req, "503 Service Unavailable", "存储被 USB 占用");
    }

    // 执行语句：char vpath[256];；它会更新一次变量、对象或程序状态。
    char vpath[256];
    // 执行语句：char real[288];；它会更新一次变量、对象或程序状态。
    char real[288];
    // 判断条件：!query_param(req, "path", vpath, sizeof(vpath) 是否成立。
    if (!query_param(req, "path", vpath, sizeof(vpath)) ||
        // 定义或声明函数 build_real_path：参数在括号里，返回类型写在函数名前面。
        !build_real_path(vpath, real, sizeof(real)) || vpath[1] == '\0')
    {
        // 返回 send_plain(req, "400 Bad Request", "路径无效") 给调用者，表示当前函数的执行结果。
        return send_plain(req, "400 Bad Request", "路径无效");
    }

    // 赋值：把右侧 "0" 的结果保存到左侧 char dirflag[8]。
    char dirflag[8] = "0";
    // 调用 query_param(req, "dir", dirflag, sizeof(dirflag));，执行这个函数并处理它的返回值或状态变化。
    query_param(req, "dir", dirflag, sizeof(dirflag));

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int rc = (dirflag[0] == '1') ? rmdir(real) : unlink(real);
    // 判断条件：rc != 0 是否成立。
    if (rc != 0)
    {
        // 返回 send_plain(req, "500 Internal Server Error", "删除失败") 给调用者，表示当前函数的执行结果。
        return send_plain(req, "500 Internal Server Error", "删除失败");
    }
    // 返回 send_plain(req, "200 OK", "ok") 给调用者，表示当前函数的执行结果。
    return send_plain(req, "200 OK", "ok");
}

// 定义或声明函数 mkdir_handler：参数在括号里，返回类型写在函数名前面。
esp_err_t mkdir_handler(httpd_req_t* req)
{
    // 判断条件：!internal_ready() 是否成立。
    if (!internal_ready())
    {
        // 返回 send_plain(req, "503 Service Unavailable", "存储被 USB 占用") 给调用者，表示当前函数的执行结果。
        return send_plain(req, "503 Service Unavailable", "存储被 USB 占用");
    }

    // 执行语句：char vpath[256];；它会更新一次变量、对象或程序状态。
    char vpath[256];
    // 执行语句：char real[288];；它会更新一次变量、对象或程序状态。
    char real[288];
    // 判断条件：!query_param(req, "path", vpath, sizeof(vpath) 是否成立。
    if (!query_param(req, "path", vpath, sizeof(vpath)) ||
        // 定义或声明函数 build_real_path：参数在括号里，返回类型写在函数名前面。
        !build_real_path(vpath, real, sizeof(real)) || vpath[1] == '\0')
    {
        // 返回 send_plain(req, "400 Bad Request", "路径无效") 给调用者，表示当前函数的执行结果。
        return send_plain(req, "400 Bad Request", "路径无效");
    }

    // 判断条件：mkdir(real, 0777) != 0 是否成立。
    if (mkdir(real, 0777) != 0)
    {
        // 返回 send_plain(req, "500 Internal Server Error", "创建失败") 给调用者，表示当前函数的执行结果。
        return send_plain(req, "500 Internal Server Error", "创建失败");
    }
    // 返回 send_plain(req, "200 OK", "ok") 给调用者，表示当前函数的执行结果。
    return send_plain(req, "200 OK", "ok");
}

// 定义或声明函数 dns_task：参数在括号里，返回类型写在函数名前面。
void dns_task(void*)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    // 判断条件：sock < 0 是否成立。
    if (sock < 0)
    {
        // 调用 ESP_LOGE(TAG, "DNS socket 创建失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "DNS socket 创建失败");
        // 调用 vTaskDelete(nullptr);，执行这个函数并处理它的返回值或状态变化。
        vTaskDelete(nullptr);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 赋值：把右侧 sock 的结果保存到左侧 s_dns_sock。
    s_dns_sock = sock;

    // 声明并默认初始化对象：sockaddr_in bind_addr{}。
    sockaddr_in bind_addr{};
    // 赋值：把右侧 AF_INET 的结果保存到左侧 bind_addr.sin_family。
    bind_addr.sin_family = AF_INET;
    // 赋值：把右侧 htons(53) 的结果保存到左侧 bind_addr.sin_port。
    bind_addr.sin_port = htons(53);
    // 赋值：把右侧 htonl(INADDR_ANY) 的结果保存到左侧 bind_addr.sin_addr.s_addr。
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // 判断条件：bind(sock, reinterpret_cast<sockaddr*>(&bind_addr), sizeof(bind_addr)) < 0 是否成立。
    if (bind(sock, reinterpret_cast<sockaddr*>(&bind_addr), sizeof(bind_addr)) < 0)
    {
        // 调用 ESP_LOGE(TAG, "DNS bind 53 失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "DNS bind 53 失败");
        // 调用 close(sock)：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
        close(sock);
        // 赋值：把右侧 -1 的结果保存到左侧 s_dns_sock。
        s_dns_sock = -1;
        // 调用 vTaskDelete(nullptr);，执行这个函数并处理它的返回值或状态变化。
        vTaskDelete(nullptr);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 声明并默认初始化对象：timeval tv{}。
    timeval tv{};
    // 赋值：把右侧 1 的结果保存到左侧 tv.tv_sec。
    tv.tv_sec = 1;
    // 调用 setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));，执行这个函数并处理它的返回值或状态变化。
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // 执行语句：uint8_t buf[512 + 16];；它会更新一次变量、对象或程序状态。
    uint8_t buf[512 + 16];
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (s_dns_run)
    {
        // 声明并默认初始化对象：sockaddr_in src{}。
        sockaddr_in src{};
        // 定义变量 socklen_t src_len 并初始化为 sizeof(src)。
        socklen_t src_len = sizeof(src);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int len = recvfrom(sock, buf, 512, 0, reinterpret_cast<sockaddr*>(&src), &src_len);
        // 判断条件：len < 12 + 5 是否成立。
        if (len < 12 + 5) // DNS 头 12 字节 + 最小问题段
        {
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        }

        // 跳过 QNAME 找到问题段结束（zero byte + QTYPE/QCLASS 各 2 字节）
        int pos = 12;
        // 循环执行：条件为真时重复运行，条件变化后退出。
        while (pos < len && buf[pos] != 0)
        {
            // 赋值：把右侧 buf[pos] + 1 的结果保存到左侧 pos +。
            pos += buf[pos] + 1;
        }
        // 赋值：把右侧 1 + 4 的结果保存到左侧 pos +。
        pos += 1 + 4;
        // 判断条件：pos > len 是否成立。
        if (pos > len)
        {
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        }

        // 头部：响应 + 递归可用，1 问题 1 应答
        buf[2] = 0x81;
        // 赋值：把右侧 0x80 的结果保存到左侧 buf[3]。
        buf[3] = 0x80;
        // 赋值：把右侧 0x00 的结果保存到左侧 buf[4]。
        buf[4] = 0x00;
        // 赋值：把右侧 0x01 的结果保存到左侧 buf[5]。
        buf[5] = 0x01; // QDCOUNT
        // 赋值：把右侧 0x00 的结果保存到左侧 buf[6]。
        buf[6] = 0x00;
        // 赋值：把右侧 0x01 的结果保存到左侧 buf[7]。
        buf[7] = 0x01; // ANCOUNT
        // 赋值：把右侧 0x00 的结果保存到左侧 buf[8]。
        buf[8] = 0x00;
        // 赋值：把右侧 0x00 的结果保存到左侧 buf[9]。
        buf[9] = 0x00;
        // 赋值：把右侧 0x00 的结果保存到左侧 buf[10]。
        buf[10] = 0x00;
        // 赋值：把右侧 0x00 的结果保存到左侧 buf[11]。
        buf[11] = 0x00;

        // 应答：指针指向问题名，A 记录，TTL 60s，IP 4 字节
        const uint32_t ip = get_ap_ip_u32();
        // 赋值：把右侧 buf + pos 的结果保存到左侧 uint8_t* p。
        uint8_t* p = buf + pos;
        *p++ = 0xC0;
        *p++ = 0x0C;
        *p++ = 0x00;
        *p++ = 0x01; // TYPE A
        *p++ = 0x00;
        *p++ = 0x01; // CLASS IN
        *p++ = 0x00;
        *p++ = 0x00;
        *p++ = 0x00;
        *p++ = 0x3C;
        *p++ = 0x00;
        *p++ = 0x04;
        // 调用 std::memcpy(p, &ip, 4);，执行这个函数并处理它的返回值或状态变化。
        std::memcpy(p, &ip, 4);
        // 赋值：把右侧 4 的结果保存到左侧 p +。
        p += 4;

        // 调用 sendto(sock, buf, static_cast<size_t>(p - buf), 0, reinterpret_cast<sockaddr*>(&src),，执行这个函数并处理它的返回值或状态变化。
        sendto(sock, buf, static_cast<size_t>(p - buf), 0, reinterpret_cast<sockaddr*>(&src),
               // 继续上一行的表达式或参数列表：src_len);。
               src_len);
    }

    // 调用 close(sock)：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
    close(sock);
    // 赋值：把右侧 -1 的结果保存到左侧 s_dns_sock。
    s_dns_sock = -1;
    // 调用 vTaskDelete(nullptr);，执行这个函数并处理它的返回值或状态变化。
    vTaskDelete(nullptr);
}

/** 让 DHCP 把本机下发为 DNS 服务器（自动弹页的前提） */
// 定义或声明函数 setup_dhcp_dns：参数在括号里，返回类型写在函数名前面。
void setup_dhcp_dns()
{
    // 赋值：把右侧 esp_netif_get_handle_from_ifkey("WIFI_AP_DEF") 的结果保存到左侧 esp_netif_t* ap。
    esp_netif_t* ap = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    // 判断条件：ap == nullptr 是否成立。
    if (ap == nullptr)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 声明并默认初始化对象：esp_netif_ip_info_t info{}。
    esp_netif_ip_info_t info{};
    // 判断条件：esp_netif_get_ip_info(ap, &info) != ESP_OK 是否成立。
    if (esp_netif_get_ip_info(ap, &info) != ESP_OK)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 声明并默认初始化对象：esp_netif_dns_info_t dns{}。
    esp_netif_dns_info_t dns{};
    // 赋值：把右侧 ESP_IPADDR_TYPE_V4 的结果保存到左侧 dns.ip.type。
    dns.ip.type = ESP_IPADDR_TYPE_V4;
    // 赋值：把右侧 info.ip.addr 的结果保存到左侧 dns.ip.u_addr.ip4.addr。
    dns.ip.u_addr.ip4.addr = info.ip.addr;

    // 调用 esp_netif_dhcps_stop(ap);，执行这个函数并处理它的返回值或状态变化。
    esp_netif_dhcps_stop(ap);
    // 定义变量 uint8_t offer 并初始化为 1。
    uint8_t offer = 1; // OFFER_DNS
    // 调用 esp_netif_dhcps_option(ap, ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER, &offer,，执行这个函数并处理它的返回值或状态变化。
    esp_netif_dhcps_option(ap, ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER, &offer,
                           // 继续上一行的表达式或参数列表：sizeof(offer));。
                           sizeof(offer));
    // 调用 esp_netif_set_dns_info(ap, ESP_NETIF_DNS_MAIN, &dns);，执行这个函数并处理它的返回值或状态变化。
    esp_netif_set_dns_info(ap, ESP_NETIF_DNS_MAIN, &dns);
    // 调用 esp_netif_dhcps_start(ap);，执行这个函数并处理它的返回值或状态变化。
    esp_netif_dhcps_start(ap);
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 函数定义/声明：调用 start()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
esp_err_t start()
{
    // 判断条件：s_httpd != nullptr 是否成立。
    if (s_httpd != nullptr)
    {
        // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
        return ESP_OK;
    }

    // 判断条件：!ensure_chunk() 是否成立。
    if (!ensure_chunk())
    {
        // 调用 ESP_LOGE(TAG, "chunk 缓冲分配失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "chunk 缓冲分配失败");
        // 返回 ESP_ERR_NO_MEM 给调用者，表示当前函数的执行结果。
        return ESP_ERR_NO_MEM;
    }

    // 调用 setup_dhcp_dns();，执行这个函数并处理它的返回值或状态变化。
    setup_dhcp_dns();

    // 定义变量 httpd_config_t cfg 并初始化为 HTTPD_DEFAULT_CONFIG()。
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    // 赋值：把右侧 8192 的结果保存到左侧 cfg.stack_size。
    cfg.stack_size = 8192;
    // 赋值：把右侧 16 的结果保存到左侧 cfg.max_uri_handlers。
    cfg.max_uri_handlers = 16;
    // 赋值：把右侧 true 的结果保存到左侧 cfg.lru_purge_enable。
    cfg.lru_purge_enable = true;
    // 赋值：把右侧 httpd_uri_match_wildcard 的结果保存到左侧 cfg.uri_match_fn。
    cfg.uri_match_fn = httpd_uri_match_wildcard;

    // 定义变量 esp_err_t err 并初始化为 httpd_start(&s_httpd, &cfg)。
    esp_err_t err = httpd_start(&s_httpd, &cfg);
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "httpd_start: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "httpd_start: %s", esp_err_to_name(err));
        // 调用 free_chunk();，执行这个函数并处理它的返回值或状态变化。
        free_chunk();
        // 返回 err 给调用者，表示当前函数的执行结果。
        return err;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const httpd_uri_t routes[] = {
        // 执行这条代码：{.uri = "/", .method = HTTP_GET, .handler = index_handler, .user_ctx = nullptr},。它会更新变量、对象状态或调用链。
        {.uri = "/", .method = HTTP_GET, .handler = index_handler, .user_ctx = nullptr},
        // 继续上一行的表达式或参数列表：{.uri = "/api/list", .method = HTTP_GET, .handler = list_handler, .user_ctx = nullptr},。
        {.uri = "/api/list", .method = HTTP_GET, .handler = list_handler, .user_ctx = nullptr},
        // 继续上一行的表达式或参数列表：{.uri = "/api/download",。
        {.uri = "/api/download",
         // 继续上一行的表达式或参数列表：.method = HTTP_GET,。
         .method = HTTP_GET,
         // 继续上一行的表达式或参数列表：.handler = download_handler,。
         .handler = download_handler,
         // 继续上一行的表达式或参数列表：.user_ctx = nullptr},。
         .user_ctx = nullptr},
        // 继续上一行的表达式或参数列表：{.uri = "/api/upload", .method = HTTP_POST, .handler = upload_handler, .user_ctx = nullptr},。
        {.uri = "/api/upload", .method = HTTP_POST, .handler = upload_handler, .user_ctx = nullptr},
        // 继续上一行的表达式或参数列表：{.uri = "/api/delete", .method = HTTP_POST, .handler = delete_handler, .user_ctx = nullptr},。
        {.uri = "/api/delete", .method = HTTP_POST, .handler = delete_handler, .user_ctx = nullptr},
        // 继续上一行的表达式或参数列表：{.uri = "/api/mkdir", .method = HTTP_POST, .handler = mkdir_handler, .user_ctx = nullptr},。
        {.uri = "/api/mkdir", .method = HTTP_POST, .handler = mkdir_handler, .user_ctx = nullptr},
        // 兜底：所有未知 GET（含各系统门户探测路径）→ 302 首页
        {.uri = "/*", .method = HTTP_GET, .handler = redirect_handler, .user_ctx = nullptr},
        // 兜底：手机后台 App 被 DNS 劫持发来的非 GET 请求 → 静默 404
        {.uri = "/*", .method = HTTP_POST, .handler = discard_handler, .user_ctx = nullptr},
        // 继续上一行的表达式或参数列表：{.uri = "/*", .method = HTTP_PUT, .handler = discard_handler, .user_ctx = nullptr},。
        {.uri = "/*", .method = HTTP_PUT, .handler = discard_handler, .user_ctx = nullptr},
        // 继续上一行的表达式或参数列表：{.uri = "/*", .method = HTTP_PATCH, .handler = discard_handler, .user_ctx = nullptr},。
        {.uri = "/*", .method = HTTP_PATCH, .handler = discard_handler, .user_ctx = nullptr},
        // 继续上一行的表达式或参数列表：{.uri = "/*", .method = HTTP_DELETE, .handler = discard_handler, .user_ctx = nullptr},。
        {.uri = "/*", .method = HTTP_DELETE, .handler = discard_handler, .user_ctx = nullptr},
        // 继续上一行的表达式或参数列表：{.uri = "/*", .method = HTTP_OPTIONS, .handler = discard_handler, .user_ctx = nullptr},。
        {.uri = "/*", .method = HTTP_OPTIONS, .handler = discard_handler, .user_ctx = nullptr},
        // 继续上一行的表达式或参数列表：{.uri = "/*", .method = HTTP_HEAD, .handler = discard_handler, .user_ctx = nullptr},。
        {.uri = "/*", .method = HTTP_HEAD, .handler = discard_handler, .user_ctx = nullptr},
    };
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (const auto& r : routes)
    {
        // 调用 httpd_register_uri_handler(s_httpd, &r);，执行这个函数并处理它的返回值或状态变化。
        httpd_register_uri_handler(s_httpd, &r);
    }

    // 赋值：把右侧 true 的结果保存到左侧 s_dns_run。
    s_dns_run = true;
    // 判断条件：xTaskCreate(dns_task, "dns_hijack", 3072, nullptr, 5, nullptr) != pdPASS 是否成立。
    if (xTaskCreate(dns_task, "dns_hijack", 3072, nullptr, 5, nullptr) != pdPASS)
    {
        // 调用 ESP_LOGW(TAG, "DNS 任务创建失败, 自动弹页不可用");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGW(TAG, "DNS 任务创建失败, 自动弹页不可用");
        // 赋值：把右侧 false 的结果保存到左侧 s_dns_run。
        s_dns_run = false;
    }

    // 执行语句：char ip[16];；它会更新一次变量、对象或程序状态。
    char ip[16];
    // 调用 get_ap_ip_str(ip, sizeof(ip));，执行这个函数并处理它的返回值或状态变化。
    get_ap_ip_str(ip, sizeof(ip));
    // 调用 ESP_LOGI(TAG, "局域网传输已启动: http:，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGI(TAG, "局域网传输已启动: http://%s/", ip);
    // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
    return ESP_OK;
}

// 函数定义/声明：调用 stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
void stop()
{
    // 赋值：把右侧 false 的结果保存到左侧 s_dns_run。
    s_dns_run = false;
    // 判断条件：s_dns_sock >= 0 是否成立。
    if (s_dns_sock >= 0)
    {
        // 调用 shutdown(s_dns_sock, SHUT_RDWR);，执行这个函数并处理它的返回值或状态变化。
        shutdown(s_dns_sock, SHUT_RDWR);
    }

    // 判断条件：s_httpd != nullptr 是否成立。
    if (s_httpd != nullptr)
    {
        // 调用 httpd_stop(s_httpd);，执行这个函数并处理它的返回值或状态变化。
        httpd_stop(s_httpd);
        // 赋值：把右侧 nullptr 的结果保存到左侧 s_httpd。
        s_httpd = nullptr;
        // 调用 ESP_LOGD(TAG, "局域网传输已停止");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGD(TAG, "局域网传输已停止");
    }
    // 调用 free_chunk();，执行这个函数并处理它的返回值或状态变化。
    free_chunk();
}

// 定义或声明函数 is_running：参数在括号里，返回类型写在函数名前面。
bool is_running()
{
    // 返回 s_httpd != nullptr 给调用者，表示当前函数的执行结果。
    return s_httpd != nullptr;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::protocol::file_server
