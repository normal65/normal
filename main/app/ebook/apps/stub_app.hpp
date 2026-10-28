// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/stub_app.hpp
 * 作用：实现尚未完成功能的占位 App。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps {

// 类定义：把数据和操作这些数据的方法绑定在一起。
class StubApp : public App
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // constexpr 构造函数：允许对象在编译期或轻量场景中直接构造。
    constexpr StubApp(AppId id, const char* title, uint32_t icon_cp)
        // 定义或声明函数 id_：参数在括号里，返回类型写在函数名前面。
        : id_(id)
        // 定义或声明函数 title_：参数在括号里，返回类型写在函数名前面。
        , title_(title)
        // 定义或声明函数 icon_cp_：参数在括号里，返回类型写在函数名前面。
        , icon_cp_(icon_cp)
    {
    }

    // 定义或声明函数 id：参数在括号里，返回类型写在函数名前面。
    AppId id() const override
    {
        // 返回 id_ 给调用者，表示当前函数的执行结果。
        return id_;
    }
    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const char* title() const override
    {
        // 返回 title_ 给调用者，表示当前函数的执行结果。
        return title_;
    }
    // 函数定义/声明：调用 icon_cp()：返回图标 codepoint。图标通常来自字体图标表，所以这里返回的是 uint32t，不是图片路径。。
    uint32_t icon_cp() const override
    {
        // 返回 icon_cp_ 给调用者，表示当前函数的执行结果。
        return icon_cp_;
    }

    // 函数调用：void paint(gfx::Canvas& canvas) override;；执行该函数并处理它的返回值或副作用。
    void paint(gfx::Canvas& canvas) override;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 执行语句：AppId id_;；它会更新一次变量、对象或程序状态。
    AppId id_;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* title_;
    // 执行语句：uint32_t icon_cp_;；它会更新一次变量、对象或程序状态。
    uint32_t icon_cp_;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps
