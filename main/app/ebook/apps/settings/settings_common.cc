/**
 * [初学者导读] 文件：main/app/ebook/apps/settings/settings_common.cc
 * 作用：提供设置页公共的标题、复制、滑条和列表辅助函数。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_internal.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/font.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "network/wifi/wifi.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::settings::detail {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义只读对象或变量：初始化后不能在当前位置修改。
const char* page_title(SettingsPage p)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (p)
    {
        // 分支匹配：case SettingsPage::Menu 成立时执行下面的代码。
        case SettingsPage::Menu:
            // 返回 ui::strings::kAppSettings 给调用者，表示当前函数的执行结果。
            return ui::strings::kAppSettings;
        // 分支匹配：case SettingsPage::Wifi 成立时执行下面的代码。
        case SettingsPage::Wifi:
            // 返回 ui::strings::kSetWifi 给调用者，表示当前函数的执行结果。
            return ui::strings::kSetWifi;
        // 分支匹配：case SettingsPage::Bluetooth 成立时执行下面的代码。
        case SettingsPage::Bluetooth:
            // 返回 ui::strings::kSetBt 给调用者，表示当前函数的执行结果。
            return ui::strings::kSetBt;
        // 分支匹配：case SettingsPage::Hotspot 成立时执行下面的代码。
        case SettingsPage::Hotspot:
            // 返回 ui::strings::kSetHotspot 给调用者，表示当前函数的执行结果。
            return ui::strings::kSetHotspot;
        // 分支匹配：case SettingsPage::Display 成立时执行下面的代码。
        case SettingsPage::Display:
            // 返回 ui::strings::kSetDisplay 给调用者，表示当前函数的执行结果。
            return ui::strings::kSetDisplay;
        // 分支匹配：case SettingsPage::Keys 成立时执行下面的代码。
        case SettingsPage::Keys:
            // 返回 ui::strings::kSetKeys 给调用者，表示当前函数的执行结果。
            return ui::strings::kSetKeys;
        // 分支匹配：case SettingsPage::KeysEdit 成立时执行下面的代码。
        case SettingsPage::KeysEdit:
            // 返回 ui::strings::kSetKeys 给调用者，表示当前函数的执行结果。
            return ui::strings::kSetKeys;
        // 分支匹配：case SettingsPage::Battery 成立时执行下面的代码。
        case SettingsPage::Battery:
            // 返回 ui::strings::kSetBattery 给调用者，表示当前函数的执行结果。
            return ui::strings::kSetBattery;
        // 分支匹配：case SettingsPage::Sound 成立时执行下面的代码。
        case SettingsPage::Sound:
            // 返回 ui::strings::kSetSound 给调用者，表示当前函数的执行结果。
            return ui::strings::kSetSound;
        // 分支匹配：case SettingsPage::Time 成立时执行下面的代码。
        case SettingsPage::Time:
            // 返回 ui::strings::kSetTime 给调用者，表示当前函数的执行结果。
            return ui::strings::kSetTime;
        // 分支匹配：case SettingsPage::Storage 成立时执行下面的代码。
        case SettingsPage::Storage:
            // 返回 ui::strings::kSetStorage 给调用者，表示当前函数的执行结果。
            return ui::strings::kSetStorage;
        // 分支匹配：case SettingsPage::Security 成立时执行下面的代码。
        case SettingsPage::Security:
            // 返回 ui::strings::kSetSecurity 给调用者，表示当前函数的执行结果。
            return ui::strings::kSetSecurity;
        // 分支匹配：case SettingsPage::About 成立时执行下面的代码。
        case SettingsPage::About:
            // 返回 ui::strings::kSetAbout 给调用者，表示当前函数的执行结果。
            return ui::strings::kSetAbout;
    }
    // 返回 ui::strings::kAppSettings 给调用者，表示当前函数的执行结果。
    return ui::strings::kAppSettings;
}

// 函数定义/声明：调用 copy_str()：安全的有限长度字符串复制。它会保证不越界并补 \0。所有用户名、SSID、密码、设备名和标题复制都应该优先使用它。。
void copy_str(char* dst, size_t cap, const char* src)
{
    // 判断条件：dst == nullptr || cap == 0 是否成立。
    if (dst == nullptr || cap == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：src == nullptr 是否成立。
    if (src == nullptr)
    {
        // 赋值：把右侧 '\0' 的结果保存到左侧 dst[0]。
        dst[0] = '\0';
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 调用 std::strncpy(dst, src, cap - 1);，执行这个函数并处理它的返回值或状态变化。
    std::strncpy(dst, src, cap - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 dst[cap - 1]。
    dst[cap - 1] = '\0';
}

// [初学者导读] ensure_wifi()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
bool ensure_wifi()
{
    // 判断条件：::app::network::wifi::WiFiMgr::get_instance().init() 是否成立。
    if (::app::network::wifi::WiFiMgr::get_instance().init())
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    // 调用 EBOOK_LOGW(TAG, "wifi init failed");，执行这个函数并处理它的返回值或状态变化。
    EBOOK_LOGW(TAG, "wifi init failed");
    // 返回 false 给调用者，表示当前函数的执行结果。
    return false;
}

// 定义或声明函数 draw_slider_row：参数在括号里，返回类型写在函数名前面。
void draw_slider_row(gfx::Canvas& c, int16_t y, const char* label, uint8_t pct)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect row{0, y, Theme::kScreenW, Theme::kListRowH};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t txt_y = static_cast<int16_t>(row.y + (row.h - Theme::kFontBody) / 2);
    // 访问对象成员：c.text(Theme::kPadLg, txt_y, label, Theme::kFontBody);；读取或修改这个对象里的成员状态。
    c.text(Theme::kPadLg, txt_y, label, Theme::kFontBody);

    // 执行语句：char pct_buf[8];；它会更新一次变量、对象或程序状态。
    char pct_buf[8];
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    (void)std::snprintf(pct_buf, sizeof(pct_buf), "%u%%", static_cast<unsigned>(pct));
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t pct_w = gfx::Font::get_instance().measure(pct_buf, Theme::kFontBody);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t bar_x = static_cast<int16_t>(Theme::kPadLg + 60);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t bar_w =
        // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kScreenW - bar_x - Theme::kPadLg - pct_w - 4);。
        static_cast<uint16_t>(Theme::kScreenW - bar_x - Theme::kPadLg - pct_w - 4);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t bar_y = static_cast<int16_t>(row.y + (row.h - 8) / 2);
    // 调用 ui::widgets::progress_bar(c, core::Rect{bar_x, bar_y, bar_w, 8}, pct);，执行这个函数并处理它的返回值或状态变化。
    ui::widgets::progress_bar(c, core::Rect{bar_x, bar_y, bar_w, 8}, pct);

    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    c.text(static_cast<int16_t>(bar_x + bar_w + 4), txt_y, pct_buf, Theme::kFontBody);
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    c.hline(Theme::kPadLg, static_cast<int16_t>(row.y + row.h - 1),
            // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kScreenW - Theme::kPadLg * 2));。
            static_cast<uint16_t>(Theme::kScreenW - Theme::kPadLg * 2));
}

// 函数定义/声明：调用 slider_hit_pct()：把触摸横坐标转换为 0 到 100 的百分比。点击绘制和命中判断必须使用同一个滑条左右边界。。
bool slider_hit_pct(int16_t x, uint8_t& out_pct)
{
    // 固定参数 bar_x = Theme::kPadLg + 60：由当前模块定义，用于控制布局、时序或硬件行为。
    constexpr int16_t bar_x = Theme::kPadLg + 60;
    // 固定参数 bar_r = Theme::kScreenW - Theme::kPadLg - 30：由当前模块定义，用于控制布局、时序或硬件行为。
    constexpr int16_t bar_r = Theme::kScreenW - Theme::kPadLg - 30;
    // 判断条件：x < bar_x || x >= bar_r 是否成立。
    if (x < bar_x || x >= bar_r)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    out_pct = static_cast<uint8_t>((static_cast<uint32_t>(x - bar_x) * 100U) /
                                   // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
                                   static_cast<uint32_t>(bar_r - bar_x));
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数调用：bool scroll_rows(uint8_t& scroll_row, uint8_t total, uint8_t visible,；执行该函数并处理它的返回值或副作用。
bool scroll_rows(uint8_t& scroll_row, uint8_t total, uint8_t visible,
                 // 继续上一行的表达式或参数列表：::app::ebook::input::EventType type)。
                 ::app::ebook::input::EventType type)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t max_off = (total > visible) ? static_cast<uint8_t>(total - visible) : 0;
    // 判断条件：type == ::app::ebook::input::EventType::SwipeUp && scroll_row < max_off 是否成立。
    if (type == ::app::ebook::input::EventType::SwipeUp && scroll_row < max_off)
    {
        // 执行语句：++scroll_row;；它会更新一次变量、对象或程序状态。
        ++scroll_row;
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 判断条件：type == ::app::ebook::input::EventType::SwipeDown && scroll_row > 0 是否成立。
    if (type == ::app::ebook::input::EventType::SwipeDown && scroll_row > 0)
    {
        // 执行语句：--scroll_row;；它会更新一次变量、对象或程序状态。
        --scroll_row;
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 返回 false 给调用者，表示当前函数的执行结果。
    return false;
}

// 函数定义/声明：调用 tap_row_index()：根据纵坐标计算点击到了第几行。它要与页面行间距和顶部偏移保持一致。。
uint8_t tap_row_index(int16_t y)
{
    // 判断条件：y < Theme::kListStartY 是否成立。
    if (y < Theme::kListStartY)
        // 返回 0xFF 给调用者，表示当前函数的执行结果。
        return 0xFF;
    // 返回 static_cast<uint8_t>((y - Theme::kListStartY) / Theme::kListRowH) 给调用者，表示当前函数的执行结果。
    return static_cast<uint8_t>((y - Theme::kListStartY) / Theme::kListRowH);
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::settings::detail
