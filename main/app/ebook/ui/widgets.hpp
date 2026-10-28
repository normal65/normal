// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/ui/widgets.hpp
 * 作用：提供工具栏、列表行、电池、滚动条、网格等通用绘制控件。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：UI 状态只能在 UiLoop/UI 线程修改，跨任务结果通过 UiBus 回 UI。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/geometry.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/canvas.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::ui::widgets {

// 函数调用：void label(gfx::Canvas& c, int16_t x, int16_t y_top, const char* text,；执行该函数并处理它的返回值或副作用。
void label(gfx::Canvas& c, int16_t x, int16_t y_top, const char* text,
           // 继续上一行的表达式或参数列表：uint8_t size_px = Theme::kFontBody);。
           uint8_t size_px = Theme::kFontBody);

// 函数调用：void label_in(gfx::Canvas& c, const core::Rect& box, const char* text, gfx::HAlign h, gfx::VAlign v,；执行该函数并处理它的返回值或副作用。
void label_in(gfx::Canvas& c, const core::Rect& box, const char* text, gfx::HAlign h, gfx::VAlign v,
              // 继续上一行的表达式或参数列表：uint8_t size_px = Theme::kFontBody);。
              uint8_t size_px = Theme::kFontBody);

/** 空状态：居中标题 + 可选多行说明 */
// 函数调用：void empty_state(gfx::Canvas& c, const core::Rect& box, const char* title,；执行该函数并处理它的返回值或副作用。
void empty_state(gfx::Canvas& c, const core::Rect& box, const char* title,
                 // 只读约束：这个值或对象在当前位置不能被修改。
                 const char* const* hint_lines = nullptr, uint8_t hint_line_count = 0);

// 函数调用：void toolbar(gfx::Canvas& c, const char* title);；执行该函数并处理它的返回值或副作用。
void toolbar(gfx::Canvas& c, const char* title);
// 调用 hit_toolbar_back(int16_tx,int16_ty)：toolbar()： 在固定工具栏矩形中画返回图标、标题和底部横线。返回图标使用 Font Awesome 的码点，而不是手绘 SVG。。
bool hit_toolbar_back(int16_t x, int16_t y);

// struct RowStyle：把相关数据字段组合在一起，表示项目中的一个概念。
struct RowStyle
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* label{nullptr};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* value{nullptr};
    // 执行语句：uint32_t icon_cp{0};；它会更新一次变量、对象或程序状态。
    uint32_t icon_cp{0};
    // 执行语句：uint32_t value_icon{0};；它会更新一次变量、对象或程序状态。
    uint32_t value_icon{0};
    // 执行语句：uint8_t value_icon_size{16};；它会更新一次变量、对象或程序状态。
    uint8_t value_icon_size{16};
    // 执行语句：uint32_t action_icon{0};；它会更新一次变量、对象或程序状态。
    uint32_t action_icon{0};
    // 执行语句：uint8_t action_icon_size{18};；它会更新一次变量、对象或程序状态。
    uint8_t action_icon_size{18};
    // 执行语句：bool show_chevron{true};；它会更新一次变量、对象或程序状态。
    bool show_chevron{true};
    // 执行语句：bool inverted{false};；它会更新一次变量、对象或程序状态。
    bool inverted{false};
};

// 赋值：把右侧 36 的结果保存到左侧 inline constexpr uint16_t kRowActionWidth。
inline constexpr uint16_t kRowActionWidth = 36;

// 函数调用：bool hit_row_action(const core::Rect& row_rect, int16_t x, int16_t y);；执行该函数并处理它的返回值或副作用。
bool hit_row_action(const core::Rect& row_rect, int16_t x, int16_t y);
// 函数调用：void list_row(gfx::Canvas& c, const core::Rect& row_rect, const RowStyle& s);；执行该函数并处理它的返回值或副作用。
void list_row(gfx::Canvas& c, const core::Rect& row_rect, const RowStyle& s);

// 函数调用：void scrollbar(gfx::Canvas& c, uint16_t track_y, uint16_t track_h, uint16_t offset, uint16_t total,；执行该函数并处理它的返回值或副作用。
void scrollbar(gfx::Canvas& c, uint16_t track_y, uint16_t track_h, uint16_t offset, uint16_t total,
               // 继续上一行的表达式或参数列表：uint16_t visible);。
               uint16_t visible);

// 函数调用：void progress_bar(gfx::Canvas& c, const core::Rect& bar, uint8_t pct);；执行该函数并处理它的返回值或副作用。
void progress_bar(gfx::Canvas& c, const core::Rect& bar, uint8_t pct);

// 函数调用：uint16_t battery_indicator(gfx::Canvas& c, int16_t right_edge, int16_t y_top, uint8_t pct,；执行该函数并处理它的返回值或副作用。
uint16_t battery_indicator(gfx::Canvas& c, int16_t right_edge, int16_t y_top, uint8_t pct,
                           // 继续上一行的表达式或参数列表：uint16_t bar_h = Theme::kStatusBarH);。
                           uint16_t bar_h = Theme::kStatusBarH);

// 函数调用：void card_border(gfx::Canvas& c, const core::Rect& r);；执行该函数并处理它的返回值或副作用。
void card_border(gfx::Canvas& c, const core::Rect& r);
// 函数调用：void cover_placeholder(gfx::Canvas& c, const core::Rect& r, const char* label);；执行该函数并处理它的返回值或副作用。
void cover_placeholder(gfx::Canvas& c, const core::Rect& r, const char* label);

// 函数调用：core::Rect grid_cell(const core::Rect& area, uint8_t cols, uint8_t rows, uint8_t index,；执行该函数并处理它的返回值或副作用。
core::Rect grid_cell(const core::Rect& area, uint8_t cols, uint8_t rows, uint8_t index,
                     // 继续上一行的表达式或参数列表：uint16_t gap = Theme::kPad);。
                     uint16_t gap = Theme::kPad);

// 函数调用：int8_t grid_hit_test(const core::Rect& area, uint8_t cols, uint8_t rows, int16_t x, int16_t y,；执行该函数并处理它的返回值或副作用。
int8_t grid_hit_test(const core::Rect& area, uint8_t cols, uint8_t rows, int16_t x, int16_t y,
                     // 继续上一行的表达式或参数列表：uint16_t gap = Theme::kPad);。
                     uint16_t gap = Theme::kPad);

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::ui::widgets
