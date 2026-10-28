/**
 * [初学者导读] 文件：main/app/ebook/apps/files/files_app.cc
 * 作用：实现文件管理界面、目录浏览、打开、删除确认和加载状态。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/files/files_app.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <mutex>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app_id.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/drawing/drawing_app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/gallery/gallery_app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/music/music_app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/reader_app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/icon.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/text_layout.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/toast.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/page_id.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/router.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "storage/storage.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "text/path_encoding.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/dialog.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_bus.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_heap_caps.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "memory/psram_alloc.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::files {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using app::common::memory::alloc_array_psram;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using app::common::memory::alloc_psram_prefer;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::dialog::ConfirmHit;

// 固定参数 kFileDlgW = 168：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kFileDlgW = 168;
// 固定参数 kFileDlgActionH = 92：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kFileDlgActionH = 92;
// 固定参数 kFileDlgInfoH = 118：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kFileDlgInfoH = 118;
// 固定参数 kFileDlgDeleteH = 76：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kFileDlgDeleteH = 76;
// 固定参数 kFileBtnH = 28：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kFileBtnH = 28;
// 固定参数 kActionBtnNum = 3：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kActionBtnNum = 3;
// 任务栈大小：6144，栈太小会崩溃，太大会浪费 RAM。
constexpr uint32_t kReloadStack = 6144;
// 固定参数 kScrollRepaintMs = 120：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr int64_t kScrollRepaintMs = 120;

// struct ReloadJob：把相关数据字段组合在一起，表示项目中的一个概念。
struct ReloadJob
{
    // 执行语句：char cwd[VfsBrowser::kPathCap]{};；它会更新一次变量、对象或程序状态。
    char cwd[VfsBrowser::kPathCap]{};
    // 执行语句：VfsBrowser::Entry entries[VfsBrowser::kMaxEntries]{};；它会更新一次变量、对象或程序状态。
    VfsBrowser::Entry entries[VfsBrowser::kMaxEntries]{};
    // 执行语句：uint8_t count{0};；它会更新一次变量、对象或程序状态。
    uint8_t count{0};
    // 执行语句：bool truncated{false};；它会更新一次变量、对象或程序状态。
    bool truncated{false};
    // 执行语句：bool has_parent{false};；它会更新一次变量、对象或程序状态。
    bool has_parent{false};
};

// 赋值：把右侧 nullptr 的结果保存到左侧 ReloadJob* g_reload_job。
ReloadJob* g_reload_job = nullptr;
// 执行语句：std::mutex g_reload_mtx;；它会更新一次变量、对象或程序状态。
std::mutex g_reload_mtx;

// 定义或声明函数 ensure_reload_job：参数在括号里，返回类型写在函数名前面。
bool ensure_reload_job()
{
    // 判断条件：g_reload_job != nullptr 是否成立。
    if (g_reload_job != nullptr)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    g_reload_job = static_cast<ReloadJob*>(
        // 继续上一行的表达式或参数列表：heap_caps_calloc(1, sizeof(ReloadJob), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));。
        heap_caps_calloc(1, sizeof(ReloadJob), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    // 判断条件：g_reload_job == nullptr 是否成立。
    if (g_reload_job == nullptr)
    {
        // 执行这条代码：g_reload_job =。它会更新变量、对象状态或调用链。
        g_reload_job =
            // 继续上一行的表达式或参数列表：static_cast<ReloadJob*>(heap_caps_calloc(1, sizeof(ReloadJob), MALLOC_CAP_8BIT));。
            static_cast<ReloadJob*>(heap_caps_calloc(1, sizeof(ReloadJob), MALLOC_CAP_8BIT));
    }
    // 返回 g_reload_job != nullptr 给调用者，表示当前函数的执行结果。
    return g_reload_job != nullptr;
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 FilesApp::instance：参数在括号里，返回类型写在函数名前面。
FilesApp& FilesApp::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static FilesApp s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// [初学者导读] FilesApp::FilesApp()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
FilesApp::FilesApp()
{
    // 调用 ensure_row_cache()：按需分配三组行缓存： rowlabel：显示名称。 rowvalue：大小文本。 rowiconcp：图标。。
    (void)ensure_row_cache();
    // 访问对象成员：list_.set_area(Theme::list_region());；读取或修改这个对象里的成员状态。
    list_.set_area(Theme::list_region());
    // 访问对象成员：list_.set_provider([this](uint8_t i, ui::widgets::RowStyle& rs) { fill_row(i, rs); });；读取或修改这个对象里的成员状态。
    list_.set_provider([this](uint8_t i, ui::widgets::RowStyle& rs) { fill_row(i, rs); });
    // 调用 on_row_tap(i)：处理列表点击： 如果点击父目录行，调用 goup()。 如果点击根卷，调用 openroot() 并调度扫描。 如果点击目录，调用 enterdir()。 如果点击文件，调用 showfileaction()。。
    list_.set_tap_handler([this](uint8_t i) { on_row_tap(i); });
}

// 函数定义/声明：调用 FilesApp::ensure_row_cache()：按需分配三组行缓存： rowlabel：显示名称。 rowvalue：大小文本。 rowiconcp：图标。。
bool FilesApp::ensure_row_cache()
{
    // 判断条件：row_label_ != nullptr && row_value_ != nullptr && row_icon_cp_ != nullptr 是否成立。
    if (row_label_ != nullptr && row_value_ != nullptr && row_icon_cp_ != nullptr)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    // 判断条件：row_label_ == nullptr 是否成立。
    if (row_label_ == nullptr)
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        row_label_ = static_cast<char (*)[VfsBrowser::kNameCap]>(
            // 继续上一行的表达式或参数列表：alloc_psram_prefer(sizeof(char[VfsBrowser::kMaxEntries][VfsBrowser::kNameCap])));。
            alloc_psram_prefer(sizeof(char[VfsBrowser::kMaxEntries][VfsBrowser::kNameCap])));
    // 判断条件：row_value_ == nullptr 是否成立。
    if (row_value_ == nullptr)
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        row_value_ = static_cast<char (*)[16]>(
            // 继续上一行的表达式或参数列表：alloc_psram_prefer(sizeof(char[VfsBrowser::kMaxEntries][16])));。
            alloc_psram_prefer(sizeof(char[VfsBrowser::kMaxEntries][16])));
    // 判断条件：row_icon_cp_ == nullptr 是否成立。
    if (row_icon_cp_ == nullptr)
        // 赋值：把右侧 alloc_array_psram<uint32_t>(VfsBrowser::kMaxEntries) 的结果保存到左侧 row_icon_cp_。
        row_icon_cp_ = alloc_array_psram<uint32_t>(VfsBrowser::kMaxEntries);
    // 返回 row_label_ != nullptr && row_value_ != nullptr && row_icon_cp_ != nullptr 给调用者，表示当前函数的执行结果。
    return row_label_ != nullptr && row_value_ != nullptr && row_icon_cp_ != nullptr;
}

// 定义只读对象或变量：初始化后不能在当前位置修改。
const char* FilesApp::title() const
{
    // 判断条件：browser_.mode() == VfsBrowser::Mode::Roots 是否成立。
    if (browser_.mode() == VfsBrowser::Mode::Roots)
        // 返回 ui::strings::kAppFiles 给调用者，表示当前函数的执行结果。
        return ui::strings::kAppFiles;

    // 调用 display_name(browser_.browse_title(), title_buf_, sizeof(title_buf_),，执行这个函数并处理它的返回值或状态变化。
    display_name(browser_.browse_title(), title_buf_, sizeof(title_buf_),
                 // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kScreenW - Theme::kPadLg * 2));。
                 static_cast<uint16_t>(Theme::kScreenW - Theme::kPadLg * 2));
    // 返回 title_buf_ 给调用者，表示当前函数的执行结果。
    return title_buf_;
}

// 函数定义/声明：调用 FilesApp::icon_cp()：返回图标 codepoint。图标通常来自字体图标表，所以这里返回的是 uint32t，不是图片路径。。
uint32_t FilesApp::icon_cp() const
{
    // 返回 gfx::icon::kFaFolderOpen 给调用者，表示当前函数的执行结果。
    return gfx::icon::kFaFolderOpen;
}

// 函数定义/声明：调用 FilesApp::request_scroll_repaint()：限制滚动时的重绘频率。代码使用约 120 ms 的节流值，避免用户连续滑动时每个触摸事件都触发一次墨水屏刷新。。
void FilesApp::request_scroll_repaint()
{
    // 判断条件：!request_repaint_if_ready(last_scroll_repaint_ms_, kScrollRepaintMs, false) 是否成立。
    if (!request_repaint_if_ready(last_scroll_repaint_ms_, kScrollRepaintMs, false))
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 定义或声明函数 FilesApp::on_enter：参数在括号里，返回类型写在函数名前面。
void FilesApp::on_enter()
{
    // 进入页面时回到根卷列表；真正的目录扫描在用户进入根卷后异步启动。
    cancel_browse_reload();
    // 调用 show_roots()：把浏览器切回根卷模式。它把 mode 设为 Roots，清空当前目录相关状态，并让后续 rowcount() 显示可见根卷。。
    browser_.show_roots();
    // 访问对象成员：list_.set_scroll(0);；读取或修改这个对象里的成员状态。
    list_.set_scroll(0);
    // 调用 sync_list_total()：把 ListView 的总行数同步为 browser.rowcount()。当根卷数量变化、进入目录或删除文件后都要调用，否则列表的滚动范围会比实际行数多或少。。
    sync_list_total();
    // 调用 close_dialog()：把 dlg 设回 None，清除选中信息和信息行。它不负责重扫目录，因为关闭弹窗通常只是恢复界面。。
    close_dialog();
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 函数定义/声明：调用 FilesApp::on_exit()：退出时取消未完成的扫描、关闭弹窗，并确保后台任务不会在对象状态失效后继续写数据。。
void FilesApp::on_exit()
{
    // 退出时必须取消扫描任务，避免后台继续写已经失效的 UI 状态。
    cancel_browse_reload();
    // 调用 show_roots()：把浏览器切回根卷模式。它把 mode 设为 Roots，清空当前目录相关状态，并让后续 rowcount() 显示可见根卷。。
    browser_.show_roots();
    // 访问对象成员：list_.set_scroll(0);；读取或修改这个对象里的成员状态。
    list_.set_scroll(0);
    // 调用 close_dialog()：把 dlg 设回 None，清除选中信息和信息行。它不负责重扫目录，因为关闭弹窗通常只是恢复界面。。
    close_dialog();
}

// 函数定义/声明：调用 FilesApp::on_semantic_back()：处理语义返回，顺序通常是： 1. 如果弹窗打开，先关闭弹窗。 2. 如果列表还在浏览目录，先返回上一级。 3. 如果已经到根卷，返回 false，让 Shell 退出 App。。
bool FilesApp::on_semantic_back()
{
    // 与工具栏返回一致：关对话框 / 上一级目录；已在根目录则退出 App
    return handle_toolbar_back();
}

// 函数定义/声明：调用 FilesApp::cancel_browse_reload()：取消或停止当前浏览重载。它需要避免旧任务完成后把已经过期的目录结果 adopt 到 UI。。
void FilesApp::cancel_browse_reload()
{
    // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    reload_task_.reset();
    // 赋值：把右侧 false 的结果保存到左侧 loading_。
    loading_ = false;
}

// 函数定义/声明：调用 FilesApp::close_dialog()：把 dlg 设回 None，清除选中信息和信息行。它不负责重扫目录，因为关闭弹窗通常只是恢复界面。。
void FilesApp::close_dialog()
{
    // 赋值：把右侧 FileDlg::None 的结果保存到左侧 dlg_。
    dlg_ = FileDlg::None;
    // 赋值：把右侧 0 的结果保存到左侧 sel_idx_。
    sel_idx_ = 0;
    // 赋值：把右侧 '\0' 的结果保存到左侧 sel_path_[0]。
    sel_path_[0] = '\0';
    // 赋值：把右侧 {} 的结果保存到左侧 sel_entry_。
    sel_entry_ = {};
    // 赋值：把右侧 false 的结果保存到左侧 sel_is_system_。
    sel_is_system_ = false;
}

// 函数定义/声明：调用 FilesApp::sync_list_total()：把 ListView 的总行数同步为 browser.rowcount()。当根卷数量变化、进入目录或删除文件后都要调用，否则列表的滚动范围会比实际行数多或少。。
void FilesApp::sync_list_total()
{
    // 调用 row_count()：返回列表应该显示多少行。它不是简单返回 entrycount，还要考虑： 是否显示“返回上一级”行。 当前是 Roots 还是 Browse。 根卷数量在不同 SD 状态下是否变化。。
    list_.set_total(browser_.row_count());
}

// 函数定义/声明：调用 FilesApp::rebuild_row_cache()：遍历当前所有列表行，调用 fillrow() 填充缓存。它通常发生在目录结果 adopt 后。。
void FilesApp::rebuild_row_cache()
{
    // 判断条件：!ensure_row_cache() 是否成立。
    if (!ensure_row_cache())
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 std::memset(row_icon_cp_, 0, sizeof(uint32_t) * VfsBrowser::kMaxEntries);，执行这个函数并处理它的返回值或状态变化。
    std::memset(row_icon_cp_, 0, sizeof(uint32_t) * VfsBrowser::kMaxEntries);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t n = browser_.dir_entry_count();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t label_w = static_cast<uint16_t>(Theme::kScreenW - 80);

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < n; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const VfsBrowser::Entry& e = browser_.entry_at(i);
        // 调用 display_name(e.name, row_label_[i], sizeof(row_label_[i]), label_w);，执行这个函数并处理它的返回值或状态变化。
        display_name(e.name, row_label_[i], sizeof(row_label_[i]), label_w);
        // 赋值：把右侧 row_icon(e) 的结果保存到左侧 row_icon_cp_[i]。
        row_icon_cp_[i] = row_icon(e);
        // 判断条件：!e.is_dir 是否成立。
        if (!e.is_dir)
            // 调用 gfx::format_file_size(e.size_bytes, row_value_[i], sizeof(row_value_[i]));，执行这个函数并处理它的返回值或状态变化。
            gfx::format_file_size(e.size_bytes, row_value_[i], sizeof(row_value_[i]));
        // 备用分支：前面的条件不成立时执行这里。
        else
            // 赋值：把右侧 '\0' 的结果保存到左侧 row_value_[i][0]。
            row_value_[i][0] = '\0';
    }
}

// 函数定义/声明：调用 FilesApp::on_browse_ready()：目录扫描完成后，在 UI 线程里把结果交给浏览器、重建缓存、更新列表总数、清除 loading 状态，并请求重绘。。
void FilesApp::on_browse_ready()
{
    // 赋值：把右侧 false 的结果保存到左侧 loading_。
    loading_ = false;
    // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    reload_task_.reset();
    // 调用 rebuild_row_cache()：遍历当前所有列表行，调用 fillrow() 填充缓存。它通常发生在目录结果 adopt 后。。
    rebuild_row_cache();
    // 调用 sync_list_total()：把 ListView 的总行数同步为 browser.rowcount()。当根卷数量变化、进入目录或删除文件后都要调用，否则列表的滚动范围会比实际行数多或少。。
    sync_list_total();
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 函数定义/声明：调用 FilesApp::finish_browse_reload()：完成本次重载的清理工作，例如释放任务对象、清除取消标记。它把“状态改变”和“资源回收”分开，便于排查任务为什么没有结束。。
void FilesApp::finish_browse_reload()
{
    {
        // 函数调用：std::lock_guard<std::mutex> lk(g_reload_mtx);；执行该函数并处理它的返回值或副作用。
        std::lock_guard<std::mutex> lk(g_reload_mtx);
        // 判断条件：g_reload_job != nullptr 是否成立。
        if (g_reload_job != nullptr)
        {
            // 函数调用：browser_.adopt_scan(g_reload_job->entries, g_reload_job->count, g_reload_job->truncated,；执行该函数并处理它的返回值或副作用。
            browser_.adopt_scan(g_reload_job->entries, g_reload_job->count, g_reload_job->truncated,
                                // 继续上一行的表达式或参数列表：g_reload_job->has_parent);。
                                g_reload_job->has_parent);
        }
    }
    // 调用 on_browse_ready()：目录扫描完成后，在 UI 线程里把结果交给浏览器、重建缓存、更新列表总数、清除 loading 状态，并请求重绘。。
    on_browse_ready();
}

// 函数定义/声明：调用 FilesApp::schedule_browse_reload()：调度后台目录扫描。关键实现细节： 任务名是 filesreload。 优先级是 LOW。 栈大小是 6144。 栈使用 PSRAM。 结果放在 greloadjob。 greloadmtx 保护任务和结果对象。 完成后用 UiBus::SystemHint::FilesReloadDone 通知 UI。。
void FilesApp::schedule_browse_reload()
{
    // 创建 files_reload 后台任务；UI 只等待 FilesReloadDone 事件。
    if (browser_.mode() != VfsBrowser::Mode::Browse || browser_.cwd()[0] == '\0')
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：!ensure_reload_job() 是否成立。
    if (!ensure_reload_job())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 调用 cancel_browse_reload()：取消或停止当前浏览重载。它需要避免旧任务完成后把已经过期的目录结果 adopt 到 UI。。
    cancel_browse_reload();
    // 赋值：把右侧 true 的结果保存到左侧 loading_。
    loading_ = true;
    // 访问对象成员：list_.set_scroll(0);；读取或修改这个对象里的成员状态。
    list_.set_scroll(0);
    // 调用 sync_list_total()：把 ListView 的总行数同步为 browser.rowcount()。当根卷数量变化、进入目录或删除文件后都要调用，否则列表的滚动范围会比实际行数多或少。。
    sync_list_total();
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();

    {
        // 函数调用：std::lock_guard<std::mutex> lk(g_reload_mtx);；执行该函数并处理它的返回值或副作用。
        std::lock_guard<std::mutex> lk(g_reload_mtx);
        // 调用 std::strncpy(g_reload_job->cwd, browser_.cwd(), sizeof(g_reload_job->cwd) - 1);，执行这个函数并处理它的返回值或状态变化。
        std::strncpy(g_reload_job->cwd, browser_.cwd(), sizeof(g_reload_job->cwd) - 1);
        // 赋值：把右侧 '\0' 的结果保存到左侧 g_reload_job->cwd[sizeof(g_reload_job->cwd) - 1]。
        g_reload_job->cwd[sizeof(g_reload_job->cwd) - 1] = '\0';
    }

    // 定义变量：::app::sys::task::Cfg cfg =
    ::app::sys::task::Cfg cfg =
        // 继续上一行的表达式或参数列表：::app::sys::task::Cfg::light("files_reload", ::app::sys::task::Priority::LOW);。
        ::app::sys::task::Cfg::light("files_reload", ::app::sys::task::Priority::LOW);
    // 赋值：把右侧 kReloadStack 的结果保存到左侧 cfg.stack_size。
    cfg.stack_size = kReloadStack;
    // 赋值：把右侧 true 的结果保存到左侧 cfg.use_psram。
    cfg.use_psram = true;

    // 执行这条代码：reload_task_ = std::make_unique<::app::sys::task::Task>(。它会更新变量、对象状态或调用链。
    reload_task_ = std::make_unique<::app::sys::task::Task>(
        // 开始代码块：下面是这个函数、分支或循环的主体。
        [](void* /*arg*/) {
            {
                // 函数调用：std::lock_guard<std::mutex> lk(g_reload_mtx);；执行该函数并处理它的返回值或副作用。
                std::lock_guard<std::mutex> lk(g_reload_mtx);
                // 判断条件：g_reload_job != nullptr 是否成立。
                if (g_reload_job != nullptr)
                {
                    // 赋值：把右侧 0 的结果保存到左侧 g_reload_job->count。
                    g_reload_job->count = 0;
                    // 赋值：把右侧 false 的结果保存到左侧 g_reload_job->truncated。
                    g_reload_job->truncated = false;
                    // 函数调用：(void)VfsBrowser::scan_entries(g_reload_job->cwd, g_reload_job->entries,；执行该函数并处理它的返回值或副作用。
                    (void)VfsBrowser::scan_entries(g_reload_job->cwd, g_reload_job->entries,
                                                   // 继续上一行的表达式或参数列表：g_reload_job->count, g_reload_job->truncated);。
                                                   g_reload_job->count, g_reload_job->truncated);
                    // 调用 VfsBrowser::is_storage_root(g_reload_job->cwd)：判断当前路径是否正好等于一个存储根卷。它常用于决定返回键行为：在根卷内部按返回应该回到根卷列表；在某个子目录按返回应该回到父目录。。
                    g_reload_job->has_parent = !VfsBrowser::is_storage_root(g_reload_job->cwd);
                }
            }

            // 函数调用：(void)ui::UiBus::get_instance().post_system_hint(ui::SystemHintKind::FilesReloadDone,；执行该函数并处理它的返回值或副作用。
            (void)ui::UiBus::get_instance().post_system_hint(ui::SystemHintKind::FilesReloadDone,
                                                             // 继续上一行的表达式或参数列表：0);。
                                                             0);
        // 结束上一行的 lambda 或初始化列表：把这段匿名代码/数据交给调用者。
        },
        // 继续上一行的表达式或参数列表：cfg, nullptr);。
        cfg, nullptr);

    // 判断条件：!reload_task_->start() 是否成立。
    if (!reload_task_->start())
    {
        // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
        reload_task_.reset();
        // 调用 reload()：重新加载当前模式： 在根卷模式时重新计算可见根卷。 在浏览模式时重新扫描当前目录。。
        browser_.reload();
        // 调用 on_browse_ready()：目录扫描完成后，在 UI 线程里把结果交给浏览器、重建缓存、更新列表总数、清除 loading 状态，并请求重绘。。
        on_browse_ready();
    }
}

