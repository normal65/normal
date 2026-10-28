// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/common/memory/double_buffer/double_buffer_impl.hpp
 * 作用：实现 double buffer 模块中的 double buffer impl 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：PSRAM 能缓解内部内存压力，但不能无视 DMA 和中断约束。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_heap_caps.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_log.h"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::common::memory::double_buffer {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {
// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "DblBuf";
}

// 模板声明：让同一段代码适配不同类型。
template <bool EnableStats>
// 定义或声明函数 DoubleBuffer<EnableStats>::DoubleBuffer：参数在括号里，返回类型写在函数名前面。
DoubleBuffer<EnableStats>::DoubleBuffer(size_t buffer_size, bool use_psram, size_t alignment)
    // 定义或声明函数 buffer_size_：参数在括号里，返回类型写在函数名前面。
    : buffer_size_(buffer_size)
    // 定义或声明函数 is_static_：参数在括号里，返回类型写在函数名前面。
    , is_static_(false)
{
    // 访问对象成员：states_[0].store(BufferState::EMPTY, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
    states_[0].store(BufferState::EMPTY, std::memory_order_relaxed);
    // 访问对象成员：states_[1].store(BufferState::EMPTY, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
    states_[1].store(BufferState::EMPTY, std::memory_order_relaxed);
    // 访问对象成员：data_sizes_[0].store(0, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
    data_sizes_[0].store(0, std::memory_order_relaxed);
    // 访问对象成员：data_sizes_[1].store(0, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
    data_sizes_[1].store(0, std::memory_order_relaxed);

    // 判断条件：buffer_size_ == 0 是否成立。
    if (buffer_size_ == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 判断条件：alignment == 0 || (alignment & (alignment - 1)) != 0 是否成立。
    if (alignment == 0 || (alignment & (alignment - 1)) != 0)
        // 赋值：把右侧 4 的结果保存到左侧 alignment。
        alignment = 4;

    // 赋值：把右侧 (buffer_size_ + alignment - 1) & ~(alignment - 1) 的结果保存到左侧 buffer_size_。
    buffer_size_ = (buffer_size_ + alignment - 1) & ~(alignment - 1);

    // 定义变量 uint32_t caps 并初始化为 MALLOC_CAP_8BIT | (use_psram ? MALLOC_CAP_SPIRAM : MALLOC_CAP_INTERNAL)。
    uint32_t caps = MALLOC_CAP_8BIT | (use_psram ? MALLOC_CAP_SPIRAM : MALLOC_CAP_INTERNAL);

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (int i = 0; i < 2; i++)
    {
        // 赋值：把右侧 static_cast<uint8_t*>(heap_caps_aligned_alloc(alignment, buffer_size_, caps)) 的结果保存到左侧 buffers_[i]。
        buffers_[i] = static_cast<uint8_t*>(heap_caps_aligned_alloc(alignment, buffer_size_, caps));

        // 判断条件：!buffers_[i] && use_psram 是否成立。
        if (!buffers_[i] && use_psram)
        {
            // 赋值：把右侧 MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL 的结果保存到左侧 caps。
            caps = MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL;
            // 执行这条代码：buffers_[i] =。它会更新变量、对象状态或调用链。
            buffers_[i] =
                // 继续上一行的表达式或参数列表：static_cast<uint8_t*>(heap_caps_aligned_alloc(alignment, buffer_size_, caps));。
                static_cast<uint8_t*>(heap_caps_aligned_alloc(alignment, buffer_size_, caps));
        }

        // 判断条件：!buffers_[i] 是否成立。
        if (!buffers_[i])
        {
            // 调用 ESP_LOGE(TAG, "alloc buf[%d] failed: %u bytes", i, (unsigned)buffer_size_);，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "alloc buf[%d] failed: %u bytes", i, (unsigned)buffer_size_);
            // 判断条件：i == 1 && buffers_[0] 是否成立。
            if (i == 1 && buffers_[0])
            {
                // 调用 heap_caps_free(buffers_[0]);，执行这个函数并处理它的返回值或状态变化。
                heap_caps_free(buffers_[0]);
                // 赋值：把右侧 nullptr 的结果保存到左侧 buffers_[0]。
                buffers_[0] = nullptr;
            }
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }
    }

    // 调用 init_mutex();，执行这个函数并处理它的返回值或状态变化。
    init_mutex();
}

// 模板声明：让同一段代码适配不同类型。
template <bool EnableStats>
// 定义或声明函数 DoubleBuffer<EnableStats>::DoubleBuffer：参数在括号里，返回类型写在函数名前面。
DoubleBuffer<EnableStats>::DoubleBuffer(uint8_t* buffer0, uint8_t* buffer1, size_t buffer_size)
    // 定义或声明函数 buffer_size_：参数在括号里，返回类型写在函数名前面。
    : buffer_size_(buffer_size)
    // 定义或声明函数 is_static_：参数在括号里，返回类型写在函数名前面。
    , is_static_(true)
{
    // 赋值：把右侧 buffer0 的结果保存到左侧 buffers_[0]。
    buffers_[0] = buffer0;
    // 赋值：把右侧 buffer1 的结果保存到左侧 buffers_[1]。
    buffers_[1] = buffer1;
    // 访问对象成员：states_[0].store(BufferState::EMPTY, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
    states_[0].store(BufferState::EMPTY, std::memory_order_relaxed);
    // 访问对象成员：states_[1].store(BufferState::EMPTY, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
    states_[1].store(BufferState::EMPTY, std::memory_order_relaxed);
    // 访问对象成员：data_sizes_[0].store(0, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
    data_sizes_[0].store(0, std::memory_order_relaxed);
    // 访问对象成员：data_sizes_[1].store(0, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
    data_sizes_[1].store(0, std::memory_order_relaxed);

    // 判断条件：!buffers_[0] || !buffers_[1] || buffer_size_ == 0 是否成立。
    if (!buffers_[0] || !buffers_[1] || buffer_size_ == 0)
    {
        // 赋值：把右侧 nullptr 的结果保存到左侧 buffers_[0]。
        buffers_[0] = nullptr;
        // 赋值：把右侧 nullptr 的结果保存到左侧 buffers_[1]。
        buffers_[1] = nullptr;
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 调用 init_mutex();，执行这个函数并处理它的返回值或状态变化。
    init_mutex();
}

// 模板声明：让同一段代码适配不同类型。
template <bool EnableStats>
// 定义或声明函数 DoubleBuffer<EnableStats>::init_mutex：参数在括号里，返回类型写在函数名前面。
void DoubleBuffer<EnableStats>::init_mutex()
{
    // 赋值：把右侧 xSemaphoreCreateMutex() 的结果保存到左侧 mutex_。
    mutex_ = xSemaphoreCreateMutex();
    // 判断条件：!mutex_ 是否成立。
    if (!mutex_)
    {
        // 调用 ESP_LOGE(TAG, "mutex create failed");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "mutex create failed");
        // 判断条件：!is_static_ 是否成立。
        if (!is_static_)
        {
            // 判断条件：buffers_[0] 是否成立。
            if (buffers_[0])
                // 调用 heap_caps_free(buffers_[0]);，执行这个函数并处理它的返回值或状态变化。
                heap_caps_free(buffers_[0]);
            // 判断条件：buffers_[1] 是否成立。
            if (buffers_[1])
                // 调用 heap_caps_free(buffers_[1]);，执行这个函数并处理它的返回值或状态变化。
                heap_caps_free(buffers_[1]);
        }
        // 赋值：把右侧 nullptr 的结果保存到左侧 buffers_[0]。
        buffers_[0] = nullptr;
        // 赋值：把右侧 nullptr 的结果保存到左侧 buffers_[1]。
        buffers_[1] = nullptr;
    }
}

// 模板声明：让同一段代码适配不同类型。
template <bool EnableStats>
// 定义或声明函数 DoubleBuffer<EnableStats>::~DoubleBuffer：参数在括号里，返回类型写在函数名前面。
DoubleBuffer<EnableStats>::~DoubleBuffer()
{
    // 判断条件：mutex_ 是否成立。
    if (mutex_)
        // 调用 vSemaphoreDelete(mutex_);，执行这个函数并处理它的返回值或状态变化。
        vSemaphoreDelete(mutex_);
    // 判断条件：!is_static_ 是否成立。
    if (!is_static_)
    {
        // 判断条件：buffers_[0] 是否成立。
        if (buffers_[0])
            // 调用 heap_caps_free(buffers_[0]);，执行这个函数并处理它的返回值或状态变化。
            heap_caps_free(buffers_[0]);
        // 判断条件：buffers_[1] 是否成立。
        if (buffers_[1])
            // 调用 heap_caps_free(buffers_[1]);，执行这个函数并处理它的返回值或状态变化。
            heap_caps_free(buffers_[1]);
    }
}

// Statistics helpers
template <bool EnableStats>
// 定义或声明函数 DoubleBuffer<EnableStats>::update_write_stats：参数在括号里，返回类型写在函数名前面。
inline void DoubleBuffer<EnableStats>::update_write_stats(size_t bytes)
{
    // 定义或声明函数 constexpr：参数在括号里，返回类型写在函数名前面。
    if constexpr (EnableStats)
    {
        // 访问对象成员：stats_.write_count.fetch_add(1, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
        stats_.write_count.fetch_add(1, std::memory_order_relaxed);
        // 访问对象成员：stats_.write_bytes.fetch_add(bytes, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
        stats_.write_bytes.fetch_add(bytes, std::memory_order_relaxed);
    }
}

// 模板声明：让同一段代码适配不同类型。
template <bool EnableStats>
// 定义或声明函数 DoubleBuffer<EnableStats>::update_read_stats：参数在括号里，返回类型写在函数名前面。
inline void DoubleBuffer<EnableStats>::update_read_stats(size_t bytes)
{
    // 定义或声明函数 constexpr：参数在括号里，返回类型写在函数名前面。
    if constexpr (EnableStats)
    {
        // 访问对象成员：stats_.read_count.fetch_add(1, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
        stats_.read_count.fetch_add(1, std::memory_order_relaxed);
        // 访问对象成员：stats_.read_bytes.fetch_add(bytes, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
        stats_.read_bytes.fetch_add(bytes, std::memory_order_relaxed);
    }
}

// 模板声明：让同一段代码适配不同类型。
template <bool EnableStats>
// 定义或声明函数 DoubleBuffer<EnableStats>::update_overflow：参数在括号里，返回类型写在函数名前面。
inline void DoubleBuffer<EnableStats>::update_overflow()
{
    // 定义或声明函数 constexpr：参数在括号里，返回类型写在函数名前面。
    if constexpr (EnableStats)
        // 访问对象成员：stats_.overflow_count.fetch_add(1, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
        stats_.overflow_count.fetch_add(1, std::memory_order_relaxed);
}

// 模板声明：让同一段代码适配不同类型。
template <bool EnableStats>
// 定义或声明函数 DoubleBuffer<EnableStats>::update_underflow：参数在括号里，返回类型写在函数名前面。
inline void DoubleBuffer<EnableStats>::update_underflow()
{
    // 定义或声明函数 constexpr：参数在括号里，返回类型写在函数名前面。
    if constexpr (EnableStats)
        // 访问对象成员：stats_.underflow_count.fetch_add(1, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
        stats_.underflow_count.fetch_add(1, std::memory_order_relaxed);
}

// 模板声明：让同一段代码适配不同类型。
template <bool EnableStats>
// 定义或声明函数 DoubleBuffer<EnableStats>::find_buffer：参数在括号里，返回类型写在函数名前面。
int DoubleBuffer<EnableStats>::find_buffer(BufferState state) const
{
    // 定义变量 int idx 并初始化为 write_index_.load(std::memory_order_relaxed)。
    int idx = write_index_.load(std::memory_order_relaxed);
    // 判断条件：states_[idx].load(std::memory_order_acquire) == state 是否成立。
    if (states_[idx].load(std::memory_order_acquire) == state)
        // 返回 idx 给调用者，表示当前函数的执行结果。
        return idx;

    // 定义变量 int other 并初始化为 1 - idx。
    int other = 1 - idx;
    // 判断条件：states_[other].load(std::memory_order_acquire) == state 是否成立。
    if (states_[other].load(std::memory_order_acquire) == state)
        // 返回 other 给调用者，表示当前函数的执行结果。
        return other;

    // 返回 -1 给调用者，表示当前函数的执行结果。
    return -1;
}

// Acquire write buffer
template <bool EnableStats>
// 定义或声明函数 DoubleBuffer<EnableStats>::acquire_write_buffer：参数在括号里，返回类型写在函数名前面。
uint8_t* DoubleBuffer<EnableStats>::acquire_write_buffer()
{
    // 判断条件：!is_valid() 是否成立。
    if (!is_valid())
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;

    // 判断条件：xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE 是否成立。
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE)
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;

    // 定义变量 int idx 并初始化为 find_buffer(BufferState::EMPTY)。
    int idx = find_buffer(BufferState::EMPTY);
    // 判断条件：idx < 0 是否成立。
    if (idx < 0)
    {
        // 调用 xSemaphoreGive(mutex_);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGive(mutex_);
        // 调用 update_overflow();，执行这个函数并处理它的返回值或状态变化。
        update_overflow();
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;
    }

    // 访问对象成员：states_[idx].store(BufferState::WRITING, std::memory_order_release);；读取或修改这个对象里的成员状态。
    states_[idx].store(BufferState::WRITING, std::memory_order_release);
    // 访问对象成员：write_index_.store(idx, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
    write_index_.store(idx, std::memory_order_relaxed);

    // 调用 xSemaphoreGive(mutex_);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGive(mutex_);
    // 返回 buffers_[idx] 给调用者，表示当前函数的执行结果。
    return buffers_[idx];
}

// Release write buffer
template <bool EnableStats>
// 定义或声明函数 DoubleBuffer<EnableStats>::release_write_buffer：参数在括号里，返回类型写在函数名前面。
void DoubleBuffer<EnableStats>::release_write_buffer(size_t data_size)
{
    // 判断条件：!is_valid() 是否成立。
    if (!is_valid())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 判断条件：xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE 是否成立。
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 定义变量 int idx 并初始化为 write_index_.load(std::memory_order_relaxed)。
    int idx = write_index_.load(std::memory_order_relaxed);
    // 判断条件：states_[idx].load(std::memory_order_acquire) == BufferState::WRITING 是否成立。
    if (states_[idx].load(std::memory_order_acquire) == BufferState::WRITING)
    {
        // 访问对象成员：data_sizes_[idx].store(data_size, std::memory_order_release);；读取或修改这个对象里的成员状态。
        data_sizes_[idx].store(data_size, std::memory_order_release);
        // 访问对象成员：states_[idx].store(BufferState::READY, std::memory_order_release);；读取或修改这个对象里的成员状态。
        states_[idx].store(BufferState::READY, std::memory_order_release);
        // 调用 update_write_stats(data_size);，执行这个函数并处理它的返回值或状态变化。
        update_write_stats(data_size);

        // 定义或声明函数 constexpr：参数在括号里，返回类型写在函数名前面。
        if constexpr (EnableStats)
            // 访问对象成员：stats_.swap_count.fetch_add(1, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
            stats_.swap_count.fetch_add(1, std::memory_order_relaxed);
    }

    // 调用 xSemaphoreGive(mutex_);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGive(mutex_);
}

// Acquire read buffer
template <bool EnableStats>
// 定义或声明函数 DoubleBuffer<EnableStats>::acquire_read_buffer：参数在括号里，返回类型写在函数名前面。
uint8_t* DoubleBuffer<EnableStats>::acquire_read_buffer(size_t* data_size)
{
    // 判断条件：!is_valid() 是否成立。
    if (!is_valid())
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;

    // 判断条件：xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE 是否成立。
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE)
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;

    // 定义变量 int idx 并初始化为 find_buffer(BufferState::READY)。
    int idx = find_buffer(BufferState::READY);
    // 判断条件：idx < 0 是否成立。
    if (idx < 0)
    {
        // 调用 xSemaphoreGive(mutex_);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGive(mutex_);
        // 调用 update_underflow();，执行这个函数并处理它的返回值或状态变化。
        update_underflow();
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;
    }

    // 访问对象成员：states_[idx].store(BufferState::READING, std::memory_order_release);；读取或修改这个对象里的成员状态。
    states_[idx].store(BufferState::READING, std::memory_order_release);
    // 访问对象成员：read_index_.store(idx, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
    read_index_.store(idx, std::memory_order_relaxed);

    // 判断条件：data_size 是否成立。
    if (data_size)
        *data_size = data_sizes_[idx].load(std::memory_order_acquire);

    // 调用 xSemaphoreGive(mutex_);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGive(mutex_);
    // 返回 buffers_[idx] 给调用者，表示当前函数的执行结果。
    return buffers_[idx];
}

