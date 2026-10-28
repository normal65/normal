// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/settings/lock_password.hpp
 * 作用：保存和验证锁屏 PIN 的 SHA-256 摘要。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstddef>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::settings {

/** @brief 锁屏 PIN 仓库（NVS: sec.lock.on / sec.lock.h，SHA256 存哈希） */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class LockPassword
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 固定参数 kPinLen = 4：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kPinLen = 4;

    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static LockPassword& get_instance();

    // 函数调用：void load();；执行该函数并处理它的返回值或副作用。
    void load();

    // 定义或声明函数 enabled：参数在括号里，返回类型写在函数名前面。
    bool enabled() const
    {
        // 返回 enabled_ 给调用者，表示当前函数的执行结果。
        return enabled_;
    }

    // 静态函数 valid_pin：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool valid_pin(const char* pin);

    // 调用 verify(constchar*pin)：先检查启用状态和 PIN 格式，再调用 matcheshash()。。
    bool verify(const char* pin) const;
    // 调用 set_new(constchar*pin)：设置新 PIN： 1. 验证格式。 2. 计算哈希。 3. 保存 NVS。 4. 设置启用标志。。
    bool set_new(const char* pin);
    // 调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    void clear();

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 LockPassword() = default;，执行这个函数并处理它的返回值或状态变化。
    LockPassword() = default;

    // 调用 hash_pin(constchar*pin,char*hex_out,size_thex_cap)：用 PSA Crypto 的 SHA256 计算摘要，并转换为小写十六进制字符串。输出缓冲区必须至少 65 字节，包含结尾 \0。。
    bool hash_pin(const char* pin, char* hex_out, size_t hex_cap) const;
    // 调用 matches_hash(constchar*pin)：对输入 PIN 做哈希，再和保存值比较。它不是“解密密码”。。
    bool matches_hash(const char* pin) const;

    // 执行语句：bool enabled_{false};；它会更新一次变量、对象或程序状态。
    bool enabled_{false};
    // 执行语句：char hash_hex_[65]{};；它会更新一次变量、对象或程序状态。
    char hash_hex_[65]{};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::settings
