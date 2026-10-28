// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/reader/reader_menu.hpp
 * 作用：绘制和处理阅读器菜单。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/reader_layout.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/canvas.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::reader {

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class MenuHit : uint8_t
{
    // 执行这条代码：None,。它会更新变量、对象状态或调用链。
    None,
    // 继续上一行的表达式或参数列表：FontPrev,。
    FontPrev,
    // 继续上一行的表达式或参数列表：FontNext,。
    FontNext,
    // 继续上一行的表达式或参数列表：RefreshPrev,。
    RefreshPrev,
    // 继续上一行的表达式或参数列表：RefreshNext,。
    RefreshNext,
    // 继续上一行的表达式或参数列表：JumpPrev10,。
    JumpPrev10,
    // 继续上一行的表达式或参数列表：PagePrev,。
    PagePrev,
    // 继续上一行的表达式或参数列表：PageNext,。
    PageNext,
    // 继续上一行的表达式或参数列表：JumpNext10,。
    JumpNext10,
    // 继续上一行的表达式或参数列表：Shelf,。
    Shelf,
    // 继续上一行的表达式或参数列表：JumpPage,。
    JumpPage,
    // 继续上一行的表达式或参数列表：Lock,。
    Lock,
};

// struct MenuPaintInfo：把相关数据字段组合在一起，表示项目中的一个概念。
struct MenuPaintInfo
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* font_label;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* refresh_label;
    // 执行语句：uint8_t read_percent;；它会更新一次变量、对象或程序状态。
    uint8_t read_percent;
};

// 函数调用：void paint_menu_panel(gfx::Canvas& c, const ReaderLayout& layout, const MenuPaintInfo& info);；执行该函数并处理它的返回值或副作用。
void paint_menu_panel(gfx::Canvas& c, const ReaderLayout& layout, const MenuPaintInfo& info);
// 函数调用：MenuHit hit_menu(int16_t x, int16_t y, const ReaderLayout& layout);；执行该函数并处理它的返回值或副作用。
MenuHit hit_menu(int16_t x, int16_t y, const ReaderLayout& layout);

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::reader
