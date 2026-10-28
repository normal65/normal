/**
 * [初学者导读] 文件：main/app/ebook/apps/settings/settings_security.cc
 * 作用：实现锁屏 PIN 设置、修改、验证和清除流程。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_app.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/lock_password.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_internal.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/keyboard.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/toast.hpp"
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
using detail::copy_str;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using detail::tap_row_index;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;

// 定义或声明函数 paint_sec_row：参数在括号里，返回类型写在函数名前面。
void paint_sec_row(gfx::Canvas& c, int16_t y, const char* label, const char* value, bool chevron)
{
    // 声明并默认初始化对象：ui::widgets::RowStyle rs{}。
    ui::widgets::RowStyle rs{};
    // 赋值：把右侧 label 的结果保存到左侧 rs.label。
    rs.label = label;
    // 赋值：把右侧 value 的结果保存到左侧 rs.value。
    rs.value = value;
    // 赋值：把右侧 chevron 的结果保存到左侧 rs.show_chevron。
    rs.show_chevron = chevron;
    // 调用 ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);，执行这个函数并处理它的返回值或状态变化。
    ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 SettingsApp::paint_security：参数在括号里，返回类型写在函数名前面。
void SettingsApp::paint_security(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto& lp = LockPassword::get_instance();
    // 定义变量 int16_t y 并初始化为 Theme::kListStartY。
    int16_t y = Theme::kListStartY;

    // 判断条件：!lp.enabled() 是否成立。
    if (!lp.enabled())
    {
        // 调用 paint_sec_row(c, y, ui::strings::kSetSecLock, ui::strings::kSetUnset, true);，执行这个函数并处理它的返回值或状态变化。
        paint_sec_row(c, y, ui::strings::kSetSecLock, ui::strings::kSetUnset, true);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 调用 paint_sec_row(c, y, ui::strings::kSetSecLock, ui::strings::kSetPinSet, false);，执行这个函数并处理它的返回值或状态变化。
    paint_sec_row(c, y, ui::strings::kSetSecLock, ui::strings::kSetPinSet, false);
    // 赋值：把右侧 static_cast<int16_t>(y + Theme::kListRowH) 的结果保存到左侧 y。
    y = static_cast<int16_t>(y + Theme::kListRowH);

    // 调用 paint_sec_row(c, y, ui::strings::kSetSecChange, nullptr, true);，执行这个函数并处理它的返回值或状态变化。
    paint_sec_row(c, y, ui::strings::kSetSecChange, nullptr, true);
    // 赋值：把右侧 static_cast<int16_t>(y + Theme::kListRowH) 的结果保存到左侧 y。
    y = static_cast<int16_t>(y + Theme::kListRowH);

    // 调用 paint_sec_row(c, y, ui::strings::kSetSecClose, nullptr, true);，执行这个函数并处理它的返回值或状态变化。
    paint_sec_row(c, y, ui::strings::kSetSecClose, nullptr, true);
}

// 函数定义/声明：调用 SettingsApp::open_pin_keyboard()：打开数字键盘。标题说明当前是设置、确认还是验证。。
void SettingsApp::open_pin_keyboard(const char* title)
{
    // 声明并默认初始化对象：overlays::KeyboardConfig kc{}。
    overlays::KeyboardConfig kc{};
    // 赋值：把右侧 overlays::KeyboardLayer::Numbers 的结果保存到左侧 kc.default_layer。
    kc.default_layer = overlays::KeyboardLayer::Numbers;
    // 赋值：把右侧 LockPassword::kPinLen 的结果保存到左侧 kc.max_len。
    kc.max_len = LockPassword::kPinLen;
    // 赋值：把右侧 title 的结果保存到左侧 kc.title。
    kc.title = title;
    // 调用 overlays::Keyboard::instance().open(kc, on_sec_pin_done, this);，执行这个函数并处理它的返回值或状态变化。
    overlays::Keyboard::instance().open(kc, on_sec_pin_done, this);
}

// 函数定义/声明：调用 SettingsApp::begin_set_pin_flow()：初始化流水线状态。requireold 为真时先验证旧 PIN，否则直接输入新 PIN。。
void SettingsApp::begin_set_pin_flow(bool require_old)
{
    // 赋值：把右侧 '\0' 的结果保存到左侧 sec_pending_pin_[0]。
    sec_pending_pin_[0] = '\0';
    // 判断条件：require_old 是否成立。
    if (require_old)
    {
        // 赋值：把右侧 SecPinPurpose::VerifyForChange 的结果保存到左侧 sec_pin_purpose_。
        sec_pin_purpose_ = SecPinPurpose::VerifyForChange;
        // 调用 open_pin_keyboard(ui::strings::kSetSecEnterOld);，执行这个函数并处理它的返回值或状态变化。
        open_pin_keyboard(ui::strings::kSetSecEnterOld);
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 赋值：把右侧 SecPinPurpose::SetNew 的结果保存到左侧 sec_pin_purpose_。
        sec_pin_purpose_ = SecPinPurpose::SetNew;
        // 调用 open_pin_keyboard(ui::strings::kSetSecEnterNew);，执行这个函数并处理它的返回值或状态变化。
        open_pin_keyboard(ui::strings::kSetSecEnterNew);
    }
}

// 定义或声明函数 SettingsApp::handle_security：参数在括号里，返回类型写在函数名前面。
shell::InputResult SettingsApp::handle_security(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;
    // 判断条件：ev.type != EventType::Tap 是否成立。
    if (ev.type != EventType::Tap)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t row = tap_row_index(static_cast<int16_t>(ev.y));
    // 判断条件：row == 0xFF 是否成立。
    if (row == 0xFF)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto& lp = LockPassword::get_instance();

    // 判断条件：!lp.enabled() 是否成立。
    if (!lp.enabled())
    {
        // 判断条件：row == 0 是否成立。
        if (row == 0)
        {
            // 调用 begin_set_pin_flow(false)：初始化流水线状态。requireold 为真时先验证旧 PIN，否则直接输入新 PIN。。
            begin_set_pin_flow(false);
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        }
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};
    }

    // 多分支判断：根据括号里的值选择不同 case。
    switch (row)
    {
        // 分支匹配：case 0 成立时执行下面的代码。
        case 0:
            // 返回 {} 给调用者，表示当前函数的执行结果。
            return {};
        // 分支匹配：case 1 成立时执行下面的代码。
        case 1:
            // 调用 begin_set_pin_flow(true)：初始化流水线状态。requireold 为真时先验证旧 PIN，否则直接输入新 PIN。。
            begin_set_pin_flow(true);
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        // 分支匹配：case 2 成立时执行下面的代码。
        case 2:
            // 赋值：把右侧 SecPinPurpose::VerifyForClear 的结果保存到左侧 sec_pin_purpose_。
            sec_pin_purpose_ = SecPinPurpose::VerifyForClear;
            // 调用 open_pin_keyboard(ui::strings::kSetSecEnterOld);，执行这个函数并处理它的返回值或状态变化。
            open_pin_keyboard(ui::strings::kSetSecEnterOld);
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 返回 {} 给调用者，表示当前函数的执行结果。
            return {};
    }
}

// 函数定义/声明：调用 SettingsApp::on_sec_pin_done()：键盘静态回调先把 user 转回 SettingsApp，再由 impl 根据 secpinpurpose 执行当前步骤。这样可以避免静态键盘回调直接乱改页面状态。。
void SettingsApp::on_sec_pin_done(const char* text, void* user)
{
    // 赋值：把右侧 static_cast<SettingsApp*>(user) 的结果保存到左侧 auto* self。
    auto* self = static_cast<SettingsApp*>(user);
    // 判断条件：self != nullptr 是否成立。
    if (self != nullptr)
        // 调用 on_sec_pin_done_impl(text)：键盘静态回调先把 user 转回 SettingsApp，再由 impl 根据 secpinpurpose 执行当前步骤。这样可以避免静态键盘回调直接乱改页面状态。。
        self->on_sec_pin_done_impl(text);
}

// 函数定义/声明：调用 SettingsApp::on_sec_pin_done_impl()：键盘静态回调先把 user 转回 SettingsApp，再由 impl 根据 secpinpurpose 执行当前步骤。这样可以避免静态键盘回调直接乱改页面状态。。
void SettingsApp::on_sec_pin_done_impl(const char* text)
{
    // 赋值：把右侧 overlays::Toast::instance() 的结果保存到左侧 auto& toast。
    auto& toast = overlays::Toast::instance();
    // 赋值：把右侧 LockPassword::get_instance() 的结果保存到左侧 auto& lp。
    auto& lp = LockPassword::get_instance();

    // 判断条件：!LockPassword::valid_pin(text) 是否成立。
    if (!LockPassword::valid_pin(text))
    {
        // 访问对象成员：toast.show(ui::strings::kSetSecPinInvalid, 2000);；读取或修改这个对象里的成员状态。
        toast.show(ui::strings::kSetSecPinInvalid, 2000);
        // 赋值：把右侧 SecPinPurpose::None 的结果保存到左侧 sec_pin_purpose_。
        sec_pin_purpose_ = SecPinPurpose::None;
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 多分支判断：根据括号里的值选择不同 case。
    switch (sec_pin_purpose_)
    {
        // 分支匹配：case SecPinPurpose::SetNew 成立时执行下面的代码。
        case SecPinPurpose::SetNew:
            // 调用 copy_str(sec_pending_pin_, sizeof(sec_pending_pin_), text);，执行这个函数并处理它的返回值或状态变化。
            copy_str(sec_pending_pin_, sizeof(sec_pending_pin_), text);
            // 赋值：把右侧 SecPinPurpose::ConfirmNew 的结果保存到左侧 sec_pin_purpose_。
            sec_pin_purpose_ = SecPinPurpose::ConfirmNew;
            // 调用 open_pin_keyboard(ui::strings::kSetSecConfirm);，执行这个函数并处理它的返回值或状态变化。
            open_pin_keyboard(ui::strings::kSetSecConfirm);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 分支匹配：case SecPinPurpose::ConfirmNew 成立时执行下面的代码。
        case SecPinPurpose::ConfirmNew:
            // 判断条件：std::strcmp(text, sec_pending_pin_) != 0 是否成立。
            if (std::strcmp(text, sec_pending_pin_) != 0)
            {
                // 访问对象成员：toast.show(ui::strings::kSetSecMismatch, 2000);；读取或修改这个对象里的成员状态。
                toast.show(ui::strings::kSetSecMismatch, 2000);
                // 赋值：把右侧 SecPinPurpose::SetNew 的结果保存到左侧 sec_pin_purpose_。
                sec_pin_purpose_ = SecPinPurpose::SetNew;
                // 赋值：把右侧 '\0' 的结果保存到左侧 sec_pending_pin_[0]。
                sec_pending_pin_[0] = '\0';
                // 调用 open_pin_keyboard(ui::strings::kSetSecEnterNew);，执行这个函数并处理它的返回值或状态变化。
                open_pin_keyboard(ui::strings::kSetSecEnterNew);
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            }
            {
                // 只读约束：这个值或对象在当前位置不能被修改。
                const bool changing = lp.enabled();
                // 判断条件：lp.set_new(text) 是否成立。
                if (lp.set_new(text))
                    // 函数调用：toast.show(changing ? ui::strings::kSetSecChanged : ui::strings::kSetSecSaved,；执行该函数并处理它的返回值或副作用。
                    toast.show(changing ? ui::strings::kSetSecChanged : ui::strings::kSetSecSaved,
                               // 继续上一行的表达式或参数列表：2000);。
                               2000);
                // 备用分支：前面的条件不成立时执行这里。
                else
                    // 访问对象成员：toast.show(ui::strings::kSetSecSaveFail, 2000);；读取或修改这个对象里的成员状态。
                    toast.show(ui::strings::kSetSecSaveFail, 2000);
            }
            // 赋值：把右侧 SecPinPurpose::None 的结果保存到左侧 sec_pin_purpose_。
            sec_pin_purpose_ = SecPinPurpose::None;
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            request_repaint();
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 分支匹配：case SecPinPurpose::VerifyForChange 成立时执行下面的代码。
        case SecPinPurpose::VerifyForChange:
            // 判断条件：!lp.verify(text) 是否成立。
            if (!lp.verify(text))
            {
                // 访问对象成员：toast.show(ui::strings::kSetSecWrong, 2000);；读取或修改这个对象里的成员状态。
                toast.show(ui::strings::kSetSecWrong, 2000);
                // 赋值：把右侧 SecPinPurpose::None 的结果保存到左侧 sec_pin_purpose_。
                sec_pin_purpose_ = SecPinPurpose::None;
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            }
            // 调用 begin_set_pin_flow(false)：初始化流水线状态。requireold 为真时先验证旧 PIN，否则直接输入新 PIN。。
            begin_set_pin_flow(false);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 分支匹配：case SecPinPurpose::VerifyForClear 成立时执行下面的代码。
        case SecPinPurpose::VerifyForClear:
            // 判断条件：!lp.verify(text) 是否成立。
            if (!lp.verify(text))
            {
                // 访问对象成员：toast.show(ui::strings::kSetSecWrong, 2000);；读取或修改这个对象里的成员状态。
                toast.show(ui::strings::kSetSecWrong, 2000);
                // 赋值：把右侧 SecPinPurpose::None 的结果保存到左侧 sec_pin_purpose_。
                sec_pin_purpose_ = SecPinPurpose::None;
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            }
            // 调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
            lp.clear();
            // 访问对象成员：toast.show(ui::strings::kSetSecCleared, 2000);；读取或修改这个对象里的成员状态。
            toast.show(ui::strings::kSetSecCleared, 2000);
            // 赋值：把右侧 SecPinPurpose::None 的结果保存到左侧 sec_pin_purpose_。
            sec_pin_purpose_ = SecPinPurpose::None;
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            request_repaint();
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 分支匹配：case SecPinPurpose::None 成立时执行下面的代码。
        case SecPinPurpose::None:
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 赋值：把右侧 SecPinPurpose::None 的结果保存到左侧 sec_pin_purpose_。
            sec_pin_purpose_ = SecPinPurpose::None;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::settings
