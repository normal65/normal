// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/ui/theme.hpp
 * 作用：定义界面尺寸、字体大小、间距和布局 token。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：UI 状态只能在 UiLoop/UI 线程修改，跨任务结果通过 UiBus 回 UI。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/geometry.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::ui {

/** 全局布局 token（constexpr；颜色由 gfx::Ink 表达） */
// struct Theme：把相关数据字段组合在一起，表示项目中的一个概念。
struct Theme
{
    // 屏幕尺寸：core::kScreenW。
    static constexpr uint16_t kScreenW = core::kScreenW;
    // 屏幕尺寸：core::kScreenH。
    static constexpr uint16_t kScreenH = core::kScreenH;

    // 界面间距：4 像素。
    static constexpr uint16_t kPad = 4;
    // 界面间距：8 像素。
    static constexpr uint16_t kPadLg = 8;
    // 界面间距：6 像素。
    static constexpr uint16_t kGap = 6;

    // 字体尺寸：10 像素。
    static constexpr uint8_t kFontSmall = 10;
    // 字体尺寸：12 像素。
    static constexpr uint8_t kFontBody = 12;
    // 字体尺寸：14 像素。
    static constexpr uint8_t kFontTitle = 14;
    // 字体尺寸：18 像素。
    static constexpr uint8_t kFontLarge = 18;
    // 字体尺寸：28 像素。
    static constexpr uint8_t kFontHuge = 28;
    // 字体尺寸：12 像素。
    static constexpr uint8_t kFontIconSm = 12;
    // 字体尺寸：16 像素。
    static constexpr uint8_t kFontIconMd = 16;
    // 字体尺寸：24 像素。
    static constexpr uint8_t kFontIconLg = 24;

    // 固定参数 kStatusBarH = 22：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kStatusBarH = 22;
    // 固定参数 kToolbarH = 32：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kToolbarH = 32;
    // 固定参数 kToolbarY = kStatusBarH：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kToolbarY = kStatusBarH;
    // 固定参数 kBackTapW = 60：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kBackTapW = 60;
    // 固定参数 kContentY = kStatusBarH：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kContentY = kStatusBarH;
    // 固定参数 kContentH = kScreenH - kStatusBarH：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kContentH = kScreenH - kStatusBarH;

    // 固定参数 kListRowH = 36：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kListRowH = 36;
    // 固定参数 kSectionHeaderH = 14：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kSectionHeaderH = 14;
    // 固定参数 kWifiStatusH = 24：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kWifiStatusH = 24;
    // 固定参数 kListStartY = kStatusBarH + kToolbarH：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kListStartY = kStatusBarH + kToolbarH;
    // 静态值 kListVisibleRows：只属于当前模块或类，并保持固定含义。
    static constexpr uint8_t kListVisibleRows =
        // 继续上一行的表达式或参数列表：static_cast<uint8_t>((kScreenH - kListStartY) / kListRowH);。
        static_cast<uint8_t>((kScreenH - kListStartY) / kListRowH);
    // 固定参数 kListRegionH = kListVisibleRows * kListRowH：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kListRegionH = kListVisibleRows * kListRowH;
    // 固定参数 kScrollbarW = 3：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kScrollbarW = 3;

    // 固定参数 kLockChromeH = kStatusBarH：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kLockChromeH = kStatusBarH;
    // 固定参数 kLockTimeY = 40：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kLockTimeY = 40;
    // 固定参数 kLockQuoteY = 110：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kLockQuoteY = 110;
    // 固定参数 kLockQuoteMaxLines = 4：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kLockQuoteMaxLines = 4;
    // 固定参数 kLockQuoteStep = 16：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kLockQuoteStep = 16;
    // 固定参数 kLockHintY = kScreenH - 20：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kLockHintY = kScreenH - 20;

    // 固定参数 kReadingCardY = kContentY + 6：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kReadingCardY = kContentY + 6;
    // 固定参数 kReadingCardH = 110：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kReadingCardH = 110;
    // 固定参数 kReadingBarH = 8：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kReadingBarH = 8;
    // 固定参数 kReadingBtnH = 24：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kReadingBtnH = 24;

    // 固定参数 kQuickY = kReadingCardY + kReadingCardH + 6：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kQuickY = kReadingCardY + kReadingCardH + 6;
    // 固定参数 kQuickRowH = (kScreenH - kQuickY) / 2：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kQuickRowH = (kScreenH - kQuickY) / 2;
    // 固定参数 kQuickCols = 2：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kQuickCols = 2;
    // 固定参数 kQuickRows = 2：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kQuickRows = 2;

    // 固定参数 kAppGridY = kStatusBarH + kToolbarH：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kAppGridY = kStatusBarH + kToolbarH;
    // 固定参数 kAppGridCols = 3：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kAppGridCols = 3;
    // 固定参数 kAppGridCellH = (kScreenH - kAppGridY) / 3：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kAppGridCellH = (kScreenH - kAppGridY) / 3;
    // 固定参数 kAppIconSize = 24：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kAppIconSize = 24;

    // 固定参数 kCtlY = kStatusBarH：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kCtlY = kStatusBarH;
    // 固定参数 kCtlPad = 8：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kCtlPad = 8;
    // 固定参数 kCtlTitleH = 22：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kCtlTitleH = 22;
    // 固定参数 kCtlTileCols = 4：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kCtlTileCols = 4;
    // 固定参数 kCtlTileGap = 6：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kCtlTileGap = 6;
    // 固定参数 kCtlTileIconSz = 16：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kCtlTileIconSz = 16;
    // 固定参数 kCtlTileCellH = 34：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kCtlTileCellH = 34;
    // 固定参数 kCtlSectionGap = 8：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kCtlSectionGap = 8;
    // 固定参数 kCtlLevelRowH = 34：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kCtlLevelRowH = 34;
    // 固定参数 kCtlLevelPitch = 38：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kCtlLevelPitch = 38;
    // 固定参数 kCtlLevelBtnSz = 28：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kCtlLevelBtnSz = 28;
    // 固定参数 kCtlLevelIconSz = 12：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kCtlLevelIconSz = 12;
    // 固定参数 kCtlFooterH = 16：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kCtlFooterH = 16;

    // 固定参数 kKbInputH = 28：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kKbInputH = 28;
    // 固定参数 kKbToolbarH = 28：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kKbToolbarH = 28;
    // 固定参数 kKbCols = 5：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kKbCols = 5;
    // 固定参数 kKbLetterRows = 6：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kKbLetterRows = 6;

    // 静态函数 status_bar_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static constexpr core::Rect status_bar_rect()
    {
        // 返回 core::Rect{0, 0, kScreenW, kStatusBarH} 给调用者，表示当前函数的执行结果。
        return core::Rect{0, 0, kScreenW, kStatusBarH};
    }
    // 静态函数 content_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static constexpr core::Rect content_rect()
    {
        // 返回 core::Rect{0, static_cast<int16_t>(kContentY), kScreenW, kContentH} 给调用者，表示当前函数的执行结果。
        return core::Rect{0, static_cast<int16_t>(kContentY), kScreenW, kContentH};
    }
    // 静态函数 toolbar_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static constexpr core::Rect toolbar_rect()
    {
        // 返回 core::Rect{0, static_cast<int16_t>(kToolbarY), kScreenW, kToolbarH} 给调用者，表示当前函数的执行结果。
        return core::Rect{0, static_cast<int16_t>(kToolbarY), kScreenW, kToolbarH};
    }
    // 静态函数 list_region：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static constexpr core::Rect list_region()
    {
        // 返回 core::Rect{0, static_cast<int16_t>(kListStartY), kScreenW, kListRegionH} 给调用者，表示当前函数的执行结果。
        return core::Rect{0, static_cast<int16_t>(kListStartY), kScreenW, kListRegionH};
    }
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::ui
