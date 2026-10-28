// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/ui/list_view.hpp
 * 作用：实现通用列表控件的绘制、滚动、命中测试和行提供者回调。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：UI 状态只能在 UiLoop/UI 线程修改，跨任务结果通过 UiBus 回 UI。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <functional>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/geometry.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/canvas.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/input_event.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::ui {

/** @brief 虚拟列表：按视口渲染，整行 swipe 滚动 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class ListView
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using RowProvider = std::function<void(uint8_t index, widgets::RowStyle& out)>;
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using TapHandler = std::function<void(uint8_t index)>;

    // 定义或声明函数 set_provider：参数在括号里，返回类型写在函数名前面。
    void set_provider(RowProvider p)
    {
        // 赋值：把右侧 std::move(p) 的结果保存到左侧 provider_。
        provider_ = std::move(p);
    }
    // 定义或声明函数 set_tap_handler：参数在括号里，返回类型写在函数名前面。
    void set_tap_handler(TapHandler h)
    {
        // 赋值：把右侧 std::move(h) 的结果保存到左侧 tap_。
        tap_ = std::move(h);
    }
    // 函数调用：void set_total(uint8_t total);；执行该函数并处理它的返回值或副作用。
    void set_total(uint8_t total);

    // 定义或声明函数 set_area：参数在括号里，返回类型写在函数名前面。
    void set_area(const core::Rect& a)
    {
        // 赋值：把右侧 a 的结果保存到左侧 area_。
        area_ = a;
    }
    // 函数调用：void set_scroll(uint8_t s);；执行该函数并处理它的返回值或副作用。
    void set_scroll(uint8_t s);

    // 定义或声明函数 total：参数在括号里，返回类型写在函数名前面。
    uint8_t total() const
    {
        // 返回 total_ 给调用者，表示当前函数的执行结果。
        return total_;
    }
    // 定义或声明函数 scroll：参数在括号里，返回类型写在函数名前面。
    uint8_t scroll() const
    {
        // 返回 scroll_ 给调用者，表示当前函数的执行结果。
        return scroll_;
    }
    // 定义或声明函数 area：参数在括号里，返回类型写在函数名前面。
    core::Rect area() const
    {
        // 返回 area_ 给调用者，表示当前函数的执行结果。
        return area_;
    }

    // 调用 paint(gfx::Canvas&c)：画文件管理主页面： 工具栏标题。 加载状态。 列表。 截断提示。。
    void paint(gfx::Canvas& c);

    // struct InputOutcome：把相关数据字段组合在一起，表示项目中的一个概念。
    struct InputOutcome
    {
        // 执行语句：bool consumed{false};；它会更新一次变量、对象或程序状态。
        bool consumed{false};
        // 执行语句：bool scroll_changed{false};；它会更新一次变量、对象或程序状态。
        bool scroll_changed{false};
        // 执行语句：bool tap_consumed{false};；它会更新一次变量、对象或程序状态。
        bool tap_consumed{false};
    };
    // 函数调用：InputOutcome handle_input(const ::app::ebook::input::Event& ev);；执行该函数并处理它的返回值或副作用。
    InputOutcome handle_input(const ::app::ebook::input::Event& ev);

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 执行语句：RowProvider provider_;；它会更新一次变量、对象或程序状态。
    RowProvider provider_;
    // 执行语句：TapHandler tap_;；它会更新一次变量、对象或程序状态。
    TapHandler tap_;
    // 执行语句：uint8_t total_{0};；它会更新一次变量、对象或程序状态。
    uint8_t total_{0};
    // 执行语句：uint8_t scroll_{0};；它会更新一次变量、对象或程序状态。
    uint8_t scroll_{0};
    // 声明并默认初始化对象：core::Rect area_{}。
    core::Rect area_{};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::ui
