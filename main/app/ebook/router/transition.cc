/**
 * [初学者导读] 文件：main/app/ebook/router/transition.cc
 * 作用：描述一次页面跳转的来源、目标和导航动作。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：路由先改页面栈，再向 Presenter 提交刷新请求；提交是异步的。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/transition.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::router {

// 只读约束：这个值或对象在当前位置不能被修改。
const char* action_name(NavAction a)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (a)
    {
        // 分支匹配：case NavAction::Forward 成立时执行下面的代码。
        case NavAction::Forward:
            // 返回 "FWD" 给调用者，表示当前函数的执行结果。
            return "FWD";
        // 分支匹配：case NavAction::Back 成立时执行下面的代码。
        case NavAction::Back:
            // 返回 "BACK" 给调用者，表示当前函数的执行结果。
            return "BACK";
        // 分支匹配：case NavAction::Replace 成立时执行下面的代码。
        case NavAction::Replace:
            // 返回 "REPLACE" 给调用者，表示当前函数的执行结果。
            return "REPLACE";
        // 分支匹配：case NavAction::Repaint 成立时执行下面的代码。
        case NavAction::Repaint:
            // 返回 "REPAINT" 给调用者，表示当前函数的执行结果。
            return "REPAINT";
    }
    // 返回 "?" 给调用者，表示当前函数的执行结果。
    return "?";
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::router
