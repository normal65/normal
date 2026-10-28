/**
 * [初学者导读] 文件：main/app/ebook/apps/settings/settings_display.cc
 * 作用：实现亮度、夜间模式和自动锁屏设置。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_app.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/auto_lock.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_internal.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/system_state.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/icon.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::settings {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using detail::draw_slider_row;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using detail::slider_hit_pct;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using detail::tap_row_index;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 SettingsApp::paint_display：参数在括号里，返回类型写在函数名前面。
void SettingsApp::paint_display(gfx::Canvas& c)
{
    // 赋值：把右侧 data::SystemState::get_instance() 的结果保存到左侧 auto& sys。
    auto& sys = data::SystemState::get_instance();
    // 调用 AutoLock::get_instance()：返回自动锁屏单例。。
    auto& al = AutoLock::get_instance();
    // 定义变量 int16_t y 并初始化为 Theme::kListStartY。
    int16_t y = Theme::kListStartY;

    // 调用 brightness()：把 0 到 100 的亮度限制在合法范围后交给背光驱动。墨水屏本身不依赖背光，背光是前光/照明系统，不能把亮度当成画面灰度。。
    draw_slider_row(c, y, ui::strings::kSetBrightness, sys.brightness());
    // 赋值：把右侧 static_cast<int16_t>(y + Theme::kListRowH) 的结果保存到左侧 y。
    y = static_cast<int16_t>(y + Theme::kListRowH);

    {
        // 声明并默认初始化对象：ui::widgets::RowStyle rs{}。
        ui::widgets::RowStyle rs{};
        // 赋值：把右侧 ui::strings::kSetNightMode 的结果保存到左侧 rs.label。
        rs.label = ui::strings::kSetNightMode;
        // 赋值：把右侧 gfx::icon::toggle(sys.night_mode()) 的结果保存到左侧 rs.value_icon。
        rs.value_icon = gfx::icon::toggle(sys.night_mode());
        // 赋值：把右侧 20 的结果保存到左侧 rs.value_icon_size。
        rs.value_icon_size = 20;
        // 赋值：把右侧 false 的结果保存到左侧 rs.show_chevron。
        rs.show_chevron = false;
        // 调用 ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);，执行这个函数并处理它的返回值或状态变化。
        ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);
    }
    // 赋值：把右侧 static_cast<int16_t>(y + Theme::kListRowH) 的结果保存到左侧 y。
    y = static_cast<int16_t>(y + Theme::kListRowH);

    {
        // 声明并默认初始化对象：ui::widgets::RowStyle rs{}。
        ui::widgets::RowStyle rs{};
        // 赋值：把右侧 ui::strings::kSetSecAutolock 的结果保存到左侧 rs.label。
        rs.label = ui::strings::kSetSecAutolock;
        // 赋值：把右侧 gfx::icon::toggle(al.enabled()) 的结果保存到左侧 rs.value_icon。
        rs.value_icon = gfx::icon::toggle(al.enabled());
        // 赋值：把右侧 20 的结果保存到左侧 rs.value_icon_size。
        rs.value_icon_size = 20;
        // 赋值：把右侧 false 的结果保存到左侧 rs.show_chevron。
        rs.show_chevron = false;
        // 调用 ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);，执行这个函数并处理它的返回值或状态变化。
        ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);
    }
    // 赋值：把右侧 static_cast<int16_t>(y + Theme::kListRowH) 的结果保存到左侧 y。
    y = static_cast<int16_t>(y + Theme::kListRowH);

    // 判断条件：al.enabled() 是否成立。
    if (al.enabled())
    {
        // 执行语句：char delay_buf[16]{};；它会更新一次变量、对象或程序状态。
        char delay_buf[16]{};
        // 访问对象成员：al.format_timeout(delay_buf, sizeof(delay_buf));；读取或修改这个对象里的成员状态。
        al.format_timeout(delay_buf, sizeof(delay_buf));

        // 声明并默认初始化对象：ui::widgets::RowStyle rs{}。
        ui::widgets::RowStyle rs{};
        // 赋值：把右侧 ui::strings::kSetAutolockDelay 的结果保存到左侧 rs.label。
        rs.label = ui::strings::kSetAutolockDelay;
        // 赋值：把右侧 delay_buf 的结果保存到左侧 rs.value。
        rs.value = delay_buf;
        // 赋值：把右侧 true 的结果保存到左侧 rs.show_chevron。
        rs.show_chevron = true;
        // 调用 ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);，执行这个函数并处理它的返回值或状态变化。
        ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);
    }
}

// 定义或声明函数 SettingsApp::handle_display：参数在括号里，返回类型写在函数名前面。
shell::InputResult SettingsApp::handle_display(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;
    // 判断条件：ev.type != EventType::Tap 是否成立。
    if (ev.type != EventType::Tap)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x = static_cast<int16_t>(ev.x);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y = static_cast<int16_t>(ev.y);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t row = tap_row_index(y);
    // 判断条件：row == 0xFF 是否成立。
    if (row == 0xFF)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 赋值：把右侧 data::SystemState::get_instance() 的结果保存到左侧 auto& sys。
    auto& sys = data::SystemState::get_instance();
    // 调用 AutoLock::get_instance()：返回自动锁屏单例。。
    auto& al = AutoLock::get_instance();

    // 判断条件：row == 0 是否成立。
    if (row == 0)
    {
        // 定义变量 uint8_t v 并初始化为 0。
        uint8_t v = 0;
        // 判断条件：slider_hit_pct(x, v) 是否成立。
        if (slider_hit_pct(x, v))
            // 调用 set_brightness(v)：把 0 到 100 的亮度限制在合法范围后交给背光驱动。墨水屏本身不依赖背光，背光是前光/照明系统，不能把亮度当成画面灰度。。
            sys.set_brightness(v);
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：row == 1 是否成立。
    if (row == 1)
    {
        // 访问对象成员：sys.set_night_mode(!sys.night_mode());；读取或修改这个对象里的成员状态。
        sys.set_night_mode(!sys.night_mode());
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：row == 2 是否成立。
    if (row == 2)
    {
        // 访问对象成员：al.set_enabled(!al.enabled());；读取或修改这个对象里的成员状态。
        al.set_enabled(!al.enabled());
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：row == 3 && al.enabled() 是否成立。
    if (row == 3 && al.enabled())
    {
        // 调用 cycle_timeout_preset()：循环到下一个时间预设并保存。显示设置页调用它改变时间。。
        al.cycle_timeout_preset();
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 返回 {} 给调用者，表示当前函数的执行结果。
    return {};
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::settings
