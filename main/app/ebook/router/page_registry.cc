/**
 * [初学者导读] 文件：main/app/ebook/router/page_registry.cc
 * 作用：维护页面名称和 ID 的映射。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：路由先改页面栈，再向 Presenter 提交刷新请求；提交是异步的。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/page_id.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app_id.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::router {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 只读约束：这个值或对象在当前位置不能被修改。
const char* shell_name(ShellPage p)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (p)
    {
        // 分支匹配：case ShellPage::Lock 成立时执行下面的代码。
        case ShellPage::Lock:
            // 返回 "Lock" 给调用者，表示当前函数的执行结果。
            return "Lock";
        // 分支匹配：case ShellPage::Home 成立时执行下面的代码。
        case ShellPage::Home:
            // 返回 "Home" 给调用者，表示当前函数的执行结果。
            return "Home";
        // 分支匹配：case ShellPage::AppGrid 成立时执行下面的代码。
        case ShellPage::AppGrid:
            // 返回 "AppGrid" 给调用者，表示当前函数的执行结果。
            return "AppGrid";
        // 分支匹配：case ShellPage::AppHost 成立时执行下面的代码。
        case ShellPage::AppHost:
            // 返回 "AppHost" 给调用者，表示当前函数的执行结果。
            return "AppHost";
    }
    // 返回 "?" 给调用者，表示当前函数的执行结果。
    return "?";
}

// 只读约束：这个值或对象在当前位置不能被修改。
const char* overlay_name(OverlayId o)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (o)
    {
        // 分支匹配：case OverlayId::None 成立时执行下面的代码。
        case OverlayId::None:
            // 返回 "None" 给调用者，表示当前函数的执行结果。
            return "None";
        // 分支匹配：case OverlayId::ControlCenter 成立时执行下面的代码。
        case OverlayId::ControlCenter:
            // 返回 "ControlCenter" 给调用者，表示当前函数的执行结果。
            return "ControlCenter";
        // 分支匹配：case OverlayId::Keyboard 成立时执行下面的代码。
        case OverlayId::Keyboard:
            // 返回 "Keyboard" 给调用者，表示当前函数的执行结果。
            return "Keyboard";
        // 分支匹配：case OverlayId::Toast 成立时执行下面的代码。
        case OverlayId::Toast:
            // 返回 "Toast" 给调用者，表示当前函数的执行结果。
            return "Toast";
    }
    // 返回 "?" 给调用者，表示当前函数的执行结果。
    return "?";
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 只读约束：这个值或对象在当前位置不能被修改。
const char* page_name(const PageId& id)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (id.zone)
    {
        // 分支匹配：case PageId::Zone::Shell 成立时执行下面的代码。
        case PageId::Zone::Shell:
            // 返回 shell_name(id.shell) 给调用者，表示当前函数的执行结果。
            return shell_name(id.shell);
        // 分支匹配：case PageId::Zone::Overlay 成立时执行下面的代码。
        case PageId::Zone::Overlay:
            // 返回 overlay_name(id.overlay) 给调用者，表示当前函数的执行结果。
            return overlay_name(id.overlay);
        // 分支匹配：case PageId::Zone::App 成立时执行下面的代码。
        case PageId::Zone::App:
            // 返回 apps::app_id_name(id.app) 给调用者，表示当前函数的执行结果。
            return apps::app_id_name(id.app);
    }
    // 返回 "?" 给调用者，表示当前函数的执行结果。
    return "?";
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::router
