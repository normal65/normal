#pragma once

/**
 * 文件：main/app/ebook/input/input_event.hpp
 *
 * 输入事件的小工具层。这里不产生事件，只做三件事：
 *   1. 给项目其他部分起几个好写的别名（Event / EventType / GestureConfig）。
 *   2. 把事件类型转成可读字符串，方便打日志。
 *   3. 判断某个事件是不是滑动 / 是不是类点击。
 *
 * 事件本体定义在 bsp/driver/ft6336u/touch_gesture.hpp 的 GestureEvent。
 * 这里直接复用，不重新定义，避免两个模块的事件结构对不上。
 */

#include "ft6336u.hpp"
#include "touch_gesture.hpp"

namespace app::ebook::input {

// 直接复用 BSP 里的事件类型。GestureEvent 是 POD（纯数据），
// 可以安全地按值复制进 FreeRTOS 队列。
using Event = ::app::bsp::driver::ft6336u::GestureEvent;
using EventType = ::app::bsp::driver::ft6336u::GestureType;
using GestureConfig = ::app::bsp::driver::ft6336u::GestureConfig;

// 打印日志用：把枚举转成人类能读的字符串。
inline const char* type_name(EventType t)
{
    switch (t)
    {
        case EventType::None:
            return "None";
        case EventType::Press:
            return "Press";
        case EventType::Move:
            return "Move";
        case EventType::Release:
            return "Release";
        case EventType::Tap:
            return "Tap";
        case EventType::LongPress:
            return "LongPress";
        case EventType::SwipeUp:
            return "SwipeUp";
        case EventType::SwipeDown:
            return "SwipeDown";
        case EventType::SwipeLeft:
            return "SwipeLeft";
        case EventType::SwipeRight:
            return "SwipeRight";
    }
    return "?";
}

// 四个方向的滑动，都算"滑动"。
inline bool is_swipe(EventType t)
{
    return t == EventType::SwipeUp || t == EventType::SwipeDown || t == EventType::SwipeLeft ||
           t == EventType::SwipeRight;
}

// 点击和长按都来自"原地按一下"，在很多页面里走同一套处理逻辑，
// 所以合并成一个概念一起判断。
inline bool is_tap_like(EventType t)
{
    return t == EventType::Tap || t == EventType::LongPress;
}

} // namespace app::ebook::input
