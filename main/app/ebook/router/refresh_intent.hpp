// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/router/refresh_intent.hpp
 * 作用：定义 Partial、Fast、Full 刷新意图和波形名称。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：路由先改页面栈，再向 Presenter 提交刷新请求；提交是异步的。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::router {

/** 刷新波形：Partial / Fast / Full（均全屏合成，无区域裁剪） */
// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class Waveform : uint8_t
{
    // 执行这条代码：Partial,。它会更新变量、对象状态或调用链。
    Partial,
    // 继续上一行的表达式或参数列表：Fast,。
    Fast,
    // 继续上一行的表达式或参数列表：Full,。
    Full,
};

// struct RefreshIntent：把相关数据字段组合在一起，表示项目中的一个概念。
struct RefreshIntent
{
    // 执行语句：Waveform waveform{Waveform::Partial};；它会更新一次变量、对象或程序状态。
    Waveform waveform{Waveform::Partial};
};

// 定义编译期常量：值在编译阶段确定，运行时不能修改。
constexpr RefreshIntent intent_partial_full()
{
    // 返回 RefreshIntent{Waveform::Partial} 给调用者，表示当前函数的执行结果。
    return RefreshIntent{Waveform::Partial};
}

// 定义编译期常量：值在编译阶段确定，运行时不能修改。
constexpr RefreshIntent intent_fast_full()
{
    // 返回 RefreshIntent{Waveform::Fast} 给调用者，表示当前函数的执行结果。
    return RefreshIntent{Waveform::Fast};
}

// 定义编译期常量：值在编译阶段确定，运行时不能修改。
constexpr RefreshIntent intent_full_full()
{
    // 返回 RefreshIntent{Waveform::Full} 给调用者，表示当前函数的执行结果。
    return RefreshIntent{Waveform::Full};
}

// 只读约束：这个值或对象在当前位置不能被修改。
const char* waveform_name(Waveform w);

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::router
