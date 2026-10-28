/**
 * [初学者导读] 文件：main/bsp/driver/audio/es8311/es8311_codec.cc
 * 作用：控制 ES8311 音频 Codec 的初始化、采样格式、音量和播放通路。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：音频使用 DMA 和环形缓冲，启动停止顺序会影响杂音和尾音。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "es8311_codec.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <algorithm>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstddef>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "config/config.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <driver/gpio.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <driver/i2s_std.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <driver/i2s_types.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_err.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_log.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "audio_codec_if.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "es8311_codec.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_codec_dev.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_codec_dev_defaults.h"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "Es8311Codec";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::bsp::driver::audio::es8311 {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 固定参数 kI2sPort = 0：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr int kI2sPort = 0;
// 固定参数 kDmaDescNum = 6：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t kDmaDescNum = 6;
// 固定参数 kDmaFrameNum = 240：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t kDmaFrameNum = 240;

// 定义或声明函数 to_data_width：参数在括号里，返回类型写在函数名前面。
i2s_data_bit_width_t to_data_width(uint8_t bits)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (bits)
    {
        // 分支匹配：case 16 成立时执行下面的代码。
        case 16:
            // 返回 I2S_DATA_BIT_WIDTH_16BIT 给调用者，表示当前函数的执行结果。
            return I2S_DATA_BIT_WIDTH_16BIT;
        // 分支匹配：case 24 成立时执行下面的代码。
        case 24:
            // 返回 I2S_DATA_BIT_WIDTH_24BIT 给调用者，表示当前函数的执行结果。
            return I2S_DATA_BIT_WIDTH_24BIT;
        // 分支匹配：case 32 成立时执行下面的代码。
        case 32:
            // 返回 I2S_DATA_BIT_WIDTH_32BIT 给调用者，表示当前函数的执行结果。
            return I2S_DATA_BIT_WIDTH_32BIT;
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 返回 I2S_DATA_BIT_WIDTH_16BIT 给调用者，表示当前函数的执行结果。
            return I2S_DATA_BIT_WIDTH_16BIT;
    }
}

/** esp_codec I2C 配置需 8-bit 写地址，组件内部会 >> 1 */
// 定义或声明函数 i2c_addr_to_8bit：参数在括号里，返回类型写在函数名前面。
uint8_t i2c_addr_to_8bit(uint8_t addr_7bit)
{
    // 返回 static_cast<uint8_t>(addr_7bit << 1) 给调用者，表示当前函数的执行结果。
    return static_cast<uint8_t>(addr_7bit << 1);
}

