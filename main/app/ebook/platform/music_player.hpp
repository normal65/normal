// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/platform/music_player.hpp
 * 作用：执行本地音乐播放、解码、写入音频缓冲和控制播放状态。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：阻塞工作放独立任务，周期小检查放 Housekeeper。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <memory>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_audio_simple_dec.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/task/task.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::platform {

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class MusicPlayState : uint8_t
{
    // 执行这条代码：Idle = 0,。它会更新变量、对象状态或调用链。
    Idle = 0,
    // 继续上一行的表达式或参数列表：Loading,。
    Loading,
    // 继续上一行的表达式或参数列表：Playing,。
    Playing,
    // 继续上一行的表达式或参数列表：Paused,。
    Paused,
    // 继续上一行的表达式或参数列表：Error,。
    Error,
};

/** @brief 本地文件播放（esp_audio_simple_dec + ES8311；PSRAM 大栈 Task） */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class MusicPlayer
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static MusicPlayer& get_instance();

    // 函数调用：bool init();；执行该函数并处理它的返回值或副作用。
    bool init();
    // 调用 stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
    void stop();

    // 定义或声明函数 state：参数在括号里，返回类型写在函数名前面。
    MusicPlayState state() const
    {
        // 返回 state_ 给调用者，表示当前函数的执行结果。
        return state_;
    }

    // 函数调用：bool play(const char* path);；执行该函数并处理它的返回值或副作用。
    bool play(const char* path);
    // 函数调用：void pause();；执行该函数并处理它的返回值或副作用。
    void pause();
    // 调用 resume()：这些是对 FreeRTOS API 的直接包装： suspend() 暂停任务。 resume() 恢复。 setpriority() 动态改优先级。 getpriority() 读取当前优先级。。
    void resume();

    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* current_path() const
    {
        // 返回 path_ 给调用者，表示当前函数的执行结果。
        return path_;
    }

    // 函数调用：uint16_t elapsed_sec() const;；执行该函数并处理它的返回值或副作用。
    uint16_t elapsed_sec() const;
    // 定义或声明函数 duration_sec：参数在括号里，返回类型写在函数名前面。
    uint16_t duration_sec() const
    {
        // 返回 duration_sec_ 给调用者，表示当前函数的执行结果。
        return duration_sec_;
    }

    // 函数调用：void hint_duration(uint16_t sec);；执行该函数并处理它的返回值或副作用。
    void hint_duration(uint16_t sec);

    /** last_error: 2=fopen 3=mem 4=decode 5=codec 6=dec_reg */
    // 定义或声明函数 last_error：参数在括号里，返回类型写在函数名前面。
    uint8_t last_error() const
    {
        // 返回 last_error_ 给调用者，表示当前函数的执行结果。
        return last_error_;
    }

    // 定义或声明函数 stop_requested：参数在括号里，返回类型写在函数名前面。
    bool stop_requested() const
    {
        // 返回 stop_req_ 给调用者，表示当前函数的执行结果。
        return stop_req_;
    }

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 MusicPlayer() = default;，执行这个函数并处理它的返回值或状态变化。
    MusicPlayer() = default;

    // 函数调用：void run_playback(const char* path);；执行该函数并处理它的返回值或副作用。
    void run_playback(const char* path);
    // 函数调用：void set_state(MusicPlayState s);；执行该函数并处理它的返回值或副作用。
    void set_state(MusicPlayState s);
    // 函数调用：void post_state_hint();；执行该函数并处理它的返回值或副作用。
    void post_state_hint();
    // 函数调用：void post_progress_hint();；执行该函数并处理它的返回值或副作用。
    void post_progress_hint();
    // 函数调用：void wait_if_paused();；执行该函数并处理它的返回值或副作用。
    void wait_if_paused();
    // 函数调用：void freeze_elapsed();；执行该函数并处理它的返回值或副作用。
    void freeze_elapsed();
    // 函数调用：void unfreeze_elapsed();；执行该函数并处理它的返回值或副作用。
    void unfreeze_elapsed();
    // 函数调用：bool try_start_output(esp_audio_simple_dec_handle_t dec);；执行该函数并处理它的返回值或副作用。
    bool try_start_output(esp_audio_simple_dec_handle_t dec);
    // 函数调用：bool decode_raw(esp_audio_simple_dec_handle_t dec, esp_audio_simple_dec_raw_t* raw,；执行该函数并处理它的返回值或副作用。
    bool decode_raw(esp_audio_simple_dec_handle_t dec, esp_audio_simple_dec_raw_t* raw,
                    // 继续上一行的表达式或参数列表：uint8_t** out_buf, size_t* out_cap, bool* started, bool* failed);。
                    uint8_t** out_buf, size_t* out_cap, bool* started, bool* failed);
    // 函数调用：bool ensure_decoders();；执行该函数并处理它的返回值或副作用。
    bool ensure_decoders();
    // 函数调用：void clear_play_task();；执行该函数并处理它的返回值或副作用。
    void clear_play_task();
    // 函数调用：void update_duration_from_bitrate(uint32_t bitrate_bps);；执行该函数并处理它的返回值或副作用。
    void update_duration_from_bitrate(uint32_t bitrate_bps);

    // 易变变量：告诉编译器该值可能被中断或硬件改变，不能随意优化读取。
    volatile bool stop_req_{false};
    // 易变变量：告诉编译器该值可能被中断或硬件改变，不能随意优化读取。
    volatile bool pause_req_{false};
    // 执行语句：MusicPlayState state_{MusicPlayState::Idle};；它会更新一次变量、对象或程序状态。
    MusicPlayState state_{MusicPlayState::Idle};
    // 执行语句：uint8_t last_error_{0};；它会更新一次变量、对象或程序状态。
    uint8_t last_error_{0};

    // 执行语句：char path_[128]{};；它会更新一次变量、对象或程序状态。
    char path_[128]{};
    // 执行语句：uint32_t file_size_{0};；它会更新一次变量、对象或程序状态。
    uint32_t file_size_{0};
    // 执行语句：uint16_t duration_sec_{0};；它会更新一次变量、对象或程序状态。
    uint16_t duration_sec_{0};
    // 执行语句：int64_t play_start_us_{0};；它会更新一次变量、对象或程序状态。
    int64_t play_start_us_{0};
    // 执行语句：int64_t elapsed_acc_us_{0};；它会更新一次变量、对象或程序状态。
    int64_t elapsed_acc_us_{0};

    // 执行语句：bool dec_registered_{false};；它会更新一次变量、对象或程序状态。
    bool dec_registered_{false};
    // 易变变量：告诉编译器该值可能被中断或硬件改变，不能随意优化读取。
    volatile bool play_task_alive_{false};
    // 执行语句：std::unique_ptr<::app::sys::task::Task> play_task_;；它会更新一次变量、对象或程序状态。
    std::unique_ptr<::app::sys::task::Task> play_task_;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::platform
