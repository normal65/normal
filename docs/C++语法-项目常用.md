# C++ 语法速成：结合 Ebook 项目看

这份文档不是完整 C++ 教材，而是专门解释本项目里最常出现的写法。阅读顺序建议：

1. 先看每一节开头的“长什么样”。
2. 再看“什么意思”。
3. 最后看“本项目在哪里用”。

读代码时不要一开始就逐行查所有符号。先判断一行属于哪一类：

- 声明变量
- 声明函数
- 调用函数
- 判断条件
- 循环
- 访问对象成员
- 加锁
- 发送事件
- 请求刷新

判断完大类，再查具体名称就轻松很多。

## 0. 一个 C++ 程序由什么组成

最简单的 C++ 文件：

```cpp
#include <cstdint>

int add(int a, int b)
{
    return a + b;
}
```

这段代码包含：

- `#include`：引入别的头文件。
- `int`：返回类型。
- `add`：函数名。
- `int a, int b`：两个参数。
- `{}`：函数体。
- `return`：返回结果。

本项目里的代码也由这些基本零件构成，只是组合得更复杂。

## 1. `.hpp` 和 `.cc` 分别是什么

### `.hpp`

头文件，通常写：

- 类型声明
- 类定义
- 函数声明
- 常量
- 成员变量

例如：

```cpp
class App
{
  public:
    virtual void paint(gfx::Canvas& canvas) = 0;
};
```

### `.cc`

源文件，通常写：

```cpp
void App::paint(gfx::Canvas& canvas)
{
    // 具体实现
}
```

`App::paint` 表示：

```text
App 类里面的 paint 函数
```

### `#pragma once`

```cpp
#pragma once
```

意思是：

```text
这个头文件在一次编译中只包含一次
```

它防止重复定义错误。

## 2. 冒号 `::` 是什么意思

`::` 叫作用域解析符。

例如：

```cpp
app::ebook::ui::UiBus
```

它的意思是：

```text
app 命名空间
  -> ebook 命名空间
  -> ui 命名空间
  -> UiBus 类型
```

项目里的例子：

```cpp
UiBus& UiBus::get_instance()
```

意思是：

```text
实现 UiBus 类里面的 get_instance 函数
```

### `using`

如果每次写全名太长，可以用：

```cpp
using apps::AppRegistry;
```

意思是：

```text
以后在当前作用域直接写 AppRegistry
```

这不会创建新对象，只是名字引入。

也可以定义类型别名：

```cpp
using Func = std::function<void(void*)>;
```

意思是：

```text
Func 是 std::function<void(void*)> 的别名
```

## 3. namespace：给名字分组

```cpp
namespace app::ebook::ui
{

class UiBus
{
};

}
```

意思是：

```text
UiBus 的完整名字是 app::ebook::ui::UiBus
```

为什么要用：

- 避免不同模块重名。
- 让目录结构和代码作用域对应。
- 通过名字就能看出代码属于哪一层。

例如：

```text
app::ebook::ui::UiBus
app::ebook::router::Router
app::common::ota::OtaMgr
```

## 4. 变量、类型和初始化

### 声明变量

```cpp
uint8_t brightness = 0;
int count = 0;
bool ready = false;
```

格式：

```text
类型 变量名 = 初始值;
```

### 固定宽度类型

项目里常见：

```cpp
uint8_t
uint16_t
uint32_t
int32_t
size_t
```

意思：

- `uint8_t`：无符号 8 位整数，通常 0 到 255。
- `uint16_t`：无符号 16 位整数。
- `uint32_t`：无符号 32 位整数。
- `int32_t`：有符号 32 位整数。
- `size_t`：表示大小或下标，具体位宽由平台决定。

### `{}` 初始化

```cpp
UiEvent ev{};
```

意思是：

```text
创建 UiEvent 对象，并把成员初始化为 0 或默认值
```

比不初始化更安全。

### `const`

```cpp
const int max_count = 48;
```

意思是：

```text
max_count 不能在后面被修改
```

函数参数也常见：

```cpp
void paint(const gfx::Canvas& canvas);
```

表示：

```text
函数不会修改 canvas 的内容
```

### `constexpr`

```cpp
constexpr uint8_t kMaxEntries = 48;
```

意思是：

```text
编译阶段就能确定这个值
```

它比普通 `const` 更强：要求编译期常量。

### `static`

函数内：

```cpp
static UiBus s;
```

意思是：

```text
这个变量只初始化一次，函数多次调用时共用同一个对象
```

文件级：

```cpp
static const char* const TAG = "Router";
```

意思是：

```text
TAG 只在本文件里可见
```

### `auto`

```cpp
auto req = make_request();
```

意思是：

```text
让编译器根据右边表达式推断左边类型
```

它不是“没有类型”，运行时仍然是强类型。

## 5. 函数

### 声明

```cpp
bool init();
```

意思是：

```text
有一个 init 函数，返回 bool，没有参数
```

### 定义

```cpp
bool UiBus::init()
{
    queue_ = xQueueCreate(kQueueDepth, sizeof(UiEvent));
    return queue_ != nullptr;
}
```

包含：

- 返回类型：`bool`
- 所属类：`UiBus`
- 函数名：`init`
- 参数：无
- 函数体：创建队列
- 返回值：成功或失败

### 参数

```cpp
void post(const UiEvent& ev, uint32_t timeout_ms);
```

两个参数：

- `ev`：事件对象。
- `timeout_ms`：等待多少毫秒。

### 默认参数

```cpp
bool check_update(const std::string& server_url, int32_t timeout_ms = 10000);
```

意思是：

```text
调用时不传 timeout_ms，就使用 10000
```

可以写：

```cpp
check_update(url);
check_update(url, 5000);
```

### 函数重载

同名函数参数不同：

```cpp
void present(Present kind);
void present(Present kind, Window win);
```

编译器根据参数数量判断调用哪一个。

## 6. 类、对象、成员

### 类定义

```cpp
class UiBus
{
  public:
    bool init();
    bool post(const UiEvent& ev);

  private:
    QueueHandle_t queue_{nullptr};
};
```

包含：

- `public`：外面可以调用。
- `private`：只有类内部可以访问。
- `queue_`：成员变量。
- `init`、`post`：成员函数。

### 创建对象

```cpp
UiBus bus;
```

在栈上创建对象。

```cpp
auto bus = std::make_unique<UiBus>();
```

在堆上创建对象，由 `unique_ptr` 管理。

### 访问成员

对象本身用 `.`：

```cpp
bus.init();
```

对象指针用 `->`：

```cpp
bus->init();
```

记住：

```text
对象.成员
指针->成员
```

### `this`

类成员函数里的 `this` 指当前对象。

```cpp
list_.set_colors(colors_);
```

等价于：

```cpp
this->colors_
```

项目里的 lambda 常见：

```cpp
[this](uint8_t i) { on_row_tap(i); }
```

这里的 `this` 表示捕获当前对象，才能在 lambda 里访问成员函数。

## 7. 构造函数和析构函数

```cpp
FilesApp::FilesApp()
{
    // 构造对象
}
```

构造函数：

- 名字和类名相同。
- 没有返回类型。
- 创建对象时自动调用。

```cpp
FilesApp::~FilesApp()
{
    // 释放资源
}
```

析构函数：

- 名字是 `~类名`。
- 对象销毁时自动调用。
- 用来释放内存、停止任务、关闭句柄。

这就是 RAII 的基础。

## 8. 引用和指针

### 引用 `&`

```cpp
void paint(gfx::Canvas& canvas);
```

意思是：

```text
直接使用调用者传进来的 Canvas 对象
```

不复制对象，而且函数里修改会作用到原对象。

### `const` 引用

```cpp
void post(const UiEvent& ev);
```

意思是：

```text
不复制 ev，但函数不能修改它
```

这是 C++ 很常见的参数写法。

### 指针 `*`

```cpp
const RefreshIntent* override_intent
```

意思是：

```text
override_intent 是一个地址
可以指向对象，也可以是 nullptr
```

访问指针指向对象：

```cpp
override_intent->waveform
```

### `nullptr`

```cpp
WiFi* wifi = nullptr;
```

表示：

```text
这个指针现在不指向任何对象
```

不要使用空指针的成员。

## 9. 继承、virtual、override

基类：

```cpp
class App : public shell::Page
```

意思是：

```text
App 继承 Page
```

`App` 拥有 `Page` 的接口，并且可以增加自己的内容。

### 虚函数

```cpp
virtual void paint(gfx::Canvas& canvas) = 0;
```

`= 0` 表示纯虚函数：

```text
基类不实现，子类必须实现
```

### `override`

```cpp
void paint(gfx::Canvas& canvas) override;
```

意思是：

```text
我是在重写基类里的 paint
```

如果签名写错，编译器会报错，这能防止“看起来重写了，其实写了另一个函数”。

项目里所有 App 的：

- `paint()`
- `on_input()`
- `on_ui_event()`
- `on_enter()`
- `on_exit()`

基本都是这个模型。

## 10. enum class

```cpp
enum class AppId : uint8_t
{
    None = 0,
    Reader,
    Settings,
    Count
};
```

意思是：

```text
定义一组有名字的状态
```

使用时：

```cpp
AppId::Reader
```

不能用裸数字替代，减少错误。

项目里的例子：

```cpp
enum class Waveform
{
    Partial,
    Fast,
    Full
};
```

## 11. 模板

```cpp
std::vector<uint8_t> data;
```

`vector` 是模板，`uint8_t` 是它装的内容。

