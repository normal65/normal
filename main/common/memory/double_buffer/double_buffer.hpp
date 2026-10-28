// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/common/memory/double_buffer/double_buffer.hpp
 * 作用：实现 double buffer 模块中的 double buffer 功能。
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
namespace app::common::memory::double_buffer {

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class BufferState : uint8_t
{
    // 执行这条代码：EMPTY,。它会更新变量、对象状态或调用链。
    EMPTY,
    // 继续上一行的表达式或参数列表：WRITING,。
    WRITING,
    // 继续上一行的表达式或参数列表：READY,。
    READY,
    // 继续上一行的表达式或参数列表：READING。
    READING
};

// struct Stats：把相关数据字段组合在一起，表示项目中的一个概念。
struct Stats
{
    // 执行语句：std::atomic<uint32_t> write_count{0};；它会更新一次变量、对象或程序状态。
    std::atomic<uint32_t> write_count{0};
    // 执行语句：std::atomic<uint32_t> read_count{0};；它会更新一次变量、对象或程序状态。
    std::atomic<uint32_t> read_count{0};
    // 执行语句：std::atomic<uint32_t> swap_count{0};；它会更新一次变量、对象或程序状态。
    std::atomic<uint32_t> swap_count{0};
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
        // 访问对象成员：swap_count.store(0, std::memory_order_relaxed);；读取或修改这个对象里的成员状态。
        swap_count.store(0, std::memory_order_relaxed);
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

/** @brief 零拷贝双缓冲，EnableStats 控制统计 */
// 模板声明：让同一段代码适配不同类型。
template <bool EnableStats = false>
// 类定义：把数据和操作这些数据的方法绑定在一起。
class DoubleBuffer
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 赋值：把右侧 true, size_t alignment = 4) 的结果保存到左侧 explicit DoubleBuffer(size_t buffer_size, bool use_psram。
    explicit DoubleBuffer(size_t buffer_size, bool use_psram = true, size_t alignment = 4);
    // 调用 DoubleBuffer(uint8_t* buffer0, uint8_t* buffer1, size_t buffer_size);，执行这个函数并处理它的返回值或状态变化。
    DoubleBuffer(uint8_t* buffer0, uint8_t* buffer1, size_t buffer_size);

    // 函数调用：~DoubleBuffer();；执行该函数并处理它的返回值或副作用。
    ~DoubleBuffer();

    // 调用 DoubleBuffer(const DoubleBuffer&) = delete;，执行这个函数并处理它的返回值或状态变化。
    DoubleBuffer(const DoubleBuffer&) = delete;
    // 赋值：把右侧 (const DoubleBuffer&) = delete 的结果保存到左侧 DoubleBuffer& operator。
    DoubleBuffer& operator=(const DoubleBuffer&) = delete;
    // 调用 DoubleBuffer(DoubleBuffer&&) = delete;，执行这个函数并处理它的返回值或状态变化。
    DoubleBuffer(DoubleBuffer&&) = delete;
    // 赋值：把右侧 (DoubleBuffer&&) = delete 的结果保存到左侧 DoubleBuffer& operator。
    DoubleBuffer& operator=(DoubleBuffer&&) = delete;

    // 函数调用：uint8_t* acquire_write_buffer();；执行该函数并处理它的返回值或副作用。
    uint8_t* acquire_write_buffer();
    // 函数调用：void release_write_buffer(size_t data_size);；执行该函数并处理它的返回值或副作用。
    void release_write_buffer(size_t data_size);
    // 函数调用：uint8_t* acquire_read_buffer(size_t* data_size);；执行该函数并处理它的返回值或副作用。
    uint8_t* acquire_read_buffer(size_t* data_size);
    // 函数调用：void release_read_buffer();；执行该函数并处理它的返回值或副作用。
    void release_read_buffer();

    // 函数调用：uint8_t* acquire_write_buffer_from_isr(BaseType_t* pxHigherPriorityTaskWoken);；执行该函数并处理它的返回值或副作用。
    uint8_t* acquire_write_buffer_from_isr(BaseType_t* pxHigherPriorityTaskWoken);
    // 函数调用：void release_write_buffer_from_isr(size_t data_size, BaseType_t* pxHigherPriorityTaskWoken);；执行该函数并处理它的返回值或副作用。
    void release_write_buffer_from_isr(size_t data_size, BaseType_t* pxHigherPriorityTaskWoken);
    // 函数调用：uint8_t* acquire_read_buffer_from_isr(size_t* data_size, BaseType_t* pxHigherPriorityTaskWoken);；执行该函数并处理它的返回值或副作用。
    uint8_t* acquire_read_buffer_from_isr(size_t* data_size, BaseType_t* pxHigherPriorityTaskWoken);
    // 函数调用：void release_read_buffer_from_isr(BaseType_t* pxHigherPriorityTaskWoken);；执行该函数并处理它的返回值或副作用。
    void release_read_buffer_from_isr(BaseType_t* pxHigherPriorityTaskWoken);

    // 函数调用：bool write(const uint8_t* data, size_t len);；执行该函数并处理它的返回值或副作用。
    bool write(const uint8_t* data, size_t len);
    // 函数调用：size_t read(uint8_t* data, size_t max_len);；执行该函数并处理它的返回值或副作用。
    size_t read(uint8_t* data, size_t max_len);

    // 定义或声明函数 has_data_fast：参数在括号里，返回类型写在函数名前面。
    bool has_data_fast() const
    {
        // 返回 states_[0].load(std::memory_order_acquire) == BufferState::READY || 给调用者，表示当前函数的执行结果。
        return states_[0].load(std::memory_order_acquire) == BufferState::READY ||
               // 继续上一行的表达式或参数列表：states_[1].load(std::memory_order_acquire) == BufferState::READY;。
               states_[1].load(std::memory_order_acquire) == BufferState::READY;
    }

    // 定义或声明函数 can_write_fast：参数在括号里，返回类型写在函数名前面。
    bool can_write_fast() const
    {
        // 返回 states_[0].load(std::memory_order_acquire) == BufferState::EMPTY || 给调用者，表示当前函数的执行结果。
        return states_[0].load(std::memory_order_acquire) == BufferState::EMPTY ||
               // 继续上一行的表达式或参数列表：states_[1].load(std::memory_order_acquire) == BufferState::EMPTY;。
               states_[1].load(std::memory_order_acquire) == BufferState::EMPTY;
    }

    // 函数调用：bool has_data() const;；执行该函数并处理它的返回值或副作用。
    bool has_data() const;
    // 函数调用：bool can_write() const;；执行该函数并处理它的返回值或副作用。
    bool can_write() const;

    // 定义或声明函数 get_buffer_size：参数在括号里，返回类型写在函数名前面。
    size_t get_buffer_size() const
    {
        // 返回 buffer_size_ 给调用者，表示当前函数的执行结果。
        return buffer_size_;
    }
    // 定义或声明函数 is_valid：参数在括号里，返回类型写在函数名前面。
    bool is_valid() const
    {
        // 返回 buffers_[0] && buffers_[1] && mutex_ 给调用者，表示当前函数的执行结果。
        return buffers_[0] && buffers_[1] && mutex_;
    }
    // 定义或声明函数 is_static_buffer：参数在括号里，返回类型写在函数名前面。
    bool is_static_buffer() const
    {
        // 返回 is_static_ 给调用者，表示当前函数的执行结果。
        return is_static_;
    }
    // 定义或声明函数 get_buffer_state：参数在括号里，返回类型写在函数名前面。
    BufferState get_buffer_state(int i) const
    {
        // 返回 (i < 0 || i > 1) ? BufferState::EMPTY : states_[i].load(std::memory_order_acquire) 给调用者，表示当前函数的执行结果。
        return (i < 0 || i > 1) ? BufferState::EMPTY : states_[i].load(std::memory_order_acquire);
    }
    // 定义或声明函数 get_data_size：参数在括号里，返回类型写在函数名前面。
    size_t get_data_size(int i) const
    {
        // 返回 (i < 0 || i > 1) ? 0 : data_sizes_[i].load(std::memory_order_acquire) 给调用者，表示当前函数的执行结果。
        return (i < 0 || i > 1) ? 0 : data_sizes_[i].load(std::memory_order_acquire);
    }
    // 定义或声明函数 get_buffer：参数在括号里，返回类型写在函数名前面。
    uint8_t* get_buffer(int i)
    {
        // 返回 (i < 0 || i > 1) ? nullptr : buffers_[i] 给调用者，表示当前函数的执行结果。
        return (i < 0 || i > 1) ? nullptr : buffers_[i];
    }
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t* get_buffer(int i) const
    {
        // 返回 (i < 0 || i > 1) ? nullptr : buffers_[i] 给调用者，表示当前函数的执行结果。
        return (i < 0 || i > 1) ? nullptr : buffers_[i];
    }

    // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    void reset();

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
    // 函数调用：void init_mutex();；执行该函数并处理它的返回值或副作用。
    void init_mutex();
    // 函数调用：int find_buffer(BufferState state) const;；执行该函数并处理它的返回值或副作用。
    int find_buffer(BufferState state) const;
    // 函数调用：void update_write_stats(size_t bytes);；执行该函数并处理它的返回值或副作用。
    void update_write_stats(size_t bytes);
    // 函数调用：void update_read_stats(size_t bytes);；执行该函数并处理它的返回值或副作用。
    void update_read_stats(size_t bytes);
    // 函数调用：void update_overflow();；执行该函数并处理它的返回值或副作用。
    void update_overflow();
    // 函数调用：void update_underflow();；执行该函数并处理它的返回值或副作用。
    void update_underflow();

    // 执行语句：uint8_t* buffers_[2]{nullptr, nullptr};；它会更新一次变量、对象或程序状态。
    uint8_t* buffers_[2]{nullptr, nullptr};
    // 执行语句：size_t buffer_size_{0};；它会更新一次变量、对象或程序状态。
    size_t buffer_size_{0};
    // 执行语句：std::atomic<BufferState> states_[2];；它会更新一次变量、对象或程序状态。
    std::atomic<BufferState> states_[2];
    // 执行语句：std::atomic<size_t> data_sizes_[2];；它会更新一次变量、对象或程序状态。
    std::atomic<size_t> data_sizes_[2];
    // 执行语句：std::atomic<int> write_index_{0};；它会更新一次变量、对象或程序状态。
    std::atomic<int> write_index_{0};
    // 执行语句：std::atomic<int> read_index_{0};；它会更新一次变量、对象或程序状态。
    std::atomic<int> read_index_{0};
    // 执行语句：SemaphoreHandle_t mutex_{nullptr};；它会更新一次变量、对象或程序状态。
    SemaphoreHandle_t mutex_{nullptr};
    // 执行语句：bool is_static_{false};；它会更新一次变量、对象或程序状态。
    bool is_static_{false};
    // 执行语句：Stats stats_;；它会更新一次变量、对象或程序状态。
    Stats stats_;
};

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using DoubleBufferBasic = DoubleBuffer<false>;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using DoubleBufferWithStats = DoubleBuffer<true>;

// 命名空间结束：离开当前模块作用域。
} // namespace app::common::memory::double_buffer

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "double_buffer_impl.hpp"
