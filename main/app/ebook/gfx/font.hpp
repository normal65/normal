// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/gfx/font.hpp
 * 作用：管理字体加载、字形缓存、文本测量和绘制。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：绘制只改 framebuffer，不直接操作 SPI。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstddef>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <mutex>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/geometry.hpp"

// struct FT_LibraryRec_：把相关数据字段组合在一起，表示项目中的一个概念。
struct FT_LibraryRec_;
// struct FT_FaceRec_：把相关数据字段组合在一起，表示项目中的一个概念。
struct FT_FaceRec_;
// 定义类型别名：给已有类型起一个更容易理解的名字。
typedef struct FT_LibraryRec_* FT_Library;
// 定义类型别名：给已有类型起一个更容易理解的名字。
typedef struct FT_FaceRec_* FT_Face;

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::gfx {

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class FontFace : uint8_t
{
    // 执行这条代码：Text = 0,。它会更新变量、对象状态或调用链。
    Text = 0,
    // 继续上一行的表达式或参数列表：Icon = 1,。
    Icon = 1,
};

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class Ink : uint8_t
{
    // 执行这条代码：Black = 0,。它会更新变量、对象状态或调用链。
    Black = 0,
    // 继续上一行的表达式或参数列表：White = 1,。
    White = 1,
};

// struct GlyphMetrics：把相关数据字段组合在一起，表示项目中的一个概念。
struct GlyphMetrics
{
    // 执行语句：int16_t bearing_x{0};；它会更新一次变量、对象或程序状态。
    int16_t bearing_x{0};
    // 执行语句：int16_t bearing_y{0};；它会更新一次变量、对象或程序状态。
    int16_t bearing_y{0};
    // 执行语句：uint16_t width{0};；它会更新一次变量、对象或程序状态。
    uint16_t width{0};
    // 执行语句：uint16_t height{0};；它会更新一次变量、对象或程序状态。
    uint16_t height{0};
    // 执行语句：uint16_t advance{0};；它会更新一次变量、对象或程序状态。
    uint16_t advance{0};
};

/**
 * @brief 1-bit 字体：BitmapFont 12/14/16 + FreeType fallback + LRU 缓存
 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class Font
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 固定参数 kCacheSlots = 256：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr size_t kCacheSlots = 256;

    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static Font& get_instance();

    // 函数调用：bool init();；执行该函数并处理它的返回值或副作用。
    bool init();
    // 调用 deinit()：创建和销毁队列。
    void deinit();
    // 定义或声明函数 ready：参数在括号里，返回类型写在函数名前面。
    bool ready() const
    {
        // 返回 ready_ 给调用者，表示当前函数的执行结果。
        return ready_;
    }

    // 函数调用：uint16_t draw_glyph(uint8_t* fb, int16_t x, int16_t y_top, uint32_t cp, uint8_t size_px,；执行该函数并处理它的返回值或副作用。
    uint16_t draw_glyph(uint8_t* fb, int16_t x, int16_t y_top, uint32_t cp, uint8_t size_px,
                        // 继续上一行的表达式或参数列表：FontFace face, Ink ink, const core::Rect& clip);。
                        FontFace face, Ink ink, const core::Rect& clip);

    // 函数调用：uint16_t draw_text(uint8_t* fb, int16_t x, int16_t y_top, const char* utf8, uint8_t size_px,；执行该函数并处理它的返回值或副作用。
    uint16_t draw_text(uint8_t* fb, int16_t x, int16_t y_top, const char* utf8, uint8_t size_px,
                       // 继续上一行的表达式或参数列表：FontFace face, Ink ink, const core::Rect& clip);。
                       FontFace face, Ink ink, const core::Rect& clip);

    // 调用 advance(uint32_tcp,uint8_tsize_px,FontFaceface)：advance(cp)： 查一个字符的前进宽度。点阵优先，FreeType 兜底。它不会绘制任何像素，适合排版测量。。
    uint16_t advance(uint32_t cp, uint8_t size_px, FontFace face);
    // 赋值：把右侧 FontFace::Text) 的结果保存到左侧 uint16_t measure(const char* utf8, uint8_t size_px, FontFace face。
    uint16_t measure(const char* utf8, uint8_t size_px, FontFace face = FontFace::Text);
    // 赋值：把右侧 FontFace::Text) 的结果保存到左侧 uint16_t line_height(uint8_t size_px, FontFace face。
    uint16_t line_height(uint8_t size_px, FontFace face = FontFace::Text);
    // 赋值：把右侧 FontFace::Text) 的结果保存到左侧 uint16_t ascent(uint8_t size_px, FontFace face。
    uint16_t ascent(uint8_t size_px, FontFace face = FontFace::Text);

    // struct Stats：把相关数据字段组合在一起，表示项目中的一个概念。
    struct Stats
    {
        // 执行语句：uint32_t hits{0};；它会更新一次变量、对象或程序状态。
        uint32_t hits{0};
        // 执行语句：uint32_t misses{0};；它会更新一次变量、对象或程序状态。
        uint32_t misses{0};
        // 执行语句：uint32_t evictions{0};；它会更新一次变量、对象或程序状态。
        uint32_t evictions{0};
        // 执行语句：uint32_t bitmap_bytes{0};；它会更新一次变量、对象或程序状态。
        uint32_t bitmap_bytes{0};
    };
    // 定义或声明函数 stats：参数在括号里，返回类型写在函数名前面。
    Stats stats() const
    {
        // 返回 stats_ 给调用者，表示当前函数的执行结果。
        return stats_;
    }
    // 定义或声明函数 reset_stats：参数在括号里，返回类型写在函数名前面。
    void reset_stats()
    {
        // 赋值：把右侧 {} 的结果保存到左侧 stats_。
        stats_ = {};
    }

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 Font() = default;，执行这个函数并处理它的返回值或状态变化。
    Font() = default;
    // 赋值：把右侧 default 的结果保存到左侧 ~Font()。
    ~Font() = default;
    // 调用 Font(const Font&) = delete;，执行这个函数并处理它的返回值或状态变化。
    Font(const Font&) = delete;
    // 赋值：把右侧 (const Font&) = delete 的结果保存到左侧 Font& operator。
    Font& operator=(const Font&) = delete;

    // struct Slot：把相关数据字段组合在一起，表示项目中的一个概念。
    struct Slot
    {
        // 执行语句：uint32_t key{0};；它会更新一次变量、对象或程序状态。
        uint32_t key{0};
        // 执行语句：uint32_t lru{0};；它会更新一次变量、对象或程序状态。
        uint32_t lru{0};
        // 声明并默认初始化对象：GlyphMetrics m{}。
        GlyphMetrics m{};
        // 执行语句：uint8_t* bitmap{nullptr};；它会更新一次变量、对象或程序状态。
        uint8_t* bitmap{nullptr};
        // 执行语句：uint16_t bitmap_bytes{0};；它会更新一次变量、对象或程序状态。
        uint16_t bitmap_bytes{0};
    };

    // 静态函数 make_key：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static uint32_t make_key(uint32_t cp, uint8_t size_px, FontFace face);
    // 静态函数 hash_slot：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static size_t hash_slot(uint32_t key);

    // 调用 cache_find_locked(uint32_tkey)：从一个 hash 位置开始，最多探测 16 个连续槽： key 完全相等就命中。 遇到空槽说明这个 key 不存在，停止探测。。
    Slot* cache_find_locked(uint32_t key);
    // 调用 cache_replace_locked(uint32_tkey)：先找空槽，找到就直接用。若 16 个探测槽全满，就在这 16 个里找 lru 最小的槽淘汰。。
    Slot* cache_replace_locked(uint32_t key);
    // 调用 render_into_locked(uint32_tcp,uint8_tsize_px,FontFaceface)：FreeType 渲染一个字。
    Slot* render_into_locked(uint32_t cp, uint8_t size_px, FontFace face);

    // 调用 active_face_locked(FontFaceface)：activefacelocked()： 根据 FontFace 返回文本字体或图标字体。。
    FT_Face active_face_locked(FontFace face);
    // 调用 select_size_locked(FontFaceface,uint8_tsize_px)：activefacelocked()： 根据 FontFace 返回文本字体或图标字体。。
    bool select_size_locked(FontFace face, uint8_t size_px);

    // 函数调用：void blit_locked(uint8_t* fb, int16_t pen_x, int16_t baseline_y, const GlyphMetrics& m,；执行该函数并处理它的返回值或副作用。
    void blit_locked(uint8_t* fb, int16_t pen_x, int16_t baseline_y, const GlyphMetrics& m,
                     // 只读约束：这个值或对象在当前位置不能被修改。
                     const uint8_t* bitmap, Ink ink, const core::Rect& clip);

    // 函数调用：void blit_bitmap_glyph_locked(uint8_t* fb, int16_t pen_x, int16_t baseline_y,；执行该函数并处理它的返回值或副作用。
    void blit_bitmap_glyph_locked(uint8_t* fb, int16_t pen_x, int16_t baseline_y,
                                  // 只读约束：这个值或对象在当前位置不能被修改。
                                  const GlyphMetrics& m, const uint8_t* bitmap, uint8_t size_px,
                                  // 继续上一行的表达式或参数列表：Ink ink, const core::Rect& clip);。
                                  Ink ink, const core::Rect& clip);

    // 执行语句：FT_Library lib_{nullptr};；它会更新一次变量、对象或程序状态。
    FT_Library lib_{nullptr};
    // 执行语句：FT_Face face_text_{nullptr};；它会更新一次变量、对象或程序状态。
    FT_Face face_text_{nullptr};
    // 执行语句：FT_Face face_icon_{nullptr};；它会更新一次变量、对象或程序状态。
    FT_Face face_icon_{nullptr};
    // 执行语句：uint8_t size_text_{0};；它会更新一次变量、对象或程序状态。
    uint8_t size_text_{0};
    // 执行语句：uint8_t size_icon_{0};；它会更新一次变量、对象或程序状态。
    uint8_t size_icon_{0};
    // 执行语句：bool ready_{false};；它会更新一次变量、对象或程序状态。
    bool ready_{false};
    // 执行语句：bool bitmap_only_text_{false};；它会更新一次变量、对象或程序状态。
    bool bitmap_only_text_{false};

    // 执行语句：Slot* cache_{nullptr};；它会更新一次变量、对象或程序状态。
    Slot* cache_{nullptr};
    // 执行语句：uint32_t access_seq_{0};；它会更新一次变量、对象或程序状态。
    uint32_t access_seq_{0};
    // 声明并默认初始化对象：Stats stats_{}。
    Stats stats_{};

    // 执行语句：mutable std::mutex mtx_;；它会更新一次变量、对象或程序状态。
    mutable std::mutex mtx_;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::gfx
