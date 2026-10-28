/**
 * [初学者导读] 文件：main/app/ebook/presenter/present_plan.cc
 * 作用：把刷新波形转换成墨水屏驱动的具体刷新步骤。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：Presenter 队列有容量限制，提交失败要调用方处理。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "presenter/present_plan.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::presenter {

// 函数定义/声明：调用 make_plan()：波形到步骤。
PresentPlan make_plan(router::RefreshIntent intent, bool can_partial)
{
    // 声明并默认初始化对象：PresentPlan plan{}。
    PresentPlan plan{};

    // 开始代码块：下面是这个函数、分支或循环的主体。
    auto push = [&](PresentOp op, const core::Rect& rect = core::Rect::full()) {
        // 判断条件：plan.count < 2 是否成立。
        if (plan.count < 2)
        {
            // 赋值：把右侧 op 的结果保存到左侧 plan.steps[plan.count].op。
            plan.steps[plan.count].op = op;
            // 赋值：把右侧 rect 的结果保存到左侧 plan.steps[plan.count].rect。
            plan.steps[plan.count].rect = rect;
            // 访问对象成员：++plan.count;；读取或修改这个对象里的成员状态。
            ++plan.count;
        }
    };

    // 多分支判断：根据括号里的值选择不同 case。
    switch (intent.waveform)
    {
        // 分支匹配：case router::Waveform::Partial 成立时执行下面的代码。
        case router::Waveform::Partial:
            // 判断条件：!can_partial 是否成立。
            if (!can_partial)
                // 调用 push(PresentOp::Bootstrap);，执行这个函数并处理它的返回值或状态变化。
                push(PresentOp::Bootstrap);
            // 调用 push(PresentOp::Partial, core::Rect::full());，执行这个函数并处理它的返回值或状态变化。
            push(PresentOp::Partial, core::Rect::full());
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case router::Waveform::Fast 成立时执行下面的代码。
        case router::Waveform::Fast:
            // 判断条件：!can_partial 是否成立。
            if (!can_partial)
                // 调用 push(PresentOp::Bootstrap);，执行这个函数并处理它的返回值或状态变化。
                push(PresentOp::Bootstrap);
            // 调用 push(PresentOp::Fast);，执行这个函数并处理它的返回值或状态变化。
            push(PresentOp::Fast);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case router::Waveform::Full 成立时执行下面的代码。
        case router::Waveform::Full:
            // 调用 push(PresentOp::Full);，执行这个函数并处理它的返回值或状态变化。
            push(PresentOp::Full);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }

    // 返回 plan 给调用者，表示当前函数的执行结果。
    return plan;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::presenter
