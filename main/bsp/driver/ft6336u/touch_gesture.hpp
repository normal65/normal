#pragma once

/**
 * 文件：main/bsp/driver/ft6336u/touch_gesture.hpp
 *
 * TouchGesture 是一个"手势状态机"。
 * 输入：一次次触摸采样（每次带 0~2 个触点坐标）。
 * 输出：Tap（点击）、LongPress（长按）、SwipeXxx（滑动）等语义事件。
 *
 * 为什么需要它？因为触摸芯片只给你坐标，不知道用户"想干什么"。
 * 状态机负责看一串坐标：起点在哪、移动多远、按住多久，
 * 然后判断这到底是点击还是滑动。
 *
 * 状态流转（想理解代码可以先记这个）：
 *   Idle --按下--> Pressed --超时/位移不够--> 继续 Pressed
 *                        |--到达长按时长--> LongPressSent（发长按）
 *   任意 --抬手--> 结束时根据时长和位移发 Tap / Swipe / 什么都不发 --> Idle
 */

#include <cstdint>

#include "ft6336u.hpp"

namespace app::bsp::driver::ft6336u {

// 手势类型。None 表示"没有事件"。
enum class GestureType : uint8_t
{
    None = 0,
    Press,       // 手指刚按下（默认不发，由 emit_press 控制）
    Move,        // 按住移动（默认不发，由 emit_move 控制）
    Release,     // 松手（默认不发，由 emit_release 控制）
    Tap,         // 点击
    LongPress,   // 长按
    SwipeUp,
    SwipeDown,
    SwipeLeft,
    SwipeRight,
};

// 状态机吐出来的一条事件。纯数据，可以直接塞进队列。
struct GestureEvent
{
    GestureType type = GestureType::None;
    uint16_t x = 0;            // 事件发生时手指的位置
    uint16_t y = 0;
    uint16_t start_x = 0;      // 本次按下的起点
    uint16_t start_y = 0;
    int16_t dx = 0;            // 相对起点的位移（可正可负）
    int16_t dy = 0;
    uint32_t duration_ms = 0;  // 从按下到现在的时长
};

/**
 * 手势判定阈值。调这里就能改"多短算点击、多快算滑动"。
 *
 * 这些数字不是随便写的：墨水屏刷新慢，用户点击的"手感"和其它设备不同，
 * 例如长按直接触发局部刷新会很卡，所以长按阈值设得比较保守。
 */
struct GestureConfig
{
    // 屏幕尺寸（像素）。坐标系用这个做边界裁剪。
    uint16_t panel_width = 176;
    uint16_t panel_height = 264;

    // 坐标偏移 / 翻转。硬件如果装反了，靠这几个开关纠正。
    int16_t map_offset_x = 0;
    int16_t map_offset_y = 0;
    bool swap_xy = false;
    bool mirror_x = false;
    bool mirror_y = false;

    // 是否把"按下 / 移动 / 松手"这种中间事件也发出去。
    // 默认关：只发最终的手势（Tap / Swipe / LongPress），更省事。
    bool emit_press = false;
    bool emit_move = false;
    bool emit_release = false;

    // ---- 点击判定 ----
    uint16_t tap_max_move_px = 15;      // 抬起前最多允许移动多少像素
    uint32_t tap_max_duration_ms = 250; // 按多久以内算点击

    // ---- 长按判定 ----
    uint32_t long_press_ms = 700;          // 按住多久触发长按
    uint8_t long_press_stable_frames = 4;  // 长按期间位置要保持稳定这么多帧

    // ---- 滑动判定 ----
    uint16_t swipe_min_distance_px = 28;   // 至少要滑这么远
    uint16_t swipe_axis_lock_px = 16;      // 主方向至少要领先这么多，避免斜着误判
    uint32_t swipe_max_duration_ms = 800;  // 超过这个时长不算滑动（算拖动）
    uint32_t gesture_cooldown_ms = 150;    // 两次手势之间的最小间隔

