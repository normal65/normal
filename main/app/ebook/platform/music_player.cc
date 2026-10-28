/**
 * [初学者导读] 文件：main/app/ebook/platform/music_player.cc
 * 作用：执行本地音乐播放、解码、写入音频缓冲和控制播放状态。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：阻塞工作放独立任务，周期小检查放 Housekeeper。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "platform/music_player.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <strings.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_heap_caps.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "common/time/time.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/system_state.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "platform/audio_codec.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/task/task.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_bus.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_event.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_audio_dec_default.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_audio_simple_dec.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_audio_simple_dec_default.h"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "MusicPlayer";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::platform {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 任务栈大小：20480，栈太小会崩溃，太大会浪费 RAM。
constexpr size_t kPlayStack = 20480;
// 缓冲区或分块大小：4096，影响单次读写占用和性能。
constexpr size_t kReadChunk = 4096;
// 固定参数 kOutBufInit = 8192：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr size_t kOutBufInit = 8192;
// 固定参数 kStopWaitMs = 3000：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t kStopWaitMs = 3000;
// 固定参数 kProgressMs = 1000：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t kProgressMs = 1000;

// 固定参数 kCapsPsram = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t kCapsPsram = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT;

// 定义或声明函数 alloc_psram：参数在括号里，返回类型写在函数名前面。
void* alloc_psram(size_t size)
{
    // 返回 heap_caps_malloc(size, kCapsPsram) 给调用者，表示当前函数的执行结果。
    return heap_caps_malloc(size, kCapsPsram);
}

// 定义或声明函数 realloc_psram：参数在括号里，返回类型写在函数名前面。
void* realloc_psram(void* ptr, size_t size)
{
    // 返回 heap_caps_realloc(ptr, size, kCapsPsram) 给调用者，表示当前函数的执行结果。
    return heap_caps_realloc(ptr, size, kCapsPsram);
}

// 定义或声明函数 dec_type_from_path：参数在括号里，返回类型写在函数名前面。
esp_audio_simple_dec_type_t dec_type_from_path(const char* path)
{
    // 判断条件：path == nullptr 是否成立。
    if (path == nullptr)
        // 返回 ESP_AUDIO_SIMPLE_DEC_TYPE_NONE 给调用者，表示当前函数的执行结果。
        return ESP_AUDIO_SIMPLE_DEC_TYPE_NONE;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* ext = std::strrchr(path, '.');
    // 判断条件：ext == nullptr 是否成立。
    if (ext == nullptr)
        // 返回 ESP_AUDIO_SIMPLE_DEC_TYPE_NONE 给调用者，表示当前函数的执行结果。
        return ESP_AUDIO_SIMPLE_DEC_TYPE_NONE;
    // 执行语句：++ext;；它会更新一次变量、对象或程序状态。
    ++ext;
    // 判断条件：strcasecmp(ext, "mp3") == 0 是否成立。
    if (strcasecmp(ext, "mp3") == 0)
        // 返回 ESP_AUDIO_SIMPLE_DEC_TYPE_MP3 给调用者，表示当前函数的执行结果。
        return ESP_AUDIO_SIMPLE_DEC_TYPE_MP3;
    // 判断条件：strcasecmp(ext, "wav") == 0 是否成立。
    if (strcasecmp(ext, "wav") == 0)
        // 返回 ESP_AUDIO_SIMPLE_DEC_TYPE_WAV 给调用者，表示当前函数的执行结果。
        return ESP_AUDIO_SIMPLE_DEC_TYPE_WAV;
    // 返回 ESP_AUDIO_SIMPLE_DEC_TYPE_NONE 给调用者，表示当前函数的执行结果。
    return ESP_AUDIO_SIMPLE_DEC_TYPE_NONE;
}

// 定义或声明函数 write_pcm：参数在括号里，返回类型写在函数名前面。
size_t write_pcm(const uint8_t* data, size_t length, bool stop)
{
    // 判断条件：stop || data == nullptr || length == 0 是否成立。
    if (stop || data == nullptr || length == 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    // 判断条件：data::SystemState::get_instance().mute() 是否成立。
    if (data::SystemState::get_instance().mute())
        // 返回 length 给调用者，表示当前函数的执行结果。
        return length;
    // 判断条件：!AudioCodec::get_instance().ready() 是否成立。
    if (!AudioCodec::get_instance().ready())
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 赋值：把右侧 AudioCodec::get_instance().codec() 的结果保存到左侧 auto& codec。
    auto& codec = AudioCodec::get_instance().codec();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int n = static_cast<int>(length / sizeof(int16_t));
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int wr = codec.write(reinterpret_cast<const int16_t*>(data), n);
    // 判断条件：wr <= 0 是否成立。
    if (wr <= 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    // 返回 static_cast<size_t>(wr) * sizeof(int16_t) 给调用者，表示当前函数的执行结果。
    return static_cast<size_t>(wr) * sizeof(int16_t);
}

// 定义或声明函数 wait_state：参数在括号里，返回类型写在函数名前面。
bool wait_state(volatile MusicPlayState& state, MusicPlayState match)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int64_t t0 = ::app::common::time::uptime_ms();
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (state != match)
    {
        // 判断条件：(::app::common::time::uptime_ms() - t0) > static_cast<int64_t>(kStopWaitMs) 是否成立。
        if ((::app::common::time::uptime_ms() - t0) > static_cast<int64_t>(kStopWaitMs))
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        // 调用 app::sys::task::TaskMgr::delay_ms(20)：调用 vTaskDelay()，这是让出 CPU 的阻塞延时。一千毫秒延时期间，任务不占 CPU，调度器可以运行别人。。
        ::app::sys::task::TaskMgr::delay_ms(20);
    }
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 MusicPlayer::get_instance：参数在括号里，返回类型写在函数名前面。
MusicPlayer& MusicPlayer::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static MusicPlayer s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义或声明函数 MusicPlayer::ensure_decoders：参数在括号里，返回类型写在函数名前面。
bool MusicPlayer::ensure_decoders()
{
    // 判断条件：dec_registered_ 是否成立。
    if (dec_registered_)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    // 判断条件：esp_audio_dec_register_default( 是否成立。
    if (esp_audio_dec_register_default() != ESP_AUDIO_ERR_OK ||
        // 定义或声明函数 esp_audio_simple_dec_register_default：参数在括号里，返回类型写在函数名前面。
        esp_audio_simple_dec_register_default() != ESP_AUDIO_ERR_OK)
    {
        // 调用 EBOOK_LOGE(TAG, "decoder register fail");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGE(TAG, "decoder register fail");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 赋值：把右侧 true 的结果保存到左侧 dec_registered_。
    dec_registered_ = true;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 MusicPlayer::init：参数在括号里，返回类型写在函数名前面。
bool MusicPlayer::init()
{
    // 判断条件：!AudioCodec::get_instance().ready() 是否成立。
    if (!AudioCodec::get_instance().ready())
    {
        // 调用 EBOOK_LOGW(TAG, "audio codec not ready");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "audio codec not ready");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 返回 ensure_decoders() 给调用者，表示当前函数的执行结果。
    return ensure_decoders();
}

// 定义或声明函数 MusicPlayer::clear_play_task：参数在括号里，返回类型写在函数名前面。
void MusicPlayer::clear_play_task()
{
    // 判断条件：play_task_ == nullptr 是否成立。
    if (play_task_ == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 任务已 vTaskDelete 时不可再 destroy
    if (play_task_alive_)
        // 调用 destroy()：直接删除，不是优雅停止。
        play_task_->destroy();
    // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    play_task_.reset();
}

// 定义或声明函数 MusicPlayer::set_state：参数在括号里，返回类型写在函数名前面。
void MusicPlayer::set_state(MusicPlayState s)
{
    // 赋值：把右侧 s 的结果保存到左侧 state_。
    state_ = s;
    // 调用 post_state_hint();，执行这个函数并处理它的返回值或状态变化。
    post_state_hint();
}

// 定义或声明函数 MusicPlayer::post_state_hint：参数在括号里，返回类型写在函数名前面。
void MusicPlayer::post_state_hint()
{
    // 函数调用：(void)ui::UiBus::get_instance().post_system_hint(ui::SystemHintKind::MusicStateChanged,；执行该函数并处理它的返回值或副作用。
    (void)ui::UiBus::get_instance().post_system_hint(ui::SystemHintKind::MusicStateChanged,
                                                     // 继续上一行的表达式或参数列表：static_cast<uint32_t>(state_));。
                                                     static_cast<uint32_t>(state_));
}

// 定义或声明函数 MusicPlayer::post_progress_hint：参数在括号里，返回类型写在函数名前面。
void MusicPlayer::post_progress_hint()
{
    // 函数调用：(void)ui::UiBus::get_instance().post_system_hint(ui::SystemHintKind::MusicProgress,；执行该函数并处理它的返回值或副作用。
    (void)ui::UiBus::get_instance().post_system_hint(ui::SystemHintKind::MusicProgress,
                                                     // 继续上一行的表达式或参数列表：static_cast<uint32_t>(elapsed_sec()));。
                                                     static_cast<uint32_t>(elapsed_sec()));
}

// 定义或声明函数 MusicPlayer::freeze_elapsed：参数在括号里，返回类型写在函数名前面。
void MusicPlayer::freeze_elapsed()
{
    // 判断条件：play_start_us_ <= 0 是否成立。
    if (play_start_us_ <= 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 ::app::common::time::uptime_us() - play_start_us_ 的结果保存到左侧 elapsed_acc_us_ +。
    elapsed_acc_us_ += ::app::common::time::uptime_us() - play_start_us_;
    // 判断条件：elapsed_acc_us_ < 0 是否成立。
    if (elapsed_acc_us_ < 0)
        // 赋值：把右侧 0 的结果保存到左侧 elapsed_acc_us_。
        elapsed_acc_us_ = 0;
    // 赋值：把右侧 0 的结果保存到左侧 play_start_us_。
    play_start_us_ = 0;
}

// 定义或声明函数 MusicPlayer::unfreeze_elapsed：参数在括号里，返回类型写在函数名前面。
void MusicPlayer::unfreeze_elapsed()
{
    // 赋值：把右侧 ::app::common::time::uptime_us() 的结果保存到左侧 play_start_us_。
    play_start_us_ = ::app::common::time::uptime_us();
}

// 定义或声明函数 MusicPlayer::elapsed_sec：参数在括号里，返回类型写在函数名前面。
uint16_t MusicPlayer::elapsed_sec() const
{
    // 定义变量 int64_t us 并初始化为 elapsed_acc_us_。
    int64_t us = elapsed_acc_us_;
    // 判断条件：play_start_us_ > 0 是否成立。
    if (play_start_us_ > 0)
        // 赋值：把右侧 ::app::common::time::uptime_us() - play_start_us_ 的结果保存到左侧 us +。
        us += ::app::common::time::uptime_us() - play_start_us_;
    // 判断条件：us < 0 是否成立。
    if (us < 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    // 返回 static_cast<uint16_t>(us / 1000000LL) 给调用者，表示当前函数的执行结果。
    return static_cast<uint16_t>(us / 1000000LL);
}

// 定义或声明函数 MusicPlayer::hint_duration：参数在括号里，返回类型写在函数名前面。
void MusicPlayer::hint_duration(uint16_t sec)
{
    // 曲库扫盘结果优先；已有值不覆盖（避免被错误 bitrate 估算冲掉）
    if (sec == 0 || duration_sec_ != 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 sec 的结果保存到左侧 duration_sec_。
    duration_sec_ = sec;
}

// 定义或声明函数 MusicPlayer::update_duration_from_bitrate：参数在括号里，返回类型写在函数名前面。
void MusicPlayer::update_duration_from_bitrate(uint32_t bitrate)
{
    // 扫盘/hint 已有可靠时长时不再用 bitrate 估算覆盖。
    // 部分解码器把 bitrate 报成 kbps（如 128），若当 bps 去除会得到
    // 天文数字并钳到 0xFFFF → UI 显示成 1092:15。
    if (duration_sec_ != 0 || bitrate == 0 || file_size_ == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 定义变量 uint32_t bps 并初始化为 bitrate。
    uint32_t bps = bitrate;
    // 判断条件：bps < 10000U 是否成立。
    if (bps < 10000U) // 典型 MP3 码率 32~320 kbps，小于此阈值视为 kbps
        // 赋值：把右侧 1000U 的结果保存到左侧 bps *。
        bps *= 1000U;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint64_t bits = static_cast<uint64_t>(file_size_) * 8ULL;
    // 定义变量 uint32_t sec 并初始化为 static_cast<uint32_t>(bits / bps)。
    uint32_t sec = static_cast<uint32_t>(bits / bps);
    // 判断条件：sec == 0 || sec >= 0xFFFFU 是否成立。
    if (sec == 0 || sec >= 0xFFFFU)
        // 提前结束当前函数，不返回具体值。
        return; // 饱和值不可信
    // 赋值：把右侧 static_cast<uint16_t>(sec) 的结果保存到左侧 duration_sec_。
    duration_sec_ = static_cast<uint16_t>(sec);
}

// 定义或声明函数 MusicPlayer::wait_if_paused：参数在括号里，返回类型写在函数名前面。
void MusicPlayer::wait_if_paused()
{
    // 判断条件：!pause_req_ || stop_req_ 是否成立。
    if (!pause_req_ || stop_req_)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 调用 AudioCodec::get_instance().codec().enable_output(false);，执行这个函数并处理它的返回值或状态变化。
    AudioCodec::get_instance().codec().enable_output(false);
    // 调用 freeze_elapsed();，执行这个函数并处理它的返回值或状态变化。
    freeze_elapsed();
    // 调用 set_state(MusicPlayState::Paused);，执行这个函数并处理它的返回值或状态变化。
    set_state(MusicPlayState::Paused);
    // 调用 post_progress_hint();，执行这个函数并处理它的返回值或状态变化。
    post_progress_hint();

    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (pause_req_ && !stop_req_)
        // 调用 app::sys::task::TaskMgr::delay_ms(20)：调用 vTaskDelay()，这是让出 CPU 的阻塞延时。一千毫秒延时期间，任务不占 CPU，调度器可以运行别人。。
        ::app::sys::task::TaskMgr::delay_ms(20);

    // 判断条件：stop_req_ 是否成立。
    if (stop_req_)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 调用 AudioCodec::get_instance().codec().enable_output(true);，执行这个函数并处理它的返回值或状态变化。
    AudioCodec::get_instance().codec().enable_output(true);
    // 调用 AudioCodec::get_instance().apply_system_volume();，执行这个函数并处理它的返回值或状态变化。
    AudioCodec::get_instance().apply_system_volume();
    // 调用 unfreeze_elapsed();，执行这个函数并处理它的返回值或状态变化。
    unfreeze_elapsed();
    // 调用 set_state(MusicPlayState::Playing);，执行这个函数并处理它的返回值或状态变化。
    set_state(MusicPlayState::Playing);
    // 调用 post_progress_hint();，执行这个函数并处理它的返回值或状态变化。
    post_progress_hint();
}

// 函数定义/声明：调用 MusicPlayer::stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
void MusicPlayer::stop()
{
    // 赋值：把右侧 false 的结果保存到左侧 pause_req_。
    pause_req_ = false;
    // 赋值：把右侧 true 的结果保存到左侧 stop_req_。
    stop_req_ = true;

    // 易变变量：告诉编译器该值可能被中断或硬件改变，不能随意优化读取。
    volatile MusicPlayState* s = &state_;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int64_t t0 = ::app::common::time::uptime_ms();
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (*s == MusicPlayState::Loading || *s == MusicPlayState::Playing ||
           *s == MusicPlayState::Paused)
    {
        // Paused 时任务仍活着，等它退出到 Idle/Error
        if (*s == MusicPlayState::Paused && play_task_alive_)
        {
            // stop_req_ 已置位，wait_if_paused 会退出；主循环也会退出
        }
        // 判断条件：(::app::common::time::uptime_ms() - t0) > static_cast<int64_t>(kStopWaitMs) 是否成立。
        if ((::app::common::time::uptime_ms() - t0) > static_cast<int64_t>(kStopWaitMs))
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 调用 app::sys::task::TaskMgr::delay_ms(20)：调用 vTaskDelay()，这是让出 CPU 的阻塞延时。一千毫秒延时期间，任务不占 CPU，调度器可以运行别人。。
        ::app::sys::task::TaskMgr::delay_ms(20);
    }

    // 赋值：把右侧 false 的结果保存到左侧 stop_req_。
    stop_req_ = false;
    // 赋值：把右侧 false 的结果保存到左侧 pause_req_。
    pause_req_ = false;
    // 赋值：把右侧 MusicPlayState::Idle 的结果保存到左侧 state_。
    state_ = MusicPlayState::Idle;
    // 赋值：把右侧 0 的结果保存到左侧 play_start_us_。
    play_start_us_ = 0;
    // 赋值：把右侧 0 的结果保存到左侧 elapsed_acc_us_。
    elapsed_acc_us_ = 0;
    // 调用 clear_play_task();，执行这个函数并处理它的返回值或状态变化。
    clear_play_task();
    // 调用 AudioCodec::get_instance().codec().enable_output(false);，执行这个函数并处理它的返回值或状态变化。
    AudioCodec::get_instance().codec().enable_output(false);
}

// 定义或声明函数 MusicPlayer::play：参数在括号里，返回类型写在函数名前面。
bool MusicPlayer::play(const char* path)
{
    // 判断条件：path == nullptr || path[0] == '\0' 是否成立。
    if (path == nullptr || path[0] == '\0')
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 调用 stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
    stop();

    // 调用 std::strncpy(path_, path, sizeof(path_) - 1);，执行这个函数并处理它的返回值或状态变化。
    std::strncpy(path_, path, sizeof(path_) - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 path_[sizeof(path_) - 1]。
    path_[sizeof(path_) - 1] = '\0';

    // 赋值：把右侧 false 的结果保存到左侧 stop_req_。
    stop_req_ = false;
    // 赋值：把右侧 false 的结果保存到左侧 pause_req_。
    pause_req_ = false;
    // 赋值：把右侧 0 的结果保存到左侧 file_size_。
    file_size_ = 0;
    // 赋值：把右侧 0 的结果保存到左侧 duration_sec_。
    duration_sec_ = 0;
    // 赋值：把右侧 0 的结果保存到左侧 play_start_us_。
    play_start_us_ = 0;
    // 赋值：把右侧 0 的结果保存到左侧 elapsed_acc_us_。
    elapsed_acc_us_ = 0;
    // 调用 set_state(MusicPlayState::Loading);，执行这个函数并处理它的返回值或状态变化。
    set_state(MusicPlayState::Loading);

    // 判断条件：!AudioCodec::get_instance().ready() 是否成立。
    if (!AudioCodec::get_instance().ready())
    {
        // 赋值：把右侧 5 的结果保存到左侧 last_error_。
        last_error_ = 5;
        // 调用 set_state(MusicPlayState::Error);，执行这个函数并处理它的返回值或状态变化。
        set_state(MusicPlayState::Error);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 判断条件：!ensure_decoders() 是否成立。
    if (!ensure_decoders())
    {
        // 赋值：把右侧 6 的结果保存到左侧 last_error_。
        last_error_ = 6;
        // 调用 set_state(MusicPlayState::Error);，执行这个函数并处理它的返回值或状态变化。
        set_state(MusicPlayState::Error);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 执行语句：::app::sys::task::Cfg cfg;；它会更新一次变量、对象或程序状态。
    ::app::sys::task::Cfg cfg;
    // 赋值：把右侧 "music_play" 的结果保存到左侧 cfg.name。
    cfg.name = "music_play";
    // 赋值：把右侧 kPlayStack 的结果保存到左侧 cfg.stack_size。
    cfg.stack_size = kPlayStack;
    // 赋值：把右侧 ::app::sys::task::Priority::NORMAL 的结果保存到左侧 cfg.priority。
    cfg.priority = ::app::sys::task::Priority::NORMAL;
    // 赋值：把右侧 true 的结果保存到左侧 cfg.use_psram。
    cfg.use_psram = true;

    // 执行这条代码：play_task_ = std::make_unique<::app::sys::task::Task>(。它会更新变量、对象状态或调用链。
    play_task_ = std::make_unique<::app::sys::task::Task>(
        // 开始代码块：下面是这个函数、分支或循环的主体。
        [](void* arg) {
            // 赋值：把右侧 static_cast<MusicPlayer*>(arg) 的结果保存到左侧 auto* self。
            auto* self = static_cast<MusicPlayer*>(arg);
            // 判断条件：self != nullptr 是否成立。
            if (self != nullptr)
                // 访问对象成员：self->run_playback(self->path_);；读取或修改这个对象里的成员状态。
                self->run_playback(self->path_);
        // 结束上一行的 lambda 或初始化列表：把这段匿名代码/数据交给调用者。
        },
        // 继续上一行的表达式或参数列表：cfg, this);。
        cfg, this);

    // 判断条件：!play_task_->start() 是否成立。
    if (!play_task_->start())
    {
        // 调用 EBOOK_LOGE(TAG, "music_play task start fail");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGE(TAG, "music_play task start fail");
        // 赋值：把右侧 false 的结果保存到左侧 play_task_alive_。
        play_task_alive_ = false;
        // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
        play_task_.reset();
        // 赋值：把右侧 3 的结果保存到左侧 last_error_。
        last_error_ = 3;
        // 调用 set_state(MusicPlayState::Error);，执行这个函数并处理它的返回值或状态变化。
        set_state(MusicPlayState::Error);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 赋值：把右侧 true 的结果保存到左侧 play_task_alive_。
    play_task_alive_ = true;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 MusicPlayer::pause：参数在括号里，返回类型写在函数名前面。
void MusicPlayer::pause()
{
    // 判断条件：state_ != MusicPlayState::Playing 是否成立。
    if (state_ != MusicPlayState::Playing)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 true 的结果保存到左侧 pause_req_。
    pause_req_ = true;
    // 函数调用：(void)wait_state(state_, MusicPlayState::Paused);；执行该函数并处理它的返回值或副作用。
    (void)wait_state(state_, MusicPlayState::Paused);
}

// 函数定义/声明：调用 MusicPlayer::resume()：这些是对 FreeRTOS API 的直接包装： suspend() 暂停任务。 resume() 恢复。 setpriority() 动态改优先级。 getpriority() 读取当前优先级。。
void MusicPlayer::resume()
{
    // 判断条件：state_ != MusicPlayState::Paused || path_[0] == '\0' 是否成立。
    if (state_ != MusicPlayState::Paused || path_[0] == '\0')
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：!play_task_alive_ 是否成立。
    if (!play_task_alive_)
    {
        // 函数调用：(void)play(path_);；执行该函数并处理它的返回值或副作用。
        (void)play(path_);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 赋值：把右侧 false 的结果保存到左侧 pause_req_。
    pause_req_ = false;
    // 函数调用：(void)wait_state(state_, MusicPlayState::Playing);；执行该函数并处理它的返回值或副作用。
    (void)wait_state(state_, MusicPlayState::Playing);
}

// 定义或声明函数 MusicPlayer::try_start_output：参数在括号里，返回类型写在函数名前面。
bool MusicPlayer::try_start_output(esp_audio_simple_dec_handle_t dec)
{
    // 定义变量 esp_audio_simple_dec_info_t info 并初始化为 {}。
    esp_audio_simple_dec_info_t info = {};
    // 判断条件：esp_audio_simple_dec_get_info(dec, &info) != ESP_AUDIO_ERR_OK || info.sample_rate == 0 是否成立。
    if (esp_audio_simple_dec_get_info(dec, &info) != ESP_AUDIO_ERR_OK || info.sample_rate == 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 调用 EBOOK_LOGD(TAG, "PCM %u Hz ch=%u br=%u", static_cast<unsigned>(info.sample_rate),，执行这个函数并处理它的返回值或状态变化。
    EBOOK_LOGD(TAG, "PCM %u Hz ch=%u br=%u", static_cast<unsigned>(info.sample_rate),
               // 继续上一行的表达式或参数列表：static_cast<unsigned>(info.channel), static_cast<unsigned>(info.bitrate));。
               static_cast<unsigned>(info.channel), static_cast<unsigned>(info.bitrate));

    // 调用 update_duration_from_bitrate(info.bitrate);，执行这个函数并处理它的返回值或状态变化。
    update_duration_from_bitrate(info.bitrate);

    // 访问对象成员：(void)AudioCodec::get_instance().ensure_pcm_format(info.sample_rate, info.channel);；读取或修改这个对象里的成员状态。
    (void)AudioCodec::get_instance().ensure_pcm_format(info.sample_rate, info.channel);
    // 调用 AudioCodec::get_instance().codec().enable_output(true);，执行这个函数并处理它的返回值或状态变化。
    AudioCodec::get_instance().codec().enable_output(true);
    // 调用 AudioCodec::get_instance().apply_system_volume();，执行这个函数并处理它的返回值或状态变化。
    AudioCodec::get_instance().apply_system_volume();

    // 赋值：把右侧 0 的结果保存到左侧 elapsed_acc_us_。
    elapsed_acc_us_ = 0;
    // 调用 unfreeze_elapsed();，执行这个函数并处理它的返回值或状态变化。
    unfreeze_elapsed();
    // 调用 set_state(MusicPlayState::Playing);，执行这个函数并处理它的返回值或状态变化。
    set_state(MusicPlayState::Playing);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数调用：bool MusicPlayer::decode_raw(esp_audio_simple_dec_handle_t dec, esp_audio_simple_dec_raw_t* raw,；执行该函数并处理它的返回值或副作用。
bool MusicPlayer::decode_raw(esp_audio_simple_dec_handle_t dec, esp_audio_simple_dec_raw_t* raw,
                             // 继续上一行的表达式或参数列表：uint8_t** out_buf, size_t* out_cap, bool* started, bool* failed)。
                             uint8_t** out_buf, size_t* out_cap, bool* started, bool* failed)
{
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (raw->len > 0 && !stop_req_)
    {
        // 调用 wait_if_paused();，执行这个函数并处理它的返回值或状态变化。
        wait_if_paused();
        // 判断条件：stop_req_ 是否成立。
        if (stop_req_)
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 声明并默认初始化对象：esp_audio_simple_dec_out_t out_frame{}。
        esp_audio_simple_dec_out_t out_frame{};
        // 赋值：把右侧 *out_buf 的结果保存到左侧 out_frame.buffer。
        out_frame.buffer = *out_buf;
        // 赋值：把右侧 static_cast<uint32_t>(*out_cap) 的结果保存到左侧 out_frame.len。
        out_frame.len = static_cast<uint32_t>(*out_cap);

        // 只读约束：这个值或对象在当前位置不能被修改。
        const esp_audio_err_t ret = esp_audio_simple_dec_process(dec, raw, &out_frame);
        // 判断条件：ret == ESP_AUDIO_ERR_BUFF_NOT_ENOUGH 是否成立。
        if (ret == ESP_AUDIO_ERR_BUFF_NOT_ENOUGH)
        {
            // 赋值：把右侧 static_cast<uint8_t*>(realloc_psram(*out_buf, out_frame.needed_size)) 的结果保存到左侧 auto* nb。
            auto* nb = static_cast<uint8_t*>(realloc_psram(*out_buf, out_frame.needed_size));
            // 判断条件：nb == nullptr 是否成立。
            if (nb == nullptr)
            {
                // 调用 EBOOK_LOGE(TAG, "out buf realloc %u fail",，执行这个函数并处理它的返回值或状态变化。
                EBOOK_LOGE(TAG, "out buf realloc %u fail",
                           // 继续上一行的表达式或参数列表：static_cast<unsigned>(out_frame.needed_size));。
                           static_cast<unsigned>(out_frame.needed_size));
                *failed = true;
                // 返回 false 给调用者，表示当前函数的执行结果。
                return false;
            }
            *out_buf = nb;
            *out_cap = out_frame.needed_size;
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        }
        // 判断条件：ret != ESP_AUDIO_ERR_OK 是否成立。
        if (ret != ESP_AUDIO_ERR_OK)
        {
            // 调用 EBOOK_LOGE(TAG, "decode failed %d", static_cast<int>(ret));，执行这个函数并处理它的返回值或状态变化。
            EBOOK_LOGE(TAG, "decode failed %d", static_cast<int>(ret));
            *failed = true;
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }

        // 判断条件：out_frame.decoded_size > 0 是否成立。
        if (out_frame.decoded_size > 0)
        {
            // 判断条件：!*started && try_start_output(dec) 是否成立。
            if (!*started && try_start_output(dec))
                *started = true;

            // 调用 wait_if_paused();，执行这个函数并处理它的返回值或状态变化。
            wait_if_paused();
            // 判断条件：stop_req_ 是否成立。
            if (stop_req_)
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;

            // 访问对象成员：(void)write_pcm(out_frame.buffer, out_frame.decoded_size, stop_req_);；读取或修改这个对象里的成员状态。
            (void)write_pcm(out_frame.buffer, out_frame.decoded_size, stop_req_);
        }

        // 赋值：把右侧 raw->consumed 的结果保存到左侧 raw->len -。
        raw->len -= raw->consumed;
        // 赋值：把右侧 raw->consumed 的结果保存到左侧 raw->buffer +。
        raw->buffer += raw->consumed;
    }
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 MusicPlayer::run_playback：参数在括号里，返回类型写在函数名前面。
void MusicPlayer::run_playback(const char* path)
{
    // 赋值：把右侧 true 的结果保存到左侧 play_task_alive_。
    play_task_alive_ = true;
    // 赋值：把右侧 0 的结果保存到左侧 last_error_。
    last_error_ = 0;
    // duration_sec_ 由 play()/hint_duration 管理，此处勿清零，否则会冲掉扫盘预填
    play_start_us_ = 0;
    // 赋值：把右侧 0 的结果保存到左侧 elapsed_acc_us_。
    elapsed_acc_us_ = 0;
    // 赋值：把右侧 0 的结果保存到左侧 file_size_。
    file_size_ = 0;

    // 判断条件：!AudioCodec::get_instance().ready() 是否成立。
    if (!AudioCodec::get_instance().ready())
    {
        // 赋值：把右侧 5 的结果保存到左侧 last_error_。
        last_error_ = 5;
        // 调用 set_state(MusicPlayState::Error);，执行这个函数并处理它的返回值或状态变化。
        set_state(MusicPlayState::Error);
        // 赋值：把右侧 false 的结果保存到左侧 play_task_alive_。
        play_task_alive_ = false;
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const esp_audio_simple_dec_type_t ftype = dec_type_from_path(path);
    // 判断条件：ftype == ESP_AUDIO_SIMPLE_DEC_TYPE_NONE 是否成立。
    if (ftype == ESP_AUDIO_SIMPLE_DEC_TYPE_NONE)
    {
        // 调用 EBOOK_LOGE(TAG, "unsupported format: %s", path);，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGE(TAG, "unsupported format: %s", path);
        // 赋值：把右侧 4 的结果保存到左侧 last_error_。
        last_error_ = 4;
        // 调用 set_state(MusicPlayState::Error);，执行这个函数并处理它的返回值或状态变化。
        set_state(MusicPlayState::Error);
        // 赋值：把右侧 false 的结果保存到左侧 play_task_alive_。
        play_task_alive_ = false;
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：!ensure_decoders() 是否成立。
    if (!ensure_decoders())
    {
        // 赋值：把右侧 6 的结果保存到左侧 last_error_。
        last_error_ = 6;
        // 调用 set_state(MusicPlayState::Error);，执行这个函数并处理它的返回值或状态变化。
        set_state(MusicPlayState::Error);
        // 赋值：把右侧 false 的结果保存到左侧 play_task_alive_。
        play_task_alive_ = false;
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 赋值：把右侧 std::fopen(path, "rb") 的结果保存到左侧 FILE* fp。
    FILE* fp = std::fopen(path, "rb");
    // 判断条件：fp == nullptr 是否成立。
    if (fp == nullptr)
    {
        // 调用 EBOOK_LOGE(TAG, "fopen fail: %s", path);，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGE(TAG, "fopen fail: %s", path);
        // 赋值：把右侧 2 的结果保存到左侧 last_error_。
        last_error_ = 2;
        // 调用 set_state(MusicPlayState::Error);，执行这个函数并处理它的返回值或状态变化。
        set_state(MusicPlayState::Error);
        // 赋值：把右侧 false 的结果保存到左侧 play_task_alive_。
        play_task_alive_ = false;
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：std::fseek(fp, 0, SEEK_END) == 0 是否成立。
    if (std::fseek(fp, 0, SEEK_END) == 0)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const long sz = std::ftell(fp);
        // 判断条件：sz > 0 是否成立。
        if (sz > 0)
            // 赋值：把右侧 static_cast<uint32_t>(sz) 的结果保存到左侧 file_size_。
            file_size_ = static_cast<uint32_t>(sz);
        // 调用 std::rewind(fp);，执行这个函数并处理它的返回值或状态变化。
        std::rewind(fp);
    }

    // 定义变量 esp_audio_simple_dec_handle_t dec 并初始化为 nullptr。
    esp_audio_simple_dec_handle_t dec = nullptr;
    // 开始代码块：下面是这个函数、分支或循环的主体。
    esp_audio_simple_dec_cfg_t dec_cfg = {
        // 执行这条代码：.dec_type = ftype,。它会更新变量、对象状态或调用链。
        .dec_type = ftype,
        // 继续上一行的表达式或参数列表：.dec_cfg = nullptr,。
        .dec_cfg = nullptr,
        // 继续上一行的表达式或参数列表：.cfg_size = 0,。
        .cfg_size = 0,
        // 继续上一行的表达式或参数列表：.use_frame_dec = false,。
        .use_frame_dec = false,
    };
    // 判断条件：esp_audio_simple_dec_open(&dec_cfg, &dec) != ESP_AUDIO_ERR_OK 是否成立。
    if (esp_audio_simple_dec_open(&dec_cfg, &dec) != ESP_AUDIO_ERR_OK)
    {
        // 调用 std::fclose(fp);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(fp);
        // 调用 EBOOK_LOGE(TAG, "dec open fail type=%d path=%s", static_cast<int>(ftype), path);，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGE(TAG, "dec open fail type=%d path=%s", static_cast<int>(ftype), path);
        // 赋值：把右侧 4 的结果保存到左侧 last_error_。
        last_error_ = 4;
        // 调用 set_state(MusicPlayState::Error);，执行这个函数并处理它的返回值或状态变化。
        set_state(MusicPlayState::Error);
        // 赋值：把右侧 false 的结果保存到左侧 play_task_alive_。
        play_task_alive_ = false;
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 赋值：把右侧 static_cast<uint8_t*>(alloc_psram(kReadChunk)) 的结果保存到左侧 auto* in_buf。
    auto* in_buf = static_cast<uint8_t*>(alloc_psram(kReadChunk));
    // 赋值：把右侧 static_cast<uint8_t*>(alloc_psram(kOutBufInit)) 的结果保存到左侧 auto* out_buf。
    auto* out_buf = static_cast<uint8_t*>(alloc_psram(kOutBufInit));
    // 定义变量 size_t out_cap 并初始化为 kOutBufInit。
    size_t out_cap = kOutBufInit;

    // 开始代码块：下面是这个函数、分支或循环的主体。
    auto cleanup = [&]() {
        // 调用 esp_audio_simple_dec_close(dec);，执行这个函数并处理它的返回值或状态变化。
        esp_audio_simple_dec_close(dec);
        // 调用 std::fclose(fp);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(fp);
        // 调用 heap_caps_free(in_buf);，执行这个函数并处理它的返回值或状态变化。
        heap_caps_free(in_buf);
        // 调用 heap_caps_free(out_buf);，执行这个函数并处理它的返回值或状态变化。
        heap_caps_free(out_buf);
        // 调用 AudioCodec::get_instance().codec().enable_output(false);，执行这个函数并处理它的返回值或状态变化。
        AudioCodec::get_instance().codec().enable_output(false);
    };

    // 判断条件：in_buf == nullptr || out_buf == nullptr 是否成立。
    if (in_buf == nullptr || out_buf == nullptr)
    {
        // 调用 EBOOK_LOGE(TAG, "decode buf alloc fail in=%p out=%p", static_cast<void*>(in_buf),，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGE(TAG, "decode buf alloc fail in=%p out=%p", static_cast<void*>(in_buf),
                   // 继续上一行的表达式或参数列表：static_cast<void*>(out_buf));。
                   static_cast<void*>(out_buf));
        // 调用 cleanup();，执行这个函数并处理它的返回值或状态变化。
        cleanup();
        // 赋值：把右侧 3 的结果保存到左侧 last_error_。
        last_error_ = 3;
        // 调用 set_state(MusicPlayState::Error);，执行这个函数并处理它的返回值或状态变化。
        set_state(MusicPlayState::Error);
        // 赋值：把右侧 false 的结果保存到左侧 play_task_alive_。
        play_task_alive_ = false;
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 定义变量 bool failed 并初始化为 false, started = false。
    bool failed = false, started = false;
    // 定义变量 uint32_t last_prog_ms 并初始化为 0。
    uint32_t last_prog_ms = 0;

    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (!stop_req_ && !failed)
    {
        // 调用 wait_if_paused();，执行这个函数并处理它的返回值或状态变化。
        wait_if_paused();
        // 判断条件：stop_req_ 是否成立。
        if (stop_req_)
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;

        // 只读约束：这个值或对象在当前位置不能被修改。
        const size_t nread = std::fread(in_buf, 1, kReadChunk, fp);
        // 只读约束：这个值或对象在当前位置不能被修改。
        const bool eof = (nread < kReadChunk) || (std::feof(fp) != 0);

        // 判断条件：nread > 0 是否成立。
        if (nread > 0)
        {
            // 声明并默认初始化对象：esp_audio_simple_dec_raw_t raw{}。
            esp_audio_simple_dec_raw_t raw{};
            // 赋值：把右侧 in_buf 的结果保存到左侧 raw.buffer。
            raw.buffer = in_buf;
            // 赋值：把右侧 static_cast<uint32_t>(nread) 的结果保存到左侧 raw.len。
            raw.len = static_cast<uint32_t>(nread);
            // 赋值：把右侧 eof 的结果保存到左侧 raw.eos。
            raw.eos = eof;
            // 赋值：把右侧 0 的结果保存到左侧 raw.consumed。
            raw.consumed = 0;
            // 赋值：把右侧 ESP_AUDIO_SIMPLE_DEC_RECOVERY_NONE 的结果保存到左侧 raw.frame_recover。
            raw.frame_recover = ESP_AUDIO_SIMPLE_DEC_RECOVERY_NONE;
            // 判断条件：!decode_raw(dec, &raw, &out_buf, &out_cap, &started, &failed) 是否成立。
            if (!decode_raw(dec, &raw, &out_buf, &out_cap, &started, &failed))
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
        }

        // 判断条件：eof 是否成立。
        if (eof)
        {
            // 声明并默认初始化对象：esp_audio_simple_dec_raw_t flush{}。
            esp_audio_simple_dec_raw_t flush{};
            // 赋值：把右侧 true 的结果保存到左侧 flush.eos。
            flush.eos = true;
            // 赋值：把右侧 ESP_AUDIO_SIMPLE_DEC_RECOVERY_NONE 的结果保存到左侧 flush.frame_recover。
            flush.frame_recover = ESP_AUDIO_SIMPLE_DEC_RECOVERY_NONE;
            // 函数调用：(void)decode_raw(dec, &flush, &out_buf, &out_cap, &started, &failed);；执行该函数并处理它的返回值或副作用。
            (void)decode_raw(dec, &flush, &out_buf, &out_cap, &started, &failed);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        }

        // 判断条件：nread == 0 是否成立。
        if (nread == 0)
        {
            // 赋值：把右侧 true 的结果保存到左侧 failed。
            failed = true;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        }

        // 判断条件：started 是否成立。
        if (started)
        {
            // 只读约束：这个值或对象在当前位置不能被修改。
            const uint32_t now_ms = static_cast<uint32_t>(::app::common::time::uptime_ms());
            // 判断条件：now_ms - last_prog_ms >= kProgressMs 是否成立。
            if (now_ms - last_prog_ms >= kProgressMs)
            {
                // 赋值：把右侧 now_ms 的结果保存到左侧 last_prog_ms。
                last_prog_ms = now_ms;
                // 调用 post_progress_hint();，执行这个函数并处理它的返回值或状态变化。
                post_progress_hint();
            }
        }

        // 调用 app::sys::task::TaskMgr::delay_ms(1)：调用 vTaskDelay()，这是让出 CPU 的阻塞延时。一千毫秒延时期间，任务不占 CPU，调度器可以运行别人。。
        ::app::sys::task::TaskMgr::delay_ms(1);
    }

    // 调用 cleanup();，执行这个函数并处理它的返回值或状态变化。
    cleanup();

    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool user_stop = stop_req_;
    // 赋值：把右侧 false 的结果保存到左侧 stop_req_。
    stop_req_ = false;
    // 赋值：把右侧 false 的结果保存到左侧 pause_req_。
    pause_req_ = false;
    // 赋值：把右侧 0 的结果保存到左侧 play_start_us_。
    play_start_us_ = 0;
    // 播完保留 elapsed；下一首 play() 会清零
    if (!user_stop && started && !failed)
        // 赋值：把右侧 static_cast<int64_t>(duration_sec_) * 1000000LL 的结果保存到左侧 elapsed_acc_us_。
        elapsed_acc_us_ = static_cast<int64_t>(duration_sec_) * 1000000LL;

    // 判断条件：failed || (!started && !user_stop) 是否成立。
    if (failed || (!started && !user_stop))
    {
        // 赋值：把右侧 4 的结果保存到左侧 last_error_。
        last_error_ = 4;
        // 调用 set_state(MusicPlayState::Error);，执行这个函数并处理它的返回值或状态变化。
        set_state(MusicPlayState::Error);
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
        // 调用 set_state(MusicPlayState::Idle);，执行这个函数并处理它的返回值或状态变化。
        set_state(MusicPlayState::Idle);

    // 赋值：把右侧 false 的结果保存到左侧 play_task_alive_。
    play_task_alive_ = false;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::platform
