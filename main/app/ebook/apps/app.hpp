// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/app.hpp
 * 作用：定义所有 App 的公共生命周期、绘制、输入和刷新接口。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app_id.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/page_id.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/refresh_intent.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "shell/page.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps {

/** App 基类：所有内置应用继承此类 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class App : public shell::Page
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 赋值：把右侧 0 的结果保存到左侧 virtual AppId id() const。
    virtual AppId id() const = 0;
    // 调用 title()：返回 App 标题。在文件浏览器内，标题有时会动态反映当前目录，因此可能存在一个 buffer，而不是只返回一个常量字符串。。
    virtual const char* title() const = 0;
    // 函数定义/声明：调用 icon_cp()：返回图标 codepoint。图标通常来自字体图标表，所以这里返回的是 uint32t，不是图片路径。。
    virtual uint32_t icon_cp() const
    {
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    }

    // 定义或声明函数 default_page：参数在括号里，返回类型写在函数名前面。
    virtual uint16_t default_page() const
    {
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    }

    /** 拨码「返回」：已处理则 true（如应用内上一级） */
    // 函数定义/声明：调用 on_semantic_back()：处理语义返回，顺序通常是： 1. 如果弹窗打开，先关闭弹窗。 2. 如果列表还在浏览目录，先返回上一级。 3. 如果已经到根卷，返回 false，让 Shell 退出 App。。
    virtual bool on_semantic_back()
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 调用 on_enter()：进入文件 App 时： 1. 重置浏览状态或恢复根卷状态。 2. 清空弹窗。 3. 设置加载状态。 4. 调度目录扫描。 5. 请求重绘。。
    void on_enter() override {}
    // 调用 on_exit()：退出时取消未完成的扫描、关闭弹窗，并确保后台任务不会在对象状态失效后继续写数据。。
    void on_exit() override {}

  // 访问控制：决定后面的成员能否被外部代码访问。
  protected:
    // 赋值：把右侧 nullptr) 的结果保存到左侧 void navigate_page(uint16_t page, const router::RefreshIntent* override_intent。
    void navigate_page(uint16_t page, const router::RefreshIntent* override_intent = nullptr);
    // 函数调用：void repaint(const router::RefreshIntent& intent);；执行该函数并处理它的返回值或副作用。
    void repaint(const router::RefreshIntent& intent);
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    void request_repaint();
    // 函数调用：void request_repaint(const router::RefreshIntent& intent);；执行该函数并处理它的返回值或副作用。
    void request_repaint(const router::RefreshIntent& intent);

    // 调用 request_repaint_if_ready(int64_t&last_ms,int64_tmin_interval_ms,boolforce=false)：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    bool request_repaint_if_ready(int64_t& last_ms, int64_t min_interval_ms, bool force = false);
    // 赋值：把右侧 nullptr) 的结果保存到左侧 void exit_to_parent(const router::RefreshIntent* override_intent。
    void exit_to_parent(const router::RefreshIntent* override_intent = nullptr);
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps
