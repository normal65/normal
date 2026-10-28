// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/common/storage/storage.hpp
 * 作用：管理内部 Flash、assets、SD 卡和 USB 文件系统挂载状态。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：挂载状态有锁和热插拔风险，慢文件操作应放后台任务。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <mutex>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_err.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "wear_levelling.h"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::common::storage {

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class MountKind : uint8_t
{
    // 执行这条代码：Internal = 0,。它会更新变量、对象状态或调用链。
    Internal = 0,
    // 继续上一行的表达式或参数列表：Sd = 1,。
    Sd = 1,
    // 继续上一行的表达式或参数列表：Usb = 2,。
    Usb = 2,
    // 继续上一行的表达式或参数列表：Assets = 3,。
    Assets = 3,
    // 继续上一行的表达式或参数列表：Invalid = 0xFF,。
    Invalid = 0xFF,
};

/**
 * @brief 挂载点单例；TF 线宽与 CD 由 config.hpp 的 TF_CARD_* 推断
 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class StorageMgr
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static StorageMgr& get_instance();

    // 调用 StorageMgr(const StorageMgr&) = delete;，执行这个函数并处理它的返回值或状态变化。
    StorageMgr(const StorageMgr&) = delete;
    // 赋值：把右侧 (const StorageMgr&) = delete 的结果保存到左侧 StorageMgr& operator。
    StorageMgr& operator=(const StorageMgr&) = delete;

    /** @brief 挂载 userdata (/int) 与 assets (/assets) */
    // 调用 init_defaults()：第一步挂载 /int，第二步挂载 /assets。如果 assets 失败，会回滚卸载 /int，避免系统处于“内部用户区可用但字体资源不可用”的半初始化状态。。
    esp_err_t init_defaults();

    // 调用 deinit()：创建和销毁队列。
    void deinit();

    // 函数调用：esp_err_t mount_assets();；执行该函数并处理它的返回值或副作用。
    esp_err_t mount_assets();
    // 函数调用：esp_err_t umount_assets();；执行该函数并处理它的返回值或副作用。
    esp_err_t umount_assets();

    // 函数调用：esp_err_t mount_internal();；执行该函数并处理它的返回值或副作用。
    esp_err_t mount_internal();
    // 函数调用：esp_err_t umount_internal();；执行该函数并处理它的返回值或副作用。
    esp_err_t umount_internal();

    // 函数调用：esp_err_t mount_sd();；执行该函数并处理它的返回值或副作用。
    esp_err_t mount_sd();
    // 函数调用：esp_err_t umount_sd();；执行该函数并处理它的返回值或副作用。
    esp_err_t umount_sd();

    // 调用 service_sd_hotplug()：这个函数被 Housekeeper 周期性调用。它先做时间节流，默认每 500 ms 最多执行一次，避免在 UI 循环里频繁访问 SD 总线。。
    void service_sd_hotplug();
    // 函数调用：void service_internal_recovery();；执行该函数并处理它的返回值或副作用。
    void service_internal_recovery();

    // 调用 mount_usb()：当前直接返回 ESPERRNOTSUPPORTED，并记录“外接 U 盘 Host 未实现”。这不是暂时没启动，而是代码层没有 USB Host 存储挂载实现。。
    esp_err_t mount_usb();
    // 函数调用：esp_err_t umount_usb();；执行该函数并处理它的返回值或副作用。
    esp_err_t umount_usb();

    // 函数调用：esp_err_t expose_internal_to_pc();；执行该函数并处理它的返回值或副作用。
    esp_err_t expose_internal_to_pc();
    // 函数调用：esp_err_t reclaim_internal_from_pc();；执行该函数并处理它的返回值或副作用。
    esp_err_t reclaim_internal_from_pc();
    // 函数调用：bool is_internal_exposed_to_pc() const;；执行该函数并处理它的返回值或副作用。
    bool is_internal_exposed_to_pc() const;

    // 调用 is_mounted(MountKindkind)：内部存储要同时看： text internalmounted && usbmscisavailabletoapp()。
    bool is_mounted(MountKind kind) const;

    // 函数调用：esp_err_t query_assets_usage(size_t* total_out, size_t* used_out) const;；执行该函数并处理它的返回值或副作用。
    esp_err_t query_assets_usage(size_t* total_out, size_t* used_out) const;
    // 函数调用：esp_err_t query_internal_usage(size_t* total_out, size_t* used_out) const;；执行该函数并处理它的返回值或副作用。
    esp_err_t query_internal_usage(size_t* total_out, size_t* used_out) const;
    // 函数调用：esp_err_t query_sd_usage(uint64_t* total_bytes_out, uint64_t* used_bytes_out) const;；执行该函数并处理它的返回值或副作用。
    esp_err_t query_sd_usage(uint64_t* total_bytes_out, uint64_t* used_bytes_out) const;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 StorageMgr() = default;，执行这个函数并处理它的返回值或状态变化。
    StorageMgr() = default;
    // 定义或声明函数 ~StorageMgr：参数在括号里，返回类型写在函数名前面。
    ~StorageMgr()
    {
        // 调用 deinit()：创建和销毁队列。
        deinit();
    }

    // 调用 mount_assets_locked()：使用 espvfslittlefsregister()： base path /assets partition label assets formatifmountfailed = 0 readonly = 1。
    esp_err_t mount_assets_locked();
    // 函数调用：esp_err_t umount_assets_locked();；执行该函数并处理它的返回值或副作用。
    esp_err_t umount_assets_locked();
    // 调用 mount_internal_locked()：两种路径。
    esp_err_t mount_internal_locked();
    // 调用 umount_internal_locked()：如果启用了 USB MSC，先检查是否暴露给电脑；如果是，先收回给应用，再 shutdown USB MSC。随后清空句柄和状态。。
    esp_err_t umount_internal_locked();
    // 调用 mount_sd_locked(boollog_errors=true)：硬件型号决定配置。
    esp_err_t mount_sd_locked(bool log_errors = true);
    // 调用 umount_sd_locked()：只有卡已挂载且指针存在时才调用 espvfsfatsdcardunmount()。成功后清空 sdcard 和状态。。
    esp_err_t umount_sd_locked();
    // 函数调用：esp_err_t umount_usb_locked();；执行该函数并处理它的返回值或副作用。
    esp_err_t umount_usb_locked();

    // 执行语句：mutable std::mutex mutex_;；它会更新一次变量、对象或程序状态。
    mutable std::mutex mutex_;
    // 执行语句：bool assets_mounted_{false};；它会更新一次变量、对象或程序状态。
    bool assets_mounted_{false};
    // 执行语句：bool internal_mounted_{false};；它会更新一次变量、对象或程序状态。
    bool internal_mounted_{false};
    // 执行语句：bool sd_mounted_{false};；它会更新一次变量、对象或程序状态。
    bool sd_mounted_{false};
    // 执行语句：bool usb_mounted_{false};；它会更新一次变量、对象或程序状态。
    bool usb_mounted_{false};
    // 执行语句：void* sd_card_{nullptr};；它会更新一次变量、对象或程序状态。
    void* sd_card_{nullptr};
    // 执行语句：wl_handle_t wl_handle_{WL_INVALID_HANDLE};；它会更新一次变量、对象或程序状态。
    wl_handle_t wl_handle_{WL_INVALID_HANDLE};
};

