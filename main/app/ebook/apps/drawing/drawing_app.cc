/**
 * [初学者导读] 文件：main/app/ebook/apps/drawing/drawing_app.cc
 * 作用：实现画板绘制、笔刷、撤销和位图保存。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/drawing/drawing_app.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdlib>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <sys/stat.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <esp_heap_caps.h>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "bsp/driver/gdey027t91/framebuffer.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "common/storage/storage.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "gfx/icon.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "input/input_router.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "overlays/toast.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/dialog.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::drawing {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::dialog::ConfirmHit;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using Fb = ::app::bsp::driver::gdey027t91::Framebuffer;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ::app::common::storage::MountKind;
// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ::app::common::storage::StorageMgr;

// 固定参数 kToolRowH = 28：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kToolRowH = 28;
// 固定参数 kToolBtnNum = 5：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kToolBtnNum = 5;
// 固定参数 kEraserHalf = 5：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kEraserHalf = 5;
// 固定参数 kFlushMs = 80：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr int64_t kFlushMs = 80;
// 固定参数 kExitDlgW = 152：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kExitDlgW = 152;
// 固定参数 kExitDlgH = 76：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint16_t kExitDlgH = 76;

// 固定参数 kEbookRootInt = "/int/Ebook"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kEbookRootInt = "/int/Ebook";
// 固定参数 kEbookRootSd = "/sd/Ebook"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kEbookRootSd = "/sd/Ebook";
// 固定参数 kDrawDirInt = DrawingApp::kSavePathInt：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kDrawDirInt = DrawingApp::kSavePathInt;
// 固定参数 kDrawDirSd = DrawingApp::kSavePathSd：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kDrawDirSd = DrawingApp::kSavePathSd;
/** 会话缓存仅放内部盘，不随用户作品目录走 SD */
// 固定参数 kSessionPath = "/int/Ebook/drawings/session.bin"：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr const char* kSessionPath = "/int/Ebook/drawings/session.bin";

// 静态查表数组 kPenHalf：运行时复用固定数据，避免每次重新计算。
constexpr uint8_t kPenHalf[3] = {0, 1, 2};
// 固定参数 kOpenPathCap = 96：由当前模块定义，用于控制布局、时序或硬件行为。
constexpr uint8_t kOpenPathCap = 96;

// 定义变量 bool s_open_on_enter_ 并初始化为 false。
bool s_open_on_enter_ = false;
// 执行语句：char s_open_path_[kOpenPathCap]{};；它会更新一次变量、对象或程序状态。
char s_open_path_[kOpenPathCap]{};

// 定义或声明函数 wr32：参数在括号里，返回类型写在函数名前面。
void wr32(uint8_t* p, uint32_t v)
{
    // 赋值：把右侧 static_cast<uint8_t>(v) 的结果保存到左侧 p[0]。
    p[0] = static_cast<uint8_t>(v);
    // 赋值：把右侧 static_cast<uint8_t>(v >> 8) 的结果保存到左侧 p[1]。
    p[1] = static_cast<uint8_t>(v >> 8);
    // 赋值：把右侧 static_cast<uint8_t>(v >> 16) 的结果保存到左侧 p[2]。
    p[2] = static_cast<uint8_t>(v >> 16);
    // 赋值：把右侧 static_cast<uint8_t>(v >> 24) 的结果保存到左侧 p[3]。
    p[3] = static_cast<uint8_t>(v >> 24);
}

// 定义或声明函数 mkdir_draw_dirs：参数在括号里，返回类型写在函数名前面。
void mkdir_draw_dirs()
{
    // 赋值：把右侧 StorageMgr::get_instance() 的结果保存到左侧 auto& sto。
    auto& sto = StorageMgr::get_instance();
    // 判断条件：sto.is_mounted(MountKind::Internal) 是否成立。
    if (sto.is_mounted(MountKind::Internal))
    {
        // 函数调用：(void)::mkdir(kEbookRootInt, 0775);；执行该函数并处理它的返回值或副作用。
        (void)::mkdir(kEbookRootInt, 0775);
        // 函数调用：(void)::mkdir(kDrawDirInt, 0775);；执行该函数并处理它的返回值或副作用。
        (void)::mkdir(kDrawDirInt, 0775);
    }
    // 判断条件：sto.is_mounted(MountKind::Sd) 是否成立。
    if (sto.is_mounted(MountKind::Sd))
    {
        // 函数调用：(void)::mkdir(kEbookRootSd, 0775);；执行该函数并处理它的返回值或副作用。
        (void)::mkdir(kEbookRootSd, 0775);
        // 函数调用：(void)::mkdir(kDrawDirSd, 0775);；执行该函数并处理它的返回值或副作用。
        (void)::mkdir(kDrawDirSd, 0775);
    }
}

// 只读约束：这个值或对象在当前位置不能被修改。
const char* preferred_draw_dir()
{
    // 赋值：把右侧 StorageMgr::get_instance() 的结果保存到左侧 auto& sto。
    auto& sto = StorageMgr::get_instance();
    // 判断条件：sto.is_mounted(MountKind::Internal) 是否成立。
    if (sto.is_mounted(MountKind::Internal))
        // 返回 kDrawDirInt 给调用者，表示当前函数的执行结果。
        return kDrawDirInt;
    // 判断条件：sto.is_mounted(MountKind::Sd) 是否成立。
    if (sto.is_mounted(MountKind::Sd))
        // 返回 kDrawDirSd 给调用者，表示当前函数的执行结果。
        return kDrawDirSd;
    // 返回 kDrawDirInt 给调用者，表示当前函数的执行结果。
    return kDrawDirInt;
}

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 DrawingApp::instance：参数在括号里，返回类型写在函数名前面。
DrawingApp& DrawingApp::instance()
{
    // 函数内静态单例：第一次调用时创建，之后一直复用同一个对象。
    static DrawingApp s;
    // 返回 s 给调用者，表示当前函数的执行结果。
    return s;
}

// 定义只读对象或变量：初始化后不能在当前位置修改。
const char* DrawingApp::title() const
{
    // 返回 ui::strings::kAppDrawing 给调用者，表示当前函数的执行结果。
    return ui::strings::kAppDrawing;
}
// 函数定义/声明：调用 DrawingApp::icon_cp()：返回图标 codepoint。图标通常来自字体图标表，所以这里返回的是 uint32t，不是图片路径。。
uint32_t DrawingApp::icon_cp() const
{
    // 返回 gfx::icon::kFaPaintBrush 给调用者，表示当前函数的执行结果。
    return gfx::icon::kFaPaintBrush;
}