// Release read buffer
template <bool EnableStats>
// 定义或声明函数 DoubleBuffer<EnableStats>::release_read_buffer：参数在括号里，返回类型写在函数名前面。
void DoubleBuffer<EnableStats>::release_read_buffer()
{
    // 判断条件：!is_valid() 是否成立。
    if (!is_valid())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 判断条件：xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE 是否成立。
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 定义变量 int idx 并初始化为 read_index_.load(std::memory_order_relaxed)。
    int idx = read_index_.load(std::memory_order_relaxed);
    // 判断条件：states_[idx].load(std::memory_order_acquire) == BufferState::READING 是否成立。
    if (states_[idx].load(std::memory_order_acquire) == BufferState::READING)
    {
        // 定义变量 size_t bytes 并初始化为 data_sizes_[idx].load(std::memory_order_acquire)。
        size_t bytes = data_sizes_[idx].load(std::memory_order_acquire);
        // 访问对象成员：data_sizes_[idx].store(0, std::memory_order_release);；读取或修改这个对象里的成员状态。
        data_sizes_[idx].store(0, std::memory_order_release);
        // 访问对象成员：states_[idx].store(BufferState::EMPTY, std::memory_order_release);；读取或修改这个对象里的成员状态。
        states_[idx].store(BufferState::EMPTY, std::memory_order_release);
        // 调用 update_read_stats(bytes);，执行这个函数并处理它的返回值或状态变化。
        update_read_stats(bytes);
    }

    // 调用 xSemaphoreGive(mutex_);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGive(mutex_);
}

