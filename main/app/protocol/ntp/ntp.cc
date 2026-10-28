/**
 * [初学者导读] 文件：main/app/protocol/ntp/ntp.cc
 * 作用：初始化 NTP、同步网络时间并广播同步状态。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：网络请求可能阻塞，必须在后台任务或专门任务中执行。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ntp.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <ctime>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_log.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_netif_sntp.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_sntp.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "freertos/FreeRTOS.h"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "NTP";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::protocol::ntp {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 只读约束：这个值或对象在当前位置不能被修改。
const std::vector<std::string> kDefaultServers = {
    // 执行这条代码："ntp.aliyun.com",。它会更新变量、对象状态或调用链。
    "ntp.aliyun.com",
    // 继续上一行的表达式或参数列表："cn.pool.ntp.org",。
    "cn.pool.ntp.org",
    // 继续上一行的表达式或参数列表："pool.ntp.org",。
    "pool.ntp.org",
};

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 NtpMgr::get_instance：参数在括号里，返回类型写在函数名前面。
NtpMgr& NtpMgr::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static NtpMgr instance;
    // 返回 instance 给调用者，表示当前函数的执行结果。
    return instance;
}

// 定义或声明函数 NtpMgr::init：参数在括号里，返回类型写在函数名前面。
bool NtpMgr::init()
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);

    // 判断条件：initialized_ 是否成立。
    if (initialized_)
    {
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 赋值：把右侧 true 的结果保存到左侧 initialized_。
    initialized_ = true;
    // 赋值：把右侧 false 的结果保存到左侧 started_。
    started_ = false;
    // 赋值：把右侧 SyncStatus::RESET 的结果保存到左侧 sync_status_。
    sync_status_ = SyncStatus::RESET;
    // 赋值：把右侧 kDefaultServers 的结果保存到左侧 servers_。
    servers_ = kDefaultServers;
    // 赋值：把右侧 nullptr 的结果保存到左侧 sync_callback_。
    sync_callback_ = nullptr;
    // 赋值：把右侧 SyncMode::IMMEDIATE 的结果保存到左侧 sync_mode_。
    sync_mode_ = SyncMode::IMMEDIATE;

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 NtpMgr::~NtpMgr：参数在括号里，返回类型写在函数名前面。
NtpMgr::~NtpMgr()
{
    // 判断条件：initialized_ 是否成立。
    if (initialized_)
    {
        // 调用 deinit()：创建和销毁队列。
        deinit();
    }
}

// 定义或声明函数 NtpMgr::deinit：参数在括号里，返回类型写在函数名前面。
void NtpMgr::deinit()
{
    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);
        // 判断条件：!initialized_ 是否成立。
        if (!initialized_)
        {
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }
    }

    // 调用 stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
    stop();

    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 赋值：把右侧 false 的结果保存到左侧 initialized_。
    initialized_ = false;
    // 赋值：把右侧 SyncStatus::RESET 的结果保存到左侧 sync_status_。
    sync_status_ = SyncStatus::RESET;
    // 赋值：把右侧 kDefaultServers 的结果保存到左侧 servers_。
    servers_ = kDefaultServers;
    // 赋值：把右侧 nullptr 的结果保存到左侧 sync_callback_。
    sync_callback_ = nullptr;
    // 赋值：把右侧 SyncMode::IMMEDIATE 的结果保存到左侧 sync_mode_。
    sync_mode_ = SyncMode::IMMEDIATE;
}

// 定义或声明函数 NtpMgr::configure：参数在括号里，返回类型写在函数名前面。
bool NtpMgr::configure(const std::vector<std::string>& servers, SyncMode sync_mode)
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);

    // 判断条件：!initialized_ 是否成立。
    if (!initialized_)
    {
        // 调用 ESP_LOGE(TAG, "NTP 管理器未初始化");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "NTP 管理器未初始化");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 判断条件：servers.empty() || servers.size() > 3 是否成立。
    if (servers.empty() || servers.size() > 3)
    {
        // 调用 ESP_LOGE(TAG, "NTP 服务器数量无效（1-3 个）");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "NTP 服务器数量无效（1-3 个）");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 判断条件：started_ 是否成立。
    if (started_)
    {
        // 调用 ESP_LOGE(TAG, "SNTP 已启动，请先停止");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "SNTP 已启动，请先停止");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 servers 的结果保存到左侧 servers_。
    servers_ = servers;
    // 赋值：把右侧 sync_mode 的结果保存到左侧 sync_mode_。
    sync_mode_ = sync_mode;

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 NtpMgr::start：参数在括号里，返回类型写在函数名前面。
bool NtpMgr::start()
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);

    // 判断条件：!initialized_ 是否成立。
    if (!initialized_)
    {
        // 调用 ESP_LOGE(TAG, "NTP 管理器未初始化");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "NTP 管理器未初始化");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 判断条件：servers_.empty() 是否成立。
    if (servers_.empty())
    {
        // 调用 ESP_LOGW(TAG, "NTP 服务器列表为空，使用默认服务器");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGW(TAG, "NTP 服务器列表为空，使用默认服务器");
        // 赋值：把右侧 kDefaultServers 的结果保存到左侧 servers_。
        servers_ = kDefaultServers;
    }

    // 判断条件：started_ 是否成立。
    if (started_)
    {
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 定义变量 esp_sntp_config_t config 并初始化为 ESP_NETIF_SNTP_DEFAULT_CONFIG(servers_[0].c_str())。
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(servers_[0].c_str());

    // 判断条件：sync_mode_ == SyncMode::SMOOTH 是否成立。
    if (sync_mode_ == SyncMode::SMOOTH)
    {
        // 赋值：把右侧 true 的结果保存到左侧 config.smooth_sync。
        config.smooth_sync = true;
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 赋值：把右侧 false 的结果保存到左侧 config.smooth_sync。
        config.smooth_sync = false;
    }

    // 赋值：把右侧 sntp_sync_callback 的结果保存到左侧 config.sync_cb。
    config.sync_cb = sntp_sync_callback;

    // 定义变量 esp_err_t ret 并初始化为 esp_netif_sntp_init(&config)。
    esp_err_t ret = esp_netif_sntp_init(&config);
    // 判断条件：ret != ESP_OK 是否成立。
    if (ret != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "SNTP 初始化失败: %s", esp_err_to_name(ret));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "SNTP 初始化失败: %s", esp_err_to_name(ret));
        // 赋值：把右侧 SyncStatus::FAILED 的结果保存到左侧 sync_status_。
        sync_status_ = SyncStatus::FAILED;
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 判断条件：servers_.size() > 1 是否成立。
    if (servers_.size() > 1)
    {
        // 调用 esp_sntp_setservername(1, servers_[1].c_str());，执行这个函数并处理它的返回值或状态变化。
        esp_sntp_setservername(1, servers_[1].c_str());
    }
    // 判断条件：servers_.size() > 2 是否成立。
    if (servers_.size() > 2)
    {
        // 调用 esp_sntp_setservername(2, servers_[2].c_str());，执行这个函数并处理它的返回值或状态变化。
        esp_sntp_setservername(2, servers_[2].c_str());
    }

    // 赋值：把右侧 true 的结果保存到左侧 started_。
    started_ = true;
    // 赋值：把右侧 SyncStatus::IN_PROGRESS 的结果保存到左侧 sync_status_。
    sync_status_ = SyncStatus::IN_PROGRESS;

    // 调用 ESP_LOGD(TAG, "SNTP 服务已启动");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "SNTP 服务已启动");

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数定义/声明：调用 NtpMgr::stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
void NtpMgr::stop()
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);

    // 判断条件：!started_ 是否成立。
    if (!started_)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 调用 esp_netif_sntp_deinit();，执行这个函数并处理它的返回值或状态变化。
    esp_netif_sntp_deinit();

    // 赋值：把右侧 false 的结果保存到左侧 started_。
    started_ = false;
    // 赋值：把右侧 SyncStatus::RESET 的结果保存到左侧 sync_status_。
    sync_status_ = SyncStatus::RESET;

    // 调用 ESP_LOGD(TAG, "SNTP 服务已停止");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "SNTP 服务已停止");
}

// 定义或声明函数 NtpMgr::wait_sync：参数在括号里，返回类型写在函数名前面。
bool NtpMgr::wait_sync(uint32_t timeout_ms)
{
    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);
        // 判断条件：!started_ 是否成立。
        if (!started_)
        {
            // 调用 ESP_LOGE(TAG, "SNTP 服务未启动");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "SNTP 服务未启动");
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }
    }

    // 定义变量 esp_err_t ret 并初始化为 esp_netif_sntp_sync_wait(pdMS_TO_TICKS(timeout_ms))。
    esp_err_t ret = esp_netif_sntp_sync_wait(pdMS_TO_TICKS(timeout_ms));

    {
        // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mutex_);
        // 判断条件：ret == ESP_OK 是否成立。
        if (ret == ESP_OK)
        {
            // 赋值：把右侧 SyncStatus::COMPLETED 的结果保存到左侧 sync_status_。
            sync_status_ = SyncStatus::COMPLETED;
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 赋值：把右侧 SyncStatus::FAILED 的结果保存到左侧 sync_status_。
            sync_status_ = SyncStatus::FAILED;
        }
    }

    // 返回 ret == ESP_OK 给调用者，表示当前函数的执行结果。
    return ret == ESP_OK;
}

// 定义或声明函数 NtpMgr::set_sync_callback：参数在括号里，返回类型写在函数名前面。
void NtpMgr::set_sync_callback(SyncCallback callback)
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 赋值：把右侧 callback 的结果保存到左侧 sync_callback_。
    sync_callback_ = callback;
}

// 定义或声明函数 NtpMgr::set_timezone：参数在括号里，返回类型写在函数名前面。
bool NtpMgr::set_timezone(const char* tz)
{
    // 判断条件：!tz 是否成立。
    if (!tz)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 调用 setenv("TZ", tz, 1);，执行这个函数并处理它的返回值或状态变化。
    setenv("TZ", tz, 1);
    // 调用 tzset();，执行这个函数并处理它的返回值或状态变化。
    tzset();

    // 调用 ESP_LOGD(TAG, "时区已设置为: %s", tz);，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "时区已设置为: %s", tz);

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 NtpMgr::get_sync_status：参数在括号里，返回类型写在函数名前面。
SyncStatus NtpMgr::get_sync_status() const
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 返回 sync_status_ 给调用者，表示当前函数的执行结果。
    return sync_status_;
}

// 定义或声明函数 NtpMgr::is_init：参数在括号里，返回类型写在函数名前面。
bool NtpMgr::is_init() const
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 返回 initialized_ 给调用者，表示当前函数的执行结果。
    return initialized_;
}

// 定义或声明函数 NtpMgr::is_started：参数在括号里，返回类型写在函数名前面。
bool NtpMgr::is_started() const
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 返回 started_ 给调用者，表示当前函数的执行结果。
    return started_;
}

// 定义或声明函数 NtpMgr::sntp_sync_callback：参数在括号里，返回类型写在函数名前面。
void NtpMgr::sntp_sync_callback(struct timeval* tv)
{
    // 赋值：把右侧 get_instance() 的结果保存到左侧 auto& instance。
    auto& instance = get_instance();

    // 定义变量 SyncCallback cb 并初始化为 nullptr。
    SyncCallback cb = nullptr;
    {
        // 调用 lock(instance.mutex_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(instance.mutex_);
        // 赋值：把右侧 SyncStatus::COMPLETED 的结果保存到左侧 instance.sync_status_。
        instance.sync_status_ = SyncStatus::COMPLETED;
        // 赋值：把右侧 instance.sync_callback_ 的结果保存到左侧 cb。
        cb = instance.sync_callback_;
    }

    // 判断条件：cb 是否成立。
    if (cb)
    {
        // 调用 cb(SyncStatus::COMPLETED);，执行这个函数并处理它的返回值或状态变化。
        cb(SyncStatus::COMPLETED);
    }

    // 调用 ESP_LOGD(TAG, "时间同步完成");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "时间同步完成");
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::protocol::ntp
