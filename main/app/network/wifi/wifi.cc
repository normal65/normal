/**
 * [初学者导读] 文件：main/app/network/wifi/wifi.cc
 * 作用：初始化 Wi-Fi、扫描 AP、连接网络、保存凭据并广播状态。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：网络回调不在 UI 线程，状态变化通常通过 UiBus 回到页面。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "wifi.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <memory>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <string>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <vector>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_log.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_netif.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_wifi.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <nvs.h>

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "Wifi";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 只读约束：这个值或对象在当前位置不能被修改。
const char* NVS_NS = "wifi";
// 只读约束：这个值或对象在当前位置不能被修改。
const char* NVS_LIST = "list";

// 定义或声明函数 build_pass_key：参数在括号里，返回类型写在函数名前面。
std::string build_pass_key(const char* ssid)
{
    // 定义变量 std::string key 并初始化为 "pass:"。
    std::string key = "pass:";
    // 赋值：把右侧 ssid 的结果保存到左侧 key +。
    key += ssid;
    // 返回 key 给调用者，表示当前函数的执行结果。
    return key;
}

// 定义或声明函数 get_ssid_list：参数在括号里，返回类型写在函数名前面。
bool get_ssid_list(std::vector<std::string>& ssids)
{
    // 执行语句：nvs_handle_t h;；它会更新一次变量、对象或程序状态。
    nvs_handle_t h;
    // 定义变量 esp_err_t ret 并初始化为 nvs_open(NVS_NS, NVS_READONLY, &h)。
    esp_err_t ret = nvs_open(NVS_NS, NVS_READONLY, &h);
    // 判断条件：ret != ESP_OK 是否成立。
    if (ret != ESP_OK)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 定义变量 size_t sz 并初始化为 0。
    size_t sz = 0;
    // 赋值：把右侧 nvs_get_str(h, NVS_LIST, nullptr, &sz) 的结果保存到左侧 ret。
    ret = nvs_get_str(h, NVS_LIST, nullptr, &sz);
    // 判断条件：ret != ESP_OK || sz == 0 是否成立。
    if (ret != ESP_OK || sz == 0)
    {
        // 调用 nvs_close(h);，执行这个函数并处理它的返回值或状态变化。
        nvs_close(h);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 函数调用：std::vector<char> buf(sz);；执行该函数并处理它的返回值或副作用。
    std::vector<char> buf(sz);
    // 赋值：把右侧 nvs_get_str(h, NVS_LIST, buf.data(), &sz) 的结果保存到左侧 ret。
    ret = nvs_get_str(h, NVS_LIST, buf.data(), &sz);
    // 调用 nvs_close(h);，执行这个函数并处理它的返回值或状态变化。
    nvs_close(h);

    // 判断条件：ret != ESP_OK 是否成立。
    if (ret != ESP_OK)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    ssids.clear();
    // 赋值：把右侧 strtok(buf.data(), ",") 的结果保存到左侧 char* tok。
    char* tok = strtok(buf.data(), ",");
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (tok)
    {
        // 访问对象成员：ssids.push_back(std::string(tok));；读取或修改这个对象里的成员状态。
        ssids.push_back(std::string(tok));
        // 赋值：把右侧 strtok(nullptr, ",") 的结果保存到左侧 tok。
        tok = strtok(nullptr, ",");
    }

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 update_ssid_list：参数在括号里，返回类型写在函数名前面。
bool update_ssid_list(const std::vector<std::string>& ssids)
{
    // 判断条件：ssids.empty() 是否成立。
    if (ssids.empty())
    {
        // 执行语句：nvs_handle_t h;；它会更新一次变量、对象或程序状态。
        nvs_handle_t h;
        // 定义变量 esp_err_t ret 并初始化为 nvs_open(NVS_NS, NVS_READWRITE, &h)。
        esp_err_t ret = nvs_open(NVS_NS, NVS_READWRITE, &h);
        // 判断条件：ret == ESP_OK 是否成立。
        if (ret == ESP_OK)
        {
            // 调用 nvs_erase_key(h, NVS_LIST);，执行这个函数并处理它的返回值或状态变化。
            nvs_erase_key(h, NVS_LIST);
            // 调用 nvs_commit(h);，执行这个函数并处理它的返回值或状态变化。
            nvs_commit(h);
            // 调用 nvs_close(h);，执行这个函数并处理它的返回值或状态变化。
            nvs_close(h);
        }
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 执行语句：std::string list;；它会更新一次变量、对象或程序状态。
    std::string list;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (size_t i = 0; i < ssids.size(); i++)
    {
        // 判断条件：i > 0 是否成立。
        if (i > 0)
            // 赋值：把右侧 "," 的结果保存到左侧 list +。
            list += ",";
        // 赋值：把右侧 ssids[i] 的结果保存到左侧 list +。
        list += ssids[i];
    }

    // 执行语句：nvs_handle_t h;；它会更新一次变量、对象或程序状态。
    nvs_handle_t h;
    // 定义变量 esp_err_t ret 并初始化为 nvs_open(NVS_NS, NVS_READWRITE, &h)。
    esp_err_t ret = nvs_open(NVS_NS, NVS_READWRITE, &h);
    // 判断条件：ret != ESP_OK 是否成立。
    if (ret != ESP_OK)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 nvs_set_str(h, NVS_LIST, list.c_str()) 的结果保存到左侧 ret。
    ret = nvs_set_str(h, NVS_LIST, list.c_str());
    // 判断条件：ret == ESP_OK 是否成立。
    if (ret == ESP_OK)
        // 赋值：把右侧 nvs_commit(h) 的结果保存到左侧 ret。
        ret = nvs_commit(h);
    // 调用 nvs_close(h);，执行这个函数并处理它的返回值或状态变化。
    nvs_close(h);

    // 返回 ret == ESP_OK 给调用者，表示当前函数的执行结果。
    return ret == ESP_OK;
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::network::wifi {

// 定义或声明函数 WiFiMgr::get_instance：参数在括号里，返回类型写在函数名前面。
WiFiMgr& WiFiMgr::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static WiFiMgr s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义或声明函数 WiFiMgr::WiFiMgr：参数在括号里，返回类型写在函数名前面。
WiFiMgr::WiFiMgr()
    // 定义或声明函数 init_：参数在括号里，返回类型写在函数名前面。
    : init_(false)
    // 定义或声明函数 conn_timeout_ms_：参数在括号里，返回类型写在函数名前面。
    , conn_timeout_ms_(0)
    // 定义或声明函数 conn_timeout_set_：参数在括号里，返回类型写在函数名前面。
    , conn_timeout_set_(false)
    // 定义或声明函数 conn_start_tick_：参数在括号里，返回类型写在函数名前面。
    , conn_start_tick_(0)
    // 定义或声明函数 disconnecting_：参数在括号里，返回类型写在函数名前面。
    , disconnecting_(false)
    // 定义或声明函数 scanning_：参数在括号里，返回类型写在函数名前面。
    , scanning_(false)
{
}

// 定义或声明函数 WiFiMgr::~WiFiMgr：参数在括号里，返回类型写在函数名前面。
WiFiMgr::~WiFiMgr()
{
    // 判断条件：init_ 是否成立。
    if (init_)
        // 调用 deinit()：创建和销毁队列。
        deinit();
}

// 定义或声明函数 WiFiMgr::init：参数在括号里，返回类型写在函数名前面。
bool WiFiMgr::init()
{
    // 函数调用：std::lock_guard<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::lock_guard<std::mutex> lk(mtx_);
    // 判断条件：init_ 是否成立。
    if (init_)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;

    // 执行语句：nvs_handle_t h;；它会更新一次变量、对象或程序状态。
    nvs_handle_t h;
    // 定义变量 esp_err_t ret 并初始化为 nvs_open(NVS_NS, NVS_READONLY, &h)。
    esp_err_t ret = nvs_open(NVS_NS, NVS_READONLY, &h);
    // 判断条件：ret == ESP_ERR_NVS_NOT_INITIALIZED 是否成立。
    if (ret == ESP_ERR_NVS_NOT_INITIALIZED)
    {
        // 调用 ESP_LOGE(TAG, "NVS 未初始化，请在启动时调用 nvs_flash_init()");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "NVS 未初始化，请在启动时调用 nvs_flash_init()");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 判断条件：ret == ESP_OK 是否成立。
    if (ret == ESP_OK)
        // 调用 nvs_close(h);，执行这个函数并处理它的返回值或状态变化。
        nvs_close(h);

    // 赋值：把右侧 app::sys::event::EventMgr::get_instance() 的结果保存到左侧 auto& evt。
    auto& evt = app::sys::event::EventMgr::get_instance();
    // 判断条件：!evt.is_init() 是否成立。
    if (!evt.is_init())
    {
        // 判断条件：!evt.init() 是否成立。
        if (!evt.init())
        {
            // 调用 ESP_LOGE(TAG, "EventMgr 未就绪");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "EventMgr 未就绪");
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }
    }

    // 赋值：把右侧 esp_netif_init() 的结果保存到左侧 ret。
    ret = esp_netif_init();
    // 判断条件：ret != ESP_OK && ret != ESP_ERR_INVALID_STATE 是否成立。
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
    {
        // 调用 ESP_LOGE(TAG, "esp_netif_init: %s", esp_err_to_name(ret));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "esp_netif_init: %s", esp_err_to_name(ret));
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 调用 esp_netif_create_default_wifi_sta();，执行这个函数并处理它的返回值或状态变化。
    esp_netif_create_default_wifi_sta();

    // 定义变量 wifi_init_config_t cfg 并初始化为 WIFI_INIT_CONFIG_DEFAULT()。
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    // 赋值：把右侧 esp_wifi_init(&cfg) 的结果保存到左侧 ret。
    ret = esp_wifi_init(&cfg);
    // 判断条件：ret != ESP_OK 是否成立。
    if (ret != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "esp_wifi_init: %s", esp_err_to_name(ret));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "esp_wifi_init: %s", esp_err_to_name(ret));
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 函数调用：evt.register_handler(；执行该函数并处理它的返回值或副作用。
    evt.register_handler(
        // 继续上一行的表达式或参数列表：WIFI_EVENT, ESP_EVENT_ANY_ID,。
        WIFI_EVENT, ESP_EVENT_ANY_ID,
        // 继续上一行的表达式或参数列表：[this](esp_event_base_t base, app::sys::event::EventId id,。
        [this](esp_event_base_t base, app::sys::event::EventId id,
               // 只读约束：这个值或对象在当前位置不能被修改。
               const app::sys::event::EventData& data) { handle_wifi_event(base, id, data); });

    // 函数调用：evt.register_handler(；执行该函数并处理它的返回值或副作用。
    evt.register_handler(
        // 继续上一行的表达式或参数列表：IP_EVENT, IP_EVENT_STA_GOT_IP,。
        IP_EVENT, IP_EVENT_STA_GOT_IP,
        // 继续上一行的表达式或参数列表：[this](esp_event_base_t base, app::sys::event::EventId id,。
        [this](esp_event_base_t base, app::sys::event::EventId id,
               // 只读约束：这个值或对象在当前位置不能被修改。
               const app::sys::event::EventData& data) { handle_ip_event(base, id, data); });

    // 赋值：把右侧 esp_wifi_set_mode(WIFI_MODE_STA) 的结果保存到左侧 ret。
    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    // 判断条件：ret != ESP_OK 是否成立。
    if (ret != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "esp_wifi_set_mode: %s", esp_err_to_name(ret));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "esp_wifi_set_mode: %s", esp_err_to_name(ret));
        // 调用 esp_wifi_deinit();，执行这个函数并处理它的返回值或状态变化。
        esp_wifi_deinit();
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 赋值：把右侧 esp_wifi_start() 的结果保存到左侧 ret。
    ret = esp_wifi_start();
    // 判断条件：ret != ESP_OK 是否成立。
    if (ret != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "esp_wifi_start: %s", esp_err_to_name(ret));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "esp_wifi_start: %s", esp_err_to_name(ret));
        // 调用 esp_wifi_deinit();，执行这个函数并处理它的返回值或状态变化。
        esp_wifi_deinit();
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 true 的结果保存到左侧 init_。
    init_ = true;
    // 调用 ESP_LOGD(TAG, "已启动 STA 模式");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "已启动 STA 模式");
    // 赋值：把右侧 State::DISCONNECTED 的结果保存到左侧 info_.state。
    info_.state = State::DISCONNECTED;
    // 赋值：把右侧 false 的结果保存到左侧 conn_timeout_set_。
    conn_timeout_set_ = false;

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 WiFiMgr::deinit：参数在括号里，返回类型写在函数名前面。
void WiFiMgr::deinit()
{
    // 函数调用：std::unique_lock<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::unique_lock<std::mutex> lk(mtx_);
    // 判断条件：!init_ 是否成立。
    if (!init_)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 访问对象成员：lk.unlock();；读取或修改这个对象里的成员状态。
    lk.unlock();
    // 调用 disconnect();，执行这个函数并处理它的返回值或状态变化。
    disconnect();
    // 调用 app::sys::task::TaskMgr::delay_ms(50)：调用 vTaskDelay()，这是让出 CPU 的阻塞延时。一千毫秒延时期间，任务不占 CPU，调度器可以运行别人。。
    app::sys::task::TaskMgr::delay_ms(50);
    // 调用 lock()：为什么使用 trylock() 而不是普通 lock()。
    lk.lock();

    // 调用 esp_wifi_stop();，执行这个函数并处理它的返回值或状态变化。
    esp_wifi_stop();
    // 调用 esp_wifi_deinit();，执行这个函数并处理它的返回值或状态变化。
    esp_wifi_deinit();

    // 赋值：把右侧 app::sys::event::EventMgr::get_instance() 的结果保存到左侧 auto& evt。
    auto& evt = app::sys::event::EventMgr::get_instance();
    // 调用 unregister_handler(WIFI_EVENT,ESP_EVENT_ANY_ID)：注册表键是 {base, id}。。
    evt.unregister_handler(WIFI_EVENT, ESP_EVENT_ANY_ID);
    // 调用 unregister_handler(IP_EVENT,IP_EVENT_STA_GOT_IP)：注册表键是 {base, id}。。
    evt.unregister_handler(IP_EVENT, IP_EVENT_STA_GOT_IP);

    // 赋值：把右侧 false 的结果保存到左侧 init_。
    init_ = false;
}

// 定义或声明函数 WiFiMgr::scan：参数在括号里，返回类型写在函数名前面。
bool WiFiMgr::scan(ScanCb cb)
{
    // 函数调用：std::lock_guard<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::lock_guard<std::mutex> lk(mtx_);
    // 判断条件：!init_ || scanning_ 是否成立。
    if (!init_ || scanning_)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 cb 的结果保存到左侧 scan_cb_。
    scan_cb_ = cb;
    // 赋值：把右侧 true 的结果保存到左侧 scanning_。
    scanning_ = true;

    // 定义变量 wifi_scan_config_t cfg 并初始化为 {}。
    wifi_scan_config_t cfg = {};
    // 赋值：把右侧 WIFI_SCAN_TYPE_ACTIVE 的结果保存到左侧 cfg.scan_type。
    cfg.scan_type = WIFI_SCAN_TYPE_ACTIVE;
    // 赋值：把右侧 100 的结果保存到左侧 cfg.scan_time.active.min。
    cfg.scan_time.active.min = 100;
    // 赋值：把右侧 300 的结果保存到左侧 cfg.scan_time.active.max。
    cfg.scan_time.active.max = 300;

    // 定义变量 esp_err_t ret 并初始化为 esp_wifi_scan_start(&cfg, false)。
    esp_err_t ret = esp_wifi_scan_start(&cfg, false);
    // 判断条件：ret != ESP_OK 是否成立。
    if (ret != ESP_OK)
    {
        // 赋值：把右侧 false 的结果保存到左侧 scanning_。
        scanning_ = false;
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 WiFiMgr::connect：参数在括号里，返回类型写在函数名前面。
bool WiFiMgr::connect(const char* ssid, const char* password, uint32_t timeout_ms)
{
    // 函数调用：std::unique_lock<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::unique_lock<std::mutex> lk(mtx_);
    // 判断条件：!init_ 是否成立。
    if (!init_)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 执行语句：Credentials creds;；它会更新一次变量、对象或程序状态。
    Credentials creds;

    // 判断条件：ssid && ssid[0] != '\0' 是否成立。
    if (ssid && ssid[0] != '\0')
    {
        // 访问对象成员：creds.set_ssid(ssid);；读取或修改这个对象里的成员状态。
        creds.set_ssid(ssid);
        // 判断条件：password 是否成立。
        if (password)
            // 访问对象成员：creds.set_password(password);；读取或修改这个对象里的成员状态。
            creds.set_password(password);
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 访问对象成员：lk.unlock();；读取或修改这个对象里的成员状态。
        lk.unlock();

        // 执行语句：std::vector<Credentials> saved;；它会更新一次变量、对象或程序状态。
        std::vector<Credentials> saved;
        // 判断条件：!get_creds(saved) || saved.empty() 是否成立。
        if (!get_creds(saved) || saved.empty())
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;

        // 判断条件：saved.size() == 1 是否成立。
        if (saved.size() == 1)
        {
            // 赋值：把右侧 saved[0] 的结果保存到左侧 creds。
            creds = saved[0];
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 定义变量 wifi_scan_config_t cfg 并初始化为 {}。
            wifi_scan_config_t cfg = {};
            // 赋值：把右侧 WIFI_SCAN_TYPE_ACTIVE 的结果保存到左侧 cfg.scan_type。
            cfg.scan_type = WIFI_SCAN_TYPE_ACTIVE;
            // 赋值：把右侧 100 的结果保存到左侧 cfg.scan_time.active.min。
            cfg.scan_time.active.min = 100;
            // 赋值：把右侧 300 的结果保存到左侧 cfg.scan_time.active.max。
            cfg.scan_time.active.max = 300;

            // 定义变量 esp_err_t ret 并初始化为 esp_wifi_scan_start(&cfg, true)。
            esp_err_t ret = esp_wifi_scan_start(&cfg, true);
            // 判断条件：ret != ESP_OK 是否成立。
            if (ret != ESP_OK)
            {
                // 赋值：把右侧 saved[0] 的结果保存到左侧 creds。
                creds = saved[0];
            }
            // 备用分支：前面的条件不成立时执行这里。
            else
            {
                // 定义变量 uint16_t cnt 并初始化为 0。
                uint16_t cnt = 0;
                // 调用 esp_wifi_scan_get_ap_num(&cnt);，执行这个函数并处理它的返回值或状态变化。
                esp_wifi_scan_get_ap_num(&cnt);

                // 执行语句：Credentials best;；它会更新一次变量、对象或程序状态。
                Credentials best;
                // 定义变量 int8_t best_rssi 并初始化为 -100。
                int8_t best_rssi = -100;

                // 判断条件：cnt > 0 是否成立。
                if (cnt > 0)
                {
                    // 函数调用：std::vector<wifi_ap_record_t> recs(cnt);；执行该函数并处理它的返回值或副作用。
                    std::vector<wifi_ap_record_t> recs(cnt);
                    // 调用 esp_wifi_scan_get_ap_records(&cnt, recs.data());，执行这个函数并处理它的返回值或状态变化。
                    esp_wifi_scan_get_ap_records(&cnt, recs.data());

                    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
                    for (uint16_t i = 0; i < cnt; i++)
                    {
                        // 只读约束：这个值或对象在当前位置不能被修改。
                        const char* ap_ssid = reinterpret_cast<const char*>(recs[i].ssid);
                        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
                        for (const auto& s : saved)
                        {
                            // 判断条件：strcmp(ap_ssid, s.ssid) == 0 是否成立。
                            if (strcmp(ap_ssid, s.ssid) == 0)
                            {
                                // 判断条件：recs[i].rssi > best_rssi 是否成立。
                                if (recs[i].rssi > best_rssi)
                                {
                                    // 赋值：把右侧 recs[i].rssi 的结果保存到左侧 best_rssi。
                                    best_rssi = recs[i].rssi;
                                    // 赋值：把右侧 s 的结果保存到左侧 best。
                                    best = s;
                                }
                                // 跳出当前循环或 switch，不再执行剩余分支。
                                break;
                            }
                        }
                    }
                }

                // 赋值：把右侧 best.valid() ? best : saved[0] 的结果保存到左侧 creds。
                creds = best.valid() ? best : saved[0];
            }
        }

        // 调用 lock()：为什么使用 trylock() 而不是普通 lock()。
        lk.lock();
    }

    // 判断条件：!creds.valid() 是否成立。
    if (!creds.valid())
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 判断条件：info_.state == State::CONNECTING || info_.state == State::CONNECTED 是否成立。
    if (info_.state == State::CONNECTING || info_.state == State::CONNECTED)
    {
        // 判断条件：strcmp(info_.ssid, creds.ssid) == 0 是否成立。
        if (strcmp(info_.ssid, creds.ssid) == 0)
            // 返回 true 给调用者，表示当前函数的执行结果。
            return true;
        // 访问对象成员：lk.unlock();；读取或修改这个对象里的成员状态。
        lk.unlock();
        // 调用 disconnect();，执行这个函数并处理它的返回值或状态变化。
        disconnect();
        // 调用 app::sys::task::TaskMgr::delay_ms(50)：调用 vTaskDelay()，这是让出 CPU 的阻塞延时。一千毫秒延时期间，任务不占 CPU，调度器可以运行别人。。
        app::sys::task::TaskMgr::delay_ms(50);
        // 调用 lock()：为什么使用 trylock() 而不是普通 lock()。
        lk.lock();
    }

    // 定义变量 wifi_config_t wcfg 并初始化为 {}。
    wifi_config_t wcfg = {};
    // 调用 strncpy(reinterpret_cast<char*>(wcfg.sta.ssid), creds.ssid, sizeof(wcfg.sta.ssid) - 1);，执行这个函数并处理它的返回值或状态变化。
    strncpy(reinterpret_cast<char*>(wcfg.sta.ssid), creds.ssid, sizeof(wcfg.sta.ssid) - 1);
    // 调用 strncpy(reinterpret_cast<char*>(wcfg.sta.password), creds.password,，执行这个函数并处理它的返回值或状态变化。
    strncpy(reinterpret_cast<char*>(wcfg.sta.password), creds.password,
            // 继续上一行的表达式或参数列表：sizeof(wcfg.sta.password) - 1);。
            sizeof(wcfg.sta.password) - 1);
    // 赋值：把右侧 WIFI_AUTH_WPA2_PSK 的结果保存到左侧 wcfg.sta.threshold.authmode。
    wcfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    // 赋值：把右侧 true 的结果保存到左侧 wcfg.sta.pmf_cfg.capable。
    wcfg.sta.pmf_cfg.capable = true;
    // 赋值：把右侧 false 的结果保存到左侧 wcfg.sta.pmf_cfg.required。
    wcfg.sta.pmf_cfg.required = false;

    // 调用 ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wcfg));，执行这个函数并处理它的返回值或状态变化。
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wcfg));

    // 赋值：把右侧 timeout_ms 的结果保存到左侧 conn_timeout_ms_。
    conn_timeout_ms_ = timeout_ms;
    // 赋值：把右侧 true 的结果保存到左侧 conn_timeout_set_。
    conn_timeout_set_ = true;
    // 赋值：把右侧 app::sys::task::TaskMgr::tick_count() 的结果保存到左侧 conn_start_tick_。
    conn_start_tick_ = app::sys::task::TaskMgr::tick_count();

    // 调用 update_state(State::CONNECTING);，执行这个函数并处理它的返回值或状态变化。
    update_state(State::CONNECTING);

    // 判断条件：timeout_ms > 0 && !timeout_task_ 是否成立。
    if (timeout_ms > 0 && !timeout_task_)
    {
        // 执行语句：app::sys::task::Cfg cfg;；它会更新一次变量、对象或程序状态。
        app::sys::task::Cfg cfg;
        // 赋值：把右侧 "wifi_to" 的结果保存到左侧 cfg.name。
        cfg.name = "wifi_to";
        // 赋值：把右侧 2048 的结果保存到左侧 cfg.stack_size。
        cfg.stack_size = 2048;
        // 赋值：把右侧 app::sys::task::Priority::NORMAL 的结果保存到左侧 cfg.priority。
        cfg.priority = app::sys::task::Priority::NORMAL;
        // 赋值：把右侧 -1 的结果保存到左侧 cfg.core_id。
        cfg.core_id = -1;

        // 赋值：把右侧 std::make_unique<app::sys::task::Task>(timeout_task_fn, cfg, this) 的结果保存到左侧 timeout_task_。
        timeout_task_ = std::make_unique<app::sys::task::Task>(timeout_task_fn, cfg, this);
        // 判断条件：!timeout_task_->start() 是否成立。
        if (!timeout_task_->start())
        {
            // 调用 ESP_LOGE(TAG, "超时任务创建失败");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "超时任务创建失败");
            // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
            timeout_task_.reset();
        }
    }

    // 定义变量 esp_err_t ret 并初始化为 esp_wifi_connect()。
    esp_err_t ret = esp_wifi_connect();
    // 判断条件：ret != ESP_OK 是否成立。
    if (ret != ESP_OK)
    {
        // 调用 update_state(State::FAILED, FailureReason::CONNECTION_FAILED);，执行这个函数并处理它的返回值或状态变化。
        update_state(State::FAILED, FailureReason::CONNECTION_FAILED);
        // 调用 clear_timeout_task();，执行这个函数并处理它的返回值或状态变化。
        clear_timeout_task();
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 WiFiMgr::disconnect：参数在括号里，返回类型写在函数名前面。
void WiFiMgr::disconnect()
{
    // 函数调用：std::lock_guard<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::lock_guard<std::mutex> lk(mtx_);
    // 判断条件：!init_ || info_.state == State::DISCONNECTED 是否成立。
    if (!init_ || info_.state == State::DISCONNECTED)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 true 的结果保存到左侧 disconnecting_。
    disconnecting_ = true;
    // 赋值：把右侧 false 的结果保存到左侧 conn_timeout_set_。
    conn_timeout_set_ = false;
    // 调用 clear_timeout_task();，执行这个函数并处理它的返回值或状态变化。
    clear_timeout_task();

    // 调用 esp_wifi_disconnect();，执行这个函数并处理它的返回值或状态变化。
    esp_wifi_disconnect();
}

// 只读约束：这个值或对象在当前位置不能被修改。
const Info& WiFiMgr::get_info() const
{
    // 函数调用：std::lock_guard<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::lock_guard<std::mutex> lk(mtx_);
    // 返回 info_ 给调用者，表示当前函数的执行结果。
    return info_;
}

// 定义或声明函数 WiFiMgr::get_state：参数在括号里，返回类型写在函数名前面。
State WiFiMgr::get_state() const
{
    // 函数调用：std::lock_guard<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::lock_guard<std::mutex> lk(mtx_);
    // 返回 info_.state 给调用者，表示当前函数的执行结果。
    return info_.state;
}

// 定义或声明函数 WiFiMgr::is_connected：参数在括号里，返回类型写在函数名前面。
bool WiFiMgr::is_connected() const
{
    // 函数调用：std::lock_guard<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::lock_guard<std::mutex> lk(mtx_);
    // 返回 info_.state == State::CONNECTED 给调用者，表示当前函数的执行结果。
    return info_.state == State::CONNECTED;
}

// 定义或声明函数 WiFiMgr::set_state_cb：参数在括号里，返回类型写在函数名前面。
void WiFiMgr::set_state_cb(StateCb cb)
{
    // 函数调用：std::lock_guard<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::lock_guard<std::mutex> lk(mtx_);
    // 赋值：把右侧 std::move(cb) 的结果保存到左侧 state_cb_。
    state_cb_ = std::move(cb);
}

// 定义或声明函数 WiFiMgr::save_creds：参数在括号里，返回类型写在函数名前面。
bool WiFiMgr::save_creds(const Credentials& creds)
{
    // 判断条件：!creds.valid() 是否成立。
    if (!creds.valid())
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 执行语句：std::vector<std::string> ssids;；它会更新一次变量、对象或程序状态。
    std::vector<std::string> ssids;
    // 调用 get_ssid_list(ssids);，执行这个函数并处理它的返回值或状态变化。
    get_ssid_list(ssids);

    // 定义变量 bool exists 并初始化为 false。
    bool exists = false;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (const auto& s : ssids)
    {
        // 判断条件：s == creds.ssid 是否成立。
        if (s == creds.ssid)
        {
            // 赋值：把右侧 true 的结果保存到左侧 exists。
            exists = true;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        }
    }

    // 判断条件：!exists 是否成立。
    if (!exists)
    {
        // 访问对象成员：ssids.push_back(std::string(creds.ssid));；读取或修改这个对象里的成员状态。
        ssids.push_back(std::string(creds.ssid));
        // 判断条件：!update_ssid_list(ssids) 是否成立。
        if (!update_ssid_list(ssids))
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
    }

    // 定义变量 std::string key 并初始化为 build_pass_key(creds.ssid)。
    std::string key = build_pass_key(creds.ssid);
    // 执行语句：nvs_handle_t h;；它会更新一次变量、对象或程序状态。
    nvs_handle_t h;
    // 定义变量 esp_err_t ret 并初始化为 nvs_open(NVS_NS, NVS_READWRITE, &h)。
    esp_err_t ret = nvs_open(NVS_NS, NVS_READWRITE, &h);
    // 判断条件：ret != ESP_OK 是否成立。
    if (ret != ESP_OK)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 nvs_set_str(h, key.c_str(), creds.password) 的结果保存到左侧 ret。
    ret = nvs_set_str(h, key.c_str(), creds.password);
    // 判断条件：ret == ESP_OK 是否成立。
    if (ret == ESP_OK)
        // 赋值：把右侧 nvs_commit(h) 的结果保存到左侧 ret。
        ret = nvs_commit(h);
    // 调用 nvs_close(h);，执行这个函数并处理它的返回值或状态变化。
    nvs_close(h);

    // 返回 ret == ESP_OK 给调用者，表示当前函数的执行结果。
    return ret == ESP_OK;
}

// 定义或声明函数 WiFiMgr::get_creds：参数在括号里，返回类型写在函数名前面。
bool WiFiMgr::get_creds(std::vector<Credentials>& creds)
{
    // 调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    creds.clear();

    // 执行语句：std::vector<std::string> ssids;；它会更新一次变量、对象或程序状态。
    std::vector<std::string> ssids;
    // 判断条件：!get_ssid_list(ssids) || ssids.empty() 是否成立。
    if (!get_ssid_list(ssids) || ssids.empty())
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 执行语句：nvs_handle_t h;；它会更新一次变量、对象或程序状态。
    nvs_handle_t h;
    // 定义变量 esp_err_t ret 并初始化为 nvs_open(NVS_NS, NVS_READONLY, &h)。
    esp_err_t ret = nvs_open(NVS_NS, NVS_READONLY, &h);
    // 判断条件：ret != ESP_OK 是否成立。
    if (ret != ESP_OK)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (const auto& ssid : ssids)
    {
        // 执行语句：Credentials c;；它会更新一次变量、对象或程序状态。
        Credentials c;
        // 调用 strncpy(c.ssid, ssid.c_str(), sizeof(c.ssid) - 1);，执行这个函数并处理它的返回值或状态变化。
        strncpy(c.ssid, ssid.c_str(), sizeof(c.ssid) - 1);
        // 赋值：把右侧 '\0' 的结果保存到左侧 c.ssid[sizeof(c.ssid) - 1]。
        c.ssid[sizeof(c.ssid) - 1] = '\0';

        // 定义变量 std::string key 并初始化为 build_pass_key(ssid.c_str())。
        std::string key = build_pass_key(ssid.c_str());
        // 定义变量 size_t sz 并初始化为 sizeof(c.password)。
        size_t sz = sizeof(c.password);
        // 赋值：把右侧 nvs_get_str(h, key.c_str(), c.password, &sz) 的结果保存到左侧 ret。
        ret = nvs_get_str(h, key.c_str(), c.password, &sz);
        // 判断条件：ret == ESP_OK && c.valid() 是否成立。
        if (ret == ESP_OK && c.valid())
        {
            // 访问对象成员：creds.push_back(c);；读取或修改这个对象里的成员状态。
            creds.push_back(c);
        }
    }

    // 调用 nvs_close(h);，执行这个函数并处理它的返回值或状态变化。
    nvs_close(h);
    // 返回 !creds.empty() 给调用者，表示当前函数的执行结果。
    return !creds.empty();
}

// 定义或声明函数 WiFiMgr::clear_creds：参数在括号里，返回类型写在函数名前面。
bool WiFiMgr::clear_creds()
{
    // 执行语句：std::vector<std::string> ssids;；它会更新一次变量、对象或程序状态。
    std::vector<std::string> ssids;
    // 判断条件：!get_ssid_list(ssids) 是否成立。
    if (!get_ssid_list(ssids))
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;

    // 执行语句：nvs_handle_t h;；它会更新一次变量、对象或程序状态。
    nvs_handle_t h;
    // 定义变量 esp_err_t ret 并初始化为 nvs_open(NVS_NS, NVS_READWRITE, &h)。
    esp_err_t ret = nvs_open(NVS_NS, NVS_READWRITE, &h);
    // 判断条件：ret != ESP_OK 是否成立。
    if (ret != ESP_OK)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (const auto& ssid : ssids)
    {
        // 定义变量 std::string key 并初始化为 build_pass_key(ssid.c_str())。
        std::string key = build_pass_key(ssid.c_str());
        // 调用 nvs_erase_key(h, key.c_str());，执行这个函数并处理它的返回值或状态变化。
        nvs_erase_key(h, key.c_str());
    }

    // 调用 nvs_erase_key(h, NVS_LIST);，执行这个函数并处理它的返回值或状态变化。
    nvs_erase_key(h, NVS_LIST);
    // 赋值：把右侧 nvs_commit(h) 的结果保存到左侧 ret。
    ret = nvs_commit(h);
    // 调用 nvs_close(h);，执行这个函数并处理它的返回值或状态变化。
    nvs_close(h);

    // 返回 ret == ESP_OK 给调用者，表示当前函数的执行结果。
    return ret == ESP_OK;
}

// 定义或声明函数 WiFiMgr::remove_creds：参数在括号里，返回类型写在函数名前面。
bool WiFiMgr::remove_creds(const char* ssid)
{
    // 判断条件：!ssid || ssid[0] == '\0' 是否成立。
    if (!ssid || ssid[0] == '\0')
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 执行语句：std::vector<std::string> ssids;；它会更新一次变量、对象或程序状态。
    std::vector<std::string> ssids;
    // 判断条件：!get_ssid_list(ssids) 是否成立。
    if (!get_ssid_list(ssids))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 定义变量 bool found 并初始化为 false。
    bool found = false;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (auto it = ssids.begin(); it != ssids.end(); ++it)
    {
        // 判断条件：*it == ssid 是否成立。
        if (*it == ssid)
        {
            // 访问对象成员：ssids.erase(it);；读取或修改这个对象里的成员状态。
            ssids.erase(it);
            // 赋值：把右侧 true 的结果保存到左侧 found。
            found = true;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        }
    }

    // 判断条件：!found 是否成立。
    if (!found)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 定义变量 std::string key 并初始化为 build_pass_key(ssid)。
    std::string key = build_pass_key(ssid);
    // 执行语句：nvs_handle_t h;；它会更新一次变量、对象或程序状态。
    nvs_handle_t h;
    // 定义变量 esp_err_t ret 并初始化为 nvs_open(NVS_NS, NVS_READWRITE, &h)。
    esp_err_t ret = nvs_open(NVS_NS, NVS_READWRITE, &h);
    // 判断条件：ret != ESP_OK 是否成立。
    if (ret != ESP_OK)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 调用 nvs_erase_key(h, key.c_str());，执行这个函数并处理它的返回值或状态变化。
    nvs_erase_key(h, key.c_str());
    // 判断条件：!update_ssid_list(ssids) 是否成立。
    if (!update_ssid_list(ssids))
    {
        // 调用 nvs_close(h);，执行这个函数并处理它的返回值或状态变化。
        nvs_close(h);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 nvs_commit(h) 的结果保存到左侧 ret。
    ret = nvs_commit(h);
    // 调用 nvs_close(h);，执行这个函数并处理它的返回值或状态变化。
    nvs_close(h);

    // 函数调用：std::unique_lock<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::unique_lock<std::mutex> lk(mtx_);
    // 判断条件：info_.state == State::CONNECTED || info_.state == State::CONNECTING 是否成立。
    if (info_.state == State::CONNECTED || info_.state == State::CONNECTING)
    {
        // 判断条件：strcmp(info_.ssid, ssid) == 0 是否成立。
        if (strcmp(info_.ssid, ssid) == 0)
        {
            // 访问对象成员：lk.unlock();；读取或修改这个对象里的成员状态。
            lk.unlock();
            // 调用 disconnect();，执行这个函数并处理它的返回值或状态变化。
            disconnect();
            // 调用 app::sys::task::TaskMgr::delay_ms(50)：调用 vTaskDelay()，这是让出 CPU 的阻塞延时。一千毫秒延时期间，任务不占 CPU，调度器可以运行别人。。
            app::sys::task::TaskMgr::delay_ms(50);
        }
    }

    // 返回 ret == ESP_OK 给调用者，表示当前函数的执行结果。
    return ret == ESP_OK;
}

// 定义或声明函数 WiFiMgr::has_saved_creds：参数在括号里，返回类型写在函数名前面。
bool WiFiMgr::has_saved_creds()
{
    // 执行语句：std::vector<std::string> ssids;；它会更新一次变量、对象或程序状态。
    std::vector<std::string> ssids;
    // 返回 get_ssid_list(ssids) && !ssids.empty() 给调用者，表示当前函数的执行结果。
    return get_ssid_list(ssids) && !ssids.empty();
}

// 定义或声明函数 WiFiMgr::persist_sta_creds：参数在括号里，返回类型写在函数名前面。
bool WiFiMgr::persist_sta_creds()
{
    // 定义变量 wifi_config_t wcfg 并初始化为 {}。
    wifi_config_t wcfg = {};
    // 判断条件：esp_wifi_get_config(WIFI_IF_STA, &wcfg) != ESP_OK 是否成立。
    if (esp_wifi_get_config(WIFI_IF_STA, &wcfg) != ESP_OK)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* ssid = reinterpret_cast<const char*>(wcfg.sta.ssid);
    // 判断条件：ssid[0] == '\0' 是否成立。
    if (ssid[0] == '\0')
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 执行语句：Credentials c;；它会更新一次变量、对象或程序状态。
    Credentials c;
    // 访问对象成员：c.set_ssid(ssid);；读取或修改这个对象里的成员状态。
    c.set_ssid(ssid);
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    c.set_password(reinterpret_cast<const char*>(wcfg.sta.password));
    // 返回 save_creds(c) 给调用者，表示当前函数的执行结果。
    return save_creds(c);
}

// 函数调用：void WiFiMgr::handle_wifi_event(esp_event_base_t base, app::sys::event::EventId id,；执行该函数并处理它的返回值或副作用。
void WiFiMgr::handle_wifi_event(esp_event_base_t base, app::sys::event::EventId id,
                                // 只读约束：这个值或对象在当前位置不能被修改。
                                const app::sys::event::EventData& data)
{
    // 函数调用：std::unique_lock<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::unique_lock<std::mutex> lk(mtx_);

    // 多分支判断：根据括号里的值选择不同 case。
    switch (id)
    {
        // 分支匹配：case WIFI_EVENT_STA_START 成立时执行下面的代码。
        case WIFI_EVENT_STA_START:
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 分支匹配：case WIFI_EVENT_STA_CONNECTED:  成立时执行下面的代码。
        case WIFI_EVENT_STA_CONNECTED: {
            // 赋值：把右侧 static_cast<wifi_event_sta_connected_t*>(data.data) 的结果保存到左侧 auto* evt。
            auto* evt = static_cast<wifi_event_sta_connected_t*>(data.data);
            // 调用 strncpy(info_.ssid, reinterpret_cast<const char*>(evt->ssid), sizeof(info_.ssid) - 1);，执行这个函数并处理它的返回值或状态变化。
            strncpy(info_.ssid, reinterpret_cast<const char*>(evt->ssid), sizeof(info_.ssid) - 1);
            // 赋值：把右侧 '\0' 的结果保存到左侧 info_.ssid[sizeof(info_.ssid) - 1]。
            info_.ssid[sizeof(info_.ssid) - 1] = '\0';

            // 访问对象成员：lk.unlock();；读取或修改这个对象里的成员状态。
            lk.unlock();
            // 执行语句：wifi_ap_record_t ap;；它会更新一次变量、对象或程序状态。
            wifi_ap_record_t ap;
            // 定义变量 esp_err_t ret 并初始化为 esp_wifi_sta_get_ap_info(&ap)。
            esp_err_t ret = esp_wifi_sta_get_ap_info(&ap);
            // 调用 lock()：为什么使用 trylock() 而不是普通 lock()。
            lk.lock();

            // 赋值：把右侧 (ret == ESP_OK) ? ap.rssi : 0 的结果保存到左侧 info_.rssi。
            info_.rssi = (ret == ESP_OK) ? ap.rssi : 0;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        }

        // 分支匹配：case WIFI_EVENT_STA_DISCONNECTED:  成立时执行下面的代码。
        case WIFI_EVENT_STA_DISCONNECTED: {
            // 赋值：把右侧 static_cast<wifi_event_sta_disconnected_t*>(data.data) 的结果保存到左侧 auto* evt。
            auto* evt = static_cast<wifi_event_sta_disconnected_t*>(data.data);

            // 赋值：把右侧 false 的结果保存到左侧 conn_timeout_set_。
            conn_timeout_set_ = false;
            // 调用 clear_timeout_task();，执行这个函数并处理它的返回值或状态变化。
            clear_timeout_task();

            // 判断条件：disconnecting_ 是否成立。
            if (disconnecting_)
            {
                // 赋值：把右侧 false 的结果保存到左侧 disconnecting_。
                disconnecting_ = false;
                // 调用 update_state(State::DISCONNECTED);，执行这个函数并处理它的返回值或状态变化。
                update_state(State::DISCONNECTED);
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            }

            // 定义变量 FailureReason reason 并初始化为 FailureReason::UNKNOWN。
            FailureReason reason = FailureReason::UNKNOWN;
            // 判断条件：evt->reason == WIFI_REASON_AUTH_EXPIRE | 是否成立。
            if (evt->reason == WIFI_REASON_AUTH_EXPIRE ||
                // 继续上一行的表达式或参数列表：evt->reason == WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT ||。
                evt->reason == WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT ||
                // 继续上一行的表达式或参数列表：evt->reason == WIFI_REASON_AUTH_FAIL)。
                evt->reason == WIFI_REASON_AUTH_FAIL)
            {
                // 赋值：把右侧 FailureReason::WRONG_PASSWORD 的结果保存到左侧 reason。
                reason = FailureReason::WRONG_PASSWORD;
            }
            // 备用分支：前面的条件不成立时执行这里。
            else if (evt->reason == WIFI_REASON_NO_AP_FOUND)
            {
                // 赋值：把右侧 FailureReason::NETWORK_NOT_FOUND 的结果保存到左侧 reason。
                reason = FailureReason::NETWORK_NOT_FOUND;
            }
            // 备用分支：前面的条件不成立时执行这里。
            else if (evt->reason == WIFI_REASON_ASSOC_FAIL)
            {
                // 赋值：把右侧 FailureReason::CONNECTION_FAILED 的结果保存到左侧 reason。
                reason = FailureReason::CONNECTION_FAILED;
            }

            // 判断条件：info_.state == State::CONNECTING 是否成立。
            if (info_.state == State::CONNECTING)
            {
                // 调用 update_state(State::FAILED, reason);，执行这个函数并处理它的返回值或状态变化。
                update_state(State::FAILED, reason);
            }
            // 备用分支：前面的条件不成立时执行这里。
            else
            {
                // 调用 update_state(State::DISCONNECTED);，执行这个函数并处理它的返回值或状态变化。
                update_state(State::DISCONNECTED);
            }
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        }

        // 分支匹配：case WIFI_EVENT_SCAN_DONE:  成立时执行下面的代码。
        case WIFI_EVENT_SCAN_DONE: {
            // 赋值：把右侧 false 的结果保存到左侧 scanning_。
            scanning_ = false;

            // 定义变量 uint16_t cnt 并初始化为 0。
            uint16_t cnt = 0;
            // 调用 esp_wifi_scan_get_ap_num(&cnt);，执行这个函数并处理它的返回值或状态变化。
            esp_wifi_scan_get_ap_num(&cnt);

            // 定义变量 ScanCb cb 并初始化为 scan_cb_。
            ScanCb cb = scan_cb_;
            // 执行语句：std::vector<ApInfo> aps;；它会更新一次变量、对象或程序状态。
            std::vector<ApInfo> aps;

            // 判断条件：cnt > 0 是否成立。
            if (cnt > 0)
            {
                // 访问对象成员：aps.reserve(cnt);；读取或修改这个对象里的成员状态。
                aps.reserve(cnt);
                // 函数调用：std::vector<wifi_ap_record_t> recs(cnt);；执行该函数并处理它的返回值或副作用。
                std::vector<wifi_ap_record_t> recs(cnt);
                // 调用 esp_wifi_scan_get_ap_records(&cnt, recs.data());，执行这个函数并处理它的返回值或状态变化。
                esp_wifi_scan_get_ap_records(&cnt, recs.data());

                // 循环执行：按括号里的初始化、条件和步进规则重复运行。
                for (uint16_t i = 0; i < cnt; i++)
                {
                    // 执行语句：ApInfo a;；它会更新一次变量、对象或程序状态。
                    ApInfo a;
                    // 调用 strncpy(a.ssid, reinterpret_cast<const char*>(recs[i].ssid),，执行这个函数并处理它的返回值或状态变化。
                    strncpy(a.ssid, reinterpret_cast<const char*>(recs[i].ssid),
                            // 继续上一行的表达式或参数列表：sizeof(a.ssid) - 1);。
                            sizeof(a.ssid) - 1);
                    // 赋值：把右侧 '\0' 的结果保存到左侧 a.ssid[sizeof(a.ssid) - 1]。
                    a.ssid[sizeof(a.ssid) - 1] = '\0';
                    // 调用 memcpy(a.bssid, recs[i].bssid, sizeof(a.bssid));，执行这个函数并处理它的返回值或状态变化。
                    memcpy(a.bssid, recs[i].bssid, sizeof(a.bssid));
                    // 赋值：把右侧 recs[i].rssi 的结果保存到左侧 a.rssi。
                    a.rssi = recs[i].rssi;
                    // 赋值：把右侧 recs[i].authmode 的结果保存到左侧 a.authmode。
                    a.authmode = recs[i].authmode;
                    // 赋值：把右侧 (recs[i].authmode != WIFI_AUTH_OPEN) 的结果保存到左侧 a.encrypted。
                    a.encrypted = (recs[i].authmode != WIFI_AUTH_OPEN);
                    // 访问对象成员：aps.push_back(a);；读取或修改这个对象里的成员状态。
                    aps.push_back(a);
                }
            }

            // 访问对象成员：lk.unlock();；读取或修改这个对象里的成员状态。
            lk.unlock();
            // 判断条件：cb 是否成立。
            if (cb)
                // 调用 cb(aps);，执行这个函数并处理它的返回值或状态变化。
                cb(aps);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        }

        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
}

// 函数调用：void WiFiMgr::handle_ip_event(esp_event_base_t base, app::sys::event::EventId id,；执行该函数并处理它的返回值或副作用。
void WiFiMgr::handle_ip_event(esp_event_base_t base, app::sys::event::EventId id,
                              // 只读约束：这个值或对象在当前位置不能被修改。
                              const app::sys::event::EventData& data)
{
    // 函数调用：std::unique_lock<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::unique_lock<std::mutex> lk(mtx_);

    // 判断条件：id == IP_EVENT_STA_GOT_IP 是否成立。
    if (id == IP_EVENT_STA_GOT_IP)
    {
        // 赋值：把右侧 static_cast<ip_event_got_ip_t*>(data.data) 的结果保存到左侧 auto* evt。
        auto* evt = static_cast<ip_event_got_ip_t*>(data.data);
        // 调用 memcpy(info_.ip, &evt->ip_info.ip.addr, 4);，执行这个函数并处理它的返回值或状态变化。
        memcpy(info_.ip, &evt->ip_info.ip.addr, 4);
        // 赋值：把右侧 false 的结果保存到左侧 conn_timeout_set_。
        conn_timeout_set_ = false;
        // 调用 clear_timeout_task();，执行这个函数并处理它的返回值或状态变化。
        clear_timeout_task();
        // 调用 update_state(State::CONNECTED);，执行这个函数并处理它的返回值或状态变化。
        update_state(State::CONNECTED);

        // 访问对象成员：lk.unlock();；读取或修改这个对象里的成员状态。
        lk.unlock();
        // 函数调用：(void)persist_sta_creds();；执行该函数并处理它的返回值或副作用。
        (void)persist_sta_creds();
    }
}

// 定义或声明函数 WiFiMgr::clear_timeout_task：参数在括号里，返回类型写在函数名前面。
void WiFiMgr::clear_timeout_task()
{
    // 判断条件：timeout_task_ 是否成立。
    if (timeout_task_)
    {
        // 调用 destroy()：直接删除，不是优雅停止。
        timeout_task_->destroy();
        // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
        timeout_task_.reset();
    }
}

// 定义或声明函数 WiFiMgr::update_state：参数在括号里，返回类型写在函数名前面。
void WiFiMgr::update_state(State state, FailureReason reason)
{
    // 判断条件：info_.state != state || info_.fail_reason != reason 是否成立。
    if (info_.state != state || info_.fail_reason != reason)
    {
        // 赋值：把右侧 state 的结果保存到左侧 info_.state。
        info_.state = state;
        // 赋值：把右侧 reason 的结果保存到左侧 info_.fail_reason。
        info_.fail_reason = reason;
        // 定义变量 StateCb cb 并初始化为 state_cb_。
        StateCb cb = state_cb_;

        // 判断条件：cb 是否成立。
        if (cb)
        {
            // 访问对象成员：mtx_.unlock();；读取或修改这个对象里的成员状态。
            mtx_.unlock();
            // 调用 cb(state, reason);，执行这个函数并处理它的返回值或状态变化。
            cb(state, reason);
            // 调用 lock()：为什么使用 trylock() 而不是普通 lock()。
            mtx_.lock();
        }
    }
}

// 定义或声明函数 WiFiMgr::check_timeout：参数在括号里，返回类型写在函数名前面。
void WiFiMgr::check_timeout()
{
    // 函数调用：std::unique_lock<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::unique_lock<std::mutex> lk(mtx_);

    // 判断条件：!conn_timeout_set_ || info_.state != State::CONNECTING 是否成立。
    if (!conn_timeout_set_ || info_.state != State::CONNECTING)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 定义变量：uint32_t elapsed =
    uint32_t elapsed =
        // 继续上一行的表达式或参数列表：(app::sys::task::TaskMgr::tick_count() - conn_start_tick_) * portTICK_PERIOD_MS;。
        (app::sys::task::TaskMgr::tick_count() - conn_start_tick_) * portTICK_PERIOD_MS;

    // 判断条件：elapsed >= conn_timeout_ms_ 是否成立。
    if (elapsed >= conn_timeout_ms_)
    {
        // 赋值：把右侧 false 的结果保存到左侧 conn_timeout_set_。
        conn_timeout_set_ = false;
        // 定义变量 StateCb cb 并初始化为 state_cb_。
        StateCb cb = state_cb_;

        // 赋值：把右侧 State::FAILED 的结果保存到左侧 info_.state。
        info_.state = State::FAILED;
        // 赋值：把右侧 FailureReason::TIMEOUT 的结果保存到左侧 info_.fail_reason。
        info_.fail_reason = FailureReason::TIMEOUT;

        // 访问对象成员：lk.unlock();；读取或修改这个对象里的成员状态。
        lk.unlock();
        // 判断条件：cb 是否成立。
        if (cb)
            // 调用 cb(State::FAILED, FailureReason::TIMEOUT);，执行这个函数并处理它的返回值或状态变化。
            cb(State::FAILED, FailureReason::TIMEOUT);
        // 调用 esp_wifi_disconnect();，执行这个函数并处理它的返回值或状态变化。
        esp_wifi_disconnect();
    }
}

// 定义或声明函数 WiFiMgr::timeout_task_fn：参数在括号里，返回类型写在函数名前面。
void WiFiMgr::timeout_task_fn(void* arg)
{
    // 赋值：把右侧 static_cast<WiFiMgr*>(arg) 的结果保存到左侧 WiFiMgr* self。
    WiFiMgr* self = static_cast<WiFiMgr*>(arg);
    // 判断条件：!self 是否成立。
    if (!self)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (true)
    {
        // 调用 app::sys::task::TaskMgr::delay_ms(1000)：调用 vTaskDelay()，这是让出 CPU 的阻塞延时。一千毫秒延时期间，任务不占 CPU，调度器可以运行别人。。
        app::sys::task::TaskMgr::delay_ms(1000);
        // 访问对象成员：self->check_timeout();；读取或修改这个对象里的成员状态。
        self->check_timeout();

        // 访问对象成员：std::lock_guard<std::mutex> lk(self->mtx_);；读取或修改这个对象里的成员状态。
        std::lock_guard<std::mutex> lk(self->mtx_);
        // 判断条件：!self->conn_timeout_set_ || self->info_.state != State::CONNECTING 是否成立。
        if (!self->conn_timeout_set_ || self->info_.state != State::CONNECTING)
        {
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        }
    }
}

// 定义或声明函数 WiFiMgr::enable_ap：参数在括号里，返回类型写在函数名前面。
bool WiFiMgr::enable_ap(const char* ssid, const char* password, uint8_t channel)
{
    // 函数调用：std::lock_guard<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::lock_guard<std::mutex> lk(mtx_);
    // 判断条件：!init_ || ssid == nullptr || ssid[0] == '\0' 是否成立。
    if (!init_ || ssid == nullptr || ssid[0] == '\0')
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 创建默认 AP netif（幂等，重复调用不会出错；首次时建立 DHCP server）
    static bool ap_netif_created = false;
    // 判断条件：!ap_netif_created 是否成立。
    if (!ap_netif_created)
    {
        // 调用 esp_netif_create_default_wifi_ap();，执行这个函数并处理它的返回值或状态变化。
        esp_netif_create_default_wifi_ap();
        // 赋值：把右侧 true 的结果保存到左侧 ap_netif_created。
        ap_netif_created = true;
    }

    // 定义变量 esp_err_t r 并初始化为 esp_wifi_set_mode(WIFI_MODE_APSTA)。
    esp_err_t r = esp_wifi_set_mode(WIFI_MODE_APSTA);
    // 判断条件：r != ESP_OK 是否成立。
    if (r != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "set_mode APSTA: %s", esp_err_to_name(r));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "set_mode APSTA: %s", esp_err_to_name(r));
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 定义变量 wifi_config_t ap_cfg 并初始化为 {}。
    wifi_config_t ap_cfg = {};
    // 调用 std::strncpy(reinterpret_cast<char*>(ap_cfg.ap.ssid), ssid, sizeof(ap_cfg.ap.ssid) - 1);，执行这个函数并处理它的返回值或状态变化。
    std::strncpy(reinterpret_cast<char*>(ap_cfg.ap.ssid), ssid, sizeof(ap_cfg.ap.ssid) - 1);
    // 赋值：把右侧 static_cast<uint8_t>(std::strlen(ssid)) 的结果保存到左侧 ap_cfg.ap.ssid_len。
    ap_cfg.ap.ssid_len = static_cast<uint8_t>(std::strlen(ssid));
    // 赋值：把右侧 (channel == 0 || channel > 13) ? 6 : channel 的结果保存到左侧 ap_cfg.ap.channel。
    ap_cfg.ap.channel = (channel == 0 || channel > 13) ? 6 : channel;
    // 赋值：把右侧 4 的结果保存到左侧 ap_cfg.ap.max_connection。
    ap_cfg.ap.max_connection = 4;
    // 赋值：把右侧 100 的结果保存到左侧 ap_cfg.ap.beacon_interval。
    ap_cfg.ap.beacon_interval = 100;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t pwd_len = (password != nullptr) ? std::strlen(password) : 0;
    // 判断条件：pwd_len >= 8 是否成立。
    if (pwd_len >= 8)
    {
        // 调用 std::strncpy(reinterpret_cast<char*>(ap_cfg.ap.password), password,，执行这个函数并处理它的返回值或状态变化。
        std::strncpy(reinterpret_cast<char*>(ap_cfg.ap.password), password,
                     // 继续上一行的表达式或参数列表：sizeof(ap_cfg.ap.password) - 1);。
                     sizeof(ap_cfg.ap.password) - 1);
        // 赋值：把右侧 WIFI_AUTH_WPA2_PSK 的结果保存到左侧 ap_cfg.ap.authmode。
        ap_cfg.ap.authmode = WIFI_AUTH_WPA2_PSK;
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 赋值：把右侧 WIFI_AUTH_OPEN 的结果保存到左侧 ap_cfg.ap.authmode。
        ap_cfg.ap.authmode = WIFI_AUTH_OPEN;
    }

    // 赋值：把右侧 esp_wifi_set_config(WIFI_IF_AP, &ap_cfg) 的结果保存到左侧 r。
    r = esp_wifi_set_config(WIFI_IF_AP, &ap_cfg);
    // 判断条件：r != ESP_OK 是否成立。
    if (r != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "set_config AP: %s", esp_err_to_name(r));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "set_config AP: %s", esp_err_to_name(r));
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 true 的结果保存到左侧 ap_active_。
    ap_active_ = true;
    // 调用 ESP_LOGI(TAG, "SoftAP enabled: SSID='%s' ch=%u auth=%s", ssid, ap_cfg.ap.channel,，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGI(TAG, "SoftAP enabled: SSID='%s' ch=%u auth=%s", ssid, ap_cfg.ap.channel,
             // 继续上一行的表达式或参数列表：(ap_cfg.ap.authmode == WIFI_AUTH_OPEN) ? "OPEN" : "WPA2");。
             (ap_cfg.ap.authmode == WIFI_AUTH_OPEN) ? "OPEN" : "WPA2");
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 WiFiMgr::disable_ap：参数在括号里，返回类型写在函数名前面。
bool WiFiMgr::disable_ap()
{
    // 函数调用：std::lock_guard<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::lock_guard<std::mutex> lk(mtx_);
    // 判断条件：!init_ 是否成立。
    if (!init_)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：!ap_active_ 是否成立。
    if (!ap_active_)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const esp_err_t r = esp_wifi_set_mode(WIFI_MODE_STA);
    // 判断条件：r != ESP_OK 是否成立。
    if (r != ESP_OK)
    {
        // 调用 ESP_LOGW(TAG, "set_mode STA: %s", esp_err_to_name(r));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGW(TAG, "set_mode STA: %s", esp_err_to_name(r));
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 赋值：把右侧 false 的结果保存到左侧 ap_active_。
    ap_active_ = false;
    // 调用 ESP_LOGD(TAG, "SoftAP disabled");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "SoftAP disabled");
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 WiFiMgr::is_ap_active：参数在括号里，返回类型写在函数名前面。
bool WiFiMgr::is_ap_active() const
{
    // 函数调用：std::lock_guard<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::lock_guard<std::mutex> lk(mtx_);
    // 返回 ap_active_ 给调用者，表示当前函数的执行结果。
    return ap_active_;
}

// 定义或声明函数 WiFiMgr::get_ap_sta_count：参数在括号里，返回类型写在函数名前面。
uint8_t WiFiMgr::get_ap_sta_count() const
{
    // 函数调用：std::lock_guard<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::lock_guard<std::mutex> lk(mtx_);
    // 判断条件：!ap_active_ 是否成立。
    if (!ap_active_)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 声明并默认初始化对象：wifi_sta_list_t list{}。
    wifi_sta_list_t list{};
    // 判断条件：esp_wifi_ap_get_sta_list(&list) != ESP_OK 是否成立。
    if (esp_wifi_ap_get_sta_list(&list) != ESP_OK)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    // 返回 list.num 给调用者，表示当前函数的执行结果。
    return list.num;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::network::wifi
