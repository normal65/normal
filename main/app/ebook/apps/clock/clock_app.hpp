// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/clock/clock_app.hpp
 * 作用：实现时钟界面、闹钟显示和闹钟编辑。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/clock/clock_store.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/list_view.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_event.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::clock {

/**
 * @brief 时钟：Clock / Alarm 标签；分钟 tick 与 ClockAlarm 经 on_ui_event
 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class ClockApp : public App
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static ClockApp& instance();

    // 定义或声明函数 id：参数在括号里，返回类型写在函数名前面。
    AppId id() const override
    {
        // 返回 AppId::Clock 给调用者，表示当前函数的执行结果。
        return AppId::Clock;
    }
    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const char* title() const override;
    // 调用 icon_cp()：返回图标 codepoint。图标通常来自字体图标表，所以这里返回的是 uint32t，不是图片路径。。
    uint32_t icon_cp() const override;

    // 调用 on_enter()：进入文件 App 时： 1. 重置浏览状态或恢复根卷状态。 2. 清空弹窗。 3. 设置加载状态。 4. 调度目录扫描。 5. 请求重绘。。
    void on_enter() override;
    // 调用 on_exit()：退出时取消未完成的扫描、关闭弹窗，并确保后台任务不会在对象状态失效后继续写数据。。
    void on_exit() override;
    // 调用 on_semantic_back()：处理语义返回，顺序通常是： 1. 如果弹窗打开，先关闭弹窗。 2. 如果列表还在浏览目录，先返回上一级。 3. 如果已经到根卷，返回 false，让 Shell 退出 App。。
    bool on_semantic_back() override;

    // 函数调用：void paint(gfx::Canvas& canvas) override;；执行该函数并处理它的返回值或副作用。
    void paint(gfx::Canvas& canvas) override;
    // 调用 on_input(const::app::ebook::input::Event&ev)：文件 App 的输入总入口。它处理： 触摸按下、移动、抬起和滑动。 工具栏返回。 列表滚动节流。 弹窗点击优先于底层列表点击。。
    shell::InputResult on_input(const ::app::ebook::input::Event& ev) override;
    // 调用 on_ui_event(constui::UiEvent&ev)：处理 UiBus 消息。文件模块最关心的是 FilesReloadDone：收到它以后，在 UI 线程调用 onbrowseready()。。
    void on_ui_event(const ui::UiEvent& ev) override;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 ClockApp() = default;，执行这个函数并处理它的返回值或状态变化。
    ClockApp() = default;

    // 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
    enum class Tab : uint8_t
    {
        // 执行这条代码：Clock = 0,。它会更新变量、对象状态或调用链。
        Clock = 0,
        // 继续上一行的表达式或参数列表：Alarm。
        Alarm
    };
    // 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
    enum class SubPage : uint8_t
    {
        // 执行这条代码：Main = 0,。它会更新变量、对象状态或调用链。
        Main = 0,
        // 继续上一行的表达式或参数列表：AlarmEdit。
        AlarmEdit
    };

    // 函数调用：void set_tab(Tab t);；执行该函数并处理它的返回值或副作用。
    void set_tab(Tab t);
    // 函数调用：void cancel_alarm_edit();；执行该函数并处理它的返回值或副作用。
    void cancel_alarm_edit();
    // 函数调用：void invalidate_body();；执行该函数并处理它的返回值或副作用。
    void invalidate_body();

    // 函数调用：void open_hour_keyboard();；执行该函数并处理它的返回值或副作用。
    void open_hour_keyboard();
    // 函数调用：void open_minute_keyboard();；执行该函数并处理它的返回值或副作用。
    void open_minute_keyboard();
    // 静态函数 on_hour_kb_done：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void on_hour_kb_done(const char* text, void* user);
    // 静态函数 on_minute_kb_done：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void on_minute_kb_done(const char* text, void* user);

    // 函数调用：void paint_tabs(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_tabs(gfx::Canvas& c);
    // 函数调用：void paint_clock_face(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_clock_face(gfx::Canvas& c);
    // 函数调用：void paint_alarm_list(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_alarm_list(gfx::Canvas& c);
    // 函数调用：void paint_alarm_edit(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_alarm_edit(gfx::Canvas& c);
    // 函数调用：void paint_alarm_edit_action_bar(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_alarm_edit_action_bar(gfx::Canvas& c);

    // 函数调用：shell::InputResult handle_tabs(int16_t x, int16_t y);；执行该函数并处理它的返回值或副作用。
    shell::InputResult handle_tabs(int16_t x, int16_t y);
    // 函数调用：shell::InputResult handle_alarm_list(int16_t x, int16_t y);；执行该函数并处理它的返回值或副作用。
    shell::InputResult handle_alarm_list(int16_t x, int16_t y);
    // 函数调用：shell::InputResult handle_alarm_edit(int16_t x, int16_t y);；执行该函数并处理它的返回值或副作用。
    shell::InputResult handle_alarm_edit(int16_t x, int16_t y);

    // 静态函数 tabs_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect tabs_rect();
    // 静态函数 content_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect content_rect();
    // 静态函数 alarm_add_btn_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect alarm_add_btn_rect();
    // 静态函数 alarm_edit_action_bar_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect alarm_edit_action_bar_rect();
    // 函数调用：core::Rect alarm_edit_ok_rect() const;；执行该函数并处理它的返回值或副作用。
    core::Rect alarm_edit_ok_rect() const;
    // 函数调用：core::Rect alarm_edit_delete_rect() const;；执行该函数并处理它的返回值或副作用。
    core::Rect alarm_edit_delete_rect() const;

    // 执行语句：Tab tab_{Tab::Clock};；它会更新一次变量、对象或程序状态。
    Tab tab_{Tab::Clock};
    // 执行语句：SubPage sub_{SubPage::Main};；它会更新一次变量、对象或程序状态。
    SubPage sub_{SubPage::Main};
    // 执行语句：uint8_t edit_alarm_idx_{0};；它会更新一次变量、对象或程序状态。
    uint8_t edit_alarm_idx_{0};
    // 执行语句：bool edit_is_new_{false};；它会更新一次变量、对象或程序状态。
    bool edit_is_new_{false};
    // 声明并默认初始化对象：AlarmEntry edit_snapshot_{}。
    AlarmEntry edit_snapshot_{};

    // 声明并默认初始化对象：ui::ListView alarm_list_{}。
    ui::ListView alarm_list_{};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::clock
