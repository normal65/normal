/**
 * [初学者导读] 文件：main/bsp/driver/qmi8658a/qmi8658a.cc
 * 作用：实现 QMI8658A 六轴传感器读取、姿态角计算和运动检测。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：自动旋转是否启用取决于系统配置，不能只凭驱动存在推断行为。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "qmi8658a.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cmath>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_log.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/task/task.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "QMI8658A";

// 固定参数 M_PI_F = 3.14159265358979323846f：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr float M_PI_F = 3.14159265358979323846f;
// 固定参数 RAD2DEG = 180.0f / M_PI_F：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr float RAD2DEG = 180.0f / M_PI_F;

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::bsp::driver::qmi8658a {
// 调用 Qmi8658a::Qmi8658a() = default;，执行这个函数并处理它的返回值或状态变化。
Qmi8658a::Qmi8658a() = default;

// 定义或声明函数 Qmi8658a::~Qmi8658a：参数在括号里，返回类型写在函数名前面。
Qmi8658a::~Qmi8658a()
{
    // 调用 stop_data_collection();，执行这个函数并处理它的返回值或状态变化。
    stop_data_collection();
    // 调用 deinit()：创建和销毁队列。
    deinit();
}

// 定义或声明函数 Qmi8658a::init：参数在括号里，返回类型写在函数名前面。
bool Qmi8658a::init(i2c_master_bus_handle_t bus_handle, uint8_t i2c_addr)
{
    // 判断条件：bus_handle == nullptr 是否成立。
    if (bus_handle == nullptr)
    {
        // 调用 ESP_LOGE(TAG, "I2C 总线句柄为空");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "I2C 总线句柄为空");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 bus_handle 的结果保存到左侧 bus_handle_。
    bus_handle_ = bus_handle;
    // 赋值：把右侧 i2c_addr 的结果保存到左侧 i2c_addr_。
    i2c_addr_ = i2c_addr;

    // 添加 I2C 设备
    i2c_device_config_t dev_cfg = {};
    // 赋值：把右侧 I2C_ADDR_BIT_LEN_7 的结果保存到左侧 dev_cfg.dev_addr_length。
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    // 赋值：把右侧 i2c_addr_ 的结果保存到左侧 dev_cfg.device_address。
    dev_cfg.device_address = i2c_addr_;
    // 赋值：把右侧 400000 的结果保存到左侧 dev_cfg.scl_speed_hz。
    dev_cfg.scl_speed_hz = 400000;

    // 定义变量 esp_err_t ret 并初始化为 i2c_master_bus_add_device(bus_handle_, &dev_cfg, &dev_handle_)。
    esp_err_t ret = i2c_master_bus_add_device(bus_handle_, &dev_cfg, &dev_handle_);
    // 判断条件：ret != ESP_OK 是否成立。
    if (ret != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "添加 I2C 设备失败: %s", esp_err_to_name(ret));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "添加 I2C 设备失败: %s", esp_err_to_name(ret));
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 检查设备 ID，带重试机制
    uint8_t who_am_i = 0;
    // 定义变量 int retry_count 并初始化为 0。
    int retry_count = 0;

    // 调用 read_register(Reg::WHO_AM_I, &who_am_i, 1);，执行这个函数并处理它的返回值或状态变化。
    read_register(Reg::WHO_AM_I, &who_am_i, 1);
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (who_am_i != 0x05 && retry_count < 3)
    {
        // 调用 app::sys::task::TaskMgr::delay_ms(100)：调用 vTaskDelay()，这是让出 CPU 的阻塞延时。一千毫秒延时期间，任务不占 CPU，调度器可以运行别人。。
        app::sys::task::TaskMgr::delay_ms(100); // 延时 100ms
        // 调用 read_register(Reg::WHO_AM_I, &who_am_i, 1);，执行这个函数并处理它的返回值或状态变化。
        read_register(Reg::WHO_AM_I, &who_am_i, 1);
        // 执行语句：retry_count++;；它会更新一次变量、对象或程序状态。
        retry_count++;
    }

    // 判断条件：who_am_i != 0x05 是否成立。
    if (who_am_i != 0x05)
    {
        // 调用 ESP_LOGE(TAG, "设备 ID 错误: 0x%02X (期望 0x05)", who_am_i);，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "设备 ID 错误: 0x%02X (期望 0x05)", who_am_i);
        // 调用 deinit()：创建和销毁队列。
        deinit();
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 调用 ESP_LOGD(TAG, "QMI8658A OK!");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "QMI8658A OK!");

    // 软复位
    write_register(Reg::RESET, 0xB0);
    // 调用 app::sys::task::TaskMgr::delay_ms(10)：调用 vTaskDelay()，这是让出 CPU 的阻塞延时。一千毫秒延时期间，任务不占 CPU，调度器可以运行别人。。
    app::sys::task::TaskMgr::delay_ms(10);

    // 配置运动检测
    // 第一步：配置运动阈值
    write_register(Reg::CATL1_L, 1);    // AnyMotionXThr (0~32)
    // 调用 write_register(Reg::CATL1_H, 1);，执行这个函数并处理它的返回值或状态变化。
    write_register(Reg::CATL1_H, 1);    // AnyMotionYThr (0~32)
    // 调用 write_register(Reg::CATL2_L, 1);，执行这个函数并处理它的返回值或状态变化。
    write_register(Reg::CATL2_L, 1);    // AnyMotionZThr (0~32)
    // 调用 write_register(Reg::CATL2_H, 1);，执行这个函数并处理它的返回值或状态变化。
    write_register(Reg::CATL2_H, 1);    // NoMotionXThr (0~32)
    // 调用 write_register(Reg::CATL3_L, 1);，执行这个函数并处理它的返回值或状态变化。
    write_register(Reg::CATL3_L, 1);    // NoMotionYThr (0~32)
    // 调用 write_register(Reg::CATL3_H, 1);，执行这个函数并处理它的返回值或状态变化。
    write_register(Reg::CATL3_H, 1);    // NoMotionZThr (0~32)
    // 调用 write_register(Reg::CATL4_L, 0x77);，执行这个函数并处理它的返回值或状态变化。
    write_register(Reg::CATL4_L, 0x77); // MOTION_MODE_CTRL (0111 0111)
    // 调用 write_register(Reg::CATL4_H, 0x01);，执行这个函数并处理它的返回值或状态变化。
    write_register(Reg::CATL4_H, 0x01); // 第1条命令
    // 调用 write_register(Reg::CTRL9, 0x0E);，执行这个函数并处理它的返回值或状态变化。
    write_register(Reg::CTRL9, 0x0E);   // 配置运动检测命令

    // 第二步：配置时间窗口
    write_register(Reg::CATL1_L, 1);    // AnyMotionWindow
    // 调用 write_register(Reg::CATL1_H, 1);，执行这个函数并处理它的返回值或状态变化。
    write_register(Reg::CATL1_H, 1);    // NoMotionWindow
    // 调用 write_register(Reg::CATL2_L, 0xE8);，执行这个函数并处理它的返回值或状态变化。
    write_register(Reg::CATL2_L, 0xE8); // SigMotionWaitWindow[7:0]
    // 调用 write_register(Reg::CATL2_H, 0x03);，执行这个函数并处理它的返回值或状态变化。
    write_register(Reg::CATL2_H, 0x03); // SigMotionWaitWindow[15:8]
    // 调用 write_register(Reg::CATL3_L, 0xE8);，执行这个函数并处理它的返回值或状态变化。
    write_register(Reg::CATL3_L, 0xE8); // SigMotionConfirmWindow[7:0]
    // 调用 write_register(Reg::CATL3_H, 0x03);，执行这个函数并处理它的返回值或状态变化。
    write_register(Reg::CATL3_H, 0x03); // SigMotionConfirmWindow[15:8]
    // 调用 write_register(Reg::CATL4_H, 0x02);，执行这个函数并处理它的返回值或状态变化。
    write_register(Reg::CATL4_H, 0x02); // 第2条命令
    // 调用 write_register(Reg::CTRL9, 0x0E);，执行这个函数并处理它的返回值或状态变化。
    write_register(Reg::CTRL9, 0x0E);   // 配置运动检测命令

    // 配置传感器
    write_register(Reg::CTRL1, 0x40); // 地址自动递增
    // 调用 write_register(Reg::CTRL7, 0x03);，执行这个函数并处理它的返回值或状态变化。
    write_register(Reg::CTRL7, 0x03); // 启用加速度计和陀螺仪
    // 调用 write_register(Reg::CTRL2, 0x95);，执行这个函数并处理它的返回值或状态变化。
    write_register(Reg::CTRL2, 0x95); // ACC: 4G, 250Hz
    // 调用 write_register(Reg::CTRL3, 0xD5);，执行这个函数并处理它的返回值或状态变化。
    write_register(Reg::CTRL3, 0xD5); // GYR: 512DPS, 250Hz
    // 调用 write_register(Reg::CTRL8, 0x0E);，执行这个函数并处理它的返回值或状态变化。
    write_register(Reg::CTRL8, 0x0E); // 启用 Any-Motion, No-Motion, Significant-Motion

    // 赋值：把右侧 true 的结果保存到左侧 initialized_。
    initialized_ = true;
    // 调用 ESP_LOGD(TAG, "初始化成功 (地址: 0x%02X, 运动检测已启用)", i2c_addr_);，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "初始化成功 (地址: 0x%02X, 运动检测已启用)", i2c_addr_);

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 Qmi8658a::deinit：参数在括号里，返回类型写在函数名前面。
void Qmi8658a::deinit()
{
    // 判断条件：dev_handle_ != nullptr 是否成立。
    if (dev_handle_ != nullptr)
    {
        // 调用 i2c_master_bus_rm_device(dev_handle_);，执行这个函数并处理它的返回值或状态变化。
        i2c_master_bus_rm_device(dev_handle_);
        // 赋值：把右侧 nullptr 的结果保存到左侧 dev_handle_。
        dev_handle_ = nullptr;
    }
    // 赋值：把右侧 nullptr 的结果保存到左侧 bus_handle_。
    bus_handle_ = nullptr;
    // 赋值：把右侧 false 的结果保存到左侧 initialized_。
    initialized_ = false;
    // 赋值：把右侧 false 的结果保存到左侧 calibrated_。
    calibrated_ = false;
}

// 定义或声明函数 Qmi8658a::read：参数在括号里，返回类型写在函数名前面。
bool Qmi8658a::read(SensorData& data, uint8_t options)
{
    // 判断条件：!initialized_ 是否成立。
    if (!initialized_)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 读取传感器数据
    if (options & READ_SENSOR)
    {
        // 读取 STATUS0 寄存器
        uint8_t status = 0;
        // 定义变量 bool data_ready 并初始化为 false。
        bool data_ready = false;

        // 调用 read_register(Reg::STATUS0, &status, 1);，执行这个函数并处理它的返回值或状态变化。
        read_register(Reg::STATUS0, &status, 1);

        // 检查数据是否就绪（bit[1:0]: 0x01=加速度, 0x02=陀螺仪, 0x03=两者）
        if (status & 0x03)
        {
            // 赋值：把右侧 true 的结果保存到左侧 data_ready。
            data_ready = true;
        }

        // 判断条件：!data_ready 是否成立。
        if (!data_ready)
        {
            // 数据未就绪，静默返回
            return false;
        }

        // 读取 12 字节数据（AX_L 到 GZ_H）
        uint8_t buffer[12];
        // 判断条件：!read_register(Reg::AX_L, buffer, 12) 是否成立。
        if (!read_register(Reg::AX_L, buffer, 12))
        {
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }

        // 解析原始数据（小端序）
        data.acc_x_raw = static_cast<int16_t>((buffer[1] << 8) | buffer[0]);
        // 赋值：把右侧 static_cast<int16_t>((buffer[3] << 8) | buffer[2]) 的结果保存到左侧 data.acc_y_raw。
        data.acc_y_raw = static_cast<int16_t>((buffer[3] << 8) | buffer[2]);
        // 赋值：把右侧 static_cast<int16_t>((buffer[5] << 8) | buffer[4]) 的结果保存到左侧 data.acc_z_raw。
        data.acc_z_raw = static_cast<int16_t>((buffer[5] << 8) | buffer[4]);
        // 赋值：把右侧 static_cast<int16_t>((buffer[7] << 8) | buffer[6]) 的结果保存到左侧 data.gyr_x_raw。
        data.gyr_x_raw = static_cast<int16_t>((buffer[7] << 8) | buffer[6]);
        // 赋值：把右侧 static_cast<int16_t>((buffer[9] << 8) | buffer[8]) 的结果保存到左侧 data.gyr_y_raw。
        data.gyr_y_raw = static_cast<int16_t>((buffer[9] << 8) | buffer[8]);
        // 赋值：把右侧 static_cast<int16_t>((buffer[11] << 8) | buffer[10]) 的结果保存到左侧 data.gyr_z_raw。
        data.gyr_z_raw = static_cast<int16_t>((buffer[11] << 8) | buffer[10]);

        // 转换为物理单位
        data.accel_x = static_cast<float>(data.acc_x_raw) * ACCEL_SCALE;
        // 赋值：把右侧 static_cast<float>(data.acc_y_raw) * ACCEL_SCALE 的结果保存到左侧 data.accel_y。
        data.accel_y = static_cast<float>(data.acc_y_raw) * ACCEL_SCALE;
        // 赋值：把右侧 static_cast<float>(data.acc_z_raw) * ACCEL_SCALE 的结果保存到左侧 data.accel_z。
        data.accel_z = static_cast<float>(data.acc_z_raw) * ACCEL_SCALE;

        // 赋值：把右侧 static_cast<float>(data.gyr_x_raw) * GYRO_SCALE 的结果保存到左侧 data.gyro_x。
        data.gyro_x = static_cast<float>(data.gyr_x_raw) * GYRO_SCALE;
        // 赋值：把右侧 static_cast<float>(data.gyr_y_raw) * GYRO_SCALE 的结果保存到左侧 data.gyro_y。
        data.gyro_y = static_cast<float>(data.gyr_y_raw) * GYRO_SCALE;
        // 赋值：把右侧 static_cast<float>(data.gyr_z_raw) * GYRO_SCALE 的结果保存到左侧 data.gyro_z。
        data.gyro_z = static_cast<float>(data.gyr_z_raw) * GYRO_SCALE;
    }

    // 计算姿态角
    if (options & READ_ATTITUDE)
    {
        // 调用 calculate_attitude(data);，执行这个函数并处理它的返回值或状态变化。
        calculate_attitude(data);
    }

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 Qmi8658a::calculate_attitude：参数在括号里，返回类型写在函数名前面。
void Qmi8658a::calculate_attitude(SensorData& data)
{
    // 使用加速度计计算倾角
    float acc_x = static_cast<float>(data.acc_x_raw);
    // 定义变量 float acc_y 并初始化为 static_cast<float>(data.acc_y_raw)。
    float acc_y = static_cast<float>(data.acc_y_raw);
    // 定义变量 float acc_z 并初始化为 static_cast<float>(data.acc_z_raw)。
    float acc_z = static_cast<float>(data.acc_z_raw);

    // AngleX (Roll)
    float temp = acc_x / sqrtf(acc_y * acc_y + acc_z * acc_z);
    // 赋值：把右侧 atanf(temp) * RAD2DEG 的结果保存到左侧 data.angle_x。
    data.angle_x = atanf(temp) * RAD2DEG;

    // AngleY (Pitch)
    temp = acc_y / sqrtf(acc_x * acc_x + acc_z * acc_z);
    // 赋值：把右侧 atanf(temp) * RAD2DEG 的结果保存到左侧 data.angle_y。
    data.angle_y = atanf(temp) * RAD2DEG;

    // AngleZ (Yaw)
    temp = sqrtf(acc_x * acc_x + acc_y * acc_y) / acc_z;
    // 赋值：把右侧 atanf(temp) * RAD2DEG 的结果保存到左侧 data.angle_z。
    data.angle_z = atanf(temp) * RAD2DEG;
}

// 定义或声明函数 Qmi8658a::get_motion_status：参数在括号里，返回类型写在函数名前面。
uint8_t Qmi8658a::get_motion_status()
{
    // 判断条件：!initialized_ 是否成立。
    if (!initialized_)
    {
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    }

    // 定义变量 uint8_t status 并初始化为 0。
    uint8_t status = 0;
    // 调用 read_register(Reg::STATUS1, &status, 1);，执行这个函数并处理它的返回值或状态变化。
    read_register(Reg::STATUS1, &status, 1);
    // 返回 status 给调用者，表示当前函数的执行结果。
    return status;
}

// 函数定义/声明：调用 Qmi8658a::close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
void Qmi8658a::close()
{
    // 判断条件：!initialized_ 是否成立。
    if (!initialized_)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 关闭芯片运行
    write_register(Reg::CTRL1, 0x01);
}

// 定义或声明函数 Qmi8658a::write_register：参数在括号里，返回类型写在函数名前面。
bool Qmi8658a::write_register(uint8_t reg, uint8_t value)
{
    // 判断条件：dev_handle_ == nullptr 是否成立。
    if (dev_handle_ == nullptr)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 {reg, value} 的结果保存到左侧 uint8_t data[2]。
    uint8_t data[2] = {reg, value};
    // 使用 1000ms 超时
    esp_err_t ret = i2c_master_transmit(dev_handle_, data, 2, 1000);

    // 返回 ret == ESP_OK 给调用者，表示当前函数的执行结果。
    return ret == ESP_OK;
}

// 定义或声明函数 Qmi8658a::read_register：参数在括号里，返回类型写在函数名前面。
bool Qmi8658a::read_register(uint8_t reg, uint8_t* buffer, size_t length)
{
    // 判断条件：dev_handle_ == nullptr || buffer == nullptr 是否成立。
    if (dev_handle_ == nullptr || buffer == nullptr)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 使用 1000ms 超时
    esp_err_t ret = i2c_master_transmit_receive(dev_handle_, &reg, 1, buffer, length, 1000);

    // 返回 ret == ESP_OK 给调用者，表示当前函数的执行结果。
    return ret == ESP_OK;
}

// 定义或声明函数 Qmi8658a::calibrate：参数在括号里，返回类型写在函数名前面。
bool Qmi8658a::calibrate()
{
    // 判断条件：!initialized_ 是否成立。
    if (!initialized_)
    {
        // 调用 ESP_LOGE(TAG, "传感器未初始化，无法标定");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "传感器未初始化，无法标定");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 执行语句：SensorData data;；它会更新一次变量、对象或程序状态。
    SensorData data;
    // 判断条件：!read(data, READ_ALL) 是否成立。
    if (!read(data, READ_ALL))
    {
        // 调用 ESP_LOGE(TAG, "读取传感器数据失败，无法标定");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "读取传感器数据失败，无法标定");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 保存当前角度作为参考位置
    reference_angle_.roll = data.angle_x;
    // 赋值：把右侧 data.angle_y 的结果保存到左侧 reference_angle_.pitch。
    reference_angle_.pitch = data.angle_y;
    // 赋值：把右侧 data.angle_z 的结果保存到左侧 reference_angle_.yaw。
    reference_angle_.yaw = data.angle_z;
    // 赋值：把右侧 true 的结果保存到左侧 calibrated_。
    calibrated_ = true;

    // 调用 ESP_LOGD(TAG, "标定完成 - 参考角度: Roll=%.2f° Pitch=%.2f° Yaw=%.2f°", reference_angle_.roll,，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "标定完成 - 参考角度: Roll=%.2f° Pitch=%.2f° Yaw=%.2f°", reference_angle_.roll,
             // 继续上一行的表达式或参数列表：reference_angle_.pitch, reference_angle_.yaw);。
             reference_angle_.pitch, reference_angle_.yaw);

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 Qmi8658a::reset_calibration：参数在括号里，返回类型写在函数名前面。
void Qmi8658a::reset_calibration()
{
    // 赋值：把右侧 false 的结果保存到左侧 calibrated_。
    calibrated_ = false;
    // 赋值：把右侧 0.0f 的结果保存到左侧 reference_angle_.roll。
    reference_angle_.roll = 0.0f;
    // 赋值：把右侧 0.0f 的结果保存到左侧 reference_angle_.pitch。
    reference_angle_.pitch = 0.0f;
    // 赋值：把右侧 0.0f 的结果保存到左侧 reference_angle_.yaw。
    reference_angle_.yaw = 0.0f;
    // 调用 ESP_LOGD(TAG, "标定已清除");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "标定已清除");
}

// 定义或声明函数 Qmi8658a::get_current_angle：参数在括号里，返回类型写在函数名前面。
bool Qmi8658a::get_current_angle(AngleData& angle)
{
    // 判断条件：!initialized_ 是否成立。
    if (!initialized_)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 执行语句：SensorData data;；它会更新一次变量、对象或程序状态。
    SensorData data;
    // 判断条件：!read(data, READ_ALL) 是否成立。
    if (!read(data, READ_ALL))
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 data.angle_x 的结果保存到左侧 angle.roll。
    angle.roll = data.angle_x;
    // 赋值：把右侧 data.angle_y 的结果保存到左侧 angle.pitch。
    angle.pitch = data.angle_y;
    // 赋值：把右侧 data.angle_z 的结果保存到左侧 angle.yaw。
    angle.yaw = data.angle_z;

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 Qmi8658a::get_relative_angle：参数在括号里，返回类型写在函数名前面。
bool Qmi8658a::get_relative_angle(AngleData& angle)
{
    // 判断条件：!initialized_ 是否成立。
    if (!initialized_)
    {
        // 调用 ESP_LOGE(TAG, "传感器未初始化");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "传感器未初始化");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 判断条件：!calibrated_ 是否成立。
    if (!calibrated_)
    {
        // 调用 ESP_LOGW(TAG, "未标定，无法获取相对角度。请先调用 calibrate()");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGW(TAG, "未标定，无法获取相对角度。请先调用 calibrate()");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 执行语句：SensorData data;；它会更新一次变量、对象或程序状态。
    SensorData data;
    // 判断条件：!read(data, READ_ALL) 是否成立。
    if (!read(data, READ_ALL))
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 计算相对于参考位置的角度
    angle.roll = data.angle_x - reference_angle_.roll;
    // 赋值：把右侧 data.angle_y - reference_angle_.pitch 的结果保存到左侧 angle.pitch。
    angle.pitch = data.angle_y - reference_angle_.pitch;
    // 赋值：把右侧 data.angle_z - reference_angle_.yaw 的结果保存到左侧 angle.yaw。
    angle.yaw = data.angle_z - reference_angle_.yaw;

    // 将角度归一化到 [-180, 180] 范围
    auto normalize_angle = [](float& angle) {
        // 循环执行：条件为真时重复运行，条件变化后退出。
        while (angle > 180.0f)
            // 赋值：把右侧 360.0f 的结果保存到左侧 angle -。
            angle -= 360.0f;
        // 循环执行：条件为真时重复运行，条件变化后退出。
        while (angle < -180.0f)
            // 赋值：把右侧 360.0f 的结果保存到左侧 angle +。
            angle += 360.0f;
    };

    // 调用 normalize_angle(angle.roll);，执行这个函数并处理它的返回值或状态变化。
    normalize_angle(angle.roll);
    // 调用 normalize_angle(angle.pitch);，执行这个函数并处理它的返回值或状态变化。
    normalize_angle(angle.pitch);
    // 调用 normalize_angle(angle.yaw);，执行这个函数并处理它的返回值或状态变化。
    normalize_angle(angle.yaw);

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 Qmi8658a::start_data_collection：参数在括号里，返回类型写在函数名前面。
bool Qmi8658a::start_data_collection(uint32_t interval_ms)
{
    // 判断条件：!initialized_ 是否成立。
    if (!initialized_)
    {
        // 调用 ESP_LOGE(TAG, "传感器未初始化，无法启动数据采集");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "传感器未初始化，无法启动数据采集");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 如果已经在运行，先停止
    if (collection_running_)
    {
        // 调用 stop_data_collection();，执行这个函数并处理它的返回值或状态变化。
        stop_data_collection();
    }

    // 赋值：把右侧 interval_ms 的结果保存到左侧 collection_interval_ms_。
    collection_interval_ms_ = interval_ms;

    // 创建数据采集任务
    app::sys::task::Cfg task_config;
    // 赋值：把右侧 "qmi8658a_collect" 的结果保存到左侧 task_config.name。
    task_config.name = "qmi8658a_collect";
    // 赋值：把右侧 2 * 1024 的结果保存到左侧 task_config.stack_size。
    task_config.stack_size = 2 * 1024;
    // 赋值：把右侧 app::sys::task::Priority::NORMAL 的结果保存到左侧 task_config.priority。
    task_config.priority = app::sys::task::Priority::NORMAL;
    // 赋值：把右侧 -1 的结果保存到左侧 task_config.core_id。
    task_config.core_id = -1;
    // 赋值：把右侧 0 的结果保存到左侧 task_config.delay_ms。
    task_config.delay_ms = 0;

    // 函数调用：data_collection_task_ = std::unique_ptr<app::sys::task::Task>(new app::sys::task::Task(；执行该函数并处理它的返回值或副作用。
    data_collection_task_ = std::unique_ptr<app::sys::task::Task>(new app::sys::task::Task(
        // 继续上一行的表达式或参数列表：[this](void* param) { this->data_collection_task_function(param); }, task_config, this));。
        [this](void* param) { this->data_collection_task_function(param); }, task_config, this));

    // 判断条件：!data_collection_task_ 是否成立。
    if (!data_collection_task_)
    {
        // 调用 ESP_LOGE(TAG, "创建数据采集任务对象失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "创建数据采集任务对象失败");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 true 的结果保存到左侧 collection_running_。
    collection_running_ = true;
    // 判断条件：!data_collection_task_->start() 是否成立。
    if (!data_collection_task_->start())
    {
        // 调用 ESP_LOGE(TAG, "启动数据采集任务失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "启动数据采集任务失败");
        // 赋值：把右侧 false 的结果保存到左侧 collection_running_。
        collection_running_ = false;
        // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
        data_collection_task_.reset();
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 调用 ESP_LOGD(TAG, "数据采集任务已启动，采集间隔: %u ms", static_cast<unsigned>(interval_ms));，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "数据采集任务已启动，采集间隔: %u ms", static_cast<unsigned>(interval_ms));
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 Qmi8658a::stop_data_collection：参数在括号里，返回类型写在函数名前面。
bool Qmi8658a::stop_data_collection()
{
    // 判断条件：!collection_running_ 是否成立。
    if (!collection_running_)
    {
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 停止任务
    collection_running_ = false;

    // 判断条件：data_collection_task_ != nullptr 是否成立。
    if (data_collection_task_ != nullptr)
    {
        // 调用 destroy()：直接删除，不是优雅停止。
        data_collection_task_->destroy();
        // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
        data_collection_task_.reset();
    }

    // 调用 ESP_LOGD(TAG, "数据采集任务已停止");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "数据采集任务已停止");
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 Qmi8658a::data_collection_task_function：参数在括号里，返回类型写在函数名前面。
void Qmi8658a::data_collection_task_function(void* param)
{
    // 执行语句：(void)param;；它会更新一次变量、对象或程序状态。
    (void)param;
    // 调用 ESP_LOGD(TAG, "数据采集任务开始运行");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "数据采集任务开始运行");

    // 定义变量 int last_motion_status 并初始化为 -1。
    int last_motion_status = -1;

    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (collection_running_)
    {
        // 执行语句：SensorData data;；它会更新一次变量、对象或程序状态。
        SensorData data;
        // 判断条件：read(data, READ_SENSOR) 是否成立。
        if (read(data, READ_SENSOR))
        {
            // 定义变量 bool has_motion 并初始化为 false。
            bool has_motion = false;
            // 定义变量 int current_status 并初始化为 -1。
            int current_status = -1;

            // 判断条件：has_last_accel_ 是否成立。
            if (has_last_accel_)
            {
                // 只读约束：这个值或对象在当前位置不能被修改。
                const float delta_x = fabsf(data.accel_x - last_accel_x_);
                // 只读约束：这个值或对象在当前位置不能被修改。
                const float delta_y = fabsf(data.accel_y - last_accel_y_);
                // 只读约束：这个值或对象在当前位置不能被修改。
                const float delta_z = fabsf(data.accel_z - last_accel_z_);
                // 只读约束：这个值或对象在当前位置不能被修改。
                const float total_change =
                    // 继续上一行的表达式或参数列表：sqrtf(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);。
                    sqrtf(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);
                // 判断条件：total_change > ACCEL_CHANGE_THRESHOLD 是否成立。
                if (total_change > ACCEL_CHANGE_THRESHOLD)
                    // 赋值：把右侧 true 的结果保存到左侧 has_motion。
                    has_motion = true;
            }
            // 备用分支：前面的条件不成立时执行这里。
            else
            {
                // 赋值：把右侧 true 的结果保存到左侧 has_last_accel_。
                has_last_accel_ = true;
            }

            // 赋值：把右侧 data.accel_x 的结果保存到左侧 last_accel_x_。
            last_accel_x_ = data.accel_x;
            // 赋值：把右侧 data.accel_y 的结果保存到左侧 last_accel_y_。
            last_accel_y_ = data.accel_y;
            // 赋值：把右侧 data.accel_z 的结果保存到左侧 last_accel_z_。
            last_accel_z_ = data.accel_z;

            // 赋值：把右侧 has_motion ? 1 : 0 的结果保存到左侧 current_status。
            current_status = has_motion ? 1 : 0;
            // 赋值：把右侧 current_status 的结果保存到左侧 current_motion_status_。
            current_motion_status_ = current_status;

            // 判断条件：last_motion_status != current_status || last_motion_status == -1 是否成立。
            if (last_motion_status != current_status || last_motion_status == -1)
            {
                // 赋值：把右侧 current_status 的结果保存到左侧 last_motion_status。
                last_motion_status = current_status;
                // 判断条件：motion_status_callback_ != nullptr 是否成立。
                if (motion_status_callback_ != nullptr)
                    // 调用 motion_status_callback_(current_status);，执行这个函数并处理它的返回值或状态变化。
                    motion_status_callback_(current_status);
            }
        }

        // 调用 app::sys::task::TaskMgr::delay_ms(collection_interval_ms_)：调用 vTaskDelay()，这是让出 CPU 的阻塞延时。一千毫秒延时期间，任务不占 CPU，调度器可以运行别人。。
        app::sys::task::TaskMgr::delay_ms(collection_interval_ms_);
    }

    // 调用 ESP_LOGD(TAG, "数据采集任务结束");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "数据采集任务结束");
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::bsp::driver::qmi8658a
