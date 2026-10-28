// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/calendar/calendar_app.hpp
 * 作用：实现日历月视图、日期选择和月份切换。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::calendar {

/** @brief 日历：月视图 + 年月 Picker */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class CalendarApp : public App
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static CalendarApp& instance();

    // 定义或声明函数 id：参数在括号里，返回类型写在函数名前面。
    AppId id() const override
    {
        // 返回 AppId::Calendar 给调用者，表示当前函数的执行结果。
        return AppId::Calendar;
    }
    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const char* title() const override;
    // 调用 icon_cp()：返回图标 codepoint。图标通常来自字体图标表，所以这里返回的是 uint32t，不是图片路径。。
    uint32_t icon_cp() const override;

    // 调用 on_enter()：进入文件 App 时： 1. 重置浏览状态或恢复根卷状态。 2. 清空弹窗。 3. 设置加载状态。 4. 调度目录扫描。 5. 请求重绘。。
    void on_enter() override;
    // 调用 on_exit()：退出时取消未完成的扫描、关闭弹窗，并确保后台任务不会在对象状态失效后继续写数据。。
    void on_exit() override;
    // 函数调用：void paint(gfx::Canvas& canvas) override;；执行该函数并处理它的返回值或副作用。
    void paint(gfx::Canvas& canvas) override;
    // 调用 on_input(const::app::ebook::input::Event&ev)：文件 App 的输入总入口。它处理： 触摸按下、移动、抬起和滑动。 工具栏返回。 列表滚动节流。 弹窗点击优先于底层列表点击。。
    shell::InputResult on_input(const ::app::ebook::input::Event& ev) override;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 CalendarApp() = default;，执行这个函数并处理它的返回值或状态变化。
    CalendarApp() = default;

    // struct DayCell：把相关数据字段组合在一起，表示项目中的一个概念。
    struct DayCell
    {
        // 执行语句：uint16_t year{0};；它会更新一次变量、对象或程序状态。
        uint16_t year{0};
        // 执行语句：uint8_t month{0};；它会更新一次变量、对象或程序状态。
        uint8_t month{0};
        // 执行语句：uint8_t day{0};；它会更新一次变量、对象或程序状态。
        uint8_t day{0};
        // 执行语句：bool in_month{false};；它会更新一次变量、对象或程序状态。
        bool in_month{false};
        // 执行语句：bool is_today{false};；它会更新一次变量、对象或程序状态。
        bool is_today{false};
    };

    // 固定参数 kGridCols = 7：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kGridCols = 7;
    // 固定参数 kGridRows = 6：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kGridRows = 6;

    // 函数调用：void sync_to_clock();；执行该函数并处理它的返回值或副作用。
    void sync_to_clock();
    // 函数调用：void shift_month(int delta);；执行该函数并处理它的返回值或副作用。
    void shift_month(int delta);
    // 函数调用：void rebuild_grid();；执行该函数并处理它的返回值或副作用。
    void rebuild_grid();
    // 函数调用：void select_cell(uint8_t index);；执行该函数并处理它的返回值或副作用。
    void select_cell(uint8_t index);

    // 函数调用：void open_picker();；执行该函数并处理它的返回值或副作用。
    void open_picker();
    // 函数调用：void close_picker();；执行该函数并处理它的返回值或副作用。
    void close_picker();
    // 函数调用：void apply_picker();；执行该函数并处理它的返回值或副作用。
    void apply_picker();
    // 函数调用：void paint_picker(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_picker(gfx::Canvas& c);
    // 函数调用：shell::InputResult handle_picker_input(int16_t x, int16_t y);；执行该函数并处理它的返回值或副作用。
    shell::InputResult handle_picker_input(int16_t x, int16_t y);

    // 静态函数 days_in_month：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static uint8_t days_in_month(uint16_t year, uint8_t month);
    // 静态函数 weekday_sun0：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static uint8_t weekday_sun0(uint16_t year, uint8_t month, uint8_t day);

    // 静态函数 month_bar_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect month_bar_rect();
    // 静态函数 month_title_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect month_title_rect();
    // 静态函数 weekday_row_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect weekday_row_rect();
    // 静态函数 grid_region_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect grid_region_rect();
    // 静态函数 cell_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect cell_rect(uint8_t row, uint8_t col);
    // 静态函数 picker_panel_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect picker_panel_rect();

    // 执行语句：uint16_t view_year_{2026};；它会更新一次变量、对象或程序状态。
    uint16_t view_year_{2026};
    // 执行语句：uint8_t view_month_{1};；它会更新一次变量、对象或程序状态。
    uint8_t view_month_{1};
    // 执行语句：uint16_t sel_year_{0};；它会更新一次变量、对象或程序状态。
    uint16_t sel_year_{0};
    // 执行语句：uint8_t sel_month_{0};；它会更新一次变量、对象或程序状态。
    uint8_t sel_month_{0};
    // 执行语句：uint8_t sel_day_{0};；它会更新一次变量、对象或程序状态。
    uint8_t sel_day_{0};
    // 执行语句：DayCell cells_[kGridCols * kGridRows]{};；它会更新一次变量、对象或程序状态。
    DayCell cells_[kGridCols * kGridRows]{};

    // 执行语句：bool picker_open_{false};；它会更新一次变量、对象或程序状态。
    bool picker_open_{false};
    // 执行语句：uint16_t picker_year_{2026};；它会更新一次变量、对象或程序状态。
    uint16_t picker_year_{2026};
    // 执行语句：uint8_t picker_month_{1};；它会更新一次变量、对象或程序状态。
    uint8_t picker_month_{1};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::calendar
