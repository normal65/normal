// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/core/geometry.hpp
 * 作用：定义 Point、Size、Rect 等基础几何类型。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：上层通过 Router、UiBus 和 SystemState 协作，不直接操作硬件。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "bsp/driver/gdey027t91/panel.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::core {

// 赋值：把右侧 ::app::bsp::driver::gdey027t91::Panel::WIDTH 的结果保存到左侧 inline constexpr uint16_t kScreenW。
inline constexpr uint16_t kScreenW = ::app::bsp::driver::gdey027t91::Panel::WIDTH;
// 赋值：把右侧 ::app::bsp::driver::gdey027t91::Panel::HEIGHT 的结果保存到左侧 inline constexpr uint16_t kScreenH。
inline constexpr uint16_t kScreenH = ::app::bsp::driver::gdey027t91::Panel::HEIGHT;
// 赋值：把右侧 ::app::bsp::driver::gdey027t91::Panel::STRIDE 的结果保存到左侧 inline constexpr uint16_t kStride。
inline constexpr uint16_t kStride = ::app::bsp::driver::gdey027t91::Panel::STRIDE;
// 赋值：把右侧 ::app::bsp::driver::gdey027t91::Panel::BUFFER_SIZE 的结果保存到左侧 inline constexpr uint32_t kFbBytes。
inline constexpr uint32_t kFbBytes = ::app::bsp::driver::gdey027t91::Panel::BUFFER_SIZE;

/** SSD1680 局刷窗口 x/w 须 8 像素对齐 */
// 赋值：把右侧 8 的结果保存到左侧 inline constexpr uint8_t kPartialAlign。
inline constexpr uint8_t kPartialAlign = 8;

// struct Point：把相关数据字段组合在一起，表示项目中的一个概念。
struct Point
{
    // 定义变量 int16_t x 并初始化为 0。
    int16_t x = 0;
    // 定义变量 int16_t y 并初始化为 0。
    int16_t y = 0;

    // constexpr 构造函数：允许对象在编译期或轻量场景中直接构造。
    constexpr Point() = default;
    // constexpr 构造函数：允许对象在编译期或轻量场景中直接构造。
    constexpr Point(int16_t px, int16_t py)
        // 定义或声明函数 x：参数在括号里，返回类型写在函数名前面。
        : x(px)
        // 定义或声明函数 y：参数在括号里，返回类型写在函数名前面。
        , y(py)
    {
    }
};

// struct Size：把相关数据字段组合在一起，表示项目中的一个概念。
struct Size
{
    // 定义变量 uint16_t w 并初始化为 0。
    uint16_t w = 0;
    // 定义变量 uint16_t h 并初始化为 0。
    uint16_t h = 0;

    // constexpr 构造函数：允许对象在编译期或轻量场景中直接构造。
    constexpr Size() = default;
    // constexpr 构造函数：允许对象在编译期或轻量场景中直接构造。
    constexpr Size(uint16_t sw, uint16_t sh)
        // 定义或声明函数 w：参数在括号里，返回类型写在函数名前面。
        : w(sw)
        // 定义或声明函数 h：参数在括号里，返回类型写在函数名前面。
        , h(sh)
    {
    }

    // 编译期函数 empty：常用于固定规则计算，能参与编译期优化。
    constexpr bool empty() const
    {
        // 返回 w == 0 || h == 0 给调用者，表示当前函数的执行结果。
        return w == 0 || h == 0;
    }
    // 编译期函数 area：常用于固定规则计算，能参与编译期优化。
    constexpr uint32_t area() const
    {
        // 返回 static_cast<uint32_t>(w) * h 给调用者，表示当前函数的执行结果。
        return static_cast<uint32_t>(w) * h;
    }
};

/** 矩形 [x, x+w) × [y, y+h)；constexpr，x/y 可负 */
// struct Rect：把相关数据字段组合在一起，表示项目中的一个概念。
struct Rect
{
    // 定义变量 int16_t x 并初始化为 0。
    int16_t x = 0;
    // 定义变量 int16_t y 并初始化为 0。
    int16_t y = 0;
    // 定义变量 uint16_t w 并初始化为 0。
    uint16_t w = 0;
    // 定义变量 uint16_t h 并初始化为 0。
    uint16_t h = 0;

    // constexpr 构造函数：允许对象在编译期或轻量场景中直接构造。
    constexpr Rect() = default;
    // constexpr 构造函数：允许对象在编译期或轻量场景中直接构造。
    constexpr Rect(int16_t rx, int16_t ry, uint16_t rw, uint16_t rh)
        // 定义或声明函数 x：参数在括号里，返回类型写在函数名前面。
        : x(rx)
        // 定义或声明函数 y：参数在括号里，返回类型写在函数名前面。
        , y(ry)
        // 定义或声明函数 w：参数在括号里，返回类型写在函数名前面。
        , w(rw)
        // 定义或声明函数 h：参数在括号里，返回类型写在函数名前面。
        , h(rh)
    {
    }

    // 静态函数 full：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static constexpr Rect full()
    {
        // 返回 {0, 0, kScreenW, kScreenH} 给调用者，表示当前函数的执行结果。
        return {0, 0, kScreenW, kScreenH};
    }

