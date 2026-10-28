/**
 * [初学者导读] 文件：main/app/ebook/apps/notepad/notepad_app.cc
 * 作用：实现记事本编辑、保存、列表和键盘输入。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/notepad/notepad_app.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <algorithm>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <sys/stat.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <unistd.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <dirent.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "common/storage/storage.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/icon.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/text_layout.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/keyboard.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/toast.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/dialog.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "memory/psram_alloc.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::notepad {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using app::common::memory::alloc_array_psram;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using app::common::memory::alloc_psram_prefer;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ::app::common::storage::MountKind;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ::app::common::storage::StorageMgr;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::dialog::ConfirmHit;

// 固定参数 kEbookRootInt = "/int/Ebook"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kEbookRootInt = "/int/Ebook";
// 固定参数 kEbookRootSd = "/sd/Ebook"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kEbookRootSd = "/sd/Ebook";
// 固定参数 kNotesDirInt = NotepadApp::kScanPathInt：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kNotesDirInt = NotepadApp::kScanPathInt;
// 固定参数 kNotesDirSd = NotepadApp::kScanPathSd：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kNotesDirSd = NotepadApp::kScanPathSd;

// 定义或声明函数 is_txt_name：参数在括号里，返回类型写在函数名前面。
bool is_txt_name(const char* name)
{
    // 判断条件：name == nullptr || name[0] == '.' 是否成立。
    if (name == nullptr || name[0] == '.')
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t len = std::strlen(name);
    // 返回 (len > 4U && std::strcmp(name + len - 4U, ".txt") == 0) 给调用者，表示当前函数的执行结果。
    return (len > 4U && std::strcmp(name + len - 4U, ".txt") == 0);
}

// 定义或声明函数 mkdir_ebook_notes：参数在括号里，返回类型写在函数名前面。
void mkdir_ebook_notes(const char* root, const char* notes_dir)
{
    // 函数调用：(void)::mkdir(root, 0775);；执行该函数并处理它的返回值或副作用。
    (void)::mkdir(root, 0775);
    // 函数调用：(void)::mkdir(notes_dir, 0775);；执行该函数并处理它的返回值或副作用。
    (void)::mkdir(notes_dir, 0775);
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 NotepadApp::instance：参数在括号里，返回类型写在函数名前面。
NotepadApp& NotepadApp::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static NotepadApp s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义或声明函数 NotepadApp::ensure_store：参数在括号里，返回类型写在函数名前面。
bool NotepadApp::ensure_store()
{
    // 判断条件：body_ != nullptr && notes_ != nullptr && size_buf_ != nullptr 是否成立。
    if (body_ != nullptr && notes_ != nullptr && size_buf_ != nullptr)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;

    // 判断条件：notes_ == nullptr 是否成立。
    if (notes_ == nullptr)
        // 赋值：把右侧 alloc_array_psram<NoteItem>(kMaxNotes) 的结果保存到左侧 notes_。
        notes_ = alloc_array_psram<NoteItem>(kMaxNotes);
    // 判断条件：size_buf_ == nullptr 是否成立。
    if (size_buf_ == nullptr)
        // 赋值：把右侧 static_cast<char (*)[12]>(alloc_psram_prefer(sizeof(char[kMaxNotes][12]))) 的结果保存到左侧 size_buf_。
        size_buf_ = static_cast<char (*)[12]>(alloc_psram_prefer(sizeof(char[kMaxNotes][12])));
    // 判断条件：body_ == nullptr 是否成立。
    if (body_ == nullptr)
    {
        // 赋值：把右侧 static_cast<char*>(alloc_psram_prefer(kBodyCap)) 的结果保存到左侧 body_。
        body_ = static_cast<char*>(alloc_psram_prefer(kBodyCap));
        // 判断条件：body_ != nullptr 是否成立。
        if (body_ != nullptr)
            // 赋值：把右侧 '\0' 的结果保存到左侧 body_[0]。
            body_[0] = '\0';
    }
    // 返回 notes_ != nullptr && size_buf_ != nullptr && body_ != nullptr 给调用者，表示当前函数的执行结果。
    return notes_ != nullptr && size_buf_ != nullptr && body_ != nullptr;
}

// 定义或声明函数 NotepadApp::NotepadApp：参数在括号里，返回类型写在函数名前面。
NotepadApp::NotepadApp()
{
    // 函数调用：(void)ensure_store();；执行该函数并处理它的返回值或副作用。
    (void)ensure_store();
    // 访问对象成员：list_.set_area(Theme::list_region());；读取或修改这个对象里的成员状态。
    list_.set_area(Theme::list_region());
    // 访问对象成员：list_.set_provider([this](uint8_t i, ui::widgets::RowStyle& rs) { fill_row(i, rs); });；读取或修改这个对象里的成员状态。
    list_.set_provider([this](uint8_t i, ui::widgets::RowStyle& rs) { fill_row(i, rs); });
    // 调用 on_row_tap(i)：处理列表点击： 如果点击父目录行，调用 goup()。 如果点击根卷，调用 openroot() 并调度扫描。 如果点击目录，调用 enterdir()。 如果点击文件，调用 showfileaction()。。
    list_.set_tap_handler([this](uint8_t i) { on_row_tap(i); });
}

// 定义只读对象或变量：初始化后不能在当前位置修改。
const char* NotepadApp::title() const
{
    // 判断条件：view_ == View::Edit && title_buf_[0] != '\0' 是否成立。
    if (view_ == View::Edit && title_buf_[0] != '\0')
        // 返回 title_buf_ 给调用者，表示当前函数的执行结果。
        return title_buf_;
    // 返回 ui::strings::kAppNotepad 给调用者，表示当前函数的执行结果。
    return ui::strings::kAppNotepad;
}

// 函数定义/声明：调用 NotepadApp::icon_cp()：返回图标 codepoint。图标通常来自字体图标表，所以这里返回的是 uint32t，不是图片路径。。
uint32_t NotepadApp::icon_cp() const
{
    // 返回 gfx::icon::kFaStickyNote 给调用者，表示当前函数的执行结果。
    return gfx::icon::kFaStickyNote;
}

// 定义或声明函数 NotepadApp::on_enter：参数在括号里，返回类型写在函数名前面。
void NotepadApp::on_enter()
{
    // 函数调用：(void)ensure_store();；执行该函数并处理它的返回值或副作用。
    (void)ensure_store();
    // 赋值：把右侧 View::List 的结果保存到左侧 view_。
    view_ = View::List;
    // 赋值：把右侧 KbMode::None 的结果保存到左侧 kb_mode_。
    kb_mode_ = KbMode::None;
    // 赋值：把右侧 Dlg::None 的结果保存到左侧 dlg_。
    dlg_ = Dlg::None;
    // 赋值：把右侧 false 的结果保存到左侧 dirty_。
    dirty_ = false;
    // 赋值：把右侧 0 的结果保存到左侧 scroll_line_。
    scroll_line_ = 0;
    // 赋值：把右侧 '\0' 的结果保存到左侧 path_[0]。
    path_[0] = '\0';
    // 赋值：把右侧 '\0' 的结果保存到左侧 title_buf_[0]。
    title_buf_[0] = '\0';
    // 判断条件：body_ != nullptr 是否成立。
    if (body_ != nullptr)
        // 赋值：把右侧 '\0' 的结果保存到左侧 body_[0]。
        body_[0] = '\0';
    // 访问对象成员：list_.set_scroll(0);；读取或修改这个对象里的成员状态。
    list_.set_scroll(0);
    // 调用 scan_notes();，执行这个函数并处理它的返回值或状态变化。
    scan_notes();
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 函数定义/声明：调用 NotepadApp::on_exit()：退出时取消未完成的扫描、关闭弹窗，并确保后台任务不会在对象状态失效后继续写数据。。
void NotepadApp::on_exit()
{
    // 判断条件：overlays::Keyboard::instance().is_open() 是否成立。
    if (overlays::Keyboard::instance().is_open())
        // 调用 close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
        overlays::Keyboard::instance().close();
    // 赋值：把右侧 Dlg::None 的结果保存到左侧 dlg_。
    dlg_ = Dlg::None;
    // 判断条件：view_ == View::Edit && dirty_ 是否成立。
    if (view_ == View::Edit && dirty_)
        // 调用 save_note();，执行这个函数并处理它的返回值或状态变化。
        save_note();
    // 赋值：把右侧 View::List 的结果保存到左侧 view_。
    view_ = View::List;
    // 赋值：把右侧 KbMode::None 的结果保存到左侧 kb_mode_。
    kb_mode_ = KbMode::None;
}

// 函数定义/声明：调用 NotepadApp::on_semantic_back()：处理语义返回，顺序通常是： 1. 如果弹窗打开，先关闭弹窗。 2. 如果列表还在浏览目录，先返回上一级。 3. 如果已经到根卷，返回 false，让 Shell 退出 App。。
bool NotepadApp::on_semantic_back()
{
    // 键盘打开时 close() 不触发回调，等同取消输入
    if (overlays::Keyboard::instance().is_open())
    {
        // 调用 close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
        overlays::Keyboard::instance().close();
        // 赋值：把右侧 KbMode::None 的结果保存到左侧 kb_mode_。
        kb_mode_ = KbMode::None;
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 判断条件：dlg_ != Dlg::None 是否成立。
    if (dlg_ != Dlg::None)
    {
        // 调用 dismiss_dialog_back();，执行这个函数并处理它的返回值或状态变化。
        dismiss_dialog_back();
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 判断条件：view_ == View::Edit 是否成立。
    if (view_ == View::Edit)
    {
        // 调用 leave_edit_to_list();，执行这个函数并处理它的返回值或状态变化。
        leave_edit_to_list();
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 列表页：交给上层退出 App
    return false;
}

// 定义或声明函数 NotepadApp::leave_edit_to_list：参数在括号里，返回类型写在函数名前面。
bool NotepadApp::leave_edit_to_list()
{
    // 保存失败则留在编辑页，避免清空内存里尚未落盘的内容
    if (dirty_ && !save_note())
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 View::List 的结果保存到左侧 view_。
    view_ = View::List;
    // 赋值：把右侧 Dlg::None 的结果保存到左侧 dlg_。
    dlg_ = Dlg::None;
    // 赋值：把右侧 0 的结果保存到左侧 scroll_line_。
    scroll_line_ = 0;
    // 赋值：把右侧 '\0' 的结果保存到左侧 path_[0]。
    path_[0] = '\0';
    // 赋值：把右侧 '\0' 的结果保存到左侧 title_buf_[0]。
    title_buf_[0] = '\0';
    // 判断条件：body_ != nullptr 是否成立。
    if (body_ != nullptr)
        // 赋值：把右侧 '\0' 的结果保存到左侧 body_[0]。
        body_[0] = '\0';
    // 赋值：把右侧 false 的结果保存到左侧 dirty_。
    dirty_ = false;
    // 调用 scan_notes();，执行这个函数并处理它的返回值或状态变化。
    scan_notes();
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 NotepadApp::abort_edit_to_list：参数在括号里，返回类型写在函数名前面。
void NotepadApp::abort_edit_to_list()
{
    // 赋值：把右侧 View::List 的结果保存到左侧 view_。
    view_ = View::List;
    // 赋值：把右侧 Dlg::None 的结果保存到左侧 dlg_。
    dlg_ = Dlg::None;
    // 赋值：把右侧 0 的结果保存到左侧 scroll_line_。
    scroll_line_ = 0;
    // 赋值：把右侧 '\0' 的结果保存到左侧 path_[0]。
    path_[0] = '\0';
    // 赋值：把右侧 '\0' 的结果保存到左侧 title_buf_[0]。
    title_buf_[0] = '\0';
    // 判断条件：body_ != nullptr 是否成立。
    if (body_ != nullptr)
        // 赋值：把右侧 '\0' 的结果保存到左侧 body_[0]。
        body_[0] = '\0';
    // 赋值：把右侧 false 的结果保存到左侧 dirty_。
    dirty_ = false;
    // 调用 scan_notes();，执行这个函数并处理它的返回值或状态变化。
    scan_notes();
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 定义或声明函数 NotepadApp::rename_note_to_stem：参数在括号里，返回类型写在函数名前面。
bool NotepadApp::rename_note_to_stem(const char* stem_in)
{
    // 判断条件：path_[0] == '\0' || stem_in == nullptr 是否成立。
    if (path_[0] == '\0' || stem_in == nullptr)
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kNoteRenameFail, 1500);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kNoteRenameFail, 1500);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 执行语句：char stem[36]{};；它会更新一次变量、对象或程序状态。
    char stem[36]{};
    // 调用 sanitize_filename(stem_in, stem, sizeof(stem));，执行这个函数并处理它的返回值或状态变化。
    sanitize_filename(stem_in, stem, sizeof(stem));
    // 判断条件：std::strcmp(stem, title_buf_) == 0 是否成立。
    if (std::strcmp(stem, title_buf_) == 0)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;

    // 执行语句：char dir[128]{};；它会更新一次变量、对象或程序状态。
    char dir[128]{};
    // 判断条件：!parent_dir_of(path_, dir, sizeof(dir)) 是否成立。
    if (!parent_dir_of(path_, dir, sizeof(dir)))
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kNoteRenameFail, 1500);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kNoteRenameFail, 1500);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 执行语句：char new_path[128]{};；它会更新一次变量、对象或程序状态。
    char new_path[128]{};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int plen = std::snprintf(new_path, sizeof(new_path), "%s/%s.txt", dir, stem);
    // 判断条件：plen < 0 || static_cast<size_t>(plen) >= sizeof(new_path) 是否成立。
    if (plen < 0 || static_cast<size_t>(plen) >= sizeof(new_path))
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kNoteRenameFail, 1500);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kNoteRenameFail, 1500);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 判断条件：std::strcmp(new_path, path_) != 0 是否成立。
    if (std::strcmp(new_path, path_) != 0)
    {
        // struct stat：把相关数据字段组合在一起，表示项目中的一个概念。
        struct stat st{};
        // 判断条件：::stat(new_path, &st) == 0 是否成立。
        if (::stat(new_path, &st) == 0)
        {
            // 调用 overlays::Toast::instance().show(ui::strings::kNoteRenameFail, 1500);，执行这个函数并处理它的返回值或状态变化。
            overlays::Toast::instance().show(ui::strings::kNoteRenameFail, 1500);
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }
    }

    // 判断条件：dirty_ && !save_note() 是否成立。
    if (dirty_ && !save_note())
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 判断条件：std::strcmp(new_path, path_) != 0 是否成立。
    if (std::strcmp(new_path, path_) != 0)
    {
        // 判断条件：::rename(path_, new_path) != 0 是否成立。
        if (::rename(path_, new_path) != 0)
        {
            // 调用 overlays::Toast::instance().show(ui::strings::kNoteRenameFail, 1500);，执行这个函数并处理它的返回值或状态变化。
            overlays::Toast::instance().show(ui::strings::kNoteRenameFail, 1500);
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }
        // 函数调用：(void)std::strncpy(path_, new_path, sizeof(path_) - 1);；执行该函数并处理它的返回值或副作用。
        (void)std::strncpy(path_, new_path, sizeof(path_) - 1);
        // 赋值：把右侧 '\0' 的结果保存到左侧 path_[sizeof(path_) - 1]。
        path_[sizeof(path_) - 1] = '\0';
    }

    // 调用 title_from_path(path_, title_buf_, sizeof(title_buf_));，执行这个函数并处理它的返回值或状态变化。
    title_from_path(path_, title_buf_, sizeof(title_buf_));
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 NotepadApp::delete_current_note：参数在括号里，返回类型写在函数名前面。
bool NotepadApp::delete_current_note()
{
    // 判断条件：path_[0] == '\0' 是否成立。
    if (path_[0] == '\0')
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kNoteDeleteFail, 1500);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kNoteDeleteFail, 1500);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 判断条件：::unlink(path_) != 0 是否成立。
    if (::unlink(path_) != 0)
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kNoteDeleteFail, 1500);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kNoteDeleteFail, 1500);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 调用 abort_edit_to_list();，执行这个函数并处理它的返回值或状态变化。
    abort_edit_to_list();
    // 调用 overlays::Toast::instance().show(ui::strings::kNoteDeleted, 1500);，执行这个函数并处理它的返回值或状态变化。
    overlays::Toast::instance().show(ui::strings::kNoteDeleted, 1500);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 NotepadApp::edit_max_scroll：参数在括号里，返回类型写在函数名前面。
uint8_t NotepadApp::edit_max_scroll() const
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect body = edit_body_rect();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* text = (body_ != nullptr) ? body_ : "";
    // 执行语句：gfx::LineSlice lines[kWrapLines]{};；它会更新一次变量、对象或程序状态。
    gfx::LineSlice lines[kWrapLines]{};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t line_count =
        // 继续上一行的表达式或参数列表：gfx::wrap_text(text, Theme::kFontBody, body.w, kWrapLines, lines, gfx::FontFace::Text);。
        gfx::wrap_text(text, Theme::kFontBody, body.w, kWrapLines, lines, gfx::FontFace::Text);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t line_h = static_cast<uint8_t>(Theme::kFontBody + 4);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t max_visible = static_cast<uint8_t>((body.h + line_h - 1) / line_h);
    // 判断条件：line_count > max_visible 是否成立。
    if (line_count > max_visible)
        // 返回 static_cast<uint8_t>(line_count - max_visible) 给调用者，表示当前函数的执行结果。
        return static_cast<uint8_t>(line_count - max_visible);
    // 返回 0 给调用者，表示当前函数的执行结果。
    return 0;
}

// 定义或声明函数 NotepadApp::clamp_scroll：参数在括号里，返回类型写在函数名前面。
void NotepadApp::clamp_scroll()
{
    // 赋值：把右侧 std::min(scroll_line_, edit_max_scroll()) 的结果保存到左侧 scroll_line_。
    scroll_line_ = std::min(scroll_line_, edit_max_scroll());
}

// 定义或声明函数 NotepadApp::ensure_notes_dirs：参数在括号里，返回类型写在函数名前面。
bool NotepadApp::ensure_notes_dirs()
{
    // 赋值：把右侧 StorageMgr::get_instance() 的结果保存到左侧 auto& sto。
    auto& sto = StorageMgr::get_instance();
    // 定义变量 bool ok 并初始化为 false。
    bool ok = false;

    // 判断条件：sto.is_mounted(MountKind::Internal) 是否成立。
    if (sto.is_mounted(MountKind::Internal))
    {
        // 调用 mkdir_ebook_notes(kEbookRootInt, kNotesDirInt);，执行这个函数并处理它的返回值或状态变化。
        mkdir_ebook_notes(kEbookRootInt, kNotesDirInt);
        // struct stat：把相关数据字段组合在一起，表示项目中的一个概念。
        struct stat st{};
        // 判断条件：::stat(kNotesDirInt, &st) == 0 是否成立。
        if (::stat(kNotesDirInt, &st) == 0)
            // 赋值：把右侧 true 的结果保存到左侧 ok。
            ok = true;
    }
    // 判断条件：sto.is_mounted(MountKind::Sd) 是否成立。
    if (sto.is_mounted(MountKind::Sd))
    {
        // 调用 mkdir_ebook_notes(kEbookRootSd, kNotesDirSd);，执行这个函数并处理它的返回值或状态变化。
        mkdir_ebook_notes(kEbookRootSd, kNotesDirSd);
        // struct stat：把相关数据字段组合在一起，表示项目中的一个概念。
        struct stat st{};
        // 判断条件：::stat(kNotesDirSd, &st) == 0 是否成立。
        if (::stat(kNotesDirSd, &st) == 0)
            // 赋值：把右侧 true 的结果保存到左侧 ok。
            ok = true;
    }
    // 返回 ok 给调用者，表示当前函数的执行结果。
    return ok;
}

// 只读约束：这个值或对象在当前位置不能被修改。
const char* NotepadApp::preferred_notes_dir()
{
    // 赋值：把右侧 StorageMgr::get_instance() 的结果保存到左侧 auto& sto。
    auto& sto = StorageMgr::get_instance();
    // 判断条件：sto.is_mounted(MountKind::Internal) 是否成立。
    if (sto.is_mounted(MountKind::Internal))
        // 返回 kNotesDirInt 给调用者，表示当前函数的执行结果。
        return kNotesDirInt;
    // 判断条件：sto.is_mounted(MountKind::Sd) 是否成立。
    if (sto.is_mounted(MountKind::Sd))
        // 返回 kNotesDirSd 给调用者，表示当前函数的执行结果。
        return kNotesDirSd;
    // 返回 kNotesDirInt 给调用者，表示当前函数的执行结果。
    return kNotesDirInt;
}

// 定义或声明函数 NotepadApp::parent_dir_of：参数在括号里，返回类型写在函数名前面。
bool NotepadApp::parent_dir_of(const char* path, char* out, size_t cap)
{
    // 判断条件：path == nullptr || out == nullptr || cap == 0 是否成立。
    if (path == nullptr || out == nullptr || cap == 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* slash = std::strrchr(path, '/');
    // 判断条件：slash == nullptr || slash == path 是否成立。
    if (slash == nullptr || slash == path)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t n = static_cast<size_t>(slash - path);
    // 判断条件：n + 1U > cap 是否成立。
    if (n + 1U > cap)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 调用 std::memcpy(out, path, n);，执行这个函数并处理它的返回值或状态变化。
    std::memcpy(out, path, n);
    // 赋值：把右侧 '\0' 的结果保存到左侧 out[n]。
    out[n] = '\0';
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 NotepadApp::title_from_path：参数在括号里，返回类型写在函数名前面。
void NotepadApp::title_from_path(const char* path, char* out, size_t out_size)
{
    // 判断条件：path == nullptr || out == nullptr || out_size == 0 是否成立。
    if (path == nullptr || out == nullptr || out_size == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* name = std::strrchr(path, '/');
    // 赋值：把右侧 (name != nullptr) ? (name + 1) : path 的结果保存到左侧 name。
    name = (name != nullptr) ? (name + 1) : path;
    // 函数调用：(void)std::strncpy(out, name, out_size - 1);；执行该函数并处理它的返回值或副作用。
    (void)std::strncpy(out, name, out_size - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 out[out_size - 1]。
    out[out_size - 1] = '\0';
    // 赋值：把右侧 std::strrchr(out, '.') 的结果保存到左侧 char* dot。
    char* dot = std::strrchr(out, '.');
    // 判断条件：dot != nullptr 是否成立。
    if (dot != nullptr)
        *dot = '\0';
}

// 定义或声明函数 NotepadApp::sanitize_filename：参数在括号里，返回类型写在函数名前面。
void NotepadApp::sanitize_filename(const char* in, char* out, size_t out_size)
{
    // 判断条件：out == nullptr || out_size == 0 是否成立。
    if (out == nullptr || out_size == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：in == nullptr || in[0] == '\0' 是否成立。
    if (in == nullptr || in[0] == '\0')
    {
        // 函数调用：(void)std::strncpy(out, "note", out_size - 1);；执行该函数并处理它的返回值或副作用。
        (void)std::strncpy(out, "note", out_size - 1);
        // 赋值：把右侧 '\0' 的结果保存到左侧 out[out_size - 1]。
        out[out_size - 1] = '\0';
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 定义变量 size_t w 并初始化为 0。
    size_t w = 0;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (size_t i = 0; in[i] != '\0' && w + 1 < out_size; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const unsigned char c = static_cast<unsigned char>(in[i]);
        // 判断条件：c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' | 是否成立。
        if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' ||
            // 继续上一行的表达式或参数列表：c == '>' || c == '|')。
            c == '>' || c == '|')
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        // 赋值：把右侧 static_cast<char>(c) 的结果保存到左侧 out[w++]。
        out[w++] = static_cast<char>(c);
    }
    // 赋值：把右侧 '\0' 的结果保存到左侧 out[w]。
    out[w] = '\0';
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (w > 0 && out[w - 1] == ' ')
    {
        // 赋值：把右侧 '\0' 的结果保存到左侧 out[w - 1]。
        out[w - 1] = '\0';
        // 执行语句：--w;；它会更新一次变量、对象或程序状态。
        --w;
    }
    // 判断条件：w == 0 是否成立。
    if (w == 0)
    {
        // 函数调用：(void)std::strncpy(out, "note", out_size - 1);；执行该函数并处理它的返回值或副作用。
        (void)std::strncpy(out, "note", out_size - 1);
        // 赋值：把右侧 '\0' 的结果保存到左侧 out[out_size - 1]。
        out[out_size - 1] = '\0';
    }
}

// 定义或声明函数 NotepadApp::scan_dir：参数在括号里，返回类型写在函数名前面。
void NotepadApp::scan_dir(const char* dir)
{
    // 判断条件：dir == nullptr || notes_ == nullptr || count_ >= kMaxNotes 是否成立。
    if (dir == nullptr || notes_ == nullptr || count_ >= kMaxNotes)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // struct stat：把相关数据字段组合在一起，表示项目中的一个概念。
    struct stat st{};
    // 判断条件：::stat(dir, &st) != 0 是否成立。
    if (::stat(dir, &st) != 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 ::opendir(dir) 的结果保存到左侧 DIR* d。
    DIR* d = ::opendir(dir);
    // 判断条件：d == nullptr 是否成立。
    if (d == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // struct dirent：把相关数据字段组合在一起，表示项目中的一个概念。
    struct dirent* ent = nullptr;
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while ((ent = ::readdir(d)) != nullptr && count_ < kMaxNotes)
    {
        // 判断条件：!is_txt_name(ent->d_name) 是否成立。
        if (!is_txt_name(ent->d_name))
            // 继续下一轮：跳过本次循环剩余代码。
            continue;

        // 赋值：把右侧 notes_[count_] 的结果保存到左侧 NoteItem& item。
        NoteItem& item = notes_[count_];
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int plen = std::snprintf(item.path, sizeof(item.path), "%s/%s", dir, ent->d_name);
        // 判断条件：plen < 0 || static_cast<size_t>(plen) >= sizeof(item.path) 是否成立。
        if (plen < 0 || static_cast<size_t>(plen) >= sizeof(item.path))
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        // 调用 title_from_path(item.path, item.title, sizeof(item.title));，执行这个函数并处理它的返回值或状态变化。
        title_from_path(item.path, item.title, sizeof(item.title));

        // 判断条件：::stat(item.path, &st) == 0 是否成立。
        if (::stat(item.path, &st) == 0)
            // 赋值：把右侧 static_cast<uint32_t>(st.st_size) 的结果保存到左侧 item.size。
            item.size = static_cast<uint32_t>(st.st_size);
        // 备用分支：前面的条件不成立时执行这里。
        else
            // 赋值：把右侧 0 的结果保存到左侧 item.size。
            item.size = 0;

        // 执行语句：++count_;；它会更新一次变量、对象或程序状态。
        ++count_;
    }
    // 函数调用：::closedir(d);；执行该函数并处理它的返回值或副作用。
    ::closedir(d);
}

// 定义或声明函数 NotepadApp::scan_notes：参数在括号里，返回类型写在函数名前面。
void NotepadApp::scan_notes()
{
    // 赋值：把右侧 0 的结果保存到左侧 count_。
    count_ = 0;
    // 判断条件：!ensure_store() 是否成立。
    if (!ensure_store())
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 函数调用：(void)ensure_notes_dirs();；执行该函数并处理它的返回值或副作用。
    (void)ensure_notes_dirs();

    // 赋值：把右侧 StorageMgr::get_instance() 的结果保存到左侧 auto& sto。
    auto& sto = StorageMgr::get_instance();
    // 判断条件：sto.is_mounted(MountKind::Internal) 是否成立。
    if (sto.is_mounted(MountKind::Internal))
        // 调用 scan_dir(kNotesDirInt);，执行这个函数并处理它的返回值或状态变化。
        scan_dir(kNotesDirInt);
    // 判断条件：sto.is_mounted(MountKind::Sd) 是否成立。
    if (sto.is_mounted(MountKind::Sd))
        // 调用 scan_dir(kNotesDirSd);，执行这个函数并处理它的返回值或状态变化。
        scan_dir(kNotesDirSd);

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i + 1 < count_; ++i)
    {
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint8_t j = 0; j + 1 < count_ - i; ++j)
        {
            // 判断条件：std::strcmp(notes_[j].title, notes_[j + 1].title) > 0 是否成立。
            if (std::strcmp(notes_[j].title, notes_[j + 1].title) > 0)
            {
                // 定义变量 NoteItem tmp 并初始化为 notes_[j]。
                NoteItem tmp = notes_[j];
                // 赋值：把右侧 notes_[j + 1] 的结果保存到左侧 notes_[j]。
                notes_[j] = notes_[j + 1];
                // 赋值：把右侧 tmp 的结果保存到左侧 notes_[j + 1]。
                notes_[j + 1] = tmp;
            }
        }
    }
}

// 定义或声明函数 NotepadApp::alloc_new_path：参数在括号里，返回类型写在函数名前面。
bool NotepadApp::alloc_new_path(char* out, size_t cap)
{
    // 判断条件：out == nullptr || cap == 0 是否成立。
    if (out == nullptr || cap == 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* dir = preferred_notes_dir();
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint16_t i = 1; i <= 999; ++i)
    {
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        (void)std::snprintf(out, cap, "%s/note_%03u.txt", dir, static_cast<unsigned>(i));
        // struct stat：把相关数据字段组合在一起，表示项目中的一个概念。
        struct stat st{};
        // 判断条件：::stat(out, &st) != 0 是否成立。
        if (::stat(out, &st) != 0)
            // 返回 true 给调用者，表示当前函数的执行结果。
            return true;
    }
    // 返回 false 给调用者，表示当前函数的执行结果。
    return false;
}

// 定义或声明函数 NotepadApp::load_note：参数在括号里，返回类型写在函数名前面。
bool NotepadApp::load_note(const char* path)
{
    // 判断条件：path == nullptr || !ensure_store() 是否成立。
    if (path == nullptr || !ensure_store())
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 std::fopen(path, "rb") 的结果保存到左侧 FILE* f。
    FILE* f = std::fopen(path, "rb");
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t n = std::fread(body_, 1, kBodyCap - 1, f);
    // 函数调用：(void)std::fclose(f);；执行该函数并处理它的返回值或副作用。
    (void)std::fclose(f);
    // 赋值：把右侧 '\0' 的结果保存到左侧 body_[n]。
    body_[n] = '\0';
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 NotepadApp::save_note：参数在括号里，返回类型写在函数名前面。
bool NotepadApp::save_note()
{
    // 判断条件：path_[0] == '\0' || body_ == nullptr 是否成立。
    if (path_[0] == '\0' || body_ == nullptr)
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kNoteSaveFail, 1500);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kNoteSaveFail, 1500);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 判断条件：!ensure_notes_dirs() 是否成立。
    if (!ensure_notes_dirs())
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kNoteSaveFail, 1500);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kNoteSaveFail, 1500);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 std::fopen(path_, "wb") 的结果保存到左侧 FILE* f。
    FILE* f = std::fopen(path_, "wb");
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kNoteSaveFail, 1500);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kNoteSaveFail, 1500);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t len = std::strlen(body_);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t w = std::fwrite(body_, 1, len, f);
    // 函数调用：(void)std::fclose(f);；执行该函数并处理它的返回值或副作用。
    (void)std::fclose(f);

    // 判断条件：w != len 是否成立。
    if (w != len)
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kNoteSaveFail, 1500);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kNoteSaveFail, 1500);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 false 的结果保存到左侧 dirty_。
    dirty_ = false;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 NotepadApp::append_text：参数在括号里，返回类型写在函数名前面。
void NotepadApp::append_text(const char* text)
{
    // 判断条件：text == nullptr || text[0] == '\0' || !ensure_store() 是否成立。
    if (text == nullptr || text[0] == '\0' || !ensure_store())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 定义变量 size_t cur 并初始化为 std::strlen(body_)。
    size_t cur = std::strlen(body_);
    // 判断条件：cur > 0 && body_[cur - 1] != '\n' 是否成立。
    if (cur > 0 && body_[cur - 1] != '\n')
    {
        // 判断条件：cur + 1 >= kBodyCap 是否成立。
        if (cur + 1 >= kBodyCap)
            // 提前返回：结束当前函数，不返回具体值。
            return;
        // 赋值：把右侧 '\n' 的结果保存到左侧 body_[cur++]。
        body_[cur++] = '\n';
        // 赋值：把右侧 '\0' 的结果保存到左侧 body_[cur]。
        body_[cur] = '\0';
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t add = std::strlen(text);
    // 判断条件：cur + add >= kBodyCap 是否成立。
    if (cur + add >= kBodyCap)
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kNoteTooLong, 1500);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kNoteTooLong, 1500);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 函数调用：(void)std::strncpy(body_ + cur, text, kBodyCap - cur - 1);；执行该函数并处理它的返回值或副作用。
    (void)std::strncpy(body_ + cur, text, kBodyCap - cur - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 body_[kBodyCap - 1]。
    body_[kBodyCap - 1] = '\0';
    // 赋值：把右侧 true 的结果保存到左侧 dirty_。
    dirty_ = true;
}

// 定义或声明函数 NotepadApp::delete_last_char：参数在括号里，返回类型写在函数名前面。
void NotepadApp::delete_last_char()
{
    // 判断条件：body_ == nullptr || body_[0] == '\0' 是否成立。
    if (body_ == nullptr || body_[0] == '\0')
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kNoteNothingDel, 1200);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kNoteNothingDel, 1200);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t len = std::strlen(body_);
    // 判断条件：len == 0 是否成立。
    if (len == 0)
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kNoteNothingDel, 1200);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kNoteNothingDel, 1200);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 定义变量 size_t pos 并初始化为 len - 1。
    size_t pos = len - 1;
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (pos > 0 && (static_cast<uint8_t>(body_[pos]) & 0xC0U) == 0x80U)
        // 执行语句：--pos;；它会更新一次变量、对象或程序状态。
        --pos;

    // 赋值：把右侧 '\0' 的结果保存到左侧 body_[pos]。
    body_[pos] = '\0';
    // 赋值：把右侧 true 的结果保存到左侧 dirty_。
    dirty_ = true;
    // 调用 clamp_scroll();，执行这个函数并处理它的返回值或状态变化。
    clamp_scroll();
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 定义或声明函数 NotepadApp::clear_body：参数在括号里，返回类型写在函数名前面。
void NotepadApp::clear_body()
{
    // 判断条件：body_ != nullptr 是否成立。
    if (body_ != nullptr)
        // 赋值：把右侧 '\0' 的结果保存到左侧 body_[0]。
        body_[0] = '\0';
    // 赋值：把右侧 true 的结果保存到左侧 dirty_。
    dirty_ = true;
    // 赋值：把右侧 0 的结果保存到左侧 scroll_line_。
    scroll_line_ = 0;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 定义或声明函数 NotepadApp::open_note：参数在括号里，返回类型写在函数名前面。
void NotepadApp::open_note(uint8_t idx)
{
    // 判断条件：notes_ == nullptr || idx >= count_ 是否成立。
    if (notes_ == nullptr || idx >= count_)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 idx 的结果保存到左侧 sel_。
    sel_ = idx;
    // 赋值：把右侧 View::Edit 的结果保存到左侧 view_。
    view_ = View::Edit;
    // 赋值：把右侧 0 的结果保存到左侧 scroll_line_。
    scroll_line_ = 0;
    // 赋值：把右侧 false 的结果保存到左侧 dirty_。
    dirty_ = false;
    // 判断条件：body_ != nullptr 是否成立。
    if (body_ != nullptr)
        // 赋值：把右侧 '\0' 的结果保存到左侧 body_[0]。
        body_[0] = '\0';
    // 访问对象成员：(void)std::strncpy(path_, notes_[idx].path, sizeof(path_) - 1);；读取或修改这个对象里的成员状态。
    (void)std::strncpy(path_, notes_[idx].path, sizeof(path_) - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 path_[sizeof(path_) - 1]。
    path_[sizeof(path_) - 1] = '\0';
    // 调用 title_from_path(path_, title_buf_, sizeof(title_buf_));，执行这个函数并处理它的返回值或状态变化。
    title_from_path(path_, title_buf_, sizeof(title_buf_));
    // 函数调用：(void)load_note(path_);；执行该函数并处理它的返回值或副作用。
    (void)load_note(path_);
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 定义或声明函数 NotepadApp::start_new_note：参数在括号里，返回类型写在函数名前面。
void NotepadApp::start_new_note()
{
    // 赋值：把右侧 KbMode::NewTitle 的结果保存到左侧 kb_mode_。
    kb_mode_ = KbMode::NewTitle;
    // 声明并默认初始化对象：overlays::KeyboardConfig kc{}。
    overlays::KeyboardConfig kc{};
    // 赋值：把右侧 31 的结果保存到左侧 kc.max_len。
    kc.max_len = 31;
    // 赋值：把右侧 ui::strings::kNoteTitleHint 的结果保存到左侧 kc.title。
    kc.title = ui::strings::kNoteTitleHint;
    // 调用 overlays::Keyboard::instance().open(kc, on_title_kb_done, this);，执行这个函数并处理它的返回值或状态变化。
    overlays::Keyboard::instance().open(kc, on_title_kb_done, this);
}

// 定义或声明函数 NotepadApp::on_title_kb_done：参数在括号里，返回类型写在函数名前面。
void NotepadApp::on_title_kb_done(const char* text, void* user)
{
    // 赋值：把右侧 static_cast<NotepadApp*>(user) 的结果保存到左侧 auto* self。
    auto* self = static_cast<NotepadApp*>(user);
    // 判断条件：self == nullptr 是否成立。
    if (self == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 KbMode::None 的结果保存到左侧 self->kb_mode_。
    self->kb_mode_ = KbMode::None;

    // 判断条件：!self->ensure_notes_dirs() 是否成立。
    if (!self->ensure_notes_dirs())
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kNoteSaveFail, 1500);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kNoteSaveFail, 1500);
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        self->request_repaint();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 执行语句：char stem[36]{};；它会更新一次变量、对象或程序状态。
    char stem[36]{};
    // 调用 NotepadApp::sanitize_filename(text, stem, sizeof(stem));，执行这个函数并处理它的返回值或状态变化。
    NotepadApp::sanitize_filename(text, stem, sizeof(stem));

    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* notes_dir = preferred_notes_dir();
    // 执行语句：char path[128]{};；它会更新一次变量、对象或程序状态。
    char path[128]{};
    // 定义变量 bool path_ok 并初始化为 false。
    bool path_ok = false;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t try_no = 0; try_no < 5; ++try_no)
    {
        // 判断条件：try_no == 0 是否成立。
        if (try_no == 0)
            // 访问对象成员：(void)std::snprintf(path, sizeof(path), "%s/%s.txt", notes_dir, stem);；读取或修改这个对象里的成员状态。
            (void)std::snprintf(path, sizeof(path), "%s/%s.txt", notes_dir, stem);
        // 备用分支：前面的条件不成立时执行这里。
        else
            // 函数调用：(void)std::snprintf(path, sizeof(path), "%s/%s_%u.txt", notes_dir, stem,；执行该函数并处理它的返回值或副作用。
            (void)std::snprintf(path, sizeof(path), "%s/%s_%u.txt", notes_dir, stem,
                                // 继续上一行的表达式或参数列表：static_cast<unsigned>(try_no));。
                                static_cast<unsigned>(try_no));

        // struct stat：把相关数据字段组合在一起，表示项目中的一个概念。
        struct stat st{};
        // 判断条件：::stat(path, &st) != 0 是否成立。
        if (::stat(path, &st) != 0)
        {
            // 赋值：把右侧 true 的结果保存到左侧 path_ok。
            path_ok = true;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        }
    }

    // 判断条件：!path_ok && !self->alloc_new_path(path, sizeof(path)) 是否成立。
    if (!path_ok && !self->alloc_new_path(path, sizeof(path)))
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kNoteSaveFail, 1500);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kNoteSaveFail, 1500);
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        self->request_repaint();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 赋值：把右侧 std::fopen(path, "wb") 的结果保存到左侧 FILE* f。
    FILE* f = std::fopen(path, "wb");
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
    {
        // 首选盘写失败时，换另一盘再试一次
        const char* alt = (std::strcmp(notes_dir, kNotesDirInt) == 0) ? kNotesDirSd : kNotesDirInt;
        // 赋值：把右侧 StorageMgr::get_instance() 的结果保存到左侧 auto& sto。
        auto& sto = StorageMgr::get_instance();
        // 只读约束：这个值或对象在当前位置不能被修改。
        const bool alt_ok = (alt == kNotesDirInt && sto.is_mounted(MountKind::Internal)) ||
                            // 继续上一行的表达式或参数列表：(alt == kNotesDirSd && sto.is_mounted(MountKind::Sd));。
                            (alt == kNotesDirSd && sto.is_mounted(MountKind::Sd));
        // 判断条件：alt_ok 是否成立。
        if (alt_ok)
        {
            // 访问对象成员：(void)std::snprintf(path, sizeof(path), "%s/%s.txt", alt, stem);；读取或修改这个对象里的成员状态。
            (void)std::snprintf(path, sizeof(path), "%s/%s.txt", alt, stem);
            // struct stat：把相关数据字段组合在一起，表示项目中的一个概念。
            struct stat st{};
            // 判断条件：::stat(path, &st) == 0 是否成立。
            if (::stat(path, &st) == 0)
                // 访问对象成员：(void)std::snprintf(path, sizeof(path), "%s/%s_1.txt", alt, stem);；读取或修改这个对象里的成员状态。
                (void)std::snprintf(path, sizeof(path), "%s/%s_1.txt", alt, stem);
            // 赋值：把右侧 std::fopen(path, "wb") 的结果保存到左侧 f。
            f = std::fopen(path, "wb");
        }
    }
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kNoteSaveFail, 1500);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kNoteSaveFail, 1500);
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        self->request_repaint();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 函数调用：(void)std::fclose(f);；执行该函数并处理它的返回值或副作用。
    (void)std::fclose(f);

    // 访问对象成员：(void)std::strncpy(self->path_, path, sizeof(self->path_) - 1);；读取或修改这个对象里的成员状态。
    (void)std::strncpy(self->path_, path, sizeof(self->path_) - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 self->path_[sizeof(self->path_) - 1]。
    self->path_[sizeof(self->path_) - 1] = '\0';
    // 调用 NotepadApp::title_from_path(self->path_, self->title_buf_, sizeof(self->title_buf_));，执行这个函数并处理它的返回值或状态变化。
    NotepadApp::title_from_path(self->path_, self->title_buf_, sizeof(self->title_buf_));
    // 判断条件：self->body_ != nullptr 是否成立。
    if (self->body_ != nullptr)
        // 赋值：把右侧 '\0' 的结果保存到左侧 self->body_[0]。
        self->body_[0] = '\0';
    // 赋值：把右侧 false 的结果保存到左侧 self->dirty_。
    self->dirty_ = false;
    // 赋值：把右侧 0 的结果保存到左侧 self->scroll_line_。
    self->scroll_line_ = 0;
    // 赋值：把右侧 View::Edit 的结果保存到左侧 self->view_。
    self->view_ = View::Edit;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    self->request_repaint();
    // 访问对象成员：self->open_append_keyboard();；读取或修改这个对象里的成员状态。
    self->open_append_keyboard();
}

// 定义或声明函数 NotepadApp::on_append_kb_done：参数在括号里，返回类型写在函数名前面。
void NotepadApp::on_append_kb_done(const char* text, void* user)
{
    // 赋值：把右侧 static_cast<NotepadApp*>(user) 的结果保存到左侧 auto* self。
    auto* self = static_cast<NotepadApp*>(user);
    // 判断条件：self == nullptr 是否成立。
    if (self == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 KbMode::None 的结果保存到左侧 self->kb_mode_。
    self->kb_mode_ = KbMode::None;
    // 判断条件：text != nullptr && text[0] != '\0' 是否成立。
    if (text != nullptr && text[0] != '\0')
        // 访问对象成员：self->append_text(text);；读取或修改这个对象里的成员状态。
        self->append_text(text);
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    self->request_repaint();
}

// 定义或声明函数 NotepadApp::open_append_keyboard：参数在括号里，返回类型写在函数名前面。
void NotepadApp::open_append_keyboard()
{
    // 赋值：把右侧 KbMode::Append 的结果保存到左侧 kb_mode_。
    kb_mode_ = KbMode::Append;
    // 声明并默认初始化对象：overlays::KeyboardConfig kc{}。
    overlays::KeyboardConfig kc{};
    // 赋值：把右侧 63 的结果保存到左侧 kc.max_len。
    kc.max_len = 63;
    // 赋值：把右侧 ui::strings::kNoteInput 的结果保存到左侧 kc.title。
    kc.title = ui::strings::kNoteInput;
    // 调用 overlays::Keyboard::instance().open(kc, on_append_kb_done, this);，执行这个函数并处理它的返回值或状态变化。
    overlays::Keyboard::instance().open(kc, on_append_kb_done, this);
}

// 定义或声明函数 NotepadApp::open_rename_keyboard：参数在括号里，返回类型写在函数名前面。
void NotepadApp::open_rename_keyboard()
{
    // 赋值：把右侧 KbMode::RenameTitle 的结果保存到左侧 kb_mode_。
    kb_mode_ = KbMode::RenameTitle;
    // 声明并默认初始化对象：overlays::KeyboardConfig kc{}。
    overlays::KeyboardConfig kc{};
    // 赋值：把右侧 31 的结果保存到左侧 kc.max_len。
    kc.max_len = 31;
    // 赋值：把右侧 ui::strings::kNoteRename 的结果保存到左侧 kc.title。
    kc.title = ui::strings::kNoteRename;
    // 赋值：把右侧 title_buf_ 的结果保存到左侧 kc.initial_text。
    kc.initial_text = title_buf_;
    // 调用 overlays::Keyboard::instance().open(kc, on_rename_kb_done, this);，执行这个函数并处理它的返回值或状态变化。
    overlays::Keyboard::instance().open(kc, on_rename_kb_done, this);
}

// 定义或声明函数 NotepadApp::on_rename_kb_done：参数在括号里，返回类型写在函数名前面。
void NotepadApp::on_rename_kb_done(const char* text, void* user)
{
    // 赋值：把右侧 static_cast<NotepadApp*>(user) 的结果保存到左侧 auto* self。
    auto* self = static_cast<NotepadApp*>(user);
    // 判断条件：self == nullptr 是否成立。
    if (self == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 KbMode::None 的结果保存到左侧 self->kb_mode_。
    self->kb_mode_ = KbMode::None;
    // 判断条件：text != nullptr && text[0] != '\0' 是否成立。
    if (text != nullptr && text[0] != '\0')
    {
        // 判断条件：self->rename_note_to_stem(text) 是否成立。
        if (self->rename_note_to_stem(text))
            // 调用 overlays::Toast::instance().show(ui::strings::kNoteRenamed, 1500);，执行这个函数并处理它的返回值或状态变化。
            overlays::Toast::instance().show(ui::strings::kNoteRenamed, 1500);
    }
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    self->request_repaint();
}

// 定义或声明函数 NotepadApp::fill_row：参数在括号里，返回类型写在函数名前面。
void NotepadApp::fill_row(uint8_t idx, ui::widgets::RowStyle& rs)
{
    // 判断条件：idx == 0 是否成立。
    if (idx == 0)
    {
        // 赋值：把右侧 ui::strings::kNoteNew 的结果保存到左侧 rs.label。
        rs.label = ui::strings::kNoteNew;
        // 赋值：把右侧 true 的结果保存到左侧 rs.show_chevron。
        rs.show_chevron = true;
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t note_idx = static_cast<uint8_t>(idx - 1);
    // 判断条件：notes_ == nullptr || size_buf_ == nullptr || note_idx >= count_ 是否成立。
    if (notes_ == nullptr || size_buf_ == nullptr || note_idx >= count_)
    {
        // 赋值：把右侧 "" 的结果保存到左侧 rs.label。
        rs.label = "";
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const NoteItem& item = notes_[note_idx];
    // 赋值：把右侧 item.title 的结果保存到左侧 rs.label。
    rs.label = item.title;
    // 赋值：把右侧 true 的结果保存到左侧 rs.show_chevron。
    rs.show_chevron = true;
    // 调用 gfx::format_file_size(item.size, size_buf_[note_idx], sizeof(size_buf_[note_idx]));，执行这个函数并处理它的返回值或状态变化。
    gfx::format_file_size(item.size, size_buf_[note_idx], sizeof(size_buf_[note_idx]));
    // 赋值：把右侧 size_buf_[note_idx] 的结果保存到左侧 rs.value。
    rs.value = size_buf_[note_idx];
}

// 函数定义/声明：调用 NotepadApp::on_row_tap()：处理列表点击： 如果点击父目录行，调用 goup()。 如果点击根卷，调用 openroot() 并调度扫描。 如果点击目录，调用 enterdir()。 如果点击文件，调用 showfileaction()。。
void NotepadApp::on_row_tap(uint8_t idx)
{
    // 判断条件：idx == 0 是否成立。
    if (idx == 0)
    {
        // 调用 start_new_note();，执行这个函数并处理它的返回值或状态变化。
        start_new_note();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 调用 open_note(static_cast<uint8_t>(idx - 1));，执行这个函数并处理它的返回值或状态变化。
    open_note(static_cast<uint8_t>(idx - 1));
}

// 定义或声明函数 NotepadApp::edit_body_rect：参数在括号里，返回类型写在函数名前面。
core::Rect NotepadApp::edit_body_rect()
{
    // 返回 core::Rect{0, static_cast<int16_t>(Theme::kListStartY), Theme::kScreenW, 给调用者，表示当前函数的执行结果。
    return core::Rect{0, static_cast<int16_t>(Theme::kListStartY), Theme::kScreenW,
                      // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kScreenH - Theme::kListStartY - kActionBarH)};。
                      static_cast<uint16_t>(Theme::kScreenH - Theme::kListStartY - kActionBarH)};
}

// 定义或声明函数 NotepadApp::action_bar_rect：参数在括号里，返回类型写在函数名前面。
core::Rect NotepadApp::action_bar_rect()
{
    // 返回 core::Rect{0, static_cast<int16_t>(Theme::kScreenH - kActionBarH), Theme::kScreenW, 给调用者，表示当前函数的执行结果。
    return core::Rect{0, static_cast<int16_t>(Theme::kScreenH - kActionBarH), Theme::kScreenW,
                      // 继续上一行的表达式或参数列表：kActionBarH};。
                      kActionBarH};
}

// 定义或声明函数 NotepadApp::action_btn_rect：参数在括号里，返回类型写在函数名前面。
core::Rect NotepadApp::action_btn_rect(uint8_t idx)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect bar = action_bar_rect();
    // 判断条件：idx >= kActionBtnNum 是否成立。
    if (idx >= kActionBtnNum)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t cw = static_cast<uint16_t>(bar.w / kActionBtnNum);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x = static_cast<int16_t>(bar.x + (idx * cw));
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t w = (idx + 1 == kActionBtnNum) ? static_cast<uint16_t>(bar.right() - x) : cw;
    // 返回 core::Rect{x, bar.y, w, bar.h} 给调用者，表示当前函数的执行结果。
    return core::Rect{x, bar.y, w, bar.h};
}

// 定义或声明函数 NotepadApp::confirm_dialog_rect：参数在括号里，返回类型写在函数名前面。
core::Rect NotepadApp::confirm_dialog_rect()
{
    // 返回 ui::dialog::centered(kConfirmDlgW, kConfirmDlgH) 给调用者，表示当前函数的执行结果。
    return ui::dialog::centered(kConfirmDlgW, kConfirmDlgH);
}

// 定义或声明函数 NotepadApp::more_dialog_rect：参数在括号里，返回类型写在函数名前面。
core::Rect NotepadApp::more_dialog_rect()
{
    // 返回 ui::dialog::centered(kMoreDlgW, kMoreDlgH) 给调用者，表示当前函数的执行结果。
    return ui::dialog::centered(kMoreDlgW, kMoreDlgH);
}

// 定义或声明函数 NotepadApp::more_dialog_btn_rect：参数在括号里，返回类型写在函数名前面。
core::Rect NotepadApp::more_dialog_btn_rect(uint8_t idx)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect dlg = more_dialog_rect();
    // 判断条件：idx >= kMoreBtnNum 是否成立。
    if (idx >= kMoreBtnNum)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};
    // 返回 core::Rect{dlg.x, static_cast<int16_t>(dlg.y + kMoreTitleH + (idx * kMoreBtnH)), dlg.w, 给调用者，表示当前函数的执行结果。
    return core::Rect{dlg.x, static_cast<int16_t>(dlg.y + kMoreTitleH + (idx * kMoreBtnH)), dlg.w,
                      // 继续上一行的表达式或参数列表：kMoreBtnH};。
                      kMoreBtnH};
}

// 定义或声明函数 NotepadApp::show_more_menu：参数在括号里，返回类型写在函数名前面。
void NotepadApp::show_more_menu()
{
    // 赋值：把右侧 Dlg::More 的结果保存到左侧 dlg_。
    dlg_ = Dlg::More;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 定义或声明函数 NotepadApp::show_clear_dialog：参数在括号里，返回类型写在函数名前面。
void NotepadApp::show_clear_dialog()
{
    // 赋值：把右侧 Dlg::Clear 的结果保存到左侧 dlg_。
    dlg_ = Dlg::Clear;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 定义或声明函数 NotepadApp::show_delete_dialog：参数在括号里，返回类型写在函数名前面。
void NotepadApp::show_delete_dialog()
{
    // 赋值：把右侧 Dlg::Delete 的结果保存到左侧 dlg_。
    dlg_ = Dlg::Delete;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 函数定义/声明：调用 NotepadApp::close_dialog()：把 dlg 设回 None，清除选中信息和信息行。它不负责重扫目录，因为关闭弹窗通常只是恢复界面。。
void NotepadApp::close_dialog()
{
    // 判断条件：dlg_ == Dlg::None 是否成立。
    if (dlg_ == Dlg::None)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 Dlg::None 的结果保存到左侧 dlg_。
    dlg_ = Dlg::None;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 定义或声明函数 NotepadApp::dismiss_dialog_back：参数在括号里，返回类型写在函数名前面。
void NotepadApp::dismiss_dialog_back()
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (dlg_)
    {
        // 分支匹配：case Dlg::Delete 成立时执行下面的代码。
        case Dlg::Delete:
            // 赋值：把右侧 Dlg::More 的结果保存到左侧 dlg_。
            dlg_ = Dlg::More;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case Dlg::More 成立时执行下面的代码。
        case Dlg::More:
        // 分支匹配：case Dlg::Clear 成立时执行下面的代码。
        case Dlg::Clear:
            // 赋值：把右侧 Dlg::None 的结果保存到左侧 dlg_。
            dlg_ = Dlg::None;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 定义或声明函数 NotepadApp::paint_clear_dialog：参数在括号里，返回类型写在函数名前面。
void NotepadApp::paint_clear_dialog(gfx::Canvas& c)
{
    // 调用 ui::dialog::paint_confirm(c, confirm_dialog_rect(), ui::strings::kNoteClearAsk, true);，执行这个函数并处理它的返回值或状态变化。
    ui::dialog::paint_confirm(c, confirm_dialog_rect(), ui::strings::kNoteClearAsk, true);
}

// 定义或声明函数 NotepadApp::paint_delete_dialog：参数在括号里，返回类型写在函数名前面。
void NotepadApp::paint_delete_dialog(gfx::Canvas& c)
{
    // 调用 ui::dialog::paint_confirm(c, confirm_dialog_rect(), ui::strings::kNoteDeleteAsk, false);，执行这个函数并处理它的返回值或状态变化。
    ui::dialog::paint_confirm(c, confirm_dialog_rect(), ui::strings::kNoteDeleteAsk, false);
}

// 定义或声明函数 NotepadApp::paint_more_dialog：参数在括号里，返回类型写在函数名前面。
void NotepadApp::paint_more_dialog(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect dlg = more_dialog_rect();
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
        // 继续上一行的表达式或参数列表：static_cast<uint16_t>(dlg.w - ui::dialog::kCloseBtnSize - (ui::dialog::kCloseBtnPad * 2));。
        static_cast<uint16_t>(dlg.w - ui::dialog::kCloseBtnSize - (ui::dialog::kCloseBtnPad * 2));
    // 访问对象成员：c.text_in(core::Rect{dlg.x, dlg.y, title_w, kMoreTitleH}, ui::strings::kNoteMore, ts);；读取或修改这个对象里的成员状态。
    c.text_in(core::Rect{dlg.x, dlg.y, title_w, kMoreTitleH}, ui::strings::kNoteMore, ts);
    // 调用 ui::dialog::paint_close_btn(c, dlg);，执行这个函数并处理它的返回值或状态变化。
    ui::dialog::paint_close_btn(c, dlg);

    // 静态查表数组 labels：运行时复用固定数据，避免每次重新计算。
    static const char* const labels[kMoreBtnNum] = {
        // 执行这条代码：ui::strings::kNoteRename,。它会更新变量、对象状态或调用链。
        ui::strings::kNoteRename,
        // 继续上一行的表达式或参数列表：ui::strings::kNoteClear,。
        ui::strings::kNoteClear,
        // 继续上一行的表达式或参数列表：ui::strings::kNoteDeleteNote,。
        ui::strings::kNoteDeleteNote,
    };
    // 赋值：把右侧 Theme::kFontSmall 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontSmall;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < kMoreBtnNum; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect btn = more_dialog_btn_rect(i);
        // 调用 rect(btn,1)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
        c.rect(btn, 1);
        // 判断条件：i > 0 是否成立。
        if (i > 0)
            // 访问对象成员：c.hline(btn.x, btn.y, btn.w);；读取或修改这个对象里的成员状态。
            c.hline(btn.x, btn.y, btn.w);
        // 调用 text_in(btn,labels[i],ts)：文本对齐。
        c.text_in(btn, labels[i], ts);
    }
}

// 定义或声明函数 NotepadApp::paint_action_bar：参数在括号里，返回类型写在函数名前面。
void NotepadApp::paint_action_bar(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect bar = action_bar_rect();
    // 访问对象成员：c.hline(bar.x, bar.y, bar.w);；读取或修改这个对象里的成员状态。
    c.hline(bar.x, bar.y, bar.w);

    // 静态查表数组 labels：运行时复用固定数据，避免每次重新计算。
    static const char* const labels[kActionBtnNum] = {
        // 执行这条代码：ui::strings::kNoteInput,。它会更新变量、对象状态或调用链。
        ui::strings::kNoteInput,
        // 继续上一行的表达式或参数列表：ui::strings::kNoteDelLine,。
        ui::strings::kNoteDelLine,
        // 继续上一行的表达式或参数列表：ui::strings::kNoteMore,。
        ui::strings::kNoteMore,
        // 继续上一行的表达式或参数列表：ui::strings::kNoteSave,。
        ui::strings::kNoteSave,
    };

    // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
    gfx::Canvas::TextStyle ts{};
    // 赋值：把右侧 Theme::kFontSmall 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontSmall;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
    ts.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
    ts.v = gfx::VAlign::Middle;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < kActionBtnNum; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect btn = action_btn_rect(i);
        // 判断条件：i > 0 是否成立。
        if (i > 0)
            // 访问对象成员：c.vline(btn.x, btn.y, btn.h);；读取或修改这个对象里的成员状态。
            c.vline(btn.x, btn.y, btn.h);
        // 调用 text_in(btn,labels[i],ts)：文本对齐。
        c.text_in(btn, labels[i], ts);
    }
}

// 定义或声明函数 NotepadApp::paint_edit：参数在括号里，返回类型写在函数名前面。
void NotepadApp::paint_edit(gfx::Canvas& c)
{
    // 调用 paint_action_bar(c);，执行这个函数并处理它的返回值或状态变化。
    paint_action_bar(c);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect body = edit_body_rect();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* text = (body_ != nullptr) ? body_ : "";
    // 执行语句：gfx::LineSlice lines[kWrapLines]{};；它会更新一次变量、对象或程序状态。
    gfx::LineSlice lines[kWrapLines]{};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t line_count =
        // 继续上一行的表达式或参数列表：gfx::wrap_text(text, Theme::kFontBody, body.w, kWrapLines, lines, gfx::FontFace::Text);。
        gfx::wrap_text(text, Theme::kFontBody, body.w, kWrapLines, lines, gfx::FontFace::Text);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t line_h = static_cast<uint8_t>(Theme::kFontBody + 4);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t max_visible = static_cast<uint8_t>((body.h + line_h - 1) / line_h);

    // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
    gfx::Canvas::TextStyle ts{};
    // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontBody;
    // 赋值：把右侧 gfx::HAlign::Left 的结果保存到左侧 ts.h。
    ts.h = gfx::HAlign::Left;
    // 赋值：把右侧 gfx::VAlign::Top 的结果保存到左侧 ts.v。
    ts.v = gfx::VAlign::Top;

    // 判断条件：line_count == 0 || text[0] == '\0' 是否成立。
    if (line_count == 0 || text[0] == '\0')
    {
        // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
        ts.h = gfx::HAlign::Center;
        // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
        ts.v = gfx::VAlign::Middle;
        // 访问对象成员：c.text_in(body, ui::strings::kNoteEditHint, ts);；读取或修改这个对象里的成员状态。
        c.text_in(body, ui::strings::kNoteEditHint, ts);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < max_visible; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t li = static_cast<uint8_t>(scroll_line_ + i);
        // 判断条件：li >= line_count 是否成立。
        if (li >= line_count)
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect row{body.x, static_cast<int16_t>(body.y + (i * line_h)), body.w, line_h};
        // 执行语句：char line_buf[128]{};；它会更新一次变量、对象或程序状态。
        char line_buf[128]{};
        // 只读约束：这个值或对象在当前位置不能被修改。
        const size_t len = static_cast<size_t>(lines[li].end - lines[li].begin);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const size_t cap = (sizeof(line_buf) < len + 1) ? sizeof(line_buf) - 1 : len;
        // 判断条件：lines[li].begin != nullptr && cap > 0 是否成立。
        if (lines[li].begin != nullptr && cap > 0)
        {
            // 访问对象成员：(void)std::strncpy(line_buf, lines[li].begin, cap);；读取或修改这个对象里的成员状态。
            (void)std::strncpy(line_buf, lines[li].begin, cap);
            // 赋值：把右侧 '\0' 的结果保存到左侧 line_buf[cap]。
            line_buf[cap] = '\0';
            // 调用 text_in(row,line_buf,ts)：文本对齐。
            c.text_in(row, line_buf, ts);
        }
    }
}

// 定义或声明函数 NotepadApp::paint：参数在括号里，返回类型写在函数名前面。
void NotepadApp::paint(gfx::Canvas& c)
{
    // 调用 title()：返回 App 标题。在文件浏览器内，标题有时会动态反映当前目录，因此可能存在一个 buffer，而不是只返回一个常量字符串。。
    ui::widgets::toolbar(c, title());
    // 判断条件：view_ == View::List 是否成立。
    if (view_ == View::List)
    {
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        list_.set_total(static_cast<uint8_t>(count_ + 1));
        // 访问对象成员：list_.paint(c);；读取或修改这个对象里的成员状态。
        list_.paint(c);
        // 判断条件：count_ == 0 是否成立。
        if (count_ == 0)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const core::Rect list = Theme::list_region();
            // 只读约束：这个值或对象在当前位置不能被修改。
            const core::Rect hint{
                // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
                list.x, static_cast<int16_t>(list.y + Theme::kListRowH), list.w,
                // 继续上一行的表达式或参数列表：static_cast<uint16_t>(list.h > Theme::kListRowH ? list.h - Theme::kListRowH : 0)};。
                static_cast<uint16_t>(list.h > Theme::kListRowH ? list.h - Theme::kListRowH : 0)};
            // 静态查表数组 hints：运行时复用固定数据，避免每次重新计算。
            static const char* const hints[] = {
                // 执行这条代码：ui::strings::kNoteEmptyHint,。它会更新变量、对象状态或调用链。
                ui::strings::kNoteEmptyHint,
                // 继续上一行的表达式或参数列表：NotepadApp::kScanPathInt,。
                NotepadApp::kScanPathInt,
                // 继续上一行的表达式或参数列表：NotepadApp::kScanPathSd,。
                NotepadApp::kScanPathSd,
            };
            // 调用 ui::widgets::empty_state(c, hint, ui::strings::kNoteEmpty, hints, 3);，执行这个函数并处理它的返回值或状态变化。
            ui::widgets::empty_state(c, hint, ui::strings::kNoteEmpty, hints, 3);
        }
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 调用 paint_edit(c);，执行这个函数并处理它的返回值或状态变化。
    paint_edit(c);
}

// 定义或声明函数 NotepadApp::paint_overlay：参数在括号里，返回类型写在函数名前面。
void NotepadApp::paint_overlay(gfx::Canvas& c)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (dlg_)
    {
        // 分支匹配：case Dlg::Clear 成立时执行下面的代码。
        case Dlg::Clear:
            // 调用 paint_clear_dialog(c);，执行这个函数并处理它的返回值或状态变化。
            paint_clear_dialog(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case Dlg::More 成立时执行下面的代码。
        case Dlg::More:
            // 调用 paint_more_dialog(c);，执行这个函数并处理它的返回值或状态变化。
            paint_more_dialog(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case Dlg::Delete 成立时执行下面的代码。
        case Dlg::Delete:
            // 调用 paint_delete_dialog(c);，执行这个函数并处理它的返回值或状态变化。
            paint_delete_dialog(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
}

// 定义或声明函数 NotepadApp::handle_dialog_input：参数在括号里，返回类型写在函数名前面。
shell::InputResult NotepadApp::handle_dialog_input(int16_t x, int16_t y)
{
    // 判断条件：dlg_ == Dlg::More 是否成立。
    if (dlg_ == Dlg::More)
    {
        // 判断条件：ui::dialog::close_hit_rect(more_dialog_rect()).contains(x, y) 是否成立。
        if (ui::dialog::close_hit_rect(more_dialog_rect()).contains(x, y))
        {
            // 调用 close_dialog()：把 dlg 设回 None，清除选中信息和信息行。它不负责重扫目录，因为关闭弹窗通常只是恢复界面。。
            close_dialog();
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        }
    }
    // 备用分支：前面的条件不成立时执行这里。
    else if (dlg_ == Dlg::Clear)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const ConfirmHit hit = ui::dialog::hit_confirm(confirm_dialog_rect(), x, y, true);
        // 判断条件：hit == ConfirmHit::Close || hit == ConfirmHit::No 是否成立。
        if (hit == ConfirmHit::Close || hit == ConfirmHit::No)
        {
            // 调用 close_dialog()：把 dlg 设回 None，清除选中信息和信息行。它不负责重扫目录，因为关闭弹窗通常只是恢复界面。。
            close_dialog();
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        }
        // 判断条件：hit == ConfirmHit::Yes 是否成立。
        if (hit == ConfirmHit::Yes)
        {
            // 调用 close_dialog()：把 dlg 设回 None，清除选中信息和信息行。它不负责重扫目录，因为关闭弹窗通常只是恢复界面。。
            close_dialog();
            // 调用 clear_body();，执行这个函数并处理它的返回值或状态变化。
            clear_body();
            // 调用 overlays::Toast::instance().show(ui::strings::kNoteCleared, 1200);，执行这个函数并处理它的返回值或状态变化。
            overlays::Toast::instance().show(ui::strings::kNoteCleared, 1200);
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        }
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 多分支判断：根据括号里的值选择不同 case。
    switch (dlg_)
    {
        // 分支匹配：case Dlg::More 成立时执行下面的代码。
        case Dlg::More:
            // 判断条件：more_dialog_btn_rect(0).contains(x, y) 是否成立。
            if (more_dialog_btn_rect(0).contains(x, y))
            {
                // 调用 close_dialog()：把 dlg 设回 None，清除选中信息和信息行。它不负责重扫目录，因为关闭弹窗通常只是恢复界面。。
                close_dialog();
                // 调用 open_rename_keyboard();，执行这个函数并处理它的返回值或状态变化。
                open_rename_keyboard();
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            }
            // 判断条件：more_dialog_btn_rect(1).contains(x, y) 是否成立。
            if (more_dialog_btn_rect(1).contains(x, y))
            {
                // 判断条件：body_ == nullptr || body_[0] == '\0' 是否成立。
                if (body_ == nullptr || body_[0] == '\0')
                {
                    // 调用 close_dialog()：把 dlg 设回 None，清除选中信息和信息行。它不负责重扫目录，因为关闭弹窗通常只是恢复界面。。
                    close_dialog();
                    // 调用 overlays::Toast::instance().show(ui::strings::kNoteNothingDel, 1200);，执行这个函数并处理它的返回值或状态变化。
                    overlays::Toast::instance().show(ui::strings::kNoteNothingDel, 1200);
                    // 返回 {true} 给调用者，表示当前函数的执行结果。
                    return {true};
                }
                // 调用 show_clear_dialog();，执行这个函数并处理它的返回值或状态变化。
                show_clear_dialog();
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            }
            // 判断条件：more_dialog_btn_rect(2).contains(x, y) 是否成立。
            if (more_dialog_btn_rect(2).contains(x, y))
            {
                // 调用 show_delete_dialog();，执行这个函数并处理它的返回值或状态变化。
                show_delete_dialog();
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            }
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};

        // 分支匹配：case Dlg::Delete:  成立时执行下面的代码。
        case Dlg::Delete: {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const ConfirmHit hit = ui::dialog::hit_confirm(confirm_dialog_rect(), x, y, false);
            // 判断条件：hit == ConfirmHit::No 是否成立。
            if (hit == ConfirmHit::No)
            {
                // 赋值：把右侧 Dlg::More 的结果保存到左侧 dlg_。
                dlg_ = Dlg::More;
                // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
                request_repaint();
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            }
            // 判断条件：hit == ConfirmHit::Yes 是否成立。
            if (hit == ConfirmHit::Yes)
            {
                // 函数调用：(void)delete_current_note();；执行该函数并处理它的返回值或副作用。
                (void)delete_current_note();
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

// 定义或声明函数 NotepadApp::handle_edit_input：参数在括号里，返回类型写在函数名前面。
shell::InputResult NotepadApp::handle_edit_input(int16_t x, int16_t y)
{
    // 判断条件：action_btn_rect(0).contains(x, y) 是否成立。
    if (action_btn_rect(0).contains(x, y))
    {
        // 调用 open_append_keyboard();，执行这个函数并处理它的返回值或状态变化。
        open_append_keyboard();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：action_btn_rect(1).contains(x, y) 是否成立。
    if (action_btn_rect(1).contains(x, y))
    {
        // 调用 delete_last_char();，执行这个函数并处理它的返回值或状态变化。
        delete_last_char();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：action_btn_rect(2).contains(x, y) 是否成立。
    if (action_btn_rect(2).contains(x, y))
    {
        // 调用 show_more_menu();，执行这个函数并处理它的返回值或状态变化。
        show_more_menu();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：action_btn_rect(3).contains(x, y) 是否成立。
    if (action_btn_rect(3).contains(x, y))
    {
        // 判断条件：save_note() 是否成立。
        if (save_note())
            // 调用 overlays::Toast::instance().show(ui::strings::kNoteSaved, 1500);，执行这个函数并处理它的返回值或状态变化。
            overlays::Toast::instance().show(ui::strings::kNoteSaved, 1500);
        // 失败时 save_note() 已 Toast，避免重复提示
        request_repaint();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect body = edit_body_rect();
    // 判断条件：!body.contains(x, y) 是否成立。
    if (!body.contains(x, y))
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t line_h = static_cast<uint8_t>(Theme::kFontBody + 4);
    // 判断条件：y < body.y + static_cast<int16_t>(line_h) 是否成立。
    if (y < body.y + static_cast<int16_t>(line_h))
    {
        // 判断条件：scroll_line_ > 0 是否成立。
        if (scroll_line_ > 0)
        {
            // 执行语句：--scroll_line_;；它会更新一次变量、对象或程序状态。
            --scroll_line_;
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            request_repaint();
        }
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：y > body.bottom() - static_cast<int16_t>(line_h) 是否成立。
    if (y > body.bottom() - static_cast<int16_t>(line_h))
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t max_scroll = edit_max_scroll();
        // 判断条件：scroll_line_ < max_scroll 是否成立。
        if (scroll_line_ < max_scroll)
        {
            // 执行语句：++scroll_line_;；它会更新一次变量、对象或程序状态。
            ++scroll_line_;
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            request_repaint();
        }
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 返回 {} 给调用者，表示当前函数的执行结果。
    return {};
}

// 定义或声明函数 NotepadApp::on_input：参数在括号里，返回类型写在函数名前面。
shell::InputResult NotepadApp::on_input(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;

    // 判断条件：overlays::Keyboard::instance().is_open() 是否成立。
    if (overlays::Keyboard::instance().is_open())
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x = static_cast<int16_t>(ev.x);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y = static_cast<int16_t>(ev.y);

    // 判断条件：view_ == View::Edit 是否成立。
    if (view_ == View::Edit)
    {
        // 判断条件：dlg_ != Dlg::None 是否成立。
        if (dlg_ != Dlg::None)
        {
            // 判断条件：ev.type == EventType::Tap 是否成立。
            if (ev.type == EventType::Tap)
                // 返回 handle_dialog_input(x, y) 给调用者，表示当前函数的执行结果。
                return handle_dialog_input(x, y);
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        }

        // 判断条件：ev.type == EventType::Tap 是否成立。
        if (ev.type == EventType::Tap)
        {
            // 判断条件：ui::widgets::hit_toolbar_back(x, y) 是否成立。
            if (ui::widgets::hit_toolbar_back(x, y))
            {
                // 调用 leave_edit_to_list();，执行这个函数并处理它的返回值或状态变化。
                leave_edit_to_list();
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            }
            // 返回 handle_edit_input(x, y) 给调用者，表示当前函数的执行结果。
            return handle_edit_input(x, y);
        }

        // 判断条件：ev.type == EventType::SwipeUp || ev.type == EventType::SwipeDown 是否成立。
        if (ev.type == EventType::SwipeUp || ev.type == EventType::SwipeDown)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint8_t max_scroll = edit_max_scroll();
            // 判断条件：ev.type == EventType::SwipeUp 是否成立。
            if (ev.type == EventType::SwipeUp)
            {
                // 判断条件：scroll_line_ < max_scroll 是否成立。
                if (scroll_line_ < max_scroll)
                    // 执行语句：++scroll_line_;；它会更新一次变量、对象或程序状态。
                    ++scroll_line_;
            }
            // 备用分支：前面的条件不成立时执行这里。
            else if (scroll_line_ > 0)
            {
                // 执行语句：--scroll_line_;；它会更新一次变量、对象或程序状态。
                --scroll_line_;
            }
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            request_repaint();
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        }
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};
    }

    // 判断条件：ev.type == EventType::Tap && ui::widgets::hit_toolbar_back(x, y) 是否成立。
    if (ev.type == EventType::Tap && ui::widgets::hit_toolbar_back(x, y))
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto out = list_.handle_input(ev);
    // 判断条件：out.scroll_changed 是否成立。
    if (out.scroll_changed)
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
    // 判断条件：out.consumed 是否成立。
    if (out.consumed)
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    // 返回 {} 给调用者，表示当前函数的执行结果。
    return {};
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::notepad
