// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/common/memory/psram_alloc.hpp
 * 作用：实现 memory 模块中的 psram alloc 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：PSRAM 能缓解内部内存压力，但不能无视 DMA 和中断约束。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstddef>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_heap_caps.h>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::common::memory {

/** 优先 PSRAM，失败回退内部堆 */
// 定义或声明函数 alloc_psram_prefer：参数在括号里，返回类型写在函数名前面。
inline void* alloc_psram_prefer(size_t bytes)
{
    // 赋值：把右侧 heap_caps_malloc(bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT) 的结果保存到左侧 void* p。
    void* p = heap_caps_malloc(bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    // 判断条件：p == nullptr 是否成立。
    if (p == nullptr)
        // 赋值：把右侧 heap_caps_malloc(bytes, MALLOC_CAP_8BIT) 的结果保存到左侧 p。
        p = heap_caps_malloc(bytes, MALLOC_CAP_8BIT);
    // 返回 p 给调用者，表示当前函数的执行结果。
    return p;
}

// 定义或声明函数 calloc_psram_prefer：参数在括号里，返回类型写在函数名前面。
inline void* calloc_psram_prefer(size_t count, size_t elem_size)
{
    // 赋值：把右侧 heap_caps_calloc(count, elem_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT) 的结果保存到左侧 void* p。
    void* p = heap_caps_calloc(count, elem_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    // 判断条件：p == nullptr 是否成立。
    if (p == nullptr)
        // 赋值：把右侧 heap_caps_calloc(count, elem_size, MALLOC_CAP_8BIT) 的结果保存到左侧 p。
        p = heap_caps_calloc(count, elem_size, MALLOC_CAP_8BIT);
    // 返回 p 给调用者，表示当前函数的执行结果。
    return p;
}

// 模板声明：让同一段代码适配不同类型。
template <typename T>
// 定义或声明函数 alloc_array_psram：参数在括号里，返回类型写在函数名前面。
inline T* alloc_array_psram(size_t count)
{
    // 返回 static_cast<T*>(calloc_psram_prefer(count, sizeof(T))) 给调用者，表示当前函数的执行结果。
    return static_cast<T*>(calloc_psram_prefer(count, sizeof(T)));
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::common::memory