    // 编译期函数 empty：常用于固定规则计算，能参与编译期优化。
    constexpr bool empty() const
    {
        // 返回 w == 0 || h == 0 给调用者，表示当前函数的执行结果。
        return w == 0 || h == 0;
    }
    // 编译期函数 right：常用于固定规则计算，能参与编译期优化。
    constexpr int16_t right() const
    {
        // 返回 static_cast<int16_t>(x + static_cast<int16_t>(w)) 给调用者，表示当前函数的执行结果。
        return static_cast<int16_t>(x + static_cast<int16_t>(w));
    }
    // 编译期函数 bottom：常用于固定规则计算，能参与编译期优化。
    constexpr int16_t bottom() const
    {
        // 返回 static_cast<int16_t>(y + static_cast<int16_t>(h)) 给调用者，表示当前函数的执行结果。
        return static_cast<int16_t>(y + static_cast<int16_t>(h));
    }
    // 编译期函数 size：常用于固定规则计算，能参与编译期优化。
    constexpr Size size() const
    {
        // 返回 Size{w, h} 给调用者，表示当前函数的执行结果。
        return Size{w, h};
    }
    // 编译期函数 area：常用于固定规则计算，能参与编译期优化。
    constexpr uint32_t area() const
    {
        // 返回 static_cast<uint32_t>(w) * h 给调用者，表示当前函数的执行结果。
        return static_cast<uint32_t>(w) * h;
    }

    // 编译期函数 contains：常用于固定规则计算，能参与编译期优化。
    constexpr bool contains(int16_t px, int16_t py) const
    {
        // 返回 !empty() && px >= x && py >= y && px < right() && py < bottom() 给调用者，表示当前函数的执行结果。
        return !empty() && px >= x && py >= y && px < right() && py < bottom();
    }

    // 编译期函数 intersects：常用于固定规则计算，能参与编译期优化。
    constexpr bool intersects(const Rect& o) const
    {
        // 返回 !empty() && !o.empty() && x < o.right() && o.x < right() && y < o.bottom() && 给调用者，表示当前函数的执行结果。
        return !empty() && !o.empty() && x < o.right() && o.x < right() && y < o.bottom() &&
               // 继续上一行的表达式或参数列表：o.y < bottom();。
               o.y < bottom();
    }

    // 静态函数 intersect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static constexpr Rect intersect(const Rect& a, const Rect& b)
    {
        // 判断条件：a.empty() || b.empty() 是否成立。
        if (a.empty() || b.empty())
            // 返回 {} 给调用者，表示当前函数的执行结果。
            return {};
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t x1 = (a.x > b.x) ? a.x : b.x;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t y1 = (a.y > b.y) ? a.y : b.y;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t x2 = (a.right() < b.right()) ? a.right() : b.right();
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t y2 = (a.bottom() < b.bottom()) ? a.bottom() : b.bottom();
        // 判断条件：x2 <= x1 || y2 <= y1 是否成立。
        if (x2 <= x1 || y2 <= y1)
            // 返回 {} 给调用者，表示当前函数的执行结果。
            return {};
        // 返回 Rect{x1, y1, static_cast<uint16_t>(x2 - x1), static_cast<uint16_t>(y2 - y1)} 给调用者，表示当前函数的执行结果。
        return Rect{x1, y1, static_cast<uint16_t>(x2 - x1), static_cast<uint16_t>(y2 - y1)};
    }

    // 静态函数 merge：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static constexpr Rect merge(const Rect& a, const Rect& b)
    {
        // 判断条件：a.empty() 是否成立。
        if (a.empty())
            // 返回 b 给调用者，表示当前函数的执行结果。
            return b;
        // 判断条件：b.empty() 是否成立。
        if (b.empty())
            // 返回 a 给调用者，表示当前函数的执行结果。
            return a;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t x1 = (a.x < b.x) ? a.x : b.x;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t y1 = (a.y < b.y) ? a.y : b.y;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t x2 = (a.right() > b.right()) ? a.right() : b.right();
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t y2 = (a.bottom() > b.bottom()) ? a.bottom() : b.bottom();
        // 返回 Rect{x1, y1, static_cast<uint16_t>(x2 - x1), static_cast<uint16_t>(y2 - y1)} 给调用者，表示当前函数的执行结果。
        return Rect{x1, y1, static_cast<uint16_t>(x2 - x1), static_cast<uint16_t>(y2 - y1)};
    }

    // 编译期函数 translated：常用于固定规则计算，能参与编译期优化。
    constexpr Rect translated(int16_t dx, int16_t dy) const
    {
        // 返回 Rect{static_cast<int16_t>(x + dx), static_cast<int16_t>(y + dy), w, h} 给调用者，表示当前函数的执行结果。
        return Rect{static_cast<int16_t>(x + dx), static_cast<int16_t>(y + dy), w, h};
    }

