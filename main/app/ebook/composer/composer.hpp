// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/composer/composer.hpp
 * 作用：按 Shell、页面、覆盖层顺序合成一帧画面。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：合成顺序决定覆盖层和页面谁先画。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::composer {

/** @brief 按 Router 栈合成 back 帧（Shell → App → Overlay） */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class Composer
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static Composer& instance();

    // 函数调用：void paint(uint8_t* back_fb);；执行该函数并处理它的返回值或副作用。
    void paint(uint8_t* back_fb);

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 Composer() = default;，执行这个函数并处理它的返回值或状态变化。
    Composer() = default;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::composer
