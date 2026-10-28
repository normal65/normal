/**
 * [初学者导读] 文件：main/app/ebook/shell/app_grid_page.cc
 * 作用：实现 shell 模块中的 app grid page 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：Shell 是 App 之外的外壳页面，负责统一返回和输入兜底。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "shell/app_grid_page.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app_registry.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/canvas.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/control_center.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/page_id.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/refresh_intent.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/router.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::shell {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;

// 定义或声明函数 viewport_rect：参数在括号里，返回类型写在函数名前面。
core::Rect viewport_rect()
{
    // 返回 core::Rect{0, static_cast<int16_t>(Theme::kAppGridY), Theme::kScreenW, 给调用者，表示当前函数的执行结果。
    return core::Rect{0, static_cast<int16_t>(Theme::kAppGridY), Theme::kScreenW,
                      // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kScreenH - Theme::kAppGridY)};。
                      static_cast<uint16_t>(Theme::kScreenH - Theme::kAppGridY)};
}

// 定义或声明函数 visible_rows：参数在括号里，返回类型写在函数名前面。
uint8_t visible_rows()
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t r = viewport_rect().h / Theme::kAppGridCellH;
    // 返回 (r == 0) ? 1U : static_cast<uint8_t>(r) 给调用者，表示当前函数的执行结果。
    return (r == 0) ? 1U : static_cast<uint8_t>(r);
}

// 定义或声明函数 total_rows：参数在括号里，返回类型写在函数名前面。
uint8_t total_rows(uint8_t count)
{
    // 返回 static_cast<uint8_t>((count + Theme::kAppGridCols - 1) / Theme::kAppGridCols) 给调用者，表示当前函数的执行结果。
    return static_cast<uint8_t>((count + Theme::kAppGridCols - 1) / Theme::kAppGridCols);
}

// 定义或声明函数 max_scroll：参数在括号里，返回类型写在函数名前面。
uint8_t max_scroll(uint8_t count)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t t = total_rows(count);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t v = visible_rows();
    // 返回 (t > v) ? static_cast<uint8_t>(t - v) : 0 给调用者，表示当前函数的执行结果。
    return (t > v) ? static_cast<uint8_t>(t - v) : 0;
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 AppGridPage::instance：参数在括号里，返回类型写在函数名前面。
AppGridPage& AppGridPage::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static AppGridPage s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// [初学者导读] AppGridPage::on_enter()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void AppGridPage::on_enter()
{
    // 赋值：把右侧 0 的结果保存到左侧 scroll_row_。
    scroll_row_ = 0;
}

// 函数定义/声明：调用 AppGridPage::paint()：绘制步骤： 1. 画标题工具栏。 2. 读取注册表条目数。 3. 把滚动位置限制在合法范围。 4. 计算可视区域和是否需要滚动条。 5. 按“可视行号”循环。 6. 把可视行号加 scrollrow 换算为真实行号。 7. 再换算为注册表索引。 8. 画格子、图标和标题。 9. 最后画滚动条。。
void AppGridPage::paint(gfx::Canvas& c)
{
    // 调用 ui::widgets::toolbar(c, ui::strings::kAppsTitle);，执行这个函数并处理它的返回值或状态变化。
    ui::widgets::toolbar(c, ui::strings::kAppsTitle);

    // 赋值：把右侧 apps::AppRegistry::instance() 的结果保存到左侧 auto& reg。
    auto& reg = apps::AppRegistry::instance();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t count = reg.entry_count();
    // 判断条件：count == 0 是否成立。
    if (count == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t vis = visible_rows();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t rmax = max_scroll(count);
    // 判断条件：scroll_row_ > rmax 是否成立。
    if (scroll_row_ > rmax)
        // 赋值：把右侧 rmax 的结果保存到左侧 scroll_row_。
        scroll_row_ = rmax;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect vp = viewport_rect();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool needs_scroll = total_rows(count) > vis;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t reserve = needs_scroll ? 6U : 0U;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect view_area{vp.x, vp.y, static_cast<uint16_t>(vp.w - reserve),
                               // 继续上一行的表达式或参数列表：static_cast<uint16_t>(vis * Theme::kAppGridCellH)};。
                               static_cast<uint16_t>(vis * Theme::kAppGridCellH)};

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t vr = 0; vr < vis; ++vr)
    {
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint8_t col = 0; col < Theme::kAppGridCols; ++col)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint8_t real_row = static_cast<uint8_t>(scroll_row_ + vr);
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint8_t idx = static_cast<uint8_t>(real_row * Theme::kAppGridCols + col);
            // 判断条件：idx >= count 是否成立。
            if (idx >= count)
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;

            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint8_t view_idx = static_cast<uint8_t>(vr * Theme::kAppGridCols + col);
            // 只读约束：这个值或对象在当前位置不能被修改。
            const core::Rect cell =
                // 继续上一行的表达式或参数列表：ui::widgets::grid_cell(view_area, Theme::kAppGridCols, vis, view_idx, Theme::kPad);。
                ui::widgets::grid_cell(view_area, Theme::kAppGridCols, vis, view_idx, Theme::kPad);

            // 只读约束：这个值或对象在当前位置不能被修改。
            const apps::AppEntry e = reg.entry_at(idx);
            // 调用 rect(cell)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
            c.rect(cell);

            // 判断条件：e.icon_cp != 0 是否成立。
            if (e.icon_cp != 0)
            {
                // 只读约束：这个值或对象在当前位置不能被修改。
                const int16_t ix =
                    // 继续上一行的表达式或参数列表：static_cast<int16_t>(cell.x + (cell.w - Theme::kAppIconSize) / 2);。
                    static_cast<int16_t>(cell.x + (cell.w - Theme::kAppIconSize) / 2);
                // 只读约束：这个值或对象在当前位置不能被修改。
                const int16_t iy = static_cast<int16_t>(cell.y + 6);
                // 访问对象成员：c.glyph(ix, iy, e.icon_cp, Theme::kAppIconSize, gfx::FontFace::Icon);；读取或修改这个对象里的成员状态。
                c.glyph(ix, iy, e.icon_cp, Theme::kAppIconSize, gfx::FontFace::Icon);
            }

            // 只读约束：这个值或对象在当前位置不能被修改。
            const int16_t lbl_y = static_cast<int16_t>(cell.y + 6 + Theme::kAppIconSize + 2);
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint16_t lbl_h =
                // 继续上一行的表达式或参数列表：static_cast<uint16_t>(cell.h - static_cast<uint16_t>(lbl_y - cell.y) - 2);。
                static_cast<uint16_t>(cell.h - static_cast<uint16_t>(lbl_y - cell.y) - 2);
            // 只读约束：这个值或对象在当前位置不能被修改。
            const core::Rect lbl{cell.x, lbl_y, cell.w, lbl_h};
            // 声明并默认初始化对象：gfx::Canvas::TextStyle s{}。
            gfx::Canvas::TextStyle s{};
            // 赋值：把右侧 Theme::kFontSmall 的结果保存到左侧 s.size_px。
            s.size_px = Theme::kFontSmall;
            // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 s.h。
            s.h = gfx::HAlign::Center;
            // 赋值：把右侧 gfx::VAlign::Top 的结果保存到左侧 s.v。
            s.v = gfx::VAlign::Top;
            // 赋值：把右侧 2 的结果保存到左侧 s.padding。
            s.padding = 2;
            // 调用 text_in(lbl,e.title,s)：文本对齐。
            c.text_in(lbl, e.title, s);
        }
    }

    // 判断条件：needs_scroll 是否成立。
    if (needs_scroll)
    {
        // 调用 ui::widgets::scrollbar(c, static_cast<uint16_t>(vp.y), vp.h, scroll_row_, total_rows(count),，执行这个函数并处理它的返回值或状态变化。
        ui::widgets::scrollbar(c, static_cast<uint16_t>(vp.y), vp.h, scroll_row_, total_rows(count),
                               // 继续上一行的表达式或参数列表：vis);。
                               vis);
    }
}

// 函数定义/声明：调用 AppGridPage::on_input()：顺序： 1. 状态栏下拉打开控制中心。 2. 在工具栏以下区域，上滑增加 scrollrow，下滑减少。 3. 点击工具栏返回键，走 Router 返回。 4. 其余点击换算为可视格子索引。 5. 再把可视格子换算成真实注册索引。 6. 读取对应 AppId，使用 pageapp(id, 0) 导航。。
InputResult AppGridPage::on_input(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;
    // 赋值：把右侧 apps::AppRegistry::instance() 的结果保存到左侧 auto& reg。
    auto& reg = apps::AppRegistry::instance();
    // 赋值：把右侧 router::Router::instance() 的结果保存到左侧 auto& r。
    auto& r = router::Router::instance();

    // 判断条件：ev.type == EventType::SwipeDown && ev.start_y <= Theme::kStatusBarH 是否成立。
    if (ev.type == EventType::SwipeDown && ev.start_y <= Theme::kStatusBarH)
    {
        // 调用 open()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
        overlays::ControlCenter::instance().open();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t count = reg.entry_count();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t vis = visible_rows();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t rmax = max_scroll(count);

    // 判断条件：ev.start_y > Theme::kStatusBarH + Theme::kToolbarH 是否成立。
    if (ev.start_y > Theme::kStatusBarH + Theme::kToolbarH)
    {
        // 判断条件：ev.type == EventType::SwipeUp && scroll_row_ < rmax 是否成立。
        if (ev.type == EventType::SwipeUp && scroll_row_ < rmax)
        {
            // 执行语句：++scroll_row_;；它会更新一次变量、对象或程序状态。
            ++scroll_row_;
            // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
            (void)r.repaint(router::intent_partial_full());
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        }
        // 判断条件：ev.type == EventType::SwipeDown && scroll_row_ > 0 是否成立。
        if (ev.type == EventType::SwipeDown && scroll_row_ > 0)
        {
            // 执行语句：--scroll_row_;；它会更新一次变量、对象或程序状态。
            --scroll_row_;
            // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
            (void)r.repaint(router::intent_partial_full());
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        }
    }

    // 判断条件：ev.type == EventType::Tap & 是否成立。
    if (ev.type == EventType::Tap &&
        // 函数定义/声明：调用 ui::widgets::hit_toolbar_back()：toolbar()： 在固定工具栏矩形中画返回图标、标题和底部横线。返回图标使用 Font Awesome 的码点，而不是手绘 SVG。。
        ui::widgets::hit_toolbar_back(static_cast<int16_t>(ev.x), static_cast<int16_t>(ev.y)))
    {
        // 调用 back()：如果最上面是 overlay，优先关闭 overlay。否则计算上一层 shell；如果当前在 AppHost，会先关闭活动 App，再提交返回转换。。
        (void)r.back();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 判断条件：ev.type != EventType::Tap 是否成立。
    if (ev.type != EventType::Tap)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect vp = viewport_rect();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect view_area{vp.x, vp.y, vp.w, static_cast<uint16_t>(vis * Theme::kAppGridCellH)};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int8_t view_idx =
        // 继续上一行的表达式或参数列表：ui::widgets::grid_hit_test(view_area, Theme::kAppGridCols, vis, static_cast<int16_t>(ev.x),。
        ui::widgets::grid_hit_test(view_area, Theme::kAppGridCols, vis, static_cast<int16_t>(ev.x),
                                   // 继续上一行的表达式或参数列表：static_cast<int16_t>(ev.y), Theme::kPad);。
                                   static_cast<int16_t>(ev.y), Theme::kPad);
    // 判断条件：view_idx < 0 是否成立。
    if (view_idx < 0)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t vr = static_cast<uint8_t>(view_idx / Theme::kAppGridCols);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t col = static_cast<uint8_t>(view_idx % Theme::kAppGridCols);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t real_idx = static_cast<uint8_t>((scroll_row_ + vr) * Theme::kAppGridCols + col);
    // 判断条件：real_idx >= count 是否成立。
    if (real_idx >= count)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const apps::AppEntry e = reg.entry_at(real_idx);
    // 判断条件：e.id == apps::AppId::None 是否成立。
    if (e.id == apps::AppId::None)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 访问对象成员：(void)r.navigate(router::page_app(e.id, 0));；读取或修改这个对象里的成员状态。
    (void)r.navigate(router::page_app(e.id, 0));
    // 返回 {true} 给调用者，表示当前函数的执行结果。
    return {true};
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::shell
