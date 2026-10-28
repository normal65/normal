/**
 * [初学者导读] 文件：main/common/storage/storage.cc
 * 作用：管理内部 Flash、assets、SD 卡和 USB 文件系统挂载状态。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：挂载状态有锁和热插拔风险，慢文件操作应放后台任务。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "storage.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "storage_usb_msc.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <inttypes.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "config/config.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "driver/gpio.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "driver/sdmmc_host.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_littlefs.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_log.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_timer.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_vfs_fat.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "sdmmc_cmd.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "sdkconfig.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "soc/soc_caps.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/task/task.hpp"

// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#ifndef CONFIG_EBOOK_STORAGE_FORMAT_IF_MOUNT_FAILED
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#define CONFIG_EBOOK_STORAGE_FORMAT_IF_MOUNT_FAILED 0
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#ifndef CONFIG_EBOOK_SD_SDMMC_ENABLE
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#define CONFIG_EBOOK_SD_SDMMC_ENABLE 0
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#ifndef CONFIG_EBOOK_SD_FORMAT_IF_MOUNT_FAILED
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#define CONFIG_EBOOK_SD_FORMAT_IF_MOUNT_FAILED 0
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#ifndef CONFIG_EBOOK_SD_SPEED_HS
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#define CONFIG_EBOOK_SD_SPEED_HS 0
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#ifndef CONFIG_EBOOK_SD_MAX_FILES
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#define CONFIG_EBOOK_SD_MAX_FILES 5
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#ifndef CONFIG_EBOOK_INTERNAL_MAX_FILES
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#define CONFIG_EBOOK_INTERNAL_MAX_FILES 10
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#ifndef CONFIG_EBOOK_SD_HOTPLUG_SERVICE_MS
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#define CONFIG_EBOOK_SD_HOTPLUG_SERVICE_MS 500
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::common::storage {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 只读约束：这个值或对象在当前位置不能被修改。
const char* const TAG = "Storage";

// 定义或声明函数 pin_valid：参数在括号里，返回类型写在函数名前面。
bool pin_valid(gpio_num_t pin)
{
    // 返回 pin != GPIO_NUM_NC 给调用者，表示当前函数的执行结果。
    return pin != GPIO_NUM_NC;
}

// 定义或声明函数 is_four_line_mode：参数在括号里，返回类型写在函数名前面。
bool is_four_line_mode()
{
    // 返回 pin_valid(config::TF_CARD_D1) && pin_valid(config::TF_CARD_D2) && 给调用者，表示当前函数的执行结果。
    return pin_valid(config::TF_CARD_D1) && pin_valid(config::TF_CARD_D2) &&
           // 继续上一行的表达式或参数列表：pin_valid(config::TF_CARD_D3);。
           pin_valid(config::TF_CARD_D3);
}

// 定义或声明函数 has_hardware_cd：参数在括号里，返回类型写在函数名前面。
bool has_hardware_cd()
{
    // 返回 pin_valid(config::TF_CARD_CD) 给调用者，表示当前函数的执行结果。
    return pin_valid(config::TF_CARD_CD);
}

// 定义或声明函数 sd_pins_configured：参数在括号里，返回类型写在函数名前面。
bool sd_pins_configured()
{
    // 返回 pin_valid(config::TF_CARD_CLK) && pin_valid(config::TF_CARD_CMD) && 给调用者，表示当前函数的执行结果。
    return pin_valid(config::TF_CARD_CLK) && pin_valid(config::TF_CARD_CMD) &&
           // 继续上一行的表达式或参数列表：pin_valid(config::TF_CARD_D0);。
           pin_valid(config::TF_CARD_D0);
}

// 定义或声明函数 log_sd_pin_config：参数在括号里，返回类型写在函数名前面。
void log_sd_pin_config()
{
    // 调用 ESP_LOGD(TAG, "SDMMC: CLK=%d CMD=%d D0=%d D1=%d D2=%d D3=%d CD=%d → %s%s",，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "SDMMC: CLK=%d CMD=%d D0=%d D1=%d D2=%d D3=%d CD=%d → %s%s",
             // 继续上一行的表达式或参数列表：static_cast<int>(config::TF_CARD_CLK), static_cast<int>(config::TF_CARD_CMD),。
             static_cast<int>(config::TF_CARD_CLK), static_cast<int>(config::TF_CARD_CMD),
             // 继续上一行的表达式或参数列表：static_cast<int>(config::TF_CARD_D0), static_cast<int>(config::TF_CARD_D1),。
             static_cast<int>(config::TF_CARD_D0), static_cast<int>(config::TF_CARD_D1),
             // 继续上一行的表达式或参数列表：static_cast<int>(config::TF_CARD_D2), static_cast<int>(config::TF_CARD_D3),。
             static_cast<int>(config::TF_CARD_D2), static_cast<int>(config::TF_CARD_D3),
             // 继续上一行的表达式或参数列表：static_cast<int>(config::TF_CARD_CD), is_four_line_mode() ? "4线" : "1线",。
             static_cast<int>(config::TF_CARD_CD), is_four_line_mode() ? "4线" : "1线",
             // 继续上一行的表达式或参数列表：has_hardware_cd() ? " 硬件CD" : " 软件轮询");。
             has_hardware_cd() ? " 硬件CD" : " 软件轮询");
}

// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if SOC_SDMMC_HOST_SUPPORTED
// 定义或声明函数 sd_cd_card_present：参数在括号里，返回类型写在函数名前面。
bool sd_cd_card_present()
{
    // 固定参数 cd = config::TF_CARD_CD：由当前模块定义，用于控制布局、时序或硬件行为。
    constexpr gpio_num_t cd = config::TF_CARD_CD;
    // 定义或声明函数 constexpr：参数在括号里，返回类型写在函数名前面。
    if constexpr (cd == GPIO_NUM_NC)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 定义变量 gpio_config_t io 并初始化为 {}。
        gpio_config_t io = {};
        // 赋值：把右侧 1ULL << static_cast<unsigned>(cd) 的结果保存到左侧 io.pin_bit_mask。
        io.pin_bit_mask = 1ULL << static_cast<unsigned>(cd);
        // 赋值：把右侧 GPIO_MODE_INPUT 的结果保存到左侧 io.mode。
        io.mode = GPIO_MODE_INPUT;
        // 赋值：把右侧 GPIO_PULLUP_ENABLE 的结果保存到左侧 io.pull_up_en。
        io.pull_up_en = GPIO_PULLUP_ENABLE;
        // 赋值：把右侧 GPIO_PULLDOWN_DISABLE 的结果保存到左侧 io.pull_down_en。
        io.pull_down_en = GPIO_PULLDOWN_DISABLE;
        // 赋值：把右侧 GPIO_INTR_DISABLE 的结果保存到左侧 io.intr_type。
        io.intr_type = GPIO_INTR_DISABLE;
        // 调用 gpio_config(&io);，执行这个函数并处理它的返回值或状态变化。
        gpio_config(&io);
        // 返回 gpio_get_level(cd) == 0 给调用者，表示当前函数的执行结果。
        return gpio_get_level(cd) == 0;
    }
}
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 StorageMgr::get_instance：参数在括号里，返回类型写在函数名前面。
StorageMgr& StorageMgr::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static StorageMgr instance;
    // 返回 instance 给调用者，表示当前函数的执行结果。
    return instance;
}

// 函数定义/声明：调用 StorageMgr::init_defaults()：第一步挂载 /int，第二步挂载 /assets。如果 assets 失败，会回滚卸载 /int，避免系统处于“内部用户区可用但字体资源不可用”的半初始化状态。。
esp_err_t StorageMgr::init_defaults()
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);

    // 定义变量并调用函数初始化：调用 mount_internal_locked()：两种路径，结果保存到 esp_err_t err。
    esp_err_t err = mount_internal_locked();
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 返回 err 给调用者，表示当前函数的执行结果。
        return err;
    }

    // 调用 mount_assets_locked()：使用 espvfslittlefsregister()： base path /assets partition label assets formatifmountfailed = 0 readonly = 1。
    err = mount_assets_locked();
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 调用 umount_internal_locked()：如果启用了 USB MSC，先检查是否暴露给电脑；如果是，先收回给应用，再 shutdown USB MSC。随后清空句柄和状态。。
        (void)umount_internal_locked();
        // 返回 err 给调用者，表示当前函数的执行结果。
        return err;
    }

    // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
    return ESP_OK;
}

// 定义或声明函数 StorageMgr::deinit：参数在括号里，返回类型写在函数名前面。
void StorageMgr::deinit()
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);

    // 判断条件：sd_mounted_ 是否成立。
    if (sd_mounted_)
    {
        // 调用 umount_sd_locked()：只有卡已挂载且指针存在时才调用 espvfsfatsdcardunmount()。成功后清空 sdcard 和状态。。
        (void)umount_sd_locked();
    }
    // 判断条件：usb_mounted_ 是否成立。
    if (usb_mounted_)
    {
        // 函数调用：(void)umount_usb_locked();；执行该函数并处理它的返回值或副作用。
        (void)umount_usb_locked();
    }
    // 判断条件：assets_mounted_ 是否成立。
    if (assets_mounted_)
    {
        // 函数调用：(void)umount_assets_locked();；执行该函数并处理它的返回值或副作用。
        (void)umount_assets_locked();
    }
    // 判断条件：internal_mounted_ 是否成立。
    if (internal_mounted_)
    {
        // 调用 umount_internal_locked()：如果启用了 USB MSC，先检查是否暴露给电脑；如果是，先收回给应用，再 shutdown USB MSC。随后清空句柄和状态。。
        (void)umount_internal_locked();
    }
}

// 定义或声明函数 StorageMgr::mount_assets：参数在括号里，返回类型写在函数名前面。
esp_err_t StorageMgr::mount_assets()
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 返回结果：调用 mount_assets_locked()：使用 espvfslittlefsregister()： base path /assets partition label assets formatifmountfailed = 0 readonly = 1。
    return mount_assets_locked();
}

// 定义或声明函数 StorageMgr::umount_assets：参数在括号里，返回类型写在函数名前面。
esp_err_t StorageMgr::umount_assets()
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 返回 umount_assets_locked() 给调用者，表示当前函数的执行结果。
    return umount_assets_locked();
}

// 定义或声明函数 StorageMgr::mount_internal：参数在括号里，返回类型写在函数名前面。
esp_err_t StorageMgr::mount_internal()
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 返回结果：调用 mount_internal_locked()：两种路径。
    return mount_internal_locked();
}

// 定义或声明函数 StorageMgr::umount_internal：参数在括号里，返回类型写在函数名前面。
esp_err_t StorageMgr::umount_internal()
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 返回结果：调用 umount_internal_locked()：如果启用了 USB MSC，先检查是否暴露给电脑；如果是，先收回给应用，再 shutdown USB MSC。随后清空句柄和状态。。
    return umount_internal_locked();
}

// 定义或声明函数 StorageMgr::mount_sd：参数在括号里，返回类型写在函数名前面。
esp_err_t StorageMgr::mount_sd()
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 返回结果：调用 mount_sd_locked(true)：硬件型号决定配置。
    return mount_sd_locked(true);
}

// 定义或声明函数 StorageMgr::umount_sd：参数在括号里，返回类型写在函数名前面。
esp_err_t StorageMgr::umount_sd()
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 返回结果：调用 umount_sd_locked()：只有卡已挂载且指针存在时才调用 espvfsfatsdcardunmount()。成功后清空 sdcard 和状态。。
    return umount_sd_locked();
}

// 函数定义/声明：调用 StorageMgr::mount_usb()：当前直接返回 ESPERRNOTSUPPORTED，并记录“外接 U 盘 Host 未实现”。这不是暂时没启动，而是代码层没有 USB Host 存储挂载实现。。
esp_err_t StorageMgr::mount_usb()
{
    // 调用 ESP_LOGW(TAG, "mount_usb: 外接 U 盘 (Host) 未实现");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGW(TAG, "mount_usb: 外接 U 盘 (Host) 未实现");
    // 返回 ESP_ERR_NOT_SUPPORTED 给调用者，表示当前函数的执行结果。
    return ESP_ERR_NOT_SUPPORTED;
}

// 定义或声明函数 StorageMgr::expose_internal_to_pc：参数在括号里，返回类型写在函数名前面。
esp_err_t StorageMgr::expose_internal_to_pc()
{
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if CONFIG_TINYUSB_MSC_ENABLED
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 判断条件：!internal_mounted_ 是否成立。
    if (!internal_mounted_)
    {
        // 定义只读对象或变量：初始化后不能在当前位置修改。
        const esp_err_t err = mount_internal_locked();
        // 判断条件：err != ESP_OK 是否成立。
        if (err != ESP_OK)
        {
            // 返回 err 给调用者，表示当前函数的执行结果。
            return err;
        }
    }
    // 返回 usb_msc_expose_to_host() 给调用者，表示当前函数的执行结果。
    return usb_msc_expose_to_host();
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#else
    // 调用 ESP_LOGW(TAG, "expose_internal_to_pc: TinyUSB MSC 未启用");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGW(TAG, "expose_internal_to_pc: TinyUSB MSC 未启用");
    // 返回 ESP_ERR_NOT_SUPPORTED 给调用者，表示当前函数的执行结果。
    return ESP_ERR_NOT_SUPPORTED;
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
}

// 定义或声明函数 StorageMgr::reclaim_internal_from_pc：参数在括号里，返回类型写在函数名前面。
esp_err_t StorageMgr::reclaim_internal_from_pc()
{
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if CONFIG_TINYUSB_MSC_ENABLED
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 判断条件：!usb_msc_is_initialized() 是否成立。
    if (!usb_msc_is_initialized())
    {
        // 返回 ESP_ERR_INVALID_STATE 给调用者，表示当前函数的执行结果。
        return ESP_ERR_INVALID_STATE;
    }
    // 返回 usb_msc_reclaim_for_app() 给调用者，表示当前函数的执行结果。
    return usb_msc_reclaim_for_app();
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#else
    // 返回 ESP_ERR_NOT_SUPPORTED 给调用者，表示当前函数的执行结果。
    return ESP_ERR_NOT_SUPPORTED;
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
}

// 定义或声明函数 StorageMgr::is_internal_exposed_to_pc：参数在括号里，返回类型写在函数名前面。
bool StorageMgr::is_internal_exposed_to_pc() const
{
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if CONFIG_TINYUSB_MSC_ENABLED
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 返回 usb_msc_is_exposed_to_host() 给调用者，表示当前函数的执行结果。
    return usb_msc_is_exposed_to_host();
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#else
    // 返回 false 给调用者，表示当前函数的执行结果。
    return false;
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
}

// 定义或声明函数 StorageMgr::umount_usb：参数在括号里，返回类型写在函数名前面。
esp_err_t StorageMgr::umount_usb()
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 返回 umount_usb_locked() 给调用者，表示当前函数的执行结果。
    return umount_usb_locked();
}

// 定义或声明函数 StorageMgr::service_internal_recovery：参数在括号里，返回类型写在函数名前面。
void StorageMgr::service_internal_recovery()
{
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if CONFIG_TINYUSB_MSC_ENABLED
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 判断条件：!internal_mounted_ 是否成立。
    if (!internal_mounted_)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 调用 usb_msc_service_recovery()：周期调用的恢复逻辑： 如果 /int 已经可访问，清空失败计数。 如果当前暴露给电脑，但 USB 总线挂起或断开，连续 3 次判定已经拔线。 如果当前不是暴露状态但 /int 又不可访问，也尝试收回。 挂回最多重试 12 次，避免无限刷日志。。
    usb_msc_service_recovery();
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
}

// 函数定义/声明：调用 StorageMgr::service_sd_hotplug()：这个函数被 Housekeeper 周期性调用。它先做时间节流，默认每 500 ms 最多执行一次，避免在 UI 循环里频繁访问 SD 总线。。
void StorageMgr::service_sd_hotplug()
{
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if !SOC_SDMMC_HOST_SUPPORTED || !CONFIG_EBOOK_SD_SDMMC_ENABLE
    // 提前返回：结束当前函数，不返回具体值。
    return;
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#else
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint32_t now_ms = static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);
    // 静态值 s_last_ms：只属于当前模块或类，并保持固定含义。
    static uint32_t s_last_ms = 0;
    // 固定参数 interval = CONFIG_EBOOK_SD_HOTPLUG_SERVICE_MS：由当前模块定义，用于控制布局、时序或硬件行为。
    constexpr uint32_t interval = CONFIG_EBOOK_SD_HOTPLUG_SERVICE_MS;
    // 判断条件：(now_ms - s_last_ms) < interval 是否成立。
    if ((now_ms - s_last_ms) < interval)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 赋值：把右侧 now_ms 的结果保存到左侧 s_last_ms。
    s_last_ms = now_ms;

    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);

    // 判断条件：has_hardware_cd() 是否成立。
    if (has_hardware_cd())
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const bool inserted = sd_cd_card_present();
        // 判断条件：!inserted && sd_mounted_ 是否成立。
        if (!inserted && sd_mounted_)
        {
            // 调用 umount_sd_locked()：只有卡已挂载且指针存在时才调用 espvfsfatsdcardunmount()。成功后清空 sdcard 和状态。。
            (void)umount_sd_locked();
            // 提前返回：结束当前函数，不返回具体值。
            return;
        }
        // 判断条件：inserted && !sd_mounted_ 是否成立。
        if (inserted && !sd_mounted_)
        {
            // 调用 mount_sd_locked(false)：硬件型号决定配置。
            (void)mount_sd_locked(false);
        }
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 判断条件：!sd_mounted_ 是否成立。
        if (!sd_mounted_)
        {
            // 调用 mount_sd_locked(false)：硬件型号决定配置。
            (void)mount_sd_locked(false);
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 赋值：把右侧 static_cast<sdmmc_card_t*>(sd_card_) 的结果保存到左侧 auto* card。
            auto* card = static_cast<sdmmc_card_t*>(sd_card_);
            // 判断条件：card == nullptr 是否成立。
            if (card == nullptr)
            {
                // 赋值：把右侧 false 的结果保存到左侧 sd_mounted_。
                sd_mounted_ = false;
                // 提前返回：结束当前函数，不返回具体值。
                return;
            }
            // 判断条件：sdmmc_get_status(card) != ESP_OK 是否成立。
            if (sdmmc_get_status(card) != ESP_OK)
            {
                // 调用 ESP_LOGW(TAG, "TF 软件检测: 卡无响应, 卸载 /sd");，执行这个函数并处理它的返回值或状态变化。
                ESP_LOGW(TAG, "TF 软件检测: 卡无响应, 卸载 /sd");
                // 调用 umount_sd_locked()：只有卡已挂载且指针存在时才调用 espvfsfatsdcardunmount()。成功后清空 sdcard 和状态。。
                (void)umount_sd_locked();
            }
        }
    }
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
}

// 函数定义/声明：调用 StorageMgr::is_mounted()：内部存储要同时看： text internalmounted && usbmscisavailabletoapp()。
bool StorageMgr::is_mounted(MountKind kind) const
{
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 多分支判断：根据括号里的值选择不同 case。
    switch (kind)
    {
        // 分支匹配：case MountKind::Internal 成立时执行下面的代码。
        case MountKind::Internal:
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if CONFIG_TINYUSB_MSC_ENABLED
            // 返回 internal_mounted_ && usb_msc_is_available_to_app() 给调用者，表示当前函数的执行结果。
            return internal_mounted_ && usb_msc_is_available_to_app();
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#else
            // 返回 internal_mounted_ 给调用者，表示当前函数的执行结果。
            return internal_mounted_;
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
        // 分支匹配：case MountKind::Sd 成立时执行下面的代码。
        case MountKind::Sd:
            // 返回 sd_mounted_ 给调用者，表示当前函数的执行结果。
            return sd_mounted_;
        // 分支匹配：case MountKind::Usb 成立时执行下面的代码。
        case MountKind::Usb:
            // 返回 usb_mounted_ 给调用者，表示当前函数的执行结果。
            return usb_mounted_;
        // 分支匹配：case MountKind::Assets 成立时执行下面的代码。
        case MountKind::Assets:
            // 返回 assets_mounted_ 给调用者，表示当前函数的执行结果。
            return assets_mounted_;
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
    }
}

// 定义或声明函数 StorageMgr::query_assets_usage：参数在括号里，返回类型写在函数名前面。
esp_err_t StorageMgr::query_assets_usage(size_t* total_out, size_t* used_out) const
{
    // 判断条件：total_out == nullptr || used_out == nullptr 是否成立。
    if (total_out == nullptr || used_out == nullptr)
    {
        // 返回 ESP_ERR_INVALID_ARG 给调用者，表示当前函数的执行结果。
        return ESP_ERR_INVALID_ARG;
    }
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 判断条件：!assets_mounted_ 是否成立。
    if (!assets_mounted_)
    {
        // 返回 ESP_ERR_INVALID_STATE 给调用者，表示当前函数的执行结果。
        return ESP_ERR_INVALID_STATE;
    }
    // 返回 esp_littlefs_info(k_label_assets, total_out, used_out) 给调用者，表示当前函数的执行结果。
    return esp_littlefs_info(k_label_assets, total_out, used_out);
}

// 定义或声明函数 StorageMgr::query_internal_usage：参数在括号里，返回类型写在函数名前面。
esp_err_t StorageMgr::query_internal_usage(size_t* total_out, size_t* used_out) const
{
    // 判断条件：total_out == nullptr || used_out == nullptr 是否成立。
    if (total_out == nullptr || used_out == nullptr)
    {
        // 返回 ESP_ERR_INVALID_ARG 给调用者，表示当前函数的执行结果。
        return ESP_ERR_INVALID_ARG;
    }
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 判断条件：!internal_mounted_ 是否成立。
    if (!internal_mounted_)
    {
        // 返回 ESP_ERR_INVALID_STATE 给调用者，表示当前函数的执行结果。
        return ESP_ERR_INVALID_STATE;
    }
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if CONFIG_TINYUSB_MSC_ENABLED
    // 判断条件：!usb_msc_is_available_to_app() 是否成立。
    if (!usb_msc_is_available_to_app())
    {
        // 返回 ESP_ERR_INVALID_STATE 给调用者，表示当前函数的执行结果。
        return ESP_ERR_INVALID_STATE;
    }
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
    // 定义变量 uint64_t total_b 并初始化为 0。
    uint64_t total_b = 0;
    // 定义变量 uint64_t free_b 并初始化为 0。
    uint64_t free_b = 0;
    // 定义变量 esp_err_t err 并初始化为 esp_vfs_fat_info(k_path_internal, &total_b, &free_b)。
    esp_err_t err = esp_vfs_fat_info(k_path_internal, &total_b, &free_b);
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 返回 err 给调用者，表示当前函数的执行结果。
        return err;
    }
    *total_out = static_cast<size_t>(total_b);
    *used_out = static_cast<size_t>((total_b >= free_b) ? (total_b - free_b) : 0);
    // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
    return ESP_OK;
}

// 定义或声明函数 StorageMgr::query_sd_usage：参数在括号里，返回类型写在函数名前面。
esp_err_t StorageMgr::query_sd_usage(uint64_t* total_bytes_out, uint64_t* used_bytes_out) const
{
    // 判断条件：total_bytes_out == nullptr || used_bytes_out == nullptr 是否成立。
    if (total_bytes_out == nullptr || used_bytes_out == nullptr)
    {
        // 返回 ESP_ERR_INVALID_ARG 给调用者，表示当前函数的执行结果。
        return ESP_ERR_INVALID_ARG;
    }
    // 调用 lock(mutex_)：为什么使用 trylock() 而不是普通 lock()。
    std::lock_guard<std::mutex> lock(mutex_);
    // 判断条件：!sd_mounted_ 是否成立。
    if (!sd_mounted_)
    {
        // 返回 ESP_ERR_INVALID_STATE 给调用者，表示当前函数的执行结果。
        return ESP_ERR_INVALID_STATE;
    }

    // 定义变量 uint64_t free_b 并初始化为 0。
    uint64_t free_b = 0;
    // 定义变量 esp_err_t err 并初始化为 esp_vfs_fat_info(k_path_sd, total_bytes_out, &free_b)。
    esp_err_t err = esp_vfs_fat_info(k_path_sd, total_bytes_out, &free_b);
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 返回 err 给调用者，表示当前函数的执行结果。
        return err;
    }

    *used_bytes_out = (*total_bytes_out >= free_b) ? (*total_bytes_out - free_b) : 0;
    // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
    return ESP_OK;
}

// 函数定义/声明：调用 StorageMgr::mount_internal_locked()：两种路径。
esp_err_t StorageMgr::mount_internal_locked()
{
    // 判断条件：internal_mounted_ 是否成立。
    if (internal_mounted_)
    {
        // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
        return ESP_OK;
    }

// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if CONFIG_TINYUSB_MSC_ENABLED
    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const esp_err_t err = usb_msc_init_internal();
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 返回 err 给调用者，表示当前函数的执行结果。
        return err;
    }

    // 定义变量 uint64_t total_b 并初始化为 0。
    uint64_t total_b = 0;
    // 定义变量 uint64_t free_b 并初始化为 0。
    uint64_t free_b = 0;
    // 判断条件：esp_vfs_fat_info(k_path_internal, &total_b, &free_b) == ESP_OK 是否成立。
    if (esp_vfs_fat_info(k_path_internal, &total_b, &free_b) == ESP_OK)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint64_t used_b = (total_b >= free_b) ? (total_b - free_b) : 0;
        // 调用 ESP_LOGD(TAG, "userdata FAT (MSC): %" PRIu64 " 总计, %" PRIu64 " 已用, %" PRIu64 " 空闲",，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGD(TAG, "userdata FAT (MSC): %" PRIu64 " 总计, %" PRIu64 " 已用, %" PRIu64 " 空闲",
                 // 继续上一行的表达式或参数列表：total_b, used_b, free_b);。
                 total_b, used_b, free_b);
    }

    // 赋值：把右侧 true 的结果保存到左侧 internal_mounted_。
    internal_mounted_ = true;
    // 赋值：把右侧 WL_INVALID_HANDLE 的结果保存到左侧 wl_handle_。
    wl_handle_ = WL_INVALID_HANDLE;
    // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
    return ESP_OK;
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#else
    // 定义变量 esp_vfs_fat_mount_config_t mount_cfg 并初始化为 VFS_FAT_MOUNT_DEFAULT_CONFIG()。
    esp_vfs_fat_mount_config_t mount_cfg = VFS_FAT_MOUNT_DEFAULT_CONFIG();
    // 赋值：把右侧 CONFIG_EBOOK_STORAGE_FORMAT_IF_MOUNT_FAILED != 0 的结果保存到左侧 mount_cfg.format_if_mount_failed。
    mount_cfg.format_if_mount_failed = CONFIG_EBOOK_STORAGE_FORMAT_IF_MOUNT_FAILED != 0;
    // 赋值：把右侧 CONFIG_EBOOK_INTERNAL_MAX_FILES 的结果保存到左侧 mount_cfg.max_files。
    mount_cfg.max_files = CONFIG_EBOOK_INTERNAL_MAX_FILES;
    // 赋值：把右侧 CONFIG_WL_SECTOR_SIZE 的结果保存到左侧 mount_cfg.allocation_unit_size。
    mount_cfg.allocation_unit_size = CONFIG_WL_SECTOR_SIZE;

    // 定义变量 wl_handle_t wl 并初始化为 WL_INVALID_HANDLE。
    wl_handle_t wl = WL_INVALID_HANDLE;
    // 定义变量：esp_err_t err =
    esp_err_t err =
        // 继续上一行的表达式或参数列表：esp_vfs_fat_spiflash_mount_rw_wl(k_path_internal, k_label_userdata, &mount_cfg, &wl);。
        esp_vfs_fat_spiflash_mount_rw_wl(k_path_internal, k_label_userdata, &mount_cfg, &wl);
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "userdata FAT 挂载失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "userdata FAT 挂载失败: %s", esp_err_to_name(err));
        // 返回 err 给调用者，表示当前函数的执行结果。
        return err;
    }

    // 赋值：把右侧 wl 的结果保存到左侧 wl_handle_。
    wl_handle_ = wl;

    // 定义变量 uint64_t total_b 并初始化为 0。
    uint64_t total_b = 0;
    // 定义变量 uint64_t free_b 并初始化为 0。
    uint64_t free_b = 0;
    // 判断条件：esp_vfs_fat_info(k_path_internal, &total_b, &free_b) == ESP_OK 是否成立。
    if (esp_vfs_fat_info(k_path_internal, &total_b, &free_b) == ESP_OK)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint64_t used_b = (total_b >= free_b) ? (total_b - free_b) : 0;
        // 调用 ESP_LOGD(TAG, "userdata FAT: %" PRIu64 " 总计, %" PRIu64 " 已用, %" PRIu64 " 空闲", total_b,，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGD(TAG, "userdata FAT: %" PRIu64 " 总计, %" PRIu64 " 已用, %" PRIu64 " 空闲", total_b,
                 // 继续上一行的表达式或参数列表：used_b, free_b);。
                 used_b, free_b);
    }

    // 赋值：把右侧 true 的结果保存到左侧 internal_mounted_。
    internal_mounted_ = true;
    // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
    return ESP_OK;
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
}

// 函数定义/声明：调用 StorageMgr::umount_internal_locked()：如果启用了 USB MSC，先检查是否暴露给电脑；如果是，先收回给应用，再 shutdown USB MSC。随后清空句柄和状态。。
esp_err_t StorageMgr::umount_internal_locked()
{
    // 判断条件：!internal_mounted_ 是否成立。
    if (!internal_mounted_)
    {
        // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
        return ESP_OK;
    }

// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if CONFIG_TINYUSB_MSC_ENABLED
    // 判断条件：usb_msc_is_exposed_to_host() 是否成立。
    if (usb_msc_is_exposed_to_host())
    {
        // 函数调用：(void)usb_msc_reclaim_for_app();；执行该函数并处理它的返回值或副作用。
        (void)usb_msc_reclaim_for_app();
    }
    // 调用 usb_msc_shutdown();，执行这个函数并处理它的返回值或状态变化。
    usb_msc_shutdown();
    // 赋值：把右侧 WL_INVALID_HANDLE 的结果保存到左侧 wl_handle_。
    wl_handle_ = WL_INVALID_HANDLE;
    // 赋值：把右侧 false 的结果保存到左侧 internal_mounted_。
    internal_mounted_ = false;
    // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
    return ESP_OK;
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#else
    // 定义变量 esp_err_t err 并初始化为 esp_vfs_fat_spiflash_unmount_rw_wl(k_path_internal, wl_handle_)。
    esp_err_t err = esp_vfs_fat_spiflash_unmount_rw_wl(k_path_internal, wl_handle_);
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "userdata FAT 卸载失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "userdata FAT 卸载失败: %s", esp_err_to_name(err));
        // 返回 err 给调用者，表示当前函数的执行结果。
        return err;
    }

    // 赋值：把右侧 WL_INVALID_HANDLE 的结果保存到左侧 wl_handle_。
    wl_handle_ = WL_INVALID_HANDLE;
    // 赋值：把右侧 false 的结果保存到左侧 internal_mounted_。
    internal_mounted_ = false;
    // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
    return ESP_OK;
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
}

// 函数定义/声明：调用 StorageMgr::mount_assets_locked()：使用 espvfslittlefsregister()： base path /assets partition label assets formatifmountfailed = 0 readonly = 1。
esp_err_t StorageMgr::mount_assets_locked()
{
    // 判断条件：assets_mounted_ 是否成立。
    if (assets_mounted_)
    {
        // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
        return ESP_OK;
    }

    // 声明并默认初始化对象：esp_vfs_littlefs_conf_t conf{}。
    esp_vfs_littlefs_conf_t conf{};
    // 赋值：把右侧 k_path_assets 的结果保存到左侧 conf.base_path。
    conf.base_path = k_path_assets;
    // 赋值：把右侧 k_label_assets 的结果保存到左侧 conf.partition_label。
    conf.partition_label = k_label_assets;
    // 赋值：把右侧 CONFIG_EBOOK_STORAGE_FORMAT_IF_MOUNT_FAILED ? 1 : 0 的结果保存到左侧 conf.format_if_mount_failed。
    conf.format_if_mount_failed = CONFIG_EBOOK_STORAGE_FORMAT_IF_MOUNT_FAILED ? 1 : 0;
    // 赋值：把右侧 1 的结果保存到左侧 conf.read_only。
    conf.read_only = 1;
    // 赋值：把右侧 0 的结果保存到左侧 conf.dont_mount。
    conf.dont_mount = 0;
    // 赋值：把右侧 0 的结果保存到左侧 conf.grow_on_mount。
    conf.grow_on_mount = 0;

    // 定义变量 esp_err_t err 并初始化为 esp_vfs_littlefs_register(&conf)。
    esp_err_t err = esp_vfs_littlefs_register(&conf);
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "assets LittleFS 挂载失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "assets LittleFS 挂载失败: %s", esp_err_to_name(err));
        // 返回 err 给调用者，表示当前函数的执行结果。
        return err;
    }

    // 定义变量 size_t total 并初始化为 0。
    size_t total = 0;
    // 定义变量 size_t used 并初始化为 0。
    size_t used = 0;
    // 判断条件：esp_littlefs_info(k_label_assets, &total, &used) == ESP_OK 是否成立。
    if (esp_littlefs_info(k_label_assets, &total, &used) == ESP_OK)
    {
        // 调用 ESP_LOGD(TAG, "assets LittleFS (ro): %u 总计, %u 已用", static_cast<unsigned>(total),，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGD(TAG, "assets LittleFS (ro): %u 总计, %u 已用", static_cast<unsigned>(total),
                 // 继续上一行的表达式或参数列表：static_cast<unsigned>(used));。
                 static_cast<unsigned>(used));
    }

    // 赋值：把右侧 true 的结果保存到左侧 assets_mounted_。
    assets_mounted_ = true;
    // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
    return ESP_OK;
}

// 定义或声明函数 StorageMgr::umount_assets_locked：参数在括号里，返回类型写在函数名前面。
esp_err_t StorageMgr::umount_assets_locked()
{
    // 判断条件：!assets_mounted_ 是否成立。
    if (!assets_mounted_)
    {
        // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
        return ESP_OK;
    }

    // 定义变量 esp_err_t err 并初始化为 esp_vfs_littlefs_unregister(k_label_assets)。
    esp_err_t err = esp_vfs_littlefs_unregister(k_label_assets);
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "注销 assets LittleFS 失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "注销 assets LittleFS 失败: %s", esp_err_to_name(err));
        // 返回 err 给调用者，表示当前函数的执行结果。
        return err;
    }

    // 赋值：把右侧 false 的结果保存到左侧 assets_mounted_。
    assets_mounted_ = false;
    // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
    return ESP_OK;
}

// 函数定义/声明：调用 StorageMgr::mount_sd_locked()：硬件型号决定配置。
esp_err_t StorageMgr::mount_sd_locked(bool log_errors)
{
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if !SOC_SDMMC_HOST_SUPPORTED
    // 调用 ESP_LOGW(TAG, "SoC 不支持 SDMMC");，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGW(TAG, "SoC 不支持 SDMMC");
    // 返回 ESP_ERR_NOT_SUPPORTED 给调用者，表示当前函数的执行结果。
    return ESP_ERR_NOT_SUPPORTED;
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#else
    // 判断条件：!CONFIG_EBOOK_SD_SDMMC_ENABLE 是否成立。
    if (!CONFIG_EBOOK_SD_SDMMC_ENABLE)
    {
        // 返回 ESP_ERR_NOT_SUPPORTED 给调用者，表示当前函数的执行结果。
        return ESP_ERR_NOT_SUPPORTED;
    }
    // 判断条件：sd_mounted_ 是否成立。
    if (sd_mounted_)
    {
        // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
        return ESP_OK;
    }

    // 判断条件：!sd_pins_configured() 是否成立。
    if (!sd_pins_configured())
    {
        // 调用 ESP_LOGE(TAG, "SDMMC 基本引脚 (CLK/CMD/D0) 未配置");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "SDMMC 基本引脚 (CLK/CMD/D0) 未配置");
        // 返回 ESP_ERR_INVALID_STATE 给调用者，表示当前函数的执行结果。
        return ESP_ERR_INVALID_STATE;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool four_line = is_four_line_mode();
    // 调用 log_sd_pin_config();，执行这个函数并处理它的返回值或状态变化。
    log_sd_pin_config();

    // 调用 app::sys::task::TaskMgr::delay_ms(200)：调用 vTaskDelay()，这是让出 CPU 的阻塞延时。一千毫秒延时期间，任务不占 CPU，调度器可以运行别人。。
    app::sys::task::TaskMgr::delay_ms(200);

    // 定义变量 esp_vfs_fat_mount_config_t mount_cfg 并初始化为 VFS_FAT_MOUNT_DEFAULT_CONFIG()。
    esp_vfs_fat_mount_config_t mount_cfg = VFS_FAT_MOUNT_DEFAULT_CONFIG();
    // 赋值：把右侧 CONFIG_EBOOK_SD_FORMAT_IF_MOUNT_FAILED != 0 的结果保存到左侧 mount_cfg.format_if_mount_failed。
    mount_cfg.format_if_mount_failed = CONFIG_EBOOK_SD_FORMAT_IF_MOUNT_FAILED != 0;
    // 赋值：把右侧 CONFIG_EBOOK_SD_MAX_FILES 的结果保存到左侧 mount_cfg.max_files。
    mount_cfg.max_files = CONFIG_EBOOK_SD_MAX_FILES;
    // 赋值：把右侧 16 * 1024 的结果保存到左侧 mount_cfg.allocation_unit_size。
    mount_cfg.allocation_unit_size = 16 * 1024;

    // 定义变量 sdmmc_host_t host 并初始化为 SDMMC_HOST_DEFAULT()。
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    // 赋值：把右侧 8 的结果保存到左侧 host.unaligned_multi_block_rw_max_chunk_size。
    host.unaligned_multi_block_rw_max_chunk_size = 8;
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if CONFIG_EBOOK_SD_SPEED_HS
    // 赋值：把右侧 SDMMC_FREQ_HIGHSPEED 的结果保存到左侧 host.max_freq_khz。
    host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#else
    // 赋值：把右侧 SDMMC_FREQ_DEFAULT 的结果保存到左侧 host.max_freq_khz。
    host.max_freq_khz = SDMMC_FREQ_DEFAULT;
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif

    // 定义变量 sdmmc_slot_config_t slot 并初始化为 SDMMC_SLOT_CONFIG_DEFAULT()。
    sdmmc_slot_config_t slot = SDMMC_SLOT_CONFIG_DEFAULT();
    // 赋值：把右侧 four_line ? 4 : 1 的结果保存到左侧 slot.width。
    slot.width = four_line ? 4 : 1;

// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if SOC_SDMMC_USE_GPIO_MATRIX
    // 赋值：把右侧 config::TF_CARD_CLK 的结果保存到左侧 slot.clk。
    slot.clk = config::TF_CARD_CLK;
    // 赋值：把右侧 config::TF_CARD_CMD 的结果保存到左侧 slot.cmd。
    slot.cmd = config::TF_CARD_CMD;
    // 赋值：把右侧 config::TF_CARD_D0 的结果保存到左侧 slot.d0。
    slot.d0 = config::TF_CARD_D0;
    // 判断条件：four_line 是否成立。
    if (four_line)
    {
        // 赋值：把右侧 config::TF_CARD_D1 的结果保存到左侧 slot.d1。
        slot.d1 = config::TF_CARD_D1;
        // 赋值：把右侧 config::TF_CARD_D2 的结果保存到左侧 slot.d2。
        slot.d2 = config::TF_CARD_D2;
        // 赋值：把右侧 config::TF_CARD_D3 的结果保存到左侧 slot.d3。
        slot.d3 = config::TF_CARD_D3;
    }
    // 判断条件：has_hardware_cd() 是否成立。
    if (has_hardware_cd())
    {
        // 赋值：把右侧 config::TF_CARD_CD 的结果保存到左侧 slot.cd。
        slot.cd = config::TF_CARD_CD;
    }
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif

    // 赋值：把右侧 SDMMC_SLOT_FLAG_INTERNAL_PULLUP 的结果保存到左侧 slot.flags |。
    slot.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    // 赋值：把右侧 nullptr 的结果保存到左侧 sdmmc_card_t* card。
    sdmmc_card_t* card = nullptr;
    // 定义变量 esp_err_t err 并初始化为 esp_vfs_fat_sdmmc_mount(k_path_sd, &host, &slot, &mount_cfg, &card)。
    esp_err_t err = esp_vfs_fat_sdmmc_mount(k_path_sd, &host, &slot, &mount_cfg, &card);

    // 判断条件：err == ESP_ERR_TIMEOUT 是否成立。
    if (err == ESP_ERR_TIMEOUT)
    {
        // 调用 ESP_LOGW(TAG, "SD 首次挂载超时, 400ms 后重试");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGW(TAG, "SD 首次挂载超时, 400ms 后重试");
        // 调用 app::sys::task::TaskMgr::delay_ms(400)：调用 vTaskDelay()，这是让出 CPU 的阻塞延时。一千毫秒延时期间，任务不占 CPU，调度器可以运行别人。。
        app::sys::task::TaskMgr::delay_ms(400);
        // 赋值：把右侧 esp_vfs_fat_sdmmc_mount(k_path_sd, &host, &slot, &mount_cfg, &card) 的结果保存到左侧 err。
        err = esp_vfs_fat_sdmmc_mount(k_path_sd, &host, &slot, &mount_cfg, &card);
    }

    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 判断条件：log_errors 是否成立。
        if (log_errors)
        {
            // 调用 ESP_LOGE(TAG, "SD 挂载失败 (/sd): %s (0x%x)", esp_err_to_name(err), err);，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "SD 挂载失败 (/sd): %s (0x%x)", esp_err_to_name(err), err);
        }
        // 返回 err 给调用者，表示当前函数的执行结果。
        return err;
    }

    // 赋值：把右侧 card 的结果保存到左侧 sd_card_。
    sd_card_ = card;
    // 赋值：把右侧 true 的结果保存到左侧 sd_mounted_。
    sd_mounted_ = true;
    // 调用 ESP_LOGD(TAG, "SD 已挂载至 /sd, 卡名: %s", card->cid.name);，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "SD 已挂载至 /sd, 卡名: %s", card->cid.name);

    // 定义变量 uint64_t fat_total 并初始化为 0。
    uint64_t fat_total = 0;
    // 定义变量 uint64_t fat_free 并初始化为 0。
    uint64_t fat_free = 0;
    // 判断条件：esp_vfs_fat_info(k_path_sd, &fat_total, &fat_free) == ESP_OK 是否成立。
    if (esp_vfs_fat_info(k_path_sd, &fat_total, &fat_free) == ESP_OK)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint64_t used = (fat_total >= fat_free) ? (fat_total - fat_free) : 0;
        // 调用 ESP_LOGD(TAG, "SD FAT: %" PRIu64 " 总计, %" PRIu64 " 已用, %" PRIu64 " 空闲", fat_total,，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGD(TAG, "SD FAT: %" PRIu64 " 总计, %" PRIu64 " 已用, %" PRIu64 " 空闲", fat_total,
                 // 继续上一行的表达式或参数列表：used, fat_free);。
                 used, fat_free);
    }

    // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
    return ESP_OK;
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
}

// 函数定义/声明：调用 StorageMgr::umount_sd_locked()：只有卡已挂载且指针存在时才调用 espvfsfatsdcardunmount()。成功后清空 sdcard 和状态。。
esp_err_t StorageMgr::umount_sd_locked()
{
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if !SOC_SDMMC_HOST_SUPPORTED
    // 赋值：把右侧 false 的结果保存到左侧 sd_mounted_。
    sd_mounted_ = false;
    // 赋值：把右侧 nullptr 的结果保存到左侧 sd_card_。
    sd_card_ = nullptr;
    // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
    return ESP_OK;
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#else
    // 判断条件：!sd_mounted_ || sd_card_ == nullptr 是否成立。
    if (!sd_mounted_ || sd_card_ == nullptr)
    {
        // 赋值：把右侧 false 的结果保存到左侧 sd_mounted_。
        sd_mounted_ = false;
        // 赋值：把右侧 nullptr 的结果保存到左侧 sd_card_。
        sd_card_ = nullptr;
        // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
        return ESP_OK;
    }

    // 赋值：把右侧 static_cast<sdmmc_card_t*>(sd_card_) 的结果保存到左侧 auto* card。
    auto* card = static_cast<sdmmc_card_t*>(sd_card_);
    // 定义变量 esp_err_t err 并初始化为 esp_vfs_fat_sdcard_unmount(k_path_sd, card)。
    esp_err_t err = esp_vfs_fat_sdcard_unmount(k_path_sd, card);
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "卸载 SD (/sd) 失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "卸载 SD (/sd) 失败: %s", esp_err_to_name(err));
        // 返回 err 给调用者，表示当前函数的执行结果。
        return err;
    }

    // 赋值：把右侧 nullptr 的结果保存到左侧 sd_card_。
    sd_card_ = nullptr;
    // 赋值：把右侧 false 的结果保存到左侧 sd_mounted_。
    sd_mounted_ = false;
    // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
    return ESP_OK;
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
}

// 定义或声明函数 StorageMgr::umount_usb_locked：参数在括号里，返回类型写在函数名前面。
esp_err_t StorageMgr::umount_usb_locked()
{
    // 赋值：把右侧 false 的结果保存到左侧 usb_mounted_。
    usb_mounted_ = false;
    // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
    return ESP_OK;
}

// 定义或声明函数 mount_kind_from_path_prefix：参数在括号里，返回类型写在函数名前面。
MountKind mount_kind_from_path_prefix(const char* path)
{
    // 判断条件：path == nullptr || path[0] != '/' 是否成立。
    if (path == nullptr || path[0] != '/')
    {
        // 返回 MountKind::Invalid 给调用者，表示当前函数的执行结果。
        return MountKind::Invalid;
    }
    // 判断条件：path_has_prefix(path, k_path_assets) 是否成立。
    if (path_has_prefix(path, k_path_assets))
    {
        // 返回 MountKind::Assets 给调用者，表示当前函数的执行结果。
        return MountKind::Assets;
    }
    // 判断条件：path_has_prefix(path, k_path_internal) 是否成立。
    if (path_has_prefix(path, k_path_internal))
    {
        // 返回 MountKind::Internal 给调用者，表示当前函数的执行结果。
        return MountKind::Internal;
    }
    // 判断条件：path_has_prefix(path, k_path_sd) 是否成立。
    if (path_has_prefix(path, k_path_sd))
    {
        // 返回 MountKind::Sd 给调用者，表示当前函数的执行结果。
        return MountKind::Sd;
    }
    // 判断条件：path_has_prefix(path, k_path_usb) 是否成立。
    if (path_has_prefix(path, k_path_usb))
    {
        // 返回 MountKind::Usb 给调用者，表示当前函数的执行结果。
        return MountKind::Usb;
    }
    // 返回 MountKind::Invalid 给调用者，表示当前函数的执行结果。
    return MountKind::Invalid;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::common::storage
