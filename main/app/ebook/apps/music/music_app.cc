/**
 * [初学者导读] 文件：main/app/ebook/apps/music/music_app.cc
 * 作用：实现音乐列表、播放控制、进度和歌曲信息显示。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/music/music_app.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/music/music_library.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/music/music_store.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/toast.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/icon.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/text_layout.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/text_layout.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "platform/music_player.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/task/task.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_bus.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::music {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using platform::MusicPlayer;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using platform::MusicPlayState;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;

// 任务栈大小：5120，栈太小会崩溃，太大会浪费 RAM。
constexpr uint32_t kScanStack = 5120;
// 固定参数 kCtrlBarH = 44：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kCtrlBarH = 44;
// 固定参数 kCtrlIconSize = 22：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kCtrlIconSize = 22;

// 定义或声明函数 fmt_mmss：参数在括号里，返回类型写在函数名前面。
void fmt_mmss(uint16_t sec, char* out, size_t cap)
{
    // 判断条件：out == nullptr || cap < 6 是否成立。
    if (out == nullptr || cap < 6)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t m = static_cast<uint16_t>(sec / 60U);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t s = static_cast<uint16_t>(sec % 60U);
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    (void)std::snprintf(out, cap, "%u:%02u", static_cast<unsigned>(m), static_cast<unsigned>(s));
}

// 定义或声明函数 title_from_path：参数在括号里，返回类型写在函数名前面。
void title_from_path(const char* path, char* out, size_t cap)
{
    // 判断条件：path == nullptr || out == nullptr || cap == 0 是否成立。
    if (path == nullptr || out == nullptr || cap == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* name = std::strrchr(path, '/');
    // 赋值：把右侧 (name != nullptr) ? (name + 1) : path 的结果保存到左侧 name。
    name = (name != nullptr) ? (name + 1) : path;
    // 函数调用：(void)std::strncpy(out, name, cap - 1);；执行该函数并处理它的返回值或副作用。
    (void)std::strncpy(out, name, cap - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 out[cap - 1]。
    out[cap - 1] = '\0';
    // 赋值：把右侧 std::strrchr(out, '.') 的结果保存到左侧 char* dot。
    char* dot = std::strrchr(out, '.');
    // 判断条件：dot != nullptr 是否成立。
    if (dot != nullptr)
        *dot = '\0';
}

// 定义或声明函数 sync_track_index：参数在括号里，返回类型写在函数名前面。
void sync_track_index(uint8_t& track_idx)
{
    // 赋值：把右侧 MusicPlayer::get_instance() 的结果保存到左侧 auto& player。
    auto& player = MusicPlayer::get_instance();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* cur = player.current_path();
    // 判断条件：cur == nullptr || cur[0] == '\0' 是否成立。
    if (cur == nullptr || cur[0] == '\0')
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 MusicLibrary::get_instance() 的结果保存到左侧 auto& lib。
    auto& lib = MusicLibrary::get_instance();
    // 定义变量 int8_t idx 并初始化为 lib.find_index(cur)。
    int8_t idx = lib.find_index(cur);
    // 判断条件：idx < 0 是否成立。
    if (idx < 0)
        // 访问对象成员：(void)lib.add_track_from_path(cur);；读取或修改这个对象里的成员状态。
        (void)lib.add_track_from_path(cur);
    // 赋值：把右侧 lib.find_index(cur) 的结果保存到左侧 idx。
    idx = lib.find_index(cur);
    // 判断条件：idx >= 0 是否成立。
    if (idx >= 0)
        // 赋值：把右侧 static_cast<uint8_t>(idx) 的结果保存到左侧 track_idx。
        track_idx = static_cast<uint8_t>(idx);
}

// 定义或声明函数 fmt_size_or_dur：参数在括号里，返回类型写在函数名前面。
void fmt_size_or_dur(uint32_t bytes, uint16_t dur_sec, char* out, size_t cap)
{
    // 判断条件：out == nullptr || cap < 4 是否成立。
    if (out == nullptr || cap < 4)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：dur_sec > 0 是否成立。
    if (dur_sec > 0)
    {
        // 执行语句：char t[16];；它会更新一次变量、对象或程序状态。
        char t[16];
        // 调用 fmt_mmss(dur_sec, t, sizeof(t));，执行这个函数并处理它的返回值或状态变化。
        fmt_mmss(dur_sec, t, sizeof(t));
        // 函数调用：(void)std::snprintf(out, cap, "%s", t);；执行该函数并处理它的返回值或副作用。
        (void)std::snprintf(out, cap, "%s", t);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 调用 gfx::format_file_size(bytes, out, cap);，执行这个函数并处理它的返回值或状态变化。
    gfx::format_file_size(bytes, out, cap);
}

// 固定参数 kPlayPathCap = 96：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kPlayPathCap = 96;

// 定义变量 bool s_play_on_enter_ 并初始化为 false。
bool s_play_on_enter_ = false;
// 执行语句：char s_play_path_[kPlayPathCap]{};；它会更新一次变量、对象或程序状态。
char s_play_path_[kPlayPathCap]{};

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 MusicApp::instance：参数在括号里，返回类型写在函数名前面。
MusicApp& MusicApp::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static MusicApp s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义或声明函数 MusicApp::MusicApp：参数在括号里，返回类型写在函数名前面。
MusicApp::MusicApp()
{
    // 访问对象成员：list_.set_area(Theme::list_region());；读取或修改这个对象里的成员状态。
    list_.set_area(Theme::list_region());
    // 开始代码块：下面是这个函数、分支或循环的主体。
    list_.set_provider([](uint8_t i, ui::widgets::RowStyle& rs) {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const auto& lib = MusicLibrary::get_instance();
        // 判断条件：i >= lib.count() 是否成立。
        if (i >= lib.count())
            // 提前返回：结束当前函数，不返回具体值。
            return;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const auto& t = lib.at(i);
        // 静态字符缓冲区 sub：保存临时文本，跨函数调用复用。
        static char sub[24];
        // 调用 fmt_size_or_dur(t.size_bytes, t.duration_sec, sub, sizeof(sub));，执行这个函数并处理它的返回值或状态变化。
        fmt_size_or_dur(t.size_bytes, t.duration_sec, sub, sizeof(sub));
        // 赋值：把右侧 t.title 的结果保存到左侧 rs.label。
        rs.label = t.title;
        // 赋值：把右侧 sub 的结果保存到左侧 rs.value。
        rs.value = sub;
        // 赋值：把右侧 gfx::icon::kFaMusic 的结果保存到左侧 rs.icon_cp。
        rs.icon_cp = gfx::icon::kFaMusic;
        // 赋值：把右侧 true 的结果保存到左侧 rs.show_chevron。
        rs.show_chevron = true;
    // 执行语句：});；它会更新一次变量、对象或程序状态。
    });
    // 访问对象成员：list_.set_tap_handler([this](uint8_t i) { play_index(i); });；读取或修改这个对象里的成员状态。
    list_.set_tap_handler([this](uint8_t i) { play_index(i); });
}

// 定义只读对象或变量：初始化后不能在当前位置修改。
const char* MusicApp::title() const
{
    // 返回 (view_ == View::Player) ? ui::strings::kMusicPlaying : ui::strings::kAppMusic 给调用者，表示当前函数的执行结果。
    return (view_ == View::Player) ? ui::strings::kMusicPlaying : ui::strings::kAppMusic;
}

// 函数定义/声明：调用 MusicApp::icon_cp()：返回图标 codepoint。图标通常来自字体图标表，所以这里返回的是 uint32t，不是图片路径。。
uint32_t MusicApp::icon_cp() const
{
    // 返回 gfx::icon::kFaMusic 给调用者，表示当前函数的执行结果。
    return gfx::icon::kFaMusic;
}

// 定义或声明函数 MusicApp::player_body_rect：参数在括号里，返回类型写在函数名前面。
core::Rect MusicApp::player_body_rect()
{
    // 返回 core::Rect{0, static_cast<int16_t>(Theme::kListStartY), Theme::kScreenW, 给调用者，表示当前函数的执行结果。
    return core::Rect{0, static_cast<int16_t>(Theme::kListStartY), Theme::kScreenW,
                      // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kScreenH - Theme::kListStartY - kCtrlBarH)};。
                      static_cast<uint16_t>(Theme::kScreenH - Theme::kListStartY - kCtrlBarH)};
}

// 定义或声明函数 MusicApp::player_controls_rect：参数在括号里，返回类型写在函数名前面。
core::Rect MusicApp::player_controls_rect()
{
    // 返回 core::Rect{0, static_cast<int16_t>(Theme::kScreenH - kCtrlBarH), Theme::kScreenW, 给调用者，表示当前函数的执行结果。
    return core::Rect{0, static_cast<int16_t>(Theme::kScreenH - kCtrlBarH), Theme::kScreenW,
                      // 继续上一行的表达式或参数列表：kCtrlBarH};。
                      kCtrlBarH};
}

// 定义或声明函数 MusicApp::ctrl_btn_rect：参数在括号里，返回类型写在函数名前面。
core::Rect MusicApp::ctrl_btn_rect(uint8_t idx)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect bar = player_controls_rect();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t w3 = static_cast<uint16_t>(bar.w / 3);
    // 返回 core::Rect{static_cast<int16_t>(bar.x + idx * w3), bar.y, w3, bar.h} 给调用者，表示当前函数的执行结果。
    return core::Rect{static_cast<int16_t>(bar.x + idx * w3), bar.y, w3, bar.h};
}

// 定义或声明函数 MusicApp::set_view：参数在括号里，返回类型写在函数名前面。
void MusicApp::set_view(View v)
{
    // 赋值：把右侧 v 的结果保存到左侧 view_。
    view_ = v;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 定义或声明函数 MusicApp::request_play_on_enter：参数在括号里，返回类型写在函数名前面。
void MusicApp::request_play_on_enter(const char* path)
{
    // 判断条件：path == nullptr || path[0] == '\0' 是否成立。
    if (path == nullptr || path[0] == '\0')
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 函数调用：(void)std::strncpy(s_play_path_, path, kPlayPathCap - 1);；执行该函数并处理它的返回值或副作用。
    (void)std::strncpy(s_play_path_, path, kPlayPathCap - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 s_play_path_[kPlayPathCap - 1]。
    s_play_path_[kPlayPathCap - 1] = '\0';
    // 赋值：把右侧 true 的结果保存到左侧 s_play_on_enter_。
    s_play_on_enter_ = true;
}

// 定义或声明函数 MusicApp::on_enter：参数在括号里，返回类型写在函数名前面。
void MusicApp::on_enter()
{
    // 赋值：把右侧 View::Library 的结果保存到左侧 view_。
    view_ = View::Library;
    // 赋值：把右侧 false 的结果保存到左侧 scanning_。
    scanning_ = false;
    // 访问对象成员：list_.set_scroll(0);；读取或修改这个对象里的成员状态。
    list_.set_scroll(0);

    // 判断条件：s_play_on_enter_ 是否成立。
    if (s_play_on_enter_)
    {
        // 赋值：把右侧 false 的结果保存到左侧 s_play_on_enter_。
        s_play_on_enter_ = false;
        // 赋值：把右侧 MusicLibrary::get_instance() 的结果保存到左侧 auto& lib。
        auto& lib = MusicLibrary::get_instance();
        // 访问对象成员：(void)lib.add_track_from_path(s_play_path_);；读取或修改这个对象里的成员状态。
        (void)lib.add_track_from_path(s_play_path_);

        // 只读约束：这个值或对象在当前位置不能被修改。
        const int8_t idx = lib.find_index(s_play_path_);
        // 判断条件：idx >= 0 是否成立。
        if (idx >= 0)
            // 赋值：把右侧 static_cast<uint8_t>(idx) 的结果保存到左侧 track_idx_。
            track_idx_ = static_cast<uint8_t>(idx);

        // 判断条件：MusicPlayer::get_instance().play(s_play_path_) 是否成立。
        if (MusicPlayer::get_instance().play(s_play_path_))
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const auto& lib2 = MusicLibrary::get_instance();
            // 判断条件：track_idx_ < lib2.count() 是否成立。
            if (track_idx_ < lib2.count())
                // 调用 MusicPlayer::get_instance().hint_duration(lib2.at(track_idx_).duration_sec);，执行这个函数并处理它的返回值或状态变化。
                MusicPlayer::get_instance().hint_duration(lib2.at(track_idx_).duration_sec);
            // 赋值：把右侧 View::Player 的结果保存到左侧 view_。
            view_ = View::Player;
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
            // 调用 overlays::Toast::instance().show(ui::strings::kMusicPlayFail, 2000);，执行这个函数并处理它的返回值或状态变化。
            overlays::Toast::instance().show(ui::strings::kMusicPlayFail, 2000);

        // 调用 start_scan();，执行这个函数并处理它的返回值或状态变化。
        start_scan();
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 调用 start_scan();，执行这个函数并处理它的返回值或状态变化。
    start_scan();
}

// 函数定义/声明：调用 MusicApp::on_exit()：退出时取消未完成的扫描、关闭弹窗，并确保后台任务不会在对象状态失效后继续写数据。。
void MusicApp::on_exit()
{
    // 调用 stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
    MusicPlayer::get_instance().stop();
    // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    scan_task_.reset();
    // 赋值：把右侧 false 的结果保存到左侧 scanning_。
    scanning_ = false;
    // 赋值：把右侧 View::Library 的结果保存到左侧 view_。
    view_ = View::Library;
}

// 函数定义/声明：调用 MusicApp::on_semantic_back()：处理语义返回，顺序通常是： 1. 如果弹窗打开，先关闭弹窗。 2. 如果列表还在浏览目录，先返回上一级。 3. 如果已经到根卷，返回 false，让 Shell 退出 App。。
bool MusicApp::on_semantic_back()
{
    // 判断条件：view_ == View::Player 是否成立。
    if (view_ == View::Player)
    {
        // 调用 stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
        MusicPlayer::get_instance().stop();
        // 调用 set_view(View::Library);，执行这个函数并处理它的返回值或状态变化。
        set_view(View::Library);
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 返回 false 给调用者，表示当前函数的执行结果。
    return false;
}

// 定义或声明函数 MusicApp::start_scan：参数在括号里，返回类型写在函数名前面。
void MusicApp::start_scan()
{
    // 判断条件：scanning_ 是否成立。
    if (scanning_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 true 的结果保存到左侧 scanning_。
    scanning_ = true;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();

    // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    scan_task_.reset();

    // 定义变量：::app::sys::task::Cfg cfg =
    ::app::sys::task::Cfg cfg =
        // 继续上一行的表达式或参数列表：::app::sys::task::Cfg::light("music_scan", ::app::sys::task::Priority::LOW);。
        ::app::sys::task::Cfg::light("music_scan", ::app::sys::task::Priority::LOW);
    // 赋值：把右侧 kScanStack 的结果保存到左侧 cfg.stack_size。
    cfg.stack_size = kScanStack;
    // 赋值：把右侧 true 的结果保存到左侧 cfg.use_psram。
    cfg.use_psram = true;

    // 执行这条代码：scan_task_ = std::make_unique<::app::sys::task::Task>(。它会更新变量、对象状态或调用链。
    scan_task_ = std::make_unique<::app::sys::task::Task>(
        // 开始代码块：下面是这个函数、分支或循环的主体。
        [](void* /*arg*/) {
            // 访问对象成员：(void)MusicLibrary::get_instance().scan();；读取或修改这个对象里的成员状态。
            (void)MusicLibrary::get_instance().scan();
            // 访问对象成员：(void)ui::UiBus::get_instance().post_system_hint(ui::SystemHintKind::MusicScanDone, 0);；读取或修改这个对象里的成员状态。
            (void)ui::UiBus::get_instance().post_system_hint(ui::SystemHintKind::MusicScanDone, 0);
        // 结束上一行的 lambda 或初始化列表：把这段匿名代码/数据交给调用者。
        },
        // 继续上一行的表达式或参数列表：cfg, nullptr);。
        cfg, nullptr);

    // 判断条件：!scan_task_->start() 是否成立。
    if (!scan_task_->start())
    {
        // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
        scan_task_.reset();
        // 赋值：把右侧 false 的结果保存到左侧 scanning_。
        scanning_ = false;
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
    }
}