// 定义或声明函数 DrawingApp::tool_row_rect：参数在括号里，返回类型写在函数名前面。
core::Rect DrawingApp::tool_row_rect()
{
    // 返回 core::Rect{0, static_cast<int16_t>(Theme::kListStartY), Theme::kScreenW, kToolRowH} 给调用者，表示当前函数的执行结果。
    return core::Rect{0, static_cast<int16_t>(Theme::kListStartY), Theme::kScreenW, kToolRowH};
}

// 定义或声明函数 DrawingApp::canvas_rect：参数在括号里，返回类型写在函数名前面。
core::Rect DrawingApp::canvas_rect()
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y = static_cast<int16_t>(Theme::kListStartY + kToolRowH);
    // 返回 core::Rect{0, y, Theme::kScreenW, static_cast<uint16_t>(Theme::kScreenH - y)} 给调用者，表示当前函数的执行结果。
    return core::Rect{0, y, Theme::kScreenW, static_cast<uint16_t>(Theme::kScreenH - y)};
}

// 定义或声明函数 DrawingApp::tool_btn_rect：参数在括号里，返回类型写在函数名前面。
core::Rect DrawingApp::tool_btn_rect(uint8_t idx)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect bar = tool_row_rect();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x0 = static_cast<int16_t>((bar.w * idx) / kToolBtnNum);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x1 = static_cast<int16_t>((bar.w * (idx + 1U)) / kToolBtnNum);
    // 返回 core::Rect{x0, bar.y, static_cast<uint16_t>(x1 - x0), bar.h} 给调用者，表示当前函数的执行结果。
    return core::Rect{x0, bar.y, static_cast<uint16_t>(x1 - x0), bar.h};
}

// 定义或声明函数 DrawingApp::exit_dialog_rect：参数在括号里，返回类型写在函数名前面。
core::Rect DrawingApp::exit_dialog_rect()
{
    // 返回 ui::dialog::centered(kExitDlgW, kExitDlgH) 给调用者，表示当前函数的执行结果。
    return ui::dialog::centered(kExitDlgW, kExitDlgH);
}

// 定义或声明函数 DrawingApp::hit_tool_btn：参数在括号里，返回类型写在函数名前面。
uint8_t DrawingApp::hit_tool_btn(int16_t x, int16_t y)
{
    // 判断条件：!tool_row_rect().contains(x, y) 是否成立。
    if (!tool_row_rect().contains(x, y))
        // 返回 UINT8_MAX 给调用者，表示当前函数的执行结果。
        return UINT8_MAX;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < kToolBtnNum; ++i)
    {
        // 判断条件：tool_btn_rect(i).contains(x, y) 是否成立。
        if (tool_btn_rect(i).contains(x, y))
            // 返回 i 给调用者，表示当前函数的执行结果。
            return i;
    }
    // 返回 UINT8_MAX 给调用者，表示当前函数的执行结果。
    return UINT8_MAX;
}

