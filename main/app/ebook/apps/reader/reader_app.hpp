// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/reader/reader_app.hpp
 * 作用：实现阅读器页面、分页、阅读进度和阅读菜单入口。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/book_shelf.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/reader_layout.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/reader_menu.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/txt_book.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "shell/page.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/list_view.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::reader {

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class ReadRefreshMode : uint8_t
{
    // 执行这条代码：Clear = 0,。它会更新变量、对象状态或调用链。
    Clear = 0,
    // 继续上一行的表达式或参数列表：Normal = 1,。
    Normal = 1,
    // 继续上一行的表达式或参数列表：Fast = 2,。
    Fast = 2,
};

/** @brief 阅读器：书架 → 阅读（顶栏 + 底栏菜单） */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class ReaderApp : public App
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static ReaderApp& instance();

    /** 主页「继续阅读」：下次 on_enter 直达上次书籍 */
    // 静态函数 request_resume_on_enter：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void request_resume_on_enter();
    /** 文件管理「打开」：下次 on_enter 按路径开书 */
    // 静态函数 request_open_on_enter：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void request_open_on_enter(const char* path);

    // 定义或声明函数 id：参数在括号里，返回类型写在函数名前面。
    AppId id() const override
    {
        // 返回 AppId::Reader 给调用者，表示当前函数的执行结果。
        return AppId::Reader;
    }
    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const char* title() const override;
    // 调用 icon_cp()：返回图标 codepoint。图标通常来自字体图标表，所以这里返回的是 uint32t，不是图片路径。。
    uint32_t icon_cp() const override;

    // 函数调用：bool wants_status_bar() const override;；执行该函数并处理它的返回值或副作用。
    bool wants_status_bar() const override;

    // 调用 on_enter()：进入文件 App 时： 1. 重置浏览状态或恢复根卷状态。 2. 清空弹窗。 3. 设置加载状态。 4. 调度目录扫描。 5. 请求重绘。。
    void on_enter() override;
    // 调用 on_exit()：退出时取消未完成的扫描、关闭弹窗，并确保后台任务不会在对象状态失效后继续写数据。。
    void on_exit() override;
    // 调用 on_ui_event(constui::UiEvent&ev)：处理 UiBus 消息。文件模块最关心的是 FilesReloadDone：收到它以后，在 UI 线程调用 onbrowseready()。。
    void on_ui_event(const ui::UiEvent& ev) override;
    // 函数调用：void paint(gfx::Canvas& canvas) override;；执行该函数并处理它的返回值或副作用。
    void paint(gfx::Canvas& canvas) override;
    // 调用 on_input(const::app::ebook::input::Event&ev)：文件 App 的输入总入口。它处理： 触摸按下、移动、抬起和滑动。 工具栏返回。 列表滚动节流。 弹窗点击优先于底层列表点击。。
    shell::InputResult on_input(const ::app::ebook::input::Event& ev) override;
    // 调用 on_semantic_back()：处理语义返回，顺序通常是： 1. 如果弹窗打开，先关闭弹窗。 2. 如果列表还在浏览目录，先返回上一级。 3. 如果已经到根卷，返回 false，让 Shell 退出 App。。
    bool on_semantic_back() override;

    // 函数调用：void toggle_reading_menu();；执行该函数并处理它的返回值或副作用。
    void toggle_reading_menu();

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 ReaderApp();，执行这个函数并处理它的返回值或状态变化。
    ReaderApp();

    // 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
    enum class Screen : uint8_t
    {
        // 执行这条代码：Shelf,。它会更新变量、对象状态或调用链。
        Shelf,
        // 继续上一行的表达式或参数列表：Opening,。
        Opening,
        // 继续上一行的表达式或参数列表：Reading,。
        Reading,
    };

    // 函数调用：void after_page_turn();；执行该函数并处理它的返回值或副作用。
    void after_page_turn();
    // 函数调用：void save_progress();；执行该函数并处理它的返回值或副作用。
    void save_progress();
    // 函数调用：void restore_progress();；执行该函数并处理它的返回值或副作用。
    void restore_progress();
    // 函数调用：void return_to_shelf();；执行该函数并处理它的返回值或副作用。
    void return_to_shelf();
    // 函数调用：void go_lock_screen();；执行该函数并处理它的返回值或副作用。
    void go_lock_screen();

    // 函数调用：bool open_book(uint8_t shelf_idx);；执行该函数并处理它的返回值或副作用。
    bool open_book(uint8_t shelf_idx);
    // 函数调用：bool open_book_by_path(const char* path);；执行该函数并处理它的返回值或副作用。
    bool open_book_by_path(const char* path);
    // 函数调用：bool try_resume_saved_book();；执行该函数并处理它的返回值或副作用。
    bool try_resume_saved_book();

    // 函数调用：void cycle_font_scale(int dir);；执行该函数并处理它的返回值或副作用。
    void cycle_font_scale(int dir);
    // 函数调用：void cycle_refresh_mode(int dir);；执行该函数并处理它的返回值或副作用。
    void cycle_refresh_mode(int dir);
    // 函数调用：void apply_font_change();；执行该函数并处理它的返回值或副作用。
    void apply_font_change();
    // 函数调用：void open_page_jump();；执行该函数并处理它的返回值或副作用。
    void open_page_jump();
    // 静态函数 on_page_jump_done：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void on_page_jump_done(const char* text, void* user);

    // 函数调用：void go_next_page();；执行该函数并处理它的返回值或副作用。
    void go_next_page();
    // 函数调用：void go_prev_page();；执行该函数并处理它的返回值或副作用。
    void go_prev_page();
    // 函数调用：void go_page_relative(int delta);；执行该函数并处理它的返回值或副作用。
    void go_page_relative(int delta);
    // 函数调用：void go_to_page(uint16_t page_index);；执行该函数并处理它的返回值或副作用。
    void go_to_page(uint16_t page_index);

    // 函数调用：void paint_shelf(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_shelf(gfx::Canvas& c);
    // 函数调用：void paint_opening(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_opening(gfx::Canvas& c);
    // 函数调用：void paint_reading(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_reading(gfx::Canvas& c);
    // 函数调用：void paint_top_bar(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_top_bar(gfx::Canvas& c);

    // 调用 fill_shelf_row(uint8_tidx,ui::widgets::RowStyle&rs)：给 ListView 的某一行填入样式和内容。它负责： 处理“返回上一级”行。 处理根卷行。 处理普通目录行。 处理普通文件行。 显示截断或加载提示。。
    void fill_shelf_row(uint8_t idx, ui::widgets::RowStyle& rs);
    // 函数调用：void on_shelf_tap(uint8_t idx);；执行该函数并处理它的返回值或副作用。
    void on_shelf_tap(uint8_t idx);
    // 函数调用：void handle_menu_hit(MenuHit hit);；执行该函数并处理它的返回值或副作用。
    void handle_menu_hit(MenuHit hit);

    // 函数调用：bool handle_reading_tap(int16_t x, int16_t y);；执行该函数并处理它的返回值或副作用。
    bool handle_reading_tap(int16_t x, int16_t y);
    // 函数调用：bool in_reading_content(int16_t x, int16_t y) const;；执行该函数并处理它的返回值或副作用。
    bool in_reading_content(int16_t x, int16_t y) const;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* font_label() const;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* refresh_label() const;
    // 函数调用：ReaderLayout layout() const;；执行该函数并处理它的返回值或副作用。
    ReaderLayout layout() const;

    // 模块私有静态对象 s_resume_on_enter_：只在当前文件或函数内共享。
    static bool s_resume_on_enter_;
    // 模块私有静态对象 s_open_on_enter_：只在当前文件或函数内共享。
    static bool s_open_on_enter_;
    // 静态字符缓冲区 s_open_path_：保存临时文本，跨函数调用复用。
    static char s_open_path_[96];

    // 执行语句：Screen screen_{Screen::Shelf};；它会更新一次变量、对象或程序状态。
    Screen screen_{Screen::Shelf};
    // 执行语句：BookShelf shelf_;；它会更新一次变量、对象或程序状态。
    BookShelf shelf_;
    // 执行语句：TxtBook book_;；它会更新一次变量、对象或程序状态。
    TxtBook book_;
    // 执行语句：ui::ListView list_;；它会更新一次变量、对象或程序状态。
    ui::ListView list_;

    // 执行语句：uint8_t font_scale_{1};；它会更新一次变量、对象或程序状态。
    uint8_t font_scale_{1};
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    uint8_t refresh_mode_{static_cast<uint8_t>(ReadRefreshMode::Normal)};
    /** 本次阅读累计翻页次数（用于分级刷新） */
    // 执行语句：uint16_t turn_count_{0};；它会更新一次变量、对象或程序状态。
    uint16_t turn_count_{0};
    // 执行语句：bool menu_visible_{false};；它会更新一次变量、对象或程序状态。
    bool menu_visible_{false};

    // 执行语句：uint8_t tick_hour_{0xFF};；它会更新一次变量、对象或程序状态。
    uint8_t tick_hour_{0xFF};
    // 执行语句：uint8_t tick_minute_{0xFF};；它会更新一次变量、对象或程序状态。
    uint8_t tick_minute_{0xFF};
    // 执行语句：uint8_t tick_battery_{0xFF};；它会更新一次变量、对象或程序状态。
    uint8_t tick_battery_{0xFF};

    // 执行语句：char open_title_[48]{};；它会更新一次变量、对象或程序状态。
    char open_title_[48]{};
    // 执行语句：char open_path_[96]{};；它会更新一次变量、对象或程序状态。
    char open_path_[96]{};
    // 执行语句：char row_title_[48]{};；它会更新一次变量、对象或程序状态。
    char row_title_[48]{};
    // 执行语句：char row_value_[16]{};；它会更新一次变量、对象或程序状态。
    char row_value_[16]{};
    // 执行语句：char jump_kb_title_[32]{};；它会更新一次变量、对象或程序状态。
    char jump_kb_title_[32]{};
    // 执行语句：mutable char title_buf_[32]{};；它会更新一次变量、对象或程序状态。
    mutable char title_buf_[32]{};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::reader