    // 编译期函数 inflated：常用于固定规则计算，能参与编译期优化。
    constexpr Rect inflated(int16_t dx, int16_t dy) const
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int32_t nw = static_cast<int32_t>(w) + 2 * dx;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int32_t nh = static_cast<int32_t>(h) + 2 * dy;
        // 判断条件：nw <= 0 || nh <= 0 是否成立。
        if (nw <= 0 || nh <= 0)
            // 返回 {} 给调用者，表示当前函数的执行结果。
            return {};
        // 返回 Rect{static_cast<int16_t>(x - dx), static_cast<int16_t>(y - dy), 给调用者，表示当前函数的执行结果。
        return Rect{static_cast<int16_t>(x - dx), static_cast<int16_t>(y - dy),
                    // 继续上一行的表达式或参数列表：static_cast<uint16_t>(nw), static_cast<uint16_t>(nh)};。
                    static_cast<uint16_t>(nw), static_cast<uint16_t>(nh)};
    }

    // 编译期函数 clamped：常用于固定规则计算，能参与编译期优化。
    constexpr Rect clamped() const
    {
        // 判断条件：empty() 是否成立。
        if (empty())
            // 返回 {} 给调用者，表示当前函数的执行结果。
            return {};
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t x1 = (x < 0) ? 0 : x;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t y1 = (y < 0) ? 0 : y;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t x2 =
            // 继续上一行的表达式或参数列表：(right() > static_cast<int16_t>(kScreenW)) ? static_cast<int16_t>(kScreenW) : right();。
            (right() > static_cast<int16_t>(kScreenW)) ? static_cast<int16_t>(kScreenW) : right();
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t y2 =
            // 继续上一行的表达式或参数列表：(bottom() > static_cast<int16_t>(kScreenH)) ? static_cast<int16_t>(kScreenH) : bottom();。
            (bottom() > static_cast<int16_t>(kScreenH)) ? static_cast<int16_t>(kScreenH) : bottom();
        // 判断条件：x2 <= x1 || y2 <= y1 是否成立。
        if (x2 <= x1 || y2 <= y1)
            // 返回 {} 给调用者，表示当前函数的执行结果。
            return {};
        // 返回 Rect{x1, y1, static_cast<uint16_t>(x2 - x1), static_cast<uint16_t>(y2 - y1)} 给调用者，表示当前函数的执行结果。
        return Rect{x1, y1, static_cast<uint16_t>(x2 - x1), static_cast<uint16_t>(y2 - y1)};
    }

    /** 裁剪到屏幕并将 x/w 对齐到 kPartialAlign（局刷提交要求） */
    // 编译期函数 aligned：常用于固定规则计算，能参与编译期优化。
    constexpr Rect aligned() const
    {
        // 定义变量 Rect c 并初始化为 clamped()。
        Rect c = clamped();
        // 判断条件：c.empty() 是否成立。
        if (c.empty())
            // 返回 {} 给调用者，表示当前函数的执行结果。
            return {};

        // 固定参数 mask = static_cast<uint16_t>(kPartialAlign - 1)：由当前模块定义，用于控制布局、时序或硬件行为。
        constexpr uint16_t mask = static_cast<uint16_t>(kPartialAlign - 1);
        // 定义变量 int16_t x1 并初始化为 static_cast<int16_t>(c.x & ~mask)。
        int16_t x1 = static_cast<int16_t>(c.x & ~mask);
        // 定义变量 int16_t x2 并初始化为 static_cast<int16_t>((c.right() + mask) & ~mask)。
        int16_t x2 = static_cast<int16_t>((c.right() + mask) & ~mask);
        // 判断条件：x2 > static_cast<int16_t>(kScreenW) 是否成立。
        if (x2 > static_cast<int16_t>(kScreenW))
            // 赋值：把右侧 static_cast<int16_t>(kScreenW) 的结果保存到左侧 x2。
            x2 = static_cast<int16_t>(kScreenW);
        // 判断条件：x2 <= x1 是否成立。
        if (x2 <= x1)
            // 返回 {} 给调用者，表示当前函数的执行结果。
            return {};

        // 返回 Rect{x1, c.y, static_cast<uint16_t>(x2 - x1), c.h} 给调用者，表示当前函数的执行结果。
        return Rect{x1, c.y, static_cast<uint16_t>(x2 - x1), c.h};
    }
};

// 编译期函数 screen_area_pct：常用于固定规则计算，能参与编译期优化。
constexpr uint32_t screen_area_pct(uint8_t pct)
{
    // 返回 (static_cast<uint32_t>(kScreenW) * kScreenH * pct) / 100U 给调用者，表示当前函数的执行结果。
    return (static_cast<uint32_t>(kScreenW) * kScreenH * pct) / 100U;
}

// 编译期函数 rect_meets_area_pct：常用于固定规则计算，能参与编译期优化。
constexpr bool rect_meets_area_pct(const Rect& r, uint8_t pct)
{
    // 判断条件：r.empty() || pct == 0 是否成立。
    if (r.empty() || pct == 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint32_t panel = static_cast<uint32_t>(kScreenW) * kScreenH;
    // 返回 r.area() * 100U >= panel * static_cast<uint32_t>(pct) 给调用者，表示当前函数的执行结果。
    return r.area() * 100U >= panel * static_cast<uint32_t>(pct);
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::core
