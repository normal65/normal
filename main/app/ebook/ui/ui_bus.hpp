// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/ui/ui_bus.hpp
 * 作用：创建 UI 事件队列，负责普通任务和中断安全的事件投递与接收。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：UI 状态只能在 UiLoop/UI 线程修改，跨任务结果通过 UiBus 回 UI。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <freertos/FreeRTOS.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <freertos/queue.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/input_event.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/physical_types.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_event.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::ui {

/** @brief 多生产者 → UiLoop 事件队列（深度 32） */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class UiBus
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 队列深度：最多缓存 32 个事件。
    static constexpr uint16_t kQueueDepth = 32;

    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static UiBus& get_instance();

    // 函数调用：bool init();；执行该函数并处理它的返回值或副作用。
    bool init();
    // 调用 deinit()：创建和销毁队列。
    void deinit();
    // 定义或声明函数 ready：参数在括号里，返回类型写在函数名前面。
    bool ready() const
    {
        // 返回 queue_ != nullptr 给调用者，表示当前函数的执行结果。
        return queue_ != nullptr;
    }

    // 赋值：把右侧 0) 的结果保存到左侧 bool post(const UiEvent& ev, uint32_t timeout_ms。
    bool post(const UiEvent& ev, uint32_t timeout_ms = 0);
    // 函数调用：bool post_isr(const UiEvent& ev);；执行该函数并处理它的返回值或副作用。
    bool post_isr(const UiEvent& ev);

    // 函数调用：bool post_input(const ::app::ebook::input::Event& ev);；执行该函数并处理它的返回值或副作用。
    bool post_input(const ::app::ebook::input::Event& ev);
    // 函数调用：bool post_physical(::app::ebook::input::PhysicalKey key,；执行该函数并处理它的返回值或副作用。
    bool post_physical(::app::ebook::input::PhysicalKey key,
                       // 继续上一行的表达式或参数列表：::app::ebook::input::PhysicalAction action);。
                       ::app::ebook::input::PhysicalAction action);
    // 调用 post_tick_clock(uint8_thour,uint8_tminute,uint8_tsecond=255)：这些函数的共同模式是： 1. 用 makeevent(kind) 创建零初始化事件。 2. 填写 payload。 3. 调用 post(e, 0)。。
    bool post_tick_clock(uint8_t hour, uint8_t minute, uint8_t second = 255);
    // 调用 post_tick_battery(uint8_tpct,uint32_tmv)：这些函数的共同模式是： 1. 用 makeevent(kind) 创建零初始化事件。 2. 填写 payload。 3. 调用 post(e, 0)。。
    bool post_tick_battery(uint8_t pct, uint32_t mv);
    // 函数调用：bool post_wifi_state(uint8_t state, uint8_t fail);；执行该函数并处理它的返回值或副作用。
    bool post_wifi_state(uint8_t state, uint8_t fail);
    // 函数调用：bool post_wifi_scan_done();；执行该函数并处理它的返回值或副作用。
    bool post_wifi_scan_done();
    // 函数调用：bool post_ntp_sync_done(uint8_t status);；执行该函数并处理它的返回值或副作用。
    bool post_ntp_sync_done(uint8_t status);
    // 调用 post_system_hint(SystemHintKindhint,uint32_tvalue=0)：关键消息不能丢。
    bool post_system_hint(SystemHintKind hint, uint32_t value = 0);

    // 调用 recv(UiEvent&out,uint32_ttimeout_ms)：UiLoop 取出事件。
    bool recv(UiEvent& out, uint32_t timeout_ms);

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 UiBus() = default;，执行这个函数并处理它的返回值或状态变化。
    UiBus() = default;
    // 赋值：把右侧 default 的结果保存到左侧 ~UiBus()。
    ~UiBus() = default;
    // 调用 UiBus(const UiBus&) = delete;，执行这个函数并处理它的返回值或状态变化。
    UiBus(const UiBus&) = delete;
    // 赋值：把右侧 (const UiBus&) = delete 的结果保存到左侧 UiBus& operator。
    UiBus& operator=(const UiBus&) = delete;

    // 执行语句：QueueHandle_t queue_{nullptr};；它会更新一次变量、对象或程序状态。
    QueueHandle_t queue_{nullptr};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::ui
