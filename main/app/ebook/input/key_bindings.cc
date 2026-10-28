/**
 * [初学者导读] 文件：main/app/ebook/input/key_bindings.cc
 * 作用：保存和查询物理按键到语义动作的映射。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：中断和输入任务不能直接修改页面，只能投递结果。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/key_bindings.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/persist.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::input {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 固定参数 kCtxN = 3：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kCtxN = 3;
// 固定参数 kKeyN = 3：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kKeyN = 3;

// 静态查表数组 kNvsKeys：运行时复用固定数据，避免每次重新计算。
constexpr const char* kNvsKeys[kCtxN][kKeyN] = {
    // 执行这条代码：{"in.bg.u", "in.bg.m", "in.bg.d"},。它会更新变量、对象状态或调用链。
    {"in.bg.u", "in.bg.m", "in.bg.d"},
    // 继续上一行的表达式或参数列表：{"in.br.u", "in.br.m", "in.br.d"},。
    {"in.br.u", "in.br.m", "in.br.d"},
    // 继续上一行的表达式或参数列表：{"in.bl.u", "in.bl.m", "in.bl.d"},。
    {"in.bl.u", "in.bl.m", "in.bl.d"},
};

// 编译期数组 kReaderBindable：运行时只读的固定查表数据。
constexpr SemanticAction kReaderBindable[] = {
    // 执行这条代码：SemanticAction::None,         SemanticAction::PagePrev,       SemanticAction::PageNext,。它会更新变量、对象状态或调用链。
    SemanticAction::None,         SemanticAction::PagePrev,       SemanticAction::PageNext,
    // 继续上一行的表达式或参数列表：SemanticAction::Menu,         SemanticAction::Back,           SemanticAction::Home,。
    SemanticAction::Menu,         SemanticAction::Back,           SemanticAction::Home,
    // 继续上一行的表达式或参数列表：SemanticAction::BrightnessUp, SemanticAction::BrightnessDown, SemanticAction::VolumeUp,。
    SemanticAction::BrightnessUp, SemanticAction::BrightnessDown, SemanticAction::VolumeUp,
    // 继续上一行的表达式或参数列表：SemanticAction::VolumeDown,。
    SemanticAction::VolumeDown,
};

// 编译期数组 kListBindable：运行时只读的固定查表数据。
constexpr SemanticAction kListBindable[] = {
    // 执行这条代码：SemanticAction::None,           SemanticAction::ListUp,   SemanticAction::ListDown,。它会更新变量、对象状态或调用链。
    SemanticAction::None,           SemanticAction::ListUp,   SemanticAction::ListDown,
    // 继续上一行的表达式或参数列表：SemanticAction::Back,           SemanticAction::Home,     SemanticAction::BrightnessUp,。
    SemanticAction::Back,           SemanticAction::Home,     SemanticAction::BrightnessUp,
    // 继续上一行的表达式或参数列表：SemanticAction::BrightnessDown, SemanticAction::VolumeUp, SemanticAction::VolumeDown,。
    SemanticAction::BrightnessDown, SemanticAction::VolumeUp, SemanticAction::VolumeDown,
};

// 编译期数组 kGlobalBindable：运行时只读的固定查表数据。
constexpr SemanticAction kGlobalBindable[] = {
    // 执行这条代码：SemanticAction::None,         SemanticAction::Back,           SemanticAction::Home,。它会更新变量、对象状态或调用链。
    SemanticAction::None,         SemanticAction::Back,           SemanticAction::Home,
    // 继续上一行的表达式或参数列表：SemanticAction::BrightnessUp, SemanticAction::BrightnessDown, SemanticAction::VolumeUp,。
    SemanticAction::BrightnessUp, SemanticAction::BrightnessDown, SemanticAction::VolumeUp,
    // 继续上一行的表达式或参数列表：SemanticAction::VolumeDown,。
    SemanticAction::VolumeDown,
};

// struct BindableList：把相关数据字段组合在一起，表示项目中的一个概念。
struct BindableList
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const SemanticAction* items;
    // 执行语句：uint8_t count;；它会更新一次变量、对象或程序状态。
    uint8_t count;
};

// 定义或声明函数 bindable_list：参数在括号里，返回类型写在函数名前面。
BindableList bindable_list(InputContext ctx)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (ctx)
    {
        // 分支匹配：case InputContext::Reader 成立时执行下面的代码。
        case InputContext::Reader:
            // 返回 {kReaderBindable, 给调用者，表示当前函数的执行结果。
            return {kReaderBindable,
                    // 继续上一行的表达式或参数列表：static_cast<uint8_t>(sizeof(kReaderBindable) / sizeof(kReaderBindable[0]))};。
                    static_cast<uint8_t>(sizeof(kReaderBindable) / sizeof(kReaderBindable[0]))};
        // 分支匹配：case InputContext::List 成立时执行下面的代码。
        case InputContext::List:
            // 返回 {kListBindable, 给调用者，表示当前函数的执行结果。
            return {kListBindable,
                    // 继续上一行的表达式或参数列表：static_cast<uint8_t>(sizeof(kListBindable) / sizeof(kListBindable[0]))};。
                    static_cast<uint8_t>(sizeof(kListBindable) / sizeof(kListBindable[0]))};
        // 分支匹配：case InputContext::Global 成立时执行下面的代码。
        case InputContext::Global:
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 返回 {kGlobalBindable, 给调用者，表示当前函数的执行结果。
            return {kGlobalBindable,
                    // 继续上一行的表达式或参数列表：static_cast<uint8_t>(sizeof(kGlobalBindable) / sizeof(kGlobalBindable[0]))};。
                    static_cast<uint8_t>(sizeof(kGlobalBindable) / sizeof(kGlobalBindable[0]))};
    }
}

// 定义或声明函数 ctx_index：参数在括号里，返回类型写在函数名前面。
uint8_t ctx_index(InputContext ctx)
{
    // 返回 static_cast<uint8_t>(ctx) 给调用者，表示当前函数的执行结果。
    return static_cast<uint8_t>(ctx);
}

// 定义或声明函数 key_index：参数在括号里，返回类型写在函数名前面。
uint8_t key_index(PhysicalKey key)
{
    // 返回 static_cast<uint8_t>(key) 给调用者，表示当前函数的执行结果。
    return static_cast<uint8_t>(key);
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 KeyBindings::instance：参数在括号里，返回类型写在函数名前面。
KeyBindings& KeyBindings::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static KeyBindings s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义或声明函数 KeyBindings::apply_defaults：参数在括号里，返回类型写在函数名前面。
void KeyBindings::apply_defaults()
{
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t c = 0; c < kContextCount; ++c)
    {
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint8_t k = 0; k < kKeyCount; ++k)
            // 赋值：把右侧 static_cast<uint8_t>(SemanticAction::None) 的结果保存到左侧 table_[c][k]。
            table_[c][k] = static_cast<uint8_t>(SemanticAction::None);
    }

    // 函数调用：table_[ctx_index(InputContext::Reader)][key_index(PhysicalKey::Up)] =；执行该函数并处理它的返回值或副作用。
    table_[ctx_index(InputContext::Reader)][key_index(PhysicalKey::Up)] =
        // 继续上一行的表达式或参数列表：static_cast<uint8_t>(SemanticAction::PagePrev);。
        static_cast<uint8_t>(SemanticAction::PagePrev);
    // 函数调用：table_[ctx_index(InputContext::Reader)][key_index(PhysicalKey::Mid)] =；执行该函数并处理它的返回值或副作用。
    table_[ctx_index(InputContext::Reader)][key_index(PhysicalKey::Mid)] =
        // 继续上一行的表达式或参数列表：static_cast<uint8_t>(SemanticAction::Back);。
        static_cast<uint8_t>(SemanticAction::Back);
    // 函数调用：table_[ctx_index(InputContext::Reader)][key_index(PhysicalKey::Down)] =；执行该函数并处理它的返回值或副作用。
    table_[ctx_index(InputContext::Reader)][key_index(PhysicalKey::Down)] =
        // 继续上一行的表达式或参数列表：static_cast<uint8_t>(SemanticAction::PageNext);。
        static_cast<uint8_t>(SemanticAction::PageNext);

    // 函数调用：table_[ctx_index(InputContext::List)][key_index(PhysicalKey::Up)] =；执行该函数并处理它的返回值或副作用。
    table_[ctx_index(InputContext::List)][key_index(PhysicalKey::Up)] =
        // 继续上一行的表达式或参数列表：static_cast<uint8_t>(SemanticAction::ListUp);。
        static_cast<uint8_t>(SemanticAction::ListUp);
    // 函数调用：table_[ctx_index(InputContext::List)][key_index(PhysicalKey::Mid)] =；执行该函数并处理它的返回值或副作用。
    table_[ctx_index(InputContext::List)][key_index(PhysicalKey::Mid)] =
        // 继续上一行的表达式或参数列表：static_cast<uint8_t>(SemanticAction::Back);。
        static_cast<uint8_t>(SemanticAction::Back);
    // 函数调用：table_[ctx_index(InputContext::List)][key_index(PhysicalKey::Down)] =；执行该函数并处理它的返回值或副作用。
    table_[ctx_index(InputContext::List)][key_index(PhysicalKey::Down)] =
        // 继续上一行的表达式或参数列表：static_cast<uint8_t>(SemanticAction::ListDown);。
        static_cast<uint8_t>(SemanticAction::ListDown);
}

// [初学者导读] KeyBindings::load()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void KeyBindings::load()
{
    // 调用 apply_defaults();，执行这个函数并处理它的返回值或状态变化。
    apply_defaults();

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t c = 0; c < kContextCount; ++c)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const auto ctx = static_cast<InputContext>(c);
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint8_t k = 0; k < kKeyCount; ++k)
        {
            // 定义变量 uint8_t raw 并初始化为 0。
            uint8_t raw = 0;
            // 判断条件：data::Persist::get_u8(kNvsKeys[c][k], raw) && valid_action(raw, ctx) 是否成立。
            if (data::Persist::get_u8(kNvsKeys[c][k], raw) && valid_action(raw, ctx))
                // 赋值：把右侧 raw 的结果保存到左侧 table_[c][k]。
                table_[c][k] = raw;
        }
    }
}

// [初学者导读] KeyBindings::save()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void KeyBindings::save()
{
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t c = 0; c < kContextCount; ++c)
    {
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint8_t k = 0; k < kKeyCount; ++k)
            // 函数调用：(void)data::Persist::set_u8(kNvsKeys[c][k], table_[c][k]);；执行该函数并处理它的返回值或副作用。
            (void)data::Persist::set_u8(kNvsKeys[c][k], table_[c][k]);
    }
    // 调用 data::Persist::commit();，执行这个函数并处理它的返回值或状态变化。
    data::Persist::commit();
}

// [初学者导读] KeyBindings::restore_defaults()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void KeyBindings::restore_defaults()
{
    // 调用 apply_defaults();，执行这个函数并处理它的返回值或状态变化。
    apply_defaults();
    // 调用 save()：这是拨码映射的持久化表： load() 先设默认值，再用 NVS 覆盖合法值。 save() 把所有槽位写入 NVS 并 commit。 lookup() 按 Context 和 Key 找动作。 cycle() 在设置页循环切换到下一个可选动作。。
    save();
}

// 定义或声明函数 KeyBindings::valid_action：参数在括号里，返回类型写在函数名前面。
bool KeyBindings::valid_action(uint8_t raw, InputContext ctx) const
{
    // 返回 is_bindable(ctx, static_cast<SemanticAction>(raw)) 给调用者，表示当前函数的执行结果。
    return is_bindable(ctx, static_cast<SemanticAction>(raw));
}

// 函数定义/声明：调用 KeyBindings::is_bindable()：判断某动作是否允许出现在当前上下文。。
bool KeyBindings::is_bindable(InputContext ctx, SemanticAction action)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const BindableList list = bindable_list(ctx);
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < list.count; ++i)
    {
        // 判断条件：list.items[i] == action 是否成立。
        if (list.items[i] == action)
            // 返回 true 给调用者，表示当前函数的执行结果。
            return true;
    }
    // 返回 false 给调用者，表示当前函数的执行结果。
    return false;
}

// 函数定义/声明：调用 KeyBindings::lookup()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
SemanticAction KeyBindings::lookup(PhysicalKey key, InputContext ctx) const
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t c = ctx_index(ctx);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t k = key_index(key);
    // 判断条件：c >= kContextCount || k >= kKeyCount 是否成立。
    if (c >= kContextCount || k >= kKeyCount)
        // 返回 SemanticAction::None 给调用者，表示当前函数的执行结果。
        return SemanticAction::None;
    // 返回 static_cast<SemanticAction>(table_[c][k]) 给调用者，表示当前函数的执行结果。
    return static_cast<SemanticAction>(table_[c][k]);
}

// 函数定义/声明：调用 KeyBindings::get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
SemanticAction KeyBindings::get(InputContext ctx, PhysicalKey key) const
{
    // 返回 lookup(key, ctx) 给调用者，表示当前函数的执行结果。
    return lookup(key, ctx);
}

// 函数定义/声明：调用 KeyBindings::set()：写入映射前检查上下文、物理键和动作是否合法。。
void KeyBindings::set(InputContext ctx, PhysicalKey key, SemanticAction action)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t c = ctx_index(ctx);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t k = key_index(key);
    // 判断条件：c >= kContextCount || k >= kKeyCount 是否成立。
    if (c >= kContextCount || k >= kKeyCount)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：!is_bindable(ctx, action) 是否成立。
    if (!is_bindable(ctx, action))
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 static_cast<uint8_t>(action) 的结果保存到左侧 table_[c][k]。
    table_[c][k] = static_cast<uint8_t>(action);
}

// 函数定义/声明：调用 KeyBindings::cycle()：在当前上下文允许的动作列表中找到当前值，再切到下一个。它会保存，所以设置页面点击一次就能持久化。。
void KeyBindings::cycle(InputContext ctx, PhysicalKey key)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const BindableList list = bindable_list(ctx);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const SemanticAction cur = get(ctx, key);
    // 定义变量 uint8_t idx 并初始化为 0。
    uint8_t idx = 0;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < list.count; ++i)
    {
        // 判断条件：list.items[i] == cur 是否成立。
        if (list.items[i] == cur)
        {
            // 赋值：把右侧 static_cast<uint8_t>((i + 1U) % list.count) 的结果保存到左侧 idx。
            idx = static_cast<uint8_t>((i + 1U) % list.count);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        }
    }
    // 调用 set(ctx,key,list.items[idx])：写入映射前检查上下文、物理键和动作是否合法。。
    set(ctx, key, list.items[idx]);
    // 调用 save()：这是拨码映射的持久化表： load() 先设默认值，再用 NVS 覆盖合法值。 save() 把所有槽位写入 NVS 并 commit。 lookup() 按 Context 和 Key 找动作。 cycle() 在设置页循环切换到下一个可选动作。。
    save();
}

// 函数定义/声明：调用 KeyBindings::bindable_count()：暴露可绑定动作列表，让设置 UI 不重复维护动作数组。。
uint8_t KeyBindings::bindable_count(InputContext ctx)
{
    // 返回 bindable_list(ctx).count 给调用者，表示当前函数的执行结果。
    return bindable_list(ctx).count;
}

// 函数定义/声明：调用 KeyBindings::bindable_at()：暴露可绑定动作列表，让设置 UI 不重复维护动作数组。。
SemanticAction KeyBindings::bindable_at(InputContext ctx, uint8_t index)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const BindableList list = bindable_list(ctx);
    // 判断条件：index >= list.count 是否成立。
    if (index >= list.count)
        // 返回 SemanticAction::None 给调用者，表示当前函数的执行结果。
        return SemanticAction::None;
    // 返回 list.items[index] 给调用者，表示当前函数的执行结果。
    return list.items[index];
}

// 只读约束：这个值或对象在当前位置不能被修改。
const char* KeyBindings::action_label(SemanticAction action)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (action)
    {
        // 分支匹配：case SemanticAction::None 成立时执行下面的代码。
        case SemanticAction::None:
            // 返回 ui::strings::kKeyActNone 给调用者，表示当前函数的执行结果。
            return ui::strings::kKeyActNone;
        // 分支匹配：case SemanticAction::PagePrev 成立时执行下面的代码。
        case SemanticAction::PagePrev:
            // 返回 ui::strings::kKeyActPagePrev 给调用者，表示当前函数的执行结果。
            return ui::strings::kKeyActPagePrev;
        // 分支匹配：case SemanticAction::PageNext 成立时执行下面的代码。
        case SemanticAction::PageNext:
            // 返回 ui::strings::kKeyActPageNext 给调用者，表示当前函数的执行结果。
            return ui::strings::kKeyActPageNext;
        // 分支匹配：case SemanticAction::ListUp 成立时执行下面的代码。
        case SemanticAction::ListUp:
            // 返回 ui::strings::kKeyActListUp 给调用者，表示当前函数的执行结果。
            return ui::strings::kKeyActListUp;
        // 分支匹配：case SemanticAction::ListDown 成立时执行下面的代码。
        case SemanticAction::ListDown:
            // 返回 ui::strings::kKeyActListDown 给调用者，表示当前函数的执行结果。
            return ui::strings::kKeyActListDown;
        // 分支匹配：case SemanticAction::Back 成立时执行下面的代码。
        case SemanticAction::Back:
            // 返回 ui::strings::kKeyActBack 给调用者，表示当前函数的执行结果。
            return ui::strings::kKeyActBack;
        // 分支匹配：case SemanticAction::Menu 成立时执行下面的代码。
        case SemanticAction::Menu:
            // 返回 ui::strings::kKeyActMenu 给调用者，表示当前函数的执行结果。
            return ui::strings::kKeyActMenu;
        // 分支匹配：case SemanticAction::Home 成立时执行下面的代码。
        case SemanticAction::Home:
            // 返回 ui::strings::kKeyActHome 给调用者，表示当前函数的执行结果。
            return ui::strings::kKeyActHome;
        // 分支匹配：case SemanticAction::BrightnessUp 成立时执行下面的代码。
        case SemanticAction::BrightnessUp:
            // 返回 ui::strings::kKeyActBrightUp 给调用者，表示当前函数的执行结果。
            return ui::strings::kKeyActBrightUp;
        // 分支匹配：case SemanticAction::BrightnessDown 成立时执行下面的代码。
        case SemanticAction::BrightnessDown:
            // 返回 ui::strings::kKeyActBrightDown 给调用者，表示当前函数的执行结果。
            return ui::strings::kKeyActBrightDown;
        // 分支匹配：case SemanticAction::VolumeUp 成立时执行下面的代码。
        case SemanticAction::VolumeUp:
            // 返回 ui::strings::kKeyActVolUp 给调用者，表示当前函数的执行结果。
            return ui::strings::kKeyActVolUp;
        // 分支匹配：case SemanticAction::VolumeDown 成立时执行下面的代码。
        case SemanticAction::VolumeDown:
            // 返回 ui::strings::kKeyActVolDown 给调用者，表示当前函数的执行结果。
            return ui::strings::kKeyActVolDown;
    }
    // 返回 ui::strings::kKeyActNone 给调用者，表示当前函数的执行结果。
    return ui::strings::kKeyActNone;
}

// 只读约束：这个值或对象在当前位置不能被修改。
const char* KeyBindings::context_label(InputContext ctx)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (ctx)
    {
        // 分支匹配：case InputContext::Reader 成立时执行下面的代码。
        case InputContext::Reader:
            // 返回 ui::strings::kKeyCtxReader 给调用者，表示当前函数的执行结果。
            return ui::strings::kKeyCtxReader;
        // 分支匹配：case InputContext::List 成立时执行下面的代码。
        case InputContext::List:
            // 返回 ui::strings::kKeyCtxList 给调用者，表示当前函数的执行结果。
            return ui::strings::kKeyCtxList;
        // 分支匹配：case InputContext::Global 成立时执行下面的代码。
        case InputContext::Global:
            // 返回 ui::strings::kKeyCtxGlobal 给调用者，表示当前函数的执行结果。
            return ui::strings::kKeyCtxGlobal;
    }
    // 返回 "" 给调用者，表示当前函数的执行结果。
    return "";
}

// 只读约束：这个值或对象在当前位置不能被修改。
const char* KeyBindings::key_label(PhysicalKey key)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (key)
    {
        // 分支匹配：case PhysicalKey::Up 成立时执行下面的代码。
        case PhysicalKey::Up:
            // 返回 ui::strings::kKeySlotUp 给调用者，表示当前函数的执行结果。
            return ui::strings::kKeySlotUp;
        // 分支匹配：case PhysicalKey::Mid 成立时执行下面的代码。
        case PhysicalKey::Mid:
            // 返回 ui::strings::kKeySlotMid 给调用者，表示当前函数的执行结果。
            return ui::strings::kKeySlotMid;
        // 分支匹配：case PhysicalKey::Down 成立时执行下面的代码。
        case PhysicalKey::Down:
            // 返回 ui::strings::kKeySlotDown 给调用者，表示当前函数的执行结果。
            return ui::strings::kKeySlotDown;
    }
    // 返回 "" 给调用者，表示当前函数的执行结果。
    return "";
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::input