// 函数定义/声明：调用 FilesApp::on_ui_event()：FilesApp::onuievent()。
void FilesApp::on_ui_event(const ui::UiEvent& ev)
{
    // UI 线程在这里采用后台扫描结果；后台任务不能直接改 browser_ 和 list_。
    if (ev.kind != ui::UiEventKind::SystemHint)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：ev.payload.system.hint != ui::SystemHintKind::FilesReloadDone 是否成立。
    if (ev.payload.system.hint != ui::SystemHintKind::FilesReloadDone)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 finish_browse_reload()：完成本次重载的清理工作，例如释放任务对象、清除取消标记。它把“状态改变”和“资源回收”分开，便于排查任务为什么没有结束。。
    finish_browse_reload();
}

// 函数定义/声明：调用 FilesApp::ends_with_ci()：做不区分大小写的后缀匹配，例如 .TXT、.Txt、.txt 都应识别为文本。它是打开类型分类的基础。。
bool FilesApp::ends_with_ci(const char* name, const char* ext)
{
    // 判断条件：name == nullptr || ext == nullptr 是否成立。
    if (name == nullptr || ext == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t nl = std::strlen(name);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t el = std::strlen(ext);
    // 判断条件：nl < el 是否成立。
    if (nl < el)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* tail = name + (nl - el);
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (size_t i = 0; i < el; ++i)
    {
        // 定义变量 char a 并初始化为 tail[i]。
        char a = tail[i];
        // 定义变量 char b 并初始化为 ext[i]。
        char b = ext[i];
        // 判断条件：a >= 'A' && a <= 'Z' 是否成立。
        if (a >= 'A' && a <= 'Z')
            // 赋值：把右侧 static_cast<char>(a + ('a' - 'A')) 的结果保存到左侧 a。
            a = static_cast<char>(a + ('a' - 'A'));
        // 判断条件：b >= 'A' && b <= 'Z' 是否成立。
        if (b >= 'A' && b <= 'Z')
            // 赋值：把右侧 static_cast<char>(b + ('a' - 'A')) 的结果保存到左侧 b。
            b = static_cast<char>(b + ('a' - 'A'));
        // 判断条件：a != b 是否成立。
        if (a != b)
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
    }
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数定义/声明：调用 FilesApp::classify_open()：按后缀返回 OpenKind： .txt Txt，交给 Reader。 .bmp Bmp，交给 Drawing。 .jpg、.jpeg Jpeg，交给 Gallery。 .mp3、.wav Audio，交给 Music。 其他 Unsupported。。
FilesApp::OpenKind FilesApp::classify_open(const char* name)
{
    // 判断条件：name == nullptr 是否成立。
    if (name == nullptr)
        // 返回 OpenKind::Unsupported 给调用者，表示当前函数的执行结果。
        return OpenKind::Unsupported;
    // 判断条件：ends_with_ci(name, ".txt") 是否成立。
    if (ends_with_ci(name, ".txt"))
        // 返回 OpenKind::Txt 给调用者，表示当前函数的执行结果。
        return OpenKind::Txt;
    // 判断条件：ends_with_ci(name, ".bmp") 是否成立。
    if (ends_with_ci(name, ".bmp"))
        // 返回 OpenKind::Bmp 给调用者，表示当前函数的执行结果。
        return OpenKind::Bmp;
    // 判断条件：ends_with_ci(name, ".jpg") || ends_with_ci(name, ".jpeg") 是否成立。
    if (ends_with_ci(name, ".jpg") || ends_with_ci(name, ".jpeg"))
        // 返回 OpenKind::Jpeg 给调用者，表示当前函数的执行结果。
        return OpenKind::Jpeg;
    // 判断条件：ends_with_ci(name, ".mp3") || ends_with_ci(name, ".wav") 是否成立。
    if (ends_with_ci(name, ".mp3") || ends_with_ci(name, ".wav"))
        // 返回 OpenKind::Audio 给调用者，表示当前函数的执行结果。
        return OpenKind::Audio;
    // 判断条件：ends_with_ci(name, ".ttf") || ends_with_ci(name, ".bin") 是否成立。
    if (ends_with_ci(name, ".ttf") || ends_with_ci(name, ".bin"))
        // 返回 OpenKind::Unsupported 给调用者，表示当前函数的执行结果。
        return OpenKind::Unsupported;
    // 返回 OpenKind::Unsupported 给调用者，表示当前函数的执行结果。
    return OpenKind::Unsupported;
}

// 定义只读对象或变量：初始化后不能在当前位置修改。
const char* FilesApp::type_label(OpenKind kind, const char* name)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (kind)
    {
        // 分支匹配：case OpenKind::Txt 成立时执行下面的代码。
        case OpenKind::Txt:
            // 返回 ui::strings::kFilesTypeTxt 给调用者，表示当前函数的执行结果。
            return ui::strings::kFilesTypeTxt;
        // 分支匹配：case OpenKind::Bmp 成立时执行下面的代码。
        case OpenKind::Bmp:
            // 返回 ui::strings::kFilesTypeBmp 给调用者，表示当前函数的执行结果。
            return ui::strings::kFilesTypeBmp;
        // 分支匹配：case OpenKind::Jpeg 成立时执行下面的代码。
        case OpenKind::Jpeg:
            // 返回 ui::strings::kFilesTypePhoto 给调用者，表示当前函数的执行结果。
            return ui::strings::kFilesTypePhoto;
        // 分支匹配：case OpenKind::Audio 成立时执行下面的代码。
        case OpenKind::Audio:
            // 返回 ui::strings::kFilesTypeAudio 给调用者，表示当前函数的执行结果。
            return ui::strings::kFilesTypeAudio;
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 判断条件：ends_with_ci(name, ".ttf") || ends_with_ci(name, ".bin") 是否成立。
            if (ends_with_ci(name, ".ttf") || ends_with_ci(name, ".bin"))
                // 返回 ui::strings::kFilesTypeFont 给调用者，表示当前函数的执行结果。
                return ui::strings::kFilesTypeFont;
            // 返回 ui::strings::kFilesTypeFile 给调用者，表示当前函数的执行结果。
            return ui::strings::kFilesTypeFile;
    }
}

// 函数定义/声明：调用 FilesApp::display_name()：把长文件名缩成适合指定宽度的显示文本。这里要考虑字体宽度，不是简单按字符数截断。。
void FilesApp::display_name(const char* raw, char* out, size_t out_cap, uint16_t max_w)
{
    // 执行语句：char norm[48];；它会更新一次变量、对象或程序状态。
    char norm[48];
    // 函数调用：(void)text::normalize_path_segment(raw, norm, sizeof(norm));；执行该函数并处理它的返回值或副作用。
    (void)text::normalize_path_segment(raw, norm, sizeof(norm));
    // 函数调用：(void)gfx::truncate_text(norm, Theme::kFontBody, max_w, out, out_cap);；执行该函数并处理它的返回值或副作用。
    (void)gfx::truncate_text(norm, Theme::kFontBody, max_w, out, out_cap);
}

// 定义或声明函数 FilesApp::row_icon：参数在括号里，返回类型写在函数名前面。
uint32_t FilesApp::row_icon(const VfsBrowser::Entry& e)
{
    // 判断条件：e.is_dir 是否成立。
    if (e.is_dir)
        // 返回 gfx::icon::kFaFolder 给调用者，表示当前函数的执行结果。
        return gfx::icon::kFaFolder;
    // 判断条件：ends_with_ci(e.name, ".txt") 是否成立。
    if (ends_with_ci(e.name, ".txt"))
        // 返回 gfx::icon::kFaBookReader 给调用者，表示当前函数的执行结果。
        return gfx::icon::kFaBookReader;
    // 判断条件：ends_with_ci(e.name, ".bmp") 是否成立。
    if (ends_with_ci(e.name, ".bmp"))
        // 返回 gfx::icon::kFaPaintBrush 给调用者，表示当前函数的执行结果。
        return gfx::icon::kFaPaintBrush;
    // 判断条件：ends_with_ci(e.name, ".jpg") || ends_with_ci(e.name, ".jpeg") 是否成立。
    if (ends_with_ci(e.name, ".jpg") || ends_with_ci(e.name, ".jpeg"))
        // 返回 gfx::icon::kFaImages 给调用者，表示当前函数的执行结果。
        return gfx::icon::kFaImages;
    // 判断条件：ends_with_ci(e.name, ".mp3") || ends_with_ci(e.name, ".wav") 是否成立。
    if (ends_with_ci(e.name, ".mp3") || ends_with_ci(e.name, ".wav"))
        // 返回 gfx::icon::kFaMusic 给调用者，表示当前函数的执行结果。
        return gfx::icon::kFaMusic;
    // 返回 gfx::icon::kFaFile 给调用者，表示当前函数的执行结果。
    return gfx::icon::kFaFile;
}

// 函数定义/声明：调用 FilesApp::file_dlg_rect()：返回不同弹窗的矩形区域。不同弹窗高度不同： 操作弹窗 92。 信息弹窗 118。 删除确认弹窗 76。。
core::Rect FilesApp::file_dlg_rect(FileDlg dlg)
{
    // 定义变量 uint16_t h 并初始化为 kFileDlgActionH。
    uint16_t h = kFileDlgActionH;
    // 判断条件：dlg == FileDlg::Info 是否成立。
    if (dlg == FileDlg::Info)
        // 赋值：把右侧 kFileDlgInfoH 的结果保存到左侧 h。
        h = kFileDlgInfoH;
    // 备用分支：前面的条件不成立时执行这里。
    else if (dlg == FileDlg::DeleteConfirm)
        // 赋值：把右侧 kFileDlgDeleteH 的结果保存到左侧 h。
        h = kFileDlgDeleteH;
    // 返回 ui::dialog::centered(kFileDlgW, h) 给调用者，表示当前函数的执行结果。
    return ui::dialog::centered(kFileDlgW, h);
}

// 函数定义/声明：调用 FilesApp::file_dlg_close_rect()：返回关闭按钮的命中区域。它必须和 paintdlgclosebtn() 的绘制位置一致。。
core::Rect FilesApp::file_dlg_close_rect(FileDlg dlg)
{
    // 返回结果：调用 file_dlg_rect(dlg)：返回不同弹窗的矩形区域。不同弹窗高度不同： 操作弹窗 92。 信息弹窗 118。 删除确认弹窗 76。。
    return ui::dialog::close_hit_rect(file_dlg_rect(dlg));
}

// 函数定义/声明：调用 FilesApp::file_action_btn_rect()：返回操作弹窗三个按钮中某个按钮的矩形。当前有三个动作按钮，所以索引越界必须被拒绝。。
core::Rect FilesApp::file_action_btn_rect(uint8_t idx)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect dlg = file_dlg_rect(FileDlg::Action);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t w3 = static_cast<uint16_t>(dlg.w / kActionBtnNum);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x = static_cast<int16_t>(dlg.x + idx * w3);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y = static_cast<int16_t>(dlg.bottom() - kFileBtnH);
    // 返回 core::Rect{x, y, w3, kFileBtnH} 给调用者，表示当前函数的执行结果。
    return core::Rect{x, y, w3, kFileBtnH};
}

