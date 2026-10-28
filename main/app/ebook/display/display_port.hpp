// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/display/display_port.hpp
 * 作用：把上层刷新意图转换成墨水屏的 Full、Fast、Partial 操作。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：墨水屏刷新是异步且耗时的，要区分 Full、Fast、Partial。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/geometry.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/result.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "bsp/driver/gdey027t91/backlight.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "bsp/driver/gdey027t91/gdey027t91.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::display {

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class SessionState : uint8_t
{
    // 执行这条代码：Cold,。它会更新变量、对象状态或调用链。
    Cold,
    // 继续上一行的表达式或参数列表：PartialReady,。
    PartialReady,
};

/** @brief EPD 上屏适配（Present 波形 + 背光） */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class DisplayPort
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static DisplayPort& instance();

    // 函数调用：bool init();；执行该函数并处理它的返回值或副作用。
    bool init();
    // 调用 deinit()：创建和销毁队列。
    void deinit();
    // 定义或声明函数 ready：参数在括号里，返回类型写在函数名前面。
    bool ready() const
    {
        // 返回 ready_ 给调用者，表示当前函数的执行结果。
        return ready_;
    }

    // 函数调用：uint8_t* framebuffer();；执行该函数并处理它的返回值或副作用。
    uint8_t* framebuffer();
    // 函数调用：SessionState session() const;；执行该函数并处理它的返回值或副作用。
    SessionState session() const;
    // 函数调用：void invalidate_session();；执行该函数并处理它的返回值或副作用。
    void invalidate_session();

    // 调用 bootstrap()：DisplayPort 是 Presenter 和具体屏幕驱动之间的适配层。它把核心层的 Rect 转成驱动的 Window，把驱动的 bool 结果转成 core::Status，并管理 session 状态。。
    core::Status bootstrap();
    // 函数调用：core::Status partial(const core::Rect& rect);；执行该函数并处理它的返回值或副作用。
    core::Status partial(const core::Rect& rect);
    // 调用 fast()：DisplayPort 是 Presenter 和具体屏幕驱动之间的适配层。它把核心层的 Rect 转成驱动的 Window，把驱动的 bool 结果转成 core::Status，并管理 session 状态。。
    core::Status fast();
    // 调用 full()：DisplayPort 是 Presenter 和具体屏幕驱动之间的适配层。它把核心层的 Rect 转成驱动的 Window，把驱动的 bool 结果转成 core::Status，并管理 session 状态。。
    core::Status full();

    // 调用 set_brightness(uint8_tpercent)：把 0 到 100 的亮度限制在合法范围后交给背光驱动。墨水屏本身不依赖背光，背光是前光/照明系统，不能把亮度当成画面灰度。。
    void set_brightness(uint8_t percent);
    // 调用 brightness()：把 0 到 100 的亮度限制在合法范围后交给背光驱动。墨水屏本身不依赖背光，背光是前光/照明系统，不能把亮度当成画面灰度。。
    uint8_t brightness() const;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 DisplayPort() = default;，执行这个函数并处理它的返回值或状态变化。
    DisplayPort() = default;

    // 执行语句：::app::bsp::driver::gdey027t91::Gdey027t91 panel_;；它会更新一次变量、对象或程序状态。
    ::app::bsp::driver::gdey027t91::Gdey027t91 panel_;
    // 执行语句：::app::bsp::driver::gdey027t91::Backlight backlight_;；它会更新一次变量、对象或程序状态。
    ::app::bsp::driver::gdey027t91::Backlight backlight_;
    // 执行语句：bool ready_{false};；它会更新一次变量、对象或程序状态。
    bool ready_{false};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::display