    // ---- 抗噪 ----
    uint16_t max_frame_jump_px = 60;  // 两帧之间跳这么远，判定为噪声，丢弃
    uint8_t edge_ignore_px = 0;       // 忽略屏幕边缘多少像素（0=不忽略）
};

/**
 * 触摸采样 -> 手势事件。
 *
 * 典型用法（见 input_router.cc）：
 *   gesture.init(touch, &config);   // 绑定设备和配置
 *   while (running) {
 *       gesture.poll();             // 读一次触摸、推进状态机
 *       while (gesture.has_event()) {
 *           auto ev = gesture.take_event();
 *           // ... 把 ev 发给 UI
 *       }
 *   }
 */
class TouchGesture
{
  public:
    TouchGesture();
    ~TouchGesture() = default;

    TouchGesture(const TouchGesture&) = delete;
    TouchGesture& operator=(const TouchGesture&) = delete;

    // 绑定触摸设备并应用配置，同时清空状态。config 传 nullptr 则沿用当前配置。
    bool init(Ft6336u* touch, const GestureConfig* config = nullptr);
    void reset();

    // 读一次触摸并推进状态机。返回 true 表示这次产生了事件。
    bool poll();

    // 直接喂一个采样给状态机（例如手指已离开时喂空采样）。
    bool process_sample(const TouchSample& sample);

    // 没有新采样时也可以调用：用来推进时间，判断长按或超时抬手。
    void tick();

    // 是否有还没被拿走的事件。
    bool has_event() const
    {
        return pending_.type != GestureType::None;
    }

    // 取走事件（取走后 has_event 变 false）。
    GestureEvent take_event();

    // 状态机是否认为手指还按着。
    bool is_pressed() const
    {
        return state_ == State::Pressed || state_ == State::LongPressSent;
    }

    // 最近一次映射、裁剪后的坐标。
    const TouchPoint& mapped_point() const
    {
        return mapped_;
    }

  private:
    // 状态机的三个状态。
    enum class State : uint8_t
    {
        Idle = 0,        // 没手指
        Pressed,         // 手指按着，还没判定
        LongPressSent,   // 已经发过长按，等抬手
    };

    // 应用偏移和裁剪。这里不做 swap/mirror，因为 Ft6336u 驱动已经做过硬件坐标校正；
    // 两个地方都翻转会把坐标绕晕，所以职责分开。
    void map_point(uint16_t raw_x, uint16_t raw_y, TouchPoint& out) const;

    // 生成一条事件。若是最终手势，顺便记录时间，用于冷却判断。
    void push_event(GestureType type, uint16_t x, uint16_t y, uint16_t sx, uint16_t sy, int16_t dx,
                    int16_t dy, uint32_t duration_ms);

    // 判断滑动方向：先看横向还是纵向位移更大，再看是否超过阈值和轴锁定。
    GestureType detect_swipe(int16_t dx, int16_t dy) const;

    // 抬手时调用：根据时长和位移决定发 Tap / Swipe，还是什么都不发。
    // 同时处理手势冷却，防止一次抬手被识别成多次操作。
    void finish_contact(uint32_t now_ms);

    // 按住时间够长且位置稳定时，发 LongPress。
    bool try_long_press(uint32_t now_ms);

    // 过滤跳变噪声：两帧坐标差太远就丢弃这一点。
    bool accept_position(uint16_t x, uint16_t y) const;

    // 询问驱动：现在还有手指吗？
    bool finger_up() const;

    // 单调递增的毫秒时间。用它算"按住多久"，不要用会跳变的墙上时钟。
    uint32_t now_ms() const;

    Ft6336u* touch_;              // 不拥有，只借用
    GestureConfig cfg_;
    State state_;
    GestureEvent pending_;        // 待取走的事件

    TouchPoint mapped_;           // 最近一次映射后的坐标
    uint16_t start_x_;            // 本次按下起点
    uint16_t start_y_;
    uint16_t last_x_;             // 上一次采样位置
    uint16_t last_y_;
    uint32_t press_start_ms_;     // 按下时刻
    uint32_t last_gesture_ms_;    // 上一个最终手势的时刻（冷却用）
    bool long_press_fired_;       // 本次按住是否已经发过长按
    uint8_t stable_frames_;       // 位置连续稳定的帧数
};

} // namespace app::bsp::driver::ft6336u
