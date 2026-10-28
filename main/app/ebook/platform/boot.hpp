// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/platform/boot.hpp
 * 作用：按依赖顺序初始化 NVS、显示、触摸、音频、数据、电池和任务。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：阻塞工作放独立任务，周期小检查放 Housekeeper。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/result.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::platform {

/** @brief 启动链：system → persist → display → touch → data → battery → tasks */
// 调用 boot()：有顺序的启动流水线。
core::Status boot();

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::platform
