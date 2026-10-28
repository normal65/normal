// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/common/memory/memory.hpp
 * 作用：实现内存池、RAII 内存管理和内存信息统计。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：PSRAM 能缓解内部内存压力，但不能无视 DMA 和中断约束。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstddef>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <map>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <memory>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <unordered_map>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <vector>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_heap_caps.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "freertos/FreeRTOS.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "freertos/semphr.h"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::common::memory {

/** @brief heap_caps_free 删除器，供 unique_ptr 使用 */
// struct EspHeapDeleter：把相关数据字段组合在一起，表示项目中的一个概念。
struct EspHeapDeleter
{
    // 定义或声明函数 operator：参数在括号里，返回类型写在函数名前面。
    void operator()(uint8_t* ptr) const
    {
        // 判断条件：ptr != nullptr 是否成立。
        if (ptr != nullptr)
        {
            // 调用 heap_caps_free(ptr);，执行这个函数并处理它的返回值或状态变化。
            heap_caps_free(ptr);
        }
    }
};

/** @brief FreeRTOS 互斥锁 RAII */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class MutexRAII
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 create：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static MutexRAII create()
    {
        // 定义变量 SemaphoreHandle_t handle 并初始化为 xSemaphoreCreateMutex()。
        SemaphoreHandle_t handle = xSemaphoreCreateMutex();
        // 返回 MutexRAII(handle) 给调用者，表示当前函数的执行结果。
        return MutexRAII(handle);
    }

    // 调用 MutexRAII(MutexRAII&& other) noexcept，执行这个函数并处理它的返回值或状态变化。
    MutexRAII(MutexRAII&& other) noexcept
        // 定义或声明函数 handle_：参数在括号里，返回类型写在函数名前面。
        : handle_(other.handle_)
    {
        // 赋值：把右侧 nullptr 的结果保存到左侧 other.handle_。
        other.handle_ = nullptr;
    }

    // 组合条件：左右两个条件共同决定这一段代码是否继续执行。
    MutexRAII& operator=(MutexRAII&& other) noexcept
    {
        // 判断条件：this != &other 是否成立。
        if (this != &other)
        {
            // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
            reset();
            // 赋值：把右侧 other.handle_ 的结果保存到左侧 handle_。
            handle_ = other.handle_;
            // 赋值：把右侧 nullptr 的结果保存到左侧 other.handle_。
            other.handle_ = nullptr;
        }
        // 返回 *this 给调用者，表示当前函数的执行结果。
        return *this;
    }

    // 调用 MutexRAII(const MutexRAII&) = delete;，执行这个函数并处理它的返回值或状态变化。
    MutexRAII(const MutexRAII&) = delete;
    // 赋值：把右侧 (const MutexRAII&) = delete 的结果保存到左侧 MutexRAII& operator。
    MutexRAII& operator=(const MutexRAII&) = delete;

    // 定义或声明函数 ~MutexRAII：参数在括号里，返回类型写在函数名前面。
    ~MutexRAII()
    {
        // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
        reset();
    }

    // 函数定义/声明：调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
    SemaphoreHandle_t get() const
    {
        // 返回 handle_ 给调用者，表示当前函数的执行结果。
        return handle_;
    }

    // 定义或声明函数 is_valid：参数在括号里，返回类型写在函数名前面。
    bool is_valid() const
    {
        // 返回 handle_ != nullptr 给调用者，表示当前函数的执行结果。
        return handle_ != nullptr;
    }

    // 定义或声明函数 SemaphoreHandle_t：参数在括号里，返回类型写在函数名前面。
    operator SemaphoreHandle_t() const
    {
        // 返回 handle_ 给调用者，表示当前函数的执行结果。
        return handle_;
    }

    // 函数定义/声明：调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    void reset()
    {
        // 判断条件：handle_ != nullptr 是否成立。
        if (handle_ != nullptr)
        {
            // 调用 vSemaphoreDelete(handle_);，执行这个函数并处理它的返回值或状态变化。
            vSemaphoreDelete(handle_);
            // 赋值：把右侧 nullptr 的结果保存到左侧 handle_。
            handle_ = nullptr;
        }
    }

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 定义或声明函数 MutexRAII：参数在括号里，返回类型写在函数名前面。
    explicit MutexRAII(SemaphoreHandle_t handle)
        // 定义或声明函数 handle_：参数在括号里，返回类型写在函数名前面。
        : handle_(handle)
    {
    }

    // 执行语句：SemaphoreHandle_t handle_;；它会更新一次变量、对象或程序状态。
    SemaphoreHandle_t handle_;
};

