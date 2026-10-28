// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/common/storage/storage_usb_msc.hpp
 * 作用：实现 USB MSC 对内部存储的挂载和恢复。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：挂载状态有锁和热插拔风险，慢文件操作应放后台任务。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_err.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "sdkconfig.h"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::common::storage {

// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if CONFIG_TINYUSB_MSC_ENABLED

/** 初始化 userdata FAT + TinyUSB MSC，默认挂载给应用 (/int) */
// 调用 usb_msc_init_internal()：步骤： 1. 创建二进制信号量，用于等待挂载完成事件。 2. 找到 userdata 分区并调用 wlmount()。 3. 配置 TinyUSB MSC 存储，默认挂载点为应用。 4. 注册存储事件回调。 5. 安装 TinyUSB 驱动，注册 USB 设备事件回调。。
esp_err_t usb_msc_init_internal();

/** 将 userdata 暴露给 USB 主机（PC）；应用不可再访问 /int */
// 函数调用：esp_err_t usb_msc_expose_to_host();；执行该函数并处理它的返回值或副作用。
esp_err_t usb_msc_expose_to_host();

/** 从 PC 收回 userdata，恢复应用访问 /int */
// 函数调用：esp_err_t usb_msc_reclaim_for_app();；执行该函数并处理它的返回值或副作用。
esp_err_t usb_msc_reclaim_for_app();

// 函数调用：bool usb_msc_is_exposed_to_host();；执行该函数并处理它的返回值或副作用。
bool usb_msc_is_exposed_to_host();

/** 应用侧是否可实际访问 /int（已挂回且 VFS 可用） */
// 函数调用：bool usb_msc_is_available_to_app();；执行该函数并处理它的返回值或副作用。
bool usb_msc_is_available_to_app();

// 函数调用：bool usb_msc_is_initialized();；执行该函数并处理它的返回值或副作用。
bool usb_msc_is_initialized();

// 函数调用：void usb_msc_shutdown();；执行该函数并处理它的返回值或副作用。
void usb_msc_shutdown();

/** 周期调用：直接拔线未安全弹出时自动挂回 /int */
// 调用 usb_msc_service_recovery()：周期调用的恢复逻辑： 如果 /int 已经可访问，清空失败计数。 如果当前暴露给电脑，但 USB 总线挂起或断开，连续 3 次判定已经拔线。 如果当前不是暴露状态但 /int 又不可访问，也尝试收回。 挂回最多重试 12 次，避免无限刷日志。。
void usb_msc_service_recovery();

// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif

// 命名空间结束：离开当前模块作用域。
} // namespace app::common::storage
