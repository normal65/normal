// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/files/files_app.hpp
 * 作用：实现文件管理界面、目录浏览、打开、删除确认和加载状态。
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
#include "apps/files/vfs_browser.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/task/task.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/list_view.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_event.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::files {

// 类定义：把数据和操作这些数据的方法绑定在一起。
class FilesApp : public App
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static FilesApp& instance();

    // 定义或声明函数 id：参数在括号里，返回类型写在函数名前面。
    AppId id() const override
    {
        // 返回 AppId::Files 给调用者，表示当前函数的执行结果。
        return AppId::Files;
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
    // 调用 on_ui_event(constui::UiEvent&ev)：处理 UiBus 消息。文件模块最关心的是 FilesReloadDone：收到它以后，在 UI 线程调用 onbrowseready()。。
    void on_ui_event(const ui::UiEvent& ev) override;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 FilesApp()：构造时主要初始化成员和缓存指针。它不应该在构造函数里做长时间扫描，因为单例可能在启动早期创建，过早扫盘会拖慢启动。。
    FilesApp();

    // 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
    enum class FileDlg : uint8_t
    {
        // 执行这条代码：None = 0,。它会更新变量、对象状态或调用链。
        None = 0,
        // 继续上一行的表达式或参数列表：Action,。
        Action,
        // 继续上一行的表达式或参数列表：Info,。
        Info,
        // 继续上一行的表达式或参数列表：DeleteConfirm,。
        DeleteConfirm,
    };

    // 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
    enum class OpenKind : uint8_t
    {
        // 执行这条代码：Unsupported = 0,。它会更新变量、对象状态或调用链。
        Unsupported = 0,
        // 继续上一行的表达式或参数列表：Txt,。
        Txt,
        // 继续上一行的表达式或参数列表：Bmp,。
        Bmp,
        // 继续上一行的表达式或参数列表：Jpeg,。
        Jpeg,
        // 继续上一行的表达式或参数列表：Audio,。
        Audio,
    };

    // 调用 sync_list_total()：把 ListView 的总行数同步为 browser.rowcount()。当根卷数量变化、进入目录或删除文件后都要调用，否则列表的滚动范围会比实际行数多或少。。
    void sync_list_total();
    // 调用 close_dialog()：把 dlg 设回 None，清除选中信息和信息行。它不负责重扫目录，因为关闭弹窗通常只是恢复界面。。
    void close_dialog();
    // 调用 rebuild_row_cache()：遍历当前所有列表行，调用 fillrow() 填充缓存。它通常发生在目录结果 adopt 后。。
    void rebuild_row_cache();
    // 调用 on_browse_ready()：目录扫描完成后，在 UI 线程里把结果交给浏览器、重建缓存、更新列表总数、清除 loading 状态，并请求重绘。。
    void on_browse_ready();
    // 调用 finish_browse_reload()：完成本次重载的清理工作，例如释放任务对象、清除取消标记。它把“状态改变”和“资源回收”分开，便于排查任务为什么没有结束。。
    void finish_browse_reload();
    // 调用 schedule_browse_reload()：调度后台目录扫描。关键实现细节： 任务名是 filesreload。 优先级是 LOW。 栈大小是 6144。 栈使用 PSRAM。 结果放在 greloadjob。 greloadmtx 保护任务和结果对象。 完成后用 UiBus::SystemHint::FilesReloadDone 通知 UI。。
    void schedule_browse_reload();
    // 调用 cancel_browse_reload()：取消或停止当前浏览重载。它需要避免旧任务完成后把已经过期的目录结果 adopt 到 UI。。
    void cancel_browse_reload();
    // 调用 request_scroll_repaint()：限制滚动时的重绘频率。代码使用约 120 ms 的节流值，避免用户连续滑动时每个触摸事件都触发一次墨水屏刷新。。
    void request_scroll_repaint();

    // 调用 fill_row(uint8_tidx,ui::widgets::RowStyle&rs)：给 ListView 的某一行填入样式和内容。它负责： 处理“返回上一级”行。 处理根卷行。 处理普通目录行。 处理普通文件行。 显示截断或加载提示。。
    void fill_row(uint8_t idx, ui::widgets::RowStyle& rs);
    // 调用 on_row_tap(uint8_tidx)：处理列表点击： 如果点击父目录行，调用 goup()。 如果点击根卷，调用 openroot() 并调度扫描。 如果点击目录，调用 enterdir()。 如果点击文件，调用 showfileaction()。。
    void on_row_tap(uint8_t idx);
    // 调用 handle_toolbar_back()：处理工具栏返回按钮。它的优先级和语义返回类似：先关弹窗，再回上一级，最后返回 false 让 Shell 退出。。
    bool handle_toolbar_back();

    // 静态函数 classify_open：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static OpenKind classify_open(const char* name);
    // 静态函数 type_label：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static const char* type_label(OpenKind kind, const char* name);
    // 静态函数 ends_with_ci：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool ends_with_ci(const char* name, const char* ext);
    // 静态函数 display_name：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void display_name(const char* raw, char* out, size_t out_cap, uint16_t max_w);
    // 静态函数 row_icon：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static uint32_t row_icon(const VfsBrowser::Entry& e);

    // 静态函数 file_dlg_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect file_dlg_rect(FileDlg dlg);
    // 静态函数 file_dlg_close_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect file_dlg_close_rect(FileDlg dlg);
    // 静态函数 file_action_btn_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect file_action_btn_rect(uint8_t idx);
    // 静态函数 file_info_back_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect file_info_back_rect();

    // 调用 paint_file_dialog(gfx::Canvas&c)：按 dlg 分发到具体弹窗绘制。它相当于弹窗的总入口。。
    void paint_file_dialog(gfx::Canvas& c);
    // 调用 paint_dlg_close_btn(gfx::Canvas&c,FileDlgdlg)：画弹窗关闭按钮，并可能处理按下或禁用视觉状态。。
    void paint_dlg_close_btn(gfx::Canvas& c, FileDlg dlg);
    // 调用 paint_action_dialog(gfx::Canvas&c)：画文件操作弹窗：打开、删除等动作，以及顶部关闭按钮。按钮布局必须和 fileactionbtnrect() 对应。。
    void paint_action_dialog(gfx::Canvas& c);
    // 调用 paint_info_dialog(gfx::Canvas&c)：画信息弹窗，把 buildinfolines() 生成的内容排到固定区域。。
    void paint_info_dialog(gfx::Canvas& c);
    // 调用 paint_delete_dialog(gfx::Canvas&c)：画删除确认。删除是不可逆操作，所以这里必须有确认步骤，不能点击一次就立即删。。
    void paint_delete_dialog(gfx::Canvas& c);

    // 调用 handle_dialog_tap(int16_tx,int16_ty)：把触摸坐标分发到当前弹窗的按钮。它先判断弹窗类型，再用前面那些 rect() 做命中。。
    shell::InputResult handle_dialog_tap(int16_t x, int16_t y);
    // 调用 show_file_action(uint8_tentry_idx)：用户点击文件后打开操作弹窗。它保存选中条目、路径和系统文件标记，然后设置 dlg。。
    void show_file_action(uint8_t entry_idx);
    // 调用 open_selected_file()：根据选中文件的 OpenKind 路由到其他 App： 1. 通过对应 App 的请求接口传递路径，例如 requestopenonenter() 或 requestplayonenter()。 2. 调用 Router 切换到目标 App。。
    void open_selected_file();
    // 调用 delete_selected_file()：执行删除前要确保： 不是目录。 不是系统文件。 路径已经由 VfsBrowser 校验。 删除失败时给出用户可见反馈。。
    void delete_selected_file();
    // 调用 build_info_lines()：生成最多四行信息文本，通常包含： 文件名。 类型。 大小。 路径或系统提示。。
    void build_info_lines();

    // 执行语句：VfsBrowser browser_;；它会更新一次变量、对象或程序状态。
    VfsBrowser browser_;
    // 执行语句：ui::ListView list_;；它会更新一次变量、对象或程序状态。
    ui::ListView list_;

    // 执行语句：FileDlg dlg_{FileDlg::None};；它会更新一次变量、对象或程序状态。
    FileDlg dlg_{FileDlg::None};
    // 执行语句：uint8_t sel_idx_{0};；它会更新一次变量、对象或程序状态。
    uint8_t sel_idx_{0};
    // 执行语句：char sel_path_[VfsBrowser::kPathCap]{};；它会更新一次变量、对象或程序状态。
    char sel_path_[VfsBrowser::kPathCap]{};
    // 声明并默认初始化对象：VfsBrowser::Entry sel_entry_{}。
    VfsBrowser::Entry sel_entry_{};
    // 执行语句：bool sel_is_system_{false};；它会更新一次变量、对象或程序状态。
    bool sel_is_system_{false};

    // 执行语句：bool loading_{false};；它会更新一次变量、对象或程序状态。
    bool loading_{false};
    // 执行语句：int64_t last_scroll_repaint_ms_{0};；它会更新一次变量、对象或程序状态。
    int64_t last_scroll_repaint_ms_{0};
    // 执行语句：std::unique_ptr<::app::sys::task::Task> reload_task_;；它会更新一次变量、对象或程序状态。
    std::unique_ptr<::app::sys::task::Task> reload_task_;

    // 调用 char (*row_label_)[VfsBrowser::kNameCap]{nullptr};，执行这个函数并处理它的返回值或状态变化。
    char (*row_label_)[VfsBrowser::kNameCap]{nullptr};
    // 调用 char (*row_value_)[16]{nullptr};，执行这个函数并处理它的返回值或状态变化。
    char (*row_value_)[16]{nullptr};
    // 执行语句：uint32_t* row_icon_cp_{nullptr};；它会更新一次变量、对象或程序状态。
    uint32_t* row_icon_cp_{nullptr};

    // 执行语句：char dlg_title_[40]{};；它会更新一次变量、对象或程序状态。
    char dlg_title_[40]{};
    // 执行语句：char info_lines_[4][64]{};；它会更新一次变量、对象或程序状态。
    char info_lines_[4][64]{};
    // 执行语句：mutable char title_buf_[32]{};；它会更新一次变量、对象或程序状态。
    mutable char title_buf_[32]{};

    // 调用 ensure_row_cache()：按需分配三组行缓存： rowlabel：显示名称。 rowvalue：大小文本。 rowiconcp：图标。。
    bool ensure_row_cache();
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::files