意思是：

```text
一个可以动态增长的 uint8_t 数组
```

项目里的：

```cpp
std::unique_ptr<::app::sys::task::Task> task_;
```

意思是：

```text
一个智能指针，专门管理 Task 对象
```

模板的重点：

```text
同一套代码，适配不同类型
```

## 12. std::unique_ptr

```cpp
std::unique_ptr<Task> task_;
```

它表示：

```text
独占所有权
```

特点：

- 一个对象只能有一个 `unique_ptr` 拥有。
- 析构时自动 `delete`。
- 不需要手写 `delete`。

创建：

```cpp
task_ = std::make_unique<Task>(cfg);
```

释放：

```cpp
task_.reset();
```

转移所有权：

```cpp
other = std::move(task_);
```

`std::move` 不是移动内存，而是表示“把所有权交给别人”。

## 13. std::function 和 lambda

### std::function

```cpp
using TapHandler = std::function<void(uint8_t index)>;
```

意思是：

```text
可以保存一个“接收 uint8_t，没有返回值”的函数
```

它让类可以把函数当成员保存。

### lambda

```cpp
list_.set_provider([this](uint8_t i, ui::widgets::RowStyle& rs) {
    fill_row(i, rs);
});
```

拆开看：

```text
[this]      捕获当前对象
(uint8_t i) 参数
{ ... }     函数体
```

`[this]` 是为了让 lambda 内部能调用当前对象的成员函数。

如果写成：

```cpp
[](){}
```

表示不捕获外部变量。

## 14. 常用标准库容器

### std::string

```cpp
std::string url = server_url + "/api/ota/check";
```

字符串对象，自动管理长度和内存。

### std::vector

```cpp
std::vector<uint8_t> body;
```

动态数组。

常用操作：

```cpp
body.push_back(0x01);
body.size();
body.clear();
body[0];
```

### std::map / std::unordered_map

```cpp
std::map<std::string, std::string> headers;
```

键值表：

```text
键 -> 值
```

`std::unordered_map` 查询通常更快，但不保证顺序。

## 15. 条件、循环、switch

### if

```cpp
if (queue_ == nullptr)
    return false;
```

如果队列还没创建，直接返回失败。

### else

```cpp
if (ready)
    start();
else
    wait();
```

### for

```cpp
for (uint8_t i = 0; i < count; ++i)
{
    process(i);
}
```

### range for

```cpp
for (const Entry& e : entries)
{
    draw(e);
}
```

意思是：

```text
逐个取出 entries 里的每一个 Entry
```

### while

```cpp
while (running_)
{
    do_one_tick();
}
```

条件为真时持续循环。

### switch

```cpp
switch (action)
{
    case NavAction::Back:
        back();
        break;
    case NavAction::Forward:
        forward();
        break;
    default:
        break;
}
```

`break` 很重要；忘记 `break` 会继续执行下一个 `case`。

### 三元表达式

```cpp
const uint32_t timeout = need_wait ? portMAX_DELAY : 0;
```

等价于：

```cpp
uint32_t timeout;
if (need_wait)
    timeout = portMAX_DELAY;
else
    timeout = 0;
```

## 16. 内存和 RAII

### 栈对象

```cpp
UiEvent ev{};
```

离开作用域自动销毁。

### 堆对象

```cpp
auto* ptr = new Task();
delete ptr;
```

需要手动释放，项目里尽量少用裸 `new/delete`。

### RAII

```cpp
std::lock_guard<std::mutex> lock(mutex_);
```

对象创建时加锁，离开作用域自动解锁。

```cpp
JsonRAII json(cJSON_Parse(text));
```

离开作用域自动释放 cJSON。

核心：

```text
资源跟着对象生命周期自动释放
```

项目里大量使用这种写法，避免忘记释放。

## 17. 多线程和锁

### std::mutex

```cpp
std::mutex mutex_;
```

互斥锁，保证同一时间只有一个任务能访问共享数据。

### lock_guard

```cpp
std::lock_guard<std::mutex> lock(mutex_);
```

进入作用域加锁，离开自动解锁。

### unique_lock

```cpp
std::unique_lock<std::mutex> lock(mutex_);
```

比 `lock_guard` 更灵活，可以中途解锁和重新加锁。

### atomic

```cpp
std::atomic<uint32_t> read_count{0};
```

表示：

```text
这个整数可以被多个任务安全地读写
```

它适合简单计数，不适合保护一整段复杂逻辑。

### mutable

```cpp
mutable std::mutex mutex_;
```

表示：

```text
即使当前方法标记为 const，也可以修改这个成员
```

锁本身不会被业务修改，所以常写成 `mutable`。

## 18. 类型转换

### static_cast

```cpp
static_cast<Presenter*>(arg)
```

意思是：

```text
把 void* 明确转换成 Presenter*
```

