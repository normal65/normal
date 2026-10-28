// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/network/bluetooth/gatt/gatt.hpp
 * 作用：实现 BLE GATT 服务和特征值读写、通知。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：网络回调不在 UI 线程，状态变化通常通过 UiBus 回到页面。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <functional>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <string>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "bluetooth.hpp"

// 类定义：把数据和操作这些数据的方法绑定在一起。
class NimBLEService;
// 类定义：把数据和操作这些数据的方法绑定在一起。
class NimBLECharacteristic;

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::network::bluetooth::gatt {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace UUID {

// 固定参数 GAP_SERVICE = "1800"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* GAP_SERVICE = "1800";
// 固定参数 GATT_SERVICE = "1801"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* GATT_SERVICE = "1801";
// 固定参数 DEVICE_INFO_SERVICE = "180A"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* DEVICE_INFO_SERVICE = "180A";
// 固定参数 BATTERY_SERVICE = "180F"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* BATTERY_SERVICE = "180F";

// 固定参数 DEVICE_NAME_CHAR = "2A00"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* DEVICE_NAME_CHAR = "2A00";
// 固定参数 APPEARANCE_CHAR = "2A01"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* APPEARANCE_CHAR = "2A01";
// 固定参数 BATTERY_LEVEL_CHAR = "2A19"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* BATTERY_LEVEL_CHAR = "2A19";
// 固定参数 MANUFACTURER_CHAR = "2A29"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* MANUFACTURER_CHAR = "2A29";
// 固定参数 MODEL_NUMBER_CHAR = "2A24"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* MODEL_NUMBER_CHAR = "2A24";
// 固定参数 SERIAL_NUMBER_CHAR = "2A25"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* SERIAL_NUMBER_CHAR = "2A25";
// 固定参数 FIRMWARE_REV_CHAR = "2A26"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* FIRMWARE_REV_CHAR = "2A26";
// 固定参数 HARDWARE_REV_CHAR = "2A27"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* HARDWARE_REV_CHAR = "2A27";
// 固定参数 SOFTWARE_REV_CHAR = "2A28"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* SOFTWARE_REV_CHAR = "2A28";

// 固定参数 PROVISION_SERVICE = "12345678-1234-5678-1234-56789abcdef0"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* PROVISION_SERVICE = "12345678-1234-5678-1234-56789abcdef0";
// 固定参数 WIFI_SSID_CHAR = "12345678-1234-5678-1234-56789abcdef1"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* WIFI_SSID_CHAR = "12345678-1234-5678-1234-56789abcdef1";
// 固定参数 WIFI_PASSWORD_CHAR = "12345678-1234-5678-1234-56789abcdef2"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* WIFI_PASSWORD_CHAR = "12345678-1234-5678-1234-56789abcdef2";
// 固定参数 WIFI_STATUS_CHAR = "12345678-1234-5678-1234-56789abcdef3"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* WIFI_STATUS_CHAR = "12345678-1234-5678-1234-56789abcdef3";
// 固定参数 WIFI_COMMAND_CHAR = "12345678-1234-5678-1234-56789abcdef4"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* WIFI_COMMAND_CHAR = "12345678-1234-5678-1234-56789abcdef4";

// 固定参数 DEVICE_CONTROL_SERVICE = "87654321-4321-8765-4321-fedcba987650"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* DEVICE_CONTROL_SERVICE = "87654321-4321-8765-4321-fedcba987650";
// 固定参数 DEVICE_STATE_CHAR = "87654321-4321-8765-4321-fedcba987651"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* DEVICE_STATE_CHAR = "87654321-4321-8765-4321-fedcba987651";
// 固定参数 DEVICE_COMMAND_CHAR = "87654321-4321-8765-4321-fedcba987652"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* DEVICE_COMMAND_CHAR = "87654321-4321-8765-4321-fedcba987652";
// 固定参数 DEVICE_DATA_CHAR = "87654321-4321-8765-4321-fedcba987653"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* DEVICE_DATA_CHAR = "87654321-4321-8765-4321-fedcba987653";

// 命名空间结束：离开当前模块作用域。
} // namespace UUID

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class ProvisionStatus : uint8_t
{
    // 执行这条代码：IDLE = 0x00,。它会更新变量、对象状态或调用链。
    IDLE = 0x00,
    // 继续上一行的表达式或参数列表：CONNECTING = 0x01,。
    CONNECTING = 0x01,
    // 继续上一行的表达式或参数列表：CONNECTED = 0x02,。
    CONNECTED = 0x02,
    // 继续上一行的表达式或参数列表：FAILED_TIMEOUT = 0x10,。
    FAILED_TIMEOUT = 0x10,
    // 继续上一行的表达式或参数列表：FAILED_WRONG_PWD = 0x11,。
    FAILED_WRONG_PWD = 0x11,
    // 继续上一行的表达式或参数列表：FAILED_NOT_FOUND = 0x12,。
    FAILED_NOT_FOUND = 0x12,
    // 继续上一行的表达式或参数列表：FAILED_UNKNOWN = 0x1F,。
    FAILED_UNKNOWN = 0x1F,
};

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class ProvisionCommand : uint8_t
{
    // 执行这条代码：CONNECT = 0x01,。它会更新变量、对象状态或调用链。
    CONNECT = 0x01,
    // 继续上一行的表达式或参数列表：DISCONNECT = 0x02,。
    DISCONNECT = 0x02,
    // 继续上一行的表达式或参数列表：SCAN = 0x03,。
    SCAN = 0x03,
    // 继续上一行的表达式或参数列表：SAVE = 0x04,。
    SAVE = 0x04,
    // 继续上一行的表达式或参数列表：CLEAR = 0x05,。
    CLEAR = 0x05,
    // 继续上一行的表达式或参数列表：GET_STATUS = 0x10,。
    GET_STATUS = 0x10,
    // 继续上一行的表达式或参数列表：GET_IP = 0x11,。
    GET_IP = 0x11,
};

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ProvisionConnectCb = std::function<void(const char* ssid, const char* password)>;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ProvisionDisconnectCb = std::function<void()>;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ProvisionCommandCb = std::function<void(ProvisionCommand cmd)>;

/** WiFi 配网 GATT；须在 BleMgr::start_server() 之前 create() */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class ProvisionService
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static ProvisionService& get_instance();

    // 函数调用：bool create();；执行该函数并处理它的返回值或副作用。
    bool create();

    // 定义或声明函数 is_created：参数在括号里，返回类型写在函数名前面。
    bool is_created() const
    {
        // 返回 created_ 给调用者，表示当前函数的执行结果。
        return created_;
    }

    // 函数调用：void set_connect_cb(ProvisionConnectCb cb);；执行该函数并处理它的返回值或副作用。
    void set_connect_cb(ProvisionConnectCb cb);
    // 函数调用：void set_disconnect_cb(ProvisionDisconnectCb cb);；执行该函数并处理它的返回值或副作用。
    void set_disconnect_cb(ProvisionDisconnectCb cb);
    // 函数调用：void set_command_cb(ProvisionCommandCb cb);；执行该函数并处理它的返回值或副作用。
    void set_command_cb(ProvisionCommandCb cb);

    // 调用 update_status(ProvisionStatusstatus)：更新状态并调用状态回调，同时根据状态决定是否调用完成回调。。
    void update_status(ProvisionStatus status);

    // 函数调用：void send_status_data(const uint8_t* data, size_t len);；执行该函数并处理它的返回值或副作用。
    void send_status_data(const uint8_t* data, size_t len);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const std::string& get_ssid() const
    {
        // 返回 ssid_ 给调用者，表示当前函数的执行结果。
        return ssid_;
    }
    // 只读约束：这个值或对象在当前位置不能被修改。
    const std::string& get_password() const
    {
        // 返回 password_ 给调用者，表示当前函数的执行结果。
        return password_;
    }

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 ProvisionService();，执行这个函数并处理它的返回值或状态变化。
    ProvisionService();
    // 赋值：把右侧 default 的结果保存到左侧 ~ProvisionService()。
    ~ProvisionService() = default;
    // 调用 ProvisionService(const ProvisionService&) = delete;，执行这个函数并处理它的返回值或状态变化。
    ProvisionService(const ProvisionService&) = delete;
    // 赋值：把右侧 (const ProvisionService&) = delete 的结果保存到左侧 ProvisionService& operator。
    ProvisionService& operator=(const ProvisionService&) = delete;

    // 函数调用：void on_ssid_write(NimBLECharacteristic* chr, const ConnInfo& conn, const uint8_t* data,；执行该函数并处理它的返回值或副作用。
    void on_ssid_write(NimBLECharacteristic* chr, const ConnInfo& conn, const uint8_t* data,
                       // 继续上一行的表达式或参数列表：size_t len);。
                       size_t len);
    // 函数调用：void on_password_write(NimBLECharacteristic* chr, const ConnInfo& conn, const uint8_t* data,；执行该函数并处理它的返回值或副作用。
    void on_password_write(NimBLECharacteristic* chr, const ConnInfo& conn, const uint8_t* data,
                           // 继续上一行的表达式或参数列表：size_t len);。
                           size_t len);
    // 函数调用：void on_command_write(NimBLECharacteristic* chr, const ConnInfo& conn, const uint8_t* data,；执行该函数并处理它的返回值或副作用。
    void on_command_write(NimBLECharacteristic* chr, const ConnInfo& conn, const uint8_t* data,
                          // 继续上一行的表达式或参数列表：size_t len);。
                          size_t len);

    // 执行语句：bool created_;；它会更新一次变量、对象或程序状态。
    bool created_;
    // 执行语句：NimBLEService* service_;；它会更新一次变量、对象或程序状态。
    NimBLEService* service_;
    // 执行语句：NimBLECharacteristic* ssid_char_;；它会更新一次变量、对象或程序状态。
    NimBLECharacteristic* ssid_char_;
    // 执行语句：NimBLECharacteristic* password_char_;；它会更新一次变量、对象或程序状态。
    NimBLECharacteristic* password_char_;
    // 执行语句：NimBLECharacteristic* status_char_;；它会更新一次变量、对象或程序状态。
    NimBLECharacteristic* status_char_;
    // 执行语句：NimBLECharacteristic* command_char_;；它会更新一次变量、对象或程序状态。
    NimBLECharacteristic* command_char_;
    // 执行语句：std::string ssid_;；它会更新一次变量、对象或程序状态。
    std::string ssid_;
    // 执行语句：std::string password_;；它会更新一次变量、对象或程序状态。
    std::string password_;
    // 执行语句：ProvisionStatus status_;；它会更新一次变量、对象或程序状态。
    ProvisionStatus status_;
    // 执行语句：ProvisionConnectCb connect_cb_;；它会更新一次变量、对象或程序状态。
    ProvisionConnectCb connect_cb_;
    // 执行语句：ProvisionDisconnectCb disconnect_cb_;；它会更新一次变量、对象或程序状态。
    ProvisionDisconnectCb disconnect_cb_;
    // 执行语句：ProvisionCommandCb command_cb_;；它会更新一次变量、对象或程序状态。
    ProvisionCommandCb command_cb_;
};

