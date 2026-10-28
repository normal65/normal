/**
 * [初学者导读] 文件：main/app/ebook/apps/wooden_fish/wooden_fish_app.cc
 * 作用：实现电子木鱼点击、计数和音效播放。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/wooden_fish/wooden_fish_app.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "bsp/driver/gdey027t91/framebuffer.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "common/time/time.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/font.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/icon.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "presenter/presenter.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/refresh_intent.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_bus.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "platform/wooden_fish_sound.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::wooden_fish {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using Fb = ::app::bsp::driver::gdey027t91::Framebuffer;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using Panel = ::app::bsp::driver::gdey027t91::Panel;

// 固定参数 kStickIconPx = 36：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kStickIconPx = 36;
// 固定参数 kStickPadPx = 4：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kStickPadPx = 4;
// 固定参数 kStickTile = kStickIconPx + kStickPadPx * 2U：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kStickTile = kStickIconPx + kStickPadPx * 2U;
// 固定参数 kStickTileStride = static_cast<uint8_t>((kStickTile + 7U) / 8U)：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kStickTileStride = static_cast<uint8_t>((kStickTile + 7U) / 8U);
// 固定参数 kFbStride = static_cast<uint8_t>(Panel::STRIDE)：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kFbStride = static_cast<uint8_t>(Panel::STRIDE);

// struct StickRot：把相关数据字段组合在一起，表示项目中的一个概念。
struct StickRot
{
    // 执行语句：float c;；它会更新一次变量、对象或程序状态。
    float c;
    // 执行语句：float s;；它会更新一次变量、对象或程序状态。
    float s;
};

// 待机 -0.35 rad / 敲击 -1.05 rad（绕 pivot 左旋）
constexpr StickRot kRotIdle = {0.939393f, -0.342900f};
// 固定参数 kRotKnock = {0.497571f, -0.867423f}：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr StickRot kRotKnock = {0.497571f, -0.867423f};

/** 1-bit：清位=黑（木鱼棒笔划），置位=白（透明底） */
// 定义或声明函数 stick_tile_black：参数在括号里，返回类型写在函数名前面。
bool stick_tile_black(const uint8_t* tile, uint8_t x, uint8_t y)
{
    // 判断条件：tile == nullptr || x >= kStickTile || y >= kStickTile 是否成立。
    if (tile == nullptr || x >= kStickTile || y >= kStickTile)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t mask = static_cast<uint8_t>(0x80U >> (x & 7U));
    // 返回 (tile[static_cast<uint32_t>(y) * kStickTileStride + (x / 8U)] & mask) == 0 给调用者，表示当前函数的执行结果。
    return (tile[static_cast<uint32_t>(y) * kStickTileStride + (x / 8U)] & mask) == 0;
}

// 定义或声明函数 stick_tile_set_black：参数在括号里，返回类型写在函数名前面。
void stick_tile_set_black(uint8_t* tile, uint8_t x, uint8_t y)
{
    // 判断条件：tile == nullptr || x >= kStickTile || y >= kStickTile 是否成立。
    if (tile == nullptr || x >= kStickTile || y >= kStickTile)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t mask = static_cast<uint8_t>(0x80U >> (x & 7U));
    // 赋值：把右侧 static_cast<uint8_t>(~mask) 的结果保存到左侧 tile[static_cast<uint32_t>(y) * kStickTileStride + (x / 8U)] &。
    tile[static_cast<uint32_t>(y) * kStickTileStride + (x / 8U)] &= static_cast<uint8_t>(~mask);
}

// 定义或声明函数 render_stick_source：参数在括号里，返回类型写在函数名前面。
void render_stick_source(uint8_t* tile)
{
    // 判断条件：tile == nullptr 是否成立。
    if (tile == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 std::memset(tile, 0xFF, static_cast<size_t>(kStickTileStride) * kStickTile);，执行这个函数并处理它的返回值或状态变化。
    std::memset(tile, 0xFF, static_cast<size_t>(kStickTileStride) * kStickTile);

    // Canvas/Fb::set_pixel 按 Panel::STRIDE 寻址，缓冲须匹配行宽
    static uint8_t tmp[static_cast<uint32_t>(kFbStride) * kStickTile]{};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect tile_r{0, 0, kStickTile, kStickTile};
    // 执行语句：gfx::Canvas tmp_canvas{tmp, tile_r};；它会更新一次变量、对象或程序状态。
    gfx::Canvas tmp_canvas{tmp, tile_r};
    // 访问对象成员：tmp_canvas.clear(gfx::Ink::White);；读取或修改这个对象里的成员状态。
    tmp_canvas.clear(gfx::Ink::White);

    // 赋值：把右侧 gfx::Font::get_instance() 的结果保存到左侧 auto& font。
    auto& font = gfx::Font::get_instance();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t adv = font.advance(gfx::icon::kEbWoodenStick, kStickIconPx, gfx::FontFace::Icon);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t gx = static_cast<int16_t>((kStickTile - adv) / 2);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t gy = static_cast<int16_t>((kStickTile - kStickIconPx) / 2);
    // 访问对象成员：(void)tmp_canvas.glyph(gx, gy, gfx::icon::kEbWoodenStick, kStickIconPx, gfx::FontFace::Icon);；读取或修改这个对象里的成员状态。
    (void)tmp_canvas.glyph(gx, gy, gfx::icon::kEbWoodenStick, kStickIconPx, gfx::FontFace::Icon);

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t y = 0; y < kStickTile; ++y)
    {
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint8_t x = 0; x < kStickTile; ++x)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint8_t mask = static_cast<uint8_t>(0x80U >> (x & 7U));
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint32_t src =
                // 继续上一行的表达式或参数列表：static_cast<uint32_t>(y) * kFbStride + static_cast<uint32_t>(x / 8U);。
                static_cast<uint32_t>(y) * kFbStride + static_cast<uint32_t>(x / 8U);
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint32_t dst =
                // 继续上一行的表达式或参数列表：static_cast<uint32_t>(y) * kStickTileStride + static_cast<uint32_t>(x / 8U);。
                static_cast<uint32_t>(y) * kStickTileStride + static_cast<uint32_t>(x / 8U);
            // 判断条件：(tmp[src] & mask) == 0 是否成立。
            if ((tmp[src] & mask) == 0)
                // 赋值：把右侧 static_cast<uint8_t>(~mask) 的结果保存到左侧 tile[dst] &。
                tile[dst] &= static_cast<uint8_t>(~mask);
        }
    }
}

/** 逆映射旋转烘焙，避免正向逐点投射产生的孤立噪点 */
// 定义或声明函数 bake_rotated_stick：参数在括号里，返回类型写在函数名前面。
void bake_rotated_stick(uint8_t* out, const uint8_t* src, const StickRot& rot)
{
    // 判断条件：out == nullptr || src == nullptr 是否成立。
    if (out == nullptr || src == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 调用 std::memset(out, 0xFF, static_cast<size_t>(kStickTileStride) * kStickTile);，执行这个函数并处理它的返回值或状态变化。
    std::memset(out, 0xFF, static_cast<size_t>(kStickTileStride) * kStickTile);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const float cx = static_cast<float>(kStickTile) * 0.5f;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const float cy = static_cast<float>(kStickTile) * 0.5f;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t dy = 0; dy < kStickTile; ++dy)
    {
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint8_t dx = 0; dx < kStickTile; ++dx)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const float ox = static_cast<float>(dx) - cx;
            // 只读约束：这个值或对象在当前位置不能被修改。
            const float oy = static_cast<float>(dy) - cy;
            // 只读约束：这个值或对象在当前位置不能被修改。
            const float sx = ox * rot.c + oy * rot.s;
            // 只读约束：这个值或对象在当前位置不能被修改。
            const float sy = -ox * rot.s + oy * rot.c;
            // 只读约束：这个值或对象在当前位置不能被修改。
            const int16_t ix = static_cast<int16_t>(sx + cx + 0.5f);
            // 只读约束：这个值或对象在当前位置不能被修改。
            const int16_t iy = static_cast<int16_t>(sy + cy + 0.5f);
            // 判断条件：ix < 0 || iy < 0 || ix >= kStickTile || iy >= kStickTile 是否成立。
            if (ix < 0 || iy < 0 || ix >= kStickTile || iy >= kStickTile)
                // 继续下一轮：跳过本次循环剩余代码。
                continue;
            // 判断条件：stick_tile_black(src, static_cast<uint8_t>(ix), static_cast<uint8_t>(iy)) 是否成立。
            if (stick_tile_black(src, static_cast<uint8_t>(ix), static_cast<uint8_t>(iy)))
                // 调用 stick_tile_set_black(out, dx, dy);，执行这个函数并处理它的返回值或状态变化。
                stick_tile_set_black(out, dx, dy);
        }
    }
}

/** 去掉完全孤立的单像素噪点 */
// 定义或声明函数 prune_isolated_stick：参数在括号里，返回类型写在函数名前面。
void prune_isolated_stick(uint8_t* tile)
{
    // 判断条件：tile == nullptr 是否成立。
    if (tile == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t y = 0; y < kStickTile; ++y)
    {
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint8_t x = 0; x < kStickTile; ++x)
        {
            // 判断条件：!stick_tile_black(tile, x, y) 是否成立。
            if (!stick_tile_black(tile, x, y))
                // 继续下一轮：跳过本次循环剩余代码。
                continue;

            // 定义变量 uint8_t n 并初始化为 0。
            uint8_t n = 0;
            // 判断条件：x > 0 && stick_tile_black(tile, static_cast<uint8_t>(x - 1U), y) 是否成立。
            if (x > 0 && stick_tile_black(tile, static_cast<uint8_t>(x - 1U), y))
                // 执行语句：++n;；它会更新一次变量、对象或程序状态。
                ++n;
            // 判断条件：x + 1U < kStickTile && stick_tile_black(tile, static_cast<uint8_t>(x + 1U), y) 是否成立。
            if (x + 1U < kStickTile && stick_tile_black(tile, static_cast<uint8_t>(x + 1U), y))
                // 执行语句：++n;；它会更新一次变量、对象或程序状态。
                ++n;
            // 判断条件：y > 0 && stick_tile_black(tile, x, static_cast<uint8_t>(y - 1U)) 是否成立。
            if (y > 0 && stick_tile_black(tile, x, static_cast<uint8_t>(y - 1U)))
                // 执行语句：++n;；它会更新一次变量、对象或程序状态。
                ++n;
            // 判断条件：y + 1U < kStickTile && stick_tile_black(tile, x, static_cast<uint8_t>(y + 1U)) 是否成立。
            if (y + 1U < kStickTile && stick_tile_black(tile, x, static_cast<uint8_t>(y + 1U)))
                // 执行语句：++n;；它会更新一次变量、对象或程序状态。
                ++n;

            // 判断条件：n == 0 是否成立。
            if (n == 0)
            {
                // 只读约束：这个值或对象在当前位置不能被修改。
                const uint8_t mask = static_cast<uint8_t>(0x80U >> (x & 7U));
                // 赋值：把右侧 mask 的结果保存到左侧 tile[static_cast<uint32_t>(y) * kStickTileStride + (x / 8U)] |。
                tile[static_cast<uint32_t>(y) * kStickTileStride + (x / 8U)] |= mask;
            }
        }
    }
}

// 函数调用：void blit_stick_tile(uint8_t* fb, const uint8_t* tile, int16_t pivot_x, int16_t pivot_y,；执行该函数并处理它的返回值或副作用。
void blit_stick_tile(uint8_t* fb, const uint8_t* tile, int16_t pivot_x, int16_t pivot_y,
                     // 只读约束：这个值或对象在当前位置不能被修改。
                     const core::Rect& clip)
{
    // 判断条件：fb == nullptr || tile == nullptr 是否成立。
    if (fb == nullptr || tile == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t ox = static_cast<int16_t>(pivot_x - kStickTile / 2);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t oy = static_cast<int16_t>(pivot_y - kStickTile / 2);

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t sy = 0; sy < kStickTile; ++sy)
    {
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint8_t sx = 0; sx < kStickTile; ++sx)
        {
            // 判断条件：!stick_tile_black(tile, sx, sy) 是否成立。
            if (!stick_tile_black(tile, sx, sy))
                // 继续下一轮：跳过本次循环剩余代码。
                continue;

            // 只读约束：这个值或对象在当前位置不能被修改。
            const int16_t px = static_cast<int16_t>(ox + sx);
            // 只读约束：这个值或对象在当前位置不能被修改。
            const int16_t py = static_cast<int16_t>(oy + sy);
            // 判断条件：!clip.contains(px, py) 是否成立。
            if (!clip.contains(px, py))
                // 继续下一轮：跳过本次循环剩余代码。
                continue;
            // 调用 Fb::set_pixel(fb, px, py, true);，执行这个函数并处理它的返回值或状态变化。
            Fb::set_pixel(fb, px, py, true);
        }
    }
}

// struct StickTiles：把相关数据字段组合在一起，表示项目中的一个概念。
struct StickTiles
{
    // 执行语句：uint8_t src[kStickTileStride * kStickTile]{};；它会更新一次变量、对象或程序状态。
    uint8_t src[kStickTileStride * kStickTile]{};
    // 执行语句：uint8_t idle[kStickTileStride * kStickTile]{};；它会更新一次变量、对象或程序状态。
    uint8_t idle[kStickTileStride * kStickTile]{};
    // 执行语句：uint8_t knock[kStickTileStride * kStickTile]{};；它会更新一次变量、对象或程序状态。
    uint8_t knock[kStickTileStride * kStickTile]{};
    // 执行语句：bool ready{false};；它会更新一次变量、对象或程序状态。
    bool ready{false};
};

// 定义或声明函数 ensure_stick_tiles：参数在括号里，返回类型写在函数名前面。
void ensure_stick_tiles(StickTiles& t)
{
    // 判断条件：t.ready 是否成立。
    if (t.ready)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 调用 render_stick_source(t.src);，执行这个函数并处理它的返回值或状态变化。
    render_stick_source(t.src);
    // 调用 bake_rotated_stick(t.idle, t.src, kRotIdle);，执行这个函数并处理它的返回值或状态变化。
    bake_rotated_stick(t.idle, t.src, kRotIdle);
    // 调用 bake_rotated_stick(t.knock, t.src, kRotKnock);，执行这个函数并处理它的返回值或状态变化。
    bake_rotated_stick(t.knock, t.src, kRotKnock);
    // 调用 prune_isolated_stick(t.idle);，执行这个函数并处理它的返回值或状态变化。
    prune_isolated_stick(t.idle);
    // 调用 prune_isolated_stick(t.knock);，执行这个函数并处理它的返回值或状态变化。
    prune_isolated_stick(t.knock);
    // 赋值：把右侧 true 的结果保存到左侧 t.ready。
    t.ready = true;
}

// 定义或声明函数 anim_timer_cb：参数在括号里，返回类型写在函数名前面。
void anim_timer_cb(void* /*arg*/)
{
    // 访问对象成员：(void)ui::UiBus::get_instance().post_system_hint(ui::SystemHintKind::WoodenFishAnimDone, 0);；读取或修改这个对象里的成员状态。
    (void)ui::UiBus::get_instance().post_system_hint(ui::SystemHintKind::WoodenFishAnimDone, 0);
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 WoodenFishApp::instance：参数在括号里，返回类型写在函数名前面。
WoodenFishApp& WoodenFishApp::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static WoodenFishApp s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义只读对象或变量：初始化后不能在当前位置修改。
const char* WoodenFishApp::title() const
{
    // 返回 ui::strings::kAppFish 给调用者，表示当前函数的执行结果。
    return ui::strings::kAppFish;
}

// 函数定义/声明：调用 WoodenFishApp::icon_cp()：返回图标 codepoint。图标通常来自字体图标表，所以这里返回的是 uint32t，不是图片路径。。
uint32_t WoodenFishApp::icon_cp() const
{
    // 返回 gfx::icon::kEbWoodenFish 给调用者，表示当前函数的执行结果。
    return gfx::icon::kEbWoodenFish;
}

// 定义或声明函数 WoodenFishApp::body_rect：参数在括号里，返回类型写在函数名前面。
core::Rect WoodenFishApp::body_rect()
{
    // 返回 core::Rect{0, static_cast<int16_t>(Theme::kListStartY), Theme::kScreenW, 给调用者，表示当前函数的执行结果。
    return core::Rect{0, static_cast<int16_t>(Theme::kListStartY), Theme::kScreenW,
                      // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kScreenH - Theme::kListStartY)};。
                      static_cast<uint16_t>(Theme::kScreenH - Theme::kListStartY)};
}

// 定义或声明函数 WoodenFishApp::fish_rect：参数在括号里，返回类型写在函数名前面。
core::Rect WoodenFishApp::fish_rect()
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect body = body_rect();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x = static_cast<int16_t>(body.x + (body.w - kFishIconSize) / 2);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y = static_cast<int16_t>(body.y + (body.h - kFishIconSize) / 2 + 8);
    // 返回 core::Rect{x, y, kFishIconSize, kFishIconSize} 给调用者，表示当前函数的执行结果。
    return core::Rect{x, y, kFishIconSize, kFishIconSize};
}

// 定义或声明函数 WoodenFishApp::stick_pivot：参数在括号里，返回类型写在函数名前面。
WoodenFishApp::StickPivot WoodenFishApp::stick_pivot()
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect fish = fish_rect();
    // 声明并默认初始化对象：StickPivot p{}。
    StickPivot p{};
    // 赋值：把右侧 static_cast<int16_t>(fish.right() + 2) 的结果保存到左侧 p.x。
    p.x = static_cast<int16_t>(fish.right() + 2);
    // 赋值：把右侧 static_cast<int16_t>(fish.y + 2) 的结果保存到左侧 p.y。
    p.y = static_cast<int16_t>(fish.y + 2);
    // 返回 p 给调用者，表示当前函数的执行结果。
    return p;
}

// 定义或声明函数 WoodenFishApp::merit_rect：参数在括号里，返回类型写在函数名前面。
core::Rect WoodenFishApp::merit_rect()
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect body = body_rect();
    // 返回 core::Rect{body.x, static_cast<int16_t>(body.y + 12), body.w, 36} 给调用者，表示当前函数的执行结果。
    return core::Rect{body.x, static_cast<int16_t>(body.y + 12), body.w, 36};
}

// 定义或声明函数 WoodenFishApp::paint_stick：参数在括号里，返回类型写在函数名前面。
void WoodenFishApp::paint_stick(gfx::Canvas& c, bool knocking)
{
    // 静态空对象 s_tiles：作为默认返回值或占位值，避免返回悬空引用。
    static StickTiles s_tiles{};
    // 调用 ensure_stick_tiles(s_tiles);，执行这个函数并处理它的返回值或状态变化。
    ensure_stick_tiles(s_tiles);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const StickPivot pivot = stick_pivot();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t* tile = knocking ? s_tiles.knock : s_tiles.idle;
    // 调用 blit_stick_tile(c.fb(), tile, pivot.x, pivot.y, c.clip());，执行这个函数并处理它的返回值或状态变化。
    blit_stick_tile(c.fb(), tile, pivot.x, pivot.y, c.clip());
}

