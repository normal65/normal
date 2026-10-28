// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/common/ota/ota.hpp
 * 作用：实现 OTA 检查更新、下载固件、MD5 校验、切换分区和状态上报。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：OTA 回调不在 UI 线程，UI 更新必须通过 UiBus。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <functional>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <memory>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <mutex>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <string>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "cJSON.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "psa/crypto.h"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::common::ota {

/** @brief cJSON 对象 RAII */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class JsonRAII
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 定义或声明函数 JsonRAII：参数在括号里，返回类型写在函数名前面。
    explicit JsonRAII(const char* json_str)
        // 定义或声明函数 json_：参数在括号里，返回类型写在函数名前面。
        : json_(cJSON_Parse(json_str))
    {
    }

    // 定义或声明函数 JsonRAII：参数在括号里，返回类型写在函数名前面。
    JsonRAII()
        // 定义或声明函数 json_：参数在括号里，返回类型写在函数名前面。
        : json_(cJSON_CreateObject())
    {
    }

    // 调用 JsonRAII(const JsonRAII&) = delete;，执行这个函数并处理它的返回值或状态变化。
    JsonRAII(const JsonRAII&) = delete;
    // 赋值：把右侧 (const JsonRAII&) = delete 的结果保存到左侧 JsonRAII& operator。
    JsonRAII& operator=(const JsonRAII&) = delete;

    // 调用 JsonRAII(JsonRAII&& other) noexcept，执行这个函数并处理它的返回值或状态变化。
    JsonRAII(JsonRAII&& other) noexcept
        // 定义或声明函数 json_：参数在括号里，返回类型写在函数名前面。
        : json_(other.json_)
    {
        // 赋值：把右侧 nullptr 的结果保存到左侧 other.json_。
        other.json_ = nullptr;
    }

    // 组合条件：左右两个条件共同决定这一段代码是否继续执行。
    JsonRAII& operator=(JsonRAII&& other) noexcept
    {
        // 判断条件：this != &other 是否成立。
        if (this != &other)
        {
            // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
            reset();
            // 赋值：把右侧 other.json_ 的结果保存到左侧 json_。
            json_ = other.json_;
            // 赋值：把右侧 nullptr 的结果保存到左侧 other.json_。
            other.json_ = nullptr;
        }
        // 返回 *this 给调用者，表示当前函数的执行结果。
        return *this;
    }

    // 定义或声明函数 ~JsonRAII：参数在括号里，返回类型写在函数名前面。
    ~JsonRAII()
    {
        // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
        reset();
    }

    // 函数定义/声明：调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
    cJSON* get() const
    {
        // 返回 json_ 给调用者，表示当前函数的执行结果。
        return json_;
    }

    // 定义或声明函数 is_valid：参数在括号里，返回类型写在函数名前面。
    bool is_valid() const
    {
        // 返回 json_ != nullptr 给调用者，表示当前函数的执行结果。
        return json_ != nullptr;
    }

    // 执行这条代码：operator cJSON*() const。它会更新变量、对象状态或调用链。
    operator cJSON*() const
    {
        // 返回 json_ 给调用者，表示当前函数的执行结果。
        return json_;
    }

    // 函数定义/声明：调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    void reset()
    {
        // 判断条件：json_ != nullptr 是否成立。
        if (json_ != nullptr)
        {
            // 调用 cJSON_Delete(json_);，执行这个函数并处理它的返回值或状态变化。
            cJSON_Delete(json_);
            // 赋值：把右侧 nullptr 的结果保存到左侧 json_。
            json_ = nullptr;
        }
    }

    // 定义或声明函数 release：参数在括号里，返回类型写在函数名前面。
    cJSON* release()
    {
        // 赋值：把右侧 json_ 的结果保存到左侧 cJSON* result。
        cJSON* result = json_;
        // 赋值：把右侧 nullptr 的结果保存到左侧 json_。
        json_ = nullptr;
        // 返回 result 给调用者，表示当前函数的执行结果。
        return result;
    }

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 执行语句：cJSON* json_;；它会更新一次变量、对象或程序状态。
    cJSON* json_;
};

/** @brief cJSON_Print 返回字符串 RAII */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class JsonStringRAII
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 定义或声明函数 JsonStringRAII：参数在括号里，返回类型写在函数名前面。
    explicit JsonStringRAII(char* json_str)
        // 定义或声明函数 str_：参数在括号里，返回类型写在函数名前面。
        : str_(json_str)
    {
    }

    // 调用 JsonStringRAII(const JsonStringRAII&) = delete;，执行这个函数并处理它的返回值或状态变化。
    JsonStringRAII(const JsonStringRAII&) = delete;
    // 赋值：把右侧 (const JsonStringRAII&) = delete 的结果保存到左侧 JsonStringRAII& operator。
    JsonStringRAII& operator=(const JsonStringRAII&) = delete;

    // 调用 JsonStringRAII(JsonStringRAII&& other) noexcept，执行这个函数并处理它的返回值或状态变化。
    JsonStringRAII(JsonStringRAII&& other) noexcept
        // 定义或声明函数 str_：参数在括号里，返回类型写在函数名前面。
        : str_(other.str_)
    {
        // 赋值：把右侧 nullptr 的结果保存到左侧 other.str_。
        other.str_ = nullptr;
    }

    // 组合条件：左右两个条件共同决定这一段代码是否继续执行。
    JsonStringRAII& operator=(JsonStringRAII&& other) noexcept
    {
        // 判断条件：this != &other 是否成立。
        if (this != &other)
        {
            // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
            reset();
            // 赋值：把右侧 other.str_ 的结果保存到左侧 str_。
            str_ = other.str_;
            // 赋值：把右侧 nullptr 的结果保存到左侧 other.str_。
            other.str_ = nullptr;
        }
        // 返回 *this 给调用者，表示当前函数的执行结果。
        return *this;
    }

    // 定义或声明函数 ~JsonStringRAII：参数在括号里，返回类型写在函数名前面。
    ~JsonStringRAII()
    {
        // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
        reset();
    }

    // 函数定义/声明：调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
    char* get() const
    {
        // 返回 str_ 给调用者，表示当前函数的执行结果。
        return str_;
    }

    // 定义或声明函数 is_valid：参数在括号里，返回类型写在函数名前面。
    bool is_valid() const
    {
        // 返回 str_ != nullptr 给调用者，表示当前函数的执行结果。
        return str_ != nullptr;
    }

    // 定义或声明函数 to_string：参数在括号里，返回类型写在函数名前面。
    std::string to_string() const
    {
        // 返回 str_ ? std::string(str_) : std::string() 给调用者，表示当前函数的执行结果。
        return str_ ? std::string(str_) : std::string();
    }

    // 函数定义/声明：调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    void reset()
    {
        // 判断条件：str_ != nullptr 是否成立。
        if (str_ != nullptr)
        {
            // 调用 free(str_);，执行这个函数并处理它的返回值或状态变化。
            free(str_);
            // 赋值：把右侧 nullptr 的结果保存到左侧 str_。
            str_ = nullptr;
        }
    }

    // 定义或声明函数 release：参数在括号里，返回类型写在函数名前面。
    char* release()
    {
        // 赋值：把右侧 str_ 的结果保存到左侧 char* result。
        char* result = str_;
        // 赋值：把右侧 nullptr 的结果保存到左侧 str_。
        str_ = nullptr;
        // 返回 result 给调用者，表示当前函数的执行结果。
        return result;
    }

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 执行语句：char* str_;；它会更新一次变量、对象或程序状态。
    char* str_;
};

/** @brief PSA Crypto MD5（ESP-IDF 6 / Mbed TLS 4，无 mbedtls/md5.h） */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class Md5ContextRAII
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 调用 Md5ContextRAII();，执行这个函数并处理它的返回值或状态变化。
    Md5ContextRAII();
    // 调用 Md5ContextRAII(const Md5ContextRAII&) = delete;，执行这个函数并处理它的返回值或状态变化。
    Md5ContextRAII(const Md5ContextRAII&) = delete;
    // 赋值：把右侧 (const Md5ContextRAII&) = delete 的结果保存到左侧 Md5ContextRAII& operator。
    Md5ContextRAII& operator=(const Md5ContextRAII&) = delete;

    // 函数调用：~Md5ContextRAII();；执行该函数并处理它的返回值或副作用。
    ~Md5ContextRAII();

    // 定义或声明函数 is_ready：参数在括号里，返回类型写在函数名前面。
    bool is_ready() const
    {
        // 返回 ready_ 给调用者，表示当前函数的执行结果。
        return ready_;
    }

    // 调用 update(constuint8_t*data,size_tlen)：update() 把一段流数据加入摘要；finish() 输出最终 16 字节。下载回调每收到一段数据就调用一次 update。。
    bool update(const uint8_t* data, size_t len);
    // 调用 finish(unsignedcharout[16])：update() 把一段流数据加入摘要；finish() 输出最终 16 字节。下载回调每收到一段数据就调用一次 update。。
    bool finish(unsigned char out[16]);

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 执行语句：psa_hash_operation_t op_;；它会更新一次变量、对象或程序状态。
    psa_hash_operation_t op_;
    // 执行语句：bool ready_;；它会更新一次变量、对象或程序状态。
    bool ready_;
};

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class OtaStatus : uint8_t
{
    // 执行这条代码：IDLE = 0,。它会更新变量、对象状态或调用链。
    IDLE = 0,
    // 继续上一行的表达式或参数列表：CHECKING = 1,。
    CHECKING = 1,
    // 继续上一行的表达式或参数列表：DOWNLOADING = 2,。
    DOWNLOADING = 2,
    // 继续上一行的表达式或参数列表：VERIFYING = 3,。
    VERIFYING = 3,
    // 继续上一行的表达式或参数列表：COMPLETED = 4,。
    COMPLETED = 4,
    // 继续上一行的表达式或参数列表：FAILED = 5。
    FAILED = 5
};

