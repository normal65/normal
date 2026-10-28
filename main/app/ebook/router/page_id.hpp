// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/router/page_id.hpp
 * 作用：实现 router 模块中的 page id 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：路由先改页面栈，再向 Presenter 提交刷新请求；提交是异步的。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app_id.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::router {

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class ShellPage : uint8_t
{
    // 执行这条代码：Lock = 0,。它会更新变量、对象状态或调用链。
    Lock = 0,
    // 继续上一行的表达式或参数列表：Home,。
    Home,
    // 继续上一行的表达式或参数列表：AppGrid,。
    AppGrid,
    // 继续上一行的表达式或参数列表：AppHost,。
    AppHost,
};

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class OverlayId : uint8_t
{
    // 执行这条代码：None = 0,。它会更新变量、对象状态或调用链。
    None = 0,
    // 继续上一行的表达式或参数列表：ControlCenter,。
    ControlCenter,
    // 继续上一行的表达式或参数列表：Keyboard,。
    Keyboard,
    // 继续上一行的表达式或参数列表：Toast,。
    Toast,
};

// struct PageId：把相关数据字段组合在一起，表示项目中的一个概念。
struct PageId
{
    // 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
    enum class Zone : uint8_t
    {
        // 执行这条代码：Shell,。它会更新变量、对象状态或调用链。
        Shell,
        // 继续上一行的表达式或参数列表：App,。
        App,
        // 继续上一行的表达式或参数列表：Overlay,。
        Overlay,
    // 继续上一行的表达式或参数列表：} zone{Zone::Shell};。
    } zone{Zone::Shell};

    // 执行语句：ShellPage shell{ShellPage::Lock};；它会更新一次变量、对象或程序状态。
    ShellPage shell{ShellPage::Lock};
    // 执行语句：apps::AppId app{apps::AppId::None};；它会更新一次变量、对象或程序状态。
    apps::AppId app{apps::AppId::None};
    // 执行语句：uint16_t app_page{0};；它会更新一次变量、对象或程序状态。
    uint16_t app_page{0};
    // 执行语句：OverlayId overlay{OverlayId::None};；它会更新一次变量、对象或程序状态。
    OverlayId overlay{OverlayId::None};

    // 定义变量 bool operator 并初始化为 =(const PageId& o) cons。
    bool operator==(const PageId& o) const
    {
        // 返回 zone == o.zone && shell == o.shell && app == o.app && app_page == o.app_page && 给调用者，表示当前函数的执行结果。
        return zone == o.zone && shell == o.shell && app == o.app && app_page == o.app_page &&
               // 继续上一行的表达式或参数列表：overlay == o.overlay;。
               overlay == o.overlay;
    }
};

// 编译期函数 page_shell：常用于固定规则计算，能参与编译期优化。
constexpr PageId page_shell(ShellPage p)
{
    // 声明并默认初始化对象：PageId id{}。
    PageId id{};
    // 赋值：把右侧 PageId::Zone::Shell 的结果保存到左侧 id.zone。
    id.zone = PageId::Zone::Shell;
    // 赋值：把右侧 p 的结果保存到左侧 id.shell。
    id.shell = p;
    // 返回 id 给调用者，表示当前函数的执行结果。
    return id;
}

// 编译期函数 page_app：常用于固定规则计算，能参与编译期优化。
constexpr PageId page_app(apps::AppId app, uint16_t sub = 0)
{
    // 声明并默认初始化对象：PageId id{}。
    PageId id{};
    // 赋值：把右侧 PageId::Zone::App 的结果保存到左侧 id.zone。
    id.zone = PageId::Zone::App;
    // 赋值：把右侧 app 的结果保存到左侧 id.app。
    id.app = app;
    // 赋值：把右侧 sub 的结果保存到左侧 id.app_page。
    id.app_page = sub;
    // 返回 id 给调用者，表示当前函数的执行结果。
    return id;
}

// 编译期函数 page_overlay：常用于固定规则计算，能参与编译期优化。
constexpr PageId page_overlay(OverlayId o)
{
    // 声明并默认初始化对象：PageId id{}。
    PageId id{};
    // 赋值：把右侧 PageId::Zone::Overlay 的结果保存到左侧 id.zone。
    id.zone = PageId::Zone::Overlay;
    // 赋值：把右侧 o 的结果保存到左侧 id.overlay。
    id.overlay = o;
    // 返回 id 给调用者，表示当前函数的执行结果。
    return id;
}

// 只读约束：这个值或对象在当前位置不能被修改。
const char* page_name(const PageId& id);

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::router
