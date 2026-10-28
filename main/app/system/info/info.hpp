// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/system/info/info.hpp
 * 作用：读取 CPU、内存、PSRAM 和系统信息。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：采样结果反映调用时刻，不是持续实时监控。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstddef>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::sys::info {

/** SRAM / PSRAM 容量与空闲 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class MemoryInfo
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_memory_info：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static MemoryInfo get_memory_info();

    // 定义或声明函数 get_sram_total：参数在括号里，返回类型写在函数名前面。
    size_t get_sram_total() const
    {
        // 返回 sram_total_ 给调用者，表示当前函数的执行结果。
        return sram_total_;
    }
    // 定义或声明函数 get_sram_free：参数在括号里，返回类型写在函数名前面。
    size_t get_sram_free() const
    {
        // 返回 sram_free_ 给调用者，表示当前函数的执行结果。
        return sram_free_;
    }
    // 定义或声明函数 get_psram_total：参数在括号里，返回类型写在函数名前面。
    size_t get_psram_total() const
    {
        // 返回 psram_total_ 给调用者，表示当前函数的执行结果。
        return psram_total_;
    }
    // 定义或声明函数 get_psram_free：参数在括号里，返回类型写在函数名前面。
    size_t get_psram_free() const
    {
        // 返回 psram_free_ 给调用者，表示当前函数的执行结果。
        return psram_free_;
    }

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 MemoryInfo();，执行这个函数并处理它的返回值或状态变化。
    MemoryInfo();

    // 执行语句：size_t sram_total_;；它会更新一次变量、对象或程序状态。
    size_t sram_total_;
    // 执行语句：size_t sram_free_;；它会更新一次变量、对象或程序状态。
    size_t sram_free_;
    // 执行语句：size_t psram_total_;；它会更新一次变量、对象或程序状态。
    size_t psram_total_;
    // 执行语句：size_t psram_free_;；它会更新一次变量、对象或程序状态。
    size_t psram_free_;
};

/** CPU 主频（Hz） */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class CpuInfo
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_cpu_info：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static CpuInfo get_cpu_info();

    // 定义或声明函数 get_cpu_freq：参数在括号里，返回类型写在函数名前面。
    uint32_t get_cpu_freq() const
    {
        // 返回 cpu_frequency_ 给调用者，表示当前函数的执行结果。
        return cpu_frequency_;
    }

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 CpuInfo();，执行这个函数并处理它的返回值或状态变化。
    CpuInfo();

    // 执行语句：uint32_t cpu_frequency_;；它会更新一次变量、对象或程序状态。
    uint32_t cpu_frequency_;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::sys::info
