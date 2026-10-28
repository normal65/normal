// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/input/physical_types.hpp
 * 作用：定义物理按键、动作和事件的基础类型。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：中断和输入任务不能直接修改页面，只能投递结果。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::input {

/** 拨码档位（与 BSP DipId 对应，ebook 层抽象） */
// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class PhysicalKey : uint8_t
{
    // 执行这条代码：Up = 0,。它会更新变量、对象状态或调用链。
    Up = 0,
    // 继续上一行的表达式或参数列表：Mid,。
    Mid,
    // 继续上一行的表达式或参数列表：Down,。
    Down,
};

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class PhysicalAction : uint8_t
{
    // 执行这条代码：Press = 0,。它会更新变量、对象状态或调用链。
    Press = 0,
    // 继续上一行的表达式或参数列表：Release,。
    Release,
};

// struct PhysicalEvent：把相关数据字段组合在一起，表示项目中的一个概念。
struct PhysicalEvent
{
    // 执行语句：PhysicalKey key{PhysicalKey::Up};；它会更新一次变量、对象或程序状态。
    PhysicalKey key{PhysicalKey::Up};
    // 执行语句：PhysicalAction action{PhysicalAction::Press};；它会更新一次变量、对象或程序状态。
    PhysicalAction action{PhysicalAction::Press};
};

// 只读约束：这个值或对象在当前位置不能被修改。
const char* physical_key_name(PhysicalKey k);
// 只读约束：这个值或对象在当前位置不能被修改。
const char* physical_action_name(PhysicalAction a);

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::input