// struct FirmwareInfo：把相关数据字段组合在一起，表示项目中的一个概念。
struct FirmwareInfo
{
    // 执行语句：std::string version;；它会更新一次变量、对象或程序状态。
    std::string version;
    // 执行语句：std::string name;；它会更新一次变量、对象或程序状态。
    std::string name;
    // 执行语句：size_t size;；它会更新一次变量、对象或程序状态。
    size_t size;
    // 执行语句：std::string info;；它会更新一次变量、对象或程序状态。
    std::string info;
    // 执行语句：std::string md5;；它会更新一次变量、对象或程序状态。
    std::string md5;
    // 执行语句：std::string time;；它会更新一次变量、对象或程序状态。
    std::string time;
};

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ProgressCallback = std::function<void(size_t received, size_t total, float percent)>;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using StatusCallback = std::function<void(OtaStatus status)>;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using CompleteCallback = std::function<void(bool success, const std::string& error)>;

/** @brief OTA 单例：检查更新、下载、校验、上报 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class OtaMgr
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static OtaMgr& get_instance();

    // 函数调用：bool init(const std::string& device_id, const std::string& current_version);；执行该函数并处理它的返回值或副作用。
    bool init(const std::string& device_id, const std::string& current_version);
    // 调用 deinit()：创建和销毁队列。
    void deinit();

    // 赋值：把右侧 10000) 的结果保存到左侧 bool check_update(const std::string& server_url, int32_t timeout_ms。
    bool check_update(const std::string& server_url, int32_t timeout_ms = 10000);
    // 函数调用：bool get_firmware_info(const std::string& server_url, FirmwareInfo& info,；执行该函数并处理它的返回值或副作用。
    bool get_firmware_info(const std::string& server_url, FirmwareInfo& info,
                           // 继续上一行的表达式或参数列表：int32_t timeout_ms = 10000);。
                           int32_t timeout_ms = 10000);
    // 函数调用：bool start_update(const std::string& server_url, const FirmwareInfo& firmware_info,；执行该函数并处理它的返回值或副作用。
    bool start_update(const std::string& server_url, const FirmwareInfo& firmware_info,
                      // 继续上一行的表达式或参数列表：int32_t timeout_ms = 120000);。
                      int32_t timeout_ms = 120000);
    // 函数调用：bool report_status(const std::string& server_url, uint8_t status, uint8_t progress,；执行该函数并处理它的返回值或副作用。
    bool report_status(const std::string& server_url, uint8_t status, uint8_t progress,
                       // 继续上一行的表达式或参数列表：int32_t timeout_ms = 5000);。
                       int32_t timeout_ms = 5000);
    // 调用 cancel()：设置取消标志。下载循环和回调看到标志后停止。取消不是“立刻 kill 任务”，任务仍然可能要先退出 HTTP 和 Flash API。。
    bool cancel();

    // 调用 get_status()：在锁保护下返回当前 OTA 状态。。
    OtaStatus get_status() const;
    // 调用 set_progress_callback(constProgressCallback&callback)：注册回调。设置回调时要注意生命周期：如果回调捕获了页面对象，页面退出后对象可能失效。更安全的做法是回调只发 UiBus 事件。。
    void set_progress_callback(const ProgressCallback& callback);
    // 调用 set_status_callback(constStatusCallback&callback)：注册回调。设置回调时要注意生命周期：如果回调捕获了页面对象，页面退出后对象可能失效。更安全的做法是回调只发 UiBus 事件。。
    void set_status_callback(const StatusCallback& callback);
    // 调用 set_complete_callback(constCompleteCallback&callback)：注册回调。设置回调时要注意生命周期：如果回调捕获了页面对象，页面退出后对象可能失效。更安全的做法是回调只发 UiBus 事件。。
    void set_complete_callback(const CompleteCallback& callback);
    // 调用 is_updating()：判断状态是否为检查、下载或校验中。。
    bool is_updating() const;
    // 调用 get_current_version()：返回当前固件版本，供关于页面显示。。
    std::string get_current_version() const;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 OtaMgr() = default;，执行这个函数并处理它的返回值或状态变化。
    OtaMgr() = default;
    // 定义或声明函数 ~OtaMgr：参数在括号里，返回类型写在函数名前面。
    ~OtaMgr()
    {
        // 调用 deinit()：创建和销毁队列。
        deinit();
    }
    // 调用 OtaMgr(const OtaMgr&) = delete;，执行这个函数并处理它的返回值或状态变化。
    OtaMgr(const OtaMgr&) = delete;
    // 赋值：把右侧 (const OtaMgr&) = delete 的结果保存到左侧 OtaMgr& operator。
    OtaMgr& operator=(const OtaMgr&) = delete;

    // 调用 get_timestamp()：生成请求时间戳。服务器可以用它记录或校验请求时间。。
    std::string get_timestamp() const;
    // 调用 build_check_update_message()：构造检查更新请求。典型内容包含设备 ID、当前版本和消息类型。。
    std::string build_check_update_message() const;
    // 调用 build_get_firmware_info_message()：构造获取固件详情请求。它通常用于在下载前读取大小、MD5 和说明。。
    std::string build_get_firmware_info_message() const;
    // 调用 build_request_firmware_message(constFirmwareInfo&info)：构造请求下载某个固件的消息，可能包含版本和服务器返回的标识。。
    std::string build_request_firmware_message(const FirmwareInfo& info) const;
    // 调用 build_report_status_message(uint8_tstatus,uint8_tprogress)：构造上报状态 JSON。服务器或后台系统可以知道设备更新到了哪一步。。
    std::string build_report_status_message(uint8_t status, uint8_t progress) const;
    // 函数调用：bool parse_reply_update(const std::string& json, int& respond, std::string& download_url);；执行该函数并处理它的返回值或副作用。
    bool parse_reply_update(const std::string& json, int& respond, std::string& download_url);
    // 函数调用：bool parse_firmware_info(const std::string& json, FirmwareInfo& info);；执行该函数并处理它的返回值或副作用。
    bool parse_firmware_info(const std::string& json, FirmwareInfo& info);
    // 函数调用：bool parse_error(const std::string& json, int& code, std::string& message);；执行该函数并处理它的返回值或副作用。
    bool parse_error(const std::string& json, int& code, std::string& message);
    // 调用 update_status(OtaStatusstatus)：更新状态并调用状态回调，同时根据状态决定是否调用完成回调。。
    void update_status(OtaStatus status);
    // 函数调用：std::string build_url(const std::string& server_url, const std::string& path) const;；执行该函数并处理它的返回值或副作用。
    std::string build_url(const std::string& server_url, const std::string& path) const;
    // 赋值：把右侧 "") 的结果保存到左侧 void handle_error(OtaStatus new_status, const std::string& error_msg。
    void handle_error(OtaStatus new_status, const std::string& error_msg = "");
    // 调用 md5_to_string(constunsignedchar*md5_bytes)：把 16 字节 MD5 转换成 32 位十六进制字符串，便于和服务器字段比较。。
    std::string md5_to_string(const unsigned char* md5_bytes) const;
    // 函数调用：std::string build_base_json_message(const std::string& type) const;；执行该函数并处理它的返回值或副作用。
    std::string build_base_json_message(const std::string& type) const;

    // 执行语句：mutable std::mutex mutex_;；它会更新一次变量、对象或程序状态。
    mutable std::mutex mutex_;
    // 执行语句：bool initialized_;；它会更新一次变量、对象或程序状态。
    bool initialized_;
    // 执行语句：std::string device_id_;；它会更新一次变量、对象或程序状态。
    std::string device_id_;
    // 执行语句：std::string current_version_;；它会更新一次变量、对象或程序状态。
    std::string current_version_;
    // 执行语句：OtaStatus status_;；它会更新一次变量、对象或程序状态。
    OtaStatus status_;
    // 执行语句：ProgressCallback progress_callback_;；它会更新一次变量、对象或程序状态。
    ProgressCallback progress_callback_;
    // 执行语句：StatusCallback status_callback_;；它会更新一次变量、对象或程序状态。
    StatusCallback status_callback_;
    // 执行语句：CompleteCallback complete_callback_;；它会更新一次变量、对象或程序状态。
    CompleteCallback complete_callback_;
    // 执行语句：void* current_update_task_;；它会更新一次变量、对象或程序状态。
    void* current_update_task_;
    // 易变变量：告诉编译器该值可能被中断或硬件改变，不能随意优化读取。
    volatile bool cancelled_;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::common::ota
