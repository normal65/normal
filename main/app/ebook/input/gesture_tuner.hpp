#pragma once

/**
 * 文件：main/app/ebook/input/gesture_tuner.hpp
 *
 * 手势参数的"预设档位"。
 *
 * 不同页面需要不同的触摸手感：
 *   - Normal   ：日常浏览，点击和滑动都要正常。
 *   - Keyboard ：软键盘，要"点得快"，所以冷却时间调短。
 *   - Drawing  ：画板，需要连续的按下/移动/抬起轨迹，
 *                所以关掉滑动和长按，改成上报原始触点流。
 *
 * make_config(Profile) 返回一份填好阈值的 GestureConfig，
 * 由 InputRouter::set_profile() 在切换页面时调用。
 */

#include <cstdint>

#include "input/input_event.hpp"

namespace app::ebook::input {

enum class Profile : uint8_t
{
    Normal = 0,  // 默认：点击 + 滑动 + 长按
    Keyboard,    // 键盘：缩短冷却，点击更跟手
    Drawing,     // 画板：上报连续轨迹，禁用滑动/长按
};

// 用来"禁用"某个手势的哨兵值：
// 把滑动距离阈值设成 UINT16_MAX，就永远达不到，等于关掉滑动。
inline constexpr uint16_t kSwipeDisabledPx = UINT16_MAX;
inline constexpr uint32_t kLongPressDisabledMs = UINT32_MAX;

// 根据档位生成一份手势配置。
GestureConfig make_config(Profile p);

} // namespace app::ebook::input
