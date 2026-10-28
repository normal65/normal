// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/bsp/driver/audio/es8311/es8311_codec.hpp
 * 作用：控制 ES8311 音频 Codec 的初始化、采样格式、音量和播放通路。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：音频使用 DMA 和环形缓冲，启动停止顺序会影响杂音和尾音。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <driver/i2c_master.h>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::bsp::driver::audio::es8311 {

// 固定参数 kI2cAddr = 0x18：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kI2cAddr = 0x18;

// struct Config：把相关数据字段组合在一起，表示项目中的一个概念。
struct Config
{
    // 定义变量 uint32_t sample_rate 并初始化为 16000。
    uint32_t sample_rate = 16000;
    // 定义变量 uint8_t channels 并初始化为 1。
    uint8_t channels = 1;
    // 定义变量 uint8_t bits_per_sample 并初始化为 16。
    uint8_t bits_per_sample = 16;
    // 定义变量 bool use_mclk 并初始化为 true。
    bool use_mclk = true;
    // 定义变量 i2c_port_t i2c_port 并初始化为 I2C_NUM_1。
    i2c_port_t i2c_port = I2C_NUM_1;
    // 定义变量 uint8_t i2c_addr 并初始化为 kI2cAddr。
    uint8_t i2c_addr = kI2cAddr;
    // 定义变量 bool pa_inverted 并初始化为 false。
    bool pa_inverted = false;
    // 定义变量 uint8_t default_output_volume 并初始化为 60。
    uint8_t default_output_volume = 60;
    // 定义变量 uint8_t default_input_gain_db 并初始化为 30。
    uint8_t default_input_gain_db = 30;
};

/** ES8311 I2S + I2C（esp_codec_dev）；enable_input/output 延迟打开通路 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class Es8311Codec
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 调用 Es8311Codec();，执行这个函数并处理它的返回值或状态变化。
    Es8311Codec();
    // 函数调用：~Es8311Codec();；执行该函数并处理它的返回值或副作用。
    ~Es8311Codec();

    // 赋值：把右侧 nullptr) 的结果保存到左侧 bool init(i2c_master_bus_handle_t bus_handle, const Config* config。
    bool init(i2c_master_bus_handle_t bus_handle, const Config* config = nullptr);
    // 调用 deinit()：创建和销毁队列。
    void deinit();

    // 函数调用：void enable_input(bool enable);；执行该函数并处理它的返回值或副作用。
    void enable_input(bool enable);
    // 函数调用：void enable_output(bool enable);；执行该函数并处理它的返回值或副作用。
    void enable_output(bool enable);

    // 函数调用：void set_output_volume(uint8_t percent);；执行该函数并处理它的返回值或副作用。
    void set_output_volume(uint8_t percent);
    // 函数调用：void set_input_gain(uint8_t gain_db);；执行该函数并处理它的返回值或副作用。
    void set_input_gain(uint8_t gain_db);
    // 函数调用：uint8_t get_output_volume() const;；执行该函数并处理它的返回值或副作用。
    uint8_t get_output_volume() const;
    // 函数调用：uint8_t get_input_gain() const;；执行该函数并处理它的返回值或副作用。
    uint8_t get_input_gain() const;

    // 函数调用：int read(int16_t* dest, int samples);；执行该函数并处理它的返回值或副作用。
    int read(int16_t* dest, int samples);
    // 函数调用：int write(const int16_t* data, int samples);；执行该函数并处理它的返回值或副作用。
    int write(const int16_t* data, int samples);

    // 定义或声明函数 is_init：参数在括号里，返回类型写在函数名前面。
    bool is_init() const
    {
        // 返回 initialized_ 给调用者，表示当前函数的执行结果。
        return initialized_;
    }
    // 定义或声明函数 is_input_enabled：参数在括号里，返回类型写在函数名前面。
    bool is_input_enabled() const
    {
        // 返回 input_enabled_ 给调用者，表示当前函数的执行结果。
        return input_enabled_;
    }
    // 定义或声明函数 is_output_enabled：参数在括号里，返回类型写在函数名前面。
    bool is_output_enabled() const
    {
        // 返回 output_enabled_ 给调用者，表示当前函数的执行结果。
        return output_enabled_;
    }

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 函数调用：bool setup_i2s();；执行该函数并处理它的返回值或副作用。
    bool setup_i2s();
    // 函数调用：bool setup_codec_stack();；执行该函数并处理它的返回值或副作用。
    bool setup_codec_stack();
    // 函数调用：void release_dev();；执行该函数并处理它的返回值或副作用。
    void release_dev();
    // 函数调用：void release_all();；执行该函数并处理它的返回值或副作用。
    void release_all();
    // 函数调用：void sync_data_path();；执行该函数并处理它的返回值或副作用。
    void sync_data_path();
    // 函数调用：void apply_pa();；执行该函数并处理它的返回值或副作用。
    void apply_pa();
    // 函数调用：void apply_dev_gain_volume();；执行该函数并处理它的返回值或副作用。
    void apply_dev_gain_volume();

    // 执行语句：i2c_master_bus_handle_t bus_handle_;；它会更新一次变量、对象或程序状态。
    i2c_master_bus_handle_t bus_handle_;
    // 执行语句：Config cfg_;；它会更新一次变量、对象或程序状态。
    Config cfg_;

    // 执行语句：void* tx_handle_;；它会更新一次变量、对象或程序状态。
    void* tx_handle_;
    // 执行语句：void* rx_handle_;；它会更新一次变量、对象或程序状态。
    void* rx_handle_;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const void* data_if_;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const void* ctrl_if_;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const void* gpio_if_;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const void* codec_if_;
    // 执行语句：void* dev_;；它会更新一次变量、对象或程序状态。
    void* dev_;

    // 执行语句：bool initialized_;；它会更新一次变量、对象或程序状态。
    bool initialized_;
    // 执行语句：bool input_enabled_;；它会更新一次变量、对象或程序状态。
    bool input_enabled_;
    // 执行语句：bool output_enabled_;；它会更新一次变量、对象或程序状态。
    bool output_enabled_;
    // 执行语句：uint8_t output_volume_;；它会更新一次变量、对象或程序状态。
    uint8_t output_volume_;
    // 执行语句：uint8_t input_gain_;；它会更新一次变量、对象或程序状态。
    uint8_t input_gain_;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::bsp::driver::audio::es8311
