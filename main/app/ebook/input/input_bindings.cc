/**
 * [初学者导读] 文件：main/app/ebook/input/input_bindings.cc
 * 作用：提供物理按键、上下文的检测和动作解析。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：中断和输入任务不能直接修改页面，只能投递结果。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/input_bindings.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app_id.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app_registry.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/key_bindings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/router.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::input {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 类型别名引入：把 apps::AppId 引入当前文件，后面直接用 AppId。
using apps::AppId;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using router::Router;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using router::ShellPage;

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// [初学者导读] detect_context()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
InputContext detect_context()
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto& stack = Router::instance().stack();

    // 判断条件：stack.shell_top() == ShellPage::AppHost 是否成立。
    if (stack.shell_top() == ShellPage::AppHost)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const AppId id = apps::AppRegistry::instance().active_id();
        // 判断条件：id == AppId::Reader 是否成立。
        if (id == AppId::Reader)
            // 返回 InputContext::Reader 给调用者，表示当前函数的执行结果。
            return InputContext::Reader;
        // 其余内置 App（记事本/时钟/天气等）统一走列表键位：上下滚动 + 中键返回
        if (id != AppId::None)
            // 返回 InputContext::List 给调用者，表示当前函数的执行结果。
            return InputContext::List;
    }

    // 判断条件：stack.shell_top() == ShellPage::Home || stack.shell_top() == ShellPage::AppGrid 是否成立。
    if (stack.shell_top() == ShellPage::Home || stack.shell_top() == ShellPage::AppGrid)
        // 返回 InputContext::List 给调用者，表示当前函数的执行结果。
        return InputContext::List;

    // 返回 InputContext::Global 给调用者，表示当前函数的执行结果。
    return InputContext::Global;
}

// 函数定义/声明：调用 resolve_physical()：resolvephysical() 只处理 Press，然后查 KeyBindings。dispatchphysical() 负责处理键盘覆盖层和上下文判定，最后调用 dispatchsemantic()。。
SemanticAction resolve_physical(PhysicalKey key, PhysicalAction action, InputContext ctx)
{
    // 判断条件：action != PhysicalAction::Press 是否成立。
    if (action != PhysicalAction::Press)
        // 返回 SemanticAction::None 给调用者，表示当前函数的执行结果。
        return SemanticAction::None;
    // 返回 KeyBindings::instance().lookup(key, ctx) 给调用者，表示当前函数的执行结果。
    return KeyBindings::instance().lookup(key, ctx);
}

// 只读约束：这个值或对象在当前位置不能被修改。
const char* semantic_action_name(SemanticAction a)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (a)
    {
        // 分支匹配：case SemanticAction::None 成立时执行下面的代码。
        case SemanticAction::None:
            // 返回 "None" 给调用者，表示当前函数的执行结果。
            return "None";
        // 分支匹配：case SemanticAction::PageNext 成立时执行下面的代码。
        case SemanticAction::PageNext:
            // 返回 "PageNext" 给调用者，表示当前函数的执行结果。
            return "PageNext";
        // 分支匹配：case SemanticAction::PagePrev 成立时执行下面的代码。
        case SemanticAction::PagePrev:
            // 返回 "PagePrev" 给调用者，表示当前函数的执行结果。
            return "PagePrev";
        // 分支匹配：case SemanticAction::ListUp 成立时执行下面的代码。
        case SemanticAction::ListUp:
            // 返回 "ListUp" 给调用者，表示当前函数的执行结果。
            return "ListUp";
        // 分支匹配：case SemanticAction::ListDown 成立时执行下面的代码。
        case SemanticAction::ListDown:
            // 返回 "ListDown" 给调用者，表示当前函数的执行结果。
            return "ListDown";
        // 分支匹配：case SemanticAction::Back 成立时执行下面的代码。
        case SemanticAction::Back:
            // 返回 "Back" 给调用者，表示当前函数的执行结果。
            return "Back";
        // 分支匹配：case SemanticAction::BrightnessUp 成立时执行下面的代码。
        case SemanticAction::BrightnessUp:
            // 返回 "BrightnessUp" 给调用者，表示当前函数的执行结果。
            return "BrightnessUp";
        // 分支匹配：case SemanticAction::BrightnessDown 成立时执行下面的代码。
        case SemanticAction::BrightnessDown:
            // 返回 "BrightnessDown" 给调用者，表示当前函数的执行结果。
            return "BrightnessDown";
        // 分支匹配：case SemanticAction::VolumeUp 成立时执行下面的代码。
        case SemanticAction::VolumeUp:
            // 返回 "VolumeUp" 给调用者，表示当前函数的执行结果。
            return "VolumeUp";
        // 分支匹配：case SemanticAction::VolumeDown 成立时执行下面的代码。
        case SemanticAction::VolumeDown:
            // 返回 "VolumeDown" 给调用者，表示当前函数的执行结果。
            return "VolumeDown";
        // 分支匹配：case SemanticAction::Menu 成立时执行下面的代码。
        case SemanticAction::Menu:
            // 返回 "Menu" 给调用者，表示当前函数的执行结果。
            return "Menu";
        // 分支匹配：case SemanticAction::Home 成立时执行下面的代码。
        case SemanticAction::Home:
            // 返回 "Home" 给调用者，表示当前函数的执行结果。
            return "Home";
    }
    // 返回 "?" 给调用者，表示当前函数的执行结果。
    return "?";
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::input
