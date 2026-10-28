// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/common/memory/ring_buffer/ring_buffer.hpp
 * 作用：实现 ring buffer 模块中的 ring buffer 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：PSRAM 能缓解内部内存压力，但不能无视 DMA 和中断约束。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstddef>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <atomic>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "freertos/FreeRTOS.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "freertos/semphr.h"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::common::memory::ring_buffer {

// struct Stats：把相关数据字段组合在一起，表示项目中的一个概念。
struct Stats
{
    // 执行语句：std::atomic<uint32_t> write_count{0};；它会更新一次变量、对象或程序状态。
    std::atomic<uint32_t> write_count{0};
    // 执行语句：std::atomic<uint32_t> read_count{0};；它会更新一次变量、对象或程序状态。
    std::atomic<uint32_t> read_count{0};
    // 执行语句：std::atomic<uint32_t> write_bytes{0};；它会更新一次变量、对象或程序状态。
    std::atomic<uint32_t> write_bytes{0};
    // 执行语句：std::atomic<uint32_t> read_bytes{0};；它会更新一次变量、对象或程序状态。
    std::atomic<uint32_t> read_bytes{0};
    // 执行语句：std::atomic<uint32_t> overflow_count{0};；它会更新一次变量、对象或程序状态。
    std::atomic<uint32_t> overflow_count{0};
    // 执行语句：std::atomic<uint32_t> underflow_count{0};；它会更新一次变量、对象或程序状态。
    std::atomic<uint32_t> underflow_count{0};

    // 函数定义/声明：调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    void reset()
    {
        // 访问对象成员：write_count.store(0, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
        write_count.store(0, std::memory_order_relaxed);
        // 访问对象成员：read_count.store(0, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
        read_count.store(0, std::memory_order_relaxed);
        // 访问对象成员：write_bytes.store(0, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
        write_bytes.store(0, std::memory_order_relaxed);
        // 访问对象成员：read_bytes.store(0, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
        read_bytes.store(0, std::memory_order_relaxed);
        // 访问对象成员：overflow_count.store(0, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
        overflow_count.store(0, std::memory_order_relaxed);
        // 访问对象成员：underflow_count.store(0, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
        underflow_count.store(0, std::memory_order_relaxed);
    }
};