// ISR: acquire write buffer
template <bool EnableStats>
// 执行这条代码：uint8_t*。它会更新变量、对象状态或调用链。
uint8_t*
// 定义或声明函数 DoubleBuffer<EnableStats>::acquire_write_buffer_from_isr：参数在括号里，返回类型写在函数名前面。
DoubleBuffer<EnableStats>::acquire_write_buffer_from_isr(BaseType_t* pxHigherPriorityTaskWoken)
{
    // 判断条件：!is_valid() 是否成立。
    if (!is_valid())
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;

    // 判断条件：xSemaphoreTakeFromISR(mutex_, pxHigherPriorityTaskWoken) != pdTRUE 是否成立。
    if (xSemaphoreTakeFromISR(mutex_, pxHigherPriorityTaskWoken) != pdTRUE)
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;

    // 定义变量 int idx 并初始化为 find_buffer(BufferState::EMPTY)。
    int idx = find_buffer(BufferState::EMPTY);
    // 判断条件：idx < 0 是否成立。
    if (idx < 0)
    {
        // 调用 xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);
        // 调用 update_overflow();，执行这个函数并处理它的返回值或状态变化。
        update_overflow();
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;
    }

    // 访问对象成员：states_[idx].store(BufferState::WRITING, std::memory_order_release);；读取或修改这个对象里的成员状态。
    states_[idx].store(BufferState::WRITING, std::memory_order_release);
    // 访问对象成员：write_index_.store(idx, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
    write_index_.store(idx, std::memory_order_relaxed);

    // 调用 xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);
    // 返回 buffers_[idx] 给调用者，表示当前函数的执行结果。
    return buffers_[idx];
}

