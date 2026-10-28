// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/presenter/frame_request.hpp
 * 作用：定义一次待刷新请求的页面、动作、波形和序号。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：Presenter 队列有容量限制，提交失败要调用方处理。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/refresh_intent.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/transition.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::presenter {

/** 提交给 Presenter 的一帧请求 */
// struct FrameRequest：把相关数据字段组合在一起，表示项目中的一个概念。
struct FrameRequest
{
    // 声明并默认初始化对象：router::Transition transition{}。
    router::Transition transition{};
    // 声明并默认初始化对象：router::RefreshIntent intent{}。
    router::RefreshIntent intent{};
    // 执行语句：uint32_t seq{0};；它会更新一次变量、对象或程序状态。
    uint32_t seq{0};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::presenter
