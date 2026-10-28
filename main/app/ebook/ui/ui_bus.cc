/**
 * [初学者导读] 文件：main/app/ebook/ui/ui_bus.cc
 * 作用：创建 UI 事件队列，负责普通任务和中断安全的事件投递与接收。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：UI 状态只能在 UiLoop/UI 线程修改，跨任务结果通过 UiBus 回 UI。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_bus.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::ui {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 构造一个 kind 已填好、其余 payload 清零的事件。
// 所有 post_* 函数最终都通过它统一初始化，避免漏掉 union 字段。
UiEvent make_event(UiEventKind k)
{
    // 声明并默认初始化对象：UiEvent ev{}。
    UiEvent ev{};
    // 赋值：把右侧 k 的结果保存到左侧 ev.kind。
    ev.kind = k;
    // 返回 ev 给调用者，表示当前函数的执行结果。
    return ev;
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 UiBus::get_instance：参数在括号里，返回类型写在函数名前面。
UiBus& UiBus::get_instance()
{
    // 全局事件总线只需要一条队列，因此使用函数内静态单例。
    static UiBus s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// [初学者导读] 创建 UiLoop 消费的 FreeRTOS 队列；重复调用是幂等的。
bool UiBus::init()
{
    // 已经初始化过就直接返回，保证 init() 可以重复调用。
    if (queue_ != nullptr)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    // 创建一个队列，元素类型是 UiEvent，队列深度由 kQueueDepth 决定。
    queue_ = xQueueCreate(kQueueDepth, sizeof(UiEvent));
    // 创建失败时 queue_ 为 nullptr，调用者能通过返回值判断失败。
    return queue_ != nullptr;
}

// 定义或声明函数 UiBus::deinit：参数在括号里，返回类型写在函数名前面。
void UiBus::deinit()
{
    // 只销毁队列，不负责停止生产者任务。
    // 如果其他任务仍在调用 post()，必须先确保它们已经停止。
    if (queue_ != nullptr)
    {
        // 删除队列本身；队列中尚未消费的事件也会一起释放。
        vQueueDelete(queue_);
        // 赋值：把右侧 nullptr 的结果保存到左侧 queue_。
        queue_ = nullptr;
    }
}

// 函数定义/声明：调用 UiBus::post()：普通任务发送事件。
bool UiBus::post(const UiEvent& ev, uint32_t timeout_ms)
{
    // 普通任务使用：timeout_ms=0 表示不等待，队列满时返回 false。
    // UI 事件通常允许丢弃瞬时 tick，但关键提示要显式选择阻塞策略。
    if (queue_ == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // xQueueSend 是普通任务版本，不能从中断调用。
    return xQueueSend(queue_, &ev, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

// 函数定义/声明：调用 UiBus::post_isr()：中断中发送事件。
bool UiBus::post_isr(const UiEvent& ev)
{
    // 中断上下文必须使用 FromISR 版本，并根据 hp_wake 触发任务切换。
    if (queue_ == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // hp_wake 用来记录本次发送是否唤醒了更高优先级任务。
    BaseType_t hp_wake = pdFALSE;
    // 中断安全版本：不能在 ISR 里使用普通 xQueueSend。
    const BaseType_t ok = xQueueSendFromISR(queue_, &ev, &hp_wake);
    // 如果唤醒了更高优先级任务，需要在退出中断前主动让出 CPU。
    portYIELD_FROM_ISR(hp_wake);
    // 返回 ok == pdTRUE 给调用者，表示当前函数的执行结果。
    return ok == pdTRUE;
}

// 函数定义/声明：调用 UiBus::post_input()：UiBus::postinput()。
bool UiBus::post_input(const ::app::ebook::input::Event& ev)
{
    // 触摸任务把已经识别出的输入事件交给 UiLoop，不能直接调用 App::on_input()。
    UiEvent e = make_event(UiEventKind::Input); // 先标记事件类型。
    // 赋值：把右侧 ev 的结果保存到左侧 e.payload.input。
    e.payload.input = ev;                      // 再复制具体输入数据。
    // 返回 post(e, 0) 给调用者，表示当前函数的执行结果。
    return post(e, 0);
}

// 函数调用：bool UiBus::post_physical(::app::ebook::input::PhysicalKey key,；执行该函数并处理它的返回值或副作用。
bool UiBus::post_physical(::app::ebook::input::PhysicalKey key,
                          // 继续上一行的表达式或参数列表：::app::ebook::input::PhysicalAction action)。
                          ::app::ebook::input::PhysicalAction action)
{
    // 定义变量 UiEvent e 并初始化为 make_event(UiEventKind::PhysicalInput)。
    UiEvent e = make_event(UiEventKind::PhysicalInput);      // 标记为物理按键事件。
    // 赋值：把右侧 static_cast<uint8_t>(key) 的结果保存到左侧 e.payload.physical.key。
    e.payload.physical.key = static_cast<uint8_t>(key);      // 保存 Up/Mid/Down 档位。
    // 赋值：把右侧 static_cast<uint8_t>(action) 的结果保存到左侧 e.payload.physical.action。
    e.payload.physical.action = static_cast<uint8_t>(action); // 保存按下或释放。
    // 返回 post(e, 0) 给调用者，表示当前函数的执行结果。
    return post(e, 0);
}

// 函数定义/声明：调用 UiBus::post_tick_clock()：这些函数的共同模式是： 1. 用 makeevent(kind) 创建零初始化事件。 2. 填写 payload。 3. 调用 post(e, 0)。。
bool UiBus::post_tick_clock(uint8_t hour, uint8_t minute, uint8_t second)
{
    // 定义变量 UiEvent e 并初始化为 make_event(UiEventKind::TickClock)。
    UiEvent e = make_event(UiEventKind::TickClock); // 时钟变化事件。
    // 赋值：把右侧 hour 的结果保存到左侧 e.payload.clock.hour。
    e.payload.clock.hour = hour;                    // 小时。
    // 赋值：把右侧 minute 的结果保存到左侧 e.payload.clock.minute。
    e.payload.clock.minute = minute;                // 分钟。
    // 赋值：把右侧 second 的结果保存到左侧 e.payload.clock.second。
    e.payload.clock.second = second;                // 秒。
    // 返回 post(e, 0) 给调用者，表示当前函数的执行结果。
    return post(e, 0);
}

// 函数定义/声明：调用 UiBus::post_tick_battery()：这些函数的共同模式是： 1. 用 makeevent(kind) 创建零初始化事件。 2. 填写 payload。 3. 调用 post(e, 0)。。
bool UiBus::post_tick_battery(uint8_t pct, uint32_t mv)
{
    // 定义变量 UiEvent e 并初始化为 make_event(UiEventKind::TickBattery)。
    UiEvent e = make_event(UiEventKind::TickBattery); // 电量变化事件。
    // 赋值：把右侧 pct 的结果保存到左侧 e.payload.battery.pct。
    e.payload.battery.pct = pct;                      // 百分比。
    // 赋值：把右侧 mv 的结果保存到左侧 e.payload.battery.mv。
    e.payload.battery.mv = mv;                        // 电压毫伏。
    // 返回 post(e, 0) 给调用者，表示当前函数的执行结果。
    return post(e, 0);
}

// 定义或声明函数 UiBus::post_wifi_state：参数在括号里，返回类型写在函数名前面。
bool UiBus::post_wifi_state(uint8_t state, uint8_t fail)
{
    // 定义变量 UiEvent e 并初始化为 make_event(UiEventKind::WifiState)。
    UiEvent e = make_event(UiEventKind::WifiState); // Wi-Fi 状态变化事件。
    // 赋值：把右侧 state 的结果保存到左侧 e.payload.wifi.state。
    e.payload.wifi.state = state;                    // 连接状态码。
    // 赋值：把右侧 fail 的结果保存到左侧 e.payload.wifi.fail。
    e.payload.wifi.fail = fail;                      // 失败原因码。
    // 返回 post(e, 0) 给调用者，表示当前函数的执行结果。
    return post(e, 0);
}

// 定义或声明函数 UiBus::post_wifi_scan_done：参数在括号里，返回类型写在函数名前面。
bool UiBus::post_wifi_scan_done()
{
    // 返回 post(make_event(UiEventKind::WifiScanDone), 0) 给调用者，表示当前函数的执行结果。
    return post(make_event(UiEventKind::WifiScanDone), 0);
}

// 定义或声明函数 UiBus::post_ntp_sync_done：参数在括号里，返回类型写在函数名前面。
bool UiBus::post_ntp_sync_done(uint8_t status)
{
    // 定义变量 UiEvent e 并初始化为 make_event(UiEventKind::NtpSyncDone)。
    UiEvent e = make_event(UiEventKind::NtpSyncDone); // NTP 同步完成事件。
    // 赋值：把右侧 SystemHintKind::None 的结果保存到左侧 e.payload.system.hint。
    e.payload.system.hint = SystemHintKind::None;      // 这里复用 system 字段。
    // 赋值：把右侧 status 的结果保存到左侧 e.payload.system.value。
    e.payload.system.value = status;                   // 保存同步结果。
    // 返回 post(e, 0) 给调用者，表示当前函数的执行结果。
    return post(e, 0);
}

// 函数定义/声明：调用 UiBus::post_system_hint()：关键消息不能丢。
bool UiBus::post_system_hint(SystemHintKind hint, uint32_t value)
{
    // 定义变量 UiEvent e 并初始化为 make_event(UiEventKind::SystemHint)。
    UiEvent e = make_event(UiEventKind::SystemHint); // 通用系统提示事件。
    // 赋值：把右侧 hint 的结果保存到左侧 e.payload.system.hint。
    e.payload.system.hint = hint;                     // 提示种类。
    // 赋值：把右侧 value 的结果保存到左侧 e.payload.system.value。
    e.payload.system.value = value;                   // 附加数值。

    // ReaderIndexDone 必须送达（队列满时阻塞），否则 UI 会一直停在索引页。
    // 其他提示允许丢弃，避免后台任务把 UI 队列拖死。
    const uint32_t to = (hint == SystemHintKind::ReaderIndexDone) ? portMAX_DELAY : 0;
    // 返回 post(e, to) 给调用者，表示当前函数的执行结果。
    return post(e, to);
}

// 函数定义/声明：调用 UiBus::recv()：UiLoop 取出事件。
bool UiBus::recv(UiEvent& out, uint32_t timeout_ms)
{
    // UiLoop 从这里取事件；timeout_ms 为 0 时用于清空积压事件。
    if (queue_ == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // UiLoop 通过这个函数获取事件，xQueueReceive 会把事件复制到 out。
    return xQueueReceive(queue_, &out, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::ui
