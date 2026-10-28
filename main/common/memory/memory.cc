/**
 * [初学者导读] 文件：main/common/memory/memory.cc
 * 作用：实现内存池、RAII 内存管理和内存信息统计。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：PSRAM 能缓解内部内存压力，但不能无视 DMA 和中断约束。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "memory.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <algorithm>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "freertos/FreeRTOS.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "freertos/semphr.h"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::common::memory {

// 定义或声明函数 MemoryPool::MemoryPool：参数在括号里，返回类型写在函数名前面。
MemoryPool::MemoryPool(size_t initial_size, size_t alignment, double expansion_factor)
    // 定义或声明函数 mutex_：参数在括号里，返回类型写在函数名前面。
    : mutex_(MutexRAII::create())
    // 定义或声明函数 alignment_：参数在括号里，返回类型写在函数名前面。
    , alignment_(alignment)
    // 定义或声明函数 expansion_factor_：参数在括号里，返回类型写在函数名前面。
    , expansion_factor_(expansion_factor)
{
    // 判断条件：!mutex_.is_valid() 是否成立。
    if (!mutex_.is_valid())
    {
        // 赋值：把右侧 alignof(std::max_align_t) 的结果保存到左侧 alignment_。
        alignment_ = alignof(std::max_align_t);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：alignment_ == 0 || (alignment_ & (alignment_ - 1)) != 0 是否成立。
    if (alignment_ == 0 || (alignment_ & (alignment_ - 1)) != 0)
    {
        // 赋值：把右侧 alignof(std::max_align_t) 的结果保存到左侧 alignment_。
        alignment_ = alignof(std::max_align_t);
    }

    // 判断条件：expansion_factor_ < 1.0 是否成立。
    if (expansion_factor_ < 1.0)
    {
        // 赋值：把右侧 1.0 的结果保存到左侧 expansion_factor_。
        expansion_factor_ = 1.0;
    }

    // 调用 init_pool(initial_size);，执行这个函数并处理它的返回值或状态变化。
    init_pool(initial_size);
}

// 定义或声明函数 MemoryPool::~MemoryPool：参数在括号里，返回类型写在函数名前面。
MemoryPool::~MemoryPool()
{
    // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    reset();
}

// 定义或声明函数 MemoryPool::init_pool：参数在括号里，返回类型写在函数名前面。
void MemoryPool::init_pool(size_t size)
{
    // 定义变量 size_t aligned_size_value 并初始化为 aligned_size(size)。
    size_t aligned_size_value = aligned_size(size);
    // 定义变量 size_t header_size 并初始化为 get_header_size()。
    size_t header_size = get_header_size();
    // 定义变量 size_t total_size 并初始化为 aligned_size_value + alignment_。
    size_t total_size = aligned_size_value + alignment_;

    // 执行这条代码：uint8_t* memory =。它会更新变量、对象状态或调用链。
    uint8_t* memory =
        // 继续上一行的表达式或参数列表：static_cast<uint8_t*>(heap_caps_malloc(total_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));。
        static_cast<uint8_t*>(heap_caps_malloc(total_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
    // 判断条件：memory == nullptr 是否成立。
    if (memory == nullptr)
    {
        // 赋值：把右侧 static_cast<uint8_t*>(heap_caps_malloc(total_size, MALLOC_CAP_DEFAULT)) 的结果保存到左侧 memory。
        memory = static_cast<uint8_t*>(heap_caps_malloc(total_size, MALLOC_CAP_DEFAULT));
    }

    // 判断条件：memory == nullptr 是否成立。
    if (memory == nullptr)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 定义变量 uintptr_t memory_addr 并初始化为 reinterpret_cast<uintptr_t>(memory)。
    uintptr_t memory_addr = reinterpret_cast<uintptr_t>(memory);
    // 定义变量 size_t offset 并初始化为 (alignment_ - (memory_addr % alignment_)) % alignment_。
    size_t offset = (alignment_ - (memory_addr % alignment_)) % alignment_;
    // 赋值：把右侧 memory + offset 的结果保存到左侧 uint8_t* aligned_memory。
    uint8_t* aligned_memory = memory + offset;

    // 执行语句：PoolBlock pool_block;；它会更新一次变量、对象或程序状态。
    PoolBlock pool_block;
    // 赋值：把右侧 std::unique_ptr<uint8_t[], EspHeapDeleter>(memory) 的结果保存到左侧 pool_block.memory。
    pool_block.memory = std::unique_ptr<uint8_t[], EspHeapDeleter>(memory);
    // 赋值：把右侧 aligned_size_value 的结果保存到左侧 pool_block.size。
    pool_block.size = aligned_size_value;

    // 赋值：把右侧 reinterpret_cast<BlockHeader*>(aligned_memory) 的结果保存到左侧 auto* first_block。
    auto* first_block = reinterpret_cast<BlockHeader*>(aligned_memory);
    // 赋值：把右侧 aligned_size_value - header_size 的结果保存到左侧 first_block->size。
    first_block->size = aligned_size_value - header_size;
    // 赋值：把右侧 true 的结果保存到左侧 first_block->is_free。
    first_block->is_free = true;
    // 赋值：把右侧 nullptr 的结果保存到左侧 first_block->next。
    first_block->next = nullptr;
    // 赋值：把右侧 nullptr 的结果保存到左侧 first_block->prev。
    first_block->prev = nullptr;

    // 赋值：把右侧 first_block 的结果保存到左侧 pool_block.first_block。
    pool_block.first_block = first_block;
    // 访问对象成员：pool_blocks_.push_back(std::move(pool_block));；读取或修改这个对象里的成员状态。
    pool_blocks_.push_back(std::move(pool_block));

    // 赋值：把右侧 reinterpret_cast<uint8_t*>(first_block) + header_size 的结果保存到左侧 void* first_block_ptr。
    void* first_block_ptr = reinterpret_cast<uint8_t*>(first_block) + header_size;
    // 定义变量 auto it 并初始化为 free_blocks_by_size_.insert({first_block->size, first_block_ptr})。
    auto it = free_blocks_by_size_.insert({first_block->size, first_block_ptr});
    // 赋值：把右侧 it 的结果保存到左侧 free_blocks_iterators_[first_block_ptr]。
    free_blocks_iterators_[first_block_ptr] = it;
}

// 定义或声明函数 MemoryPool::allocate：参数在括号里，返回类型写在函数名前面。
void* MemoryPool::allocate(size_t size)
{
    // 判断条件：size == 0 || !mutex_.is_valid() 是否成立。
    if (size == 0 || !mutex_.is_valid())
    {
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;
    }

    // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
    MutexLockGuard lock(mutex_.get());
    // 判断条件：!lock.is_locked() 是否成立。
    if (!lock.is_locked())
    {
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;
    }

    // 定义变量 size_t aligned_request_size 并初始化为 aligned_size(size)。
    size_t aligned_request_size = aligned_size(size);
    // 定义变量 size_t pool_index 并初始化为 0。
    size_t pool_index = 0;
    // 赋值：把右侧 nullptr 的结果保存到左侧 BlockHeader* block。
    BlockHeader* block = nullptr;
    // 赋值：把右侧 nullptr 的结果保存到左侧 void* found_ptr。
    void* found_ptr = nullptr;

    // 定义变量 auto it 并初始化为 free_blocks_by_size_.lower_bound(aligned_request_size)。
    auto it = free_blocks_by_size_.lower_bound(aligned_request_size);
    // 判断条件：it != free_blocks_by_size_.end() 是否成立。
    if (it != free_blocks_by_size_.end())
    {
        // 赋值：把右侧 it->second 的结果保存到左侧 found_ptr。
        found_ptr = it->second;
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        block = reinterpret_cast<BlockHeader*>(reinterpret_cast<uint8_t*>(found_ptr) -
                                               // 调用 get_header_size());，执行这个函数并处理它的返回值或状态变化。
                                               get_header_size());
        // 定义变量并调用函数初始化：调用 find(found_ptr)：find(id) 根据 AppId 找对象；entryat(idx) 根据网格索引返回 {id, title, iconcp} 这样的轻量条目。，结果保存到 auto map_it。
        auto map_it = pointer_map_.find(found_ptr);
        // 判断条件：map_it != pointer_map_.end() 是否成立。
        if (map_it != pointer_map_.end())
        {
            // 赋值：把右侧 map_it->second 的结果保存到左侧 pool_index。
            pool_index = map_it->second;
        }
        // 访问对象成员：free_blocks_by_size_.erase(it);；读取或修改这个对象里的成员状态。
        free_blocks_by_size_.erase(it);
        // 访问对象成员：free_blocks_iterators_.erase(found_ptr);；读取或修改这个对象里的成员状态。
        free_blocks_iterators_.erase(found_ptr);
    }

    // 判断条件：!block 是否成立。
    if (!block)
    {
        // 调用 expand_pool(aligned_request_size);，执行这个函数并处理它的返回值或状态变化。
        expand_pool(aligned_request_size);
        // 赋值：把右侧 pool_blocks_.size() - 1 的结果保存到左侧 pool_index。
        pool_index = pool_blocks_.size() - 1;
        // 赋值：把右侧 pool_blocks_[pool_index].first_block 的结果保存到左侧 block。
        block = pool_blocks_[pool_index].first_block;
        // 判断条件：!block || !block->is_free || block->size < aligned_request_size 是否成立。
        if (!block || !block->is_free || block->size < aligned_request_size)
        {
            // 返回 nullptr 给调用者，表示当前函数的执行结果。
            return nullptr;
        }

        // 赋值：把右侧 reinterpret_cast<uint8_t*>(block) + get_header_size() 的结果保存到左侧 void* block_ptr。
        void* block_ptr = reinterpret_cast<uint8_t*>(block) + get_header_size();
        // 定义变量并调用函数初始化：调用 find(block_ptr)：find(id) 根据 AppId 找对象；entryat(idx) 根据网格索引返回 {id, title, iconcp} 这样的轻量条目。，结果保存到 auto iter_it。
        auto iter_it = free_blocks_iterators_.find(block_ptr);
        // 判断条件：iter_it != free_blocks_iterators_.end() 是否成立。
        if (iter_it != free_blocks_iterators_.end())
        {
            // 访问对象成员：free_blocks_by_size_.erase(iter_it->second);；读取或修改这个对象里的成员状态。
            free_blocks_by_size_.erase(iter_it->second);
            // 访问对象成员：free_blocks_iterators_.erase(iter_it);；读取或修改这个对象里的成员状态。
            free_blocks_iterators_.erase(iter_it);
        }
    }

    // 调用 split_block(block, aligned_request_size);，执行这个函数并处理它的返回值或状态变化。
    split_block(block, aligned_request_size);
    // 赋值：把右侧 false 的结果保存到左侧 block->is_free。
    block->is_free = false;

    // 赋值：把右侧 reinterpret_cast<uint8_t*>(block) + get_header_size() 的结果保存到左侧 auto* user_data。
    auto* user_data = reinterpret_cast<uint8_t*>(block) + get_header_size();
    // 赋值：把右侧 pool_index 的结果保存到左侧 pointer_map_[user_data]。
    pointer_map_[user_data] = pool_index;

    // 返回 user_data 给调用者，表示当前函数的执行结果。
    return user_data;
}

// 定义或声明函数 MemoryPool::deallocate：参数在括号里，返回类型写在函数名前面。
void MemoryPool::deallocate(void* ptr)
{
    // 判断条件：!ptr || !mutex_.is_valid() 是否成立。
    if (!ptr || !mutex_.is_valid())
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
    MutexLockGuard lock(mutex_.get());
    // 判断条件：!lock.is_locked() 是否成立。
    if (!lock.is_locked())
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 定义变量并调用函数初始化：调用 find(ptr)：find(id) 根据 AppId 找对象；entryat(idx) 根据网格索引返回 {id, title, iconcp} 这样的轻量条目。，结果保存到 auto pointer_iter。
    auto pointer_iter = pointer_map_.find(ptr);
    // 判断条件：pointer_iter == pointer_map_.end() 是否成立。
    if (pointer_iter == pointer_map_.end())
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 定义变量 size_t pool_index 并初始化为 pointer_iter->second。
    size_t pool_index = pointer_iter->second;
    // 判断条件：pool_index >= pool_blocks_.size() 是否成立。
    if (pool_index >= pool_blocks_.size())
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 执行这条代码：auto* block =。它会更新变量、对象状态或调用链。
    auto* block =
        // 继续上一行的表达式或参数列表：reinterpret_cast<BlockHeader*>(reinterpret_cast<uint8_t*>(ptr) - get_header_size());。
        reinterpret_cast<BlockHeader*>(reinterpret_cast<uint8_t*>(ptr) - get_header_size());

    // 赋值：把右侧 true 的结果保存到左侧 block->is_free。
    block->is_free = true;
    // 赋值：把右侧 reinterpret_cast<uint8_t*>(block) + get_header_size() 的结果保存到左侧 void* block_ptr。
    void* block_ptr = reinterpret_cast<uint8_t*>(block) + get_header_size();
    // 定义变量 auto it 并初始化为 free_blocks_by_size_.insert({block->size, block_ptr})。
    auto it = free_blocks_by_size_.insert({block->size, block_ptr});
    // 赋值：把右侧 it 的结果保存到左侧 free_blocks_iterators_[block_ptr]。
    free_blocks_iterators_[block_ptr] = it;

    // 调用 coalesce_blocks(block);，执行这个函数并处理它的返回值或状态变化。
    coalesce_blocks(block);
    // 访问对象成员：pointer_map_.erase(pointer_iter);；读取或修改这个对象里的成员状态。
    pointer_map_.erase(pointer_iter);
}

// 函数定义/声明：调用 MemoryPool::reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
void MemoryPool::reset()
{
    // 判断条件：!mutex_.is_valid() 是否成立。
    if (!mutex_.is_valid())
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
    MutexLockGuard lock(mutex_.get());
    // 判断条件：!lock.is_locked() 是否成立。
    if (!lock.is_locked())
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    pool_blocks_.clear();
    // 调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    pointer_map_.clear();
    // 调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    free_blocks_by_size_.clear();
    // 调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    free_blocks_iterators_.clear();
}

// 定义或声明函数 MemoryPool::get_stats：参数在括号里，返回类型写在函数名前面。
MemoryPool::Stats MemoryPool::get_stats() const
{
    // 执行语句：Stats stats{0, 0, 0, 0, 0};；它会更新一次变量、对象或程序状态。
    Stats stats{0, 0, 0, 0, 0};

    // 判断条件：!mutex_.is_valid() 是否成立。
    if (!mutex_.is_valid())
    {
        // 返回 stats 给调用者，表示当前函数的执行结果。
        return stats;
    }

    // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
    MutexLockGuard lock(mutex_.get());
    // 判断条件：!lock.is_locked() 是否成立。
    if (!lock.is_locked())
    {
        // 返回 stats 给调用者，表示当前函数的执行结果。
        return stats;
    }

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (const auto& pool_block : pool_blocks_)
    {
        // 赋值：把右侧 pool_block.size 的结果保存到左侧 stats.total_memory +。
        stats.total_memory += pool_block.size;

        // 赋值：把右侧 pool_block.first_block 的结果保存到左侧 BlockHeader* current。
        BlockHeader* current = pool_block.first_block;
        // 循环执行：条件为真时重复运行，条件变化后退出。
        while (current)
        {
            // 判断条件：current->is_free 是否成立。
            if (current->is_free)
            {
                // 赋值：把右侧 current->size 的结果保存到左侧 stats.free_memory +。
                stats.free_memory += current->size;
                // 访问对象成员：stats.free_blocks++;；读取或修改这个对象里的成员状态。
                stats.free_blocks++;
            }
            // 备用分支：前面的条件不成立时执行这里。
            else
            {
                // 赋值：把右侧 current->size 的结果保存到左侧 stats.used_memory +。
                stats.used_memory += current->size;
                // 访问对象成员：stats.allocated_blocks++;；读取或修改这个对象里的成员状态。
                stats.allocated_blocks++;
            }
            // 赋值：把右侧 current->next 的结果保存到左侧 current。
            current = current->next;
        }
    }

    // 返回 stats 给调用者，表示当前函数的执行结果。
    return stats;
}

// 定义或声明函数 MemoryPool::expand_pool：参数在括号里，返回类型写在函数名前面。
void MemoryPool::expand_pool(size_t required_size)
{
    // 定义变量 auto new_size 并初始化为 static_cast<size_t>。
    auto new_size = static_cast<size_t>(
        // 继续上一行的表达式或参数列表：pool_blocks_.empty() ? required_size : pool_blocks_.back().size * expansion_factor_);。
        pool_blocks_.empty() ? required_size : pool_blocks_.back().size * expansion_factor_);

    // 赋值：把右侧 std::max(new_size, required_size + get_header_size()) 的结果保存到左侧 new_size。
    new_size = std::max(new_size, required_size + get_header_size());
    // 调用 init_pool(new_size);，执行这个函数并处理它的返回值或状态变化。
    init_pool(new_size);
}

// 定义或声明函数 MemoryPool::find_free_block：参数在括号里，返回类型写在函数名前面。
MemoryPool::BlockHeader* MemoryPool::find_free_block(size_t size)
{
    // 定义变量 auto it 并初始化为 free_blocks_by_size_.lower_bound(size)。
    auto it = free_blocks_by_size_.lower_bound(size);
    // 判断条件：it != free_blocks_by_size_.end() 是否成立。
    if (it != free_blocks_by_size_.end())
    {
        // 赋值：把右侧 it->second 的结果保存到左侧 void* ptr。
        void* ptr = it->second;
        // 执行这条代码：auto* block =。它会更新变量、对象状态或调用链。
        auto* block =
            // 继续上一行的表达式或参数列表：reinterpret_cast<BlockHeader*>(reinterpret_cast<uint8_t*>(ptr) - get_header_size());。
            reinterpret_cast<BlockHeader*>(reinterpret_cast<uint8_t*>(ptr) - get_header_size());
        // 返回 block 给调用者，表示当前函数的执行结果。
        return block;
    }

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (auto& pool_block : pool_blocks_)
    {
        // 赋值：把右侧 pool_block.first_block 的结果保存到左侧 BlockHeader* current。
        BlockHeader* current = pool_block.first_block;
        // 循环执行：条件为真时重复运行，条件变化后退出。
        while (current)
        {
            // 判断条件：current->is_free && current->size >= size 是否成立。
            if (current->is_free && current->size >= size)
            {
                // 返回 current 给调用者，表示当前函数的执行结果。
                return current;
            }
            // 赋值：把右侧 current->next 的结果保存到左侧 current。
            current = current->next;
        }
    }

    // 返回 nullptr 给调用者，表示当前函数的执行结果。
    return nullptr;
}

// 定义或声明函数 MemoryPool::split_block：参数在括号里，返回类型写在函数名前面。
void MemoryPool::split_block(BlockHeader* block, size_t size)
{
    // 定义变量 size_t min_block_size 并初始化为 get_header_size() + alignment_。
    size_t min_block_size = get_header_size() + alignment_;
    // 判断条件：block->size >= size + min_block_size 是否成立。
    if (block->size >= size + min_block_size)
    {
        // 赋值：把右侧 reinterpret_cast<uint8_t*>(block) + get_header_size() + size 的结果保存到左侧 auto* new_block_address。
        auto* new_block_address = reinterpret_cast<uint8_t*>(block) + get_header_size() + size;
        // 赋值：把右侧 reinterpret_cast<BlockHeader*>(new_block_address) 的结果保存到左侧 auto* new_block。
        auto* new_block = reinterpret_cast<BlockHeader*>(new_block_address);

        // 赋值：把右侧 block->size - size - get_header_size() 的结果保存到左侧 new_block->size。
        new_block->size = block->size - size - get_header_size();
        // 赋值：把右侧 true 的结果保存到左侧 new_block->is_free。
        new_block->is_free = true;
        // 赋值：把右侧 block->next 的结果保存到左侧 new_block->next。
        new_block->next = block->next;
        // 赋值：把右侧 block 的结果保存到左侧 new_block->prev。
        new_block->prev = block;

        // 赋值：把右侧 size 的结果保存到左侧 block->size。
        block->size = size;
        // 赋值：把右侧 new_block 的结果保存到左侧 block->next。
        block->next = new_block;

        // 判断条件：new_block->next 是否成立。
        if (new_block->next)
        {
            // 赋值：把右侧 new_block 的结果保存到左侧 new_block->next->prev。
            new_block->next->prev = new_block;
        }

        // 赋值：把右侧 reinterpret_cast<uint8_t*>(new_block) + get_header_size() 的结果保存到左侧 void* new_block_ptr。
        void* new_block_ptr = reinterpret_cast<uint8_t*>(new_block) + get_header_size();
        // 定义变量 auto it 并初始化为 free_blocks_by_size_.insert({new_block->size, new_block_ptr})。
        auto it = free_blocks_by_size_.insert({new_block->size, new_block_ptr});
        // 赋值：把右侧 it 的结果保存到左侧 free_blocks_iterators_[new_block_ptr]。
        free_blocks_iterators_[new_block_ptr] = it;
    }
}

// 定义或声明函数 MemoryPool::coalesce_blocks：参数在括号里，返回类型写在函数名前面。
void MemoryPool::coalesce_blocks(BlockHeader* block)
{
    // 赋值：把右侧 block 的结果保存到左侧 BlockHeader* final_block。
    BlockHeader* final_block = block;
    // 赋值：把右侧 reinterpret_cast<uint8_t*>(block) + get_header_size() 的结果保存到左侧 void* block_ptr。
    void* block_ptr = reinterpret_cast<uint8_t*>(block) + get_header_size();

    // 定义变量并调用函数初始化：调用 find(block_ptr)：find(id) 根据 AppId 找对象；entryat(idx) 根据网格索引返回 {id, title, iconcp} 这样的轻量条目。，结果保存到 auto iter_it。
    auto iter_it = free_blocks_iterators_.find(block_ptr);
    // 判断条件：iter_it != free_blocks_iterators_.end() 是否成立。
    if (iter_it != free_blocks_iterators_.end())
    {
        // 访问对象成员：free_blocks_by_size_.erase(iter_it->second);；读取或修改这个对象里的成员状态。
        free_blocks_by_size_.erase(iter_it->second);
        // 访问对象成员：free_blocks_iterators_.erase(iter_it);；读取或修改这个对象里的成员状态。
        free_blocks_iterators_.erase(iter_it);
    }

    // 判断条件：block->next && block->next->is_free 是否成立。
    if (block->next && block->next->is_free)
    {
        // 赋值：把右侧 block->next 的结果保存到左侧 BlockHeader* next_block。
        BlockHeader* next_block = block->next;
        // 赋值：把右侧 reinterpret_cast<uint8_t*>(next_block) + get_header_size() 的结果保存到左侧 void* next_block_ptr。
        void* next_block_ptr = reinterpret_cast<uint8_t*>(next_block) + get_header_size();

        // 定义变量并调用函数初始化：调用 find(next_block_ptr)：find(id) 根据 AppId 找对象；entryat(idx) 根据网格索引返回 {id, title, iconcp} 这样的轻量条目。，结果保存到 auto next_iter_it。
        auto next_iter_it = free_blocks_iterators_.find(next_block_ptr);
        // 判断条件：next_iter_it != free_blocks_iterators_.end() 是否成立。
        if (next_iter_it != free_blocks_iterators_.end())
        {
            // 访问对象成员：free_blocks_by_size_.erase(next_iter_it->second);；读取或修改这个对象里的成员状态。
            free_blocks_by_size_.erase(next_iter_it->second);
            // 访问对象成员：free_blocks_iterators_.erase(next_iter_it);；读取或修改这个对象里的成员状态。
            free_blocks_iterators_.erase(next_iter_it);
        }

        // 赋值：把右侧 next_block->size + get_header_size() 的结果保存到左侧 block->size +。
        block->size += next_block->size + get_header_size();
        // 赋值：把右侧 next_block->next 的结果保存到左侧 block->next。
        block->next = next_block->next;

        // 判断条件：next_block->next 是否成立。
        if (next_block->next)
        {
            // 赋值：把右侧 block 的结果保存到左侧 next_block->next->prev。
            next_block->next->prev = block;
        }
    }

    // 判断条件：block->prev && block->prev->is_free 是否成立。
    if (block->prev && block->prev->is_free)
    {
        // 赋值：把右侧 block->prev 的结果保存到左侧 BlockHeader* previous_block。
        BlockHeader* previous_block = block->prev;
        // 赋值：把右侧 reinterpret_cast<uint8_t*>(previous_block) + get_header_size() 的结果保存到左侧 void* prev_block_ptr。
        void* prev_block_ptr = reinterpret_cast<uint8_t*>(previous_block) + get_header_size();

        // 定义变量并调用函数初始化：调用 find(prev_block_ptr)：find(id) 根据 AppId 找对象；entryat(idx) 根据网格索引返回 {id, title, iconcp} 这样的轻量条目。，结果保存到 auto prev_iter_it。
        auto prev_iter_it = free_blocks_iterators_.find(prev_block_ptr);
        // 判断条件：prev_iter_it != free_blocks_iterators_.end() 是否成立。
        if (prev_iter_it != free_blocks_iterators_.end())
        {
            // 访问对象成员：free_blocks_by_size_.erase(prev_iter_it->second);；读取或修改这个对象里的成员状态。
            free_blocks_by_size_.erase(prev_iter_it->second);
            // 访问对象成员：free_blocks_iterators_.erase(prev_iter_it);；读取或修改这个对象里的成员状态。
            free_blocks_iterators_.erase(prev_iter_it);
        }

        // 赋值：把右侧 block->size + get_header_size() 的结果保存到左侧 previous_block->size +。
        previous_block->size += block->size + get_header_size();
        // 赋值：把右侧 block->next 的结果保存到左侧 previous_block->next。
        previous_block->next = block->next;

        // 判断条件：block->next 是否成立。
        if (block->next)
        {
            // 赋值：把右侧 previous_block 的结果保存到左侧 block->next->prev。
            block->next->prev = previous_block;
        }

        // 赋值：把右侧 previous_block 的结果保存到左侧 final_block。
        final_block = previous_block;
    }

    // 赋值：把右侧 reinterpret_cast<uint8_t*>(final_block) + get_header_size() 的结果保存到左侧 void* final_block_ptr。
    void* final_block_ptr = reinterpret_cast<uint8_t*>(final_block) + get_header_size();
    // 定义变量 auto it 并初始化为 free_blocks_by_size_.insert({final_block->size, final_block_ptr})。
    auto it = free_blocks_by_size_.insert({final_block->size, final_block_ptr});
    // 赋值：把右侧 it 的结果保存到左侧 free_blocks_iterators_[final_block_ptr]。
    free_blocks_iterators_[final_block_ptr] = it;
}

// 定义或声明函数 MemoryPool::aligned_size：参数在括号里，返回类型写在函数名前面。
size_t MemoryPool::aligned_size(size_t size) const
{
    // 返回 (size + alignment_ - 1) & ~(alignment_ - 1) 给调用者，表示当前函数的执行结果。
    return (size + alignment_ - 1) & ~(alignment_ - 1);
}

// 定义或声明函数 MemoryPool::get_header_size：参数在括号里，返回类型写在函数名前面。
size_t MemoryPool::get_header_size() const
{
    // 返回 aligned_size(sizeof(BlockHeader)) 给调用者，表示当前函数的执行结果。
    return aligned_size(sizeof(BlockHeader));
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::common::memory
