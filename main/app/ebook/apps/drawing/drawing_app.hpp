// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/drawing/drawing_app.hpp
 * 作用：实现画板绘制、笔刷、撤销和位图保存。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "core/geometry.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::drawing {

// 类定义：把数据和操作这些数据的方法绑定在一起。
class DrawingApp : public App
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static DrawingApp& instance();

    // 静态函数 request_open_on_enter：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void request_open_on_enter(const char* path);

    /** BMP 导出目录（int 优先，失败再试 sd）；会话缓存仍在 int */
    // 固定参数 kSavePathInt = "/int/Ebook/drawings"：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr const char* kSavePathInt = "/int/Ebook/drawings";
    // 固定参数 kSavePathSd = "/sd/Ebook/drawings"：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr const char* kSavePathSd = "/sd/Ebook/drawings";

    // 定义或声明函数 id：参数在括号里，返回类型写在函数名前面。
    AppId id() const override
    {
        // 返回 AppId::Drawing 给调用者，表示当前函数的执行结果。
        return AppId::Drawing;
    }
    // 定义只读对象或变量：初始化后不能在当前位置修改。
    const char* title() const override;
    // 调用 icon_cp()：返回图标 codepoint。图标通常来自字体图标表，所以这里返回的是 uint32t，不是图片路径。。
    uint32_t icon_cp() const override;

    // 调用 on_enter()：进入文件 App 时： 1. 重置浏览状态或恢复根卷状态。 2. 清空弹窗。 3. 设置加载状态。 4. 调度目录扫描。 5. 请求重绘。。
    void on_enter() override;
    // 调用 on_exit()：退出时取消未完成的扫描、关闭弹窗，并确保后台任务不会在对象状态失效后继续写数据。。
    void on_exit() override;
    // 调用 on_semantic_back()：处理语义返回，顺序通常是： 1. 如果弹窗打开，先关闭弹窗。 2. 如果列表还在浏览目录，先返回上一级。 3. 如果已经到根卷，返回 false，让 Shell 退出 App。。
    bool on_semantic_back() override;

    // 函数调用：void paint(gfx::Canvas& canvas) override;；执行该函数并处理它的返回值或副作用。
    void paint(gfx::Canvas& canvas) override;
    // 函数调用：void paint_overlay(gfx::Canvas& canvas) override;；执行该函数并处理它的返回值或副作用。
    void paint_overlay(gfx::Canvas& canvas) override;
    // 调用 on_input(const::app::ebook::input::Event&ev)：文件 App 的输入总入口。它处理： 触摸按下、移动、抬起和滑动。 工具栏返回。 列表滚动节流。 弹窗点击优先于底层列表点击。。
    shell::InputResult on_input(const ::app::ebook::input::Event& ev) override;

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 DrawingApp() = default;，执行这个函数并处理它的返回值或状态变化。
    DrawingApp() = default;

    // 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
    enum class Tool : uint8_t
    {
        // 执行这条代码：Pen = 0,。它会更新变量、对象状态或调用链。
        Pen = 0,
        // 继续上一行的表达式或参数列表：Eraser。
        Eraser
    };

    // 静态函数 tool_row_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect tool_row_rect();
    // 静态函数 canvas_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect canvas_rect();
    // 静态函数 tool_btn_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect tool_btn_rect(uint8_t idx);
    // 静态函数 hit_tool_btn：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static uint8_t hit_tool_btn(int16_t x, int16_t y);

    // 静态函数 exit_dialog_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect exit_dialog_rect();

    // 函数调用：bool ensure_buffer();；执行该函数并处理它的返回值或副作用。
    bool ensure_buffer();
    // 函数调用：uint8_t brush_half() const;；执行该函数并处理它的返回值或副作用。
    uint8_t brush_half() const;
    // 函数调用：bool canvas_has_strokes() const;；执行该函数并处理它的返回值或副作用。
    bool canvas_has_strokes() const;

    // 函数调用：void stamp(int16_t cx, int16_t cy);；执行该函数并处理它的返回值或副作用。
    void stamp(int16_t cx, int16_t cy);
    // 函数调用：void stroke_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1);；执行该函数并处理它的返回值或副作用。
    void stroke_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
    // 函数调用：void flush_dirty(bool force);；执行该函数并处理它的返回值或副作用。
    void flush_dirty(bool force);
    // 函数调用：void end_stroke();；执行该函数并处理它的返回值或副作用。
    void end_stroke();

    // 函数调用：void clear_canvas();；执行该函数并处理它的返回值或副作用。
    void clear_canvas();
    // 函数调用：bool save_bmp(char* out_name, uint8_t name_cap);；执行该函数并处理它的返回值或副作用。
    bool save_bmp(char* out_name, uint8_t name_cap);
    // 函数调用：bool save_session();；执行该函数并处理它的返回值或副作用。
    bool save_session();
    // 函数调用：bool load_session();；执行该函数并处理它的返回值或副作用。
    bool load_session();
    // 函数调用：bool load_bmp_file(const char* path);；执行该函数并处理它的返回值或副作用。
    bool load_bmp_file(const char* path);
    // 函数调用：void remove_session();；执行该函数并处理它的返回值或副作用。
    void remove_session();
    // 函数调用：void reset_canvas_white();；执行该函数并处理它的返回值或副作用。
    void reset_canvas_white();

    // 静态函数 consume_pending_open：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool consume_pending_open(char* path, size_t cap);

    // 函数调用：void paint_tool_row(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_tool_row(gfx::Canvas& c);
    // 函数调用：void paint_exit_dialog(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_exit_dialog(gfx::Canvas& c);
    // 函数调用：shell::InputResult handle_tool_tap(uint8_t idx);；执行该函数并处理它的返回值或副作用。
    shell::InputResult handle_tool_tap(uint8_t idx);
    // 函数调用：shell::InputResult handle_exit_dialog(int16_t x, int16_t y);；执行该函数并处理它的返回值或副作用。
    shell::InputResult handle_exit_dialog(int16_t x, int16_t y);
    // 函数调用：void show_exit_dialog();；执行该函数并处理它的返回值或副作用。
    void show_exit_dialog();
    // 函数调用：void finish_exit(bool discard);；执行该函数并处理它的返回值或副作用。
    void finish_exit(bool discard);

    // 执行语句：uint8_t* buf_{nullptr};；它会更新一次变量、对象或程序状态。
    uint8_t* buf_{nullptr};
    // 执行语句：Tool tool_{Tool::Pen};；它会更新一次变量、对象或程序状态。
    Tool tool_{Tool::Pen};
    // 执行语句：uint8_t size_idx_{1};；它会更新一次变量、对象或程序状态。
    uint8_t size_idx_{1};
    // 执行语句：bool stroking_{false};；它会更新一次变量、对象或程序状态。
    bool stroking_{false};
    // 执行语句：bool exit_dialog_{false};；它会更新一次变量、对象或程序状态。
    bool exit_dialog_{false};
    // 执行语句：int16_t last_x_{0};；它会更新一次变量、对象或程序状态。
    int16_t last_x_{0};
    // 执行语句：int16_t last_y_{0};；它会更新一次变量、对象或程序状态。
    int16_t last_y_{0};
    // 声明并默认初始化对象：core::Rect dirty_{}。
    core::Rect dirty_{};
    // 执行语句：int64_t last_flush_ms_{0};；它会更新一次变量、对象或程序状态。
    int64_t last_flush_ms_{0};
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::drawing
