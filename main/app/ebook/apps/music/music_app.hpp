// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/music/music_app.hpp
 * 作用：实现音乐列表、播放控制、进度和歌曲信息显示。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <memory>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/task/task.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/list_view.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_event.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::music {

/**
 * @brief 音乐：Library / Player；进度经 SystemHint::MusicProgress 局刷
 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class MusicApp : public App
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static MusicApp& instance();

    // 静态函数 request_play_on_enter：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void request_play_on_enter(const char* path);

    // 定义或声明函数 id：参数在括号里，返回类型写在函数名前面。
    AppId id() const override
    {
        // 返回 AppId::Music 给调用者，表示当前函数的执行结果。
        return AppId::Music;
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
    // 调用 MusicApp();，执行这个函数并处理它的返回值或状态变化。
    MusicApp();

    // 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
    enum class View : uint8_t
    {
        // 执行这条代码：Library = 0,。它会更新变量、对象状态或调用链。
        Library = 0,
        // 继续上一行的表达式或参数列表：Player。
        Player
    };

    // 函数调用：void set_view(View v);；执行该函数并处理它的返回值或副作用。
    void set_view(View v);
    // 函数调用：void start_scan();；执行该函数并处理它的返回值或副作用。
    void start_scan();
    // 函数调用：void play_index(uint8_t idx);；执行该函数并处理它的返回值或副作用。
    void play_index(uint8_t idx);
    // 函数调用：void play_adjacent(int8_t delta);；执行该函数并处理它的返回值或副作用。
    void play_adjacent(int8_t delta);

    // 静态函数 player_body_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect player_body_rect();
    // 静态函数 player_controls_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect player_controls_rect();
    // 静态函数 ctrl_btn_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect ctrl_btn_rect(uint8_t idx);

    // 函数调用：void paint_library(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_library(gfx::Canvas& c);
    // 函数调用：void paint_player(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_player(gfx::Canvas& c);

    // 函数调用：shell::InputResult handle_player(int16_t x, int16_t y);；执行该函数并处理它的返回值或副作用。
    shell::InputResult handle_player(int16_t x, int16_t y);

    // 执行语句：View view_{View::Library};；它会更新一次变量、对象或程序状态。
    View view_{View::Library};
    // 执行语句：uint8_t track_idx_{0};；它会更新一次变量、对象或程序状态。
    uint8_t track_idx_{0};
    // 执行语句：bool scanning_{false};；它会更新一次变量、对象或程序状态。
    bool scanning_{false};

    // 执行语句：std::unique_ptr<::app::sys::task::Task> scan_task_;；它会更新一次变量、对象或程序状态。
    std::unique_ptr<::app::sys::task::Task> scan_task_;
    // 声明并默认初始化对象：ui::ListView list_{}。
    ui::ListView list_{};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::music
