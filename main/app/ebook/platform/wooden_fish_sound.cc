/**
 * [初学者导读] 文件：main/app/ebook/platform/wooden_fish_sound.cc
 * 作用：加载并播放电子木鱼音效。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：阻塞工作放独立任务，周期小检查放 Housekeeper。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "platform/wooden_fish_sound.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_heap_caps.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/system_state.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "platform/audio_codec.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "platform/music_player.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/task/task.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "FishSound";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::platform {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 固定参数 kKnockPath = "/assets/audio/woden_fish.wav"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kKnockPath = "/assets/audio/woden_fish.wav";
// 固定参数 kDstRate = 16000：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t kDstRate = 16000;
// 任务栈大小：4096，栈太小会崩溃，太大会浪费 RAM。
constexpr size_t kPlayStack = 4096;
// 缓冲区或分块大小：512，影响单次读写占用和性能。
constexpr size_t kWriteChunk = 512;
// 固定参数 kCapsPsram = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t kCapsPsram = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT;
// 固定参数 kOutputOffPadMs = 25：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t kOutputOffPadMs = 25;

// struct WavInfo：把相关数据字段组合在一起，表示项目中的一个概念。
struct WavInfo
{
    // 执行语句：uint32_t rate{0};；它会更新一次变量、对象或程序状态。
    uint32_t rate{0};
    // 执行语句：uint8_t channels{0};；它会更新一次变量、对象或程序状态。
    uint8_t channels{0};
    // 执行语句：uint16_t bits{0};；它会更新一次变量、对象或程序状态。
    uint16_t bits{0};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const void* data{nullptr};
    // 执行语句：size_t data_bytes{0};；它会更新一次变量、对象或程序状态。
    size_t data_bytes{0};
};

// 定义或声明函数 read_u32_le：参数在括号里，返回类型写在函数名前面。
bool read_u32_le(const uint8_t* p, uint32_t* out)
{
    // 判断条件：p == nullptr || out == nullptr 是否成立。
    if (p == nullptr || out == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    *out = static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
           // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
           (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 read_u16_le：参数在括号里，返回类型写在函数名前面。
bool read_u16_le(const uint8_t* p, uint16_t* out)
{
    // 判断条件：p == nullptr || out == nullptr 是否成立。
    if (p == nullptr || out == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    *out = static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 parse_wav_pcm：参数在括号里，返回类型写在函数名前面。
bool parse_wav_pcm(const uint8_t* buf, size_t len, WavInfo* info)
{
    // 判断条件：buf == nullptr || info == nullptr || len < 44 是否成立。
    if (buf == nullptr || info == nullptr || len < 44)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：std::memcmp(buf, "RIFF", 4) != 0 || std::memcmp(buf + 8, "WAVE", 4) != 0 是否成立。
    if (std::memcmp(buf, "RIFF", 4) != 0 || std::memcmp(buf + 8, "WAVE", 4) != 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 定义变量 size_t pos 并初始化为 12。
    size_t pos = 12;
    // 定义变量 bool got_fmt 并初始化为 false。
    bool got_fmt = false;
    // 定义变量 bool got_data 并初始化为 false。
    bool got_data = false;

    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (pos + 8 <= len)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t* ch = buf + pos;
        // 定义变量 uint32_t chunk_sz 并初始化为 0。
        uint32_t chunk_sz = 0;
        // 判断条件：!read_u32_le(ch + 4, &chunk_sz) 是否成立。
        if (!read_u32_le(ch + 4, &chunk_sz))
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        // 赋值：把右侧 8 的结果保存到左侧 pos +。
        pos += 8;
        // 判断条件：pos + chunk_sz > len 是否成立。
        if (pos + chunk_sz > len)
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;

        // 判断条件：std::memcmp(ch, "fmt ", 4) == 0 && chunk_sz >= 16 是否成立。
        if (std::memcmp(ch, "fmt ", 4) == 0 && chunk_sz >= 16)
        {
            // 定义变量 uint16_t audio_fmt 并初始化为 0。
            uint16_t audio_fmt = 0;
            // 定义变量 uint16_t channels 并初始化为 0。
            uint16_t channels = 0;
            // 判断条件：!read_u16_le(buf + pos, &audio_fmt) || audio_fmt != 1 是否成立。
            if (!read_u16_le(buf + pos, &audio_fmt) || audio_fmt != 1)
                // 返回 false 给调用者，表示当前函数的执行结果。
                return false;
            // 判断条件：!read_u16_le(buf + pos + 2, &channels) 是否成立。
            if (!read_u16_le(buf + pos + 2, &channels))
                // 返回 false 给调用者，表示当前函数的执行结果。
                return false;
            // 赋值：把右侧 static_cast<uint8_t>(channels) 的结果保存到左侧 info->channels。
            info->channels = static_cast<uint8_t>(channels);
            // 判断条件：!read_u32_le(buf + pos + 4, &info->rate) 是否成立。
            if (!read_u32_le(buf + pos + 4, &info->rate))
                // 返回 false 给调用者，表示当前函数的执行结果。
                return false;
            // 判断条件：!read_u16_le(buf + pos + 14, &info->bits) 是否成立。
            if (!read_u16_le(buf + pos + 14, &info->bits))
                // 返回 false 给调用者，表示当前函数的执行结果。
                return false;
            // 判断条件：info->bits != 16 || info->channels == 0 || info->rate == 0 是否成立。
            if (info->bits != 16 || info->channels == 0 || info->rate == 0)
                // 返回 false 给调用者，表示当前函数的执行结果。
                return false;
            // 赋值：把右侧 true 的结果保存到左侧 got_fmt。
            got_fmt = true;
        }
        // 备用分支：前面的条件不成立时执行这里。
        else if (std::memcmp(ch, "data", 4) == 0)
        {
            // 赋值：把右侧 buf + pos 的结果保存到左侧 info->data。
            info->data = buf + pos;
            // 赋值：把右侧 chunk_sz 的结果保存到左侧 info->data_bytes。
            info->data_bytes = chunk_sz;
            // 赋值：把右侧 true 的结果保存到左侧 got_data。
            got_data = true;
        }

        // 赋值：把右侧 chunk_sz + (chunk_sz & 1U) 的结果保存到左侧 pos +。
        pos += chunk_sz + (chunk_sz & 1U);
        // 判断条件：got_fmt && got_data 是否成立。
        if (got_fmt && got_data)
            // 返回 true 给调用者，表示当前函数的执行结果。
            return true;
    }
    // 返回 false 给调用者，表示当前函数的执行结果。
    return false;
}

// 函数调用：int16_t* mono_from_interleaved(const int16_t* src, size_t frame_count, uint8_t channels,；执行该函数并处理它的返回值或副作用。
int16_t* mono_from_interleaved(const int16_t* src, size_t frame_count, uint8_t channels,
                               // 继续上一行的表达式或参数列表：size_t* out_count)。
                               size_t* out_count)
{
    // 判断条件：src == nullptr || frame_count == 0 || channels == 0 || out_count == nullptr 是否成立。
    if (src == nullptr || frame_count == 0 || channels == 0 || out_count == nullptr)
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;

    *out_count = frame_count;
    // 判断条件：channels == 1 是否成立。
    if (channels == 1)
    {
        // 执行这条代码：auto* dst =。它会更新变量、对象状态或调用链。
        auto* dst =
            // 继续上一行的表达式或参数列表：static_cast<int16_t*>(heap_caps_malloc(frame_count * sizeof(int16_t), kCapsPsram));。
            static_cast<int16_t*>(heap_caps_malloc(frame_count * sizeof(int16_t), kCapsPsram));
        // 判断条件：dst == nullptr 是否成立。
        if (dst == nullptr)
            // 返回 nullptr 给调用者，表示当前函数的执行结果。
            return nullptr;
        // 调用 std::memcpy(dst, src, frame_count * sizeof(int16_t));，执行这个函数并处理它的返回值或状态变化。
        std::memcpy(dst, src, frame_count * sizeof(int16_t));
        // 返回 dst 给调用者，表示当前函数的执行结果。
        return dst;
    }

    // 赋值：把右侧 static_cast<int16_t*>(heap_caps_malloc(frame_count * sizeof(int16_t), kCapsPsram)) 的结果保存到左侧 auto* dst。
    auto* dst = static_cast<int16_t*>(heap_caps_malloc(frame_count * sizeof(int16_t), kCapsPsram));
    // 判断条件：dst == nullptr 是否成立。
    if (dst == nullptr)
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (size_t i = 0; i < frame_count; ++i)
    {
        // 定义变量 int32_t sum 并初始化为 0。
        int32_t sum = 0;
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint8_t ch = 0; ch < channels; ++ch)
            // 赋值：把右侧 src[i * channels + ch] 的结果保存到左侧 sum +。
            sum += src[i * channels + ch];
        // 赋值：把右侧 static_cast<int16_t>(sum / static_cast<int32_t>(channels)) 的结果保存到左侧 dst[i]。
        dst[i] = static_cast<int16_t>(sum / static_cast<int32_t>(channels));
    }
    // 返回 dst 给调用者，表示当前函数的执行结果。
    return dst;
}

// 函数调用：int16_t* resample_linear(const int16_t* src, size_t src_count, uint32_t src_rate, uint32_t dst_rate,；执行该函数并处理它的返回值或副作用。
int16_t* resample_linear(const int16_t* src, size_t src_count, uint32_t src_rate, uint32_t dst_rate,
                         // 继续上一行的表达式或参数列表：size_t* out_count)。
                         size_t* out_count)
{
    // 判断条件：src == nullptr || src_count == 0 || src_rate == 0 || dst_rate == 0 || out_count == nullptr 是否成立。
    if (src == nullptr || src_count == 0 || src_rate == 0 || dst_rate == 0 || out_count == nullptr)
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;

    // 判断条件：src_rate == dst_rate 是否成立。
    if (src_rate == dst_rate)
    {
        // 执行这条代码：auto* dst =。它会更新变量、对象状态或调用链。
        auto* dst =
            // 继续上一行的表达式或参数列表：static_cast<int16_t*>(heap_caps_malloc(src_count * sizeof(int16_t), kCapsPsram));。
            static_cast<int16_t*>(heap_caps_malloc(src_count * sizeof(int16_t), kCapsPsram));
        // 判断条件：dst == nullptr 是否成立。
        if (dst == nullptr)
            // 返回 nullptr 给调用者，表示当前函数的执行结果。
            return nullptr;
        // 调用 std::memcpy(dst, src, src_count * sizeof(int16_t));，执行这个函数并处理它的返回值或状态变化。
        std::memcpy(dst, src, src_count * sizeof(int16_t));
        *out_count = src_count;
        // 返回 dst 给调用者，表示当前函数的执行结果。
        return dst;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t dst_count =
        // 继续上一行的表达式或参数列表：static_cast<size_t>((static_cast<uint64_t>(src_count) * dst_rate) / src_rate);。
        static_cast<size_t>((static_cast<uint64_t>(src_count) * dst_rate) / src_rate);
    // 判断条件：dst_count == 0 是否成立。
    if (dst_count == 0)
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;

    // 赋值：把右侧 static_cast<int16_t*>(heap_caps_malloc(dst_count * sizeof(int16_t), kCapsPsram)) 的结果保存到左侧 auto* dst。
    auto* dst = static_cast<int16_t*>(heap_caps_malloc(dst_count * sizeof(int16_t), kCapsPsram));
    // 判断条件：dst == nullptr 是否成立。
    if (dst == nullptr)
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (size_t i = 0; i < dst_count; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const float pos =
            // 继续上一行的表达式或参数列表：static_cast<float>(i) * static_cast<float>(src_rate) / static_cast<float>(dst_rate);。
            static_cast<float>(i) * static_cast<float>(src_rate) / static_cast<float>(dst_rate);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const size_t idx = static_cast<size_t>(pos);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const float frac = pos - static_cast<float>(idx);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t a = src[idx];
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t b = src[(idx + 1 < src_count) ? (idx + 1) : idx];
        // 赋值：把右侧 static_cast<int16_t>(static_cast<float>(a) + frac * static_cast<float>(b - a)) 的结果保存到左侧 dst[i]。
        dst[i] = static_cast<int16_t>(static_cast<float>(a) + frac * static_cast<float>(b - a));
    }

    *out_count = dst_count;
    // 返回 dst 给调用者，表示当前函数的执行结果。
    return dst;
}

// 定义或声明函数 load_knock_pcm：参数在括号里，返回类型写在函数名前面。
bool load_knock_pcm(int16_t** out_pcm, size_t* out_count, uint32_t* out_rate)
{
    // 判断条件：out_pcm == nullptr || out_count == nullptr || out_rate == nullptr 是否成立。
    if (out_pcm == nullptr || out_count == nullptr || out_rate == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 std::fopen(kKnockPath, "rb") 的结果保存到左侧 FILE* fp。
    FILE* fp = std::fopen(kKnockPath, "rb");
    // 判断条件：fp == nullptr 是否成立。
    if (fp == nullptr)
    {
        // 调用 EBOOK_LOGE(TAG, "fopen fail %s", kKnockPath);，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGE(TAG, "fopen fail %s", kKnockPath);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 判断条件：std::fseek(fp, 0, SEEK_END) != 0 是否成立。
    if (std::fseek(fp, 0, SEEK_END) != 0)
    {
        // 调用 std::fclose(fp);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(fp);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 只读约束：这个值或对象在当前位置不能被修改。
    const long file_sz = std::ftell(fp);
    // 判断条件：file_sz <= 0 是否成立。
    if (file_sz <= 0)
    {
        // 调用 std::fclose(fp);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(fp);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 判断条件：std::fseek(fp, 0, SEEK_SET) != 0 是否成立。
    if (std::fseek(fp, 0, SEEK_SET) != 0)
    {
        // 调用 std::fclose(fp);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(fp);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 执行这条代码：auto* file_buf =。它会更新变量、对象状态或调用链。
    auto* file_buf =
        // 继续上一行的表达式或参数列表：static_cast<uint8_t*>(heap_caps_malloc(static_cast<size_t>(file_sz), kCapsPsram));。
        static_cast<uint8_t*>(heap_caps_malloc(static_cast<size_t>(file_sz), kCapsPsram));
    // 判断条件：file_buf == nullptr 是否成立。
    if (file_buf == nullptr)
    {
        // 调用 std::fclose(fp);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(fp);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t rd = std::fread(file_buf, 1, static_cast<size_t>(file_sz), fp);
    // 调用 std::fclose(fp);，执行这个函数并处理它的返回值或状态变化。
    std::fclose(fp);
    // 判断条件：rd != static_cast<size_t>(file_sz) 是否成立。
    if (rd != static_cast<size_t>(file_sz))
    {
        // 调用 heap_caps_free(file_buf);，执行这个函数并处理它的返回值或状态变化。
        heap_caps_free(file_buf);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 声明并默认初始化对象：WavInfo info{}。
    WavInfo info{};
    // 判断条件：!parse_wav_pcm(file_buf, static_cast<size_t>(file_sz), &info) 是否成立。
    if (!parse_wav_pcm(file_buf, static_cast<size_t>(file_sz), &info))
    {
        // 调用 heap_caps_free(file_buf);，执行这个函数并处理它的返回值或状态变化。
        heap_caps_free(file_buf);
        // 调用 EBOOK_LOGE(TAG, "wav parse fail");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGE(TAG, "wav parse fail");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t frame_count =
        // 继续上一行的表达式或参数列表：info.data_bytes / (static_cast<size_t>(info.bits / 8U) * info.channels);。
        info.data_bytes / (static_cast<size_t>(info.bits / 8U) * info.channels);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto* src = static_cast<const int16_t*>(info.data);

    // 定义变量 size_t mono_count 并初始化为 0。
    size_t mono_count = 0;
    // 赋值：把右侧 mono_from_interleaved(src, frame_count, info.channels, &mono_count) 的结果保存到左侧 int16_t* mono。
    int16_t* mono = mono_from_interleaved(src, frame_count, info.channels, &mono_count);
    // 调用 heap_caps_free(file_buf);，执行这个函数并处理它的返回值或状态变化。
    heap_caps_free(file_buf);
    // 判断条件：mono == nullptr 是否成立。
    if (mono == nullptr)
    {
        // 调用 EBOOK_LOGE(TAG, "mono alloc fail");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGE(TAG, "mono alloc fail");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 定义变量 size_t dst_count 并初始化为 0。
    size_t dst_count = 0;
    // 赋值：把右侧 resample_linear(mono, mono_count, info.rate, kDstRate, &dst_count) 的结果保存到左侧 int16_t* dst。
    int16_t* dst = resample_linear(mono, mono_count, info.rate, kDstRate, &dst_count);
    // 调用 heap_caps_free(mono);，执行这个函数并处理它的返回值或状态变化。
    heap_caps_free(mono);
    // 判断条件：dst == nullptr || dst_count == 0 是否成立。
    if (dst == nullptr || dst_count == 0)
    {
        // 调用 EBOOK_LOGE(TAG, "resample fail");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGE(TAG, "resample fail");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    *out_pcm = dst;
    *out_count = dst_count;
    *out_rate = kDstRate;
    // 调用 EBOOK_LOGD(TAG, "knock ready %u Hz %u samples", static_cast<unsigned>(kDstRate),，执行这个函数并处理它的返回值或状态变化。
    EBOOK_LOGD(TAG, "knock ready %u Hz %u samples", static_cast<unsigned>(kDstRate),
               // 继续上一行的表达式或参数列表：static_cast<unsigned>(dst_count));。
               static_cast<unsigned>(dst_count));
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 WoodenFishSound::get_instance：参数在括号里，返回类型写在函数名前面。
WoodenFishSound& WoodenFishSound::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static WoodenFishSound s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义或声明函数 WoodenFishSound::init：参数在括号里，返回类型写在函数名前面。
bool WoodenFishSound::init()
{
    // 判断条件：ready_ 是否成立。
    if (ready_)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    // 判断条件：!AudioCodec::get_instance().ready() 是否成立。
    if (!AudioCodec::get_instance().ready())
    {
        // 调用 EBOOK_LOGW(TAG, "codec not ready");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "codec not ready");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 nullptr 的结果保存到左侧 int16_t* pcm。
    int16_t* pcm = nullptr;
    // 定义变量 size_t count 并初始化为 0。
    size_t count = 0;
    // 定义变量 uint32_t rate 并初始化为 0。
    uint32_t rate = 0;
    // 判断条件：!load_knock_pcm(&pcm, &count, &rate) 是否成立。
    if (!load_knock_pcm(&pcm, &count, &rate))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 pcm 的结果保存到左侧 pcm_。
    pcm_ = pcm;
    // 赋值：把右侧 count 的结果保存到左侧 pcm_count_。
    pcm_count_ = count;
    // 赋值：把右侧 rate 的结果保存到左侧 dst_rate_。
    dst_rate_ = rate;
    // 赋值：把右侧 true 的结果保存到左侧 ready_。
    ready_ = true;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 WoodenFishSound::clear_play_task：参数在括号里，返回类型写在函数名前面。
void WoodenFishSound::clear_play_task()
{
    // 判断条件：play_task_ == nullptr 是否成立。
    if (play_task_ == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：play_task_alive_ 是否成立。
    if (play_task_alive_)
        // 调用 destroy()：直接删除，不是优雅停止。
        play_task_->destroy();
    // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    play_task_.reset();
}

// 定义或声明函数 WoodenFishSound::run_play：参数在括号里，返回类型写在函数名前面。
void WoodenFishSound::run_play()
{
    // 赋值：把右侧 true 的结果保存到左侧 play_task_alive_。
    play_task_alive_ = true;

    // 开始代码块：下面是这个函数、分支或循环的主体。
    auto finish = [this]() {
        // 赋值：把右侧 false 的结果保存到左侧 play_task_alive_。
        play_task_alive_ = false;
    };

    // 判断条件：!ready_ || pcm_ == nullptr || pcm_count_ == 0 是否成立。
    if (!ready_ || pcm_ == nullptr || pcm_count_ == 0)
    {
        // 调用 finish()：update() 把一段流数据加入摘要；finish() 输出最终 16 字节。下载回调每收到一段数据就调用一次 update。。
        finish();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：data::SystemState::get_instance().mute() 是否成立。
    if (data::SystemState::get_instance().mute())
    {
        // 调用 finish()：update() 把一段流数据加入摘要；finish() 输出最终 16 字节。下载回调每收到一段数据就调用一次 update。。
        finish();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：!AudioCodec::get_instance().ready() 是否成立。
    if (!AudioCodec::get_instance().ready())
    {
        // 调用 finish()：update() 把一段流数据加入摘要；finish() 输出最终 16 字节。下载回调每收到一段数据就调用一次 update。。
        finish();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：!AudioCodec::get_instance().ensure_sample_rate(dst_rate_) 是否成立。
    if (!AudioCodec::get_instance().ensure_sample_rate(dst_rate_))
    {
        // 调用 finish()：update() 把一段流数据加入摘要；finish() 输出最终 16 字节。下载回调每收到一段数据就调用一次 update。。
        finish();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 赋值：把右侧 AudioCodec::get_instance().codec() 的结果保存到左侧 auto& codec。
    auto& codec = AudioCodec::get_instance().codec();
    // 调用 AudioCodec::get_instance().apply_system_volume();，执行这个函数并处理它的返回值或状态变化。
    AudioCodec::get_instance().apply_system_volume();
    // 访问对象成员：codec.enable_output(true);；读取或修改这个对象里的成员状态。
    codec.enable_output(true);

    // 定义变量 size_t offset 并初始化为 0。
    size_t offset = 0;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t total_bytes = pcm_count_ * sizeof(int16_t);
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (offset < total_bytes)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const size_t chunk =
            // 继续上一行的表达式或参数列表：(total_bytes - offset > kWriteChunk) ? kWriteChunk : (total_bytes - offset);。
            (total_bytes - offset > kWriteChunk) ? kWriteChunk : (total_bytes - offset);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const auto* ptr =
            // 继续上一行的表达式或参数列表：reinterpret_cast<const int16_t*>(reinterpret_cast<const uint8_t*>(pcm_) + offset);。
            reinterpret_cast<const int16_t*>(reinterpret_cast<const uint8_t*>(pcm_) + offset);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int samples = static_cast<int>(chunk / sizeof(int16_t));
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int wr = codec.write(ptr, samples);
        // 判断条件：wr <= 0 是否成立。
        if (wr <= 0)
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 赋值：把右侧 static_cast<size_t>(wr) * sizeof(int16_t) 的结果保存到左侧 offset +。
        offset += static_cast<size_t>(wr) * sizeof(int16_t);
    }

    // 调用 app::sys::task::TaskMgr::delay_ms(kOutputOffPadMs)：调用 vTaskDelay()，这是让出 CPU 的阻塞延时。一千毫秒延时期间，任务不占 CPU，调度器可以运行别人。。
    ::app::sys::task::TaskMgr::delay_ms(kOutputOffPadMs);
    // 访问对象成员：codec.enable_output(false);；读取或修改这个对象里的成员状态。
    codec.enable_output(false);
    // 调用 finish()：update() 把一段流数据加入摘要；finish() 输出最终 16 字节。下载回调每收到一段数据就调用一次 update。。
    finish();
}

// 定义或声明函数 WoodenFishSound::play_knock：参数在括号里，返回类型写在函数名前面。
bool WoodenFishSound::play_knock()
{
    // 判断条件：!ready_ || pcm_ == nullptr || pcm_count_ == 0 是否成立。
    if (!ready_ || pcm_ == nullptr || pcm_count_ == 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：data::SystemState::get_instance().mute() 是否成立。
    if (data::SystemState::get_instance().mute())
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：!AudioCodec::get_instance().ready() 是否成立。
    if (!AudioCodec::get_instance().ready())
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：MusicPlayer::get_instance().state() == MusicPlayState::Playing 是否成立。
    if (MusicPlayer::get_instance().state() == MusicPlayState::Playing)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：play_task_alive_ 是否成立。
    if (play_task_alive_)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 调用 clear_play_task();，执行这个函数并处理它的返回值或状态变化。
    clear_play_task();

    // 定义变量：::app::sys::task::Cfg cfg =
    ::app::sys::task::Cfg cfg =
        // 继续上一行的表达式或参数列表：::app::sys::task::Cfg::light("fish_snd", ::app::sys::task::Priority::LOW);。
        ::app::sys::task::Cfg::light("fish_snd", ::app::sys::task::Priority::LOW);
    // 赋值：把右侧 kPlayStack 的结果保存到左侧 cfg.stack_size。
    cfg.stack_size = kPlayStack;
    // 赋值：把右侧 false 的结果保存到左侧 cfg.use_psram。
    cfg.use_psram = false;

    // 执行这条代码：play_task_ = std::make_unique<::app::sys::task::Task>(。它会更新变量、对象状态或调用链。
    play_task_ = std::make_unique<::app::sys::task::Task>(
        // 开始代码块：下面是这个函数、分支或循环的主体。
        [](void* arg) {
            // 赋值：把右侧 static_cast<WoodenFishSound*>(arg) 的结果保存到左侧 auto* self。
            auto* self = static_cast<WoodenFishSound*>(arg);
            // 判断条件：self != nullptr 是否成立。
            if (self != nullptr)
                // 访问对象成员：self->run_play();；读取或修改这个对象里的成员状态。
                self->run_play();
        // 结束上一行的 lambda 或初始化列表：把这段匿名代码/数据交给调用者。
        },
        // 继续上一行的表达式或参数列表：cfg, this);。
        cfg, this);

    // 判断条件：!play_task_->start() 是否成立。
    if (!play_task_->start())
    {
        // 调用 clear_play_task();，执行这个函数并处理它的返回值或状态变化。
        clear_play_task();
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::platform
