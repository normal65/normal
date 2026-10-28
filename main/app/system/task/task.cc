/**
 * [初学者导读] 文件：main/app/system/task/task.cc
 * 作用：封装 FreeRTOS 任务创建、栈配置、优先级和销毁。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：任务栈、优先级和销毁顺序是主要风险点。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "task.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <memory>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <mutex>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <unordered_map>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_cpu.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_heap_caps.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_log.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <freertos/FreeRTOS.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <freertos/task.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/info/info.hpp"

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "Task";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::sys::task {

// struct StartParam：把相关数据字段组合在一起，表示项目中的一个概念。
struct StartParam
{
    // 执行语句：Task::Func fn;；它会更新一次变量、对象或程序状态。
    Task::Func fn;
    // 执行语句：void* user_param;；它会更新一次变量、对象或程序状态。
    void* user_param;
    // 执行语句：Task* self;；它会更新一次变量、对象或程序状态。
    Task* self;
};

// 模块私有映射表：保存需要跨调用共享的键值数据，通常配合互斥锁保护。
static std::unordered_map<TaskHandle_t, std::unique_ptr<StartParam>> s_params;
// 模块私有互斥锁：保护下面这些静态数据不会被多个任务同时修改。
static std::mutex s_mtx;

// 函数定义/声明：调用 Task::Task()：构造函数只保存函数、配置和参数，不立即创建任务。。
Task::Task(Func fn, const Cfg& cfg, void* param)
    // 定义或声明函数 fn_：参数在括号里，返回类型写在函数名前面。
    : fn_(fn)
    // 定义或声明函数 cfg_：参数在括号里，返回类型写在函数名前面。
    , cfg_(cfg)
    // 定义或声明函数 param_：参数在括号里，返回类型写在函数名前面。
    , param_(param)
    // 定义或声明函数 handle_：参数在括号里，返回类型写在函数名前面。
    , handle_(nullptr)
    // 定义或声明函数 started_：参数在括号里，返回类型写在函数名前面。
    , started_(false)
    // 定义或声明函数 stack_buf_：参数在括号里，返回类型写在函数名前面。
    , stack_buf_(nullptr)
    // 定义或声明函数 tcb_：参数在括号里，返回类型写在函数名前面。
    , tcb_(nullptr)
{
}

// 调用 Task()：构造函数只保存函数、配置和参数，不立即创建任务。。
Task::~Task() = default;

// 函数定义/声明：调用 Task::wrapper()：构造函数只保存函数、配置和参数，不立即创建任务。。
void Task::wrapper(void* param)
{
    // 等待 start() 完成 s_params 注册后再执行，避免与创建线程竞态
    (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // 赋值：把右侧 static_cast<StartParam*>(param) 的结果保存到左侧 auto* p。
    auto* p = static_cast<StartParam*>(param);
    // 判断条件：p && p->fn 是否成立。
    if (p && p->fn)
    {
        // 访问对象成员：p->fn(p->user_param);；读取或修改这个对象里的成员状态。
        p->fn(p->user_param);
    }

    // 定义变量 TaskHandle_t self 并初始化为 xTaskGetCurrentTaskHandle()。
    TaskHandle_t self = xTaskGetCurrentTaskHandle();
    {
        // 函数调用：std::lock_guard<std::mutex> lk(s_mtx);；执行该函数并处理它的返回值或副作用。
        std::lock_guard<std::mutex> lk(s_mtx);
        // 访问对象成员：s_params.erase(self);；读取或修改这个对象里的成员状态。
        s_params.erase(self);
    }
    // 调用 vTaskDelete(nullptr);，执行这个函数并处理它的返回值或状态变化。
    vTaskDelete(nullptr);
}

// [初学者导读] Task::start()：创建普通栈或 PSRAM 栈。详见 docs/代码详解-初学者版.md。
bool Task::start()
{
    // 判断条件：started_ || handle_ != nullptr 是否成立。
    if (started_ || handle_ != nullptr)
    {
        // 调用 ESP_LOGW(TAG, "%s 已启动", cfg_.name);，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGW(TAG, "%s 已启动", cfg_.name);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 判断条件：!fn_ 是否成立。
    if (!fn_)
    {
        // 调用 ESP_LOGE(TAG, "任务函数为空");，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "任务函数为空");
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 定义变量 auto sp 并初始化为 std::make_unique<StartParam>(StartParam{fn_, param_, this})。
    auto sp = std::make_unique<StartParam>(StartParam{fn_, param_, this});

    // 判断条件：cfg_.delay_ms > 0 是否成立。
    if (cfg_.delay_ms > 0)
    {
        // 调用 vTaskDelay(pdMS_TO_TICKS(cfg_.delay_ms));，执行这个函数并处理它的返回值或状态变化。
        vTaskDelay(pdMS_TO_TICKS(cfg_.delay_ms));
    }

    // 定义变量 size_t depth 并初始化为 cfg_.stack_size / sizeof(StackType_t)。
    size_t depth = cfg_.stack_size / sizeof(StackType_t);

    // 判断条件：cfg_.use_psram 是否成立。
    if (cfg_.use_psram)
    {
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        stack_buf_ = static_cast<StackType_t*>(
            // 继续上一行的表达式或参数列表：heap_caps_malloc(cfg_.stack_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));。
            heap_caps_malloc(cfg_.stack_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        tcb_ = static_cast<StaticTask_t*>(
            // 继续上一行的表达式或参数列表：heap_caps_malloc(sizeof(StaticTask_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));。
            heap_caps_malloc(sizeof(StaticTask_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));

        // 判断条件：!stack_buf_ || !tcb_ 是否成立。
        if (!stack_buf_ || !tcb_)
        {
            // 调用 ESP_LOGE(TAG, "PSRAM分配失败: %s", cfg_.name);，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "PSRAM分配失败: %s", cfg_.name);
            // 判断条件：stack_buf_ 是否成立。
            if (stack_buf_)
                // 调用 heap_caps_free(stack_buf_);，执行这个函数并处理它的返回值或状态变化。
                heap_caps_free(stack_buf_);
            // 判断条件：tcb_ 是否成立。
            if (tcb_)
                // 调用 heap_caps_free(tcb_);，执行这个函数并处理它的返回值或状态变化。
                heap_caps_free(tcb_);
            // 赋值：把右侧 nullptr 的结果保存到左侧 stack_buf_。
            stack_buf_ = nullptr;
            // 赋值：把右侧 nullptr 的结果保存到左侧 tcb_。
            tcb_ = nullptr;
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }

        // 判断条件：cfg_.core_id == -1 是否成立。
        if (cfg_.core_id == -1)
        {
            // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
            handle_ = xTaskCreateStatic(wrapper, cfg_.name, depth, sp.get(),
                                        // 继续上一行的表达式或参数列表：static_cast<UBaseType_t>(cfg_.priority), stack_buf_, tcb_);。
                                        static_cast<UBaseType_t>(cfg_.priority), stack_buf_, tcb_);
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
            handle_ = xTaskCreateStaticPinnedToCore(wrapper, cfg_.name, depth, sp.get(),
                                                    // 继续上一行的表达式或参数列表：static_cast<UBaseType_t>(cfg_.priority),。
                                                    static_cast<UBaseType_t>(cfg_.priority),
                                                    // 继续上一行的表达式或参数列表：stack_buf_, tcb_, cfg_.core_id);。
                                                    stack_buf_, tcb_, cfg_.core_id);
        }

        // 判断条件：!handle_ 是否成立。
        if (!handle_)
        {
            // 调用 ESP_LOGE(TAG, "创建失败(PSRAM): %s", cfg_.name);，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "创建失败(PSRAM): %s", cfg_.name);
            // 调用 heap_caps_free(stack_buf_);，执行这个函数并处理它的返回值或状态变化。
            heap_caps_free(stack_buf_);
            // 调用 heap_caps_free(tcb_);，执行这个函数并处理它的返回值或状态变化。
            heap_caps_free(tcb_);
            // 赋值：把右侧 nullptr 的结果保存到左侧 stack_buf_。
            stack_buf_ = nullptr;
            // 赋值：把右侧 nullptr 的结果保存到左侧 tcb_。
            tcb_ = nullptr;
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }

        // 调用 ESP_LOGI(TAG, "%s 已创建 (PSRAM, %uB)", cfg_.name, static_cast<unsigned>(cfg_.stack_size));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGI(TAG, "%s 已创建 (PSRAM, %uB)", cfg_.name, static_cast<unsigned>(cfg_.stack_size));
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 执行语句：BaseType_t ret;；它会更新一次变量、对象或程序状态。
        BaseType_t ret;
        // 判断条件：cfg_.core_id == -1 是否成立。
        if (cfg_.core_id == -1)
        {
            // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
            ret = xTaskCreate(wrapper, cfg_.name, depth, sp.get(),
                              // 继续上一行的表达式或参数列表：static_cast<UBaseType_t>(cfg_.priority), &handle_);。
                              static_cast<UBaseType_t>(cfg_.priority), &handle_);
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 调用 get()：读取某个上下文和物理键对应的语义动作。推荐用 lookup()，它更清楚地表达“查表”。。
            ret = xTaskCreatePinnedToCore(wrapper, cfg_.name, depth, sp.get(),
                                          // 继续上一行的表达式或参数列表：static_cast<UBaseType_t>(cfg_.priority), &handle_,。
                                          static_cast<UBaseType_t>(cfg_.priority), &handle_,
                                          // 继续上一行的表达式或参数列表：cfg_.core_id);。
                                          cfg_.core_id);
        }

        // 判断条件：ret != pdPASS 是否成立。
        if (ret != pdPASS)
        {
            // 调用 ESP_LOGE(TAG, "创建失败: %s", cfg_.name);，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "创建失败: %s", cfg_.name);
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }
    }

    {
        // 函数调用：std::lock_guard<std::mutex> lk(s_mtx);；执行该函数并处理它的返回值或副作用。
        std::lock_guard<std::mutex> lk(s_mtx);
        // 赋值：把右侧 std::move(sp) 的结果保存到左侧 s_params[handle_]。
        s_params[handle_] = std::move(sp);
    }
    // 函数调用：(void)xTaskNotifyGive(handle_);；执行该函数并处理它的返回值或副作用。
    (void)xTaskNotifyGive(handle_);

    // 赋值：把右侧 true 的结果保存到左侧 started_。
    started_ = true;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// [初学者导读] Task::destroy()：直接删除，不是优雅停止。详见 docs/代码详解-初学者版.md。
void Task::destroy()
{
    // 判断条件：handle_ 是否成立。
    if (handle_)
    {
        // 调用 vTaskDelete(handle_);，执行这个函数并处理它的返回值或状态变化。
        vTaskDelete(handle_);

        {
            // 函数调用：std::lock_guard<std::mutex> lk(s_mtx);；执行该函数并处理它的返回值或副作用。
            std::lock_guard<std::mutex> lk(s_mtx);
            // 访问对象成员：s_params.erase(handle_);；读取或修改这个对象里的成员状态。
            s_params.erase(handle_);
        }

        // 赋值：把右侧 nullptr 的结果保存到左侧 handle_。
        handle_ = nullptr;
        // 赋值：把右侧 false 的结果保存到左侧 started_。
        started_ = false;

        // 判断条件：stack_buf_ 是否成立。
        if (stack_buf_)
        {
            // 调用 heap_caps_free(stack_buf_);，执行这个函数并处理它的返回值或状态变化。
            heap_caps_free(stack_buf_);
            // 赋值：把右侧 nullptr 的结果保存到左侧 stack_buf_。
            stack_buf_ = nullptr;
        }
        // 判断条件：tcb_ 是否成立。
        if (tcb_)
        {
            // 调用 heap_caps_free(tcb_);，执行这个函数并处理它的返回值或状态变化。
            heap_caps_free(tcb_);
            // 赋值：把右侧 nullptr 的结果保存到左侧 tcb_。
            tcb_ = nullptr;
        }
    }
}

// 函数定义/声明：调用 Task::suspend()：这些是对 FreeRTOS API 的直接包装： suspend() 暂停任务。 resume() 恢复。 setpriority() 动态改优先级。 getpriority() 读取当前优先级。。
void Task::suspend()
{
    // 判断条件：handle_ 是否成立。
    if (handle_)
        // 调用 vTaskSuspend(handle_);，执行这个函数并处理它的返回值或状态变化。
        vTaskSuspend(handle_);
}

// 函数定义/声明：调用 Task::resume()：这些是对 FreeRTOS API 的直接包装： suspend() 暂停任务。 resume() 恢复。 setpriority() 动态改优先级。 getpriority() 读取当前优先级。。
void Task::resume()
{
    // 判断条件：handle_ 是否成立。
    if (handle_)
        // 调用 vTaskResume(handle_);，执行这个函数并处理它的返回值或状态变化。
        vTaskResume(handle_);
}

// 函数定义/声明：调用 Task::set_priority()：这些是对 FreeRTOS API 的直接包装： suspend() 暂停任务。 resume() 恢复。 setpriority() 动态改优先级。 getpriority() 读取当前优先级。。
void Task::set_priority(Priority p)
{
    // 判断条件：handle_ 是否成立。
    if (handle_)
    {
        // 调用 vTaskPrioritySet(handle_, static_cast<UBaseType_t>(p));，执行这个函数并处理它的返回值或状态变化。
        vTaskPrioritySet(handle_, static_cast<UBaseType_t>(p));
        // 赋值：把右侧 p 的结果保存到左侧 cfg_.priority。
        cfg_.priority = p;
    }
}

// 函数定义/声明：调用 Task::get_priority()：这些是对 FreeRTOS API 的直接包装： suspend() 暂停任务。 resume() 恢复。 setpriority() 动态改优先级。 getpriority() 读取当前优先级。。
Priority Task::get_priority() const
{
    // 判断条件：handle_ 是否成立。
    if (handle_)
    {
        // 返回 static_cast<Priority>(uxTaskPriorityGet(handle_)) 给调用者，表示当前函数的执行结果。
        return static_cast<Priority>(uxTaskPriorityGet(handle_));
    }
    // 返回 cfg_.priority 给调用者，表示当前函数的执行结果。
    return cfg_.priority;
}

// [初学者导读] Task::info()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
Info Task::info() const
{
    // 执行语句：Info i;；它会更新一次变量、对象或程序状态。
    Info i;
    // 判断条件：handle_ 是否成立。
    if (handle_)
    {
        // 赋值：把右侧 cfg_.name 的结果保存到左侧 i.name。
        i.name = cfg_.name;
        // 赋值：把右侧 uxTaskPriorityGet(handle_) 的结果保存到左侧 i.priority。
        i.priority = uxTaskPriorityGet(handle_);

        // 定义变量 eTaskState st 并初始化为 eTaskGetState(handle_)。
        eTaskState st = eTaskGetState(handle_);
        // 多分支判断：根据括号里的值选择不同 case。
        switch (st)
        {
            // 分支匹配：case eRunning 成立时执行下面的代码。
            case eRunning:
                // 赋值：把右侧 State::RUNNING 的结果保存到左侧 i.state。
                i.state = State::RUNNING;
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 分支匹配：case eReady 成立时执行下面的代码。
            case eReady:
                // 赋值：把右侧 State::READY 的结果保存到左侧 i.state。
                i.state = State::READY;
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 分支匹配：case eBlocked 成立时执行下面的代码。
            case eBlocked:
                // 赋值：把右侧 State::BLOCKED 的结果保存到左侧 i.state。
                i.state = State::BLOCKED;
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 分支匹配：case eSuspended 成立时执行下面的代码。
            case eSuspended:
                // 赋值：把右侧 State::SUSPENDED 的结果保存到左侧 i.state。
                i.state = State::SUSPENDED;
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            // 执行这条代码：default:。它会更新变量、对象状态或调用链。
            default:
                // 赋值：把右侧 State::DELETED 的结果保存到左侧 i.state。
                i.state = State::DELETED;
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
        }
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 赋值：把右侧 cfg_.name 的结果保存到左侧 i.name。
        i.name = cfg_.name;
        // 赋值：把右侧 static_cast<UBaseType_t>(cfg_.priority) 的结果保存到左侧 i.priority。
        i.priority = static_cast<UBaseType_t>(cfg_.priority);
        // 赋值：把右侧 State::DELETED 的结果保存到左侧 i.state。
        i.state = State::DELETED;
    }
    // 返回 i 给调用者，表示当前函数的执行结果。
    return i;
}

// 定义或声明函数 TaskMgr::get_instance：参数在括号里，返回类型写在函数名前面。
TaskMgr& TaskMgr::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static TaskMgr s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 函数定义/声明：调用 TaskMgr::find()：find(id) 根据 AppId 找对象；entryat(idx) 根据网格索引返回 {id, title, iconcp} 这样的轻量条目。。
TaskHandle_t TaskMgr::find(const char* name)
{
    // 返回 xTaskGetHandle(name) 给调用者，表示当前函数的执行结果。
    return xTaskGetHandle(name);
}

// 定义或声明函数 TaskMgr::get_info：参数在括号里，返回类型写在函数名前面。
Info TaskMgr::get_info(TaskHandle_t h)
{
    // 执行语句：Info i;；它会更新一次变量、对象或程序状态。
    Info i;
    // 判断条件：!h 是否成立。
    if (!h)
        // 返回 i 给调用者，表示当前函数的执行结果。
        return i;

    // 赋值：把右侧 pcTaskGetName(h) 的结果保存到左侧 i.name。
    i.name = pcTaskGetName(h);
    // 赋值：把右侧 uxTaskPriorityGet(h) 的结果保存到左侧 i.priority。
    i.priority = uxTaskPriorityGet(h);

    // 定义变量 eTaskState st 并初始化为 eTaskGetState(h)。
    eTaskState st = eTaskGetState(h);
    // 多分支判断：根据括号里的值选择不同 case。
    switch (st)
    {
        // 分支匹配：case eRunning 成立时执行下面的代码。
        case eRunning:
            // 赋值：把右侧 State::RUNNING 的结果保存到左侧 i.state。
            i.state = State::RUNNING;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case eReady 成立时执行下面的代码。
        case eReady:
            // 赋值：把右侧 State::READY 的结果保存到左侧 i.state。
            i.state = State::READY;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case eBlocked 成立时执行下面的代码。
        case eBlocked:
            // 赋值：把右侧 State::BLOCKED 的结果保存到左侧 i.state。
            i.state = State::BLOCKED;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case eSuspended 成立时执行下面的代码。
        case eSuspended:
            // 赋值：把右侧 State::SUSPENDED 的结果保存到左侧 i.state。
            i.state = State::SUSPENDED;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 赋值：把右侧 State::DELETED 的结果保存到左侧 i.state。
            i.state = State::DELETED;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
    }

    // 返回 i 给调用者，表示当前函数的执行结果。
    return i;
}

// 定义或声明函数 TaskMgr::current_info：参数在括号里，返回类型写在函数名前面。
Info TaskMgr::current_info()
{
    // 返回 get_info(xTaskGetCurrentTaskHandle()) 给调用者，表示当前函数的执行结果。
    return get_info(xTaskGetCurrentTaskHandle());
}

// 定义或声明函数 TaskMgr::count：参数在括号里，返回类型写在函数名前面。
uint32_t TaskMgr::count()
{
    // 返回 uxTaskGetNumberOfTasks() 给调用者，表示当前函数的执行结果。
    return uxTaskGetNumberOfTasks();
}

// 函数定义/声明：调用 TaskMgr::delay_ms()：调用 vTaskDelay()，这是让出 CPU 的阻塞延时。一千毫秒延时期间，任务不占 CPU，调度器可以运行别人。。
void TaskMgr::delay_ms(uint32_t ms)
{
    // 调用 vTaskDelay(pdMS_TO_TICKS(ms));，执行这个函数并处理它的返回值或状态变化。
    vTaskDelay(pdMS_TO_TICKS(ms));
}

// 函数定义/声明：调用 TaskMgr::delay_us()：短时间忙等。
void TaskMgr::delay_us(uint32_t us)
{
    // 判断条件：us == 0 是否成立。
    if (us == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 定义变量 auto cpu_info 并初始化为 app::sys::info::CpuInfo::get_cpu_info()。
    auto cpu_info = app::sys::info::CpuInfo::get_cpu_info();
    // 定义变量 uint32_t freq_hz 并初始化为 cpu_info.get_cpu_freq()。
    uint32_t freq_hz = cpu_info.get_cpu_freq();

    // 固定参数 US_PER_SEC = 1000000ULL：由当前模块定义，用于控制布局、时序或硬件行为。
    constexpr uint64_t US_PER_SEC = 1000000ULL;
    // 定义变量 uint64_t cycles 并初始化为 static_cast<uint64_t>(us) * freq_hz / US_PER_SEC。
    uint64_t cycles = static_cast<uint64_t>(us) * freq_hz / US_PER_SEC;
    // 定义变量 uint64_t start 并初始化为 esp_cpu_get_cycle_count()。
    uint64_t start = esp_cpu_get_cycle_count();

    // 循环执行：条件为真时重复运行，条件变化后退出。
    while ((esp_cpu_get_cycle_count() - start) < cycles)
    {
    }
}

// 定义或声明函数 TaskMgr::tick_count：参数在括号里，返回类型写在函数名前面。
uint32_t TaskMgr::tick_count()
{
    // 返回 xTaskGetTickCount() 给调用者，表示当前函数的执行结果。
    return xTaskGetTickCount();
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::sys::task
