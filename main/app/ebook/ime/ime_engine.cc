/**
 * [初学者导读] 文件：main/app/ebook/ime/ime_engine.cc
 * 作用：实现拼音输入法候选查询和候选字管理。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：上层通过 Router、UiBus 和 SystemState 协作，不直接操作硬件。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ime/ime_engine.hpp"

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
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "zh_pinyin_decoder.h"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "ImeEngine";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::ime {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {
// 固定参数 kBinRelPath = "/ime/zh_pinyin.bin"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kBinRelPath = "/ime/zh_pinyin.bin";
// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 ImeEngine::get_instance：参数在括号里，返回类型写在函数名前面。
ImeEngine& ImeEngine::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static ImeEngine s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义或声明函数 ImeEngine::init：参数在括号里，返回类型写在函数名前面。
bool ImeEngine::init()
{
    // 判断条件：data_ != nullptr 是否成立。
    if (data_ != nullptr)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;

    // 执行语句：char path[64];；它会更新一次变量、对象或程序状态。
    char path[64];
    // 函数调用：(void)std::snprintf(path, sizeof(path), "%s%s", ::app::common::storage::k_path_assets,；执行该函数并处理它的返回值或副作用。
    (void)std::snprintf(path, sizeof(path), "%s%s", ::app::common::storage::k_path_assets,
                        // 继续上一行的表达式或参数列表：kBinRelPath);。
                        kBinRelPath);

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
    // 判断条件：sz <= 0 是否成立。
    if (sz <= 0)
    {
        // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(f);
        // 调用 EBOOK_LOGW(TAG, "empty bin: %s", path);，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "empty bin: %s", path);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    auto* buf = static_cast<uint8_t*>(
        // 继续上一行的表达式或参数列表：heap_caps_malloc(static_cast<size_t>(sz), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));。
        heap_caps_malloc(static_cast<size_t>(sz), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    // 判断条件：buf == nullptr 是否成立。
    if (buf == nullptr)
        // 赋值：把右侧 static_cast<uint8_t*>(heap_caps_malloc(static_cast<size_t>(sz), MALLOC_CAP_8BIT)) 的结果保存到左侧 buf。
        buf = static_cast<uint8_t*>(heap_caps_malloc(static_cast<size_t>(sz), MALLOC_CAP_8BIT));
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

    // 判断条件：zh_pinyin_bin_load(buf, got) != 0 是否成立。
    if (zh_pinyin_bin_load(buf, got) != 0)
    {
        // 调用 heap_caps_free(buf);，执行这个函数并处理它的返回值或状态变化。
        heap_caps_free(buf);
        // 调用 EBOOK_LOGE(TAG, "zh_pinyin_bin_load failed");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGE(TAG, "zh_pinyin_bin_load failed");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 buf 的结果保存到左侧 data_。
    data_ = buf;
    // 赋值：把右侧 got 的结果保存到左侧 data_size_。
    data_size_ = got;
    // 调用 EBOOK_LOGI(TAG, "loaded %u bytes from %s", static_cast<unsigned>(got), path);，执行这个函数并处理它的返回值或状态变化。
    EBOOK_LOGI(TAG, "loaded %u bytes from %s", static_cast<unsigned>(got), path);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 ImeEngine::deinit：参数在括号里，返回类型写在函数名前面。
void ImeEngine::deinit()
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
    // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    reset();
}

// 函数定义/声明：调用 ImeEngine::reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
void ImeEngine::reset()
{
    // 赋值：把右侧 '\0' 的结果保存到左侧 compose_[0]。
    compose_[0] = '\0';
    // 赋值：把右侧 0 的结果保存到左侧 compose_len_。
    compose_len_ = 0;
}

// 定义或声明函数 ImeEngine::push_ascii：参数在括号里，返回类型写在函数名前面。
bool ImeEngine::push_ascii(char c)
{
    // 判断条件：c < 'a' || c > 'z' 是否成立。
    if (c < 'a' || c > 'z')
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：compose_len_ >= kComposeCap 是否成立。
    if (compose_len_ >= kComposeCap)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 赋值：把右侧 c 的结果保存到左侧 compose_[compose_len_++]。
    compose_[compose_len_++] = c;
    // 赋值：把右侧 '\0' 的结果保存到左侧 compose_[compose_len_]。
    compose_[compose_len_] = '\0';
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 ImeEngine::pop_ascii：参数在括号里，返回类型写在函数名前面。
void ImeEngine::pop_ascii()
{
    // 判断条件：compose_len_ == 0 是否成立。
    if (compose_len_ == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 执行语句：--compose_len_;；它会更新一次变量、对象或程序状态。
    --compose_len_;
    // 赋值：把右侧 '\0' 的结果保存到左侧 compose_[compose_len_]。
    compose_[compose_len_] = '\0';
}

// 定义或声明函数 ImeEngine::query_candidates：参数在括号里，返回类型写在函数名前面。
uint8_t ImeEngine::query_candidates(char* out_utf8, uint8_t max_chars)
{
    // 判断条件：out_utf8 == nullptr || max_chars == 0 是否成立。
    if (out_utf8 == nullptr || max_chars == 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    // 赋值：把右侧 '\0' 的结果保存到左侧 out_utf8[0]。
    out_utf8[0] = '\0';
    // 判断条件：data_ == nullptr || compose_len_ == 0 是否成立。
    if (data_ == nullptr || compose_len_ == 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 定义变量 uint8_t br 并初始化为 0。
    uint8_t br = 0;
    // 判断条件：zh_match_code_vague(compose_, out_utf8, max_chars, &br) != 0 是否成立。
    if (zh_match_code_vague(compose_, out_utf8, max_chars, &br) != 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    // 返回 br 给调用者，表示当前函数的执行结果。
    return br;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::ime