/** 标准设备信息服务 0x180A */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class DeviceInfoService
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static DeviceInfoService& get_instance();

    // 函数调用：bool create(const char* manufacturer = "EmotiPet", const char* model = "EP-001",；执行该函数并处理它的返回值或副作用。
    bool create(const char* manufacturer = "EmotiPet", const char* model = "EP-001",
                // 只读约束：这个值或对象在当前位置不能被修改。
                const char* serial = "000001", const char* firmware_rev = "1.0.0",
                // 只读约束：这个值或对象在当前位置不能被修改。
                const char* hardware_rev = "1.0", const char* software_rev = "1.0.0");

    // 定义或声明函数 is_created：参数在括号里，返回类型写在函数名前面。
    bool is_created() const
    {
        // 返回 created_ 给调用者，表示当前函数的执行结果。
        return created_;
    }

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 DeviceInfoService();，执行这个函数并处理它的返回值或状态变化。
    DeviceInfoService();
    // 赋值：把右侧 default 的结果保存到左侧 ~DeviceInfoService()。
    ~DeviceInfoService() = default;
    // 调用 DeviceInfoService(const DeviceInfoService&) = delete;，执行这个函数并处理它的返回值或状态变化。
    DeviceInfoService(const DeviceInfoService&) = delete;
    // 赋值：把右侧 (const DeviceInfoService&) = delete 的结果保存到左侧 DeviceInfoService& operator。
    DeviceInfoService& operator=(const DeviceInfoService&) = delete;

    // 执行语句：bool created_;；它会更新一次变量、对象或程序状态。
    bool created_;
    // 执行语句：NimBLEService* service_;；它会更新一次变量、对象或程序状态。
    NimBLEService* service_;
};

