/**
 * [初学者导读] 文件：main/app/ebook/apps/stub_app.cc
 * 作用：实现尚未完成功能的占位 App。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/stub_app.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps {

// 定义或声明函数 StubApp::paint：参数在括号里，返回类型写在函数名前面。
void StubApp::paint(gfx::Canvas& c)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ui::Theme;

    // 调用 ui::widgets::toolbar(c,title_)：toolbar()： 在固定工具栏矩形中画返回图标、标题和底部横线。返回图标使用 Font Awesome 的码点，而不是手绘 SVG。。
    ui::widgets::toolbar(c, title_);

    // 判断条件：icon_cp_ != 0 是否成立。
    if (icon_cp_ != 0)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t x = static_cast<int16_t>((Theme::kScreenW - Theme::kFontIconLg) / 2);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t y = static_cast<int16_t>(Theme::kListStartY + 40);
        // 访问对象成员：c.glyph(x, y, icon_cp_, Theme::kFontIconLg, gfx::FontFace::Icon);；读取或修改这个对象里的成员状态。
        c.glyph(x, y, icon_cp_, Theme::kFontIconLg, gfx::FontFace::Icon);
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect text_box{
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        0, static_cast<int16_t>(Theme::kListStartY + 40 + Theme::kFontIconLg + 12), Theme::kScreenW,
        // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kFontBody + 4)};。
        static_cast<uint16_t>(Theme::kFontBody + 4)};
    // 声明并默认初始化对象：gfx::Canvas::TextStyle s{}。
    gfx::Canvas::TextStyle s{};
    // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 s.size_px。
    s.size_px = Theme::kFontBody;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 s.h。
    s.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Top 的结果保存到左侧 s.v。
    s.v = gfx::VAlign::Top;
    // 访问对象成员：c.text_in(text_box, ui::strings::kWip, s);；读取或修改这个对象里的成员状态。
    c.text_in(text_box, ui::strings::kWip, s);
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps
