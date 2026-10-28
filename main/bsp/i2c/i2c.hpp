// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/bsp/i2c/i2c.hpp
 * 作用：初始化和管理 I2C 总线，为触摸、传感器等设备提供通信。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：总线访问可能失败或超时，调用者不能假设设备一定在线。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <driver/gpio.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <driver/i2c_master.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "config/config.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::bsp::i2c {

// struct Config：把相关数据字段组合在一起，表示项目中的一个概念。
struct Config
{
    // 执行语句：i2c_port_t port;；它会更新一次变量、对象或程序状态。
    i2c_port_t port;
    // 执行语句：gpio_num_t sda_pin;；它会更新一次变量、对象或程序状态。
    gpio_num_t sda_pin;
    // 执行语句：gpio_num_t scl_pin;；它会更新一次变量、对象或程序状态。
    gpio_num_t scl_pin;
    // 执行语句：bool enable_internal_pullup;；它会更新一次变量、对象或程序状态。
    bool enable_internal_pullup;

    // 定义或声明函数 Config：参数在括号里，返回类型写在函数名前面。
    Config()
        // 定义或声明函数 port：参数在括号里，返回类型写在函数名前面。
        : port(I2C_NUM_1)
        // 定义或声明函数 sda_pin：参数在括号里，返回类型写在函数名前面。
        , sda_pin(config::I2C_SDA)
        // 定义或声明函数 scl_pin：参数在括号里，返回类型写在函数名前面。
        , scl_pin(config::I2C_SCL)
        // 定义或声明函数 enable_internal_pullup：参数在括号里，返回类型写在函数名前面。
        , enable_internal_pullup(true)
    {
    }
};

/** @brief I2C master bus（引用计数共享单 bus） */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class I2C
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 调用 I2C();，执行这个函数并处理它的返回值或状态变化。
    I2C();
    // 函数调用：~I2C();；执行该函数并处理它的返回值或副作用。
    ~I2C();

    // 赋值：把右侧 nullptr) 的结果保存到左侧 bool init(const Config* config。
    bool init(const Config* config = nullptr);
    // 赋值：把右侧 200) 的结果保存到左侧 int scan(uint32_t timeout_ms。
    int scan(uint32_t timeout_ms = 200);

    // 定义或声明函数 is_init：参数在括号里，返回类型写在函数名前面。
    bool is_init() const
    {
        // 返回 bus_handle_ != nullptr 给调用者，表示当前函数的执行结果。
        return bus_handle_ != nullptr;
    }

    // 定义或声明函数 get_bus_handle：参数在括号里，返回类型写在函数名前面。
    i2c_master_bus_handle_t get_bus_handle() const
    {
        // 返回 bus_handle_ 给调用者，表示当前函数的执行结果。
        return bus_handle_;
    }

    // 调用 deinit()：创建和销毁队列。
    void deinit();

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 执行语句：i2c_master_bus_handle_t bus_handle_;；它会更新一次变量、对象或程序状态。
    i2c_master_bus_handle_t bus_handle_;
    // 执行语句：Config config_;；它会更新一次变量、对象或程序状态。
    Config config_;
    // 执行语句：bool initialized_;；它会更新一次变量、对象或程序状态。
    bool initialized_;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::bsp::i2c