/** @brief 互斥锁作用域守卫 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class MutexLockGuard
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 定义或声明函数 MutexLockGuard：参数在括号里，返回类型写在函数名前面。
    explicit MutexLockGuard(SemaphoreHandle_t mutex, TickType_t timeout = portMAX_DELAY)
        // 定义或声明函数 mutex_：参数在括号里，返回类型写在函数名前面。
        : mutex_(mutex)
        // 定义或声明函数 locked_：参数在括号里，返回类型写在函数名前面。
        , locked_(false)
    {
        // 判断条件：mutex_ != nullptr 是否成立。
        if (mutex_ != nullptr)
        {
            // 赋值：把右侧 (xSemaphoreTake(mutex_, timeout) == pdTRUE) 的结果保存到左侧 locked_。
            locked_ = (xSemaphoreTake(mutex_, timeout) == pdTRUE);
        }
    }

    // 定义或声明函数 ~MutexLockGuard：参数在括号里，返回类型写在函数名前面。
    ~MutexLockGuard()
    {
        // 调用 release();，执行这个函数并处理它的返回值或状态变化。
        release();
    }

    // 调用 MutexLockGuard(const MutexLockGuard&) = delete;，执行这个函数并处理它的返回值或状态变化。
    MutexLockGuard(const MutexLockGuard&) = delete;
    // 赋值：把右侧 (const MutexLockGuard&) = delete 的结果保存到左侧 MutexLockGuard& operator。
    MutexLockGuard& operator=(const MutexLockGuard&) = delete;

    // 定义或声明函数 is_locked：参数在括号里，返回类型写在函数名前面。
    bool is_locked() const
    {
        // 返回 locked_ 给调用者，表示当前函数的执行结果。
        return locked_;
    }

    // 定义或声明函数 release：参数在括号里，返回类型写在函数名前面。
    void release()
    {
        // 判断条件：locked_ && mutex_ != nullptr 是否成立。
        if (locked_ && mutex_ != nullptr)
        {
            // 调用 xSemaphoreGive(mutex_);，执行这个函数并处理它的返回值或状态变化。
            xSemaphoreGive(mutex_);
            // 赋值：把右侧 false 的结果保存到左侧 locked_。
            locked_ = false;
        }
    }

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 执行语句：SemaphoreHandle_t mutex_;；它会更新一次变量、对象或程序状态。
    SemaphoreHandle_t mutex_;
    // 执行语句：bool locked_;；它会更新一次变量、对象或程序状态。
    bool locked_;
};

// 固定参数 BYTES_PER_KILOBYTE = 1024：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr size_t BYTES_PER_KILOBYTE = 1024;
// 固定参数 BYTES_PER_MEGABYTE = BYTES_PER_KILOBYTE * 1024：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr size_t BYTES_PER_MEGABYTE = BYTES_PER_KILOBYTE * 1024;
// 固定参数 DEFAULT_INITIAL_POOL_SIZE = 64 * BYTES_PER_KILOBYTE：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr size_t DEFAULT_INITIAL_POOL_SIZE = 64 * BYTES_PER_KILOBYTE;
// 固定参数 DEFAULT_EXPANSION_FACTOR = 2.0：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr double DEFAULT_EXPANSION_FACTOR = 2.0;

/** @brief 内部 SRAM 优先的堆内存池 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class MemoryPool
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // struct Stats：把相关数据字段组合在一起，表示项目中的一个概念。
    struct Stats
    {
        // 执行语句：size_t total_memory;；它会更新一次变量、对象或程序状态。
        size_t total_memory;
        // 执行语句：size_t used_memory;；它会更新一次变量、对象或程序状态。
        size_t used_memory;
        // 执行语句：size_t free_memory;；它会更新一次变量、对象或程序状态。
        size_t free_memory;
        // 执行语句：size_t allocated_blocks;；它会更新一次变量、对象或程序状态。
        size_t allocated_blocks;
        // 执行语句：size_t free_blocks;；它会更新一次变量、对象或程序状态。
        size_t free_blocks;
    };

    // 函数调用：explicit MemoryPool(size_t initial_size = DEFAULT_INITIAL_POOL_SIZE,；执行该函数并处理它的返回值或副作用。
    explicit MemoryPool(size_t initial_size = DEFAULT_INITIAL_POOL_SIZE,
                        // 继续上一行的表达式或参数列表：size_t alignment = alignof(std::max_align_t),。
                        size_t alignment = alignof(std::max_align_t),
                        // 继续上一行的表达式或参数列表：double expansion_factor = DEFAULT_EXPANSION_FACTOR);。
                        double expansion_factor = DEFAULT_EXPANSION_FACTOR);

    // 函数调用：~MemoryPool();；执行该函数并处理它的返回值或副作用。
    ~MemoryPool();

    // 调用 MemoryPool(const MemoryPool&) = delete;，执行这个函数并处理它的返回值或状态变化。
    MemoryPool(const MemoryPool&) = delete;
    // 赋值：把右侧 (const MemoryPool&) = delete 的结果保存到左侧 MemoryPool& operator。
    MemoryPool& operator=(const MemoryPool&) = delete;

    // 函数调用：void* allocate(size_t size);；执行该函数并处理它的返回值或副作用。
    void* allocate(size_t size);
    // 函数调用：void deallocate(void* ptr);；执行该函数并处理它的返回值或副作用。
    void deallocate(void* ptr);
    // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    void reset();
    // 函数调用：Stats get_stats() const;；执行该函数并处理它的返回值或副作用。
    Stats get_stats() const;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // struct BlockHeader：把相关数据字段组合在一起，表示项目中的一个概念。
    struct BlockHeader
    {
        // 执行语句：size_t size;；它会更新一次变量、对象或程序状态。
        size_t size;
        // 执行语句：bool is_free;；它会更新一次变量、对象或程序状态。
        bool is_free;
        // 执行语句：BlockHeader* next;；它会更新一次变量、对象或程序状态。
        BlockHeader* next;
        // 执行语句：BlockHeader* prev;；它会更新一次变量、对象或程序状态。
        BlockHeader* prev;
    };

    // struct PoolBlock：把相关数据字段组合在一起，表示项目中的一个概念。
    struct PoolBlock
    {
        // 执行语句：std::unique_ptr<uint8_t[], EspHeapDeleter> memory;；它会更新一次变量、对象或程序状态。
        std::unique_ptr<uint8_t[], EspHeapDeleter> memory;
        // 执行语句：size_t size;；它会更新一次变量、对象或程序状态。
        size_t size;
        // 执行语句：BlockHeader* first_block;；它会更新一次变量、对象或程序状态。
        BlockHeader* first_block;
    };

    // 执行语句：MutexRAII mutex_;；它会更新一次变量、对象或程序状态。
    MutexRAII mutex_;
    // 执行语句：size_t alignment_;；它会更新一次变量、对象或程序状态。
    size_t alignment_;
    // 执行语句：double expansion_factor_;；它会更新一次变量、对象或程序状态。
    double expansion_factor_;
    // 执行语句：std::vector<PoolBlock> pool_blocks_;；它会更新一次变量、对象或程序状态。
    std::vector<PoolBlock> pool_blocks_;
    // 执行语句：std::unordered_map<void*, size_t> pointer_map_;；它会更新一次变量、对象或程序状态。
    std::unordered_map<void*, size_t> pointer_map_;
    // 执行语句：mutable std::multimap<size_t, void*> free_blocks_by_size_;；它会更新一次变量、对象或程序状态。
    mutable std::multimap<size_t, void*> free_blocks_by_size_;
    // 执行这条代码：mutable std::unordered_map<void*, std::multimap<size_t, void*>::iterator>。它会更新变量、对象状态或调用链。
    mutable std::unordered_map<void*, std::multimap<size_t, void*>::iterator>
        // 执行语句：free_blocks_iterators_;；它会更新一次变量、对象或程序状态。
        free_blocks_iterators_;

    // 函数调用：void init_pool(size_t size);；执行该函数并处理它的返回值或副作用。
    void init_pool(size_t size);
    // 函数调用：void expand_pool(size_t required_size);；执行该函数并处理它的返回值或副作用。
    void expand_pool(size_t required_size);
    // 函数调用：BlockHeader* find_free_block(size_t size);；执行该函数并处理它的返回值或副作用。
    BlockHeader* find_free_block(size_t size);
    // 函数调用：void split_block(BlockHeader* block, size_t size);；执行该函数并处理它的返回值或副作用。
    void split_block(BlockHeader* block, size_t size);
    // 函数调用：void coalesce_blocks(BlockHeader* block);；执行该函数并处理它的返回值或副作用。
    void coalesce_blocks(BlockHeader* block);
    // 函数调用：size_t aligned_size(size_t size) const;；执行该函数并处理它的返回值或副作用。
    size_t aligned_size(size_t size) const;
    // 函数调用：size_t get_header_size() const;；执行该函数并处理它的返回值或副作用。
    size_t get_header_size() const;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::common::memory
