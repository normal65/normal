// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/ui/ui_event.hpp
 * 作用：实现 ui 模块中的 ui event 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：UI 状态只能在 UiLoop/UI 线程修改，跨任务结果通过 UiBus 回 UI。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/input_event.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/physical_types.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::ui {

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class UiEventKind : uint8_t
{
    // 执行这条代码：None = 0,。它会更新变量、对象状态或调用链。
    None = 0,
    // 继续上一行的表达式或参数列表：Input,。
    Input,
    // 继续上一行的表达式或参数列表：PhysicalInput,。
    PhysicalInput,
    // 继续上一行的表达式或参数列表：TickClock,。
    TickClock,
    // 继续上一行的表达式或参数列表：TickBattery,。
    TickBattery,
    // 继续上一行的表达式或参数列表：WifiState,。
    WifiState,
    // 继续上一行的表达式或参数列表：WifiScanDone,。
    WifiScanDone,
    // 继续上一行的表达式或参数列表：NtpSyncDone,。
    NtpSyncDone,
    // 继续上一行的表达式或参数列表：SystemHint,。
    SystemHint,
};

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class SystemHintKind : uint8_t
{
    // 执行这条代码：None = 0,。它会更新变量、对象状态或调用链。
    None = 0,
    // 继续上一行的表达式或参数列表：ReaderIndexProgress,。
    ReaderIndexProgress,
    // 继续上一行的表达式或参数列表：ReaderIndexDone,。
    ReaderIndexDone,
    // 继续上一行的表达式或参数列表：ClockAlarm,。
    ClockAlarm,
    // 继续上一行的表达式或参数列表：WeatherFetchDone,。
    WeatherFetchDone,
    // 继续上一行的表达式或参数列表：MusicScanDone,。
    MusicScanDone,
    // 继续上一行的表达式或参数列表：GalleryScanDone,。
    GalleryScanDone,
    // 继续上一行的表达式或参数列表：GalleryDecodeDone,。
    GalleryDecodeDone,
    // 继续上一行的表达式或参数列表：FilesReloadDone,。
    FilesReloadDone,
    // 继续上一行的表达式或参数列表：MusicStateChanged,。
    MusicStateChanged,
    // 继续上一行的表达式或参数列表：MusicProgress,。
    MusicProgress,
    // 继续上一行的表达式或参数列表：ToastExpire,。
    ToastExpire,
    // 继续上一行的表达式或参数列表：WoodenFishAnimDone,。
    WoodenFishAnimDone,
    // 继续上一行的表达式或参数列表：AutoLock,。
    AutoLock,
};

// struct UiEvent：把相关数据字段组合在一起，表示项目中的一个概念。
struct UiEvent
{
    // 执行语句：UiEventKind kind{UiEventKind::None};；它会更新一次变量、对象或程序状态。
    UiEventKind kind{UiEventKind::None};

    // 执行这条代码：union Payload。它会更新变量、对象状态或调用链。
    union Payload
    {
        // 定义或声明函数 Payload：参数在括号里，返回类型写在函数名前面。
        Payload()
            // 执行这条代码：: raw{}。它会更新变量、对象状态或调用链。
            : raw{}
        {
        }

        // 执行语句：::app::ebook::input::Event input;；它会更新一次变量、对象或程序状态。
        ::app::ebook::input::Event input;

        // 执行这条代码：struct。它会更新变量、对象状态或调用链。
        struct
        {
            // 执行语句：uint8_t key;；它会更新一次变量、对象或程序状态。
            uint8_t key;
            // 执行语句：uint8_t action;；它会更新一次变量、对象或程序状态。
            uint8_t action;
        // 执行语句：} physical;；它会更新一次变量、对象或程序状态。
        } physical;

        // 执行这条代码：struct。它会更新变量、对象状态或调用链。
        struct
        {
            // 执行语句：uint8_t pct;；它会更新一次变量、对象或程序状态。
            uint8_t pct;
            // 执行语句：uint32_t mv;；它会更新一次变量、对象或程序状态。
            uint32_t mv;
        // 执行语句：} battery;；它会更新一次变量、对象或程序状态。
        } battery;

        // 执行这条代码：struct。它会更新变量、对象状态或调用链。
        struct
        {
            // 执行语句：uint8_t hour;；它会更新一次变量、对象或程序状态。
            uint8_t hour;
            // 执行语句：uint8_t minute;；它会更新一次变量、对象或程序状态。
            uint8_t minute;
            // 执行语句：uint8_t second;；它会更新一次变量、对象或程序状态。
            uint8_t second;
        // 执行语句：} clock;；它会更新一次变量、对象或程序状态。
        } clock;

        // 执行这条代码：struct。它会更新变量、对象状态或调用链。
        struct
        {
            // 执行语句：uint8_t state;；它会更新一次变量、对象或程序状态。
            uint8_t state;
            // 执行语句：uint8_t fail;；它会更新一次变量、对象或程序状态。
            uint8_t fail;
        // 执行语句：} wifi;；它会更新一次变量、对象或程序状态。
        } wifi;

        // 执行这条代码：struct。它会更新变量、对象状态或调用链。
        struct
        {
            // 执行语句：SystemHintKind hint;；它会更新一次变量、对象或程序状态。
            SystemHintKind hint;
            // 执行语句：uint32_t value;；它会更新一次变量、对象或程序状态。
            uint32_t value;
        // 执行语句：} system;；它会更新一次变量、对象或程序状态。
        } system;

        // 执行语句：uint8_t raw[16];；它会更新一次变量、对象或程序状态。
        uint8_t raw[16];
    // 执行语句：} payload;；它会更新一次变量、对象或程序状态。
    } payload;

    // 调用 UiEvent() = default;，执行这个函数并处理它的返回值或状态变化。
    UiEvent() = default;
};

// 调用 static_assert(sizeof(UiEvent) <= 32, "UiEvent must stay compact for fifo");，执行这个函数并处理它的返回值或状态变化。
static_assert(sizeof(UiEvent) <= 32, "UiEvent must stay compact for fifo");

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::ui