// 定义或声明函数 WoodenFishApp::ensure_anim_timer：参数在括号里，返回类型写在函数名前面。
void WoodenFishApp::ensure_anim_timer()
{
    // 判断条件：anim_timer_ != nullptr 是否成立。
    if (anim_timer_ != nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 声明并默认初始化对象：esp_timer_create_args_t args{}。
    esp_timer_create_args_t args{};
    // 赋值：把右侧 anim_timer_cb 的结果保存到左侧 args.callback。
    args.callback = anim_timer_cb;
    // 赋值：把右侧 nullptr 的结果保存到左侧 args.arg。
    args.arg = nullptr;
    // 赋值：把右侧 "fish_anim" 的结果保存到左侧 args.name。
    args.name = "fish_anim";
    // 赋值：把右侧 ESP_TIMER_TASK 的结果保存到左侧 args.dispatch_method。
    args.dispatch_method = ESP_TIMER_TASK;
    // 赋值：把右侧 true 的结果保存到左侧 args.skip_unhandled_events。
    args.skip_unhandled_events = true;
    // 函数调用：(void)esp_timer_create(&args, &anim_timer_);；执行该函数并处理它的返回值或副作用。
    (void)esp_timer_create(&args, &anim_timer_);
}

// 定义或声明函数 WoodenFishApp::arm_anim_timer：参数在括号里，返回类型写在函数名前面。
void WoodenFishApp::arm_anim_timer()
{
    // 调用 ensure_anim_timer();，执行这个函数并处理它的返回值或状态变化。
    ensure_anim_timer();
    // 判断条件：anim_timer_ == nullptr 是否成立。
    if (anim_timer_ == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 函数调用：(void)esp_timer_stop(anim_timer_);；执行该函数并处理它的返回值或副作用。
    (void)esp_timer_stop(anim_timer_);
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    (void)esp_timer_start_once(anim_timer_, static_cast<uint64_t>(kKnockAnimMs) * 1000ULL);
}

// 定义或声明函数 WoodenFishApp::on_enter：参数在括号里，返回类型写在函数名前面。
void WoodenFishApp::on_enter()
{
    // 调用 ensure_anim_timer();，执行这个函数并处理它的返回值或状态变化。
    ensure_anim_timer();
    // 赋值：把右侧 0 的结果保存到左侧 knock_anim_until_ms_。
    knock_anim_until_ms_ = 0;
    // 赋值：把右侧 0 的结果保存到左侧 last_repaint_ms_。
    last_repaint_ms_ = 0;
    // 赋值：把右侧 false 的结果保存到左侧 pending_repaint_。
    pending_repaint_ = false;
    // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
    request_repaint(router::intent_partial_full());
}

// 函数定义/声明：调用 WoodenFishApp::on_exit()：退出时取消未完成的扫描、关闭弹窗，并确保后台任务不会在对象状态失效后继续写数据。。
void WoodenFishApp::on_exit()
{
    // 赋值：把右侧 0 的结果保存到左侧 knock_anim_until_ms_。
    knock_anim_until_ms_ = 0;
    // 判断条件：anim_timer_ != nullptr 是否成立。
    if (anim_timer_ != nullptr)
        // 函数调用：(void)esp_timer_stop(anim_timer_);；执行该函数并处理它的返回值或副作用。
        (void)esp_timer_stop(anim_timer_);
}

// 定义或声明函数 WoodenFishApp::schedule_repaint：参数在括号里，返回类型写在函数名前面。
void WoodenFishApp::schedule_repaint(bool force)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int64_t now = ::app::common::time::uptime_ms();
    // 判断条件：!force 是否成立。
    if (!force)
    {
        // 判断条件：(now - last_repaint_ms_) < kRepaintMinMs 是否成立。
        if ((now - last_repaint_ms_) < kRepaintMinMs)
        {
            // 赋值：把右侧 true 的结果保存到左侧 pending_repaint_。
            pending_repaint_ = true;
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }
        // 判断条件：!presenter::Presenter::instance().wait_idle(0) 是否成立。
        if (!presenter::Presenter::instance().wait_idle(0))
        {
            // 赋值：把右侧 true 的结果保存到左侧 pending_repaint_。
            pending_repaint_ = true;
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }
    }

    // 赋值：把右侧 now 的结果保存到左侧 last_repaint_ms_。
    last_repaint_ms_ = now;
    // 赋值：把右侧 false 的结果保存到左侧 pending_repaint_。
    pending_repaint_ = false;
    // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
    request_repaint(router::intent_partial_full());
}

// 定义或声明函数 WoodenFishApp::on_knock：参数在括号里，返回类型写在函数名前面。
void WoodenFishApp::on_knock()
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int64_t now = ::app::common::time::uptime_ms();
    // 赋值：把右侧 now + kKnockAnimMs 的结果保存到左侧 knock_anim_until_ms_。
    knock_anim_until_ms_ = now + kKnockAnimMs;
    // 执行语句：++merit_;；它会更新一次变量、对象或程序状态。
    ++merit_;
    // 访问对象成员：(void)platform::WoodenFishSound::get_instance().play_knock();；读取或修改这个对象里的成员状态。
    (void)platform::WoodenFishSound::get_instance().play_knock();
    // 调用 arm_anim_timer();，执行这个函数并处理它的返回值或状态变化。
    arm_anim_timer();
    // 调用 schedule_repaint(true);，执行这个函数并处理它的返回值或状态变化。
    schedule_repaint(true);
}