// 定义或声明函数 MusicApp::on_ui_event：参数在括号里，返回类型写在函数名前面。
void MusicApp::on_ui_event(const ui::UiEvent& ev)
{
    // 判断条件：ev.kind != ui::UiEventKind::SystemHint 是否成立。
    if (ev.kind != ui::UiEventKind::SystemHint)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 多分支判断：根据括号里的值选择不同 case。
    switch (ev.payload.system.hint)
    {
        // 分支匹配：case ui::SystemHintKind::MusicScanDone:  成立时执行下面的代码。
        case ui::SystemHintKind::MusicScanDone: {
            // 赋值：把右侧 false 的结果保存到左侧 scanning_。
            scanning_ = false;
            // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
            scan_task_.reset();
            // 只读约束：这个值或对象在当前位置不能被修改。
            const auto& lib = MusicLibrary::get_instance();
            // 访问对象成员：list_.set_total(lib.count());；读取或修改这个对象里的成员状态。
            list_.set_total(lib.count());
            // 访问对象成员：list_.set_scroll(0);；读取或修改这个对象里的成员状态。
            list_.set_scroll(0);

            // 判断条件：view_ == View::Player 是否成立。
            if (view_ == View::Player)
            {
                // 调用 sync_track_index(track_idx_);，执行这个函数并处理它的返回值或状态变化。
                sync_track_index(track_idx_);
            }
            // 备用分支：前面的条件不成立时执行这里。
            else
            {
                // 定义变量 uint8_t idx 并初始化为 MusicStore::get_instance().last_index()。
                uint8_t idx = MusicStore::get_instance().last_index();
                // 判断条件：idx >= lib.count() 是否成立。
                if (idx >= lib.count())
                    // 赋值：把右侧 0 的结果保存到左侧 idx。
                    idx = 0;
                // 赋值：把右侧 idx 的结果保存到左侧 track_idx_。
                track_idx_ = idx;
            }
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            request_repaint();
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        }
        // 分支匹配：case ui::SystemHintKind::MusicProgress 成立时执行下面的代码。
        case ui::SystemHintKind::MusicProgress:
        // 分支匹配：case ui::SystemHintKind::MusicStateChanged 成立时执行下面的代码。
        case ui::SystemHintKind::MusicStateChanged:
            // 判断条件：view_ == View::Player 是否成立。
            if (view_ == View::Player)
            {
                // 仅在曲库尚无时长时回填；禁止把错误的 0xFFFF 估算写回列表
                auto& pl = MusicPlayer::get_instance();
                // 赋值：把右侧 MusicLibrary::get_instance() 的结果保存到左侧 auto& lib。
                auto& lib = MusicLibrary::get_instance();
                // 只读约束：这个值或对象在当前位置不能被修改。
                const uint16_t dur = pl.duration_sec();
                // 判断条件：dur > 0 && dur < 0xFFFFU && track_idx_ < lib.count( 是否成立。
                if (dur > 0 && dur < 0xFFFFU && track_idx_ < lib.count() &&
                    // 定义或声明函数 at：参数在括号里，返回类型写在函数名前面。
                    lib.at(track_idx_).duration_sec == 0)
                {
                    // 访问对象成员：lib.set_duration(track_idx_, dur);；读取或修改这个对象里的成员状态。
                    lib.set_duration(track_idx_, dur);
                }
                // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
                request_repaint();
            }
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
}

// 定义或声明函数 MusicApp::play_index：参数在括号里，返回类型写在函数名前面。
void MusicApp::play_index(uint8_t idx)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto& lib = MusicLibrary::get_instance();
    // 判断条件：idx >= lib.count() 是否成立。
    if (idx >= lib.count())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 idx 的结果保存到左侧 track_idx_。
    track_idx_ = idx;
    // 调用 MusicStore::get_instance().set_last_index(idx);，执行这个函数并处理它的返回值或状态变化。
    MusicStore::get_instance().set_last_index(idx);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto& t = lib.at(idx);
    // 调用 set_view(View::Player);，执行这个函数并处理它的返回值或状态变化。
    set_view(View::Player);
    // 赋值：把右侧 MusicPlayer::get_instance() 的结果保存到左侧 auto& pl。
    auto& pl = MusicPlayer::get_instance();
    // 判断条件：pl.play(t.path) 是否成立。
    if (pl.play(t.path))
        // 访问对象成员：pl.hint_duration(t.duration_sec);；读取或修改这个对象里的成员状态。
        pl.hint_duration(t.duration_sec);
}

