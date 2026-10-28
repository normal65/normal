/**
 * [初学者导读] 文件：main/app/ebook/apps/settings/lock_password.cc
 * 作用：保存和验证锁屏 PIN 的 SHA-256 摘要。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/lock_password.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <psa/crypto.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/persist.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::settings {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 固定参数 kKOn = "sec.lock.on"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kKOn = "sec.lock.on";
// 固定参数 kKHash = "sec.lock.h"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kKHash = "sec.lock.h";

// 定义或声明函数 is_digit_pin：参数在括号里，返回类型写在函数名前面。
bool is_digit_pin(const char* pin)
{
    // 判断条件：pin == nullptr 是否成立。
    if (pin == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < LockPassword::kPinLen; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char c = pin[i];
        // 判断条件：c < '0' || c > '9' 是否成立。
        if (c < '0' || c > '9')
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
    }
    // 返回 pin[LockPassword::kPinLen] == '\0' 给调用者，表示当前函数的执行结果。
    return pin[LockPassword::kPinLen] == '\0';
}

// 定义或声明函数 bytes_to_hex：参数在括号里，返回类型写在函数名前面。
void bytes_to_hex(const unsigned char* in, size_t n, char* out, size_t out_cap)
{
    // 判断条件：out == nullptr || out_cap < n * 2U + 1U 是否成立。
    if (out == nullptr || out_cap < n * 2U + 1U)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (size_t i = 0; i < n; ++i)
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        (void)std::snprintf(out + i * 2U, out_cap - i * 2U, "%02x", static_cast<unsigned>(in[i]));
    // 赋值：把右侧 '\0' 的结果保存到左侧 out[n * 2U]。
    out[n * 2U] = '\0';
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 LockPassword::get_instance：参数在括号里，返回类型写在函数名前面。
LockPassword& LockPassword::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static LockPassword s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// [初学者导读] LockPassword::load()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void LockPassword::load()
{
    // 赋值：把右侧 false 的结果保存到左侧 enabled_。
    enabled_ = false;
    // 赋值：把右侧 '\0' 的结果保存到左侧 hash_hex_[0]。
    hash_hex_[0] = '\0';

    // 定义变量 bool on 并初始化为 false。
    bool on = false;
    // 判断条件：data::Persist::get_bool(kKOn, on) 是否成立。
    if (data::Persist::get_bool(kKOn, on))
        // 赋值：把右侧 on 的结果保存到左侧 enabled_。
        enabled_ = on;

    // 赋值：把右侧 {} 的结果保存到左侧 char buf[sizeof(hash_hex_)]。
    char buf[sizeof(hash_hex_)] = {};
    // 判断条件：data::Persist::get_str(kKHash, buf, sizeof(buf)) && buf[0] != '\0' 是否成立。
    if (data::Persist::get_str(kKHash, buf, sizeof(buf)) && buf[0] != '\0')
    {
        // 调用 std::strncpy(hash_hex_, buf, sizeof(hash_hex_) - 1);，执行这个函数并处理它的返回值或状态变化。
        std::strncpy(hash_hex_, buf, sizeof(hash_hex_) - 1);
        // 赋值：把右侧 '\0' 的结果保存到左侧 hash_hex_[sizeof(hash_hex_) - 1]。
        hash_hex_[sizeof(hash_hex_) - 1] = '\0';
    }

    // 判断条件：enabled_ && hash_hex_[0] == '\0' 是否成立。
    if (enabled_ && hash_hex_[0] == '\0')
        // 赋值：把右侧 false 的结果保存到左侧 enabled_。
        enabled_ = false;
}

// 函数定义/声明：调用 LockPassword::valid_pin()：只允许恰好四位数字。这个函数是设置新 PIN 和验证输入的第一道检查。。
bool LockPassword::valid_pin(const char* pin)
{
    // 返回 is_digit_pin(pin) 给调用者，表示当前函数的执行结果。
    return is_digit_pin(pin);
}

// 函数定义/声明：调用 LockPassword::hash_pin()：用 PSA Crypto 的 SHA256 计算摘要，并转换为小写十六进制字符串。输出缓冲区必须至少 65 字节，包含结尾 \0。。
bool LockPassword::hash_pin(const char* pin, char* hex_out, size_t hex_cap) const
{
    // 判断条件：pin == nullptr || hex_out == nullptr || hex_cap < 65U 是否成立。
    if (pin == nullptr || hex_out == nullptr || hex_cap < 65U)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 执行语句：unsigned char digest[32]{};；它会更新一次变量、对象或程序状态。
    unsigned char digest[32]{};
    // 定义变量 size_t hash_len 并初始化为 0。
    size_t hash_len = 0;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const psa_status_t st = psa_hash_compute(PSA_ALG_SHA_256, reinterpret_cast<const uint8_t*>(pin),
                                             // 继续上一行的表达式或参数列表：std::strlen(pin), digest, sizeof(digest), &hash_len);。
                                             std::strlen(pin), digest, sizeof(digest), &hash_len);
    // 判断条件：st != PSA_SUCCESS || hash_len != 32U 是否成立。
    if (st != PSA_SUCCESS || hash_len != 32U)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 调用 bytes_to_hex(digest, 32, hex_out, hex_cap);，执行这个函数并处理它的返回值或状态变化。
    bytes_to_hex(digest, 32, hex_out, hex_cap);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数定义/声明：调用 LockPassword::matches_hash()：对输入 PIN 做哈希，再和保存值比较。它不是“解密密码”。。
bool LockPassword::matches_hash(const char* pin) const
{
    // 判断条件：!valid_pin(pin) || hash_hex_[0] == '\0' 是否成立。
    if (!valid_pin(pin) || hash_hex_[0] == '\0')
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 执行语句：char hex[65]{};；它会更新一次变量、对象或程序状态。
    char hex[65]{};
    // 判断条件：!hash_pin(pin, hex, sizeof(hex)) 是否成立。
    if (!hash_pin(pin, hex, sizeof(hex)))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 返回 std::strcmp(hex, hash_hex_) == 0 给调用者，表示当前函数的执行结果。
    return std::strcmp(hex, hash_hex_) == 0;
}

// 函数定义/声明：调用 LockPassword::verify()：先检查启用状态和 PIN 格式，再调用 matcheshash()。。
bool LockPassword::verify(const char* pin) const
{
    // 判断条件：!enabled_ 是否成立。
    if (!enabled_)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    // 返回结果：调用 matches_hash(pin)：对输入 PIN 做哈希，再和保存值比较。它不是“解密密码”。。
    return matches_hash(pin);
}

// 函数定义/声明：调用 LockPassword::set_new()：设置新 PIN： 1. 验证格式。 2. 计算哈希。 3. 保存 NVS。 4. 设置启用标志。。
bool LockPassword::set_new(const char* pin)
{
    // 判断条件：!valid_pin(pin) 是否成立。
    if (!valid_pin(pin))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 执行语句：char hex[65]{};；它会更新一次变量、对象或程序状态。
    char hex[65]{};
    // 判断条件：!hash_pin(pin, hex, sizeof(hex)) 是否成立。
    if (!hash_pin(pin, hex, sizeof(hex)))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 调用 std::strncpy(hash_hex_, hex, sizeof(hash_hex_) - 1);，执行这个函数并处理它的返回值或状态变化。
    std::strncpy(hash_hex_, hex, sizeof(hash_hex_) - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 hash_hex_[sizeof(hash_hex_) - 1]。
    hash_hex_[sizeof(hash_hex_) - 1] = '\0';
    // 赋值：把右侧 true 的结果保存到左侧 enabled_。
    enabled_ = true;

    // 函数调用：(void)data::Persist::set_bool(kKOn, true);；执行该函数并处理它的返回值或副作用。
    (void)data::Persist::set_bool(kKOn, true);
    // 函数调用：(void)data::Persist::set_str(kKHash, hash_hex_);；执行该函数并处理它的返回值或副作用。
    (void)data::Persist::set_str(kKHash, hash_hex_);
    // 调用 data::Persist::commit();，执行这个函数并处理它的返回值或状态变化。
    data::Persist::commit();
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// [初学者导读] LockPassword::clear()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void LockPassword::clear()
{
    // 赋值：把右侧 false 的结果保存到左侧 enabled_。
    enabled_ = false;
    // 赋值：把右侧 '\0' 的结果保存到左侧 hash_hex_[0]。
    hash_hex_[0] = '\0';
    // 函数调用：(void)data::Persist::set_bool(kKOn, false);；执行该函数并处理它的返回值或副作用。
    (void)data::Persist::set_bool(kKOn, false);
    // 函数调用：(void)data::Persist::set_str(kKHash, "");；执行该函数并处理它的返回值或副作用。
    (void)data::Persist::set_str(kKHash, "");
    // 调用 data::Persist::commit();，执行这个函数并处理它的返回值或状态变化。
    data::Persist::commit();
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::settings
