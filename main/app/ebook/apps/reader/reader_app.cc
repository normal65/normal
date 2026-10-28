/**
 * [初学者导读] 文件：main/app/ebook/apps/reader/reader_app.cc
 * 作用：实现阅读器页面、分页、阅读进度和阅读菜单入口。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/reader_app.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdlib>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <sys/stat.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/reader_menu.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/clock_provider.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/persist.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/reading_store.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/txt_index_cache.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/system_state.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/icon.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/text_layout.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/keyboard.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/toast.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/refresh_intent.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/router.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "text/path_encoding.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::reader {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;

// 固定参数 kNvsFont = "reader.font"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kNvsFont = "reader.font";
// 固定参数 kNvsRefresh = "reader.refresh"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kNvsRefresh = "reader.refresh";

// 固定参数 kClearFullEvery = 20：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kClearFullEvery = 20;
// 固定参数 kNormalFastEvery = 10：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kNormalFastEvery = 10;
// 固定参数 kNormalFullEvery = 30：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kNormalFullEvery = 30;

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 赋值：把右侧 false 的结果保存到左侧 bool ReaderApp::s_resume_on_enter_。
bool ReaderApp::s_resume_on_enter_ = false;
// 赋值：把右侧 false 的结果保存到左侧 bool ReaderApp::s_open_on_enter_。
bool ReaderApp::s_open_on_enter_ = false;
// 执行语句：char ReaderApp::s_open_path_[96]{};；它会更新一次变量、对象或程序状态。
char ReaderApp::s_open_path_[96]{};

// 定义或声明函数 ReaderApp::instance：参数在括号里，返回类型写在函数名前面。
ReaderApp& ReaderApp::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static ReaderApp s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义或声明函数 ReaderApp::request_resume_on_enter：参数在括号里，返回类型写在函数名前面。
void ReaderApp::request_resume_on_enter()
{
    // 赋值：把右侧 true 的结果保存到左侧 s_resume_on_enter_。
    s_resume_on_enter_ = true;
}

// 定义或声明函数 ReaderApp::request_open_on_enter：参数在括号里，返回类型写在函数名前面。
void ReaderApp::request_open_on_enter(const char* path)
{
    // 判断条件：path == nullptr || path[0] == '\0' 是否成立。
    if (path == nullptr || path[0] == '\0')
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 函数调用：(void)std::strncpy(s_open_path_, path, sizeof(s_open_path_) - 1);；执行该函数并处理它的返回值或副作用。
    (void)std::strncpy(s_open_path_, path, sizeof(s_open_path_) - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 s_open_path_[sizeof(s_open_path_) - 1]。
    s_open_path_[sizeof(s_open_path_) - 1] = '\0';
    // 赋值：把右侧 true 的结果保存到左侧 s_open_on_enter_。
    s_open_on_enter_ = true;
}

// 定义或声明函数 ReaderApp::ReaderApp：参数在括号里，返回类型写在函数名前面。
ReaderApp::ReaderApp()
{
    // 访问对象成员：list_.set_area(Theme::list_region());；读取或修改这个对象里的成员状态。
    list_.set_area(Theme::list_region());
    // 访问对象成员：list_.set_provider([this](uint8_t i, ui::widgets::RowStyle& rs) { fill_shelf_row(i, rs); });；读取或修改这个对象里的成员状态。
    list_.set_provider([this](uint8_t i, ui::widgets::RowStyle& rs) { fill_shelf_row(i, rs); });
    // 访问对象成员：list_.set_tap_handler([this](uint8_t i) { on_shelf_tap(i); });；读取或修改这个对象里的成员状态。
    list_.set_tap_handler([this](uint8_t i) { on_shelf_tap(i); });
}

// 定义只读对象或变量：初始化后不能在当前位置修改。
const char* ReaderApp::title() const
{
    // 判断条件：screen_ == Screen::Reading && open_title_[0] != '\0' 是否成立。
    if (screen_ == Screen::Reading && open_title_[0] != '\0')
    {
        // 执行语句：char norm[48];；它会更新一次变量、对象或程序状态。
        char norm[48];
        // 函数调用：(void)text::normalize_path_segment(open_title_, norm, sizeof(norm));；执行该函数并处理它的返回值或副作用。
        (void)text::normalize_path_segment(open_title_, norm, sizeof(norm));
        // 函数调用：(void)gfx::truncate_text(norm, Theme::kFontBody,；执行该函数并处理它的返回值或副作用。
        (void)gfx::truncate_text(norm, Theme::kFontBody,
                                 // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kScreenW - Theme::kPadLg * 2),。
                                 static_cast<uint16_t>(Theme::kScreenW - Theme::kPadLg * 2),
                                 // 继续上一行的表达式或参数列表：title_buf_, sizeof(title_buf_));。
                                 title_buf_, sizeof(title_buf_));
        // 返回 title_buf_ 给调用者，表示当前函数的执行结果。
        return title_buf_;
    }
    // 返回 ui::strings::kAppReader 给调用者，表示当前函数的执行结果。
    return ui::strings::kAppReader;
}

// 函数定义/声明：调用 ReaderApp::icon_cp()：返回图标 codepoint。图标通常来自字体图标表，所以这里返回的是 uint32t，不是图片路径。。
uint32_t ReaderApp::icon_cp() const
{
    // 返回 gfx::icon::kFaBookReader 给调用者，表示当前函数的执行结果。
    return gfx::icon::kFaBookReader;
}

// 定义或声明函数 ReaderApp::wants_status_bar：参数在括号里，返回类型写在函数名前面。
bool ReaderApp::wants_status_bar() const
{
    // 返回 screen_ == Screen::Shelf 给调用者，表示当前函数的执行结果。
    return screen_ == Screen::Shelf;
}

// 定义或声明函数 ReaderApp::layout：参数在括号里，返回类型写在函数名前面。
ReaderLayout ReaderApp::layout() const
{
    // 返回 ReaderLayout(ReaderLayout::kFontSizes[font_scale_ % 3U]) 给调用者，表示当前函数的执行结果。
    return ReaderLayout(ReaderLayout::kFontSizes[font_scale_ % 3U]);
}

// 只读约束：这个值或对象在当前位置不能被修改。
const char* ReaderApp::font_label() const
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (font_scale_ % 3U)
    {
        // 分支匹配：case 0 成立时执行下面的代码。
        case 0:
            // 返回 "\u5C0F" 给调用者，表示当前函数的执行结果。
            return "\u5C0F";
        // 分支匹配：case 2 成立时执行下面的代码。
        case 2:
            // 返回 "\u5927" 给调用者，表示当前函数的执行结果。
            return "\u5927";
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 返回 "\u4E2D" 给调用者，表示当前函数的执行结果。
            return "\u4E2D";
    }
}

// 只读约束：这个值或对象在当前位置不能被修改。
const char* ReaderApp::refresh_label() const
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (static_cast<ReadRefreshMode>(refresh_mode_))
    {
        // 分支匹配：case ReadRefreshMode::Clear 成立时执行下面的代码。
        case ReadRefreshMode::Clear:
            // 返回 "\u6E05\u6670" 给调用者，表示当前函数的执行结果。
            return "\u6E05\u6670";
        // 分支匹配：case ReadRefreshMode::Fast 成立时执行下面的代码。
        case ReadRefreshMode::Fast:
            // 返回 "\u5FEB\u901F" 给调用者，表示当前函数的执行结果。
            return "\u5FEB\u901F";
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 返回 "\u666E\u901A" 给调用者，表示当前函数的执行结果。
            return "\u666E\u901A";
    }
}

// 定义或声明函数 ReaderApp::on_enter：参数在括号里，返回类型写在函数名前面。
void ReaderApp::on_enter()
{
    // 定义变量 uint8_t saved_scale 并初始化为 1。
    uint8_t saved_scale = 1;
    // 判断条件：data::Persist::get_u8(kNvsFont, saved_scale) && saved_scale < 3U 是否成立。
    if (data::Persist::get_u8(kNvsFont, saved_scale) && saved_scale < 3U)
        // 赋值：把右侧 saved_scale 的结果保存到左侧 font_scale_。
        font_scale_ = saved_scale;

    // 定义变量 uint8_t saved_refresh 并初始化为 static_cast<uint8_t>(ReadRefreshMode::Normal)。
    uint8_t saved_refresh = static_cast<uint8_t>(ReadRefreshMode::Normal);
    // 判断条件：data::Persist::get_u8(kNvsRefresh, saved_refresh) && saved_refresh < 3U 是否成立。
    if (data::Persist::get_u8(kNvsRefresh, saved_refresh) && saved_refresh < 3U)
        // 赋值：把右侧 saved_refresh 的结果保存到左侧 refresh_mode_。
        refresh_mode_ = saved_refresh;

    // 赋值：把右侧 Screen::Shelf 的结果保存到左侧 screen_。
    screen_ = Screen::Shelf;
    // 赋值：把右侧 false 的结果保存到左侧 menu_visible_。
    menu_visible_ = false;
    // 调用 close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
    book_.close();
    // 赋值：把右侧 '\0' 的结果保存到左侧 open_title_[0]。
    open_title_[0] = '\0';
    // 赋值：把右侧 '\0' 的结果保存到左侧 open_path_[0]。
    open_path_[0] = '\0';
    // 赋值：把右侧 0xFF 的结果保存到左侧 tick_hour_。
    tick_hour_ = 0xFF;
    // 赋值：把右侧 0xFF 的结果保存到左侧 tick_minute_。
    tick_minute_ = 0xFF;
    // 赋值：把右侧 0xFF 的结果保存到左侧 tick_battery_。
    tick_battery_ = 0xFF;
    // 赋值：把右侧 0 的结果保存到左侧 turn_count_。
    turn_count_ = 0;

    // 访问对象成员：shelf_.scan();；读取或修改这个对象里的成员状态。
    shelf_.scan();
    // 访问对象成员：list_.set_scroll(0);；读取或修改这个对象里的成员状态。
    list_.set_scroll(0);
    // 访问对象成员：list_.set_total(shelf_.count());；读取或修改这个对象里的成员状态。
    list_.set_total(shelf_.count());

    // 判断条件：s_resume_on_enter_ 是否成立。
    if (s_resume_on_enter_)
    {
        // 赋值：把右侧 false 的结果保存到左侧 s_resume_on_enter_。
        s_resume_on_enter_ = false;
        // 函数调用：(void)try_resume_saved_book();；执行该函数并处理它的返回值或副作用。
        (void)try_resume_saved_book();
    }
    // 备用分支：前面的条件不成立时执行这里。
    else if (s_open_on_enter_)
    {
        // 赋值：把右侧 false 的结果保存到左侧 s_open_on_enter_。
        s_open_on_enter_ = false;
        // 函数调用：(void)open_book_by_path(s_open_path_);；执行该函数并处理它的返回值或副作用。
        (void)open_book_by_path(s_open_path_);
    }
}

// 函数定义/声明：调用 ReaderApp::on_exit()：退出时取消未完成的扫描、关闭弹窗，并确保后台任务不会在对象状态失效后继续写数据。。
void ReaderApp::on_exit()
{
    // 判断条件：overlays::Keyboard::instance().is_open() 是否成立。
    if (overlays::Keyboard::instance().is_open())
        // 调用 close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
        overlays::Keyboard::instance().close();
    // 调用 save_progress();，执行这个函数并处理它的返回值或状态变化。
    save_progress();
    // 调用 close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
    book_.close();
    // 赋值：把右侧 Screen::Shelf 的结果保存到左侧 screen_。
    screen_ = Screen::Shelf;
    // 赋值：把右侧 false 的结果保存到左侧 menu_visible_。
    menu_visible_ = false;
}

// 定义或声明函数 ReaderApp::save_progress：参数在括号里，返回类型写在函数名前面。
void ReaderApp::save_progress()
{
    // 判断条件：screen_ != Screen::Reading || !book_.is_open() || open_path_[0] == '\0' 是否成立。
    if (screen_ != Screen::Reading || !book_.is_open() || open_path_[0] == '\0')
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 访问对象成员：(void)TxtIndexCache::save_progress(open_path_, book_.page(), book_.page_count());；读取或修改这个对象里的成员状态。
    (void)TxtIndexCache::save_progress(open_path_, book_.page(), book_.page_count());

    // 调用 ReadingStore::get_instance().set_book(open_title_, open_path_, book_.page(),，执行这个函数并处理它的返回值或状态变化。
    ReadingStore::get_instance().set_book(open_title_, open_path_, book_.page(),
                                          // 继续上一行的表达式或参数列表：book_.page_count());。
                                          book_.page_count());
}

// 定义或声明函数 ReaderApp::restore_progress：参数在括号里，返回类型写在函数名前面。
void ReaderApp::restore_progress()
{
    // 定义变量 uint16_t saved 并初始化为 0。
    uint16_t saved = 0;
    // 判断条件：open_path_[0] != '\0' && TxtIndexCache::load_progress(open_path_, &saved) 是否成立。
    if (open_path_[0] != '\0' && TxtIndexCache::load_progress(open_path_, &saved))
    {
        // 判断条件：saved < book_.page_count() 是否成立。
        if (saved < book_.page_count())
            // 访问对象成员：(void)book_.set_page(saved);；读取或修改这个对象里的成员状态。
            (void)book_.set_page(saved);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 赋值：把右侧 ReadingStore::get_instance() 的结果保存到左侧 auto& store。
    auto& store = ReadingStore::get_instance();
    // 判断条件：store.has_book() && std::strcmp(store.book_path(), open_path_ 是否成立。
    if (store.has_book() && std::strcmp(store.book_path(), open_path_) == 0 &&
        // 定义或声明函数 total_pages：参数在括号里，返回类型写在函数名前面。
        store.total_pages() == book_.page_count())
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint16_t p = store.current_page();
        // 判断条件：p < book_.page_count() 是否成立。
        if (p < book_.page_count())
            // 访问对象成员：(void)book_.set_page(p);；读取或修改这个对象里的成员状态。
            (void)book_.set_page(p);
    }
}

// 定义或声明函数 ReaderApp::return_to_shelf：参数在括号里，返回类型写在函数名前面。
void ReaderApp::return_to_shelf()
{
    // 调用 save_progress();，执行这个函数并处理它的返回值或状态变化。
    save_progress();
    // 调用 close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
    book_.close();
    // 赋值：把右侧 '\0' 的结果保存到左侧 open_title_[0]。
    open_title_[0] = '\0';
    // 赋值：把右侧 '\0' 的结果保存到左侧 open_path_[0]。
    open_path_[0] = '\0';
    // 赋值：把右侧 Screen::Shelf 的结果保存到左侧 screen_。
    screen_ = Screen::Shelf;
    // 赋值：把右侧 false 的结果保存到左侧 menu_visible_。
    menu_visible_ = false;
    // 访问对象成员：shelf_.scan();；读取或修改这个对象里的成员状态。
    shelf_.scan();
    // 访问对象成员：list_.set_scroll(0);；读取或修改这个对象里的成员状态。
    list_.set_scroll(0);
    // 访问对象成员：list_.set_total(shelf_.count());；读取或修改这个对象里的成员状态。
    list_.set_total(shelf_.count());
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 定义或声明函数 ReaderApp::toggle_reading_menu：参数在括号里，返回类型写在函数名前面。
void ReaderApp::toggle_reading_menu()
{
    // 判断条件：screen_ != Screen::Reading 是否成立。
    if (screen_ != Screen::Reading)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 !menu_visible_ 的结果保存到左侧 menu_visible_。
    menu_visible_ = !menu_visible_;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 函数定义/声明：调用 ReaderApp::on_semantic_back()：处理语义返回，顺序通常是： 1. 如果弹窗打开，先关闭弹窗。 2. 如果列表还在浏览目录，先返回上一级。 3. 如果已经到根卷，返回 false，让 Shell 退出 App。。
bool ReaderApp::on_semantic_back()
{
    // 判断条件：screen_ == Screen::Opening 是否成立。
    if (screen_ == Screen::Opening)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    // 判断条件：menu_visible_ 是否成立。
    if (menu_visible_)
    {
        // 赋值：把右侧 false 的结果保存到左侧 menu_visible_。
        menu_visible_ = false;
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 判断条件：screen_ == Screen::Reading 是否成立。
    if (screen_ == Screen::Reading)
    {
        // 调用 return_to_shelf();，执行这个函数并处理它的返回值或状态变化。
        return_to_shelf();
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 返回 false 给调用者，表示当前函数的执行结果。
    return false;
}

// 定义或声明函数 ReaderApp::go_lock_screen：参数在括号里，返回类型写在函数名前面。
void ReaderApp::go_lock_screen()
{
    // 调用 save_progress();，执行这个函数并处理它的返回值或状态变化。
    save_progress();
    // 访问对象成员：(void)router::Router::instance().replace_shell(router::ShellPage::Lock);；读取或修改这个对象里的成员状态。
    (void)router::Router::instance().replace_shell(router::ShellPage::Lock);
}

// 定义或声明函数 ReaderApp::after_page_turn：参数在括号里，返回类型写在函数名前面。
void ReaderApp::after_page_turn()
{
    // 调用 save_progress();，执行这个函数并处理它的返回值或状态变化。
    save_progress();
    // 执行语句：++turn_count_;；它会更新一次变量、对象或程序状态。
    ++turn_count_;

    // 多分支判断：根据括号里的值选择不同 case。
    switch (static_cast<ReadRefreshMode>(refresh_mode_))
    {
        // 分支匹配：case ReadRefreshMode::Clear 成立时执行下面的代码。
        case ReadRefreshMode::Clear:
            // 判断条件：turn_count_ % kClearFullEvery == 0 是否成立。
            if (turn_count_ % kClearFullEvery == 0)
                // 调用 router::intent_full_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
                request_repaint(router::intent_full_full());
            // 备用分支：前面的条件不成立时执行这里。
            else
                // 调用 router::intent_fast_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
                request_repaint(router::intent_fast_full());
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 分支匹配：case ReadRefreshMode::Normal 成立时执行下面的代码。
        case ReadRefreshMode::Normal:
            // 判断条件：turn_count_ % kNormalFullEvery == 0 是否成立。
            if (turn_count_ % kNormalFullEvery == 0)
                // 调用 router::intent_full_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
                request_repaint(router::intent_full_full());
            // 备用分支：前面的条件不成立时执行这里。
            else if (turn_count_ % kNormalFastEvery == 0)
                // 调用 router::intent_fast_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
                request_repaint(router::intent_fast_full());
            // 备用分支：前面的条件不成立时执行这里。
            else
                // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
                request_repaint();
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            request_repaint();
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
}

// 定义或声明函数 ReaderApp::open_book_by_path：参数在括号里，返回类型写在函数名前面。
bool ReaderApp::open_book_by_path(const char* path)
{
    // 判断条件：path == nullptr || path[0] == '\0' 是否成立。
    if (path == nullptr || path[0] == '\0')
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // struct stat：把相关数据字段组合在一起，表示项目中的一个概念。
    struct stat st{};
    // 判断条件：::stat(path, &st) != 0 || S_ISDIR(st.st_mode) 是否成立。
    if (::stat(path, &st) != 0 || S_ISDIR(st.st_mode))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* base = std::strrchr(path, '/');
    // 赋值：把右侧 (base != nullptr) ? (base + 1) : path 的结果保存到左侧 base。
    base = (base != nullptr) ? (base + 1) : path;

    // 函数调用：char title[sizeof(open_title_)];；执行该函数并处理它的返回值或副作用。
    char title[sizeof(open_title_)];
    // 赋值：把右侧 '\0' 的结果保存到左侧 title[0]。
    title[0] = '\0';
    {
        // 执行语句：char raw[48];；它会更新一次变量、对象或程序状态。
        char raw[48];
        // 只读约束：这个值或对象在当前位置不能被修改。
        const size_t flen = std::strlen(base);
        // 定义变量 size_t copy_len 并初始化为 flen。
        size_t copy_len = flen;
        // 判断条件：flen >= 4 是否成立。
        if (flen >= 4)
            // 赋值：把右侧 flen - 4U 的结果保存到左侧 copy_len。
            copy_len = flen - 4U;
        // 判断条件：copy_len >= sizeof(raw) 是否成立。
        if (copy_len >= sizeof(raw))
            // 赋值：把右侧 sizeof(raw) - 1U 的结果保存到左侧 copy_len。
            copy_len = sizeof(raw) - 1U;
        // 调用 std::memcpy(raw, base, copy_len);，执行这个函数并处理它的返回值或状态变化。
        std::memcpy(raw, base, copy_len);
        // 赋值：把右侧 '\0' 的结果保存到左侧 raw[copy_len]。
        raw[copy_len] = '\0';
        // 函数调用：(void)text::normalize_path_segment(raw, title, sizeof(title));；执行该函数并处理它的返回值或副作用。
        (void)text::normalize_path_segment(raw, title, sizeof(title));
        // 判断条件：title[0] == '\0' 是否成立。
        if (title[0] == '\0')
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const size_t n = (copy_len < sizeof(title) - 1) ? copy_len : (sizeof(title) - 1);
            // 调用 std::memcpy(title, raw, n);，执行这个函数并处理它的返回值或状态变化。
            std::memcpy(title, raw, n);
            // 赋值：把右侧 '\0' 的结果保存到左侧 title[n]。
            title[n] = '\0';
        }
    }

    // 函数调用：(void)std::strncpy(open_title_, title, sizeof(open_title_) - 1);；执行该函数并处理它的返回值或副作用。
    (void)std::strncpy(open_title_, title, sizeof(open_title_) - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 open_title_[sizeof(open_title_) - 1]。
    open_title_[sizeof(open_title_) - 1] = '\0';
    // 函数调用：(void)std::strncpy(open_path_, path, sizeof(open_path_) - 1);；执行该函数并处理它的返回值或副作用。
    (void)std::strncpy(open_path_, path, sizeof(open_path_) - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 open_path_[sizeof(open_path_) - 1]。
    open_path_[sizeof(open_path_) - 1] = '\0';

    // 判断条件：static_cast<uint32_t>(st.st_size) > TxtBook::kMaxFileBytes 是否成立。
    if (static_cast<uint32_t>(st.st_size) > TxtBook::kMaxFileBytes)
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kReaderTooLarge, 2000);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kReaderTooLarge, 2000);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 Screen::Opening 的结果保存到左侧 screen_。
    screen_ = Screen::Opening;
    // 赋值：把右侧 false 的结果保存到左侧 menu_visible_。
    menu_visible_ = false;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();

    // 只读约束：这个值或对象在当前位置不能被修改。
    const ReaderLayout ly = layout();
    // 判断条件：!book_.open(open_path_, ly) 是否成立。
    if (!book_.open(open_path_, ly))
    {
        // 赋值：把右侧 Screen::Shelf 的结果保存到左侧 screen_。
        screen_ = Screen::Shelf;
        // 调用 overlays::Toast::instance().show(ui::strings::kReaderOpenFail, 2000);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kReaderOpenFail, 2000);
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 调用 restore_progress();，执行这个函数并处理它的返回值或状态变化。
    restore_progress();
    // 赋值：把右侧 0 的结果保存到左侧 turn_count_。
    turn_count_ = 0;
    // 赋值：把右侧 Screen::Reading 的结果保存到左侧 screen_。
    screen_ = Screen::Reading;
    // 调用 save_progress();，执行这个函数并处理它的返回值或状态变化。
    save_progress();
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 ReaderApp::open_book：参数在括号里，返回类型写在函数名前面。
bool ReaderApp::open_book(uint8_t shelf_idx)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const BookItem& item = shelf_.item(shelf_idx);
    // 判断条件：item.path[0] == '\0' 是否成立。
    if (item.path[0] == '\0')
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 返回 open_book_by_path(item.path) 给调用者，表示当前函数的执行结果。
    return open_book_by_path(item.path);
}

// 定义或声明函数 ReaderApp::try_resume_saved_book：参数在括号里，返回类型写在函数名前面。
bool ReaderApp::try_resume_saved_book()
{
    // 赋值：把右侧 ReadingStore::get_instance() 的结果保存到左侧 auto& store。
    auto& store = ReadingStore::get_instance();
    // 判断条件：!store.has_book() 是否成立。
    if (!store.has_book())
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* path = store.book_path();
    // 判断条件：path == nullptr || path[0] == '\0' 是否成立。
    if (path == nullptr || path[0] == '\0')
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < shelf_.count(); ++i)
    {
        // 判断条件：std::strcmp(shelf_.item(i).path, path) == 0 是否成立。
        if (std::strcmp(shelf_.item(i).path, path) == 0)
            // 返回 open_book(i) 给调用者，表示当前函数的执行结果。
            return open_book(i);
    }
    // 返回 open_book_by_path(path) 给调用者，表示当前函数的执行结果。
    return open_book_by_path(path);
}

// 定义或声明函数 ReaderApp::fill_shelf_row：参数在括号里，返回类型写在函数名前面。
void ReaderApp::fill_shelf_row(uint8_t idx, ui::widgets::RowStyle& rs)
{
    // 赋值：把右侧 {} 的结果保存到左侧 rs。
    rs = {};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const BookItem& item = shelf_.item(idx);
    // 判断条件：item.path[0] == '\0' 是否成立。
    if (item.path[0] == '\0')
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 判断条件：item.title[0] == '\0' 是否成立。
    if (item.title[0] == '\0')
        // 函数调用：(void)std::strncpy(row_title_, "?", sizeof(row_title_) - 1);；执行该函数并处理它的返回值或副作用。
        (void)std::strncpy(row_title_, "?", sizeof(row_title_) - 1);
    // 备用分支：前面的条件不成立时执行这里。
    else
        // 函数调用：(void)gfx::truncate_text(item.title, Theme::kFontBody,；执行该函数并处理它的返回值或副作用。
        (void)gfx::truncate_text(item.title, Theme::kFontBody,
                                 // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kScreenW - 80), row_title_,。
                                 static_cast<uint16_t>(Theme::kScreenW - 80), row_title_,
                                 // 继续上一行的表达式或参数列表：sizeof(row_title_));。
                                 sizeof(row_title_));
    // 赋值：把右侧 '\0' 的结果保存到左侧 row_title_[sizeof(row_title_) - 1]。
    row_title_[sizeof(row_title_) - 1] = '\0';
    // 赋值：把右侧 row_title_ 的结果保存到左侧 rs.label。
    rs.label = row_title_;
    // 赋值：把右侧 gfx::icon::kFaBookReader 的结果保存到左侧 rs.icon_cp。
    rs.icon_cp = gfx::icon::kFaBookReader;

    // 定义变量 uint8_t pct 并初始化为 0。
    uint8_t pct = 0;
    // 判断条件：TxtIndexCache::query_read_percent(item.path, &pct) && pct > 0 是否成立。
    if (TxtIndexCache::query_read_percent(item.path, &pct) && pct > 0)
    {
        // 函数调用：(void)std::snprintf(row_value_, sizeof(row_value_), ui::strings::kReaderReadFmt,；执行该函数并处理它的返回值或副作用。
        (void)std::snprintf(row_value_, sizeof(row_value_), ui::strings::kReaderReadFmt,
                            // 继续上一行的表达式或参数列表：static_cast<unsigned>(pct));。
                            static_cast<unsigned>(pct));
        // 赋值：把右侧 row_value_ 的结果保存到左侧 rs.value。
        rs.value = row_value_;
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 调用 gfx::format_file_size(item.size_bytes, row_value_, sizeof(row_value_));，执行这个函数并处理它的返回值或状态变化。
        gfx::format_file_size(item.size_bytes, row_value_, sizeof(row_value_));
        // 赋值：把右侧 row_value_ 的结果保存到左侧 rs.value。
        rs.value = row_value_;
    }
    // 赋值：把右侧 true 的结果保存到左侧 rs.show_chevron。
    rs.show_chevron = true;
}

// 定义或声明函数 ReaderApp::on_shelf_tap：参数在括号里，返回类型写在函数名前面。
void ReaderApp::on_shelf_tap(uint8_t idx)
{
    // 函数调用：(void)open_book(idx);；执行该函数并处理它的返回值或副作用。
    (void)open_book(idx);
}

// 定义或声明函数 ReaderApp::paint_shelf：参数在括号里，返回类型写在函数名前面。
void ReaderApp::paint_shelf(gfx::Canvas& c)
{
    // 调用 ui::widgets::toolbar(c, ui::strings::kAppReader);，执行这个函数并处理它的返回值或状态变化。
    ui::widgets::toolbar(c, ui::strings::kAppReader);
    // 判断条件：shelf_.count() == 0 是否成立。
    if (shelf_.count() == 0)
    {
        // 静态查表数组 kHints：运行时复用固定数据，避免每次重新计算。
        static const char* const kHints[] = {
            // 执行这条代码：BookShelf::kScanFmtHint,。它会更新变量、对象状态或调用链。
            BookShelf::kScanFmtHint,
            // 继续上一行的表达式或参数列表：BookShelf::kScanPathInt,。
            BookShelf::kScanPathInt,
            // 继续上一行的表达式或参数列表：BookShelf::kScanPathSd,。
            BookShelf::kScanPathSd,
        };
        // 调用 ui::widgets::empty_state(c, Theme::list_region(), ui::strings::kReaderShelfEmpty, kHints,，执行这个函数并处理它的返回值或状态变化。
        ui::widgets::empty_state(c, Theme::list_region(), ui::strings::kReaderShelfEmpty, kHints,
                                 // 继续上一行的表达式或参数列表：2);。
                                 2);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 访问对象成员：list_.paint(c);；读取或修改这个对象里的成员状态。
    list_.paint(c);
}

// 定义或声明函数 ReaderApp::paint_opening：参数在括号里，返回类型写在函数名前面。
void ReaderApp::paint_opening(gfx::Canvas& c)
{
    // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
    gfx::Canvas::TextStyle ts{};
    // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontBody;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
    ts.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
    ts.v = gfx::VAlign::Middle;
    // 访问对象成员：c.text_in(layout().content_rect(), ui::strings::kReaderOpening, ts);；读取或修改这个对象里的成员状态。
    c.text_in(layout().content_rect(), ui::strings::kReaderOpening, ts);
}

// 定义或声明函数 ReaderApp::paint_top_bar：参数在括号里，返回类型写在函数名前面。
void ReaderApp::paint_top_bar(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const ReaderLayout ly = layout();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect bar = ly.top_bar_rect();
    // 访问对象成员：c.fill(bar, gfx::Ink::White);；读取或修改这个对象里的成员状态。
    c.fill(bar, gfx::Ink::White);

    // 定义变量 auto clk 并初始化为 data::Clock::now()。
    auto clk = data::Clock::now();
    // 执行语句：char time_buf[8];；它会更新一次变量、对象或程序状态。
    char time_buf[8];
    // 访问对象成员：clk.format_time_hm(time_buf, sizeof(time_buf));；读取或修改这个对象里的成员状态。
    clk.format_time_hm(time_buf, sizeof(time_buf));

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t ty = static_cast<int16_t>((ReaderLayout::kTopBarH - Theme::kFontSmall) / 2);
    // 访问对象成员：c.text(Theme::kPad, ty, time_buf, Theme::kFontSmall);；读取或修改这个对象里的成员状态。
    c.text(Theme::kPad, ty, time_buf, Theme::kFontSmall);

    // 执行语句：char page_str[16];；它会更新一次变量、对象或程序状态。
    char page_str[16];
    // 函数调用：(void)std::snprintf(page_str, sizeof(page_str), "%u/%u",；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(page_str, sizeof(page_str), "%u/%u",
                        // 继续上一行的表达式或参数列表：static_cast<unsigned>(book_.page() + 1U),。
                        static_cast<unsigned>(book_.page() + 1U),
                        // 继续上一行的表达式或参数列表：static_cast<unsigned>(book_.page_count() > 0 ? book_.page_count() : 1U));。
                        static_cast<unsigned>(book_.page_count() > 0 ? book_.page_count() : 1U));

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t bat = data::SystemState::get_instance().battery_pct();
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    (void)ui::widgets::battery_indicator(c, static_cast<int16_t>(Theme::kScreenW - Theme::kPad), 0,
                                         // 继续上一行的表达式或参数列表：bat, ReaderLayout::kTopBarH);。
                                         bat, ReaderLayout::kTopBarH);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t pw = gfx::Font::get_instance().measure(page_str, Theme::kFontSmall);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t px = static_cast<int16_t>((Theme::kScreenW - pw) / 2);
    // 访问对象成员：c.text(px, ty, page_str, Theme::kFontSmall);；读取或修改这个对象里的成员状态。
    c.text(px, ty, page_str, Theme::kFontSmall);

    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    c.hline(0, static_cast<int16_t>(ReaderLayout::kTopBarH - 1), Theme::kScreenW);
}

// 定义或声明函数 ReaderApp::paint_reading：参数在括号里，返回类型写在函数名前面。
void ReaderApp::paint_reading(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const ReaderLayout ly = layout();
    // 调用 paint_top_bar(c);，执行这个函数并处理它的返回值或状态变化。
    paint_top_bar(c);
    // 访问对象成员：book_.paint_page(c, ly.content_rect());；读取或修改这个对象里的成员状态。
    book_.paint_page(c, ly.content_rect());
    // 判断条件：menu_visible_ 是否成立。
    if (menu_visible_)
    {
        // 定义变量 uint8_t pct 并初始化为 0。
        uint8_t pct = 0;
        // 判断条件：book_.page_count() > 0 是否成立。
        if (book_.page_count() > 0)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint32_t raw = (static_cast<uint32_t>(book_.page() + 1U) * 100U) /
                                 // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
                                 static_cast<uint32_t>(book_.page_count());
            // 赋值：把右侧 static_cast<uint8_t>(raw > 100U ? 100U : raw) 的结果保存到左侧 pct。
            pct = static_cast<uint8_t>(raw > 100U ? 100U : raw);
        }
        // 函数调用：MenuPaintInfo info{font_label(), refresh_label(), pct};；执行该函数并处理它的返回值或副作用。
        MenuPaintInfo info{font_label(), refresh_label(), pct};
        // 调用 paint_menu_panel(c, ly, info);，执行这个函数并处理它的返回值或状态变化。
        paint_menu_panel(c, ly, info);
    }
}

// 定义或声明函数 ReaderApp::paint：参数在括号里，返回类型写在函数名前面。
void ReaderApp::paint(gfx::Canvas& c)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (screen_)
    {
        // 分支匹配：case Screen::Shelf 成立时执行下面的代码。
        case Screen::Shelf:
            // 调用 paint_shelf(c);，执行这个函数并处理它的返回值或状态变化。
            paint_shelf(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case Screen::Opening 成立时执行下面的代码。
        case Screen::Opening:
            // 调用 paint_opening(c);，执行这个函数并处理它的返回值或状态变化。
            paint_opening(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case Screen::Reading 成立时执行下面的代码。
        case Screen::Reading:
            // 调用 paint_reading(c);，执行这个函数并处理它的返回值或状态变化。
            paint_reading(c);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
}

// 定义或声明函数 ReaderApp::on_ui_event：参数在括号里，返回类型写在函数名前面。
void ReaderApp::on_ui_event(const ui::UiEvent& ev)
{
    // 判断条件：screen_ != Screen::Reading 是否成立。
    if (screen_ != Screen::Reading)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 判断条件：ev.kind == ui::UiEventKind::TickClock 是否成立。
    if (ev.kind == ui::UiEventKind::TickClock)
    {
        // 判断条件：ev.payload.clock.hour == tick_hour_ && ev.payload.clock.minute == tick_minute_ 是否成立。
        if (ev.payload.clock.hour == tick_hour_ && ev.payload.clock.minute == tick_minute_)
            // 提前返回：结束当前函数，不返回具体值。
            return;
        // 赋值：把右侧 ev.payload.clock.hour 的结果保存到左侧 tick_hour_。
        tick_hour_ = ev.payload.clock.hour;
        // 赋值：把右侧 ev.payload.clock.minute 的结果保存到左侧 tick_minute_。
        tick_minute_ = ev.payload.clock.minute;
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：ev.kind == ui::UiEventKind::TickBattery 是否成立。
    if (ev.kind == ui::UiEventKind::TickBattery)
    {
        // 判断条件：ev.payload.battery.pct == tick_battery_ 是否成立。
        if (ev.payload.battery.pct == tick_battery_)
            // 提前返回：结束当前函数，不返回具体值。
            return;
        // 赋值：把右侧 ev.payload.battery.pct 的结果保存到左侧 tick_battery_。
        tick_battery_ = ev.payload.battery.pct;
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
    }
}

// 定义或声明函数 ReaderApp::in_reading_content：参数在括号里，返回类型写在函数名前面。
bool ReaderApp::in_reading_content(int16_t x, int16_t y) const
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const ReaderLayout ly = layout();
    // 判断条件：y < ReaderLayout::kTopBarH 是否成立。
    if (y < ReaderLayout::kTopBarH)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：menu_visible_ && y >= ly.menu_panel_y() 是否成立。
    if (menu_visible_ && y >= ly.menu_panel_y())
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 执行语句：(void)x;；它会更新一次变量、对象或程序状态。
    (void)x;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 ReaderApp::go_next_page：参数在括号里，返回类型写在函数名前面。
void ReaderApp::go_next_page()
{
    // 判断条件：!book_.next_page() 是否成立。
    if (!book_.next_page())
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 after_page_turn();，执行这个函数并处理它的返回值或状态变化。
    after_page_turn();
}

// 定义或声明函数 ReaderApp::go_prev_page：参数在括号里，返回类型写在函数名前面。
void ReaderApp::go_prev_page()
{
    // 判断条件：!book_.prev_page() 是否成立。
    if (!book_.prev_page())
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 after_page_turn();，执行这个函数并处理它的返回值或状态变化。
    after_page_turn();
}

// 定义或声明函数 ReaderApp::go_page_relative：参数在括号里，返回类型写在函数名前面。
void ReaderApp::go_page_relative(int delta)
{
    // 判断条件：delta == 0 || book_.page_count() == 0 是否成立。
    if (delta == 0 || book_.page_count() == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 定义变量 int32_t np 并初始化为 static_cast<int32_t>(book_.page()) + delta。
    int32_t np = static_cast<int32_t>(book_.page()) + delta;
    // 判断条件：np < 0 是否成立。
    if (np < 0)
        // 赋值：把右侧 0 的结果保存到左侧 np。
        np = 0;
    // 判断条件：np >= book_.page_count() 是否成立。
    if (np >= book_.page_count())
        // 赋值：把右侧 static_cast<int32_t>(book_.page_count()) - 1 的结果保存到左侧 np。
        np = static_cast<int32_t>(book_.page_count()) - 1;
    // 调用 go_to_page(static_cast<uint16_t>(np));，执行这个函数并处理它的返回值或状态变化。
    go_to_page(static_cast<uint16_t>(np));
}

// 定义或声明函数 ReaderApp::go_to_page：参数在括号里，返回类型写在函数名前面。
void ReaderApp::go_to_page(uint16_t page_index)
{
    // 判断条件：book_.page_count() == 0 || page_index >= book_.page_count() 是否成立。
    if (book_.page_count() == 0 || page_index >= book_.page_count())
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：!book_.set_page(page_index) 是否成立。
    if (!book_.set_page(page_index))
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 after_page_turn();，执行这个函数并处理它的返回值或状态变化。
    after_page_turn();
}

// 定义或声明函数 ReaderApp::cycle_font_scale：参数在括号里，返回类型写在函数名前面。
void ReaderApp::cycle_font_scale(int dir)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int next = static_cast<int>(font_scale_) + dir;
    // 赋值：把右侧 static_cast<uint8_t>((next % 3 + 3) % 3) 的结果保存到左侧 font_scale_。
    font_scale_ = static_cast<uint8_t>((next % 3 + 3) % 3);
    // 调用 data::Persist::set_u8(kNvsFont, font_scale_);，执行这个函数并处理它的返回值或状态变化。
    data::Persist::set_u8(kNvsFont, font_scale_);
    // 调用 data::Persist::commit();，执行这个函数并处理它的返回值或状态变化。
    data::Persist::commit();

    // 执行语句：char msg[24];；它会更新一次变量、对象或程序状态。
    char msg[24];
    // 函数调用：(void)std::snprintf(msg, sizeof(msg), "\u5B57\u53F7\uFF1A%s", font_label());；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(msg, sizeof(msg), "\u5B57\u53F7\uFF1A%s", font_label());
    // 调用 show(msg,1200)：ensuretimer()：第一次调用创建一次性软件定时器，后续修改周期。。
    overlays::Toast::instance().show(msg, 1200);
    // 调用 apply_font_change();，执行这个函数并处理它的返回值或状态变化。
    apply_font_change();
}

// 定义或声明函数 ReaderApp::apply_font_change：参数在括号里，返回类型写在函数名前面。
void ReaderApp::apply_font_change()
{
    // 判断条件：!book_.is_open() 是否成立。
    if (!book_.is_open())
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 save_progress();，执行这个函数并处理它的返回值或状态变化。
    save_progress();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const ReaderLayout ly = layout();
    // 访问对象成员：(void)book_.set_layout(ly);；读取或修改这个对象里的成员状态。
    (void)book_.set_layout(ly);
    // 调用 restore_progress();，执行这个函数并处理它的返回值或状态变化。
    restore_progress();
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 定义或声明函数 ReaderApp::cycle_refresh_mode：参数在括号里，返回类型写在函数名前面。
void ReaderApp::cycle_refresh_mode(int dir)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int next = static_cast<int>(refresh_mode_) + dir;
    // 赋值：把右侧 static_cast<uint8_t>((next % 3 + 3) % 3) 的结果保存到左侧 refresh_mode_。
    refresh_mode_ = static_cast<uint8_t>((next % 3 + 3) % 3);
    // 调用 data::Persist::set_u8(kNvsRefresh, refresh_mode_);，执行这个函数并处理它的返回值或状态变化。
    data::Persist::set_u8(kNvsRefresh, refresh_mode_);
    // 调用 data::Persist::commit();，执行这个函数并处理它的返回值或状态变化。
    data::Persist::commit();
    // 赋值：把右侧 0 的结果保存到左侧 turn_count_。
    turn_count_ = 0;

    // 执行语句：char msg[32];；它会更新一次变量、对象或程序状态。
    char msg[32];
    // 函数调用：(void)std::snprintf(msg, sizeof(msg), "\u5237\u65B0\uFF1A%s", refresh_label());；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(msg, sizeof(msg), "\u5237\u65B0\uFF1A%s", refresh_label());
    // 调用 show(msg,1200)：ensuretimer()：第一次调用创建一次性软件定时器，后续修改周期。。
    overlays::Toast::instance().show(msg, 1200);
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 定义或声明函数 ReaderApp::open_page_jump：参数在括号里，返回类型写在函数名前面。
void ReaderApp::open_page_jump()
{
    // 判断条件：book_.page_count() == 0 是否成立。
    if (book_.page_count() == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 执行语句：char init[8];；它会更新一次变量、对象或程序状态。
    char init[8];
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    (void)std::snprintf(init, sizeof(init), "%u", static_cast<unsigned>(book_.page() + 1U));
    // 函数调用：(void)std::snprintf(jump_kb_title_, sizeof(jump_kb_title_), "\u9875\u7801 1-%u",；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(jump_kb_title_, sizeof(jump_kb_title_), "\u9875\u7801 1-%u",
                        // 继续上一行的表达式或参数列表：static_cast<unsigned>(book_.page_count()));。
                        static_cast<unsigned>(book_.page_count()));

    // 声明并默认初始化对象：overlays::KeyboardConfig kc{}。
    overlays::KeyboardConfig kc{};
    // 赋值：把右侧 overlays::KeyboardLayer::Numbers 的结果保存到左侧 kc.default_layer。
    kc.default_layer = overlays::KeyboardLayer::Numbers;
    // 赋值：把右侧 5 的结果保存到左侧 kc.max_len。
    kc.max_len = 5;
    // 赋值：把右侧 init 的结果保存到左侧 kc.initial_text。
    kc.initial_text = init;
    // 赋值：把右侧 jump_kb_title_ 的结果保存到左侧 kc.title。
    kc.title = jump_kb_title_;
    // 调用 overlays::Keyboard::instance().open(kc, on_page_jump_done, this);，执行这个函数并处理它的返回值或状态变化。
    overlays::Keyboard::instance().open(kc, on_page_jump_done, this);
    // 赋值：把右侧 false 的结果保存到左侧 menu_visible_。
    menu_visible_ = false;
}

// 定义或声明函数 ReaderApp::on_page_jump_done：参数在括号里，返回类型写在函数名前面。
void ReaderApp::on_page_jump_done(const char* text, void* user)
{
    // 赋值：把右侧 static_cast<ReaderApp*>(user) 的结果保存到左侧 auto* self。
    auto* self = static_cast<ReaderApp*>(user);
    // 判断条件：self == nullptr 是否成立。
    if (self == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 判断条件：text == nullptr || text[0] == '\0' 是否成立。
    if (text == nullptr || text[0] == '\0')
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kReaderJumpEmpty, 1500);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kReaderJumpEmpty, 1500);
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        self->request_repaint();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (const char* p = text; *p != '\0'; ++p)
    {
        // 判断条件：*p < '0' || *p > '9' 是否成立。
        if (*p < '0' || *p > '9')
        {
            // 调用 overlays::Toast::instance().show(ui::strings::kReaderJumpInvalid, 1500);，执行这个函数并处理它的返回值或状态变化。
            overlays::Toast::instance().show(ui::strings::kReaderJumpInvalid, 1500);
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            self->request_repaint();
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }
    }

    // 赋值：把右侧 nullptr 的结果保存到左侧 char* end。
    char* end = nullptr;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const unsigned long val = std::strtoul(text, &end, 10);
    // 判断条件：end == text || val < 1UL || val > self->book_.page_count() 是否成立。
    if (end == text || val < 1UL || val > self->book_.page_count())
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kReaderJumpInvalid, 1500);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kReaderJumpInvalid, 1500);
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        self->request_repaint();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    self->go_to_page(static_cast<uint16_t>(val - 1UL));
}

// 定义或声明函数 ReaderApp::handle_menu_hit：参数在括号里，返回类型写在函数名前面。
void ReaderApp::handle_menu_hit(MenuHit hit)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (hit)
    {
        // 分支匹配：case MenuHit::FontPrev 成立时执行下面的代码。
        case MenuHit::FontPrev:
            // 调用 cycle_font_scale(-1);，执行这个函数并处理它的返回值或状态变化。
            cycle_font_scale(-1);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case MenuHit::FontNext 成立时执行下面的代码。
        case MenuHit::FontNext:
            // 调用 cycle_font_scale(1);，执行这个函数并处理它的返回值或状态变化。
            cycle_font_scale(1);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case MenuHit::RefreshPrev 成立时执行下面的代码。
        case MenuHit::RefreshPrev:
            // 调用 cycle_refresh_mode(-1);，执行这个函数并处理它的返回值或状态变化。
            cycle_refresh_mode(-1);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case MenuHit::RefreshNext 成立时执行下面的代码。
        case MenuHit::RefreshNext:
            // 调用 cycle_refresh_mode(1);，执行这个函数并处理它的返回值或状态变化。
            cycle_refresh_mode(1);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case MenuHit::JumpPrev10 成立时执行下面的代码。
        case MenuHit::JumpPrev10:
            // 调用 go_page_relative(-10);，执行这个函数并处理它的返回值或状态变化。
            go_page_relative(-10);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case MenuHit::PagePrev 成立时执行下面的代码。
        case MenuHit::PagePrev:
            // 调用 go_prev_page();，执行这个函数并处理它的返回值或状态变化。
            go_prev_page();
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case MenuHit::PageNext 成立时执行下面的代码。
        case MenuHit::PageNext:
            // 调用 go_next_page();，执行这个函数并处理它的返回值或状态变化。
            go_next_page();
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case MenuHit::JumpNext10 成立时执行下面的代码。
        case MenuHit::JumpNext10:
            // 调用 go_page_relative(10);，执行这个函数并处理它的返回值或状态变化。
            go_page_relative(10);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case MenuHit::Shelf 成立时执行下面的代码。
        case MenuHit::Shelf:
            // 调用 return_to_shelf();，执行这个函数并处理它的返回值或状态变化。
            return_to_shelf();
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case MenuHit::JumpPage 成立时执行下面的代码。
        case MenuHit::JumpPage:
            // 调用 open_page_jump();，执行这个函数并处理它的返回值或状态变化。
            open_page_jump();
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case MenuHit::Lock 成立时执行下面的代码。
        case MenuHit::Lock:
            // 调用 go_lock_screen();，执行这个函数并处理它的返回值或状态变化。
            go_lock_screen();
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
}

// 定义或声明函数 ReaderApp::handle_reading_tap：参数在括号里，返回类型写在函数名前面。
bool ReaderApp::handle_reading_tap(int16_t x, int16_t y)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const ReaderLayout ly = layout();

    // 判断条件：menu_visible_ 是否成立。
    if (menu_visible_)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const MenuHit hit = hit_menu(x, y, ly);
        // 判断条件：hit != MenuHit::None 是否成立。
        if (hit != MenuHit::None)
        {
            // 调用 handle_menu_hit(hit);，执行这个函数并处理它的返回值或状态变化。
            handle_menu_hit(hit);
            // 返回 true 给调用者，表示当前函数的执行结果。
            return true;
        }
    }

    // 判断条件：!in_reading_content(x, y) 是否成立。
    if (!in_reading_content(x, y))
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t third = static_cast<int16_t>(Theme::kScreenW / 3);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t local_x = x;

    // 判断条件：local_x < third 是否成立。
    if (local_x < third)
    {
        // 调用 go_prev_page();，执行这个函数并处理它的返回值或状态变化。
        go_prev_page();
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 判断条件：local_x > third * 2 是否成立。
    if (local_x > third * 2)
    {
        // 调用 go_next_page();，执行这个函数并处理它的返回值或状态变化。
        go_next_page();
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 赋值：把右侧 !menu_visible_ 的结果保存到左侧 menu_visible_。
    menu_visible_ = !menu_visible_;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 ReaderApp::on_input：参数在括号里，返回类型写在函数名前面。
shell::InputResult ReaderApp::on_input(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;

    // 判断条件：overlays::Keyboard::instance().is_open() 是否成立。
    if (overlays::Keyboard::instance().is_open())
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 判断条件：screen_ == Screen::Shelf 是否成立。
    if (screen_ == Screen::Shelf)
    {
        // 判断条件：ev.type == EventType::Tap & 是否成立。
        if (ev.type == EventType::Tap &&
            // 函数定义/声明：调用 ui::widgets::hit_toolbar_back()：toolbar()： 在固定工具栏矩形中画返回图标、标题和底部横线。返回图标使用 Font Awesome 的码点，而不是手绘 SVG。。
            ui::widgets::hit_toolbar_back(static_cast<int16_t>(ev.x), static_cast<int16_t>(ev.y)))
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

    // 判断条件：screen_ == Screen::Opening 是否成立。
    if (screen_ == Screen::Opening)
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};

    // 判断条件：screen_ == Screen::Reading 是否成立。
    if (screen_ == Screen::Reading)
    {
        // 判断条件：ev.type == EventType::Tap 是否成立。
        if (ev.type == EventType::Tap)
        {
            // 判断条件：handle_reading_tap(static_cast<int16_t>(ev.x), static_cast<int16_t>(ev.y)) 是否成立。
            if (handle_reading_tap(static_cast<int16_t>(ev.x), static_cast<int16_t>(ev.y)))
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
        }
        // 判断条件：ev.type == EventType::SwipeLeft 是否成立。
        if (ev.type == EventType::SwipeLeft)
        {
            // 调用 go_next_page();，执行这个函数并处理它的返回值或状态变化。
            go_next_page();
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        }
        // 判断条件：ev.type == EventType::SwipeRight 是否成立。
        if (ev.type == EventType::SwipeRight)
        {
            // 调用 go_prev_page();，执行这个函数并处理它的返回值或状态变化。
            go_prev_page();
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        }
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 返回 {} 给调用者，表示当前函数的执行结果。
    return {};
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::reader
