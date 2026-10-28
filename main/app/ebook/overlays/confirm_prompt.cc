/**
 * [初学者导读] 文件：main/app/ebook/overlays/confirm_prompt.cc
 * 作用：实现 overlays 模块中的 confirm prompt 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：覆盖层优先接收输入，打开和关闭会改变路由栈。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/confirm_prompt.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/refresh_intent.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/router.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/dialog.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::overlays {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::dialog::ConfirmHit;

// 固定参数 kDlgW = 160：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kDlgW = 160;
// 固定参数 kDlgH = 88：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kDlgH = 88;

// 定义或声明函数 dialog_rect：参数在括号里，返回类型写在函数名前面。
core::Rect dialog_rect()
{
    // 返回 ui::dialog::centered(kDlgW, kDlgH) 给调用者，表示当前函数的执行结果。
    return ui::dialog::centered(kDlgW, kDlgH);
}

// [初学者导读] repaint()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void repaint()
{
    // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
    (void)router::Router::instance().repaint(router::intent_partial_full());
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 ConfirmPrompt::instance：参数在括号里，返回类型写在函数名前面。
ConfirmPrompt& ConfirmPrompt::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static ConfirmPrompt s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 函数定义/声明：调用 ConfirmPrompt::show()：ensuretimer()：第一次调用创建一次性软件定时器，后续修改周期。。
void ConfirmPrompt::show(const char* message, ChoiceHandler handler, void* user)
{
    // 判断条件：message == nullptr || handler == nullptr 是否成立。
    if (message == nullptr || handler == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 函数调用：(void)std::strncpy(message_, message, kMsgMax);；执行该函数并处理它的返回值或副作用。
    (void)std::strncpy(message_, message, kMsgMax);
    // 赋值：把右侧 '\0' 的结果保存到左侧 message_[kMsgMax]。
    message_[kMsgMax] = '\0';
    // 赋值：把右侧 handler 的结果保存到左侧 handler_。
    handler_ = handler;
    // 赋值：把右侧 user 的结果保存到左侧 user_。
    user_ = user;
    // 赋值：把右侧 true 的结果保存到左侧 open_。
    open_ = true;
    // 调用 repaint()：这些函数只是用明确的参数调用统一导航逻辑，避免每个页面自己拼 Transition。。
    repaint();
}

// 函数定义/声明：调用 ConfirmPrompt::close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
void ConfirmPrompt::close()
{
    // 判断条件：!open_ 是否成立。
    if (!open_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 false 的结果保存到左侧 open_。
    open_ = false;
    // 赋值：把右侧 nullptr 的结果保存到左侧 handler_。
    handler_ = nullptr;
    // 赋值：把右侧 nullptr 的结果保存到左侧 user_。
    user_ = nullptr;
    // 赋值：把右侧 '\0' 的结果保存到左侧 message_[0]。
    message_[0] = '\0';
    // 调用 repaint()：这些函数只是用明确的参数调用统一导航逻辑，避免每个页面自己拼 Transition。。
    repaint();
}

// 定义或声明函数 ConfirmPrompt::paint：参数在括号里，返回类型写在函数名前面。
void ConfirmPrompt::paint(gfx::Canvas& c)
{
    // 判断条件：!open_ || message_[0] == '\0' 是否成立。
    if (!open_ || message_[0] == '\0')
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 访问对象成员：c.fill(core::Rect::full(), gfx::Ink::White);；读取或修改这个对象里的成员状态。
    c.fill(core::Rect::full(), gfx::Ink::White);
    // 访问对象成员：c.rect(core::Rect::full());；读取或修改这个对象里的成员状态。
    c.rect(core::Rect::full());
    // 调用 ui::dialog::paint_confirm(c, dialog_rect(), message_, false);，执行这个函数并处理它的返回值或状态变化。
    ui::dialog::paint_confirm(c, dialog_rect(), message_, false);
}

// 定义或声明函数 ConfirmPrompt::on_input：参数在括号里，返回类型写在函数名前面。
shell::InputResult ConfirmPrompt::on_input(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;
    // 判断条件：!open_ || ev.type != EventType::Tap 是否成立。
    if (!open_ || ev.type != EventType::Tap)
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x = static_cast<int16_t>(ev.x);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y = static_cast<int16_t>(ev.y);

    // 定义变量 ChoiceHandler handler 并初始化为 handler_。
    ChoiceHandler handler = handler_;
    // 赋值：把右侧 user_ 的结果保存到左侧 void* user。
    void* user = user_;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const ConfirmHit hit = ui::dialog::hit_confirm(dialog_rect(), x, y, false);
    // 判断条件：hit == ConfirmHit::Yes 是否成立。
    if (hit == ConfirmHit::Yes)
    {
        // 调用 close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
        close();
        // 判断条件：handler 是否成立。
        if (handler)
            // 调用 handler(true, user);，执行这个函数并处理它的返回值或状态变化。
            handler(true, user);
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：hit == ConfirmHit::No 是否成立。
    if (hit == ConfirmHit::No)
    {
        // 调用 close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
        close();
        // 判断条件：handler 是否成立。
        if (handler)
            // 调用 handler(false, user);，执行这个函数并处理它的返回值或状态变化。
            handler(false, user);
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 返回 {true} 给调用者，表示当前函数的执行结果。
    return {true};
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::overlays
