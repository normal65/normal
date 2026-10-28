// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/protocol/file_server/file_server.hpp
 * 作用：启动局域网文件服务，让电脑或手机访问设备文件。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：网络请求可能阻塞，必须在后台任务或专门任务中执行。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_err.h"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::protocol::file_server {

/** @brief 热点局域网文件传输（HTTP 80 + DNS Captive Portal，操作 /int） */
// 调用 start()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
esp_err_t start();

// 调用 stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
void stop();

// 函数调用：bool is_running();；执行该函数并处理它的返回值或副作用。
bool is_running();

// 命名空间结束：离开当前模块作用域。
} // namespace app::protocol::file_server
