/**
 * [初学者导读] 文件：main/app/ebook/ui/list_view.cc
 * 作用：实现通用列表控件的绘制、滚动、命中测试和行提供者回调。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：UI 状态只能在 UiLoop/UI 线程修改，跨任务结果通过 UiBus 回 UI。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/list_view.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::ui {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 定义或声明函数 effective_area：参数在括号里，返回类型写在函数名前面。
core::Rect effective_area(const core::Rect& a)
{
    // 返回 a.empty() ? Theme::list_region() : a 给调用者，表示当前函数的执行结果。
    return a.empty() ? Theme::list_region() : a;
}

// 定义或声明函数 visible_rows：参数在括号里，返回类型写在函数名前面。
uint8_t visible_rows(const core::Rect& a)
{
    // 判断条件：a.h == 0 是否成立。
    if (a.h == 0)
        // 返回 Theme::kListVisibleRows 给调用者，表示当前函数的执行结果。
        return Theme::kListVisibleRows;
    // 返回 static_cast<uint8_t>(a.h / Theme::kListRowH) 给调用者，表示当前函数的执行结果。
    return static_cast<uint8_t>(a.h / Theme::kListRowH);
}

// 定义或声明函数 cap_scroll：参数在括号里，返回类型写在函数名前面。
uint8_t cap_scroll(uint8_t total, uint8_t visible)
{
    // 返回 (total > visible) ? static_cast<uint8_t>(total - visible) : 0 给调用者，表示当前函数的执行结果。
    return (total > visible) ? static_cast<uint8_t>(total - visible) : 0;
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 ListView::set_total：参数在括号里，返回类型写在函数名前面。
void ListView::set_total(uint8_t total)
{
    // 赋值：把右侧 total 的结果保存到左侧 total_。
    total_ = total;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t mx = cap_scroll(total_, visible_rows(effective_area(area_)));
    // 判断条件：scroll_ > mx 是否成立。
    if (scroll_ > mx)
        // 赋值：把右侧 mx 的结果保存到左侧 scroll_。
        scroll_ = mx;
}

// 定义或声明函数 ListView::set_scroll：参数在括号里，返回类型写在函数名前面。
void ListView::set_scroll(uint8_t s)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t mx = cap_scroll(total_, visible_rows(effective_area(area_)));
    // 赋值：把右侧 (s > mx) ? mx : s 的结果保存到左侧 scroll_。
    scroll_ = (s > mx) ? mx : s;
}

// 定义或声明函数 ListView::paint：参数在括号里，返回类型写在函数名前面。
void ListView::paint(gfx::Canvas& c)
{
    // 判断条件：!provider_ 是否成立。
    if (!provider_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect area = effective_area(area_);
    // 判断条件：area.empty() 是否成立。
    if (area.empty())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t visible = visible_rows(area);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t end = static_cast<uint8_t>(scroll_ + visible);

    // 定义变量 int16_t y 并初始化为 area.y。
    int16_t y = area.y;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = scroll_; i < end && i < total_; ++i)
    {
        // 声明并默认初始化对象：widgets::RowStyle s{}。
        widgets::RowStyle s{};
        // 调用 provider_(i, s);，执行这个函数并处理它的返回值或状态变化。
        provider_(i, s);

        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect row{area.x, y, static_cast<uint16_t>(area.w - Theme::kScrollbarW - 2),
                             // 继续上一行的表达式或参数列表：Theme::kListRowH};。
                             Theme::kListRowH};
        // 调用 widgets::list_row(c,row,s)：RowStyle 描述一行可能有的元素： 左侧图标。 主标签。 右侧文本或值图标。 最右侧动作图标。 是否显示 chevron。 是否反色。。
        widgets::list_row(c, row, s);
        // 赋值：把右侧 static_cast<int16_t>(y + Theme::kListRowH) 的结果保存到左侧 y。
        y = static_cast<int16_t>(y + Theme::kListRowH);
    }

    // 调用 widgets::scrollbar(c, static_cast<uint16_t>(area.y), area.h, scroll_, total_, visible);，执行这个函数并处理它的返回值或状态变化。
    widgets::scrollbar(c, static_cast<uint16_t>(area.y), area.h, scroll_, total_, visible);
}

// 定义或声明函数 ListView::handle_input：参数在括号里，返回类型写在函数名前面。
ListView::InputOutcome ListView::handle_input(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;

    // 声明并默认初始化对象：InputOutcome out{}。
    InputOutcome out{};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect area = effective_area(area_);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t visible = visible_rows(area);

    // 判断条件：ev.type == EventType::Tap 是否成立。
    if (ev.type == EventType::Tap)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t x = static_cast<int16_t>(ev.x);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t y = static_cast<int16_t>(ev.y);
        // 判断条件：!area.contains(x, y) 是否成立。
        if (!area.contains(x, y))
            // 返回 out 给调用者，表示当前函数的执行结果。
            return out;

        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t local = static_cast<uint8_t>((y - area.y) / Theme::kListRowH);
        // 判断条件：local >= visible 是否成立。
        if (local >= visible)
            // 返回 out 给调用者，表示当前函数的执行结果。
            return out;

        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t idx = static_cast<uint8_t>(scroll_ + local);
        // 判断条件：idx >= total_ 是否成立。
        if (idx >= total_)
            // 返回 out 给调用者，表示当前函数的执行结果。
            return out;

        // 赋值：把右侧 true 的结果保存到左侧 out.consumed。
        out.consumed = true;
        // 赋值：把右侧 true 的结果保存到左侧 out.tap_consumed。
        out.tap_consumed = true;
        // 判断条件：tap_ 是否成立。
        if (tap_)
            // 调用 tap_(idx);，执行这个函数并处理它的返回值或状态变化。
            tap_(idx);
        // 返回 out 给调用者，表示当前函数的执行结果。
        return out;
    }

    // 判断条件：ev.type == EventType::SwipeUp || ev.type == EventType::SwipeDown 是否成立。
    if (ev.type == EventType::SwipeUp || ev.type == EventType::SwipeDown)
    {
        // 判断条件：!area.contains(static_cast<int16_t>(ev.start_x), static_cast<int16_t>(ev.start_y)) 是否成立。
        if (!area.contains(static_cast<int16_t>(ev.start_x), static_cast<int16_t>(ev.start_y)))
            // 返回 out 给调用者，表示当前函数的执行结果。
            return out;

        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t mx = cap_scroll(total_, visible);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t old = scroll_;
        // 判断条件：ev.type == EventType::SwipeUp && scroll_ < mx 是否成立。
        if (ev.type == EventType::SwipeUp && scroll_ < mx)
            // 执行语句：++scroll_;；它会更新一次变量、对象或程序状态。
            ++scroll_;
        // 备用分支：前面的条件不成立时执行这里。
        else if (ev.type == EventType::SwipeDown && scroll_ > 0)
            // 执行语句：--scroll_;；它会更新一次变量、对象或程序状态。
            --scroll_;

        // 赋值：把右侧 true 的结果保存到左侧 out.consumed。
        out.consumed = true;
        // 赋值：把右侧 (scroll_ != old) 的结果保存到左侧 out.scroll_changed。
        out.scroll_changed = (scroll_ != old);
        // 返回 out 给调用者，表示当前函数的执行结果。
        return out;
    }

    // 返回 out 给调用者，表示当前函数的执行结果。
    return out;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::ui
