/**
 * [初学者导读] 文件：main/app/ebook/apps/music/music_library.cc
 * 作用：扫描音乐目录并维护歌曲元数据。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/music/music_library.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cctype>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <dirent.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <sys/stat.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_heap_caps.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "memory/psram_alloc.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "storage/storage.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "text/path_encoding.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::music {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using app::common::memory::alloc_array_psram;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using app::common::memory::calloc_psram_prefer;
// 固定参数 kDirInt = MusicLibrary::kScanPathInt：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kDirInt = MusicLibrary::kScanPathInt;
// 固定参数 kDirSd = MusicLibrary::kScanPathSd：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kDirSd = MusicLibrary::kScanPathSd;

// 定义或声明函数 ext_lower：参数在括号里，返回类型写在函数名前面。
bool ext_lower(const char* name, char* out, size_t cap)
{
    // 判断条件：name == nullptr || out == nullptr || cap == 0 是否成立。
    if (name == nullptr || out == nullptr || cap == 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* dot = std::strrchr(name, '.');
    // 判断条件：dot == nullptr 是否成立。
    if (dot == nullptr)
    {
        // 赋值：把右侧 '\0' 的结果保存到左侧 out[0]。
        out[0] = '\0';
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 执行语句：++dot;；它会更新一次变量、对象或程序状态。
    ++dot;
    // 定义变量 size_t i 并初始化为 0。
    size_t i = 0;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (; dot[i] != '\0' && i < cap - 1; ++i)
        // 赋值：把右侧 static_cast<char>(std::tolower(static_cast<unsigned char>(dot[i]))) 的结果保存到左侧 out[i]。
        out[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(dot[i])));
    // 赋值：把右侧 '\0' 的结果保存到左侧 out[i]。
    out[i] = '\0';
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 is_audio：参数在括号里，返回类型写在函数名前面。
bool is_audio(const char* name)
{
    // 执行语句：char ext[8]{};；它会更新一次变量、对象或程序状态。
    char ext[8]{};
    // 判断条件：!ext_lower(name, ext, sizeof(ext)) 是否成立。
    if (!ext_lower(name, ext, sizeof(ext)))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 返回 (std::strcmp(ext, "mp3") == 0 || std::strcmp(ext, "wav") == 0) 给调用者，表示当前函数的执行结果。
    return (std::strcmp(ext, "mp3") == 0 || std::strcmp(ext, "wav") == 0);
}

// 定义或声明函数 strip_ext：参数在括号里，返回类型写在函数名前面。
void strip_ext(const char* name, char* out, size_t cap)
{
    // 判断条件：name == nullptr || out == nullptr || cap == 0 是否成立。
    if (name == nullptr || out == nullptr || cap == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 std::strncpy(out, name, cap - 1);，执行这个函数并处理它的返回值或状态变化。
    std::strncpy(out, name, cap - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 out[cap - 1]。
    out[cap - 1] = '\0';
    // 赋值：把右侧 std::strrchr(out, '.') 的结果保存到左侧 char* dot。
    char* dot = std::strrchr(out, '.');
    // 判断条件：dot != nullptr 是否成立。
    if (dot != nullptr)
        *dot = '\0';
}

// 定义或声明函数 read_le32：参数在括号里，返回类型写在函数名前面。
uint32_t read_le32(const uint8_t* p)
{
    // 返回 static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) | 给调用者，表示当前函数的执行结果。
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
           // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
           (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}
// 定义或声明函数 read_le16：参数在括号里，返回类型写在函数名前面。
uint16_t read_le16(const uint8_t* p)
{
    // 返回 static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8) 给调用者，表示当前函数的执行结果。
    return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 MusicLibrary::get_instance：参数在括号里，返回类型写在函数名前面。
MusicLibrary& MusicLibrary::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static MusicLibrary s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义或声明函数 MusicLibrary::ensure_tracks：参数在括号里，返回类型写在函数名前面。
bool MusicLibrary::ensure_tracks()
{
    // 判断条件：tracks_ != nullptr 是否成立。
    if (tracks_ != nullptr)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    // 赋值：把右侧 alloc_array_psram<MusicTrack>(kMaxTracks) 的结果保存到左侧 tracks_。
    tracks_ = alloc_array_psram<MusicTrack>(kMaxTracks);
    // 返回 tracks_ != nullptr 给调用者，表示当前函数的执行结果。
    return tracks_ != nullptr;
}

// 只读约束：这个值或对象在当前位置不能被修改。
const MusicTrack& MusicLibrary::at(uint8_t idx) const
{
    // 静态空对象 kEmpty：作为默认返回值或占位值，避免返回悬空引用。
    static const MusicTrack kEmpty{};
    // 判断条件：tracks_ == nullptr || idx >= count_ 是否成立。
    if (tracks_ == nullptr || idx >= count_)
        // 返回 kEmpty 给调用者，表示当前函数的执行结果。
        return kEmpty;
    // 返回 tracks_[idx] 给调用者，表示当前函数的执行结果。
    return tracks_[idx];
}

// 定义或声明函数 MusicLibrary::parse_wav_duration：参数在括号里，返回类型写在函数名前面。
bool MusicLibrary::parse_wav_duration(const char* path, uint16_t& sec_out) const
{
    // 赋值：把右侧 0 的结果保存到左侧 sec_out。
    sec_out = 0;
    // 赋值：把右侧 std::fopen(path, "rb") 的结果保存到左侧 FILE* f。
    FILE* f = std::fopen(path, "rb");
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 执行语句：uint8_t hdr[128];；它会更新一次变量、对象或程序状态。
    uint8_t hdr[128];
    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t n = std::fread(hdr, 1, sizeof(hdr), f);
    // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
    std::fclose(f);
    // 判断条件：n < 44 是否成立。
    if (n < 44)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 判断条件：std::memcmp(hdr, "RIFF", 4) != 0 || std::memcmp(hdr + 8, "WAVE", 4) != 0 是否成立。
    if (std::memcmp(hdr, "RIFF", 4) != 0 || std::memcmp(hdr + 8, "WAVE", 4) != 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 定义变量 uint32_t rate 并初始化为 0, data_sz = 0。
    uint32_t rate = 0, data_sz = 0;
    // 定义变量 uint16_t channels 并初始化为 0, bits = 0。
    uint16_t channels = 0, bits = 0;

    // 扫描 fmt + data chunks
    size_t off = 12;
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (off + 8 <= n)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* id = reinterpret_cast<const char*>(hdr + off);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint32_t sz = read_le32(hdr + off + 4);
        // 赋值：把右侧 8 的结果保存到左侧 off +。
        off += 8;
        // 判断条件：off + sz > n 是否成立。
        if (off + sz > n)
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 判断条件：std::memcmp(id, "fmt ", 4) == 0 && sz >= 16 是否成立。
        if (std::memcmp(id, "fmt ", 4) == 0 && sz >= 16)
        {
            // 赋值：把右侧 read_le16(hdr + off + 2) 的结果保存到左侧 channels。
            channels = read_le16(hdr + off + 2);
            // 赋值：把右侧 read_le32(hdr + off + 4) 的结果保存到左侧 rate。
            rate = read_le32(hdr + off + 4);
            // 赋值：把右侧 read_le16(hdr + off + 14) 的结果保存到左侧 bits。
            bits = read_le16(hdr + off + 14);
        }
        // 备用分支：前面的条件不成立时执行这里。
        else if (std::memcmp(id, "data", 4) == 0)
        {
            // 赋值：把右侧 sz 的结果保存到左侧 data_sz。
            data_sz = sz;
        }
        // 赋值：把右侧 sz 的结果保存到左侧 off +。
        off += sz;
    }

    // 判断条件：rate == 0 || channels == 0 || bits == 0 || data_sz == 0 是否成立。
    if (rate == 0 || channels == 0 || bits == 0 || data_sz == 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint32_t bps = rate * channels * (static_cast<uint32_t>(bits) / 8U);
    // 判断条件：bps == 0 是否成立。
    if (bps == 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint32_t sec = data_sz / bps;
    // 赋值：把右侧 (sec > 0xFFFFU) ? 0xFFFFU : static_cast<uint16_t>(sec) 的结果保存到左侧 sec_out。
    sec_out = (sec > 0xFFFFU) ? 0xFFFFU : static_cast<uint16_t>(sec);
    // 返回 sec_out > 0 给调用者，表示当前函数的执行结果。
    return sec_out > 0;
}

// 定义或声明函数 MusicLibrary::parse_mp3_duration：参数在括号里，返回类型写在函数名前面。
bool MusicLibrary::parse_mp3_duration(const char* path, uint32_t file_size, uint16_t& sec_out) const
{
    // 轻量估算：跳过 ID3v2 → 找首帧同步字 → 读 bitrate；
    // 若有 Xing/Info 帧数则用帧数算更准，否则用 file_size*8/bitrate。
    // 探测缓冲放堆上：music_scan 任务栈只有几 KB，不能在栈上开 8KB。
    sec_out = 0;
    // 判断条件：path == nullptr || file_size < 4 是否成立。
    if (path == nullptr || file_size < 4)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 std::fopen(path, "rb") 的结果保存到左侧 FILE* f。
    FILE* f = std::fopen(path, "rb");
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 固定参数 kProbe = 4096：由当前模块定义，用于控制布局、时序或硬件行为。
    constexpr size_t kProbe = 4096;
    // 执行这条代码：auto* buf =。它会更新变量、对象状态或调用链。
    auto* buf =
        // 继续上一行的表达式或参数列表：static_cast<uint8_t*>(heap_caps_malloc(kProbe, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));。
        static_cast<uint8_t*>(heap_caps_malloc(kProbe, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    // 判断条件：buf == nullptr 是否成立。
    if (buf == nullptr)
        // 赋值：把右侧 static_cast<uint8_t*>(heap_caps_malloc(kProbe, MALLOC_CAP_8BIT)) 的结果保存到左侧 buf。
        buf = static_cast<uint8_t*>(heap_caps_malloc(kProbe, MALLOC_CAP_8BIT));
    // 判断条件：buf == nullptr 是否成立。
    if (buf == nullptr)
    {
        // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(f);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 定义变量 size_t n 并初始化为 std::fread(buf, 1, kProbe, f)。
    size_t n = std::fread(buf, 1, kProbe, f);
    // 判断条件：n < 4 是否成立。
    if (n < 4)
    {
        // 调用 heap_caps_free(buf);，执行这个函数并处理它的返回值或状态变化。
        heap_caps_free(buf);
        // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(f);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 定义变量 size_t off 并初始化为 0。
    size_t off = 0;
    // 定义变量 uint32_t id3_skip 并初始化为 0。
    uint32_t id3_skip = 0;
    // 判断条件：n >= 10 && buf[0] == 'I' && buf[1] == 'D' && buf[2] == '3' 是否成立。
    if (n >= 10 && buf[0] == 'I' && buf[1] == 'D' && buf[2] == '3')
    {
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        id3_skip = 10U + ((static_cast<uint32_t>(buf[6] & 0x7F) << 21) |
                          // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
                          (static_cast<uint32_t>(buf[7] & 0x7F) << 14) |
                          // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
                          (static_cast<uint32_t>(buf[8] & 0x7F) << 7) |
                          // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
                          static_cast<uint32_t>(buf[9] & 0x7F));
        // 判断条件：id3_skip >= file_size 是否成立。
        if (id3_skip >= file_size)
        {
            // 调用 heap_caps_free(buf);，执行这个函数并处理它的返回值或状态变化。
            heap_caps_free(buf);
            // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
            std::fclose(f);
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }
        // ID3 可能大于本次读入长度：跳过后再读一帧探测窗口
        if (id3_skip >= n)
        {
            // 判断条件：std::fseek(f, static_cast<long>(id3_skip), SEEK_SET) != 0 是否成立。
            if (std::fseek(f, static_cast<long>(id3_skip), SEEK_SET) != 0)
            {
                // 调用 heap_caps_free(buf);，执行这个函数并处理它的返回值或状态变化。
                heap_caps_free(buf);
                // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
                std::fclose(f);
                // 返回 false 给调用者，表示当前函数的执行结果。
                return false;
            }
            // 赋值：把右侧 std::fread(buf, 1, kProbe, f) 的结果保存到左侧 n。
            n = std::fread(buf, 1, kProbe, f);
            // 判断条件：n < 4 是否成立。
            if (n < 4)
            {
                // 调用 heap_caps_free(buf);，执行这个函数并处理它的返回值或状态变化。
                heap_caps_free(buf);
                // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
                std::fclose(f);
                // 返回 false 给调用者，表示当前函数的执行结果。
                return false;
            }
            // 赋值：把右侧 0 的结果保存到左侧 off。
            off = 0;
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 赋值：把右侧 id3_skip 的结果保存到左侧 off。
            off = id3_skip;
        }
    }
    // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
    std::fclose(f);

    // MPEG1 Layer3 bitrate 表 (kbps)，索引 1..14
    static constexpr uint16_t kBrMpeg1L3[16] = {0,   32,  40,  48,  56,  64,  80,  96,
                                                // 继续上一行的表达式或参数列表：112, 128, 160, 192, 224, 256, 320, 0};。
                                                112, 128, 160, 192, 224, 256, 320, 0};
    // 静态查表数组 kBrMpeg2L3：运行时复用固定数据，避免每次重新计算。
    static constexpr uint16_t kBrMpeg2L3[16] = {0,  8,  16, 24,  32,  40,  48,  56,
                                                // 继续上一行的表达式或参数列表：64, 80, 96, 112, 128, 144, 160, 0};。
                                                64, 80, 96, 112, 128, 144, 160, 0};
    // 静态查表数组 kSrMpeg1：运行时复用固定数据，避免每次重新计算。
    static constexpr uint16_t kSrMpeg1[4] = {44100, 48000, 32000, 0};
    // 静态查表数组 kSrMpeg2：运行时复用固定数据，避免每次重新计算。
    static constexpr uint16_t kSrMpeg2[4] = {22050, 24000, 16000, 0};
    // 静态查表数组 kSrMpeg25：运行时复用固定数据，避免每次重新计算。
    static constexpr uint16_t kSrMpeg25[4] = {11025, 12000, 8000, 0};

    // 定义变量 bool ok 并初始化为 false。
    bool ok = false;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (; off + 4 < n; ++off)
    {
        // 判断条件：buf[off] != 0xFF || (buf[off + 1] & 0xE0) != 0xE0 是否成立。
        if (buf[off] != 0xFF || (buf[off + 1] & 0xE0) != 0xE0)
            // 继续下一轮：跳过本次循环剩余代码。
            continue;

        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t b1 = buf[off + 1];
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t b2 = buf[off + 2];
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t ver_bits = static_cast<uint8_t>((b1 >> 3) & 0x03); // 3=MPEG1, 2=MPEG2, 0=2.5
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t layer = static_cast<uint8_t>((b1 >> 1) & 0x03);    // 1=Layer3
        // 判断条件：layer != 1 是否成立。
        if (layer != 1)
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        // 判断条件：ver_bits == 1 是否成立。
        if (ver_bits == 1)
            // 继续下一轮：跳过本次循环剩余代码。
            continue;

        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t br_idx = static_cast<uint8_t>((b2 >> 4) & 0x0F);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t sr_idx = static_cast<uint8_t>((b2 >> 2) & 0x03);
        // 判断条件：br_idx == 0 || br_idx == 15 || sr_idx == 3 是否成立。
        if (br_idx == 0 || br_idx == 15 || sr_idx == 3)
            // 继续下一轮：跳过本次循环剩余代码。
            continue;

        // 定义变量 uint16_t bitrate_kbps 并初始化为 0。
        uint16_t bitrate_kbps = 0;
        // 定义变量 uint16_t sample_rate 并初始化为 0。
        uint16_t sample_rate = 0;
        // 判断条件：ver_bits == 3 是否成立。
        if (ver_bits == 3)
        {
            // 赋值：把右侧 kBrMpeg1L3[br_idx] 的结果保存到左侧 bitrate_kbps。
            bitrate_kbps = kBrMpeg1L3[br_idx];
            // 赋值：把右侧 kSrMpeg1[sr_idx] 的结果保存到左侧 sample_rate。
            sample_rate = kSrMpeg1[sr_idx];
        }
        // 备用分支：前面的条件不成立时执行这里。
        else if (ver_bits == 2)
        {
            // 赋值：把右侧 kBrMpeg2L3[br_idx] 的结果保存到左侧 bitrate_kbps。
            bitrate_kbps = kBrMpeg2L3[br_idx];
            // 赋值：把右侧 kSrMpeg2[sr_idx] 的结果保存到左侧 sample_rate。
            sample_rate = kSrMpeg2[sr_idx];
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 赋值：把右侧 kBrMpeg2L3[br_idx] 的结果保存到左侧 bitrate_kbps。
            bitrate_kbps = kBrMpeg2L3[br_idx];
            // 赋值：把右侧 kSrMpeg25[sr_idx] 的结果保存到左侧 sample_rate。
            sample_rate = kSrMpeg25[sr_idx];
        }
        // 判断条件：bitrate_kbps == 0 || sample_rate == 0 是否成立。
        if (bitrate_kbps == 0 || sample_rate == 0)
            // 继续下一轮：跳过本次循环剩余代码。
            continue;

        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t padding = static_cast<uint8_t>((b2 >> 1) & 0x01);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint32_t samples_per_frame = (ver_bits == 3) ? 1152U : 576U;
        // 定义变量：uint32_t frame_bytes =
        uint32_t frame_bytes =
            // 继续上一行的表达式或参数列表：(samples_per_frame / 8U) * static_cast<uint32_t>(bitrate_kbps) * 1000U / sample_rate;。
            (samples_per_frame / 8U) * static_cast<uint32_t>(bitrate_kbps) * 1000U / sample_rate;
        // 判断条件：padding 是否成立。
        if (padding)
            // 执行语句：++frame_bytes;；它会更新一次变量、对象或程序状态。
            ++frame_bytes;
        // 判断条件：frame_bytes < 4 是否成立。
        if (frame_bytes < 4)
            // 继续下一轮：跳过本次循环剩余代码。
            continue;

        // 定义变量 uint32_t xing_frames 并初始化为 0。
        uint32_t xing_frames = 0;
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (size_t p = off + 4; p + 12 < n && p < off + 4 + 64; ++p)
        {
            // 判断条件：(buf[p] == 'X' && buf[p + 1] == 'i' && buf[p + 2] == 'n' && buf[p + 3] == 'g' 是否成立。
            if ((buf[p] == 'X' && buf[p + 1] == 'i' && buf[p + 2] == 'n' && buf[p + 3] == 'g') ||
                // 继续上一行的表达式或参数列表：(buf[p] == 'I' && buf[p + 1] == 'n' && buf[p + 2] == 'f' && buf[p + 3] == 'o'))。
                (buf[p] == 'I' && buf[p + 1] == 'n' && buf[p + 2] == 'f' && buf[p + 3] == 'o'))
            {
                // 只读约束：这个值或对象在当前位置不能被修改。
                const uint32_t flags = (static_cast<uint32_t>(buf[p + 4]) << 24) |
                                       // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
                                       (static_cast<uint32_t>(buf[p + 5]) << 16) |
                                       // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
                                       (static_cast<uint32_t>(buf[p + 6]) << 8) |
                                       // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
                                       static_cast<uint32_t>(buf[p + 7]);
                // 判断条件：flags & 0x1U 是否成立。
                if (flags & 0x1U)
                {
                    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
                    xing_frames = (static_cast<uint32_t>(buf[p + 8]) << 24) |
                                  // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
                                  (static_cast<uint32_t>(buf[p + 9]) << 16) |
                                  // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
                                  (static_cast<uint32_t>(buf[p + 10]) << 8) |
                                  // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
                                  static_cast<uint32_t>(buf[p + 11]);
                }
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            }
        }

        // 定义变量 uint32_t sec 并初始化为 0。
        uint32_t sec = 0;
        // 判断条件：xing_frames > 0 是否成立。
        if (xing_frames > 0)
        {
            // 赋值：把右侧 (xing_frames * samples_per_frame) / sample_rate 的结果保存到左侧 sec。
            sec = (xing_frames * samples_per_frame) / sample_rate;
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 定义变量 uint32_t payload 并初始化为 file_size。
            uint32_t payload = file_size;
            // 判断条件：id3_skip > 0 && id3_skip < payload 是否成立。
            if (id3_skip > 0 && id3_skip < payload)
                // 赋值：把右侧 id3_skip 的结果保存到左侧 payload -。
                payload -= id3_skip;
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint32_t bps = static_cast<uint32_t>(bitrate_kbps) * 1000U;
            // 判断条件：bps == 0 是否成立。
            if (bps == 0)
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 赋值：把右侧 static_cast<uint32_t>((static_cast<uint64_t>(payload) * 8ULL) / bps) 的结果保存到左侧 sec。
            sec = static_cast<uint32_t>((static_cast<uint64_t>(payload) * 8ULL) / bps);
        }

        // 判断条件：sec == 0 是否成立。
        if (sec == 0)
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 赋值：把右侧 (sec > 0xFFFFU) ? 0xFFFFU : static_cast<uint16_t>(sec) 的结果保存到左侧 sec_out。
        sec_out = (sec > 0xFFFFU) ? 0xFFFFU : static_cast<uint16_t>(sec);
        // 赋值：把右侧 true 的结果保存到左侧 ok。
        ok = true;
        // 跳出当前循环或 switch，不再执行剩余分支。
        break;
    }

    // 调用 heap_caps_free(buf);，执行这个函数并处理它的返回值或状态变化。
    heap_caps_free(buf);
    // 返回 ok 给调用者，表示当前函数的执行结果。
    return ok;
}

// 函数调用：bool MusicLibrary::add_track(const char* full_path, const char* name, uint32_t size,；执行该函数并处理它的返回值或副作用。
bool MusicLibrary::add_track(const char* full_path, const char* name, uint32_t size,
                             // 继续上一行的表达式或参数列表：MusicTrack* out, uint8_t& out_count)。
                             MusicTrack* out, uint8_t& out_count)
{
    // 判断条件：out_count >= kMaxTracks || full_path == nullptr || name == nullptr 是否成立。
    if (out_count >= kMaxTracks || full_path == nullptr || name == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 out[out_count] 的结果保存到左侧 MusicTrack& t。
    MusicTrack& t = out[out_count];
    // 调用 std::strncpy(t.path, full_path, sizeof(t.path) - 1);，执行这个函数并处理它的返回值或状态变化。
    std::strncpy(t.path, full_path, sizeof(t.path) - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 t.path[sizeof(t.path) - 1]。
    t.path[sizeof(t.path) - 1] = '\0';

    // 执行语句：char norm[48];；它会更新一次变量、对象或程序状态。
    char norm[48];
    // 函数调用：(void)text::normalize_path_segment(name, norm, sizeof(norm));；执行该函数并处理它的返回值或副作用。
    (void)text::normalize_path_segment(name, norm, sizeof(norm));
    // 调用 strip_ext(norm, t.title, sizeof(t.title));，执行这个函数并处理它的返回值或状态变化。
    strip_ext(norm, t.title, sizeof(t.title));

    // 赋值：把右侧 size 的结果保存到左侧 t.size_bytes。
    t.size_bytes = size;
    // 赋值：把右侧 0 的结果保存到左侧 t.duration_sec。
    t.duration_sec = 0;

    // 执行语句：char ext[8]{};；它会更新一次变量、对象或程序状态。
    char ext[8]{};
    // 判断条件：ext_lower(name, ext, sizeof(ext)) 是否成立。
    if (ext_lower(name, ext, sizeof(ext)))
    {
        // 判断条件：std::strcmp(ext, "wav") == 0 是否成立。
        if (std::strcmp(ext, "wav") == 0)
            // 访问对象成员：(void)parse_wav_duration(full_path, t.duration_sec);；读取或修改这个对象里的成员状态。
            (void)parse_wav_duration(full_path, t.duration_sec);
        // 备用分支：前面的条件不成立时执行这里。
        else if (std::strcmp(ext, "mp3") == 0)
            // 访问对象成员：(void)parse_mp3_duration(full_path, size, t.duration_sec);；读取或修改这个对象里的成员状态。
            (void)parse_mp3_duration(full_path, size, t.duration_sec);
    }

    // 执行语句：++out_count;；它会更新一次变量、对象或程序状态。
    ++out_count;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 MusicLibrary::find_index：参数在括号里，返回类型写在函数名前面。
int8_t MusicLibrary::find_index(const char* full_path) const
{
    // 判断条件：full_path == nullptr || tracks_ == nullptr 是否成立。
    if (full_path == nullptr || tracks_ == nullptr)
        // 返回 -1 给调用者，表示当前函数的执行结果。
        return -1;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < count_; ++i)
    {
        // 判断条件：std::strcmp(tracks_[i].path, full_path) == 0 是否成立。
        if (std::strcmp(tracks_[i].path, full_path) == 0)
            // 返回 static_cast<int8_t>(i) 给调用者，表示当前函数的执行结果。
            return static_cast<int8_t>(i);
    }
    // 返回 -1 给调用者，表示当前函数的执行结果。
    return -1;
}

// 定义或声明函数 MusicLibrary::set_duration：参数在括号里，返回类型写在函数名前面。
void MusicLibrary::set_duration(uint8_t idx, uint16_t sec)
{
    // 只允许从 0 写入一次；拒绝 0xFFFF（bitrate 误判饱和值）
    if (tracks_ == nullptr || idx >= count_ || sec == 0 || sec == 0xFFFFU)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：tracks_[idx].duration_sec == 0 是否成立。
    if (tracks_[idx].duration_sec == 0)
        // 赋值：把右侧 sec 的结果保存到左侧 tracks_[idx].duration_sec。
        tracks_[idx].duration_sec = sec;
}

// 定义或声明函数 MusicLibrary::add_track_from_path：参数在括号里，返回类型写在函数名前面。
bool MusicLibrary::add_track_from_path(const char* full_path)
{
    // 判断条件：full_path == nullptr || !ensure_tracks() 是否成立。
    if (full_path == nullptr || !ensure_tracks())
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：find_index(full_path) >= 0 是否成立。
    if (find_index(full_path) >= 0)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    // 判断条件：count_ >= kMaxTracks 是否成立。
    if (count_ >= kMaxTracks)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // struct stat：把相关数据字段组合在一起，表示项目中的一个概念。
    struct stat st{};
    // 判断条件：stat(full_path, &st) != 0 || !S_ISREG(st.st_mode) 是否成立。
    if (stat(full_path, &st) != 0 || !S_ISREG(st.st_mode))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* name = std::strrchr(full_path, '/');
    // 赋值：把右侧 (name != nullptr) ? (name + 1) : full_path 的结果保存到左侧 name。
    name = (name != nullptr) ? (name + 1) : full_path;
    // 判断条件：!is_audio(name) 是否成立。
    if (!is_audio(name))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 返回 add_track(full_path, name, static_cast<uint32_t>(st.st_size), tracks_, count_) 给调用者，表示当前函数的执行结果。
    return add_track(full_path, name, static_cast<uint32_t>(st.st_size), tracks_, count_);
}

// 定义或声明函数 MusicLibrary::scan_dir：参数在括号里，返回类型写在函数名前面。
bool MusicLibrary::scan_dir(const char* dir, MusicTrack* out, uint8_t& out_count)
{
    // 判断条件：dir == nullptr || out == nullptr 是否成立。
    if (dir == nullptr || out == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // struct stat：把相关数据字段组合在一起，表示项目中的一个概念。
    struct stat dst{};
    // 判断条件：stat(dir, &dst) != 0 || !S_ISDIR(dst.st_mode) 是否成立。
    if (stat(dir, &dst) != 0 || !S_ISDIR(dst.st_mode))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 opendir(dir) 的结果保存到左侧 DIR* d。
    DIR* d = opendir(dir);
    // 判断条件：d == nullptr 是否成立。
    if (d == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 固定参数 kPathCap = 128：由当前模块定义，用于控制布局、时序或硬件行为。
    constexpr size_t kPathCap = 128;

    // struct dirent：把相关数据字段组合在一起，表示项目中的一个概念。
    struct dirent* ent;
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while ((ent = readdir(d)) != nullptr && out_count < kMaxTracks)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* name = ent->d_name;
        // 判断条件：name == nullptr || name[0] == '.' || !is_audio(name) 是否成立。
        if (name == nullptr || name[0] == '.' || !is_audio(name))
            // 继续下一轮：跳过本次循环剩余代码。
            continue;

        // 只读约束：这个值或对象在当前位置不能被修改。
        const size_t dir_len = std::strlen(dir);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const size_t name_len = std::strlen(name);
        // 判断条件：dir_len + 1 + name_len >= kPathCap 是否成立。
        if (dir_len + 1 + name_len >= kPathCap)
            // 继续下一轮：跳过本次循环剩余代码。
            continue;

        // 执行语句：char full[kPathCap];；它会更新一次变量、对象或程序状态。
        char full[kPathCap];
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int n = std::snprintf(full, sizeof(full), "%s/%s", dir, name);
        // 判断条件：n < 0 || static_cast<size_t>(n) >= kPathCap 是否成立。
        if (n < 0 || static_cast<size_t>(n) >= kPathCap)
            // 继续下一轮：跳过本次循环剩余代码。
            continue;

        // struct stat：把相关数据字段组合在一起，表示项目中的一个概念。
        struct stat st{};
        // 判断条件：stat(full, &st) != 0 || !S_ISREG(st.st_mode) 是否成立。
        if (stat(full, &st) != 0 || !S_ISREG(st.st_mode))
            // 继续下一轮：跳过本次循环剩余代码。
            continue;

        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        (void)add_track(full, name, static_cast<uint32_t>(st.st_size), out, out_count);
    }

    // 调用 closedir(d);，执行这个函数并处理它的返回值或状态变化。
    closedir(d);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 MusicLibrary::scan：参数在括号里，返回类型写在函数名前面。
bool MusicLibrary::scan()
{
    // 判断条件：!ensure_tracks() 是否成立。
    if (!ensure_tracks())
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 ::app::common::storage::StorageMgr::get_instance() 的结果保存到左侧 auto& sto。
    auto& sto = ::app::common::storage::StorageMgr::get_instance();
    // 判断条件：sto.is_mounted(::app::common::storage::MountKind::Internal) 是否成立。
    if (sto.is_mounted(::app::common::storage::MountKind::Internal))
    {
        // 函数调用：(void)::mkdir("/int/Ebook", 0775);；执行该函数并处理它的返回值或副作用。
        (void)::mkdir("/int/Ebook", 0775);
        // 函数调用：(void)::mkdir(kDirInt, 0775);；执行该函数并处理它的返回值或副作用。
        (void)::mkdir(kDirInt, 0775);
    }
    // 判断条件：sto.is_mounted(::app::common::storage::MountKind::Sd) 是否成立。
    if (sto.is_mounted(::app::common::storage::MountKind::Sd))
    {
        // 函数调用：(void)::mkdir("/sd/Ebook", 0775);；执行该函数并处理它的返回值或副作用。
        (void)::mkdir("/sd/Ebook", 0775);
        // 函数调用：(void)::mkdir(kDirSd, 0775);；执行该函数并处理它的返回值或副作用。
        (void)::mkdir(kDirSd, 0775);
    }

    // 扫描缓冲放 PSRAM，扫完再原子替换到 tracks_，避免扫描中途列表半成品
    auto* scan_buf = static_cast<MusicTrack*>(calloc_psram_prefer(kMaxTracks, sizeof(MusicTrack)));
    // 判断条件：scan_buf == nullptr 是否成立。
    if (scan_buf == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 定义变量 uint8_t n 并初始化为 0。
    uint8_t n = 0;
    // 判断条件：sto.is_mounted(::app::common::storage::MountKind::Internal) 是否成立。
    if (sto.is_mounted(::app::common::storage::MountKind::Internal))
        // 函数调用：(void)scan_dir(kDirInt, scan_buf, n);；执行该函数并处理它的返回值或副作用。
        (void)scan_dir(kDirInt, scan_buf, n);
    // 判断条件：sto.is_mounted(::app::common::storage::MountKind::Sd) 是否成立。
    if (sto.is_mounted(::app::common::storage::MountKind::Sd))
        // 函数调用：(void)scan_dir(kDirSd, scan_buf, n);；执行该函数并处理它的返回值或副作用。
        (void)scan_dir(kDirSd, scan_buf, n);

    // 调用 std::memcpy(tracks_, scan_buf, sizeof(MusicTrack) * n);，执行这个函数并处理它的返回值或状态变化。
    std::memcpy(tracks_, scan_buf, sizeof(MusicTrack) * n);
    // 判断条件：n < kMaxTracks 是否成立。
    if (n < kMaxTracks)
        // 调用 std::memset(tracks_ + n, 0, sizeof(MusicTrack) * (kMaxTracks - n));，执行这个函数并处理它的返回值或状态变化。
        std::memset(tracks_ + n, 0, sizeof(MusicTrack) * (kMaxTracks - n));
    // 赋值：把右侧 n 的结果保存到左侧 count_。
    count_ = n;
    // 调用 heap_caps_free(scan_buf);，执行这个函数并处理它的返回值或状态变化。
    heap_caps_free(scan_buf);
    // 返回 count_ > 0 给调用者，表示当前函数的执行结果。
    return count_ > 0;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::music
