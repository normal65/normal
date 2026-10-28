// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/settings/auto_lock.hpp
 * 作用：根据空闲时间判断何时发出自动锁屏提示。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstddef>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::settings {

/**
 * @brief 自动锁屏（NVS: disp.autolock.on / disp.autolock.min）
 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class AutoLock
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static AutoLock& get_instance();

    // 函数调用：void load();；执行该函数并处理它的返回值或副作用。
    void load();

    // 定义或声明函数 enabled：参数在括号里，返回类型写在函数名前面。
    bool enabled() const
    {
        // 返回 enabled_ 给调用者，表示当前函数的执行结果。
        return enabled_;
    }
    // 调用 timeout_min()：返回当前预设对应的分钟数。。
    uint8_t timeout_min() const;
    // 调用 preset_count()：返回预设数量，供设置页面循环。。
    uint8_t preset_count() const;
    // 调用 preset_index()：返回当前预设下标。。
    uint8_t preset_index() const;

    // 函数调用：void set_enabled(bool on);；执行该函数并处理它的返回值或副作用。
    void set_enabled(bool on);
    // 调用 cycle_timeout_preset()：循环到下一个时间预设并保存。显示设置页调用它改变时间。。
    void cycle_timeout_preset();

    // 调用 notify_activity()：记录用户活动时间。触摸、按键或页面交互后应该调用它，否则用户刚操作完也可能被锁屏。。
    void notify_activity();
    // 函数调用：void tick();；执行该函数并处理它的返回值或副作用。
    void tick();

    // 调用 format_timeout(char*buf,size_tcap)：把分钟数格式化成界面文字。必须有长度上限，不能直接拼接未知长度字符串。。
    void format_timeout(char* buf, size_t cap) const;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 AutoLock() = default;，执行这个函数并处理它的返回值或状态变化。
    AutoLock() = default;

    // 静态函数 now_ms：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static uint32_t now_ms();

    // 执行语句：bool enabled_{false};；它会更新一次变量、对象或程序状态。
    bool enabled_{false};
    // 执行语句：uint8_t preset_idx_{2};；它会更新一次变量、对象或程序状态。
    uint8_t preset_idx_{2};
    // 执行语句：uint32_t last_activity_ms_{0};；它会更新一次变量、对象或程序状态。
    uint32_t last_activity_ms_{0};
    // 执行语句：bool lock_hint_sent_{false};；它会更新一次变量、对象或程序状态。
    bool lock_hint_sent_{false};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::settings
