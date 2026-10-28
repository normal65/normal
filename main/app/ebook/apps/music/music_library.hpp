// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/music/music_library.hpp
 * 作用：扫描音乐目录并维护歌曲元数据。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::music {

// struct MusicTrack：把相关数据字段组合在一起，表示项目中的一个概念。
struct MusicTrack
{
    // 执行语句：char path[128];；它会更新一次变量、对象或程序状态。
    char path[128];
    // 执行语句：char title[48];；它会更新一次变量、对象或程序状态。
    char title[48];
    // 执行语句：uint32_t size_bytes;；它会更新一次变量、对象或程序状态。
    uint32_t size_bytes;
    // 执行语句：uint16_t duration_sec;；它会更新一次变量、对象或程序状态。
    uint16_t duration_sec;
};

// 类定义：把数据和操作这些数据的方法绑定在一起。
class MusicLibrary
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 固定参数 kMaxTracks = 64：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kMaxTracks = 64;

    /** 与 is_audio() / scan_dir() 一致，供空状态提示 */
    // 固定参数 kScanFmtHint = "MP3 / WAV"：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr const char* kScanFmtHint = "MP3 / WAV";
    // 固定参数 kScanPathInt = "/int/Ebook/music"：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr const char* kScanPathInt = "/int/Ebook/music";
    // 固定参数 kScanPathSd = "/sd/Ebook/music"：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr const char* kScanPathSd = "/sd/Ebook/music";

    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static MusicLibrary& get_instance();

    // 函数调用：bool scan();；执行该函数并处理它的返回值或副作用。
    bool scan();

    // 函数调用：bool add_track_from_path(const char* full_path);；执行该函数并处理它的返回值或副作用。
    bool add_track_from_path(const char* full_path);
    // 函数调用：int8_t find_index(const char* full_path) const;；执行该函数并处理它的返回值或副作用。
    int8_t find_index(const char* full_path) const;

    // 函数调用：void set_duration(uint8_t idx, uint16_t sec);；执行该函数并处理它的返回值或副作用。
    void set_duration(uint8_t idx, uint16_t sec);

    // 定义或声明函数 count：参数在括号里，返回类型写在函数名前面。
    uint8_t count() const
    {
        // 返回 count_ 给调用者，表示当前函数的执行结果。
        return count_;
    }
    // 只读约束：这个值或对象在当前位置不能被修改。
    const MusicTrack& at(uint8_t idx) const;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 MusicLibrary() = default;，执行这个函数并处理它的返回值或状态变化。
    MusicLibrary() = default;

    // 函数调用：bool scan_dir(const char* dir, MusicTrack* out, uint8_t& out_count);；执行该函数并处理它的返回值或副作用。
    bool scan_dir(const char* dir, MusicTrack* out, uint8_t& out_count);
    // 函数调用：bool add_track(const char* full_path, const char* name, uint32_t size, MusicTrack* out,；执行该函数并处理它的返回值或副作用。
    bool add_track(const char* full_path, const char* name, uint32_t size, MusicTrack* out,
                   // 继续上一行的表达式或参数列表：uint8_t& out_count);。
                   uint8_t& out_count);
    // 函数调用：bool parse_wav_duration(const char* path, uint16_t& sec_out) const;；执行该函数并处理它的返回值或副作用。
    bool parse_wav_duration(const char* path, uint16_t& sec_out) const;
    // 函数调用：bool parse_mp3_duration(const char* path, uint32_t file_size, uint16_t& sec_out) const;；执行该函数并处理它的返回值或副作用。
    bool parse_mp3_duration(const char* path, uint32_t file_size, uint16_t& sec_out) const;
    // 函数调用：bool ensure_tracks();；执行该函数并处理它的返回值或副作用。
    bool ensure_tracks();

    // 执行语句：MusicTrack* tracks_{nullptr};；它会更新一次变量、对象或程序状态。
    MusicTrack* tracks_{nullptr};
    // 执行语句：uint8_t count_{0};；它会更新一次变量、对象或程序状态。
    uint8_t count_{0};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::music
