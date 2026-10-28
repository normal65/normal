// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/presenter/present_plan.hpp
 * 作用：把刷新波形转换成墨水屏驱动的具体刷新步骤。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：Presenter 队列有容量限制，提交失败要调用方处理。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/geometry.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/refresh_intent.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::presenter {

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class PresentOp : uint8_t
{
    // 执行这条代码：Bootstrap,。它会更新变量、对象状态或调用链。
    Bootstrap,
    // 继续上一行的表达式或参数列表：Partial,。
    Partial,
    // 继续上一行的表达式或参数列表：Fast,。
    Fast,
    // 继续上一行的表达式或参数列表：Full,。
    Full,
};

// struct PresentStep：把相关数据字段组合在一起，表示项目中的一个概念。
struct PresentStep
{
    // 执行语句：PresentOp op{PresentOp::Partial};；它会更新一次变量、对象或程序状态。
    PresentOp op{PresentOp::Partial};
    // 声明并默认初始化对象：core::Rect rect{}。
    core::Rect rect{};
};

// struct PresentPlan：把相关数据字段组合在一起，表示项目中的一个概念。
struct PresentPlan
{
    // 执行语句：PresentStep steps[2]{};；它会更新一次变量、对象或程序状态。
    PresentStep steps[2]{};
    // 执行语句：uint8_t count{0};；它会更新一次变量、对象或程序状态。
    uint8_t count{0};
};

/** Waveform → Bootstrap? + Partial/Fast/Full 步骤 */
// 函数调用：PresentPlan make_plan(router::RefreshIntent intent, bool can_partial);；执行该函数并处理它的返回值或副作用。
PresentPlan make_plan(router::RefreshIntent intent, bool can_partial);

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::presenter
