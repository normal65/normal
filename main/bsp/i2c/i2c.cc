/**
 * [初学者导读] 文件：main/bsp/i2c/i2c.cc
 * 作用：初始化和管理 I2C 总线，为触摸、传感器等设备提供通信。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：总线访问可能失败或超时，调用者不能假设设备一定在线。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "i2c.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_log.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "freertos/FreeRTOS.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "freertos/task.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <driver/i2c_master.h>

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "I2C";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 定义变量 i2c_master_bus_handle_t g_bus_handle 并初始化为 nullptr。
i2c_master_bus_handle_t g_bus_handle = nullptr;
// 定义变量 int g_bus_ref_count 并初始化为 0。
int g_bus_ref_count = 0;

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::bsp::i2c {
// 定义或声明函数 I2C::I2C：参数在括号里，返回类型写在函数名前面。
I2C::I2C()
    // 定义或声明函数 bus_handle_：参数在括号里，返回类型写在函数名前面。
    : bus_handle_(nullptr)
    // 定义或声明函数 initialized_：参数在括号里，返回类型写在函数名前面。
    , initialized_(false)
{
}

// 定义或声明函数 I2C::~I2C：参数在括号里，返回类型写在函数名前面。
I2C::~I2C()
{
    // 调用 deinit()：创建和销毁队列。
    deinit();
}

// 定义或声明函数 I2C::init：参数在括号里，返回类型写在函数名前面。
bool I2C::init(const Config* config)
{
    // 判断条件：initialized_ 是否成立。
    if (initialized_)
    {
        // 调用 ESP_LOGW(TAG, "I2C 实例已初始化");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGW(TAG, "I2C 实例已初始化");
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 判断条件：config != nullptr 是否成立。
    if (config != nullptr)
    {
        // 赋值：把右侧 *config 的结果保存到左侧 config_。
        config_ = *config;
    }

    // 判断条件：g_bus_handle != nullptr 是否成立。
    if (g_bus_handle != nullptr)
    {
        // 赋值：把右侧 g_bus_handle 的结果保存到左侧 bus_handle_。
        bus_handle_ = g_bus_handle;
        // 赋值：把右侧 true 的结果保存到左侧 initialized_。
        initialized_ = true;
        // 执行语句：g_bus_ref_count++;；它会更新一次变量、对象或程序状态。
        g_bus_ref_count++;
        // 调用 ESP_LOGD(TAG, "复用已有 I2C bus，引用计数=%d", g_bus_ref_count);，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGD(TAG, "复用已有 I2C bus，引用计数=%d", g_bus_ref_count);
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 配置 I2C master bus
    i2c_master_bus_config_t i2c_bus_cfg = {};
    // 赋值：把右侧 config_.port 的结果保存到左侧 i2c_bus_cfg.i2c_port。
    i2c_bus_cfg.i2c_port = config_.port;
    // 赋值：把右侧 config_.sda_pin 的结果保存到左侧 i2c_bus_cfg.sda_io_num。
    i2c_bus_cfg.sda_io_num = config_.sda_pin;
    // 赋值：把右侧 config_.scl_pin 的结果保存到左侧 i2c_bus_cfg.scl_io_num。
    i2c_bus_cfg.scl_io_num = config_.scl_pin;
    // 赋值：把右侧 I2C_CLK_SRC_DEFAULT 的结果保存到左侧 i2c_bus_cfg.clk_source。
    i2c_bus_cfg.clk_source = I2C_CLK_SRC_DEFAULT;
    // 赋值：把右侧 7 的结果保存到左侧 i2c_bus_cfg.glitch_ignore_cnt。
    i2c_bus_cfg.glitch_ignore_cnt = 7;
    // 赋值：把右侧 0 的结果保存到左侧 i2c_bus_cfg.intr_priority。
    i2c_bus_cfg.intr_priority = 0;
    // 赋值：把右侧 0 的结果保存到左侧 i2c_bus_cfg.trans_queue_depth。
    i2c_bus_cfg.trans_queue_depth = 0;
    // 赋值：把右侧 config_.enable_internal_pullup 的结果保存到左侧 i2c_bus_cfg.flags.enable_internal_pullup。
    i2c_bus_cfg.flags.enable_internal_pullup = config_.enable_internal_pullup;
    // 赋值：把右侧 false 的结果保存到左侧 i2c_bus_cfg.flags.allow_pd。
    i2c_bus_cfg.flags.allow_pd = false;

    // 创建 I2C master bus
    esp_err_t ret = i2c_new_master_bus(&i2c_bus_cfg, &bus_handle_);
    // 判断条件：ret != ESP_OK 是否成立。
    if (ret != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "创建 I2C master bus 失败: %s", esp_err_to_name(ret));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "创建 I2C master bus 失败: %s", esp_err_to_name(ret));
        // 赋值：把右侧 nullptr 的结果保存到左侧 bus_handle_。
        bus_handle_ = nullptr;
        // 赋值：把右侧 false 的结果保存到左侧 initialized_。
        initialized_ = false;
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 bus_handle_ 的结果保存到左侧 g_bus_handle。
    g_bus_handle = bus_handle_;
    // 赋值：把右侧 1 的结果保存到左侧 g_bus_ref_count。
    g_bus_ref_count = 1;
    // 赋值：把右侧 true 的结果保存到左侧 initialized_。
    initialized_ = true;
    // 调用 ESP_LOGD(TAG, "I2C master bus 初始化成功: 端口=%d, sda=%d, scl=%d", config_.port,，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "I2C master bus 初始化成功: 端口=%d, sda=%d, scl=%d", config_.port,
             // 继续上一行的表达式或参数列表：config_.sda_pin, config_.scl_pin);。
             config_.sda_pin, config_.scl_pin);

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 I2C::scan：参数在括号里，返回类型写在函数名前面。
int I2C::scan(uint32_t timeout_ms)
{
    // 判断条件：!initialized_ || bus_handle_ == nullptr 是否成立。
    if (!initialized_ || bus_handle_ == nullptr)
    {
        // 调用 ESP_LOGE(TAG, "I2C 未初始化，请先调用 init()");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "I2C 未初始化，请先调用 init()");
        // 返回 -1 给调用者，表示当前函数的执行结果。
        return -1;
    }

    // 调用 ESP_LOGD(TAG, "正在扫描 I2C 总线...");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "正在扫描 I2C 总线...");
    // 调用 printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");，执行这个函数并处理它的返回值或状态变化。
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");

    // 定义变量 int device_count 并初始化为 0。
    int device_count = 0;
    // 定义变量 TickType_t timeout_ticks 并初始化为 pdMS_TO_TICKS(timeout_ms)。
    TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);

    // 扫描所有 I2C 地址 (0x00 - 0x7F)
    for (int i = 0; i < 128; i += 16)
    {
        // 调用 printf("%02x: ", i);，执行这个函数并处理它的返回值或状态变化。
        printf("%02x: ", i);
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (int j = 0; j < 16; j++)
        {
            // 定义变量 uint8_t address 并初始化为 i + j。
            uint8_t address = i + j;

            // 调用 fflush(stdout);，执行这个函数并处理它的返回值或状态变化。
            fflush(stdout);

            // 探测设备
            esp_err_t ret = i2c_master_probe(bus_handle_, address, timeout_ticks);
            // 判断条件：ret == ESP_OK 是否成立。
            if (ret == ESP_OK)
            {
                // 调用 printf("%02x ", address);，执行这个函数并处理它的返回值或状态变化。
                printf("%02x ", address);
                // 执行语句：device_count++;；它会更新一次变量、对象或程序状态。
                device_count++;
            }
            // 备用分支：前面的条件不成立时执行这里。
            else if (ret == ESP_ERR_TIMEOUT)
            {
                // 调用 printf("UU ");，执行这个函数并处理它的返回值或状态变化。
                printf("UU "); // 超时（可能是总线问题）
            }
            // 备用分支：前面的条件不成立时执行这里。
            else
            {
                // 调用 printf("-- ");，执行这个函数并处理它的返回值或状态变化。
                printf("-- "); // 无设备
            }
        }
        // 调用 printf("\r\n");，执行这个函数并处理它的返回值或状态变化。
        printf("\r\n");
    }

    // 判断条件：device_count > 0 是否成立。
    if (device_count > 0)
    {
        // 调用 ESP_LOGD(TAG, "找到 %d 个 I2C 设备", device_count);，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGD(TAG, "找到 %d 个 I2C 设备", device_count);
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 调用 ESP_LOGW(TAG, "未找到 I2C 设备");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGW(TAG, "未找到 I2C 设备");
    }

    // 返回 device_count 给调用者，表示当前函数的执行结果。
    return device_count;
}

// 定义或声明函数 I2C::deinit：参数在括号里，返回类型写在函数名前面。
void I2C::deinit()
{
    // 判断条件：!initialized_ 是否成立。
    if (!initialized_)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 赋值：把右侧 false 的结果保存到左侧 initialized_。
    initialized_ = false;
    // 赋值：把右侧 nullptr 的结果保存到左侧 bus_handle_。
    bus_handle_ = nullptr;
    // 执行语句：g_bus_ref_count--;；它会更新一次变量、对象或程序状态。
    g_bus_ref_count--;

    // 判断条件：g_bus_ref_count > 0 是否成立。
    if (g_bus_ref_count > 0)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：g_bus_handle != nullptr 是否成立。
    if (g_bus_handle != nullptr)
    {
        // 定义变量 esp_err_t ret 并初始化为 i2c_del_master_bus(g_bus_handle)。
        esp_err_t ret = i2c_del_master_bus(g_bus_handle);
        // 判断条件：ret != ESP_OK 是否成立。
        if (ret != ESP_OK)
        {
            // 调用 ESP_LOGE(TAG, "删除 I2C master bus 失败: %s", esp_err_to_name(ret));，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "删除 I2C master bus 失败: %s", esp_err_to_name(ret));
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 调用 ESP_LOGD(TAG, "I2C master bus 已反初始化");，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGD(TAG, "I2C master bus 已反初始化");
        }
        // 赋值：把右侧 nullptr 的结果保存到左侧 g_bus_handle。
        g_bus_handle = nullptr;
    }
}
// 命名空间结束：离开当前模块作用域。
} // namespace app::bsp::i2c
