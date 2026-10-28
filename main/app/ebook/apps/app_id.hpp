// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/app_id.hpp
 * 作用：定义所有 App 的公共生命周期、绘制、输入和刷新接口。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps {

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class AppId : uint8_t
{
    // 执行这条代码：None = 0,。它会更新变量、对象状态或调用链。
    None = 0,
    // 继续上一行的表达式或参数列表：Reader,。
    Reader,
    // 继续上一行的表达式或参数列表：Notepad,。
    Notepad,
    // 继续上一行的表达式或参数列表：Gallery,。
    Gallery,
    // 继续上一行的表达式或参数列表：Drawing,。
    Drawing,
    // 继续上一行的表达式或参数列表：WoodenFish,。
    WoodenFish,
    // 继续上一行的表达式或参数列表：Music,。
    Music,
    // 继续上一行的表达式或参数列表：Weather,。
    Weather,
    // 继续上一行的表达式或参数列表：Clock,。
    Clock,
    // 继续上一行的表达式或参数列表：Calendar,。
    Calendar,
    // 继续上一行的表达式或参数列表：Files,。
    Files,
    // 继续上一行的表达式或参数列表：Update,。
    Update,
    // 继续上一行的表达式或参数列表：Settings,。
    Settings,
    // 继续上一行的表达式或参数列表：Count。
    Count
};

// 编译期函数 app_id_name：常用于固定规则计算，能参与编译期优化。
constexpr const char* app_id_name(AppId id)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (id)
    {
        // 分支匹配：case AppId::Reader 成立时执行下面的代码。
        case AppId::Reader:
            // 返回 "reader" 给调用者，表示当前函数的执行结果。
            return "reader";
        // 分支匹配：case AppId::Notepad 成立时执行下面的代码。
        case AppId::Notepad:
            // 返回 "notepad" 给调用者，表示当前函数的执行结果。
            return "notepad";
        // 分支匹配：case AppId::Gallery 成立时执行下面的代码。
        case AppId::Gallery:
            // 返回 "gallery" 给调用者，表示当前函数的执行结果。
            return "gallery";
        // 分支匹配：case AppId::Drawing 成立时执行下面的代码。
        case AppId::Drawing:
            // 返回 "drawing" 给调用者，表示当前函数的执行结果。
            return "drawing";
        // 分支匹配：case AppId::WoodenFish 成立时执行下面的代码。
        case AppId::WoodenFish:
            // 返回 "wooden_fish" 给调用者，表示当前函数的执行结果。
            return "wooden_fish";
        // 分支匹配：case AppId::Music 成立时执行下面的代码。
        case AppId::Music:
            // 返回 "music" 给调用者，表示当前函数的执行结果。
            return "music";
        // 分支匹配：case AppId::Weather 成立时执行下面的代码。
        case AppId::Weather:
            // 返回 "weather" 给调用者，表示当前函数的执行结果。
            return "weather";
        // 分支匹配：case AppId::Clock 成立时执行下面的代码。
        case AppId::Clock:
            // 返回 "clock" 给调用者，表示当前函数的执行结果。
            return "clock";
        // 分支匹配：case AppId::Calendar 成立时执行下面的代码。
        case AppId::Calendar:
            // 返回 "calendar" 给调用者，表示当前函数的执行结果。
            return "calendar";
        // 分支匹配：case AppId::Files 成立时执行下面的代码。
        case AppId::Files:
            // 返回 "files" 给调用者，表示当前函数的执行结果。
            return "files";
        // 分支匹配：case AppId::Update 成立时执行下面的代码。
        case AppId::Update:
            // 返回 "update" 给调用者，表示当前函数的执行结果。
            return "update";
        // 分支匹配：case AppId::Settings 成立时执行下面的代码。
        case AppId::Settings:
            // 返回 "settings" 给调用者，表示当前函数的执行结果。
            return "settings";
        // 分支匹配：case AppId::None 成立时执行下面的代码。
        case AppId::None:
        // 分支匹配：case AppId::Count 成立时执行下面的代码。
        case AppId::Count:
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
    // 返回 "?" 给调用者，表示当前函数的执行结果。
    return "?";
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps
