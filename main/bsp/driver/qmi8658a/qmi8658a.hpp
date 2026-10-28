// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/bsp/driver/qmi8658a/qmi8658a.hpp
 * 作用：实现 QMI8658A 六轴传感器读取、姿态角计算和运动检测。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：自动旋转是否启用取决于系统配置，不能只凭驱动存在推断行为。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <functional>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <memory>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <driver/i2c_master.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/task/task.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::bsp::driver::qmi8658a {

// QMI8658A 在 I2C 上的低地址版本，SDO 接地时通常使用 0x6A。
constexpr uint8_t QMI8658A_ADDR_LOW = 0x6A;
// QMI8658A 在 I2C 上的高地址版本，SDO 接高电平时通常使用 0x6B。
constexpr uint8_t QMI8658A_ADDR_HIGH = 0x6B;

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum ReadOption : uint8_t
{
    // 执行这条代码：READ_SENSOR = 0x01,。它会更新变量、对象状态或调用链。
    READ_SENSOR = 0x01,
    // 继续上一行的表达式或参数列表：READ_ATTITUDE = 0x02,。
    READ_ATTITUDE = 0x02,
    // 继续上一行的表达式或参数列表：READ_ALL = READ_SENSOR | READ_ATTITUDE,。
    READ_ALL = READ_SENSOR | READ_ATTITUDE,
};

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum MotionStatus : uint8_t
{
    // 执行这条代码：ANY_MOTION = 0x20,。它会更新变量、对象状态或调用链。
    ANY_MOTION = 0x20,
    // 继续上一行的表达式或参数列表：NO_MOTION = 0x40,。
    NO_MOTION = 0x40,
    // 继续上一行的表达式或参数列表：SIGNIFICANT_MOTION = 0x80,。
    SIGNIFICANT_MOTION = 0x80,
};

// QMI8658A 的原始值、物理量和姿态角结果。
struct SensorData
{
    // 执行语句：int16_t acc_x_raw;；它会更新一次变量、对象或程序状态。
    int16_t acc_x_raw;
    // 执行语句：int16_t acc_y_raw;；它会更新一次变量、对象或程序状态。
    int16_t acc_y_raw;
    // 执行语句：int16_t acc_z_raw;；它会更新一次变量、对象或程序状态。
    int16_t acc_z_raw;
    // 执行语句：int16_t gyr_x_raw;；它会更新一次变量、对象或程序状态。
    int16_t gyr_x_raw;
    // 执行语句：int16_t gyr_y_raw;；它会更新一次变量、对象或程序状态。
    int16_t gyr_y_raw;
    // 执行语句：int16_t gyr_z_raw;；它会更新一次变量、对象或程序状态。
    int16_t gyr_z_raw;

    // 执行语句：float accel_x;；它会更新一次变量、对象或程序状态。
    float accel_x;
    // 执行语句：float accel_y;；它会更新一次变量、对象或程序状态。
    float accel_y;
    // 执行语句：float accel_z;；它会更新一次变量、对象或程序状态。
    float accel_z;
    // 执行语句：float gyro_x;；它会更新一次变量、对象或程序状态。
    float gyro_x;
    // 执行语句：float gyro_y;；它会更新一次变量、对象或程序状态。
    float gyro_y;
    // 执行语句：float gyro_z;；它会更新一次变量、对象或程序状态。
    float gyro_z;

    // 执行语句：float angle_x;；它会更新一次变量、对象或程序状态。
    float angle_x;
    // 执行语句：float angle_y;；它会更新一次变量、对象或程序状态。
    float angle_y;
    // 执行语句：float angle_z;；它会更新一次变量、对象或程序状态。
    float angle_z;
};

// 设备姿态角：roll、pitch、yaw。
struct AngleData
{
    // 执行语句：float roll;；它会更新一次变量、对象或程序状态。
    float roll;
    // 执行语句：float pitch;；它会更新一次变量、对象或程序状态。
    float pitch;
    // 执行语句：float yaw;；它会更新一次变量、对象或程序状态。
    float yaw;
};

/** QMI8658A 六轴 IMU + 可选后台运动检测任务 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class Qmi8658a
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 调用 Qmi8658a();，执行这个函数并处理它的返回值或状态变化。
    Qmi8658a();
    // 函数调用：~Qmi8658a();；执行该函数并处理它的返回值或副作用。
    ~Qmi8658a();

    // 赋值：把右侧 QMI8658A_ADDR_LOW) 的结果保存到左侧 bool init(i2c_master_bus_handle_t bus_handle, uint8_t i2c_addr。
    bool init(i2c_master_bus_handle_t bus_handle, uint8_t i2c_addr = QMI8658A_ADDR_LOW);
    // 调用 deinit()：创建和销毁队列。
    void deinit();

    // 赋值：把右侧 READ_SENSOR) 的结果保存到左侧 bool read(SensorData& data, uint8_t options。
    bool read(SensorData& data, uint8_t options = READ_SENSOR);
    // 函数调用：uint8_t get_motion_status();；执行该函数并处理它的返回值或副作用。
    uint8_t get_motion_status();
    // 调用 close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
    void close();

    // 定义或声明函数 is_init：参数在括号里，返回类型写在函数名前面。
    bool is_init() const
    {
        // 返回 initialized_ 给调用者，表示当前函数的执行结果。
        return initialized_;
    }

    // 赋值：把右侧 100) 的结果保存到左侧 bool start_data_collection(uint32_t interval_ms。
    bool start_data_collection(uint32_t interval_ms = 100);
    // 函数调用：bool stop_data_collection();；执行该函数并处理它的返回值或副作用。
    bool stop_data_collection();

    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using MotionStatusCallback = std::function<void(int motion_status)>;

    // 定义或声明函数 set_motion_status_callback：参数在括号里，返回类型写在函数名前面。
    void set_motion_status_callback(MotionStatusCallback callback)
    {
        // 赋值：把右侧 callback 的结果保存到左侧 motion_status_callback_。
        motion_status_callback_ = callback;
    }

    // 定义或声明函数 get_current_motion_status：参数在括号里，返回类型写在函数名前面。
    int get_current_motion_status() const
    {
        // 返回 current_motion_status_ 给调用者，表示当前函数的执行结果。
        return current_motion_status_;
    }

    // 函数调用：bool calibrate();；执行该函数并处理它的返回值或副作用。
    bool calibrate();
    // 函数调用：void reset_calibration();；执行该函数并处理它的返回值或副作用。
    void reset_calibration();
    // 定义或声明函数 is_calibrated：参数在括号里，返回类型写在函数名前面。
    bool is_calibrated() const
    {
        // 返回 calibrated_ 给调用者，表示当前函数的执行结果。
        return calibrated_;
    }

    // 函数调用：bool get_current_angle(AngleData& angle);；执行该函数并处理它的返回值或副作用。
    bool get_current_angle(AngleData& angle);
    // 函数调用：bool get_relative_angle(AngleData& angle);；执行该函数并处理它的返回值或副作用。
    bool get_relative_angle(AngleData& angle);

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
    enum Reg : uint8_t
    {
        // 执行这条代码：WHO_AM_I = 0x00,。它会更新变量、对象状态或调用链。
        WHO_AM_I = 0x00,
        // 继续上一行的表达式或参数列表：REVISION_ID = 0x01,。
        REVISION_ID = 0x01,
        // 继续上一行的表达式或参数列表：CTRL1 = 0x02,。
        CTRL1 = 0x02,
        // 继续上一行的表达式或参数列表：CTRL2 = 0x03,。
        CTRL2 = 0x03,
        // 继续上一行的表达式或参数列表：CTRL3 = 0x04,。
        CTRL3 = 0x04,
        // 继续上一行的表达式或参数列表：CTRL7 = 0x08,。
        CTRL7 = 0x08,
        // 继续上一行的表达式或参数列表：CTRL8 = 0x09,。
        CTRL8 = 0x09,
        // 继续上一行的表达式或参数列表：CTRL9 = 0x0A,。
        CTRL9 = 0x0A,
        // 继续上一行的表达式或参数列表：CATL1_L = 0x0B,。
        CATL1_L = 0x0B,
        // 继续上一行的表达式或参数列表：CATL1_H = 0x0C,。
        CATL1_H = 0x0C,
        // 继续上一行的表达式或参数列表：CATL2_L = 0x0D,。
        CATL2_L = 0x0D,
        // 继续上一行的表达式或参数列表：CATL2_H = 0x0E,。
        CATL2_H = 0x0E,
        // 继续上一行的表达式或参数列表：CATL3_L = 0x0F,。
        CATL3_L = 0x0F,
        // 继续上一行的表达式或参数列表：CATL3_H = 0x10,。
        CATL3_H = 0x10,
        // 继续上一行的表达式或参数列表：CATL4_L = 0x11,。
        CATL4_L = 0x11,
        // 继续上一行的表达式或参数列表：CATL4_H = 0x12,。
        CATL4_H = 0x12,
        // 继续上一行的表达式或参数列表：STATUS0 = 0x2E,。
        STATUS0 = 0x2E,
        // 继续上一行的表达式或参数列表：STATUS1 = 0x2F,。
        STATUS1 = 0x2F,
        // 继续上一行的表达式或参数列表：AX_L = 0x35,。
        AX_L = 0x35,
        // 继续上一行的表达式或参数列表：RESET = 0x60,。
        RESET = 0x60,
    };

    // 函数调用：bool write_register(uint8_t reg, uint8_t value);；执行该函数并处理它的返回值或副作用。
    bool write_register(uint8_t reg, uint8_t value);
    // 函数调用：bool read_register(uint8_t reg, uint8_t* buffer, size_t length);；执行该函数并处理它的返回值或副作用。
    bool read_register(uint8_t reg, uint8_t* buffer, size_t length);
    // 函数调用：void calculate_attitude(SensorData& data);；执行该函数并处理它的返回值或副作用。
    void calculate_attitude(SensorData& data);
    // 函数调用：void data_collection_task_function(void* param);；执行该函数并处理它的返回值或副作用。
    void data_collection_task_function(void* param);

    // 定义变量 i2c_master_bus_handle_t bus_handle_ 并初始化为 nullptr。
    i2c_master_bus_handle_t bus_handle_ = nullptr;
    // 定义变量 i2c_master_dev_handle_t dev_handle_ 并初始化为 nullptr。
    i2c_master_dev_handle_t dev_handle_ = nullptr;
    // 定义变量 bool initialized_ 并初始化为 false。
    bool initialized_ = false;
    // 定义变量 uint8_t i2c_addr_ 并初始化为 QMI8658A_ADDR_LOW。
    uint8_t i2c_addr_ = QMI8658A_ADDR_LOW;

    // 定义变量 bool calibrated_ 并初始化为 false。
    bool calibrated_ = false;
    // 声明并默认初始化对象：AngleData reference_angle_{}。
    AngleData reference_angle_{};

    // 加速度原始值换算成 m/s²；量程配置为 ±4G 时对应 8192 LSB/g。
    static constexpr float ACCEL_SCALE = 9.807f / 8192.0f;
    // 角速度原始值换算成弧度/秒；量程配置为 ±64 dps 时对应 64 LSB/dps。
    static constexpr float GYRO_SCALE = 0.0174533f / 64.0f;

    // 执行语句：std::unique_ptr<app::sys::task::Task> data_collection_task_;；它会更新一次变量、对象或程序状态。
    std::unique_ptr<app::sys::task::Task> data_collection_task_;
    // 定义变量 uint32_t collection_interval_ms_ 并初始化为 100。
    uint32_t collection_interval_ms_ = 100;
    // 定义变量 bool collection_running_ 并初始化为 false。
    bool collection_running_ = false;

    // 定义变量 MotionStatusCallback motion_status_callback_ 并初始化为 nullptr。
    MotionStatusCallback motion_status_callback_ = nullptr;
    // 定义变量 int current_motion_status_ 并初始化为 -1。
    int current_motion_status_ = -1;

    // 运动检测阈值：相邻两次加速度变化超过约 2.0 m/s² 视为运动。
    static constexpr float ACCEL_CHANGE_THRESHOLD = 2.0f;

    // 定义变量 float last_accel_x_ 并初始化为 0.0f。
    float last_accel_x_ = 0.0f;
    // 定义变量 float last_accel_y_ 并初始化为 0.0f。
    float last_accel_y_ = 0.0f;
    // 定义变量 float last_accel_z_ 并初始化为 0.0f。
    float last_accel_z_ = 0.0f;
    // 定义变量 bool has_last_accel_ 并初始化为 false。
    bool has_last_accel_ = false;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::bsp::driver::qmi8658a
