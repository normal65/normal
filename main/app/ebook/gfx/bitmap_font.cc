/**
 * [初学者导读] 文件：main/app/ebook/gfx/bitmap_font.cc
 * 作用：加载和绘制预烘焙点阵字体。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：绘制只改 framebuffer，不直接操作 SPI。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/bitmap_font.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_heap_caps.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "common/storage/storage.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "BitmapFont";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::gfx {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 静态查表数组 kMagic：运行时复用固定数据，避免每次重新计算。
constexpr char kMagic[4] = {'E', 'B', 'F', '1'};

// 定义或声明函数 pack：参数在括号里，返回类型写在函数名前面。
#pragma pack(push, 1)
// struct FileHeader：把相关数据字段组合在一起，表示项目中的一个概念。
struct FileHeader
{
    // 执行语句：char magic[4];；它会更新一次变量、对象或程序状态。
    char magic[4];
    // 执行语句：uint16_t version;；它会更新一次变量、对象或程序状态。
    uint16_t version;
    // 执行语句：uint8_t size_px;；它会更新一次变量、对象或程序状态。
    uint8_t size_px;
    // 执行语句：uint8_t ascent;；它会更新一次变量、对象或程序状态。
    uint8_t ascent;
    // 执行语句：uint8_t line_height;；它会更新一次变量、对象或程序状态。
    uint8_t line_height;
    // 执行语句：uint8_t reserved;；它会更新一次变量、对象或程序状态。
    uint8_t reserved;
    // 执行语句：uint16_t glyph_count;；它会更新一次变量、对象或程序状态。
    uint16_t glyph_count;
    // 执行语句：uint32_t lut_ofs;；它会更新一次变量、对象或程序状态。
    uint32_t lut_ofs;
    // 执行语句：uint32_t dir_ofs;；它会更新一次变量、对象或程序状态。
    uint32_t dir_ofs;
    // 执行语句：uint32_t bitmap_ofs;；它会更新一次变量、对象或程序状态。
    uint32_t bitmap_ofs;
    // 执行语句：uint32_t file_size;；它会更新一次变量、对象或程序状态。
    uint32_t file_size;
};
// 定义或声明函数 pack：参数在括号里，返回类型写在函数名前面。
#pragma pack(pop)

// 调用 static_assert(sizeof(FileHeader) == 28, "bitmap font header size");，执行这个函数并处理它的返回值或状态变化。
static_assert(sizeof(FileHeader) == 28, "bitmap font header size");

// 定义或声明函数 alloc_psram_or_internal：参数在括号里，返回类型写在函数名前面。
uint8_t* alloc_psram_or_internal(size_t n)
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

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 BitmapFontFace::init：参数在括号里，返回类型写在函数名前面。
bool BitmapFontFace::init(const char* path)
{
    // 判断条件：data_ != nullptr 是否成立。
    if (data_ != nullptr)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    // 判断条件：path == nullptr || path[0] == '\0' 是否成立。
    if (path == nullptr || path[0] == '\0')
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 std::fopen(path, "rb") 的结果保存到左侧 FILE* f。
    FILE* f = std::fopen(path, "rb");
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
    {
        // 调用 EBOOK_LOGW(TAG, "open failed: %s", path);，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "open failed: %s", path);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 判断条件：std::fseek(f, 0, SEEK_END) != 0 是否成立。
    if (std::fseek(f, 0, SEEK_END) != 0)
    {
        // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(f);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 只读约束：这个值或对象在当前位置不能被修改。
    const long sz = std::ftell(f);
    // 判断条件：sz <= static_cast<long>(sizeof(FileHeader)) 是否成立。
    if (sz <= static_cast<long>(sizeof(FileHeader)))
    {
        // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(f);
        // 调用 EBOOK_LOGW(TAG, "file too small: %s", path);，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "file too small: %s", path);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 alloc_psram_or_internal(static_cast<size_t>(sz)) 的结果保存到左侧 uint8_t* buf。
    uint8_t* buf = alloc_psram_or_internal(static_cast<size_t>(sz));
    // 判断条件：buf == nullptr 是否成立。
    if (buf == nullptr)
    {
        // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(f);
        // 调用 EBOOK_LOGE(TAG, "alloc %u bytes failed", static_cast<unsigned>(sz));，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGE(TAG, "alloc %u bytes failed", static_cast<unsigned>(sz));
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 调用 std::rewind(f);，执行这个函数并处理它的返回值或状态变化。
    std::rewind(f);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t got = std::fread(buf, 1, static_cast<size_t>(sz), f);
    // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
    std::fclose(f);
    // 判断条件：got != static_cast<size_t>(sz) 是否成立。
    if (got != static_cast<size_t>(sz))
    {
        // 调用 heap_caps_free(buf);，执行这个函数并处理它的返回值或状态变化。
        heap_caps_free(buf);
        // 调用 EBOOK_LOGE(TAG, "read failed: %s", path);，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGE(TAG, "read failed: %s", path);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto* hdr = reinterpret_cast<const FileHeader*>(buf);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool magic_ok = std::memcmp(hdr->magic, kMagic, 4) == 0;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool version_ok = hdr->version == 1;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool size_ok = hdr->file_size == static_cast<uint32_t>(sz);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool size_px_ok = hdr->size_px != 0;
    // 判断条件：!magic_ok || !version_ok || !size_ok || !size_px_ok 是否成立。
    if (!magic_ok || !version_ok || !size_ok || !size_px_ok)
    {
        // 调用 heap_caps_free(buf);，执行这个函数并处理它的返回值或状态变化。
        heap_caps_free(buf);
        // 调用 EBOOK_LOGW(TAG, "header mismatch: %s", path);，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "header mismatch: %s", path);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 buf 的结果保存到左侧 data_。
    data_ = buf;
    // 赋值：把右侧 static_cast<size_t>(sz) 的结果保存到左侧 data_size_。
    data_size_ = static_cast<size_t>(sz);
    // 赋值：把右侧 hdr->size_px 的结果保存到左侧 size_px_。
    size_px_ = hdr->size_px;
    // 赋值：把右侧 hdr->ascent 的结果保存到左侧 ascent_。
    ascent_ = hdr->ascent;
    // 赋值：把右侧 hdr->line_height 的结果保存到左侧 line_height_。
    line_height_ = hdr->line_height;
    // 赋值：把右侧 hdr->glyph_count 的结果保存到左侧 glyph_count_。
    glyph_count_ = hdr->glyph_count;
    // 赋值：把右侧 reinterpret_cast<const uint16_t*>(buf + hdr->lut_ofs) 的结果保存到左侧 lut_。
    lut_ = reinterpret_cast<const uint16_t*>(buf + hdr->lut_ofs);
    // 赋值：把右侧 reinterpret_cast<const DirEntry*>(buf + hdr->dir_ofs) 的结果保存到左侧 dir_。
    dir_ = reinterpret_cast<const DirEntry*>(buf + hdr->dir_ofs);
    // 赋值：把右侧 buf + hdr->bitmap_ofs 的结果保存到左侧 pool_。
    pool_ = buf + hdr->bitmap_ofs;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 BitmapFontFace::deinit：参数在括号里，返回类型写在函数名前面。
void BitmapFontFace::deinit()
{
    // 判断条件：data_ != nullptr 是否成立。
    if (data_ != nullptr)
    {
        // 调用 heap_caps_free(data_);，执行这个函数并处理它的返回值或状态变化。
        heap_caps_free(data_);
        // 赋值：把右侧 nullptr 的结果保存到左侧 data_。
        data_ = nullptr;
    }
    // 赋值：把右侧 0 的结果保存到左侧 data_size_。
    data_size_ = 0;
    // 赋值：把右侧 0 的结果保存到左侧 size_px_。
    size_px_ = 0;
    // 赋值：把右侧 0 的结果保存到左侧 ascent_。
    ascent_ = 0;
    // 赋值：把右侧 0 的结果保存到左侧 line_height_。
    line_height_ = 0;
    // 赋值：把右侧 0 的结果保存到左侧 glyph_count_。
    glyph_count_ = 0;
    // 赋值：把右侧 nullptr 的结果保存到左侧 lut_。
    lut_ = nullptr;
    // 赋值：把右侧 nullptr 的结果保存到左侧 dir_。
    dir_ = nullptr;
    // 赋值：把右侧 nullptr 的结果保存到左侧 pool_。
    pool_ = nullptr;
}

// 定义或声明函数 BitmapFontFace::lookup：参数在括号里，返回类型写在函数名前面。
bool BitmapFontFace::lookup(uint32_t cp, GlyphMetrics& m, const uint8_t*& bitmap) const
{
    // 赋值：把右侧 nullptr 的结果保存到左侧 bitmap。
    bitmap = nullptr;
    // 判断条件：data_ == nullptr || cp >= 65536U 是否成立。
    if (data_ == nullptr || cp >= 65536U)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t gid = lut_[cp];
    // 判断条件：gid == 0 || gid > glyph_count_ 是否成立。
    if (gid == 0 || gid > glyph_count_)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const DirEntry& e = dir_[gid - 1];
    // 赋值：把右侧 e.advance 的结果保存到左侧 m.advance。
    m.advance = e.advance;
    // 赋值：把右侧 e.bearing_x 的结果保存到左侧 m.bearing_x。
    m.bearing_x = e.bearing_x;
    // 赋值：把右侧 e.bearing_y 的结果保存到左侧 m.bearing_y。
    m.bearing_y = e.bearing_y;
    // 赋值：把右侧 e.width 的结果保存到左侧 m.width。
    m.width = e.width;
    // 赋值：把右侧 e.height 的结果保存到左侧 m.height。
    m.height = e.height;

    // 判断条件：e.width != 0 && e.height != 0 && e.pitch != 0 是否成立。
    if (e.width != 0 && e.height != 0 && e.pitch != 0)
        // 赋值：把右侧 pool_ + e.bitmap_ofs 的结果保存到左侧 bitmap。
        bitmap = pool_ + e.bitmap_ofs;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数定义/声明：调用 BitmapFontFace::advance()：advance(cp)： 查一个字符的前进宽度。点阵优先，FreeType 兜底。它不会绘制任何像素，适合排版测量。。
uint16_t BitmapFontFace::advance(uint32_t cp) const
{
    // 判断条件：data_ == nullptr || cp >= 65536U 是否成立。
    if (data_ == nullptr || cp >= 65536U)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t gid = lut_[cp];
    // 判断条件：gid == 0 || gid > glyph_count_ 是否成立。
    if (gid == 0 || gid > glyph_count_)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    // 返回 dir_[gid - 1].advance 给调用者，表示当前函数的执行结果。
    return dir_[gid - 1].advance;
}

// 定义或声明函数 BitmapFontSet::get_instance：参数在括号里，返回类型写在函数名前面。
BitmapFontSet& BitmapFontSet::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static BitmapFontSet s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义或声明函数 BitmapFontSet::init：参数在括号里，返回类型写在函数名前面。
bool BitmapFontSet::init()
{
    // 判断条件：loaded_ > 0 是否成立。
    if (loaded_ > 0)
        // 返回 any_ready() 给调用者，表示当前函数的执行结果。
        return any_ready();

    // 执行语句：char path[80];；它会更新一次变量、对象或程序状态。
    char path[80];
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (size_t i = 0; i < kSizeCount && loaded_ < kSizeCount; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t px = kSizes[i];
        // 函数调用：(void)std::snprintf(path, sizeof(path), "%s/fonts/simhei_%u.bin",；执行该函数并处理它的返回值或副作用。
        (void)std::snprintf(path, sizeof(path), "%s/fonts/simhei_%u.bin",
                            // 继续上一行的表达式或参数列表：::app::common::storage::k_path_assets, static_cast<unsigned>(px));。
                            ::app::common::storage::k_path_assets, static_cast<unsigned>(px));
        // 判断条件：faces_[loaded_].init(path) 是否成立。
        if (faces_[loaded_].init(path))
            // 执行语句：++loaded_;；它会更新一次变量、对象或程序状态。
            ++loaded_;
    }

    // 判断条件：loaded_ == 0 是否成立。
    if (loaded_ == 0)
        // 调用 EBOOK_LOGW(TAG, "no bitmap fonts loaded (FreeType fallback only)");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "no bitmap fonts loaded (FreeType fallback only)");
    // 备用分支：前面的条件不成立时执行这里。
    else
        // 调用 EBOOK_LOGI(TAG, "loaded %u/%u faces from /assets/fonts", static_cast<unsigned>(loaded_),，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGI(TAG, "loaded %u/%u faces from /assets/fonts", static_cast<unsigned>(loaded_),
                   // 继续上一行的表达式或参数列表：static_cast<unsigned>(kSizeCount));。
                   static_cast<unsigned>(kSizeCount));
    // 返回 any_ready() 给调用者，表示当前函数的执行结果。
    return any_ready();
}

// 定义或声明函数 BitmapFontSet::deinit：参数在括号里，返回类型写在函数名前面。
void BitmapFontSet::deinit()
{
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (size_t i = 0; i < kSizeCount; ++i)
        // 调用 deinit()：创建和销毁队列。
        faces_[i].deinit();
    // 赋值：把右侧 0 的结果保存到左侧 loaded_。
    loaded_ = 0;
}

// 定义或声明函数 BitmapFontSet::any_ready：参数在括号里，返回类型写在函数名前面。
bool BitmapFontSet::any_ready() const
{
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (size_t i = 0; i < kSizeCount; ++i)
        // 判断条件：faces_[i].ready() 是否成立。
        if (faces_[i].ready())
            // 返回 true 给调用者，表示当前函数的执行结果。
            return true;
    // 返回 false 给调用者，表示当前函数的执行结果。
    return false;
}

// 只读约束：这个值或对象在当前位置不能被修改。
const BitmapFontFace* BitmapFontSet::get(uint8_t size_px) const
{
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (size_t i = 0; i < kSizeCount; ++i)
        // 判断条件：faces_[i].ready() && faces_[i].size_px() == size_px 是否成立。
        if (faces_[i].ready() && faces_[i].size_px() == size_px)
            // 返回 &faces_[i] 给调用者，表示当前函数的执行结果。
            return &faces_[i];
    // 返回 nullptr 给调用者，表示当前函数的执行结果。
    return nullptr;
}

// 只读约束：这个值或对象在当前位置不能被修改。
const BitmapFontFace* BitmapFontSet::nearest(uint8_t size_px) const
{
    // 优先「不小于 size_px 的最小」
    const BitmapFontFace* best = nullptr;
    // 定义变量 uint8_t best_px 并初始化为 255。
    uint8_t best_px = 255;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (size_t i = 0; i < kSizeCount; ++i)
    {
        // 判断条件：!faces_[i].ready() 是否成立。
        if (!faces_[i].ready())
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t px = faces_[i].size_px();
        // 判断条件：px >= size_px && px < best_px 是否成立。
        if (px >= size_px && px < best_px)
        {
            // 赋值：把右侧 px 的结果保存到左侧 best_px。
            best_px = px;
            // 赋值：把右侧 &faces_[i] 的结果保存到左侧 best。
            best = &faces_[i];
        }
    }
    // 判断条件：best != nullptr 是否成立。
    if (best != nullptr)
        // 返回 best 给调用者，表示当前函数的执行结果。
        return best;

    // 退化：返回最大已加载
    for (size_t i = 0; i < kSizeCount; ++i)
    {
        // 判断条件：!faces_[i].ready() 是否成立。
        if (!faces_[i].ready())
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        // 判断条件：best == nullptr || faces_[i].size_px() > best->size_px() 是否成立。
        if (best == nullptr || faces_[i].size_px() > best->size_px())
            // 赋值：把右侧 &faces_[i] 的结果保存到左侧 best。
            best = &faces_[i];
    }
    // 返回 best 给调用者，表示当前函数的执行结果。
    return best;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::gfx
