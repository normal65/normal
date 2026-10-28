/**
 * 文件：main/app/ebook/input/gesture_tuner.cc
 *
 * 各档位手势参数的具体数值。想微调手感就改这里。
 */

#include "input/gesture_tuner.hpp"

#include "core/geometry.hpp"

namespace app::ebook::input {

GestureConfig make_config(Profile p)
{
    GestureConfig cfg{};

    // 屏幕尺寸从统一的几何常量取，避免写死数字后换屏忘记改。
    cfg.panel_width = core::kScreenW;
    cfg.panel_height = core::kScreenH;

    // 键盘：缩短手势冷却，让连续快速点按不被"冷却"吃掉。
    if (p == Profile::Keyboard)
        cfg.gesture_cooldown_ms = 30;

    // 画板：把原始轨迹全报上来，同时关掉滑动和长按，
    // 否则画一条线会被识别成滑动、按住不动会触发长按。
    if (p == Profile::Drawing)
    {
        cfg.emit_press = true;
        cfg.emit_move = true;
        cfg.emit_release = true;
        cfg.swipe_min_distance_px = kSwipeDisabledPx;
        cfg.long_press_ms = kLongPressDisabledMs;
        cfg.gesture_cooldown_ms = 0;
    }

    return cfg;
}

} // namespace app::ebook::input
