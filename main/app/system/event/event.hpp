// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/system/event/event.hpp
 * 作用：封装 ESP-IDF 事件循环、事件注册和投递。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：事件 base 的身份必须长期稳定，不要指向临时字符串。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstddef>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <functional>

// 执行这条代码：extern "C"。它会更新变量、对象状态或调用链。
extern "C"
{
    // 定义类型别名：给已有类型起一个更容易理解的名字。
    typedef const char* esp_event_base_t;
}

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::sys::event {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using EventId = int32_t;

// struct EventData：把相关数据字段组合在一起，表示项目中的一个概念。
struct EventData
{
    // 执行语句：void* data;；它会更新一次变量、对象或程序状态。
    void* data;
    // 执行语句：size_t size;；它会更新一次变量、对象或程序状态。
    size_t size;

    // 定义或声明函数 EventData：参数在括号里，返回类型写在函数名前面。
    EventData()
        // 定义或声明函数 data：参数在括号里，返回类型写在函数名前面。
        : data(nullptr)
        // 定义或声明函数 size：参数在括号里，返回类型写在函数名前面。
        , size(0)
    {
    }
    // 定义或声明函数 EventData：参数在括号里，返回类型写在函数名前面。
    EventData(void* d, size_t s)
        // 定义或声明函数 data：参数在括号里，返回类型写在函数名前面。
        : data(d)
        // 定义或声明函数 size：参数在括号里，返回类型写在函数名前面。
        , size(s)
    {
    }
};

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using EventHandler = std::function<void(esp_event_base_t base, EventId id, const EventData& data)>;

/** @brief esp_event 默认循环 + C++ handler 映射 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class EventMgr
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static EventMgr& get_instance();

    // 函数调用：bool init();；执行该函数并处理它的返回值或副作用。
    bool init();
    // 调用 deinit()：创建和销毁队列。
    void deinit();

    // 调用 create_base(constchar*name)：身份必须长期稳定。
    esp_event_base_t create_base(const char* name);
    // 调用 register_handler(esp_event_base_tbase,EventIdid,EventHandlerhandler)：注册表键是 {base, id}。。
    bool register_handler(esp_event_base_t base, EventId id, EventHandler handler);
    // 调用 unregister_handler(esp_event_base_tbase,EventIdid)：注册表键是 {base, id}。。
    bool unregister_handler(esp_event_base_t base, EventId id) const;

    // 函数调用：bool post(esp_event_base_t base, EventId id, const EventData& data = EventData(),；执行该函数并处理它的返回值或副作用。
    bool post(esp_event_base_t base, EventId id, const EventData& data = EventData(),
              // 继续上一行的表达式或参数列表：uint32_t timeout_ms = 0) const;。
              uint32_t timeout_ms = 0) const;
    // 赋值：把右侧 EventData()) const 的结果保存到左侧 bool post_isr(esp_event_base_t base, EventId id, const EventData& data。
    bool post_isr(esp_event_base_t base, EventId id, const EventData& data = EventData()) const;

    // 定义或声明函数 is_init：参数在括号里，返回类型写在函数名前面。
    bool is_init() const
    {
        // 返回 init_ 给调用者，表示当前函数的执行结果。
        return init_;
    }

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 EventMgr();，执行这个函数并处理它的返回值或状态变化。
    EventMgr();
    // 函数调用：~EventMgr();；执行该函数并处理它的返回值或副作用。
    ~EventMgr();
    // 调用 EventMgr(const EventMgr&) = delete;，执行这个函数并处理它的返回值或状态变化。
    EventMgr(const EventMgr&) = delete;
    // 赋值：把右侧 (const EventMgr&) = delete 的结果保存到左侧 EventMgr& operator。
    EventMgr& operator=(const EventMgr&) = delete;

    // 静态函数 c_handler：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void c_handler(void* arg, esp_event_base_t base, int32_t id, void* data);

    // 执行语句：bool init_;；它会更新一次变量、对象或程序状态。
    bool init_;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::sys::event
