// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/ui/dialog.hpp
 * 作用：实现通用对话框、按钮布局和关闭按钮绘制。
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

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::ui::dialog {

// 赋值：把右侧 28 的结果保存到左侧 inline constexpr uint16_t kBtnH。
inline constexpr uint16_t kBtnH = 28;
// 赋值：把右侧 20 的结果保存到左侧 inline constexpr uint8_t kCloseBtnSize。
inline constexpr uint8_t kCloseBtnSize = 20;
// 赋值：把右侧 14 的结果保存到左侧 inline constexpr uint8_t kCloseIconSize。
inline constexpr uint8_t kCloseIconSize = 14;
// 赋值：把右侧 2 的结果保存到左侧 inline constexpr uint8_t kCloseBtnPad。
inline constexpr uint8_t kCloseBtnPad = 2;

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class ConfirmHit : uint8_t
{
    // 执行这条代码：None = 0,。它会更新变量、对象状态或调用链。
    None = 0,
    // 继续上一行的表达式或参数列表：Close,。
    Close,
    // 继续上一行的表达式或参数列表：Yes,。
    Yes,
    // 继续上一行的表达式或参数列表：No,。
    No,
};

// 函数调用：core::Rect centered(uint16_t w, uint16_t h);；执行该函数并处理它的返回值或副作用。
core::Rect centered(uint16_t w, uint16_t h);
// 函数调用：core::Rect close_draw_rect(const core::Rect& dlg);；执行该函数并处理它的返回值或副作用。
core::Rect close_draw_rect(const core::Rect& dlg);
// 函数调用：core::Rect close_hit_rect(const core::Rect& dlg);；执行该函数并处理它的返回值或副作用。
core::Rect close_hit_rect(const core::Rect& dlg);
// 赋值：把右侧 kBtnH) 的结果保存到左侧 core::Rect yes_rect(const core::Rect& dlg, uint16_t btn_h。
core::Rect yes_rect(const core::Rect& dlg, uint16_t btn_h = kBtnH);
// 赋值：把右侧 kBtnH) 的结果保存到左侧 core::Rect no_rect(const core::Rect& dlg, uint16_t btn_h。
core::Rect no_rect(const core::Rect& dlg, uint16_t btn_h = kBtnH);

// 函数调用：void paint_frame(gfx::Canvas& c, const core::Rect& dlg);；执行该函数并处理它的返回值或副作用。
void paint_frame(gfx::Canvas& c, const core::Rect& dlg);
// 函数调用：void paint_close_btn(gfx::Canvas& c, const core::Rect& dlg);；执行该函数并处理它的返回值或副作用。
void paint_close_btn(gfx::Canvas& c, const core::Rect& dlg);
// 赋值：把右侧 kBtnH) 的结果保存到左侧 void paint_yes_no_row(gfx::Canvas& c, const core::Rect& dlg, uint16_t btn_h。
void paint_yes_no_row(gfx::Canvas& c, const core::Rect& dlg, uint16_t btn_h = kBtnH);

/** 白底框 + 文案 + 是/否；with_close 时右上角关闭钮并给文案让宽 */
// 函数调用：void paint_confirm(gfx::Canvas& c, const core::Rect& dlg, const char* message, bool with_close,；执行该函数并处理它的返回值或副作用。
void paint_confirm(gfx::Canvas& c, const core::Rect& dlg, const char* message, bool with_close,
                   // 继续上一行的表达式或参数列表：uint16_t btn_h = kBtnH);。
                   uint16_t btn_h = kBtnH);

// 函数调用：ConfirmHit hit_confirm(const core::Rect& dlg, int16_t x, int16_t y, bool with_close,；执行该函数并处理它的返回值或副作用。
ConfirmHit hit_confirm(const core::Rect& dlg, int16_t x, int16_t y, bool with_close,
                       // 继续上一行的表达式或参数列表：uint16_t btn_h = kBtnH);。
                       uint16_t btn_h = kBtnH);

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::ui::dialog
