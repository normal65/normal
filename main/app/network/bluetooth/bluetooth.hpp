// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/network/bluetooth/bluetooth.hpp
 * 作用：初始化和控制 BLE 广播、连接、设备名和连接数。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：网络回调不在 UI 线程，状态变化通常通过 UiBus 回到页面。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <functional>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <mutex>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <string>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <vector>

// 类定义：把数据和操作这些数据的方法绑定在一起。
class NimBLEServer;
// 类定义：把数据和操作这些数据的方法绑定在一起。
class NimBLEService;
// 类定义：把数据和操作这些数据的方法绑定在一起。
class NimBLECharacteristic;

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::network::bluetooth {

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class State
{
    // 执行这条代码：UNINIT,。它会更新变量、对象状态或调用链。
    UNINIT,
    // 继续上一行的表达式或参数列表：IDLE,。
    IDLE,
    // 继续上一行的表达式或参数列表：ADVERTISING,。
    ADVERTISING,
    // 继续上一行的表达式或参数列表：CONNECTED,。
    CONNECTED,
};

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class SecMode
{
    // 执行这条代码：NONE,。它会更新变量、对象状态或调用链。
    NONE,
    // 继续上一行的表达式或参数列表：PASSKEY,。
    PASSKEY,
    // 继续上一行的表达式或参数列表：NUMERIC_COMP,。
    NUMERIC_COMP,
};

// struct ConnInfo：把相关数据字段组合在一起，表示项目中的一个概念。
struct ConnInfo
{
    // 执行语句：uint16_t conn_handle;；它会更新一次变量、对象或程序状态。
    uint16_t conn_handle;
    // 执行语句：uint8_t address[6];；它会更新一次变量、对象或程序状态。
    uint8_t address[6];
    // 执行语句：uint16_t mtu;；它会更新一次变量、对象或程序状态。
    uint16_t mtu;
    // 执行语句：bool encrypted;；它会更新一次变量、对象或程序状态。
    bool encrypted;

    // 定义或声明函数 ConnInfo：参数在括号里，返回类型写在函数名前面。
    ConnInfo()
        // 定义或声明函数 conn_handle：参数在括号里，返回类型写在函数名前面。
        : conn_handle(0)
        // 执行这条代码：, address{}。它会更新变量、对象状态或调用链。
        , address{}
        // 定义或声明函数 mtu：参数在括号里，返回类型写在函数名前面。
        , mtu(23)
        // 定义或声明函数 encrypted：参数在括号里，返回类型写在函数名前面。
        , encrypted(false)
    {
    }
};

// struct AdvCfg：把相关数据字段组合在一起，表示项目中的一个概念。
struct AdvCfg
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* device_name;
    // 执行语句：uint16_t min_interval;；它会更新一次变量、对象或程序状态。
    uint16_t min_interval;
    // 执行语句：uint16_t max_interval;；它会更新一次变量、对象或程序状态。
    uint16_t max_interval;
    // 执行语句：bool connectable;；它会更新一次变量、对象或程序状态。
    bool connectable;
    // 执行语句：bool scan_response;；它会更新一次变量、对象或程序状态。
    bool scan_response;
    // 执行语句：uint16_t appearance;；它会更新一次变量、对象或程序状态。
    uint16_t appearance;

    // 定义或声明函数 AdvCfg：参数在括号里，返回类型写在函数名前面。
    AdvCfg()
        // 定义或声明函数 device_name：参数在括号里，返回类型写在函数名前面。
        : device_name("EmotiPet")
        // 定义或声明函数 min_interval：参数在括号里，返回类型写在函数名前面。
        , min_interval(160)
        // 定义或声明函数 max_interval：参数在括号里，返回类型写在函数名前面。
        , max_interval(320)
        // 定义或声明函数 connectable：参数在括号里，返回类型写在函数名前面。
        , connectable(true)
        // 定义或声明函数 scan_response：参数在括号里，返回类型写在函数名前面。
        , scan_response(true)
        // 定义或声明函数 appearance：参数在括号里，返回类型写在函数名前面。
        , appearance(0)
    {
    }
};

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace Prop {

// 固定参数 READ = 0x0001：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t READ = 0x0001;
// 固定参数 WRITE = 0x0002：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t WRITE = 0x0002;
// 固定参数 WRITE_NR = 0x0004：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t WRITE_NR = 0x0004;
// 固定参数 NOTIFY = 0x0008：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t NOTIFY = 0x0008;
// 固定参数 INDICATE = 0x0010：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t INDICATE = 0x0010;
// 固定参数 READ_ENC = 0x0100：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t READ_ENC = 0x0100;
// 固定参数 WRITE_ENC = 0x0200：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t WRITE_ENC = 0x0200;
// 固定参数 READ_AUTHEN = 0x0400：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t READ_AUTHEN = 0x0400;
// 固定参数 WRITE_AUTHEN = 0x0800：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t WRITE_AUTHEN = 0x0800;

// 命名空间结束：离开当前模块作用域。
} // namespace Prop

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using StateCb = std::function<void(State state)>;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ConnectCb = std::function<void(const ConnInfo& info)>;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using DisconnectCb = std::function<void(const ConnInfo& info, int reason)>;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using CharReadCb = std::function<void(NimBLECharacteristic* chr, const ConnInfo& conn)>;
// 定义变量 using CharWriteCb 并初始化为 std::function<void(NimBLECharacteristic* chr, const ConnInfo& conn。
using CharWriteCb = std::function<void(NimBLECharacteristic* chr, const ConnInfo& conn,
                                       // 只读约束：这个值或对象在当前位置不能被修改。
                                       const uint8_t* data, size_t len)>;
