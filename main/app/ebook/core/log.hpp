// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/core/log.hpp
 * 作用：实现 core 模块中的 log 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：上层通过 Router、UiBus 和 SystemState 协作，不直接操作硬件。
 * 详细解释：docs/代码详解-初学者版.md。
 */
/** @brief ebook 日志门面（EBOOK_LOG*；粒度由 EBOOK_LOG_LEVEL 编译期裁剪） */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_log.h>

// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#ifndef EBOOK_LOG_LEVEL
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#define EBOOK_LOG_LEVEL ESP_LOG_INFO
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif

// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if EBOOK_LOG_LEVEL >= ESP_LOG_ERROR
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#define EBOOK_LOGE(tag, fmt, ...) ESP_LOGE(tag, fmt, ##__VA_ARGS__)
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#else
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#define EBOOK_LOGE(tag, fmt, ...) ((void)0)
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif

// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if EBOOK_LOG_LEVEL >= ESP_LOG_WARN
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#define EBOOK_LOGW(tag, fmt, ...) ESP_LOGW(tag, fmt, ##__VA_ARGS__)
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#else
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#define EBOOK_LOGW(tag, fmt, ...) ((void)0)
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif

// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if EBOOK_LOG_LEVEL >= ESP_LOG_INFO
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#define EBOOK_LOGI(tag, fmt, ...) ESP_LOGI(tag, fmt, ##__VA_ARGS__)
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#else
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#define EBOOK_LOGI(tag, fmt, ...) ((void)0)
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif

// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if EBOOK_LOG_LEVEL >= ESP_LOG_DEBUG
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#define EBOOK_LOGD(tag, fmt, ...) ESP_LOGD(tag, fmt, ##__VA_ARGS__)
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#else
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#define EBOOK_LOGD(tag, fmt, ...) ((void)0)
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif

// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if EBOOK_LOG_LEVEL >= ESP_LOG_VERBOSE
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#define EBOOK_LOGV(tag, fmt, ...) ESP_LOGV(tag, fmt, ##__VA_ARGS__)
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#else
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#define EBOOK_LOGV(tag, fmt, ...) ((void)0)
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