// ISR: release write buffer
template <bool EnableStats>
// 函数调用：void DoubleBuffer<EnableStats>::release_write_buffer_from_isr(size_t data_size,；执行该函数并处理它的返回值或副作用。
void DoubleBuffer<EnableStats>::release_write_buffer_from_isr(size_t data_size,
                                                              // 继续上一行的表达式或参数列表：BaseType_t* pxHigherPriorityTaskWoken)。
                                                              BaseType_t* pxHigherPriorityTaskWoken)
{
    // 判断条件：!is_valid() 是否成立。
    if (!is_valid())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 判断条件：xSemaphoreTakeFromISR(mutex_, pxHigherPriorityTaskWoken) != pdTRUE 是否成立。
    if (xSemaphoreTakeFromISR(mutex_, pxHigherPriorityTaskWoken) != pdTRUE)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 定义变量 int idx 并初始化为 write_index_.load(std::memory_order_relaxed)。
    int idx = write_index_.load(std::memory_order_relaxed);
    // 判断条件：states_[idx].load(std::memory_order_acquire) == BufferState::WRITING 是否成立。
    if (states_[idx].load(std::memory_order_acquire) == BufferState::WRITING)
    {
        // 访问对象成员：data_sizes_[idx].store(data_size, std::memory_order_release);；读取或修改这个对象里的成员状态。
        data_sizes_[idx].store(data_size, std::memory_order_release);
        // 访问对象成员：states_[idx].store(BufferState::READY, std::memory_order_release);；读取或修改这个对象里的成员状态。
        states_[idx].store(BufferState::READY, std::memory_order_release);
        // 调用 update_write_stats(data_size);，执行这个函数并处理它的返回值或状态变化。
        update_write_stats(data_size);

        // 定义或声明函数 constexpr：参数在括号里，返回类型写在函数名前面。
        if constexpr (EnableStats)
            // 访问对象成员：stats_.swap_count.fetch_add(1, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
            stats_.swap_count.fetch_add(1, std::memory_order_relaxed);
    }

    // 调用 xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);
}

