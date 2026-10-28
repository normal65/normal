/**
 * [初学者导读] 文件：main/app/system/info/info.cc
 * 作用：读取 CPU、内存、PSRAM 和系统信息。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：采样结果反映调用时刻，不是持续实时监控。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "info.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_private/esp_clk.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_heap_caps.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_psram.h"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::sys::info {

// 定义或声明函数 MemoryInfo::MemoryInfo：参数在括号里，返回类型写在函数名前面。
MemoryInfo::MemoryInfo()
    // 定义或声明函数 sram_total_：参数在括号里，返回类型写在函数名前面。
    : sram_total_(heap_caps_get_total_size(MALLOC_CAP_INTERNAL))
    // 定义或声明函数 sram_free_：参数在括号里，返回类型写在函数名前面。
    , sram_free_(heap_caps_get_free_size(MALLOC_CAP_INTERNAL))
    // 定义或声明函数 psram_total_：参数在括号里，返回类型写在函数名前面。
    , psram_total_(esp_psram_is_initialized() ? esp_psram_get_size() : 0)
    // 定义或声明函数 psram_free_：参数在括号里，返回类型写在函数名前面。
    , psram_free_(esp_psram_is_initialized() ? heap_caps_get_free_size(MALLOC_CAP_SPIRAM) : 0)
{
}

// 定义或声明函数 MemoryInfo::get_memory_info：参数在括号里，返回类型写在函数名前面。
MemoryInfo MemoryInfo::get_memory_info()
{
    // 返回 MemoryInfo() 给调用者，表示当前函数的执行结果。
    return MemoryInfo();
}

// 定义或声明函数 CpuInfo::CpuInfo：参数在括号里，返回类型写在函数名前面。
CpuInfo::CpuInfo()
    // 定义或声明函数 cpu_frequency_：参数在括号里，返回类型写在函数名前面。
    : cpu_frequency_(static_cast<uint32_t>(esp_clk_cpu_freq()))
{
}

// 定义或声明函数 CpuInfo::get_cpu_info：参数在括号里，返回类型写在函数名前面。
CpuInfo CpuInfo::get_cpu_info()
{
    // 返回 CpuInfo() 给调用者，表示当前函数的执行结果。
    return CpuInfo();
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::sys::info
