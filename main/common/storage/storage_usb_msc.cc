/**
 * [初学者导读] 文件：main/common/storage/storage_usb_msc.cc
 * 作用：实现 USB MSC 对内部存储的挂载和恢复。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：挂载状态有锁和热插拔风险，慢文件操作应放后台任务。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "storage_usb_msc.hpp"

// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#if CONFIG_TINYUSB_MSC_ENABLED

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "storage.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_log.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_partition.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "esp_vfs_fat.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "freertos/FreeRTOS.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "freertos/semphr.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "sdkconfig.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "tinyusb.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "tinyusb_default_config.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "tinyusb_msc.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "tusb.h"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "wear_levelling.h"

// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#ifndef CONFIG_EBOOK_STORAGE_FORMAT_IF_MOUNT_FAILED
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#define CONFIG_EBOOK_STORAGE_FORMAT_IF_MOUNT_FAILED 0
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#ifndef CONFIG_EBOOK_INTERNAL_MAX_FILES
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#define CONFIG_EBOOK_INTERNAL_MAX_FILES 10
// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::common::storage {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 只读约束：这个值或对象在当前位置不能被修改。
const char* const TAG = "UsbMsc";

// 固定参数 kMountWaitMs = 8000：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint32_t kMountWaitMs = 8000;

// 执行语句：wl_handle_t s_wl{WL_INVALID_HANDLE};；它会更新一次变量、对象或程序状态。
wl_handle_t s_wl{WL_INVALID_HANDLE};
// 执行语句：tinyusb_msc_storage_handle_t s_storage{nullptr};；它会更新一次变量、对象或程序状态。
tinyusb_msc_storage_handle_t s_storage{nullptr};
// 执行语句：bool s_tinyusb_installed{false};；它会更新一次变量、对象或程序状态。
bool s_tinyusb_installed{false};

// 静态字符缓冲区 s_base_path：保存临时文本，跨函数调用复用。
static char s_base_path[] = "/int";

// 执行语句：SemaphoreHandle_t s_mount_sem{nullptr};；它会更新一次变量、对象或程序状态。
SemaphoreHandle_t s_mount_sem{nullptr};
// 易变变量：告诉编译器该值可能被中断或硬件改变，不能随意优化读取。
volatile bool s_mount_waiting{false};
// 执行语句：esp_err_t s_mount_wait_err{ESP_OK};；它会更新一次变量、对象或程序状态。
esp_err_t s_mount_wait_err{ESP_OK};

// 易变变量：告诉编译器该值可能被中断或硬件改变，不能随意优化读取。
volatile bool s_host_was_active{false};
// 执行语句：uint8_t s_idle_streak{0};；它会更新一次变量、对象或程序状态。
uint8_t s_idle_streak{0};
// 执行语句：uint8_t s_recovery_fail_streak{0};；它会更新一次变量、对象或程序状态。
uint8_t s_recovery_fail_streak{0};

// 固定参数 kRecoveryMaxStreak = 12：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kRecoveryMaxStreak = 12;
// S3 无 VBUS 检测: 直接拔线不会触发安全弹出, 仅表现为总线挂起
constexpr uint8_t kIdleStreakReclaim = 3;

// 定义或声明函数 internal_vfs_ok：参数在括号里，返回类型写在函数名前面。
bool internal_vfs_ok()
{
    // 定义变量 uint64_t total_b 并初始化为 0。
    uint64_t total_b = 0;
    // 定义变量 uint64_t free_b 并初始化为 0。
    uint64_t free_b = 0;
    // 返回 esp_vfs_fat_info(s_base_path, &total_b, &free_b) == ESP_OK 给调用者，表示当前函数的执行结果。
    return esp_vfs_fat_info(s_base_path, &total_b, &free_b) == ESP_OK;
}

// 定义或声明函数 finish_mount_wait：参数在括号里，返回类型写在函数名前面。
void finish_mount_wait(esp_err_t err)
{
    // 判断条件：!s_mount_waiting 是否成立。
    if (!s_mount_waiting)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 赋值：把右侧 err 的结果保存到左侧 s_mount_wait_err。
    s_mount_wait_err = err;
    // 赋值：把右侧 false 的结果保存到左侧 s_mount_waiting。
    s_mount_waiting = false;
    // 判断条件：s_mount_sem != nullptr 是否成立。
    if (s_mount_sem != nullptr)
    {
        // 函数调用：(void)xSemaphoreGive(s_mount_sem);；执行该函数并处理它的返回值或副作用。
        (void)xSemaphoreGive(s_mount_sem);
    }
}

// 定义或声明函数 on_usb_device_event：参数在括号里，返回类型写在函数名前面。
void on_usb_device_event(tinyusb_event_t* event, void* arg)
{
    // 执行语句：(void)arg;；它会更新一次变量、对象或程序状态。
    (void)arg;
    // 判断条件：event == nullptr 是否成立。
    if (event == nullptr)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：event->id == TINYUSB_EVENT_ATTACHED 是否成立。
    if (event->id == TINYUSB_EVENT_ATTACHED)
    {
        // 赋值：把右侧 true 的结果保存到左侧 s_host_was_active。
        s_host_was_active = true;
    }
    // 备用分支：前面的条件不成立时执行这里。
    else if (event->id == TINYUSB_EVENT_DETACHED)
    {
        // 调用 ESP_LOGD(TAG, "USB 主机已断开");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGD(TAG, "USB 主机已断开");
        // 赋值：把右侧 false 的结果保存到左侧 s_host_was_active。
        s_host_was_active = false;
    }
}

// 定义或声明函数 on_storage_event：参数在括号里，返回类型写在函数名前面。
void on_storage_event(tinyusb_msc_storage_handle_t handle, tinyusb_msc_event_t* event, void* arg)
{
    // 执行语句：(void)handle;；它会更新一次变量、对象或程序状态。
    (void)handle;
    // 执行语句：(void)arg;；它会更新一次变量、对象或程序状态。
    (void)arg;
    // 判断条件：event == nullptr 是否成立。
    if (event == nullptr)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 多分支判断：根据括号里的值选择不同 case。
    switch (event->id)
    {
        // 分支匹配：case TINYUSB_MSC_EVENT_MOUNT_COMPLETE 成立时执行下面的代码。
        case TINYUSB_MSC_EVENT_MOUNT_COMPLETE:
            // 调用 ESP_LOGD(TAG, "挂载完成: %s",，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGD(TAG, "挂载完成: %s",
                     // 继续上一行的表达式或参数列表：event->mount_point == TINYUSB_MSC_STORAGE_MOUNT_USB ? "USB 主机" : "应用");。
                     event->mount_point == TINYUSB_MSC_STORAGE_MOUNT_USB ? "USB 主机" : "应用");
            // 调用 finish_mount_wait(ESP_OK);，执行这个函数并处理它的返回值或状态变化。
            finish_mount_wait(ESP_OK);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case TINYUSB_MSC_EVENT_MOUNT_FAILED 成立时执行下面的代码。
        case TINYUSB_MSC_EVENT_MOUNT_FAILED:
        // 分支匹配：case TINYUSB_MSC_EVENT_FORMAT_REQUIRED 成立时执行下面的代码。
        case TINYUSB_MSC_EVENT_FORMAT_REQUIRED:
        // 分支匹配：case TINYUSB_MSC_EVENT_FORMAT_FAILED 成立时执行下面的代码。
        case TINYUSB_MSC_EVENT_FORMAT_FAILED:
            // 调用 ESP_LOGW(TAG, "挂载事件 %d", static_cast<int>(event->id));，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGW(TAG, "挂载事件 %d", static_cast<int>(event->id));
            // 调用 finish_mount_wait(ESP_FAIL);，执行这个函数并处理它的返回值或状态变化。
            finish_mount_wait(ESP_FAIL);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }
}

/**
 * 必须在 tinyusb_msc_set_storage_mount_point() 之前调用：
 * 无主机时切换是同步完成的，事件会在 set 返回前就触发，
 * 若先 set 再布防等待，事件会被丢掉导致空等超时。
 */
