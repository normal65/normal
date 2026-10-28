/**
 * [初学者导读] 文件：main/app/ebook/router/refresh_intent.cc
 * 作用：定义 Partial、Fast、Full 刷新意图和波形名称。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：路由先改页面栈，再向 Presenter 提交刷新请求；提交是异步的。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/refresh_intent.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::router {

// 只读约束：这个值或对象在当前位置不能被修改。
const char* waveform_name(Waveform w)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (w)
    {
        // 分支匹配：case Waveform::Partial 成立时执行下面的代码。
        case Waveform::Partial:
            // 返回 "Partial" 给调用者，表示当前函数的执行结果。
            return "Partial";
        // 分支匹配：case Waveform::Fast 成立时执行下面的代码。
        case Waveform::Fast:
            // 返回 "Fast" 给调用者，表示当前函数的执行结果。
            return "Fast";
        // 分支匹配：case Waveform::Full 成立时执行下面的代码。
        case Waveform::Full:
            // 返回 "Full" 给调用者，表示当前函数的执行结果。
            return "Full";
    }
    // 返回 "?" 给调用者，表示当前函数的执行结果。
    return "?";
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::router
