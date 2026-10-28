// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/common/memory/ring_buffer/ring_buffer_impl.hpp
 * 作用：实现 ring buffer 模块中的 ring buffer impl 功能。
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
namespace app::common::memory::ring_buffer {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {
// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "RingBuf";
}

// 模板声明：让同一段代码适配不同类型。
template <bool EnableStats>
// 定义或声明函数 RingBuffer<EnableStats>::RingBuffer：参数在括号里，返回类型写在函数名前面。
RingBuffer<EnableStats>::RingBuffer(size_t capacity, bool use_psram, size_t alignment)
    // 定义或声明函数 capacity_：参数在括号里，返回类型写在函数名前面。
    : capacity_(capacity)
    // 定义或声明函数 is_static_：参数在括号里，返回类型写在函数名前面。
    , is_static_(false)
{
    // 判断条件：capacity_ == 0 是否成立。
    if (capacity_ == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 判断条件：alignment == 0 || (alignment & (alignment - 1)) != 0 是否成立。
    if (alignment == 0 || (alignment & (alignment - 1)) != 0)
        // 赋值：把右侧 4 的结果保存到左侧 alignment。
        alignment = 4;

    // 赋值：把右侧 (capacity_ + alignment - 1) & ~(alignment - 1) 的结果保存到左侧 capacity_。
    capacity_ = (capacity_ + alignment - 1) & ~(alignment - 1);

    // 定义变量 uint32_t caps 并初始化为 MALLOC_CAP_8BIT | (use_psram ? MALLOC_CAP_SPIRAM : MALLOC_CAP_INTERNAL)。
    uint32_t caps = MALLOC_CAP_8BIT | (use_psram ? MALLOC_CAP_SPIRAM : MALLOC_CAP_INTERNAL);
    // 赋值：把右侧 static_cast<uint8_t*>(heap_caps_aligned_alloc(alignment, capacity_, caps)) 的结果保存到左侧 buffer_。
    buffer_ = static_cast<uint8_t*>(heap_caps_aligned_alloc(alignment, capacity_, caps));

    // 判断条件：!buffer_ && use_psram 是否成立。
    if (!buffer_ && use_psram)
    {
        // 赋值：把右侧 MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL 的结果保存到左侧 caps。
        caps = MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL;
        // 赋值：把右侧 static_cast<uint8_t*>(heap_caps_aligned_alloc(alignment, capacity_, caps)) 的结果保存到左侧 buffer_。
        buffer_ = static_cast<uint8_t*>(heap_caps_aligned_alloc(alignment, capacity_, caps));
    }

    // 判断条件：!buffer_ 是否成立。
    if (!buffer_)
    {
        // 调用 ESP_LOGE(TAG, "alloc failed: %u bytes", (unsigned)capacity_);，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "alloc failed: %u bytes", (unsigned)capacity_);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 调用 init_semaphores();，执行这个函数并处理它的返回值或状态变化。
    init_semaphores();
}

// 模板声明：让同一段代码适配不同类型。
template <bool EnableStats>
// 定义或声明函数 RingBuffer<EnableStats>::RingBuffer：参数在括号里，返回类型写在函数名前面。
RingBuffer<EnableStats>::RingBuffer(uint8_t* static_buffer, size_t capacity)
    // 定义或声明函数 buffer_：参数在括号里，返回类型写在函数名前面。
    : buffer_(static_buffer)
    // 定义或声明函数 capacity_：参数在括号里，返回类型写在函数名前面。
    , capacity_(capacity)
    // 定义或声明函数 is_static_：参数在括号里，返回类型写在函数名前面。
    , is_static_(true)
{
    // 判断条件：!buffer_ || capacity_ == 0 是否成立。
    if (!buffer_ || capacity_ == 0)
    {
        // 赋值：把右侧 nullptr 的结果保存到左侧 buffer_。
        buffer_ = nullptr;
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 调用 init_semaphores();，执行这个函数并处理它的返回值或状态变化。
    init_semaphores();
}

// 模板声明：让同一段代码适配不同类型。
template <bool EnableStats>
// 定义或声明函数 RingBuffer<EnableStats>::init_semaphores：参数在括号里，返回类型写在函数名前面。
void RingBuffer<EnableStats>::init_semaphores()
{
    // 赋值：把右侧 xSemaphoreCreateMutex() 的结果保存到左侧 mutex_。
    mutex_ = xSemaphoreCreateMutex();
    // 判断条件：!mutex_ 是否成立。
    if (!mutex_)
    {
        // 调用 ESP_LOGE(TAG, "mutex create failed");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "mutex create failed");
        // 判断条件：!is_static_ && buffer_ 是否成立。
        if (!is_static_ && buffer_)
        {
            // 调用 heap_caps_free(buffer_);，执行这个函数并处理它的返回值或状态变化。
            heap_caps_free(buffer_);
            // 赋值：把右侧 nullptr 的结果保存到左侧 buffer_。
            buffer_ = nullptr;
        }
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 赋值：把右侧 xSemaphoreCreateBinary() 的结果保存到左侧 read_sem_。
    read_sem_ = xSemaphoreCreateBinary();
    // 赋值：把右侧 xSemaphoreCreateBinary() 的结果保存到左侧 write_sem_。
    write_sem_ = xSemaphoreCreateBinary();

    // 判断条件：!read_sem_ || !write_sem_ 是否成立。
    if (!read_sem_ || !write_sem_)
    {
        // 调用 ESP_LOGE(TAG, "semaphore create failed");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "semaphore create failed");
        // 判断条件：read_sem_ 是否成立。
        if (read_sem_)
            // 调用 vSemaphoreDelete(read_sem_);，执行这个函数并处理它的返回值或状态变化。
            vSemaphoreDelete(read_sem_);
        // 判断条件：write_sem_ 是否成立。
        if (write_sem_)
            // 调用 vSemaphoreDelete(write_sem_);，执行这个函数并处理它的返回值或状态变化。
            vSemaphoreDelete(write_sem_);
        // 调用 vSemaphoreDelete(mutex_);，执行这个函数并处理它的返回值或状态变化。
        vSemaphoreDelete(mutex_);
        // 判断条件：!is_static_ && buffer_ 是否成立。
        if (!is_static_ && buffer_)
            // 调用 heap_caps_free(buffer_);，执行这个函数并处理它的返回值或状态变化。
            heap_caps_free(buffer_);
        // 赋值：把右侧 nullptr 的结果保存到左侧 buffer_。
        buffer_ = nullptr;
        // 赋值：把右侧 nullptr 的结果保存到左侧 mutex_。
        mutex_ = nullptr;
        // 赋值：把右侧 nullptr 的结果保存到左侧 read_sem_。
        read_sem_ = nullptr;
        // 赋值：把右侧 nullptr 的结果保存到左侧 write_sem_。
        write_sem_ = nullptr;
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 调用 xSemaphoreGive(write_sem_);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGive(write_sem_);
}

// 模板声明：让同一段代码适配不同类型。
template <bool EnableStats>
// 定义或声明函数 RingBuffer<EnableStats>::~RingBuffer：参数在括号里，返回类型写在函数名前面。
RingBuffer<EnableStats>::~RingBuffer()
{
    // 判断条件：read_sem_ 是否成立。
    if (read_sem_)
        // 调用 vSemaphoreDelete(read_sem_);，执行这个函数并处理它的返回值或状态变化。
        vSemaphoreDelete(read_sem_);
    // 判断条件：write_sem_ 是否成立。
    if (write_sem_)
        // 调用 vSemaphoreDelete(write_sem_);，执行这个函数并处理它的返回值或状态变化。
        vSemaphoreDelete(write_sem_);
    // 判断条件：mutex_ 是否成立。
    if (mutex_)
        // 调用 vSemaphoreDelete(mutex_);，执行这个函数并处理它的返回值或状态变化。
        vSemaphoreDelete(mutex_);
    // 判断条件：!is_static_ && buffer_ 是否成立。
    if (!is_static_ && buffer_)
        // 调用 heap_caps_free(buffer_);，执行这个函数并处理它的返回值或状态变化。
        heap_caps_free(buffer_);
}

// Statistics helpers
template <bool EnableStats>
// 定义或声明函数 RingBuffer<EnableStats>::update_write_stats：参数在括号里，返回类型写在函数名前面。
inline void RingBuffer<EnableStats>::update_write_stats(size_t bytes)
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
// 定义或声明函数 RingBuffer<EnableStats>::update_read_stats：参数在括号里，返回类型写在函数名前面。
inline void RingBuffer<EnableStats>::update_read_stats(size_t bytes)
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
// 定义或声明函数 RingBuffer<EnableStats>::update_overflow：参数在括号里，返回类型写在函数名前面。
inline void RingBuffer<EnableStats>::update_overflow()
{
    // 定义或声明函数 constexpr：参数在括号里，返回类型写在函数名前面。
    if constexpr (EnableStats)
        // 访问对象成员：stats_.overflow_count.fetch_add(1, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
        stats_.overflow_count.fetch_add(1, std::memory_order_relaxed);
}

// 模板声明：让同一段代码适配不同类型。
template <bool EnableStats>
// 定义或声明函数 RingBuffer<EnableStats>::update_underflow：参数在括号里，返回类型写在函数名前面。
inline void RingBuffer<EnableStats>::update_underflow()
{
    // 定义或声明函数 constexpr：参数在括号里，返回类型写在函数名前面。
    if constexpr (EnableStats)
        // 访问对象成员：stats_.underflow_count.fetch_add(1, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
        stats_.underflow_count.fetch_add(1, std::memory_order_relaxed);
}

// Write
template <bool EnableStats>
// 定义或声明函数 RingBuffer<EnableStats>::write：参数在括号里，返回类型写在函数名前面。
size_t RingBuffer<EnableStats>::write(const uint8_t* data, size_t len, int timeout_ms)
{
    // 判断条件：!is_valid() || !data || len == 0 是否成立。
    if (!is_valid() || !data || len == 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 判断条件：timeout_ms == 0 && free_space_fast() == 0 是否成立。
    if (timeout_ms == 0 && free_space_fast() == 0)
    {
        // 调用 update_overflow();，执行这个函数并处理它的返回值或状态变化。
        update_overflow();
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    }

    // 判断条件：timeout_ms != 0 是否成立。
    if (timeout_ms != 0)
    {
        // 定义变量 TickType_t ticks 并初始化为 (timeout_ms < 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms)。
        TickType_t ticks = (timeout_ms < 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
        // 判断条件：xSemaphoreTake(write_sem_, ticks) != pdTRUE 是否成立。
        if (xSemaphoreTake(write_sem_, ticks) != pdTRUE)
        {
            // 调用 update_overflow();，执行这个函数并处理它的返回值或状态变化。
            update_overflow();
            // 返回 0 给调用者，表示当前函数的执行结果。
            return 0;
        }
    }

    // 判断条件：xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE 是否成立。
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        // 判断条件：timeout_ms != 0 是否成立。
        if (timeout_ms != 0)
            // 调用 xSemaphoreGive(write_sem_);，执行这个函数并处理它的返回值或状态变化。
            xSemaphoreGive(write_sem_);
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    }

    // 定义变量 size_t current 并初始化为 data_size_.load(std::memory_order_relaxed)。
    size_t current = data_size_.load(std::memory_order_relaxed);
    // 定义变量 size_t free 并初始化为 capacity_ - current。
    size_t free = capacity_ - current;
    // 定义变量 size_t to_write 并初始化为 (len < free) ? len : free。
    size_t to_write = (len < free) ? len : free;

    // 判断条件：to_write > 0 是否成立。
    if (to_write > 0)
    {
        // 定义变量 size_t wp 并初始化为 write_pos_.load(std::memory_order_relaxed)。
        size_t wp = write_pos_.load(std::memory_order_relaxed);
        // 定义变量 size_t first 并初始化为 capacity_ - wp。
        size_t first = capacity_ - wp;

        // 判断条件：to_write <= first 是否成立。
        if (to_write <= first)
        {
            // 调用 std::memcpy(buffer_ + wp, data, to_write);，执行这个函数并处理它的返回值或状态变化。
            std::memcpy(buffer_ + wp, data, to_write);
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 调用 std::memcpy(buffer_ + wp, data, first);，执行这个函数并处理它的返回值或状态变化。
            std::memcpy(buffer_ + wp, data, first);
            // 调用 std::memcpy(buffer_, data + first, to_write - first);，执行这个函数并处理它的返回值或状态变化。
            std::memcpy(buffer_, data + first, to_write - first);
        }

        // 访问对象成员：write_pos_.store((wp + to_write) % capacity_, std::memory_order_release);；读取或修改这个对象里的成员状态。
        write_pos_.store((wp + to_write) % capacity_, std::memory_order_release);
        // 访问对象成员：data_size_.fetch_add(to_write, std::memory_order_release);；读取或修改这个对象里的成员状态。
        data_size_.fetch_add(to_write, std::memory_order_release);

        // 调用 update_write_stats(to_write);，执行这个函数并处理它的返回值或状态变化。
        update_write_stats(to_write);
        // 调用 xSemaphoreGive(read_sem_);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGive(read_sem_);
    }

    // 判断条件：data_size_.load(std::memory_order_acquire) < capacity_ 是否成立。
    if (data_size_.load(std::memory_order_acquire) < capacity_)
        // 调用 xSemaphoreGive(write_sem_);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGive(write_sem_);

    // 调用 xSemaphoreGive(mutex_);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGive(mutex_);
    // 返回 to_write 给调用者，表示当前函数的执行结果。
    return to_write;
}

// Read
template <bool EnableStats>
// 函数定义/声明：调用 read()：read() 从寄存器读出触摸状态和最多两个触点，解析成 TouchSample。istouched() 是更简单的查询，用于判断当前是否仍有手指接触。。
size_t RingBuffer<EnableStats>::read(uint8_t* data, size_t len, int timeout_ms)
{
    // 判断条件：!is_valid() || !data || len == 0 是否成立。
    if (!is_valid() || !data || len == 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 判断条件：timeout_ms == 0 && available_fast() == 0 是否成立。
    if (timeout_ms == 0 && available_fast() == 0)
    {
        // 调用 update_underflow();，执行这个函数并处理它的返回值或状态变化。
        update_underflow();
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    }

    // 判断条件：timeout_ms != 0 是否成立。
    if (timeout_ms != 0)
    {
        // 定义变量 TickType_t ticks 并初始化为 (timeout_ms < 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms)。
        TickType_t ticks = (timeout_ms < 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
        // 判断条件：xSemaphoreTake(read_sem_, ticks) != pdTRUE 是否成立。
        if (xSemaphoreTake(read_sem_, ticks) != pdTRUE)
        {
            // 调用 update_underflow();，执行这个函数并处理它的返回值或状态变化。
            update_underflow();
            // 返回 0 给调用者，表示当前函数的执行结果。
            return 0;
        }
    }

    // 判断条件：xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE 是否成立。
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        // 判断条件：timeout_ms != 0 是否成立。
        if (timeout_ms != 0)
            // 调用 xSemaphoreGive(read_sem_);，执行这个函数并处理它的返回值或状态变化。
            xSemaphoreGive(read_sem_);
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    }

    // 定义变量 size_t current 并初始化为 data_size_.load(std::memory_order_relaxed)。
    size_t current = data_size_.load(std::memory_order_relaxed);
    // 定义变量 size_t to_read 并初始化为 (len < current) ? len : current。
    size_t to_read = (len < current) ? len : current;

    // 判断条件：to_read > 0 是否成立。
    if (to_read > 0)
    {
        // 定义变量 size_t rp 并初始化为 read_pos_.load(std::memory_order_relaxed)。
        size_t rp = read_pos_.load(std::memory_order_relaxed);
        // 定义变量 size_t first 并初始化为 capacity_ - rp。
        size_t first = capacity_ - rp;

        // 判断条件：to_read <= first 是否成立。
        if (to_read <= first)
        {
            // 调用 std::memcpy(data, buffer_ + rp, to_read);，执行这个函数并处理它的返回值或状态变化。
            std::memcpy(data, buffer_ + rp, to_read);
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 调用 std::memcpy(data, buffer_ + rp, first);，执行这个函数并处理它的返回值或状态变化。
            std::memcpy(data, buffer_ + rp, first);
            // 调用 std::memcpy(data + first, buffer_, to_read - first);，执行这个函数并处理它的返回值或状态变化。
            std::memcpy(data + first, buffer_, to_read - first);
        }

        // 访问对象成员：read_pos_.store((rp + to_read) % capacity_, std::memory_order_release);；读取或修改这个对象里的成员状态。
        read_pos_.store((rp + to_read) % capacity_, std::memory_order_release);
        // 访问对象成员：data_size_.fetch_sub(to_read, std::memory_order_release);；读取或修改这个对象里的成员状态。
        data_size_.fetch_sub(to_read, std::memory_order_release);

        // 调用 update_read_stats(to_read);，执行这个函数并处理它的返回值或状态变化。
        update_read_stats(to_read);
        // 调用 xSemaphoreGive(write_sem_);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGive(write_sem_);
    }

    // 判断条件：data_size_.load(std::memory_order_acquire) > 0 是否成立。
    if (data_size_.load(std::memory_order_acquire) > 0)
        // 调用 xSemaphoreGive(read_sem_);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGive(read_sem_);

    // 调用 xSemaphoreGive(mutex_);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGive(mutex_);
    // 返回 to_read 给调用者，表示当前函数的执行结果。
    return to_read;
}

// Single byte write
template <bool EnableStats>
// 定义或声明函数 RingBuffer<EnableStats>::write_byte：参数在括号里，返回类型写在函数名前面。
bool RingBuffer<EnableStats>::write_byte(uint8_t byte)
{
    // 判断条件：!is_valid() || free_space_fast() == 0 是否成立。
    if (!is_valid() || free_space_fast() == 0)
    {
        // 调用 update_overflow();，执行这个函数并处理它的返回值或状态变化。
        update_overflow();
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 判断条件：xSemaphoreTake(mutex_, pdMS_TO_TICKS(10)) != pdTRUE 是否成立。
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(10)) != pdTRUE)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 判断条件：data_size_.load(std::memory_order_relaxed) >= capacity_ 是否成立。
    if (data_size_.load(std::memory_order_relaxed) >= capacity_)
    {
        // 调用 xSemaphoreGive(mutex_);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGive(mutex_);
        // 调用 update_overflow();，执行这个函数并处理它的返回值或状态变化。
        update_overflow();
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 定义变量 size_t wp 并初始化为 write_pos_.load(std::memory_order_relaxed)。
    size_t wp = write_pos_.load(std::memory_order_relaxed);
    // 赋值：把右侧 byte 的结果保存到左侧 buffer_[wp]。
    buffer_[wp] = byte;
    // 访问对象成员：write_pos_.store((wp + 1) % capacity_, std::memory_order_release);；读取或修改这个对象里的成员状态。
    write_pos_.store((wp + 1) % capacity_, std::memory_order_release);
    // 访问对象成员：data_size_.fetch_add(1, std::memory_order_release);；读取或修改这个对象里的成员状态。
    data_size_.fetch_add(1, std::memory_order_release);

    // 调用 update_write_stats(1);，执行这个函数并处理它的返回值或状态变化。
    update_write_stats(1);
    // 调用 xSemaphoreGive(read_sem_);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGive(read_sem_);

    // 判断条件：data_size_.load(std::memory_order_acquire) < capacity_ 是否成立。
    if (data_size_.load(std::memory_order_acquire) < capacity_)
        // 调用 xSemaphoreGive(write_sem_);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGive(write_sem_);

    // 调用 xSemaphoreGive(mutex_);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGive(mutex_);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// Single byte read
template <bool EnableStats>
// 定义或声明函数 RingBuffer<EnableStats>::read_byte：参数在括号里，返回类型写在函数名前面。
bool RingBuffer<EnableStats>::read_byte(uint8_t* byte)
{
    // 判断条件：!is_valid() || !byte || available_fast() == 0 是否成立。
    if (!is_valid() || !byte || available_fast() == 0)
    {
        // 调用 update_underflow();，执行这个函数并处理它的返回值或状态变化。
        update_underflow();
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 判断条件：xSemaphoreTake(mutex_, pdMS_TO_TICKS(10)) != pdTRUE 是否成立。
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(10)) != pdTRUE)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 判断条件：data_size_.load(std::memory_order_relaxed) == 0 是否成立。
    if (data_size_.load(std::memory_order_relaxed) == 0)
    {
        // 调用 xSemaphoreGive(mutex_);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGive(mutex_);
        // 调用 update_underflow();，执行这个函数并处理它的返回值或状态变化。
        update_underflow();
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 定义变量 size_t rp 并初始化为 read_pos_.load(std::memory_order_relaxed)。
    size_t rp = read_pos_.load(std::memory_order_relaxed);
    *byte = buffer_[rp];
    // 访问对象成员：read_pos_.store((rp + 1) % capacity_, std::memory_order_release);；读取或修改这个对象里的成员状态。
    read_pos_.store((rp + 1) % capacity_, std::memory_order_release);
    // 访问对象成员：data_size_.fetch_sub(1, std::memory_order_release);；读取或修改这个对象里的成员状态。
    data_size_.fetch_sub(1, std::memory_order_release);

    // 调用 update_read_stats(1);，执行这个函数并处理它的返回值或状态变化。
    update_read_stats(1);
    // 调用 xSemaphoreGive(write_sem_);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGive(write_sem_);

    // 判断条件：data_size_.load(std::memory_order_acquire) > 0 是否成立。
    if (data_size_.load(std::memory_order_acquire) > 0)
        // 调用 xSemaphoreGive(read_sem_);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGive(read_sem_);

    // 调用 xSemaphoreGive(mutex_);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGive(mutex_);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// ISR write
template <bool EnableStats>
// 函数调用：size_t RingBuffer<EnableStats>::write_from_isr(const uint8_t* data, size_t len,；执行该函数并处理它的返回值或副作用。
size_t RingBuffer<EnableStats>::write_from_isr(const uint8_t* data, size_t len,
                                               // 继续上一行的表达式或参数列表：BaseType_t* pxHigherPriorityTaskWoken)。
                                               BaseType_t* pxHigherPriorityTaskWoken)
{
    // 判断条件：!is_valid() || !data || len == 0 是否成立。
    if (!is_valid() || !data || len == 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 判断条件：xSemaphoreTakeFromISR(mutex_, pxHigherPriorityTaskWoken) != pdTRUE 是否成立。
    if (xSemaphoreTakeFromISR(mutex_, pxHigherPriorityTaskWoken) != pdTRUE)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 定义变量 size_t current 并初始化为 data_size_.load(std::memory_order_relaxed)。
    size_t current = data_size_.load(std::memory_order_relaxed);
    // 定义变量 size_t free 并初始化为 capacity_ - current。
    size_t free = capacity_ - current;
    // 定义变量 size_t to_write 并初始化为 (len < free) ? len : free。
    size_t to_write = (len < free) ? len : free;

    // 判断条件：to_write > 0 是否成立。
    if (to_write > 0)
    {
        // 定义变量 size_t wp 并初始化为 write_pos_.load(std::memory_order_relaxed)。
        size_t wp = write_pos_.load(std::memory_order_relaxed);
        // 定义变量 size_t first 并初始化为 capacity_ - wp。
        size_t first = capacity_ - wp;

        // 判断条件：to_write <= first 是否成立。
        if (to_write <= first)
            // 调用 std::memcpy(buffer_ + wp, data, to_write);，执行这个函数并处理它的返回值或状态变化。
            std::memcpy(buffer_ + wp, data, to_write);
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 调用 std::memcpy(buffer_ + wp, data, first);，执行这个函数并处理它的返回值或状态变化。
            std::memcpy(buffer_ + wp, data, first);
            // 调用 std::memcpy(buffer_, data + first, to_write - first);，执行这个函数并处理它的返回值或状态变化。
            std::memcpy(buffer_, data + first, to_write - first);
        }

        // 访问对象成员：write_pos_.store((wp + to_write) % capacity_, std::memory_order_release);；读取或修改这个对象里的成员状态。
        write_pos_.store((wp + to_write) % capacity_, std::memory_order_release);
        // 访问对象成员：data_size_.fetch_add(to_write, std::memory_order_release);；读取或修改这个对象里的成员状态。
        data_size_.fetch_add(to_write, std::memory_order_release);

        // 调用 update_write_stats(to_write);，执行这个函数并处理它的返回值或状态变化。
        update_write_stats(to_write);
        // 调用 xSemaphoreGiveFromISR(read_sem_, pxHigherPriorityTaskWoken);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGiveFromISR(read_sem_, pxHigherPriorityTaskWoken);
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 调用 update_overflow();，执行这个函数并处理它的返回值或状态变化。
        update_overflow();
    }

    // 判断条件：data_size_.load(std::memory_order_acquire) < capacity_ 是否成立。
    if (data_size_.load(std::memory_order_acquire) < capacity_)
        // 调用 xSemaphoreGiveFromISR(write_sem_, pxHigherPriorityTaskWoken);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGiveFromISR(write_sem_, pxHigherPriorityTaskWoken);

    // 调用 xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);
    // 返回 to_write 给调用者，表示当前函数的执行结果。
    return to_write;
}

// ISR read
template <bool EnableStats>
// 函数调用：size_t RingBuffer<EnableStats>::read_from_isr(uint8_t* data, size_t len,；执行该函数并处理它的返回值或副作用。
size_t RingBuffer<EnableStats>::read_from_isr(uint8_t* data, size_t len,
                                              // 继续上一行的表达式或参数列表：BaseType_t* pxHigherPriorityTaskWoken)。
                                              BaseType_t* pxHigherPriorityTaskWoken)
{
    // 判断条件：!is_valid() || !data || len == 0 是否成立。
    if (!is_valid() || !data || len == 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 判断条件：xSemaphoreTakeFromISR(mutex_, pxHigherPriorityTaskWoken) != pdTRUE 是否成立。
    if (xSemaphoreTakeFromISR(mutex_, pxHigherPriorityTaskWoken) != pdTRUE)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 定义变量 size_t current 并初始化为 data_size_.load(std::memory_order_relaxed)。
    size_t current = data_size_.load(std::memory_order_relaxed);
    // 定义变量 size_t to_read 并初始化为 (len < current) ? len : current。
    size_t to_read = (len < current) ? len : current;

    // 判断条件：to_read > 0 是否成立。
    if (to_read > 0)
    {
        // 定义变量 size_t rp 并初始化为 read_pos_.load(std::memory_order_relaxed)。
        size_t rp = read_pos_.load(std::memory_order_relaxed);
        // 定义变量 size_t first 并初始化为 capacity_ - rp。
        size_t first = capacity_ - rp;

        // 判断条件：to_read <= first 是否成立。
        if (to_read <= first)
            // 调用 std::memcpy(data, buffer_ + rp, to_read);，执行这个函数并处理它的返回值或状态变化。
            std::memcpy(data, buffer_ + rp, to_read);
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 调用 std::memcpy(data, buffer_ + rp, first);，执行这个函数并处理它的返回值或状态变化。
            std::memcpy(data, buffer_ + rp, first);
            // 调用 std::memcpy(data + first, buffer_, to_read - first);，执行这个函数并处理它的返回值或状态变化。
            std::memcpy(data + first, buffer_, to_read - first);
        }

        // 访问对象成员：read_pos_.store((rp + to_read) % capacity_, std::memory_order_release);；读取或修改这个对象里的成员状态。
        read_pos_.store((rp + to_read) % capacity_, std::memory_order_release);
        // 访问对象成员：data_size_.fetch_sub(to_read, std::memory_order_release);；读取或修改这个对象里的成员状态。
        data_size_.fetch_sub(to_read, std::memory_order_release);

        // 调用 update_read_stats(to_read);，执行这个函数并处理它的返回值或状态变化。
        update_read_stats(to_read);
        // 调用 xSemaphoreGiveFromISR(write_sem_, pxHigherPriorityTaskWoken);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGiveFromISR(write_sem_, pxHigherPriorityTaskWoken);
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 调用 update_underflow();，执行这个函数并处理它的返回值或状态变化。
        update_underflow();
    }

    // 判断条件：data_size_.load(std::memory_order_acquire) > 0 是否成立。
    if (data_size_.load(std::memory_order_acquire) > 0)
        // 调用 xSemaphoreGiveFromISR(read_sem_, pxHigherPriorityTaskWoken);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGiveFromISR(read_sem_, pxHigherPriorityTaskWoken);

    // 调用 xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);
    // 返回 to_read 给调用者，表示当前函数的执行结果。
    return to_read;
}

// ISR single byte write
template <bool EnableStats>
// 函数调用：bool RingBuffer<EnableStats>::write_byte_from_isr(uint8_t byte,；执行该函数并处理它的返回值或副作用。
bool RingBuffer<EnableStats>::write_byte_from_isr(uint8_t byte,
                                                  // 继续上一行的表达式或参数列表：BaseType_t* pxHigherPriorityTaskWoken)。
                                                  BaseType_t* pxHigherPriorityTaskWoken)
{
    // 判断条件：!is_valid() 是否成立。
    if (!is_valid())
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 判断条件：xSemaphoreTakeFromISR(mutex_, pxHigherPriorityTaskWoken) != pdTRUE 是否成立。
    if (xSemaphoreTakeFromISR(mutex_, pxHigherPriorityTaskWoken) != pdTRUE)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 判断条件：data_size_.load(std::memory_order_relaxed) >= capacity_ 是否成立。
    if (data_size_.load(std::memory_order_relaxed) >= capacity_)
    {
        // 调用 xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);
        // 调用 update_overflow();，执行这个函数并处理它的返回值或状态变化。
        update_overflow();
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 定义变量 size_t wp 并初始化为 write_pos_.load(std::memory_order_relaxed)。
    size_t wp = write_pos_.load(std::memory_order_relaxed);
    // 赋值：把右侧 byte 的结果保存到左侧 buffer_[wp]。
    buffer_[wp] = byte;
    // 访问对象成员：write_pos_.store((wp + 1) % capacity_, std::memory_order_release);；读取或修改这个对象里的成员状态。
    write_pos_.store((wp + 1) % capacity_, std::memory_order_release);
    // 访问对象成员：data_size_.fetch_add(1, std::memory_order_release);；读取或修改这个对象里的成员状态。
    data_size_.fetch_add(1, std::memory_order_release);

    // 调用 update_write_stats(1);，执行这个函数并处理它的返回值或状态变化。
    update_write_stats(1);
    // 调用 xSemaphoreGiveFromISR(read_sem_, pxHigherPriorityTaskWoken);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGiveFromISR(read_sem_, pxHigherPriorityTaskWoken);

    // 判断条件：data_size_.load(std::memory_order_acquire) < capacity_ 是否成立。
    if (data_size_.load(std::memory_order_acquire) < capacity_)
        // 调用 xSemaphoreGiveFromISR(write_sem_, pxHigherPriorityTaskWoken);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGiveFromISR(write_sem_, pxHigherPriorityTaskWoken);

    // 调用 xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// ISR single byte read
template <bool EnableStats>
// 函数调用：bool RingBuffer<EnableStats>::read_byte_from_isr(uint8_t* byte,；执行该函数并处理它的返回值或副作用。
bool RingBuffer<EnableStats>::read_byte_from_isr(uint8_t* byte,
                                                 // 继续上一行的表达式或参数列表：BaseType_t* pxHigherPriorityTaskWoken)。
                                                 BaseType_t* pxHigherPriorityTaskWoken)
{
    // 判断条件：!is_valid() || !byte 是否成立。
    if (!is_valid() || !byte)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 判断条件：xSemaphoreTakeFromISR(mutex_, pxHigherPriorityTaskWoken) != pdTRUE 是否成立。
    if (xSemaphoreTakeFromISR(mutex_, pxHigherPriorityTaskWoken) != pdTRUE)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 判断条件：data_size_.load(std::memory_order_relaxed) == 0 是否成立。
    if (data_size_.load(std::memory_order_relaxed) == 0)
    {
        // 调用 xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);
        // 调用 update_underflow();，执行这个函数并处理它的返回值或状态变化。
        update_underflow();
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 定义变量 size_t rp 并初始化为 read_pos_.load(std::memory_order_relaxed)。
    size_t rp = read_pos_.load(std::memory_order_relaxed);
    *byte = buffer_[rp];
    // 访问对象成员：read_pos_.store((rp + 1) % capacity_, std::memory_order_release);；读取或修改这个对象里的成员状态。
    read_pos_.store((rp + 1) % capacity_, std::memory_order_release);
    // 访问对象成员：data_size_.fetch_sub(1, std::memory_order_release);；读取或修改这个对象里的成员状态。
    data_size_.fetch_sub(1, std::memory_order_release);

    // 调用 update_read_stats(1);，执行这个函数并处理它的返回值或状态变化。
    update_read_stats(1);
    // 调用 xSemaphoreGiveFromISR(write_sem_, pxHigherPriorityTaskWoken);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGiveFromISR(write_sem_, pxHigherPriorityTaskWoken);

    // 判断条件：data_size_.load(std::memory_order_acquire) > 0 是否成立。
    if (data_size_.load(std::memory_order_acquire) > 0)
        // 调用 xSemaphoreGiveFromISR(read_sem_, pxHigherPriorityTaskWoken);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGiveFromISR(read_sem_, pxHigherPriorityTaskWoken);

    // 调用 xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGiveFromISR(mutex_, pxHigherPriorityTaskWoken);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// Peek
template <bool EnableStats>
// 定义或声明函数 RingBuffer<EnableStats>::peek：参数在括号里，返回类型写在函数名前面。
size_t RingBuffer<EnableStats>::peek(uint8_t* data, size_t len) const
{
    // 判断条件：!is_valid() || !data || len == 0 是否成立。
    if (!is_valid() || !data || len == 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 判断条件：xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE 是否成立。
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 定义变量 size_t current 并初始化为 data_size_.load(std::memory_order_relaxed)。
    size_t current = data_size_.load(std::memory_order_relaxed);
    // 定义变量 size_t to_peek 并初始化为 (len < current) ? len : current。
    size_t to_peek = (len < current) ? len : current;

    // 判断条件：to_peek > 0 是否成立。
    if (to_peek > 0)
    {
        // 定义变量 size_t rp 并初始化为 read_pos_.load(std::memory_order_relaxed)。
        size_t rp = read_pos_.load(std::memory_order_relaxed);
        // 定义变量 size_t first 并初始化为 capacity_ - rp。
        size_t first = capacity_ - rp;

        // 判断条件：to_peek <= first 是否成立。
        if (to_peek <= first)
            // 调用 std::memcpy(data, buffer_ + rp, to_peek);，执行这个函数并处理它的返回值或状态变化。
            std::memcpy(data, buffer_ + rp, to_peek);
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 调用 std::memcpy(data, buffer_ + rp, first);，执行这个函数并处理它的返回值或状态变化。
            std::memcpy(data, buffer_ + rp, first);
            // 调用 std::memcpy(data + first, buffer_, to_peek - first);，执行这个函数并处理它的返回值或状态变化。
            std::memcpy(data + first, buffer_, to_peek - first);
        }
    }

    // 调用 xSemaphoreGive(mutex_);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGive(mutex_);
    // 返回 to_peek 给调用者，表示当前函数的执行结果。
    return to_peek;
}

// Skip
template <bool EnableStats>
// 定义或声明函数 RingBuffer<EnableStats>::skip：参数在括号里，返回类型写在函数名前面。
size_t RingBuffer<EnableStats>::skip(size_t len)
{
    // 判断条件：!is_valid() || len == 0 是否成立。
    if (!is_valid() || len == 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 判断条件：xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE 是否成立。
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 定义变量 size_t current 并初始化为 data_size_.load(std::memory_order_relaxed)。
    size_t current = data_size_.load(std::memory_order_relaxed);
    // 定义变量 size_t to_skip 并初始化为 (len < current) ? len : current。
    size_t to_skip = (len < current) ? len : current;

    // 判断条件：to_skip > 0 是否成立。
    if (to_skip > 0)
    {
        // 定义变量 size_t rp 并初始化为 read_pos_.load(std::memory_order_relaxed)。
        size_t rp = read_pos_.load(std::memory_order_relaxed);
        // 访问对象成员：read_pos_.store((rp + to_skip) % capacity_, std::memory_order_release);；读取或修改这个对象里的成员状态。
        read_pos_.store((rp + to_skip) % capacity_, std::memory_order_release);
        // 访问对象成员：data_size_.fetch_sub(to_skip, std::memory_order_release);；读取或修改这个对象里的成员状态。
        data_size_.fetch_sub(to_skip, std::memory_order_release);
        // 调用 xSemaphoreGive(write_sem_);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGive(write_sem_);
    }

    // 判断条件：data_size_.load(std::memory_order_acquire) > 0 是否成立。
    if (data_size_.load(std::memory_order_acquire) > 0)
        // 调用 xSemaphoreGive(read_sem_);，执行这个函数并处理它的返回值或状态变化。
        xSemaphoreGive(read_sem_);

    // 调用 xSemaphoreGive(mutex_);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGive(mutex_);
    // 返回 to_skip 给调用者，表示当前函数的执行结果。
    return to_skip;
}

// Clear
template <bool EnableStats>
// 函数定义/声明：调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
void RingBuffer<EnableStats>::clear()
{
    // 判断条件：!is_valid() 是否成立。
    if (!is_valid())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 判断条件：xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE 是否成立。
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 访问对象成员：read_pos_.store(0, std::memory_order_release);；读取或修改这个对象里的成员状态。
    read_pos_.store(0, std::memory_order_release);
    // 访问对象成员：write_pos_.store(0, std::memory_order_release);；读取或修改这个对象里的成员状态。
    write_pos_.store(0, std::memory_order_release);
    // 访问对象成员：data_size_.store(0, std::memory_order_release);；读取或修改这个对象里的成员状态。
    data_size_.store(0, std::memory_order_release);

    // 调用 xSemaphoreTake(read_sem_, 0);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreTake(read_sem_, 0);
    // 调用 xSemaphoreGive(write_sem_);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGive(write_sem_);
    // 调用 xSemaphoreGive(mutex_);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGive(mutex_);
}

// Available (locked)
template <bool EnableStats>
// 定义或声明函数 RingBuffer<EnableStats>::available：参数在括号里，返回类型写在函数名前面。
size_t RingBuffer<EnableStats>::available() const
{
    // 判断条件：!is_valid() 是否成立。
    if (!is_valid())
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 判断条件：xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE 是否成立。
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE)
        // 返回 available_fast() 给调用者，表示当前函数的执行结果。
        return available_fast();

    // 定义变量 size_t result 并初始化为 data_size_.load(std::memory_order_acquire)。
    size_t result = data_size_.load(std::memory_order_acquire);
    // 调用 xSemaphoreGive(mutex_);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGive(mutex_);
    // 返回 result 给调用者，表示当前函数的执行结果。
    return result;
}

// Free space (locked)
template <bool EnableStats>
// 定义或声明函数 RingBuffer<EnableStats>::free_space：参数在括号里，返回类型写在函数名前面。
size_t RingBuffer<EnableStats>::free_space() const
{
    // 判断条件：!is_valid() 是否成立。
    if (!is_valid())
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 判断条件：xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE 是否成立。
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE)
        // 返回 free_space_fast() 给调用者，表示当前函数的执行结果。
        return free_space_fast();

    // 定义变量 size_t result 并初始化为 capacity_ - data_size_.load(std::memory_order_acquire)。
    size_t result = capacity_ - data_size_.load(std::memory_order_acquire);
    // 调用 xSemaphoreGive(mutex_);，执行这个函数并处理它的返回值或状态变化。
    xSemaphoreGive(mutex_);
    // 返回 result 给调用者，表示当前函数的执行结果。
    return result;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::common::memory::ring_buffer
