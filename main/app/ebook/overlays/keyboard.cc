/**
 * [初学者导读] 文件：main/app/ebook/overlays/keyboard.cc
 * 作用：实现 overlays 模块中的 keyboard 功能。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：覆盖层优先接收输入，打开和关闭会改变路由栈。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/keyboard.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ime/ime_engine.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/input_router.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/refresh_intent.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "router/router.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::overlays {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;

// 固定参数 kLetterRows = 6：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kLetterRows = 6;
// 固定参数 kNumberRows = 4：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kNumberRows = 4;
// 固定参数 kSymbolRows = 4：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kSymbolRows = 4;

// 字母页最后一行：z / Shift / 双格 Space / Backspace
constexpr char kLetters[kLetterRows][Theme::kKbCols + 1] = {"abcde", "fghij", "klmno",
                                                            // 继续上一行的表达式或参数列表："pqrst", "uvwxy", "z    "};。
                                                            "pqrst", "uvwxy", "z    "};

// 数字/符号页最后一列：@ _ / + 或 / : ; + Space + Backspace
constexpr char kNumbers[kNumberRows][Theme::kKbCols + 1] = {"12345", "67890", ".,!?-", "@_/  "};

// 静态查表数组 kSymbols：运行时复用固定数据，避免每次重新计算。
constexpr char kSymbols[kSymbolRows][Theme::kKbCols + 1] = {"@.-_!", "?#$%&", "*()+=", "/:;  "};

// 编译期函数 panel_y：常用于固定规则计算，能参与编译期优化。
constexpr uint16_t panel_y()
{
    // 返回 Theme::kStatusBarH 给调用者，表示当前函数的执行结果。
    return Theme::kStatusBarH;
}
// 编译期函数 panel_h：常用于固定规则计算，能参与编译期优化。
constexpr uint16_t panel_h()
{
    // 返回 Theme::kScreenH - Theme::kStatusBarH 给调用者，表示当前函数的执行结果。
    return Theme::kScreenH - Theme::kStatusBarH;
}
// 编译期函数 keys_y：常用于固定规则计算，能参与编译期优化。
constexpr uint16_t keys_y()
{
    // 返回 panel_y() + Theme::kKbInputH + Theme::kKbToolbarH 给调用者，表示当前函数的执行结果。
    return panel_y() + Theme::kKbInputH + Theme::kKbToolbarH;
}
// 编译期函数 keys_h：常用于固定规则计算，能参与编译期优化。
constexpr uint16_t keys_h()
{
    // 返回 Theme::kScreenH - keys_y() 给调用者，表示当前函数的执行结果。
    return Theme::kScreenH - keys_y();
}
// 编译期函数 col_w：常用于固定规则计算，能参与编译期优化。
constexpr uint16_t col_w()
{
    // 返回 Theme::kScreenW / Theme::kKbCols 给调用者，表示当前函数的执行结果。
    return Theme::kScreenW / Theme::kKbCols;
}

// 工具栏比字母格多一个「拼音」入口，独立列宽，与字符网格列宽解耦
constexpr uint8_t kToolbarCols = 6;
// 编译期函数 toolbar_col_w：常用于固定规则计算，能参与编译期优化。
constexpr uint16_t toolbar_col_w()
{
    // 返回 Theme::kScreenW / kToolbarCols 给调用者，表示当前函数的执行结果。
    return Theme::kScreenW / kToolbarCols;
}

// 固定参数 kCandidateH = 20：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kCandidateH = 20; // Pinyin 层候选条高度，从 keys_rect() 顶部让出

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 Keyboard::instance：参数在括号里，返回类型写在函数名前面。
Keyboard& Keyboard::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static Keyboard s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义或声明函数 Keyboard::bounds：参数在括号里，返回类型写在函数名前面。
core::Rect Keyboard::bounds() const
{
    // 返回 core::Rect{0, static_cast<int16_t>(panel_y()), Theme::kScreenW, panel_h()} 给调用者，表示当前函数的执行结果。
    return core::Rect{0, static_cast<int16_t>(panel_y()), Theme::kScreenW, panel_h()};
}

// 定义或声明函数 Keyboard::input_rect：参数在括号里，返回类型写在函数名前面。
core::Rect Keyboard::input_rect() const
{
    // 返回 core::Rect{0, static_cast<int16_t>(panel_y()), Theme::kScreenW, Theme::kKbInputH} 给调用者，表示当前函数的执行结果。
    return core::Rect{0, static_cast<int16_t>(panel_y()), Theme::kScreenW, Theme::kKbInputH};
}

// 定义或声明函数 Keyboard::toolbar_rect：参数在括号里，返回类型写在函数名前面。
core::Rect Keyboard::toolbar_rect() const
{
    // 返回 core::Rect{0, static_cast<int16_t>(panel_y() + Theme::kKbInputH), Theme::kScreenW, 给调用者，表示当前函数的执行结果。
    return core::Rect{0, static_cast<int16_t>(panel_y() + Theme::kKbInputH), Theme::kScreenW,
                      // 继续上一行的表达式或参数列表：Theme::kKbToolbarH};。
                      Theme::kKbToolbarH};
}

// 定义或声明函数 Keyboard::keys_rect：参数在括号里，返回类型写在函数名前面。
core::Rect Keyboard::keys_rect() const
{
    // 返回 core::Rect{0, static_cast<int16_t>(keys_y()), Theme::kScreenW, keys_h()} 给调用者，表示当前函数的执行结果。
    return core::Rect{0, static_cast<int16_t>(keys_y()), Theme::kScreenW, keys_h()};
}

// 定义或声明函数 Keyboard::candidate_rect：参数在括号里，返回类型写在函数名前面。
core::Rect Keyboard::candidate_rect() const
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect area = keys_rect();
    // 返回 core::Rect{area.x, area.y, area.w, kCandidateH} 给调用者，表示当前函数的执行结果。
    return core::Rect{area.x, area.y, area.w, kCandidateH};
}

// 定义或声明函数 Keyboard::letters_area_rect：参数在括号里，返回类型写在函数名前面。
core::Rect Keyboard::letters_area_rect() const
{
    // 定义变量 core::Rect area 并初始化为 keys_rect()。
    core::Rect area = keys_rect();
    // 判断条件：layer_ == KeyboardLayer::Pinyin 是否成立。
    if (layer_ == KeyboardLayer::Pinyin)
    {
        // 赋值：把右侧 static_cast<int16_t>(area.y + kCandidateH) 的结果保存到左侧 area.y。
        area.y = static_cast<int16_t>(area.y + kCandidateH);
        // 赋值：把右侧 static_cast<uint16_t>(area.h - kCandidateH) 的结果保存到左侧 area.h。
        area.h = static_cast<uint16_t>(area.h - kCandidateH);
    }
    // 返回 area 给调用者，表示当前函数的执行结果。
    return area;
}

// 定义或声明函数 Keyboard::buf_clear：参数在括号里，返回类型写在函数名前面。
void Keyboard::buf_clear()
{
    // 赋值：把右侧 '\0' 的结果保存到左侧 buf_[0]。
    buf_[0] = '\0';
    // 赋值：把右侧 0 的结果保存到左侧 buf_len_。
    buf_len_ = 0;
}

// 定义或声明函数 Keyboard::buf_append：参数在括号里，返回类型写在函数名前面。
void Keyboard::buf_append(char c)
{
    // 判断条件：buf_len_ >= max_len_ 是否成立。
    if (buf_len_ >= max_len_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 c 的结果保存到左侧 buf_[buf_len_++]。
    buf_[buf_len_++] = c;
    // 赋值：把右侧 '\0' 的结果保存到左侧 buf_[buf_len_]。
    buf_[buf_len_] = '\0';
}

// 定义或声明函数 Keyboard::buf_backspace：参数在括号里，返回类型写在函数名前面。
void Keyboard::buf_backspace()
{
    // UTF-8 安全退格：先退到本字符起始字节（跳过 0x80-0xBF 续字节），
    // 避免中文候选上屏后退格截断成半个字导致后续渲染乱码。
    if (buf_len_ == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 定义变量 uint8_t pos 并初始化为 static_cast<uint8_t>(buf_len_ - 1)。
    uint8_t pos = static_cast<uint8_t>(buf_len_ - 1);
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (pos > 0 && (static_cast<uint8_t>(buf_[pos]) & 0xC0U) == 0x80U)
        // 执行语句：--pos;；它会更新一次变量、对象或程序状态。
        --pos;
    // 赋值：把右侧 pos 的结果保存到左侧 buf_len_。
    buf_len_ = pos;
    // 赋值：把右侧 '\0' 的结果保存到左侧 buf_[pos]。
    buf_[pos] = '\0';
}

// 定义或声明函数 Keyboard::buf_append_utf8_char：参数在括号里，返回类型写在函数名前面。
void Keyboard::buf_append_utf8_char(const char* utf8_3bytes)
{
    // 判断条件：utf8_3bytes == nullptr 是否成立。
    if (utf8_3bytes == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：static_cast<uint16_t>(buf_len_) + 3U > max_len_ 是否成立。
    if (static_cast<uint16_t>(buf_len_) + 3U > max_len_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 utf8_3bytes[0] 的结果保存到左侧 buf_[buf_len_++]。
    buf_[buf_len_++] = utf8_3bytes[0];
    // 赋值：把右侧 utf8_3bytes[1] 的结果保存到左侧 buf_[buf_len_++]。
    buf_[buf_len_++] = utf8_3bytes[1];
    // 赋值：把右侧 utf8_3bytes[2] 的结果保存到左侧 buf_[buf_len_++]。
    buf_[buf_len_++] = utf8_3bytes[2];
    // 赋值：把右侧 '\0' 的结果保存到左侧 buf_[buf_len_]。
    buf_[buf_len_] = '\0';
}

// 定义或声明函数 Keyboard::buf_set：参数在括号里，返回类型写在函数名前面。
void Keyboard::buf_set(const char* s)
{
    // 调用 buf_clear();，执行这个函数并处理它的返回值或状态变化。
    buf_clear();
    // 判断条件：s == nullptr 是否成立。
    if (s == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (*s != '\0' && buf_len_ < max_len_)
        // 赋值：把右侧 *s++ 的结果保存到左侧 buf_[buf_len_++]。
        buf_[buf_len_++] = *s++;
    // 赋值：把右侧 '\0' 的结果保存到左侧 buf_[buf_len_]。
    buf_[buf_len_] = '\0';
}

// 函数定义/声明：调用 Keyboard::open()：open()： 1. 设置图层、大小写、回调和最大长度。 2. 复制标题。 3. 设置初始文本。 4. 增加 opengen。 5. 打开始终为 true。 6. 进入或退出拼音引擎。 7. 把 InputRouter 切到 Keyboard Profile。 8. 请求刷新。。
void Keyboard::open(const KeyboardConfig& cfg, KeyboardCallback cb, void* user)
{
    // 赋值：把右侧 cfg.default_layer 的结果保存到左侧 layer_。
    layer_ = cfg.default_layer;
    // 赋值：把右侧 false 的结果保存到左侧 upper_。
    upper_ = false;
    // 赋值：把右侧 cb 的结果保存到左侧 cb_。
    cb_ = cb;
    // 赋值：把右侧 user 的结果保存到左侧 user_。
    user_ = user;
    // 赋值：把右侧 (cfg.max_len > kBufCap) ? kBufCap : cfg.max_len 的结果保存到左侧 max_len_。
    max_len_ = (cfg.max_len > kBufCap) ? kBufCap : cfg.max_len;

    // 判断条件：cfg.title != nullptr 是否成立。
    if (cfg.title != nullptr)
    {
        // 调用 std::strncpy(title_, cfg.title, sizeof(title_) - 1);，执行这个函数并处理它的返回值或状态变化。
        std::strncpy(title_, cfg.title, sizeof(title_) - 1);
        // 赋值：把右侧 '\0' 的结果保存到左侧 title_[sizeof(title_) - 1]。
        title_[sizeof(title_) - 1] = '\0';
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 赋值：把右侧 '\0' 的结果保存到左侧 title_[0]。
        title_[0] = '\0';
    }
    // 调用 buf_set(cfg.initial_text);，执行这个函数并处理它的返回值或状态变化。
    buf_set(cfg.initial_text);
    // 执行语句：++open_gen_;；它会更新一次变量、对象或程序状态。
    ++open_gen_;
    // 赋值：把右侧 true 的结果保存到左侧 open_。
    open_ = true;

    // 判断条件：layer_ == KeyboardLayer::Pinyin 是否成立。
    if (layer_ == KeyboardLayer::Pinyin)
        // 调用 enter_pinyin_layer();，执行这个函数并处理它的返回值或状态变化。
        enter_pinyin_layer();
    // 备用分支：前面的条件不成立时执行这里。
    else
        // 调用 leave_pinyin_layer();，执行这个函数并处理它的返回值或状态变化。
        leave_pinyin_layer();

    // 函数调用：::app::ebook::input::InputRouter::get_instance().set_profile(；执行该函数并处理它的返回值或副作用。
    ::app::ebook::input::InputRouter::get_instance().set_profile(
        // 继续上一行的表达式或参数列表：::app::ebook::input::Profile::Keyboard);。
        ::app::ebook::input::Profile::Keyboard);

    // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
    (void)router::Router::instance().repaint(router::intent_partial_full());
}

// 函数定义/声明：调用 Keyboard::close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
void Keyboard::close()
{
    // 赋值：把右侧 false 的结果保存到左侧 open_。
    open_ = false;
    // 赋值：把右侧 nullptr 的结果保存到左侧 cb_。
    cb_ = nullptr;
    // 赋值：把右侧 nullptr 的结果保存到左侧 user_。
    user_ = nullptr;
    // 赋值：把右侧 '\0' 的结果保存到左侧 title_[0]。
    title_[0] = '\0';
    // 调用 buf_clear();，执行这个函数并处理它的返回值或状态变化。
    buf_clear();
    // 调用 leave_pinyin_layer();，执行这个函数并处理它的返回值或状态变化。
    leave_pinyin_layer();

    // 函数调用：::app::ebook::input::InputRouter::get_instance().set_profile(；执行该函数并处理它的返回值或副作用。
    ::app::ebook::input::InputRouter::get_instance().set_profile(
        // 继续上一行的表达式或参数列表：::app::ebook::input::Profile::Normal);。
        ::app::ebook::input::Profile::Normal);

    // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
    (void)router::Router::instance().repaint(router::intent_partial_full());
}

// 定义或声明函数 Keyboard::rows_of_current_layer：参数在括号里，返回类型写在函数名前面。
uint8_t Keyboard::rows_of_current_layer() const
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (layer_)
    {
        // 分支匹配：case KeyboardLayer::Letters 成立时执行下面的代码。
        case KeyboardLayer::Letters:
            // 返回 kLetterRows 给调用者，表示当前函数的执行结果。
            return kLetterRows;
        // 分支匹配：case KeyboardLayer::Numbers 成立时执行下面的代码。
        case KeyboardLayer::Numbers:
            // 返回 kNumberRows 给调用者，表示当前函数的执行结果。
            return kNumberRows;
        // 分支匹配：case KeyboardLayer::Symbols 成立时执行下面的代码。
        case KeyboardLayer::Symbols:
            // 返回 kSymbolRows 给调用者，表示当前函数的执行结果。
            return kSymbolRows;
        // 分支匹配：case KeyboardLayer::Pinyin 成立时执行下面的代码。
        case KeyboardLayer::Pinyin:
            // 返回 kLetterRows 给调用者，表示当前函数的执行结果。
            return kLetterRows; // 复用字母网格（拼音全小写）
    }
    // 返回 0 给调用者，表示当前函数的执行结果。
    return 0;
}

// 定义或声明函数 Keyboard::is_shift_cell：参数在括号里，返回类型写在函数名前面。
bool Keyboard::is_shift_cell(uint8_t row, uint8_t col) const
{
    // 返回 layer_ == KeyboardLayer::Letters && row == kLetterRows - 1 && col == 1 给调用者，表示当前函数的执行结果。
    return layer_ == KeyboardLayer::Letters && row == kLetterRows - 1 && col == 1;
}

// 定义或声明函数 Keyboard::is_space_cell：参数在括号里，返回类型写在函数名前面。
bool Keyboard::is_space_cell(uint8_t row, uint8_t col) const
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t last = static_cast<uint8_t>(rows_of_current_layer() - 1);
    // 判断条件：row != last 是否成立。
    if (row != last)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：layer_ == KeyboardLayer::Pinyin 是否成立。
    if (layer_ == KeyboardLayer::Pinyin)
        // 返回 col == 1 || col == 2 给调用者，表示当前函数的执行结果。
        return col == 1 || col == 2;
    // 判断条件：layer_ == KeyboardLayer::Letters 是否成立。
    if (layer_ == KeyboardLayer::Letters)
        // 返回 col == 2 || col == 3 给调用者，表示当前函数的执行结果。
        return col == 2 || col == 3;
    // 返回 col == 3 给调用者，表示当前函数的执行结果。
    return col == 3;
}

// 定义或声明函数 Keyboard::is_backspace_cell：参数在括号里，返回类型写在函数名前面。
bool Keyboard::is_backspace_cell(uint8_t row, uint8_t col) const
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t last = static_cast<uint8_t>(rows_of_current_layer() - 1);
    // 判断条件：row != last 是否成立。
    if (row != last)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 判断条件：layer_ == KeyboardLayer::Pinyin 是否成立。
    if (layer_ == KeyboardLayer::Pinyin)
        // 返回 col == 3 || col == 4 给调用者，表示当前函数的执行结果。
        return col == 3 || col == 4;
    // 返回 col == 4 给调用者，表示当前函数的执行结果。
    return col == 4;
}

// 定义或声明函数 Keyboard::char_at：参数在括号里，返回类型写在函数名前面。
char Keyboard::char_at(uint8_t row, uint8_t col) const
{
    // 判断条件：row >= rows_of_current_layer() || col >= Theme::kKbCols 是否成立。
    if (row >= rows_of_current_layer() || col >= Theme::kKbCols)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    // 判断条件：is_shift_cell(row, col) || is_space_cell(row, col) || is_backspace_cell(row, col) 是否成立。
    if (is_shift_cell(row, col) || is_space_cell(row, col) || is_backspace_cell(row, col))
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;

    // 定义变量 char ch 并初始化为 0。
    char ch = 0;
    // 多分支判断：根据括号里的值选择不同 case。
    switch (layer_)
    {
        // 分支匹配：case KeyboardLayer::Letters 成立时执行下面的代码。
        case KeyboardLayer::Letters:
            // 赋值：把右侧 kLetters[row][col] 的结果保存到左侧 ch。
            ch = kLetters[row][col];
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case KeyboardLayer::Numbers 成立时执行下面的代码。
        case KeyboardLayer::Numbers:
            // 赋值：把右侧 kNumbers[row][col] 的结果保存到左侧 ch。
            ch = kNumbers[row][col];
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case KeyboardLayer::Symbols 成立时执行下面的代码。
        case KeyboardLayer::Symbols:
            // 赋值：把右侧 kSymbols[row][col] 的结果保存到左侧 ch。
            ch = kSymbols[row][col];
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case KeyboardLayer::Pinyin 成立时执行下面的代码。
        case KeyboardLayer::Pinyin:
            // 赋值：把右侧 kLetters[row][col] 的结果保存到左侧 ch。
            ch = kLetters[row][col];
            // 跳出当前循环或 switch，不再执行剩余分支。
            break; // 拼音无大小写
    }
    // 判断条件：ch == ' ' || ch == 0 是否成立。
    if (ch == ' ' || ch == 0)
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    // 判断条件：layer_ == KeyboardLayer::Letters && upper_ 是否成立。
    if (layer_ == KeyboardLayer::Letters && upper_)
        // 赋值：把右侧 static_cast<char>(ch - 'a' + 'A') 的结果保存到左侧 ch。
        ch = static_cast<char>(ch - 'a' + 'A');
    // 返回 ch 给调用者，表示当前函数的执行结果。
    return ch;
}

// 定义或声明函数 Keyboard::draw_key：参数在括号里，返回类型写在函数名前面。
void Keyboard::draw_key(gfx::Canvas& c, const core::Rect& r, const char* label, bool inverted)
{
    // 调用 rect(r)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    c.rect(r);
    // 判断条件：label == nullptr || label[0] == '\0' 是否成立。
    if (label == nullptr || label[0] == '\0')
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 声明并默认初始化对象：gfx::Canvas::TextStyle s{}。
    gfx::Canvas::TextStyle s{};
    // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 s.size_px。
    s.size_px = Theme::kFontBody;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 s.h。
    s.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 s.v。
    s.v = gfx::VAlign::Middle;
    // 赋值：把右侧 2 的结果保存到左侧 s.padding。
    s.padding = 2;
    // 调用 text_in(r,label,s)：文本对齐。
    c.text_in(r, label, s);

    // 判断条件：inverted 是否成立。
    if (inverted)
        // 访问对象成员：c.invert(r);；读取或修改这个对象里的成员状态。
        c.invert(r);
}

// 定义或声明函数 Keyboard::paint_input：参数在括号里，返回类型写在函数名前面。
void Keyboard::paint_input(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect r = input_rect();
    // 访问对象成员：c.fill(r, gfx::Ink::White);；读取或修改这个对象里的成员状态。
    c.fill(r, gfx::Ink::White);
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    c.rect(core::Rect{static_cast<int16_t>(r.x + Theme::kPadLg), static_cast<int16_t>(r.y + 4),
                      // 继续上一行的表达式或参数列表：static_cast<uint16_t>(r.w - Theme::kPadLg * 2),。
                      static_cast<uint16_t>(r.w - Theme::kPadLg * 2),
                      // 继续上一行的表达式或参数列表：static_cast<uint16_t>(r.h - 8)});。
                      static_cast<uint16_t>(r.h - 8)});

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t tx = static_cast<int16_t>(r.x + Theme::kPadLg + 4);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t ty = static_cast<int16_t>(r.y + (r.h - Theme::kFontBody) / 2);

    // 拼音组合中：已上屏文字照常显示，未提交的拼音字母临时追加在后面
    // 并加下划线，提示这段还没有转成汉字（类似手机输入法的预编辑态）。
    const char* compose = nullptr;
    // 判断条件：layer_ == KeyboardLayer::Pinyin 是否成立。
    if (layer_ == KeyboardLayer::Pinyin)
    {
        // 赋值：把右侧 ime::ImeEngine::get_instance() 的结果保存到左侧 auto& engine。
        auto& engine = ime::ImeEngine::get_instance();
        // 判断条件：!engine.compose_empty() 是否成立。
        if (!engine.compose_empty())
            // 赋值：把右侧 engine.compose() 的结果保存到左侧 compose。
            compose = engine.compose();
    }

    // 判断条件：compose != nullptr 是否成立。
    if (compose != nullptr)
    {
        // 定义变量 int16_t x 并初始化为 tx。
        int16_t x = tx;
        // 判断条件：buf_len_ > 0 是否成立。
        if (buf_len_ > 0)
            // 赋值：把右侧 static_cast<int16_t>(x + c.text(x, ty, buf_, Theme::kFontBody)) 的结果保存到左侧 x。
            x = static_cast<int16_t>(x + c.text(x, ty, buf_, Theme::kFontBody));
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint16_t cw = c.text(x, ty, compose, Theme::kFontBody);
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        c.hline(x, static_cast<int16_t>(ty + Theme::kFontBody), cw);
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* show = (buf_len_ == 0) ? title_ : buf_;
    // 判断条件：show != nullptr && show[0] != '\0' 是否成立。
    if (show != nullptr && show[0] != '\0')
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t fs = (buf_len_ == 0) ? Theme::kFontSmall : Theme::kFontBody;
        // 调用 text(tx,ty,show,fs)：glyph()： 接收一个 Unicode 码点，委托 Font::drawglyph() 画一个字形，并返回该字形的推进宽度。。
        c.text(tx, ty, show, fs);
    }
}

// 定义或声明函数 Keyboard::paint_toolbar：参数在括号里，返回类型写在函数名前面。
void Keyboard::paint_toolbar(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect bar = toolbar_rect();
    // 访问对象成员：c.fill(bar, gfx::Ink::White);；读取或修改这个对象里的成员状态。
    c.fill(bar, gfx::Ink::White);
    // 访问对象成员：c.hline(0, bar.y, Theme::kScreenW);；读取或修改这个对象里的成员状态。
    c.hline(0, bar.y, Theme::kScreenW);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t cw = toolbar_col_w();

    // struct Btn：把相关数据字段组合在一起，表示项目中的一个概念。
    struct Btn
    {
        // 执行语句：core::Rect r;；它会更新一次变量、对象或程序状态。
        core::Rect r;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* lbl;
        // 执行语句：bool active;；它会更新一次变量、对象或程序状态。
        bool active;
    };
    // 只读约束：这个值或对象在当前位置不能被修改。
    const Btn btns[] = {
        // 执行这条代码：{{0, bar.y, cw, bar.h}, ui::strings::kKbExit, false},。它会更新变量、对象状态或调用链。
        {{0, bar.y, cw, bar.h}, ui::strings::kKbExit, false},
        // 继续上一行的表达式或参数列表：{{static_cast<int16_t>(cw), bar.y, cw, bar.h},。
        {{static_cast<int16_t>(cw), bar.y, cw, bar.h},
         // 继续上一行的表达式或参数列表：ui::strings::kKb123,。
         ui::strings::kKb123,
         // 继续上一行的表达式或参数列表：layer_ == KeyboardLayer::Numbers},。
         layer_ == KeyboardLayer::Numbers},
        // 继续上一行的表达式或参数列表：{{static_cast<int16_t>(cw * 2), bar.y, cw, bar.h},。
        {{static_cast<int16_t>(cw * 2), bar.y, cw, bar.h},
         // 继续上一行的表达式或参数列表：ui::strings::kKbABC,。
         ui::strings::kKbABC,
         // 继续上一行的表达式或参数列表：layer_ == KeyboardLayer::Letters},。
         layer_ == KeyboardLayer::Letters},
        // 继续上一行的表达式或参数列表：{{static_cast<int16_t>(cw * 3), bar.y, cw, bar.h},。
        {{static_cast<int16_t>(cw * 3), bar.y, cw, bar.h},
         // 继续上一行的表达式或参数列表：ui::strings::kKbSymbol,。
         ui::strings::kKbSymbol,
         // 继续上一行的表达式或参数列表：layer_ == KeyboardLayer::Symbols},。
         layer_ == KeyboardLayer::Symbols},
        // 继续上一行的表达式或参数列表：{{static_cast<int16_t>(cw * 4), bar.y, cw, bar.h},。
        {{static_cast<int16_t>(cw * 4), bar.y, cw, bar.h},
         // 继续上一行的表达式或参数列表：ui::strings::kKbPinyin,。
         ui::strings::kKbPinyin,
         // 继续上一行的表达式或参数列表：layer_ == KeyboardLayer::Pinyin},。
         layer_ == KeyboardLayer::Pinyin},
        // 继续上一行的表达式或参数列表：{{static_cast<int16_t>(cw * 5), bar.y, cw, bar.h}, ui::strings::kKbDone, false},。
        {{static_cast<int16_t>(cw * 5), bar.y, cw, bar.h}, ui::strings::kKbDone, false},
    };
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (const Btn& b : btns)
        // 调用 draw_key(c, b.r, b.lbl, b.active);，执行这个函数并处理它的返回值或状态变化。
        draw_key(c, b.r, b.lbl, b.active);
}

// 定义或声明函数 Keyboard::paint_candidates：参数在括号里，返回类型写在函数名前面。
void Keyboard::paint_candidates(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect area = candidate_rect();
    // 访问对象成员：c.fill(area, gfx::Ink::White);；读取或修改这个对象里的成员状态。
    c.fill(area, gfx::Ink::White);
    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    c.hline(area.x, static_cast<int16_t>(area.y + area.h - 1), area.w);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t cw = static_cast<uint16_t>(area.w / kCandCols);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool has_prev = cand_offset_ > 0;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const bool has_next = static_cast<uint8_t>(cand_offset_ + kCandPageSize) < cand_count_;

    // 访问对象成员：core::Rect prev_cell{area.x, area.y, cw, area.h};；读取或修改这个对象里的成员状态。
    core::Rect prev_cell{area.x, area.y, cw, area.h};
    // 调用 draw_key(c, prev_cell, has_prev ? "<" : "", false);，执行这个函数并处理它的返回值或状态变化。
    draw_key(c, prev_cell, has_prev ? "<" : "", false);

    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    core::Rect next_cell{static_cast<int16_t>(area.x + cw * (kCandCols - 1)), area.y, cw, area.h};
    // 调用 draw_key(c, next_cell, has_next ? ">" : "", false);，执行这个函数并处理它的返回值或状态变化。
    draw_key(c, next_cell, has_next ? ">" : "", false);

    // 判断条件：cand_count_ == 0 是否成立。
    if (cand_count_ == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t remain = static_cast<uint8_t>(cand_count_ - cand_offset_);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t visible = (remain < kCandPageSize) ? remain : kCandPageSize;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < visible; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t idx = static_cast<uint8_t>(cand_offset_ + i);
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        core::Rect cell{static_cast<int16_t>(area.x + cw * (i + 1)), area.y, cw, area.h};
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char label[4] = {cand_buf_[idx * 3], cand_buf_[idx * 3 + 1], cand_buf_[idx * 3 + 2],
                               // 继续上一行的表达式或参数列表：'\0'};。
                               '\0'};
        // 调用 draw_key(c, cell, label, false);，执行这个函数并处理它的返回值或状态变化。
        draw_key(c, cell, label, false);
    }
}

// 定义或声明函数 Keyboard::paint_keys：参数在括号里，返回类型写在函数名前面。
void Keyboard::paint_keys(gfx::Canvas& c)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect area = letters_area_rect();
    // 访问对象成员：c.fill(area, gfx::Ink::White);；读取或修改这个对象里的成员状态。
    c.fill(area, gfx::Ink::White);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t rows = rows_of_current_layer();
    // 判断条件：rows == 0 是否成立。
    if (rows == 0)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t row_h = static_cast<uint16_t>(area.h / rows);

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t row = 0; row < rows; ++row)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t y = static_cast<int16_t>(area.y + row * row_h);
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint8_t col = 0; col < Theme::kKbCols; ++col)
        {
            // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
            core::Rect cell{static_cast<int16_t>(col * col_w()), y, col_w(), row_h};

            // 判断条件：is_shift_cell(row, col) 是否成立。
            if (is_shift_cell(row, col))
            {
                // 调用 draw_key(c, cell, upper_ ? "AA" : "Aa", upper_);，执行这个函数并处理它的返回值或状态变化。
                draw_key(c, cell, upper_ ? "AA" : "Aa", upper_);
                // 继续下一轮：跳过本次循环剩余代码。
                continue;
            }
            // 判断条件：is_space_cell(row, col) 是否成立。
            if (is_space_cell(row, col))
            {
                // 判断条件：layer_ == KeyboardLayer::Pinyin 是否成立。
                if (layer_ == KeyboardLayer::Pinyin)
                {
                    // 判断条件：col == 1 是否成立。
                    if (col == 1)
                    {
                        // 赋值：把右侧 static_cast<uint16_t>(cell.w * 2) 的结果保存到左侧 cell.w。
                        cell.w = static_cast<uint16_t>(cell.w * 2);
                        // 调用 draw_key(c, cell, ui::strings::kKbSpace, false);，执行这个函数并处理它的返回值或状态变化。
                        draw_key(c, cell, ui::strings::kKbSpace, false);
                    }
                }
                // 备用分支：前面的条件不成立时执行这里。
                else if (layer_ == KeyboardLayer::Letters)
                {
                    // 判断条件：col == 2 是否成立。
                    if (col == 2)
                    {
                        // 赋值：把右侧 static_cast<uint16_t>(cell.w * 2) 的结果保存到左侧 cell.w。
                        cell.w = static_cast<uint16_t>(cell.w * 2);
                        // 调用 draw_key(c, cell, ui::strings::kKbSpace, false);，执行这个函数并处理它的返回值或状态变化。
                        draw_key(c, cell, ui::strings::kKbSpace, false);
                    }
                }
                // 备用分支：前面的条件不成立时执行这里。
                else
                {
                    // 调用 draw_key(c, cell, ui::strings::kKbSpace, false);，执行这个函数并处理它的返回值或状态变化。
                    draw_key(c, cell, ui::strings::kKbSpace, false);
                }
                // 继续下一轮：跳过本次循环剩余代码。
                continue;
            }
            // 判断条件：is_backspace_cell(row, col) 是否成立。
            if (is_backspace_cell(row, col))
            {
                // 判断条件：layer_ == KeyboardLayer::Pinyin 是否成立。
                if (layer_ == KeyboardLayer::Pinyin)
                {
                    // 判断条件：col == 3 是否成立。
                    if (col == 3)
                    {
                        // 赋值：把右侧 static_cast<uint16_t>(cell.w * 2) 的结果保存到左侧 cell.w。
                        cell.w = static_cast<uint16_t>(cell.w * 2);
                        // 调用 draw_key(c, cell, "\u2190", false);，执行这个函数并处理它的返回值或状态变化。
                        draw_key(c, cell, "\u2190", false);
                    }
                }
                // 备用分支：前面的条件不成立时执行这里。
                else
                {
                    // 调用 draw_key(c, cell, "\u2190", false);，执行这个函数并处理它的返回值或状态变化。
                    draw_key(c, cell, "\u2190", false);
                }
                // 继续下一轮：跳过本次循环剩余代码。
                continue;
            }

            // 只读约束：这个值或对象在当前位置不能被修改。
            const char ch = char_at(row, col);
            // 判断条件：ch == 0 是否成立。
            if (ch == 0)
                // 继续下一轮：跳过本次循环剩余代码。
                continue;
            // 只读约束：这个值或对象在当前位置不能被修改。
            const char label[2] = {ch, '\0'};
            // 调用 draw_key(c, cell, label, false);，执行这个函数并处理它的返回值或状态变化。
            draw_key(c, cell, label, false);
        }
    }
}

// 定义或声明函数 Keyboard::paint：参数在括号里，返回类型写在函数名前面。
void Keyboard::paint(gfx::Canvas& c)
{
    // 判断条件：!open_ 是否成立。
    if (!open_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect b = bounds();
    // 访问对象成员：c.fill(b, gfx::Ink::White);；读取或修改这个对象里的成员状态。
    c.fill(b, gfx::Ink::White);
    // 调用 paint_input(c);，执行这个函数并处理它的返回值或状态变化。
    paint_input(c);
    // 调用 paint_toolbar(c);，执行这个函数并处理它的返回值或状态变化。
    paint_toolbar(c);
    // 判断条件：layer_ == KeyboardLayer::Pinyin 是否成立。
    if (layer_ == KeyboardLayer::Pinyin)
        // 调用 paint_candidates(c);，执行这个函数并处理它的返回值或状态变化。
        paint_candidates(c);
    // 调用 paint_keys(c);，执行这个函数并处理它的返回值或状态变化。
    paint_keys(c);
    // 调用 rect(b)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
    c.rect(b);
}

// 定义或声明函数 Keyboard::on_input：参数在括号里，返回类型写在函数名前面。
ui::InputResult Keyboard::on_input(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;
    // 判断条件：!open_ 是否成立。
    if (!open_)
        // 返回 {} 给调用者，表示当前函数的执行结果。
        return {};
    // 判断条件：ev.type != EventType::Tap 是否成立。
    if (ev.type != EventType::Tap)
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x = static_cast<int16_t>(ev.x);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y = static_cast<int16_t>(ev.y);

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect tb = toolbar_rect();
    // 判断条件：tb.contains(x, y) 是否成立。
    if (tb.contains(x, y))
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t col = static_cast<uint8_t>(x / toolbar_col_w());
        // 多分支判断：根据括号里的值选择不同 case。
        switch (col)
        {
            // 分支匹配：case 0 成立时执行下面的代码。
            case 0:
                // 调用 close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
                close();
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            // 分支匹配：case 1 成立时执行下面的代码。
            case 1:
                // 调用 leave_pinyin_layer();，执行这个函数并处理它的返回值或状态变化。
                leave_pinyin_layer();
                // 赋值：把右侧 KeyboardLayer::Numbers 的结果保存到左侧 layer_。
                layer_ = KeyboardLayer::Numbers;
                // 赋值：把右侧 false 的结果保存到左侧 upper_。
                upper_ = false;
                // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
                (void)router::Router::instance().repaint(router::intent_partial_full());
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            // 分支匹配：case 2 成立时执行下面的代码。
            case 2:
                // 调用 leave_pinyin_layer();，执行这个函数并处理它的返回值或状态变化。
                leave_pinyin_layer();
                // 赋值：把右侧 KeyboardLayer::Letters 的结果保存到左侧 layer_。
                layer_ = KeyboardLayer::Letters;
                // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
                (void)router::Router::instance().repaint(router::intent_partial_full());
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            // 分支匹配：case 3 成立时执行下面的代码。
            case 3:
                // 调用 leave_pinyin_layer();，执行这个函数并处理它的返回值或状态变化。
                leave_pinyin_layer();
                // 赋值：把右侧 KeyboardLayer::Symbols 的结果保存到左侧 layer_。
                layer_ = KeyboardLayer::Symbols;
                // 赋值：把右侧 false 的结果保存到左侧 upper_。
                upper_ = false;
                // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
                (void)router::Router::instance().repaint(router::intent_partial_full());
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            // 分支匹配：case 4 成立时执行下面的代码。
            case 4:
                // 判断条件：layer_ != KeyboardLayer::Pinyin 是否成立。
                if (layer_ != KeyboardLayer::Pinyin)
                    // 调用 enter_pinyin_layer();，执行这个函数并处理它的返回值或状态变化。
                    enter_pinyin_layer();
                // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
                (void)router::Router::instance().repaint(router::intent_partial_full());
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            // 开始代码块：下面是这个函数、分支或循环的主体。
            default: {
                // 只读约束：这个值或对象在当前位置不能被修改。
                const uint32_t gen = open_gen_;
                // 判断条件：cb_ != nullptr 是否成立。
                if (cb_ != nullptr)
                    // 调用 cb_(buf_, user_);，执行这个函数并处理它的返回值或状态变化。
                    cb_(buf_, user_);
                // 回调内可再次 open()；未重开时才 close
                if (open_gen_ == gen)
                    // 调用 close()：isopen() 查询路由栈顶部覆盖层是不是控制中心。。
                    close();
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            }
        }
    }

    // 判断条件：layer_ == KeyboardLayer::Pinyin && handle_candidate_tap(x, y) 是否成立。
    if (layer_ == KeyboardLayer::Pinyin && handle_candidate_tap(x, y))
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect ka = letters_area_rect();
    // 判断条件：!ka.contains(x, y) 是否成立。
    if (!ka.contains(x, y))
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t rows = rows_of_current_layer();
    // 判断条件：rows == 0 是否成立。
    if (rows == 0)
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t row_h = static_cast<uint16_t>(ka.h / rows);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t row = static_cast<uint8_t>((y - ka.y) / row_h);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t col = static_cast<uint8_t>(x / col_w());
    // 判断条件：row >= rows || col >= Theme::kKbCols 是否成立。
    if (row >= rows || col >= Theme::kKbCols)
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};

    // 判断条件：layer_ == KeyboardLayer::Pinyin 是否成立。
    if (layer_ == KeyboardLayer::Pinyin)
    {
        // 判断条件：handle_pinyin_key_tap(row, col) 是否成立。
        if (handle_pinyin_key_tap(row, col))
            // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
            (void)router::Router::instance().repaint(router::intent_partial_full());
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 判断条件：is_shift_cell(row, col) 是否成立。
    if (is_shift_cell(row, col))
    {
        // 赋值：把右侧 !upper_ 的结果保存到左侧 upper_。
        upper_ = !upper_;
        // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
        (void)router::Router::instance().repaint(router::intent_partial_full());
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：is_space_cell(row, col) 是否成立。
    if (is_space_cell(row, col))
    {
        // 调用 buf_append(' ');，执行这个函数并处理它的返回值或状态变化。
        buf_append(' ');
        // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
        (void)router::Router::instance().repaint(router::intent_partial_full());
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：is_backspace_cell(row, col) 是否成立。
    if (is_backspace_cell(row, col))
    {
        // 调用 buf_backspace();，执行这个函数并处理它的返回值或状态变化。
        buf_backspace();
        // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
        (void)router::Router::instance().repaint(router::intent_partial_full());
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const char ch = char_at(row, col);
    // 判断条件：ch != 0 是否成立。
    if (ch != 0)
    {
        // 调用 buf_append(ch);，执行这个函数并处理它的返回值或状态变化。
        buf_append(ch);
        // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
        (void)router::Router::instance().repaint(router::intent_partial_full());
    }
    // 返回 {true} 给调用者，表示当前函数的执行结果。
    return {true};
}

// 定义或声明函数 Keyboard::refresh_candidates：参数在括号里，返回类型写在函数名前面。
void Keyboard::refresh_candidates()
{
    // 赋值：把右侧 0 的结果保存到左侧 cand_offset_。
    cand_offset_ = 0; // 组合区一变，候选池整体重查，翻页状态清零
    // 赋值：把右侧 ime::ImeEngine::get_instance() 的结果保存到左侧 auto& engine。
    auto& engine = ime::ImeEngine::get_instance();
    // 判断条件：!engine.ready() || engine.compose_empty() 是否成立。
    if (!engine.ready() || engine.compose_empty())
    {
        // 赋值：把右侧 0 的结果保存到左侧 cand_count_。
        cand_count_ = 0;
        // 赋值：把右侧 '\0' 的结果保存到左侧 cand_buf_[0]。
        cand_buf_[0] = '\0';
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 赋值：把右侧 engine.query_candidates(cand_buf_, kCandPoolCap) 的结果保存到左侧 cand_count_。
    cand_count_ = engine.query_candidates(cand_buf_, kCandPoolCap);
}

// 定义或声明函数 Keyboard::enter_pinyin_layer：参数在括号里，返回类型写在函数名前面。
void Keyboard::enter_pinyin_layer()
{
    // 赋值：把右侧 KeyboardLayer::Pinyin 的结果保存到左侧 layer_。
    layer_ = KeyboardLayer::Pinyin;
    // 赋值：把右侧 false 的结果保存到左侧 upper_。
    upper_ = false;
    // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    ime::ImeEngine::get_instance().reset();
    // 调用 refresh_candidates();，执行这个函数并处理它的返回值或状态变化。
    refresh_candidates();
}

// 定义或声明函数 Keyboard::leave_pinyin_layer：参数在括号里，返回类型写在函数名前面。
void Keyboard::leave_pinyin_layer()
{
    // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
    ime::ImeEngine::get_instance().reset();
    // 赋值：把右侧 0 的结果保存到左侧 cand_count_。
    cand_count_ = 0;
    // 赋值：把右侧 0 的结果保存到左侧 cand_offset_。
    cand_offset_ = 0;
    // 赋值：把右侧 '\0' 的结果保存到左侧 cand_buf_[0]。
    cand_buf_[0] = '\0';
}

// 定义或声明函数 Keyboard::scroll_candidates：参数在括号里，返回类型写在函数名前面。
bool Keyboard::scroll_candidates(bool forward)
{
    // 判断条件：layer_ != KeyboardLayer::Pinyin || cand_count_ == 0 是否成立。
    if (layer_ != KeyboardLayer::Pinyin || cand_count_ == 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 判断条件：forward 是否成立。
    if (forward)
    {
        // 判断条件：static_cast<uint8_t>(cand_offset_ + kCandPageSize) >= cand_count_ 是否成立。
        if (static_cast<uint8_t>(cand_offset_ + kCandPageSize) >= cand_count_)
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        // 赋值：把右侧 static_cast<uint8_t>(cand_offset_ + kCandPageSize) 的结果保存到左侧 cand_offset_。
        cand_offset_ = static_cast<uint8_t>(cand_offset_ + kCandPageSize);
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 判断条件：cand_offset_ == 0 是否成立。
        if (cand_offset_ == 0)
            // 返回 false 给调用者，表示当前函数的执行结果。
            return false;
        // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
        cand_offset_ = static_cast<uint8_t>(
            // 继续上一行的表达式或参数列表：(cand_offset_ >= kCandPageSize) ? (cand_offset_ - kCandPageSize) : 0);。
            (cand_offset_ >= kCandPageSize) ? (cand_offset_ - kCandPageSize) : 0);
    }
    // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
    (void)router::Router::instance().repaint(router::intent_partial_full());
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 Keyboard::handle_pinyin_key_tap：参数在括号里，返回类型写在函数名前面。
bool Keyboard::handle_pinyin_key_tap(uint8_t row, uint8_t col)
{
    // 赋值：把右侧 ime::ImeEngine::get_instance() 的结果保存到左侧 auto& engine。
    auto& engine = ime::ImeEngine::get_instance();

    // 判断条件：is_space_cell(row, col) 是否成立。
    if (is_space_cell(row, col))
    {
        // 判断条件：!engine.compose_empty() && cand_count_ > 0 是否成立。
        if (!engine.compose_empty() && cand_count_ > 0)
        {
            // 调用 buf_append_utf8_char(cand_buf_);，执行这个函数并处理它的返回值或状态变化。
            buf_append_utf8_char(cand_buf_); // 空格提交首个候选（沿用手机拼音习惯）
            // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
            engine.reset();
            // 调用 refresh_candidates();，执行这个函数并处理它的返回值或状态变化。
            refresh_candidates();
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 调用 buf_append(' ');，执行这个函数并处理它的返回值或状态变化。
            buf_append(' ');
        }
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 判断条件：is_backspace_cell(row, col) 是否成立。
    if (is_backspace_cell(row, col))
    {
        // 判断条件：!engine.compose_empty() 是否成立。
        if (!engine.compose_empty())
        {
            // 访问对象成员：engine.pop_ascii();；读取或修改这个对象里的成员状态。
            engine.pop_ascii();
            // 调用 refresh_candidates();，执行这个函数并处理它的返回值或状态变化。
            refresh_candidates();
        }
        // 备用分支：前面的条件不成立时执行这里。
        else
        {
            // 调用 buf_backspace();，执行这个函数并处理它的返回值或状态变化。
            buf_backspace();
        }
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }

    // 只读约束：这个值或对象在当前位置不能被修改。
    const char ch = char_at(row, col);
    // 判断条件：ch != 0 && engine.push_ascii(ch) 是否成立。
    if (ch != 0 && engine.push_ascii(ch))
        // 调用 refresh_candidates();，执行这个函数并处理它的返回值或状态变化。
        refresh_candidates();
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 Keyboard::handle_candidate_tap：参数在括号里，返回类型写在函数名前面。
bool Keyboard::handle_candidate_tap(int16_t x, int16_t y)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect area = candidate_rect();
    // 判断条件：!area.contains(x, y) 是否成立。
    if (!area.contains(x, y))
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t cw = static_cast<uint16_t>(area.w / kCandCols);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t col = static_cast<uint8_t>((x - area.x) / cw);

    // 判断条件：col == 0 是否成立。
    if (col == 0)
    {
        // 函数调用：(void)scroll_candidates(false);；执行该函数并处理它的返回值或副作用。
        (void)scroll_candidates(false);
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 判断条件：col == static_cast<uint8_t>(kCandCols - 1) 是否成立。
    if (col == static_cast<uint8_t>(kCandCols - 1))
    {
        // 函数调用：(void)scroll_candidates(true);；执行该函数并处理它的返回值或副作用。
        (void)scroll_candidates(true);
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 判断条件：cand_count_ == 0 是否成立。
    if (cand_count_ == 0)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true; // 消费掉该次点击，避免穿透到字母网格

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t page_col = static_cast<uint8_t>(col - 1);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t idx = static_cast<uint8_t>(cand_offset_ + page_col);
    // 判断条件：page_col < kCandPageSize && idx < cand_count_ 是否成立。
    if (page_col < kCandPageSize && idx < cand_count_)
    {
        // 调用 buf_append_utf8_char(cand_buf_ + static_cast<size_t>(idx) * 3);，执行这个函数并处理它的返回值或状态变化。
        buf_append_utf8_char(cand_buf_ + static_cast<size_t>(idx) * 3);
        // 调用 reset()：init() 保存触摸设备指针和手势阈值。reset() 清空状态、起点、终点和 pending 事件。。
        ime::ImeEngine::get_instance().reset();
        // 调用 refresh_candidates();，执行这个函数并处理它的返回值或状态变化。
        refresh_candidates();
        // 调用 router::intent_partial_full()：名字里的 full 表示当前合成和提交都按整屏处理，前面的 Partial/Fast/Full 表示波形。也就是说，代码暂时没有真正按局部矩形只画脏区；局刷是“全屏 framebuffer 提交，但使用 Partial 波形”。。
        (void)router::Router::instance().repaint(router::intent_partial_full());
    }
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::overlays