// ISR: acquire read buffer
template <bool EnableStats>
// 执行这条代码：uint8_t*。它会更新变量、对象状态或调用链。
uint8_t*
// 函数调用：DoubleBuffer<EnableStats>::acquire_read_buffer_from_isr(size_t* data_size,；执行该函数并处理它的返回值或副作用。
DoubleBuffer<EnableStats>::acquire_read_buffer_from_isr(size_t* data_size,
                                                        // 继续上一行的表达式或参数列表：BaseType_t* pxHigherPriorityTaskWoken)。
                                                        BaseType_t* pxHigherPriorityTaskWoken)
{
    // 判断条件：!is_valid() 是否成立。
    if (!is_valid())
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;

    // 判断条件：xSemaphoreTakeFromISR(mutex_, pxHigherPriorityTaskWoken) != pdTRUE 是否成立。
    if (xSemaphoreTakeFromISR(mutex_, pxHigherPriorityTaskWoken) != pdTRUE)
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;

    // 定义变量 int idx 并初始化为 find_buffer(BufferState::READY)。
    int idx = find_buffer(BufferState::READY);
    // 判断条件：idx < 0 是否成立。
    if (idx < 0)
    {
        // 调用 xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);
        // 调用 update_underflow();，执行这个函数并处理它的返回值或状态变化。
        update_underflow();
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;
    }

    // 访问对象成员：states_[idx].store(BufferState::READING, std::memory_order_release);；读取或修改这个对象里的成员状态。
    states_[idx].store(BufferState::READING, std::memory_order_release);
    // 访问对象成员：read_index_.store(idx, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
    read_index_.store(idx, std::memory_order_relaxed);

    // 判断条件：data_size 是否成立。
    if (data_size)
        *data_size = data_sizes_[idx].load(std::memory_order_acquire);

    // 调用 xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);
    // 返回 buffers_[idx] 给调用者，表示当前函数的执行结果。
    return buffers_[idx];
}

// ISR: release read buffer
template <bool EnableStats>
// 定义或声明函数 DoubleBuffer<EnableStats>::release_read_buffer_from_isr：参数在括号里，返回类型写在函数名前面。
void DoubleBuffer<EnableStats>::release_read_buffer_from_isr(BaseType_t* pxHigherPriorityTaskWoken)
{
    // 判断条件：!is_valid() 是否成立。
    if (!is_valid())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 判断条件：xSemaphoreTakeFromISR(mutex_, pxHigherPriorityTaskWoken) != pdTRUE 是否成立。
    if (xSemaphoreTakeFromISR(mutex_, pxHigherPriorityTaskWoken) != pdTRUE)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 定义变量 int idx 并初始化为 read_index_.load(std::memory_order_relaxed)。
    int idx = read_index_.load(std::memory_order_relaxed);
    // 判断条件：states_[idx].load(std::memory_order_acquire) == BufferState::READING 是否成立。
    if (states_[idx].load(std::memory_order_acquire) == BufferState::READING)
    {
        // 定义变量 size_t bytes 并初始化为 data_sizes_[idx].load(std::memory_order_acquire)。
        size_t bytes = data_sizes_[idx].load(std::memory_order_acquire);
        // 访问对象成员：data_sizes_[idx].store(0, std::memory_order_release);；读取或修改这个对象里的成员状态。
        data_sizes_[idx].store(0, std::memory_order_release);
        // 访问对象成员：states_[idx].store(BufferState::EMPTY, std::memory_order_release);；读取或修改这个对象里的成员状态。
        states_[idx].store(BufferState::EMPTY, std::memory_order_release);
        // 调用 update_read_stats(bytes);，执行这个函数并处理它的返回值或状态变化。
        update_read_stats(bytes);
    }

    // 调用 xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);
}

