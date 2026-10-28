// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/router/refresh_edges.hpp
 * 作用：根据页面跳转类型决定使用哪种刷新波形。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：路由先改页面栈，再向 Presenter 提交刷新请求；提交是异步的。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/refresh_intent.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/transition.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::router {

/** 跳转刷新边表项（见 refresh_edges.cc 中 kEdges[]） */
// struct RefreshEdge：把相关数据字段组合在一起，表示项目中的一个概念。
struct RefreshEdge
{
    // 执行语句：PageId from;；它会更新一次变量、对象或程序状态。
    PageId from;
    // 执行语句：PageId to;；它会更新一次变量、对象或程序状态。
    PageId to;
    // 执行语句：NavAction action;；它会更新一次变量、对象或程序状态。
    NavAction action;
    // 执行语句：RefreshIntent intent;；它会更新一次变量、对象或程序状态。
    RefreshIntent intent;
};

/** @brief from→to 边表查表；缺省 fallback Partial */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class RefreshEdges
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 resolve：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static RefreshIntent resolve(const Transition& t);
    // 静态函数 fallback：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static RefreshIntent fallback();
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::router