// 函数定义/声明：调用 FilesApp::file_info_back_rect()：返回信息弹窗中“返回”按钮的区域。。
core::Rect FilesApp::file_info_back_rect()
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect dlg = file_dlg_rect(FileDlg::Info);
    // 返回 core::Rect{dlg.x, static_cast<int16_t>(dlg.bottom() - kFileBtnH), dlg.w, kFileBtnH} 给调用者，表示当前函数的执行结果。
    return core::Rect{dlg.x, static_cast<int16_t>(dlg.bottom() - kFileBtnH), dlg.w, kFileBtnH};
}

// 函数定义/声明：调用 FilesApp::build_info_lines()：生成最多四行信息文本，通常包含： 文件名。 类型。 大小。 路径或系统提示。。
void FilesApp::build_info_lines()
{
    // 调用 display_name(sel_entry_.name, dlg_title_, sizeof(dlg_title_), 120);，执行这个函数并处理它的返回值或状态变化。
    display_name(sel_entry_.name, dlg_title_, sizeof(dlg_title_), 120);

    // 执行语句：char sz[16];；它会更新一次变量、对象或程序状态。
    char sz[16];
    // 调用 gfx::format_file_size(sel_entry_.size_bytes, sz, sizeof(sz));，执行这个函数并处理它的返回值或状态变化。
    gfx::format_file_size(sel_entry_.size_bytes, sz, sizeof(sz));
    // 函数调用：(void)std::snprintf(info_lines_[0], sizeof(info_lines_[0]), "%s: %s",；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(info_lines_[0], sizeof(info_lines_[0]), "%s: %s",
                        // 继续上一行的表达式或参数列表：ui::strings::kFilesInfoName, dlg_title_);。
                        ui::strings::kFilesInfoName, dlg_title_);
    // 函数调用：(void)std::snprintf(info_lines_[1], sizeof(info_lines_[1]), "%s: %s",；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(info_lines_[1], sizeof(info_lines_[1]), "%s: %s",
                        // 继续上一行的表达式或参数列表：ui::strings::kFilesInfoSize, sz);。
                        ui::strings::kFilesInfoSize, sz);

    // 执行语句：char path_shown[40];；它会更新一次变量、对象或程序状态。
    char path_shown[40];
    // 调用 display_name(sel_path_, path_shown, sizeof(path_shown), 100);，执行这个函数并处理它的返回值或状态变化。
    display_name(sel_path_, path_shown, sizeof(path_shown), 100);
    // 函数调用：(void)std::snprintf(info_lines_[2], sizeof(info_lines_[2]), "%s: %s",；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(info_lines_[2], sizeof(info_lines_[2]), "%s: %s",
                        // 继续上一行的表达式或参数列表：ui::strings::kFilesInfoPath, path_shown);。
                        ui::strings::kFilesInfoPath, path_shown);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const OpenKind kind = classify_open(sel_entry_.name);
    // 函数调用：(void)std::snprintf(info_lines_[3], sizeof(info_lines_[3]), "%s: %s",；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(info_lines_[3], sizeof(info_lines_[3]), "%s: %s",
                        // 继续上一行的表达式或参数列表：ui::strings::kFilesInfoType, type_label(kind, sel_entry_.name));。
                        ui::strings::kFilesInfoType, type_label(kind, sel_entry_.name));
}