// Convenience write (with copy)
template <bool EnableStats>
// 定义或声明函数 DoubleBuffer<EnableStats>::write：参数在括号里，返回类型写在函数名前面。
bool DoubleBuffer<EnableStats>::write(const uint8_t* data, size_t len)
{
    // 判断条件：!data || len == 0 || len > buffer_size_ 是否成立。
    if (!data || len == 0 || len > buffer_size_)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 acquire_write_buffer() 的结果保存到左侧 uint8_t* buf。
    uint8_t* buf = acquire_write_buffer();
    // 判断条件：!buf 是否成立。
    if (!buf)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 调用 std::memcpy(buf, data, len);，执行这个函数并处理它的返回值或状态变化。
    std::memcpy(buf, data, len);
    // 调用 release_write_buffer(len);，执行这个函数并处理它的返回值或状态变化。
    release_write_buffer(len);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// Convenience read (with copy)
template <bool EnableStats>
// 函数定义/声明：调用 read()：read() 从寄存器读出触摸状态和最多两个触点，解析成 TouchSample。istouched() 是更简单的查询，用于判断当前是否仍有手指接触。。
size_t DoubleBuffer<EnableStats>::read(uint8_t* data, size_t max_len)
{
    // 判断条件：!data || max_len == 0 是否成立。
    if (!data || max_len == 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 定义变量 size_t size 并初始化为 0。
    size_t size = 0;
    // 赋值：把右侧 acquire_read_buffer(&size) 的结果保存到左侧 uint8_t* buf。
    uint8_t* buf = acquire_read_buffer(&size);
    // 判断条件：!buf 是否成立。
    if (!buf)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 定义变量 size_t to_read 并初始化为 (size < max_len) ? size : max_len。
    size_t to_read = (size < max_len) ? size : max_len;
    // 调用 std::memcpy(data, buf, to_read);，执行这个函数并处理它的返回值或状态变化。
    std::memcpy(data, buf, to_read);
    // 调用 release_read_buffer();，执行这个函数并处理它的返回值或状态变化。
    release_read_buffer();
    // 返回 to_read 给调用者，表示当前函数的执行结果。
    return to_read;
}

// Has data (locked)
template <bool EnableStats>
// 定义或声明函数 DoubleBuffer<EnableStats>::has_data：参数在括号里，返回类型写在函数名前面。
bool DoubleBuffer<EnableStats>::has_data() const
{
    // 判断条件：!is_valid() 是否成立。
    if (!is_valid())
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 判断条件：xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE 是否成立。
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE)
        // 返回 has_data_fast() 给调用者，表示当前函数的执行结果。
        return has_data_fast();

    // 定义变量 bool result 并初始化为 states_[0].load(std::memory_order_acquire) == BufferState::READY |。
    bool result = states_[0].load(std::memory_order_acquire) == BufferState::READY ||
                  // 继续上一行的表达式或参数列表：states_[1].load(std::memory_order_acquire) == BufferState::READY;。
                  states_[1].load(std::memory_order_acquire) == BufferState::READY;

    // 调用 xSemaphoreGive(mutex_);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGive(mutex_);
    // 返回 result 给调用者，表示当前函数的执行结果。
    return result;
}

// Can write (locked)
template <bool EnableStats>
// 定义或声明函数 DoubleBuffer<EnableStats>::can_write：参数在括号里，返回类型写在函数名前面。
bool DoubleBuffer<EnableStats>::can_write() const
{
    // 判断条件：!is_valid() 是否成立。
    if (!is_valid())
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 判断条件：xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE 是否成立。
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE)
        // 返回 can_write_fast() 给调用者，表示当前函数的执行结果。
        return can_write_fast();

    // 定义变量 bool result 并初始化为 states_[0].load(std::memory_order_acquire) == BufferState::EMPTY |。
    bool result = states_[0].load(std::memory_order_acquire) == BufferState::EMPTY ||
                  // 继续上一行的表达式或参数列表：states_[1].load(std::memory_order_acquire) == BufferState::EMPTY;。
                  states_[1].load(std::memory_order_acquire) == BufferState::EMPTY;

    // 调用 xSemaphoreGive(mutex_);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGive(mutex_);
    // 返回 result 给调用者，表示当前函数的执行结果。
    return result;
}

