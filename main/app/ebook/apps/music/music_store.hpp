// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/music/music_store.hpp
 * 作用：保存音乐播放列表和播放偏好。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::music {

/** 记忆「上次播放的曲目索引」 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class MusicStore
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static MusicStore& get_instance();

    // 函数调用：void load();；执行该函数并处理它的返回值或副作用。
    void load();
    // 调用 save()：这是拨码映射的持久化表： load() 先设默认值，再用 NVS 覆盖合法值。 save() 把所有槽位写入 NVS 并 commit。 lookup() 按 Context 和 Key 找动作。 cycle() 在设置页循环切换到下一个可选动作。。
    void save();

    // 定义或声明函数 last_index：参数在括号里，返回类型写在函数名前面。
    uint8_t last_index() const
    {
        // 返回 last_index_ 给调用者，表示当前函数的执行结果。
        return last_index_;
    }
    // 函数调用：void set_last_index(uint8_t idx);；执行该函数并处理它的返回值或副作用。
    void set_last_index(uint8_t idx);

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 MusicStore() = default;，执行这个函数并处理它的返回值或状态变化。
    MusicStore() = default;

    // 执行语句：uint8_t last_index_{0};；它会更新一次变量、对象或程序状态。
    uint8_t last_index_{0};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::music
