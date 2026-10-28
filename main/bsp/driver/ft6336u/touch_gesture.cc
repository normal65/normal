/**
 * 文件：main/bsp/driver/ft6336u/touch_gesture.cc
 *
 * 手势状态机的实现。看代码时把握三条主线：
 *
 *   1. process_sample() 是核心：每收到一个触摸采样就走一遍。
 *      刚按下 -> 记录起点；按住 -> 累计稳定帧、判断长按；抬手 -> 结算成 Tap/Swipe。
 *
 *   2. tick() 处理"没有新采样时的时间流逝"。例如用户按住不动，
 *      这时没有新数据，需要靠 tick 推进长按计时。
 *
 *   3. finish_contact() 是所有手势的"结算点"。不管点击还是滑动，
 *      都在抬手这一刻统一判断。
 */

#include "touch_gesture.hpp"

#include <algorithm>
#include <cstdlib>

#include <esp_timer.h>

namespace app::bsp::driver::ft6336u {

namespace {

// 最终手势：会"消费"一次操作，需要参与冷却计时。
// Press / Move / Release 只是过程事件，不算。
bool is_final_gesture(GestureType type)
{
    switch (type)
    {
        case GestureType::Tap:
        case GestureType::LongPress:
        case GestureType::SwipeUp:
        case GestureType::SwipeDown:
        case GestureType::SwipeLeft:
        case GestureType::SwipeRight:
            return true;
        default:
            return false;
    }
}

} // namespace

TouchGesture::TouchGesture()
    : touch_(nullptr)
    , state_(State::Idle)
    , start_x_(0)
    , start_y_(0)
    , last_x_(0)
    , last_y_(0)
    , press_start_ms_(0)
    , last_gesture_ms_(0)
    , long_press_fired_(false)
    , stable_frames_(0)
{
}

// 绑定设备 + 应用配置 + 清空状态。
bool TouchGesture::init(Ft6336u* touch, const GestureConfig* config)
{
    if (touch == nullptr || !touch->is_init())
        return false;

    touch_ = touch;
    if (config != nullptr)
        cfg_ = *config;

    reset();
    return true;
}

// 回到"没手指"的初始状态，丢掉所有中间变量和未取走的事件。
void TouchGesture::reset()
{
    state_ = State::Idle;
    pending_ = {};
    mapped_ = {};
    start_x_ = 0;
    start_y_ = 0;
    last_x_ = 0;
    last_y_ = 0;
    press_start_ms_ = 0;
    long_press_fired_ = false;
    stable_frames_ = 0;
}

// 单调时钟，单位毫秒。用 esp_timer 而不是系统时间，
// 因为系统时间可能被 NTP 校正跳变，会把"按住时长"算成负数。
uint32_t TouchGesture::now_ms() const
{
    return static_cast<uint32_t>(esp_timer_get_time() / 1000);
}

// 把驱动给的原始坐标加上偏移并裁到屏幕范围内。
// swap/mirror 不在这里做：驱动已经做过一次，做两遍会翻回来。
void TouchGesture::map_point(uint16_t raw_x, uint16_t raw_y, TouchPoint& out) const
{
    int32_t x = static_cast<int32_t>(raw_x) + cfg_.map_offset_x;
    int32_t y = static_cast<int32_t>(raw_y) + cfg_.map_offset_y;

    // 裁剪：保证坐标一定落在 [0, 宽/高-1]，否则后面的矩形命中判断会出错。
    x = std::max<int32_t>(0, std::min<int32_t>(x, cfg_.panel_width - 1));
    y = std::max<int32_t>(0, std::min<int32_t>(y, cfg_.panel_height - 1));

    out.x = static_cast<uint16_t>(x);
    out.y = static_cast<uint16_t>(y);
}

// 把一条手势写进 pending_，等待外面 take_event() 取走。
// 注意这里只存一条：如果事件还没被取走就又来一条，会覆盖旧的。
// 之所以敢这样，是因为调用方（InputRouter）每次推进后都会立刻 drain。
void TouchGesture::push_event(GestureType type, uint16_t x, uint16_t y, uint16_t sx, uint16_t sy,
                              int16_t dx, int16_t dy, uint32_t duration_ms)
{
    pending_ = {type, x, y, sx, sy, dx, dy, duration_ms};

    if (is_final_gesture(type))
        last_gesture_ms_ = now_ms();
}

// 取走事件，并清空 pending_。取走后 has_event() 返回 false。
GestureEvent TouchGesture::take_event()
{
    const GestureEvent ev = pending_;
    pending_ = {};
    return ev;
}

// 问驱动：现在还有手指吗？touch_ 为空时也当作"已离开"，
// 这样即使驱动掉了，状态机也能正常收尾。
bool TouchGesture::finger_up() const
{
    return touch_ == nullptr || !touch_->is_touched();
}

// 噪声过滤。
//   - 边缘过滤：如果配置要求忽略屏幕边缘，落在边缘的点直接丢。
//   - 跳变过滤：手指不可能在一帧内瞬移很远，超过 max_frame_jump_px 的点当噪声丢弃。
// 这能避免 I2C 读错一两字节导致的手势乱跳。
bool TouchGesture::accept_position(uint16_t x, uint16_t y) const
{
    if (cfg_.edge_ignore_px > 0)
    {
        const uint16_t max_x = static_cast<uint16_t>(cfg_.panel_width - 1);
        const uint16_t max_y = static_cast<uint16_t>(cfg_.panel_height - 1);
        if (x <= cfg_.edge_ignore_px || x >= static_cast<uint16_t>(max_x - cfg_.edge_ignore_px) ||
            y <= cfg_.edge_ignore_px || y >= static_cast<uint16_t>(max_y - cfg_.edge_ignore_px))
        {
            return false;
        }
    }

    if (state_ == State::Idle)
        return true;  // 还没建立参考点，第一帧永远接受

    const int32_t dx = std::abs(static_cast<int32_t>(x) - static_cast<int32_t>(last_x_));
    const int32_t dy = std::abs(static_cast<int32_t>(y) - static_cast<int32_t>(last_y_));
    if (dx + dy > static_cast<int32_t>(cfg_.max_frame_jump_px))
        return false;

    return true;
}

// 判断滑动方向。
// 步骤：
//   1. 总位移太小 -> 不是滑动。
//   2. 哪个方向的位移更大，就以哪个方向为主（轴锁定），
//      这样斜着划也不会一会儿横一会儿竖。
//   3. 主方向位移超过 swipe_axis_lock_px 才认，否则当作抖动。
GestureType TouchGesture::detect_swipe(int16_t dx, int16_t dy) const
{
    const uint16_t adx = static_cast<uint16_t>(std::abs(dx));
    const uint16_t ady = static_cast<uint16_t>(std::abs(dy));

    if (adx < cfg_.swipe_min_distance_px && ady < cfg_.swipe_min_distance_px)
        return GestureType::None;

    if (adx >= ady)
    {
        if (adx < cfg_.swipe_axis_lock_px)
            return GestureType::None;
        // dx>0 表示手指往右走，屏幕内容是"右滑"。
        return (dx > 0) ? GestureType::SwipeRight : GestureType::SwipeLeft;
    }

    if (ady < cfg_.swipe_axis_lock_px)
        return GestureType::None;
    // dy>0 表示手指往下走。
    return (dy > 0) ? GestureType::SwipeDown : GestureType::SwipeUp;
}

// 抬手结算：所有手势最终都在这里决定。
// 逻辑顺序：
//   1. 如果刚发过长按，或者还在冷却期，就不重复发。
//   2. 先试滑动（位移够大就是滑动）。
//   3. 不是滑动，再看是不是点击（位移小 + 时间短）。
//   4. 都不是就不发（除非配置要求发 Release）。
void TouchGesture::finish_contact(uint32_t t)
{
    const uint32_t duration = t - press_start_ms_;
    const int16_t dx = static_cast<int16_t>(last_x_ - start_x_);
    const int16_t dy = static_cast<int16_t>(last_y_ - start_y_);
    const int16_t adx = static_cast<int16_t>(std::abs(dx));
    const int16_t ady = static_cast<int16_t>(std::abs(dy));

    GestureType type = GestureType::None;

    if (!long_press_fired_ && (t - last_gesture_ms_) >= cfg_.gesture_cooldown_ms)
    {
        // 时间太长就不当滑动（可能是在慢慢拖动）。
        if (duration <= cfg_.swipe_max_duration_ms)
            type = detect_swipe(dx, dy);

        // 位移小且时间短 -> 点击。
        if (type == GestureType::None && adx <= static_cast<int16_t>(cfg_.tap_max_move_px) &&
            ady <= static_cast<int16_t>(cfg_.tap_max_move_px) &&
            duration <= cfg_.tap_max_duration_ms)
        {
            type = GestureType::Tap;
        }
    }

    if (type != GestureType::None)
        push_event(type, last_x_, last_y_, start_x_, start_y_, dx, dy, duration);
    else if (cfg_.emit_release)
        push_event(GestureType::Release, last_x_, last_y_, start_x_, start_y_, dx, dy, duration);

    state_ = State::Idle;
    long_press_fired_ = false;
}

// 长按判定。
// 必须同时满足：按住够久、位移够小、位置连续稳定够多帧。
// "稳定帧数"这个条件是为了防止手指在屏幕上抖动时误触发长按。
bool TouchGesture::try_long_press(uint32_t t)
{
    if (long_press_fired_ || state_ != State::Pressed)
        return false;

    const int16_t dx = static_cast<int16_t>(last_x_ - start_x_);
    const int16_t dy = static_cast<int16_t>(last_y_ - start_y_);
    const int16_t adx = static_cast<int16_t>(std::abs(dx));
    const int16_t ady = static_cast<int16_t>(std::abs(dy));
    const uint32_t held = t - press_start_ms_;

    if (held < cfg_.long_press_ms || adx > static_cast<int16_t>(cfg_.tap_max_move_px) ||
        ady > static_cast<int16_t>(cfg_.tap_max_move_px) ||
        stable_frames_ < cfg_.long_press_stable_frames)
    {
        return false;
    }

    long_press_fired_ = true;
    state_ = State::LongPressSent;
    push_event(GestureType::LongPress, last_x_, last_y_, start_x_, start_y_, dx, dy, held);
    return true;
}

// 定时推进。没有新触摸数据时靠它处理两件事：
//   - 手指已经离开但最后一帧没收到 -> 结算
//   - 按住超过 3 秒的兜底 -> 强制结算，防止状态卡死
// 之后再看是否满足长按条件。
void TouchGesture::tick()
{
    // 已有事件没取走，先别产生新事件，避免覆盖。
    if (pending_.type != GestureType::None)
        return;

    const uint32_t t = now_ms();

    if (state_ != State::Idle)
    {
        // 驱动报告手指已离开，但我们状态机还没收到空采样 -> 主动结算。
        if (finger_up())
        {
            finish_contact(t);
            return;
        }
        // 兜底：按住超过 3 秒直接结算，避免异常时一直卡在 Pressed。
        if ((t - press_start_ms_) > 3000)
        {
            finish_contact(t);
            return;
        }
    }

    if (state_ != State::Pressed || long_press_fired_)
        return;

    try_long_press(t);
}

// 状态机核心入口：喂一个触摸采样，返回是否产生了事件。
//
// 三个分支：
//   A. 没有触点了（sample.count == 0）-> 结算或忽略
//   B. 状态是 Idle 且来了触点 -> 记录起点，进入 Pressed
//   C. 已经在按 -> 更新位置、判断长按/移动
bool TouchGesture::process_sample(const TouchSample& sample)
{
    // 上一条事件还没被取走：先别处理新采样，否则会丢事件。
    if (pending_.type != GestureType::None)
        return true;

    const uint32_t t = now_ms();

    // ── A. 采样里没有触点 ───────────────────────────────
    if (sample.count == 0)
    {
        if (state_ == State::Idle)
            return false;  // 本来就没按，忽略

        // 驱动还认为有手指，说明这只是短暂读空，先不结算。
        if (!finger_up())
            return false;

        finish_contact(t);
        return pending_.type != GestureType::None;
    }

    // ── B/C. 有触点 ────────────────────────────────────
    map_point(sample.points[0].x, sample.points[0].y, mapped_);
    if (!accept_position(mapped_.x, mapped_.y))
        return false;  // 噪声点，丢弃

    // 从"没按"到"刚按下"：记录起点，进入 Pressed。
    if (state_ == State::Idle)
    {
        state_ = State::Pressed;
        start_x_ = last_x_ = mapped_.x;
        start_y_ = last_y_ = mapped_.y;
        press_start_ms_ = t;
        long_press_fired_ = false;
        stable_frames_ = 1;

        // 只有在配置要求时才发 Press 事件。
        if (cfg_.emit_press)
        {
            push_event(GestureType::Press, mapped_.x, mapped_.y, start_x_, start_y_, 0, 0, 0);
            return true;
        }
        return false;
    }

    // 已经在按：先更新"稳定帧数"。
    // 两帧之间几乎没动 -> 稳定帧 +1；动了 -> 清零。
    const int32_t dfx = std::abs(static_cast<int32_t>(mapped_.x) - static_cast<int32_t>(last_x_));
    const int32_t dfy = std::abs(static_cast<int32_t>(mapped_.y) - static_cast<int32_t>(last_y_));
    if (dfx <= 1 && dfy <= 1)
        stable_frames_ = static_cast<uint8_t>(std::min<uint16_t>(255, stable_frames_ + 1));
    else
        stable_frames_ = 0;

    last_x_ = mapped_.x;
    last_y_ = mapped_.y;

    const int16_t dx = static_cast<int16_t>(mapped_.x - start_x_);
    const int16_t dy = static_cast<int16_t>(mapped_.y - start_y_);
    const int16_t adx = static_cast<int16_t>(std::abs(dx));
    const int16_t ady = static_cast<int16_t>(std::abs(dy));
    const uint32_t held = t - press_start_ms_;

    // 先判断长按，长按优先级高于移动。
    if (try_long_press(t))
        return true;

    // 配置要求发 Move 且确实在动 -> 发 Move 事件。
    if (cfg_.emit_move && state_ == State::Pressed && (adx > 2 || ady > 2))
    {
        push_event(GestureType::Move, mapped_.x, mapped_.y, start_x_, start_y_, dx, dy, held);
        return true;
    }

    return false;
}

// 从驱动读一次触摸，然后交给状态机。
// 这是 InputRouter 里每次采样调用的入口。
bool TouchGesture::poll()
{
    if (touch_ == nullptr || pending_.type != GestureType::None)
        return pending_.type != GestureType::None;

    TouchSample sample = {};
    if (!touch_->read(sample))
        return false;

    return process_sample(sample);
}

} // namespace app::bsp::driver::ft6336u