// 定义或声明函数 destroy_i2s_channel：参数在括号里，返回类型写在函数名前面。
void destroy_i2s_channel(void*& handle)
{
    // 判断条件：handle == nullptr 是否成立。
    if (handle == nullptr)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 赋值：把右侧 static_cast<i2s_chan_handle_t>(handle) 的结果保存到左侧 auto* ch。
    auto* ch = static_cast<i2s_chan_handle_t>(handle);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const esp_err_t dis = i2s_channel_disable(ch);
    // 判断条件：dis != ESP_OK && dis != ESP_ERR_INVALID_STATE 是否成立。
    if (dis != ESP_OK && dis != ESP_ERR_INVALID_STATE)
    {
        // 调用 ESP_LOGW(TAG, "i2s disable: %s", esp_err_to_name(dis));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGW(TAG, "i2s disable: %s", esp_err_to_name(dis));
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const esp_err_t del = i2s_del_channel(ch);
    // 判断条件：del != ESP_OK 是否成立。
    if (del != ESP_OK)
    {
        // 调用 ESP_LOGW(TAG, "i2s del_channel: %s", esp_err_to_name(del));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGW(TAG, "i2s del_channel: %s", esp_err_to_name(del));
    }
    // 赋值：把右侧 nullptr 的结果保存到左侧 handle。
    handle = nullptr;
}

// 定义或声明函数 fill_i2s_std_config：参数在括号里，返回类型写在函数名前面。
void fill_i2s_std_config(const Config& cfg, i2s_std_config_t& out)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const i2s_data_bit_width_t dbw = to_data_width(cfg.bits_per_sample);

    // 赋值：把右侧 {} 的结果保存到左侧 out。
    out = {};
    // 赋值：把右侧 cfg.sample_rate 的结果保存到左侧 out.clk_cfg.sample_rate_hz。
    out.clk_cfg.sample_rate_hz = cfg.sample_rate;
    // 赋值：把右侧 I2S_CLK_SRC_DEFAULT 的结果保存到左侧 out.clk_cfg.clk_src。
    out.clk_cfg.clk_src = I2S_CLK_SRC_DEFAULT;
    // 赋值：把右侧 I2S_MCLK_MULTIPLE_256 的结果保存到左侧 out.clk_cfg.mclk_multiple。
    out.clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_256;

    // 赋值：把右侧 dbw 的结果保存到左侧 out.slot_cfg.data_bit_width。
    out.slot_cfg.data_bit_width = dbw;
    // 赋值：把右侧 I2S_SLOT_BIT_WIDTH_AUTO 的结果保存到左侧 out.slot_cfg.slot_bit_width。
    out.slot_cfg.slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO;
    // 赋值：把右侧 I2S_SLOT_MODE_STEREO 的结果保存到左侧 out.slot_cfg.slot_mode。
    out.slot_cfg.slot_mode = I2S_SLOT_MODE_STEREO;
    // 赋值：把右侧 I2S_STD_SLOT_BOTH 的结果保存到左侧 out.slot_cfg.slot_mask。
    out.slot_cfg.slot_mask = I2S_STD_SLOT_BOTH;
    // 赋值：把右侧 dbw 的结果保存到左侧 out.slot_cfg.ws_width。
    out.slot_cfg.ws_width = dbw;
    // 赋值：把右侧 false 的结果保存到左侧 out.slot_cfg.ws_pol。
    out.slot_cfg.ws_pol = false;
    // 赋值：把右侧 true 的结果保存到左侧 out.slot_cfg.bit_shift。
    out.slot_cfg.bit_shift = true;

    // 赋值：把右侧 config::I2S_MCLK 的结果保存到左侧 out.gpio_cfg.mclk。
    out.gpio_cfg.mclk = config::I2S_MCLK;
    // 赋值：把右侧 config::I2S_SCLK 的结果保存到左侧 out.gpio_cfg.bclk。
    out.gpio_cfg.bclk = config::I2S_SCLK;
    // 赋值：把右侧 config::I2S_LRCK 的结果保存到左侧 out.gpio_cfg.ws。
    out.gpio_cfg.ws = config::I2S_LRCK;
    // 赋值：把右侧 config::I2S_DO 的结果保存到左侧 out.gpio_cfg.dout。
    out.gpio_cfg.dout = config::I2S_DO;
    // 赋值：把右侧 config::I2S_DI 的结果保存到左侧 out.gpio_cfg.din。
    out.gpio_cfg.din = config::I2S_DI;
    // 赋值：把右侧 false 的结果保存到左侧 out.gpio_cfg.invert_flags.mclk_inv。
    out.gpio_cfg.invert_flags.mclk_inv = false;
    // 赋值：把右侧 false 的结果保存到左侧 out.gpio_cfg.invert_flags.bclk_inv。
    out.gpio_cfg.invert_flags.bclk_inv = false;
    // 赋值：把右侧 false 的结果保存到左侧 out.gpio_cfg.invert_flags.ws_inv。
    out.gpio_cfg.invert_flags.ws_inv = false;
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 Es8311Codec::Es8311Codec：参数在括号里，返回类型写在函数名前面。
Es8311Codec::Es8311Codec()
    // 定义或声明函数 bus_handle_：参数在括号里，返回类型写在函数名前面。
    : bus_handle_(nullptr)
    // 执行这条代码：, cfg_{}。它会更新变量、对象状态或调用链。
    , cfg_{}
    // 定义或声明函数 tx_handle_：参数在括号里，返回类型写在函数名前面。
    , tx_handle_(nullptr)
    // 定义或声明函数 rx_handle_：参数在括号里，返回类型写在函数名前面。
    , rx_handle_(nullptr)
    // 定义或声明函数 data_if_：参数在括号里，返回类型写在函数名前面。
    , data_if_(nullptr)
    // 定义或声明函数 ctrl_if_：参数在括号里，返回类型写在函数名前面。
    , ctrl_if_(nullptr)
    // 定义或声明函数 gpio_if_：参数在括号里，返回类型写在函数名前面。
    , gpio_if_(nullptr)
    // 定义或声明函数 codec_if_：参数在括号里，返回类型写在函数名前面。
    , codec_if_(nullptr)
    // 定义或声明函数 dev_：参数在括号里，返回类型写在函数名前面。
    , dev_(nullptr)
    // 定义或声明函数 initialized_：参数在括号里，返回类型写在函数名前面。
    , initialized_(false)
    // 定义或声明函数 input_enabled_：参数在括号里，返回类型写在函数名前面。
    , input_enabled_(false)
    // 定义或声明函数 output_enabled_：参数在括号里，返回类型写在函数名前面。
    , output_enabled_(false)
    // 定义或声明函数 output_volume_：参数在括号里，返回类型写在函数名前面。
    , output_volume_(60)
    // 定义或声明函数 input_gain_：参数在括号里，返回类型写在函数名前面。
    , input_gain_(30)
{
}

// 定义或声明函数 Es8311Codec::~Es8311Codec：参数在括号里，返回类型写在函数名前面。
Es8311Codec::~Es8311Codec()
{
    // 调用 deinit()：创建和销毁队列。
    deinit();
}

// 定义或声明函数 Es8311Codec::release_dev：参数在括号里，返回类型写在函数名前面。
void Es8311Codec::release_dev()
{
    // 判断条件：dev_ == nullptr 是否成立。
    if (dev_ == nullptr)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 赋值：把右侧 static_cast<esp_codec_dev_handle_t>(dev_) 的结果保存到左侧 auto* handle。
    auto* handle = static_cast<esp_codec_dev_handle_t>(dev_);
    // 调用 esp_codec_dev_close(handle);，执行这个函数并处理它的返回值或状态变化。
    esp_codec_dev_close(handle);
    // 调用 esp_codec_dev_delete(handle);，执行这个函数并处理它的返回值或状态变化。
    esp_codec_dev_delete(handle);
    // 赋值：把右侧 nullptr 的结果保存到左侧 dev_。
    dev_ = nullptr;
}

// 定义或声明函数 Es8311Codec::release_all：参数在括号里，返回类型写在函数名前面。
void Es8311Codec::release_all()
{
    // 调用 release_dev();，执行这个函数并处理它的返回值或状态变化。
    release_dev();

    // 判断条件：codec_if_ != nullptr 是否成立。
    if (codec_if_ != nullptr)
    {
        // 调用 audio_codec_delete_codec_if(static_cast<const audio_codec_if_t*>(codec_if_));，执行这个函数并处理它的返回值或状态变化。
        audio_codec_delete_codec_if(static_cast<const audio_codec_if_t*>(codec_if_));
        // 赋值：把右侧 nullptr 的结果保存到左侧 codec_if_。
        codec_if_ = nullptr;
    }
    // 判断条件：ctrl_if_ != nullptr 是否成立。
    if (ctrl_if_ != nullptr)
    {
        // 调用 audio_codec_delete_ctrl_if(static_cast<const audio_codec_ctrl_if_t*>(ctrl_if_));，执行这个函数并处理它的返回值或状态变化。
        audio_codec_delete_ctrl_if(static_cast<const audio_codec_ctrl_if_t*>(ctrl_if_));
        // 赋值：把右侧 nullptr 的结果保存到左侧 ctrl_if_。
        ctrl_if_ = nullptr;
    }
    // 判断条件：gpio_if_ != nullptr 是否成立。
    if (gpio_if_ != nullptr)
    {
        // 调用 audio_codec_delete_gpio_if(static_cast<const audio_codec_gpio_if_t*>(gpio_if_));，执行这个函数并处理它的返回值或状态变化。
        audio_codec_delete_gpio_if(static_cast<const audio_codec_gpio_if_t*>(gpio_if_));
        // 赋值：把右侧 nullptr 的结果保存到左侧 gpio_if_。
        gpio_if_ = nullptr;
    }
    // 判断条件：data_if_ != nullptr 是否成立。
    if (data_if_ != nullptr)
    {
        // 调用 audio_codec_delete_data_if(static_cast<const audio_codec_data_if_t*>(data_if_));，执行这个函数并处理它的返回值或状态变化。
        audio_codec_delete_data_if(static_cast<const audio_codec_data_if_t*>(data_if_));
        // 赋值：把右侧 nullptr 的结果保存到左侧 data_if_。
        data_if_ = nullptr;
    }

    // 调用 destroy_i2s_channel(tx_handle_);，执行这个函数并处理它的返回值或状态变化。
    destroy_i2s_channel(tx_handle_);
    // 调用 destroy_i2s_channel(rx_handle_);，执行这个函数并处理它的返回值或状态变化。
    destroy_i2s_channel(rx_handle_);

    // 赋值：把右侧 nullptr 的结果保存到左侧 bus_handle_。
    bus_handle_ = nullptr;
    // 赋值：把右侧 false 的结果保存到左侧 initialized_。
    initialized_ = false;
    // 赋值：把右侧 false 的结果保存到左侧 input_enabled_。
    input_enabled_ = false;
    // 赋值：把右侧 false 的结果保存到左侧 output_enabled_。
    output_enabled_ = false;
}

// 定义或声明函数 Es8311Codec::setup_i2s：参数在括号里，返回类型写在函数名前面。
bool Es8311Codec::setup_i2s()
{
    // 定义变量 i2s_chan_config_t chan_cfg 并初始化为 {}。
    i2s_chan_config_t chan_cfg = {};
    // 赋值：把右侧 kI2sPort 的结果保存到左侧 chan_cfg.id。
    chan_cfg.id = kI2sPort;
    // 赋值：把右侧 I2S_ROLE_MASTER 的结果保存到左侧 chan_cfg.role。
    chan_cfg.role = I2S_ROLE_MASTER;
    // 赋值：把右侧 kDmaDescNum 的结果保存到左侧 chan_cfg.dma_desc_num。
    chan_cfg.dma_desc_num = kDmaDescNum;
    // 赋值：把右侧 kDmaFrameNum 的结果保存到左侧 chan_cfg.dma_frame_num。
    chan_cfg.dma_frame_num = kDmaFrameNum;
    // 赋值：把右侧 true 的结果保存到左侧 chan_cfg.auto_clear_after_cb。
    chan_cfg.auto_clear_after_cb = true;
    // 赋值：把右侧 false 的结果保存到左侧 chan_cfg.auto_clear_before_cb。
    chan_cfg.auto_clear_before_cb = false;

    // 定义变量 i2s_chan_handle_t tx 并初始化为 nullptr。
    i2s_chan_handle_t tx = nullptr;
    // 定义变量 i2s_chan_handle_t rx 并初始化为 nullptr。
    i2s_chan_handle_t rx = nullptr;

    // 定义变量 esp_err_t err 并初始化为 i2s_new_channel(&chan_cfg, &tx, &rx)。
    esp_err_t err = i2s_new_channel(&chan_cfg, &tx, &rx);
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "i2s_new_channel 失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "i2s_new_channel 失败: %s", esp_err_to_name(err));
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 赋值：把右侧 tx 的结果保存到左侧 tx_handle_。
    tx_handle_ = tx;
    // 赋值：把右侧 rx 的结果保存到左侧 rx_handle_。
    rx_handle_ = rx;

    // 定义变量 i2s_std_config_t std_cfg 并初始化为 {}。
    i2s_std_config_t std_cfg = {};
    // 调用 fill_i2s_std_config(cfg_, std_cfg);，执行这个函数并处理它的返回值或状态变化。
    fill_i2s_std_config(cfg_, std_cfg);

    // 赋值：把右侧 i2s_channel_init_std_mode(tx, &std_cfg) 的结果保存到左侧 err。
    err = i2s_channel_init_std_mode(tx, &std_cfg);
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "i2s TX init 失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "i2s TX init 失败: %s", esp_err_to_name(err));
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 赋值：把右侧 i2s_channel_init_std_mode(rx, &std_cfg) 的结果保存到左侧 err。
    err = i2s_channel_init_std_mode(rx, &std_cfg);
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "i2s RX init 失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "i2s RX init 失败: %s", esp_err_to_name(err));
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 赋值：把右侧 i2s_channel_enable(tx) 的结果保存到左侧 err。
    err = i2s_channel_enable(tx);
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "i2s TX enable 失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "i2s TX enable 失败: %s", esp_err_to_name(err));
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 赋值：把右侧 i2s_channel_enable(rx) 的结果保存到左侧 err。
    err = i2s_channel_enable(rx);
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "i2s RX enable 失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "i2s RX enable 失败: %s", esp_err_to_name(err));
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 Es8311Codec::setup_codec_stack：参数在括号里，返回类型写在函数名前面。
bool Es8311Codec::setup_codec_stack()
{
    // 赋值：把右侧 static_cast<i2s_chan_handle_t>(tx_handle_) 的结果保存到左侧 auto* tx。
    auto* tx = static_cast<i2s_chan_handle_t>(tx_handle_);
    // 赋值：把右侧 static_cast<i2s_chan_handle_t>(rx_handle_) 的结果保存到左侧 auto* rx。
    auto* rx = static_cast<i2s_chan_handle_t>(rx_handle_);

    // 定义变量 audio_codec_i2s_cfg_t i2s_data_cfg 并初始化为 {}。
    audio_codec_i2s_cfg_t i2s_data_cfg = {};
    // 赋值：把右侧 static_cast<uint8_t>(kI2sPort) 的结果保存到左侧 i2s_data_cfg.port。
    i2s_data_cfg.port = static_cast<uint8_t>(kI2sPort);
    // 赋值：把右侧 rx 的结果保存到左侧 i2s_data_cfg.rx_handle。
    i2s_data_cfg.rx_handle = rx;
    // 赋值：把右侧 tx 的结果保存到左侧 i2s_data_cfg.tx_handle。
    i2s_data_cfg.tx_handle = tx;
    // 赋值：把右侧 0 的结果保存到左侧 i2s_data_cfg.clk_src。
    i2s_data_cfg.clk_src = 0;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const audio_codec_data_if_t* data_if = audio_codec_new_i2s_data(&i2s_data_cfg);
    // 判断条件：data_if == nullptr 是否成立。
    if (data_if == nullptr)
    {
        // 调用 ESP_LOGE(TAG, "audio_codec_new_i2s_data 失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "audio_codec_new_i2s_data 失败");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 赋值：把右侧 static_cast<const void*>(data_if) 的结果保存到左侧 data_if_。
    data_if_ = static_cast<const void*>(data_if);

    // 定义变量 audio_codec_i2c_cfg_t i2c_cfg 并初始化为 {}。
    audio_codec_i2c_cfg_t i2c_cfg = {};
    // 赋值：把右侧 static_cast<uint8_t>(cfg_.i2c_port) 的结果保存到左侧 i2c_cfg.port。
    i2c_cfg.port = static_cast<uint8_t>(cfg_.i2c_port);
    // 赋值：把右侧 i2c_addr_to_8bit(cfg_.i2c_addr) 的结果保存到左侧 i2c_cfg.addr。
    i2c_cfg.addr = i2c_addr_to_8bit(cfg_.i2c_addr);
    // 赋值：把右侧 bus_handle_ 的结果保存到左侧 i2c_cfg.bus_handle。
    i2c_cfg.bus_handle = bus_handle_;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const audio_codec_ctrl_if_t* ctrl_if = audio_codec_new_i2c_ctrl(&i2c_cfg);
    // 判断条件：ctrl_if == nullptr 是否成立。
    if (ctrl_if == nullptr)
    {
        // 调用 ESP_LOGE(TAG, "audio_codec_new_i2c_ctrl 失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "audio_codec_new_i2c_ctrl 失败");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 赋值：把右侧 static_cast<const void*>(ctrl_if) 的结果保存到左侧 ctrl_if_。
    ctrl_if_ = static_cast<const void*>(ctrl_if);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const audio_codec_gpio_if_t* gpio_if = audio_codec_new_gpio();
    // 判断条件：gpio_if == nullptr 是否成立。
    if (gpio_if == nullptr)
    {
        // 调用 ESP_LOGE(TAG, "audio_codec_new_gpio 失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "audio_codec_new_gpio 失败");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 赋值：把右侧 static_cast<const void*>(gpio_if) 的结果保存到左侧 gpio_if_。
    gpio_if_ = static_cast<const void*>(gpio_if);

    // 定义变量 es8311_codec_cfg_t es8311_cfg 并初始化为 {}。
    es8311_codec_cfg_t es8311_cfg = {};
    // 赋值：把右侧 ctrl_if 的结果保存到左侧 es8311_cfg.ctrl_if。
    es8311_cfg.ctrl_if = ctrl_if;
    // 赋值：把右侧 gpio_if 的结果保存到左侧 es8311_cfg.gpio_if。
    es8311_cfg.gpio_if = gpio_if;
    // 赋值：把右侧 ESP_CODEC_DEV_WORK_MODE_BOTH 的结果保存到左侧 es8311_cfg.codec_mode。
    es8311_cfg.codec_mode = ESP_CODEC_DEV_WORK_MODE_BOTH;
    // 赋值：把右侧 static_cast<int16_t>(config::I2S_PA) 的结果保存到左侧 es8311_cfg.pa_pin。
    es8311_cfg.pa_pin = static_cast<int16_t>(config::I2S_PA);
    // 赋值：把右侧 cfg_.use_mclk 的结果保存到左侧 es8311_cfg.use_mclk。
    es8311_cfg.use_mclk = cfg_.use_mclk;
    // 赋值：把右侧 5.0F 的结果保存到左侧 es8311_cfg.hw_gain.pa_voltage。
    es8311_cfg.hw_gain.pa_voltage = 5.0F;
    // 赋值：把右侧 3.3F 的结果保存到左侧 es8311_cfg.hw_gain.codec_dac_voltage。
    es8311_cfg.hw_gain.codec_dac_voltage = 3.3F;
    // 赋值：把右侧 cfg_.pa_inverted 的结果保存到左侧 es8311_cfg.pa_reverted。
    es8311_cfg.pa_reverted = cfg_.pa_inverted;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const audio_codec_if_t* codec_if = es8311_codec_new(&es8311_cfg);
    // 判断条件：codec_if == nullptr 是否成立。
    if (codec_if == nullptr)
    {
        // 调用 ESP_LOGE(TAG, "es8311_codec_new 失败");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "es8311_codec_new 失败");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 赋值：把右侧 static_cast<const void*>(codec_if) 的结果保存到左侧 codec_if_。
    codec_if_ = static_cast<const void*>(codec_if);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 Es8311Codec::init：参数在括号里，返回类型写在函数名前面。
bool Es8311Codec::init(i2c_master_bus_handle_t bus_handle, const Config* config)
{
    // 判断条件：initialized_ 是否成立。
    if (initialized_)
    {
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 判断条件：bus_handle == nullptr 是否成立。
    if (bus_handle == nullptr)
    {
        // 调用 ESP_LOGE(TAG, "bus_handle 为空");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "bus_handle 为空");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 (config != nullptr) ? *config : Config() 的结果保存到左侧 cfg_。
    cfg_ = (config != nullptr) ? *config : Config();
    // 赋值：把右侧 bus_handle 的结果保存到左侧 bus_handle_。
    bus_handle_ = bus_handle;
    // 赋值：把右侧 cfg_.default_output_volume 的结果保存到左侧 output_volume_。
    output_volume_ = cfg_.default_output_volume;
    // 赋值：把右侧 cfg_.default_input_gain_db 的结果保存到左侧 input_gain_。
    input_gain_ = cfg_.default_input_gain_db;

    // 判断条件：cfg_.bits_per_sample != 16 是否成立。
    if (cfg_.bits_per_sample != 16)
    {
        // 调用 ESP_LOGW(TAG, "当前仅验证 16-bit，将按 16-bit 运行");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGW(TAG, "当前仅验证 16-bit，将按 16-bit 运行");
    }

    // 判断条件：!setup_i2s() 是否成立。
    if (!setup_i2s())
    {
        // 调用 release_all();，执行这个函数并处理它的返回值或状态变化。
        release_all();
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 判断条件：!setup_codec_stack() 是否成立。
    if (!setup_codec_stack())
    {
        // 调用 release_all();，执行这个函数并处理它的返回值或状态变化。
        release_all();
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 true 的结果保存到左侧 initialized_。
    initialized_ = true;
    // 调用 ESP_LOGD(TAG, "OK  %u Hz  ch=%u  I2C=0x%02X  I2S=%d", static_cast<unsigned>(cfg_.sample_rate),，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "OK  %u Hz  ch=%u  I2C=0x%02X  I2S=%d", static_cast<unsigned>(cfg_.sample_rate),
             // 继续上一行的表达式或参数列表：static_cast<unsigned>(cfg_.channels), static_cast<unsigned>(cfg_.i2c_addr), kI2sPort);。
             static_cast<unsigned>(cfg_.channels), static_cast<unsigned>(cfg_.i2c_addr), kI2sPort);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 Es8311Codec::deinit：参数在括号里，返回类型写在函数名前面。
void Es8311Codec::deinit()
{
    // 判断条件：!initialized_ 是否成立。
    if (!initialized_)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 调用 release_all();，执行这个函数并处理它的返回值或状态变化。
    release_all();
}

// 定义或声明函数 Es8311Codec::apply_dev_gain_volume：参数在括号里，返回类型写在函数名前面。
void Es8311Codec::apply_dev_gain_volume()
{
    // 判断条件：dev_ == nullptr 是否成立。
    if (dev_ == nullptr)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 赋值：把右侧 static_cast<esp_codec_dev_handle_t>(dev_) 的结果保存到左侧 auto* handle。
    auto* handle = static_cast<esp_codec_dev_handle_t>(dev_);

    // 定义变量 esp_err_t err 并初始化为 esp_codec_dev_set_in_gain(handle, static_cast<float>(input_gain_))。
    esp_err_t err = esp_codec_dev_set_in_gain(handle, static_cast<float>(input_gain_));
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 调用 ESP_LOGW(TAG, "set_in_gain 失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGW(TAG, "set_in_gain 失败: %s", esp_err_to_name(err));
    }
    // 赋值：把右侧 esp_codec_dev_set_out_vol(handle, static_cast<float>(output_volume_)) 的结果保存到左侧 err。
    err = esp_codec_dev_set_out_vol(handle, static_cast<float>(output_volume_));
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 调用 ESP_LOGW(TAG, "set_out_vol 失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGW(TAG, "set_out_vol 失败: %s", esp_err_to_name(err));
    }
}

// 定义或声明函数 Es8311Codec::apply_pa：参数在括号里，返回类型写在函数名前面。
void Es8311Codec::apply_pa()
{
    // 判断条件：config::I2S_PA == GPIO_NUM_NC 是否成立。
    if (config::I2S_PA == GPIO_NUM_NC)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 定义变量 int level 并初始化为 output_enabled_ ? 1 : 0。
    int level = output_enabled_ ? 1 : 0;
    // 判断条件：cfg_.pa_inverted 是否成立。
    if (cfg_.pa_inverted)
    {
        // 赋值：把右侧 (level != 0) ? 0 : 1 的结果保存到左侧 level。
        level = (level != 0) ? 0 : 1;
    }
    // 调用 gpio_set_level(config::I2S_PA, level);，执行这个函数并处理它的返回值或状态变化。
    gpio_set_level(config::I2S_PA, level);
}

// 定义或声明函数 Es8311Codec::sync_data_path：参数在括号里，返回类型写在函数名前面。
void Es8311Codec::sync_data_path()
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool need_dev = input_enabled_ || output_enabled_;

    // 判断条件：need_dev && dev_ == nullptr 是否成立。
    if (need_dev && dev_ == nullptr)
    {
        // 定义变量 esp_codec_dev_cfg_t dev_cfg 并初始化为 {}。
        esp_codec_dev_cfg_t dev_cfg = {};
        // 赋值：把右侧 ESP_CODEC_DEV_TYPE_IN_OUT 的结果保存到左侧 dev_cfg.dev_type。
        dev_cfg.dev_type = ESP_CODEC_DEV_TYPE_IN_OUT;
        // 赋值：把右侧 static_cast<const audio_codec_if_t*>(codec_if_) 的结果保存到左侧 dev_cfg.codec_if。
        dev_cfg.codec_if = static_cast<const audio_codec_if_t*>(codec_if_);
        // 赋值：把右侧 static_cast<const audio_codec_data_if_t*>(data_if_) 的结果保存到左侧 dev_cfg.data_if。
        dev_cfg.data_if = static_cast<const audio_codec_data_if_t*>(data_if_);

        // 定义变量 esp_codec_dev_handle_t handle 并初始化为 esp_codec_dev_new(&dev_cfg)。
        esp_codec_dev_handle_t handle = esp_codec_dev_new(&dev_cfg);
        // 判断条件：handle == nullptr 是否成立。
        if (handle == nullptr)
        {
            // 调用 ESP_LOGE(TAG, "esp_codec_dev_new 失败");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "esp_codec_dev_new 失败");
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }

        // 定义变量 esp_codec_dev_sample_info_t fs 并初始化为 {}。
        esp_codec_dev_sample_info_t fs = {};
        // 赋值：把右侧 cfg_.bits_per_sample 的结果保存到左侧 fs.bits_per_sample。
        fs.bits_per_sample = cfg_.bits_per_sample;
        // 赋值：把右侧 cfg_.channels 的结果保存到左侧 fs.channel。
        fs.channel = cfg_.channels;
        // 赋值：把右侧 0 的结果保存到左侧 fs.channel_mask。
        fs.channel_mask = 0;
        // 赋值：把右侧 cfg_.sample_rate 的结果保存到左侧 fs.sample_rate。
        fs.sample_rate = cfg_.sample_rate;
        // 赋值：把右侧 0 的结果保存到左侧 fs.mclk_multiple。
        fs.mclk_multiple = 0;

        // 定义变量 esp_err_t err 并初始化为 esp_codec_dev_open(handle, &fs)。
        esp_err_t err = esp_codec_dev_open(handle, &fs);
        // 判断条件：err != ESP_OK 是否成立。
        if (err != ESP_OK)
        {
            // 调用 ESP_LOGE(TAG, "esp_codec_dev_open 失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "esp_codec_dev_open 失败: %s", esp_err_to_name(err));
            // 调用 esp_codec_dev_delete(handle);，执行这个函数并处理它的返回值或状态变化。
            esp_codec_dev_delete(handle);
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }

        // 赋值：把右侧 handle 的结果保存到左侧 dev_。
        dev_ = handle;
        // 调用 apply_dev_gain_volume();，执行这个函数并处理它的返回值或状态变化。
        apply_dev_gain_volume();
    }
    // 备用分支：前面的条件不成立时执行这里。
    else if (!need_dev && dev_ != nullptr)
    {
        // 调用 release_dev();，执行这个函数并处理它的返回值或状态变化。
        release_dev();
    }

    // 调用 apply_pa();，执行这个函数并处理它的返回值或状态变化。
    apply_pa();
}

// 定义或声明函数 Es8311Codec::enable_input：参数在括号里，返回类型写在函数名前面。
void Es8311Codec::enable_input(bool enable)
{
    // 判断条件：!initialized_ || enable == input_enabled_ 是否成立。
    if (!initialized_ || enable == input_enabled_)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 赋值：把右侧 enable 的结果保存到左侧 input_enabled_。
    input_enabled_ = enable;
    // 调用 sync_data_path();，执行这个函数并处理它的返回值或状态变化。
    sync_data_path();
}

// 定义或声明函数 Es8311Codec::enable_output：参数在括号里，返回类型写在函数名前面。
void Es8311Codec::enable_output(bool enable)
{
    // 判断条件：!initialized_ || enable == output_enabled_ 是否成立。
    if (!initialized_ || enable == output_enabled_)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 赋值：把右侧 enable 的结果保存到左侧 output_enabled_。
    output_enabled_ = enable;
    // 调用 sync_data_path();，执行这个函数并处理它的返回值或状态变化。
    sync_data_path();
}

// 定义或声明函数 Es8311Codec::set_output_volume：参数在括号里，返回类型写在函数名前面。
void Es8311Codec::set_output_volume(uint8_t percent)
{
    // 赋值：把右侧 std::min<uint8_t>(percent, static_cast<uint8_t>(100)) 的结果保存到左侧 output_volume_。
    output_volume_ = std::min<uint8_t>(percent, static_cast<uint8_t>(100));
    // 判断条件：dev_ != nullptr 是否成立。
    if (dev_ != nullptr)
    {
        // 赋值：把右侧 static_cast<esp_codec_dev_handle_t>(dev_) 的结果保存到左侧 auto* handle。
        auto* handle = static_cast<esp_codec_dev_handle_t>(dev_);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const esp_err_t err = esp_codec_dev_set_out_vol(handle, static_cast<float>(output_volume_));
        // 判断条件：err != ESP_OK 是否成立。
        if (err != ESP_OK)
        {
            // 调用 ESP_LOGW(TAG, "set_out_vol 失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGW(TAG, "set_out_vol 失败: %s", esp_err_to_name(err));
        }
    }
}

// 定义或声明函数 Es8311Codec::set_input_gain：参数在括号里，返回类型写在函数名前面。
void Es8311Codec::set_input_gain(uint8_t gain_db)
{
    // 赋值：把右侧 gain_db 的结果保存到左侧 input_gain_。
    input_gain_ = gain_db;
    // 判断条件：dev_ != nullptr 是否成立。
    if (dev_ != nullptr)
    {
        // 赋值：把右侧 static_cast<esp_codec_dev_handle_t>(dev_) 的结果保存到左侧 auto* handle。
        auto* handle = static_cast<esp_codec_dev_handle_t>(dev_);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const esp_err_t err = esp_codec_dev_set_in_gain(handle, static_cast<float>(input_gain_));
        // 判断条件：err != ESP_OK 是否成立。
        if (err != ESP_OK)
        {
            // 调用 ESP_LOGW(TAG, "set_in_gain 失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGW(TAG, "set_in_gain 失败: %s", esp_err_to_name(err));
        }
    }
}

// 定义或声明函数 Es8311Codec::get_output_volume：参数在括号里，返回类型写在函数名前面。
uint8_t Es8311Codec::get_output_volume() const
{
    // 返回 output_volume_ 给调用者，表示当前函数的执行结果。
    return output_volume_;
}

// 定义或声明函数 Es8311Codec::get_input_gain：参数在括号里，返回类型写在函数名前面。
uint8_t Es8311Codec::get_input_gain() const
{
    // 返回 input_gain_ 给调用者，表示当前函数的执行结果。
    return input_gain_;
}

// 定义或声明函数 Es8311Codec::read：参数在括号里，返回类型写在函数名前面。
int Es8311Codec::read(int16_t* dest, int samples)
{
    // 判断条件：dest == nullptr || samples <= 0 || !initialized_ || !input_enabled_ || dev_ == nullptr 是否成立。
    if (dest == nullptr || samples <= 0 || !initialized_ || !input_enabled_ || dev_ == nullptr)
    {
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int bytes = static_cast<int>(static_cast<size_t>(samples) * sizeof(int16_t));
    // 只读约束：这个值或对象在当前位置不能被修改。
    const esp_err_t err =
        // 继续上一行的表达式或参数列表：esp_codec_dev_read(static_cast<esp_codec_dev_handle_t>(dev_), dest, bytes);。
        esp_codec_dev_read(static_cast<esp_codec_dev_handle_t>(dev_), dest, bytes);
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "read 失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "read 失败: %s", esp_err_to_name(err));
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    }
    // 返回 samples 给调用者，表示当前函数的执行结果。
    return samples;
}

// 定义或声明函数 Es8311Codec::write：参数在括号里，返回类型写在函数名前面。
int Es8311Codec::write(const int16_t* data, int samples)
{
    // 判断条件：data == nullptr || samples <= 0 || !initialized_ || !output_enabled_ || dev_ == nullptr 是否成立。
    if (data == nullptr || samples <= 0 || !initialized_ || !output_enabled_ || dev_ == nullptr)
    {
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int bytes = static_cast<int>(static_cast<size_t>(samples) * sizeof(int16_t));
    // 只读约束：这个值或对象在当前位置不能被修改。
    const esp_err_t err =
        // 继续上一行的表达式或参数列表：esp_codec_dev_write(static_cast<esp_codec_dev_handle_t>(dev_),。
        esp_codec_dev_write(static_cast<esp_codec_dev_handle_t>(dev_),
                            // 继续上一行的表达式或参数列表：const_cast<void*>(static_cast<const void*>(data)), bytes);。
                            const_cast<void*>(static_cast<const void*>(data)), bytes);
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "write 失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "write 失败: %s", esp_err_to_name(err));
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    }
    // 返回 samples 给调用者，表示当前函数的执行结果。
    return samples;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::bsp::driver::audio::es8311
