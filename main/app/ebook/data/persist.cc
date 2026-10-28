/**
 * [初学者导读] 文件：main/app/ebook/data/persist.cc
 * 作用：封装 NVS 的初始化、读写、提交和命名空间管理。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：小配置写 NVS，运行时共享状态放 SystemState。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/persist.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <nvs.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <nvs_flash.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/log.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "Persist";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::data {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 固定参数 kNamespace = "ebook"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kNamespace = "ebook";

// struct Ctx：把相关数据字段组合在一起，表示项目中的一个概念。
struct Ctx
{
    // 执行语句：nvs_handle_t handle{0};；它会更新一次变量、对象或程序状态。
    nvs_handle_t handle{0};
    // 执行语句：bool ready{false};；它会更新一次变量、对象或程序状态。
    bool ready{false};
};

// 声明并默认初始化对象：Ctx g_ctx{}。
Ctx g_ctx{};

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 Persist::init：参数在括号里，返回类型写在函数名前面。
core::Status Persist::init()
{
    // 判断条件：g_ctx.ready 是否成立。
    if (g_ctx.ready)
        // 返回 core::Status::Ok 给调用者，表示当前函数的执行结果。
        return core::Status::Ok;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const esp_err_t r = nvs_open(kNamespace, NVS_READWRITE, &g_ctx.handle);
    // 判断条件：r != ESP_OK 是否成立。
    if (r != ESP_OK)
    {
        // 调用 EBOOK_LOGW(TAG, "nvs_open: %s", esp_err_to_name(r));，执行这个函数并处理它的返回值或状态变化。
        EBOOK_LOGW(TAG, "nvs_open: %s", esp_err_to_name(r));
        // 返回 core::Status::IoError 给调用者，表示当前函数的执行结果。
        return core::Status::IoError;
    }
    // 赋值：把右侧 true 的结果保存到左侧 g_ctx.ready。
    g_ctx.ready = true;
    // 返回 core::Status::Ok 给调用者，表示当前函数的执行结果。
    return core::Status::Ok;
}

// 定义或声明函数 Persist::deinit：参数在括号里，返回类型写在函数名前面。
void Persist::deinit()
{
    // 判断条件：!g_ctx.ready 是否成立。
    if (!g_ctx.ready)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 调用 nvs_close(g_ctx.handle);，执行这个函数并处理它的返回值或状态变化。
    nvs_close(g_ctx.handle);
    // 赋值：把右侧 {} 的结果保存到左侧 g_ctx。
    g_ctx = {};
}

// 定义或声明函数 Persist::get_u8：参数在括号里，返回类型写在函数名前面。
bool Persist::get_u8(const char* key, uint8_t& out)
{
    // 判断条件：!g_ctx.ready 是否成立。
    if (!g_ctx.ready)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 返回 nvs_get_u8(g_ctx.handle, key, &out) == ESP_OK 给调用者，表示当前函数的执行结果。
    return nvs_get_u8(g_ctx.handle, key, &out) == ESP_OK;
}

// 定义或声明函数 Persist::set_u8：参数在括号里，返回类型写在函数名前面。
bool Persist::set_u8(const char* key, uint8_t v)
{
    // 判断条件：!g_ctx.ready 是否成立。
    if (!g_ctx.ready)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 返回 nvs_set_u8(g_ctx.handle, key, v) == ESP_OK 给调用者，表示当前函数的执行结果。
    return nvs_set_u8(g_ctx.handle, key, v) == ESP_OK;
}

// 定义或声明函数 Persist::get_bool：参数在括号里，返回类型写在函数名前面。
bool Persist::get_bool(const char* key, bool& out)
{
    // 定义变量 uint8_t v 并初始化为 0。
    uint8_t v = 0;
    // 判断条件：!get_u8(key, v) 是否成立。
    if (!get_u8(key, v))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 赋值：把右侧 (v != 0) 的结果保存到左侧 out。
    out = (v != 0);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 Persist::set_bool：参数在括号里，返回类型写在函数名前面。
bool Persist::set_bool(const char* key, bool v)
{
    // 返回 set_u8(key, v ? 1U : 0U) 给调用者，表示当前函数的执行结果。
    return set_u8(key, v ? 1U : 0U);
}

// 定义或声明函数 Persist::get_str：参数在括号里，返回类型写在函数名前面。
bool Persist::get_str(const char* key, char* out, size_t out_size)
{
    // 判断条件：!g_ctx.ready || out == nullptr || out_size == 0 是否成立。
    if (!g_ctx.ready || out == nullptr || out_size == 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 定义变量 size_t len 并初始化为 out_size。
    size_t len = out_size;
    // 判断条件：nvs_get_str(g_ctx.handle, key, out, &len) != ESP_OK 是否成立。
    if (nvs_get_str(g_ctx.handle, key, out, &len) != ESP_OK)
    {
        // 赋值：把右侧 '\0' 的结果保存到左侧 out[0]。
        out[0] = '\0';
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 Persist::set_str：参数在括号里，返回类型写在函数名前面。
bool Persist::set_str(const char* key, const char* v)
{
    // 判断条件：!g_ctx.ready || v == nullptr 是否成立。
    if (!g_ctx.ready || v == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 返回 nvs_set_str(g_ctx.handle, key, v) == ESP_OK 给调用者，表示当前函数的执行结果。
    return nvs_set_str(g_ctx.handle, key, v) == ESP_OK;
}

// 定义或声明函数 Persist::commit：参数在括号里，返回类型写在函数名前面。
void Persist::commit()
{
    // 判断条件：g_ctx.ready 是否成立。
    if (g_ctx.ready)
        // 调用 nvs_commit(g_ctx.handle);，执行这个函数并处理它的返回值或状态变化。
        nvs_commit(g_ctx.handle);
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::data