// 定义或声明函数 FilesApp::fill_row：参数在括号里，返回类型写在函数名前面。
void FilesApp::fill_row(uint8_t idx, ui::widgets::RowStyle& rs)
{
    // 赋值：把右侧 {} 的结果保存到左侧 rs。
    rs = {};

    // 判断条件：browser_.mode() == VfsBrowser::Mode::Roots 是否成立。
    if (browser_.mode() == VfsBrowser::Mode::Roots)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const VfsBrowser::RootVol* vol = VfsBrowser::root_at_visible(idx);
        // 判断条件：vol == nullptr 是否成立。
        if (vol == nullptr)
            // 提前返回：结束当前函数，不返回具体值。
            return;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* label = vol->label;
        // 判断条件：vol->kind == ::app::common::storage::MountKind::Internal 是否成立。
        if (vol->kind == ::app::common::storage::MountKind::Internal)
            // 赋值：把右侧 ui::strings::kFilesInternal 的结果保存到左侧 label。
            label = ui::strings::kFilesInternal;
        // 备用分支：前面的条件不成立时执行这里。
        else if (vol->kind == ::app::common::storage::MountKind::Sd)
            // 赋值：把右侧 ui::strings::kFilesSd 的结果保存到左侧 label。
            label = ui::strings::kFilesSd;
        // 备用分支：前面的条件不成立时执行这里。
        else if (vol->kind == ::app::common::storage::MountKind::Assets)
            // 赋值：把右侧 ui::strings::kFilesAssets 的结果保存到左侧 label。
            label = ui::strings::kFilesAssets;
        // 赋值：把右侧 label 的结果保存到左侧 rs.label。
        rs.label = label;
        // 赋值：把右侧 gfx::icon::kFaHdd 的结果保存到左侧 rs.icon_cp。
        rs.icon_cp = gfx::icon::kFaHdd;
        // 赋值：把右侧 true 的结果保存到左侧 rs.show_chevron。
        rs.show_chevron = true;
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：row_label_ == nullptr || row_value_ == nullptr || row_icon_cp_ == nullptr 是否成立。
    if (row_label_ == nullptr || row_value_ == nullptr || row_icon_cp_ == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 判断条件：browser_.has_parent_row() && idx == 0 是否成立。
    if (browser_.has_parent_row() && idx == 0)
    {
        // 赋值：把右侧 ui::strings::kFilesParent 的结果保存到左侧 rs.label。
        rs.label = ui::strings::kFilesParent;
        // 赋值：把右侧 gfx::icon::kFaChevronLeft 的结果保存到左侧 rs.icon_cp。
        rs.icon_cp = gfx::icon::kFaChevronLeft;
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t base = browser_.has_parent_row() ? 1U : 0U;
    // 判断条件：idx < base 是否成立。
    if (idx < base)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t eidx = static_cast<uint8_t>(idx - base);
    // 判断条件：eidx >= browser_.dir_entry_count() 是否成立。
    if (eidx >= browser_.dir_entry_count())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 row_label_[eidx] 的结果保存到左侧 rs.label。
    rs.label = row_label_[eidx];
    // 赋值：把右侧 row_icon_cp_[eidx] 的结果保存到左侧 rs.icon_cp。
    rs.icon_cp = row_icon_cp_[eidx];
    // 赋值：把右侧 browser_.entry_at(eidx).is_dir 的结果保存到左侧 rs.show_chevron。
    rs.show_chevron = browser_.entry_at(eidx).is_dir;
    // 判断条件：row_value_[eidx][0] != '\0' 是否成立。
    if (row_value_[eidx][0] != '\0')
        // 赋值：把右侧 row_value_[eidx] 的结果保存到左侧 rs.value。
        rs.value = row_value_[eidx];
}

// 函数定义/声明：调用 FilesApp::show_file_action()：用户点击文件后打开操作弹窗。它保存选中条目、路径和系统文件标记，然后设置 dlg。。
void FilesApp::show_file_action(uint8_t entry_idx)
{
    // 判断条件：loading_ 是否成立。
    if (loading_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：!browser_.entry_path(entry_idx, sel_path_, sizeof(sel_path_)) 是否成立。
    if (!browser_.entry_path(entry_idx, sel_path_, sizeof(sel_path_)))
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 entry_idx 的结果保存到左侧 sel_idx_。
    sel_idx_ = entry_idx;
    // 赋值：把右侧 browser_.entry_at(entry_idx) 的结果保存到左侧 sel_entry_。
    sel_entry_ = browser_.entry_at(entry_idx);
    // 赋值：把右侧 ::app::common::storage::path_is_system(sel_path_) 的结果保存到左侧 sel_is_system_。
    sel_is_system_ = ::app::common::storage::path_is_system(sel_path_);
    // 调用 display_name(sel_entry_.name, dlg_title_, sizeof(dlg_title_), 120);，执行这个函数并处理它的返回值或状态变化。
    display_name(sel_entry_.name, dlg_title_, sizeof(dlg_title_), 120);

    // 判断条件：sel_is_system_ 是否成立。
    if (sel_is_system_)
    {
        // 调用 build_info_lines()：生成最多四行信息文本，通常包含： 文件名。 类型。 大小。 路径或系统提示。。
        build_info_lines();
        // 赋值：把右侧 FileDlg::Info 的结果保存到左侧 dlg_。
        dlg_ = FileDlg::Info;
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 赋值：把右侧 FileDlg::Action 的结果保存到左侧 dlg_。
        dlg_ = FileDlg::Action;
    }
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 函数定义/声明：调用 FilesApp::on_row_tap()：处理列表点击： 如果点击父目录行，调用 goup()。 如果点击根卷，调用 openroot() 并调度扫描。 如果点击目录，调用 enterdir()。 如果点击文件，调用 showfileaction()。。
void FilesApp::on_row_tap(uint8_t idx)
{
    // 点击优先级：返回上一级 -> 根卷 -> 目录 -> 文件操作弹窗。
    if (browser_.mode() == VfsBrowser::Mode::Roots)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const VfsBrowser::RootVol* vol = VfsBrowser::root_at_visible(idx);
        // 判断条件：vol == nullptr 是否成立。
        if (vol == nullptr)
            // 提前返回：结束当前函数，不返回具体值。
            return;
        // 判断条件：!browser_.open_root(vol->path, vol->label) 是否成立。
        if (!browser_.open_root(vol->path, vol->label))
        {
            // 调用 overlays::Toast::instance().show(ui::strings::kFilesNotMounted, 2000);，执行这个函数并处理它的返回值或状态变化。
            overlays::Toast::instance().show(ui::strings::kFilesNotMounted, 2000);
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            request_repaint();
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }
        // 调用 schedule_browse_reload()：调度后台目录扫描。关键实现细节： 任务名是 filesreload。 优先级是 LOW。 栈大小是 6144。 栈使用 PSRAM。 结果放在 greloadjob。 greloadmtx 保护任务和结果对象。 完成后用 UiBus::SystemHint::FilesReloadDone 通知 UI。。
        schedule_browse_reload();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：browser_.has_parent_row() && idx == 0 是否成立。
    if (browser_.has_parent_row() && idx == 0)
    {
        // 调用 cancel_browse_reload()：取消或停止当前浏览重载。它需要避免旧任务完成后把已经过期的目录结果 adopt 到 UI。。
        cancel_browse_reload();
        // 调用 go_up()：返回上一级目录： 1. 如果当前是子目录，把 cwd 缩短到父目录。 2. 如果在根卷目录，回到 Roots。 3. 更新标题、父目录行和条目视图。。
        browser_.go_up();
        // 访问对象成员：list_.set_scroll(0);；读取或修改这个对象里的成员状态。
        list_.set_scroll(0);
        // 判断条件：browser_.mode() == VfsBrowser::Mode::Roots 是否成立。
        if (browser_.mode() == VfsBrowser::Mode::Roots)
        {
            // 调用 sync_list_total()：把 ListView 的总行数同步为 browser.rowcount()。当根卷数量变化、进入目录或删除文件后都要调用，否则列表的滚动范围会比实际行数多或少。。
            sync_list_total();
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            request_repaint();
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
            // 调用 schedule_browse_reload()：调度后台目录扫描。关键实现细节： 任务名是 filesreload。 优先级是 LOW。 栈大小是 6144。 栈使用 PSRAM。 结果放在 greloadjob。 greloadmtx 保护任务和结果对象。 完成后用 UiBus::SystemHint::FilesReloadDone 通知 UI。。
            schedule_browse_reload();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：loading_ 是否成立。
    if (loading_)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t base = browser_.has_parent_row() ? 1U : 0U;
    // 判断条件：idx < base 是否成立。
    if (idx < base)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t eidx = static_cast<uint8_t>(idx - base);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const VfsBrowser::Entry& e = browser_.entry_at(eidx);
    // 判断条件：e.name[0] == '\0' 是否成立。
    if (e.name[0] == '\0')
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 判断条件：e.is_dir 是否成立。
    if (e.is_dir)
    {
        // 判断条件：browser_.enter_dir(eidx) 是否成立。
        if (browser_.enter_dir(eidx))
            // 调用 schedule_browse_reload()：调度后台目录扫描。关键实现细节： 任务名是 filesreload。 优先级是 LOW。 栈大小是 6144。 栈使用 PSRAM。 结果放在 greloadjob。 greloadmtx 保护任务和结果对象。 完成后用 UiBus::SystemHint::FilesReloadDone 通知 UI。。
            schedule_browse_reload();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 调用 show_file_action(eidx)：用户点击文件后打开操作弹窗。它保存选中条目、路径和系统文件标记，然后设置 dlg。。
    show_file_action(eidx);
}

// 函数定义/声明：调用 FilesApp::open_selected_file()：根据选中文件的 OpenKind 路由到其他 App： 1. 通过对应 App 的请求接口传递路径，例如 requestopenonenter() 或 requestplayonenter()。 2. 调用 Router 切换到目标 App。。
void FilesApp::open_selected_file()
{
    // 先向目标 App 写入待打开路径，再切到目标 App 的 on_enter()。
    switch (classify_open(sel_entry_.name))
    {
        // 分支匹配：case OpenKind::Txt 成立时执行下面的代码。
        case OpenKind::Txt:
            // 调用 reader::ReaderApp::request_open_on_enter(sel_path_);，执行这个函数并处理它的返回值或状态变化。
            reader::ReaderApp::request_open_on_enter(sel_path_);
            // 访问对象成员：(void)router::Router::instance().navigate(router::page_app(AppId::Reader));；读取或修改这个对象里的成员状态。
            (void)router::Router::instance().navigate(router::page_app(AppId::Reader));
            // 调用 close_dialog()：把 dlg 设回 None，清除选中信息和信息行。它不负责重扫目录，因为关闭弹窗通常只是恢复界面。。
            close_dialog();
            // 提前返回：结束当前函数，不返回具体值。
            return;
        // 分支匹配：case OpenKind::Bmp 成立时执行下面的代码。
        case OpenKind::Bmp:
            // 调用 drawing::DrawingApp::request_open_on_enter(sel_path_);，执行这个函数并处理它的返回值或状态变化。
            drawing::DrawingApp::request_open_on_enter(sel_path_);
            // 访问对象成员：(void)router::Router::instance().navigate(router::page_app(AppId::Drawing));；读取或修改这个对象里的成员状态。
            (void)router::Router::instance().navigate(router::page_app(AppId::Drawing));
            // 调用 close_dialog()：把 dlg 设回 None，清除选中信息和信息行。它不负责重扫目录，因为关闭弹窗通常只是恢复界面。。
            close_dialog();
            // 提前返回：结束当前函数，不返回具体值。
            return;
        // 分支匹配：case OpenKind::Jpeg 成立时执行下面的代码。
        case OpenKind::Jpeg:
            // 调用 gallery::GalleryApp::request_open_on_enter(sel_path_);，执行这个函数并处理它的返回值或状态变化。
            gallery::GalleryApp::request_open_on_enter(sel_path_);
            // 访问对象成员：(void)router::Router::instance().navigate(router::page_app(AppId::Gallery));；读取或修改这个对象里的成员状态。
            (void)router::Router::instance().navigate(router::page_app(AppId::Gallery));
            // 调用 close_dialog()：把 dlg 设回 None，清除选中信息和信息行。它不负责重扫目录，因为关闭弹窗通常只是恢复界面。。
            close_dialog();
            // 提前返回：结束当前函数，不返回具体值。
            return;
        // 分支匹配：case OpenKind::Audio 成立时执行下面的代码。
        case OpenKind::Audio:
            // 调用 music::MusicApp::request_play_on_enter(sel_path_);，执行这个函数并处理它的返回值或状态变化。
            music::MusicApp::request_play_on_enter(sel_path_);
            // 访问对象成员：(void)router::Router::instance().navigate(router::page_app(AppId::Music));；读取或修改这个对象里的成员状态。
            (void)router::Router::instance().navigate(router::page_app(AppId::Music));
            // 调用 close_dialog()：把 dlg 设回 None，清除选中信息和信息行。它不负责重扫目录，因为关闭弹窗通常只是恢复界面。。
            close_dialog();
            // 提前返回：结束当前函数，不返回具体值。
            return;
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 调用 overlays::Toast::instance().show(ui::strings::kFilesOpenUnsupported, 2000);，执行这个函数并处理它的返回值或状态变化。
            overlays::Toast::instance().show(ui::strings::kFilesOpenUnsupported, 2000);
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            request_repaint();
            // 提前返回：结束当前函数，不返回具体值。
            return;
    }
}

// 函数定义/声明：调用 FilesApp::delete_selected_file()：执行删除前要确保： 不是目录。 不是系统文件。 路径已经由 VfsBrowser 校验。 删除失败时给出用户可见反馈。。
void FilesApp::delete_selected_file()
{
    // 删除是不可逆操作，系统文件保护失败时不调用底层 remove()。
    if (sel_is_system_)
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kFilesSystemProtected, 2000);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kFilesSystemProtected, 2000);
        // 赋值：把右侧 FileDlg::Action 的结果保存到左侧 dlg_。
        dlg_ = FileDlg::Action;
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：!browser_.delete_file(sel_idx_) 是否成立。
    if (!browser_.delete_file(sel_idx_))
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kFilesDeleteFail, 2000);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kFilesDeleteFail, 2000);
        // 赋值：把右侧 FileDlg::Action 的结果保存到左侧 dlg_。
        dlg_ = FileDlg::Action;
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 调用 close_dialog()：把 dlg 设回 None，清除选中信息和信息行。它不负责重扫目录，因为关闭弹窗通常只是恢复界面。。
    close_dialog();
    // 调用 schedule_browse_reload()：调度后台目录扫描。关键实现细节： 任务名是 filesreload。 优先级是 LOW。 栈大小是 6144。 栈使用 PSRAM。 结果放在 greloadjob。 greloadmtx 保护任务和结果对象。 完成后用 UiBus::SystemHint::FilesReloadDone 通知 UI。。
    schedule_browse_reload();
}

// 定义或声明函数 FilesApp::handle_dialog_tap：参数在括号里，返回类型写在函数名前面。
shell::InputResult FilesApp::handle_dialog_tap(int16_t x, int16_t y)
{
    // 判断条件：dlg_ != FileDlg::None && file_dlg_close_rect(dlg_).contains(x, y) 是否成立。
    if (dlg_ != FileDlg::None && file_dlg_close_rect(dlg_).contains(x, y))
    {
        // 调用 close_dialog()：把 dlg 设回 None，清除选中信息和信息行。它不负责重扫目录，因为关闭弹窗通常只是恢复界面。。
        close_dialog();
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 多分支判断：根据括号里的值选择不同 case。
    switch (dlg_)
    {
        // 分支匹配：case FileDlg::Action 成立时执行下面的代码。
        case FileDlg::Action:
            // 判断条件：file_action_btn_rect(0).contains(x, y) 是否成立。
            if (file_action_btn_rect(0).contains(x, y))
            {
                // 调用 build_info_lines()：生成最多四行信息文本，通常包含： 文件名。 类型。 大小。 路径或系统提示。。
                build_info_lines();
                // 赋值：把右侧 FileDlg::Info 的结果保存到左侧 dlg_。
                dlg_ = FileDlg::Info;
                // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
                request_repaint();
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            }
            // 判断条件：file_action_btn_rect(1).contains(x, y) 是否成立。
            if (file_action_btn_rect(1).contains(x, y))
            {
                // 调用 open_selected_file()：根据选中文件的 OpenKind 路由到其他 App： 1. 通过对应 App 的请求接口传递路径，例如 requestopenonenter() 或 requestplayonenter()。 2. 调用 Router 切换到目标 App。。
                open_selected_file();
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            }
            // 判断条件：file_action_btn_rect(2).contains(x, y) 是否成立。
            if (file_action_btn_rect(2).contains(x, y))
            {
                // 赋值：把右侧 FileDlg::DeleteConfirm 的结果保存到左侧 dlg_。
                dlg_ = FileDlg::DeleteConfirm;
                // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
                request_repaint();
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            }
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};

        // 分支匹配：case FileDlg::Info 成立时执行下面的代码。
        case FileDlg::Info:
            // 判断条件：file_info_back_rect().contains(x, y) 是否成立。
            if (file_info_back_rect().contains(x, y))
            {
                // 判断条件：sel_is_system_ 是否成立。
                if (sel_is_system_)
                    // 调用 close_dialog()：把 dlg 设回 None，清除选中信息和信息行。它不负责重扫目录，因为关闭弹窗通常只是恢复界面。。
                    close_dialog();
                // 备用分支：前面的条件不成立时执行这里。
                else
                    // 赋值：把右侧 FileDlg::Action 的结果保存到左侧 dlg_。
                    dlg_ = FileDlg::Action;
                // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
                request_repaint();
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            }
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};

        // 分支匹配：case FileDlg::DeleteConfirm:  成立时执行下面的代码。
        case FileDlg::DeleteConfirm: {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const ConfirmHit hit = ui::dialog::hit_confirm(file_dlg_rect(FileDlg::DeleteConfirm), x,
                                                           // 继续上一行的表达式或参数列表：y, true, kFileBtnH);。
                                                           y, true, kFileBtnH);
            // 关闭钮已在上面统一处理；这里只处理是/否
            if (hit == ConfirmHit::Yes)
            {
                // 调用 delete_selected_file()：执行删除前要确保： 不是目录。 不是系统文件。 路径已经由 VfsBrowser 校验。 删除失败时给出用户可见反馈。。
                delete_selected_file();
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            }
            // 判断条件：hit == ConfirmHit::No 是否成立。
            if (hit == ConfirmHit::No)
            {
                // 赋值：把右侧 FileDlg::Action 的结果保存到左侧 dlg_。
                dlg_ = FileDlg::Action;
                // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
                request_repaint();
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            }
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        }

        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 返回 {} 给调用者，表示当前函数的执行结果。
            return {};
    }
}

// 函数定义/声明：调用 FilesApp::handle_toolbar_back()：处理工具栏返回按钮。它的优先级和语义返回类似：先关弹窗，再回上一级，最后返回 false 让 Shell 退出。。
bool FilesApp::handle_toolbar_back()
{
    // 判断条件：dlg_ != FileDlg::None 是否成立。
    if (dlg_ != FileDlg::None)
    {
        // 判断条件：dlg_ == FileDlg::Info && !sel_is_system_ 是否成立。
        if (dlg_ == FileDlg::Info && !sel_is_system_)
            // 赋值：把右侧 FileDlg::Action 的结果保存到左侧 dlg_。
            dlg_ = FileDlg::Action;
        // 备用分支：前面的条件不成立时执行这里。
        else
            // 调用 close_dialog()：把 dlg 设回 None，清除选中信息和信息行。它不负责重扫目录，因为关闭弹窗通常只是恢复界面。。
            close_dialog();
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 判断条件：browser_.mode() == VfsBrowser::Mode::Roots 是否成立。
    if (browser_.mode() == VfsBrowser::Mode::Roots)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 调用 cancel_browse_reload()：取消或停止当前浏览重载。它需要避免旧任务完成后把已经过期的目录结果 adopt 到 UI。。
    cancel_browse_reload();
    // 调用 go_up()：返回上一级目录： 1. 如果当前是子目录，把 cwd 缩短到父目录。 2. 如果在根卷目录，回到 Roots。 3. 更新标题、父目录行和条目视图。。
    browser_.go_up();
    // 访问对象成员：list_.set_scroll(0);；读取或修改这个对象里的成员状态。
    list_.set_scroll(0);
    // 判断条件：browser_.mode() == VfsBrowser::Mode::Roots 是否成立。
    if (browser_.mode() == VfsBrowser::Mode::Roots)
    {
        // 调用 sync_list_total()：把 ListView 的总行数同步为 browser.rowcount()。当根卷数量变化、进入目录或删除文件后都要调用，否则列表的滚动范围会比实际行数多或少。。
        sync_list_total();
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
        // 调用 schedule_browse_reload()：调度后台目录扫描。关键实现细节： 任务名是 filesreload。 优先级是 LOW。 栈大小是 6144。 栈使用 PSRAM。 结果放在 greloadjob。 greloadmtx 保护任务和结果对象。 完成后用 UiBus::SystemHint::FilesReloadDone 通知 UI。。
        schedule_browse_reload();
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 FilesApp::paint_dlg_close_btn：参数在括号里，返回类型写在函数名前面。
void FilesApp::paint_dlg_close_btn(gfx::Canvas& c, FileDlg dlg)
{
    // 调用 file_dlg_rect(dlg)：返回不同弹窗的矩形区域。不同弹窗高度不同： 操作弹窗 92。 信息弹窗 118。 删除确认弹窗 76。。
    ui::dialog::paint_close_btn(c, file_dlg_rect(dlg));
}

// 定义或声明函数 FilesApp::paint_action_dialog：参数在括号里，返回类型写在函数名前面。
void FilesApp::paint_action_dialog(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect dlg = file_dlg_rect(FileDlg::Action);
    // 调用 ui::dialog::paint_frame(c, dlg);，执行这个函数并处理它的返回值或状态变化。
    ui::dialog::paint_frame(c, dlg);

    // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
    gfx::Canvas::TextStyle ts{};
    // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontBody;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
    ts.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
    ts.v = gfx::VAlign::Middle;
    // 赋值：把右侧 2 的结果保存到左侧 ts.padding。
    ts.padding = 2;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t title_w =
        // 继续上一行的表达式或参数列表：static_cast<uint16_t>(dlg.w - ui::dialog::kCloseBtnSize - ui::dialog::kCloseBtnPad * 2);。
        static_cast<uint16_t>(dlg.w - ui::dialog::kCloseBtnSize - ui::dialog::kCloseBtnPad * 2);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect title{dlg.x, static_cast<int16_t>(dlg.y + 4), title_w,
                           // 继续上一行的表达式或参数列表：static_cast<uint16_t>(dlg.h - kFileBtnH - 8)};。
                           static_cast<uint16_t>(dlg.h - kFileBtnH - 8)};
    // 调用 text_in(title,dlg_title_,ts)：文本对齐。
    c.text_in(title, dlg_title_, ts);

    // 静态查表：保存界面显示用的固定文字标签。
    static constexpr const char* kLabels[kActionBtnNum] = {
        // 执行这条代码：ui::strings::kFilesView,。它会更新变量、对象状态或调用链。
        ui::strings::kFilesView,
        // 继续上一行的表达式或参数列表：ui::strings::kFilesOpen,。
        ui::strings::kFilesOpen,
        // 继续上一行的表达式或参数列表：ui::strings::kFilesDelete,。
        ui::strings::kFilesDelete,
    };
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < kActionBtnNum; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect btn = file_action_btn_rect(i);
        // 调用 rect(btn,1)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
        c.rect(btn, 1);
        // 调用 text_in(btn,kLabels[i],ts)：文本对齐。
        c.text_in(btn, kLabels[i], ts);
        // 判断条件：i > 0 是否成立。
        if (i > 0)
            // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
            c.vline(static_cast<int16_t>(btn.x), static_cast<int16_t>(btn.y), kFileBtnH);
    }
}

// 定义或声明函数 FilesApp::paint_info_dialog：参数在括号里，返回类型写在函数名前面。
void FilesApp::paint_info_dialog(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect dlg = file_dlg_rect(FileDlg::Info);
    // 调用 ui::dialog::paint_frame(c, dlg);，执行这个函数并处理它的返回值或状态变化。
    ui::dialog::paint_frame(c, dlg);

    // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
    gfx::Canvas::TextStyle ts{};
    // 赋值：把右侧 Theme::kFontSmall 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontSmall;
    // 赋值：把右侧 gfx::HAlign::Left 的结果保存到左侧 ts.h。
    ts.h = gfx::HAlign::Left;
    // 赋值：把右侧 gfx::VAlign::Top 的结果保存到左侧 ts.v。
    ts.v = gfx::VAlign::Top;
    // 赋值：把右侧 2 的结果保存到左侧 ts.padding。
    ts.padding = 2;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t line_w =
        // 继续上一行的表达式或参数列表：static_cast<uint16_t>(dlg.w - ui::dialog::kCloseBtnSize - ui::dialog::kCloseBtnPad * 2 - 4);。
        static_cast<uint16_t>(dlg.w - ui::dialog::kCloseBtnSize - ui::dialog::kCloseBtnPad * 2 - 4);
    // 定义变量 int16_t y 并初始化为 static_cast<int16_t>(dlg.y + 6)。
    int16_t y = static_cast<int16_t>(dlg.y + 6);
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < 4; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect line{static_cast<int16_t>(dlg.x + 4), y, line_w, 14};
        // 调用 text_in(line,info_lines_[i],ts)：文本对齐。
        c.text_in(line, info_lines_[i], ts);
        // 赋值：把右侧 static_cast<int16_t>(y + 14) 的结果保存到左侧 y。
        y = static_cast<int16_t>(y + 14);
    }

    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const core::Rect back = file_info_back_rect();
    // 调用 rect(back,1)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    c.rect(back, 1);
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
    ts.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
    ts.v = gfx::VAlign::Middle;
    // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontBody;
    // 访问对象成员：c.text_in(back, ui::strings::kBack, ts);；读取或修改这个对象里的成员状态。
    c.text_in(back, ui::strings::kBack, ts);
}

// 定义或声明函数 FilesApp::paint_delete_dialog：参数在括号里，返回类型写在函数名前面。
void FilesApp::paint_delete_dialog(gfx::Canvas& c)
{
    // 关闭钮由 paint_file_dialog 统一绘制
    ui::dialog::paint_confirm(c, file_dlg_rect(FileDlg::DeleteConfirm),
                              // 继续上一行的表达式或参数列表：ui::strings::kFilesDeleteAsk, false, kFileBtnH);。
                              ui::strings::kFilesDeleteAsk, false, kFileBtnH);
}

// 定义或声明函数 FilesApp::paint_file_dialog：参数在括号里，返回类型写在函数名前面。
void FilesApp::paint_file_dialog(gfx::Canvas& c)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (dlg_)
    {
        // 分支匹配：case FileDlg::Action 成立时执行下面的代码。
        case FileDlg::Action:
            // 调用 paint_action_dialog(c);，执行这个函数并处理它的返回值或状态变化。
            paint_action_dialog(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case FileDlg::Info 成立时执行下面的代码。
        case FileDlg::Info:
            // 调用 paint_info_dialog(c);，执行这个函数并处理它的返回值或状态变化。
            paint_info_dialog(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case FileDlg::DeleteConfirm 成立时执行下面的代码。
        case FileDlg::DeleteConfirm:
            // 调用 paint_delete_dialog(c);，执行这个函数并处理它的返回值或状态变化。
            paint_delete_dialog(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }

    // 判断条件：dlg_ != FileDlg::None 是否成立。
    if (dlg_ != FileDlg::None)
        // 调用 paint_dlg_close_btn(c, dlg_);，执行这个函数并处理它的返回值或状态变化。
        paint_dlg_close_btn(c, dlg_);
}

// 定义或声明函数 FilesApp::paint：参数在括号里，返回类型写在函数名前面。
void FilesApp::paint(gfx::Canvas& c)
{
    // 调用 title()：返回 App 标题。在文件浏览器内，标题有时会动态反映当前目录，因此可能存在一个 buffer，而不是只返回一个常量字符串。。
    ui::widgets::toolbar(c, title());

    // 判断条件：loading_ && browser_.mode() == VfsBrowser::Mode::Browse 是否成立。
    if (loading_ && browser_.mode() == VfsBrowser::Mode::Browse)
    {
        // 访问对象成员：list_.paint(c);；读取或修改这个对象里的成员状态。
        list_.paint(c);
        // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
        gfx::Canvas::TextStyle ts{};
        // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 ts.size_px。
        ts.size_px = Theme::kFontBody;
        // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
        ts.h = gfx::HAlign::Center;
        // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
        ts.v = gfx::VAlign::Middle;
        // 访问对象成员：c.text_in(Theme::list_region(), ui::strings::kFilesLoading, ts);；读取或修改这个对象里的成员状态。
        c.text_in(Theme::list_region(), ui::strings::kFilesLoading, ts);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 访问对象成员：list_.paint(c);；读取或修改这个对象里的成员状态。
    list_.paint(c);

    // 判断条件：browser_.mode() == VfsBrowser::Mode::Roots && list_.total() == 0 是否成立。
    if (browser_.mode() == VfsBrowser::Mode::Roots && list_.total() == 0)
    {
        // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
        gfx::Canvas::TextStyle ts{};
        // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 ts.size_px。
        ts.size_px = Theme::kFontBody;
        // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
        ts.h = gfx::HAlign::Center;
        // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
        ts.v = gfx::VAlign::Middle;
        // 访问对象成员：c.text_in(Theme::list_region(), ui::strings::kFilesEmpty, ts);；读取或修改这个对象里的成员状态。
        c.text_in(Theme::list_region(), ui::strings::kFilesEmpty, ts);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：browser_.truncated() 是否成立。
    if (browser_.truncated())
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect hint{
            // 执行这条代码：Theme::kPadLg,。它会更新变量、对象状态或调用链。
            Theme::kPadLg,
            // 继续上一行的表达式或参数列表：static_cast<int16_t>(Theme::kListStartY + Theme::kListRegionH - Theme::kFontSmall - 2),。
            static_cast<int16_t>(Theme::kListStartY + Theme::kListRegionH - Theme::kFontSmall - 2),
            // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kScreenW - Theme::kPadLg * 2),。
            static_cast<uint16_t>(Theme::kScreenW - Theme::kPadLg * 2),
            // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kFontSmall + 2)};。
            static_cast<uint16_t>(Theme::kFontSmall + 2)};
        // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
        gfx::Canvas::TextStyle ts{};
        // 赋值：把右侧 Theme::kFontSmall 的结果保存到左侧 ts.size_px。
        ts.size_px = Theme::kFontSmall;
        // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
        ts.h = gfx::HAlign::Center;
        // 赋值：把右侧 gfx::VAlign::Bottom 的结果保存到左侧 ts.v。
        ts.v = gfx::VAlign::Bottom;
        // 访问对象成员：c.text_in(hint, ui::strings::kFilesTruncated, ts);；读取或修改这个对象里的成员状态。
        c.text_in(hint, ui::strings::kFilesTruncated, ts);
    }
}

// 定义或声明函数 FilesApp::paint_overlay：参数在括号里，返回类型写在函数名前面。
void FilesApp::paint_overlay(gfx::Canvas& c)
{
    // 判断条件：dlg_ != FileDlg::None 是否成立。
    if (dlg_ != FileDlg::None)
        // 调用 paint_file_dialog(c);，执行这个函数并处理它的返回值或状态变化。
        paint_file_dialog(c);
}

// 函数定义/声明：调用 FilesApp::on_input()：FilesApp::oninput()。
shell::InputResult FilesApp::on_input(const ::app::ebook::input::Event& ev)
{
    // 输入优先级：弹窗 -> 工具栏返回 -> 列表滚动 -> 行点击。
    using ::app::ebook::input::EventType;

    // 判断条件：dlg_ != FileDlg::None 是否成立。
    if (dlg_ != FileDlg::None)
    {
        // 判断条件：ev.type == EventType::Tap 是否成立。
        if (ev.type == EventType::Tap)
            // 返回 handle_dialog_tap(static_cast<int16_t>(ev.x), static_cast<int16_t>(ev.y)) 给调用者，表示当前函数的执行结果。
            return handle_dialog_tap(static_cast<int16_t>(ev.x), static_cast<int16_t>(ev.y));
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 判断条件：ev.type == EventType::Tap & 是否成立。
    if (ev.type == EventType::Tap &&
        // 函数定义/声明：调用 ui::widgets::hit_toolbar_back()：toolbar()： 在固定工具栏矩形中画返回图标、标题和底部横线。返回图标使用 Font Awesome 的码点，而不是手绘 SVG。。
        ui::widgets::hit_toolbar_back(static_cast<int16_t>(ev.x), static_cast<int16_t>(ev.y)))
    {
        // 判断条件：handle_toolbar_back() 是否成立。
        if (handle_toolbar_back())
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto out = list_.handle_input(ev);
    // 判断条件：out.scroll_changed 是否成立。
    if (out.scroll_changed)
        // 调用 request_scroll_repaint()：限制滚动时的重绘频率。代码使用约 120 ms 的节流值，避免用户连续滑动时每个触摸事件都触发一次墨水屏刷新。。
        request_scroll_repaint();
    // 判断条件：out.consumed 是否成立。
    if (out.consumed)
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    // 返回 {} 给调用者，表示当前函数的执行结果。
    return {};
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::files