// 定义或声明函数 arm_mount_wait：参数在括号里，返回类型写在函数名前面。
void arm_mount_wait()
{
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (xSemaphoreTake(s_mount_sem, 0) == pdTRUE)
    {
    }
    // 赋值：把右侧 ESP_ERR_TIMEOUT 的结果保存到左侧 s_mount_wait_err。
    s_mount_wait_err = ESP_ERR_TIMEOUT;
    // 赋值：把右侧 true 的结果保存到左侧 s_mount_waiting。
    s_mount_waiting = true;
}

// 定义或声明函数 wait_mount_switch：参数在括号里，返回类型写在函数名前面。
esp_err_t wait_mount_switch(tinyusb_msc_mount_point_t target)
{
    // 判断条件：xSemaphoreTake(s_mount_sem, pdMS_TO_TICKS(kMountWaitMs)) != pdTRUE 是否成立。
    if (xSemaphoreTake(s_mount_sem, pdMS_TO_TICKS(kMountWaitMs)) != pdTRUE)
    {
        // 赋值：把右侧 false 的结果保存到左侧 s_mount_waiting。
        s_mount_waiting = false;

        // 兜底：事件可能丢失, 以实际挂载状态为准
        tinyusb_msc_mount_point_t cur = TINYUSB_MSC_STORAGE_MOUNT_USB;
        // 判断条件：tinyusb_msc_get_storage_mount_point(s_storage, &cur) == ESP_OK && cur == target 是否成立。
        if (tinyusb_msc_get_storage_mount_point(s_storage, &cur) == ESP_OK && cur == target)
        {
            // 判断条件：target != TINYUSB_MSC_STORAGE_MOUNT_APP || internal_vfs_ok() 是否成立。
            if (target != TINYUSB_MSC_STORAGE_MOUNT_APP || internal_vfs_ok())
            {
                // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
                return ESP_OK;
            }
        }
        // 调用 ESP_LOGE(TAG, "挂载切换超时");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "挂载切换超时");
        // 返回 ESP_ERR_TIMEOUT 给调用者，表示当前函数的执行结果。
        return ESP_ERR_TIMEOUT;
    }

    // 返回 s_mount_wait_err 给调用者，表示当前函数的执行结果。
    return s_mount_wait_err;
}

