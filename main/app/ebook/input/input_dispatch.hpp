// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/input/input_dispatch.hpp
 * 作用：把物理按键和语义动作分发到 Router、App 或全局功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：中断和输入任务不能直接修改页面，只能投递结果。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/input_bindings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/physical_types.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::input {

/** @brief 将语义动作转为现有触摸分发或全局副作用（UI 线程内调用） */
// 调用 dispatch_semantic(SemanticActionaction)：语义动作不包含“哪个按键”，只描述用户意图，例如 PageNext、Back、Home、VolumeUp。dispatchsemantic() 把意图变成真实的 Router 导航、音量/亮度修改或阅读菜单操作。。
void dispatch_semantic(SemanticAction action);

/** @brief 拨码事件入口：解析上下文 → 语义 → dispatch_semantic */
// 调用 dispatch_physical(constPhysicalEvent&ev)：resolvephysical() 只处理 Press，然后查 KeyBindings。dispatchphysical() 负责处理键盘覆盖层和上下文判定，最后调用 dispatchsemantic()。。
void dispatch_physical(const PhysicalEvent& ev);

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::input
