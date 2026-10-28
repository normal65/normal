// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/gfx/icon.hpp
 * 作用：实现 gfx 模块中的 icon 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：绘制只改 framebuffer，不直接操作 SPI。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::gfx::icon {

// 赋值：把右侧 0xF112 的结果保存到左侧 inline constexpr uint32_t kFaReply。
inline constexpr uint32_t kFaReply = 0xF112;
// 赋值：把右侧 0xF053 的结果保存到左侧 inline constexpr uint32_t kFaChevronLeft。
inline constexpr uint32_t kFaChevronLeft = 0xF053;
// 赋值：把右侧 0xF054 的结果保存到左侧 inline constexpr uint32_t kFaChevronRight。
inline constexpr uint32_t kFaChevronRight = 0xF054;
// 赋值：把右侧 0xF077 的结果保存到左侧 inline constexpr uint32_t kFaChevronUp。
inline constexpr uint32_t kFaChevronUp = 0xF077;
// 赋值：把右侧 0xF078 的结果保存到左侧 inline constexpr uint32_t kFaChevronDown。
inline constexpr uint32_t kFaChevronDown = 0xF078;
// 赋值：把右侧 0xF00C 的结果保存到左侧 inline constexpr uint32_t kFaCheck。
inline constexpr uint32_t kFaCheck = 0xF00C;
// 赋值：把右侧 0xF00D 的结果保存到左侧 inline constexpr uint32_t kFaTimes。
inline constexpr uint32_t kFaTimes = 0xF00D;
// 赋值：把右侧 0xF067 的结果保存到左侧 inline constexpr uint32_t kFaPlus。
inline constexpr uint32_t kFaPlus = 0xF067;
// 赋值：把右侧 0xF068 的结果保存到左侧 inline constexpr uint32_t kFaMinus。
inline constexpr uint32_t kFaMinus = 0xF068;
// 赋值：把右侧 0xF002 的结果保存到左侧 inline constexpr uint32_t kFaSearch。
inline constexpr uint32_t kFaSearch = 0xF002;
// 赋值：把右侧 0xF021 的结果保存到左侧 inline constexpr uint32_t kFaSync。
inline constexpr uint32_t kFaSync = 0xF021;
// 赋值：把右侧 0xF55A 的结果保存到左侧 inline constexpr uint32_t kFaBackspace。
inline constexpr uint32_t kFaBackspace = 0xF55A;
// 赋值：把右侧 0xF023 的结果保存到左侧 inline constexpr uint32_t kFaLock。
inline constexpr uint32_t kFaLock = 0xF023;
// 赋值：把右侧 0xF09C 的结果保存到左侧 inline constexpr uint32_t kFaUnlock。
inline constexpr uint32_t kFaUnlock = 0xF09C;
// 赋值：把右侧 0xF141 的结果保存到左侧 inline constexpr uint32_t kFaEllipsisH。
inline constexpr uint32_t kFaEllipsisH = 0xF141;
// 赋值：把右侧 0xF204 的结果保存到左侧 inline constexpr uint32_t kFaToggleOff。
inline constexpr uint32_t kFaToggleOff = 0xF204;
// 赋值：把右侧 0xF205 的结果保存到左侧 inline constexpr uint32_t kFaToggleOn。
inline constexpr uint32_t kFaToggleOn = 0xF205;
// 赋值：把右侧 0xF1EB 的结果保存到左侧 inline constexpr uint32_t kFaWifi。
inline constexpr uint32_t kFaWifi = 0xF1EB;
// 赋值：把右侧 0xF186 的结果保存到左侧 inline constexpr uint32_t kFaMoon。
inline constexpr uint32_t kFaMoon = 0xF186;
// 赋值：把右侧 0xF185 的结果保存到左侧 inline constexpr uint32_t kFaSun。
inline constexpr uint32_t kFaSun = 0xF185;
// 赋值：把右侧 0xF028 的结果保存到左侧 inline constexpr uint32_t kFaVolumeUp。
inline constexpr uint32_t kFaVolumeUp = 0xF028;
// 赋值：把右侧 0xF6A9 的结果保存到左侧 inline constexpr uint32_t kFaVolumeMute。
inline constexpr uint32_t kFaVolumeMute = 0xF6A9;
// 赋值：把右侧 0xF1E6 的结果保存到左侧 inline constexpr uint32_t kFaPlug。
inline constexpr uint32_t kFaPlug = 0xF1E6;
// 赋值：把右侧 0xF240 的结果保存到左侧 inline constexpr uint32_t kFaBatteryFull。
inline constexpr uint32_t kFaBatteryFull = 0xF240;
// 赋值：把右侧 0xF243 的结果保存到左侧 inline constexpr uint32_t kFaBatteryQuarter。
inline constexpr uint32_t kFaBatteryQuarter = 0xF243;
// 赋值：把右侧 0xF5DA 的结果保存到左侧 inline constexpr uint32_t kFaBookReader。
inline constexpr uint32_t kFaBookReader = 0xF5DA;
// 赋值：把右侧 0xF11C 的结果保存到左侧 inline constexpr uint32_t kFaKeyboard。
inline constexpr uint32_t kFaKeyboard = 0xF11C;
// 赋值：把右侧 0xF249 的结果保存到左侧 inline constexpr uint32_t kFaStickyNote。
inline constexpr uint32_t kFaStickyNote = 0xF249;
// 赋值：把右侧 0xF302 的结果保存到左侧 inline constexpr uint32_t kFaImages。
inline constexpr uint32_t kFaImages = 0xF302;
// 赋值：把右侧 0xF1FC 的结果保存到左侧 inline constexpr uint32_t kFaPaintBrush。
inline constexpr uint32_t kFaPaintBrush = 0xF1FC;
// 赋值：把右侧 0xF6C4 的结果保存到左侧 inline constexpr uint32_t kFaCloudSun。
inline constexpr uint32_t kFaCloudSun = 0xF6C4;
// 赋值：把右侧 0xF001 的结果保存到左侧 inline constexpr uint32_t kFaMusic。
inline constexpr uint32_t kFaMusic = 0xF001;
// 赋值：把右侧 0xF04B 的结果保存到左侧 inline constexpr uint32_t kFaPlay。
inline constexpr uint32_t kFaPlay = 0xF04B;
// 赋值：把右侧 0xF04C 的结果保存到左侧 inline constexpr uint32_t kFaPause。
inline constexpr uint32_t kFaPause = 0xF04C;
// 赋值：把右侧 0xF048 的结果保存到左侧 inline constexpr uint32_t kFaStepBackward。
inline constexpr uint32_t kFaStepBackward = 0xF048;
// 赋值：把右侧 0xF051 的结果保存到左侧 inline constexpr uint32_t kFaStepForward。
inline constexpr uint32_t kFaStepForward = 0xF051;
// 赋值：把右侧 0xF017 的结果保存到左侧 inline constexpr uint32_t kFaClock。
inline constexpr uint32_t kFaClock = 0xF017;
// 赋值：把右侧 0xF073 的结果保存到左侧 inline constexpr uint32_t kFaCalendarAlt。
inline constexpr uint32_t kFaCalendarAlt = 0xF073;
// 赋值：把右侧 0xF07B 的结果保存到左侧 inline constexpr uint32_t kFaFolder。
inline constexpr uint32_t kFaFolder = 0xF07B;
// 赋值：把右侧 0xF07C 的结果保存到左侧 inline constexpr uint32_t kFaFolderOpen。
inline constexpr uint32_t kFaFolderOpen = 0xF07C;
// 赋值：把右侧 0xF15B 的结果保存到左侧 inline constexpr uint32_t kFaFile。
inline constexpr uint32_t kFaFile = 0xF15B;
// 赋值：把右侧 0xF7C2 的结果保存到左侧 inline constexpr uint32_t kFaSdCard。
inline constexpr uint32_t kFaSdCard = 0xF7C2;
// 赋值：把右侧 0xF362 的结果保存到左侧 inline constexpr uint32_t kFaExchangeAlt。
inline constexpr uint32_t kFaExchangeAlt = 0xF362;
// 赋值：把右侧 0xF013 的结果保存到左侧 inline constexpr uint32_t kFaCog。
inline constexpr uint32_t kFaCog = 0xF013;
// 赋值：把右侧 0xF05A 的结果保存到左侧 inline constexpr uint32_t kFaInfoCircle。
inline constexpr uint32_t kFaInfoCircle = 0xF05A;
// 赋值：把右侧 0xF519 的结果保存到左侧 inline constexpr uint32_t kFaHotspot。
inline constexpr uint32_t kFaHotspot = 0xF519;
// 赋值：把右侧 0xF0A0 的结果保存到左侧 inline constexpr uint32_t kFaHdd。
inline constexpr uint32_t kFaHdd = 0xF0A0;
// 赋值：把右侧 0xF132 的结果保存到左侧 inline constexpr uint32_t kFaShield。
inline constexpr uint32_t kFaShield = 0xF132;
// 赋值：把右侧 0xF019 的结果保存到左侧 inline constexpr uint32_t kFaDownload。
inline constexpr uint32_t kFaDownload = 0xF019;
// 赋值：把右侧 0xF26C 的结果保存到左侧 inline constexpr uint32_t kFaTv。
inline constexpr uint32_t kFaTv = 0xF26C;
// 赋值：把右侧 0xE001 的结果保存到左侧 inline constexpr uint32_t kEbBluetooth。
inline constexpr uint32_t kEbBluetooth = 0xE001;
// 赋值：把右侧 0xE002 的结果保存到左侧 inline constexpr uint32_t kEbWoodenStick。
inline constexpr uint32_t kEbWoodenStick = 0xE002;
// 赋值：把右侧 0xE003 的结果保存到左侧 inline constexpr uint32_t kEbWoodenFish。
inline constexpr uint32_t kEbWoodenFish = 0xE003;

/** 设置项开关图标 */
// 定义或声明函数 toggle：参数在括号里，返回类型写在函数名前面。
inline uint32_t toggle(bool on)
{
    // 返回 on ? kFaToggleOn : kFaToggleOff 给调用者，表示当前函数的执行结果。
    return on ? kFaToggleOn : kFaToggleOff;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::gfx::icon
