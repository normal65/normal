// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/gfx/bitmap_font.hpp
 * 作用：加载和绘制预烘焙点阵字体。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：绘制只改 framebuffer，不直接操作 SPI。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstddef>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/font.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::gfx {

/**
 * @brief 预烘焙位图字库（simhei_12/14/16.bin，magic "EBF1" v1，整文件载入 PSRAM）
 *
 * 布局：FileHeader(28B) + cp→gid LUT + DirEntry[16B] + 1-bit bitmap pool。
 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class BitmapFontFace
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 调用 BitmapFontFace() = default;，执行这个函数并处理它的返回值或状态变化。
    BitmapFontFace() = default;
    // 定义或声明函数 ~BitmapFontFace：参数在括号里，返回类型写在函数名前面。
    ~BitmapFontFace()
    {
        // 调用 deinit()：创建和销毁队列。
        deinit();
    }
    // 调用 BitmapFontFace(const BitmapFontFace&) = delete;，执行这个函数并处理它的返回值或状态变化。
    BitmapFontFace(const BitmapFontFace&) = delete;
    // 赋值：把右侧 (const BitmapFontFace&) = delete 的结果保存到左侧 BitmapFontFace& operator。
    BitmapFontFace& operator=(const BitmapFontFace&) = delete;

    // 函数调用：bool init(const char* path);；执行该函数并处理它的返回值或副作用。
    bool init(const char* path);
    // 调用 deinit()：创建和销毁队列。
    void deinit();

    // 定义或声明函数 ready：参数在括号里，返回类型写在函数名前面。
    bool ready() const
    {
        // 返回 data_ != nullptr 给调用者，表示当前函数的执行结果。
        return data_ != nullptr;
    }
    // 定义或声明函数 size_px：参数在括号里，返回类型写在函数名前面。
    uint8_t size_px() const
    {
        // 返回 size_px_ 给调用者，表示当前函数的执行结果。
        return size_px_;
    }
    // 函数定义/声明：调用 ascent()：advance(cp)： 查一个字符的前进宽度。点阵优先，FreeType 兜底。它不会绘制任何像素，适合排版测量。。
    uint16_t ascent() const
    {
        // 返回 ascent_ 给调用者，表示当前函数的执行结果。
        return ascent_;
    }
    // 函数定义/声明：调用 line_height()：advance(cp)： 查一个字符的前进宽度。点阵优先，FreeType 兜底。它不会绘制任何像素，适合排版测量。。
    uint16_t line_height() const
    {
        // 返回 line_height_ 给调用者，表示当前函数的执行结果。
        return line_height_;
    }

    // 函数调用：bool lookup(uint32_t cp, GlyphMetrics& m, const uint8_t*& bitmap) const;；执行该函数并处理它的返回值或副作用。
    bool lookup(uint32_t cp, GlyphMetrics& m, const uint8_t*& bitmap) const;
    // 调用 advance(uint32_tcp)：advance(cp)： 查一个字符的前进宽度。点阵优先，FreeType 兜底。它不会绘制任何像素，适合排版测量。。
    uint16_t advance(uint32_t cp) const;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // struct DirEntry：把相关数据字段组合在一起，表示项目中的一个概念。
    struct DirEntry
    {
        // 执行语句：uint16_t advance;；它会更新一次变量、对象或程序状态。
        uint16_t advance;
        // 执行语句：int16_t bearing_x;；它会更新一次变量、对象或程序状态。
        int16_t bearing_x;
        // 执行语句：int16_t bearing_y;；它会更新一次变量、对象或程序状态。
        int16_t bearing_y;
        // 执行语句：uint16_t width;；它会更新一次变量、对象或程序状态。
        uint16_t width;
        // 执行语句：uint16_t height;；它会更新一次变量、对象或程序状态。
        uint16_t height;
        // 执行语句：uint16_t pitch;；它会更新一次变量、对象或程序状态。
        uint16_t pitch;
        // 执行语句：uint32_t bitmap_ofs;；它会更新一次变量、对象或程序状态。
        uint32_t bitmap_ofs;
    };
    // 调用 static_assert(sizeof(DirEntry) == 16, "DirEntry layout (matches simhei_*.bin format)");，执行这个函数并处理它的返回值或状态变化。
    static_assert(sizeof(DirEntry) == 16, "DirEntry layout (matches simhei_*.bin format)");

    // 执行语句：uint8_t* data_{nullptr};；它会更新一次变量、对象或程序状态。
    uint8_t* data_{nullptr};
    // 执行语句：size_t data_size_{0};；它会更新一次变量、对象或程序状态。
    size_t data_size_{0};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t* lut_{nullptr};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const DirEntry* dir_{nullptr};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t* pool_{nullptr};
    // 执行语句：uint16_t glyph_count_{0};；它会更新一次变量、对象或程序状态。
    uint16_t glyph_count_{0};
    // 执行语句：uint8_t size_px_{0};；它会更新一次变量、对象或程序状态。
    uint8_t size_px_{0};
    // 执行语句：uint8_t ascent_{0};；它会更新一次变量、对象或程序状态。
    uint8_t ascent_{0};
    // 执行语句：uint8_t line_height_{0};；它会更新一次变量、对象或程序状态。
    uint8_t line_height_{0};
};

/** @brief 12 / 14 / 16 px 预烘焙字库集合（/assets/fonts） */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class BitmapFontSet
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态查表数组 kSizes：运行时复用固定数据，避免每次重新计算。
    static constexpr uint8_t kSizes[] = {12, 14, 16};
    // 固定参数 kSizeCount = sizeof(kSizes) / sizeof(kSizes[0])：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr size_t kSizeCount = sizeof(kSizes) / sizeof(kSizes[0]);

    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static BitmapFontSet& get_instance();

    // 函数调用：bool init();；执行该函数并处理它的返回值或副作用。
    bool init();
    // 调用 deinit()：创建和销毁队列。
    void deinit();

    // 函数调用：bool any_ready() const;；执行该函数并处理它的返回值或副作用。
    bool any_ready() const;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const BitmapFontFace* get(uint8_t size_px) const;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const BitmapFontFace* nearest(uint8_t size_px) const;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 BitmapFontSet() = default;，执行这个函数并处理它的返回值或状态变化。
    BitmapFontSet() = default;
    // 定义或声明函数 ~BitmapFontSet：参数在括号里，返回类型写在函数名前面。
    ~BitmapFontSet()
    {
        // 调用 deinit()：创建和销毁队列。
        deinit();
    }

    // 执行语句：BitmapFontFace faces_[kSizeCount]{};；它会更新一次变量、对象或程序状态。
    BitmapFontFace faces_[kSizeCount]{};
    // 执行语句：uint8_t loaded_{0};；它会更新一次变量、对象或程序状态。
    uint8_t loaded_{0};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::gfx
