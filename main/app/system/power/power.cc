/**
 * [初学者导读] 文件：main/app/system/power/power.cc
 * 作用：管理轻睡眠、深睡眠和唤醒后的恢复。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：进入低功耗前要确认外设和内存状态，唤醒后需要重新校准。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "power.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_log.h"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "Power";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::sys::power {

// 定义或声明函数 PowerMgr::get_instance：参数在括号里，返回类型写在函数名前面。
PowerMgr& PowerMgr::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static PowerMgr instance;
    // 返回 instance 给调用者，表示当前函数的执行结果。
    return instance;
}

// 函数定义/声明：调用 PowerMgr::set_exit_low_power_cb()：保存一个退出轻睡眠后的回调。。
void PowerMgr::set_exit_low_power_cb(ExitLowPowerCallback callback)
{
    // 赋值：把右侧 callback 的结果保存到左侧 exit_callback_。
    exit_callback_ = callback;
}

// [初学者导读] PowerMgr::enter_light_sleep()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void PowerMgr::enter_light_sleep()
{
    // 调用 ESP_LOGI(TAG, "进入 Light Sleep");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGI(TAG, "进入 Light Sleep");
    // 调用 esp_light_sleep_start();，执行这个函数并处理它的返回值或状态变化。
    esp_light_sleep_start();
    // 判断条件：exit_callback_ 是否成立。
    if (exit_callback_)
    {
        // 调用 exit_callback_();，执行这个函数并处理它的返回值或状态变化。
        exit_callback_();
    }
}

// [初学者导读] PowerMgr::enter_deep_sleep()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void PowerMgr::enter_deep_sleep()
{
    // 调用 ESP_LOGI(TAG, "进入 Deep Sleep");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGI(TAG, "进入 Deep Sleep");
    // 调用 esp_deep_sleep_start();，执行这个函数并处理它的返回值或状态变化。
    esp_deep_sleep_start();
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::sys::power
