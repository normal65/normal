#pragma once

/**
 * 文件：main/bsp/driver/gdey027t91/ssd1680.hpp
 *
 * SSD1680 控制器的命令值和参数值。
 * 这些数字来自 SSD1680 手册（Rev 0.14）第 7–8 章。
 *
 * 命令通过 cmd() 发送（D/C#=0），参数通过 dat() 发送（D/C#=1）。
 */

#include <cstdint>

namespace app::bsp::driver::gdey027t91::ssd1680 {

// 命令字节。
struct Cmd
{
    static constexpr uint8_t kDriverOutputControl = 0x01; // 驱动输出行数、扫描方向
    static constexpr uint8_t kDataEntryMode = 0x11;       // 写 RAM 后地址递增方向
    static constexpr uint8_t kSwReset = 0x12;             // 软件复位
    static constexpr uint8_t kDeepSleep = 0x10;           // 深睡眠
    static constexpr uint8_t kTempSensor = 0x18;          // 温度传感器选择
    static constexpr uint8_t kDisplayUpdateCtrl1 = 0x22;  // 选择波形 / 电源序列
    static constexpr uint8_t kDisplayUpdateCtrl2 = 0x20;  // 触发执行
    static constexpr uint8_t kBorderWaveform = 0x3C;      // 边框波形
    static constexpr uint8_t kSetRamX = 0x44;             // 设 RAM X 窗口
    static constexpr uint8_t kSetRamY = 0x45;             // 设 RAM Y 窗口
    static constexpr uint8_t kSetRamXCounter = 0x4E;      // 设 RAM X 光标
    static constexpr uint8_t kSetRamYCounter = 0x4F;      // 设 RAM Y 光标
    static constexpr uint8_t kWriteRamBw = 0x24;          // 写当前帧
    static constexpr uint8_t kWriteRamBwPrev = 0x26;      // 写上一帧
};

// 0x22 的序列值：选择不同刷新波形和电源流程。
struct Seq
{
    static constexpr uint8_t kFull = 0xF7;    // 全刷：最干净、最慢
    static constexpr uint8_t kFast = 0xD7;    // 快刷：速度和残影折中
    static constexpr uint8_t kPartial = 0xFC; // 局刷：最快，依赖基线
    static constexpr uint8_t kPowerOn = 0xE0; // 内部升压上电
    static constexpr uint8_t kPowerOff = 0x83; // 内部升压断电
};

// 命令参数值。
struct Param
{
    static constexpr uint8_t kDataEntryNormal = 0x03; // X 递增、Y 递增
    static constexpr uint8_t kBorderNormal = 0x05;    // 常规边框波形
    static constexpr uint8_t kTempInternal = 0x80;    // 使用内部温度传感器
    static constexpr uint8_t kDeepSleepMode = 0x01;   // 深睡眠模式 1
    static constexpr uint8_t kBoosterTiming = 0x64;   // 快刷 booster 时序
};

} // namespace app::bsp::driver::gdey027t91::ssd1680
