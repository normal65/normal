// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/ebook_app.hpp
 * 作用：实现 ebook 模块中的 ebook app 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：上层通过 Router、UiBus 和 SystemState 协作，不直接操作硬件。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#ifdef __cplusplus
// 执行这条代码：extern "C"。它会更新变量、对象状态或调用链。
extern "C"
{
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif

    /** @brief main.cc 入口：platform::boot() 后进入 idle loop */
    // 调用 ebook_app_run(void)：项目总入口。
    void ebook_app_run(void);

// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#ifdef __cplusplus
}
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
