#pragma once

/**
 * 文件：main/bsp/driver/gdey027t91/panel.hpp
 *
 * GDEY027T91 面板的固定参数：
 *   - 分辨率 176×264
 *   - 1-bit framebuffer 的行宽和总大小
 *   - RAM X/Y 边界
 *   - BUSY 超时
 *   - 复位稳定时间
 *
 * 这些值描述的是硬件屏幕本身，换屏前必须核对新面板的数据手册。
 */

#include <algorithm>
#include <cstdint>

namespace app::bsp::driver::gdey027t91 {

struct Panel
{
    // 可见区域：176 列 × 264 行。
    static constexpr uint16_t WIDTH = 176;
    static constexpr uint16_t HEIGHT = 264;

    // 1-bit 屏幕：8 个像素打包成 1 个字节。
    static constexpr uint16_t STRIDE = WIDTH / 8;              // 22 字节/行
    static constexpr uint32_t BUFFER_SIZE = STRIDE * HEIGHT;   // 5808 字节

    // RAM X 地址以“字节”为单位，22 字节对应 X = 0..21。
    static constexpr uint8_t RAM_X_END = static_cast<uint8_t>((WIDTH / 8U) - 1U);

    // 各种刷新方式的最长等待时间。
    static constexpr uint32_t BUSY_TIMEOUT_FULL_MS = 15000;    // 全刷最慢
    static constexpr uint32_t BUSY_TIMEOUT_FAST_MS = 6000;     // 快刷
    static constexpr uint32_t BUSY_TIMEOUT_PARTIAL_MS = 3000;  // 局刷
    static constexpr uint32_t BUSY_TIMEOUT_CMD_MS = 2000;      // 普通命令后

    // RES# 电平变化后的稳定等待时间。
    // 手册只明确写了“复位后 Wait 10ms”，10ms 是工程保守值。
    static constexpr uint32_t RESET_SETTLE_MS = 10;
};

// 局刷窗口：描述要更新的矩形区域。
// SSD1680 的 X 地址按字节计算，所以窗口要按 8 像素对齐。
struct Window
{
    uint16_t x = 0;  // 左上角 X（像素）
    uint16_t y = 0;  // 左上角 Y（像素）
    uint16_t w = 0;  // 宽度（像素）
    uint16_t h = 0;  // 高度（像素）

    bool valid() const
    {
        return w > 0 && h > 0;
    }

    // 裁剪到屏幕范围内，并把 X 边界对齐到 8 像素。
    // 对齐后如果宽度不是 8 的整数倍，或者面积为 0，返回 false。
    bool normalize()
    {
        if (w == 0 || h == 0 || x >= Panel::WIDTH || y >= Panel::HEIGHT)
            return false;

        uint16_t x2 = static_cast<uint16_t>(x + w);
        uint16_t y2 = static_cast<uint16_t>(y + h);

        x2 = std::min(x2, Panel::WIDTH);
        y2 = std::min(y2, Panel::HEIGHT);
        if (x2 <= x || y2 <= y)
            return false;

        // X 地址以 8 像素为单位：左边界向下对齐，右边界向上对齐。
        x = static_cast<uint16_t>(x & ~7U);
        x2 = static_cast<uint16_t>((x2 + 7U) & ~7U);
        x2 = std::min(x2, Panel::WIDTH);

        w = static_cast<uint16_t>(x2 - x);
        h = static_cast<uint16_t>(y2 - y);

        return w > 0 && h > 0 && (w & 7U) == 0U;
    }
};

} // namespace app::bsp::driver::gdey027t91