// 定义或声明函数 needs_mount_reset_for_app：参数在括号里，返回类型写在函数名前面。
bool needs_mount_reset_for_app()
{
    // 判断条件：s_storage == nullptr || internal_vfs_ok() 是否成立。
    if (s_storage == nullptr || internal_vfs_ok())
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 定义变量 tinyusb_msc_mount_point_t current 并初始化为 TINYUSB_MSC_STORAGE_MOUNT_USB。
    tinyusb_msc_mount_point_t current = TINYUSB_MSC_STORAGE_MOUNT_USB;
    // 判断条件：tinyusb_msc_get_storage_mount_point(s_storage, &current) != ESP_OK 是否成立。
    if (tinyusb_msc_get_storage_mount_point(s_storage, &current) != ESP_OK)
    {
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 返回 current == TINYUSB_MSC_STORAGE_MOUNT_APP 给调用者，表示当前函数的执行结果。
    return current == TINYUSB_MSC_STORAGE_MOUNT_APP;
}

// 函数定义/声明：调用 switch_mount_point()：它处理几种情况： 目标就是应用且 /int 已经可访问，直接成功。 目标应用但 VFS 不可访问，先尝试切到 USB 复位。 当前状态已经等于目标，且符合要求，直接成功。 否则 arm 等待、set 挂载点、等待结果。 目标为应用时，最后还验证 /int 的 FAT 信息是否真的可读。。
esp_err_t switch_mount_point(tinyusb_msc_mount_point_t target)
{
    // 判断条件：s_storage == nullptr 是否成立。
    if (s_storage == nullptr)
    {
        // 返回 ESP_ERR_INVALID_STATE 给调用者，表示当前函数的执行结果。
        return ESP_ERR_INVALID_STATE;
    }

    // 判断条件：target == TINYUSB_MSC_STORAGE_MOUNT_APP && internal_vfs_ok() 是否成立。
    if (target == TINYUSB_MSC_STORAGE_MOUNT_APP && internal_vfs_ok())
    {
        // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
        return ESP_OK;
    }

    // 判断条件：target == TINYUSB_MSC_STORAGE_MOUNT_APP && needs_mount_reset_for_app() 是否成立。
    if (target == TINYUSB_MSC_STORAGE_MOUNT_APP && needs_mount_reset_for_app())
    {
        // 调用 ESP_LOGW(TAG, "挂载状态异常, 经 USB 复位后挂回 /int");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGW(TAG, "挂载状态异常, 经 USB 复位后挂回 /int");
        // 调用 switch_mount_point(TINYUSB_MSC_STORAGE_MOUNT_USB)：它处理几种情况： 目标就是应用且 /int 已经可访问，直接成功。 目标应用但 VFS 不可访问，先尝试切到 USB 复位。 当前状态已经等于目标，且符合要求，直接成功。 否则 arm 等待、set 挂载点、等待结果。 目标为应用时，最后还验证 /int 的 FAT 信息是否真的可读。。
        (void)switch_mount_point(TINYUSB_MSC_STORAGE_MOUNT_USB);
    }

    // 定义变量 tinyusb_msc_mount_point_t current 并初始化为 TINYUSB_MSC_STORAGE_MOUNT_USB。
    tinyusb_msc_mount_point_t current = TINYUSB_MSC_STORAGE_MOUNT_USB;
    // 判断条件：tinyusb_msc_get_storage_mount_point(s_storage, &current 是否成立。
    if (tinyusb_msc_get_storage_mount_point(s_storage, &current) == ESP_OK && current == target &&
        // 继续上一行的表达式或参数列表：target == TINYUSB_MSC_STORAGE_MOUNT_USB)。
        target == TINYUSB_MSC_STORAGE_MOUNT_USB)
    {
        // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
        return ESP_OK;
    }

    // 判断条件：current == target && target == TINYUSB_MSC_STORAGE_MOUNT_APP && internal_vfs_ok() 是否成立。
    if (current == target && target == TINYUSB_MSC_STORAGE_MOUNT_APP && internal_vfs_ok())
    {
        // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
        return ESP_OK;
    }

    // 判断条件：s_mount_sem == nullptr 是否成立。
    if (s_mount_sem == nullptr)
    {
        // 返回 ESP_ERR_INVALID_STATE 给调用者，表示当前函数的执行结果。
        return ESP_ERR_INVALID_STATE;
    }

    // 调用 arm_mount_wait();，执行这个函数并处理它的返回值或状态变化。
    arm_mount_wait();

    // 只读约束：这个值或对象在当前位置不能被修改。
    const esp_err_t set_err = tinyusb_msc_set_storage_mount_point(s_storage, target);
    // 判断条件：set_err != ESP_OK 是否成立。
    if (set_err != ESP_OK)
    {
        // 赋值：把右侧 false 的结果保存到左侧 s_mount_waiting。
        s_mount_waiting = false;
        // 返回 set_err 给调用者，表示当前函数的执行结果。
        return set_err;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const esp_err_t wait_err = wait_mount_switch(target);
    // 判断条件：wait_err != ESP_OK 是否成立。
    if (wait_err != ESP_OK)
    {
        // 返回 wait_err 给调用者，表示当前函数的执行结果。
        return wait_err;
    }

    // 判断条件：target == TINYUSB_MSC_STORAGE_MOUNT_APP && !internal_vfs_ok() 是否成立。
    if (target == TINYUSB_MSC_STORAGE_MOUNT_APP && !internal_vfs_ok())
    {
        // 调用 ESP_LOGE(TAG, "/int 仍不可访问");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "/int 仍不可访问");
        // 返回 ESP_FAIL 给调用者，表示当前函数的执行结果。
        return ESP_FAIL;
    }

    // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
    return ESP_OK;
}

// 定义或声明函数 mount_wl_partition：参数在括号里，返回类型写在函数名前面。
esp_err_t mount_wl_partition()
{
    // 判断条件：s_wl != WL_INVALID_HANDLE 是否成立。
    if (s_wl != WL_INVALID_HANDLE)
    {
        // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
        return ESP_OK;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const esp_partition_t* part = esp_partition_find_first(
        // 继续上一行的表达式或参数列表：ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, k_label_userdata);。
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, k_label_userdata);
    // 判断条件：part == nullptr 是否成立。
    if (part == nullptr)
    {
        // 调用 ESP_LOGE(TAG, "未找到 userdata FAT 分区");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "未找到 userdata FAT 分区");
        // 返回 ESP_ERR_NOT_FOUND 给调用者，表示当前函数的执行结果。
        return ESP_ERR_NOT_FOUND;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const esp_err_t err = wl_mount(part, &s_wl);
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "wl_mount 失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "wl_mount 失败: %s", esp_err_to_name(err));
        // 赋值：把右侧 WL_INVALID_HANDLE 的结果保存到左侧 s_wl。
        s_wl = WL_INVALID_HANDLE;
    }
    // 返回 err 给调用者，表示当前函数的执行结果。
    return err;
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// [初学者导读] usb_msc_init_internal()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
esp_err_t usb_msc_init_internal()
{
    // 判断条件：s_storage != nullptr 是否成立。
    if (s_storage != nullptr)
    {
        // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
        return ESP_OK;
    }

    // 判断条件：s_mount_sem == nullptr 是否成立。
    if (s_mount_sem == nullptr)
    {
        // 赋值：把右侧 xSemaphoreCreateBinary() 的结果保存到左侧 s_mount_sem。
        s_mount_sem = xSemaphoreCreateBinary();
        // 判断条件：s_mount_sem == nullptr 是否成立。
        if (s_mount_sem == nullptr)
        {
            // 返回 ESP_ERR_NO_MEM 给调用者，表示当前函数的执行结果。
            return ESP_ERR_NO_MEM;
        }
    }

    // 定义变量 esp_err_t err 并初始化为 mount_wl_partition()。
    esp_err_t err = mount_wl_partition();
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 返回 err 给调用者，表示当前函数的执行结果。
        return err;
    }

    // 声明并默认初始化对象：tinyusb_msc_storage_config_t cfg{}。
    tinyusb_msc_storage_config_t cfg{};
    // 赋值：把右侧 s_wl 的结果保存到左侧 cfg.medium.wl_handle。
    cfg.medium.wl_handle = s_wl;
    // 赋值：把右侧 TINYUSB_MSC_STORAGE_MOUNT_APP 的结果保存到左侧 cfg.mount_point。
    cfg.mount_point = TINYUSB_MSC_STORAGE_MOUNT_APP;
    // 赋值：把右侧 s_base_path 的结果保存到左侧 cfg.fat_fs.base_path。
    cfg.fat_fs.base_path = s_base_path;
    // 赋值：把右侧 CONFIG_EBOOK_INTERNAL_MAX_FILES 的结果保存到左侧 cfg.fat_fs.config.max_files。
    cfg.fat_fs.config.max_files = CONFIG_EBOOK_INTERNAL_MAX_FILES;
    // 赋值：把右侧 CONFIG_EBOOK_STORAGE_FORMAT_IF_MOUNT_FAILED != 0 的结果保存到左侧 cfg.fat_fs.config.format_if_mount_failed。
    cfg.fat_fs.config.format_if_mount_failed = CONFIG_EBOOK_STORAGE_FORMAT_IF_MOUNT_FAILED != 0;
    // 赋值：把右侧 CONFIG_WL_SECTOR_SIZE 的结果保存到左侧 cfg.fat_fs.config.allocation_unit_size。
    cfg.fat_fs.config.allocation_unit_size = CONFIG_WL_SECTOR_SIZE;

    // 赋值：把右侧 tinyusb_msc_new_storage_spiflash(&cfg, &s_storage) 的结果保存到左侧 err。
    err = tinyusb_msc_new_storage_spiflash(&cfg, &s_storage);
    // 判断条件：err != ESP_OK 是否成立。
    if (err != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "tinyusb_msc_new_storage_spiflash 失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "tinyusb_msc_new_storage_spiflash 失败: %s", esp_err_to_name(err));
        // 函数调用：(void)wl_unmount(s_wl);；执行该函数并处理它的返回值或副作用。
        (void)wl_unmount(s_wl);
        // 赋值：把右侧 WL_INVALID_HANDLE 的结果保存到左侧 s_wl。
        s_wl = WL_INVALID_HANDLE;
        // 赋值：把右侧 nullptr 的结果保存到左侧 s_storage。
        s_storage = nullptr;
        // 返回 err 给调用者，表示当前函数的执行结果。
        return err;
    }

    // 函数调用：(void)tinyusb_msc_set_storage_callback(on_storage_event, nullptr);；执行该函数并处理它的返回值或副作用。
    (void)tinyusb_msc_set_storage_callback(on_storage_event, nullptr);

    // 判断条件：!s_tinyusb_installed 是否成立。
    if (!s_tinyusb_installed)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const tinyusb_config_t tusb_cfg = TINYUSB_DEFAULT_CONFIG(on_usb_device_event);
        // 赋值：把右侧 tinyusb_driver_install(&tusb_cfg) 的结果保存到左侧 err。
        err = tinyusb_driver_install(&tusb_cfg);
        // 判断条件：err != ESP_OK 是否成立。
        if (err != ESP_OK)
        {
            // 调用 ESP_LOGE(TAG, "tinyusb_driver_install 失败: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "tinyusb_driver_install 失败: %s", esp_err_to_name(err));
            // 函数调用：(void)tinyusb_msc_delete_storage(s_storage);；执行该函数并处理它的返回值或副作用。
            (void)tinyusb_msc_delete_storage(s_storage);
            // 赋值：把右侧 nullptr 的结果保存到左侧 s_storage。
            s_storage = nullptr;
            // 函数调用：(void)wl_unmount(s_wl);；执行该函数并处理它的返回值或副作用。
            (void)wl_unmount(s_wl);
            // 赋值：把右侧 WL_INVALID_HANDLE 的结果保存到左侧 s_wl。
            s_wl = WL_INVALID_HANDLE;
            // 返回 err 给调用者，表示当前函数的执行结果。
            return err;
        }
        // 赋值：把右侧 true 的结果保存到左侧 s_tinyusb_installed。
        s_tinyusb_installed = true;
    }

    // 调用 ESP_LOGD(TAG, "userdata MSC 已就绪, 挂载 %s", s_base_path);，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGD(TAG, "userdata MSC 已就绪, 挂载 %s", s_base_path);
    // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
    return ESP_OK;
}

// 定义或声明函数 usb_msc_expose_to_host：参数在括号里，返回类型写在函数名前面。
esp_err_t usb_msc_expose_to_host()
{
    // 返回结果：调用 switch_mount_point(TINYUSB_MSC_STORAGE_MOUNT_USB)：它处理几种情况： 目标就是应用且 /int 已经可访问，直接成功。 目标应用但 VFS 不可访问，先尝试切到 USB 复位。 当前状态已经等于目标，且符合要求，直接成功。 否则 arm 等待、set 挂载点、等待结果。 目标为应用时，最后还验证 /int 的 FAT 信息是否真的可读。。
    return switch_mount_point(TINYUSB_MSC_STORAGE_MOUNT_USB);
}

// 定义或声明函数 usb_msc_reclaim_for_app：参数在括号里，返回类型写在函数名前面。
esp_err_t usb_msc_reclaim_for_app()
{
    // 定义变量并调用函数初始化：调用 switch_mount_point(TINYUSB_MSC_STORAGE_MOUNT_APP)：它处理几种情况： 目标就是应用且 /int 已经可访问，直接成功。 目标应用但 VFS 不可访问，先尝试切到 USB 复位。 当前状态已经等于目标，且符合要求，直接成功。 否则 arm 等待、set 挂载点、等待结果。 目标为应用时，最后还验证 /int 的 FAT 信息是否真的可读。，结果保存到 esp_err_t err。
    esp_err_t err = switch_mount_point(TINYUSB_MSC_STORAGE_MOUNT_APP);
    // 判断条件：err == ESP_OK 是否成立。
    if (err == ESP_OK)
    {
        // 返回 ESP_OK 给调用者，表示当前函数的执行结果。
        return ESP_OK;
    }

    // 调用 ESP_LOGW(TAG, "挂回失败, 尝试强制复位: %s", esp_err_to_name(err));，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGW(TAG, "挂回失败, 尝试强制复位: %s", esp_err_to_name(err));
    // 调用 switch_mount_point(TINYUSB_MSC_STORAGE_MOUNT_USB)：它处理几种情况： 目标就是应用且 /int 已经可访问，直接成功。 目标应用但 VFS 不可访问，先尝试切到 USB 复位。 当前状态已经等于目标，且符合要求，直接成功。 否则 arm 等待、set 挂载点、等待结果。 目标为应用时，最后还验证 /int 的 FAT 信息是否真的可读。。
    (void)switch_mount_point(TINYUSB_MSC_STORAGE_MOUNT_USB);
    // 返回结果：调用 switch_mount_point(TINYUSB_MSC_STORAGE_MOUNT_APP)：它处理几种情况： 目标就是应用且 /int 已经可访问，直接成功。 目标应用但 VFS 不可访问，先尝试切到 USB 复位。 当前状态已经等于目标，且符合要求，直接成功。 否则 arm 等待、set 挂载点、等待结果。 目标为应用时，最后还验证 /int 的 FAT 信息是否真的可读。。
    return switch_mount_point(TINYUSB_MSC_STORAGE_MOUNT_APP);
}

// [初学者导读] usb_msc_service_recovery()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
void usb_msc_service_recovery()
{
    // 判断条件：s_storage == nullptr 是否成立。
    if (s_storage == nullptr)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 判断条件：internal_vfs_ok() 是否成立。
    if (internal_vfs_ok())
    {
        // 赋值：把右侧 0 的结果保存到左侧 s_idle_streak。
        s_idle_streak = 0;
        // 赋值：把右侧 0 的结果保存到左侧 s_recovery_fail_streak。
        s_recovery_fail_streak = 0;
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 定义变量 bool need_reclaim 并初始化为 false。
    bool need_reclaim = false;

    // 判断条件：usb_msc_is_exposed_to_host() 是否成立。
    if (usb_msc_is_exposed_to_host())
    {
        // 判断条件：tud_mounted() 是否成立。
        if (tud_mounted())
        {
            // 赋值：把右侧 true 的结果保存到左侧 s_host_was_active。
            s_host_was_active = true;
        }

        // 判断条件：s_host_was_active && (tud_suspended() || !tud_mounted()) 是否成立。
        if (s_host_was_active && (tud_suspended() || !tud_mounted()))
        {
            // 判断条件：++s_idle_streak >= kIdleStreakReclaim 是否成立。
            if (++s_idle_streak >= kIdleStreakReclaim)
            {
                // 调用 ESP_LOGD(TAG, "USB 总线挂起, 判定已拔线, 自动挂回 /int");，执行这个函数并处理它的返回值或状态变化。
                ESP_LOGD(TAG, "USB 总线挂起, 判定已拔线, 自动挂回 /int");
                // 赋值：把右侧 false 的结果保存到左侧 s_host_was_active。
                s_host_was_active = false;
                // 赋值：把右侧 0 的结果保存到左侧 s_idle_streak。
                s_idle_streak = 0;
                // 赋值：把右侧 true 的结果保存到左侧 need_reclaim。
                need_reclaim = true;
            }
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 赋值：把右侧 0 的结果保存到左侧 s_idle_streak。
            s_idle_streak = 0;
        }
    }
    // 备用分支：前面的条件不成立时执行这里。
    else if (!internal_vfs_ok())
    {
        // 赋值：把右侧 true 的结果保存到左侧 need_reclaim。
        need_reclaim = true;
    }

    // 判断条件：!need_reclaim || s_recovery_fail_streak >= kRecoveryMaxStreak 是否成立。
    if (!need_reclaim || s_recovery_fail_streak >= kRecoveryMaxStreak)
    {
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const esp_err_t err = usb_msc_reclaim_for_app();
    // 判断条件：err == ESP_OK 是否成立。
    if (err == ESP_OK)
    {
        // 调用 ESP_LOGD(TAG, "userdata 已恢复本机访问");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGD(TAG, "userdata 已恢复本机访问");
        // 赋值：把右侧 0 的结果保存到左侧 s_recovery_fail_streak。
        s_recovery_fail_streak = 0;
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 执行语句：++s_recovery_fail_streak;；它会更新一次变量、对象或程序状态。
    ++s_recovery_fail_streak;
    // 调用 ESP_LOGW(TAG, "userdata 挂回重试 %u/%u: %s", static_cast<unsigned>(s_recovery_fail_streak),，执行这个函数并处理它的返回值或状态变化。
    ESP_LOGW(TAG, "userdata 挂回重试 %u/%u: %s", static_cast<unsigned>(s_recovery_fail_streak),
             // 继续上一行的表达式或参数列表：static_cast<unsigned>(kRecoveryMaxStreak), esp_err_to_name(err));。
             static_cast<unsigned>(kRecoveryMaxStreak), esp_err_to_name(err));
}

// 定义或声明函数 usb_msc_is_exposed_to_host：参数在括号里，返回类型写在函数名前面。
bool usb_msc_is_exposed_to_host()
{
    // 判断条件：s_storage == nullptr 是否成立。
    if (s_storage == nullptr)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 定义变量 tinyusb_msc_mount_point_t mp 并初始化为 TINYUSB_MSC_STORAGE_MOUNT_APP。
    tinyusb_msc_mount_point_t mp = TINYUSB_MSC_STORAGE_MOUNT_APP;
    // 判断条件：tinyusb_msc_get_storage_mount_point(s_storage, &mp) != ESP_OK 是否成立。
    if (tinyusb_msc_get_storage_mount_point(s_storage, &mp) != ESP_OK)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 返回 mp == TINYUSB_MSC_STORAGE_MOUNT_USB 给调用者，表示当前函数的执行结果。
    return mp == TINYUSB_MSC_STORAGE_MOUNT_USB;
}

// 定义或声明函数 usb_msc_is_available_to_app：参数在括号里，返回类型写在函数名前面。
bool usb_msc_is_available_to_app()
{
    // 判断条件：s_storage == nullptr 是否成立。
    if (s_storage == nullptr)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 判断条件：usb_msc_is_exposed_to_host() 是否成立。
    if (usb_msc_is_exposed_to_host())
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 返回 internal_vfs_ok() 给调用者，表示当前函数的执行结果。
    return internal_vfs_ok();
}

// 定义或声明函数 usb_msc_is_initialized：参数在括号里，返回类型写在函数名前面。
bool usb_msc_is_initialized()
{
    // 返回 s_storage != nullptr 给调用者，表示当前函数的执行结果。
    return s_storage != nullptr;
}

// 定义或声明函数 usb_msc_shutdown：参数在括号里，返回类型写在函数名前面。
void usb_msc_shutdown()
{
    // 赋值：把右侧 false 的结果保存到左侧 s_mount_waiting。
    s_mount_waiting = false;
    // 赋值：把右侧 false 的结果保存到左侧 s_host_was_active。
    s_host_was_active = false;
    // 赋值：把右侧 0 的结果保存到左侧 s_idle_streak。
    s_idle_streak = 0;
    // 赋值：把右侧 0 的结果保存到左侧 s_recovery_fail_streak。
    s_recovery_fail_streak = 0;

    // 判断条件：s_storage != nullptr 是否成立。
    if (s_storage != nullptr)
    {
        // 函数调用：(void)tinyusb_msc_delete_storage(s_storage);；执行该函数并处理它的返回值或副作用。
        (void)tinyusb_msc_delete_storage(s_storage);
        // 赋值：把右侧 nullptr 的结果保存到左侧 s_storage。
        s_storage = nullptr;
    }

    // 判断条件：s_tinyusb_installed 是否成立。
    if (s_tinyusb_installed)
    {
        // 函数调用：(void)tinyusb_driver_uninstall();；执行该函数并处理它的返回值或副作用。
        (void)tinyusb_driver_uninstall();
        // 赋值：把右侧 false 的结果保存到左侧 s_tinyusb_installed。
        s_tinyusb_installed = false;
    }

    // 判断条件：s_wl != WL_INVALID_HANDLE 是否成立。
    if (s_wl != WL_INVALID_HANDLE)
    {
        // 函数调用：(void)wl_unmount(s_wl);；执行该函数并处理它的返回值或副作用。
        (void)wl_unmount(s_wl);
        // 赋值：把右侧 WL_INVALID_HANDLE 的结果保存到左侧 s_wl。
        s_wl = WL_INVALID_HANDLE;
    }
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::common::storage

// 预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。
#endif