/** @brief 线程安全环形缓冲，EnableStats 控制统计 */
// 模板声明：让同一段代码适配不同类型。
template <bool EnableStats = false>
// 类定义：把数据和操作这些数据的方法绑定在一起。
class RingBuffer
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 赋值：把右侧 true, size_t alignment = 4) 的结果保存到左侧 explicit RingBuffer(size_t capacity, bool use_psram。
    explicit RingBuffer(size_t capacity, bool use_psram = true, size_t alignment = 4);
    // 调用 RingBuffer(uint8_t* static_buffer, size_t capacity);，执行这个函数并处理它的返回值或状态变化。
    RingBuffer(uint8_t* static_buffer, size_t capacity);

    // 函数调用：~RingBuffer();；执行该函数并处理它的返回值或副作用。
    ~RingBuffer();

    // 调用 RingBuffer(const RingBuffer&) = delete;，执行这个函数并处理它的返回值或状态变化。
    RingBuffer(const RingBuffer&) = delete;
    // 赋值：把右侧 (const RingBuffer&) = delete 的结果保存到左侧 RingBuffer& operator。
    RingBuffer& operator=(const RingBuffer&) = delete;
    // 调用 RingBuffer(RingBuffer&&) = delete;，执行这个函数并处理它的返回值或状态变化。
    RingBuffer(RingBuffer&&) = delete;
    // 赋值：把右侧 (RingBuffer&&) = delete 的结果保存到左侧 RingBuffer& operator。
    RingBuffer& operator=(RingBuffer&&) = delete;

    // 赋值：把右侧 0) 的结果保存到左侧 size_t write(const uint8_t* data, size_t len, int timeout_ms。
    size_t write(const uint8_t* data, size_t len, int timeout_ms = 0);
    // 赋值：把右侧 0) 的结果保存到左侧 size_t read(uint8_t* data, size_t len, int timeout_ms。
    size_t read(uint8_t* data, size_t len, int timeout_ms = 0);
    // 函数调用：size_t peek(uint8_t* data, size_t len) const;；执行该函数并处理它的返回值或副作用。
    size_t peek(uint8_t* data, size_t len) const;
    // 函数调用：size_t skip(size_t len);；执行该函数并处理它的返回值或副作用。
    size_t skip(size_t len);
    // 调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    void clear();

    // 函数调用：bool write_byte(uint8_t byte);；执行该函数并处理它的返回值或副作用。
    bool write_byte(uint8_t byte);
    // 函数调用：bool read_byte(uint8_t* byte);；执行该函数并处理它的返回值或副作用。
    bool read_byte(uint8_t* byte);

    // 模板声明：让同一段代码适配不同类型。
    template <size_t N>
    // 定义或声明函数 write_fixed：参数在括号里，返回类型写在函数名前面。
    size_t write_fixed(const uint8_t (&data)[N])
    {
        // 返回 write(data, N, 0) 给调用者，表示当前函数的执行结果。
        return write(data, N, 0);
    }

    // 模板声明：让同一段代码适配不同类型。
    template <size_t N>
    // 定义或声明函数 read_fixed：参数在括号里，返回类型写在函数名前面。
    size_t read_fixed(uint8_t (&data)[N])
    {
        // 返回 read(data, N, 0) 给调用者，表示当前函数的执行结果。
        return read(data, N, 0);
    }

    // 函数调用：size_t write_from_isr(const uint8_t* data, size_t len, BaseType_t* pxHigherPriorityTaskWoken);；执行该函数并处理它的返回值或副作用。
    size_t write_from_isr(const uint8_t* data, size_t len, BaseType_t* pxHigherPriorityTaskWoken);
    // 函数调用：size_t read_from_isr(uint8_t* data, size_t len, BaseType_t* pxHigherPriorityTaskWoken);；执行该函数并处理它的返回值或副作用。
    size_t read_from_isr(uint8_t* data, size_t len, BaseType_t* pxHigherPriorityTaskWoken);
    // 函数调用：bool write_byte_from_isr(uint8_t byte, BaseType_t* pxHigherPriorityTaskWoken);；执行该函数并处理它的返回值或副作用。
    bool write_byte_from_isr(uint8_t byte, BaseType_t* pxHigherPriorityTaskWoken);
    // 函数调用：bool read_byte_from_isr(uint8_t* byte, BaseType_t* pxHigherPriorityTaskWoken);；执行该函数并处理它的返回值或副作用。
    bool read_byte_from_isr(uint8_t* byte, BaseType_t* pxHigherPriorityTaskWoken);

    // 定义或声明函数 available_fast：参数在括号里，返回类型写在函数名前面。
    size_t available_fast() const
    {
        // 返回 data_size_.load(std::memory_order_acquire) 给调用者，表示当前函数的执行结果。
        return data_size_.load(std::memory_order_acquire);
    }
    // 定义或声明函数 free_space_fast：参数在括号里，返回类型写在函数名前面。
    size_t free_space_fast() const
    {
        // 返回 capacity_ - data_size_.load(std::memory_order_acquire) 给调用者，表示当前函数的执行结果。
        return capacity_ - data_size_.load(std::memory_order_acquire);
    }

    // 函数调用：size_t available() const;；执行该函数并处理它的返回值或副作用。
    size_t available() const;
    // 函数调用：size_t free_space() const;；执行该函数并处理它的返回值或副作用。
    size_t free_space() const;

    // 定义或声明函数 capacity：参数在括号里，返回类型写在函数名前面。
    size_t capacity() const
    {
        // 返回 capacity_ 给调用者，表示当前函数的执行结果。
        return capacity_;
    }
    // 定义或声明函数 is_empty：参数在括号里，返回类型写在函数名前面。
    bool is_empty() const
    {
        // 返回 available_fast() == 0 给调用者，表示当前函数的执行结果。
        return available_fast() == 0;
    }
    // 定义或声明函数 is_full：参数在括号里，返回类型写在函数名前面。
    bool is_full() const
    {
        // 返回 free_space_fast() == 0 给调用者，表示当前函数的执行结果。
        return free_space_fast() == 0;
    }
    // 定义或声明函数 is_valid：参数在括号里，返回类型写在函数名前面。
    bool is_valid() const
    {
        // 返回 buffer_ != nullptr && mutex_ != nullptr 给调用者，表示当前函数的执行结果。
        return buffer_ != nullptr && mutex_ != nullptr;
    }
    // 定义或声明函数 is_static_buffer：参数在括号里，返回类型写在函数名前面。
    bool is_static_buffer() const
    {
        // 返回 is_static_ 给调用者，表示当前函数的执行结果。
        return is_static_;
    }
    // 定义或声明函数 get_buffer：参数在括号里，返回类型写在函数名前面。
    uint8_t* get_buffer()
    {
        // 返回 buffer_ 给调用者，表示当前函数的执行结果。
        return buffer_;
    }
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t* get_buffer() const
    {
        // 返回 buffer_ 给调用者，表示当前函数的执行结果。
        return buffer_;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const Stats* get_stats() const
    {
        // 定义或声明函数 constexpr：参数在括号里，返回类型写在函数名前面。
        if constexpr (EnableStats)
            // 返回 &stats_ 给调用者，表示当前函数的执行结果。
            return &stats_;
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;
    }

    // 定义或声明函数 reset_stats：参数在括号里，返回类型写在函数名前面。
    void reset_stats()
    {
        // 定义或声明函数 constexpr：参数在括号里，返回类型写在函数名前面。
        if constexpr (EnableStats)
            // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
            stats_.reset();
    }

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 函数调用：void init_semaphores();；执行该函数并处理它的返回值或副作用。
    void init_semaphores();
    // 函数调用：void update_write_stats(size_t bytes);；执行该函数并处理它的返回值或副作用。
    void update_write_stats(size_t bytes);
    // 函数调用：void update_read_stats(size_t bytes);；执行该函数并处理它的返回值或副作用。
    void update_read_stats(size_t bytes);
    // 函数调用：void update_overflow();；执行该函数并处理它的返回值或副作用。
    void update_overflow();
    // 函数调用：void update_underflow();；执行该函数并处理它的返回值或副作用。
    void update_underflow();

    // 执行语句：uint8_t* buffer_{nullptr};；它会更新一次变量、对象或程序状态。
    uint8_t* buffer_{nullptr};
    // 执行语句：size_t capacity_{0};；它会更新一次变量、对象或程序状态。
    size_t capacity_{0};
    // 执行语句：std::atomic<size_t> read_pos_{0};；它会更新一次变量、对象或程序状态。
    std::atomic<size_t> read_pos_{0};
    // 执行语句：std::atomic<size_t> write_pos_{0};；它会更新一次变量、对象或程序状态。
    std::atomic<size_t> write_pos_{0};
    // 执行语句：std::atomic<size_t> data_size_{0};；它会更新一次变量、对象或程序状态。
    std::atomic<size_t> data_size_{0};
    // 执行语句：SemaphoreHandle_t mutex_{nullptr};；它会更新一次变量、对象或程序状态。
    SemaphoreHandle_t mutex_{nullptr};
    // 执行语句：SemaphoreHandle_t read_sem_{nullptr};；它会更新一次变量、对象或程序状态。
    SemaphoreHandle_t read_sem_{nullptr};
    // 执行语句：SemaphoreHandle_t write_sem_{nullptr};；它会更新一次变量、对象或程序状态。
    SemaphoreHandle_t write_sem_{nullptr};
    // 执行语句：bool is_static_{false};；它会更新一次变量、对象或程序状态。
    bool is_static_{false};
    // 执行语句：Stats stats_;；它会更新一次变量、对象或程序状态。
    Stats stats_;
};

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using RingBufferBasic = RingBuffer<false>;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using RingBufferWithStats = RingBuffer<true>;

// 命名空间结束：离开当前模块作用域。
} // namespace app::common::memory::ring_buffer

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ring_buffer_impl.hpp"
