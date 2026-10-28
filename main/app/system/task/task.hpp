// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/system/task/task.hpp
 * 作用：封装 FreeRTOS 任务创建、栈配置、优先级和销毁。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：任务栈、优先级和销毁顺序是主要风险点。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstddef>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <functional>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <string>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <freertos/FreeRTOS.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <freertos/task.h>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::sys::task {

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class Priority : UBaseType_t
{
    // 执行这条代码：IDLE = 0,。它会更新变量、对象状态或调用链。
    IDLE = 0,
    // 继续上一行的表达式或参数列表：LOW = 1,。
    LOW = 1,
    // 继续上一行的表达式或参数列表：NORMAL = 5,。
    NORMAL = 5,
    // 继续上一行的表达式或参数列表：HIGH = 10,。
    HIGH = 10,
    // 继续上一行的表达式或参数列表：REALTIME = 15,。
    REALTIME = 15,
};

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class State
{
    // 执行这条代码：RUNNING,。它会更新变量、对象状态或调用链。
    RUNNING,
    // 继续上一行的表达式或参数列表：READY,。
    READY,
    // 继续上一行的表达式或参数列表：BLOCKED,。
    BLOCKED,
    // 继续上一行的表达式或参数列表：SUSPENDED,。
    SUSPENDED,
    // 继续上一行的表达式或参数列表：DELETED,。
    DELETED,
};

// struct Cfg：把相关数据字段组合在一起，表示项目中的一个概念。
struct Cfg
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* name;
    // 执行语句：size_t stack_size;；它会更新一次变量、对象或程序状态。
    size_t stack_size;
    // 执行语句：Priority priority;；它会更新一次变量、对象或程序状态。
    Priority priority;
    // 执行语句：BaseType_t core_id;；它会更新一次变量、对象或程序状态。
    BaseType_t core_id;
    // 执行语句：uint32_t delay_ms;；它会更新一次变量、对象或程序状态。
    uint32_t delay_ms;
    // 执行语句：bool use_psram;；它会更新一次变量、对象或程序状态。
    bool use_psram;

    // 定义或声明函数 Cfg：参数在括号里，返回类型写在函数名前面。
    Cfg()
        // 定义或声明函数 name：参数在括号里，返回类型写在函数名前面。
        : name("task")
        // 定义或声明函数 stack_size：参数在括号里，返回类型写在函数名前面。
        , stack_size(4096)
        // 定义或声明函数 priority：参数在括号里，返回类型写在函数名前面。
        , priority(Priority::NORMAL)
        // 定义或声明函数 core_id：参数在括号里，返回类型写在函数名前面。
        , core_id(-1)
        // 函数定义/声明：调用 delay_ms()：调用 vTaskDelay()，这是让出 CPU 的阻塞延时。一千毫秒延时期间，任务不占 CPU，调度器可以运行别人。。
        , delay_ms(0)
        // 定义或声明函数 use_psram：参数在括号里，返回类型写在函数名前面。
        , use_psram(false)
    {
    }

    // 静态值 p：只属于当前模块或类，并保持固定含义。
    static Cfg light(const char* n, Priority p = Priority::NORMAL)
    {
        // 执行语句：Cfg c;；它会更新一次变量、对象或程序状态。
        Cfg c;
        // 赋值：把右侧 n 的结果保存到左侧 c.name。
        c.name = n;
        // 赋值：把右侧 2048 的结果保存到左侧 c.stack_size。
        c.stack_size = 2048;
        // 赋值：把右侧 p 的结果保存到左侧 c.priority。
        c.priority = p;
        // 返回 c 给调用者，表示当前函数的执行结果。
        return c;
    }

    // 静态值 p：只属于当前模块或类，并保持固定含义。
    static Cfg small(const char* n, Priority p = Priority::LOW)
    {
        // 执行语句：Cfg c;；它会更新一次变量、对象或程序状态。
        Cfg c;
        // 赋值：把右侧 n 的结果保存到左侧 c.name。
        c.name = n;
        // 赋值：把右侧 1024 的结果保存到左侧 c.stack_size。
        c.stack_size = 1024;
        // 赋值：把右侧 p 的结果保存到左侧 c.priority。
        c.priority = p;
        // 返回 c 给调用者，表示当前函数的执行结果。
        return c;
    }

    // 静态值 p：只属于当前模块或类，并保持固定含义。
    static Cfg large(const char* n, Priority p = Priority::NORMAL)
    {
        // 执行语句：Cfg c;；它会更新一次变量、对象或程序状态。
        Cfg c;
        // 赋值：把右侧 n 的结果保存到左侧 c.name。
        c.name = n;
        // 赋值：把右侧 8192 的结果保存到左侧 c.stack_size。
        c.stack_size = 8192;
        // 赋值：把右侧 p 的结果保存到左侧 c.priority。
        c.priority = p;
        // 返回 c 给调用者，表示当前函数的执行结果。
        return c;
    }
};

// struct Info：把相关数据字段组合在一起，表示项目中的一个概念。
struct Info
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* name;
    // 执行语句：State state;；它会更新一次变量、对象或程序状态。
    State state;
    // 执行语句：UBaseType_t priority;；它会更新一次变量、对象或程序状态。
    UBaseType_t priority;
    // 执行语句：uint32_t runtime_pct;；它会更新一次变量、对象或程序状态。
    uint32_t runtime_pct;

    // 定义或声明函数 Info：参数在括号里，返回类型写在函数名前面。
    Info()
        // 定义或声明函数 name：参数在括号里，返回类型写在函数名前面。
        : name(nullptr)
        // 定义或声明函数 state：参数在括号里，返回类型写在函数名前面。
        , state(State::DELETED)
        // 定义或声明函数 priority：参数在括号里，返回类型写在函数名前面。
        , priority(0)
        // 定义或声明函数 runtime_pct：参数在括号里，返回类型写在函数名前面。
        , runtime_pct(0)
    {
    }
};

