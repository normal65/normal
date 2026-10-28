// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/reader/reader_layout.hpp
 * 作用：实现 reader 模块中的 reader layout 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/geometry.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/txt_index_cache.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/font.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::reader {

/** 阅读器版面（竖屏 176×264，随字号变化） */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class ReaderLayout
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态查表数组 kFontSizes：运行时复用固定数据，避免每次重新计算。
    static constexpr uint8_t kFontSizes[3] = {12, 14, 16};
    // 固定参数 kTopBarH = 18：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kTopBarH = 18;
    // 界面间距：8 像素。
    static constexpr uint16_t kPadH = 8;
    // 固定参数 kContentPadY = 4：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kContentPadY = 4;

    /** 底栏菜单：浮在正文上方，贴屏幕底边 */
    // 固定参数 kMenuRowH = 20：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kMenuRowH = 20;
    // 固定参数 kMenuRowGap = 1：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kMenuRowGap = 1;
    // 固定参数 kMenuRows = 4：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kMenuRows = 4;
    // 固定参数 kMenuPanelH = kMenuRowH * kMenuRows + kMenuRowGap * (kMenuRows - 1)：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint16_t kMenuPanelH = kMenuRowH * kMenuRows + kMenuRowGap * (kMenuRows - 1);

    // 定义或声明函数 ReaderLayout：参数在括号里，返回类型写在函数名前面。
    explicit ReaderLayout(uint8_t font_px = 12)
        // 定义或声明函数 font_px_：参数在括号里，返回类型写在函数名前面。
        : font_px_(font_px)
    {
    }

    // 定义或声明函数 font_px：参数在括号里，返回类型写在函数名前面。
    uint8_t font_px() const
    {
        // 返回 font_px_ 给调用者，表示当前函数的执行结果。
        return font_px_;
    }

    // 函数定义/声明：调用 line_height()：advance(cp)： 查一个字符的前进宽度。点阵优先，FreeType 兜底。它不会绘制任何像素，适合排版测量。。
    uint16_t line_height() const
    {
        // 返回 gfx::Font::get_instance().line_height(font_px_, gfx::FontFace::Text) 给调用者，表示当前函数的执行结果。
        return gfx::Font::get_instance().line_height(font_px_, gfx::FontFace::Text);
    }

    // 定义或声明函数 text_width：参数在括号里，返回类型写在函数名前面。
    uint16_t text_width() const
    {
        // 返回 static_cast<uint16_t>(ui::Theme::kScreenW - kPadH * 2) 给调用者，表示当前函数的执行结果。
        return static_cast<uint16_t>(ui::Theme::kScreenW - kPadH * 2);
    }

    /** 正文区：顶栏以下至屏幕底，不受菜单显隐影响 */
    // 定义或声明函数 content_rect：参数在括号里，返回类型写在函数名前面。
    core::Rect content_rect() const
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint16_t h = static_cast<uint16_t>(ui::Theme::kScreenH - kTopBarH);
        // 返回 core::Rect{static_cast<int16_t>(kPadH), static_cast<int16_t>(kTopBarH), text_width(), 给调用者，表示当前函数的执行结果。
        return core::Rect{static_cast<int16_t>(kPadH), static_cast<int16_t>(kTopBarH), text_width(),
                          // 继续上一行的表达式或参数列表：h};。
                          h};
    }

    // 定义或声明函数 lines_per_page：参数在括号里，返回类型写在函数名前面。
    uint8_t lines_per_page() const
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect area = content_rect();
        // 定义只读对象或变量：初始化后不能在当前位置修改。
        const uint16_t lh = line_height();
        // 判断条件：lh == 0 是否成立。
        if (lh == 0)
            // 返回 1 给调用者，表示当前函数的执行结果。
            return 1;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint16_t usable =
            // 继续上一行的表达式或参数列表：(area.h > kContentPadY * 2) ? static_cast<uint16_t>(area.h - kContentPadY * 2) : area.h;。
            (area.h > kContentPadY * 2) ? static_cast<uint16_t>(area.h - kContentPadY * 2) : area.h;
        // 定义变量 uint8_t n 并初始化为 static_cast<uint8_t>(usable / lh)。
        uint8_t n = static_cast<uint8_t>(usable / lh);
        // 返回 (n < 1) ? 1 : n 给调用者，表示当前函数的执行结果。
        return (n < 1) ? 1 : n;
    }

    // 定义或声明函数 top_bar_rect：参数在括号里，返回类型写在函数名前面。
    core::Rect top_bar_rect() const
    {
        // 返回 core::Rect{0, 0, ui::Theme::kScreenW, kTopBarH} 给调用者，表示当前函数的执行结果。
        return core::Rect{0, 0, ui::Theme::kScreenW, kTopBarH};
    }

    // 定义或声明函数 menu_panel_y：参数在括号里，返回类型写在函数名前面。
    int16_t menu_panel_y() const
    {
        // 返回 static_cast<int16_t>(ui::Theme::kScreenH - kMenuPanelH) 给调用者，表示当前函数的执行结果。
        return static_cast<int16_t>(ui::Theme::kScreenH - kMenuPanelH);
    }

    // 定义或声明函数 menu_panel_rect：参数在括号里，返回类型写在函数名前面。
    core::Rect menu_panel_rect() const
    {
        // 返回 core::Rect{0, menu_panel_y(), ui::Theme::kScreenW, kMenuPanelH} 给调用者，表示当前函数的执行结果。
        return core::Rect{0, menu_panel_y(), ui::Theme::kScreenW, kMenuPanelH};
    }

    // 定义或声明函数 make_index_layout：参数在括号里，返回类型写在函数名前面。
    TxtIndexLayout make_index_layout(uint8_t text_encoding, uint32_t file_size) const
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect area = content_rect();
        // 声明并默认初始化对象：TxtIndexLayout l{}。
        TxtIndexLayout l{};
        // 赋值：把右侧 area.w 的结果保存到左侧 l.viewport_w。
        l.viewport_w = area.w;
        // 赋值：把右侧 area.h 的结果保存到左侧 l.viewport_h。
        l.viewport_h = area.h;
        // 赋值：把右侧 lines_per_page() 的结果保存到左侧 l.lines_per_page。
        l.lines_per_page = lines_per_page();
        // 赋值：把右侧 font_px_ 的结果保存到左侧 l.font_size。
        l.font_size = font_px_;
        // 调用 line_height()：advance(cp)： 查一个字符的前进宽度。点阵优先，FreeType 兜底。它不会绘制任何像素，适合排版测量。。
        l.line_height = line_height();
        // 赋值：把右侧 ui::Theme::kScreenW 的结果保存到左侧 l.screen_w。
        l.screen_w = ui::Theme::kScreenW;
        // 赋值：把右侧 ui::Theme::kScreenH 的结果保存到左侧 l.screen_h。
        l.screen_h = ui::Theme::kScreenH;
        // 赋值：把右侧 file_size 的结果保存到左侧 l.file_size。
        l.file_size = file_size;
        // 赋值：把右侧 text_encoding 的结果保存到左侧 l.text_encoding。
        l.text_encoding = text_encoding;
        // 返回 l 给调用者，表示当前函数的执行结果。
        return l;
    }

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 执行语句：uint8_t font_px_{12};；它会更新一次变量、对象或程序状态。
    uint8_t font_px_{12};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::reader
