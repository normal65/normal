// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/input/input_bindings.hpp
 * 作用：提供物理按键、上下文的检测和动作解析。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：中断和输入任务不能直接修改页面，只能投递结果。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/physical_types.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::input {

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class InputContext : uint8_t
{
    // 执行这条代码：Global = 0,。它会更新变量、对象状态或调用链。
    Global = 0,
    // 继续上一行的表达式或参数列表：Reader,。
    Reader,
    // 继续上一行的表达式或参数列表：List,。
    List,
};

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class SemanticAction : uint8_t
{
    // 执行这条代码：None = 0,。它会更新变量、对象状态或调用链。
    None = 0,
    // 继续上一行的表达式或参数列表：PageNext,。
    PageNext,
    // 继续上一行的表达式或参数列表：PagePrev,。
    PagePrev,
    // 继续上一行的表达式或参数列表：ListUp,。
    ListUp,
    // 继续上一行的表达式或参数列表：ListDown,。
    ListDown,
    // 继续上一行的表达式或参数列表：Back,。
    Back,
    // 继续上一行的表达式或参数列表：BrightnessUp,。
    BrightnessUp,
    // 继续上一行的表达式或参数列表：BrightnessDown,。
    BrightnessDown,
    // 继续上一行的表达式或参数列表：VolumeUp,。
    VolumeUp,
    // 继续上一行的表达式或参数列表：VolumeDown,。
    VolumeDown,
    // 继续上一行的表达式或参数列表：Menu,。
    Menu,
    // 继续上一行的表达式或参数列表：Home,。
    Home,
};

// 调用 detect_context()：上下文表示“当前输入应该按哪张表解释”： Reader：阅读器，支持上一页/下一页和菜单。 List：主页、应用列表和大部分 App，支持列表上下和返回。 Global：锁屏等特殊页面，普通按键大多忽略。。
InputContext detect_context();
// 调用 resolve_physical(PhysicalKeykey,PhysicalActionaction,InputContextctx)：resolvephysical() 只处理 Press，然后查 KeyBindings。dispatchphysical() 负责处理键盘覆盖层和上下文判定，最后调用 dispatchsemantic()。。
SemanticAction resolve_physical(PhysicalKey key, PhysicalAction action, InputContext ctx);

// 只读约束：这个值或对象在当前位置不能被修改。
const char* semantic_action_name(SemanticAction a);

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::input