// 定义变量：using CharSubscribeCb =
using CharSubscribeCb =
    // 继续上一行的表达式或参数列表：std::function<void(NimBLECharacteristic* chr, const ConnInfo& conn, uint16_t sub_value)>;。
    std::function<void(NimBLECharacteristic* chr, const ConnInfo& conn, uint16_t sub_value)>;

// struct CharCbs：把相关数据字段组合在一起，表示项目中的一个概念。
struct CharCbs
{
    // 执行语句：CharReadCb on_read;；它会更新一次变量、对象或程序状态。
    CharReadCb on_read;
    // 执行语句：CharWriteCb on_write;；它会更新一次变量、对象或程序状态。
    CharWriteCb on_write;
    // 执行语句：CharSubscribeCb on_subscribe;；它会更新一次变量、对象或程序状态。
    CharSubscribeCb on_subscribe;
};

/** @brief NimBLE 服务器封装（广播、连接、GATT 特征回调） */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class BleMgr
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static BleMgr& get_instance();

    // 赋值：把右侧 "EmotiPet") 的结果保存到左侧 bool init(const char* device_name。
    bool init(const char* device_name = "EmotiPet");
    // 赋值：把右侧 false) 的结果保存到左侧 void deinit(bool clear_all。
    void deinit(bool clear_all = false);

    // 定义或声明函数 is_init：参数在括号里，返回类型写在函数名前面。
    bool is_init() const
    {
        // 返回 init_ 给调用者，表示当前函数的执行结果。
        return init_;
    }

    // 函数调用：NimBLEService* create_service(const char* uuid);；执行该函数并处理它的返回值或副作用。
    NimBLEService* create_service(const char* uuid);

    // 函数调用：NimBLECharacteristic* create_characteristic(NimBLEService* service, const char* uuid,；执行该函数并处理它的返回值或副作用。
    NimBLECharacteristic* create_characteristic(NimBLEService* service, const char* uuid,
                                                // 继续上一行的表达式或参数列表：uint32_t properties, uint16_t max_len = 512);。
                                                uint32_t properties, uint16_t max_len = 512);

    // 函数调用：void set_char_cbs(NimBLECharacteristic* chr, const CharCbs& cbs);；执行该函数并处理它的返回值或副作用。
    void set_char_cbs(NimBLECharacteristic* chr, const CharCbs& cbs);

    // 函数调用：bool start_service(NimBLEService* service);；执行该函数并处理它的返回值或副作用。
    bool start_service(NimBLEService* service);

    // 函数调用：bool start_server();；执行该函数并处理它的返回值或副作用。
    bool start_server();

    // 赋值：把右侧 AdvCfg(), uint32_t duration_ms = 0) 的结果保存到左侧 bool start_advertising(const AdvCfg& config。
    bool start_advertising(const AdvCfg& config = AdvCfg(), uint32_t duration_ms = 0);

    // 函数调用：bool stop_advertising();；执行该函数并处理它的返回值或副作用。
    bool stop_advertising();

    // 赋值：把右侧 0xFFFF, uint8_t reason = 0x13) 的结果保存到左侧 bool disconnect(uint16_t conn_handle。
    bool disconnect(uint16_t conn_handle = 0xFFFF, uint8_t reason = 0x13);

    // 定义或声明函数 get_state：参数在括号里，返回类型写在函数名前面。
    State get_state() const
    {
        // 返回 state_ 给调用者，表示当前函数的执行结果。
        return state_;
    }

    // 定义或声明函数 is_connected：参数在括号里，返回类型写在函数名前面。
    bool is_connected() const
    {
        // 返回 state_ == State::CONNECTED 给调用者，表示当前函数的执行结果。
        return state_ == State::CONNECTED;
    }

    // 定义或声明函数 is_advertising：参数在括号里，返回类型写在函数名前面。
    bool is_advertising() const
    {
        // 返回 state_ == State::ADVERTISING 给调用者，表示当前函数的执行结果。
        return state_ == State::ADVERTISING;
    }

    // 函数调用：uint8_t get_conn_count() const;；执行该函数并处理它的返回值或副作用。
    uint8_t get_conn_count() const;
    // 函数调用：std::vector<ConnInfo> get_connected() const;；执行该函数并处理它的返回值或副作用。
    std::vector<ConnInfo> get_connected() const;

    // 函数调用：void set_state_cb(StateCb cb);；执行该函数并处理它的返回值或副作用。
    void set_state_cb(StateCb cb);
    // 函数调用：void set_connect_cb(ConnectCb cb);；执行该函数并处理它的返回值或副作用。
    void set_connect_cb(ConnectCb cb);
    // 函数调用：void set_disconnect_cb(DisconnectCb cb);；执行该函数并处理它的返回值或副作用。
    void set_disconnect_cb(DisconnectCb cb);

    // 赋值：把右侧 123456) 的结果保存到左侧 void set_security(SecMode mode, uint32_t passkey。
    void set_security(SecMode mode, uint32_t passkey = 123456);

    // 函数调用：bool set_mtu(uint16_t mtu);；执行该函数并处理它的返回值或副作用。
    bool set_mtu(uint16_t mtu);
    // 函数调用：uint16_t get_mtu() const;；执行该函数并处理它的返回值或副作用。
    uint16_t get_mtu() const;

    // 函数调用：bool set_tx_power(int8_t dbm);；执行该函数并处理它的返回值或副作用。
    bool set_tx_power(int8_t dbm);

    // 函数调用：std::string get_addr_str() const;；执行该函数并处理它的返回值或副作用。
    std::string get_addr_str() const;

    // 定义或声明函数 get_server：参数在括号里，返回类型写在函数名前面。
    NimBLEServer* get_server() const
    {
        // 返回 server_ 给调用者，表示当前函数的执行结果。
        return server_;
    }

    // 函数调用：void on_connect(const ConnInfo& info);；执行该函数并处理它的返回值或副作用。
    void on_connect(const ConnInfo& info);
    // 函数调用：void on_disconnect(const ConnInfo& info, int reason);；执行该函数并处理它的返回值或副作用。
    void on_disconnect(const ConnInfo& info, int reason);
    // 函数调用：void on_mtu_change(uint16_t mtu, uint16_t conn_handle);；执行该函数并处理它的返回值或副作用。
    void on_mtu_change(uint16_t mtu, uint16_t conn_handle);

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 BleMgr();，执行这个函数并处理它的返回值或状态变化。
    BleMgr();
    // 函数调用：~BleMgr();；执行该函数并处理它的返回值或副作用。
    ~BleMgr();
    // 调用 BleMgr(const BleMgr&) = delete;，执行这个函数并处理它的返回值或状态变化。
    BleMgr(const BleMgr&) = delete;
    // 赋值：把右侧 (const BleMgr&) = delete 的结果保存到左侧 BleMgr& operator。
    BleMgr& operator=(const BleMgr&) = delete;

    // 函数调用：void update_state(State new_state);；执行该函数并处理它的返回值或副作用。
    void update_state(State new_state);

    // 执行语句：mutable std::mutex mtx_;；它会更新一次变量、对象或程序状态。
    mutable std::mutex mtx_;
    // 执行语句：bool init_;；它会更新一次变量、对象或程序状态。
    bool init_;
    // 执行语句：State state_;；它会更新一次变量、对象或程序状态。
    State state_;
    // 执行语句：NimBLEServer* server_;；它会更新一次变量、对象或程序状态。
    NimBLEServer* server_;
    // 执行语句：std::vector<NimBLEService*> services_;；它会更新一次变量、对象或程序状态。
    std::vector<NimBLEService*> services_;
    // 执行语句：StateCb state_cb_;；它会更新一次变量、对象或程序状态。
    StateCb state_cb_;
    // 执行语句：ConnectCb connect_cb_;；它会更新一次变量、对象或程序状态。
    ConnectCb connect_cb_;
    // 执行语句：DisconnectCb disconnect_cb_;；它会更新一次变量、对象或程序状态。
    DisconnectCb disconnect_cb_;
    // 执行语句：SecMode sec_mode_;；它会更新一次变量、对象或程序状态。
    SecMode sec_mode_;
    // 执行语句：uint32_t passkey_;；它会更新一次变量、对象或程序状态。
    uint32_t passkey_;
    // 执行语句：AdvCfg adv_cfg_;；它会更新一次变量、对象或程序状态。
    AdvCfg adv_cfg_;
    // 执行语句：std::vector<ConnInfo> conns_;；它会更新一次变量、对象或程序状态。
    std::vector<ConnInfo> conns_;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::network::bluetooth
