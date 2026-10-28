// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/wooden_fish/wooden_fish_app.hpp
 * 作用：实现电子木鱼点击、计数和音效播放。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_timer.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_event.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::wooden_fish {

/** @brief 电子木鱼：敲击动画经 UiBus 投递到 UI 线程刷新 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class WoodenFishApp : public App
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static WoodenFishApp& instance();

    // 定义或声明函数 id：参数在括号里，返回类型写在函数名前面。
    AppId id() const override
    {
        // 返回 AppId::WoodenFish 给调用者，表示当前函数的执行结果。
        return AppId::WoodenFish;
    }
    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const char* title() const override;
    // 调用 icon_cp()：返回图标 codepoint。图标通常来自字体图标表，所以这里返回的是 uint32t，不是图片路径。。
    uint32_t icon_cp() const override;

    // 调用 on_enter()：进入文件 App 时： 1. 重置浏览状态或恢复根卷状态。 2. 清空弹窗。 3. 设置加载状态。 4. 调度目录扫描。 5. 请求重绘。。
    void on_enter() override;
    // 调用 on_exit()：退出时取消未完成的扫描、关闭弹窗，并确保后台任务不会在对象状态失效后继续写数据。。
    void on_exit() override;
    // 函数调用：void paint(gfx::Canvas& canvas) override;；执行该函数并处理它的返回值或副作用。
    void paint(gfx::Canvas& canvas) override;
    // 调用 on_input(const::app::ebook::input::Event&ev)：文件 App 的输入总入口。它处理： 触摸按下、移动、抬起和滑动。 工具栏返回。 列表滚动节流。 弹窗点击优先于底层列表点击。。
    shell::InputResult on_input(const ::app::ebook::input::Event& ev) override;
    // 调用 on_ui_event(constui::UiEvent&ev)：处理 UiBus 消息。文件模块最关心的是 FilesReloadDone：收到它以后，在 UI 线程调用 onbrowseready()。。
    void on_ui_event(const ui::UiEvent& ev) override;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 WoodenFishApp() = default;，执行这个函数并处理它的返回值或状态变化。
    WoodenFishApp() = default;

    // 函数调用：void on_knock();；执行该函数并处理它的返回值或副作用。
    void on_knock();
    // 函数调用：void schedule_repaint(bool force);；执行该函数并处理它的返回值或副作用。
    void schedule_repaint(bool force);
    // 函数调用：void arm_anim_timer();；执行该函数并处理它的返回值或副作用。
    void arm_anim_timer();
    // 函数调用：void ensure_anim_timer();；执行该函数并处理它的返回值或副作用。
    void ensure_anim_timer();

    // 静态函数 body_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect body_rect();
    // 静态函数 fish_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect fish_rect();
    // 静态函数 merit_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect merit_rect();

    // struct StickPivot：把相关数据字段组合在一起，表示项目中的一个概念。
    struct StickPivot
    {
        // 执行语句：int16_t x{0};；它会更新一次变量、对象或程序状态。
        int16_t x{0};
        // 执行语句：int16_t y{0};；它会更新一次变量、对象或程序状态。
        int16_t y{0};
    };

    // 静态函数 stick_pivot：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static StickPivot stick_pivot();
    // 静态函数 paint_stick：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void paint_stick(gfx::Canvas& c, bool knocking);

    // 执行语句：uint32_t merit_{0};；它会更新一次变量、对象或程序状态。
    uint32_t merit_{0};
    // 执行语句：int64_t knock_anim_until_ms_{0};；它会更新一次变量、对象或程序状态。
    int64_t knock_anim_until_ms_{0};
    // 执行语句：int64_t last_repaint_ms_{0};；它会更新一次变量、对象或程序状态。
    int64_t last_repaint_ms_{0};
    // 执行语句：bool pending_repaint_{false};；它会更新一次变量、对象或程序状态。
    bool pending_repaint_{false};
    // 执行语句：esp_timer_handle_t anim_timer_{nullptr};；它会更新一次变量、对象或程序状态。
    esp_timer_handle_t anim_timer_{nullptr};

    // 固定参数 kFishIconSize = 56：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kFishIconSize = 56;
    // 固定参数 kStickIconSize = 36：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kStickIconSize = 36;
    // 固定参数 kKnockAnimMs = 160：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr int64_t kKnockAnimMs = 160;
    // 固定参数 kRepaintMinMs = 90：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr int64_t kRepaintMinMs = 90;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::wooden_fish
