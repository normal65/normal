// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/notepad/notepad_app.hpp
 * 作用：实现记事本编辑、保存、列表和键盘输入。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstddef>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/list_view.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::notepad {

/** @brief 记事本：扫描 notes 目录，列表 + 正文编辑 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class NotepadApp : public App
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static NotepadApp& instance();

    // 定义或声明函数 id：参数在括号里，返回类型写在函数名前面。
    AppId id() const override
    {
        // 返回 AppId::Notepad 给调用者，表示当前函数的执行结果。
        return AppId::Notepad;
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
    // 函数调用：void paint_overlay(gfx::Canvas& canvas) override;；执行该函数并处理它的返回值或副作用。
    void paint_overlay(gfx::Canvas& canvas) override;
    // 调用 on_input(const::app::ebook::input::Event&ev)：文件 App 的输入总入口。它处理： 触摸按下、移动、抬起和滑动。 工具栏返回。 列表滚动节流。 弹窗点击优先于底层列表点击。。
    shell::InputResult on_input(const ::app::ebook::input::Event& ev) override;

    /** 与 scan_dir() 一致，供空状态提示；新建默认写 int */
    // 固定参数 kScanFmtHint = "TXT"：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr const char* kScanFmtHint = "TXT";
    // 固定参数 kScanPathInt = "/int/Ebook/notes"：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr const char* kScanPathInt = "/int/Ebook/notes";
    // 固定参数 kScanPathSd = "/sd/Ebook/notes"：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr const char* kScanPathSd = "/sd/Ebook/notes";

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 NotepadApp();，执行这个函数并处理它的返回值或状态变化。
    NotepadApp();

    // 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
    enum class View : uint8_t
    {
        // 执行这条代码：List = 0,。它会更新变量、对象状态或调用链。
        List = 0,
        // 继续上一行的表达式或参数列表：Edit。
        Edit
    };
    // 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
    enum class KbMode : uint8_t
    {
        // 执行这条代码：None = 0,。它会更新变量、对象状态或调用链。
        None = 0,
        // 继续上一行的表达式或参数列表：NewTitle,。
        NewTitle,
        // 继续上一行的表达式或参数列表：Append,。
        Append,
        // 继续上一行的表达式或参数列表：RenameTitle。
        RenameTitle
    };
    // 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
    enum class Dlg : uint8_t
    {
        // 执行这条代码：None = 0,。它会更新变量、对象状态或调用链。
        None = 0,
        // 继续上一行的表达式或参数列表：Clear,。
        Clear,
        // 继续上一行的表达式或参数列表：More,。
        More,
        // 继续上一行的表达式或参数列表：Delete。
        Delete
    };

    // ---- 文件 / 正文 ----
    void scan_notes();
    // 函数调用：void open_note(uint8_t idx);；执行该函数并处理它的返回值或副作用。
    void open_note(uint8_t idx);
    // 函数调用：void start_new_note();；执行该函数并处理它的返回值或副作用。
    void start_new_note();
    // 函数调用：bool save_note();；执行该函数并处理它的返回值或副作用。
    bool save_note();
    // 函数调用：bool load_note(const char* path);；执行该函数并处理它的返回值或副作用。
    bool load_note(const char* path);
    // 函数调用：bool ensure_notes_dirs();；执行该函数并处理它的返回值或副作用。
    bool ensure_notes_dirs();
    // 函数调用：bool alloc_new_path(char* out, size_t cap);；执行该函数并处理它的返回值或副作用。
    bool alloc_new_path(char* out, size_t cap);
    // 函数调用：void scan_dir(const char* dir);；执行该函数并处理它的返回值或副作用。
    void scan_dir(const char* dir);
    // 静态函数 preferred_notes_dir：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static const char* preferred_notes_dir();
    // 静态函数 parent_dir_of：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool parent_dir_of(const char* path, char* out, size_t cap);
    // 函数调用：void append_text(const char* text);；执行该函数并处理它的返回值或副作用。
    void append_text(const char* text);
    // 函数调用：void delete_last_char();；执行该函数并处理它的返回值或副作用。
    void delete_last_char();
    // 函数调用：void clear_body();；执行该函数并处理它的返回值或副作用。
    void clear_body();
    // 函数调用：bool rename_note_to_stem(const char* stem);；执行该函数并处理它的返回值或副作用。
    bool rename_note_to_stem(const char* stem);
    // 函数调用：bool delete_current_note();；执行该函数并处理它的返回值或副作用。
    bool delete_current_note();
    // 函数调用：void abort_edit_to_list();；执行该函数并处理它的返回值或副作用。
    void abort_edit_to_list();
    // 函数调用：bool leave_edit_to_list();；执行该函数并处理它的返回值或副作用。
    bool leave_edit_to_list();
    // 函数调用：uint8_t edit_max_scroll() const;；执行该函数并处理它的返回值或副作用。
    uint8_t edit_max_scroll() const;
    // 函数调用：void clamp_scroll();；执行该函数并处理它的返回值或副作用。
    void clamp_scroll();

    // ---- 列表 ----
    void fill_row(uint8_t idx, ui::widgets::RowStyle& rs);
    // 调用 on_row_tap(uint8_tidx)：处理列表点击： 如果点击父目录行，调用 goup()。 如果点击根卷，调用 openroot() 并调度扫描。 如果点击目录，调用 enterdir()。 如果点击文件，调用 showfileaction()。。
    void on_row_tap(uint8_t idx);

    // ---- 绘制 ----
    void paint_edit(gfx::Canvas& c);
    // 函数调用：void paint_action_bar(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_action_bar(gfx::Canvas& c);
    // 函数调用：void paint_clear_dialog(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_clear_dialog(gfx::Canvas& c);
    // 函数调用：void paint_more_dialog(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_more_dialog(gfx::Canvas& c);
    // 调用 paint_delete_dialog(gfx::Canvas&c)：画删除确认。删除是不可逆操作，所以这里必须有确认步骤，不能点击一次就立即删。。
    void paint_delete_dialog(gfx::Canvas& c);

    // ---- 输入 ----
    shell::InputResult handle_edit_input(int16_t x, int16_t y);
    // 函数调用：shell::InputResult handle_dialog_input(int16_t x, int16_t y);；执行该函数并处理它的返回值或副作用。
    shell::InputResult handle_dialog_input(int16_t x, int16_t y);

    // ---- 对话框状态 ----
    void show_more_menu();
    // 函数调用：void show_clear_dialog();；执行该函数并处理它的返回值或副作用。
    void show_clear_dialog();
    // 函数调用：void show_delete_dialog();；执行该函数并处理它的返回值或副作用。
    void show_delete_dialog();
    // 调用 close_dialog()：把 dlg 设回 None，清除选中信息和信息行。它不负责重扫目录，因为关闭弹窗通常只是恢复界面。。
    void close_dialog();
    // 函数调用：void dismiss_dialog_back();；执行该函数并处理它的返回值或副作用。
    void dismiss_dialog_back();

    // ---- 键盘 ----
    void open_append_keyboard();
    // 函数调用：void open_rename_keyboard();；执行该函数并处理它的返回值或副作用。
    void open_rename_keyboard();
    // 静态函数 on_title_kb_done：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void on_title_kb_done(const char* text, void* user);
    // 静态函数 on_append_kb_done：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void on_append_kb_done(const char* text, void* user);
    // 静态函数 on_rename_kb_done：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void on_rename_kb_done(const char* text, void* user);

    // 静态函数 title_from_path：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void title_from_path(const char* path, char* out, size_t out_size);
    // 静态函数 sanitize_filename：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void sanitize_filename(const char* in, char* out, size_t out_size);

    // ---- 布局几何 ----
    static core::Rect edit_body_rect();
    // 静态函数 action_bar_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect action_bar_rect();
    // 静态函数 action_btn_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect action_btn_rect(uint8_t idx);
    // 静态函数 confirm_dialog_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect confirm_dialog_rect();
    // 静态函数 more_dialog_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect more_dialog_rect();
    // 静态函数 more_dialog_btn_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect more_dialog_btn_rect(uint8_t idx);

    // 执行语句：View view_{View::List};；它会更新一次变量、对象或程序状态。
    View view_{View::List};
    // 执行语句：KbMode kb_mode_{KbMode::None};；它会更新一次变量、对象或程序状态。
    KbMode kb_mode_{KbMode::None};
    // 执行语句：Dlg dlg_{Dlg::None};；它会更新一次变量、对象或程序状态。
    Dlg dlg_{Dlg::None};
    // 执行语句：uint8_t sel_{0};；它会更新一次变量、对象或程序状态。
    uint8_t sel_{0};
    // 执行语句：bool dirty_{false};；它会更新一次变量、对象或程序状态。
    bool dirty_{false};
    // 执行语句：uint8_t scroll_line_{0};；它会更新一次变量、对象或程序状态。
    uint8_t scroll_line_{0};

    // struct NoteItem：把相关数据字段组合在一起，表示项目中的一个概念。
    struct NoteItem
    {
        // 执行语句：char path[128]{};；它会更新一次变量、对象或程序状态。
        char path[128]{};
        // 执行语句：char title[40]{};；它会更新一次变量、对象或程序状态。
        char title[40]{};
        // 执行语句：uint32_t size{0};；它会更新一次变量、对象或程序状态。
        uint32_t size{0};
    };

    // 固定参数 kMaxNotes = 32：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kMaxNotes = 32;
    // 固定参数 kBodyCap = 4096：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kBodyCap = 4096;
    // 固定参数 kWrapLines = 48：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kWrapLines = 48;
    // 固定参数 kActionBarH = 30：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kActionBarH = 30;
    // 固定参数 kActionBtnNum = 4：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kActionBtnNum = 4;

    // 固定参数 kConfirmDlgW = 152：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kConfirmDlgW = 152;
    // 固定参数 kConfirmDlgH = 76：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kConfirmDlgH = 76;

    // 固定参数 kMoreDlgW = 152：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kMoreDlgW = 152;
    // 固定参数 kMoreDlgH = 108：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kMoreDlgH = 108;
    // 固定参数 kMoreBtnNum = 3：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kMoreBtnNum = 3;
    // 固定参数 kMoreBtnH = 28：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kMoreBtnH = 28;
    // 固定参数 kMoreTitleH = 24：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kMoreTitleH = 24;

    // 函数调用：bool ensure_store();；执行该函数并处理它的返回值或副作用。
    bool ensure_store();

    // 执行语句：NoteItem* notes_{nullptr};；它会更新一次变量、对象或程序状态。
    NoteItem* notes_{nullptr};
    // 调用 char (*size_buf_)[12]{nullptr};，执行这个函数并处理它的返回值或状态变化。
    char (*size_buf_)[12]{nullptr};
    // 执行语句：uint8_t count_{0};；它会更新一次变量、对象或程序状态。
    uint8_t count_{0};

    // 执行语句：char path_[128]{};；它会更新一次变量、对象或程序状态。
    char path_[128]{};
    // 执行语句：char title_buf_[40]{};；它会更新一次变量、对象或程序状态。
    char title_buf_[40]{};
    // 执行语句：char* body_{nullptr};；它会更新一次变量、对象或程序状态。
    char* body_{nullptr};

    // 执行语句：ui::ListView list_;；它会更新一次变量、对象或程序状态。
    ui::ListView list_;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::notepad
