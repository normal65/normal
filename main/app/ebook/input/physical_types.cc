/**
 * [初学者导读] 文件：main/app/ebook/input/physical_types.cc
 * 作用：定义物理按键、动作和事件的基础类型。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：中断和输入任务不能直接修改页面，只能投递结果。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/physical_types.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::input {

// 只读约束：这个值或对象在当前位置不能被修改。
const char* physical_key_name(PhysicalKey k)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (k)
    {
        // 分支匹配：case PhysicalKey::Up 成立时执行下面的代码。
        case PhysicalKey::Up:
            // 返回 "Up" 给调用者，表示当前函数的执行结果。
            return "Up";
        // 分支匹配：case PhysicalKey::Mid 成立时执行下面的代码。
        case PhysicalKey::Mid:
            // 返回 "Mid" 给调用者，表示当前函数的执行结果。
            return "Mid";
        // 分支匹配：case PhysicalKey::Down 成立时执行下面的代码。
        case PhysicalKey::Down:
            // 返回 "Down" 给调用者，表示当前函数的执行结果。
            return "Down";
    }
    // 返回 "?" 给调用者，表示当前函数的执行结果。
    return "?";
}

// 只读约束：这个值或对象在当前位置不能被修改。
const char* physical_action_name(PhysicalAction a)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (a)
    {
        // 分支匹配：case PhysicalAction::Press 成立时执行下面的代码。
        case PhysicalAction::Press:
            // 返回 "Press" 给调用者，表示当前函数的执行结果。
            return "Press";
        // 分支匹配：case PhysicalAction::Release 成立时执行下面的代码。
        case PhysicalAction::Release:
            // 返回 "Release" 给调用者，表示当前函数的执行结果。
            return "Release";
    }
    // 返回 "?" 给调用者，表示当前函数的执行结果。
    return "?";
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::input
