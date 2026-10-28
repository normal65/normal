/**
 * [初学者导读] 文件：main/app/network/bluetooth/bluetooth.cc
 * 作用：初始化和控制 BLE 广播、连接、设备名和连接数。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：网络回调不在 UI 线程，状态变化通常通过 UiBus 回到页面。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "bluetooth.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "NimBLEDevice.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_log.h"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <algorithm>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <map>

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "Bluetooth";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::network::bluetooth {

// 模块私有映射表：保存需要跨调用共享的键值数据，通常配合互斥锁保护。
static std::map<NimBLECharacteristic*, CharCbs> s_char_cbs;

// 类定义：把数据和操作这些数据的方法绑定在一起。
class ServerCbs : public NimBLEServerCallbacks
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 定义或声明函数 onConnect：参数在括号里，返回类型写在函数名前面。
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override
    {
        // 执行语句：ConnInfo info;；它会更新一次变量、对象或程序状态。
        ConnInfo info;
        // 赋值：把右侧 connInfo.getConnHandle() 的结果保存到左侧 info.conn_handle。
        info.conn_handle = connInfo.getConnHandle();
        // 赋值：把右侧 23 的结果保存到左侧 info.mtu。
        info.mtu = 23;
        // 赋值：把右侧 connInfo.isEncrypted() 的结果保存到左侧 info.encrypted。
        info.encrypted = connInfo.isEncrypted();

        // 定义变量 NimBLEAddress addr 并初始化为 connInfo.getAddress()。
        NimBLEAddress addr = connInfo.getAddress();
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t* val = addr.getVal();
        // 判断条件：val != nullptr 是否成立。
        if (val != nullptr)
        {
            // 调用 memcpy(info.address, val, 6);，执行这个函数并处理它的返回值或状态变化。
            memcpy(info.address, val, 6);
        }

        // 调用 BleMgr::get_instance().on_connect(info);，执行这个函数并处理它的返回值或状态变化。
        BleMgr::get_instance().on_connect(info);
    }

    // 定义或声明函数 onDisconnect：参数在括号里，返回类型写在函数名前面。
    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override
    {
        // 执行语句：ConnInfo info;；它会更新一次变量、对象或程序状态。
        ConnInfo info;
        // 赋值：把右侧 connInfo.getConnHandle() 的结果保存到左侧 info.conn_handle。
        info.conn_handle = connInfo.getConnHandle();

        // 定义变量 NimBLEAddress addr 并初始化为 connInfo.getAddress()。
        NimBLEAddress addr = connInfo.getAddress();
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t* val = addr.getVal();
        // 判断条件：val != nullptr 是否成立。
        if (val != nullptr)
        {
            // 调用 memcpy(info.address, val, 6);，执行这个函数并处理它的返回值或状态变化。
            memcpy(info.address, val, 6);
        }

        // 调用 BleMgr::get_instance().on_disconnect(info, reason);，执行这个函数并处理它的返回值或状态变化。
        BleMgr::get_instance().on_disconnect(info, reason);
    }

    // 定义或声明函数 onMTUChange：参数在括号里，返回类型写在函数名前面。
    void onMTUChange(uint16_t MTU, NimBLEConnInfo& connInfo) override
    {
        // 调用 BleMgr::get_instance().on_mtu_change(MTU, connInfo.getConnHandle());，执行这个函数并处理它的返回值或状态变化。
        BleMgr::get_instance().on_mtu_change(MTU, connInfo.getConnHandle());
    }

    // 定义或声明函数 onPassKeyDisplay：参数在括号里，返回类型写在函数名前面。
    uint32_t onPassKeyDisplay() override
    {
        // 返回 123456 给调用者，表示当前函数的执行结果。
        return 123456;
    }

    // 定义或声明函数 onConfirmPassKey：参数在括号里，返回类型写在函数名前面。
    void onConfirmPassKey(NimBLEConnInfo& connInfo, uint32_t pin) override
    {
        // 调用 NimBLEDevice::injectConfirmPasskey(connInfo, true);，执行这个函数并处理它的返回值或状态变化。
        NimBLEDevice::injectConfirmPasskey(connInfo, true);
    }

    // 定义或声明函数 onAuthenticationComplete：参数在括号里，返回类型写在函数名前面。
    void onAuthenticationComplete(NimBLEConnInfo& connInfo) override
    {
        // 判断条件：!connInfo.isEncrypted() 是否成立。
        if (!connInfo.isEncrypted())
        {
            // 调用 ESP_LOGW(TAG, "认证失败");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGW(TAG, "认证失败");
        }
    }
};

// 类定义：把数据和操作这些数据的方法绑定在一起。
class CharCbsImpl : public NimBLECharacteristicCallbacks
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 定义或声明函数 onRead：参数在括号里，返回类型写在函数名前面。
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override
    {
        // 定义变量并调用函数初始化：调用 find(pCharacteristic)：find(id) 根据 AppId 找对象；entryat(idx) 根据网格索引返回 {id, title, iconcp} 这样的轻量条目。，结果保存到 auto it。
        auto it = s_char_cbs.find(pCharacteristic);
        // 判断条件：it != s_char_cbs.end() && it->second.on_read 是否成立。
        if (it != s_char_cbs.end() && it->second.on_read)
        {
            // 执行语句：ConnInfo info;；它会更新一次变量、对象或程序状态。
            ConnInfo info;
            // 赋值：把右侧 connInfo.getConnHandle() 的结果保存到左侧 info.conn_handle。
            info.conn_handle = connInfo.getConnHandle();
            // 赋值：把右侧 connInfo.isEncrypted() 的结果保存到左侧 info.encrypted。
            info.encrypted = connInfo.isEncrypted();

            // 定义变量 NimBLEAddress addr 并初始化为 connInfo.getAddress()。
            NimBLEAddress addr = connInfo.getAddress();
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint8_t* val = addr.getVal();
            // 判断条件：val != nullptr 是否成立。
            if (val != nullptr)
            {
                // 调用 memcpy(info.address, val, 6);，执行这个函数并处理它的返回值或状态变化。
                memcpy(info.address, val, 6);
            }

            // 访问对象成员：it->second.on_read(pCharacteristic, info);；读取或修改这个对象里的成员状态。
            it->second.on_read(pCharacteristic, info);
        }
    }

    // 定义或声明函数 onWrite：参数在括号里，返回类型写在函数名前面。
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override
    {
        // 定义变量并调用函数初始化：调用 find(pCharacteristic)：find(id) 根据 AppId 找对象；entryat(idx) 根据网格索引返回 {id, title, iconcp} 这样的轻量条目。，结果保存到 auto it。
        auto it = s_char_cbs.find(pCharacteristic);
        // 判断条件：it != s_char_cbs.end() && it->second.on_write 是否成立。
        if (it != s_char_cbs.end() && it->second.on_write)
        {
            // 执行语句：ConnInfo info;；它会更新一次变量、对象或程序状态。
            ConnInfo info;
            // 赋值：把右侧 connInfo.getConnHandle() 的结果保存到左侧 info.conn_handle。
            info.conn_handle = connInfo.getConnHandle();
            // 赋值：把右侧 connInfo.isEncrypted() 的结果保存到左侧 info.encrypted。
            info.encrypted = connInfo.isEncrypted();

            // 定义变量 NimBLEAddress addr 并初始化为 connInfo.getAddress()。
            NimBLEAddress addr = connInfo.getAddress();
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint8_t* addrVal = addr.getVal();
            // 判断条件：addrVal != nullptr 是否成立。
            if (addrVal != nullptr)
            {
                // 调用 memcpy(info.address, addrVal, 6);，执行这个函数并处理它的返回值或状态变化。
                memcpy(info.address, addrVal, 6);
            }

            // 定义变量 NimBLEAttValue chrVal 并初始化为 pCharacteristic->getValue()。
            NimBLEAttValue chrVal = pCharacteristic->getValue();
            // 访问对象成员：it->second.on_write(pCharacteristic, info, chrVal.data(), chrVal.size());；读取或修改这个对象里的成员状态。
            it->second.on_write(pCharacteristic, info, chrVal.data(), chrVal.size());
        }
    }

    // 函数调用：void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo,；执行该函数并处理它的返回值或副作用。
    void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo,
                     // 继续上一行的表达式或参数列表：uint16_t subValue) override。
                     uint16_t subValue) override
    {
        // 定义变量并调用函数初始化：调用 find(pCharacteristic)：find(id) 根据 AppId 找对象；entryat(idx) 根据网格索引返回 {id, title, iconcp} 这样的轻量条目。，结果保存到 auto it。
        auto it = s_char_cbs.find(pCharacteristic);
        // 判断条件：it != s_char_cbs.end() && it->second.on_subscribe 是否成立。
        if (it != s_char_cbs.end() && it->second.on_subscribe)
        {
            // 执行语句：ConnInfo info;；它会更新一次变量、对象或程序状态。
            ConnInfo info;
            // 赋值：把右侧 connInfo.getConnHandle() 的结果保存到左侧 info.conn_handle。
            info.conn_handle = connInfo.getConnHandle();
            // 赋值：把右侧 connInfo.isEncrypted() 的结果保存到左侧 info.encrypted。
            info.encrypted = connInfo.isEncrypted();

            // 定义变量 NimBLEAddress addr 并初始化为 connInfo.getAddress()。
            NimBLEAddress addr = connInfo.getAddress();
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint8_t* val = addr.getVal();
            // 判断条件：val != nullptr 是否成立。
            if (val != nullptr)
            {
                // 调用 memcpy(info.address, val, 6);，执行这个函数并处理它的返回值或状态变化。
                memcpy(info.address, val, 6);
            }

            // 访问对象成员：it->second.on_subscribe(pCharacteristic, info, subValue);；读取或修改这个对象里的成员状态。
            it->second.on_subscribe(pCharacteristic, info, subValue);
        }
    }
};

// 模块私有静态对象 s_server_cbs：只在当前文件或函数内共享。
static ServerCbs s_server_cbs;
// 模块私有静态对象 s_char_cbs_impl：只在当前文件或函数内共享。
static CharCbsImpl s_char_cbs_impl;

// 定义或声明函数 BleMgr::get_instance：参数在括号里，返回类型写在函数名前面。
BleMgr& BleMgr::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static BleMgr s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义或声明函数 BleMgr::BleMgr：参数在括号里，返回类型写在函数名前面。
BleMgr::BleMgr()
    // 定义或声明函数 init_：参数在括号里，返回类型写在函数名前面。
    : init_(false)
    // 定义或声明函数 state_：参数在括号里，返回类型写在函数名前面。
    , state_(State::UNINIT)
    // 定义或声明函数 server_：参数在括号里，返回类型写在函数名前面。
    , server_(nullptr)
    // 定义或声明函数 sec_mode_：参数在括号里，返回类型写在函数名前面。
    , sec_mode_(SecMode::NONE)
    // 定义或声明函数 passkey_：参数在括号里，返回类型写在函数名前面。
    , passkey_(123456)
{
}

// 定义或声明函数 BleMgr::~BleMgr：参数在括号里，返回类型写在函数名前面。
BleMgr::~BleMgr()
{
    // 调用 deinit(true);，执行这个函数并处理它的返回值或状态变化。
    deinit(true);
}

// 定义或声明函数 BleMgr::init：参数在括号里，返回类型写在函数名前面。
bool BleMgr::init(const char* device_name)
{
    // 调用 lock(mtx_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mtx_);

    // 判断条件：init_ 是否成立。
    if (init_)
    {
        // 调用 ESP_LOGW(TAG, "已初始化");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGW(TAG, "已初始化");
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 判断条件：!NimBLEDevice::init(device_name ? device_name : "EmotiPet") 是否成立。
    if (!NimBLEDevice::init(device_name ? device_name : "EmotiPet"))
    {
        // 调用 ESP_LOGE(TAG, "初始化失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "初始化失败");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 device_name 的结果保存到左侧 adv_cfg_.device_name。
    adv_cfg_.device_name = device_name;
    // 赋值：把右侧 true 的结果保存到左侧 init_。
    init_ = true;
    // 赋值：把右侧 State::IDLE 的结果保存到左侧 state_。
    state_ = State::IDLE;

    // 调用 ESP_LOGD(TAG, "初始化完成: %s", device_name);，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "初始化完成: %s", device_name);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 BleMgr::deinit：参数在括号里，返回类型写在函数名前面。
void BleMgr::deinit(bool clear_all)
{
    // 调用 lock(mtx_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mtx_);

    // 判断条件：!init_ 是否成立。
    if (!init_)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 调用 stop_advertising();，执行这个函数并处理它的返回值或状态变化。
    stop_advertising();
    // 调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    s_char_cbs.clear();

    // 调用 NimBLEDevice::deinit(clear_all);，执行这个函数并处理它的返回值或状态变化。
    NimBLEDevice::deinit(clear_all);

    // 赋值：把右侧 nullptr 的结果保存到左侧 server_。
    server_ = nullptr;
    // 调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    services_.clear();
    // 赋值：把右侧 false 的结果保存到左侧 init_。
    init_ = false;
    // 赋值：把右侧 State::UNINIT 的结果保存到左侧 state_。
    state_ = State::UNINIT;
    // 调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    conns_.clear();

    // 调用 ESP_LOGD(TAG, "反初始化完成");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "反初始化完成");
}

// 定义或声明函数 BleMgr::create_service：参数在括号里，返回类型写在函数名前面。
NimBLEService* BleMgr::create_service(const char* uuid)
{
    // 调用 lock(mtx_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mtx_);

    // 判断条件：!init_ 是否成立。
    if (!init_)
    {
        // 调用 ESP_LOGE(TAG, "未初始化");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "未初始化");
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;
    }

    // 判断条件：!server_ 是否成立。
    if (!server_)
    {
        // 赋值：把右侧 NimBLEDevice::createServer() 的结果保存到左侧 server_。
        server_ = NimBLEDevice::createServer();
        // 判断条件：!server_ 是否成立。
        if (!server_)
        {
            // 调用 ESP_LOGE(TAG, "创建服务器失败");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "创建服务器失败");
            // 返回 nullptr 给调用者，表示当前函数的执行结果。
            return nullptr;
        }
        // 访问对象成员：server_->setCallbacks(&s_server_cbs);；读取或修改这个对象里的成员状态。
        server_->setCallbacks(&s_server_cbs);
    }

    // 赋值：把右侧 server_->createService(uuid) 的结果保存到左侧 NimBLEService* svc。
    NimBLEService* svc = server_->createService(uuid);
    // 判断条件：svc 是否成立。
    if (svc)
    {
        // 访问对象成员：services_.push_back(svc);；读取或修改这个对象里的成员状态。
        services_.push_back(svc);
    }
    // 返回 svc 给调用者，表示当前函数的执行结果。
    return svc;
}

// 函数调用：NimBLECharacteristic* BleMgr::create_characteristic(NimBLEService* service, const char* uuid,；执行该函数并处理它的返回值或副作用。
NimBLECharacteristic* BleMgr::create_characteristic(NimBLEService* service, const char* uuid,
                                                    // 继续上一行的表达式或参数列表：uint32_t properties, uint16_t max_len)。
                                                    uint32_t properties, uint16_t max_len)
{
    // 判断条件：!service 是否成立。
    if (!service)
    {
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;
    }

    // 返回 service->createCharacteristic(uuid, properties, max_len) 给调用者，表示当前函数的执行结果。
    return service->createCharacteristic(uuid, properties, max_len);
}

// 定义或声明函数 BleMgr::set_char_cbs：参数在括号里，返回类型写在函数名前面。
void BleMgr::set_char_cbs(NimBLECharacteristic* chr, const CharCbs& cbs)
{
    // 判断条件：!chr 是否成立。
    if (!chr)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 赋值：把右侧 cbs 的结果保存到左侧 s_char_cbs[chr]。
    s_char_cbs[chr] = cbs;
    // 访问对象成员：chr->setCallbacks(&s_char_cbs_impl);；读取或修改这个对象里的成员状态。
    chr->setCallbacks(&s_char_cbs_impl);
}

// 定义或声明函数 BleMgr::start_service：参数在括号里，返回类型写在函数名前面。
bool BleMgr::start_service(NimBLEService* service)
{
    // 服务在 start_server() -> server_->start() 时一并启动
    return service != nullptr;
}

// 定义或声明函数 BleMgr::start_server：参数在括号里，返回类型写在函数名前面。
bool BleMgr::start_server()
{
    // 调用 lock(mtx_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mtx_);

    // 判断条件：!init_ || !server_ 是否成立。
    if (!init_ || !server_)
    {
        // 调用 ESP_LOGE(TAG, "未初始化或服务器不存在");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "未初始化或服务器不存在");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 调用 start()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
    server_->start();
    // 调用 ESP_LOGD(TAG, "服务器已启动，包含 %d 个服务", (int)services_.size());，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "服务器已启动，包含 %d 个服务", (int)services_.size());
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 BleMgr::start_advertising：参数在括号里，返回类型写在函数名前面。
bool BleMgr::start_advertising(const AdvCfg& config, uint32_t duration_ms)
{
    // 执行语句：StateCb cb;；它会更新一次变量、对象或程序状态。
    StateCb cb;
    {
        // 调用 lock(mtx_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mtx_);

        // 判断条件：!init_ 是否成立。
        if (!init_)
        {
            // 调用 ESP_LOGE(TAG, "未初始化");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "未初始化");
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }

        // 判断条件：state_ == State::ADVERTISING 是否成立。
        if (state_ == State::ADVERTISING)
        {
            // 调用 ESP_LOGW(TAG, "已在广播中");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGW(TAG, "已在广播中");
            // 返回 true 给调用者，表示当前函数的执行结果。
            return true;
        }

        // 赋值：把右侧 config 的结果保存到左侧 adv_cfg_。
        adv_cfg_ = config;

        // 赋值：把右侧 NimBLEDevice::getAdvertising() 的结果保存到左侧 NimBLEAdvertising* pAdv。
        NimBLEAdvertising* pAdv = NimBLEDevice::getAdvertising();
        // 判断条件：!pAdv 是否成立。
        if (!pAdv)
        {
            // 调用 ESP_LOGE(TAG, "获取广播对象失败");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "获取广播对象失败");
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }

        // 访问对象成员：pAdv->setName(config.device_name ? config.device_name : "EmotiPet");；读取或修改这个对象里的成员状态。
        pAdv->setName(config.device_name ? config.device_name : "EmotiPet");
        // 访问对象成员：pAdv->setMinInterval(config.min_interval);；读取或修改这个对象里的成员状态。
        pAdv->setMinInterval(config.min_interval);
        // 访问对象成员：pAdv->setMaxInterval(config.max_interval);；读取或修改这个对象里的成员状态。
        pAdv->setMaxInterval(config.max_interval);
        // 访问对象成员：pAdv->enableScanResponse(config.scan_response);；读取或修改这个对象里的成员状态。
        pAdv->enableScanResponse(config.scan_response);

        // 判断条件：config.appearance != 0 是否成立。
        if (config.appearance != 0)
        {
            // 访问对象成员：pAdv->setAppearance(config.appearance);；读取或修改这个对象里的成员状态。
            pAdv->setAppearance(config.appearance);
        }

        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (auto* svc : services_)
        {
            // 访问对象成员：pAdv->addServiceUUID(svc->getUUID());；读取或修改这个对象里的成员状态。
            pAdv->addServiceUUID(svc->getUUID());
        }

        // 判断条件：!pAdv->start(duration_ms) 是否成立。
        if (!pAdv->start(duration_ms))
        {
            // 调用 ESP_LOGE(TAG, "启动广播失败");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "启动广播失败");
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }

        // 赋值：把右侧 State::ADVERTISING 的结果保存到左侧 state_。
        state_ = State::ADVERTISING;
        // 赋值：把右侧 state_cb_ 的结果保存到左侧 cb。
        cb = state_cb_;
    }

    // 判断条件：cb 是否成立。
    if (cb)
    {
        // 调用 cb(State::ADVERTISING);，执行这个函数并处理它的返回值或状态变化。
        cb(State::ADVERTISING);
    }

    // 调用 ESP_LOGD(TAG, "广播已启动: %s", config.device_name);，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "广播已启动: %s", config.device_name);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 BleMgr::stop_advertising：参数在括号里，返回类型写在函数名前面。
bool BleMgr::stop_advertising()
{
    // 执行语句：StateCb cb;；它会更新一次变量、对象或程序状态。
    StateCb cb;
    // 定义变量 State new_state 并初始化为 State::IDLE。
    State new_state = State::IDLE;
    {
        // 调用 lock(mtx_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mtx_);

        // 判断条件：!init_ || state_ != State::ADVERTISING 是否成立。
        if (!init_ || state_ != State::ADVERTISING)
        {
            // 返回 true 给调用者，表示当前函数的执行结果。
            return true;
        }

        // 判断条件：!NimBLEDevice::stopAdvertising() 是否成立。
        if (!NimBLEDevice::stopAdvertising())
        {
            // 调用 ESP_LOGE(TAG, "停止广播失败");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "停止广播失败");
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }

        // 赋值：把右侧 conns_.empty() ? State::IDLE : State::CONNECTED 的结果保存到左侧 new_state。
        new_state = conns_.empty() ? State::IDLE : State::CONNECTED;
        // 赋值：把右侧 new_state 的结果保存到左侧 state_。
        state_ = new_state;
        // 赋值：把右侧 state_cb_ 的结果保存到左侧 cb。
        cb = state_cb_;
    }

    // 判断条件：cb 是否成立。
    if (cb)
    {
        // 调用 cb(new_state);，执行这个函数并处理它的返回值或状态变化。
        cb(new_state);
    }

    // 调用 ESP_LOGD(TAG, "广播已停止");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "广播已停止");
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 BleMgr::disconnect：参数在括号里，返回类型写在函数名前面。
bool BleMgr::disconnect(uint16_t conn_handle, uint8_t reason)
{
    // 调用 lock(mtx_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mtx_);

    // 判断条件：!init_ || !server_ 是否成立。
    if (!init_ || !server_)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 判断条件：conn_handle == 0xFFFF 是否成立。
    if (conn_handle == 0xFFFF)
    {
        // 定义变量 auto peers 并初始化为 server_->getPeerDevices()。
        auto peers = server_->getPeerDevices();
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (auto handle : peers)
        {
            // 访问对象成员：server_->disconnect(handle, reason);；读取或修改这个对象里的成员状态。
            server_->disconnect(handle, reason);
        }
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 返回 server_->disconnect(conn_handle, reason) 给调用者，表示当前函数的执行结果。
    return server_->disconnect(conn_handle, reason);
}

// 定义或声明函数 BleMgr::get_conn_count：参数在括号里，返回类型写在函数名前面。
uint8_t BleMgr::get_conn_count() const
{
    // 调用 lock(mtx_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mtx_);

    // 判断条件：!server_ 是否成立。
    if (!server_)
    {
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    }
    // 返回 server_->getConnectedCount() 给调用者，表示当前函数的执行结果。
    return server_->getConnectedCount();
}

// 定义或声明函数 BleMgr::get_connected：参数在括号里，返回类型写在函数名前面。
std::vector<ConnInfo> BleMgr::get_connected() const
{
    // 调用 lock(mtx_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mtx_);
    // 返回 conns_ 给调用者，表示当前函数的执行结果。
    return conns_;
}

// 定义或声明函数 BleMgr::set_state_cb：参数在括号里，返回类型写在函数名前面。
void BleMgr::set_state_cb(StateCb cb)
{
    // 调用 lock(mtx_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mtx_);
    // 赋值：把右侧 std::move(cb) 的结果保存到左侧 state_cb_。
    state_cb_ = std::move(cb);
}

// 定义或声明函数 BleMgr::set_connect_cb：参数在括号里，返回类型写在函数名前面。
void BleMgr::set_connect_cb(ConnectCb cb)
{
    // 调用 lock(mtx_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mtx_);
    // 赋值：把右侧 std::move(cb) 的结果保存到左侧 connect_cb_。
    connect_cb_ = std::move(cb);
}

// 定义或声明函数 BleMgr::set_disconnect_cb：参数在括号里，返回类型写在函数名前面。
void BleMgr::set_disconnect_cb(DisconnectCb cb)
{
    // 调用 lock(mtx_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mtx_);
    // 赋值：把右侧 std::move(cb) 的结果保存到左侧 disconnect_cb_。
    disconnect_cb_ = std::move(cb);
}

// 定义或声明函数 BleMgr::set_security：参数在括号里，返回类型写在函数名前面。
void BleMgr::set_security(SecMode mode, uint32_t passkey)
{
    // 调用 lock(mtx_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mtx_);

    // 赋值：把右侧 mode 的结果保存到左侧 sec_mode_。
    sec_mode_ = mode;
    // 赋值：把右侧 passkey 的结果保存到左侧 passkey_。
    passkey_ = passkey;

    // 判断条件：!init_ 是否成立。
    if (!init_)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 多分支判断：根据括号里的值选择不同 case。
    switch (mode)
    {
        // 分支匹配：case SecMode::NONE 成立时执行下面的代码。
        case SecMode::NONE:
            // 调用 NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);，执行这个函数并处理它的返回值或状态变化。
            NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SecMode::PASSKEY 成立时执行下面的代码。
        case SecMode::PASSKEY:
            // 调用 NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);，执行这个函数并处理它的返回值或状态变化。
            NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);
            // 调用 NimBLEDevice::setSecurityPasskey(passkey);，执行这个函数并处理它的返回值或状态变化。
            NimBLEDevice::setSecurityPasskey(passkey);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case SecMode::NUMERIC_COMP 成立时执行下面的代码。
        case SecMode::NUMERIC_COMP:
            // 调用 NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO);，执行这个函数并处理它的返回值或状态变化。
            NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
}

// 定义或声明函数 BleMgr::set_mtu：参数在括号里，返回类型写在函数名前面。
bool BleMgr::set_mtu(uint16_t mtu)
{
    // 返回 NimBLEDevice::setMTU(mtu) 给调用者，表示当前函数的执行结果。
    return NimBLEDevice::setMTU(mtu);
}

// 定义或声明函数 BleMgr::get_mtu：参数在括号里，返回类型写在函数名前面。
uint16_t BleMgr::get_mtu() const
{
    // 返回 NimBLEDevice::getMTU() 给调用者，表示当前函数的执行结果。
    return NimBLEDevice::getMTU();
}

// 定义或声明函数 BleMgr::set_tx_power：参数在括号里，返回类型写在函数名前面。
bool BleMgr::set_tx_power(int8_t dbm)
{
    // 返回 NimBLEDevice::setPower(dbm) 给调用者，表示当前函数的执行结果。
    return NimBLEDevice::setPower(dbm);
}

// 定义或声明函数 BleMgr::get_addr_str：参数在括号里，返回类型写在函数名前面。
std::string BleMgr::get_addr_str() const
{
    // 返回 NimBLEDevice::getAddress().toString() 给调用者，表示当前函数的执行结果。
    return NimBLEDevice::getAddress().toString();
}

// 定义或声明函数 BleMgr::on_connect：参数在括号里，返回类型写在函数名前面。
void BleMgr::on_connect(const ConnInfo& info)
{
    // 执行语句：ConnectCb cb;；它会更新一次变量、对象或程序状态。
    ConnectCb cb;
    // 执行语句：StateCb state_cb;；它会更新一次变量、对象或程序状态。
    StateCb state_cb;
    // 定义变量 bool state_changed 并初始化为 false。
    bool state_changed = false;
    {
        // 调用 lock(mtx_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mtx_);
        // 访问对象成员：conns_.push_back(info);；读取或修改这个对象里的成员状态。
        conns_.push_back(info);
        // 赋值：把右侧 connect_cb_ 的结果保存到左侧 cb。
        cb = connect_cb_;

        // 判断条件：state_ != State::CONNECTED 是否成立。
        if (state_ != State::CONNECTED)
        {
            // 赋值：把右侧 State::CONNECTED 的结果保存到左侧 state_。
            state_ = State::CONNECTED;
            // 赋值：把右侧 true 的结果保存到左侧 state_changed。
            state_changed = true;
            // 赋值：把右侧 state_cb_ 的结果保存到左侧 state_cb。
            state_cb = state_cb_;
        }
    }

    // 判断条件：state_changed && state_cb 是否成立。
    if (state_changed && state_cb)
    {
        // 调用 state_cb(State::CONNECTED);，执行这个函数并处理它的返回值或状态变化。
        state_cb(State::CONNECTED);
    }

    // 判断条件：cb 是否成立。
    if (cb)
    {
        // 调用 cb(info);，执行这个函数并处理它的返回值或状态变化。
        cb(info);
    }

    // 调用 ESP_LOGD(TAG, "设备已连接: 句柄=%d", info.conn_handle);，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "设备已连接: 句柄=%d", info.conn_handle);
}

// 定义或声明函数 BleMgr::on_disconnect：参数在括号里，返回类型写在函数名前面。
void BleMgr::on_disconnect(const ConnInfo& info, int reason)
{
    // 执行语句：DisconnectCb cb;；它会更新一次变量、对象或程序状态。
    DisconnectCb cb;
    // 执行语句：StateCb state_cb;；它会更新一次变量、对象或程序状态。
    StateCb state_cb;
    // 定义变量 bool state_changed 并初始化为 false。
    bool state_changed = false;
    // 定义变量 State new_state 并初始化为 State::IDLE。
    State new_state = State::IDLE;
    {
        // 调用 lock(mtx_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mtx_);
        // 函数调用：conns_.erase(；执行该函数并处理它的返回值或副作用。
        conns_.erase(
            // 继续上一行的表达式或参数列表：std::remove_if(conns_.begin(), conns_.end(),。
            std::remove_if(conns_.begin(), conns_.end(),
                           // 继续上一行的表达式或参数列表：[&](const ConnInfo& c) { return c.conn_handle == info.conn_handle; }),。
                           [&](const ConnInfo& c) { return c.conn_handle == info.conn_handle; }),
            // 继续上一行的表达式或参数列表：conns_.end());。
            conns_.end());

        // 赋值：把右侧 disconnect_cb_ 的结果保存到左侧 cb。
        cb = disconnect_cb_;

        // 判断条件：conns_.empty() && state_ != State::IDLE 是否成立。
        if (conns_.empty() && state_ != State::IDLE)
        {
            // 赋值：把右侧 State::IDLE 的结果保存到左侧 state_。
            state_ = State::IDLE;
            // 赋值：把右侧 true 的结果保存到左侧 state_changed。
            state_changed = true;
            // 赋值：把右侧 state_cb_ 的结果保存到左侧 state_cb。
            state_cb = state_cb_;
        }
    }

    // 判断条件：state_changed && state_cb 是否成立。
    if (state_changed && state_cb)
    {
        // 调用 state_cb(new_state);，执行这个函数并处理它的返回值或状态变化。
        state_cb(new_state);
    }

    // 判断条件：cb 是否成立。
    if (cb)
    {
        // 调用 cb(info, reason);，执行这个函数并处理它的返回值或状态变化。
        cb(info, reason);
    }

    // 调用 ESP_LOGD(TAG, "设备已断开: 句柄=%d, 原因=%d", info.conn_handle, reason);，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "设备已断开: 句柄=%d, 原因=%d", info.conn_handle, reason);
}

// 定义或声明函数 BleMgr::on_mtu_change：参数在括号里，返回类型写在函数名前面。
void BleMgr::on_mtu_change(uint16_t mtu, uint16_t conn_handle)
{
    // 调用 lock(mtx_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mtx_);

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (auto& conn : conns_)
    {
        // 判断条件：conn.conn_handle == conn_handle 是否成立。
        if (conn.conn_handle == conn_handle)
        {
            // 赋值：把右侧 mtu 的结果保存到左侧 conn.mtu。
            conn.mtu = mtu;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        }
    }

    // 调用 ESP_LOGD(TAG, "MTU 已更新: 句柄=%d, MTU=%d", conn_handle, mtu);，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "MTU 已更新: 句柄=%d, MTU=%d", conn_handle, mtu);
}

// 定义或声明函数 BleMgr::update_state：参数在括号里，返回类型写在函数名前面。
void BleMgr::update_state(State new_state)
{
    // 执行语句：StateCb cb;；它会更新一次变量、对象或程序状态。
    StateCb cb;
    {
        // 调用 lock(mtx_)：为什么使用 trylock() 而不是普通 lock()。
        std::lock_guard<std::mutex> lock(mtx_);
        // 判断条件：state_ == new_state 是否成立。
        if (state_ == new_state)
        {
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }
        // 赋值：把右侧 new_state 的结果保存到左侧 state_。
        state_ = new_state;
        // 赋值：把右侧 state_cb_ 的结果保存到左侧 cb。
        cb = state_cb_;
    }

    // 判断条件：cb 是否成立。
    if (cb)
    {
        // 调用 cb(new_state);，执行这个函数并处理它的返回值或状态变化。
        cb(new_state);
    }
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::network::bluetooth