// 赋值：把右侧 "/int" 的结果保存到左侧 inline constexpr const char* k_path_internal。
inline constexpr const char* k_path_internal = "/int";
// 赋值：把右侧 "/sd" 的结果保存到左侧 inline constexpr const char* k_path_sd。
inline constexpr const char* k_path_sd = "/sd";
// 赋值：把右侧 "/usb" 的结果保存到左侧 inline constexpr const char* k_path_usb。
inline constexpr const char* k_path_usb = "/usb";
// 赋值：把右侧 "/assets" 的结果保存到左侧 inline constexpr const char* k_path_assets。
inline constexpr const char* k_path_assets = "/assets";

// 赋值：把右侧 "assets" 的结果保存到左侧 inline constexpr const char* k_label_assets。
inline constexpr const char* k_label_assets = "assets";
// 赋值：把右侧 "userdata" 的结果保存到左侧 inline constexpr const char* k_label_userdata。
inline constexpr const char* k_label_userdata = "userdata";

// 定义或声明函数 path_has_prefix：参数在括号里，返回类型写在函数名前面。
inline bool path_has_prefix(const char* path, const char* prefix)
{
    // 判断条件：path == nullptr || prefix == nullptr 是否成立。
    if (path == nullptr || prefix == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const size_t n = std::strlen(prefix);
    // 判断条件：std::strncmp(path, prefix, n) != 0 是否成立。
    if (std::strncmp(path, prefix, n) != 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 返回 path[n] == '\0' || path[n] == '/' 给调用者，表示当前函数的执行结果。
    return path[n] == '\0' || path[n] == '/';
}

// 定义或声明函数 path_is_system：参数在括号里，返回类型写在函数名前面。
inline bool path_is_system(const char* path)
{
    // 返回 path_has_prefix(path, k_path_assets) 给调用者，表示当前函数的执行结果。
    return path_has_prefix(path, k_path_assets);
}

// 函数调用：MountKind mount_kind_from_path_prefix(const char* path);；执行该函数并处理它的返回值或副作用。
MountKind mount_kind_from_path_prefix(const char* path);

// 命名空间结束：离开当前模块作用域。
} // namespace app::common::storage
