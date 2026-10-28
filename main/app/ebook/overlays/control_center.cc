/**
 * [初学者导读] 文件：main/app/ebook/overlays/control_center.cc
 * 作用：实现 overlays 模块中的 control center 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：覆盖层优先接收输入，打开和关闭会改变路由栈。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/control_center.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/system_state.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/font.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/icon.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/page_id.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/refresh_intent.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/router.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/power_save.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::overlays {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;

// 固定参数 kTileCount = 5：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kTileCount = 5;
// 固定参数 kLevelCount = 2：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kLevelCount = 2;
// 固定参数 kLevelStep = 10：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kLevelStep = 10;

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using apps::settings::PowerSave;

// 固定参数 kLevelIconColW = 14：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kLevelIconColW = 14;
// 固定参数 kLevelLabelW = 28：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kLevelLabelW = 28;
// 固定参数 kLevelGap = 4：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kLevelGap = 4;

// 静态查表数组 kLevelIcons：运行时复用固定数据，避免每次重新计算。
constexpr uint32_t kLevelIcons[kLevelCount] = {
    // 执行这条代码：gfx::icon::kFaSun,。它会更新变量、对象状态或调用链。
    gfx::icon::kFaSun,
    // 继续上一行的表达式或参数列表：gfx::icon::kFaVolumeUp,。
    gfx::icon::kFaVolumeUp,
};

// 编译期函数 panel_y：常用于固定规则计算，能参与编译期优化。
constexpr uint16_t panel_y()
{
    // 返回 Theme::kCtlY 给调用者，表示当前函数的执行结果。
    return Theme::kCtlY;
}
// 编译期函数 panel_h：常用于固定规则计算，能参与编译期优化。
constexpr uint16_t panel_h()
{
    // 返回 Theme::kScreenH - Theme::kCtlY 给调用者，表示当前函数的执行结果。
    return Theme::kScreenH - Theme::kCtlY;
}

// 编译期函数 tile_w：常用于固定规则计算，能参与编译期优化。
constexpr uint16_t tile_w()
{
    // 返回 static_cast<uint16_t>( 给调用者，表示当前函数的执行结果。
    return static_cast<uint16_t>(
        // 继续上一行的表达式或参数列表：(Theme::kScreenW - Theme::kCtlPad * 2 - (kTileCount - 1) * Theme::kCtlTileGap) /。
        (Theme::kScreenW - Theme::kCtlPad * 2 - (kTileCount - 1) * Theme::kCtlTileGap) /
        // 执行语句：kTileCount);；它会更新一次变量、对象或程序状态。
        kTileCount);
}

// 编译期函数 header_bottom：常用于固定规则计算，能参与编译期优化。
constexpr uint16_t header_bottom()
{
    // 返回 panel_y() + Theme::kCtlTitleH 给调用者，表示当前函数的执行结果。
    return panel_y() + Theme::kCtlTitleH;
}
// 编译期函数 tiles_top：常用于固定规则计算，能参与编译期优化。
constexpr uint16_t tiles_top()
{
    // 返回 static_cast<uint16_t>(header_bottom() + 5) 给调用者，表示当前函数的执行结果。
    return static_cast<uint16_t>(header_bottom() + 5);
}
// 编译期函数 tiles_bottom：常用于固定规则计算，能参与编译期优化。
constexpr uint16_t tiles_bottom()
{
    // 返回 static_cast<uint16_t>(tiles_top() + Theme::kCtlTileCellH) 给调用者，表示当前函数的执行结果。
    return static_cast<uint16_t>(tiles_top() + Theme::kCtlTileCellH);
}
// 编译期函数 levels_top：常用于固定规则计算，能参与编译期优化。
constexpr uint16_t levels_top()
{
    // 返回 static_cast<uint16_t>(tiles_bottom() + Theme::kCtlSectionGap) 给调用者，表示当前函数的执行结果。
    return static_cast<uint16_t>(tiles_bottom() + Theme::kCtlSectionGap);
}
// 编译期函数 footer_top：常用于固定规则计算，能参与编译期优化。
constexpr uint16_t footer_top()
{
    // 返回 static_cast<uint16_t>(panel_y() + panel_h() - Theme::kCtlFooterH) 给调用者，表示当前函数的执行结果。
    return static_cast<uint16_t>(panel_y() + panel_h() - Theme::kCtlFooterH);
}

// 静态查表数组 kTileIcons：运行时复用固定数据，避免每次重新计算。
constexpr uint32_t kTileIcons[kTileCount - 1] = {
    // 执行这条代码：gfx::icon::kFaWifi,。它会更新变量、对象状态或调用链。
    gfx::icon::kFaWifi,
    // 继续上一行的表达式或参数列表：gfx::icon::kEbBluetooth,。
    gfx::icon::kEbBluetooth,
    // 继续上一行的表达式或参数列表：gfx::icon::kFaMoon,。
    gfx::icon::kFaMoon,
    // 继续上一行的表达式或参数列表：gfx::icon::kFaLock,。
    gfx::icon::kFaLock,
};

// 定义或声明函数 tile_icon：参数在括号里，返回类型写在函数名前面。
uint32_t tile_icon(uint8_t idx)
{
    // 判断条件：idx == 4 是否成立。
    if (idx == 4)
    {
        // 返回 PowerSave::get_instance().enabled() ? gfx::icon::kFaBatteryQuarter 给调用者，表示当前函数的执行结果。
        return PowerSave::get_instance().enabled() ? gfx::icon::kFaBatteryQuarter
                                                   // 执行语句：: gfx::icon::kFaBatteryFull;；它会更新一次变量、对象或程序状态。
                                                   : gfx::icon::kFaBatteryFull;
    }
    // 判断条件：idx < kTileCount - 1 是否成立。
    if (idx < kTileCount - 1)
        // 返回 kTileIcons[idx] 给调用者，表示当前函数的执行结果。
        return kTileIcons[idx];
    // 返回 gfx::icon::kFaBatteryFull 给调用者，表示当前函数的执行结果。
    return gfx::icon::kFaBatteryFull;
}

// struct LevelRowLayout：把相关数据字段组合在一起，表示项目中的一个概念。
struct LevelRowLayout
{
    // 执行语句：core::Rect row;；它会更新一次变量、对象或程序状态。
    core::Rect row;
    // 执行语句：core::Rect minus;；它会更新一次变量、对象或程序状态。
    core::Rect minus;
    // 执行语句：core::Rect plus;；它会更新一次变量、对象或程序状态。
    core::Rect plus;
    // 执行语句：core::Rect pct_area;；它会更新一次变量、对象或程序状态。
    core::Rect pct_area;
};

// 定义或声明函数 step_clamp：参数在括号里，返回类型写在函数名前面。
uint8_t step_clamp(uint8_t v, int delta)
{
    // 定义变量 int n 并初始化为 static_cast<int>(v) + delta。
    int n = static_cast<int>(v) + delta;
    // 判断条件：n < 0 是否成立。
    if (n < 0)
        // 赋值：把右侧 0 的结果保存到左侧 n。
        n = 0;
    // 判断条件：n > 100 是否成立。
    if (n > 100)
        // 赋值：把右侧 100 的结果保存到左侧 n。
        n = 100;
    // 返回 static_cast<uint8_t>(n) 给调用者，表示当前函数的执行结果。
    return static_cast<uint8_t>(n);
}

// 定义或声明函数 make_tile_rect：参数在括号里，返回类型写在函数名前面。
core::Rect make_tile_rect(uint8_t idx)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t col = idx % kTileCount;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t x =
        // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kCtlPad + col * (tile_w() + Theme::kCtlTileGap));。
        static_cast<uint16_t>(Theme::kCtlPad + col * (tile_w() + Theme::kCtlTileGap));
    // 返回 core::Rect{static_cast<int16_t>(x), static_cast<int16_t>(tiles_top()), tile_w(), 给调用者，表示当前函数的执行结果。
    return core::Rect{static_cast<int16_t>(x), static_cast<int16_t>(tiles_top()), tile_w(),
                      // 继续上一行的表达式或参数列表：Theme::kCtlTileCellH};。
                      Theme::kCtlTileCellH};
}

// 定义或声明函数 make_level_row_rect：参数在括号里，返回类型写在函数名前面。
core::Rect make_level_row_rect(uint8_t idx)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t y = static_cast<uint16_t>(levels_top() + idx * Theme::kCtlLevelPitch);
    // 返回 core::Rect{static_cast<int16_t>(Theme::kCtlPad), static_cast<int16_t>(y), 给调用者，表示当前函数的执行结果。
    return core::Rect{static_cast<int16_t>(Theme::kCtlPad), static_cast<int16_t>(y),
                      // 继续上一行的表达式或参数列表：Theme::kScreenW - Theme::kCtlPad * 2, Theme::kCtlLevelRowH};。
                      Theme::kScreenW - Theme::kCtlPad * 2, Theme::kCtlLevelRowH};
}

// 定义或声明函数 make_level_layout：参数在括号里，返回类型写在函数名前面。
LevelRowLayout make_level_layout(uint8_t idx)
{
    // 声明并默认初始化对象：LevelRowLayout L{}。
    LevelRowLayout L{};
    // 赋值：把右侧 make_level_row_rect(idx) 的结果保存到左侧 L.row。
    L.row = make_level_row_rect(idx);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t plus_x = L.row.right() - static_cast<int16_t>(Theme::kCtlLevelBtnSz);
    // 赋值：把右侧 core::Rect{plus_x, L.row.y, Theme::kCtlLevelBtnSz, L.row.h} 的结果保存到左侧 L.plus。
    L.plus = core::Rect{plus_x, L.row.y, Theme::kCtlLevelBtnSz, L.row.h};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t minus_x = plus_x - static_cast<int16_t>(kLevelGap + Theme::kCtlLevelBtnSz);
    // 赋值：把右侧 core::Rect{minus_x, L.row.y, Theme::kCtlLevelBtnSz, L.row.h} 的结果保存到左侧 L.minus。
    L.minus = core::Rect{minus_x, L.row.y, Theme::kCtlLevelBtnSz, L.row.h};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t mid_x = L.row.x + static_cast<int16_t>(kLevelIconColW + kLevelLabelW + kLevelGap);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t mid_w = minus_x - mid_x - static_cast<int16_t>(kLevelGap);
    // 判断条件：mid_w > 0 是否成立。
    if (mid_w > 0)
        // 赋值：把右侧 core::Rect{mid_x, L.row.y, static_cast<uint16_t>(mid_w), L.row.h} 的结果保存到左侧 L.pct_area。
        L.pct_area = core::Rect{mid_x, L.row.y, static_cast<uint16_t>(mid_w), L.row.h};

    // 返回 L 给调用者，表示当前函数的执行结果。
    return L;
}

// 定义或声明函数 paint_header：参数在括号里，返回类型写在函数名前面。
void paint_header(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect bar{0, static_cast<int16_t>(panel_y()), Theme::kScreenW, Theme::kCtlTitleH};
    // 调用 ui::widgets::label_in(c, bar, ui::strings::kCtlTitle, gfx::HAlign::Center, gfx::VAlign::Middle,，执行这个函数并处理它的返回值或状态变化。
    ui::widgets::label_in(c, bar, ui::strings::kCtlTitle, gfx::HAlign::Center, gfx::VAlign::Middle,
                          // 继续上一行的表达式或参数列表：Theme::kFontBody);。
                          Theme::kFontBody);

    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    c.hline(Theme::kPad, static_cast<int16_t>(header_bottom()), Theme::kScreenW - Theme::kPad * 2);
}

// 定义或声明函数 paint_tile：参数在括号里，返回类型写在函数名前面。
void paint_tile(gfx::Canvas& c, const core::Rect& cell, uint32_t icon_cp, bool active)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t icon_x = static_cast<int16_t>(cell.x + (cell.w - Theme::kCtlTileIconSz) / 2);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t icon_y = static_cast<int16_t>(cell.y + (cell.h - Theme::kCtlTileIconSz) / 2);
    // 访问对象成员：c.glyph(icon_x, icon_y, icon_cp, Theme::kCtlTileIconSz, gfx::FontFace::Icon);；读取或修改这个对象里的成员状态。
    c.glyph(icon_x, icon_y, icon_cp, Theme::kCtlTileIconSz, gfx::FontFace::Icon);

    // 判断条件：active 是否成立。
    if (active)
        // 访问对象成员：c.invert(cell);；读取或修改这个对象里的成员状态。
        c.invert(cell);
}

// 定义或声明函数 draw_step_btn：参数在括号里，返回类型写在函数名前面。
void draw_step_btn(gfx::Canvas& c, const core::Rect& r, uint32_t icon_cp)
{
    // 调用 rect(r)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    c.rect(r);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t ix = static_cast<int16_t>(r.x + (r.w - Theme::kFontIconMd) / 2);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t iy = static_cast<int16_t>(r.y + (r.h - Theme::kFontIconMd) / 2);
    // 访问对象成员：c.glyph(ix, iy, icon_cp, Theme::kFontIconMd, gfx::FontFace::Icon);；读取或修改这个对象里的成员状态。
    c.glyph(ix, iy, icon_cp, Theme::kFontIconMd, gfx::FontFace::Icon);
}

// 函数调用：void paint_level_row(gfx::Canvas& c, const LevelRowLayout& L, uint32_t row_icon, const char* label,；执行该函数并处理它的返回值或副作用。
void paint_level_row(gfx::Canvas& c, const LevelRowLayout& L, uint32_t row_icon, const char* label,
                     // 继续上一行的表达式或参数列表：uint8_t pct)。
                     uint8_t pct)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t ty = static_cast<int16_t>(L.row.y + (L.row.h - Theme::kFontSmall) / 2);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t ix =
        // 继续上一行的表达式或参数列表：static_cast<int16_t>(L.row.x + (kLevelIconColW - Theme::kCtlLevelIconSz) / 2);。
        static_cast<int16_t>(L.row.x + (kLevelIconColW - Theme::kCtlLevelIconSz) / 2);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t iy = static_cast<int16_t>(L.row.y + (L.row.h - Theme::kCtlLevelIconSz) / 2);
    // 访问对象成员：c.glyph(ix, iy, row_icon, Theme::kCtlLevelIconSz, gfx::FontFace::Icon);；读取或修改这个对象里的成员状态。
    c.glyph(ix, iy, row_icon, Theme::kCtlLevelIconSz, gfx::FontFace::Icon);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t label_x = L.row.x + static_cast<int16_t>(kLevelIconColW);
    // 访问对象成员：c.text(label_x, ty, label, Theme::kFontSmall);；读取或修改这个对象里的成员状态。
    c.text(label_x, ty, label, Theme::kFontSmall);

    // 判断条件：!L.pct_area.empty() 是否成立。
    if (!L.pct_area.empty())
    {
        // 执行语句：char buf[8];；它会更新一次变量、对象或程序状态。
        char buf[8];
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        (void)std::snprintf(buf, sizeof(buf), "%u%%", static_cast<unsigned>(pct));
        // 调用 ui::widgets::label_in(c, L.pct_area, buf, gfx::HAlign::Center, gfx::VAlign::Middle,，执行这个函数并处理它的返回值或状态变化。
        ui::widgets::label_in(c, L.pct_area, buf, gfx::HAlign::Center, gfx::VAlign::Middle,
                              // 继续上一行的表达式或参数列表：Theme::kFontSmall);。
                              Theme::kFontSmall);
    }

    // 调用 draw_step_btn(c, L.minus, gfx::icon::kFaMinus);，执行这个函数并处理它的返回值或状态变化。
    draw_step_btn(c, L.minus, gfx::icon::kFaMinus);
    // 调用 draw_step_btn(c, L.plus, gfx::icon::kFaPlus);，执行这个函数并处理它的返回值或状态变化。
    draw_step_btn(c, L.plus, gfx::icon::kFaPlus);
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 ControlCenter::instance：参数在括号里，返回类型写在函数名前面。
ControlCenter& ControlCenter::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static ControlCenter s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// [初学者导读] ControlCenter::is_open()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
bool ControlCenter::is_open() const
{
    // 返回结果：调用 overlay_top()：overlay 是覆盖在页面上的临时层，例如控制中心。最多四层，overlaytop() 返回当前最上面一层，没有时返回 None。。
    return router::Router::instance().stack().overlay_top() == router::OverlayId::ControlCenter;
}

// 定义或声明函数 ControlCenter::bounds：参数在括号里，返回类型写在函数名前面。
core::Rect ControlCenter::bounds() const
{
    // 返回 core::Rect{0, static_cast<int16_t>(panel_y()), Theme::kScreenW, panel_h()} 给调用者，表示当前函数的执行结果。
    return core::Rect{0, static_cast<int16_t>(panel_y()), Theme::kScreenW, panel_h()};
}

// 定义或声明函数 ControlCenter::repaint_panel：参数在括号里，返回类型写在函数名前面。
void ControlCenter::repaint_panel()
{
    // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
    (void)router::Router::instance().repaint(router::intent_partial_full());
}

// 定义或声明函数 ControlCenter::open：参数在括号里，返回类型写在函数名前面。
void ControlCenter::open()
{
    // 判断条件：is_open() 是否成立。
    if (is_open())
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const router::RefreshIntent intent = router::intent_partial_full();
    // 访问对象成员：(void)router::Router::instance().open_overlay(router::OverlayId::ControlCenter, &intent);；读取或修改这个对象里的成员状态。
    (void)router::Router::instance().open_overlay(router::OverlayId::ControlCenter, &intent);
}

// 函数定义/声明：调用 ControlCenter::close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
void ControlCenter::close()
{
    // 判断条件：!is_open() 是否成立。
    if (!is_open())
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const router::RefreshIntent intent = router::intent_partial_full();
    // 调用 close_overlay(&intent)：这些函数只是用明确的参数调用统一导航逻辑，避免每个页面自己拼 Transition。。
    (void)router::Router::instance().close_overlay(&intent);
}

// 定义或声明函数 ControlCenter::tile_rect：参数在括号里，返回类型写在函数名前面。
core::Rect ControlCenter::tile_rect(uint8_t idx) const
{
    // 返回 make_tile_rect(idx) 给调用者，表示当前函数的执行结果。
    return make_tile_rect(idx);
}

// 定义或声明函数 ControlCenter::level_row_rect：参数在括号里，返回类型写在函数名前面。
core::Rect ControlCenter::level_row_rect(uint8_t idx) const
{
    // 返回 make_level_row_rect(idx) 给调用者，表示当前函数的执行结果。
    return make_level_row_rect(idx);
}

// 定义或声明函数 ControlCenter::level_minus_rect：参数在括号里，返回类型写在函数名前面。
core::Rect ControlCenter::level_minus_rect(uint8_t idx) const
{
    // 返回 make_level_layout(idx).minus 给调用者，表示当前函数的执行结果。
    return make_level_layout(idx).minus;
}

// 定义或声明函数 ControlCenter::level_plus_rect：参数在括号里，返回类型写在函数名前面。
core::Rect ControlCenter::level_plus_rect(uint8_t idx) const
{
    // 返回 make_level_layout(idx).plus 给调用者，表示当前函数的执行结果。
    return make_level_layout(idx).plus;
}

// 函数定义/声明：调用 ControlCenter::adjust_level()：索引 0 调亮度，索引 1 调音量。它先计算夹紧后的新值： 值没变时返回 false，调用者不刷新。 值变化时调用 SystemState::set()，返回 true。。
bool ControlCenter::adjust_level(uint8_t idx, int delta)
{
    // 赋值：把右侧 data::SystemState::get_instance() 的结果保存到左侧 auto& sys。
    auto& sys = data::SystemState::get_instance();
    // 判断条件：idx == 0 是否成立。
    if (idx == 0)
    {
        // 定义只读对象或变量：初始化后不能在当前位置修改。
        const uint8_t v = step_clamp(sys.brightness(), delta);
        // 判断条件：v == sys.brightness() 是否成立。
        if (v == sys.brightness())
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        // 调用 set_brightness(v)：把 0 到 100 的亮度限制在合法范围后交给背光驱动。墨水屏本身不依赖背光，背光是前光/照明系统，不能把亮度当成画面灰度。。
        sys.set_brightness(v);
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 判断条件：idx == 1 是否成立。
    if (idx == 1)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t v = step_clamp(sys.volume(), delta);
        // 判断条件：v == sys.volume() 是否成立。
        if (v == sys.volume())
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        // 访问对象成员：sys.set_volume(v);；读取或修改这个对象里的成员状态。
        sys.set_volume(v);
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 返回 false 给调用者，表示当前函数的执行结果。
    return false;
}

// 函数定义/声明：调用 ControlCenter::on_input()：处理顺序： 1. 面板没打开时返回未消费。 2. 上滑关闭。 3. 非点击事件被覆盖层吞掉，防止透传到下面页面。 4. 点击亮度/音量加减。 5. 点击五个开关：WiFi、蓝牙、夜间、锁屏、省电。。
shell::InputResult ControlCenter::on_input(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;
    // 判断条件：!is_open() 是否成立。
    if (!is_open())
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 赋值：把右侧 data::SystemState::get_instance() 的结果保存到左侧 auto& sys。
    auto& sys = data::SystemState::get_instance();
    // 赋值：把右侧 router::Router::instance() 的结果保存到左侧 auto& r。
    auto& r = router::Router::instance();

    // 判断条件：ev.type == EventType::SwipeUp 是否成立。
    if (ev.type == EventType::SwipeUp)
    {
        // 调用 close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
        close();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 判断条件：ev.type != EventType::Tap 是否成立。
    if (ev.type != EventType::Tap)
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x = static_cast<int16_t>(ev.x);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y = static_cast<int16_t>(ev.y);

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < kLevelCount; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const LevelRowLayout L = make_level_layout(i);
        // 判断条件：L.minus.contains(x, y) 是否成立。
        if (L.minus.contains(x, y))
        {
            // 判断条件：adjust_level(i, -static_cast<int>(kLevelStep)) 是否成立。
            if (adjust_level(i, -static_cast<int>(kLevelStep)))
                // 调用 repaint_panel();，执行这个函数并处理它的返回值或状态变化。
                repaint_panel();
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        }
        // 判断条件：L.plus.contains(x, y) 是否成立。
        if (L.plus.contains(x, y))
        {
            // 判断条件：adjust_level(i, static_cast<int>(kLevelStep)) 是否成立。
            if (adjust_level(i, static_cast<int>(kLevelStep)))
                // 调用 repaint_panel();，执行这个函数并处理它的返回值或状态变化。
                repaint_panel();
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        }
    }

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < kTileCount; ++i)
    {
        // 判断条件：!make_tile_rect(i).contains(x, y) 是否成立。
        if (!make_tile_rect(i).contains(x, y))
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        // 多分支判断：根据括号里的值选择不同 case。
        switch (i)
        {
            // 分支匹配：case 0 成立时执行下面的代码。
            case 0:
                // 访问对象成员：sys.set_wifi(!sys.wifi());；读取或修改这个对象里的成员状态。
                sys.set_wifi(!sys.wifi());
                // 调用 apps::settings::SettingsApp::instance().wifi_apply_state();，执行这个函数并处理它的返回值或状态变化。
                apps::settings::SettingsApp::instance().wifi_apply_state();
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 分支匹配：case 1 成立时执行下面的代码。
            case 1:
                // 访问对象成员：sys.set_bluetooth(!sys.bluetooth());；读取或修改这个对象里的成员状态。
                sys.set_bluetooth(!sys.bluetooth());
                // 调用 bt_apply_state()：根据 SystemState.bluetooth： 打开时调用 BleMgr.init()，并按需要开始广播。 关闭时停止广播。。
                apps::settings::SettingsApp::instance().bt_apply_state();
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 分支匹配：case 2 成立时执行下面的代码。
            case 2:
                // 访问对象成员：sys.set_night_mode(!sys.night_mode());；读取或修改这个对象里的成员状态。
                sys.set_night_mode(!sys.night_mode());
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            // 分支匹配：case 3 成立时执行下面的代码。
            case 3:
                // replace_shell 会清 overlay，无需先 close
                (void)r.replace_shell(router::ShellPage::Lock);
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            // 分支匹配：case 4 成立时执行下面的代码。
            case 4:
                // 调用 toggle_manual()：用户点击开关时调用，通常先弹出确认提示。。
                PowerSave::get_instance().toggle_manual();
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 执行这条代码：default:。它会更新变量、对象状态或调用链。
            default:
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
        }
        // 调用 repaint_panel();，执行这个函数并处理它的返回值或状态变化。
        repaint_panel();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 返回 {true} 给调用者，表示当前函数的执行结果。
    return {true};
}

// 函数定义/声明：调用 ControlCenter::paint()：先填充白色面板，再画标题、五个开关格、分隔线、亮度和音量行、底部提示、边框。开关是否高亮由 SystemState 和 PowerSave 状态决定。。
void ControlCenter::paint(gfx::Canvas& c)
{
    // 赋值：把右侧 data::SystemState::get_instance() 的结果保存到左侧 auto& sys。
    auto& sys = data::SystemState::get_instance();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool on_lock = router::Router::instance().stack().shell_top() == router::ShellPage::Lock;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect panel = bounds();
    // 访问对象成员：c.fill(panel, gfx::Ink::White);；读取或修改这个对象里的成员状态。
    c.fill(panel, gfx::Ink::White);

    // 调用 paint_header(c);，执行这个函数并处理它的返回值或状态变化。
    paint_header(c);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool states[kTileCount] = {
        // 函数调用：sys.wifi(), sys.bluetooth(), sys.night_mode(), on_lock, PowerSave::get_instance().enabled(),；执行该函数并处理它的返回值或副作用。
        sys.wifi(), sys.bluetooth(), sys.night_mode(), on_lock, PowerSave::get_instance().enabled(),
    };
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < kTileCount; ++i)
        // 调用 paint_tile(c, make_tile_rect(i), tile_icon(i), states[i]);，执行这个函数并处理它的返回值或状态变化。
        paint_tile(c, make_tile_rect(i), tile_icon(i), states[i]);

    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    c.hline(Theme::kPad, static_cast<int16_t>(tiles_bottom()), Theme::kScreenW - Theme::kPad * 2);

    // 静态查表数组 level_labels：运行时复用固定数据，避免每次重新计算。
    static const char* level_labels[kLevelCount] = {
        // 执行这条代码：ui::strings::kCtlBrightness,。它会更新变量、对象状态或调用链。
        ui::strings::kCtlBrightness,
        // 继续上一行的表达式或参数列表：ui::strings::kCtlVolume,。
        ui::strings::kCtlVolume,
    };
    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const uint8_t level_vals[kLevelCount] = {sys.brightness(), sys.volume()};
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < kLevelCount; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const LevelRowLayout L = make_level_layout(i);
        // 调用 paint_level_row(c, L, kLevelIcons[i], level_labels[i], level_vals[i]);，执行这个函数并处理它的返回值或状态变化。
        paint_level_row(c, L, kLevelIcons[i], level_labels[i], level_vals[i]);
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect footer{0, static_cast<int16_t>(footer_top()), Theme::kScreenW,
                            // 继续上一行的表达式或参数列表：Theme::kCtlFooterH};。
                            Theme::kCtlFooterH};
    // 调用 ui::widgets::label_in(c, footer, ui::strings::kCtlSwipeUp, gfx::HAlign::Center,，执行这个函数并处理它的返回值或状态变化。
    ui::widgets::label_in(c, footer, ui::strings::kCtlSwipeUp, gfx::HAlign::Center,
                          // 继续上一行的表达式或参数列表：gfx::VAlign::Middle, Theme::kFontSmall);。
                          gfx::VAlign::Middle, Theme::kFontSmall);

    // 调用 rect(panel)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    c.rect(panel);
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::overlays