/** FreeRTOS 任务包装（支持 PSRAM 栈 + 静态创建） */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class Task
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using Func = std::function<void(void*)>;

    // 调用 Task(Funcfn,constCfg&cfg,void*param=nullptr)：构造函数只保存函数、配置和参数，不立即创建任务。。
    Task(Func fn, const Cfg& cfg, void* param = nullptr);
    // 调用 Task()：构造函数只保存函数、配置和参数，不立即创建任务。。
    ~Task();

    // 调用 Task(constTask&)：构造函数只保存函数、配置和参数，不立即创建任务。。
    Task(const Task&) = delete;
    // 赋值：把右侧 (const Task&) = delete 的结果保存到左侧 Task& operator。
    Task& operator=(const Task&) = delete;
    // 调用 Task(Task&&)：构造函数只保存函数、配置和参数，不立即创建任务。。
    Task(Task&&) = delete;
    // 赋值：把右侧 (Task&&) = delete 的结果保存到左侧 Task& operator。
    Task& operator=(Task&&) = delete;

    // 调用 start()：init() 确认触摸驱动可用，创建 Normal 配置的手势对象，并尽量打开中断。start() 创建高优先级输入任务。stop() 设置 running = false 并销毁任务对象。。
    bool start();
    // 调用 destroy()：直接删除，不是优雅停止。
    void destroy();
    // 调用 suspend()：这些是对 FreeRTOS API 的直接包装： suspend() 暂停任务。 resume() 恢复。 setpriority() 动态改优先级。 getpriority() 读取当前优先级。。
    void suspend();
    // 调用 resume()：这些是对 FreeRTOS API 的直接包装： suspend() 暂停任务。 resume() 恢复。 setpriority() 动态改优先级。 getpriority() 读取当前优先级。。
    void resume();

    // 调用 set_priority(Priorityp)：这些是对 FreeRTOS API 的直接包装： suspend() 暂停任务。 resume() 恢复。 setpriority() 动态改优先级。 getpriority() 读取当前优先级。。
    void set_priority(Priority p);
    // 调用 get_priority()：这些是对 FreeRTOS API 的直接包装： suspend() 暂停任务。 resume() 恢复。 setpriority() 动态改优先级。 getpriority() 读取当前优先级。。
    Priority get_priority() const;

    // 定义或声明函数 handle：参数在括号里，返回类型写在函数名前面。
    TaskHandle_t handle() const
    {
        // 返回 handle_ 给调用者，表示当前函数的执行结果。
        return handle_;
    }
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* name() const
    {
        // 返回 cfg_.name 给调用者，表示当前函数的执行结果。
        return cfg_.name;
    }
    // 调用 info()：Task::info() 把当前句柄转换成可读状态。。
    Info info() const;
    // 定义或声明函数 valid：参数在括号里，返回类型写在函数名前面。
    bool valid() const
    {
        // 返回 handle_ != nullptr 给调用者，表示当前函数的执行结果。
        return handle_ != nullptr;
    }

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 静态函数 wrapper：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void wrapper(void* param);

    // 执行语句：Func fn_;；它会更新一次变量、对象或程序状态。
    Func fn_;
    // 执行语句：Cfg cfg_;；它会更新一次变量、对象或程序状态。
    Cfg cfg_;
    // 执行语句：void* param_;；它会更新一次变量、对象或程序状态。
    void* param_;
    // 执行语句：TaskHandle_t handle_;；它会更新一次变量、对象或程序状态。
    TaskHandle_t handle_;
    // 执行语句：bool started_;；它会更新一次变量、对象或程序状态。
    bool started_;
    // 执行语句：StackType_t* stack_buf_;；它会更新一次变量、对象或程序状态。
    StackType_t* stack_buf_;
    // 执行语句：StaticTask_t* tcb_;；它会更新一次变量、对象或程序状态。
    StaticTask_t* tcb_;
};

/** 任务查询与 delay_ms / tick_count 工具 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class TaskMgr
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static TaskMgr& get_instance();

    // 调用 find(constchar*name)：find(id) 根据 AppId 找对象；entryat(idx) 根据网格索引返回 {id, title, iconcp} 这样的轻量条目。。
    TaskHandle_t find(const char* name);
    // 函数调用：Info get_info(TaskHandle_t h);；执行该函数并处理它的返回值或副作用。
    Info get_info(TaskHandle_t h);
    // 函数调用：Info current_info();；执行该函数并处理它的返回值或副作用。
    Info current_info();
    // 函数调用：uint32_t count();；执行该函数并处理它的返回值或副作用。
    uint32_t count();

    // 静态函数 delay_ms：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void delay_ms(uint32_t ms);
    // 静态函数 delay_us：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void delay_us(uint32_t us);
    // 静态函数 tick_count：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static uint32_t tick_count();

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 TaskMgr() = default;，执行这个函数并处理它的返回值或状态变化。
    TaskMgr() = default;
    // 赋值：把右侧 default 的结果保存到左侧 ~TaskMgr()。
    ~TaskMgr() = default;
    // 调用 TaskMgr(const TaskMgr&) = delete;，执行这个函数并处理它的返回值或状态变化。
    TaskMgr(const TaskMgr&) = delete;
    // 赋值：把右侧 (const TaskMgr&) = delete 的结果保存到左侧 TaskMgr& operator。
    TaskMgr& operator=(const TaskMgr&) = delete;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::sys::task