// 定义或声明函数 MusicApp::play_adjacent：参数在括号里，返回类型写在函数名前面。
void MusicApp::play_adjacent(int8_t delta)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto& lib = MusicLibrary::get_instance();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t n = lib.count();
    // 判断条件：n == 0 是否成立。
    if (n == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 定义变量 int16_t next 并初始化为 static_cast<int16_t>(track_idx_) + delta。
    int16_t next = static_cast<int16_t>(track_idx_) + delta;
    // 判断条件：next < 0 是否成立。
    if (next < 0)
        // 赋值：把右侧 static_cast<int16_t>(n - 1) 的结果保存到左侧 next。
        next = static_cast<int16_t>(n - 1);
    // 判断条件：next >= static_cast<int16_t>(n) 是否成立。
    if (next >= static_cast<int16_t>(n))
        // 赋值：把右侧 0 的结果保存到左侧 next。
        next = 0;
    // 调用 play_index(static_cast<uint8_t>(next));，执行这个函数并处理它的返回值或状态变化。
    play_index(static_cast<uint8_t>(next));
}

// 定义或声明函数 MusicApp::paint_library：参数在括号里，返回类型写在函数名前面。
void MusicApp::paint_library(gfx::Canvas& c)
{
    // 判断条件：scanning_ 是否成立。
    if (scanning_)
    {
        // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
        gfx::Canvas::TextStyle ts{};
        // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 ts.size_px。
        ts.size_px = Theme::kFontBody;
        // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
        ts.h = gfx::HAlign::Center;
        // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
        ts.v = gfx::VAlign::Middle;
        // 访问对象成员：c.text_in(Theme::list_region(), ui::strings::kMusicScanning, ts);；读取或修改这个对象里的成员状态。
        c.text_in(Theme::list_region(), ui::strings::kMusicScanning, ts);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 判断条件：MusicLibrary::get_instance().count() == 0 是否成立。
    if (MusicLibrary::get_instance().count() == 0)
    {
        // 静态查表数组 kHints：运行时复用固定数据，避免每次重新计算。
        static const char* const kHints[] = {
            // 执行这条代码：MusicLibrary::kScanFmtHint,。它会更新变量、对象状态或调用链。
            MusicLibrary::kScanFmtHint,
            // 继续上一行的表达式或参数列表：MusicLibrary::kScanPathInt,。
            MusicLibrary::kScanPathInt,
            // 继续上一行的表达式或参数列表：MusicLibrary::kScanPathSd,。
            MusicLibrary::kScanPathSd,
        };
        // 调用 ui::widgets::empty_state(c, Theme::list_region(), ui::strings::kMusicEmpty, kHints, 3);，执行这个函数并处理它的返回值或状态变化。
        ui::widgets::empty_state(c, Theme::list_region(), ui::strings::kMusicEmpty, kHints, 3);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 访问对象成员：list_.paint(c);；读取或修改这个对象里的成员状态。
    list_.paint(c);
}

// 定义或声明函数 MusicApp::paint_player：参数在括号里，返回类型写在函数名前面。
void MusicApp::paint_player(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto& lib = MusicLibrary::get_instance();

    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* title_src = nullptr;
    // 执行语句：char fallback_title[48]{};；它会更新一次变量、对象或程序状态。
    char fallback_title[48]{};
    // 定义变量 uint16_t total 并初始化为 0。
    uint16_t total = 0;

    // 赋值：把右侧 MusicPlayer::get_instance() 的结果保存到左侧 auto& player。
    auto& player = MusicPlayer::get_instance();

    // 列表扫盘时长优先；播放器估算仅作兜底（避免 bitrate 误判盖掉正确值）
    if (track_idx_ < lib.count())
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const auto& t = lib.at(track_idx_);
        // 赋值：把右侧 t.title 的结果保存到左侧 title_src。
        title_src = t.title;
        // 赋值：把右侧 t.duration_sec 的结果保存到左侧 total。
        total = t.duration_sec;
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* path = player.current_path();
        // 判断条件：path == nullptr || path[0] == '\0' 是否成立。
        if (path == nullptr || path[0] == '\0')
            // 提前返回：结束当前函数，不返回具体值。
            return;
        // 调用 title_from_path(path, fallback_title, sizeof(fallback_title));，执行这个函数并处理它的返回值或状态变化。
        title_from_path(path, fallback_title, sizeof(fallback_title));
        // 赋值：把右侧 fallback_title 的结果保存到左侧 title_src。
        title_src = fallback_title;
    }
    // 判断条件：total == 0 是否成立。
    if (total == 0)
        // 赋值：把右侧 player.duration_sec() 的结果保存到左侧 total。
        total = player.duration_sec();
    // 判断条件：total == 0xFFFFU 是否成立。
    if (total == 0xFFFFU) // 饱和哨兵，当作未知
        // 赋值：把右侧 0 的结果保存到左侧 total。
        total = 0;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect body = player_body_rect();

    // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
    gfx::Canvas::TextStyle ts{};
    // 赋值：把右侧 Theme::kFontTitle 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontTitle;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
    ts.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Top 的结果保存到左侧 ts.v。
    ts.v = gfx::VAlign::Top;

    // 执行语句：char title_show[48];；它会更新一次变量、对象或程序状态。
    char title_show[48];
    // 函数调用：(void)gfx::truncate_text(title_src, Theme::kFontTitle,；执行该函数并处理它的返回值或副作用。
    (void)gfx::truncate_text(title_src, Theme::kFontTitle,
                             // 继续上一行的表达式或参数列表：static_cast<uint16_t>(body.w > 8 ? body.w - 8 : body.w), title_show,。
                             static_cast<uint16_t>(body.w > 8 ? body.w - 8 : body.w), title_show,
                             // 继续上一行的表达式或参数列表：sizeof(title_show));。
                             sizeof(title_show));
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect title_box{body.x, static_cast<int16_t>(body.y + 12), body.w, 24};
    // 调用 text_in(title_box,title_show,ts)：文本对齐。
    c.text_in(title_box, title_show, ts);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t elapsed = player.elapsed_sec();

    // 执行语句：char time_line[32];；它会更新一次变量、对象或程序状态。
    char time_line[32];
    // 执行语句：char a[12], b[12];；它会更新一次变量、对象或程序状态。
    char a[12], b[12];
    // 调用 fmt_mmss(elapsed, a, sizeof(a));，执行这个函数并处理它的返回值或状态变化。
    fmt_mmss(elapsed, a, sizeof(a));
    // 判断条件：total > 0 是否成立。
    if (total > 0)
    {
        // 调用 fmt_mmss(total, b, sizeof(b));，执行这个函数并处理它的返回值或状态变化。
        fmt_mmss(total, b, sizeof(b));
        // 函数调用：(void)std::snprintf(time_line, sizeof(time_line), "%s / %s", a, b);；执行该函数并处理它的返回值或副作用。
        (void)std::snprintf(time_line, sizeof(time_line), "%s / %s", a, b);
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
        // 函数调用：(void)std::snprintf(time_line, sizeof(time_line), "%s / --", a);；执行该函数并处理它的返回值或副作用。
        (void)std::snprintf(time_line, sizeof(time_line), "%s / --", a);

    // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontBody;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect time_box{body.x, static_cast<int16_t>(title_box.bottom() + 16), body.w, 18};
    // 调用 text_in(time_box,time_line,ts)：文本对齐。
    c.text_in(time_box, time_line, ts);

    // 判断条件：player.state() == MusicPlayState::Loading || player.state() == MusicPlayState::Error 是否成立。
    if (player.state() == MusicPlayState::Loading || player.state() == MusicPlayState::Error)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* msg = ui::strings::kMusicLoading;
        // 判断条件：player.state() == MusicPlayState::Error 是否成立。
        if (player.state() == MusicPlayState::Error)
        {
            // 多分支判断：根据括号里的值选择不同 case。
            switch (player.last_error())
            {
                // 分支匹配：case 2 成立时执行下面的代码。
                case 2:
                    // 赋值：把右侧 ui::strings::kMusicOpenFail 的结果保存到左侧 msg。
                    msg = ui::strings::kMusicOpenFail;
                    // 跳出当前循环或 switch，不再执行剩余分支。
                    break;
                // 分支匹配：case 3 成立时执行下面的代码。
                case 3:
                    // 赋值：把右侧 ui::strings::kMusicNoMem 的结果保存到左侧 msg。
                    msg = ui::strings::kMusicNoMem;
                    // 跳出当前循环或 switch，不再执行剩余分支。
                    break;
                // 分支匹配：case 5 成立时执行下面的代码。
                case 5:
                    // 赋值：把右侧 ui::strings::kMusicPlayFail 的结果保存到左侧 msg。
                    msg = ui::strings::kMusicPlayFail;
                    // 跳出当前循环或 switch，不再执行剩余分支。
                    break;
                // 分支匹配：case 6 成立时执行下面的代码。
                case 6:
                    // 赋值：把右侧 ui::strings::kMusicPlayFail 的结果保存到左侧 msg。
                    msg = ui::strings::kMusicPlayFail;
                    // 跳出当前循环或 switch，不再执行剩余分支。
                    break;
                // 执行这条代码：default:。它会更新变量、对象状态或调用链。
                default:
                    // 赋值：把右侧 ui::strings::kMusicPlayFail 的结果保存到左侧 msg。
                    msg = ui::strings::kMusicPlayFail;
                    // 跳出当前循环或 switch，不再执行剩余分支。
                    break;
            }
        }
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect st_box{body.x, static_cast<int16_t>(time_box.bottom() + 8), body.w, 18};
        // 调用 text_in(st_box,msg,ts)：文本对齐。
        c.text_in(st_box, msg, ts);
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect bar = player_controls_rect();
    // 访问对象成员：c.hline(bar.x, bar.y, bar.w);；读取或修改这个对象里的成员状态。
    c.hline(bar.x, bar.y, bar.w);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool playing = (player.state() == MusicPlayState::Playing);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint32_t play_icon = playing ? gfx::icon::kFaPause : gfx::icon::kFaPlay;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < 3; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect btn = ctrl_btn_rect(i);
        // 调用 rect(btn,1)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
        c.rect(btn, 1);
        // 定义变量 uint32_t icon 并初始化为 gfx::icon::kFaStepBackward。
        uint32_t icon = gfx::icon::kFaStepBackward;
        // 判断条件：i == 1 是否成立。
        if (i == 1)
            // 赋值：把右侧 play_icon 的结果保存到左侧 icon。
            icon = play_icon;
        // 判断条件：i == 2 是否成立。
        if (i == 2)
            // 赋值：把右侧 gfx::icon::kFaStepForward 的结果保存到左侧 icon。
            icon = gfx::icon::kFaStepForward;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t ax = static_cast<int16_t>(btn.x + (btn.w - kCtrlIconSize) / 2);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t ay = static_cast<int16_t>(btn.y + (btn.h - kCtrlIconSize) / 2);
        // 访问对象成员：c.glyph(ax, ay, icon, kCtrlIconSize, gfx::FontFace::Icon);；读取或修改这个对象里的成员状态。
        c.glyph(ax, ay, icon, kCtrlIconSize, gfx::FontFace::Icon);
    }
}

// 定义或声明函数 MusicApp::paint：参数在括号里，返回类型写在函数名前面。
void MusicApp::paint(gfx::Canvas& c)
{
    // 调用 title()：返回 App 标题。在文件浏览器内，标题有时会动态反映当前目录，因此可能存在一个 buffer，而不是只返回一个常量字符串。。
    ui::widgets::toolbar(c, title());
    // 判断条件：view_ == View::Library 是否成立。
    if (view_ == View::Library)
        // 调用 paint_library(c);，执行这个函数并处理它的返回值或状态变化。
        paint_library(c);
    // 备用分支：前面的条件不成立时执行这里。
    else
        // 调用 paint_player(c);，执行这个函数并处理它的返回值或状态变化。
        paint_player(c);
}

// 定义或声明函数 MusicApp::handle_player：参数在括号里，返回类型写在函数名前面。
shell::InputResult MusicApp::handle_player(int16_t x, int16_t y)
{
    // 判断条件：!player_controls_rect().contains(x, y) 是否成立。
    if (!player_controls_rect().contains(x, y))
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 判断条件：ctrl_btn_rect(0).contains(x, y) 是否成立。
    if (ctrl_btn_rect(0).contains(x, y))
    {
        // 调用 play_adjacent(-1);，执行这个函数并处理它的返回值或状态变化。
        play_adjacent(-1);
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：ctrl_btn_rect(2).contains(x, y) 是否成立。
    if (ctrl_btn_rect(2).contains(x, y))
    {
        // 调用 play_adjacent(1);，执行这个函数并处理它的返回值或状态变化。
        play_adjacent(1);
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：ctrl_btn_rect(1).contains(x, y) 是否成立。
    if (ctrl_btn_rect(1).contains(x, y))
    {
        // 赋值：把右侧 MusicPlayer::get_instance() 的结果保存到左侧 auto& pl。
        auto& pl = MusicPlayer::get_instance();
        // 判断条件：pl.state() == MusicPlayState::Playing 是否成立。
        if (pl.state() == MusicPlayState::Playing)
            // 访问对象成员：pl.pause();；读取或修改这个对象里的成员状态。
            pl.pause();
        // 备用分支：前面的条件不成立时执行这里。
        else if (pl.state() == MusicPlayState::Paused)
            // 调用 resume()：这些是对 FreeRTOS API 的直接包装： suspend() 暂停任务。 resume() 恢复。 setpriority() 动态改优先级。 getpriority() 读取当前优先级。。
            pl.resume();
        // 备用分支：前面的条件不成立时执行这里。
        else if (track_idx_ < MusicLibrary::get_instance().count())
            // 访问对象成员：(void)pl.play(MusicLibrary::get_instance().at(track_idx_).path);；读取或修改这个对象里的成员状态。
            (void)pl.play(MusicLibrary::get_instance().at(track_idx_).path);
        // 备用分支：前面的条件不成立时执行这里。
        else if (pl.current_path()[0] != '\0')
            // 访问对象成员：(void)pl.play(pl.current_path());；读取或修改这个对象里的成员状态。
            (void)pl.play(pl.current_path());
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 返回 {} 给调用者，表示当前函数的执行结果。
    return {};
}

// 定义或声明函数 MusicApp::on_input：参数在括号里，返回类型写在函数名前面。
shell::InputResult MusicApp::on_input(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;

    // 判断条件：ev.type == EventType::Tap 是否成立。
    if (ev.type == EventType::Tap)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t x = static_cast<int16_t>(ev.x);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t y = static_cast<int16_t>(ev.y);

        // 判断条件：ui::widgets::hit_toolbar_back(x, y) 是否成立。
        if (ui::widgets::hit_toolbar_back(x, y))
        {
            // 判断条件：view_ == View::Player 是否成立。
            if (view_ == View::Player)
            {
                // 调用 stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
                MusicPlayer::get_instance().stop();
                // 调用 set_view(View::Library);，执行这个函数并处理它的返回值或状态变化。
                set_view(View::Library);
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            }
            // 返回 {} 给调用者，表示当前函数的执行结果。
            return {};
        }

        // 判断条件：view_ == View::Library 是否成立。
        if (view_ == View::Library)
        {
            // 定义变量 auto out 并初始化为 list_.handle_input(ev)。
            auto out = list_.handle_input(ev);
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
        // 返回 handle_player(x, y) 给调用者，表示当前函数的执行结果。
        return handle_player(x, y);
    }

    // 判断条件：view_ == View::Library 是否成立。
    if (view_ == View::Library)
    {
        // 定义变量 auto out 并初始化为 list_.handle_input(ev)。
        auto out = list_.handle_input(ev);
        // 判断条件：out.scroll_changed 是否成立。
        if (out.scroll_changed)
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            request_repaint();
        // 判断条件：out.consumed 是否成立。
        if (out.consumed)
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
    }
    // 返回 {} 给调用者，表示当前函数的执行结果。
    return {};
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::music