这是项目里最常见的转换。

### reinterpret_cast

```cpp
reinterpret_cast<uint8_t*>(buffer)
```

表示：

```text
按二进制重新解释这块内存
```

它会绕过类型检查，只有确实需要处理原始内存时使用。

不要随便把 `reinterpret_cast` 当成万能转换。

## 19. extern "C"

```cpp
extern "C" void app_main(void)
{
    ebook_app_run();
}
```

意思是：

```text
这个函数按 C 的符号规则导出
```

ESP-IDF 启动代码是 C，它需要找到 `app_main`。C++ 默认会做名称修饰，所以这里必须写 `extern "C"`。

## 20. 这个项目最常见的 10 种句式

### 单例

```cpp
UiBus& UiBus::get_instance()
{
    static UiBus s;
    return s;
}
```

### 发送事件

```cpp
UiBus::get_instance().post_tick_battery(pct, mv);
```

### 请求刷新

```cpp
request_repaint();
request_repaint(router::intent_partial_full());
```

### 调用 Router

```cpp
Router::instance().navigate(router::page_app(AppId::Reader));
```

### 加锁

```cpp
std::lock_guard<std::mutex> lock(mutex_);
```

### 创建后台任务

```cpp
auto task = std::make_unique<Task>(entry, cfg, this);
task->start();
```

### 注册回调

```cpp
list_.set_tap_handler([this](uint8_t i) { on_row_tap(i); });
```

### 读取配置

```cpp
Persist::get_u8("key", value);
```

### 判断失败并返回

```cpp
if (queue_ == nullptr)
    return false;
```

### 使用引用避免复制

```cpp
void paint(gfx::Canvas& canvas);
void post(const UiEvent& ev);
```

## 21. 初学看代码时容易混淆的点

### `&` 的三个意思

```cpp
int a = 1;
int& ref = a;               // 引用
int* ptr = &a;              // 取地址
if (a & 1) { }              // 按位与
```

### `*` 的三个意思

```cpp
int* ptr;                   // 指针类型
*ptr = 3;                   // 解引用
int a = 2 * 3;              // 乘法
```

### `.` 和 `->`

```cpp
object.method();            // 对象
pointer->method();          // 指针
```

### `::` 和 `.`

```cpp
AppRegistry::instance()     // 类型/命名空间里的静态成员
app.instance()              // 对象成员
```

### `=` 和 `==`

```cpp
a = 1;                      // 赋值
if (a == 1) { }             // 比较
```

### `const` 位置

```cpp
const int a = 1;            // a 不能改
void f() const;             // 这个成员函数不能改成员
void f(const T& value);     // 不修改传入对象
```

## 22. 建议的学习方法

不要一次背完整本 C++。按下面顺序练：

1. 看懂一个变量声明。
2. 看懂一个函数声明。
3. 找到这个函数的实现。
4. 找到谁调用它。
5. 判断它运行在哪个任务。
6. 判断它有没有加锁。
7. 判断它失败时返回什么。
8. 判断它最后有没有触发 UI 刷新。

这个项目里，掌握下面这几条链路已经能读懂大部分代码：

```text
App::paint()
-> Canvas
-> framebuffer
-> Presenter
```

```text
App::on_input()
-> UiBus
-> UiLoop
-> Router
```

```text
后台任务
-> UiBus
-> UiLoop
-> App::on_ui_event()
```

```text
AppRegistry
-> App::on_enter()
-> App::on_exit()
```

## 23. 一句话记忆表

| 写法 | 含义 |
| --- | --- |
| `Type name;` | 声明变量 |
| `Type& name` | 引用 |
| `Type* name` | 指针 |
| `const Type&` | 只读引用 |
| `Class::func()` | 类里的静态函数或实现 |
| `object.func()` | 对象成员 |
| `pointer->func()` | 指针成员 |
| `class A : public B` | A 继承 B |
| `virtual ... = 0` | 纯虚函数 |
| `override` | 重写基类函数 |
| `enum class` | 强类型枚举 |
| `auto` | 编译器推断类型 |
| `constexpr` | 编译期常量 |
| `static` | 静态或文件内可见 |
| `std::unique_ptr<T>` | 独占所有权的智能指针 |
| `std::function<...>` | 可保存的函数对象 |
| `[...](...){}` | lambda 匿名函数 |
| `std::vector<T>` | 动态数组 |
| `std::string` | 字符串 |
| `std::mutex` | 互斥锁 |
| `std::lock_guard` | 自动加锁解锁 |
| `std::atomic<T>` | 可安全共享的简单变量 |
| `static_cast<T>` | 常规显式转换 |
| `reinterpret_cast<T>` | 按二进制重新解释转换 |
| `extern "C"` | 用 C 规则导出符号 |

读代码时如果遇到不认识的写法，先回到这张表查类别，再去对应章节看解释。