// Reset
template <bool EnableStats>
// 函数定义/声明：调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
void DoubleBuffer<EnableStats>::reset()
{
    // 判断条件：!is_valid() 是否成立。
    if (!is_valid())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 判断条件：xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE 是否成立。
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 访问对象成员：states_[0].store(BufferState::EMPTY, std::memory_order_release);；读取或修改这个对象里的成员状态。
    states_[0].store(BufferState::EMPTY, std::memory_order_release);
    // 访问对象成员：states_[1].store(BufferState::EMPTY, std::memory_order_release);；读取或修改这个对象里的成员状态。
    states_[1].store(BufferState::EMPTY, std::memory_order_release);
    // 访问对象成员：data_sizes_[0].store(0, std::memory_order_release);；读取或修改这个对象里的成员状态。
    data_sizes_[0].store(0, std::memory_order_release);
    // 访问对象成员：data_sizes_[1].store(0, std::memory_order_release);；读取或修改这个对象里的成员状态。
    data_sizes_[1].store(0, std::memory_order_release);
    // 访问对象成员：write_index_.store(0, std::memory_order_release);；读取或修改这个对象里的成员状态。
    write_index_.store(0, std::memory_order_release);
    // 访问对象成员：read_index_.store(0, std::memory_order_release);；读取或修改这个对象里的成员状态。
    read_index_.store(0, std::memory_order_release);

    // 调用 xSemaphoreGive(mutex_);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGive(mutex_);
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::common::memory::double_buffer
