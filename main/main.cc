/**
 * [初学者导读] 文件：main/main.cc
 * 作用：实现 main 模块中的 main 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：app_main 只负责把控制权交给项目入口，随后由常驻任务运行。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ebook/ebook_app.hpp"

// 函数定义/声明：调用 app_main()：ESPIDF 固件入口。
extern "C" void app_main(void)
{
    // 调用 ebook_app_run()：项目总入口。
    ebook_app_run();
}
