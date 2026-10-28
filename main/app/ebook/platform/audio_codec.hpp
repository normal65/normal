// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/platform/audio_codec.hpp
 * 作用：把系统音量、静音和音频格式应用到 ES8311/I2S 通路。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：阻塞工作放独立任务，周期小检查放 Housekeeper。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <mutex>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <driver/i2c_master.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "es8311_codec.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::platform {

/** @brief 全机 ES8311；SystemState 观察者触发 apply_system_volume() */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class AudioCodec
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static AudioCodec& get_instance();

    // 赋值：把右侧 16000) 的结果保存到左侧 bool init(i2c_master_bus_handle_t bus, uint32_t sample_rate_hz。
    bool init(i2c_master_bus_handle_t bus, uint32_t sample_rate_hz = 16000);
    // 调用 deinit()：创建和销毁队列。
    void deinit();

    // 赋值：把右侧 1) 的结果保存到左侧 bool ensure_pcm_format(uint32_t sample_rate_hz, uint8_t channels。
    bool ensure_pcm_format(uint32_t sample_rate_hz, uint8_t channels = 1);
    // 函数调用：bool ensure_sample_rate(uint32_t sample_rate_hz);；执行该函数并处理它的返回值或副作用。
    bool ensure_sample_rate(uint32_t sample_rate_hz);

    // 定义或声明函数 ready：参数在括号里，返回类型写在函数名前面。
    bool ready() const
    {
        // 返回 ready_ 给调用者，表示当前函数的执行结果。
        return ready_;
    }
    // 定义或声明函数 sample_rate：参数在括号里，返回类型写在函数名前面。
    uint32_t sample_rate() const
    {
        // 返回 sample_rate_ 给调用者，表示当前函数的执行结果。
        return sample_rate_;
    }
    // 定义或声明函数 channels：参数在括号里，返回类型写在函数名前面。
    uint8_t channels() const
    {
        // 返回 channels_ 给调用者，表示当前函数的执行结果。
        return channels_;
    }

    // 函数调用：::app::bsp::driver::audio::es8311::Es8311Codec& codec();；执行该函数并处理它的返回值或副作用。
    ::app::bsp::driver::audio::es8311::Es8311Codec& codec();

    // 函数调用：void apply_system_volume();；执行该函数并处理它的返回值或副作用。
    void apply_system_volume();

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 AudioCodec() = default;，执行这个函数并处理它的返回值或状态变化。
    AudioCodec() = default;

    // 执行语句：i2c_master_bus_handle_t bus_{nullptr};；它会更新一次变量、对象或程序状态。
    i2c_master_bus_handle_t bus_{nullptr};
    // 执行语句：uint32_t sample_rate_{16000};；它会更新一次变量、对象或程序状态。
    uint32_t sample_rate_{16000};
    // 执行语句：uint8_t channels_{1};；它会更新一次变量、对象或程序状态。
    uint8_t channels_{1};
    // 执行语句：bool ready_{false};；它会更新一次变量、对象或程序状态。
    bool ready_{false};
    // 声明并默认初始化对象：std::mutex mtx_{}。
    std::mutex mtx_{};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::platform
