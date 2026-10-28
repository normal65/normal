// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/overlays/keyboard.hpp
 * 作用：实现 overlays 模块中的 keyboard 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：覆盖层优先接收输入，打开和关闭会改变路由栈。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/layer.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::overlays {

// 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
enum class KeyboardLayer : uint8_t
{
    // 执行这条代码：Letters = 0,。它会更新变量、对象状态或调用链。
    Letters = 0,
    // 继续上一行的表达式或参数列表：Numbers,。
    Numbers,
    // 继续上一行的表达式或参数列表：Symbols,。
    Symbols,
    // 继续上一行的表达式或参数列表：Pinyin,。
    Pinyin, // 拼音（单字候选，来自 ime::ImeEngine）
};

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using KeyboardCallback = void (*)(const char* text, void* user_data);

// struct KeyboardConfig：把相关数据字段组合在一起，表示项目中的一个概念。
struct KeyboardConfig
{
    // 执行语句：KeyboardLayer default_layer{KeyboardLayer::Letters};；它会更新一次变量、对象或程序状态。
    KeyboardLayer default_layer{KeyboardLayer::Letters};
    // 执行语句：uint8_t max_len{32};；它会更新一次变量、对象或程序状态。
    uint8_t max_len{32};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* initial_text{nullptr};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* title{nullptr};
};

/** @brief 软键盘（Letters/Numbers/Symbols；open 时切换 Input Profile::Keyboard） */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class Keyboard : public ui::Layer
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static Keyboard& instance();

    // 函数定义/声明：调用 is_open()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
    bool is_open() const
    {
        // 返回 open_ 给调用者，表示当前函数的执行结果。
        return open_;
    }
    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* buffer() const
    {
        // 返回 buf_ 给调用者，表示当前函数的执行结果。
        return buf_;
    }

    // 赋值：把右侧 nullptr) 的结果保存到左侧 void open(const KeyboardConfig& cfg, KeyboardCallback cb, void* user。
    void open(const KeyboardConfig& cfg, KeyboardCallback cb, void* user = nullptr);
    // 调用 close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
    void close();

    /**
     * @brief 供拨码/拨轮调用：翻页浏览拼音候选字（Pinyin 层且有候选时才生效）。
     * @param forward true 翻到下一页，false 翻到上一页
     * @return 是否实际翻动了页（无更多候选、非拼音层等情况返回 false）
     */
    // 函数调用：bool scroll_candidates(bool forward);；执行该函数并处理它的返回值或副作用。
    bool scroll_candidates(bool forward);

    // 函数定义/声明：调用 visible()：状态栏是否显示由两件事决定： 当前 Shell 页面。 如果当前是 AppHost，还要问活动 App 的 wantsstatusbar()。。
    bool visible() const override
    {
        // 返回 open_ 给调用者，表示当前函数的执行结果。
        return open_;
    }
    // 定义或声明函数 modal：参数在括号里，返回类型写在函数名前面。
    bool modal() const override
    {
        // 返回 open_ 给调用者，表示当前函数的执行结果。
        return open_;
    }
    // 定义或声明函数 wants_status_bar：参数在括号里，返回类型写在函数名前面。
    bool wants_status_bar() const override
    {
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 函数调用：core::Rect bounds() const override;；执行该函数并处理它的返回值或副作用。
    core::Rect bounds() const override;

    // 调用 on_input(const::app::ebook::input::Event&ev)：文件 App 的输入总入口。它处理： 触摸按下、移动、抬起和滑动。 工具栏返回。 列表滚动节流。 弹窗点击优先于底层列表点击。。
    ui::InputResult on_input(const ::app::ebook::input::Event& ev) override;
    // 函数调用：void paint(gfx::Canvas& canvas) override;；执行该函数并处理它的返回值或副作用。
    void paint(gfx::Canvas& canvas) override;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 Keyboard() = default;，执行这个函数并处理它的返回值或状态变化。
    Keyboard() = default;

    // 固定参数 kBufCap = 64：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kBufCap = 64;
    // 候选条共 kCandCols 格：最左/最右各 1 格是翻页箭头，中间 kCandPageSize 格才是候选字。
    static constexpr uint8_t kCandCols = 8;
    // 固定参数 kCandPageSize = static_cast<uint8_t>(kCandCols - 2)：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kCandPageSize = static_cast<uint8_t>(kCandCols - 2);
    // 固定参数 kCandPoolCap = 32：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kCandPoolCap = 32; // 候选池缓存上限，供拨码/箭头翻页用

    // 函数调用：void buf_clear();；执行该函数并处理它的返回值或副作用。
    void buf_clear();
    // 函数调用：void buf_append(char c);；执行该函数并处理它的返回值或副作用。
    void buf_append(char c);
    // 函数调用：void buf_append_utf8_char(const char* utf8_3bytes);；执行该函数并处理它的返回值或副作用。
    void buf_append_utf8_char(const char* utf8_3bytes);
    // 函数调用：void buf_backspace();；执行该函数并处理它的返回值或副作用。
    void buf_backspace(); // UTF-8 安全退格（不会截断多字节字符）
    // 函数调用：void buf_set(const char* s);；执行该函数并处理它的返回值或副作用。
    void buf_set(const char* s);

    // 函数调用：uint8_t rows_of_current_layer() const;；执行该函数并处理它的返回值或副作用。
    uint8_t rows_of_current_layer() const;
    // 函数调用：char char_at(uint8_t row, uint8_t col) const;；执行该函数并处理它的返回值或副作用。
    char char_at(uint8_t row, uint8_t col) const;
    // 函数调用：bool is_shift_cell(uint8_t row, uint8_t col) const;；执行该函数并处理它的返回值或副作用。
    bool is_shift_cell(uint8_t row, uint8_t col) const;
    // 函数调用：bool is_space_cell(uint8_t row, uint8_t col) const;；执行该函数并处理它的返回值或副作用。
    bool is_space_cell(uint8_t row, uint8_t col) const;
    // 函数调用：bool is_backspace_cell(uint8_t row, uint8_t col) const;；执行该函数并处理它的返回值或副作用。
    bool is_backspace_cell(uint8_t row, uint8_t col) const;

    // 函数调用：core::Rect input_rect() const;；执行该函数并处理它的返回值或副作用。
    core::Rect input_rect() const;
    // 函数调用：core::Rect toolbar_rect() const;；执行该函数并处理它的返回值或副作用。
    core::Rect toolbar_rect() const;
    // 函数调用：core::Rect keys_rect() const;；执行该函数并处理它的返回值或副作用。
    core::Rect keys_rect() const;
    // 函数调用：core::Rect candidate_rect() const;；执行该函数并处理它的返回值或副作用。
    core::Rect candidate_rect() const;    // 仅 Pinyin 层有效
    // 函数调用：core::Rect letters_area_rect() const;；执行该函数并处理它的返回值或副作用。
    core::Rect letters_area_rect() const; // 字母网格实际绘制区域（Pinyin 层会让出候选条）

    // 函数调用：void paint_input(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_input(gfx::Canvas& c);
    // 函数调用：void paint_toolbar(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_toolbar(gfx::Canvas& c);
    // 函数调用：void paint_candidates(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_candidates(gfx::Canvas& c);
    // 函数调用：void paint_keys(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_keys(gfx::Canvas& c);
    // 函数调用：void draw_key(gfx::Canvas& c, const core::Rect& r, const char* label, bool inverted);；执行该函数并处理它的返回值或副作用。
    void draw_key(gfx::Canvas& c, const core::Rect& r, const char* label, bool inverted);

    // 函数调用：void refresh_candidates();；执行该函数并处理它的返回值或副作用。
    void refresh_candidates();
    // 函数调用：void enter_pinyin_layer();；执行该函数并处理它的返回值或副作用。
    void enter_pinyin_layer();
    // 函数调用：void leave_pinyin_layer();；执行该函数并处理它的返回值或副作用。
    void leave_pinyin_layer();
    // 函数调用：bool handle_pinyin_key_tap(uint8_t row, uint8_t col);；执行该函数并处理它的返回值或副作用。
    bool handle_pinyin_key_tap(uint8_t row, uint8_t col);
    // 函数调用：bool handle_candidate_tap(int16_t x, int16_t y);；执行该函数并处理它的返回值或副作用。
    bool handle_candidate_tap(int16_t x, int16_t y);

    // 执行语句：char buf_[kBufCap + 1]{};；它会更新一次变量、对象或程序状态。
    char buf_[kBufCap + 1]{};
    // 执行语句：uint8_t buf_len_{0};；它会更新一次变量、对象或程序状态。
    uint8_t buf_len_{0};
    // 执行语句：uint8_t max_len_{kBufCap};；它会更新一次变量、对象或程序状态。
    uint8_t max_len_{kBufCap};

    // 执行语句：char cand_buf_[kCandPoolCap * 3 + 1]{};；它会更新一次变量、对象或程序状态。
    char cand_buf_[kCandPoolCap * 3 + 1]{}; // 候选池，一次查询缓存多页
    // 执行语句：uint8_t cand_count_{0};；它会更新一次变量、对象或程序状态。
    uint8_t cand_count_{0};                 // 候选池中实际候选总数
    // 执行语句：uint8_t cand_offset_{0};；它会更新一次变量、对象或程序状态。
    uint8_t cand_offset_{0};                // 当前页在候选池中的起始下标

    // 执行语句：KeyboardLayer layer_{KeyboardLayer::Letters};；它会更新一次变量、对象或程序状态。
    KeyboardLayer layer_{KeyboardLayer::Letters};
    // 执行语句：bool upper_{false};；它会更新一次变量、对象或程序状态。
    bool upper_{false};
    // 执行语句：bool open_{false};；它会更新一次变量、对象或程序状态。
    bool open_{false};
    // 执行语句：KeyboardCallback cb_{nullptr};；它会更新一次变量、对象或程序状态。
    KeyboardCallback cb_{nullptr};
    // 执行语句：void* user_{nullptr};；它会更新一次变量、对象或程序状态。
    void* user_{nullptr};
    // 执行语句：char title_[48]{};；它会更新一次变量、对象或程序状态。
    char title_[48]{};
    // 执行语句：uint32_t open_gen_{0};；它会更新一次变量、对象或程序状态。
    uint32_t open_gen_{0};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::overlays
