/**
 * [初学者导读] 文件：main/app/ebook/display/display_port.cc
 * 作用：把上层刷新意图转换成墨水屏的 Full、Fast、Partial 操作。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：墨水屏刷新是异步且耗时的，要区分 Full、Fast、Partial。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "display/display_port.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "DisplayPort";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::display {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using Present = ::app::bsp::driver::gdey027t91::Present;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using Window = ::app::bsp::driver::gdey027t91::Window;

// 定义或声明函数 make_window：参数在括号里，返回类型写在函数名前面。
Window make_window(const core::Rect& r)
{
    // 声明并默认初始化对象：Window w{}。
    Window w{};
    // 赋值：把右侧 static_cast<uint16_t>(r.x) 的结果保存到左侧 w.x。
    w.x = static_cast<uint16_t>(r.x);
    // 赋值：把右侧 static_cast<uint16_t>(r.y) 的结果保存到左侧 w.y。
    w.y = static_cast<uint16_t>(r.y);
    // 赋值：把右侧 r.w 的结果保存到左侧 w.w。
    w.w = r.w;
    // 赋值：把右侧 r.h 的结果保存到左侧 w.h。
    w.h = r.h;
    // 返回 w 给调用者，表示当前函数的执行结果。
    return w;
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 DisplayPort::instance：参数在括号里，返回类型写在函数名前面。
DisplayPort& DisplayPort::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static DisplayPort s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// [初学者导读] DisplayPort::init()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
bool DisplayPort::init()
{
    // 判断条件：ready_ 是否成立。
    if (ready_)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;

    // 判断条件：!panel_.init() 是否成立。
    if (!panel_.init())
    {
        // 调用 EBOOK_LOGE(TAG, "panel init failed");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGE(TAG, "panel init failed");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 判断条件：!backlight_.init() 是否成立。
    if (!backlight_.init())
        // 调用 EBOOK_LOGW(TAG, "backlight init failed");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "backlight init failed");

    // 调用 fill(0xFF)：fill()： 只把 framebuffer 填成一个字节颜色，不改屏幕。。
    panel_.fill(0xFF);
    // 访问对象成员：panel_.invalidate_session();；读取或修改这个对象里的成员状态。
    panel_.invalidate_session();
    // 赋值：把右侧 true 的结果保存到左侧 ready_。
    ready_ = true;
    // 调用 EBOOK_LOGI(TAG, "ready");，执行这个函数并处理它的返回值或状态变化。
    EBOOK_LOGI(TAG, "ready");
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 DisplayPort::deinit：参数在括号里，返回类型写在函数名前面。
void DisplayPort::deinit()
{
    // 判断条件：!ready_ 是否成立。
    if (!ready_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 deinit()：创建和销毁队列。
    panel_.deinit();
    // 调用 deinit()：创建和销毁队列。
    backlight_.deinit();
    // 赋值：把右侧 false 的结果保存到左侧 ready_。
    ready_ = false;
}

// 定义或声明函数 DisplayPort::framebuffer：参数在括号里，返回类型写在函数名前面。
uint8_t* DisplayPort::framebuffer()
{
    // 返回 panel_.fb() 给调用者，表示当前函数的执行结果。
    return panel_.fb();
}

// 定义或声明函数 DisplayPort::session：参数在括号里，返回类型写在函数名前面。
SessionState DisplayPort::session() const
{
    // 判断条件：!ready_ 是否成立。
    if (!ready_)
        // 返回 SessionState::Cold 给调用者，表示当前函数的执行结果。
        return SessionState::Cold;
    // 返回 panel_.session_ready() ? SessionState::PartialReady : SessionState::Cold 给调用者，表示当前函数的执行结果。
    return panel_.session_ready() ? SessionState::PartialReady : SessionState::Cold;
}

// 定义或声明函数 DisplayPort::invalidate_session：参数在括号里，返回类型写在函数名前面。
void DisplayPort::invalidate_session()
{
    // 判断条件：ready_ 是否成立。
    if (ready_)
        // 访问对象成员：panel_.invalidate_session();；读取或修改这个对象里的成员状态。
        panel_.invalidate_session();
}

// 函数定义/声明：调用 DisplayPort::bootstrap()：DisplayPort 是 Presenter 和具体屏幕驱动之间的适配层。它把核心层的 Rect 转成驱动的 Window，把驱动的 bool 结果转成 core::Status，并管理 session 状态。。
core::Status DisplayPort::bootstrap()
{
    // 判断条件：!ready_ 是否成立。
    if (!ready_)
        // 返回 core::Status::NotInit 给调用者，表示当前函数的执行结果。
        return core::Status::NotInit;
    // 返回 panel_.present(Present::Base) ? core::Status::Ok : core::Status::IoError 给调用者，表示当前函数的执行结果。
    return panel_.present(Present::Base) ? core::Status::Ok : core::Status::IoError;
}

// 函数定义/声明：调用 DisplayPort::partial()：DisplayPort 是 Presenter 和具体屏幕驱动之间的适配层。它把核心层的 Rect 转成驱动的 Window，把驱动的 bool 结果转成 core::Status，并管理 session 状态。。
core::Status DisplayPort::partial(const core::Rect& rect)
{
    // 判断条件：!ready_ || rect.empty() 是否成立。
    if (!ready_ || rect.empty())
        // 返回 core::Status::InvalidArg 给调用者，表示当前函数的执行结果。
        return core::Status::InvalidArg;
    // 返回 panel_.present(Present::Partial, make_window(rect)) ? core::Status::Ok 给调用者，表示当前函数的执行结果。
    return panel_.present(Present::Partial, make_window(rect)) ? core::Status::Ok
                                                               // 执行语句：: core::Status::IoError;；它会更新一次变量、对象或程序状态。
                                                               : core::Status::IoError;
}

// 函数定义/声明：调用 DisplayPort::fast()：DisplayPort 是 Presenter 和具体屏幕驱动之间的适配层。它把核心层的 Rect 转成驱动的 Window，把驱动的 bool 结果转成 core::Status，并管理 session 状态。。
core::Status DisplayPort::fast()
{
    // 判断条件：!ready_ 是否成立。
    if (!ready_)
        // 返回 core::Status::NotInit 给调用者，表示当前函数的执行结果。
        return core::Status::NotInit;
    // 返回 panel_.present(Present::Fast) ? core::Status::Ok : core::Status::IoError 给调用者，表示当前函数的执行结果。
    return panel_.present(Present::Fast) ? core::Status::Ok : core::Status::IoError;
}

// 定义或声明函数 DisplayPort::full：参数在括号里，返回类型写在函数名前面。
core::Status DisplayPort::full()
{
    // 判断条件：!ready_ 是否成立。
    if (!ready_)
        // 返回 core::Status::NotInit 给调用者，表示当前函数的执行结果。
        return core::Status::NotInit;
    // 返回 panel_.present(Present::Full) ? core::Status::Ok : core::Status::IoError 给调用者，表示当前函数的执行结果。
    return panel_.present(Present::Full) ? core::Status::Ok : core::Status::IoError;
}

// 函数定义/声明：调用 DisplayPort::set_brightness()：把 0 到 100 的亮度限制在合法范围后交给背光驱动。墨水屏本身不依赖背光，背光是前光/照明系统，不能把亮度当成画面灰度。。
void DisplayPort::set_brightness(uint8_t percent)
{
    // 判断条件：percent > 100 是否成立。
    if (percent > 100)
        // 赋值：把右侧 100 的结果保存到左侧 percent。
        percent = 100;
    // 调用 set_brightness(percent)：把 0 到 100 的亮度限制在合法范围后交给背光驱动。墨水屏本身不依赖背光，背光是前光/照明系统，不能把亮度当成画面灰度。。
    backlight_.set_brightness(percent);
}

// 函数定义/声明：调用 DisplayPort::brightness()：把 0 到 100 的亮度限制在合法范围后交给背光驱动。墨水屏本身不依赖背光，背光是前光/照明系统，不能把亮度当成画面灰度。。
uint8_t DisplayPort::brightness() const
{
    // 返回结果：调用 brightness()：把 0 到 100 的亮度限制在合法范围后交给背光驱动。墨水屏本身不依赖背光，背光是前光/照明系统，不能把亮度当成画面灰度。。
    return backlight_.brightness();
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::display
