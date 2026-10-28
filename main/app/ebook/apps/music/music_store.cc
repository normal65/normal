/**
 * [初学者导读] 文件：main/app/ebook/apps/music/music_store.cc
 * 作用：保存音乐播放列表和播放偏好。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/music/music_store.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/persist.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::music {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {
// 固定参数 kKLastIndex = "music.idx"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kKLastIndex = "music.idx";
}

// 定义或声明函数 MusicStore::get_instance：参数在括号里，返回类型写在函数名前面。
MusicStore& MusicStore::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static MusicStore s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义或声明函数 MusicStore::load：参数在括号里，返回类型写在函数名前面。
void MusicStore::load()
{
    // 定义变量 uint8_t idx 并初始化为 0。
    uint8_t idx = 0;
    // 判断条件：data::Persist::get_u8(kKLastIndex, idx) 是否成立。
    if (data::Persist::get_u8(kKLastIndex, idx))
        // 赋值：把右侧 idx 的结果保存到左侧 last_index_。
        last_index_ = idx;
}

// 定义或声明函数 MusicStore::save：参数在括号里，返回类型写在函数名前面。
void MusicStore::save()
{
    // 函数调用：(void)data::Persist::set_u8(kKLastIndex, last_index_);；执行该函数并处理它的返回值或副作用。
    (void)data::Persist::set_u8(kKLastIndex, last_index_);
    // 调用 data::Persist::commit();，执行这个函数并处理它的返回值或状态变化。
    data::Persist::commit();
}

// 定义或声明函数 MusicStore::set_last_index：参数在括号里，返回类型写在函数名前面。
void MusicStore::set_last_index(uint8_t idx)
{
    // 判断条件：idx == last_index_ 是否成立。
    if (idx == last_index_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 idx 的结果保存到左侧 last_index_。
    last_index_ = idx;
    // 调用 save()：这是拨码映射的持久化表： load() 先设默认值，再用 NVS 覆盖合法值。 save() 把所有槽位写入 NVS 并 commit。 lookup() 按 Context 和 Key 找动作。 cycle() 在设置页循环切换到下一个可选动作。。
    save();
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::music
