// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/overlays/control_center.hpp
 * 作用：实现 overlays 模块中的 control center 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：覆盖层优先接收输入，打开和关闭会改变路由栈。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/geometry.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/canvas.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/input_event.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "shell/page.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::overlays {

/**
 * @brief 控制中心（状态栏下滑）；局刷 partial_full；锁屏走 refresh_edges Fast
 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class ControlCenter
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static ControlCenter& instance();

    // 调用 is_open()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
    bool is_open() const;
    // 调用 open()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
    void open();
    // 调用 close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
    void close();

    // 函数调用：core::Rect bounds() const;；执行该函数并处理它的返回值或副作用。
    core::Rect bounds() const;
    // 函数调用：void paint(gfx::Canvas& canvas);；执行该函数并处理它的返回值或副作用。
    void paint(gfx::Canvas& canvas);
    // 调用 on_input(const::app::ebook::input::Event&ev)：文件 App 的输入总入口。它处理： 触摸按下、移动、抬起和滑动。 工具栏返回。 列表滚动节流。 弹窗点击优先于底层列表点击。。
    shell::InputResult on_input(const ::app::ebook::input::Event& ev);

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 ControlCenter() = default;，执行这个函数并处理它的返回值或状态变化。
    ControlCenter() = default;

    // 函数调用：core::Rect tile_rect(uint8_t idx) const;；执行该函数并处理它的返回值或副作用。
    core::Rect tile_rect(uint8_t idx) const;
    // 函数调用：core::Rect level_row_rect(uint8_t idx) const;；执行该函数并处理它的返回值或副作用。
    core::Rect level_row_rect(uint8_t idx) const;
    // 函数调用：core::Rect level_minus_rect(uint8_t idx) const;；执行该函数并处理它的返回值或副作用。
    core::Rect level_minus_rect(uint8_t idx) const;
    // 函数调用：core::Rect level_plus_rect(uint8_t idx) const;；执行该函数并处理它的返回值或副作用。
    core::Rect level_plus_rect(uint8_t idx) const;

    // 调用 adjust_level(uint8_tidx,intdelta)：索引 0 调亮度，索引 1 调音量。它先计算夹紧后的新值： 值没变时返回 false，调用者不刷新。 值变化时调用 SystemState::set()，返回 true。。
    bool adjust_level(uint8_t idx, int delta);
    // 函数调用：void repaint_panel();；执行该函数并处理它的返回值或副作用。
    void repaint_panel();
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::overlays
