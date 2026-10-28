/**
 * [初学者导读] 文件：main/app/system/event/event.cc
 * 作用：封装 ESP-IDF 事件循环、事件注册和投递。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：事件 base 的身份必须长期稳定，不要指向临时字符串。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "event.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <mutex>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <unordered_map>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <unordered_set>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <vector>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_event.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_log.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <freertos/FreeRTOS.h>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <freertos/task.h>

// 静态值 TAG：只属于当前模块或类，并保持固定含义。
static const char* const TAG = "Event";

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::sys::event {

// struct HandlerKey：把相关数据字段组合在一起，表示项目中的一个概念。
struct HandlerKey
{
    // 执行语句：esp_event_base_t base;；它会更新一次变量、对象或程序状态。
    esp_event_base_t base;
    // 执行语句：EventId id;；它会更新一次变量、对象或程序状态。
    EventId id;

    // 定义变量 bool operator 并初始化为 =(const HandlerKey& o) cons。
    bool operator==(const HandlerKey& o) const
    {
        // 返回 base == o.base && id == o.id 给调用者，表示当前函数的执行结果。
        return base == o.base && id == o.id;
    }
};

// struct HandlerKeyHash：把相关数据字段组合在一起，表示项目中的一个概念。
struct HandlerKeyHash
{
    // 定义或声明函数 operator：参数在括号里，返回类型写在函数名前面。
    size_t operator()(const HandlerKey& k) const
    {
        // 返回 std::hash<const char*>{}(k.base) ^ (std::hash<EventId>{}(k.id) << 1) 给调用者，表示当前函数的执行结果。
        return std::hash<const char*>{}(k.base) ^ (std::hash<EventId>{}(k.id) << 1);
    }
};

// 模块私有映射表：保存需要跨调用共享的键值数据，通常配合互斥锁保护。
static std::unordered_map<HandlerKey, EventHandler, HandlerKeyHash> s_handlers;
// 模块私有互斥锁：保护下面这些静态数据不会被多个任务同时修改。
static std::mutex s_mtx;
// 模块私有静态表 s_bases：保存该模块需要跨调用共享的数据。
static std::unordered_set<esp_event_base_t> s_bases;
// 模块私有映射表：保存需要跨调用共享的键值数据，通常配合互斥锁保护。
static std::unordered_map<esp_event_base_t, size_t> s_base_cnt;

// 定义或声明函数 EventMgr::EventMgr：参数在括号里，返回类型写在函数名前面。
EventMgr::EventMgr()
    // 定义或声明函数 init_：参数在括号里，返回类型写在函数名前面。
    : init_(false)
{
}

// 定义或声明函数 EventMgr::~EventMgr：参数在括号里，返回类型写在函数名前面。
EventMgr::~EventMgr()
{
    // 判断条件：init_ 是否成立。
    if (init_)
        // 调用 deinit()：创建和销毁队列。
        deinit();
}

// 定义或声明函数 EventMgr::get_instance：参数在括号里，返回类型写在函数名前面。
EventMgr& EventMgr::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static EventMgr s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// [初学者导读] EventMgr::init()：实现流程与设计原因见详解对应小节。详见 docs/代码详解-初学者版.md。
bool EventMgr::init()
{
    // 判断条件：init_ 是否成立。
    if (init_)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;

    // 定义变量 esp_err_t ret 并初始化为 esp_event_loop_create_default()。
    esp_err_t ret = esp_event_loop_create_default();
    // 判断条件：ret != ESP_OK && ret != ESP_ERR_INVALID_STATE 是否成立。
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 赋值：把右侧 true 的结果保存到左侧 init_。
    init_ = true;
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 EventMgr::deinit：参数在括号里，返回类型写在函数名前面。
void EventMgr::deinit()
{
    // 判断条件：!init_ 是否成立。
    if (!init_)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 执行语句：std::vector<esp_event_base_t> bases;；它会更新一次变量、对象或程序状态。
    std::vector<esp_event_base_t> bases;
    {
        // 函数调用：std::lock_guard<std::mutex> lk(s_mtx);；执行该函数并处理它的返回值或副作用。
        std::lock_guard<std::mutex> lk(s_mtx);
        // 访问对象成员：bases.reserve(s_bases.size());；读取或修改这个对象里的成员状态。
        bases.reserve(s_bases.size());
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (const auto& b : s_bases)
        {
            // 访问对象成员：bases.push_back(b);；读取或修改这个对象里的成员状态。
            bases.push_back(b);
        }
        // 调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
        s_handlers.clear();
        // 调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
        s_bases.clear();
        // 调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
        s_base_cnt.clear();
    }

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (auto b : bases)
    {
        // 调用 esp_event_handler_unregister(b, ESP_EVENT_ANY_ID, &c_handler);，执行这个函数并处理它的返回值或状态变化。
        esp_event_handler_unregister(b, ESP_EVENT_ANY_ID, &c_handler);
    }

    // 赋值：把右侧 false 的结果保存到左侧 init_。
    init_ = false;
}

// 函数定义/声明：调用 EventMgr::create_base()：身份必须长期稳定。
esp_event_base_t EventMgr::create_base(const char* name)
{
    // 判断条件：!name 是否成立。
    if (!name)
        // 返回 nullptr 给调用者，表示当前函数的执行结果。
        return nullptr;
    // 返回 name 给调用者，表示当前函数的执行结果。
    return name;
}

// 函数定义/声明：调用 EventMgr::register_handler()：注册表键是 {base, id}。。
bool EventMgr::register_handler(esp_event_base_t base, EventId id, EventHandler handler)
{
    // 判断条件：!init_ || !base || !handler 是否成立。
    if (!init_ || !base || !handler)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 执行语句：HandlerKey key{base, id};；它会更新一次变量、对象或程序状态。
    HandlerKey key{base, id};

    // 函数调用：std::lock_guard<std::mutex> lk(s_mtx);；执行该函数并处理它的返回值或副作用。
    std::lock_guard<std::mutex> lk(s_mtx);

    // 定义变量并调用函数初始化：调用 find(key)：find(id) 根据 AppId 找对象；entryat(idx) 根据网格索引返回 {id, title, iconcp} 这样的轻量条目。，结果保存到 bool is_new。
    bool is_new = (s_handlers.find(key) == s_handlers.end());
    // 定义变量并调用函数初始化：调用 find(base)：find(id) 根据 AppId 找对象；entryat(idx) 根据网格索引返回 {id, title, iconcp} 这样的轻量条目。，结果保存到 bool need_reg。
    bool need_reg = (s_bases.find(base) == s_bases.end());

    // 判断条件：need_reg 是否成立。
    if (need_reg)
    {
        // 定义变量 esp_err_t ret 并初始化为 esp_event_handler_register(base, ESP_EVENT_ANY_ID, &c_handler, nullptr)。
        esp_err_t ret = esp_event_handler_register(base, ESP_EVENT_ANY_ID, &c_handler, nullptr);
        // 判断条件：ret != ESP_OK 是否成立。
        if (ret != ESP_OK)
        {
            // 调用 ESP_LOGE(TAG, "注册失败: %s", esp_err_to_name(ret));，执行这个函数并处理它的返回值或状态变化。
            ESP_LOGE(TAG, "注册失败: %s", esp_err_to_name(ret));
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        }
        // 访问对象成员：s_bases.insert(base);；读取或修改这个对象里的成员状态。
        s_bases.insert(base);
    }

    // 赋值：把右侧 handler 的结果保存到左侧 s_handlers[key]。
    s_handlers[key] = handler;

    // 判断条件：is_new 是否成立。
    if (is_new)
    {
        // 执行语句：s_base_cnt[base]++;；它会更新一次变量、对象或程序状态。
        s_base_cnt[base]++;
    }

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数定义/声明：调用 EventMgr::unregister_handler()：注册表键是 {base, id}。。
bool EventMgr::unregister_handler(esp_event_base_t base, EventId id) const
{
    // 判断条件：!init_ 是否成立。
    if (!init_)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 执行语句：HandlerKey key{base, id};；它会更新一次变量、对象或程序状态。
    HandlerKey key{base, id};

    // 定义变量 bool need_unreg 并初始化为 false。
    bool need_unreg = false;
    {
        // 函数调用：std::lock_guard<std::mutex> lk(s_mtx);；执行该函数并处理它的返回值或副作用。
        std::lock_guard<std::mutex> lk(s_mtx);

        // 定义变量并调用函数初始化：调用 find(key)：find(id) 根据 AppId 找对象；entryat(idx) 根据网格索引返回 {id, title, iconcp} 这样的轻量条目。，结果保存到 auto it。
        auto it = s_handlers.find(key);
        // 判断条件：it == s_handlers.end() 是否成立。
        if (it == s_handlers.end())
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;

        // 访问对象成员：s_handlers.erase(it);；读取或修改这个对象里的成员状态。
        s_handlers.erase(it);

        // 定义变量并调用函数初始化：调用 find(base)：find(id) 根据 AppId 找对象；entryat(idx) 根据网格索引返回 {id, title, iconcp} 这样的轻量条目。，结果保存到 auto cnt_it。
        auto cnt_it = s_base_cnt.find(base);
        // 判断条件：cnt_it != s_base_cnt.end() 是否成立。
        if (cnt_it != s_base_cnt.end())
        {
            // 判断条件：cnt_it->second > 1 是否成立。
            if (cnt_it->second > 1)
            {
                // 访问对象成员：cnt_it->second--;；读取或修改这个对象里的成员状态。
                cnt_it->second--;
            }
            // 备用分支：前面的条件不成立时执行这里。
            else
            {
                // 访问对象成员：s_base_cnt.erase(cnt_it);；读取或修改这个对象里的成员状态。
                s_base_cnt.erase(cnt_it);
                // 判断条件：s_bases.find(base) != s_bases.end() 是否成立。
                if (s_bases.find(base) != s_bases.end())
                {
                    // 赋值：把右侧 true 的结果保存到左侧 need_unreg。
                    need_unreg = true;
                    // 访问对象成员：s_bases.erase(base);；读取或修改这个对象里的成员状态。
                    s_bases.erase(base);
                }
            }
        }
    }

    // 判断条件：need_unreg 是否成立。
    if (need_unreg)
    {
        // 调用 esp_event_handler_unregister(base, ESP_EVENT_ANY_ID, &c_handler);，执行这个函数并处理它的返回值或状态变化。
        esp_event_handler_unregister(base, ESP_EVENT_ANY_ID, &c_handler);
    }

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数调用：bool EventMgr::post(esp_event_base_t base, EventId id, const EventData& data,；执行该函数并处理它的返回值或副作用。
bool EventMgr::post(esp_event_base_t base, EventId id, const EventData& data,
                    // 继续上一行的表达式或参数列表：uint32_t timeout_ms) const。
                    uint32_t timeout_ms) const
{
    // 判断条件：!init_ || !base 是否成立。
    if (!init_ || !base)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 定义变量 TickType_t ticks 并初始化为 timeout_ms > 0 ? pdMS_TO_TICKS(timeout_ms) : 0。
    TickType_t ticks = timeout_ms > 0 ? pdMS_TO_TICKS(timeout_ms) : 0;

    // 定义变量 esp_err_t ret 并初始化为 esp_event_post(base, id, data.data, data.size, ticks)。
    esp_err_t ret = esp_event_post(base, id, data.data, data.size, ticks);
    // 判断条件：ret != ESP_OK 是否成立。
    if (ret != ESP_OK)
    {
        // 调用 ESP_LOGE(TAG, "发送失败: %s", esp_err_to_name(ret));，执行这个函数并处理它的返回值或状态变化。
        ESP_LOGE(TAG, "发送失败: %s", esp_err_to_name(ret));
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 EventMgr::post_isr：参数在括号里，返回类型写在函数名前面。
bool EventMgr::post_isr(esp_event_base_t base, EventId id, const EventData& data) const
{
    // 判断条件：!init_ || !base 是否成立。
    if (!init_ || !base)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 定义变量 BaseType_t unblocked 并初始化为 pdFALSE。
    BaseType_t unblocked = pdFALSE;
    // 定义变量 esp_err_t ret 并初始化为 esp_event_isr_post(base, id, data.data, data.size, &unblocked)。
    esp_err_t ret = esp_event_isr_post(base, id, data.data, data.size, &unblocked);
    // 判断条件：ret != ESP_OK 是否成立。
    if (ret != ESP_OK)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 判断条件：unblocked == pdTRUE 是否成立。
    if (unblocked == pdTRUE)
    {
        // 调用 portYIELD_FROM_ISR();，执行这个函数并处理它的返回值或状态变化。
        portYIELD_FROM_ISR();
    }

    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 函数定义/声明：调用 EventMgr::c_handler()：底层只注册一次这个静态回调。收到事件后： 1. 查找精确 {base, id} 处理器。 2. 再查找 {base, ANYID} 处理器。 3. 先调用精确处理器。 4. 再调用通用处理器。。
void EventMgr::c_handler(void* /*arg*/, esp_event_base_t base, int32_t id, void* data)
{
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    HandlerKey key{base, static_cast<EventId>(id)};

    // 执行语句：EventHandler specific;；它会更新一次变量、对象或程序状态。
    EventHandler specific;
    // 执行语句：EventHandler any;；它会更新一次变量、对象或程序状态。
    EventHandler any;
    {
        // 函数调用：std::lock_guard<std::mutex> lk(s_mtx);；执行该函数并处理它的返回值或副作用。
        std::lock_guard<std::mutex> lk(s_mtx);

        // 定义变量并调用函数初始化：调用 find(key)：find(id) 根据 AppId 找对象；entryat(idx) 根据网格索引返回 {id, title, iconcp} 这样的轻量条目。，结果保存到 auto it。
        auto it = s_handlers.find(key);
        // 判断条件：it != s_handlers.end() 是否成立。
        if (it != s_handlers.end())
        {
            // 赋值：把右侧 it->second 的结果保存到左侧 specific。
            specific = it->second;
        }

        // 执行语句：HandlerKey any_key{base, ESP_EVENT_ANY_ID};；它会更新一次变量、对象或程序状态。
        HandlerKey any_key{base, ESP_EVENT_ANY_ID};
        // 定义变量并调用函数初始化：调用 find(any_key)：find(id) 根据 AppId 找对象；entryat(idx) 根据网格索引返回 {id, title, iconcp} 这样的轻量条目。，结果保存到 auto any_it。
        auto any_it = s_handlers.find(any_key);
        // 判断条件：any_it != s_handlers.end() 是否成立。
        if (any_it != s_handlers.end())
        {
            // 赋值：把右侧 any_it->second 的结果保存到左侧 any。
            any = any_it->second;
        }
    }

    // 执行语句：EventData d{data, 0};；它会更新一次变量、对象或程序状态。
    EventData d{data, 0};

    // 判断条件：specific 是否成立。
    if (specific)
        // 调用 specific(base, static_cast<EventId>(id), d);，执行这个函数并处理它的返回值或状态变化。
        specific(base, static_cast<EventId>(id), d);
    // 判断条件：any 是否成立。
    if (any)
        // 调用 any(base, static_cast<EventId>(id), d);，执行这个函数并处理它的返回值或状态变化。
        any(base, static_cast<EventId>(id), d);
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::sys::event
