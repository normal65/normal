/**
 * [初学者导读] 文件：main/common/ota/ota.cc
 * 作用：实现 OTA 检查更新、下载固件、MD5 校验、切换分区和状态上报。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：OTA 回调不在 UI 线程，UI 更新必须通过 UiBus。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ota.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <algorithm>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <ctime>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <memory>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <mutex>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "protocol/http/http.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "protocol/ntp/ntp.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "common/time/time.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "cJSON.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_log.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_ota_ops.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/task/task.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "OTA";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::common::ota {

// [初学者导读] OtaMgr::get_instance()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
OtaMgr& OtaMgr::get_instance()
{
    // OTA 全局只允许一个管理器，避免两个下载任务同时写 Flash。
    static OtaMgr instance;
    // 返回 instance 给调用者，表示当前函数的执行结果。
    return instance;
}

// 定义或声明函数 OtaMgr::init：参数在括号里，返回类型写在函数名前面。
bool OtaMgr::init(const std::string& device_id, const std::string& current_version)
{
    // 只准备设备身份、版本和回调槽；真正的网络检查由 check_update() 开始。
    std::lock_guard<std::mutex> lock(mutex_);
    // 判断条件：initialized_ 是否成立。
    if (initialized_)
    {
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 赋值：把右侧 device_id 的结果保存到左侧 device_id_。
    device_id_ = device_id;
    // 赋值：把右侧 current_version 的结果保存到左侧 current_version_。
    current_version_ = current_version;
    // 赋值：把右侧 OtaStatus::IDLE 的结果保存到左侧 status_。
    status_ = OtaStatus::IDLE;
    // 赋值：把右侧 nullptr 的结果保存到左侧 progress_callback_。
    progress_callback_ = nullptr;
    // 赋值：把右侧 nullptr 的结果保存到左侧 status_callback_。
    status_callback_ = nullptr;
    // 赋值：把右侧 nullptr 的结果保存到左侧 complete_callback_。
    complete_callback_ = nullptr;
    // 赋值：把右侧 nullptr 的结果保存到左侧 current_update_task_。
    current_update_task_ = nullptr;
    // 赋值：把右侧 false 的结果保存到左侧 cancelled_。
    cancelled_ = false;
    // 赋值：把右侧 true 的结果保存到左侧 initialized_。
    initialized_ = true;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// [初学者导读] OtaMgr::deinit()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void OtaMgr::deinit()
{
    // 升级中拒绝清理，防止后台任务继续使用已经被置空的状态。
    std::lock_guard<std::mutex> lock(mutex_);
    // 判断条件：!initialized_ 是否成立。
    if (!initialized_)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：status_ == OtaStatus::DOWNLOADING || status_ == OtaStatus::VERIFYING | 是否成立。
    if (status_ == OtaStatus::DOWNLOADING || status_ == OtaStatus::VERIFYING ||
        // 继续上一行的表达式或参数列表：status_ == OtaStatus::CHECKING)。
        status_ == OtaStatus::CHECKING)
    {
        // 调用 ESP_LOGW(TAG, "OTA 升级进行中，无法去初始化");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGW(TAG, "OTA 升级进行中，无法去初始化");
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 赋值：把右侧 false 的结果保存到左侧 initialized_。
    initialized_ = false;
    // 赋值：把右侧 OtaStatus::IDLE 的结果保存到左侧 status_。
    status_ = OtaStatus::IDLE;
    // 赋值：把右侧 nullptr 的结果保存到左侧 progress_callback_。
    progress_callback_ = nullptr;
    // 赋值：把右侧 nullptr 的结果保存到左侧 status_callback_。
    status_callback_ = nullptr;
    // 赋值：把右侧 nullptr 的结果保存到左侧 complete_callback_。
    complete_callback_ = nullptr;
    // 赋值：把右侧 nullptr 的结果保存到左侧 current_update_task_。
    current_update_task_ = nullptr;
    // 赋值：把右侧 false 的结果保存到左侧 cancelled_。
    cancelled_ = false;
}

// [初学者导读] OtaMgr::get_timestamp()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
std::string OtaMgr::get_timestamp() const
{
    // 赋值：把右侧 app::protocol::ntp::NtpMgr::get_instance() 的结果保存到左侧 auto& ntp_mgr。
    auto& ntp_mgr = app::protocol::ntp::NtpMgr::get_instance();
    // 判断条件：ntp_mgr.is_init() && ntp_mgr.get_sync_status() != app::protocol::ntp::SyncStatus::COMPLETED 是否成立。
    if (ntp_mgr.is_init() && ntp_mgr.get_sync_status() != app::protocol::ntp::SyncStatus::COMPLETED)
    {
        // 调用 ESP_LOGW(TAG, "NTP 未同步，使用系统时间");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGW(TAG, "NTP 未同步，使用系统时间");
    }

    // 返回 app::common::time::iso8601_timestamp() 给调用者，表示当前函数的执行结果。
    return app::common::time::iso8601_timestamp();
}

// 函数定义/声明：调用 OtaMgr::build_url()：把服务器地址和 API 路径拼成完整 URL。必须避免重复斜杠，并保留已有查询参数处理规则。。
std::string OtaMgr::build_url(const std::string& server_url, const std::string& path) const
{
    // 统一处理服务器地址和 API 路径中间的斜杠，避免出现双斜杠或漏斜杠。
    std::string url = server_url;
    // 判断条件：!url.empty() && url.back() != '/' 是否成立。
    if (!url.empty() && url.back() != '/')
    {
        // 赋值：把右侧 '/' 的结果保存到左侧 url +。
        url += '/';
    }
    // 判断条件：!path.empty() && path.front() == '/' 是否成立。
    if (!path.empty() && path.front() == '/')
    {
        // 赋值：把右侧 path.substr(1) 的结果保存到左侧 url +。
        url += path.substr(1);
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 赋值：把右侧 path 的结果保存到左侧 url +。
        url += path;
    }
    // 返回 url 给调用者，表示当前函数的执行结果。
    return url;
}

// 函数定义/声明：调用 OtaMgr::handle_error()：统一把状态置为失败并触发最终回调。集中处理错误可以让下载、校验和网络失败行为一致。。
void OtaMgr::handle_error(OtaStatus new_status, const std::string& error_msg)
{
    // 所有失败路径通过这里统一更新状态并通知上层。
    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);
        // 赋值：把右侧 new_status 的结果保存到左侧 status_。
        status_ = new_status;
    }
    // 判断条件：status_callback_ 是否成立。
    if (status_callback_)
    {
        // 调用 status_callback_(new_status);，执行这个函数并处理它的返回值或状态变化。
        status_callback_(new_status);
    }
    // 判断条件：!error_msg.empty() 是否成立。
    if (!error_msg.empty())
    {
        // 调用 ESP_LOGE(TAG, "%s", error_msg.c_str());，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "%s", error_msg.c_str());
    }
}

// 函数定义/声明：调用 OtaMgr::md5_to_string()：把 16 字节 MD5 转换成 32 位十六进制字符串，便于和服务器字段比较。。
std::string OtaMgr::md5_to_string(const unsigned char* md5_bytes) const
{
    // 执行语句：char md5_str[33];；它会更新一次变量、对象或程序状态。
    char md5_str[33];
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (int i = 0; i < 16; i++)
    {
        // 调用 snprintf(md5_str + i * 2, 3, "%02x", md5_bytes[i]);，执行这个函数并处理它的返回值或状态变化。
        snprintf(md5_str + i * 2, 3, "%02x", md5_bytes[i]);
    }
    // 赋值：把右侧 '\0' 的结果保存到左侧 md5_str[32]。
    md5_str[32] = '\0';
    // 返回 std::string(md5_str) 给调用者，表示当前函数的执行结果。
    return std::string(md5_str);
}

// 定义或声明函数 Md5ContextRAII::Md5ContextRAII：参数在括号里，返回类型写在函数名前面。
Md5ContextRAII::Md5ContextRAII()
    // 定义或声明函数 op_：参数在括号里，返回类型写在函数名前面。
    : op_(PSA_HASH_OPERATION_INIT)
    // 定义或声明函数 ready_：参数在括号里，返回类型写在函数名前面。
    , ready_(false)
{
    // 模块私有静态对象 psa_init_once：只在当前文件或函数内共享。
    static std::once_flag psa_init_once;
    // 调用 std::call_once(psa_init_once, []() { (void)psa_crypto_init(); });，执行这个函数并处理它的返回值或状态变化。
    std::call_once(psa_init_once, []() { (void)psa_crypto_init(); });
    // 定义变量 psa_status_t s 并初始化为 psa_hash_setup(&op_, PSA_ALG_MD5)。
    psa_status_t s = psa_hash_setup(&op_, PSA_ALG_MD5);
    // 赋值：把右侧 (s == PSA_SUCCESS) 的结果保存到左侧 ready_。
    ready_ = (s == PSA_SUCCESS);
    // 判断条件：!ready_ 是否成立。
    if (!ready_)
    {
        // 调用 ESP_LOGE(TAG, "psa_hash_setup(MD5) 失败: %d", static_cast<int>(s));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "psa_hash_setup(MD5) 失败: %d", static_cast<int>(s));
    }
}

// 定义或声明函数 Md5ContextRAII::~Md5ContextRAII：参数在括号里，返回类型写在函数名前面。
Md5ContextRAII::~Md5ContextRAII()
{
    // 函数调用：(void)psa_hash_abort(&op_);；执行该函数并处理它的返回值或副作用。
    (void)psa_hash_abort(&op_);
}

// 函数定义/声明：调用 Md5ContextRAII::update()：update() 把一段流数据加入摘要；finish() 输出最终 16 字节。下载回调每收到一段数据就调用一次 update。。
bool Md5ContextRAII::update(const uint8_t* data, size_t len)
{
    // 判断条件：!ready_ 是否成立。
    if (!ready_)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 返回 psa_hash_update(&op_, data, len) == PSA_SUCCESS 给调用者，表示当前函数的执行结果。
    return psa_hash_update(&op_, data, len) == PSA_SUCCESS;
}

// 函数定义/声明：调用 Md5ContextRAII::finish()：update() 把一段流数据加入摘要；finish() 输出最终 16 字节。下载回调每收到一段数据就调用一次 update。。
bool Md5ContextRAII::finish(unsigned char out[16])
{
    // 判断条件：!ready_ 是否成立。
    if (!ready_)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 定义变量 size_t hash_len 并初始化为 0。
    size_t hash_len = 0;
    // 定义变量 psa_status_t s 并初始化为 psa_hash_finish(&op_, out, 16, &hash_len)。
    psa_status_t s = psa_hash_finish(&op_, out, 16, &hash_len);
    // 赋值：把右侧 false 的结果保存到左侧 ready_。
    ready_ = false;
    // 返回 (s == PSA_SUCCESS) && (hash_len == 16U) 给调用者，表示当前函数的执行结果。
    return (s == PSA_SUCCESS) && (hash_len == 16U);
}

// 函数定义/声明：调用 OtaMgr::build_base_json_message()：创建所有 OTA 请求共有的字段，例如消息类型、设备 ID 和时间。它减少每个构造函数重复拼 JSON。。
std::string OtaMgr::build_base_json_message(const std::string& type) const
{
    // 执行语句：JsonRAII json;；它会更新一次变量、对象或程序状态。
    JsonRAII json;
    // 判断条件：!json.is_valid() 是否成立。
    if (!json.is_valid())
    {
        // 返回 "" 给调用者，表示当前函数的执行结果。
        return "";
    }

    // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
    cJSON_AddStringToObject(json.get(), "type", type.c_str());
    // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
    cJSON_AddStringToObject(json.get(), "from", device_id_.c_str());
    // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
    cJSON_AddStringToObject(json.get(), "to", "ota_server");
    // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
    cJSON_AddStringToObject(json.get(), "timestamp", get_timestamp().c_str());

    // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
    JsonStringRAII json_str(cJSON_Print(json.get()));
    // 返回 json_str.is_valid() ? json_str.to_string() : std::string() 给调用者，表示当前函数的执行结果。
    return json_str.is_valid() ? json_str.to_string() : std::string();
}

// 函数定义/声明：调用 OtaMgr::build_check_update_message()：构造检查更新请求。典型内容包含设备 ID、当前版本和消息类型。。
std::string OtaMgr::build_check_update_message() const
{
    // 定义变量并调用函数初始化：调用 build_base_json_message("check_update")：创建所有 OTA 请求共有的字段，例如消息类型、设备 ID 和时间。它减少每个构造函数重复拼 JSON。，结果保存到 std::string base_json。
    std::string base_json = build_base_json_message("check_update");
    // 访问对象成员：JsonRAII json(base_json.c_str());；读取或修改这个对象里的成员状态。
    JsonRAII json(base_json.c_str());
    // 判断条件：!json.is_valid() 是否成立。
    if (!json.is_valid())
    {
        // 执行语句：JsonRAII fallback_json;；它会更新一次变量、对象或程序状态。
        JsonRAII fallback_json;
        // 判断条件：!fallback_json.is_valid() 是否成立。
        if (!fallback_json.is_valid())
        {
            // 返回 "" 给调用者，表示当前函数的执行结果。
            return "";
        }
        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        cJSON_AddStringToObject(fallback_json.get(), "type", "check_update");
        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        cJSON_AddStringToObject(fallback_json.get(), "from", device_id_.c_str());
        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        cJSON_AddStringToObject(fallback_json.get(), "to", "ota_server");
        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        cJSON_AddStringToObject(fallback_json.get(), "current_version", current_version_.c_str());
        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        cJSON_AddStringToObject(fallback_json.get(), "timestamp", get_timestamp().c_str());

        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        JsonStringRAII json_str(cJSON_Print(fallback_json.get()));
        // 返回 json_str.is_valid() ? json_str.to_string() : std::string() 给调用者，表示当前函数的执行结果。
        return json_str.is_valid() ? json_str.to_string() : std::string();
    }

    // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
    cJSON_AddStringToObject(json.get(), "current_version", current_version_.c_str());

    // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
    JsonStringRAII json_str(cJSON_Print(json.get()));
    // 返回 json_str.is_valid() ? json_str.to_string() : std::string() 给调用者，表示当前函数的执行结果。
    return json_str.is_valid() ? json_str.to_string() : std::string();
}

// 函数定义/声明：调用 OtaMgr::build_get_firmware_info_message()：构造获取固件详情请求。它通常用于在下载前读取大小、MD5 和说明。。
std::string OtaMgr::build_get_firmware_info_message() const
{
    // 返回结果：调用 build_base_json_message("get_firmware_info")：创建所有 OTA 请求共有的字段，例如消息类型、设备 ID 和时间。它减少每个构造函数重复拼 JSON。。
    return build_base_json_message("get_firmware_info");
}

// 函数定义/声明：调用 OtaMgr::build_request_firmware_message()：构造请求下载某个固件的消息，可能包含版本和服务器返回的标识。。
std::string OtaMgr::build_request_firmware_message(const FirmwareInfo& info) const
{
    // 定义变量并调用函数初始化：调用 build_base_json_message("request_firmware")：创建所有 OTA 请求共有的字段，例如消息类型、设备 ID 和时间。它减少每个构造函数重复拼 JSON。，结果保存到 std::string base_json。
    std::string base_json = build_base_json_message("request_firmware");
    // 访问对象成员：JsonRAII json(base_json.c_str());；读取或修改这个对象里的成员状态。
    JsonRAII json(base_json.c_str());
    // 判断条件：!json.is_valid() 是否成立。
    if (!json.is_valid())
    {
        // 执行语句：JsonRAII fallback_json;；它会更新一次变量、对象或程序状态。
        JsonRAII fallback_json;
        // 判断条件：!fallback_json.is_valid() 是否成立。
        if (!fallback_json.is_valid())
        {
            // 返回 "" 给调用者，表示当前函数的执行结果。
            return "";
        }
        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        cJSON_AddStringToObject(fallback_json.get(), "type", "request_firmware");
        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        cJSON_AddStringToObject(fallback_json.get(), "from", device_id_.c_str());
        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        cJSON_AddStringToObject(fallback_json.get(), "to", "ota_server");
        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        cJSON_AddStringToObject(fallback_json.get(), "timestamp", get_timestamp().c_str());

        // 执行语句：JsonRAII data;；它会更新一次变量、对象或程序状态。
        JsonRAII data;
        // 判断条件：data.is_valid() 是否成立。
        if (data.is_valid())
        {
            // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
            cJSON_AddStringToObject(data.get(), "name", info.name.c_str());
            // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
            cJSON_AddStringToObject(data.get(), "target_version", info.version.c_str());
            // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
            cJSON_AddStringToObject(data.get(), "md5", info.md5.c_str());
            // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
            cJSON_AddItemToObject(fallback_json.get(), "data", data.release());
        }

        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        JsonStringRAII json_str(cJSON_Print(fallback_json.get()));
        // 返回 json_str.is_valid() ? json_str.to_string() : std::string() 给调用者，表示当前函数的执行结果。
        return json_str.is_valid() ? json_str.to_string() : std::string();
    }

    // 执行语句：JsonRAII data;；它会更新一次变量、对象或程序状态。
    JsonRAII data;
    // 判断条件：data.is_valid() 是否成立。
    if (data.is_valid())
    {
        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        cJSON_AddStringToObject(data.get(), "name", info.name.c_str());
        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        cJSON_AddStringToObject(data.get(), "target_version", info.version.c_str());
        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        cJSON_AddStringToObject(data.get(), "md5", info.md5.c_str());
        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        cJSON_AddItemToObject(json.get(), "data", data.release());
    }

    // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
    JsonStringRAII json_str(cJSON_Print(json.get()));
    // 返回 json_str.is_valid() ? json_str.to_string() : std::string() 给调用者，表示当前函数的执行结果。
    return json_str.is_valid() ? json_str.to_string() : std::string();
}

// 函数定义/声明：调用 OtaMgr::build_report_status_message()：构造上报状态 JSON。服务器或后台系统可以知道设备更新到了哪一步。。
std::string OtaMgr::build_report_status_message(uint8_t status, uint8_t progress) const
{
    // 定义变量并调用函数初始化：调用 build_base_json_message("report_status")：创建所有 OTA 请求共有的字段，例如消息类型、设备 ID 和时间。它减少每个构造函数重复拼 JSON。，结果保存到 std::string base_json。
    std::string base_json = build_base_json_message("report_status");
    // 访问对象成员：JsonRAII json(base_json.c_str());；读取或修改这个对象里的成员状态。
    JsonRAII json(base_json.c_str());
    // 判断条件：!json.is_valid() 是否成立。
    if (!json.is_valid())
    {
        // 执行语句：JsonRAII fallback_json;；它会更新一次变量、对象或程序状态。
        JsonRAII fallback_json;
        // 判断条件：!fallback_json.is_valid() 是否成立。
        if (!fallback_json.is_valid())
        {
            // 返回 "" 给调用者，表示当前函数的执行结果。
            return "";
        }
        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        cJSON_AddStringToObject(fallback_json.get(), "type", "report_status");
        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        cJSON_AddStringToObject(fallback_json.get(), "from", device_id_.c_str());
        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        cJSON_AddStringToObject(fallback_json.get(), "to", "ota_server");
        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        cJSON_AddStringToObject(fallback_json.get(), "timestamp", get_timestamp().c_str());

        // 执行语句：JsonRAII data;；它会更新一次变量、对象或程序状态。
        JsonRAII data;
        // 判断条件：data.is_valid() 是否成立。
        if (data.is_valid())
        {
            // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
            cJSON_AddNumberToObject(data.get(), "status", status);
            // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
            cJSON_AddNumberToObject(data.get(), "progress", progress);
            // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
            cJSON_AddStringToObject(data.get(), "current_version", current_version_.c_str());
            // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
            cJSON_AddItemToObject(fallback_json.get(), "data", data.release());
        }

        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        JsonStringRAII json_str(cJSON_Print(fallback_json.get()));
        // 返回 json_str.is_valid() ? json_str.to_string() : std::string() 给调用者，表示当前函数的执行结果。
        return json_str.is_valid() ? json_str.to_string() : std::string();
    }

    // 执行语句：JsonRAII data;；它会更新一次变量、对象或程序状态。
    JsonRAII data;
    // 判断条件：data.is_valid() 是否成立。
    if (data.is_valid())
    {
        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        cJSON_AddNumberToObject(data.get(), "status", status);
        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        cJSON_AddNumberToObject(data.get(), "progress", progress);
        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        cJSON_AddStringToObject(data.get(), "current_version", current_version_.c_str());
        // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        cJSON_AddItemToObject(json.get(), "data", data.release());
    }

    // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
    JsonStringRAII json_str(cJSON_Print(json.get()));
    // 返回 json_str.is_valid() ? json_str.to_string() : std::string() 给调用者，表示当前函数的执行结果。
    return json_str.is_valid() ? json_str.to_string() : std::string();
}

// 函数定义/声明：调用 OtaMgr::parse_reply_update()：解析检查更新响应： respond 表示是否有更新或服务器响应代码。 downloadurl 是后续下载地址。。
bool OtaMgr::parse_reply_update(const std::string& json, int& respond, std::string& download_url)
{
    // 访问对象成员：JsonRAII root(json.c_str());；读取或修改这个对象里的成员状态。
    JsonRAII root(json.c_str());
    // 判断条件：!root.is_valid() 是否成立。
    if (!root.is_valid())
    {
        // 调用 ESP_LOGE(TAG, "JSON 解析失败: %s", cJSON_GetErrorPtr());，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "JSON 解析失败: %s", cJSON_GetErrorPtr());
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
    cJSON* respond_item = cJSON_GetObjectItem(root.get(), "respond");
    // 判断条件：cJSON_IsNumber(respond_item) 是否成立。
    if (cJSON_IsNumber(respond_item))
    {
        // 赋值：把右侧 respond_item->valueint 的结果保存到左侧 respond。
        respond = respond_item->valueint;
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
    cJSON* url_item = cJSON_GetObjectItem(root.get(), "download_url");
    // 判断条件：cJSON_IsString(url_item) 是否成立。
    if (cJSON_IsString(url_item))
    {
        // 赋值：把右侧 url_item->valuestring 的结果保存到左侧 download_url。
        download_url = url_item->valuestring;
    }

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数定义/声明：调用 OtaMgr::parse_firmware_info()：解析版本、名称、大小、说明、MD5 和时间。特别要检查 size 是否为正、MD5 格式是否合理。。
bool OtaMgr::parse_firmware_info(const std::string& json, FirmwareInfo& info)
{
    // 访问对象成员：JsonRAII root(json.c_str());；读取或修改这个对象里的成员状态。
    JsonRAII root(json.c_str());
    // 判断条件：!root.is_valid() 是否成立。
    if (!root.is_valid())
    {
        // 调用 ESP_LOGE(TAG, "JSON 解析失败: %s", cJSON_GetErrorPtr());，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "JSON 解析失败: %s", cJSON_GetErrorPtr());
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
    cJSON* file_item = cJSON_GetObjectItem(root.get(), "file");
    // 判断条件：!file_item || !cJSON_IsObject(file_item) 是否成立。
    if (!file_item || !cJSON_IsObject(file_item))
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 cJSON_GetObjectItem(file_item, "version") 的结果保存到左侧 cJSON* version_item。
    cJSON* version_item = cJSON_GetObjectItem(file_item, "version");
    // 判断条件：cJSON_IsString(version_item) 是否成立。
    if (cJSON_IsString(version_item))
    {
        // 赋值：把右侧 version_item->valuestring 的结果保存到左侧 info.version。
        info.version = version_item->valuestring;
    }

    // 赋值：把右侧 cJSON_GetObjectItem(file_item, "name") 的结果保存到左侧 cJSON* name_item。
    cJSON* name_item = cJSON_GetObjectItem(file_item, "name");
    // 判断条件：cJSON_IsString(name_item) 是否成立。
    if (cJSON_IsString(name_item))
    {
        // 赋值：把右侧 name_item->valuestring 的结果保存到左侧 info.name。
        info.name = name_item->valuestring;
    }

    // 赋值：把右侧 cJSON_GetObjectItem(file_item, "size") 的结果保存到左侧 cJSON* size_item。
    cJSON* size_item = cJSON_GetObjectItem(file_item, "size");
    // 判断条件：cJSON_IsNumber(size_item) 是否成立。
    if (cJSON_IsNumber(size_item))
    {
        // 赋值：把右侧 (size_t)size_item->valueint 的结果保存到左侧 info.size。
        info.size = (size_t)size_item->valueint;
    }

    // 赋值：把右侧 cJSON_GetObjectItem(file_item, "info") 的结果保存到左侧 cJSON* info_item。
    cJSON* info_item = cJSON_GetObjectItem(file_item, "info");
    // 判断条件：cJSON_IsString(info_item) 是否成立。
    if (cJSON_IsString(info_item))
    {
        // 赋值：把右侧 info_item->valuestring 的结果保存到左侧 info.info。
        info.info = info_item->valuestring;
    }

    // 赋值：把右侧 cJSON_GetObjectItem(file_item, "md5") 的结果保存到左侧 cJSON* md5_item。
    cJSON* md5_item = cJSON_GetObjectItem(file_item, "md5");
    // 判断条件：cJSON_IsString(md5_item) 是否成立。
    if (cJSON_IsString(md5_item))
    {
        // 赋值：把右侧 md5_item->valuestring 的结果保存到左侧 info.md5。
        info.md5 = md5_item->valuestring;
    }

    // 赋值：把右侧 cJSON_GetObjectItem(file_item, "time") 的结果保存到左侧 cJSON* time_item。
    cJSON* time_item = cJSON_GetObjectItem(file_item, "time");
    // 判断条件：cJSON_IsString(time_item) 是否成立。
    if (cJSON_IsString(time_item))
    {
        // 赋值：把右侧 time_item->valuestring 的结果保存到左侧 info.time。
        info.time = time_item->valuestring;
    }

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数定义/声明：调用 OtaMgr::parse_error()：解析服务器错误结构和错误文字，供日志或 UI 提示。。
bool OtaMgr::parse_error(const std::string& json, int& code, std::string& message)
{
    // 访问对象成员：JsonRAII root(json.c_str());；读取或修改这个对象里的成员状态。
    JsonRAII root(json.c_str());
    // 判断条件：!root.is_valid() 是否成立。
    if (!root.is_valid())
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
    cJSON* type_item = cJSON_GetObjectItem(root.get(), "type");
    // 判断条件：!type_item || !cJSON_IsString(type_item) || strcmp(type_item->valuestring, "error") != 0 是否成立。
    if (!type_item || !cJSON_IsString(type_item) || strcmp(type_item->valuestring, "error") != 0)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
    cJSON* data_item = cJSON_GetObjectItem(root.get(), "data");
    // 判断条件：!data_item || !cJSON_IsObject(data_item) 是否成立。
    if (!data_item || !cJSON_IsObject(data_item))
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 cJSON_GetObjectItem(data_item, "code") 的结果保存到左侧 cJSON* code_item。
    cJSON* code_item = cJSON_GetObjectItem(data_item, "code");
    // 判断条件：cJSON_IsNumber(code_item) 是否成立。
    if (cJSON_IsNumber(code_item))
    {
        // 赋值：把右侧 code_item->valueint 的结果保存到左侧 code。
        code = code_item->valueint;
    }

    // 赋值：把右侧 cJSON_GetObjectItem(data_item, "message") 的结果保存到左侧 cJSON* message_item。
    cJSON* message_item = cJSON_GetObjectItem(data_item, "message");
    // 判断条件：cJSON_IsString(message_item) 是否成立。
    if (cJSON_IsString(message_item))
    {
        // 赋值：把右侧 message_item->valuestring 的结果保存到左侧 message。
        message = message_item->valuestring;
    }

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数定义/声明：调用 OtaMgr::check_update()：OtaMgr::checkupdate()。
bool OtaMgr::check_update(const std::string& server_url, int32_t timeout_ms)
{
    // 请求 api/ota/check。长超时版本不能在 UI 线程调用。
    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);
        // 判断条件：!initialized_ 是否成立。
        if (!initialized_)
        {
            // 调用 ESP_LOGE(TAG, "OTA 管理器未初始化");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "OTA 管理器未初始化");
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }

        // 判断条件：status_ == OtaStatus::DOWNLOADING || status_ == OtaStatus::VERIFYING 是否成立。
        if (status_ == OtaStatus::DOWNLOADING || status_ == OtaStatus::VERIFYING)
        {
            // 调用 ESP_LOGE(TAG, "OTA 升级进行中，无法检查更新");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "OTA 升级进行中，无法检查更新");
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }

        // 赋值：把右侧 OtaStatus::CHECKING 的结果保存到左侧 status_。
        status_ = OtaStatus::CHECKING;
    }

    // 判断条件：status_callback_ 是否成立。
    if (status_callback_)
    {
        // 调用 status_callback_(OtaStatus::CHECKING);，执行这个函数并处理它的返回值或状态变化。
        status_callback_(OtaStatus::CHECKING);
    }

    // 赋值：把右侧 app::protocol::http::HttpMgr::get_instance() 的结果保存到左侧 auto& http_mgr。
    auto& http_mgr = app::protocol::http::HttpMgr::get_instance();
    // 定义变量并调用函数初始化：调用 build_url(server_url,"api/ota/check")：把服务器地址和 API 路径拼成完整 URL。必须避免重复斜杠，并保留已有查询参数处理规则。，结果保存到 std::string url。
    std::string url = build_url(server_url, "api/ota/check");
    // 定义变量并调用函数初始化：调用 build_check_update_message()：构造检查更新请求。典型内容包含设备 ID、当前版本和消息类型。，结果保存到 std::string request_body。
    std::string request_body = build_check_update_message();

    // 执行语句：app::protocol::http::HttpResponse response;；它会更新一次变量、对象或程序状态。
    app::protocol::http::HttpResponse response;
    // 定义变量 bool http_result 并初始化为 http_mgr.post(url, request_body, response, timeout_ms)。
    bool http_result = http_mgr.post(url, request_body, response, timeout_ms);

    // 判断条件：!http_result 是否成立。
    if (!http_result)
    {
        // 调用 handle_error(OtaStatus::FAILED, "检查更新失败：HTTP 请求失败");，执行这个函数并处理它的返回值或状态变化。
        handle_error(OtaStatus::FAILED, "检查更新失败：HTTP 请求失败");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 判断条件：response.status_code != app::protocol::http::HttpStatus::OK 是否成立。
    if (response.status_code != app::protocol::http::HttpStatus::OK)
    {
        // 执行语句：char error_buf[64];；它会更新一次变量、对象或程序状态。
        char error_buf[64];
        // 调用 snprintf(error_buf, sizeof(error_buf), "检查更新失败：HTTP 状态码 %d",，执行这个函数并处理它的返回值或状态变化。
        snprintf(error_buf, sizeof(error_buf), "检查更新失败：HTTP 状态码 %d",
                 // 继续上一行的表达式或参数列表：static_cast<int>(response.status_code_int));。
                 static_cast<int>(response.status_code_int));
        // 调用 handle_error(OtaStatus::FAILED, error_buf);，执行这个函数并处理它的返回值或状态变化。
        handle_error(OtaStatus::FAILED, error_buf);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 访问对象成员：std::string body_str(response.body.begin(), response.body.end());；读取或修改这个对象里的成员状态。
    std::string body_str(response.body.begin(), response.body.end());

    // 执行语句：int error_code;；它会更新一次变量、对象或程序状态。
    int error_code;
    // 执行语句：std::string error_message;；它会更新一次变量、对象或程序状态。
    std::string error_message;
    // 判断条件：parse_error(body_str, error_code, error_message) 是否成立。
    if (parse_error(body_str, error_code, error_message))
    {
        // 执行语句：char error_buf[128];；它会更新一次变量、对象或程序状态。
        char error_buf[128];
        // 调用 snprintf(error_buf, sizeof(error_buf), "检查更新失败：%s (错误码: %d)",，执行这个函数并处理它的返回值或状态变化。
        snprintf(error_buf, sizeof(error_buf), "检查更新失败：%s (错误码: %d)",
                 // 继续上一行的表达式或参数列表：error_message.c_str(), error_code);。
                 error_message.c_str(), error_code);
        // 调用 handle_error(OtaStatus::FAILED, error_buf);，执行这个函数并处理它的返回值或状态变化。
        handle_error(OtaStatus::FAILED, error_buf);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 执行语句：int respond;；它会更新一次变量、对象或程序状态。
    int respond;
    // 执行语句：std::string download_url;；它会更新一次变量、对象或程序状态。
    std::string download_url;
    // 判断条件：!parse_reply_update(body_str, respond, download_url) 是否成立。
    if (!parse_reply_update(body_str, respond, download_url))
    {
        // 调用 handle_error(OtaStatus::FAILED, "检查更新失败：无法解析服务器响应");，执行这个函数并处理它的返回值或状态变化。
        handle_error(OtaStatus::FAILED, "检查更新失败：无法解析服务器响应");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);
        // 赋值：把右侧 OtaStatus::IDLE 的结果保存到左侧 status_。
        status_ = OtaStatus::IDLE;
    }
    // 判断条件：status_callback_ 是否成立。
    if (status_callback_)
    {
        // 调用 status_callback_(OtaStatus::IDLE);，执行这个函数并处理它的返回值或状态变化。
        status_callback_(OtaStatus::IDLE);
    }
    // 返回 respond == 1 || respond == 2 给调用者，表示当前函数的执行结果。
    return respond == 1 || respond == 2;
}

// 函数调用：bool OtaMgr::get_firmware_info(const std::string& server_url, FirmwareInfo& info,；执行该函数并处理它的返回值或副作用。
bool OtaMgr::get_firmware_info(const std::string& server_url, FirmwareInfo& info,
                               // 继续上一行的表达式或参数列表：int32_t timeout_ms)。
                               int32_t timeout_ms)
{
    // 下载前先读取服务器返回的版本、大小、MD5 和说明。
    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);

        // 判断条件：!initialized_ 是否成立。
        if (!initialized_)
        {
            // 调用 ESP_LOGE(TAG, "OTA 管理器未初始化");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "OTA 管理器未初始化");
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }
    }

    // 赋值：把右侧 app::protocol::http::HttpMgr::get_instance() 的结果保存到左侧 auto& http_mgr。
    auto& http_mgr = app::protocol::http::HttpMgr::get_instance();

    // 定义变量并调用函数初始化：调用 build_url(server_url,"api/ota/info")：把服务器地址和 API 路径拼成完整 URL。必须避免重复斜杠，并保留已有查询参数处理规则。，结果保存到 std::string url。
    std::string url = build_url(server_url, "api/ota/info");
    // 定义变量并调用函数初始化：调用 build_get_firmware_info_message()：构造获取固件详情请求。它通常用于在下载前读取大小、MD5 和说明。，结果保存到 std::string request_body。
    std::string request_body = build_get_firmware_info_message();

    // 执行语句：app::protocol::http::HttpResponse response;；它会更新一次变量、对象或程序状态。
    app::protocol::http::HttpResponse response;
    // 判断条件：!http_mgr.post(url, request_body, response, timeout_ms) 是否成立。
    if (!http_mgr.post(url, request_body, response, timeout_ms))
    {
        // 调用 ESP_LOGE(TAG, "获取固件信息失败：HTTP 请求失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "获取固件信息失败：HTTP 请求失败");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 判断条件：response.status_code != app::protocol::http::HttpStatus::OK 是否成立。
    if (response.status_code != app::protocol::http::HttpStatus::OK)
    {
        // 调用 ESP_LOGE(TAG, "获取固件信息失败：HTTP 状态码 %d",，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "获取固件信息失败：HTTP 状态码 %d",
                 // 继续上一行的表达式或参数列表：static_cast<int>(response.status_code_int));。
                 static_cast<int>(response.status_code_int));
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 访问对象成员：std::string body_str(response.body.begin(), response.body.end());；读取或修改这个对象里的成员状态。
    std::string body_str(response.body.begin(), response.body.end());

    // 执行语句：int error_code;；它会更新一次变量、对象或程序状态。
    int error_code;
    // 执行语句：std::string error_message;；它会更新一次变量、对象或程序状态。
    std::string error_message;
    // 判断条件：parse_error(body_str, error_code, error_message) 是否成立。
    if (parse_error(body_str, error_code, error_message))
    {
        // 调用 ESP_LOGE(TAG, "获取固件信息失败：%s (错误码: %d)", error_message.c_str(), error_code);，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "获取固件信息失败：%s (错误码: %d)", error_message.c_str(), error_code);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 判断条件：!parse_firmware_info(body_str, info) 是否成立。
    if (!parse_firmware_info(body_str, info))
    {
        // 调用 ESP_LOGE(TAG, "获取固件信息失败：无法解析服务器响应");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "获取固件信息失败：无法解析服务器响应");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数调用：bool OtaMgr::start_update(const std::string& server_url, const FirmwareInfo& firmware_info,；执行该函数并处理它的返回值或副作用。
bool OtaMgr::start_update(const std::string& server_url, const FirmwareInfo& firmware_info,
                          // 继续上一行的表达式或参数列表：int32_t timeout_ms)。
                          int32_t timeout_ms)
{
    // 创建 ota_update 后台任务；返回成功只表示任务已启动，不表示固件已完成。
    ESP_LOGD(TAG, "开始启动 OTA 升级，固件版本: %s", firmware_info.version.c_str());

    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);

        // 判断条件：!initialized_ 是否成立。
        if (!initialized_)
        {
            // 调用 ESP_LOGE(TAG, "OTA 管理器未初始化");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "OTA 管理器未初始化");
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }

        // 判断条件：status_ == OtaStatus::DOWNLOADING || status_ == OtaStatus::VERIFYING 是否成立。
        if (status_ == OtaStatus::DOWNLOADING || status_ == OtaStatus::VERIFYING)
        {
            // 调用 ESP_LOGE(TAG, "OTA 升级已在进行中");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "OTA 升级已在进行中");
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }
    }

    // 定义变量 auto task_config 并初始化为 app::sys::task::Cfg()。
    auto task_config = app::sys::task::Cfg();
    // 赋值：把右侧 "ota_update" 的结果保存到左侧 task_config.name。
    task_config.name = "ota_update";
    // 赋值：把右侧 16 * 1024 的结果保存到左侧 task_config.stack_size。
    task_config.stack_size = 16 * 1024;
    // 赋值：把右侧 app::sys::task::Priority::HIGH 的结果保存到左侧 task_config.priority。
    task_config.priority = app::sys::task::Priority::HIGH;
    // 赋值：把右侧 -1 的结果保存到左侧 task_config.core_id。
    task_config.core_id = -1;
    // 赋值：把右侧 true 的结果保存到左侧 task_config.use_psram。
    task_config.use_psram = true;

    // struct UpdateContext：把相关数据字段组合在一起，表示项目中的一个概念。
    struct UpdateContext
    {
        // 执行语句：std::string server_url;；它会更新一次变量、对象或程序状态。
        std::string server_url;
        // 执行语句：FirmwareInfo firmware_info;；它会更新一次变量、对象或程序状态。
        FirmwareInfo firmware_info;
        // 执行语句：int32_t timeout_ms;；它会更新一次变量、对象或程序状态。
        int32_t timeout_ms;
        // 执行语句：OtaMgr* manager;；它会更新一次变量、对象或程序状态。
        OtaMgr* manager;
        // 执行语句：ProgressCallback progress_callback;；它会更新一次变量、对象或程序状态。
        ProgressCallback progress_callback;
        // 执行语句：CompleteCallback complete_callback;；它会更新一次变量、对象或程序状态。
        CompleteCallback complete_callback;

        // 调用 UpdateContext(const std::string& url, const FirmwareInfo& info, int32_t timeout,，执行这个函数并处理它的返回值或状态变化。
        UpdateContext(const std::string& url, const FirmwareInfo& info, int32_t timeout,
                      // 继续上一行的表达式或参数列表：OtaMgr* mgr, const ProgressCallback& prog_cb, const CompleteCallback& comp_cb)。
                      OtaMgr* mgr, const ProgressCallback& prog_cb, const CompleteCallback& comp_cb)
            // 定义或声明函数 server_url：参数在括号里，返回类型写在函数名前面。
            : server_url(url)
            // 定义或声明函数 firmware_info：参数在括号里，返回类型写在函数名前面。
            , firmware_info(info)
            // 定义或声明函数 timeout_ms：参数在括号里，返回类型写在函数名前面。
            , timeout_ms(timeout)
            // 定义或声明函数 manager：参数在括号里，返回类型写在函数名前面。
            , manager(mgr)
            // 定义或声明函数 progress_callback：参数在括号里，返回类型写在函数名前面。
            , progress_callback(prog_cb)
            // 定义或声明函数 complete_callback：参数在括号里，返回类型写在函数名前面。
            , complete_callback(comp_cb)
        {
        }
    };

    // 定义变量 auto ctx 并初始化为 std::make_shared<UpdateContext>(server_url, firmware_info, timeout_ms, this。
    auto ctx = std::make_shared<UpdateContext>(server_url, firmware_info, timeout_ms, this,
                                               // 继续上一行的表达式或参数列表：progress_callback_, complete_callback_);。
                                               progress_callback_, complete_callback_);

    // 定义变量 auto ctx_ptr 并初始化为 std::make_unique<std::shared_ptr<UpdateContext>>(ctx)。
    auto ctx_ptr = std::make_unique<std::shared_ptr<UpdateContext>>(ctx);

    // 开始代码块：下面是这个函数、分支或循环的主体。
    app::sys::task::Task::Func task_function = [](void* param) {
        // 函数调用：std::unique_ptr<std::shared_ptr<UpdateContext>> ctx_owner(；执行该函数并处理它的返回值或副作用。
        std::unique_ptr<std::shared_ptr<UpdateContext>> ctx_owner(
            // 继续上一行的表达式或参数列表：static_cast<std::shared_ptr<UpdateContext>*>(param));。
            static_cast<std::shared_ptr<UpdateContext>*>(param));
        // 定义变量 auto ctx 并初始化为 *ctx_owner。
        auto ctx = *ctx_owner;
        // 赋值：把右侧 *ctx->manager 的结果保存到左侧 auto& manager。
        auto& manager = *ctx->manager;

        // 调用 app::sys::task::TaskMgr::delay_ms(10)：调用 vTaskDelay()，这是让出 CPU 的阻塞延时。一千毫秒延时期间，任务不占 CPU，调度器可以运行别人。。
        app::sys::task::TaskMgr::delay_ms(10);

        {
            // 调用 lock(manager.mutex_)：为什么使用 trylock() 而不是普通 lock()。
            std::lock_guard<std::mutex> lock(manager.mutex_);
            // 赋值：把右侧 app::sys::task::TaskMgr::get_instance() 的结果保存到左侧 auto& task_mgr。
            auto& task_mgr = app::sys::task::TaskMgr::get_instance();
            // 定义变量并调用函数初始化：调用 find("ota_update")：find(id) 根据 AppId 找对象；entryat(idx) 根据网格索引返回 {id, title, iconcp} 这样的轻量条目。，结果保存到 TaskHandle_t task_handle。
            TaskHandle_t task_handle = task_mgr.find("ota_update");
            // 判断条件：manager.current_update_task_ == nullptr 是否成立。
            if (manager.current_update_task_ == nullptr)
            {
                // 赋值：把右侧 task_handle 的结果保存到左侧 manager.current_update_task_。
                manager.current_update_task_ = task_handle;
            }
            // 赋值：把右侧 false 的结果保存到左侧 manager.cancelled_。
            manager.cancelled_ = false;
        }

        // 访问对象成员：manager.update_status(OtaStatus::DOWNLOADING);；读取或修改这个对象里的成员状态。
        manager.update_status(OtaStatus::DOWNLOADING);

        // 只读约束：这个值或对象在当前位置不能被修改。
        const esp_partition_t* update_partition = nullptr;
        // 定义变量 esp_ota_handle_t ota_handle 并初始化为 0。
        esp_ota_handle_t ota_handle = 0;

        // 只读约束：这个值或对象在当前位置不能被修改。
        const esp_partition_t* running = esp_ota_get_running_partition();
        // 执行语句：esp_ota_img_states_t ota_state;；它会更新一次变量、对象或程序状态。
        esp_ota_img_states_t ota_state;
        // 判断条件：esp_ota_get_state_partition(running, &ota_state) == ESP_OK 是否成立。
        if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK)
        {
            // 判断条件：ota_state == ESP_OTA_IMG_PENDING_VERIFY 是否成立。
            if (ota_state == ESP_OTA_IMG_PENDING_VERIFY)
            {
                // 调用 ESP_LOGW(TAG, "检测到待验证的 OTA 镜像，标记为有效");，执行这个函数并处理它的返回值或状态变化。
                ESP_LOGW(TAG, "检测到待验证的 OTA 镜像，标记为有效");
                // 调用 esp_ota_mark_app_valid_cancel_rollback();，执行这个函数并处理它的返回值或状态变化。
                esp_ota_mark_app_valid_cancel_rollback();
            }
        }

        // 判断条件：running->subtype == ESP_PARTITION_SUBTYPE_APP_OTA_0 是否成立。
        if (running->subtype == ESP_PARTITION_SUBTYPE_APP_OTA_0)
        {
            // 函数调用：update_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP,；执行该函数并处理它的返回值或副作用。
            update_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP,
                                                        // 继续上一行的表达式或参数列表：ESP_PARTITION_SUBTYPE_APP_OTA_1, nullptr);。
                                                        ESP_PARTITION_SUBTYPE_APP_OTA_1, nullptr);
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 函数调用：update_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP,；执行该函数并处理它的返回值或副作用。
            update_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP,
                                                        // 继续上一行的表达式或参数列表：ESP_PARTITION_SUBTYPE_APP_OTA_0, nullptr);。
                                                        ESP_PARTITION_SUBTYPE_APP_OTA_0, nullptr);
        }

        // 判断条件：update_partition == nullptr 是否成立。
        if (update_partition == nullptr)
        {
            // 调用 ESP_LOGE(TAG, "未找到 OTA 分区");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "未找到 OTA 分区");
            // 访问对象成员：manager.update_status(OtaStatus::FAILED);；读取或修改这个对象里的成员状态。
            manager.update_status(OtaStatus::FAILED);
            // 判断条件：ctx->complete_callback 是否成立。
            if (ctx->complete_callback)
            {
                // 访问对象成员：ctx->complete_callback(false, "未找到 OTA 分区");；读取或修改这个对象里的成员状态。
                ctx->complete_callback(false, "未找到 OTA 分区");
            }
            {
                // 调用 lock(manager.mutex_)：为什么使用 trylock() 而不是普通 lock()。
                std::lock_guard<std::mutex> lock(manager.mutex_);
                // 赋值：把右侧 nullptr 的结果保存到左侧 manager.current_update_task_。
                manager.current_update_task_ = nullptr;
            }
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }

        // 调用 ESP_LOGD(TAG, "使用 OTA 分区：%s, 偏移: 0x%08x, 大小: %u", update_partition->label,，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGD(TAG, "使用 OTA 分区：%s, 偏移: 0x%08x, 大小: %u", update_partition->label,
                 // 继续上一行的表达式或参数列表：(unsigned int)update_partition->address, (unsigned int)update_partition->size);。
                 (unsigned int)update_partition->address, (unsigned int)update_partition->size);

        // 定义变量 esp_err_t err 并初始化为 esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle)。
        esp_err_t err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
        // 判断条件：err != ESP_OK 是否成立。
        if (err != ESP_OK)
        {
            // 调用 ESP_LOGE(TAG, "esp_ota_begin 失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "esp_ota_begin 失败: %s", esp_err_to_name(err));
            // 访问对象成员：manager.update_status(OtaStatus::FAILED);；读取或修改这个对象里的成员状态。
            manager.update_status(OtaStatus::FAILED);
            // 判断条件：ctx->complete_callback 是否成立。
            if (ctx->complete_callback)
            {
                // 执行语句：char error_buf[128];；它会更新一次变量、对象或程序状态。
                char error_buf[128];
                // 调用 snprintf(error_buf, sizeof(error_buf), "esp_ota_begin 失败: %s",，执行这个函数并处理它的返回值或状态变化。
                snprintf(error_buf, sizeof(error_buf), "esp_ota_begin 失败: %s",
                         // 继续上一行的表达式或参数列表：esp_err_to_name(err));。
                         esp_err_to_name(err));
                // 访问对象成员：ctx->complete_callback(false, error_buf);；读取或修改这个对象里的成员状态。
                ctx->complete_callback(false, error_buf);
            }
            {
                // 调用 lock(manager.mutex_)：为什么使用 trylock() 而不是普通 lock()。
                std::lock_guard<std::mutex> lock(manager.mutex_);
                // 赋值：把右侧 nullptr 的结果保存到左侧 manager.current_update_task_。
                manager.current_update_task_ = nullptr;
            }
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }

        // 定义变量并调用函数初始化：调用 build_url(ctx->server_url,"firmware/"+ctx->firmware_info.name)：把服务器地址和 API 路径拼成完整 URL。必须避免重复斜杠，并保留已有查询参数处理规则。，结果保存到 std::string url。
        std::string url = manager.build_url(ctx->server_url, "firmware/" + ctx->firmware_info.name);

        // 执行语句：app::protocol::http::HttpRequest request;；它会更新一次变量、对象或程序状态。
        app::protocol::http::HttpRequest request;
        // 赋值：把右侧 url 的结果保存到左侧 request.url。
        request.url = url;
        // 赋值：把右侧 app::protocol::http::HttpMethod::GET 的结果保存到左侧 request.method。
        request.method = app::protocol::http::HttpMethod::GET;
        // 赋值：把右侧 ctx->timeout_ms 的结果保存到左侧 request.timeout_ms。
        request.timeout_ms = ctx->timeout_ms;

        // 执行语句：Md5ContextRAII md5_ctx;；它会更新一次变量、对象或程序状态。
        Md5ContextRAII md5_ctx;
        // 判断条件：!md5_ctx.is_ready() 是否成立。
        if (!md5_ctx.is_ready())
        {
            // 调用 ESP_LOGE(TAG, "MD5 上下文初始化失败");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "MD5 上下文初始化失败");
            // 调用 esp_ota_abort(ota_handle);，执行这个函数并处理它的返回值或状态变化。
            esp_ota_abort(ota_handle);
            // 访问对象成员：manager.update_status(OtaStatus::FAILED);；读取或修改这个对象里的成员状态。
            manager.update_status(OtaStatus::FAILED);
            // 判断条件：ctx->complete_callback 是否成立。
            if (ctx->complete_callback)
            {
                // 访问对象成员：ctx->complete_callback(false, "MD5 初始化失败");；读取或修改这个对象里的成员状态。
                ctx->complete_callback(false, "MD5 初始化失败");
            }
            {
                // 调用 lock(manager.mutex_)：为什么使用 trylock() 而不是普通 lock()。
                std::lock_guard<std::mutex> lock(manager.mutex_);
                // 赋值：把右侧 nullptr 的结果保存到左侧 manager.current_update_task_。
                manager.current_update_task_ = nullptr;
            }
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }

        // 定义变量 size_t total_received 并初始化为 0。
        size_t total_received = 0;
        // 定义变量 size_t total_size 并初始化为 ctx->firmware_info.size。
        size_t total_size = ctx->firmware_info.size;
        // 定义变量 bool download_ok 并初始化为 false。
        bool download_ok = false;

        // 赋值：把右侧 app::protocol::http::HttpMgr::get_instance() 的结果保存到左侧 auto& http_mgr。
        auto& http_mgr = app::protocol::http::HttpMgr::get_instance();

        // 定义变量 bool http_success 并初始化为 http_mgr.perform。
        bool http_success = http_mgr.perform(
            // 继续上一行的表达式或参数列表：request,。
            request,
            // 开始代码块：下面是这个函数、分支或循环的主体。
            [&total_size](const app::protocol::http::HttpResponse& resp) {
                // 判断条件：resp.content_length > 0 是否成立。
                if (resp.content_length > 0)
                {
                    // 赋值：把右侧 resp.content_length 的结果保存到左侧 total_size。
                    total_size = resp.content_length;
                }
                // 返回 resp.status_code == app::protocol::http::HttpStatus::OK 给调用者，表示当前函数的执行结果。
                return resp.status_code == app::protocol::http::HttpStatus::OK;
            // 结束上一行的 lambda 或初始化列表：把这段匿名代码/数据交给调用者。
            },
            // 继续上一行的表达式或参数列表：[ctx, &ota_handle, &md5_ctx, &total_received, &total_size,。
            [ctx, &ota_handle, &md5_ctx, &total_received, &total_size,
             // 开始代码块：下面是这个函数、分支或循环的主体。
             &download_ok](const uint8_t* data, size_t len) -> bool {
                {
                    // 调用 lock(ctx->manager->mutex_)：为什么使用 trylock() 而不是普通 lock()。
                    std::lock_guard<std::mutex> lock(ctx->manager->mutex_);
                    // 判断条件：ctx->manager->cancelled_ 是否成立。
                    if (ctx->manager->cancelled_)
                    {
                        // 调用 ESP_LOGW(TAG, "下载已取消");，执行这个函数并处理它的返回值或状态变化。
                        ESP_LOGW(TAG, "下载已取消");
                        // 赋值：把右侧 false 的结果保存到左侧 download_ok。
                        download_ok = false;
                        // 返回 false 给调用者，表示当前函数的执行结果。
                        return false;
                    }
                }

                // 定义变量 esp_err_t err 并初始化为 esp_ota_write(ota_handle, data, len)。
                esp_err_t err = esp_ota_write(ota_handle, data, len);
                // 判断条件：err != ESP_OK 是否成立。
                if (err != ESP_OK)
                {
                    // 调用 ESP_LOGE(TAG, "esp_ota_write 失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
                    ESP_LOGE(TAG, "esp_ota_write 失败: %s", esp_err_to_name(err));
                    // 赋值：把右侧 false 的结果保存到左侧 download_ok。
                    download_ok = false;
                    // 返回 false 给调用者，表示当前函数的执行结果。
                    return false;
                }

                // 判断条件：!md5_ctx.update(data, len) 是否成立。
                if (!md5_ctx.update(data, len))
                {
                    // 调用 ESP_LOGE(TAG, "MD5 更新失败");，执行这个函数并处理它的返回值或状态变化。
                    ESP_LOGE(TAG, "MD5 更新失败");
                    // 赋值：把右侧 false 的结果保存到左侧 download_ok。
                    download_ok = false;
                    // 返回 false 给调用者，表示当前函数的执行结果。
                    return false;
                }
                // 赋值：把右侧 len 的结果保存到左侧 total_received +。
                total_received += len;
                // 定义变量 float percent 并初始化为 0.0f。
                float percent = 0.0f;
                // 判断条件：total_size > 0 是否成立。
                if (total_size > 0)
                {
                    // 赋值：把右侧 (float)total_received * 100.0f / (float)total_size 的结果保存到左侧 percent。
                    percent = (float)total_received * 100.0f / (float)total_size;
                }

                // 判断条件：ctx->progress_callback 是否成立。
                if (ctx->progress_callback)
                {
                    // 访问对象成员：ctx->progress_callback(total_received, total_size, percent);；读取或修改这个对象里的成员状态。
                    ctx->progress_callback(total_received, total_size, percent);
                }

                // 赋值：把右侧 true 的结果保存到左侧 download_ok。
                download_ok = true;
                // 返回 true 给调用者，表示当前函数的执行结果。
                return true;
            // 执行语句：});；它会更新一次变量、对象或程序状态。
            });

        // 定义变量 bool was_cancelled 并初始化为 false。
        bool was_cancelled = false;
        {
            // 调用 lock(manager.mutex_)：为什么使用 trylock() 而不是普通 lock()。
            std::lock_guard<std::mutex> lock(manager.mutex_);
            // 赋值：把右侧 manager.cancelled_ 的结果保存到左侧 was_cancelled。
            was_cancelled = manager.cancelled_;
        }

        // 判断条件：!http_success || !download_ok 是否成立。
        if (!http_success || !download_ok)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const char* error_msg = was_cancelled ? "下载已取消" : "下载失败";
            // 调用 ESP_LOGE(TAG, "%s", error_msg);，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "%s", error_msg);
            // 调用 esp_ota_abort(ota_handle);，执行这个函数并处理它的返回值或状态变化。
            esp_ota_abort(ota_handle);
            // 访问对象成员：manager.update_status(OtaStatus::FAILED);；读取或修改这个对象里的成员状态。
            manager.update_status(OtaStatus::FAILED);
            // 判断条件：ctx->complete_callback 是否成立。
            if (ctx->complete_callback)
            {
                // 访问对象成员：ctx->complete_callback(false, error_msg);；读取或修改这个对象里的成员状态。
                ctx->complete_callback(false, error_msg);
            }
            {
                // 调用 lock(manager.mutex_)：为什么使用 trylock() 而不是普通 lock()。
                std::lock_guard<std::mutex> lock(manager.mutex_);
                // 赋值：把右侧 nullptr 的结果保存到左侧 manager.current_update_task_。
                manager.current_update_task_ = nullptr;
            }
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }

        // 执行语句：unsigned char md5_result[16];；它会更新一次变量、对象或程序状态。
        unsigned char md5_result[16];
        // 判断条件：!md5_ctx.finish(md5_result) 是否成立。
        if (!md5_ctx.finish(md5_result))
        {
            // 调用 ESP_LOGE(TAG, "MD5 结束计算失败");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "MD5 结束计算失败");
            // 调用 esp_ota_abort(ota_handle);，执行这个函数并处理它的返回值或状态变化。
            esp_ota_abort(ota_handle);
            // 访问对象成员：manager.update_status(OtaStatus::FAILED);；读取或修改这个对象里的成员状态。
            manager.update_status(OtaStatus::FAILED);
            // 判断条件：ctx->complete_callback 是否成立。
            if (ctx->complete_callback)
            {
                // 访问对象成员：ctx->complete_callback(false, "MD5 计算失败");；读取或修改这个对象里的成员状态。
                ctx->complete_callback(false, "MD5 计算失败");
            }
            {
                // 调用 lock(manager.mutex_)：为什么使用 trylock() 而不是普通 lock()。
                std::lock_guard<std::mutex> lock(manager.mutex_);
                // 赋值：把右侧 nullptr 的结果保存到左侧 manager.current_update_task_。
                manager.current_update_task_ = nullptr;
            }
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }

        // 定义变量并调用函数初始化：调用 md5_to_string(md5_result)：把 16 字节 MD5 转换成 32 位十六进制字符串，便于和服务器字段比较。，结果保存到 std::string md5_str。
        std::string md5_str = manager.md5_to_string(md5_result);
        // 访问对象成员：manager.update_status(OtaStatus::VERIFYING);；读取或修改这个对象里的成员状态。
        manager.update_status(OtaStatus::VERIFYING);

        // 定义变量 std::string expected_md5 并初始化为 ctx->firmware_info.md5。
        std::string expected_md5 = ctx->firmware_info.md5;
        // 定义变量 std::string actual_md5 并初始化为 md5_str。
        std::string actual_md5 = md5_str;
        // 调用 std::transform(expected_md5.begin(), expected_md5.end(), expected_md5.begin(), ::tolower);，执行这个函数并处理它的返回值或状态变化。
        std::transform(expected_md5.begin(), expected_md5.end(), expected_md5.begin(), ::tolower);
        // 调用 std::transform(actual_md5.begin(), actual_md5.end(), actual_md5.begin(), ::tolower);，执行这个函数并处理它的返回值或状态变化。
        std::transform(actual_md5.begin(), actual_md5.end(), actual_md5.begin(), ::tolower);

        // 判断条件：expected_md5 != actual_md5 是否成立。
        if (expected_md5 != actual_md5)
        {
            // 调用 ESP_LOGE(TAG, "MD5 校验失败：期望=%s, 实际=%s", ctx->firmware_info.md5.c_str(),，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "MD5 校验失败：期望=%s, 实际=%s", ctx->firmware_info.md5.c_str(),
                     // 继续上一行的表达式或参数列表：md5_str.c_str());。
                     md5_str.c_str());
            // 调用 esp_ota_abort(ota_handle);，执行这个函数并处理它的返回值或状态变化。
            esp_ota_abort(ota_handle);
            // 访问对象成员：manager.update_status(OtaStatus::FAILED);；读取或修改这个对象里的成员状态。
            manager.update_status(OtaStatus::FAILED);
            // 判断条件：ctx->complete_callback 是否成立。
            if (ctx->complete_callback)
            {
                // 访问对象成员：ctx->complete_callback(false, "MD5 校验失败");；读取或修改这个对象里的成员状态。
                ctx->complete_callback(false, "MD5 校验失败");
            }
            {
                // 调用 lock(manager.mutex_)：为什么使用 trylock() 而不是普通 lock()。
                std::lock_guard<std::mutex> lock(manager.mutex_);
                // 赋值：把右侧 nullptr 的结果保存到左侧 manager.current_update_task_。
                manager.current_update_task_ = nullptr;
            }
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }

        // 赋值：把右侧 esp_ota_end(ota_handle) 的结果保存到左侧 err。
        err = esp_ota_end(ota_handle);
        // 判断条件：err != ESP_OK 是否成立。
        if (err != ESP_OK)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const char* error_msg =
                // 继续上一行的表达式或参数列表：(err == ESP_ERR_OTA_VALIDATE_FAILED) ? "OTA 镜像验证失败" : "esp_ota_end 失败";。
                (err == ESP_ERR_OTA_VALIDATE_FAILED) ? "OTA 镜像验证失败" : "esp_ota_end 失败";
            // 调用 ESP_LOGE(TAG, "%s: %s", error_msg, esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "%s: %s", error_msg, esp_err_to_name(err));
            // 访问对象成员：manager.update_status(OtaStatus::FAILED);；读取或修改这个对象里的成员状态。
            manager.update_status(OtaStatus::FAILED);
            // 判断条件：ctx->complete_callback 是否成立。
            if (ctx->complete_callback)
            {
                // 执行语句：char error_buf[128];；它会更新一次变量、对象或程序状态。
                char error_buf[128];
                // 调用 snprintf(error_buf, sizeof(error_buf), "%s: %s", error_msg, esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
                snprintf(error_buf, sizeof(error_buf), "%s: %s", error_msg, esp_err_to_name(err));
                // 访问对象成员：ctx->complete_callback(false, error_buf);；读取或修改这个对象里的成员状态。
                ctx->complete_callback(false, error_buf);
            }
            {
                // 调用 lock(manager.mutex_)：为什么使用 trylock() 而不是普通 lock()。
                std::lock_guard<std::mutex> lock(manager.mutex_);
                // 赋值：把右侧 nullptr 的结果保存到左侧 manager.current_update_task_。
                manager.current_update_task_ = nullptr;
            }
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }

        // 赋值：把右侧 esp_ota_set_boot_partition(update_partition) 的结果保存到左侧 err。
        err = esp_ota_set_boot_partition(update_partition);
        // 判断条件：err != ESP_OK 是否成立。
        if (err != ESP_OK)
        {
            // 调用 ESP_LOGE(TAG, "esp_ota_set_boot_partition 失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "esp_ota_set_boot_partition 失败: %s", esp_err_to_name(err));
            // 访问对象成员：manager.update_status(OtaStatus::FAILED);；读取或修改这个对象里的成员状态。
            manager.update_status(OtaStatus::FAILED);
            // 判断条件：ctx->complete_callback 是否成立。
            if (ctx->complete_callback)
            {
                // 执行语句：char error_buf[128];；它会更新一次变量、对象或程序状态。
                char error_buf[128];
                // 调用 snprintf(error_buf, sizeof(error_buf), "设置引导分区失败: %s",，执行这个函数并处理它的返回值或状态变化。
                snprintf(error_buf, sizeof(error_buf), "设置引导分区失败: %s",
                         // 继续上一行的表达式或参数列表：esp_err_to_name(err));。
                         esp_err_to_name(err));
                // 访问对象成员：ctx->complete_callback(false, error_buf);；读取或修改这个对象里的成员状态。
                ctx->complete_callback(false, error_buf);
            }
            {
                // 调用 lock(manager.mutex_)：为什么使用 trylock() 而不是普通 lock()。
                std::lock_guard<std::mutex> lock(manager.mutex_);
                // 赋值：把右侧 nullptr 的结果保存到左侧 manager.current_update_task_。
                manager.current_update_task_ = nullptr;
            }
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }

        // 访问对象成员：manager.update_status(OtaStatus::COMPLETED);；读取或修改这个对象里的成员状态。
        manager.update_status(OtaStatus::COMPLETED);
        // 判断条件：ctx->complete_callback 是否成立。
        if (ctx->complete_callback)
        {
            // 访问对象成员：ctx->complete_callback(true, "");；读取或修改这个对象里的成员状态。
            ctx->complete_callback(true, "");
        }

        {
            // 调用 lock(manager.mutex_)：为什么使用 trylock() 而不是普通 lock()。
            std::lock_guard<std::mutex> lock(manager.mutex_);
            // 赋值：把右侧 nullptr 的结果保存到左侧 manager.current_update_task_。
            manager.current_update_task_ = nullptr;
        }

        // 调用 app::sys::task::TaskMgr::delay_ms(1000)：调用 vTaskDelay()，这是让出 CPU 的阻塞延时。一千毫秒延时期间，任务不占 CPU，调度器可以运行别人。。
        app::sys::task::TaskMgr::delay_ms(1000);
        // 调用 esp_restart();，执行这个函数并处理它的返回值或状态变化。
        esp_restart();
    };

    // 赋值：把右侧 ctx_ptr.release() 的结果保存到左侧 void* task_param。
    void* task_param = ctx_ptr.release();
    // 定义变量 auto task 并初始化为 std::make_unique<app::sys::task::Task>(task_function, task_config, task_param)。
    auto task = std::make_unique<app::sys::task::Task>(task_function, task_config, task_param);

    // 判断条件：!task->start() 是否成立。
    if (!task->start())
    {
        // 调用 ESP_LOGE(TAG, "启动 OTA 升级任务失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "启动 OTA 升级任务失败");
        // 调用 handle_error(OtaStatus::FAILED, "启动 OTA 升级任务失败");，执行这个函数并处理它的返回值或状态变化。
        handle_error(OtaStatus::FAILED, "启动 OTA 升级任务失败");
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        delete static_cast<std::shared_ptr<UpdateContext>*>(task_param);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);
        // 赋值：把右侧 task->handle() 的结果保存到左侧 current_update_task_。
        current_update_task_ = task->handle();
    }

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数调用：bool OtaMgr::report_status(const std::string& server_url, uint8_t status, uint8_t progress,；执行该函数并处理它的返回值或副作用。
bool OtaMgr::report_status(const std::string& server_url, uint8_t status, uint8_t progress,
                           // 继续上一行的表达式或参数列表：int32_t timeout_ms)。
                           int32_t timeout_ms)
{
    // 向服务器上报 OTA 状态；上报失败不应影响本地已经写入的固件。
    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);

        // 判断条件：!initialized_ 是否成立。
        if (!initialized_)
        {
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }
    }

    // 赋值：把右侧 app::protocol::http::HttpMgr::get_instance() 的结果保存到左侧 auto& http_mgr。
    auto& http_mgr = app::protocol::http::HttpMgr::get_instance();

    // 定义变量并调用函数初始化：调用 build_url(server_url,"api/ota/status")：把服务器地址和 API 路径拼成完整 URL。必须避免重复斜杠，并保留已有查询参数处理规则。，结果保存到 std::string url。
    std::string url = build_url(server_url, "api/ota/status");
    // 定义变量并调用函数初始化：调用 build_report_status_message(status,progress)：构造上报状态 JSON。服务器或后台系统可以知道设备更新到了哪一步。，结果保存到 std::string request_body。
    std::string request_body = build_report_status_message(status, progress);

    // 执行语句：app::protocol::http::HttpResponse response;；它会更新一次变量、对象或程序状态。
    app::protocol::http::HttpResponse response;
    // 访问对象成员：http_mgr.post(url, request_body, response, timeout_ms);；读取或修改这个对象里的成员状态。
    http_mgr.post(url, request_body, response, timeout_ms);

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数定义/声明：调用 OtaMgr::cancel()：设置取消标志。下载循环和回调看到标志后停止。取消不是“立刻 kill 任务”，任务仍然可能要先退出 HTTP 和 Flash API。。
bool OtaMgr::cancel()
{
    // 取消标志由下载循环轮询，取消不是强制终止正在进行的 Flash 操作。
    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);

        // 判断条件：status_ != OtaStatus::DOWNLOADING && status_ != OtaStatus::VERIFYING 是否成立。
        if (status_ != OtaStatus::DOWNLOADING && status_ != OtaStatus::VERIFYING)
        {
            // 调用 ESP_LOGW(TAG, "当前没有进行中的 OTA 升级");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGW(TAG, "当前没有进行中的 OTA 升级");
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }

        // 赋值：把右侧 true 的结果保存到左侧 cancelled_。
        cancelled_ = true;
        // 赋值：把右侧 OtaStatus::IDLE 的结果保存到左侧 status_。
        status_ = OtaStatus::IDLE;
    }

    // 判断条件：status_callback_ 是否成立。
    if (status_callback_)
    {
        // 调用 status_callback_(OtaStatus::IDLE);，执行这个函数并处理它的返回值或状态变化。
        status_callback_(OtaStatus::IDLE);
    }

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数定义/声明：调用 OtaMgr::get_status()：在锁保护下返回当前 OTA 状态。。
OtaStatus OtaMgr::get_status() const
{
    // 加锁读取状态，避免和后台 OTA 任务并发访问字符串或枚举。
    std::lock_guard<std::mutex> lock(mutex_);
    // 返回 status_ 给调用者，表示当前函数的执行结果。
    return status_;
}

// 函数定义/声明：调用 OtaMgr::is_updating()：判断状态是否为检查、下载或校验中。。
bool OtaMgr::is_updating() const
{
    // 检查、下载和校验都算“升级进行中”。
    std::lock_guard<std::mutex> lock(mutex_);
    // 返回 status_ == OtaStatus::DOWNLOADING || status_ == OtaStatus::VERIFYING || 给调用者，表示当前函数的执行结果。
    return status_ == OtaStatus::DOWNLOADING || status_ == OtaStatus::VERIFYING ||
           // 继续上一行的表达式或参数列表：status_ == OtaStatus::CHECKING;。
           status_ == OtaStatus::CHECKING;
}

// 函数定义/声明：调用 OtaMgr::get_current_version()：返回当前固件版本，供关于页面显示。。
std::string OtaMgr::get_current_version() const
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 返回 current_version_ 给调用者，表示当前函数的执行结果。
    return current_version_;
}

// 函数定义/声明：调用 OtaMgr::set_progress_callback()：注册回调。设置回调时要注意生命周期：如果回调捕获了页面对象，页面退出后对象可能失效。更安全的做法是回调只发 UiBus 事件。。
void OtaMgr::set_progress_callback(const ProgressCallback& callback)
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 赋值：把右侧 callback 的结果保存到左侧 progress_callback_。
    progress_callback_ = callback;
}

// 函数定义/声明：调用 OtaMgr::set_status_callback()：注册回调。设置回调时要注意生命周期：如果回调捕获了页面对象，页面退出后对象可能失效。更安全的做法是回调只发 UiBus 事件。。
void OtaMgr::set_status_callback(const StatusCallback& callback)
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 赋值：把右侧 callback 的结果保存到左侧 status_callback_。
    status_callback_ = callback;
}

// 函数定义/声明：调用 OtaMgr::set_complete_callback()：注册回调。设置回调时要注意生命周期：如果回调捕获了页面对象，页面退出后对象可能失效。更安全的做法是回调只发 UiBus 事件。。
void OtaMgr::set_complete_callback(const CompleteCallback& callback)
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 赋值：把右侧 callback 的结果保存到左侧 complete_callback_。
    complete_callback_ = callback;
}

// 函数定义/声明：调用 OtaMgr::update_status()：更新状态并调用状态回调，同时根据状态决定是否调用完成回调。。
void OtaMgr::update_status(OtaStatus status)
{
    // 注意：这里在持锁期间调用回调。
    // 回调中不要再次调用需要 mutex_ 的 OtaMgr 方法，否则可能死锁。
    std::lock_guard<std::mutex> lock(mutex_);
    // 赋值：把右侧 status 的结果保存到左侧 status_。
    status_ = status;

    // 判断条件：status_callback_ 是否成立。
    if (status_callback_)
    {
        // 调用 status_callback_(status);，执行这个函数并处理它的返回值或状态变化。
        status_callback_(status);
    }
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::common::ota
