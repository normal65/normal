// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/protocol/ntp/ntp.hpp
 * 作用：初始化 NTP、同步网络时间并广播同步状态。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：网络请求可能阻塞，必须在后台任务或专门任务中执行。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <functional>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <mutex>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <string>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <vector>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::protocol::ntp {

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class SyncStatus
{
    // 执行这条代码：RESET,。它会更新变量、对象状态或调用链。
    RESET,
    // 继续上一行的表达式或参数列表：IN_PROGRESS,。
    IN_PROGRESS,
    // 继续上一行的表达式或参数列表：COMPLETED,。
    COMPLETED,
    // 继续上一行的表达式或参数列表：FAILED,。
    FAILED,
};

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class SyncMode
{
    // 执行这条代码：IMMEDIATE,。它会更新变量、对象状态或调用链。
    IMMEDIATE,
    // 继续上一行的表达式或参数列表：SMOOTH,。
    SMOOTH,
};

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using SyncCallback = std::function<void(SyncStatus status)>;

/** @brief SNTP 时间同步（最多 3 个服务器） */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class NtpMgr
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static NtpMgr& get_instance();

    // 函数调用：bool init();；执行该函数并处理它的返回值或副作用。
    bool init();
    // 调用 deinit()：创建和销毁队列。
    void deinit();

    // 函数调用：bool configure(const std::vector<std::string>& servers,；执行该函数并处理它的返回值或副作用。
    bool configure(const std::vector<std::string>& servers,
                   // 继续上一行的表达式或参数列表：SyncMode sync_mode = SyncMode::IMMEDIATE);。
                   SyncMode sync_mode = SyncMode::IMMEDIATE);

    // 调用 start()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
    bool start();
    // 调用 stop()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
    void stop();

    // 函数调用：bool wait_sync(uint32_t timeout_ms);；执行该函数并处理它的返回值或副作用。
    bool wait_sync(uint32_t timeout_ms);

    // 函数调用：void set_sync_callback(SyncCallback callback);；执行该函数并处理它的返回值或副作用。
    void set_sync_callback(SyncCallback callback);

    /** 时区字符串，如 "CST-8" */
    // 函数调用：bool set_timezone(const char* tz);；执行该函数并处理它的返回值或副作用。
    bool set_timezone(const char* tz);

    // 函数调用：SyncStatus get_sync_status() const;；执行该函数并处理它的返回值或副作用。
    SyncStatus get_sync_status() const;
    // 函数调用：bool is_init() const;；执行该函数并处理它的返回值或副作用。
    bool is_init() const;
    // 函数调用：bool is_started() const;；执行该函数并处理它的返回值或副作用。
    bool is_started() const;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 NtpMgr() = default;，执行这个函数并处理它的返回值或状态变化。
    NtpMgr() = default;
    // 函数调用：~NtpMgr();；执行该函数并处理它的返回值或副作用。
    ~NtpMgr();
    // 调用 NtpMgr(const NtpMgr&) = delete;，执行这个函数并处理它的返回值或状态变化。
    NtpMgr(const NtpMgr&) = delete;
    // 赋值：把右侧 (const NtpMgr&) = delete 的结果保存到左侧 NtpMgr& operator。
    NtpMgr& operator=(const NtpMgr&) = delete;

    // 静态函数 sntp_sync_callback：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void sntp_sync_callback(struct timeval* tv);

    // 执行语句：mutable std::mutex mutex_;；它会更新一次变量、对象或程序状态。
    mutable std::mutex mutex_;
    // 执行语句：bool initialized_{false};；它会更新一次变量、对象或程序状态。
    bool initialized_{false};
    // 执行语句：bool started_{false};；它会更新一次变量、对象或程序状态。
    bool started_{false};
    // 执行语句：SyncStatus sync_status_{SyncStatus::RESET};；它会更新一次变量、对象或程序状态。
    SyncStatus sync_status_{SyncStatus::RESET};
    // 执行语句：SyncCallback sync_callback_;；它会更新一次变量、对象或程序状态。
    SyncCallback sync_callback_;
    // 声明并默认初始化对象：std::vector<std::string> servers_{}。
    std::vector<std::string> servers_{};
    // 执行语句：SyncMode sync_mode_{SyncMode::IMMEDIATE};；它会更新一次变量、对象或程序状态。
    SyncMode sync_mode_{SyncMode::IMMEDIATE};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::protocol::ntp
