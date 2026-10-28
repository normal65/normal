/**
 * [初学者导读] 文件：main/app/ebook/apps/reader/reading_store.cc
 * 作用：保存阅读进度、最近阅读和阅读器偏好。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/reader/reading_store.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "data/persist.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::reader {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 固定参数 kKTitle = "bk.title"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kKTitle = "bk.title";
// 固定参数 kKPath = "bk.path"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kKPath = "bk.path";
// 固定参数 kKCurLo = "bk.cur"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kKCurLo = "bk.cur";
// 固定参数 kKCurHi = "bk.curh"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kKCurHi = "bk.curh";
// 固定参数 kKTotLo = "bk.tot"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kKTotLo = "bk.tot";
// 固定参数 kKTotHi = "bk.toth"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kKTotHi = "bk.toth";

// 定义或声明函数 load_u16：参数在括号里，返回类型写在函数名前面。
uint16_t load_u16(const char* lo_key, const char* hi_key)
{
    // 定义变量 uint8_t lo 并初始化为 0, hi = 0。
    uint8_t lo = 0, hi = 0;
    // 判断条件：!data::Persist::get_u8(lo_key, lo) 是否成立。
    if (!data::Persist::get_u8(lo_key, lo))
        // 返回 0 给调用者，表示当前函数的执行结果。
        return 0;
    // 函数调用：(void)data::Persist::get_u8(hi_key, hi);；执行该函数并处理它的返回值或副作用。
    (void)data::Persist::get_u8(hi_key, hi); // 高字节可缺失
    // 返回 static_cast<uint16_t>((hi << 8) | lo) 给调用者，表示当前函数的执行结果。
    return static_cast<uint16_t>((hi << 8) | lo);
}

// 定义或声明函数 store_u16：参数在括号里，返回类型写在函数名前面。
void store_u16(const char* lo_key, const char* hi_key, uint16_t v)
{
    // 调用 data::Persist::set_u8(lo_key, static_cast<uint8_t>(v & 0xFF));，执行这个函数并处理它的返回值或状态变化。
    data::Persist::set_u8(lo_key, static_cast<uint8_t>(v & 0xFF));
    // 调用 data::Persist::set_u8(hi_key, static_cast<uint8_t>((v >> 8) & 0xFF));，执行这个函数并处理它的返回值或状态变化。
    data::Persist::set_u8(hi_key, static_cast<uint8_t>((v >> 8) & 0xFF));
}

// 定义或声明函数 copy_clamped：参数在括号里，返回类型写在函数名前面。
void copy_clamped(char* dst, size_t cap, const char* src)
{
    // 判断条件：src == nullptr || cap == 0 是否成立。
    if (src == nullptr || cap == 0)
    {
        // 判断条件：cap > 0 是否成立。
        if (cap > 0)
            // 赋值：把右侧 '\0' 的结果保存到左侧 dst[0]。
            dst[0] = '\0';
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }
    // 调用 std::strncpy(dst, src, cap - 1);，执行这个函数并处理它的返回值或状态变化。
    std::strncpy(dst, src, cap - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 dst[cap - 1]。
    dst[cap - 1] = '\0';
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 ReadingStore::get_instance：参数在括号里，返回类型写在函数名前面。
ReadingStore& ReadingStore::get_instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static ReadingStore s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义或声明函数 ReadingStore::load：参数在括号里，返回类型写在函数名前面。
void ReadingStore::load()
{
    // 赋值：把右侧 false 的结果保存到左侧 has_book_。
    has_book_ = false;
    // 赋值：把右侧 '\0' 的结果保存到左侧 title_[0]。
    title_[0] = '\0';
    // 赋值：把右侧 '\0' 的结果保存到左侧 path_[0]。
    path_[0] = '\0';

    // 赋值：把右侧 {} 的结果保存到左侧 char buf[kPathMax + 1]。
    char buf[kPathMax + 1] = {};

    // 判断条件：data::Persist::get_str(kKTitle, buf, sizeof(buf)) && buf[0] != '\0' 是否成立。
    if (data::Persist::get_str(kKTitle, buf, sizeof(buf)) && buf[0] != '\0')
    {
        // 调用 copy_clamped(title_, sizeof(title_), buf);，执行这个函数并处理它的返回值或状态变化。
        copy_clamped(title_, sizeof(title_), buf);
        // 赋值：把右侧 true 的结果保存到左侧 has_book_。
        has_book_ = true;
    }

    // 赋值：把右侧 '\0' 的结果保存到左侧 buf[0]。
    buf[0] = '\0';
    // 判断条件：data::Persist::get_str(kKPath, buf, sizeof(buf)) && buf[0] != '\0' 是否成立。
    if (data::Persist::get_str(kKPath, buf, sizeof(buf)) && buf[0] != '\0')
        // 调用 copy_clamped(path_, sizeof(path_), buf);，执行这个函数并处理它的返回值或状态变化。
        copy_clamped(path_, sizeof(path_), buf);

    // 赋值：把右侧 load_u16(kKCurLo, kKCurHi) 的结果保存到左侧 current_page_。
    current_page_ = load_u16(kKCurLo, kKCurHi);
    // 赋值：把右侧 load_u16(kKTotLo, kKTotHi) 的结果保存到左侧 total_pages_。
    total_pages_ = load_u16(kKTotLo, kKTotHi);
}

// 定义或声明函数 ReadingStore::set_book：参数在括号里，返回类型写在函数名前面。
void ReadingStore::set_book(const char* title, const char* path, uint16_t current, uint16_t total)
{
    // 判断条件：title == nullptr 是否成立。
    if (title == nullptr)
    {
        // 调用 clear()：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
        clear();
        // 提前返回：结束当前函数，不返回具体值。
        return;
    }

    // 调用 copy_clamped(title_, sizeof(title_), title);，执行这个函数并处理它的返回值或状态变化。
    copy_clamped(title_, sizeof(title_), title);
    // 调用 copy_clamped(path_, sizeof(path_), path);，执行这个函数并处理它的返回值或状态变化。
    copy_clamped(path_, sizeof(path_), path);
    // 赋值：把右侧 current 的结果保存到左侧 current_page_。
    current_page_ = current;
    // 赋值：把右侧 total 的结果保存到左侧 total_pages_。
    total_pages_ = total;
    // 赋值：把右侧 (title_[0] != '\0') 的结果保存到左侧 has_book_。
    has_book_ = (title_[0] != '\0');

    // 调用 data::Persist::set_str(kKTitle, title_);，执行这个函数并处理它的返回值或状态变化。
    data::Persist::set_str(kKTitle, title_);
    // 调用 data::Persist::set_str(kKPath, path_);，执行这个函数并处理它的返回值或状态变化。
    data::Persist::set_str(kKPath, path_);
    // 调用 store_u16(kKCurLo, kKCurHi, current);，执行这个函数并处理它的返回值或状态变化。
    store_u16(kKCurLo, kKCurHi, current);
    // 调用 store_u16(kKTotLo, kKTotHi, total);，执行这个函数并处理它的返回值或状态变化。
    store_u16(kKTotLo, kKTotHi, total);
    // 调用 data::Persist::commit();，执行这个函数并处理它的返回值或状态变化。
    data::Persist::commit();
}

// 定义或声明函数 ReadingStore::clear：参数在括号里，返回类型写在函数名前面。
void ReadingStore::clear()
{
    // 赋值：把右侧 false 的结果保存到左侧 has_book_。
    has_book_ = false;
    // 赋值：把右侧 '\0' 的结果保存到左侧 title_[0]。
    title_[0] = '\0';
    // 赋值：把右侧 '\0' 的结果保存到左侧 path_[0]。
    path_[0] = '\0';
    // 赋值：把右侧 0 的结果保存到左侧 current_page_。
    current_page_ = 0;
    // 赋值：把右侧 0 的结果保存到左侧 total_pages_。
    total_pages_ = 0;
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::reader
