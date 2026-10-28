// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/ui/layer.hpp
 * 作用：定义可绘制覆盖层的公共接口。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：UI 状态只能在 UiLoop/UI 线程修改，跨任务结果通过 UiBus 回 UI。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/geometry.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/canvas.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/input_event.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "shell/page.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_event.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::ui {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using InputResult = shell::InputResult;

/** 浮层基类（键盘等）；`InputResult` 为 shell 别名 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class Layer
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 赋值：把右侧 default 的结果保存到左侧 virtual ~Layer()。
    virtual ~Layer() = default;

    // 函数调用：virtual void on_attach() {}；执行该函数并处理它的返回值或副作用。
    virtual void on_attach() {}
    // 函数调用：virtual void on_detach() {}；执行该函数并处理它的返回值或副作用。
    virtual void on_detach() {}

    // 函数定义/声明：调用 visible()：状态栏是否显示由两件事决定： 当前 Shell 页面。 如果当前是 AppHost，还要问活动 App 的 wantsstatusbar()。。
    virtual bool visible() const
    {
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 定义或声明函数 modal：参数在括号里，返回类型写在函数名前面。
    virtual bool modal() const
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 定义或声明函数 wants_status_bar：参数在括号里，返回类型写在函数名前面。
    virtual bool wants_status_bar() const
    {
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 赋值：把右侧 0 的结果保存到左侧 virtual core::Rect bounds() const。
    virtual core::Rect bounds() const = 0;

    // 函数定义/声明：调用 on_input()：1. 检查工具栏：退出、切数字、切字母、切符号、切拼音、完成。 2. 拼音模式下检查候选条。 3. 检查字母区域。 4. 处理 Shift、空格、退格或普通字符。。
    virtual InputResult on_input(const ::app::ebook::input::Event& ev)
    {
        // 执行语句：(void)ev;；它会更新一次变量、对象或程序状态。
        (void)ev;
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};
    }

    // 赋值：把右侧 0 的结果保存到左侧 virtual void paint(gfx::Canvas& canvas)。
    virtual void paint(gfx::Canvas& canvas) = 0;

    // 定义或声明函数 on_ui_event：参数在括号里，返回类型写在函数名前面。
    virtual void on_ui_event(const UiEvent& ev)
    {
        // 多分支判断：根据括号里的值选择不同 case。
        switch (ev.kind)
        {
            // 分支匹配：case UiEventKind::TickClock 成立时执行下面的代码。
            case UiEventKind::TickClock:
                // 调用 on_tick_clock(ev.payload.clock.hour, ev.payload.clock.minute,，执行这个函数并处理它的返回值或状态变化。
                on_tick_clock(ev.payload.clock.hour, ev.payload.clock.minute,
                              // 继续上一行的表达式或参数列表：ev.payload.clock.second);。
                              ev.payload.clock.second);
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 分支匹配：case UiEventKind::TickBattery 成立时执行下面的代码。
            case UiEventKind::TickBattery:
                // 调用 on_tick_battery(ev.payload.battery.pct)：两者只在数值变化且状态栏可见时重绘，避免后台页面推动刷新。。
                on_tick_battery(ev.payload.battery.pct);
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 分支匹配：case UiEventKind::WifiState 成立时执行下面的代码。
            case UiEventKind::WifiState:
                // 调用 on_wifi_state(ev.payload.wifi.state, ev.payload.wifi.fail);，执行这个函数并处理它的返回值或状态变化。
                on_wifi_state(ev.payload.wifi.state, ev.payload.wifi.fail);
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 分支匹配：case UiEventKind::WifiScanDone 成立时执行下面的代码。
            case UiEventKind::WifiScanDone:
                // 调用 on_wifi_scan_done()：扫描回调先把结果复制到 gpending，并用 gpendingmtx 保护；完成后投递 WifiScanDone。UI 线程收到事件后再读取 gpending 更新 wifiscan。。
                on_wifi_scan_done();
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 分支匹配：case UiEventKind::NtpSyncDone 成立时执行下面的代码。
            case UiEventKind::NtpSyncDone:
                // 调用 on_ntp_sync_done(static_cast<uint8_t>(ev.payload.system.value));，执行这个函数并处理它的返回值或状态变化。
                on_ntp_sync_done(static_cast<uint8_t>(ev.payload.system.value));
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 分支匹配：case UiEventKind::SystemHint 成立时执行下面的代码。
            case UiEventKind::SystemHint:
                // 调用 on_system_hint(ev.payload.system.hint, ev.payload.system.value);，执行这个函数并处理它的返回值或状态变化。
                on_system_hint(ev.payload.system.hint, ev.payload.system.value);
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 执行这条代码：default:。它会更新变量、对象状态或调用链。
            default:
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
        }
    }

    // 函数定义/声明：调用 on_tick_clock()：两者只在数值变化且状态栏可见时重绘，避免后台页面推动刷新。。
    virtual void on_tick_clock(uint8_t h, uint8_t m, uint8_t s)
    {
        // 执行语句：(void)h;；它会更新一次变量、对象或程序状态。
        (void)h;
        // 执行语句：(void)m;；它会更新一次变量、对象或程序状态。
        (void)m;
        // 执行语句：(void)s;；它会更新一次变量、对象或程序状态。
        (void)s;
    }
    // 函数定义/声明：调用 on_tick_battery()：两者只在数值变化且状态栏可见时重绘，避免后台页面推动刷新。。
    virtual void on_tick_battery(uint8_t pct)
    {
        // 执行语句：(void)pct;；它会更新一次变量、对象或程序状态。
        (void)pct;
    }
    // 函数定义/声明：调用 on_wifi_state()：onwifistate()。
    virtual void on_wifi_state(uint8_t state, uint8_t fail)
    {
        // 执行语句：(void)state;；它会更新一次变量、对象或程序状态。
        (void)state;
        // 执行语句：(void)fail;；它会更新一次变量、对象或程序状态。
        (void)fail;
    }
    // 调用 on_wifi_scan_done()：扫描回调先把结果复制到 gpending，并用 gpendingmtx 保护；完成后投递 WifiScanDone。UI 线程收到事件后再读取 gpending 更新 wifiscan。。
    virtual void on_wifi_scan_done() {}
    // 函数定义/声明：调用 on_ntp_sync_done()：处理同步完成事件，更新页面状态。它由 UI 线程消费。。
    virtual void on_ntp_sync_done(uint8_t status)
    {
        // 执行语句：(void)status;；它会更新一次变量、对象或程序状态。
        (void)status;
    }
    // 定义或声明函数 on_system_hint：参数在括号里，返回类型写在函数名前面。
    virtual void on_system_hint(SystemHintKind kind, uint32_t value)
    {
        // 执行语句：(void)kind;；它会更新一次变量、对象或程序状态。
        (void)kind;
        // 执行语句：(void)value;；它会更新一次变量、对象或程序状态。
        (void)value;
    }
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::ui
