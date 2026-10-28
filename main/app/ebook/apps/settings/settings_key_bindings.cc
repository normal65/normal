/**
 * [初学者导读] 文件：main/app/ebook/apps/settings/settings_key_bindings.cc
 * 作用：实现拨码按键映射设置页。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_app.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_internal.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/key_bindings.hpp"
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
using ::app::ebook::input::InputContext;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ::app::ebook::input::KeyBindings;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ::app::ebook::input::PhysicalKey;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using detail::tap_row_index;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;

// 固定参数 kKeysMenuRows = 4：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kKeysMenuRows = 4;

// 定义或声明函数 ctx_from_menu_row：参数在括号里，返回类型写在函数名前面。
InputContext ctx_from_menu_row(uint8_t row)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (row)
    {
        // 分支匹配：case 0 成立时执行下面的代码。
        case 0:
            // 返回 InputContext::Reader 给调用者，表示当前函数的执行结果。
            return InputContext::Reader;
        // 分支匹配：case 1 成立时执行下面的代码。
        case 1:
            // 返回 InputContext::List 给调用者，表示当前函数的执行结果。
            return InputContext::List;
        // 分支匹配：case 2 成立时执行下面的代码。
        case 2:
            // 返回 InputContext::Global 给调用者，表示当前函数的执行结果。
            return InputContext::Global;
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 返回 InputContext::Reader 给调用者，表示当前函数的执行结果。
            return InputContext::Reader;
    }
}

// 静态查表数组 kSlotKeys：运行时复用固定数据，避免每次重新计算。
constexpr PhysicalKey kSlotKeys[3] = {
    // 执行这条代码：PhysicalKey::Up,。它会更新变量、对象状态或调用链。
    PhysicalKey::Up,
    // 继续上一行的表达式或参数列表：PhysicalKey::Mid,。
    PhysicalKey::Mid,
    // 继续上一行的表达式或参数列表：PhysicalKey::Down,。
    PhysicalKey::Down,
};

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 函数定义/声明：调用 SettingsApp::enter_keys_edit()：进入具体按键编辑页，并记录正在编辑哪个上下文。。
void SettingsApp::enter_keys_edit(InputContext ctx)
{
    // 赋值：把右侧 ctx 的结果保存到左侧 keys_edit_ctx_。
    keys_edit_ctx_ = ctx;
    // 赋值：把右侧 SettingsPage::KeysEdit 的结果保存到左侧 page_。
    page_ = SettingsPage::KeysEdit;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 定义或声明函数 SettingsApp::paint_keys：参数在括号里，返回类型写在函数名前面。
void SettingsApp::paint_keys(gfx::Canvas& c)
{
    // 定义变量 int16_t y 并初始化为 Theme::kListStartY。
    int16_t y = Theme::kListStartY;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < 3; ++i)
    {
        // 声明并默认初始化对象：ui::widgets::RowStyle rs{}。
        ui::widgets::RowStyle rs{};
        // 赋值：把右侧 KeyBindings::context_label(ctx_from_menu_row(i)) 的结果保存到左侧 rs.label。
        rs.label = KeyBindings::context_label(ctx_from_menu_row(i));
        // 赋值：把右侧 true 的结果保存到左侧 rs.show_chevron。
        rs.show_chevron = true;
        // 调用 ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);，执行这个函数并处理它的返回值或状态变化。
        ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);
        // 赋值：把右侧 static_cast<int16_t>(y + Theme::kListRowH) 的结果保存到左侧 y。
        y = static_cast<int16_t>(y + Theme::kListRowH);
    }

    {
        // 声明并默认初始化对象：ui::widgets::RowStyle rs{}。
        ui::widgets::RowStyle rs{};
        // 赋值：把右侧 ui::strings::kKeyRestore 的结果保存到左侧 rs.label。
        rs.label = ui::strings::kKeyRestore;
        // 赋值：把右侧 false 的结果保存到左侧 rs.show_chevron。
        rs.show_chevron = false;
        // 调用 ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);，执行这个函数并处理它的返回值或状态变化。
        ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);
    }
}

// 定义或声明函数 SettingsApp::paint_keys_edit：参数在括号里，返回类型写在函数名前面。
void SettingsApp::paint_keys_edit(gfx::Canvas& c)
{
    // 赋值：把右侧 KeyBindings::instance() 的结果保存到左侧 auto& kb。
    auto& kb = KeyBindings::instance();
    // 定义变量 int16_t y 并初始化为 Theme::kListStartY。
    int16_t y = Theme::kListStartY;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < 3; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const PhysicalKey slot = kSlotKeys[i];
        // 声明并默认初始化对象：ui::widgets::RowStyle rs{}。
        ui::widgets::RowStyle rs{};
        // 调用 KeyBindings::key_label(slot)：把枚举转换成界面文字。新增动作时这三类标签必须一起更新，否则页面会显示空文字或错误名称。。
        rs.label = KeyBindings::key_label(slot);
        // 调用 get(keys_edit_ctx_,slot)：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
        rs.value = KeyBindings::action_label(kb.get(keys_edit_ctx_, slot));
        // 赋值：把右侧 false 的结果保存到左侧 rs.show_chevron。
        rs.show_chevron = false;
        // 调用 ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);，执行这个函数并处理它的返回值或状态变化。
        ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);
        // 赋值：把右侧 static_cast<int16_t>(y + Theme::kListRowH) 的结果保存到左侧 y。
        y = static_cast<int16_t>(y + Theme::kListRowH);
    }
}

// 定义或声明函数 SettingsApp::handle_keys：参数在括号里，返回类型写在函数名前面。
shell::InputResult SettingsApp::handle_keys(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;
    // 判断条件：ev.type != EventType::Tap 是否成立。
    if (ev.type != EventType::Tap)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t row = tap_row_index(static_cast<int16_t>(ev.y));
    // 判断条件：row >= kKeysMenuRows 是否成立。
    if (row >= kKeysMenuRows)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 判断条件：row < 3 是否成立。
    if (row < 3)
    {
        // 调用 enter_keys_edit(ctx_from_menu_row(row));，执行这个函数并处理它的返回值或状态变化。
        enter_keys_edit(ctx_from_menu_row(row));
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 调用 restore_defaults()：恢复默认映射并保存。。
    KeyBindings::instance().restore_defaults();
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
    // 返回 {true} 给调用者，表示当前函数的执行结果。
    return {true};
}

// 定义或声明函数 SettingsApp::handle_keys_edit：参数在括号里，返回类型写在函数名前面。
shell::InputResult SettingsApp::handle_keys_edit(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;
    // 判断条件：ev.type != EventType::Tap 是否成立。
    if (ev.type != EventType::Tap)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t row = tap_row_index(static_cast<int16_t>(ev.y));
    // 判断条件：row >= 3 是否成立。
    if (row >= 3)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 调用 KeyBindings::instance().cycle(keys_edit_ctx_, kSlotKeys[row]);，执行这个函数并处理它的返回值或状态变化。
    KeyBindings::instance().cycle(keys_edit_ctx_, kSlotKeys[row]);
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
    // 返回 {true} 给调用者，表示当前函数的执行结果。
    return {true};
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::settings
