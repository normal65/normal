// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/bsp/config/config.hpp
 * 作用：集中定义开发板 GPIO、外设总线和硬件参数。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：引脚修改必须同时核对原理图、复用、总线和 Kconfig。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <driver/gpio.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <driver/i2c_master.h>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::config {
// i2c pins
const gpio_num_t I2C_SDA = GPIO_NUM_2;
// 只读约束：这个值或对象在当前位置不能被修改。
const gpio_num_t I2C_SCL = GPIO_NUM_1;

// es8311 (i2s)
const gpio_num_t I2S_MCLK = GPIO_NUM_41;
// 只读约束：这个值或对象在当前位置不能被修改。
const gpio_num_t I2S_SCLK = GPIO_NUM_40;
// 只读约束：这个值或对象在当前位置不能被修改。
const gpio_num_t I2S_LRCK = GPIO_NUM_39;
// 只读约束：这个值或对象在当前位置不能被修改。
const gpio_num_t I2S_DI = GPIO_NUM_42;
// 只读约束：这个值或对象在当前位置不能被修改。
const gpio_num_t I2S_DO = GPIO_NUM_38;
// 只读约束：这个值或对象在当前位置不能被修改。
const gpio_num_t I2S_PA = GPIO_NUM_7;

// battery
const gpio_num_t BATTERY_VBAT = GPIO_NUM_6;
// ADC 两点标定：满电时实测约 4170mV
constexpr uint32_t BATTERY_FULL_MV = 4170;
// 固定参数 BATTERY_EMPTY_MV = 3000：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t BATTERY_EMPTY_MV = 3000;

// gdey027t91 pins
const gpio_num_t GDEY027T91_MOSI = GPIO_NUM_3;
// 只读约束：这个值或对象在当前位置不能被修改。
const gpio_num_t GDEY027T91_MISO = GPIO_NUM_NC; // not used
// 只读约束：这个值或对象在当前位置不能被修改。
const gpio_num_t GDEY027T91_SCLK = GPIO_NUM_46;
// 只读约束：这个值或对象在当前位置不能被修改。
const gpio_num_t GDEY027T91_CS = GPIO_NUM_9;
// 只读约束：这个值或对象在当前位置不能被修改。
const gpio_num_t GDEY027T91_DC = GPIO_NUM_10;
// 只读约束：这个值或对象在当前位置不能被修改。
const gpio_num_t GDEY027T91_RST = GPIO_NUM_11;
// 只读约束：这个值或对象在当前位置不能被修改。
const gpio_num_t GDEY027T91_BUSY = GPIO_NUM_12;
// 只读约束：这个值或对象在当前位置不能被修改。
const gpio_num_t GDEY027T91_Backlight = GPIO_NUM_17; // 背光，通过pwm驱动

// FT6336
const gpio_num_t FT6336_RST = GPIO_NUM_18;
// 只读约束：这个值或对象在当前位置不能被修改。
const gpio_num_t FT6336_INT = GPIO_NUM_5;
// 固定参数 TOUCH_SWAP_XY = false：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr bool TOUCH_SWAP_XY = false;
// 固定参数 TOUCH_MIRROR_X = false：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr bool TOUCH_MIRROR_X = false;
// 固定参数 TOUCH_MIRROR_Y = false：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr bool TOUCH_MIRROR_Y = false;

// tf-card pins
const gpio_num_t TF_CARD_D0 = GPIO_NUM_14;
// 只读约束：这个值或对象在当前位置不能被修改。
const gpio_num_t TF_CARD_D1 = GPIO_NUM_13;
// 只读约束：这个值或对象在当前位置不能被修改。
const gpio_num_t TF_CARD_D2 = GPIO_NUM_45;
// 只读约束：这个值或对象在当前位置不能被修改。
const gpio_num_t TF_CARD_D3 = GPIO_NUM_48;
// 只读约束：这个值或对象在当前位置不能被修改。
const gpio_num_t TF_CARD_CLK = GPIO_NUM_21;
// 只读约束：这个值或对象在当前位置不能被修改。
const gpio_num_t TF_CARD_CMD = GPIO_NUM_47;
// 只读约束：这个值或对象在当前位置不能被修改。
const gpio_num_t TF_CARD_CD = GPIO_NUM_4;

// ADC 拨码 — GPIO15；实测 up=551mV mid=1102mV down=1651mV 未按=3300mV
const gpio_num_t DIP_SWITCH = GPIO_NUM_15;
// 阈值：超过或低于 830 时触发对应逻辑。
constexpr uint32_t DIP_SWITCH_THRESHOLD_UP_MID_MV = 830;    // (551 + 1102) / 2
// 阈值：超过或低于 1380 时触发对应逻辑。
constexpr uint32_t DIP_SWITCH_THRESHOLD_MID_DOWN_MV = 1380; // (1102 + 1651) / 2
// 阈值：超过或低于 2100 时触发对应逻辑。
constexpr uint32_t DIP_SWITCH_THRESHOLD_RELEASED_MV = 2100; // 高于 down，低于未按

// 命名空间结束：离开当前模块作用域。
} // namespace app::config
