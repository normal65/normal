/**
 * [初学者导读] 文件：main/app/network/bluetooth/gatt/gatt.cc
 * 作用：实现 BLE GATT 服务和特征值读写、通知。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：网络回调不在 UI 线程，状态变化通常通过 UiBus 回到页面。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gatt.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "NimBLEDevice.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_log.h"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "BLE_GATT";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::network::bluetooth::gatt {

// 定义或声明函数 ProvisionService::get_instance：参数在括号里，返回类型写在函数名前面。
ProvisionService& ProvisionService::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static ProvisionService instance;
    // 返回 instance 给调用者，表示当前函数的执行结果。
    return instance;
}

// 定义或声明函数 ProvisionService::ProvisionService：参数在括号里，返回类型写在函数名前面。
ProvisionService::ProvisionService()
    // 定义或声明函数 created_：参数在括号里，返回类型写在函数名前面。
    : created_(false)
    // 定义或声明函数 service_：参数在括号里，返回类型写在函数名前面。
    , service_(nullptr)
    // 定义或声明函数 ssid_char_：参数在括号里，返回类型写在函数名前面。
    , ssid_char_(nullptr)
    // 定义或声明函数 password_char_：参数在括号里，返回类型写在函数名前面。
    , password_char_(nullptr)
    // 定义或声明函数 status_char_：参数在括号里，返回类型写在函数名前面。
    , status_char_(nullptr)
    // 定义或声明函数 command_char_：参数在括号里，返回类型写在函数名前面。
    , command_char_(nullptr)
    // 定义或声明函数 status_：参数在括号里，返回类型写在函数名前面。
    , status_(ProvisionStatus::IDLE)
{
}

// 定义或声明函数 ProvisionService::create：参数在括号里，返回类型写在函数名前面。
bool ProvisionService::create()
{
    // 判断条件：created_ 是否成立。
    if (created_)
    {
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 赋值：把右侧 BleMgr::get_instance() 的结果保存到左侧 auto& mgr。
    auto& mgr = BleMgr::get_instance();
    // 判断条件：!mgr.is_init() 是否成立。
    if (!mgr.is_init())
    {
        // 调用 ESP_LOGE(TAG, "BLE 未初始化");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "BLE 未初始化");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 mgr.create_service(UUID::PROVISION_SERVICE) 的结果保存到左侧 service_。
    service_ = mgr.create_service(UUID::PROVISION_SERVICE);
    // 判断条件：!service_ 是否成立。
    if (!service_)
    {
        // 调用 ESP_LOGE(TAG, "创建配网服务失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "创建配网服务失败");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 函数调用：ssid_char_ = mgr.create_characteristic(service_, UUID::WIFI_SSID_CHAR,；执行该函数并处理它的返回值或副作用。
    ssid_char_ = mgr.create_characteristic(service_, UUID::WIFI_SSID_CHAR,
                                           // 继续上一行的表达式或参数列表：NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ, 32);。
                                           NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ, 32);

    // 执行这条代码：password_char_ =。它会更新变量、对象状态或调用链。
    password_char_ =
        // 继续上一行的表达式或参数列表：mgr.create_characteristic(service_, UUID::WIFI_PASSWORD_CHAR, NIMBLE_PROPERTY::WRITE, 64);。
        mgr.create_characteristic(service_, UUID::WIFI_PASSWORD_CHAR, NIMBLE_PROPERTY::WRITE, 64);

    // 函数调用：status_char_ = mgr.create_characteristic(service_, UUID::WIFI_STATUS_CHAR,；执行该函数并处理它的返回值或副作用。
    status_char_ = mgr.create_characteristic(service_, UUID::WIFI_STATUS_CHAR,
                                             // 继续上一行的表达式或参数列表：NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY, 32);。
                                             NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY, 32);

    // 执行这条代码：command_char_ =。它会更新变量、对象状态或调用链。
    command_char_ =
        // 继续上一行的表达式或参数列表：mgr.create_characteristic(service_, UUID::WIFI_COMMAND_CHAR, NIMBLE_PROPERTY::WRITE, 16);。
        mgr.create_characteristic(service_, UUID::WIFI_COMMAND_CHAR, NIMBLE_PROPERTY::WRITE, 16);

    // 判断条件：!ssid_char_ || !password_char_ || !status_char_ || !command_char_ 是否成立。
    if (!ssid_char_ || !password_char_ || !status_char_ || !command_char_)
    {
        // 调用 ESP_LOGE(TAG, "创建特征失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "创建特征失败");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 执行语句：CharCbs ssid_cbs;；它会更新一次变量、对象或程序状态。
    CharCbs ssid_cbs;
    // 执行这条代码：ssid_cbs.on_write = [this](NimBLECharacteristic* chr, const ConnInfo& conn, const uint8_t* data,。它会更新变量、对象状态或调用链。
    ssid_cbs.on_write = [this](NimBLECharacteristic* chr, const ConnInfo& conn, const uint8_t* data,
                               // 开始代码块：下面是这个函数、分支或循环的主体。
                               size_t len) {
        // 调用 on_ssid_write(chr, conn, data, len);，执行这个函数并处理它的返回值或状态变化。
        on_ssid_write(chr, conn, data, len);
    };
    // 访问对象成员：mgr.set_char_cbs(ssid_char_, ssid_cbs);；读取或修改这个对象里的成员状态。
    mgr.set_char_cbs(ssid_char_, ssid_cbs);

    // 执行语句：CharCbs pwd_cbs;；它会更新一次变量、对象或程序状态。
    CharCbs pwd_cbs;
    // 执行这条代码：pwd_cbs.on_write = [this](NimBLECharacteristic* chr, const ConnInfo& conn, const uint8_t* data,。它会更新变量、对象状态或调用链。
    pwd_cbs.on_write = [this](NimBLECharacteristic* chr, const ConnInfo& conn, const uint8_t* data,
                              // 开始代码块：下面是这个函数、分支或循环的主体。
                              size_t len) {
        // 调用 on_password_write(chr, conn, data, len);，执行这个函数并处理它的返回值或状态变化。
        on_password_write(chr, conn, data, len);
    };
    // 访问对象成员：mgr.set_char_cbs(password_char_, pwd_cbs);；读取或修改这个对象里的成员状态。
    mgr.set_char_cbs(password_char_, pwd_cbs);

    // 执行语句：CharCbs cmd_cbs;；它会更新一次变量、对象或程序状态。
    CharCbs cmd_cbs;
    // 执行这条代码：cmd_cbs.on_write = [this](NimBLECharacteristic* chr, const ConnInfo& conn, const uint8_t* data,。它会更新变量、对象状态或调用链。
    cmd_cbs.on_write = [this](NimBLECharacteristic* chr, const ConnInfo& conn, const uint8_t* data,
                              // 开始代码块：下面是这个函数、分支或循环的主体。
                              size_t len) {
        // 调用 on_command_write(chr, conn, data, len);，执行这个函数并处理它的返回值或状态变化。
        on_command_write(chr, conn, data, len);
    };
    // 访问对象成员：mgr.set_char_cbs(command_char_, cmd_cbs);；读取或修改这个对象里的成员状态。
    mgr.set_char_cbs(command_char_, cmd_cbs);

    // 定义变量 uint8_t initial_status 并初始化为 static_cast<uint8_t>(ProvisionStatus::IDLE)。
    uint8_t initial_status = static_cast<uint8_t>(ProvisionStatus::IDLE);
    // 访问对象成员：status_char_->setValue(&initial_status, 1);；读取或修改这个对象里的成员状态。
    status_char_->setValue(&initial_status, 1);

    // 访问对象成员：mgr.start_service(service_);；读取或修改这个对象里的成员状态。
    mgr.start_service(service_);

    // 赋值：把右侧 true 的结果保存到左侧 created_。
    created_ = true;
    // 调用 ESP_LOGD(TAG, "配网服务创建完成");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "配网服务创建完成");
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 ProvisionService::set_connect_cb：参数在括号里，返回类型写在函数名前面。
void ProvisionService::set_connect_cb(ProvisionConnectCb cb)
{
    // 赋值：把右侧 std::move(cb) 的结果保存到左侧 connect_cb_。
    connect_cb_ = std::move(cb);
}

// 定义或声明函数 ProvisionService::set_disconnect_cb：参数在括号里，返回类型写在函数名前面。
void ProvisionService::set_disconnect_cb(ProvisionDisconnectCb cb)
{
    // 赋值：把右侧 std::move(cb) 的结果保存到左侧 disconnect_cb_。
    disconnect_cb_ = std::move(cb);
}

// 定义或声明函数 ProvisionService::set_command_cb：参数在括号里，返回类型写在函数名前面。
void ProvisionService::set_command_cb(ProvisionCommandCb cb)
{
    // 赋值：把右侧 std::move(cb) 的结果保存到左侧 command_cb_。
    command_cb_ = std::move(cb);
}

// 函数定义/声明：调用 ProvisionService::update_status()：更新状态并调用状态回调，同时根据状态决定是否调用完成回调。。
void ProvisionService::update_status(ProvisionStatus status)
{
    // 赋值：把右侧 status 的结果保存到左侧 status_。
    status_ = status;
    // 判断条件：status_char_ 是否成立。
    if (status_char_)
    {
        // 定义变量 uint8_t val 并初始化为 static_cast<uint8_t>(status)。
        uint8_t val = static_cast<uint8_t>(status);
        // 访问对象成员：status_char_->setValue(&val, 1);；读取或修改这个对象里的成员状态。
        status_char_->setValue(&val, 1);
        // 访问对象成员：status_char_->notify();；读取或修改这个对象里的成员状态。
        status_char_->notify();
    }
}

// 定义或声明函数 ProvisionService::send_status_data：参数在括号里，返回类型写在函数名前面。
void ProvisionService::send_status_data(const uint8_t* data, size_t len)
{
    // 判断条件：status_char_ && data != nullptr && len > 0 是否成立。
    if (status_char_ && data != nullptr && len > 0)
    {
        // 访问对象成员：status_char_->notify(data, len);；读取或修改这个对象里的成员状态。
        status_char_->notify(data, len);
    }
}

// 函数调用：void ProvisionService::on_ssid_write(NimBLECharacteristic* chr, const ConnInfo& conn,；执行该函数并处理它的返回值或副作用。
void ProvisionService::on_ssid_write(NimBLECharacteristic* chr, const ConnInfo& conn,
                                     // 只读约束：这个值或对象在当前位置不能被修改。
                                     const uint8_t* data, size_t len)
{
    // 执行语句：(void)chr;；它会更新一次变量、对象或程序状态。
    (void)chr;
    // 执行语句：(void)conn;；它会更新一次变量、对象或程序状态。
    (void)conn;
    // 判断条件：data != nullptr && len > 0 && len < 32 是否成立。
    if (data != nullptr && len > 0 && len < 32)
    {
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        ssid_.assign(reinterpret_cast<const char*>(data), len);
        // 调用 ESP_LOGD(TAG, "SSID 已设置: %s", ssid_.c_str());，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGD(TAG, "SSID 已设置: %s", ssid_.c_str());
    }
}

// 函数调用：void ProvisionService::on_password_write(NimBLECharacteristic* chr, const ConnInfo& conn,；执行该函数并处理它的返回值或副作用。
void ProvisionService::on_password_write(NimBLECharacteristic* chr, const ConnInfo& conn,
                                         // 只读约束：这个值或对象在当前位置不能被修改。
                                         const uint8_t* data, size_t len)
{
    // 执行语句：(void)chr;；它会更新一次变量、对象或程序状态。
    (void)chr;
    // 执行语句：(void)conn;；它会更新一次变量、对象或程序状态。
    (void)conn;
    // 判断条件：data != nullptr && len > 0 && len < 64 是否成立。
    if (data != nullptr && len > 0 && len < 64)
    {
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        password_.assign(reinterpret_cast<const char*>(data), len);
        // 调用 ESP_LOGD(TAG, "密码已设置 len=%u", static_cast<unsigned>(len));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGD(TAG, "密码已设置 len=%u", static_cast<unsigned>(len));
    }
}

// 函数调用：void ProvisionService::on_command_write(NimBLECharacteristic* chr, const ConnInfo& conn,；执行该函数并处理它的返回值或副作用。
void ProvisionService::on_command_write(NimBLECharacteristic* chr, const ConnInfo& conn,
                                        // 只读约束：这个值或对象在当前位置不能被修改。
                                        const uint8_t* data, size_t len)
{
    // 执行语句：(void)chr;；它会更新一次变量、对象或程序状态。
    (void)chr;
    // 执行语句：(void)conn;；它会更新一次变量、对象或程序状态。
    (void)conn;
    // 判断条件：data == nullptr || len < 1 是否成立。
    if (data == nullptr || len < 1)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 定义变量 auto cmd 并初始化为 static_cast<ProvisionCommand>(data[0])。
    auto cmd = static_cast<ProvisionCommand>(data[0]);
    // 调用 ESP_LOGD(TAG, "收到命令: 0x%02X", data[0]);，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "收到命令: 0x%02X", data[0]);

    // 多分支判断：根据括号里的值选择不同 case。
    switch (cmd)
    {
        // 分支匹配：case ProvisionCommand::CONNECT 成立时执行下面的代码。
        case ProvisionCommand::CONNECT:
            // 判断条件：!ssid_.empty() && connect_cb_ 是否成立。
            if (!ssid_.empty() && connect_cb_)
            {
                // 调用 connect_cb_(ssid_.c_str(), password_.c_str());，执行这个函数并处理它的返回值或状态变化。
                connect_cb_(ssid_.c_str(), password_.c_str());
            }
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 分支匹配：case ProvisionCommand::DISCONNECT 成立时执行下面的代码。
        case ProvisionCommand::DISCONNECT:
            // 判断条件：disconnect_cb_ 是否成立。
            if (disconnect_cb_)
            {
                // 调用 disconnect_cb_();，执行这个函数并处理它的返回值或状态变化。
                disconnect_cb_();
            }
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 判断条件：command_cb_ 是否成立。
            if (command_cb_)
            {
                // 调用 command_cb_(cmd);，执行这个函数并处理它的返回值或状态变化。
                command_cb_(cmd);
            }
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
}

// 定义或声明函数 DeviceInfoService::get_instance：参数在括号里，返回类型写在函数名前面。
DeviceInfoService& DeviceInfoService::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static DeviceInfoService instance;
    // 返回 instance 给调用者，表示当前函数的执行结果。
    return instance;
}

// 定义或声明函数 DeviceInfoService::DeviceInfoService：参数在括号里，返回类型写在函数名前面。
DeviceInfoService::DeviceInfoService()
    // 定义或声明函数 created_：参数在括号里，返回类型写在函数名前面。
    : created_(false)
    // 定义或声明函数 service_：参数在括号里，返回类型写在函数名前面。
    , service_(nullptr)
{
}

// 函数调用：bool DeviceInfoService::create(const char* manufacturer, const char* model, const char* serial,；执行该函数并处理它的返回值或副作用。
bool DeviceInfoService::create(const char* manufacturer, const char* model, const char* serial,
                               // 只读约束：这个值或对象在当前位置不能被修改。
                               const char* firmware_rev, const char* hardware_rev,
                               // 只读约束：这个值或对象在当前位置不能被修改。
                               const char* software_rev)
{
    // 判断条件：created_ 是否成立。
    if (created_)
    {
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 赋值：把右侧 BleMgr::get_instance() 的结果保存到左侧 auto& mgr。
    auto& mgr = BleMgr::get_instance();
    // 判断条件：!mgr.is_init() 是否成立。
    if (!mgr.is_init())
    {
        // 调用 ESP_LOGE(TAG, "BLE 未初始化");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "BLE 未初始化");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 mgr.create_service(UUID::DEVICE_INFO_SERVICE) 的结果保存到左侧 service_。
    service_ = mgr.create_service(UUID::DEVICE_INFO_SERVICE);
    // 判断条件：!service_ 是否成立。
    if (!service_)
    {
        // 调用 ESP_LOGE(TAG, "创建设备信息服务失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "创建设备信息服务失败");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 判断条件：manufacturer != nullptr 是否成立。
    if (manufacturer != nullptr)
    {
        // 执行这条代码：auto* chr =。它会更新变量、对象状态或调用链。
        auto* chr =
            // 继续上一行的表达式或参数列表：mgr.create_characteristic(service_, UUID::MANUFACTURER_CHAR, NIMBLE_PROPERTY::READ);。
            mgr.create_characteristic(service_, UUID::MANUFACTURER_CHAR, NIMBLE_PROPERTY::READ);
        // 判断条件：chr != nullptr 是否成立。
        if (chr != nullptr)
        {
            // 访问对象成员：chr->setValue(manufacturer);；读取或修改这个对象里的成员状态。
            chr->setValue(manufacturer);
        }
    }

    // 判断条件：model != nullptr 是否成立。
    if (model != nullptr)
    {
        // 执行这条代码：auto* chr =。它会更新变量、对象状态或调用链。
        auto* chr =
            // 继续上一行的表达式或参数列表：mgr.create_characteristic(service_, UUID::MODEL_NUMBER_CHAR, NIMBLE_PROPERTY::READ);。
            mgr.create_characteristic(service_, UUID::MODEL_NUMBER_CHAR, NIMBLE_PROPERTY::READ);
        // 判断条件：chr != nullptr 是否成立。
        if (chr != nullptr)
        {
            // 访问对象成员：chr->setValue(model);；读取或修改这个对象里的成员状态。
            chr->setValue(model);
        }
    }

    // 判断条件：serial != nullptr 是否成立。
    if (serial != nullptr)
    {
        // 执行这条代码：auto* chr =。它会更新变量、对象状态或调用链。
        auto* chr =
            // 继续上一行的表达式或参数列表：mgr.create_characteristic(service_, UUID::SERIAL_NUMBER_CHAR, NIMBLE_PROPERTY::READ);。
            mgr.create_characteristic(service_, UUID::SERIAL_NUMBER_CHAR, NIMBLE_PROPERTY::READ);
        // 判断条件：chr != nullptr 是否成立。
        if (chr != nullptr)
        {
            // 访问对象成员：chr->setValue(serial);；读取或修改这个对象里的成员状态。
            chr->setValue(serial);
        }
    }

    // 判断条件：firmware_rev != nullptr 是否成立。
    if (firmware_rev != nullptr)
    {
        // 执行这条代码：auto* chr =。它会更新变量、对象状态或调用链。
        auto* chr =
            // 继续上一行的表达式或参数列表：mgr.create_characteristic(service_, UUID::FIRMWARE_REV_CHAR, NIMBLE_PROPERTY::READ);。
            mgr.create_characteristic(service_, UUID::FIRMWARE_REV_CHAR, NIMBLE_PROPERTY::READ);
        // 判断条件：chr != nullptr 是否成立。
        if (chr != nullptr)
        {
            // 访问对象成员：chr->setValue(firmware_rev);；读取或修改这个对象里的成员状态。
            chr->setValue(firmware_rev);
        }
    }

    // 判断条件：hardware_rev != nullptr 是否成立。
    if (hardware_rev != nullptr)
    {
        // 执行这条代码：auto* chr =。它会更新变量、对象状态或调用链。
        auto* chr =
            // 继续上一行的表达式或参数列表：mgr.create_characteristic(service_, UUID::HARDWARE_REV_CHAR, NIMBLE_PROPERTY::READ);。
            mgr.create_characteristic(service_, UUID::HARDWARE_REV_CHAR, NIMBLE_PROPERTY::READ);
        // 判断条件：chr != nullptr 是否成立。
        if (chr != nullptr)
        {
            // 访问对象成员：chr->setValue(hardware_rev);；读取或修改这个对象里的成员状态。
            chr->setValue(hardware_rev);
        }
    }

    // 判断条件：software_rev != nullptr 是否成立。
    if (software_rev != nullptr)
    {
        // 执行这条代码：auto* chr =。它会更新变量、对象状态或调用链。
        auto* chr =
            // 继续上一行的表达式或参数列表：mgr.create_characteristic(service_, UUID::SOFTWARE_REV_CHAR, NIMBLE_PROPERTY::READ);。
            mgr.create_characteristic(service_, UUID::SOFTWARE_REV_CHAR, NIMBLE_PROPERTY::READ);
        // 判断条件：chr != nullptr 是否成立。
        if (chr != nullptr)
        {
            // 访问对象成员：chr->setValue(software_rev);；读取或修改这个对象里的成员状态。
            chr->setValue(software_rev);
        }
    }

    // 访问对象成员：mgr.start_service(service_);；读取或修改这个对象里的成员状态。
    mgr.start_service(service_);

    // 赋值：把右侧 true 的结果保存到左侧 created_。
    created_ = true;
    // 调用 ESP_LOGD(TAG, "设备信息服务创建完成");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "设备信息服务创建完成");
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 BatteryService::get_instance：参数在括号里，返回类型写在函数名前面。
BatteryService& BatteryService::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static BatteryService instance;
    // 返回 instance 给调用者，表示当前函数的执行结果。
    return instance;
}

// 定义或声明函数 BatteryService::BatteryService：参数在括号里，返回类型写在函数名前面。
BatteryService::BatteryService()
    // 定义或声明函数 created_：参数在括号里，返回类型写在函数名前面。
    : created_(false)
    // 定义或声明函数 service_：参数在括号里，返回类型写在函数名前面。
    , service_(nullptr)
    // 定义或声明函数 level_char_：参数在括号里，返回类型写在函数名前面。
    , level_char_(nullptr)
{
}

// 定义或声明函数 BatteryService::create：参数在括号里，返回类型写在函数名前面。
bool BatteryService::create()
{
    // 判断条件：created_ 是否成立。
    if (created_)
    {
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 赋值：把右侧 BleMgr::get_instance() 的结果保存到左侧 auto& mgr。
    auto& mgr = BleMgr::get_instance();
    // 判断条件：!mgr.is_init() 是否成立。
    if (!mgr.is_init())
    {
        // 调用 ESP_LOGE(TAG, "BLE 未初始化");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "BLE 未初始化");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 mgr.create_service(UUID::BATTERY_SERVICE) 的结果保存到左侧 service_。
    service_ = mgr.create_service(UUID::BATTERY_SERVICE);
    // 判断条件：!service_ 是否成立。
    if (!service_)
    {
        // 调用 ESP_LOGE(TAG, "创建电池服务失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "创建电池服务失败");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 函数调用：level_char_ = mgr.create_characteristic(service_, UUID::BATTERY_LEVEL_CHAR,；执行该函数并处理它的返回值或副作用。
    level_char_ = mgr.create_characteristic(service_, UUID::BATTERY_LEVEL_CHAR,
                                            // 继续上一行的表达式或参数列表：NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY, 1);。
                                            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY, 1);

    // 判断条件：!level_char_ 是否成立。
    if (!level_char_)
    {
        // 调用 ESP_LOGE(TAG, "创建电池电量特征失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "创建电池电量特征失败");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 定义变量 uint8_t initial_level 并初始化为 100。
    uint8_t initial_level = 100;
    // 访问对象成员：level_char_->setValue(&initial_level, 1);；读取或修改这个对象里的成员状态。
    level_char_->setValue(&initial_level, 1);

    // 访问对象成员：mgr.start_service(service_);；读取或修改这个对象里的成员状态。
    mgr.start_service(service_);

    // 赋值：把右侧 true 的结果保存到左侧 created_。
    created_ = true;
    // 调用 ESP_LOGD(TAG, "电池服务创建完成");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "电池服务创建完成");
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 BatteryService::update_level：参数在括号里，返回类型写在函数名前面。
void BatteryService::update_level(uint8_t level)
{
    // 判断条件：level > 100 是否成立。
    if (level > 100)
    {
        // 赋值：把右侧 100 的结果保存到左侧 level。
        level = 100;
    }

    // 判断条件：level_char_ 是否成立。
    if (level_char_)
    {
        // 访问对象成员：level_char_->setValue(&level, 1);；读取或修改这个对象里的成员状态。
        level_char_->setValue(&level, 1);
        // 访问对象成员：level_char_->notify();；读取或修改这个对象里的成员状态。
        level_char_->notify();
    }
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::network::bluetooth::gatt
