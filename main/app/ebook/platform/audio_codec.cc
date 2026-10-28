/**
 * [初学者导读] 文件：main/app/ebook/platform/audio_codec.cc
 * 作用：把系统音量、静音和音频格式应用到 ES8311/I2S 通路。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：阻塞工作放独立任务，周期小检查放 Housekeeper。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "platform/audio_codec.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/system_state.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "es8311_codec.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "AudioCodec";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::platform {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ::app::bsp::driver::audio::es8311::Config;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ::app::bsp::driver::audio::es8311::Es8311Codec;

// 执行语句：Es8311Codec g_codec;；它会更新一次变量、对象或程序状态。
Es8311Codec g_codec;

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 AudioCodec::get_instance：参数在括号里，返回类型写在函数名前面。
AudioCodec& AudioCodec::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static AudioCodec s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义或声明函数 AudioCodec::codec：参数在括号里，返回类型写在函数名前面。
::app::bsp::driver::audio::es8311::Es8311Codec& AudioCodec::codec()
{
    // 返回 g_codec 给调用者，表示当前函数的执行结果。
    return g_codec;
}

// 定义或声明函数 AudioCodec::init：参数在括号里，返回类型写在函数名前面。
bool AudioCodec::init(i2c_master_bus_handle_t bus, uint32_t sample_rate_hz)
{
    // 判断条件：bus == nullptr 是否成立。
    if (bus == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 赋值：把右侧 bus 的结果保存到左侧 bus_。
    bus_ = bus;
    // 返回 ensure_pcm_format(sample_rate_hz, 1) 给调用者，表示当前函数的执行结果。
    return ensure_pcm_format(sample_rate_hz, 1);
}

// 定义或声明函数 AudioCodec::deinit：参数在括号里，返回类型写在函数名前面。
void AudioCodec::deinit()
{
    // 判断条件：!ready_ 是否成立。
    if (!ready_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 访问对象成员：g_codec.enable_output(false);；读取或修改这个对象里的成员状态。
    g_codec.enable_output(false);
    // 调用 deinit()：创建和销毁队列。
    g_codec.deinit();
    // 赋值：把右侧 false 的结果保存到左侧 ready_。
    ready_ = false;
    // 赋值：把右侧 nullptr 的结果保存到左侧 bus_。
    bus_ = nullptr;
    // 赋值：把右侧 16000 的结果保存到左侧 sample_rate_。
    sample_rate_ = 16000;
    // 赋值：把右侧 1 的结果保存到左侧 channels_。
    channels_ = 1;
}

// 定义或声明函数 AudioCodec::ensure_sample_rate：参数在括号里，返回类型写在函数名前面。
bool AudioCodec::ensure_sample_rate(uint32_t sample_rate_hz)
{
    // 返回 ensure_pcm_format(sample_rate_hz, 1) 给调用者，表示当前函数的执行结果。
    return ensure_pcm_format(sample_rate_hz, 1);
}

// 定义或声明函数 AudioCodec::ensure_pcm_format：参数在括号里，返回类型写在函数名前面。
bool AudioCodec::ensure_pcm_format(uint32_t sample_rate_hz, uint8_t channels)
{
    // 函数调用：std::lock_guard<std::mutex> lk(mtx_);；执行该函数并处理它的返回值或副作用。
    std::lock_guard<std::mutex> lk(mtx_);
    // 判断条件：bus_ == nullptr 是否成立。
    if (bus_ == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t ch = (channels >= 2) ? 2 : 1;
    // 判断条件：ready_ && sample_rate_hz == sample_rate_ && ch == channels_ 是否成立。
    if (ready_ && sample_rate_hz == sample_rate_ && ch == channels_)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;

    // 判断条件：ready_ 是否成立。
    if (ready_)
    {
        // 访问对象成员：g_codec.enable_output(false);；读取或修改这个对象里的成员状态。
        g_codec.enable_output(false);
        // 调用 deinit()：创建和销毁队列。
        g_codec.deinit();
        // 赋值：把右侧 false 的结果保存到左侧 ready_。
        ready_ = false;
    }

    // 声明并默认初始化对象：Config cfg{}。
    Config cfg{};
    // 赋值：把右侧 sample_rate_hz 的结果保存到左侧 cfg.sample_rate。
    cfg.sample_rate = sample_rate_hz;
    // 赋值：把右侧 ch 的结果保存到左侧 cfg.channels。
    cfg.channels = ch;
    // 判断条件：!g_codec.init(bus_, &cfg) 是否成立。
    if (!g_codec.init(bus_, &cfg))
    {
        // 调用 EBOOK_LOGW(TAG, "codec init fail %u Hz ch=%u", static_cast<unsigned>(sample_rate_hz),，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "codec init fail %u Hz ch=%u", static_cast<unsigned>(sample_rate_hz),
                   // 继续上一行的表达式或参数列表：static_cast<unsigned>(ch));。
                   static_cast<unsigned>(ch));
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 sample_rate_hz 的结果保存到左侧 sample_rate_。
    sample_rate_ = sample_rate_hz;
    // 赋值：把右侧 ch 的结果保存到左侧 channels_。
    channels_ = ch;
    // 赋值：把右侧 true 的结果保存到左侧 ready_。
    ready_ = true;
    // 调用 apply_system_volume();，执行这个函数并处理它的返回值或状态变化。
    apply_system_volume();
    // 调用 EBOOK_LOGD(TAG, "ready %u Hz ch=%u", static_cast<unsigned>(sample_rate_hz),，执行这个函数并处理它的返回值或状态变化。
    EBOOK_LOGD(TAG, "ready %u Hz ch=%u", static_cast<unsigned>(sample_rate_hz),
               // 继续上一行的表达式或参数列表：static_cast<unsigned>(ch));。
               static_cast<unsigned>(ch));
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 AudioCodec::apply_system_volume：参数在括号里，返回类型写在函数名前面。
void AudioCodec::apply_system_volume()
{
    // 判断条件：!ready_ 是否成立。
    if (!ready_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const auto& st = data::SystemState::get_instance();
    // 访问对象成员：g_codec.set_output_volume(st.mute() ? 0 : st.volume());；读取或修改这个对象里的成员状态。
    g_codec.set_output_volume(st.mute() ? 0 : st.volume());
    // 判断条件：g_codec.is_output_enabled() 是否成立。
    if (g_codec.is_output_enabled())
        // 访问对象成员：g_codec.enable_output(!st.mute());；读取或修改这个对象里的成员状态。
        g_codec.enable_output(!st.mute());
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::platform
