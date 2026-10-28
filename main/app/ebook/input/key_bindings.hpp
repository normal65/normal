// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/input/key_bindings.hpp
 * 作用：保存和查询物理按键到语义动作的映射。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：中断和输入任务不能直接修改页面，只能投递结果。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/input_bindings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/physical_types.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::input {

/**
 * @brief 拨码映射表（NVS）；Context × Key → SemanticAction
 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class KeyBindings
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static KeyBindings& instance();

    // 函数调用：void load();；执行该函数并处理它的返回值或副作用。
    void load();
    // 调用 restore_defaults()：恢复默认映射并保存。。
    void restore_defaults();
    // 调用 save()：这是拨码映射的持久化表： load() 先设默认值，再用 NVS 覆盖合法值。 save() 把所有槽位写入 NVS 并 commit。 lookup() 按 Context 和 Key 找动作。 cycle() 在设置页循环切换到下一个可选动作。。
    void save();

    // 函数调用：SemanticAction lookup(PhysicalKey key, InputContext ctx) const;；执行该函数并处理它的返回值或副作用。
    SemanticAction lookup(PhysicalKey key, InputContext ctx) const;

    // 调用 get(InputContextctx,PhysicalKeykey)：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
    SemanticAction get(InputContext ctx, PhysicalKey key) const;
    // 调用 set(InputContextctx,PhysicalKeykey,SemanticActionaction)：写入映射前检查上下文、物理键和动作是否合法。。
    void set(InputContext ctx, PhysicalKey key, SemanticAction action);
    // 函数调用：void cycle(InputContext ctx, PhysicalKey key);；执行该函数并处理它的返回值或副作用。
    void cycle(InputContext ctx, PhysicalKey key);

    // 静态函数 bindable_count：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static uint8_t bindable_count(InputContext ctx);
    // 静态函数 bindable_at：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static SemanticAction bindable_at(InputContext ctx, uint8_t index);
    // 静态函数 is_bindable：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool is_bindable(InputContext ctx, SemanticAction action);
    // 静态函数 action_label：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static const char* action_label(SemanticAction action);

    // 静态函数 context_label：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static const char* context_label(InputContext ctx);
    // 静态函数 key_label：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static const char* key_label(PhysicalKey key);

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 KeyBindings() = default;，执行这个函数并处理它的返回值或状态变化。
    KeyBindings() = default;

    // 固定参数 kContextCount = 3：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kContextCount = 3;
    // 固定参数 kKeyCount = 3：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kKeyCount = 3;

    // 函数调用：void apply_defaults();；执行该函数并处理它的返回值或副作用。
    void apply_defaults();
    // 函数调用：bool valid_action(uint8_t raw, InputContext ctx) const;；执行该函数并处理它的返回值或副作用。
    bool valid_action(uint8_t raw, InputContext ctx) const;

    // 执行语句：uint8_t table_[kContextCount][kKeyCount]{};；它会更新一次变量、对象或程序状态。
    uint8_t table_[kContextCount][kKeyCount]{};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::input