/** 标准电池服务 0x180F */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class BatteryService
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static BatteryService& get_instance();

    // 函数调用：bool create();；执行该函数并处理它的返回值或副作用。
    bool create();

    // 定义或声明函数 is_created：参数在括号里，返回类型写在函数名前面。
    bool is_created() const
    {
        // 返回 created_ 给调用者，表示当前函数的执行结果。
        return created_;
    }

    // 函数调用：void update_level(uint8_t level);；执行该函数并处理它的返回值或副作用。
    void update_level(uint8_t level);

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 BatteryService();，执行这个函数并处理它的返回值或状态变化。
    BatteryService();
    // 赋值：把右侧 default 的结果保存到左侧 ~BatteryService()。
    ~BatteryService() = default;
    // 调用 BatteryService(const BatteryService&) = delete;，执行这个函数并处理它的返回值或状态变化。
    BatteryService(const BatteryService&) = delete;
    // 赋值：把右侧 (const BatteryService&) = delete 的结果保存到左侧 BatteryService& operator。
    BatteryService& operator=(const BatteryService&) = delete;

    // 执行语句：bool created_;；它会更新一次变量、对象或程序状态。
    bool created_;
    // 执行语句：NimBLEService* service_;；它会更新一次变量、对象或程序状态。
    NimBLEService* service_;
    // 执行语句：NimBLECharacteristic* level_char_;；它会更新一次变量、对象或程序状态。
    NimBLECharacteristic* level_char_;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::network::bluetooth::gatt
