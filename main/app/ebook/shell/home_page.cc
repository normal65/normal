/**
 * [初学者导读] 文件：main/app/ebook/shell/home_page.cc
 * 作用：实现 shell 模块中的 home page 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：Shell 是 App 之外的外壳页面，负责统一返回和输入兜底。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "shell/home_page.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/reader_app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/reading_store.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/icon.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/control_center.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/page_id.hpp"
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

// 类型别名引入：把 apps::AppId 直接导入当前命名空间，后面写代码不用每次写完整长名字。
using apps::AppId;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;

// struct QuickItem：把相关数据字段组合在一起，表示项目中的一个概念。
struct QuickItem
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* label;
    // 执行语句：uint32_t icon;；它会更新一次变量、对象或程序状态。
    uint32_t icon;
    // 执行语句：AppId target;；它会更新一次变量、对象或程序状态。
    AppId target;
};

// 编译期数组 kQuick：运行时只读的固定查表数据。
constexpr QuickItem kQuick[] = {
    // 执行这条代码：{ui::strings::kAppReader, gfx::icon::kFaBookReader, AppId::Reader},。它会更新变量、对象状态或调用链。
    {ui::strings::kAppReader, gfx::icon::kFaBookReader, AppId::Reader},
    // 继续上一行的表达式或参数列表：{ui::strings::kAppNotepad, gfx::icon::kFaStickyNote, AppId::Notepad},。
    {ui::strings::kAppNotepad, gfx::icon::kFaStickyNote, AppId::Notepad},
    // 继续上一行的表达式或参数列表：{ui::strings::kAppFish, gfx::icon::kEbWoodenFish, AppId::WoodenFish},。
    {ui::strings::kAppFish, gfx::icon::kEbWoodenFish, AppId::WoodenFish},
    // 继续上一行的表达式或参数列表：{ui::strings::kAppMore, gfx::icon::kFaEllipsisH, AppId::None},。
    {ui::strings::kAppMore, gfx::icon::kFaEllipsisH, AppId::None},
};

// 定义或声明函数 reading_card_rect：参数在括号里，返回类型写在函数名前面。
core::Rect reading_card_rect()
{
    // 返回 core::Rect{Theme::kPadLg, static_cast<int16_t>(Theme::kReadingCardY), 给调用者，表示当前函数的执行结果。
    return core::Rect{Theme::kPadLg, static_cast<int16_t>(Theme::kReadingCardY),
                      // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kScreenW - Theme::kPadLg * 2),。
                      static_cast<uint16_t>(Theme::kScreenW - Theme::kPadLg * 2),
                      // 继续上一行的表达式或参数列表：Theme::kReadingCardH};。
                      Theme::kReadingCardH};
}

// 定义或声明函数 reading_btn_rect：参数在括号里，返回类型写在函数名前面。
core::Rect reading_btn_rect()
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect card = reading_card_rect();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t bw = static_cast<uint16_t>(card.w - 16);
    // 返回 core::Rect{static_cast<int16_t>(card.x + 8), 给调用者，表示当前函数的执行结果。
    return core::Rect{static_cast<int16_t>(card.x + 8),
                      // 继续上一行的表达式或参数列表：static_cast<int16_t>(card.y + card.h - Theme::kReadingBtnH - 6), bw,。
                      static_cast<int16_t>(card.y + card.h - Theme::kReadingBtnH - 6), bw,
                      // 继续上一行的表达式或参数列表：Theme::kReadingBtnH};。
                      Theme::kReadingBtnH};
}

// 定义或声明函数 quick_area_rect：参数在括号里，返回类型写在函数名前面。
core::Rect quick_area_rect()
{
    // 返回 core::Rect{0, static_cast<int16_t>(Theme::kQuickY), Theme::kScreenW, 给调用者，表示当前函数的执行结果。
    return core::Rect{0, static_cast<int16_t>(Theme::kQuickY), Theme::kScreenW,
                      // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kQuickRows * Theme::kQuickRowH)};。
                      static_cast<uint16_t>(Theme::kQuickRows * Theme::kQuickRowH)};
}

// 定义或声明函数 paint_reading_card：参数在括号里，返回类型写在函数名前面。
void paint_reading_card(gfx::Canvas& c)
{
    // 赋值：把右侧 apps::reader::ReadingStore::get_instance() 的结果保存到左侧 auto& store。
    auto& store = apps::reader::ReadingStore::get_instance();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect card = reading_card_rect();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect btn = reading_btn_rect();
    // 调用 rect(card)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    c.rect(card);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect lbl_box{card.x, static_cast<int16_t>(card.y + 4), card.w,
                             // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kFontSmall + 2)};。
                             static_cast<uint16_t>(Theme::kFontSmall + 2)};
    // 声明并默认初始化对象：gfx::Canvas::TextStyle ls{}。
    gfx::Canvas::TextStyle ls{};
    // 赋值：把右侧 Theme::kFontSmall 的结果保存到左侧 ls.size_px。
    ls.size_px = Theme::kFontSmall;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ls.h。
    ls.h = gfx::HAlign::Center;
    // 访问对象成员：c.text_in(lbl_box, ui::strings::kReading, ls);；读取或修改这个对象里的成员状态。
    c.text_in(lbl_box, ui::strings::kReading, ls);

    // 判断条件：store.has_book() 是否成立。
    if (store.has_book())
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect title_box{card.x, static_cast<int16_t>(card.y + 20), card.w,
                                   // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kFontTitle + 4)};。
                                   static_cast<uint16_t>(Theme::kFontTitle + 4)};
        // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
        gfx::Canvas::TextStyle ts{};
        // 赋值：把右侧 Theme::kFontTitle 的结果保存到左侧 ts.size_px。
        ts.size_px = Theme::kFontTitle;
        // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
        ts.h = gfx::HAlign::Center;
        // 调用 title()：返回 App 标题。在文件浏览器内，标题有时会动态反映当前目录，因此可能存在一个 buffer，而不是只返回一个常量字符串。。
        c.text_in(title_box, store.title(), ts);

        // 执行语句：char info[40];；它会更新一次变量、对象或程序状态。
        char info[40];
        // 函数调用：(void)std::snprintf(；执行该函数并处理它的返回值或副作用。
        (void)std::snprintf(
            // 继续上一行的表达式或参数列表：info, sizeof(info), "%u / %u  %u%%", static_cast<unsigned>(store.current_page() + 1U),。
            info, sizeof(info), "%u / %u  %u%%", static_cast<unsigned>(store.current_page() + 1U),
            // 继续上一行的表达式或参数列表：static_cast<unsigned>(store.total_pages()), static_cast<unsigned>(store.percent()));。
            static_cast<unsigned>(store.total_pages()), static_cast<unsigned>(store.percent()));
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect info_box{card.x, static_cast<int16_t>(card.y + 46), card.w,
                                  // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kFontSmall + 2)};。
                                  static_cast<uint16_t>(Theme::kFontSmall + 2)};
        // 声明并默认初始化对象：gfx::Canvas::TextStyle ps{}。
        gfx::Canvas::TextStyle ps{};
        // 赋值：把右侧 Theme::kFontSmall 的结果保存到左侧 ps.size_px。
        ps.size_px = Theme::kFontSmall;
        // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ps.h。
        ps.h = gfx::HAlign::Center;
        // 调用 text_in(info_box,info,ps)：文本对齐。
        c.text_in(info_box, info, ps);

        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect bar{static_cast<int16_t>(card.x + 10), static_cast<int16_t>(card.y + 62),
                             // 继续上一行的表达式或参数列表：static_cast<uint16_t>(card.w - 20), Theme::kReadingBarH};。
                             static_cast<uint16_t>(card.w - 20), Theme::kReadingBarH};
        // 调用 ui::widgets::progress_bar(c, bar, store.percent());，执行这个函数并处理它的返回值或状态变化。
        ui::widgets::progress_bar(c, bar, store.percent());

        // 调用 rect(btn)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
        c.rect(btn);
        // 声明并默认初始化对象：gfx::Canvas::TextStyle bs{}。
        gfx::Canvas::TextStyle bs{};
        // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 bs.size_px。
        bs.size_px = Theme::kFontBody;
        // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 bs.h。
        bs.h = gfx::HAlign::Center;
        // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 bs.v。
        bs.v = gfx::VAlign::Middle;
        // 访问对象成员：c.text_in(btn, ui::strings::kContinueRead, bs);；读取或修改这个对象里的成员状态。
        c.text_in(btn, ui::strings::kContinueRead, bs);
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect empty_box{card.x, static_cast<int16_t>(card.y + 40), card.w,
                                   // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kFontBody + 4)};。
                                   static_cast<uint16_t>(Theme::kFontBody + 4)};
        // 声明并默认初始化对象：gfx::Canvas::TextStyle es{}。
        gfx::Canvas::TextStyle es{};
        // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 es.size_px。
        es.size_px = Theme::kFontBody;
        // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 es.h。
        es.h = gfx::HAlign::Center;
        // 访问对象成员：c.text_in(empty_box, ui::strings::kNoBook, es);；读取或修改这个对象里的成员状态。
        c.text_in(empty_box, ui::strings::kNoBook, es);

        // 调用 rect(btn)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
        c.rect(btn);
        // 声明并默认初始化对象：gfx::Canvas::TextStyle bs{}。
        gfx::Canvas::TextStyle bs{};
        // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 bs.size_px。
        bs.size_px = Theme::kFontBody;
        // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 bs.h。
        bs.h = gfx::HAlign::Center;
        // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 bs.v。
        bs.v = gfx::VAlign::Middle;
        // 访问对象成员：c.text_in(btn, ui::strings::kGotoShelf, bs);；读取或修改这个对象里的成员状态。
        c.text_in(btn, ui::strings::kGotoShelf, bs);
    }
}

// 定义或声明函数 paint_quick_grid：参数在括号里，返回类型写在函数名前面。
void paint_quick_grid(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect area = quick_area_rect();
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < 4; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect cell =
            // 继续上一行的表达式或参数列表：ui::widgets::grid_cell(area, Theme::kQuickCols, Theme::kQuickRows, i);。
            ui::widgets::grid_cell(area, Theme::kQuickCols, Theme::kQuickRows, i);
        // 调用 rect(cell)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
        c.rect(cell);

        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t ix = static_cast<int16_t>(cell.x + (cell.w - Theme::kFontIconLg) / 2);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t iy = static_cast<int16_t>(cell.y + (cell.h - 38) / 2);
        // 访问对象成员：c.glyph(ix, iy, kQuick[i].icon, Theme::kFontIconLg, gfx::FontFace::Icon);；读取或修改这个对象里的成员状态。
        c.glyph(ix, iy, kQuick[i].icon, Theme::kFontIconLg, gfx::FontFace::Icon);

        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect lbl_box{cell.x, static_cast<int16_t>(iy + Theme::kFontIconLg + 4), cell.w,
                                 // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kFontSmall + 2)};。
                                 static_cast<uint16_t>(Theme::kFontSmall + 2)};
        // 声明并默认初始化对象：gfx::Canvas::TextStyle s{}。
        gfx::Canvas::TextStyle s{};
        // 赋值：把右侧 Theme::kFontSmall 的结果保存到左侧 s.size_px。
        s.size_px = Theme::kFontSmall;
        // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 s.h。
        s.h = gfx::HAlign::Center;
        // 调用 text_in(lbl_box,kQuick[i].label,s)：文本对齐。
        c.text_in(lbl_box, kQuick[i].label, s);
    }
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 HomePage::instance：参数在括号里，返回类型写在函数名前面。
HomePage& HomePage::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static HomePage s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 函数定义/声明：调用 HomePage::paint()：只按顺序画： 1. 最近阅读卡片。 2. 四个快捷入口。。
void HomePage::paint(gfx::Canvas& c)
{
    // 调用 paint_reading_card(c);，执行这个函数并处理它的返回值或状态变化。
    paint_reading_card(c);
    // 调用 paint_quick_grid(c);，执行这个函数并处理它的返回值或状态变化。
    paint_quick_grid(c);
}

// 函数定义/声明：调用 HomePage::on_input()：输入顺序： 1. 状态栏区域向下滑，打开控制中心。 2. 非点击事件直接返回未消费。 3. 点击最近阅读卡片：设置 Reader 下次进入时恢复阅读，然后导航到 Reader。 4. 点击快捷格子：Reader、Notepad、WoodenFish 直接进 App；“更多”进入 AppGrid。。
InputResult HomePage::on_input(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;
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

    // 判断条件：ev.type != EventType::Tap 是否成立。
    if (ev.type != EventType::Tap)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x = static_cast<int16_t>(ev.x);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y = static_cast<int16_t>(ev.y);

    // 判断条件：reading_card_rect().contains(x, y) 是否成立。
    if (reading_card_rect().contains(x, y))
    {
        // 调用 apps::reader::ReaderApp::request_resume_on_enter();，执行这个函数并处理它的返回值或状态变化。
        apps::reader::ReaderApp::request_resume_on_enter();
        // 访问对象成员：(void)r.navigate(router::page_app(AppId::Reader));；读取或修改这个对象里的成员状态。
        (void)r.navigate(router::page_app(AppId::Reader));
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect area = quick_area_rect();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int8_t i = ui::widgets::grid_hit_test(area, Theme::kQuickCols, Theme::kQuickRows, x, y);
    // 判断条件：i < 0 是否成立。
    if (i < 0)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 判断条件：kQuick[i].target == AppId::None 是否成立。
    if (kQuick[i].target == AppId::None)
    {
        // 访问对象成员：(void)r.navigate(router::page_shell(router::ShellPage::AppGrid));；读取或修改这个对象里的成员状态。
        (void)r.navigate(router::page_shell(router::ShellPage::AppGrid));
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 访问对象成员：(void)r.navigate(router::page_app(kQuick[i].target));；读取或修改这个对象里的成员状态。
        (void)r.navigate(router::page_app(kQuick[i].target));
    }
    // 返回 {true} 给调用者，表示当前函数的执行结果。
    return {true};
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::shell
