#pragma once

/**
 * 文件：main/bsp/driver/gdey027t91/backlight.hpp
 *
 * 墨水屏前光（背光）亮度控制。
 *
 * 用 LEDC PWM 调节亮度：
 *   - 0%   = 关闭
 *   - 100% = 最亮
 *
 * 注意：前光只影响照明，不影响墨水屏本身的画面灰度。
 */

#include <cstdint>

namespace app::bsp::driver::gdey027t91 {

class Backlight
{
  public:
    Backlight();
    ~Backlight();

    Backlight(const Backlight&) = delete;
    Backlight& operator=(const Backlight&) = delete;

    // 配置 LEDC 定时器和通道。
    bool init();
    // 停止 PWM 输出。
    void deinit();

    // 设置亮度，范围 0~100。
    void set_brightness(uint8_t percent);
    // 恢复到关闭前的亮度（如果之前是 0，则默认 80%）。
    void on();
    // 关闭前光，并记住当前亮度。
    void off();

    uint8_t brightness() const
    {
        return brightness_;
    }

    bool ready() const
    {
        return ready_;
    }

  private:
    // 把 brightness_ 换算成 PWM duty 并写入 LEDC。
    void apply_duty();

    uint8_t brightness_ = 50; // 当前亮度（0~100）
    uint8_t saved_ = 50;      // off() 之前的亮度
    bool ready_ = false;      // LEDC 是否已配置
};

} // namespace app::bsp::driver::gdey027t91
