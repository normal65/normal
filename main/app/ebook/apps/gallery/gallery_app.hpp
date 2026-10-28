// 头文件保护：保证这个头文件在一次编译单元里只被包含一次。
#pragma once

/**
 * [初学者导读] 文件：main/app/ebook/apps/gallery/gallery_app.hpp
 * 作用：实现相册列表、图片解码、缩放和查看。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstddef>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdint>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <memory>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/app.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "system/task/task.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/list_view.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/ui_event.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::gallery {

/** @brief 相册：扫描 photos 目录，浏览 JPEG/JPG 图片 */
// 类定义：把数据和操作这些数据的方法绑定在一起。
class GalleryApp : public App
{
  // 访问控制：决定后面的成员能否被外部代码访问。
  public:
    // 静态函数 instance：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static GalleryApp& instance();

    // 定义或声明函数 id：参数在括号里，返回类型写在函数名前面。
    AppId id() const override
    {
        // 返回 AppId::Gallery 给调用者，表示当前函数的执行结果。
        return AppId::Gallery;
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
    // 调用 on_input(const::app::ebook::input::Event&ev)：文件 App 的输入总入口。它处理： 触摸按下、移动、抬起和滑动。 工具栏返回。 列表滚动节流。 弹窗点击优先于底层列表点击。。
    shell::InputResult on_input(const ::app::ebook::input::Event& ev) override;
    // 调用 on_ui_event(constui::UiEvent&ev)：处理 UiBus 消息。文件模块最关心的是 FilesReloadDone：收到它以后，在 UI 线程调用 onbrowseready()。。
    void on_ui_event(const ui::UiEvent& ev) override;

    // 静态函数 request_open_on_enter：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void request_open_on_enter(const char* path);

    /** 与 is_jpeg_name() / scan_dir() 一致，供空状态提示 */
    // 固定参数 kScanFmtHint = "JPG / JPEG"：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr const char* kScanFmtHint = "JPG / JPEG";
    // 固定参数 kScanPathInt = "/int/Ebook/photos"：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr const char* kScanPathInt = "/int/Ebook/photos";
    // 固定参数 kScanPathSd = "/sd/Ebook/photos"：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr const char* kScanPathSd = "/sd/Ebook/photos";

  // 访问控制：决定后面的成员能否被外部代码访问。
  private:
    // 调用 GalleryApp();，执行这个函数并处理它的返回值或状态变化。
    GalleryApp();

    // 枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。
    enum class View : uint8_t
    {
        // 执行这条代码：List = 0,。它会更新变量、对象状态或调用链。
        List = 0,
        // 继续上一行的表达式或参数列表：Viewer。
        Viewer
    };

    // 函数调用：void start_scan();；执行该函数并处理它的返回值或副作用。
    void start_scan();
    // 函数调用：void scan_photos();；执行该函数并处理它的返回值或副作用。
    void scan_photos();
    // 函数调用：void open_photo(uint8_t idx);；执行该函数并处理它的返回值或副作用。
    void open_photo(uint8_t idx);
    // 函数调用：void show_prev();；执行该函数并处理它的返回值或副作用。
    void show_prev();
    // 函数调用：void show_next();；执行该函数并处理它的返回值或副作用。
    void show_next();
    // 函数调用：void start_decode(const char* path);；执行该函数并处理它的返回值或副作用。
    void start_decode(const char* path);
    // 函数调用：void cancel_decode();；执行该函数并处理它的返回值或副作用。
    void cancel_decode();

    // 调用 fill_row(uint8_tidx,ui::widgets::RowStyle&rs)：给 ListView 的某一行填入样式和内容。它负责： 处理“返回上一级”行。 处理根卷行。 处理普通目录行。 处理普通文件行。 显示截断或加载提示。。
    void fill_row(uint8_t idx, ui::widgets::RowStyle& rs);
    // 调用 on_row_tap(uint8_tidx)：处理列表点击： 如果点击父目录行，调用 goup()。 如果点击根卷，调用 openroot() 并调度扫描。 如果点击目录，调用 enterdir()。 如果点击文件，调用 showfileaction()。。
    void on_row_tap(uint8_t idx);

    // 函数调用：void paint_list(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_list(gfx::Canvas& c);
    // 函数调用：void paint_viewer(gfx::Canvas& c);；执行该函数并处理它的返回值或副作用。
    void paint_viewer(gfx::Canvas& c);

    // 静态函数 title_from_path：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void title_from_path(const char* path, char* out, size_t out_size);
    // 静态函数 is_jpeg_name：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static bool is_jpeg_name(const char* name);
    // 函数调用：void scan_dir(const char* dir, uint8_t& count);；执行该函数并处理它的返回值或副作用。
    void scan_dir(const char* dir, uint8_t& count);

    // 静态函数 decode_worker：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static void decode_worker(void* arg);

    // 静态函数 viewer_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect viewer_rect();
    // 静态函数 viewer_image_rect：不创建对象也可以调用，属于当前类型或文件的辅助功能。
    static core::Rect viewer_image_rect();

    // 执行语句：View view_{View::List};；它会更新一次变量、对象或程序状态。
    View view_{View::List};
    // 执行语句：uint8_t sel_{0};；它会更新一次变量、对象或程序状态。
    uint8_t sel_{0};
    // 执行语句：bool scanning_{false};；它会更新一次变量、对象或程序状态。
    bool scanning_{false};
    // 执行语句：bool decoding_{false};；它会更新一次变量、对象或程序状态。
    bool decoding_{false};
    // 执行语句：uint8_t decode_job_id_{0};；它会更新一次变量、对象或程序状态。
    uint8_t decode_job_id_{0};
    // 执行语句：uint8_t decode_err_{0};；它会更新一次变量、对象或程序状态。
    uint8_t decode_err_{0};
    // 执行语句：bool image_ready_{false};；它会更新一次变量、对象或程序状态。
    bool image_ready_{false};

    // struct PhotoItem：把相关数据字段组合在一起，表示项目中的一个概念。
    struct PhotoItem
    {
        // 执行语句：char path[128]{};；它会更新一次变量、对象或程序状态。
        char path[128]{};
        // 执行语句：char title[40]{};；它会更新一次变量、对象或程序状态。
        char title[40]{};
        // 执行语句：uint32_t size{0};；它会更新一次变量、对象或程序状态。
        uint32_t size{0};
    };

    // 固定参数 kMaxPhotos = 48：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint8_t kMaxPhotos = 48;
    // 任务栈大小：8192，栈太小会崩溃，太大会浪费 RAM。
    static constexpr uint32_t kDecodeStack = 8192;
    // 任务栈大小：3072，栈太小会崩溃，太大会浪费 RAM。
    static constexpr uint32_t kScanStack = 3072;
    // 固定参数 kMaxJpegBytes = 2U * 1024U * 1024U：由当前模块定义，用于控制布局、时序或硬件行为。
    static constexpr uint32_t kMaxJpegBytes = 2U * 1024U * 1024U;

    // 函数调用：bool ensure_store();；执行该函数并处理它的返回值或副作用。
    bool ensure_store();

    // 执行语句：PhotoItem* photos_{nullptr};；它会更新一次变量、对象或程序状态。
    PhotoItem* photos_{nullptr};
    // 调用 char (*size_buf_)[12]{nullptr};，执行这个函数并处理它的返回值或状态变化。
    char (*size_buf_)[12]{nullptr};
    // 执行语句：uint8_t count_{0};；它会更新一次变量、对象或程序状态。
    uint8_t count_{0};

    // 执行语句：char path_[128]{};；它会更新一次变量、对象或程序状态。
    char path_[128]{};
    // 执行语句：char title_buf_[40]{};；它会更新一次变量、对象或程序状态。
    char title_buf_[40]{};

    // 执行语句：uint8_t* viewer_fb_{nullptr};；它会更新一次变量、对象或程序状态。
    uint8_t* viewer_fb_{nullptr};
    // 声明并默认初始化对象：core::Rect image_rect_{}。
    core::Rect image_rect_{};

    // 执行语句：std::unique_ptr<::app::sys::task::Task> scan_task_;；它会更新一次变量、对象或程序状态。
    std::unique_ptr<::app::sys::task::Task> scan_task_;
    // 执行语句：std::unique_ptr<::app::sys::task::Task> decode_task_;；它会更新一次变量、对象或程序状态。
    std::unique_ptr<::app::sys::task::Task> decode_task_;
    // 执行语句：ui::ListView list_;；它会更新一次变量、对象或程序状态。
    ui::ListView list_;
};

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::gallery
