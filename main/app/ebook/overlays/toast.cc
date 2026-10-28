/**
 * [初学者导读] 文件：main/app/ebook/overlays/toast.cc
 * 作用：实现 overlays 模块中的 toast 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：覆盖层优先接收输入，打开和关闭会改变路由栈。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/toast.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_bus.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::overlays {

// 定义或声明函数 Toast::instance：参数在括号里，返回类型写在函数名前面。
Toast& Toast::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static Toast s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 函数定义/声明：调用 Toast::timer_cb()：定时器不直接碰 UI。
void Toast::timer_cb(TimerHandle_t t)
{
    // 赋值：把右侧 static_cast<Toast*>(pvTimerGetTimerID(t)) 的结果保存到左侧 auto* self。
    auto* self = static_cast<Toast*>(pvTimerGetTimerID(t));
    // 执行语句：(void)self;；它会更新一次变量、对象或程序状态。
    (void)self;
    // 访问对象成员：(void)ui::UiBus::get_instance().post_system_hint(ui::SystemHintKind::ToastExpire, 0);；读取或修改这个对象里的成员状态。
    (void)ui::UiBus::get_instance().post_system_hint(ui::SystemHintKind::ToastExpire, 0);
}

// 函数定义/声明：调用 Toast::ensure_timer()：ensuretimer()：第一次调用创建一次性软件定时器，后续修改周期。。
void Toast::ensure_timer(uint32_t duration_ms)
{
    // 判断条件：duration_ms == 0 是否成立。
    if (duration_ms == 0)
        // 赋值：把右侧 1 的结果保存到左侧 duration_ms。
        duration_ms = 1;
    // 判断条件：timer_ == nullptr 是否成立。
    if (timer_ == nullptr)
    {
        // 赋值：把右侧 xTimerCreate("toast", pdMS_TO_TICKS(duration_ms), pdFALSE, this, &Toast::timer_cb) 的结果保存到左侧 timer_。
        timer_ = xTimerCreate("toast", pdMS_TO_TICKS(duration_ms), pdFALSE, this, &Toast::timer_cb);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 函数调用：(void)xTimerChangePeriod(timer_, pdMS_TO_TICKS(duration_ms), 0);；执行该函数并处理它的返回值或副作用。
    (void)xTimerChangePeriod(timer_, pdMS_TO_TICKS(duration_ms), 0);
}

// 函数定义/声明：调用 Toast::show()：ensuretimer()：第一次调用创建一次性软件定时器，后续修改周期。。
void Toast::show(const char* text, uint16_t duration_ms)
{
    // 判断条件：text == nullptr 是否成立。
    if (text == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：duration_ms == 0 是否成立。
    if (duration_ms == 0)
        // 赋值：把右侧 1500 的结果保存到左侧 duration_ms。
        duration_ms = 1500;

    // 函数调用：(void)std::strncpy(text_, text, kMaxLen);；执行该函数并处理它的返回值或副作用。
    (void)std::strncpy(text_, text, kMaxLen);
    // 赋值：把右侧 '\0' 的结果保存到左侧 text_[kMaxLen]。
    text_[kMaxLen] = '\0';
    // 赋值：把右侧 true 的结果保存到左侧 visible_。
    visible_ = true;

    // 调用 ensure_timer(duration_ms)：ensuretimer()：第一次调用创建一次性软件定时器，后续修改周期。。
    ensure_timer(duration_ms);
    // 函数调用：(void)xTimerReset(timer_, 0);；执行该函数并处理它的返回值或副作用。
    (void)xTimerReset(timer_, 0);
}

// 函数定义/声明：调用 Toast::hide()：ensuretimer()：第一次调用创建一次性软件定时器，后续修改周期。。
void Toast::hide()
{
    // 判断条件：!visible_ 是否成立。
    if (!visible_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 false 的结果保存到左侧 visible_。
    visible_ = false;
    // 赋值：把右侧 '\0' 的结果保存到左侧 text_[0]。
    text_[0] = '\0';
    // 判断条件：timer_ != nullptr 是否成立。
    if (timer_ != nullptr)
        // 函数调用：(void)xTimerStop(timer_, 0);；执行该函数并处理它的返回值或副作用。
        (void)xTimerStop(timer_, 0);
}

// 函数定义/声明：调用 Toast::paint()：在屏幕底部画黑色矩形和白色小字。它不负责刷新；调用者通常需要自己提交重绘。。
void Toast::paint(gfx::Canvas& c)
{
    // 判断条件：!visible_ || text_[0] == '\0' 是否成立。
    if (!visible_ || text_[0] == '\0')
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect box{ui::Theme::kPad, static_cast<int16_t>(ui::Theme::kScreenH - 36),
                         // 继续上一行的表达式或参数列表：static_cast<uint16_t>(ui::Theme::kScreenW - ui::Theme::kPad * 2), 28};。
                         static_cast<uint16_t>(ui::Theme::kScreenW - ui::Theme::kPad * 2), 28};
    // 访问对象成员：c.fill(box, gfx::Ink::Black);；读取或修改这个对象里的成员状态。
    c.fill(box, gfx::Ink::Black);
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    c.text(static_cast<int16_t>(box.x + 4), static_cast<int16_t>(box.y + 6), text_,
           // 继续上一行的表达式或参数列表：ui::Theme::kFontSmall, gfx::FontFace::Text, gfx::Ink::White);。
           ui::Theme::kFontSmall, gfx::FontFace::Text, gfx::Ink::White);
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::overlays