// 定义或声明函数 DrawingApp::ensure_buffer：参数在括号里，返回类型写在函数名前面。
bool DrawingApp::ensure_buffer()
{
    // 判断条件：buf_ != nullptr 是否成立。
    if (buf_ != nullptr)
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;

    // 显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。
    buf_ = static_cast<uint8_t*>(
        // 继续上一行的表达式或参数列表：heap_caps_malloc(core::kFbBytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));。
        heap_caps_malloc(core::kFbBytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    // 判断条件：buf_ == nullptr 是否成立。
    if (buf_ == nullptr)
        // 赋值：把右侧 static_cast<uint8_t*>(heap_caps_malloc(core::kFbBytes, MALLOC_CAP_8BIT)) 的结果保存到左侧 buf_。
        buf_ = static_cast<uint8_t*>(heap_caps_malloc(core::kFbBytes, MALLOC_CAP_8BIT));
    // 判断条件：buf_ == nullptr 是否成立。
    if (buf_ == nullptr)
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kDrawNoMem, 1500);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kDrawNoMem, 1500);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 调用 std::memset(buf_, 0xFF, core::kFbBytes);，执行这个函数并处理它的返回值或状态变化。
    std::memset(buf_, 0xFF, core::kFbBytes);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 DrawingApp::reset_canvas_white：参数在括号里，返回类型写在函数名前面。
void DrawingApp::reset_canvas_white()
{
    // 判断条件：buf_ == nullptr 是否成立。
    if (buf_ == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect cv = canvas_rect();
    // 调用 Fb::fill_rect(buf_, static_cast<uint16_t>(cv.x), static_cast<uint16_t>(cv.y), cv.w, cv.h,，执行这个函数并处理它的返回值或状态变化。
    Fb::fill_rect(buf_, static_cast<uint16_t>(cv.x), static_cast<uint16_t>(cv.y), cv.w, cv.h,
                  // 继续上一行的表达式或参数列表：false);。
                  false);
}

// 定义或声明函数 DrawingApp::canvas_has_strokes：参数在括号里，返回类型写在函数名前面。
bool DrawingApp::canvas_has_strokes() const
{
    // 判断条件：buf_ == nullptr 是否成立。
    if (buf_ == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect cv = canvas_rect();
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (int16_t y = cv.y; y < cv.bottom(); ++y)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t* row = buf_ + static_cast<uint32_t>(y) * core::kStride;
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint16_t i = 0; i < core::kStride; ++i)
        {
            // 判断条件：row[i] != 0xFF 是否成立。
            if (row[i] != 0xFF)
                // 返回 true 给调用者，表示当前函数的执行结果。
                return true;
        }
    }
    // 返回 false 给调用者，表示当前函数的执行结果。
    return false;
}

// 定义或声明函数 DrawingApp::save_session：参数在括号里，返回类型写在函数名前面。
bool DrawingApp::save_session()
{
    // 判断条件：buf_ == nullptr || !canvas_has_strokes() 是否成立。
    if (buf_ == nullptr || !canvas_has_strokes())
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 调用 mkdir_draw_dirs();，执行这个函数并处理它的返回值或状态变化。
    mkdir_draw_dirs();

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect cv = canvas_rect();

    // 赋值：把右侧 std::fopen(kSessionPath, "wb") 的结果保存到左侧 FILE* f。
    FILE* f = std::fopen(kSessionPath, "wb");
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 定义变量 bool ok 并初始化为 true。
    bool ok = true;
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (int16_t y = cv.y; ok && y < cv.bottom(); ++y)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const uint8_t* row = buf_ + static_cast<uint32_t>(y) * core::kStride;
        // 赋值：把右侧 (std::fwrite(row, 1, core::kStride, f) == core::kStride) 的结果保存到左侧 ok。
        ok = (std::fwrite(row, 1, core::kStride, f) == core::kStride);
    }
    // 判断条件：!ok 是否成立。
    if (!ok)
    {
        // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(f);
        // 调用 std::remove(kSessionPath)：VFS remove()。
        (void)std::remove(kSessionPath);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
    std::fclose(f);
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 DrawingApp::load_session：参数在括号里，返回类型写在函数名前面。
bool DrawingApp::load_session()
{
    // 判断条件：buf_ == nullptr 是否成立。
    if (buf_ == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 std::fopen(kSessionPath, "rb") 的结果保存到左侧 FILE* f。
    FILE* f = std::fopen(kSessionPath, "rb");
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect cv = canvas_rect();
    // 定义变量 bool ok 并初始化为 true。
    bool ok = true;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (int16_t y = cv.y; ok && y < cv.bottom(); ++y)
    {
        // 赋值：把右侧 buf_ + static_cast<uint32_t>(y) * core::kStride 的结果保存到左侧 uint8_t* row。
        uint8_t* row = buf_ + static_cast<uint32_t>(y) * core::kStride;
        // 赋值：把右侧 (std::fread(row, 1, core::kStride, f) == core::kStride) 的结果保存到左侧 ok。
        ok = (std::fread(row, 1, core::kStride, f) == core::kStride);
    }
    // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
    std::fclose(f);

    // 判断条件：!ok 是否成立。
    if (!ok)
    {
        // 调用 reset_canvas_white();，执行这个函数并处理它的返回值或状态变化。
        reset_canvas_white();
        // 调用 std::remove(kSessionPath)：VFS remove()。
        (void)std::remove(kSessionPath);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 返回 canvas_has_strokes() 给调用者，表示当前函数的执行结果。
    return canvas_has_strokes();
}

// 定义或声明函数 DrawingApp::remove_session：参数在括号里，返回类型写在函数名前面。
void DrawingApp::remove_session()
{
    // 调用 std::remove(kSessionPath)：VFS remove()。
    (void)std::remove(kSessionPath);
}

// 定义或声明函数 DrawingApp::load_bmp_file：参数在括号里，返回类型写在函数名前面。
bool DrawingApp::load_bmp_file(const char* path)
{
    // 判断条件：path == nullptr || !ensure_buffer() 是否成立。
    if (path == nullptr || !ensure_buffer())
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 std::fopen(path, "rb") 的结果保存到左侧 FILE* f。
    FILE* f = std::fopen(path, "rb");
    // 判断条件：f == nullptr 是否成立。
    if (f == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 赋值：把右侧 {} 的结果保存到左侧 uint8_t hdr[62]。
    uint8_t hdr[62] = {};
    // 判断条件：std::fread(hdr, 1, sizeof(hdr), f) != sizeof(hdr) 是否成立。
    if (std::fread(hdr, 1, sizeof(hdr), f) != sizeof(hdr))
    {
        // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(f);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 判断条件：hdr[0] != 'B' || hdr[1] != 'M' || hdr[28] != 1 是否成立。
    if (hdr[0] != 'B' || hdr[1] != 'M' || hdr[28] != 1)
    {
        // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(f);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 定义变量 int32_t w 并初始化为 0。
    int32_t w = 0;
    // 定义变量 int32_t h 并初始化为 0。
    int32_t h = 0;
    // 调用 std::memcpy(&w, hdr + 18, 4);，执行这个函数并处理它的返回值或状态变化。
    std::memcpy(&w, hdr + 18, 4);
    // 调用 std::memcpy(&h, hdr + 22, 4);，执行这个函数并处理它的返回值或状态变化。
    std::memcpy(&h, hdr + 22, 4);
    // 判断条件：w <= 0 || h == 0 是否成立。
    if (w <= 0 || h == 0)
    {
        // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(f);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 判断条件：h < 0 是否成立。
    if (h < 0)
        // 赋值：把右侧 -h 的结果保存到左侧 h。
        h = -h;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect cv = canvas_rect();
    // 判断条件：static_cast<uint16_t>(w) != cv.w || static_cast<uint16_t>(h) > cv.h 是否成立。
    if (static_cast<uint16_t>(w) != cv.w || static_cast<uint16_t>(h) > cv.h)
    {
        // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
        std::fclose(f);
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }

    // 调用 reset_canvas_white();，执行这个函数并处理它的返回值或状态变化。
    reset_canvas_white();

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t row_bytes = static_cast<uint16_t>(((static_cast<uint32_t>(w) + 31U) / 32U) * 4U);
    // 赋值：把右侧 {} 的结果保存到左侧 uint8_t row[64]。
    uint8_t row[64] = {};
    // 定义变量 bool ok 并初始化为 true。
    bool ok = true;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (int32_t row_i = h - 1; ok && row_i >= 0; --row_i)
    {
        // 判断条件：std::fread(row, 1, row_bytes, f) != row_bytes 是否成立。
        if (std::fread(row, 1, row_bytes, f) != row_bytes)
        {
            // 赋值：把右侧 false 的结果保存到左侧 ok。
            ok = false;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        }
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t y = static_cast<int16_t>(cv.y + row_i);
        // 判断条件：y >= cv.bottom() 是否成立。
        if (y >= cv.bottom())
            // 继续下一轮：跳过本次循环剩余代码。
            continue;
        // 调用 std::memcpy(buf_ + static_cast<uint32_t>(y) * core::kStride, row, core::kStride);，执行这个函数并处理它的返回值或状态变化。
        std::memcpy(buf_ + static_cast<uint32_t>(y) * core::kStride, row, core::kStride);
    }
    // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
    std::fclose(f);

    // 判断条件：!ok 是否成立。
    if (!ok)
    {
        // 调用 reset_canvas_white();，执行这个函数并处理它的返回值或状态变化。
        reset_canvas_white();
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    }
    // 返回 canvas_has_strokes() 给调用者，表示当前函数的执行结果。
    return canvas_has_strokes();
}

// 定义或声明函数 DrawingApp::request_open_on_enter：参数在括号里，返回类型写在函数名前面。
void DrawingApp::request_open_on_enter(const char* path)
{
    // 判断条件：path == nullptr || path[0] == '\0' 是否成立。
    if (path == nullptr || path[0] == '\0')
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 函数调用：(void)std::strncpy(s_open_path_, path, kOpenPathCap - 1);；执行该函数并处理它的返回值或副作用。
    (void)std::strncpy(s_open_path_, path, kOpenPathCap - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 s_open_path_[kOpenPathCap - 1]。
    s_open_path_[kOpenPathCap - 1] = '\0';
    // 赋值：把右侧 true 的结果保存到左侧 s_open_on_enter_。
    s_open_on_enter_ = true;
}

// 定义或声明函数 DrawingApp::consume_pending_open：参数在括号里，返回类型写在函数名前面。
bool DrawingApp::consume_pending_open(char* path, size_t cap)
{
    // 判断条件：!s_open_on_enter_ || path == nullptr || cap == 0 是否成立。
    if (!s_open_on_enter_ || path == nullptr || cap == 0)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;
    // 赋值：把右侧 false 的结果保存到左侧 s_open_on_enter_。
    s_open_on_enter_ = false;
    // 函数调用：(void)std::strncpy(path, s_open_path_, cap - 1);；执行该函数并处理它的返回值或副作用。
    (void)std::strncpy(path, s_open_path_, cap - 1);
    // 赋值：把右侧 '\0' 的结果保存到左侧 path[cap - 1]。
    path[cap - 1] = '\0';
    // 返回 true 给调用者，表示当前函数的执行结果。
    return true;
}

// 定义或声明函数 DrawingApp::on_enter：参数在括号里，返回类型写在函数名前面。
void DrawingApp::on_enter()
{
    // 赋值：把右侧 false 的结果保存到左侧 exit_dialog_。
    exit_dialog_ = false;
    // 赋值：把右侧 false 的结果保存到左侧 stroking_。
    stroking_ = false;
    // 赋值：把右侧 {} 的结果保存到左侧 dirty_。
    dirty_ = {};
    // 判断条件：!ensure_buffer() 是否成立。
    if (!ensure_buffer())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 调用 reset_canvas_white();，执行这个函数并处理它的返回值或状态变化。
    reset_canvas_white();
    // 执行语句：char pending[kOpenPathCap];；它会更新一次变量、对象或程序状态。
    char pending[kOpenPathCap];
    // 判断条件：consume_pending_open(pending, sizeof(pending)) 是否成立。
    if (consume_pending_open(pending, sizeof(pending)))
    {
        // 判断条件：!load_bmp_file(pending) 是否成立。
        if (!load_bmp_file(pending))
            // 调用 overlays::Toast::instance().show(ui::strings::kDrawSaveFail, 1500);，执行这个函数并处理它的返回值或状态变化。
            overlays::Toast::instance().show(ui::strings::kDrawSaveFail, 1500);
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 函数调用：(void)load_session();；执行该函数并处理它的返回值或副作用。
        (void)load_session();
    }
    // 调用 input::InputRouter::get_instance().set_profile(input::Profile::Drawing);，执行这个函数并处理它的返回值或状态变化。
    input::InputRouter::get_instance().set_profile(input::Profile::Drawing);
}

// 函数定义/声明：调用 DrawingApp::on_exit()：退出时取消未完成的扫描、关闭弹窗，并确保后台任务不会在对象状态失效后继续写数据。。
void DrawingApp::on_exit()
{
    // 赋值：把右侧 false 的结果保存到左侧 stroking_。
    stroking_ = false;
    // 赋值：把右侧 false 的结果保存到左侧 exit_dialog_。
    exit_dialog_ = false;
    // 调用 input::InputRouter::get_instance().set_profile(input::Profile::Normal);，执行这个函数并处理它的返回值或状态变化。
    input::InputRouter::get_instance().set_profile(input::Profile::Normal);

    // 判断条件：!canvas_has_strokes() 是否成立。
    if (!canvas_has_strokes())
        // 调用 remove_session();，执行这个函数并处理它的返回值或状态变化。
        remove_session();
}

// 函数定义/声明：调用 DrawingApp::on_semantic_back()：处理语义返回，顺序通常是： 1. 如果弹窗打开，先关闭弹窗。 2. 如果列表还在浏览目录，先返回上一级。 3. 如果已经到根卷，返回 false，让 Shell 退出 App。。
bool DrawingApp::on_semantic_back()
{
    // 判断条件：exit_dialog_ 是否成立。
    if (exit_dialog_)
    {
        // 赋值：把右侧 false 的结果保存到左侧 exit_dialog_。
        exit_dialog_ = false;
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 判断条件：canvas_has_strokes() 是否成立。
    if (canvas_has_strokes())
    {
        // 调用 show_exit_dialog();，执行这个函数并处理它的返回值或状态变化。
        show_exit_dialog();
        // 返回 true 给调用者，表示当前函数的执行结果。
        return true;
    }
    // 返回 false 给调用者，表示当前函数的执行结果。
    return false;
}

// 定义或声明函数 DrawingApp::brush_half：参数在括号里，返回类型写在函数名前面。
uint8_t DrawingApp::brush_half() const
{
    // 返回 (tool_ == Tool::Eraser) ? kEraserHalf : kPenHalf[size_idx_ % 3U] 给调用者，表示当前函数的执行结果。
    return (tool_ == Tool::Eraser) ? kEraserHalf : kPenHalf[size_idx_ % 3U];
}

// 定义或声明函数 DrawingApp::stamp：参数在括号里，返回类型写在函数名前面。
void DrawingApp::stamp(int16_t cx, int16_t cy)
{
    // 判断条件：buf_ == nullptr 是否成立。
    if (buf_ == nullptr)
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint8_t half = brush_half();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect r{static_cast<int16_t>(cx - half), static_cast<int16_t>(cy - half),
                       // 继续上一行的表达式或参数列表：static_cast<uint16_t>(2U * half + 1U),。
                       static_cast<uint16_t>(2U * half + 1U),
                       // 继续上一行的表达式或参数列表：static_cast<uint16_t>(2U * half + 1U)};。
                       static_cast<uint16_t>(2U * half + 1U)};
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect cr = core::Rect::intersect(r, canvas_rect());
    // 判断条件：cr.empty() 是否成立。
    if (cr.empty())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 调用 Fb::fill_rect(buf_, static_cast<uint16_t>(cr.x), static_cast<uint16_t>(cr.y), cr.w, cr.h,，执行这个函数并处理它的返回值或状态变化。
    Fb::fill_rect(buf_, static_cast<uint16_t>(cr.x), static_cast<uint16_t>(cr.y), cr.w, cr.h,
                  // 继续上一行的表达式或参数列表：tool_ == Tool::Pen);。
                  tool_ == Tool::Pen);
    // 赋值：把右侧 core::Rect::merge(dirty_, cr) 的结果保存到左侧 dirty_。
    dirty_ = core::Rect::merge(dirty_, cr);
}

// 定义或声明函数 DrawingApp::stroke_line：参数在括号里，返回类型写在函数名前面。
void DrawingApp::stroke_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t dx = static_cast<int16_t>(std::abs(x1 - x0));
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t dy = static_cast<int16_t>(std::abs(y1 - y0));
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t sx = (x0 < x1) ? 1 : -1;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t sy = (y0 < y1) ? 1 : -1;
    // 定义变量 int16_t err 并初始化为 static_cast<int16_t>(dx - dy)。
    int16_t err = static_cast<int16_t>(dx - dy);

    // 循环执行：条件为真时重复运行，条件变化后退出。
    while (true)
    {
        // 调用 stamp(x0, y0);，执行这个函数并处理它的返回值或状态变化。
        stamp(x0, y0);
        // 判断条件：x0 == x1 && y0 == y1 是否成立。
        if (x0 == x1 && y0 == y1)
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const int16_t e2 = static_cast<int16_t>(2 * err);
        // 判断条件：e2 > -dy 是否成立。
        if (e2 > -dy)
        {
            // 赋值：把右侧 static_cast<int16_t>(err - dy) 的结果保存到左侧 err。
            err = static_cast<int16_t>(err - dy);
            // 赋值：把右侧 static_cast<int16_t>(x0 + sx) 的结果保存到左侧 x0。
            x0 = static_cast<int16_t>(x0 + sx);
        }
        // 判断条件：e2 < dx 是否成立。
        if (e2 < dx)
        {
            // 赋值：把右侧 static_cast<int16_t>(err + dx) 的结果保存到左侧 err。
            err = static_cast<int16_t>(err + dx);
            // 赋值：把右侧 static_cast<int16_t>(y0 + sy) 的结果保存到左侧 y0。
            y0 = static_cast<int16_t>(y0 + sy);
        }
    }
}

// 定义或声明函数 DrawingApp::flush_dirty：参数在括号里，返回类型写在函数名前面。
void DrawingApp::flush_dirty(bool force)
{
    // 判断条件：dirty_.empty() 是否成立。
    if (dirty_.empty())
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 判断条件：!request_repaint_if_ready(last_flush_ms_, kFlushMs, force) 是否成立。
    if (!request_repaint_if_ready(last_flush_ms_, kFlushMs, force))
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 {} 的结果保存到左侧 dirty_。
    dirty_ = {};
}

// 定义或声明函数 DrawingApp::end_stroke：参数在括号里，返回类型写在函数名前面。
void DrawingApp::end_stroke()
{
    // 判断条件：!stroking_ 是否成立。
    if (!stroking_)
        // 提前返回：结束当前函数，不返回具体值。
        return;
    // 赋值：把右侧 false 的结果保存到左侧 stroking_。
    stroking_ = false;
    // 调用 flush_dirty(true);，执行这个函数并处理它的返回值或状态变化。
    flush_dirty(true);
}

// 定义或声明函数 DrawingApp::clear_canvas：参数在括号里，返回类型写在函数名前面。
void DrawingApp::clear_canvas()
{
    // 判断条件：!ensure_buffer() 是否成立。
    if (!ensure_buffer())
        // 提前返回：结束当前函数，不返回具体值。
        return;

    // 调用 reset_canvas_white();，执行这个函数并处理它的返回值或状态变化。
    reset_canvas_white();
    // 赋值：把右侧 {} 的结果保存到左侧 dirty_。
    dirty_ = {};
    // 调用 remove_session();，执行这个函数并处理它的返回值或状态变化。
    remove_session();
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 定义或声明函数 DrawingApp::save_bmp：参数在括号里，返回类型写在函数名前面。
bool DrawingApp::save_bmp(char* out_name, uint8_t name_cap)
{
    // 判断条件：buf_ == nullptr 是否成立。
    if (buf_ == nullptr)
        // 返回 false 给调用者，表示当前函数的执行结果。
        return false;

    // 调用 mkdir_draw_dirs();，执行这个函数并处理它的返回值或状态变化。
    mkdir_draw_dirs();

    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* dirs[2] = {preferred_draw_dir(), nullptr};
    // 判断条件：dirs[0] == kDrawDirInt && StorageMgr::get_instance().is_mounted(MountKind::Sd) 是否成立。
    if (dirs[0] == kDrawDirInt && StorageMgr::get_instance().is_mounted(MountKind::Sd))
        // 赋值：把右侧 kDrawDirSd 的结果保存到左侧 dirs[1]。
        dirs[1] = kDrawDirSd;
    // 备用分支：前面的条件不成立时执行这里。
    else if (dirs[0] == kDrawDirSd && StorageMgr::get_instance().is_mounted(MountKind::Internal))
        // 赋值：把右侧 kDrawDirInt 的结果保存到左侧 dirs[1]。
        dirs[1] = kDrawDirInt;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect cv = canvas_rect();
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint16_t row_bytes = static_cast<uint16_t>(((cv.w + 31U) / 32U) * 4U);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint32_t img_bytes = static_cast<uint32_t>(row_bytes) * cv.h;
    // 只读约束：这个值或对象在当前位置不能被修改。
    const uint32_t file_size = 62U + img_bytes;

    // 赋值：把右侧 {} 的结果保存到左侧 uint8_t hdr[62]。
    uint8_t hdr[62] = {};
    // 赋值：把右侧 'B' 的结果保存到左侧 hdr[0]。
    hdr[0] = 'B';
    // 赋值：把右侧 'M' 的结果保存到左侧 hdr[1]。
    hdr[1] = 'M';
    // 调用 wr32(hdr + 2, file_size);，执行这个函数并处理它的返回值或状态变化。
    wr32(hdr + 2, file_size);
    // 调用 wr32(hdr + 10, 62U);，执行这个函数并处理它的返回值或状态变化。
    wr32(hdr + 10, 62U);
    // 调用 wr32(hdr + 14, 40U);，执行这个函数并处理它的返回值或状态变化。
    wr32(hdr + 14, 40U);
    // 调用 wr32(hdr + 18, cv.w);，执行这个函数并处理它的返回值或状态变化。
    wr32(hdr + 18, cv.w);
    // 调用 wr32(hdr + 22, cv.h);，执行这个函数并处理它的返回值或状态变化。
    wr32(hdr + 22, cv.h);
    // 赋值：把右侧 1 的结果保存到左侧 hdr[26]。
    hdr[26] = 1;
    // 赋值：把右侧 1 的结果保存到左侧 hdr[28]。
    hdr[28] = 1;
    // 调用 wr32(hdr + 34, img_bytes);，执行这个函数并处理它的返回值或状态变化。
    wr32(hdr + 34, img_bytes);
    // 调用 wr32(hdr + 46, 2U);，执行这个函数并处理它的返回值或状态变化。
    wr32(hdr + 46, 2U);
    // 赋值：把右侧 0xFF 的结果保存到左侧 hdr[58]。
    hdr[58] = 0xFF;
    // 赋值：把右侧 0xFF 的结果保存到左侧 hdr[59]。
    hdr[59] = 0xFF;
    // 赋值：把右侧 0xFF 的结果保存到左侧 hdr[60]。
    hdr[60] = 0xFF;

    // 执行语句：char path[96]{};；它会更新一次变量、对象或程序状态。
    char path[96]{};
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t di = 0; di < 2; ++di)
    {
        // 判断条件：dirs[di] == nullptr 是否成立。
        if (dirs[di] == nullptr)
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 循环执行：按括号里的初始化、条件和步进规则重复运行。
        for (uint16_t i = 1; i <= 999; ++i)
        {
            // 函数调用：(void)std::snprintf(path, sizeof(path), "%s/draw_%03u.bmp", dirs[di],；执行该函数并处理它的返回值或副作用。
            (void)std::snprintf(path, sizeof(path), "%s/draw_%03u.bmp", dirs[di],
                                // 继续上一行的表达式或参数列表：static_cast<unsigned>(i));。
                                static_cast<unsigned>(i));
            // struct stat：把相关数据字段组合在一起，表示项目中的一个概念。
            struct stat st{};
            // 判断条件：::stat(path, &st) == 0 是否成立。
            if (::stat(path, &st) == 0)
                // 继续下一轮：跳过本次循环剩余代码。
                continue;

            // 赋值：把右侧 std::fopen(path, "wb") 的结果保存到左侧 FILE* f。
            FILE* f = std::fopen(path, "wb");
            // 判断条件：f == nullptr 是否成立。
            if (f == nullptr)
                // 跳出当前循环或 switch，不再执行剩余分支。
                break; // 本盘不可写，试下一盘

            // 定义变量 bool ok 并初始化为 (std::fwrite(hdr, 1, sizeof(hdr), f) == sizeof(hdr))。
            bool ok = (std::fwrite(hdr, 1, sizeof(hdr), f) == sizeof(hdr));
            // 赋值：把右侧 {} 的结果保存到左侧 uint8_t row[32]。
            uint8_t row[32] = {};
            // 循环执行：按括号里的初始化、条件和步进规则重复运行。
            for (int16_t y = static_cast<int16_t>(cv.bottom() - 1); ok && y >= cv.y; --y)
            {
                // 只读约束：这个值或对象在当前位置不能被修改。
                const uint8_t* src = buf_ + static_cast<uint32_t>(y) * core::kStride;
                // 调用 std::memcpy(row, src, core::kStride);，执行这个函数并处理它的返回值或状态变化。
                std::memcpy(row, src, core::kStride);
                // 调用 std::memset(row + core::kStride, 0, static_cast<size_t>(row_bytes) - core::kStride);，执行这个函数并处理它的返回值或状态变化。
                std::memset(row + core::kStride, 0, static_cast<size_t>(row_bytes) - core::kStride);
                // 赋值：把右侧 (std::fwrite(row, 1, row_bytes, f) == row_bytes) 的结果保存到左侧 ok。
                ok = (std::fwrite(row, 1, row_bytes, f) == row_bytes);
            }
            // 调用 std::fclose(f);，执行这个函数并处理它的返回值或状态变化。
            std::fclose(f);

            // 判断条件：!ok 是否成立。
            if (!ok)
            {
                // 调用 std::remove(path)：VFS remove()。
                (void)std::remove(path);
                // 跳出当前循环或 switch，不再执行剩余分支。
                break;
            }

            // 判断条件：out_name != nullptr && name_cap > 0 是否成立。
            if (out_name != nullptr && name_cap > 0)
            {
                // 只读约束：这个值或对象在当前位置不能被修改。
                const char* base = std::strrchr(path, '/');
                // 函数调用：(void)std::snprintf(out_name, name_cap, "%s",；执行该函数并处理它的返回值或副作用。
                (void)std::snprintf(out_name, name_cap, "%s",
                                    // 继续上一行的表达式或参数列表：(base != nullptr) ? (base + 1) : path);。
                                    (base != nullptr) ? (base + 1) : path);
            }
            // 返回 true 给调用者，表示当前函数的执行结果。
            return true;
        }
    }
    // 返回 false 给调用者，表示当前函数的执行结果。
    return false;
}

// 定义或声明函数 DrawingApp::paint_tool_row：参数在括号里，返回类型写在函数名前面。
void DrawingApp::paint_tool_row(gfx::Canvas& c)
{
    // 静态查表数组 kSizeLbl：运行时复用固定数据，避免每次重新计算。
    static constexpr const char* kSizeLbl[3] = {
        // 执行这条代码：ui::strings::kDrawSizeFine,。它会更新变量、对象状态或调用链。
        ui::strings::kDrawSizeFine,
        // 继续上一行的表达式或参数列表：ui::strings::kDrawSizeMid,。
        ui::strings::kDrawSizeMid,
        // 继续上一行的表达式或参数列表：ui::strings::kDrawSizeBold,。
        ui::strings::kDrawSizeBold,
    };

    // 只读约束：这个值或对象在当前位置不能被修改。
    const char* labels[kToolBtnNum] = {
        // 执行这条代码：ui::strings::kDrawPen,   ui::strings::kDrawEraser, kSizeLbl[size_idx_ % 3U],。它会更新变量、对象状态或调用链。
        ui::strings::kDrawPen,   ui::strings::kDrawEraser, kSizeLbl[size_idx_ % 3U],
        // 继续上一行的表达式或参数列表：ui::strings::kDrawClear, ui::strings::kDrawSave,。
        ui::strings::kDrawClear, ui::strings::kDrawSave,
    };

    // 声明并默认初始化对象：gfx::Canvas::TextStyle ts{}。
    gfx::Canvas::TextStyle ts{};
    // 赋值：把右侧 Theme::kFontBody 的结果保存到左侧 ts.size_px。
    ts.size_px = Theme::kFontBody;
    // 赋值：把右侧 gfx::HAlign::Center 的结果保存到左侧 ts.h。
    ts.h = gfx::HAlign::Center;
    // 赋值：把右侧 gfx::VAlign::Middle 的结果保存到左侧 ts.v。
    ts.v = gfx::VAlign::Middle;

    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (uint8_t i = 0; i < kToolBtnNum; ++i)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect btn = tool_btn_rect(i);
        // 调用 rect(btn,1)：clear(ink)： 把整个裁剪区填成指定颜色。它使用 clip 而不是整个屏幕，因此可以在状态栏区域内清空，而不会擦掉下面的 App 内容。。
        c.rect(btn, 1);
        // 调用 text_in(btn,labels[i],ts)：文本对齐。
        c.text_in(btn, labels[i], ts);

        // 只读约束：这个值或对象在当前位置不能被修改。
        const bool active = (i == 0 && tool_ == Tool::Pen) || (i == 1 && tool_ == Tool::Eraser);
        // 判断条件：active 是否成立。
        if (active)
            // 访问对象成员：c.invert(btn);；读取或修改这个对象里的成员状态。
            c.invert(btn);
    }
}

// 定义或声明函数 DrawingApp::paint_exit_dialog：参数在括号里，返回类型写在函数名前面。
void DrawingApp::paint_exit_dialog(gfx::Canvas& c)
{
    // 调用 ui::dialog::paint_confirm(c, exit_dialog_rect(), ui::strings::kDrawExitAsk, true);，执行这个函数并处理它的返回值或状态变化。
    ui::dialog::paint_confirm(c, exit_dialog_rect(), ui::strings::kDrawExitAsk, true);
}

// 定义或声明函数 DrawingApp::paint：参数在括号里，返回类型写在函数名前面。
void DrawingApp::paint(gfx::Canvas& c)
{
    // 调用 title()：返回 App 标题。在文件浏览器内，标题有时会动态反映当前目录，因此可能存在一个 buffer，而不是只返回一个常量字符串。。
    ui::widgets::toolbar(c, title());
    // 调用 paint_tool_row(c);，执行这个函数并处理它的返回值或状态变化。
    paint_tool_row(c);

    // 判断条件：buf_ != nullptr && c.fb() != nullptr 是否成立。
    if (buf_ != nullptr && c.fb() != nullptr)
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const core::Rect cr = core::Rect::intersect(canvas_rect(), c.clip());
        // 判断条件：!cr.empty() 是否成立。
        if (!cr.empty())
        {
            // 调用 Fb::blit_rect(c.fb(), buf_, cr.x, cr.y, cr.w, cr.h, core::kStride);，执行这个函数并处理它的返回值或状态变化。
            Fb::blit_rect(c.fb(), buf_, cr.x, cr.y, cr.w, cr.h, core::kStride);
            // 判断条件：c.dark() 是否成立。
            if (c.dark())
                // 访问对象成员：c.invert(cr);；读取或修改这个对象里的成员状态。
                c.invert(cr);
        }
    }
}

// 定义或声明函数 DrawingApp::paint_overlay：参数在括号里，返回类型写在函数名前面。
void DrawingApp::paint_overlay(gfx::Canvas& c)
{
    // 判断条件：exit_dialog_ 是否成立。
    if (exit_dialog_)
        // 调用 paint_exit_dialog(c);，执行这个函数并处理它的返回值或状态变化。
        paint_exit_dialog(c);
}

// 定义或声明函数 DrawingApp::show_exit_dialog：参数在括号里，返回类型写在函数名前面。
void DrawingApp::show_exit_dialog()
{
    // 判断条件：stroking_ 是否成立。
    if (stroking_)
    {
        // 赋值：把右侧 false 的结果保存到左侧 stroking_。
        stroking_ = false;
        // 赋值：把右侧 {} 的结果保存到左侧 dirty_。
        dirty_ = {};
    }
    // 赋值：把右侧 true 的结果保存到左侧 exit_dialog_。
    exit_dialog_ = true;
    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
}

// 定义或声明函数 DrawingApp::finish_exit：参数在括号里，返回类型写在函数名前面。
void DrawingApp::finish_exit(bool discard)
{
    // 赋值：把右侧 false 的结果保存到左侧 exit_dialog_。
    exit_dialog_ = false;
    // 判断条件：discard 是否成立。
    if (discard)
    {
        // 调用 remove_session();，执行这个函数并处理它的返回值或状态变化。
        remove_session();
        // 调用 reset_canvas_white();，执行这个函数并处理它的返回值或状态变化。
        reset_canvas_white();
    }
    // 备用分支：前面的条件不成立时执行这里。
    else if (!save_session())
    {
        // 调用 overlays::Toast::instance().show(ui::strings::kDrawSaveFail, 1500);，执行这个函数并处理它的返回值或状态变化。
        overlays::Toast::instance().show(ui::strings::kDrawSaveFail, 1500);
    }
    // 调用 exit_to_parent()：它是应用内“返回上一层”的统一入口，最终通常走 Router::back()。子页面或编辑器应该优先调用它，而不是自己 pop 路由栈。。
    exit_to_parent();
}

// 定义或声明函数 DrawingApp::handle_exit_dialog：参数在括号里，返回类型写在函数名前面。
shell::InputResult DrawingApp::handle_exit_dialog(int16_t x, int16_t y)
{
    // 只读约束：这个值或对象在当前位置不能被修改。
    const ConfirmHit hit = ui::dialog::hit_confirm(exit_dialog_rect(), x, y, true);
    // 判断条件：hit == ConfirmHit::Close 是否成立。
    if (hit == ConfirmHit::Close)
    {
        // 赋值：把右侧 false 的结果保存到左侧 exit_dialog_。
        exit_dialog_ = false;
        // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
        request_repaint();
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：hit == ConfirmHit::Yes 是否成立。
    if (hit == ConfirmHit::Yes)
    {
        // 调用 finish_exit(true);，执行这个函数并处理它的返回值或状态变化。
        finish_exit(true);
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 判断条件：hit == ConfirmHit::No 是否成立。
    if (hit == ConfirmHit::No)
    {
        // 调用 finish_exit(false);，执行这个函数并处理它的返回值或状态变化。
        finish_exit(false);
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }
    // 返回 {true} 给调用者，表示当前函数的执行结果。
    return {true};
}

// 定义或声明函数 DrawingApp::handle_tool_tap：参数在括号里，返回类型写在函数名前面。
shell::InputResult DrawingApp::handle_tool_tap(uint8_t idx)
{
    // 多分支判断：根据括号里的值选择不同 case。
    switch (idx)
    {
        // 分支匹配：case 0 成立时执行下面的代码。
        case 0:
            // 赋值：把右侧 Tool::Pen 的结果保存到左侧 tool_。
            tool_ = Tool::Pen;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case 1 成立时执行下面的代码。
        case 1:
            // 赋值：把右侧 Tool::Eraser 的结果保存到左侧 tool_。
            tool_ = Tool::Eraser;
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case 2 成立时执行下面的代码。
        case 2:
            // 赋值：把右侧 static_cast<uint8_t>((size_idx_ + 1U) % 3U) 的结果保存到左侧 size_idx_。
            size_idx_ = static_cast<uint8_t>((size_idx_ + 1U) % 3U);
            // 跳出当前循环或 switch，不再执行剩余分支。
            break;
        // 分支匹配：case 3 成立时执行下面的代码。
        case 3:
            // 调用 clear_canvas();，执行这个函数并处理它的返回值或状态变化。
            clear_canvas();
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        // 分支匹配：case 4:  成立时执行下面的代码。
        case 4: {
            // 执行语句：char name[20];；它会更新一次变量、对象或程序状态。
            char name[20];
            // 判断条件：save_bmp(name, sizeof(name)) 是否成立。
            if (save_bmp(name, sizeof(name)))
            {
                // 执行语句：char msg[48];；它会更新一次变量、对象或程序状态。
                char msg[48];
                // 函数调用：(void)std::snprintf(msg, sizeof(msg), "%s%s", ui::strings::kDrawSaved, name);；执行该函数并处理它的返回值或副作用。
                (void)std::snprintf(msg, sizeof(msg), "%s%s", ui::strings::kDrawSaved, name);
                // 调用 show(msg,1800)：ensuretimer()：第一次调用创建一次性软件定时器，后续修改周期。。
                overlays::Toast::instance().show(msg, 1800);
            }
            // 备用分支：前面的条件不成立时执行这里。
            else
            {
                // 调用 overlays::Toast::instance().show(ui::strings::kDrawSaveFail, 1500);，执行这个函数并处理它的返回值或状态变化。
                overlays::Toast::instance().show(ui::strings::kDrawSaveFail, 1500);
            }
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
        }
        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};
    }

    // 调用 request_repaint()：repaint(intent) 把刷新意图交给 Router。Router 再决定当前页面变化属于哪个刷新边。。
    request_repaint();
    // 返回 {true} 给调用者，表示当前函数的执行结果。
    return {true};
}

// 定义或声明函数 DrawingApp::on_input：参数在括号里，返回类型写在函数名前面。
shell::InputResult DrawingApp::on_input(const ::app::ebook::input::Event& ev)
{
    // 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
    using ::app::ebook::input::EventType;

    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t x = static_cast<int16_t>(ev.x);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const int16_t y = static_cast<int16_t>(ev.y);
    // 只读约束：这个值或对象在当前位置不能被修改。
    const core::Rect cv = canvas_rect();

    // 判断条件：exit_dialog_ 是否成立。
    if (exit_dialog_)
    {
        // 判断条件：ev.type == EventType::Tap 是否成立。
        if (ev.type == EventType::Tap)
            // 返回 handle_exit_dialog(x, y) 给调用者，表示当前函数的执行结果。
            return handle_exit_dialog(x, y);
        // 返回 {true} 给调用者，表示当前函数的执行结果。
        return {true};
    }

    // 多分支判断：根据括号里的值选择不同 case。
    switch (ev.type)
    {
        // 分支匹配：case EventType::Press 成立时执行下面的代码。
        case EventType::Press:
            // 判断条件：cv.contains(x, y) && ensure_buffer() 是否成立。
            if (cv.contains(x, y) && ensure_buffer())
            {
                // 赋值：把右侧 true 的结果保存到左侧 stroking_。
                stroking_ = true;
                // 赋值：把右侧 x 的结果保存到左侧 last_x_。
                last_x_ = x;
                // 赋值：把右侧 y 的结果保存到左侧 last_y_。
                last_y_ = y;
                // 调用 stamp(x, y);，执行这个函数并处理它的返回值或状态变化。
                stamp(x, y);
                // 调用 flush_dirty(false);，执行这个函数并处理它的返回值或状态变化。
                flush_dirty(false);
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            }
            // 返回 {} 给调用者，表示当前函数的执行结果。
            return {};

        // 分支匹配：case EventType::Move 成立时执行下面的代码。
        case EventType::Move:
            // 判断条件：!stroking_ 是否成立。
            if (!stroking_)
                // 返回 {} 给调用者，表示当前函数的执行结果。
                return {};
            // 调用 stroke_line(last_x_, last_y_, x, y);，执行这个函数并处理它的返回值或状态变化。
            stroke_line(last_x_, last_y_, x, y);
            // 赋值：把右侧 x 的结果保存到左侧 last_x_。
            last_x_ = x;
            // 赋值：把右侧 y 的结果保存到左侧 last_y_。
            last_y_ = y;
            // 调用 flush_dirty(false);，执行这个函数并处理它的返回值或状态变化。
            flush_dirty(false);
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};

        // 分支匹配：case EventType::Release 成立时执行下面的代码。
        case EventType::Release:
            // 判断条件：!stroking_ 是否成立。
            if (!stroking_)
                // 返回 {} 给调用者，表示当前函数的执行结果。
                return {};
            // 调用 stroke_line(last_x_, last_y_, x, y);，执行这个函数并处理它的返回值或状态变化。
            stroke_line(last_x_, last_y_, x, y);
            // 调用 end_stroke();，执行这个函数并处理它的返回值或状态变化。
            end_stroke();
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};

        // 分支匹配：case EventType::Tap 成立时执行下面的代码。
        case EventType::Tap:
            // 调用 end_stroke();，执行这个函数并处理它的返回值或状态变化。
            end_stroke();
            // 判断条件：ui::widgets::hit_toolbar_back(x, y) 是否成立。
            if (ui::widgets::hit_toolbar_back(x, y))
            {
                // 判断条件：canvas_has_strokes() 是否成立。
                if (canvas_has_strokes())
                {
                    // 调用 show_exit_dialog();，执行这个函数并处理它的返回值或状态变化。
                    show_exit_dialog();
                    // 返回 {true} 给调用者，表示当前函数的执行结果。
                    return {true};
                }
                // 调用 exit_to_parent()：它是应用内“返回上一层”的统一入口，最终通常走 Router::back()。子页面或编辑器应该优先调用它，而不是自己 pop 路由栈。。
                exit_to_parent();
                // 返回 {true} 给调用者，表示当前函数的执行结果。
                return {true};
            }
            {
                // 只读约束：这个值或对象在当前位置不能被修改。
                const uint8_t idx = hit_tool_btn(x, y);
                // 判断条件：idx != UINT8_MAX 是否成立。
                if (idx != UINT8_MAX)
                    // 返回 handle_tool_tap(idx) 给调用者，表示当前函数的执行结果。
                    return handle_tool_tap(idx);
            }
            // 返回 {cv.contains(x, y)} 给调用者，表示当前函数的执行结果。
            return {cv.contains(x, y)};

        // 分支匹配：case EventType::LongPress 成立时执行下面的代码。
        case EventType::LongPress:
            // 调用 end_stroke();，执行这个函数并处理它的返回值或状态变化。
            end_stroke();
            // 返回 {true} 给调用者，表示当前函数的执行结果。
            return {true};

        // 执行这条代码：default:。它会更新变量、对象状态或调用链。
        default:
            // 调用 end_stroke();，执行这个函数并处理它的返回值或状态变化。
            end_stroke();
            // 返回 {} 给调用者，表示当前函数的执行结果。
            return {};
    }
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::drawing
