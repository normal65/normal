/**
 * [初学者导读] 文件：main/app/ebook/gfx/font.cc
 * 作用：管理字体加载、字形缓存、文本测量和绘制。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：绘制只改 framebuffer，不直接操作 SPI。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/font.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <climits>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_heap_caps.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <ft2build.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include FT_FREETYPE_H

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "common/storage/storage.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/bitmap_font.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/utf8.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "memory/psram_alloc.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "Font";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::gfx {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using app::common::memory::alloc_array_psram;
// 字体尺寸："/fonts/simhei.ttf" 像素。
constexpr const char* kFontFileText = "/fonts/simhei.ttf";
// 字体尺寸："/fonts/font_awesome_6.ttf" 像素。
constexpr const char* kFontFileIcon = "/fonts/font_awesome_6.ttf";
// 原始值换算比例：把传感器读数转换成物理单位。
constexpr uint8_t kBitmapScaleMinPx = 24;
// 固定参数 kProbeMax = 16：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr size_t kProbeMax = 16;

// 定义或声明函数 face_id：参数在括号里，返回类型写在函数名前面。
uint8_t face_id(FontFace f)
{
    // 返回 (f == FontFace::Icon) ? 1U : 0U 给调用者，表示当前函数的执行结果。
    return (f == FontFace::Icon) ? 1U : 0U;
}

// 定义或声明函数 psram_alloc：参数在括号里，返回类型写在函数名前面。
uint8_t* psram_alloc(size_t n)
{
    // 赋值：把右侧 static_cast<uint8_t*>(heap_caps_malloc(n, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)) 的结果保存到左侧 auto* p。
    auto* p = static_cast<uint8_t*>(heap_caps_malloc(n, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    // 判断条件：p == nullptr 是否成立。
    if (p == nullptr)
        // 赋值：把右侧 static_cast<uint8_t*>(heap_caps_malloc(n, MALLOC_CAP_8BIT)) 的结果保存到左侧 p。
        p = static_cast<uint8_t*>(heap_caps_malloc(n, MALLOC_CAP_8BIT));
    // 返回 p 给调用者，表示当前函数的执行结果。
    return p;
}

// 定义或声明函数 psram_free：参数在括号里，返回类型写在函数名前面。
void psram_free(uint8_t* p)
{
    // 判断条件：p != nullptr 是否成立。
    if (p != nullptr)
        // 调用 heap_caps_free(p);，执行这个函数并处理它的返回值或状态变化。
        heap_caps_free(p);
}

// 定义或声明函数 scale_metric：参数在括号里，返回类型写在函数名前面。
uint16_t scale_metric(uint16_t v, uint8_t from_px, uint8_t to_px)
{
    // 判断条件：from_px == 0 || to_px == from_px 是否成立。
    if (from_px == 0 || to_px == from_px)
        // 返回 v 给调用者，表示当前函数的执行结果。
        return v;
    // 返回 static_cast<uint16_t>((static_cast<uint32_t>(v) * to_px + from_px / 2U) / from_px) 给调用者，表示当前函数的执行结果。
    return static_cast<uint16_t>((static_cast<uint32_t>(v) * to_px + from_px / 2U) / from_px);
}

// 定义或声明函数 pick_scale：参数在括号里，返回类型写在函数名前面。
uint8_t pick_scale(uint8_t size_px, uint8_t base_px)
{
    // 判断条件：base_px == 0 || size_px <= base_px || size_px < kBitmapScaleMinPx 是否成立。
    if (base_px == 0 || size_px <= base_px || size_px < kBitmapScaleMinPx)
        // 返回 1 给调用者，表示当前函数的执行结果。
        return 1;
    // 返回 static_cast<uint8_t>((static_cast<uint16_t>(size_px) + base_px / 2U) / base_px) 给调用者，表示当前函数的执行结果。
    return static_cast<uint8_t>((static_cast<uint16_t>(size_px) + base_px / 2U) / base_px);
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 Font::get_instance：参数在括号里，返回类型写在函数名前面。
Font& Font::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static Font s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义或声明函数 Font::init：参数在括号里，返回类型写在函数名前面。
bool Font::init()
{
    // 函数调用：std::lock_guard<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::lock_guard<std::mutex> lk(mtx_);
    // 判断条件：ready_ 是否成立。
    if (ready_)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;

    // 执行语句：char path_text[64];；它会更新一次变量、对象或程序状态。
    char path_text[64];
    // 执行语句：char path_icon[64];；它会更新一次变量、对象或程序状态。
    char path_icon[64];
    // 函数调用：(void)std::snprintf(path_text, sizeof(path_text), "%s%s", ::app::common::storage::k_path_assets,；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(path_text, sizeof(path_text), "%s%s", ::app::common::storage::k_path_assets,
                        // 继续上一行的表达式或参数列表：kFontFileText);。
                        kFontFileText);
    // 函数调用：(void)std::snprintf(path_icon, sizeof(path_icon), "%s%s", ::app::common::storage::k_path_assets,；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(path_icon, sizeof(path_icon), "%s%s", ::app::common::storage::k_path_assets,
                        // 继续上一行的表达式或参数列表：kFontFileIcon);。
                        kFontFileIcon);

    // 判断条件：FT_Init_FreeType(&lib_) != 0 是否成立。
    if (FT_Init_FreeType(&lib_) != 0)
    {
        // 调用 EBOOK_LOGE(TAG, "FT_Init_FreeType failed");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGE(TAG, "FT_Init_FreeType failed");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool bitmap_ok = BitmapFontSet::get_instance().init();

    // 判断条件：FT_New_Face(lib_, path_text, 0, &face_text_) != 0 是否成立。
    if (FT_New_Face(lib_, path_text, 0, &face_text_) != 0)
    {
        // 判断条件：bitmap_ok 是否成立。
        if (bitmap_ok)
            // 调用 EBOOK_LOGW(TAG, "text TTF missing: %s (bitmap font only)", path_text);，执行这个函数并处理它的返回值或状态变化。
            EBOOK_LOGW(TAG, "text TTF missing: %s (bitmap font only)", path_text);
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 调用 EBOOK_LOGE(TAG, "load text font failed: %s", path_text);，执行这个函数并处理它的返回值或状态变化。
            EBOOK_LOGE(TAG, "load text font failed: %s", path_text);
            // 调用 deinit()：创建和销毁队列。
            BitmapFontSet::get_instance().deinit();
            // 调用 FT_Done_FreeType(lib_);，执行这个函数并处理它的返回值或状态变化。
            FT_Done_FreeType(lib_);
            // 赋值：把右侧 nullptr 的结果保存到左侧 lib_。
            lib_ = nullptr;
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }
    }

    // 判断条件：FT_New_Face(lib_, path_icon, 0, &face_icon_) != 0 是否成立。
    if (FT_New_Face(lib_, path_icon, 0, &face_icon_) != 0)
    {
        // 调用 EBOOK_LOGW(TAG, "load icon font failed: %s (icons disabled)", path_icon);，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "load icon font failed: %s (icons disabled)", path_icon);
        // 赋值：把右侧 nullptr 的结果保存到左侧 face_icon_。
        face_icon_ = nullptr;
    }

    // 赋值：把右侧 0 的结果保存到左侧 size_text_。
    size_text_ = 0;
    // 赋值：把右侧 0 的结果保存到左侧 size_icon_。
    size_icon_ = 0;
    // 赋值：把右侧 (face_text_ == nullptr && bitmap_ok) 的结果保存到左侧 bitmap_only_text_。
    bitmap_only_text_ = (face_text_ == nullptr && bitmap_ok);
    // 判断条件：cache_ == nullptr 是否成立。
    if (cache_ == nullptr)
    {
        // 赋值：把右侧 alloc_array_psram<Slot>(kCacheSlots) 的结果保存到左侧 cache_。
        cache_ = alloc_array_psram<Slot>(kCacheSlots);
        // 判断条件：cache_ == nullptr 是否成立。
        if (cache_ == nullptr)
        {
            // 调用 EBOOK_LOGE(TAG, "glyph cache alloc failed");，执行这个函数并处理它的返回值或状态变化。
            EBOOK_LOGE(TAG, "glyph cache alloc failed");
            // 判断条件：face_icon_ != nullptr 是否成立。
            if (face_icon_ != nullptr)
            {
                // 调用 FT_Done_Face(face_icon_);，执行这个函数并处理它的返回值或状态变化。
                FT_Done_Face(face_icon_);
                // 赋值：把右侧 nullptr 的结果保存到左侧 face_icon_。
                face_icon_ = nullptr;
            }
            // 判断条件：face_text_ != nullptr 是否成立。
            if (face_text_ != nullptr)
            {
                // 调用 FT_Done_Face(face_text_);，执行这个函数并处理它的返回值或状态变化。
                FT_Done_Face(face_text_);
                // 赋值：把右侧 nullptr 的结果保存到左侧 face_text_。
                face_text_ = nullptr;
            }
            // 调用 deinit()：创建和销毁队列。
            BitmapFontSet::get_instance().deinit();
            // 调用 FT_Done_FreeType(lib_);，执行这个函数并处理它的返回值或状态变化。
            FT_Done_FreeType(lib_);
            // 赋值：把右侧 nullptr 的结果保存到左侧 lib_。
            lib_ = nullptr;
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }
    }
    // 赋值：把右侧 true 的结果保存到左侧 ready_。
    ready_ = true;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 Font::deinit：参数在括号里，返回类型写在函数名前面。
void Font::deinit()
{
    // 函数调用：std::lock_guard<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::lock_guard<std::mutex> lk(mtx_);
    // 判断条件：!ready_ 是否成立。
    if (!ready_)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 判断条件：cache_ != nullptr 是否成立。
    if (cache_ != nullptr)
    {
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (size_t i = 0; i < kCacheSlots; ++i)
        {
            // 赋值：把右侧 cache_[i] 的结果保存到左侧 Slot& s。
            Slot& s = cache_[i];
            // 判断条件：s.bitmap != nullptr 是否成立。
            if (s.bitmap != nullptr)
            {
                // 调用 psram_free(s.bitmap);，执行这个函数并处理它的返回值或状态变化。
                psram_free(s.bitmap);
                // 赋值：把右侧 nullptr 的结果保存到左侧 s.bitmap。
                s.bitmap = nullptr;
            }
            // 赋值：把右侧 0 的结果保存到左侧 s.key。
            s.key = 0;
            // 赋值：把右侧 0 的结果保存到左侧 s.bitmap_bytes。
            s.bitmap_bytes = 0;
        }
        // 调用 heap_caps_free(cache_);，执行这个函数并处理它的返回值或状态变化。
        heap_caps_free(cache_);
        // 赋值：把右侧 nullptr 的结果保存到左侧 cache_。
        cache_ = nullptr;
    }
    // 判断条件：face_text_ != nullptr 是否成立。
    if (face_text_ != nullptr)
        // 调用 FT_Done_Face(face_text_);，执行这个函数并处理它的返回值或状态变化。
        FT_Done_Face(face_text_);
    // 判断条件：face_icon_ != nullptr 是否成立。
    if (face_icon_ != nullptr)
        // 调用 FT_Done_Face(face_icon_);，执行这个函数并处理它的返回值或状态变化。
        FT_Done_Face(face_icon_);
    // 判断条件：lib_ != nullptr 是否成立。
    if (lib_ != nullptr)
        // 调用 FT_Done_FreeType(lib_);，执行这个函数并处理它的返回值或状态变化。
        FT_Done_FreeType(lib_);
    // 调用 deinit()：创建和销毁队列。
    BitmapFontSet::get_instance().deinit();

    // 赋值：把右侧 nullptr 的结果保存到左侧 face_text_。
    face_text_ = nullptr;
    // 赋值：把右侧 nullptr 的结果保存到左侧 face_icon_。
    face_icon_ = nullptr;
    // 赋值：把右侧 nullptr 的结果保存到左侧 lib_。
    lib_ = nullptr;
    // 赋值：把右侧 false 的结果保存到左侧 ready_。
    ready_ = false;
    // 赋值：把右侧 false 的结果保存到左侧 bitmap_only_text_。
    bitmap_only_text_ = false;
    // 赋值：把右侧 0 的结果保存到左侧 access_seq_。
    access_seq_ = 0;
    // 赋值：把右侧 {} 的结果保存到左侧 stats_。
    stats_ = {};
}

// 定义或声明函数 Font::make_key：参数在括号里，返回类型写在函数名前面。
uint32_t Font::make_key(uint32_t cp, uint8_t size_px, FontFace face)
{
    // key: face(4) | size(7) | cp(20)
    return 0x80000000U | (static_cast<uint32_t>(face_id(face)) << 27) |
           // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
           (static_cast<uint32_t>(size_px) << 20) | (cp & 0x000FFFFFU);
}

// 定义或声明函数 Font::hash_slot：参数在括号里，返回类型写在函数名前面。
size_t Font::hash_slot(uint32_t key)
{
    // 定义变量 uint32_t h 并初始化为 key。
    uint32_t h = key;
    // 赋值：把右侧 h >> 16 的结果保存到左侧 h ^。
    h ^= h >> 16;
    // 赋值：把右侧 0x7feb352dU 的结果保存到左侧 h *。
    h *= 0x7feb352dU;
    // 赋值：把右侧 h >> 15 的结果保存到左侧 h ^。
    h ^= h >> 15;
    // 赋值：把右侧 0x846ca68bU 的结果保存到左侧 h *。
    h *= 0x846ca68bU;
    // 赋值：把右侧 h >> 16 的结果保存到左侧 h ^。
    h ^= h >> 16;
    // 返回 h & (kCacheSlots - 1) 给调用者，表示当前函数的执行结果。
    return h & (kCacheSlots - 1);
}

// 函数定义/声明：调用 Font::cache_find_locked()：从一个 hash 位置开始，最多探测 16 个连续槽： key 完全相等就命中。 遇到空槽说明这个 key 不存在，停止探测。。
Font::Slot* Font::cache_find_locked(uint32_t key)
{
    // 判断条件：cache_ == nullptr 是否成立。
    if (cache_ == nullptr)
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t start = hash_slot(key);
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (size_t i = 0; i < kProbeMax; ++i)
    {
        // 赋值：把右侧 cache_[(start + i) & (kCacheSlots - 1)] 的结果保存到左侧 Slot& s。
        Slot& s = cache_[(start + i) & (kCacheSlots - 1)];
        // 判断条件：s.key == key 是否成立。
        if (s.key == key)
            // 返回 &s 给调用者，表示当前函数的执行结果。
            return &s;
        // 判断条件：s.key == 0 是否成立。
        if (s.key == 0)
            // 返回 nullptr 给调用者，表示当前函数的执行结果。
            return nullptr;
    }
    // 返回 nullptr 给调用者，表示当前函数的执行结果。
    return nullptr;
}

// 函数定义/声明：调用 Font::cache_replace_locked()：先找空槽，找到就直接用。若 16 个探测槽全满，就在这 16 个里找 lru 最小的槽淘汰。。
Font::Slot* Font::cache_replace_locked(uint32_t key)
{
    // 判断条件：cache_ == nullptr 是否成立。
    if (cache_ == nullptr)
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t start = hash_slot(key);

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (size_t i = 0; i < kProbeMax; ++i)
    {
        // 赋值：把右侧 cache_[(start + i) & (kCacheSlots - 1)] 的结果保存到左侧 Slot& s。
        Slot& s = cache_[(start + i) & (kCacheSlots - 1)];
        // 判断条件：s.key == 0 是否成立。
        if (s.key == 0)
            // 返回 &s 给调用者，表示当前函数的执行结果。
            return &s;
    }
    // 赋值：把右侧 nullptr 的结果保存到左侧 Slot* victim。
    Slot* victim = nullptr;
    // 定义变量 uint32_t oldest 并初始化为 UINT32_MAX。
    uint32_t oldest = UINT32_MAX;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (size_t i = 0; i < kProbeMax; ++i)
    {
        // 赋值：把右侧 cache_[(start + i) & (kCacheSlots - 1)] 的结果保存到左侧 Slot& s。
        Slot& s = cache_[(start + i) & (kCacheSlots - 1)];
        // 判断条件：s.lru < oldest 是否成立。
        if (s.lru < oldest)
        {
            // 赋值：把右侧 s.lru 的结果保存到左侧 oldest。
            oldest = s.lru;
            // 赋值：把右侧 &s 的结果保存到左侧 victim。
            victim = &s;
        }
    }
    // 判断条件：victim == nullptr 是否成立。
    if (victim == nullptr)
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;

    // 判断条件：victim->bitmap != nullptr 是否成立。
    if (victim->bitmap != nullptr)
    {
        // 赋值：把右侧 victim->bitmap_bytes 的结果保存到左侧 stats_.bitmap_bytes -。
        stats_.bitmap_bytes -= victim->bitmap_bytes;
        // 调用 psram_free(victim->bitmap);，执行这个函数并处理它的返回值或状态变化。
        psram_free(victim->bitmap);
        // 赋值：把右侧 nullptr 的结果保存到左侧 victim->bitmap。
        victim->bitmap = nullptr;
        // 赋值：把右侧 0 的结果保存到左侧 victim->bitmap_bytes。
        victim->bitmap_bytes = 0;
        // 访问对象成员：++stats_.evictions;；读取或修改这个对象里的成员状态。
        ++stats_.evictions;
    }
    // 返回 victim 给调用者，表示当前函数的执行结果。
    return victim;
}

// 函数定义/声明：调用 Font::active_face_locked()：activefacelocked()： 根据 FontFace 返回文本字体或图标字体。。
FT_Face Font::active_face_locked(FontFace face)
{
    // 返回 (face == FontFace::Icon) ? face_icon_ : face_text_ 给调用者，表示当前函数的执行结果。
    return (face == FontFace::Icon) ? face_icon_ : face_text_;
}

// 函数定义/声明：调用 Font::select_size_locked()：activefacelocked()： 根据 FontFace 返回文本字体或图标字体。。
bool Font::select_size_locked(FontFace face, uint8_t size_px)
{
    // 判断条件：size_px == 0 是否成立。
    if (size_px == 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 定义变量并调用函数初始化：调用 active_face_locked(face)：activefacelocked()： 根据 FontFace 返回文本字体或图标字体。，结果保存到 FT_Face f。
    FT_Face f = active_face_locked(face);
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 (face == FontFace::Icon) ? &size_icon_ : &size_text_ 的结果保存到左侧 uint8_t* cached。
    uint8_t* cached = (face == FontFace::Icon) ? &size_icon_ : &size_text_;
    // 判断条件：*cached == size_px 是否成立。
    if (*cached == size_px)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    // 判断条件：FT_Set_Pixel_Sizes(f, 0, size_px) != 0 是否成立。
    if (FT_Set_Pixel_Sizes(f, 0, size_px) != 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    *cached = size_px;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数定义/声明：调用 Font::render_into_locked()：FreeType 渲染一个字。
Font::Slot* Font::render_into_locked(uint32_t cp, uint8_t size_px, FontFace face)
{
    // 判断条件：!select_size_locked(face, size_px) 是否成立。
    if (!select_size_locked(face, size_px))
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;
    // 定义变量并调用函数初始化：调用 active_face_locked(face)：activefacelocked()： 根据 FontFace 返回文本字体或图标字体。，结果保存到 FT_Face f。
    FT_Face f = active_face_locked(face);
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;

    // 判断条件：FT_Load_Char(f, cp, FT_LOAD_RENDER | FT_LOAD_TARGET_MONO) != 0 是否成立。
    if (FT_Load_Char(f, cp, FT_LOAD_RENDER | FT_LOAD_TARGET_MONO) != 0)
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const FT_Bitmap& bm = f->glyph->bitmap;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint32_t key = make_key(cp, size_px, face);
    // 调用 cache_replace_locked(key)：先找空槽，找到就直接用。若 16 个探测槽全满，就在这 16 个里找 lru 最小的槽淘汰。。
    Slot* slot = cache_replace_locked(key);
    // 判断条件：slot == nullptr 是否成立。
    if (slot == nullptr)
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;

    // 赋值：把右侧 key 的结果保存到左侧 slot->key。
    slot->key = key;
    // 赋值：把右侧 ++access_seq_ 的结果保存到左侧 slot->lru。
    slot->lru = ++access_seq_;
    // 赋值：把右侧 static_cast<int16_t>(f->glyph->bitmap_left) 的结果保存到左侧 slot->m.bearing_x。
    slot->m.bearing_x = static_cast<int16_t>(f->glyph->bitmap_left);
    // 赋值：把右侧 static_cast<int16_t>(f->glyph->bitmap_top) 的结果保存到左侧 slot->m.bearing_y。
    slot->m.bearing_y = static_cast<int16_t>(f->glyph->bitmap_top);
    // 赋值：把右侧 static_cast<uint16_t>(bm.width) 的结果保存到左侧 slot->m.width。
    slot->m.width = static_cast<uint16_t>(bm.width);
    // 赋值：把右侧 static_cast<uint16_t>(bm.rows) 的结果保存到左侧 slot->m.height。
    slot->m.height = static_cast<uint16_t>(bm.rows);
    // 赋值：把右侧 static_cast<uint16_t>(f->glyph->advance.x >> 6) 的结果保存到左侧 slot->m.advance。
    slot->m.advance = static_cast<uint16_t>(f->glyph->advance.x >> 6);

    // 判断条件：bm.buffer == nullptr || bm.width == 0 || bm.rows == 0 是否成立。
    if (bm.buffer == nullptr || bm.width == 0 || bm.rows == 0)
    {
        // 赋值：把右侧 nullptr 的结果保存到左侧 slot->bitmap。
        slot->bitmap = nullptr;
        // 赋值：把右侧 0 的结果保存到左侧 slot->bitmap_bytes。
        slot->bitmap_bytes = 0;
        // 返回 slot 给调用者，表示当前函数的执行结果。
        return slot;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t pitch = static_cast<uint16_t>((bm.width + 7) / 8);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint32_t nbytes = static_cast<uint32_t>(pitch) * bm.rows;
    // 赋值：把右侧 psram_alloc(nbytes) 的结果保存到左侧 uint8_t* buf。
    uint8_t* buf = psram_alloc(nbytes);
    // 判断条件：buf == nullptr 是否成立。
    if (buf == nullptr)
    {
        // 赋值：把右侧 nullptr 的结果保存到左侧 slot->bitmap。
        slot->bitmap = nullptr;
        // 赋值：把右侧 0 的结果保存到左侧 slot->bitmap_bytes。
        slot->bitmap_bytes = 0;
        // 返回 slot 给调用者，表示当前函数的执行结果。
        return slot;
    }

    // 判断条件：bm.pixel_mode == FT_PIXEL_MODE_MONO 是否成立。
    if (bm.pixel_mode == FT_PIXEL_MODE_MONO)
    {
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (unsigned r = 0; r < bm.rows; ++r)
            // 调用 std::memcpy(buf + r * pitch, bm.buffer + r * bm.pitch, pitch);，执行这个函数并处理它的返回值或状态变化。
            std::memcpy(buf + r * pitch, bm.buffer + r * bm.pitch, pitch);
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 灰度兜底（MONO 目标下不应触发）
        std::memset(buf, 0, nbytes);
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (unsigned r = 0; r < bm.rows; ++r)
            // 循环执行：按括号里的初始化、条件和步进规则重复运行。
            for (unsigned c = 0; c < bm.width; ++c)
                // 判断条件：bm.buffer[r * static_cast<unsigned>(bm.pitch) + c] >= 128 是否成立。
                if (bm.buffer[r * static_cast<unsigned>(bm.pitch) + c] >= 128)
                    // 赋值：把右侧 static_cast<uint8_t>(0x80U >> (c & 7U)) 的结果保存到左侧 buf[r * pitch + (c / 8)] |。
                    buf[r * pitch + (c / 8)] |= static_cast<uint8_t>(0x80U >> (c & 7U));
    }

    // 赋值：把右侧 buf 的结果保存到左侧 slot->bitmap。
    slot->bitmap = buf;
    // 赋值：把右侧 static_cast<uint16_t>(nbytes) 的结果保存到左侧 slot->bitmap_bytes。
    slot->bitmap_bytes = static_cast<uint16_t>(nbytes);
    // 赋值：把右侧 slot->bitmap_bytes 的结果保存到左侧 stats_.bitmap_bytes +。
    stats_.bitmap_bytes += slot->bitmap_bytes;
    // 返回 slot 给调用者，表示当前函数的执行结果。
    return slot;
}

// 函数调用：void Font::blit_locked(uint8_t* fb, int16_t pen_x, int16_t baseline_y, const GlyphMetrics& m,；执行该函数并处理它的返回值或副作用。
void Font::blit_locked(uint8_t* fb, int16_t pen_x, int16_t baseline_y, const GlyphMetrics& m,
                       // 只读约束：这个值或对象在当前位置不能被修改。
                       const uint8_t* bitmap, Ink ink, const core::Rect& clip)
{
    // 判断条件：fb == nullptr || bitmap == nullptr || m.width == 0 || m.height == 0 是否成立。
    if (fb == nullptr || bitmap == nullptr || m.width == 0 || m.height == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t gx0 = static_cast<int16_t>(pen_x + m.bearing_x);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t gy0 = static_cast<int16_t>(baseline_y - m.bearing_y);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t gx1 = static_cast<int16_t>(gx0 + m.width);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t gy1 = static_cast<int16_t>(gy0 + m.height);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x0 = (gx0 > clip.x) ? gx0 : clip.x;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y0 = (gy0 > clip.y) ? gy0 : clip.y;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x1 = (gx1 < clip.right()) ? gx1 : clip.right();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y1 = (gy1 < clip.bottom()) ? gy1 : clip.bottom();
    // 判断条件：x1 <= x0 || y1 <= y0 是否成立。
    if (x1 <= x0 || y1 <= y0)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t src_pitch = static_cast<uint16_t>((m.width + 7) / 8);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool black = (ink == Ink::Black);

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (int16_t py = y0; py < y1; ++py)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int gly_row = py - gy0;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t* sp = bitmap + gly_row * src_pitch;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint32_t row_off = static_cast<uint32_t>(py) * core::kStride;

        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (int16_t px = x0; px < x1; ++px)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const int gly_col = px - gx0;
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint8_t bit = sp[gly_col >> 3] & static_cast<uint8_t>(0x80U >> (gly_col & 7));
            // 判断条件：bit == 0 是否成立。
            if (bit == 0)
                // 继续下一轮：跳过本次循环剩余代码。
                continue;

            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint32_t fb_idx = row_off + (static_cast<uint16_t>(px) >> 3);
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint8_t fb_mask = static_cast<uint8_t>(0x80U >> (static_cast<uint16_t>(px) & 7));
            // 判断条件：black 是否成立。
            if (black)
                // 赋值：把右侧 static_cast<uint8_t>(~fb_mask) 的结果保存到左侧 fb[fb_idx] &。
                fb[fb_idx] &= static_cast<uint8_t>(~fb_mask);
            // 备用分支：前面的条件不成立时执行这里。
            else
                // 赋值：把右侧 fb_mask 的结果保存到左侧 fb[fb_idx] |。
                fb[fb_idx] |= fb_mask;
        }
    }
}

// 函数调用：void Font::blit_bitmap_glyph_locked(uint8_t* fb, int16_t pen_x, int16_t baseline_y,；执行该函数并处理它的返回值或副作用。
void Font::blit_bitmap_glyph_locked(uint8_t* fb, int16_t pen_x, int16_t baseline_y,
                                    // 只读约束：这个值或对象在当前位置不能被修改。
                                    const GlyphMetrics& m, const uint8_t* bitmap, uint8_t size_px,
                                    // 继续上一行的表达式或参数列表：Ink ink, const core::Rect& clip)。
                                    Ink ink, const core::Rect& clip)
{
    // bitmap_only_text_ 且 ≥24px 时点阵整数倍放大
    const BitmapFontFace* near_bf = BitmapFontSet::get_instance().nearest(size_px);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t base_px = (near_bf != nullptr) ? near_bf->size_px() : size_px;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t scale = pick_scale(size_px, base_px);
    // 判断条件：scale <= 1 || bitmap == nullptr || m.width == 0 || m.height == 0 是否成立。
    if (scale <= 1 || bitmap == nullptr || m.width == 0 || m.height == 0)
    {
        // 调用 blit_locked(fb,pen_x,baseline_y,m,bitmap,ink,clip)：把一个字形位图贴到 framebuffer。
        blit_locked(fb, pen_x, baseline_y, m, bitmap, ink, clip);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t bx = static_cast<int16_t>(m.bearing_x * scale);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t by = static_cast<int16_t>(m.bearing_y * scale);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t gw = static_cast<int16_t>(m.width * scale);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t gh = static_cast<int16_t>(m.height * scale);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t gx0 = static_cast<int16_t>(pen_x + bx);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t gy0 = static_cast<int16_t>(baseline_y - by);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t gx1 = static_cast<int16_t>(gx0 + gw);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t gy1 = static_cast<int16_t>(gy0 + gh);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x0 = (gx0 > clip.x) ? gx0 : clip.x;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y0 = (gy0 > clip.y) ? gy0 : clip.y;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x1 = (gx1 < clip.right()) ? gx1 : clip.right();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y1 = (gy1 < clip.bottom()) ? gy1 : clip.bottom();
    // 判断条件：x1 <= x0 || y1 <= y0 是否成立。
    if (x1 <= x0 || y1 <= y0)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t src_pitch = static_cast<uint16_t>((m.width + 7) / 8);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool black = (ink == Ink::Black);

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint16_t row = 0; row < m.height; ++row)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t* sp = bitmap + row * src_pitch;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t dy0 = static_cast<int16_t>(gy0 + row * scale);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t dy1 = static_cast<int16_t>(dy0 + scale);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t py0 = (dy0 > y0) ? dy0 : y0;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t py1 = (dy1 < y1) ? dy1 : y1;
        // 判断条件：py1 <= py0 是否成立。
        if (py1 <= py0)
            // 继续下一轮：跳过本次循环剩余代码。
            continue;

        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint16_t col = 0; col < m.width; ++col)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint8_t bit = sp[col >> 3] & static_cast<uint8_t>(0x80U >> (col & 7));
            // 判断条件：bit == 0 是否成立。
            if (bit == 0)
                // 继续下一轮：跳过本次循环剩余代码。
                continue;

            // 只读约束：这个值或对象在当前位置不能被修改。
            const int16_t dx0 = static_cast<int16_t>(gx0 + col * scale);
            // 只读约束：这个值或对象在当前位置不能被修改。
            const int16_t dx1 = static_cast<int16_t>(dx0 + scale);
            // 只读约束：这个值或对象在当前位置不能被修改。
            const int16_t px0 = (dx0 > x0) ? dx0 : x0;
            // 只读约束：这个值或对象在当前位置不能被修改。
            const int16_t px1 = (dx1 < x1) ? dx1 : x1;
            // 判断条件：px1 <= px0 是否成立。
            if (px1 <= px0)
                // 继续下一轮：跳过本次循环剩余代码。
                continue;

            // 循环执行：按括号里的初始化、条件和步进规则重复运行。
            for (int16_t py = py0; py < py1; ++py)
            {
                // 只读约束：这个值或对象在当前位置不能被修改。
                const uint32_t row_off = static_cast<uint32_t>(py) * core::kStride;
                // 循环执行：按括号里的初始化、条件和步进规则重复运行。
                for (int16_t px = px0; px < px1; ++px)
                {
                    // 只读约束：这个值或对象在当前位置不能被修改。
                    const uint32_t fb_idx = row_off + (static_cast<uint16_t>(px) >> 3);
                    // 只读约束：这个值或对象在当前位置不能被修改。
                    const uint8_t fb_mask =
                        // 继续上一行的表达式或参数列表：static_cast<uint8_t>(0x80U >> (static_cast<uint16_t>(px) & 7));。
                        static_cast<uint8_t>(0x80U >> (static_cast<uint16_t>(px) & 7));
                    // 判断条件：black 是否成立。
                    if (black)
                        // 赋值：把右侧 static_cast<uint8_t>(~fb_mask) 的结果保存到左侧 fb[fb_idx] &。
                        fb[fb_idx] &= static_cast<uint8_t>(~fb_mask);
                    // 备用分支：前面的条件不成立时执行这里。
                    else
                        // 赋值：把右侧 fb_mask 的结果保存到左侧 fb[fb_idx] |。
                        fb[fb_idx] |= fb_mask;
                }
            }
        }
    }
}

// 函数调用：uint16_t Font::draw_glyph(uint8_t* fb, int16_t x, int16_t y_top, uint32_t cp, uint8_t size_px,；执行该函数并处理它的返回值或副作用。
uint16_t Font::draw_glyph(uint8_t* fb, int16_t x, int16_t y_top, uint32_t cp, uint8_t size_px,
                          // 继续上一行的表达式或参数列表：FontFace face, Ink ink, const core::Rect& clip)。
                          FontFace face, Ink ink, const core::Rect& clip)
{
    // 函数调用：std::lock_guard<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::lock_guard<std::mutex> lk(mtx_);
    // 判断条件：!ready_ || fb == nullptr 是否成立。
    if (!ready_ || fb == nullptr)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 赋值：把右侧 BitmapFontSet::get_instance() 的结果保存到左侧 auto& bset。
    auto& bset = BitmapFontSet::get_instance();

    // 1) 预烘焙点阵（text 12/14/16）
    if (face == FontFace::Text)
    {
        // 判断条件：const BitmapFontFace* bf = bset.get(size_px) 是否成立。
        if (const BitmapFontFace* bf = bset.get(size_px))
        {
            // 声明并默认初始化对象：GlyphMetrics m{}。
            GlyphMetrics m{};
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint8_t* bm = nullptr;
            // 判断条件：bf->lookup(cp, m, bm) 是否成立。
            if (bf->lookup(cp, m, bm))
            {
                // 定义只读对象或变量：初始化后不能在当前位置修改。
                const int16_t baseline = static_cast<int16_t>(y_top + bf->ascent());
                // 调用 blit_locked(fb,x,baseline,m,bm,ink,clip)：把一个字形位图贴到 framebuffer。
                blit_locked(fb, x, baseline, m, bm, ink, clip);
                // 返回 m.advance 给调用者，表示当前函数的执行结果。
                return m.advance;
            }
        }
    }

    // 2) FT cache fallback
    const uint32_t key = make_key(cp, size_px, face);
    // 调用 cache_find_locked(key)：从一个 hash 位置开始，最多探测 16 个连续槽： key 完全相等就命中。 遇到空槽说明这个 key 不存在，停止探测。。
    Slot* g = cache_find_locked(key);
    // 判断条件：g != nullptr 是否成立。
    if (g != nullptr)
    {
        // 访问对象成员：++stats_.hits;；读取或修改这个对象里的成员状态。
        ++stats_.hits;
        // 赋值：把右侧 ++access_seq_ 的结果保存到左侧 g->lru。
        g->lru = ++access_seq_;
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 访问对象成员：++stats_.misses;；读取或修改这个对象里的成员状态。
        ++stats_.misses;
        // 调用 render_into_locked(cp,size_px,face)：FreeType 渲染一个字。
        g = render_into_locked(cp, size_px, face);
        // 判断条件：g == nullptr 是否成立。
        if (g == nullptr)
            // 返回 0 给调用者，表示当前函数的执行结果。
            return 0;
    }

    // 判断条件：!select_size_locked(face, size_px) 是否成立。
    if (!select_size_locked(face, size_px))
        // 返回 g->m.advance 给调用者，表示当前函数的执行结果。
        return g->m.advance;
    // 定义变量并调用函数初始化：调用 active_face_locked(face)：activefacelocked()： 根据 FontFace 返回文本字体或图标字体。，结果保存到 FT_Face f。
    FT_Face f = active_face_locked(face);
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
        // 返回 g->m.advance 给调用者，表示当前函数的执行结果。
        return g->m.advance;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t baseline = static_cast<int16_t>(y_top + (f->size->metrics.ascender >> 6));
    // 调用 blit_locked(fb,x,baseline,g->m,g->bitmap,ink,clip)：把一个字形位图贴到 framebuffer。
    blit_locked(fb, x, baseline, g->m, g->bitmap, ink, clip);
    // 返回 g->m.advance 给调用者，表示当前函数的执行结果。
    return g->m.advance;
}

// 函数调用：uint16_t Font::draw_text(uint8_t* fb, int16_t x, int16_t y_top, const char* utf8, uint8_t size_px,；执行该函数并处理它的返回值或副作用。
uint16_t Font::draw_text(uint8_t* fb, int16_t x, int16_t y_top, const char* utf8, uint8_t size_px,
                         // 继续上一行的表达式或参数列表：FontFace face, Ink ink, const core::Rect& clip)。
                         FontFace face, Ink ink, const core::Rect& clip)
{
    // 判断条件：fb == nullptr || utf8 == nullptr 是否成立。
    if (fb == nullptr || utf8 == nullptr)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    // 函数调用：std::lock_guard<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::lock_guard<std::mutex> lk(mtx_);
    // 判断条件：!ready_ 是否成立。
    if (!ready_)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 赋值：把右侧 BitmapFontSet::get_instance() 的结果保存到左侧 auto& bset。
    auto& bset = BitmapFontSet::get_instance();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const BitmapFontFace* exact_bf = (face == FontFace::Text) ? bset.get(size_px) : nullptr;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const BitmapFontFace* near_bf =
        // 定义或声明函数 nearest：参数在括号里，返回类型写在函数名前面。
        (face == FontFace::Text && exact_bf == nullptr && bitmap_only_text_) ? bset.nearest(size_px)
                                                                             // 执行语句：: nullptr;；它会更新一次变量、对象或程序状态。
                                                                             : nullptr;

    // 定义变量并调用函数初始化：调用 active_face_locked(face)：activefacelocked()： 根据 FontFace 返回文本字体或图标字体。，结果保存到 FT_Face f。
    FT_Face f = active_face_locked(face);
    // 判断条件：exact_bf == nullptr && f != nullptr 是否成立。
    if (exact_bf == nullptr && f != nullptr)
        // 调用 select_size_locked(face,size_px)：activefacelocked()： 根据 FontFace 返回文本字体或图标字体。。
        (void)select_size_locked(face, size_px);

    // 基线：点阵 / 邻近点阵 / FreeType
    int16_t baseline;
    // 判断条件：exact_bf != nullptr 是否成立。
    if (exact_bf != nullptr)
        // 调用 ascent()：advance(cp)： 查一个字符的前进宽度。点阵优先，FreeType 兜底。它不会绘制任何像素，适合排版测量。。
        baseline = static_cast<int16_t>(y_top + exact_bf->ascent());
    // 备用分支：前面的条件不成立时执行这里。
    else if (near_bf != nullptr)
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        baseline = static_cast<int16_t>(
            // 继续上一行的表达式或参数列表：y_top + scale_metric(near_bf->ascent(), near_bf->size_px(), size_px));。
            y_top + scale_metric(near_bf->ascent(), near_bf->size_px(), size_px));
    // 备用分支：前面的条件不成立时执行这里。
    else if (f != nullptr)
        // 赋值：把右侧 static_cast<int16_t>(y_top + (f->size->metrics.ascender >> 6)) 的结果保存到左侧 baseline。
        baseline = static_cast<int16_t>(y_top + (f->size->metrics.ascender >> 6));
    // 备用分支：前面的条件不成立时执行这里。
    else
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t clip_right = clip.right();
    // 定义变量 int32_t pen_x 并初始化为 x。
    int32_t pen_x = x;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* p = utf8;

    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (*p != '\0')
    {
        // 定义变量 uint32_t cp 并初始化为 0。
        uint32_t cp = 0;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t n = utf8_decode(p, cp);
        // 判断条件：n == 0 是否成立。
        if (n == 0)
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 赋值：把右侧 n 的结果保存到左侧 p +。
        p += n;

        // 声明并默认初始化对象：GlyphMetrics m{}。
        GlyphMetrics m{};
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t* bm = nullptr;

        // 1) 精确点阵
        if (exact_bf != nullptr && exact_bf->lookup(cp, m, bm))
        {
            // 调用 blit_locked(fb, static_cast<int16_t>(pen_x), baseline, m, bm, ink, clip);，执行这个函数并处理它的返回值或状态变化。
            blit_locked(fb, static_cast<int16_t>(pen_x), baseline, m, bm, ink, clip);
            // 赋值：把右侧 m.advance 的结果保存到左侧 pen_x +。
            pen_x += m.advance;
        }
        // 2) 邻近点阵（bitmap_only_text_）
        else if (near_bf != nullptr && near_bf->lookup(cp, m, bm))
        {
            // 判断条件：size_px >= kBitmapScaleMinPx 是否成立。
            if (size_px >= kBitmapScaleMinPx)
                // 调用 blit_bitmap_glyph_locked(fb, static_cast<int16_t>(pen_x), baseline, m, bm, size_px,，执行这个函数并处理它的返回值或状态变化。
                blit_bitmap_glyph_locked(fb, static_cast<int16_t>(pen_x), baseline, m, bm, size_px,
                                         // 继续上一行的表达式或参数列表：ink, clip);。
                                         ink, clip);
            // 备用分支：前面的条件不成立时执行这里。
            else
                // 调用 blit_locked(fb, static_cast<int16_t>(pen_x), baseline, m, bm, ink, clip);，执行这个函数并处理它的返回值或状态变化。
                blit_locked(fb, static_cast<int16_t>(pen_x), baseline, m, bm, ink, clip);

            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint8_t base_px = near_bf->size_px();
            // 赋值：把右侧 (base_px == size_px) ? m.advance : scale_metric(m.advance, base_px, size_px) 的结果保存到左侧 pen_x +。
            pen_x += (base_px == size_px) ? m.advance : scale_metric(m.advance, base_px, size_px);
        }
        // 3) FT cache
        else if (f != nullptr)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint32_t key = make_key(cp, size_px, face);
            // 调用 cache_find_locked(key)：从一个 hash 位置开始，最多探测 16 个连续槽： key 完全相等就命中。 遇到空槽说明这个 key 不存在，停止探测。。
            Slot* g = cache_find_locked(key);
            // 判断条件：g != nullptr 是否成立。
            if (g != nullptr)
            {
                // 访问对象成员：++stats_.hits;；读取或修改这个对象里的成员状态。
                ++stats_.hits;
                // 赋值：把右侧 ++access_seq_ 的结果保存到左侧 g->lru。
                g->lru = ++access_seq_;
            }
            // 备用分支：前面的条件不成立时执行这里。
            else
            {
                // 访问对象成员：++stats_.misses;；读取或修改这个对象里的成员状态。
                ++stats_.misses;
                // 调用 render_into_locked(cp,size_px,face)：FreeType 渲染一个字。
                g = render_into_locked(cp, size_px, face);
                // 判断条件：g == nullptr 是否成立。
                if (g == nullptr)
                    // 继续下一轮：跳过本次循环剩余代码。
                    continue;
            }
            // 调用 blit_locked(fb, static_cast<int16_t>(pen_x), baseline, g->m, g->bitmap, ink, clip);，执行这个函数并处理它的返回值或状态变化。
            blit_locked(fb, static_cast<int16_t>(pen_x), baseline, g->m, g->bitmap, ink, clip);
            // 赋值：把右侧 g->m.advance 的结果保存到左侧 pen_x +。
            pen_x += g->m.advance;
        }

        // 判断条件：pen_x >= clip_right + 64 是否成立。
        if (pen_x >= clip_right + 64)
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }

    // 返回 static_cast<uint16_t>(pen_x - x) 给调用者，表示当前函数的执行结果。
    return static_cast<uint16_t>(pen_x - x);
}

// 函数定义/声明：调用 Font::advance()：advance(cp)： 查一个字符的前进宽度。点阵优先，FreeType 兜底。它不会绘制任何像素，适合排版测量。。
uint16_t Font::advance(uint32_t cp, uint8_t size_px, FontFace face)
{
    // 赋值：把右侧 BitmapFontSet::get_instance() 的结果保存到左侧 auto& bset。
    auto& bset = BitmapFontSet::get_instance();

    // 判断条件：face == FontFace::Text 是否成立。
    if (face == FontFace::Text)
    {
        // 判断条件：const BitmapFontFace* exact = bset.get(size_px) 是否成立。
        if (const BitmapFontFace* exact = bset.get(size_px))
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint16_t adv = exact->advance(cp);
            // 判断条件：adv > 0 是否成立。
            if (adv > 0)
                // 返回 adv 给调用者，表示当前函数的执行结果。
                return adv;
        }
        // 判断条件：bitmap_only_text_ 是否成立。
        if (bitmap_only_text_)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const BitmapFontFace* near_bf = bset.nearest(size_px);
            // 判断条件：near_bf != nullptr 是否成立。
            if (near_bf != nullptr)
            {
                // 只读约束：这个值或对象在当前位置不能被修改。
                const uint16_t adv = near_bf->advance(cp);
                // 判断条件：adv > 0 是否成立。
                if (adv > 0)
                    // 返回 scale_metric(adv, near_bf->size_px(), size_px) 给调用者，表示当前函数的执行结果。
                    return scale_metric(adv, near_bf->size_px(), size_px);
            }
        }
    }

    // 函数调用：std::lock_guard<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::lock_guard<std::mutex> lk(mtx_);
    // 判断条件：!ready_ 是否成立。
    if (!ready_)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint32_t key = make_key(cp, size_px, face);
    // 调用 cache_find_locked(key)：从一个 hash 位置开始，最多探测 16 个连续槽： key 完全相等就命中。 遇到空槽说明这个 key 不存在，停止探测。。
    Slot* g = cache_find_locked(key);
    // 判断条件：g != nullptr 是否成立。
    if (g != nullptr)
    {
        // 访问对象成员：++stats_.hits;；读取或修改这个对象里的成员状态。
        ++stats_.hits;
        // 赋值：把右侧 ++access_seq_ 的结果保存到左侧 g->lru。
        g->lru = ++access_seq_;
        // 返回 g->m.advance 给调用者，表示当前函数的执行结果。
        return g->m.advance;
    }

    // 访问对象成员：++stats_.misses;；读取或修改这个对象里的成员状态。
    ++stats_.misses;
    // 调用 render_into_locked(cp,size_px,face)：FreeType 渲染一个字。
    g = render_into_locked(cp, size_px, face);
    // 返回 (g != nullptr) ? g->m.advance : 0 给调用者，表示当前函数的执行结果。
    return (g != nullptr) ? g->m.advance : 0;
}

// 函数定义/声明：调用 Font::measure()：advance(cp)： 查一个字符的前进宽度。点阵优先，FreeType 兜底。它不会绘制任何像素，适合排版测量。。
uint16_t Font::measure(const char* utf8, uint8_t size_px, FontFace face)
{
    // 判断条件：utf8 == nullptr 是否成立。
    if (utf8 == nullptr)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 赋值：把右侧 BitmapFontSet::get_instance() 的结果保存到左侧 auto& bset。
    auto& bset = BitmapFontSet::get_instance();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const BitmapFontFace* exact_bf = (face == FontFace::Text) ? bset.get(size_px) : nullptr;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const BitmapFontFace* near_bf =
        // 定义或声明函数 nearest：参数在括号里，返回类型写在函数名前面。
        (face == FontFace::Text && exact_bf == nullptr && bitmap_only_text_) ? bset.nearest(size_px)
                                                                             // 执行语句：: nullptr;；它会更新一次变量、对象或程序状态。
                                                                             : nullptr;

    // 函数调用：std::lock_guard<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::lock_guard<std::mutex> lk(mtx_);
    // 判断条件：!ready_ 是否成立。
    if (!ready_)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 定义变量 uint32_t total 并初始化为 0。
    uint32_t total = 0;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* p = utf8;
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (*p != '\0')
    {
        // 定义变量 uint32_t cp 并初始化为 0。
        uint32_t cp = 0;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t n = utf8_decode(p, cp);
        // 判断条件：n == 0 是否成立。
        if (n == 0)
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 赋值：把右侧 n 的结果保存到左侧 p +。
        p += n;

        // 判断条件：exact_bf != nullptr 是否成立。
        if (exact_bf != nullptr)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint16_t adv = exact_bf->advance(cp);
            // 判断条件：adv > 0 是否成立。
            if (adv > 0)
            {
                // 赋值：把右侧 adv 的结果保存到左侧 total +。
                total += adv;
                // 继续下一轮：跳过本次循环剩余代码。
                continue;
            }
        }
        // 备用分支：前面的条件不成立时执行这里。
        else if (near_bf != nullptr)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint16_t adv = near_bf->advance(cp);
            // 判断条件：adv > 0 是否成立。
            if (adv > 0)
            {
                // 赋值：把右侧 scale_metric(adv, near_bf->size_px(), size_px) 的结果保存到左侧 total +。
                total += scale_metric(adv, near_bf->size_px(), size_px);
                // 继续下一轮：跳过本次循环剩余代码。
                continue;
            }
        }

        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint32_t key = make_key(cp, size_px, face);
        // 调用 cache_find_locked(key)：从一个 hash 位置开始，最多探测 16 个连续槽： key 完全相等就命中。 遇到空槽说明这个 key 不存在，停止探测。。
        Slot* g = cache_find_locked(key);
        // 判断条件：g != nullptr 是否成立。
        if (g != nullptr)
        {
            // 访问对象成员：++stats_.hits;；读取或修改这个对象里的成员状态。
            ++stats_.hits;
            // 赋值：把右侧 ++access_seq_ 的结果保存到左侧 g->lru。
            g->lru = ++access_seq_;
            // 赋值：把右侧 g->m.advance 的结果保存到左侧 total +。
            total += g->m.advance;
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 访问对象成员：++stats_.misses;；读取或修改这个对象里的成员状态。
            ++stats_.misses;
            // 调用 render_into_locked(cp,size_px,face)：FreeType 渲染一个字。
            g = render_into_locked(cp, size_px, face);
            // 判断条件：g != nullptr 是否成立。
            if (g != nullptr)
                // 赋值：把右侧 g->m.advance 的结果保存到左侧 total +。
                total += g->m.advance;
        }
    }
    // 返回 (total > UINT16_MAX) ? UINT16_MAX : static_cast<uint16_t>(total) 给调用者，表示当前函数的执行结果。
    return (total > UINT16_MAX) ? UINT16_MAX : static_cast<uint16_t>(total);
}

// 函数定义/声明：调用 Font::line_height()：advance(cp)： 查一个字符的前进宽度。点阵优先，FreeType 兜底。它不会绘制任何像素，适合排版测量。。
uint16_t Font::line_height(uint8_t size_px, FontFace face)
{
    // 赋值：把右侧 BitmapFontSet::get_instance() 的结果保存到左侧 auto& bset。
    auto& bset = BitmapFontSet::get_instance();
    // 判断条件：face == FontFace::Text 是否成立。
    if (face == FontFace::Text)
    {
        // 判断条件：const BitmapFontFace* exact = bset.get(size_px) 是否成立。
        if (const BitmapFontFace* exact = bset.get(size_px))
            // 返回结果：调用 line_height()：advance(cp)： 查一个字符的前进宽度。点阵优先，FreeType 兜底。它不会绘制任何像素，适合排版测量。。
            return exact->line_height();
        // 判断条件：bitmap_only_text_ 是否成立。
        if (bitmap_only_text_)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const BitmapFontFace* near_bf = bset.nearest(size_px);
            // 判断条件：near_bf != nullptr 是否成立。
            if (near_bf != nullptr)
                // 返回结果：调用 line_height()：advance(cp)： 查一个字符的前进宽度。点阵优先，FreeType 兜底。它不会绘制任何像素，适合排版测量。。
                return scale_metric(near_bf->line_height(), near_bf->size_px(), size_px);
        }
    }

    // 函数调用：std::lock_guard<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::lock_guard<std::mutex> lk(mtx_);
    // 判断条件：!ready_ || !select_size_locked(face, size_px) 是否成立。
    if (!ready_ || !select_size_locked(face, size_px))
        // 返回 size_px 给调用者，表示当前函数的执行结果。
        return size_px;
    // 定义变量并调用函数初始化：调用 active_face_locked(face)：activefacelocked()： 根据 FontFace 返回文本字体或图标字体。，结果保存到 FT_Face f。
    FT_Face f = active_face_locked(face);
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
        // 返回 size_px 给调用者，表示当前函数的执行结果。
        return size_px;
    // 返回 static_cast<uint16_t>(f->size->metrics.height >> 6) 给调用者，表示当前函数的执行结果。
    return static_cast<uint16_t>(f->size->metrics.height >> 6);
}

// 函数定义/声明：调用 Font::ascent()：advance(cp)： 查一个字符的前进宽度。点阵优先，FreeType 兜底。它不会绘制任何像素，适合排版测量。。
uint16_t Font::ascent(uint8_t size_px, FontFace face)
{
    // 赋值：把右侧 BitmapFontSet::get_instance() 的结果保存到左侧 auto& bset。
    auto& bset = BitmapFontSet::get_instance();
    // 判断条件：face == FontFace::Text 是否成立。
    if (face == FontFace::Text)
    {
        // 判断条件：const BitmapFontFace* exact = bset.get(size_px) 是否成立。
        if (const BitmapFontFace* exact = bset.get(size_px))
            // 返回结果：调用 ascent()：advance(cp)： 查一个字符的前进宽度。点阵优先，FreeType 兜底。它不会绘制任何像素，适合排版测量。。
            return exact->ascent();
        // 判断条件：bitmap_only_text_ 是否成立。
        if (bitmap_only_text_)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const BitmapFontFace* near_bf = bset.nearest(size_px);
            // 判断条件：near_bf != nullptr 是否成立。
            if (near_bf != nullptr)
                // 返回结果：调用 ascent()：advance(cp)： 查一个字符的前进宽度。点阵优先，FreeType 兜底。它不会绘制任何像素，适合排版测量。。
                return scale_metric(near_bf->ascent(), near_bf->size_px(), size_px);
        }
    }

    // 函数调用：std::lock_guard<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::lock_guard<std::mutex> lk(mtx_);
    // 判断条件：!ready_ || !select_size_locked(face, size_px) 是否成立。
    if (!ready_ || !select_size_locked(face, size_px))
        // 返回 size_px 给调用者，表示当前函数的执行结果。
        return size_px;
    // 定义变量并调用函数初始化：调用 active_face_locked(face)：activefacelocked()： 根据 FontFace 返回文本字体或图标字体。，结果保存到 FT_Face f。
    FT_Face f = active_face_locked(face);
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
        // 返回 size_px 给调用者，表示当前函数的执行结果。
        return size_px;
    // 返回 static_cast<uint16_t>(f->size->metrics.ascender >> 6) 给调用者，表示当前函数的执行结果。
    return static_cast<uint16_t>(f->size->metrics.ascender >> 6);
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::gfx
