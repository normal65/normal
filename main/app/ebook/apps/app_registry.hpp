// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/app_registry.hpp
 * 作用：登记所有 App，并负责打开、关闭和查找应用。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps {

// struct AppEntry：把相关数据字段组合在一起，表示项目中的一个概念。
struct AppEntry
{
    // 执行语句：AppId id;；它会更新一次变量、对象或程序状态。
    AppId id;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* title;
    // 执行语句：uint32_t icon_cp;；它会更新一次变量、对象或程序状态。
    uint32_t icon_cp;
};

// 类定义：把数据和操作这些数据的方法绑定在一起。
class AppRegistry
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 固定参数 kMaxApps = 16：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kMaxApps = 16;

    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static AppRegistry& instance();

    // 调用 register_app(App*app)：登记应用对象。
    void register_app(App* app);
    // 调用 register_defaults()：这里按固定顺序注册 Reader、Notepad、Gallery、Drawing、Music、Weather、Clock、Calendar、WoodenFish、Files、Update Stub、Settings。。
    void register_defaults();

    // 调用 find(AppIdid)：find(id) 根据 AppId 找对象；entryat(idx) 根据网格索引返回 {id, title, iconcp} 这样的轻量条目。。
    App* find(AppId id);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const App* find(AppId id) const;

    // 赋值：把右侧 0) 的结果保存到左侧 bool open(AppId id, uint16_t page。
    bool open(AppId id, uint16_t page = 0);
    // 调用 close_active()：关闭当前 App 时： 1. 调用活动 App 的 onexit()。 2. 清空 active。 3. 把页面编号清回 0。。
    void close_active();

    // 定义或声明函数 active：参数在括号里，返回类型写在函数名前面。
    App* active()
    {
        // 返回 active_ 给调用者，表示当前函数的执行结果。
        return active_;
    }
    // 定义或声明函数 active_id：参数在括号里，返回类型写在函数名前面。
    AppId active_id() const
    {
        // 返回 active_ != nullptr ? active_->id() : AppId::None 给调用者，表示当前函数的执行结果。
        return active_ != nullptr ? active_->id() : AppId::None;
    }
    // 定义或声明函数 active_page：参数在括号里，返回类型写在函数名前面。
    uint16_t active_page() const
    {
        // 返回 active_page_ 给调用者，表示当前函数的执行结果。
        return active_page_;
    }
    // 定义或声明函数 is_active：参数在括号里，返回类型写在函数名前面。
    bool is_active() const
    {
        // 返回 active_ != nullptr 给调用者，表示当前函数的执行结果。
        return active_ != nullptr;
    }

    // 定义或声明函数 entry_count：参数在括号里，返回类型写在函数名前面。
    uint8_t entry_count() const
    {
        // 返回 count_ 给调用者，表示当前函数的执行结果。
        return count_;
    }
    // 调用 entry_at(uint8_tidx)：返回指定下标的 Entry 引用。它不做复杂的排序，只负责读取。调用前通常要先通过 ensureentries() 保证目录已经打开。。
    AppEntry entry_at(uint8_t idx) const;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 AppRegistry() = default;，执行这个函数并处理它的返回值或状态变化。
    AppRegistry() = default;

    // 执行语句：App* apps_[kMaxApps]{};；它会更新一次变量、对象或程序状态。
    App* apps_[kMaxApps]{};
    // 执行语句：uint8_t count_{0};；它会更新一次变量、对象或程序状态。
    uint8_t count_{0};
    // 执行语句：App* active_{nullptr};；它会更新一次变量、对象或程序状态。
    App* active_{nullptr};
    // 执行语句：uint16_t active_page_{0};；它会更新一次变量、对象或程序状态。
    uint16_t active_page_{0};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps
