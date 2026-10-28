/**
 * [初学者导读] 文件：main/app/ebook/apps/settings/settings_storage.cc
 * 作用：显示内部存储、assets 和 SD 卡容量。
 * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。
 * 约束：App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。
 * 详细解释：docs/代码详解-初学者版.md。
 */

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "apps/settings/settings_app.hpp"

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstdio>
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include <cstring>

// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "storage/storage.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/strings.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/theme.hpp"
// 引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。
#include "ui/widgets.hpp"

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace app::ebook::apps::settings {

// 打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。
namespace {

// 名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。
using ui::Theme;

// 命名空间边界：标记项目模块的作用域范围。
} // namespace

// 定义或声明函数 SettingsApp::paint_storage：参数在括号里，返回类型写在函数名前面。
void SettingsApp::paint_storage(gfx::Canvas& c)
{
    // 赋值：把右侧 ::app::common::storage::StorageMgr::get_instance() 的结果保存到左侧 auto& sto。
    auto& sto = ::app::common::storage::StorageMgr::get_instance();

    // 赋值：把右侧 "-" 的结果保存到左侧 char internal_str[24]。
    char internal_str[24] = "-";
    // 赋值：把右侧 "-" 的结果保存到左侧 char assets_str[24]。
    char assets_str[24] = "-";
    // 赋值：把右侧 {} 的结果保存到左侧 char sd_str[40]。
    char sd_str[40] = {};

    // 定义变量 size_t total 并初始化为 0。
    size_t total = 0;
    // 定义变量 size_t used 并初始化为 0。
    size_t used = 0;
    // 判断条件：sto.query_internal_usage(&total, &used) == ESP_OK 是否成立。
    if (sto.query_internal_usage(&total, &used) == ESP_OK)
    {
        // 函数调用：(void)std::snprintf(internal_str, sizeof(internal_str), "%u/%u KB",；执行该函数并处理它的返回值或副作用。
        (void)std::snprintf(internal_str, sizeof(internal_str), "%u/%u KB",
                            // 继续上一行的表达式或参数列表：static_cast<unsigned>(used / 1024),。
                            static_cast<unsigned>(used / 1024),
                            // 继续上一行的表达式或参数列表：static_cast<unsigned>(total / 1024));。
                            static_cast<unsigned>(total / 1024));
    }
    // 判断条件：sto.query_assets_usage(&total, &used) == ESP_OK 是否成立。
    if (sto.query_assets_usage(&total, &used) == ESP_OK)
    {
        // 函数调用：(void)std::snprintf(assets_str, sizeof(assets_str), "%u/%u KB",；执行该函数并处理它的返回值或副作用。
        (void)std::snprintf(assets_str, sizeof(assets_str), "%u/%u KB",
                            // 继续上一行的表达式或参数列表：static_cast<unsigned>(used / 1024),。
                            static_cast<unsigned>(used / 1024),
                            // 继续上一行的表达式或参数列表：static_cast<unsigned>(total / 1024));。
                            static_cast<unsigned>(total / 1024));
    }

    // 定义变量 uint64_t sd_total 并初始化为 0。
    uint64_t sd_total = 0;
    // 定义变量 uint64_t sd_used 并初始化为 0。
    uint64_t sd_used = 0;
    // 判断条件：sto.query_sd_usage(&sd_total, &sd_used) == ESP_OK && sd_total > 0 是否成立。
    if (sto.query_sd_usage(&sd_total, &sd_used) == ESP_OK && sd_total > 0)
    {
        // 函数调用：(void)std::snprintf(sd_str, sizeof(sd_str), "%lu/%lu MB",；执行该函数并处理它的返回值或副作用。
        (void)std::snprintf(sd_str, sizeof(sd_str), "%lu/%lu MB",
                            // 继续上一行的表达式或参数列表：static_cast<unsigned long>(sd_used / (1024ULL * 1024ULL)),。
                            static_cast<unsigned long>(sd_used / (1024ULL * 1024ULL)),
                            // 继续上一行的表达式或参数列表：static_cast<unsigned long>(sd_total / (1024ULL * 1024ULL)));。
                            static_cast<unsigned long>(sd_total / (1024ULL * 1024ULL)));
    }
    // 备用分支：前面的条件不成立时执行这里。
    else
    {
        // 调用 std::strncpy(sd_str, ui::strings::kSetStNotIn, sizeof(sd_str) - 1);，执行这个函数并处理它的返回值或状态变化。
        std::strncpy(sd_str, ui::strings::kSetStNotIn, sizeof(sd_str) - 1);
        // 赋值：把右侧 '\0' 的结果保存到左侧 sd_str[sizeof(sd_str) - 1]。
        sd_str[sizeof(sd_str) - 1] = '\0';
    }

    // 定义变量 int16_t y 并初始化为 Theme::kListStartY。
    int16_t y = Theme::kListStartY;
    // struct RowDef：把相关数据字段组合在一起，表示项目中的一个概念。
    struct RowDef
    {
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* lbl;
        // 只读约束：这个值或对象在当前位置不能被修改。
        const char* val;
    };
    // 只读约束：这个值或对象在当前位置不能被修改。
    const RowDef rows[] = {
        // 执行这条代码：{ui::strings::kSetStInternal, internal_str},。它会更新变量、对象状态或调用链。
        {ui::strings::kSetStInternal, internal_str},
        // 继续上一行的表达式或参数列表：{ui::strings::kSetStAssets, assets_str},。
        {ui::strings::kSetStAssets, assets_str},
        // 继续上一行的表达式或参数列表：{ui::strings::kSetStSd, sd_str},。
        {ui::strings::kSetStSd, sd_str},
    };
    // 循环执行：按括号里的初始化、条件和步进规则重复运行。
    for (const RowDef& r : rows)
    {
        // 声明并默认初始化对象：ui::widgets::RowStyle rs{}。
        ui::widgets::RowStyle rs{};
        // 赋值：把右侧 r.lbl 的结果保存到左侧 rs.label。
        rs.label = r.lbl;
        // 赋值：把右侧 r.val 的结果保存到左侧 rs.value。
        rs.value = r.val;
        // 赋值：把右侧 false 的结果保存到左侧 rs.show_chevron。
        rs.show_chevron = false;
        // 调用 ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);，执行这个函数并处理它的返回值或状态变化。
        ui::widgets::list_row(c, core::Rect{0, y, Theme::kScreenW, Theme::kListRowH}, rs);
        // 赋值：把右侧 static_cast<int16_t>(y + Theme::kListRowH) 的结果保存到左侧 y。
        y = static_cast<int16_t>(y + Theme::kListRowH);
    }
}

// 命名空间结束：离开当前模块作用域。
} // namespace app::ebook::apps::settings
