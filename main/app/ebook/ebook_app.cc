/**
 * [初学者导读] 文件：main/app/ebook/ebook_app.cc
 * 作用：实现 ebook 模块中的 ebook app 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：上层通过 Router、UiBus 和 SystemState 协作，不直接操作硬件。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ebook_app.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/result.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "platform/boot.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/task/task.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "EbookApp";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// [初学者导读] main_idle_loop()：主任务空闲循环。详见 docs/代码详解-初学者版.md。
void main_idle_loop()
{
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (true)
        // 调用 app::sys::task::TaskMgr::delay_ms(5000)：调用 vTaskDelay()，这是让出 CPU 的阻塞延时。一千毫秒延时期间，任务不占 CPU，调度器可以运行别人。。
        ::app::sys::task::TaskMgr::delay_ms(5000);
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 函数定义/声明：调用 ebook_app_run()：项目总入口。
extern "C" void ebook_app_run(void)
{
    // 判断条件：::app::ebook::core::ok(::app::ebook::platform::boot()) 是否成立。
    if (::app::ebook::core::ok(::app::ebook::platform::boot()))
        // 调用 EBOOK_LOGI(TAG, "running");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGI(TAG, "running");
    // 备用分支：前面的条件不成立时执行这里。
    else
        // 调用 EBOOK_LOGE(TAG, "boot failed");，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGE(TAG, "boot failed");

    // 调用 main_idle_loop()：主任务空闲循环。
    main_idle_loop();
}
