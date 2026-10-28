// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/ime/ime_engine.hpp
 * 作用：实现拼音输入法候选查询和候选字管理。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：上层通过 Router、UiBus 和 SystemState 协作，不直接操作硬件。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstddef>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::ime {

/**
 * @brief 拼音单字输入引擎（薄封装，隐藏 third_party/zh_pinyin 的 C API）
 *
 * P0 范围：仅单字全拼/模糊候选（zh_match_code_vague），不做词组匹配。
 * 数据源：/assets/ime/zh_pinyin.bin，启动时一次性读入 RAM，避免逐字符 fopen。
 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class ImeEngine
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 get_instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static ImeEngine& get_instance();

    // 调用 ImeEngine(const ImeEngine&) = delete;，执行这个函数并处理它的返回值或状态变化。
    ImeEngine(const ImeEngine&) = delete;
    // 赋值：把右侧 (const ImeEngine&) = delete 的结果保存到左侧 ImeEngine& operator。
    ImeEngine& operator=(const ImeEngine&) = delete;

    /** @brief 读取 zh_pinyin.bin 并注入解码器；重复调用直接返回 true */
    // 函数调用：bool init();；执行该函数并处理它的返回值或副作用。
    bool init();
    // 调用 deinit()：创建和销毁队列。
    void deinit();
    // 定义或声明函数 ready：参数在括号里，返回类型写在函数名前面。
    bool ready() const
    {
        // 返回 data_ != nullptr 给调用者，表示当前函数的执行结果。
        return data_ != nullptr;
    }

    /** @brief 清空拼音组合区（不影响已上屏文字） */
    // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    void reset();
    // 定义或声明函数 compose_empty：参数在括号里，返回类型写在函数名前面。
    bool compose_empty() const
    {
        // 返回 compose_len_ == 0 给调用者，表示当前函数的执行结果。
        return compose_len_ == 0;
    }
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* compose() const
    {
        // 返回 compose_ 给调用者，表示当前函数的执行结果。
        return compose_;
    }

    /** @brief 追加一个拼音字母（a-z）；组合区已满或非法字符返回 false */
    // 函数调用：bool push_ascii(char c);；执行该函数并处理它的返回值或副作用。
    bool push_ascii(char c);
    /** @brief 删除组合区最后一个字母 */
    // 函数调用：void pop_ascii();；执行该函数并处理它的返回值或副作用。
    void pop_ascii();

    /**
     * @brief 模糊匹配候选汉字（UTF-8，每字固定 3 字节，无 '\0' 分隔）
     * @param out_utf8   输出缓冲，容量 >= 3*max_chars+1
     * @param max_chars  期望候选字数上限
     * @return 实际候选字数（0 表示组合区为空或未命中）
     */
    // 函数调用：uint8_t query_candidates(char* out_utf8, uint8_t max_chars);；执行该函数并处理它的返回值或副作用。
    uint8_t query_candidates(char* out_utf8, uint8_t max_chars);

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 ImeEngine() = default;，执行这个函数并处理它的返回值或状态变化。
    ImeEngine() = default;

    // 固定参数 kComposeCap = 20：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kComposeCap = 20; // 对应第三方库 ZH_MAX_STRING_LENGTH

    // 执行语句：char compose_[kComposeCap + 1]{};；它会更新一次变量、对象或程序状态。
    char compose_[kComposeCap + 1]{};
    // 执行语句：uint8_t compose_len_{0};；它会更新一次变量、对象或程序状态。
    uint8_t compose_len_{0};
    // 执行语句：uint8_t* data_{nullptr};；它会更新一次变量、对象或程序状态。
    uint8_t* data_{nullptr};
    // 执行语句：size_t data_size_{0};；它会更新一次变量、对象或程序状态。
    size_t data_size_{0};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::ime