// 定义或声明函数 WoodenFishApp::on_ui_event：参数在括号里，返回类型写在函数名前面。
void WoodenFishApp::on_ui_event(const ui::UiEvent& ev)
{
    // 判断条件：ev.kind != ui::UiEventKind::SystemHint 是否成立。
    if (ev.kind != ui::UiEventKind::SystemHint)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：ev.payload.system.hint != ui::SystemHintKind::WoodenFishAnimDone 是否成立。
    if (ev.payload.system.hint != ui::SystemHintKind::WoodenFishAnimDone)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 调用 schedule_repaint(true);，执行这个函数并处理它的返回值或状态变化。
    schedule_repaint(true);
}

// 定义或声明函数 WoodenFishApp::paint：参数在括号里，返回类型写在函数名前面。
void WoodenFishApp::paint(gfx::Canvas& c)
{
    // 调用 title()：返回 App 标题。在文件浏览器内，标题有时会动态反映当前目录，因此可能存在一个 buffer，而不是只返回一个常量字符串。。
    ui::widgets::toolbar(c, title());

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int64_t now = ::app::common::time::uptime_ms();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool knocking = now < knock_anim_until_ms_;

    // 执行语句：char merit_buf[24];；它会更新一次变量、对象或程序状态。
    char merit_buf[24];
    // 函数调用：(void)std::snprintf(merit_buf, sizeof(merit_buf), ui::strings::kFishMeritFmt,；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(merit_buf, sizeof(merit_buf), ui::strings::kFishMeritFmt,
                        // 继续上一行的表达式或参数列表：static_cast<unsigned>(merit_));。
                        static_cast<unsigned>(merit_));

    // 声明并默认初始化对象：gfx::Canvas::TextStyle merit_ts{}。
    gfx::Canvas::TextStyle merit_ts{};
    // 赋值：把右侧 Theme::kFontLarge 的结果保存到左侧 merit_ts.size_px。
    merit_ts.size_px = Theme::kFontLarge;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 merit_ts.h。
    merit_ts.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 merit_ts.v。
    merit_ts.v = gfx::VAlign::Middle;
    // 访问对象成员：c.text_in(merit_rect(), merit_buf, merit_ts);；读取或修改这个对象里的成员状态。
    c.text_in(merit_rect(), merit_buf, merit_ts);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect fish = fish_rect();
    // 访问对象成员：c.glyph(fish.x, fish.y, gfx::icon::kEbWoodenFish, kFishIconSize, gfx::FontFace::Icon);；读取或修改这个对象里的成员状态。
    c.glyph(fish.x, fish.y, gfx::icon::kEbWoodenFish, kFishIconSize, gfx::FontFace::Icon);
    // 调用 paint_stick(c, knocking);，执行这个函数并处理它的返回值或状态变化。
    paint_stick(c, knocking);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect body = body_rect();
    // 声明并默认初始化对象：gfx::Canvas::TextStyle hint_ts{}。
    gfx::Canvas::TextStyle hint_ts{};
    // 赋值：把右侧 Theme::kFontSmall 的结果保存到左侧 hint_ts.size_px。
    hint_ts.size_px = Theme::kFontSmall;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 hint_ts.h。
    hint_ts.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Bottom 的结果保存到左侧 hint_ts.v。
    hint_ts.v = gfx::VAlign::Bottom;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect hint_r{body.x, static_cast<int16_t>(body.bottom() - 22), body.w, 18};
    // 访问对象成员：c.text_in(hint_r, ui::strings::kFishTapHint, hint_ts);；读取或修改这个对象里的成员状态。
    c.text_in(hint_r, ui::strings::kFishTapHint, hint_ts);
}

// 定义或声明函数 WoodenFishApp::on_input：参数在括号里，返回类型写在函数名前面。
shell::InputResult WoodenFishApp::on_input(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;

    // 判断条件：ev.type != EventType::Tap 是否成立。
    if (ev.type != EventType::Tap)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x = static_cast<int16_t>(ev.x);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y = static_cast<int16_t>(ev.y);

    // 判断条件：ui::widgets::hit_toolbar_back(x, y) 是否成立。
    if (ui::widgets::hit_toolbar_back(x, y))
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 判断条件：!body_rect().contains(x, y) 是否成立。
    if (!body_rect().contains(x, y))
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};

    // 调用 on_knock();，执行这个函数并处理它的返回值或状态变化。
    on_knock();
    // 返回 {true} 给调用者，表示当前函数的执行结果。
    return {true};
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::wooden_fish
