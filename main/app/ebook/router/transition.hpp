// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/router/transition.hpp
 * 作用：描述一次页面跳转的来源、目标和导航动作。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：路由先改页面栈，再向 Presenter 提交刷新请求；提交是异步的。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/page_id.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::router {

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class NavAction : uint8_t
{
    // 执行这条代码：Forward,。它会更新变量、对象状态或调用链。
    Forward,
    // 继续上一行的表达式或参数列表：Back,。
    Back,
    // 继续上一行的表达式或参数列表：Replace,。
    Replace,
    // 继续上一行的表达式或参数列表：Repaint,。
    Repaint,
};

// struct Transition：把相关数据字段组合在一起，表示项目中的一个概念。
struct Transition
{
    // 声明并默认初始化对象：PageId from{}。
    PageId from{};
    // 声明并默认初始化对象：PageId to{}。
    PageId to{};
    // 执行语句：NavAction action{NavAction::Forward};；它会更新一次变量、对象或程序状态。
    NavAction action{NavAction::Forward};
};

// 只读约束：这个值或对象在当前位置不能被修改。
const char* action_name(NavAction a);

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::router
