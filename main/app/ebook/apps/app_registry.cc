/**
 * [初学者导读] 文件：main/app/ebook/apps/app_registry.cc
 * 作用：登记所有 App，并负责打开、关闭和查找应用。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app_registry.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/clock/clock_app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/drawing/drawing_app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/files/files_app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/music/music_app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/reader_app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/calendar/calendar_app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/gallery/gallery_app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/notepad/notepad_app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/weather/weather_app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/wooden_fish/wooden_fish_app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/stub_app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/icon.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "AppRegistry";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 函数调用：StubApp s_update_app(AppId::Update, ui::strings::kAppUpdate, gfx::icon::kFaDownload);；执行该函数并处理它的返回值或副作用。
StubApp s_update_app(AppId::Update, ui::strings::kAppUpdate, gfx::icon::kFaDownload);

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 AppRegistry::instance：参数在括号里，返回类型写在函数名前面。
AppRegistry& AppRegistry::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static AppRegistry s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 函数定义/声明：调用 AppRegistry::register_app()：登记应用对象。
void AppRegistry::register_app(App* app)
{
    // 判断条件：app == nullptr || count_ >= kMaxApps 是否成立。
    if (app == nullptr || count_ >= kMaxApps)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 app 的结果保存到左侧 apps_[count_++]。
    apps_[count_++] = app;
}

// [初学者导读] AppRegistry::register_defaults()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void AppRegistry::register_defaults()
{
    // 调用 register_app(&reader::ReaderApp::instance());，执行这个函数并处理它的返回值或状态变化。
    register_app(&reader::ReaderApp::instance());
    // 调用 register_app(&notepad::NotepadApp::instance());，执行这个函数并处理它的返回值或状态变化。
    register_app(&notepad::NotepadApp::instance());
    // 调用 register_app(&gallery::GalleryApp::instance());，执行这个函数并处理它的返回值或状态变化。
    register_app(&gallery::GalleryApp::instance());
    // 调用 register_app(&drawing::DrawingApp::instance());，执行这个函数并处理它的返回值或状态变化。
    register_app(&drawing::DrawingApp::instance());
    // 调用 register_app(&music::MusicApp::instance());，执行这个函数并处理它的返回值或状态变化。
    register_app(&music::MusicApp::instance());
    // 调用 register_app(&weather::WeatherApp::instance());，执行这个函数并处理它的返回值或状态变化。
    register_app(&weather::WeatherApp::instance());
    // 调用 register_app(&clock::ClockApp::instance());，执行这个函数并处理它的返回值或状态变化。
    register_app(&clock::ClockApp::instance());
    // 调用 register_app(&calendar::CalendarApp::instance());，执行这个函数并处理它的返回值或状态变化。
    register_app(&calendar::CalendarApp::instance());
    // 调用 register_app(&wooden_fish::WoodenFishApp::instance());，执行这个函数并处理它的返回值或状态变化。
    register_app(&wooden_fish::WoodenFishApp::instance());
    // 调用 register_app(&files::FilesApp::instance());，执行这个函数并处理它的返回值或状态变化。
    register_app(&files::FilesApp::instance());
    // 调用 register_app(&s_update_app)：登记应用对象。
    register_app(&s_update_app);
    // 调用 register_app(&settings::SettingsApp::instance());，执行这个函数并处理它的返回值或状态变化。
    register_app(&settings::SettingsApp::instance());
    // 调用 EBOOK_LOGI(TAG, "registered %u apps", static_cast<unsigned>(count_));，执行这个函数并处理它的返回值或状态变化。
    EBOOK_LOGI(TAG, "registered %u apps", static_cast<unsigned>(count_));
}

// 函数定义/声明：调用 AppRegistry::find()：find(id) 根据 AppId 找对象；entryat(idx) 根据网格索引返回 {id, title, iconcp} 这样的轻量条目。。
App* AppRegistry::find(AppId id)
{
    // 判断条件：id == AppId::None 是否成立。
    if (id == AppId::None)
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < count_; ++i)
        // 判断条件：apps_[i] != nullptr && apps_[i]->id() == id 是否成立。
        if (apps_[i] != nullptr && apps_[i]->id() == id)
            // 返回 apps_[i] 给调用者，表示当前函数的执行结果。
            return apps_[i];
    // 返回 nullptr 给调用者，表示当前函数的执行结果。
    return nullptr;
}

// 只读约束：这个值或对象在当前位置不能被修改。
const App* AppRegistry::find(AppId id) const
{
    // 返回结果：调用 find(id)：find(id) 根据 AppId 找对象；entryat(idx) 根据网格索引返回 {id, title, iconcp} 这样的轻量条目。。
    return const_cast<AppRegistry*>(this)->find(id);
}

// 函数定义/声明：调用 AppRegistry::open()：打开 App 时真正发生的事。
bool AppRegistry::open(AppId id, uint16_t page)
{
    // 调用 find(id)：find(id) 根据 AppId 找对象；entryat(idx) 根据网格索引返回 {id, title, iconcp} 这样的轻量条目。。
    App* a = find(id);
    // 判断条件：a == nullptr 是否成立。
    if (a == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：active_ != nullptr && active_ != a 是否成立。
    if (active_ != nullptr && active_ != a)
        // 调用 on_exit()：退出时取消未完成的扫描、关闭弹窗，并确保后台任务不会在对象状态失效后继续写数据。。
        active_->on_exit();
    // 赋值：把右侧 a 的结果保存到左侧 active_。
    active_ = a;
    // 赋值：把右侧 page 的结果保存到左侧 active_page_。
    active_page_ = page;
    // 调用 on_enter()：进入文件 App 时： 1. 重置浏览状态或恢复根卷状态。 2. 清空弹窗。 3. 设置加载状态。 4. 调度目录扫描。 5. 请求重绘。。
    active_->on_enter();
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// [初学者导读] AppRegistry::close_active()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void AppRegistry::close_active()
{
    // 判断条件：active_ == nullptr 是否成立。
    if (active_ == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 on_exit()：退出时取消未完成的扫描、关闭弹窗，并确保后台任务不会在对象状态失效后继续写数据。。
    active_->on_exit();
    // 赋值：把右侧 nullptr 的结果保存到左侧 active_。
    active_ = nullptr;
    // 赋值：把右侧 0 的结果保存到左侧 active_page_。
    active_page_ = 0;
}

// 定义或声明函数 AppRegistry::entry_at：参数在括号里，返回类型写在函数名前面。
AppEntry AppRegistry::entry_at(uint8_t idx) const
{
    // 判断条件：idx >= count_ || apps_[idx] == nullptr 是否成立。
    if (idx >= count_ || apps_[idx] == nullptr)
        // 返回 AppEntry{AppId::None, "", 0} 给调用者，表示当前函数的执行结果。
        return AppEntry{AppId::None, "", 0};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const App* a = apps_[idx];
    // 返回结果：调用 title()：返回 App 标题。在文件浏览器内，标题有时会动态反映当前目录，因此可能存在一个 buffer，而不是只返回一个常量字符串。。
    return AppEntry{a->id(), a->title(), a->icon_cp()};
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps
