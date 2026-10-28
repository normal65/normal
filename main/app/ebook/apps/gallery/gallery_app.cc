/**
 * [初学者导读] 文件：main/app/ebook/apps/gallery/gallery_app.cc
 * 作用：实现相册列表、图片解码、缩放和查看。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/gallery/gallery_app.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cctype>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <sys/stat.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <dirent.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_heap_caps.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <jpeg_decoder.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "bsp/driver/gdey027t91/framebuffer.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/icon.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/text_layout.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/toast.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_bus.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "memory/psram_alloc.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::gallery {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using Fb = ::app::bsp::driver::gdey027t91::Framebuffer;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using app::common::memory::alloc_array_psram;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using app::common::memory::alloc_psram_prefer;

// 固定参数 kPhotoRootInt = "/int/Ebook"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kPhotoRootInt = "/int/Ebook";
// 固定参数 kPhotoRootSd = "/sd/Ebook"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kPhotoRootSd = "/sd/Ebook";
// 固定参数 kPhotoDirInt = GalleryApp::kScanPathInt：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kPhotoDirInt = GalleryApp::kScanPathInt;
// 固定参数 kPhotoDirSd = GalleryApp::kScanPathSd：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kPhotoDirSd = GalleryApp::kScanPathSd;

// 定义变量 bool s_open_on_enter_ 并初始化为 false。
bool s_open_on_enter_ = false;
// 固定参数 kOpenPathCap = 96：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kOpenPathCap = 96;
// 执行语句：char s_open_path_[kOpenPathCap]{};；它会更新一次变量、对象或程序状态。
char s_open_path_[kOpenPathCap]{};

// 定义或声明函数 rgb565_gray：参数在括号里，返回类型写在函数名前面。
uint8_t rgb565_gray(uint16_t px)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t r5 = static_cast<uint8_t>((px >> 11) & 0x1FU);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t g6 = static_cast<uint8_t>((px >> 5) & 0x3FU);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t b5 = static_cast<uint8_t>(px & 0x1FU);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t r = static_cast<uint8_t>((r5 << 3) | (r5 >> 2));
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t g = static_cast<uint8_t>((g6 << 2) | (g6 >> 4));
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t b = static_cast<uint8_t>((b5 << 3) | (b5 >> 2));
    // 返回 static_cast<uint8_t>((static_cast<uint16_t>(r) * 77U + static_cast<uint16_t>(g) * 150U + 给调用者，表示当前函数的执行结果。
    return static_cast<uint8_t>((static_cast<uint16_t>(r) * 77U + static_cast<uint16_t>(g) * 150U +
                                 // 继续上一行的表达式或参数列表：static_cast<uint16_t>(b) * 29U) >>。
                                 static_cast<uint16_t>(b) * 29U) >>
                                // 执行语句：8);；它会更新一次变量、对象或程序状态。
                                8);
}

// 定义或声明函数 sample_gray_bilinear：参数在括号里，返回类型写在函数名前面。
uint8_t sample_gray_bilinear(const uint8_t* src, uint16_t sw, uint16_t sh, float sx, float sy)
{
    // 判断条件：src == nullptr || sw == 0 || sh == 0 是否成立。
    if (src == nullptr || sw == 0 || sh == 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 判断条件：sx < 0.0f 是否成立。
    if (sx < 0.0f)
        // 赋值：把右侧 0.0f 的结果保存到左侧 sx。
        sx = 0.0f;
    // 判断条件：sy < 0.0f 是否成立。
    if (sy < 0.0f)
        // 赋值：把右侧 0.0f 的结果保存到左侧 sy。
        sy = 0.0f;
    // 判断条件：sx > static_cast<float>(sw - 1U) 是否成立。
    if (sx > static_cast<float>(sw - 1U))
        // 赋值：把右侧 static_cast<float>(sw - 1U) 的结果保存到左侧 sx。
        sx = static_cast<float>(sw - 1U);
    // 判断条件：sy > static_cast<float>(sh - 1U) 是否成立。
    if (sy > static_cast<float>(sh - 1U))
        // 赋值：把右侧 static_cast<float>(sh - 1U) 的结果保存到左侧 sy。
        sy = static_cast<float>(sh - 1U);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t x0 = static_cast<uint16_t>(sx);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t y0 = static_cast<uint16_t>(sy);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t x1 = (x0 + 1U < sw) ? static_cast<uint16_t>(x0 + 1U) : x0;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t y1 = (y0 + 1U < sh) ? static_cast<uint16_t>(y0 + 1U) : y0;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const float fx = sx - static_cast<float>(x0);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const float fy = sy - static_cast<float>(y0);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const float w00 = (1.0f - fx) * (1.0f - fy);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const float w10 = fx * (1.0f - fy);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const float w01 = (1.0f - fx) * fy;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const float w11 = fx * fy;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const float v = static_cast<float>(src[static_cast<uint32_t>(y0) * sw + x0]) * w00 +
                    // 继续上一行的表达式或参数列表：static_cast<float>(src[static_cast<uint32_t>(y0) * sw + x1]) * w10 +。
                    static_cast<float>(src[static_cast<uint32_t>(y0) * sw + x1]) * w10 +
                    // 继续上一行的表达式或参数列表：static_cast<float>(src[static_cast<uint32_t>(y1) * sw + x0]) * w01 +。
                    static_cast<float>(src[static_cast<uint32_t>(y1) * sw + x0]) * w01 +
                    // 继续上一行的表达式或参数列表：static_cast<float>(src[static_cast<uint32_t>(y1) * sw + x1]) * w11;。
                    static_cast<float>(src[static_cast<uint32_t>(y1) * sw + x1]) * w11;

    // 判断条件：v <= 0.0f 是否成立。
    if (v <= 0.0f)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    // 判断条件：v >= 255.0f 是否成立。
    if (v >= 255.0f)
        // 返回 255 给调用者，表示当前函数的执行结果。
        return 255;
    // 返回 static_cast<uint8_t>(v + 0.5f) 给调用者，表示当前函数的执行结果。
    return static_cast<uint8_t>(v + 0.5f);
}

// 函数调用：void fit_draw_size(uint16_t src_w, uint16_t src_h, uint16_t max_w, uint16_t max_h, uint16_t& dst_w,；执行该函数并处理它的返回值或副作用。
void fit_draw_size(uint16_t src_w, uint16_t src_h, uint16_t max_w, uint16_t max_h, uint16_t& dst_w,
                   // 继续上一行的表达式或参数列表：uint16_t& dst_h)。
                   uint16_t& dst_h)
{
    // 判断条件：src_w == 0 || src_h == 0 || max_w == 0 || max_h == 0 是否成立。
    if (src_w == 0 || src_h == 0 || max_w == 0 || max_h == 0)
    {
        // 赋值：把右侧 src_w 的结果保存到左侧 dst_w。
        dst_w = src_w;
        // 赋值：把右侧 src_h 的结果保存到左侧 dst_h。
        dst_h = src_h;
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const float sx = static_cast<float>(max_w) / static_cast<float>(src_w);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const float sy = static_cast<float>(max_h) / static_cast<float>(src_h);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const float s = (sx < sy) ? sx : sy;

    // 赋值：把右侧 static_cast<uint16_t>(static_cast<float>(src_w) * s + 0.5f) 的结果保存到左侧 dst_w。
    dst_w = static_cast<uint16_t>(static_cast<float>(src_w) * s + 0.5f);
    // 赋值：把右侧 static_cast<uint16_t>(static_cast<float>(src_h) * s + 0.5f) 的结果保存到左侧 dst_h。
    dst_h = static_cast<uint16_t>(static_cast<float>(src_h) * s + 0.5f);
    // 判断条件：dst_w == 0 是否成立。
    if (dst_w == 0)
        // 赋值：把右侧 1 的结果保存到左侧 dst_w。
        dst_w = 1;
    // 判断条件：dst_h == 0 是否成立。
    if (dst_h == 0)
        // 赋值：把右侧 1 的结果保存到左侧 dst_h。
        dst_h = 1;
    // 判断条件：dst_w > max_w 是否成立。
    if (dst_w > max_w)
        // 赋值：把右侧 max_w 的结果保存到左侧 dst_w。
        dst_w = max_w;
    // 判断条件：dst_h > max_h 是否成立。
    if (dst_h > max_h)
        // 赋值：把右侧 max_h 的结果保存到左侧 dst_h。
        dst_h = max_h;
}

// 函数调用：bool scale_gray_fit(const uint8_t* src, uint16_t src_w, uint16_t src_h, uint8_t* dst,；执行该函数并处理它的返回值或副作用。
bool scale_gray_fit(const uint8_t* src, uint16_t src_w, uint16_t src_h, uint8_t* dst,
                    // 继续上一行的表达式或参数列表：uint16_t dst_w, uint16_t dst_h)。
                    uint16_t dst_w, uint16_t dst_h)
{
    // 判断条件：src == nullptr || dst == nullptr || src_w == 0 || src_h == 0 || dst_w == 0 || dst_h == 0 是否成立。
    if (src == nullptr || dst == nullptr || src_w == 0 || src_h == 0 || dst_w == 0 || dst_h == 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint16_t y = 0; y < dst_h; ++y)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const float sy =
            // 继续上一行的表达式或参数列表：(static_cast<float>(y) + 0.5f) * static_cast<float>(src_h) / static_cast<float>(dst_h) -。
            (static_cast<float>(y) + 0.5f) * static_cast<float>(src_h) / static_cast<float>(dst_h) -
            // 访问对象成员：0.5f;；读取或修改这个对象里的成员状态。
            0.5f;
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint16_t x = 0; x < dst_w; ++x)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const float sx = (static_cast<float>(x) + 0.5f) * static_cast<float>(src_w) /
                                 // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
                                 static_cast<float>(dst_w) -
                             // 访问对象成员：0.5f;；读取或修改这个对象里的成员状态。
                             0.5f;
            // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
            dst[static_cast<uint32_t>(y) * dst_w + x] =
                // 继续上一行的表达式或参数列表：sample_gray_bilinear(src, src_w, src_h, sx, sy);。
                sample_gray_bilinear(src, src_w, src_h, sx, sy);
        }
    }
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 dither_gray_to_fb：参数在括号里，返回类型写在函数名前面。
void dither_gray_to_fb(uint8_t* gray, uint16_t w, uint16_t h, uint8_t* fb, int16_t ox, int16_t oy)
{
    // 判断条件：gray == nullptr || fb == nullptr || w == 0 || h == 0 是否成立。
    if (gray == nullptr || fb == nullptr || w == 0 || h == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint16_t y = 0; y < h; ++y)
    {
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint16_t x = 0; x < w; ++x)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint32_t idx = static_cast<uint32_t>(y) * w + x;
            // 定义变量 int16_t old 并初始化为 static_cast<int16_t>(gray[idx])。
            int16_t old = static_cast<int16_t>(gray[idx]);
            // 判断条件：old < 0 是否成立。
            if (old < 0)
                // 赋值：把右侧 0 的结果保存到左侧 old。
                old = 0;
            // 判断条件：old > 255 是否成立。
            if (old > 255)
                // 赋值：把右侧 255 的结果保存到左侧 old。
                old = 255;

            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint8_t new_px = (old < 128) ? 0U : 255U;
            // 只读约束：这个值或对象在当前位置不能被修改。
            const int16_t err = static_cast<int16_t>(old - static_cast<int16_t>(new_px));
            // 赋值：把右侧 new_px 的结果保存到左侧 gray[idx]。
            gray[idx] = new_px;

            // 调用 Fb::set_pixel(fb, static_cast<int16_t>(ox + x), static_cast<int16_t>(oy + y),，执行这个函数并处理它的返回值或状态变化。
            Fb::set_pixel(fb, static_cast<int16_t>(ox + x), static_cast<int16_t>(oy + y),
                          // 继续上一行的表达式或参数列表：new_px == 0U);。
                          new_px == 0U);

            // 判断条件：x + 1U < w 是否成立。
            if (x + 1U < w)
            {
                // 定义变量：int16_t v =
                int16_t v =
                    // 继续上一行的表达式或参数列表：static_cast<int16_t>(gray[idx + 1U]) + static_cast<int16_t>((err * 7) / 16);。
                    static_cast<int16_t>(gray[idx + 1U]) + static_cast<int16_t>((err * 7) / 16);
                // 赋值：把右侧 static_cast<uint8_t>(v < 0 ? 0 : (v > 255 ? 255 : v)) 的结果保存到左侧 gray[idx + 1U]。
                gray[idx + 1U] = static_cast<uint8_t>(v < 0 ? 0 : (v > 255 ? 255 : v));
            }
            // 判断条件：y + 1U < h 是否成立。
            if (y + 1U < h)
            {
                // 判断条件：x > 0 是否成立。
                if (x > 0)
                {
                    // 只读约束：这个值或对象在当前位置不能被修改。
                    const uint32_t li = static_cast<uint32_t>(y + 1U) * w + (x - 1U);
                    // 定义变量：int16_t v =
                    int16_t v =
                        // 继续上一行的表达式或参数列表：static_cast<int16_t>(gray[li]) + static_cast<int16_t>((err * 3) / 16);。
                        static_cast<int16_t>(gray[li]) + static_cast<int16_t>((err * 3) / 16);
                    // 赋值：把右侧 static_cast<uint8_t>(v < 0 ? 0 : (v > 255 ? 255 : v)) 的结果保存到左侧 gray[li]。
                    gray[li] = static_cast<uint8_t>(v < 0 ? 0 : (v > 255 ? 255 : v));
                }
                {
                    // 只读约束：这个值或对象在当前位置不能被修改。
                    const uint32_t ci = static_cast<uint32_t>(y + 1U) * w + x;
                    // 定义变量：int16_t v =
                    int16_t v =
                        // 继续上一行的表达式或参数列表：static_cast<int16_t>(gray[ci]) + static_cast<int16_t>((err * 5) / 16);。
                        static_cast<int16_t>(gray[ci]) + static_cast<int16_t>((err * 5) / 16);
                    // 赋值：把右侧 static_cast<uint8_t>(v < 0 ? 0 : (v > 255 ? 255 : v)) 的结果保存到左侧 gray[ci]。
                    gray[ci] = static_cast<uint8_t>(v < 0 ? 0 : (v > 255 ? 255 : v));
                }
                // 判断条件：x + 1U < w 是否成立。
                if (x + 1U < w)
                {
                    // 只读约束：这个值或对象在当前位置不能被修改。
                    const uint32_t ri = static_cast<uint32_t>(y + 1U) * w + (x + 1U);
                    // 定义变量：int16_t v =
                    int16_t v =
                        // 继续上一行的表达式或参数列表：static_cast<int16_t>(gray[ri]) + static_cast<int16_t>((err * 1) / 16);。
                        static_cast<int16_t>(gray[ri]) + static_cast<int16_t>((err * 1) / 16);
                    // 赋值：把右侧 static_cast<uint8_t>(v < 0 ? 0 : (v > 255 ? 255 : v)) 的结果保存到左侧 gray[ri]。
                    gray[ri] = static_cast<uint8_t>(v < 0 ? 0 : (v > 255 ? 255 : v));
                }
            }
        }
    }
}

// 定义或声明函数 jpeg_scale_div：参数在括号里，返回类型写在函数名前面。
uint8_t jpeg_scale_div(esp_jpeg_image_scale_t scale)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (scale)
    {
        // 分支匹配：case JPEG_IMAGE_SCALE_0 成立时执行下面的代码。
        case JPEG_IMAGE_SCALE_0:
            // 返回 1U 给调用者，表示当前函数的执行结果。
            return 1U;
        // 分支匹配：case JPEG_IMAGE_SCALE_1_2 成立时执行下面的代码。
        case JPEG_IMAGE_SCALE_1_2:
            // 返回 2U 给调用者，表示当前函数的执行结果。
            return 2U;
        // 分支匹配：case JPEG_IMAGE_SCALE_1_4 成立时执行下面的代码。
        case JPEG_IMAGE_SCALE_1_4:
            // 返回 4U 给调用者，表示当前函数的执行结果。
            return 4U;
        // 分支匹配：case JPEG_IMAGE_SCALE_1_8 成立时执行下面的代码。
        case JPEG_IMAGE_SCALE_1_8:
            // 返回 8U 给调用者，表示当前函数的执行结果。
            return 8U;
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 返回 1U 给调用者，表示当前函数的执行结果。
            return 1U;
    }
}

// 函数定义/声明：调用 ends_with_ci()：做不区分大小写的后缀匹配，例如 .TXT、.Txt、.txt 都应识别为文本。它是打开类型分类的基础。。
bool ends_with_ci(const char* name, const char* ext)
{
    // 判断条件：name == nullptr || ext == nullptr 是否成立。
    if (name == nullptr || ext == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t nlen = std::strlen(name);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t elen = std::strlen(ext);
    // 判断条件：nlen < elen 是否成立。
    if (nlen < elen)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* tail = name + nlen - elen;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (size_t i = 0; i < elen; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const unsigned char a = static_cast<unsigned char>(tail[i]);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const unsigned char b = static_cast<unsigned char>(ext[i]);
        // 判断条件：std::tolower(a) != std::tolower(b) 是否成立。
        if (std::tolower(a) != std::tolower(b))
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
    }
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 GalleryApp::instance：参数在括号里，返回类型写在函数名前面。
GalleryApp& GalleryApp::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static GalleryApp s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义或声明函数 GalleryApp::GalleryApp：参数在括号里，返回类型写在函数名前面。
GalleryApp::GalleryApp()
{
    // 函数调用：(void)ensure_store();；执行该函数并处理它的返回值或副作用。
    (void)ensure_store();
    // 访问对象成员：list_.set_area(Theme::list_region());；读取或修改这个对象里的成员状态。
    list_.set_area(Theme::list_region());
    // 访问对象成员：list_.set_provider([this](uint8_t i, ui::widgets::RowStyle& rs) { fill_row(i, rs); });；读取或修改这个对象里的成员状态。
    list_.set_provider([this](uint8_t i, ui::widgets::RowStyle& rs) { fill_row(i, rs); });
    // 调用 on_row_tap(i)：处理列表点击： 如果点击父目录行，调用 goup()。 如果点击根卷，调用 openroot() 并调度扫描。 如果点击目录，调用 enterdir()。 如果点击文件，调用 showfileaction()。。
    list_.set_tap_handler([this](uint8_t i) { on_row_tap(i); });
}

// 定义或声明函数 GalleryApp::ensure_store：参数在括号里，返回类型写在函数名前面。
bool GalleryApp::ensure_store()
{
    // 判断条件：photos_ != nullptr && size_buf_ != nullptr 是否成立。
    if (photos_ != nullptr && size_buf_ != nullptr)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    // 判断条件：photos_ == nullptr 是否成立。
    if (photos_ == nullptr)
        // 赋值：把右侧 alloc_array_psram<PhotoItem>(kMaxPhotos) 的结果保存到左侧 photos_。
        photos_ = alloc_array_psram<PhotoItem>(kMaxPhotos);
    // 判断条件：size_buf_ == nullptr 是否成立。
    if (size_buf_ == nullptr)
        // 赋值：把右侧 static_cast<char (*)[12]>(alloc_psram_prefer(sizeof(char[kMaxPhotos][12]))) 的结果保存到左侧 size_buf_。
        size_buf_ = static_cast<char (*)[12]>(alloc_psram_prefer(sizeof(char[kMaxPhotos][12])));
    // 返回 photos_ != nullptr && size_buf_ != nullptr 给调用者，表示当前函数的执行结果。
    return photos_ != nullptr && size_buf_ != nullptr;
}

// 定义只读对象或变量：初始化后不能在当前位置修改。
const char* GalleryApp::title() const
{
    // 判断条件：view_ == View::Viewer && title_buf_[0] != '\0' 是否成立。
    if (view_ == View::Viewer && title_buf_[0] != '\0')
        // 返回 title_buf_ 给调用者，表示当前函数的执行结果。
        return title_buf_;
    // 返回 ui::strings::kAppGallery 给调用者，表示当前函数的执行结果。
    return ui::strings::kAppGallery;
}

// 函数定义/声明：调用 GalleryApp::icon_cp()：返回图标 codepoint。图标通常来自字体图标表，所以这里返回的是 uint32t，不是图片路径。。
uint32_t GalleryApp::icon_cp() const
{
    // 返回 gfx::icon::kFaImages 给调用者，表示当前函数的执行结果。
    return gfx::icon::kFaImages;
}

// 定义或声明函数 GalleryApp::viewer_rect：参数在括号里，返回类型写在函数名前面。
core::Rect GalleryApp::viewer_rect()
{
    // 返回 core::Rect{0, Theme::kContentY, Theme::kScreenW, 给调用者，表示当前函数的执行结果。
    return core::Rect{0, Theme::kContentY, Theme::kScreenW,
                      // 继续上一行的表达式或参数列表：static_cast<uint16_t>(Theme::kScreenH - Theme::kContentY)};。
                      static_cast<uint16_t>(Theme::kScreenH - Theme::kContentY)};
}

// 定义或声明函数 GalleryApp::viewer_image_rect：参数在括号里，返回类型写在函数名前面。
core::Rect GalleryApp::viewer_image_rect()
{
    // 返回 viewer_rect() 给调用者，表示当前函数的执行结果。
    return viewer_rect();
}

// 定义或声明函数 GalleryApp::is_jpeg_name：参数在括号里，返回类型写在函数名前面。
bool GalleryApp::is_jpeg_name(const char* name)
{
    // 判断条件：name == nullptr || name[0] == '.' 是否成立。
    if (name == nullptr || name[0] == '.')
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 返回结果：调用 ends_with_ci(name,".jpg")：做不区分大小写的后缀匹配，例如 .TXT、.Txt、.txt 都应识别为文本。它是打开类型分类的基础。。
    return ends_with_ci(name, ".jpg") || ends_with_ci(name, ".jpeg");
}

// 定义或声明函数 GalleryApp::title_from_path：参数在括号里，返回类型写在函数名前面。
void GalleryApp::title_from_path(const char* path, char* out, size_t out_size)
{
    // 判断条件：path == nullptr || out == nullptr || out_size == 0 是否成立。
    if (path == nullptr || out == nullptr || out_size == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* name = std::strrchr(path, '/');
    // 赋值：把右侧 (name != nullptr) ? (name + 1) : path 的结果保存到左侧 name。
    name = (name != nullptr) ? (name + 1) : path;
    // 函数调用：(void)std::strncpy(out, name, out_size - 1);；执行该函数并处理它的返回值或副作用。
    (void)std::strncpy(out, name, out_size - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 out[out_size - 1]。
    out[out_size - 1] = '\0';
    // 赋值：把右侧 std::strrchr(out, '.') 的结果保存到左侧 char* dot。
    char* dot = std::strrchr(out, '.');
    // 判断条件：dot != nullptr 是否成立。
    if (dot != nullptr)
        *dot = '\0';
}

// 定义或声明函数 GalleryApp::scan_dir：参数在括号里，返回类型写在函数名前面。
void GalleryApp::scan_dir(const char* dir, uint8_t& count)
{
    // 判断条件：dir == nullptr || photos_ == nullptr || count >= kMaxPhotos 是否成立。
    if (dir == nullptr || photos_ == nullptr || count >= kMaxPhotos)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // struct stat：把相关数据字段组合在一起，表示项目中的一个概念。
    struct stat st{};
    // 判断条件：::stat(dir, &st) != 0 是否成立。
    if (::stat(dir, &st) != 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 赋值：把右侧 ::opendir(dir) 的结果保存到左侧 DIR* d。
    DIR* d = ::opendir(dir);
    // 判断条件：d == nullptr 是否成立。
    if (d == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // struct dirent：把相关数据字段组合在一起，表示项目中的一个概念。
    struct dirent* ent = nullptr;
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while ((ent = ::readdir(d)) != nullptr && count < kMaxPhotos)
    {
        // 判断条件：!is_jpeg_name(ent->d_name) 是否成立。
        if (!is_jpeg_name(ent->d_name))
            // 继续下一轮：跳过本次循环剩余代码。
            continue;

        // 赋值：把右侧 photos_[count] 的结果保存到左侧 PhotoItem& item。
        PhotoItem& item = photos_[count];
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int plen = std::snprintf(item.path, sizeof(item.path), "%s/%s", dir, ent->d_name);
        // 判断条件：plen < 0 || static_cast<size_t>(plen) >= sizeof(item.path) 是否成立。
        if (plen < 0 || static_cast<size_t>(plen) >= sizeof(item.path))
            // 继续下一轮：跳过本次循环剩余代码。
            continue;

        // 调用 title_from_path(item.path, item.title, sizeof(item.title));，执行这个函数并处理它的返回值或状态变化。
        title_from_path(item.path, item.title, sizeof(item.title));
        // 判断条件：::stat(item.path, &st) == 0 是否成立。
        if (::stat(item.path, &st) == 0)
            // 赋值：把右侧 static_cast<uint32_t>(st.st_size) 的结果保存到左侧 item.size。
            item.size = static_cast<uint32_t>(st.st_size);
        // 备用分支：前面的条件不成立时执行这里。
        else
            // 赋值：把右侧 0 的结果保存到左侧 item.size。
            item.size = 0;

        // 执行语句：++count;；它会更新一次变量、对象或程序状态。
        ++count;
    }
    // 函数调用：::closedir(d);；执行该函数并处理它的返回值或副作用。
    ::closedir(d);
}

// 定义或声明函数 GalleryApp::scan_photos：参数在括号里，返回类型写在函数名前面。
void GalleryApp::scan_photos()
{
    // 赋值：把右侧 0 的结果保存到左侧 count_。
    count_ = 0;
    // 判断条件：!ensure_store() 是否成立。
    if (!ensure_store())
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 函数调用：(void)::mkdir(kPhotoRootInt, 0775);；执行该函数并处理它的返回值或副作用。
    (void)::mkdir(kPhotoRootInt, 0775);
    // 函数调用：(void)::mkdir(kPhotoDirInt, 0775);；执行该函数并处理它的返回值或副作用。
    (void)::mkdir(kPhotoDirInt, 0775);
    // 函数调用：(void)::mkdir(kPhotoRootSd, 0775);；执行该函数并处理它的返回值或副作用。
    (void)::mkdir(kPhotoRootSd, 0775);
    // 函数调用：(void)::mkdir(kPhotoDirSd, 0775);；执行该函数并处理它的返回值或副作用。
    (void)::mkdir(kPhotoDirSd, 0775);

    // 调用 scan_dir(kPhotoDirInt, count_);，执行这个函数并处理它的返回值或状态变化。
    scan_dir(kPhotoDirInt, count_);
    // 调用 scan_dir(kPhotoDirSd, count_);，执行这个函数并处理它的返回值或状态变化。
    scan_dir(kPhotoDirSd, count_);

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i + 1 < count_; ++i)
    {
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint8_t j = 0; j + 1 < count_ - i; ++j)
        {
            // 判断条件：std::strcmp(photos_[j].title, photos_[j + 1].title) > 0 是否成立。
            if (std::strcmp(photos_[j].title, photos_[j + 1].title) > 0)
            {
                // 定义变量 PhotoItem tmp 并初始化为 photos_[j]。
                PhotoItem tmp = photos_[j];
                // 赋值：把右侧 photos_[j + 1] 的结果保存到左侧 photos_[j]。
                photos_[j] = photos_[j + 1];
                // 赋值：把右侧 tmp 的结果保存到左侧 photos_[j + 1]。
                photos_[j + 1] = tmp;
            }
        }
    }
}

// 定义或声明函数 GalleryApp::request_open_on_enter：参数在括号里，返回类型写在函数名前面。
void GalleryApp::request_open_on_enter(const char* path)
{
    // 判断条件：path == nullptr || path[0] == '\0' 是否成立。
    if (path == nullptr || path[0] == '\0')
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 函数调用：(void)std::strncpy(s_open_path_, path, kOpenPathCap - 1);；执行该函数并处理它的返回值或副作用。
    (void)std::strncpy(s_open_path_, path, kOpenPathCap - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 s_open_path_[kOpenPathCap - 1]。
    s_open_path_[kOpenPathCap - 1] = '\0';
    // 赋值：把右侧 true 的结果保存到左侧 s_open_on_enter_。
    s_open_on_enter_ = true;
}

// 定义或声明函数 GalleryApp::on_enter：参数在括号里，返回类型写在函数名前面。
void GalleryApp::on_enter()
{
    // 赋值：把右侧 View::List 的结果保存到左侧 view_。
    view_ = View::List;
    // 赋值：把右侧 0 的结果保存到左侧 sel_。
    sel_ = 0;
    // 赋值：把右侧 false 的结果保存到左侧 decoding_。
    decoding_ = false;
    // 赋值：把右侧 0 的结果保存到左侧 decode_err_。
    decode_err_ = 0;
    // 赋值：把右侧 false 的结果保存到左侧 image_ready_。
    image_ready_ = false;
    // 赋值：把右侧 '\0' 的结果保存到左侧 path_[0]。
    path_[0] = '\0';
    // 赋值：把右侧 '\0' 的结果保存到左侧 title_buf_[0]。
    title_buf_[0] = '\0';
    // 访问对象成员：list_.set_scroll(0);；读取或修改这个对象里的成员状态。
    list_.set_scroll(0);

    // 判断条件：viewer_fb_ == nullptr 是否成立。
    if (viewer_fb_ == nullptr)
    {
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        viewer_fb_ = static_cast<uint8_t*>(
            // 继续上一行的表达式或参数列表：heap_caps_malloc(core::kFbBytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));。
            heap_caps_malloc(core::kFbBytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
        // 判断条件：viewer_fb_ == nullptr 是否成立。
        if (viewer_fb_ == nullptr)
            // 赋值：把右侧 static_cast<uint8_t*>(heap_caps_malloc(core::kFbBytes, MALLOC_CAP_8BIT)) 的结果保存到左侧 viewer_fb_。
            viewer_fb_ = static_cast<uint8_t*>(heap_caps_malloc(core::kFbBytes, MALLOC_CAP_8BIT));
    }

    // 调用 start_scan();，执行这个函数并处理它的返回值或状态变化。
    start_scan();
}

// 函数定义/声明：调用 GalleryApp::on_exit()：退出时取消未完成的扫描、关闭弹窗，并确保后台任务不会在对象状态失效后继续写数据。。
void GalleryApp::on_exit()
{
    // 调用 cancel_decode();，执行这个函数并处理它的返回值或状态变化。
    cancel_decode();
    // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    scan_task_.reset();
    // 赋值：把右侧 false 的结果保存到左侧 scanning_。
    scanning_ = false;
    // 赋值：把右侧 View::List 的结果保存到左侧 view_。
    view_ = View::List;
}

// 函数定义/声明：调用 GalleryApp::on_semantic_back()：处理语义返回，顺序通常是： 1. 如果弹窗打开，先关闭弹窗。 2. 如果列表还在浏览目录，先返回上一级。 3. 如果已经到根卷，返回 false，让 Shell 退出 App。。
bool GalleryApp::on_semantic_back()
{
    // 判断条件：view_ == View::Viewer 是否成立。
    if (view_ == View::Viewer)
    {
        // 调用 cancel_decode();，执行这个函数并处理它的返回值或状态变化。
        cancel_decode();
        // 赋值：把右侧 View::List 的结果保存到左侧 view_。
        view_ = View::List;
        // 赋值：把右侧 false 的结果保存到左侧 image_ready_。
        image_ready_ = false;
        // 赋值：把右侧 0 的结果保存到左侧 decode_err_。
        decode_err_ = 0;
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 返回 false 给调用者，表示当前函数的执行结果。
    return false;
}

// 定义或声明函数 GalleryApp::start_scan：参数在括号里，返回类型写在函数名前面。
void GalleryApp::start_scan()
{
    // 判断条件：scanning_ 是否成立。
    if (scanning_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 true 的结果保存到左侧 scanning_。
    scanning_ = true;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();

    // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    scan_task_.reset();

    // 定义变量：::app::sys::task::Cfg cfg =
    ::app::sys::task::Cfg cfg =
        // 继续上一行的表达式或参数列表：::app::sys::task::Cfg::light("gallery_scan", ::app::sys::task::Priority::LOW);。
        ::app::sys::task::Cfg::light("gallery_scan", ::app::sys::task::Priority::LOW);
    // 赋值：把右侧 kScanStack 的结果保存到左侧 cfg.stack_size。
    cfg.stack_size = kScanStack;
    // 赋值：把右侧 true 的结果保存到左侧 cfg.use_psram。
    cfg.use_psram = true;

    // 执行这条代码：scan_task_ = std::make_unique<::app::sys::task::Task>(。它会更新变量、对象状态或调用链。
    scan_task_ = std::make_unique<::app::sys::task::Task>(
        // 开始代码块：下面是这个函数、分支或循环的主体。
        [](void* arg) {
            // 赋值：把右侧 static_cast<GalleryApp*>(arg) 的结果保存到左侧 auto* self。
            auto* self = static_cast<GalleryApp*>(arg);
            // 访问对象成员：self->scan_photos();；读取或修改这个对象里的成员状态。
            self->scan_photos();
            // 函数调用：(void)ui::UiBus::get_instance().post_system_hint(ui::SystemHintKind::GalleryScanDone,；执行该函数并处理它的返回值或副作用。
            (void)ui::UiBus::get_instance().post_system_hint(ui::SystemHintKind::GalleryScanDone,
                                                             // 继续上一行的表达式或参数列表：0);。
                                                             0);
        // 结束上一行的 lambda 或初始化列表：把这段匿名代码/数据交给调用者。
        },
        // 继续上一行的表达式或参数列表：cfg, this);。
        cfg, this);

    // 判断条件：!scan_task_->start() 是否成立。
    if (!scan_task_->start())
    {
        // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
        scan_task_.reset();
        // 赋值：把右侧 false 的结果保存到左侧 scanning_。
        scanning_ = false;
        // 调用 scan_photos();，执行这个函数并处理它的返回值或状态变化。
        scan_photos();
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
    }
}

// 定义或声明函数 GalleryApp::on_ui_event：参数在括号里，返回类型写在函数名前面。
void GalleryApp::on_ui_event(const ui::UiEvent& ev)
{
    // 判断条件：ev.kind != ui::UiEventKind::SystemHint 是否成立。
    if (ev.kind != ui::UiEventKind::SystemHint)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 多分支判断：根据括号里的值选择不同 case。
    switch (ev.payload.system.hint)
    {
        // 分支匹配：case ui::SystemHintKind::GalleryScanDone:  成立时执行下面的代码。
        case ui::SystemHintKind::GalleryScanDone: {
            // 赋值：把右侧 false 的结果保存到左侧 scanning_。
            scanning_ = false;
            // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
            scan_task_.reset();
            // 访问对象成员：list_.set_total(count_);；读取或修改这个对象里的成员状态。
            list_.set_total(count_);
            // 访问对象成员：list_.set_scroll(0);；读取或修改这个对象里的成员状态。
            list_.set_scroll(0);

            // 判断条件：s_open_on_enter_ 是否成立。
            if (s_open_on_enter_)
            {
                // 赋值：把右侧 false 的结果保存到左侧 s_open_on_enter_。
                s_open_on_enter_ = false;
                // 循环执行：按括号里的初始化、条件和步进规则重复运行。
                for (uint8_t i = 0; i < count_; ++i)
                {
                    // 判断条件：photos_ != nullptr && std::strcmp(photos_[i].path, s_open_path_) == 0 是否成立。
                    if (photos_ != nullptr && std::strcmp(photos_[i].path, s_open_path_) == 0)
                    {
                        // 调用 open_photo(i);，执行这个函数并处理它的返回值或状态变化。
                        open_photo(i);
                        // 提前返回：结束当前函数，不返回具体值。
                        return;
                    }
                }
                // 函数调用：(void)std::strncpy(path_, s_open_path_, sizeof(path_) - 1);；执行该函数并处理它的返回值或副作用。
                (void)std::strncpy(path_, s_open_path_, sizeof(path_) - 1);
                // 赋值：把右侧 '\0' 的结果保存到左侧 path_[sizeof(path_) - 1]。
                path_[sizeof(path_) - 1] = '\0';
                // 调用 title_from_path(path_, title_buf_, sizeof(title_buf_));，执行这个函数并处理它的返回值或状态变化。
                title_from_path(path_, title_buf_, sizeof(title_buf_));
                // 赋值：把右侧 View::Viewer 的结果保存到左侧 view_。
                view_ = View::Viewer;
                // 调用 start_decode(path_);，执行这个函数并处理它的返回值或状态变化。
                start_decode(path_);
            }
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            request_repaint();
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        }
        // 分支匹配：case ui::SystemHintKind::GalleryDecodeDone:  成立时执行下面的代码。
        case ui::SystemHintKind::GalleryDecodeDone: {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint32_t packed = ev.payload.system.value;
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint8_t job_id = static_cast<uint8_t>(packed >> 8);
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint8_t code = static_cast<uint8_t>(packed & 0xFFU);
            // 判断条件：job_id != decode_job_id_ 是否成立。
            if (job_id != decode_job_id_)
                // 提前返回：结束当前函数，不返回具体值。
                return;
            // 赋值：把右侧 false 的结果保存到左侧 decoding_。
            decoding_ = false;
            // 赋值：把右侧 code 的结果保存到左侧 decode_err_。
            decode_err_ = code;
            // 赋值：把右侧 (decode_err_ == 0) 的结果保存到左侧 image_ready_。
            image_ready_ = (decode_err_ == 0);
            // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
            decode_task_.reset();
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            request_repaint();
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        }
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
}

// 定义或声明函数 GalleryApp::fill_row：参数在括号里，返回类型写在函数名前面。
void GalleryApp::fill_row(uint8_t idx, ui::widgets::RowStyle& rs)
{
    // 赋值：把右侧 {} 的结果保存到左侧 rs。
    rs = {};
    // 判断条件：photos_ == nullptr || size_buf_ == nullptr || idx >= count_ 是否成立。
    if (photos_ == nullptr || size_buf_ == nullptr || idx >= count_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 photos_[idx].title 的结果保存到左侧 rs.label。
    rs.label = photos_[idx].title;
    // 赋值：把右侧 size_buf_[idx] 的结果保存到左侧 rs.value。
    rs.value = size_buf_[idx];
    // 赋值：把右侧 gfx::icon::kFaImages 的结果保存到左侧 rs.icon_cp。
    rs.icon_cp = gfx::icon::kFaImages;
    // 赋值：把右侧 true 的结果保存到左侧 rs.show_chevron。
    rs.show_chevron = true;
    // 调用 gfx::format_file_size(photos_[idx].size, size_buf_[idx], sizeof(size_buf_[idx]));，执行这个函数并处理它的返回值或状态变化。
    gfx::format_file_size(photos_[idx].size, size_buf_[idx], sizeof(size_buf_[idx]));
}

// 定义或声明函数 GalleryApp::open_photo：参数在括号里，返回类型写在函数名前面。
void GalleryApp::open_photo(uint8_t idx)
{
    // 判断条件：photos_ == nullptr || idx >= count_ 是否成立。
    if (photos_ == nullptr || idx >= count_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 idx 的结果保存到左侧 sel_。
    sel_ = idx;
    // 访问对象成员：(void)std::strncpy(path_, photos_[idx].path, sizeof(path_) - 1);；读取或修改这个对象里的成员状态。
    (void)std::strncpy(path_, photos_[idx].path, sizeof(path_) - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 path_[sizeof(path_) - 1]。
    path_[sizeof(path_) - 1] = '\0';
    // 调用 title_from_path(path_, title_buf_, sizeof(title_buf_));，执行这个函数并处理它的返回值或状态变化。
    title_from_path(path_, title_buf_, sizeof(title_buf_));
    // 赋值：把右侧 View::Viewer 的结果保存到左侧 view_。
    view_ = View::Viewer;
    // 赋值：把右侧 false 的结果保存到左侧 image_ready_。
    image_ready_ = false;
    // 赋值：把右侧 0 的结果保存到左侧 decode_err_。
    decode_err_ = 0;
    // 调用 start_decode(path_);，执行这个函数并处理它的返回值或状态变化。
    start_decode(path_);
}

// 函数定义/声明：调用 GalleryApp::on_row_tap()：处理列表点击： 如果点击父目录行，调用 goup()。 如果点击根卷，调用 openroot() 并调度扫描。 如果点击目录，调用 enterdir()。 如果点击文件，调用 showfileaction()。。
void GalleryApp::on_row_tap(uint8_t idx)
{
    // 判断条件：scanning_ || decoding_ 是否成立。
    if (scanning_ || decoding_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 open_photo(idx);，执行这个函数并处理它的返回值或状态变化。
    open_photo(idx);
}

// 定义或声明函数 GalleryApp::show_prev：参数在括号里，返回类型写在函数名前面。
void GalleryApp::show_prev()
{
    // 判断条件：count_ == 0 || decoding_ 是否成立。
    if (count_ == 0 || decoding_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：sel_ == 0 是否成立。
    if (sel_ == 0)
        // 赋值：把右侧 static_cast<uint8_t>(count_ - 1) 的结果保存到左侧 sel_。
        sel_ = static_cast<uint8_t>(count_ - 1);
    // 备用分支：前面的条件不成立时执行这里。
    else
        // 执行语句：--sel_;；它会更新一次变量、对象或程序状态。
        --sel_;
    // 调用 open_photo(sel_);，执行这个函数并处理它的返回值或状态变化。
    open_photo(sel_);
}

// 定义或声明函数 GalleryApp::show_next：参数在括号里，返回类型写在函数名前面。
void GalleryApp::show_next()
{
    // 判断条件：count_ == 0 || decoding_ 是否成立。
    if (count_ == 0 || decoding_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 static_cast<uint8_t>((sel_ + 1U) % count_) 的结果保存到左侧 sel_。
    sel_ = static_cast<uint8_t>((sel_ + 1U) % count_);
    // 调用 open_photo(sel_);，执行这个函数并处理它的返回值或状态变化。
    open_photo(sel_);
}

// 定义或声明函数 GalleryApp::cancel_decode：参数在括号里，返回类型写在函数名前面。
void GalleryApp::cancel_decode()
{
    // 执行语句：++decode_job_id_;；它会更新一次变量、对象或程序状态。
    ++decode_job_id_;
    // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    decode_task_.reset();
    // 赋值：把右侧 false 的结果保存到左侧 decoding_。
    decoding_ = false;
}

// 定义或声明函数 GalleryApp::start_decode：参数在括号里，返回类型写在函数名前面。
void GalleryApp::start_decode(const char* path)
{
    // 调用 cancel_decode();，执行这个函数并处理它的返回值或状态变化。
    cancel_decode();
    // 赋值：把右侧 true 的结果保存到左侧 decoding_。
    decoding_ = true;
    // 赋值：把右侧 false 的结果保存到左侧 image_ready_。
    image_ready_ = false;
    // 赋值：把右侧 0 的结果保存到左侧 decode_err_。
    decode_err_ = 0;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();

    // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    decode_task_.reset();

    // 函数调用：(void)std::strncpy(path_, path, sizeof(path_) - 1);；执行该函数并处理它的返回值或副作用。
    (void)std::strncpy(path_, path, sizeof(path_) - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 path_[sizeof(path_) - 1]。
    path_[sizeof(path_) - 1] = '\0';

    // 定义变量：::app::sys::task::Cfg cfg =
    ::app::sys::task::Cfg cfg =
        // 继续上一行的表达式或参数列表：::app::sys::task::Cfg::light("gallery_dec", ::app::sys::task::Priority::LOW);。
        ::app::sys::task::Cfg::light("gallery_dec", ::app::sys::task::Priority::LOW);
    // 赋值：把右侧 kDecodeStack 的结果保存到左侧 cfg.stack_size。
    cfg.stack_size = kDecodeStack;
    // 赋值：把右侧 true 的结果保存到左侧 cfg.use_psram。
    cfg.use_psram = true;

    // 赋值：把右侧 std::make_unique<::app::sys::task::Task>(&GalleryApp::decode_worker, cfg, this) 的结果保存到左侧 decode_task_。
    decode_task_ = std::make_unique<::app::sys::task::Task>(&GalleryApp::decode_worker, cfg, this);

    // 判断条件：!decode_task_->start() 是否成立。
    if (!decode_task_->start())
    {
        // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
        decode_task_.reset();
        // 赋值：把右侧 false 的结果保存到左侧 decoding_。
        decoding_ = false;
        // 赋值：把右侧 4 的结果保存到左侧 decode_err_。
        decode_err_ = 4;
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
    }
}

// 定义或声明函数 GalleryApp::decode_worker：参数在括号里，返回类型写在函数名前面。
void GalleryApp::decode_worker(void* arg)
{
    // 赋值：把右侧 static_cast<GalleryApp*>(arg) 的结果保存到左侧 auto* self。
    auto* self = static_cast<GalleryApp*>(arg);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t job_id = self->decode_job_id_;
    // 定义变量 uint8_t err 并初始化为 0。
    uint8_t err = 0;

    // 判断条件：self->viewer_fb_ == nullptr 是否成立。
    if (self->viewer_fb_ == nullptr)
    {
        // 赋值：把右侧 4 的结果保存到左侧 err。
        err = 4;
        // 执行语句：goto done;；它会更新一次变量、对象或程序状态。
        goto done;
    }

    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* path = self->path_;
        // struct stat：把相关数据字段组合在一起，表示项目中的一个概念。
        struct stat st{};
        // 判断条件：::stat(path, &st) != 0 是否成立。
        if (::stat(path, &st) != 0)
        {
            // 赋值：把右侧 1 的结果保存到左侧 err。
            err = 1;
            // 执行语句：goto done;；它会更新一次变量、对象或程序状态。
            goto done;
        }
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint32_t file_sz = static_cast<uint32_t>(st.st_size);
        // 判断条件：file_sz == 0 || file_sz > kMaxJpegBytes 是否成立。
        if (file_sz == 0 || file_sz > kMaxJpegBytes)
        {
            // 赋值：把右侧 2 的结果保存到左侧 err。
            err = 2;
            // 执行语句：goto done;；它会更新一次变量、对象或程序状态。
            goto done;
        }

        // 执行这条代码：uint8_t* file_buf =。它会更新变量、对象状态或调用链。
        uint8_t* file_buf =
            // 继续上一行的表达式或参数列表：static_cast<uint8_t*>(heap_caps_malloc(file_sz, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));。
            static_cast<uint8_t*>(heap_caps_malloc(file_sz, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
        // 判断条件：file_buf == nullptr 是否成立。
        if (file_buf == nullptr)
            // 赋值：把右侧 static_cast<uint8_t*>(heap_caps_malloc(file_sz, MALLOC_CAP_8BIT)) 的结果保存到左侧 file_buf。
            file_buf = static_cast<uint8_t*>(heap_caps_malloc(file_sz, MALLOC_CAP_8BIT));
        // 判断条件：file_buf == nullptr 是否成立。
        if (file_buf == nullptr)
        {
            // 赋值：把右侧 4 的结果保存到左侧 err。
            err = 4;
            // 执行语句：goto done;；它会更新一次变量、对象或程序状态。
            goto done;
        }

        // 赋值：把右侧 std::fopen(path, "rb") 的结果保存到左侧 FILE* f。
        FILE* f = std::fopen(path, "rb");
        // 判断条件：f == nullptr 是否成立。
        if (f == nullptr)
        {
            // 调用 heap_caps_free(file_buf);，执行这个函数并处理它的返回值或状态变化。
            heap_caps_free(file_buf);
            // 赋值：把右侧 1 的结果保存到左侧 err。
            err = 1;
            // 执行语句：goto done;；它会更新一次变量、对象或程序状态。
            goto done;
        }
        // 只读约束：这个值或对象在当前位置不能被修改。
        const size_t nread = std::fread(file_buf, 1, file_sz, f);
        // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(f);
        // 判断条件：nread != file_sz 是否成立。
        if (nread != file_sz)
        {
            // 调用 heap_caps_free(file_buf);，执行这个函数并处理它的返回值或状态变化。
            heap_caps_free(file_buf);
            // 赋值：把右侧 1 的结果保存到左侧 err。
            err = 1;
            // 执行语句：goto done;；它会更新一次变量、对象或程序状态。
            goto done;
        }

        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect vr = viewer_image_rect();
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint16_t max_w = vr.w;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint16_t max_h = vr.h;

        // 调用 Fb::fill(self->viewer_fb_, core::kFbBytes, Fb::kWhite);，执行这个函数并处理它的返回值或状态变化。
        Fb::fill(self->viewer_fb_, core::kFbBytes, Fb::kWhite);

        // 静态查表数组 scales：运行时复用固定数据，避免每次重新计算。
        static const esp_jpeg_image_scale_t scales[] = {
            // 执行这条代码：JPEG_IMAGE_SCALE_0,。它会更新变量、对象状态或调用链。
            JPEG_IMAGE_SCALE_0,
            // 继续上一行的表达式或参数列表：JPEG_IMAGE_SCALE_1_2,。
            JPEG_IMAGE_SCALE_1_2,
            // 继续上一行的表达式或参数列表：JPEG_IMAGE_SCALE_1_4,。
            JPEG_IMAGE_SCALE_1_4,
            // 继续上一行的表达式或参数列表：JPEG_IMAGE_SCALE_1_8,。
            JPEG_IMAGE_SCALE_1_8,
        };

        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint32_t max_rgb_bytes = static_cast<uint32_t>(max_w) * max_h * 2U;

        // 声明并默认初始化对象：esp_jpeg_image_cfg_t cfg{}。
        esp_jpeg_image_cfg_t cfg{};
        // 赋值：把右侧 file_buf 的结果保存到左侧 cfg.indata。
        cfg.indata = file_buf;
        // 赋值：把右侧 file_sz 的结果保存到左侧 cfg.indata_size。
        cfg.indata_size = file_sz;
        // 赋值：把右侧 JPEG_IMAGE_FORMAT_RGB565 的结果保存到左侧 cfg.out_format。
        cfg.out_format = JPEG_IMAGE_FORMAT_RGB565;
        // 赋值：把右侧 1 的结果保存到左侧 cfg.flags.swap_color_bytes。
        cfg.flags.swap_color_bytes = 1;

        // 声明并默认初始化对象：esp_jpeg_image_output_t pick_info{}。
        esp_jpeg_image_output_t pick_info{};
        // 定义变量 bool have_scale 并初始化为 false。
        bool have_scale = false;

        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (size_t si = 0; si < sizeof(scales) / sizeof(scales[0]); ++si)
        {
            // 赋值：把右侧 scales[si] 的结果保存到左侧 cfg.out_scale。
            cfg.out_scale = scales[si];
            // 声明并默认初始化对象：esp_jpeg_image_output_t info{}。
            esp_jpeg_image_output_t info{};
            // 判断条件：esp_jpeg_get_image_info(&cfg, &info) != ESP_OK 是否成立。
            if (esp_jpeg_get_image_info(&cfg, &info) != ESP_OK)
                // 继续下一轮：跳过本次循环剩余代码。
                continue;
            // 判断条件：info.width == 0 || info.height == 0 是否成立。
            if (info.width == 0 || info.height == 0)
                // 继续下一轮：跳过本次循环剩余代码。
                continue;

            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint8_t scale_div = jpeg_scale_div(scales[si]);
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint16_t scaled_w = static_cast<uint16_t>(info.width / scale_div);
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint16_t scaled_h = static_cast<uint16_t>(info.height / scale_div);
            // 判断条件：scaled_w == 0 || scaled_h == 0 是否成立。
            if (scaled_w == 0 || scaled_h == 0)
                // 继续下一轮：跳过本次循环剩余代码。
                continue;
            // 判断条件：scaled_w > max_w || scaled_h > max_h 是否成立。
            if (scaled_w > max_w || scaled_h > max_h)
                // 继续下一轮：跳过本次循环剩余代码。
                continue;
            // 判断条件：info.output_len == 0 || info.output_len > max_rgb_bytes 是否成立。
            if (info.output_len == 0 || info.output_len > max_rgb_bytes)
                // 继续下一轮：跳过本次循环剩余代码。
                continue;

            // 赋值：把右侧 info 的结果保存到左侧 pick_info。
            pick_info = info;
            // 赋值：把右侧 true 的结果保存到左侧 have_scale。
            have_scale = true;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        }

        // 定义变量 bool decoded 并初始化为 false。
        bool decoded = false;
        // 声明并默认初始化对象：esp_jpeg_image_output_t outimg{}。
        esp_jpeg_image_output_t outimg{};
        // 赋值：把右侧 nullptr 的结果保存到左侧 uint8_t* rgb_buf。
        uint8_t* rgb_buf = nullptr;

        // 判断条件：have_scale 是否成立。
        if (have_scale)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint32_t out_cap = static_cast<uint32_t>(pick_info.output_len);
            // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
            rgb_buf = static_cast<uint8_t*>(
                // 继续上一行的表达式或参数列表：heap_caps_malloc(out_cap, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));。
                heap_caps_malloc(out_cap, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
            // 判断条件：rgb_buf == nullptr 是否成立。
            if (rgb_buf == nullptr)
                // 赋值：把右侧 static_cast<uint8_t*>(heap_caps_malloc(out_cap, MALLOC_CAP_8BIT)) 的结果保存到左侧 rgb_buf。
                rgb_buf = static_cast<uint8_t*>(heap_caps_malloc(out_cap, MALLOC_CAP_8BIT));

            // 判断条件：rgb_buf != nullptr 是否成立。
            if (rgb_buf != nullptr)
            {
                // 赋值：把右侧 rgb_buf 的结果保存到左侧 cfg.outbuf。
                cfg.outbuf = rgb_buf;
                // 赋值：把右侧 out_cap 的结果保存到左侧 cfg.outbuf_size。
                cfg.outbuf_size = out_cap;
                // 判断条件：esp_jpeg_decode(&cfg, &outimg 是否成立。
                if (esp_jpeg_decode(&cfg, &outimg) == ESP_OK && outimg.width > 0 &&
                    // 继续上一行的表达式或参数列表：outimg.height > 0)。
                    outimg.height > 0)
                    // 赋值：把右侧 true 的结果保存到左侧 decoded。
                    decoded = true;
            }
        }

        // 判断条件：decoded && rgb_buf != nullptr 是否成立。
        if (decoded && rgb_buf != nullptr)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint32_t gray_sz = static_cast<uint32_t>(outimg.width) * outimg.height;
            // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
            uint8_t* gray_src = static_cast<uint8_t*>(
                // 继续上一行的表达式或参数列表：heap_caps_malloc(gray_sz, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));。
                heap_caps_malloc(gray_sz, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
            // 判断条件：gray_src == nullptr 是否成立。
            if (gray_src == nullptr)
                // 赋值：把右侧 static_cast<uint8_t*>(heap_caps_malloc(gray_sz, MALLOC_CAP_8BIT)) 的结果保存到左侧 gray_src。
                gray_src = static_cast<uint8_t*>(heap_caps_malloc(gray_sz, MALLOC_CAP_8BIT));

            // 判断条件：gray_src != nullptr 是否成立。
            if (gray_src != nullptr)
            {
                // 循环执行：按括号里的初始化、条件和步进规则重复运行。
                for (uint16_t row = 0; row < outimg.height; ++row)
                {
                    // 只读约束：这个值或对象在当前位置不能被修改。
                    const uint16_t* src_row = reinterpret_cast<const uint16_t*>(
                        // 继续上一行的表达式或参数列表：rgb_buf + static_cast<uint32_t>(row) * outimg.width * 2U);。
                        rgb_buf + static_cast<uint32_t>(row) * outimg.width * 2U);
                    // 赋值：把右侧 gray_src + static_cast<uint32_t>(row) * outimg.width 的结果保存到左侧 uint8_t* dst_row。
                    uint8_t* dst_row = gray_src + static_cast<uint32_t>(row) * outimg.width;
                    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
                    for (uint16_t col = 0; col < outimg.width; ++col)
                        // 赋值：把右侧 rgb565_gray(src_row[col]) 的结果保存到左侧 dst_row[col]。
                        dst_row[col] = rgb565_gray(src_row[col]);
                }

                // 定义变量 uint16_t draw_w 并初始化为 0。
                uint16_t draw_w = 0;
                // 定义变量 uint16_t draw_h 并初始化为 0。
                uint16_t draw_h = 0;
                // 调用 fit_draw_size(outimg.width, outimg.height, max_w, max_h, draw_w, draw_h);，执行这个函数并处理它的返回值或状态变化。
                fit_draw_size(outimg.width, outimg.height, max_w, max_h, draw_w, draw_h);

                // 只读约束：这个值或对象在当前位置不能被修改。
                const uint32_t scaled_sz = static_cast<uint32_t>(draw_w) * draw_h;
                // 赋值：把右侧 gray_src 的结果保存到左侧 uint8_t* scaled_gray。
                uint8_t* scaled_gray = gray_src;
                // 判断条件：draw_w != outimg.width || draw_h != outimg.height 是否成立。
                if (draw_w != outimg.width || draw_h != outimg.height)
                {
                    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
                    scaled_gray = static_cast<uint8_t*>(
                        // 继续上一行的表达式或参数列表：heap_caps_malloc(scaled_sz, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));。
                        heap_caps_malloc(scaled_sz, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
                    // 判断条件：scaled_gray == nullptr 是否成立。
                    if (scaled_gray == nullptr)
                        // 执行这条代码：scaled_gray =。它会更新变量、对象状态或调用链。
                        scaled_gray =
                            // 继续上一行的表达式或参数列表：static_cast<uint8_t*>(heap_caps_malloc(scaled_sz, MALLOC_CAP_8BIT));。
                            static_cast<uint8_t*>(heap_caps_malloc(scaled_sz, MALLOC_CAP_8BIT));
                    // 判断条件：scaled_gray != nullptr & 是否成立。
                    if (scaled_gray != nullptr &&
                        // 继续上一行的表达式或参数列表：scale_gray_fit(gray_src, outimg.width, outimg.height, scaled_gray, draw_w,。
                        scale_gray_fit(gray_src, outimg.width, outimg.height, scaled_gray, draw_w,
                                       // 继续上一行的表达式或参数列表：draw_h))。
                                       draw_h))
                    {
                        // 调用 heap_caps_free(gray_src);，执行这个函数并处理它的返回值或状态变化。
                        heap_caps_free(gray_src);
                    }
                    // 备用分支：前面的条件不成立时执行这里。
                    else
                    {
                        // 判断条件：scaled_gray != nullptr && scaled_gray != gray_src 是否成立。
                        if (scaled_gray != nullptr && scaled_gray != gray_src)
                            // 调用 heap_caps_free(scaled_gray);，执行这个函数并处理它的返回值或状态变化。
                            heap_caps_free(scaled_gray);
                        // 赋值：把右侧 gray_src 的结果保存到左侧 scaled_gray。
                        scaled_gray = gray_src;
                        // 赋值：把右侧 outimg.width 的结果保存到左侧 draw_w。
                        draw_w = outimg.width;
                        // 赋值：把右侧 outimg.height 的结果保存到左侧 draw_h。
                        draw_h = outimg.height;
                    }
                }

                // 只读约束：这个值或对象在当前位置不能被修改。
                const int16_t ox = static_cast<int16_t>(vr.x + (max_w - draw_w) / 2U);
                // 只读约束：这个值或对象在当前位置不能被修改。
                const int16_t oy = static_cast<int16_t>(vr.y + (max_h - draw_h) / 2U);
                // 调用 dither_gray_to_fb(scaled_gray, draw_w, draw_h, self->viewer_fb_, ox, oy);，执行这个函数并处理它的返回值或状态变化。
                dither_gray_to_fb(scaled_gray, draw_w, draw_h, self->viewer_fb_, ox, oy);
                // 赋值：把右侧 core::Rect{ox, oy, draw_w, draw_h} 的结果保存到左侧 self->image_rect_。
                self->image_rect_ = core::Rect{ox, oy, draw_w, draw_h};
                // 调用 heap_caps_free(scaled_gray);，执行这个函数并处理它的返回值或状态变化。
                heap_caps_free(scaled_gray);
                // 赋值：把右侧 true 的结果保存到左侧 decoded。
                decoded = true;
            }
            // 备用分支：前面的条件不成立时执行这里。
            else
            {
                // 赋值：把右侧 false 的结果保存到左侧 decoded。
                decoded = false;
            }
            // 调用 heap_caps_free(rgb_buf);，执行这个函数并处理它的返回值或状态变化。
            heap_caps_free(rgb_buf);
            // 赋值：把右侧 nullptr 的结果保存到左侧 rgb_buf。
            rgb_buf = nullptr;
        }

        // 调用 heap_caps_free(file_buf);，执行这个函数并处理它的返回值或状态变化。
        heap_caps_free(file_buf);
        // 判断条件：!decoded 是否成立。
        if (!decoded)
            // 赋值：把右侧 3 的结果保存到左侧 err。
            err = 3;
    }

// 执行这条代码：done:。它会更新变量、对象状态或调用链。
done:
    // 函数调用：(void)ui::UiBus::get_instance().post_system_hint(ui::SystemHintKind::GalleryDecodeDone,；执行该函数并处理它的返回值或副作用。
    (void)ui::UiBus::get_instance().post_system_hint(ui::SystemHintKind::GalleryDecodeDone,
                                                     // 继续上一行的表达式或参数列表：(static_cast<uint32_t>(job_id) << 8) |。
                                                     (static_cast<uint32_t>(job_id) << 8) |
                                                         // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
                                                         static_cast<uint32_t>(err));
}

// 定义或声明函数 GalleryApp::paint_list：参数在括号里，返回类型写在函数名前面。
void GalleryApp::paint_list(gfx::Canvas& c)
{
    // 判断条件：scanning_ 是否成立。
    if (scanning_)
    {
        // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
        gfx::Canvas::TextStyle ts{};
        // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 ts.size_px。
        ts.size_px = Theme::kFontBody;
        // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
        ts.h = gfx::HAlign::Center;
        // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
        ts.v = gfx::VAlign::Middle;
        // 访问对象成员：c.text_in(Theme::list_region(), ui::strings::kGalleryScanning, ts);；读取或修改这个对象里的成员状态。
        c.text_in(Theme::list_region(), ui::strings::kGalleryScanning, ts);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：count_ == 0 是否成立。
    if (count_ == 0)
    {
        // 静态查表数组 kHints：运行时复用固定数据，避免每次重新计算。
        static const char* const kHints[] = {
            // 执行这条代码：GalleryApp::kScanFmtHint,。它会更新变量、对象状态或调用链。
            GalleryApp::kScanFmtHint,
            // 继续上一行的表达式或参数列表：GalleryApp::kScanPathInt,。
            GalleryApp::kScanPathInt,
            // 继续上一行的表达式或参数列表：GalleryApp::kScanPathSd,。
            GalleryApp::kScanPathSd,
        };
        // 调用 ui::widgets::empty_state(c, Theme::list_region(), ui::strings::kGalleryEmpty, kHints, 3);，执行这个函数并处理它的返回值或状态变化。
        ui::widgets::empty_state(c, Theme::list_region(), ui::strings::kGalleryEmpty, kHints, 3);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 访问对象成员：list_.set_total(count_);；读取或修改这个对象里的成员状态。
    list_.set_total(count_);
    // 访问对象成员：list_.paint(c);；读取或修改这个对象里的成员状态。
    list_.paint(c);
}

// 定义或声明函数 GalleryApp::paint_viewer：参数在括号里，返回类型写在函数名前面。
void GalleryApp::paint_viewer(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect vr = viewer_rect();

    // 判断条件：decoding_ 是否成立。
    if (decoding_)
    {
        // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
        gfx::Canvas::TextStyle ts{};
        // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 ts.size_px。
        ts.size_px = Theme::kFontBody;
        // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
        ts.h = gfx::HAlign::Center;
        // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
        ts.v = gfx::VAlign::Middle;
        // 访问对象成员：c.text_in(vr, ui::strings::kGalleryLoading, ts);；读取或修改这个对象里的成员状态。
        c.text_in(vr, ui::strings::kGalleryLoading, ts);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：decode_err_ != 0 || !image_ready_ 是否成立。
    if (decode_err_ != 0 || !image_ready_)
    {
        // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
        gfx::Canvas::TextStyle ts{};
        // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 ts.size_px。
        ts.size_px = Theme::kFontBody;
        // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
        ts.h = gfx::HAlign::Center;
        // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
        ts.v = gfx::VAlign::Middle;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* msg = ui::strings::kGalleryOpenFail;
        // 判断条件：decode_err_ == 2 是否成立。
        if (decode_err_ == 2)
            // 赋值：把右侧 ui::strings::kGalleryTooLarge 的结果保存到左侧 msg。
            msg = ui::strings::kGalleryTooLarge;
        // 备用分支：前面的条件不成立时执行这里。
        else if (decode_err_ == 4)
            // 赋值：把右侧 ui::strings::kGalleryNoMem 的结果保存到左侧 msg。
            msg = ui::strings::kGalleryNoMem;
        // 调用 text_in(vr,msg,ts)：文本对齐。
        c.text_in(vr, msg, ts);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：viewer_fb_ != nullptr && !image_rect_.empty() 是否成立。
    if (viewer_fb_ != nullptr && !image_rect_.empty())
        // 调用 Fb::blit_rect(c.fb(), viewer_fb_, image_rect_.x, image_rect_.y, image_rect_.w,，执行这个函数并处理它的返回值或状态变化。
        Fb::blit_rect(c.fb(), viewer_fb_, image_rect_.x, image_rect_.y, image_rect_.w,
                      // 继续上一行的表达式或参数列表：image_rect_.h, core::kStride);。
                      image_rect_.h, core::kStride);

    // 判断条件：count_ > 1 是否成立。
    if (count_ > 1)
    {
        // 执行语句：char hint[24];；它会更新一次变量、对象或程序状态。
        char hint[24];
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        (void)std::snprintf(hint, sizeof(hint), "%u / %u", static_cast<unsigned>(sel_ + 1U),
                            // 继续上一行的表达式或参数列表：static_cast<unsigned>(count_));。
                            static_cast<unsigned>(count_));
        // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
        gfx::Canvas::TextStyle ts{};
        // 赋值：把右侧 Theme::kFontSmall 的结果保存到左侧 ts.size_px。
        ts.size_px = Theme::kFontSmall;
        // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
        ts.h = gfx::HAlign::Center;
        // 赋值：把右侧 gfx::VAlign::Bottom 的结果保存到左侧 ts.v。
        ts.v = gfx::VAlign::Bottom;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect hint_r{vr.x, static_cast<int16_t>(vr.bottom() - 16), vr.w, 14};
        // 调用 text_in(hint_r,hint,ts)：文本对齐。
        c.text_in(hint_r, hint, ts);
    }
}

// 定义或声明函数 GalleryApp::paint：参数在括号里，返回类型写在函数名前面。
void GalleryApp::paint(gfx::Canvas& canvas)
{
    // 调用 title()：返回 App 标题。在文件浏览器内，标题有时会动态反映当前目录，因此可能存在一个 buffer，而不是只返回一个常量字符串。。
    ui::widgets::toolbar(canvas, title());
    // 判断条件：view_ == View::List 是否成立。
    if (view_ == View::List)
        // 调用 paint_list(canvas);，执行这个函数并处理它的返回值或状态变化。
        paint_list(canvas);
    // 备用分支：前面的条件不成立时执行这里。
    else
        // 调用 paint_viewer(canvas);，执行这个函数并处理它的返回值或状态变化。
        paint_viewer(canvas);
}

// 定义或声明函数 GalleryApp::on_input：参数在括号里，返回类型写在函数名前面。
shell::InputResult GalleryApp::on_input(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x = static_cast<int16_t>(ev.x);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y = static_cast<int16_t>(ev.y);

    // 判断条件：view_ == View::List 是否成立。
    if (view_ == View::List)
    {
        // 判断条件：ev.type == EventType::Tap && ui::widgets::hit_toolbar_back(x, y) 是否成立。
        if (ev.type == EventType::Tap && ui::widgets::hit_toolbar_back(x, y))
            // 返回 {} 给调用者，表示当前函数的执行结果。
            return {};

        // 只读约束：这个值或对象在当前位置不能被修改。
        const auto out = list_.handle_input(ev);
        // 判断条件：out.scroll_changed 是否成立。
        if (out.scroll_changed)
            // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
            request_repaint();
        // 判断条件：out.consumed 是否成立。
        if (out.consumed)
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};
    }

    // 判断条件：ev.type == EventType::Tap && ui::widgets::hit_toolbar_back(x, y) 是否成立。
    if (ev.type == EventType::Tap && ui::widgets::hit_toolbar_back(x, y))
    {
        // 调用 cancel_decode();，执行这个函数并处理它的返回值或状态变化。
        cancel_decode();
        // 赋值：把右侧 View::List 的结果保存到左侧 view_。
        view_ = View::List;
        // 赋值：把右侧 false 的结果保存到左侧 image_ready_。
        image_ready_ = false;
        // 赋值：把右侧 0 的结果保存到左侧 decode_err_。
        decode_err_ = 0;
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 判断条件：ev.type == EventType::SwipeLeft 是否成立。
    if (ev.type == EventType::SwipeLeft)
    {
        // 调用 show_next();，执行这个函数并处理它的返回值或状态变化。
        show_next();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：ev.type == EventType::SwipeRight 是否成立。
    if (ev.type == EventType::SwipeRight)
    {
        // 调用 show_prev();，执行这个函数并处理它的返回值或状态变化。
        show_prev();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：ev.type == EventType::Tap 是否成立。
    if (ev.type == EventType::Tap)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect vr = viewer_rect();
        // 判断条件：!vr.contains(x, y) 是否成立。
        if (!vr.contains(x, y))
            // 返回 {} 给调用者，表示当前函数的执行结果。
            return {};

        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t mid = static_cast<int16_t>(vr.x + vr.w / 2);
        // 判断条件：x < mid 是否成立。
        if (x < mid)
            // 调用 show_prev();，执行这个函数并处理它的返回值或状态变化。
            show_prev();
        // 备用分支：前面的条件不成立时执行这里。
        else
            // 调用 show_next();，执行这个函数并处理它的返回值或状态变化。
            show_next();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 返回 {} 给调用者，表示当前函数的执行结果。
    return {};
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::gallery
