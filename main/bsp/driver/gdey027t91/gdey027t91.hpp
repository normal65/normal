#pragma once

/**
 * 文件：main/bsp/driver/gdey027t91/gdey027t91.hpp
 *
 * GDEY027T91 墨水屏驱动（SSD1680 兼容控制器）。
 *
 * 负责：
 *   - 4 线 SPI 通信（SCLK / MOSI / CS / D/C#）
 *   - 复位（RES#）和忙状态查询（BUSY）
 *   - 整屏 / 快刷 / 局刷
 *   - 深睡眠与唤醒
 *
 * 并发约束：所有 SPI 访问都要先拿 BusGuard，避免刷新和休眠互相打断。
 */

#include <cstdint>
#include <mutex>

#include "panel.hpp"

namespace app::bsp::driver::gdey027t91 {

// 刷新方式。
enum class Present : uint8_t
{
    Base = 0,  // 建立第一张局刷基线
    Partial,   // 局部刷新（必须带窗口）
    Fast,      // 快速刷新整屏
    Full,      // 全屏刷新，最干净
};

/**
 * GDEY027T91 屏幕驱动。
 *
 * 典型用法：
 *   Gdey027t91 epd;
 *   epd.init();
 *   epd.fb() 里画图；
 *   epd.present(Present::Full);
 */
class Gdey027t91
{
  public:
    // 面板几何参数，来自 panel.hpp。
    static constexpr uint16_t WIDTH = Panel::WIDTH;            // 176 像素
    static constexpr uint16_t HEIGHT = Panel::HEIGHT;          // 264 像素
    static constexpr uint16_t STRIDE = Panel::STRIDE;          // 22 字节/行
    static constexpr uint32_t BUFFER_SIZE = Panel::BUFFER_SIZE; // 5808 字节

    Gdey027t91();
    ~Gdey027t91();

    Gdey027t91(const Gdey027t91&) = delete;
    Gdey027t91& operator=(const Gdey027t91&) = delete;

    // 初始化 GPIO / SPI / framebuffer，并完成复位和基础配置。
    bool init();
    // 释放 framebuffer 和 SPI（会先让屏幕进深睡眠）。
    void deinit();

    // 返回整屏 framebuffer，调用者可以直接在上面画图。
    uint8_t* fb()
    {
        return framebuffer_;
    }
    const uint8_t* fb() const
    {
        return framebuffer_;
    }

    // 只填充内存 framebuffer，不立即上屏。
    void fill(uint8_t color = 0xFF);

    // 刷新整屏或建立基线；Partial 必须用带窗口的重载。
    bool present(Present kind);
    // 局部刷新：win 会被裁剪并对齐到 8 像素。
    bool present(Present kind, Window win);

    // 进入深睡眠 / 从深睡眠唤醒。
    void sleep();
    void wake();

    // 局刷基线是否有效（有效才能走 Partial）。
    bool session_ready() const
    {
        return base_ok_;
    }

    // 主动丢弃局刷基线，下次刷新前必须重建。
    void invalidate_session();

    // 驱动是否已经初始化。
    bool initialized() const
    {
        return initialized_;
    }

    // BUSY 高电平表示屏幕还在忙。
    bool is_busy() const;
    // 等 BUSY 变低，最多等 timeout_ms。
    bool wait_busy(uint32_t timeout_ms = Panel::BUSY_TIMEOUT_FULL_MS) const;

  private:
    // SPI 设备句柄的封装，避免头文件暴露 ESP-IDF 类型。
    struct Spi;

    // RAII 总线锁：构造时拿锁，析构时自动释放 CS 和锁。
    class BusGuard
    {
      public:
        explicit BusGuard(Gdey027t91& dev);
        ~BusGuard();

        BusGuard(const BusGuard&) = delete;
        BusGuard& operator=(const BusGuard&) = delete;

        bool ok() const
        {
            return acquired_;
        }

      private:
        Gdey027t91& dev_;
        bool acquired_;
    };

    // 统一检查是否已经初始化。
    bool ensure_hw() const;
    // 让电源状态和局刷基线失效。
    void drop_session();
    // 复位 + 重新配置 + 上电（屏幕状态不确定时用）。
    bool cold_boot();

    // 给屏幕内部升压电路上电 / 断电。
    bool ensure_power();
    void set_fast_booster_timing();
    bool power_on();
    bool power_off();

    // 触发一次刷新：0x22 + sequence，再发 0x20。
    bool trigger_update(uint8_t sequence, uint32_t busy_timeout_ms);

    // 硬件复位：RES# 低 10ms，高 10ms。
    static void hw_reset();
    // 软件复位：0x12。
    bool sw_reset();
    // 一次性配置：0x01 / 0x18 / 0x3C / RAM 窗口。
    void configure_panel();
    // 把窗口设成全屏，并把光标归零。
    void set_full_ram_entry();
    // 设置 RAM X/Y 窗口。
    void set_ram_area(uint8_t x_start, uint8_t x_end, uint8_t y_start, uint8_t y_start_hi,
                      uint8_t y_end, uint8_t y_end_hi);
    // 设置 RAM 写指针。
    void set_ram_cursor(uint16_t x_byte, uint16_t y);
    // 把局刷窗口换算成 RAM 地址并设置光标。
    void apply_window(const Window& win);
    // 设置边框波形（0x3C）。
    void set_border_waveform(uint8_t value);

    // 写整屏 / 写窗口。
    void write_ram_full(uint8_t cmd_id, const uint8_t* src);
    void write_ram_window(uint8_t cmd_id, const Window& win, const uint8_t* region_tl);

    // 三种刷新路径。
    bool present_base();
    bool present_full();
    bool present_fast();
    bool present_partial(const Window& win);

    // 底层 SPI 操作。
    void cmd(uint8_t c);
    void dat(uint8_t d);
    void dat_bulk(const uint8_t* buf, uint32_t len);
    void cs_assert();
    void cs_release();

    Spi* spi_;
    uint8_t* framebuffer_;       // 5808 字节，1-bit 整屏图像
    mutable std::mutex bus_mutex_;
    bool initialized_;
    bool powered_;               // 内部升压是否已打开
    bool base_ok_;               // 局刷基线是否有效
    bool cs_active_;             // CS 当前是否被拉低
};

} // namespace app::bsp::driver::gdey027t91
