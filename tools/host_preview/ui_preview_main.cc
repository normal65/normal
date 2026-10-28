#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

#include "bsp/driver/gdey027t91/framebuffer.hpp"
#include "gfx/canvas.hpp"
#include "gfx/icon.hpp"
#include "ui/theme.hpp"
#include "ui/widgets.hpp"

namespace {

using app::bsp::driver::gdey027t91::Panel;
using app::ebook::core::Rect;
using app::ebook::gfx::Canvas;
using app::ebook::gfx::FontFace;
using app::ebook::gfx::HAlign;
using app::ebook::gfx::Ink;
using app::ebook::gfx::VAlign;
using app::ebook::ui::Theme;
using app::ebook::ui::widgets::RowStyle;

struct PreviewPage
{
    std::string id;
    std::string title;
    std::function<void(Canvas&)> draw;
};

void status(Canvas& c, const char* time_text, uint8_t battery)
{
    app::ebook::ui::widgets::label(c, 4, 2, time_text, Theme::kFontSmall);
    app::ebook::ui::widgets::battery_indicator(c, static_cast<int16_t>(Theme::kScreenW - 4), 2,
                                               battery, Theme::kStatusBarH);
}

void title(Canvas& c, const char* text)
{
    status(c, "09:41", 76);
    app::ebook::ui::widgets::toolbar(c, text);
}

void list_page(Canvas& c, const char* const* labels, const char* const* values, int count,
               bool chevron = true)
{
    int16_t y = static_cast<int16_t>(Theme::kListStartY + 4);
    for (int i = 0; i < count; ++i)
    {
        RowStyle style{};
        style.label = labels[i];
        style.value = values != nullptr ? values[i] : nullptr;
        style.show_chevron = chevron;
        app::ebook::ui::widgets::list_row(c, Rect{0, y, Theme::kScreenW, Theme::kListRowH}, style);
        y = static_cast<int16_t>(y + Theme::kListRowH);
    }
}

void page_lock(Canvas& c)
{
    c.clear(Ink::White);
    status(c, "08:42", 64);
    app::ebook::ui::widgets::label_in(c, Rect{0, 42, Theme::kScreenW, 32}, "08:42", HAlign::Center,
                                      VAlign::Top, Theme::kFontHuge);
    app::ebook::ui::widgets::label_in(c, Rect{0, 74, Theme::kScreenW, 18}, "10 月 14 日 星期三",
                                      HAlign::Center, VAlign::Top, Theme::kFontSmall);
    app::ebook::ui::widgets::card_border(c, Rect{10, 112, 156, 78});
    app::ebook::ui::widgets::label_in(c, Rect{16, 120, 144, 60}, "慢慢来，比较快。", HAlign::Center,
                                      VAlign::Middle, Theme::kFontBody);
    app::ebook::ui::widgets::label_in(c, Rect{0, 222, Theme::kScreenW, 18}, "向上滑动解锁",
                                      HAlign::Center, VAlign::Top, Theme::kFontSmall);
}

void page_home(Canvas& c)
{
    c.clear(Ink::White);
    status(c, "09:10", 88);
    app::ebook::ui::widgets::label(c, 8, 28, "继续阅读", Theme::kFontTitle);
    app::ebook::ui::widgets::card_border(c, Rect{8, 52, 160, 82});
    app::ebook::ui::widgets::label(c, 16, 62, "嵌入式 C++ 入门", Theme::kFontBody);
    app::ebook::ui::widgets::label(c, 16, 84, "第 3 章 / 共 12 章", Theme::kFontSmall);
    app::ebook::ui::widgets::progress_bar(c, Rect{16, 112, 144, 8}, 36);

    const char* labels[] = {"阅读", "笔记本", "音乐", "设置"};
    const uint32_t icons[] = {app::ebook::gfx::icon::kFaBookReader,
                              app::ebook::gfx::icon::kFaStickyNote,
                              app::ebook::gfx::icon::kFaMusic,
                              app::ebook::gfx::icon::kFaCog};
    for (int i = 0; i < 4; ++i)
    {
        const Rect cell = app::ebook::ui::widgets::grid_cell(Rect{6, 146, 164, 100}, 2, 2, i, 6);
        app::ebook::ui::widgets::card_border(c, cell);
        c.glyph(static_cast<int16_t>(cell.x + 40), static_cast<int16_t>(cell.y + 10), icons[i],
                Theme::kFontIconLg, FontFace::Icon);
        app::ebook::ui::widgets::label_in(c, Rect{cell.x, static_cast<int16_t>(cell.y + 42), cell.w, 18},
                                          labels[i], HAlign::Center, VAlign::Top, Theme::kFontSmall);
    }
}

void page_app_grid(Canvas& c)
{
    c.clear(Ink::White);
    title(c, "应用");
    const char* labels[] = {"阅读", "记事", "相册", "画板", "木鱼", "音乐",
                            "天气", "时钟", "日历", "文件", "更新", "设置"};
    const uint32_t icons[] = {
        app::ebook::gfx::icon::kFaBookReader, app::ebook::gfx::icon::kFaStickyNote,
        app::ebook::gfx::icon::kFaImages,     app::ebook::gfx::icon::kFaPaintBrush,
        app::ebook::gfx::icon::kEbWoodenFish, app::ebook::gfx::icon::kFaMusic,
        app::ebook::gfx::icon::kFaCloudSun,   app::ebook::gfx::icon::kFaClock,
        app::ebook::gfx::icon::kFaCalendarAlt, app::ebook::gfx::icon::kFaFolder,
        app::ebook::gfx::icon::kFaDownload,   app::ebook::gfx::icon::kFaCog};
    for (int i = 0; i < 12; ++i)
    {
        const Rect cell = app::ebook::ui::widgets::grid_cell(
            Rect{0, Theme::kListStartY, Theme::kScreenW, Theme::kScreenH - Theme::kListStartY}, 3,
            4, static_cast<uint8_t>(i), 4);
        c.glyph(static_cast<int16_t>(cell.x + 26), static_cast<int16_t>(cell.y + 8), icons[i],
                Theme::kFontIconLg, FontFace::Icon);
        app::ebook::ui::widgets::label_in(c, Rect{cell.x, static_cast<int16_t>(cell.y + 40), cell.w, 16},
                                          labels[i], HAlign::Center, VAlign::Top, Theme::kFontSmall);
    }
}

void page_reader(Canvas& c)
{
    c.clear(Ink::White);
    title(c, "嵌入式 C++");
    app::ebook::ui::widgets::label(c, 10, 62, "第 3 章  从函数到类", Theme::kFontTitle);
    for (int i = 0; i < 9; ++i)
        app::ebook::ui::widgets::label(c, 10, static_cast<int16_t>(88 + i * 18),
                                       "这里是阅读器正文示例，用于检查行距和版面。",
                                       Theme::kFontSmall);
    app::ebook::ui::widgets::label_in(c, Rect{0, 244, Theme::kScreenW, 16}, "36%", HAlign::Center,
                                      VAlign::Top, Theme::kFontSmall);
}

void page_notepad(Canvas& c)
{
    c.clear(Ink::White);
    title(c, "记事本");
    const char* labels[] = {"新建笔记", "学习计划", "调试记录", "想法"};
    const char* values[] = {"", "10 月 12 日", "10 月 13 日", "10 月 14 日"};
    list_page(c, labels, values, 4, false);
}

void page_gallery(Canvas& c)
{
    c.clear(Ink::White);
    title(c, "相册");
    for (int i = 0; i < 6; ++i)
    {
        const Rect cell = app::ebook::ui::widgets::grid_cell(Rect{4, 58, 168, 190}, 2, 3, i, 6);
        app::ebook::ui::widgets::cover_placeholder(c, cell, "图片");
    }
}

void page_drawing(Canvas& c)
{
    c.clear(Ink::White);
    title(c, "画板");
    app::ebook::ui::widgets::card_border(c, Rect{6, 58, 164, 152});
    c.hline(24, 104, 126, Ink::Black);
    c.rect(Rect{58, 132, 64, 46}, 2);
    app::ebook::ui::widgets::label_in(c, Rect{6, 216, 164, 30}, "画笔  粗细  撤销  保存",
                                      HAlign::Center, VAlign::Middle, Theme::kFontSmall);
}

void page_wooden_fish(Canvas& c)
{
    c.clear(Ink::White);
    title(c, "电子木鱼");
    c.glyph(66, 98, app::ebook::gfx::icon::kEbWoodenFish, 64, FontFace::Icon);
    app::ebook::ui::widgets::label_in(c, Rect{0, 178, Theme::kScreenW, 30}, "功德 128",
                                      HAlign::Center, VAlign::Top, Theme::kFontLarge);
    app::ebook::ui::widgets::label_in(c, Rect{0, 214, Theme::kScreenW, 18}, "点击屏幕敲击",
                                      HAlign::Center, VAlign::Top, Theme::kFontSmall);
}

void page_music(Canvas& c)
{
    c.clear(Ink::White);
    title(c, "音乐");
    app::ebook::ui::widgets::cover_placeholder(c, Rect{38, 66, 100, 100}, "封面");
    app::ebook::ui::widgets::label_in(c, Rect{0, 176, Theme::kScreenW, 20}, "夜航", HAlign::Center,
                                      VAlign::Top, Theme::kFontTitle);
    app::ebook::ui::widgets::label_in(c, Rect{0, 196, Theme::kScreenW, 18}, "示例歌手",
                                      HAlign::Center, VAlign::Top, Theme::kFontSmall);
    app::ebook::ui::widgets::progress_bar(c, Rect{16, 220, 144, 8}, 42);
    app::ebook::ui::widgets::label_in(c, Rect{0, 230, Theme::kScreenW, 20}, "上一首  ▶  下一首",
                                      HAlign::Center, VAlign::Top, Theme::kFontSmall);
}

void page_weather(Canvas& c)
{
    c.clear(Ink::White);
    title(c, "天气");
    app::ebook::ui::widgets::label_in(c, Rect{0, 72, Theme::kScreenW, 24}, "香港", HAlign::Center,
                                      VAlign::Top, Theme::kFontTitle);
    app::ebook::ui::widgets::label_in(c, Rect{0, 98, Theme::kScreenW, 42}, "26°", HAlign::Center,
                                      VAlign::Top, Theme::kFontHuge);
    app::ebook::ui::widgets::label_in(c, Rect{0, 146, Theme::kScreenW, 18}, "多云  26° / 31°",
                                      HAlign::Center, VAlign::Top, Theme::kFontSmall);
    const char* days[] = {"今天", "明天", "后天"};
    const char* temps[] = {"26°", "25°", "27°"};
    list_page(c, days, temps, 3, false);
}

void page_clock(Canvas& c)
{
    c.clear(Ink::White);
    title(c, "时钟");
    app::ebook::ui::widgets::label_in(c, Rect{0, 74, Theme::kScreenW, 48}, "09:41", HAlign::Center,
                                      VAlign::Top, Theme::kFontHuge);
    const char* labels[] = {"07:00", "12:30", "22:00"};
    const char* values[] = {"工作日", "午休", "睡觉"};
    list_page(c, labels, values, 3, false);
}

void page_calendar(Canvas& c)
{
    c.clear(Ink::White);
    title(c, "2026 年 10 月");
    app::ebook::ui::widgets::label_in(c, Rect{0, 58, Theme::kScreenW, 18}, "日 一 二 三 四 五 六",
                                      HAlign::Center, VAlign::Top, Theme::kFontSmall);
    for (int row = 0; row < 5; ++row)
    {
        for (int col = 0; col < 7; ++col)
        {
            const int value = row * 7 + col + 1;
            if (value > 31)
                continue;
            const Rect cell = app::ebook::ui::widgets::grid_cell(
                Rect{4, 78, 168, 160}, 7, 5, static_cast<uint8_t>(row * 7 + col), 2);
            const std::string number = std::to_string(value);
            app::ebook::ui::widgets::label_in(c, cell, number.c_str(), HAlign::Center,
                                              VAlign::Middle, Theme::kFontSmall);
        }
    }
}

void page_files(Canvas& c)
{
    c.clear(Ink::White);
    title(c, "文件");
    const char* labels[] = {"..", "books", "music", "photo.jpg", "notes.txt", "demo.mp3"};
    const char* values[] = {"", "文件夹", "文件夹", "128 KB", "2 KB", "3.1 MB"};
    list_page(c, labels, values, 6, false);
}

void page_update(Canvas& c)
{
    c.clear(Ink::White);
    title(c, "更新");
    app::ebook::ui::widgets::label(c, 12, 76, "当前版本 1.0.0", Theme::kFontBody);
    app::ebook::ui::widgets::label(c, 12, 102, "发现新版本 1.1.0", Theme::kFontBody);
    app::ebook::ui::widgets::progress_bar(c, Rect{12, 136, 152, 12}, 64);
    app::ebook::ui::widgets::label_in(c, Rect{12, 154, 152, 20}, "正在下载 64%", HAlign::Center,
                                      VAlign::Top, Theme::kFontSmall);
}

void page_settings_menu(Canvas& c)
{
    c.clear(Ink::White);
    title(c, "设置");
    const char* labels[] = {"Wi-Fi", "蓝牙", "热点", "显示", "按键", "声音",
                            "时间", "存储", "安全", "关于"};
    const char* values[] = {"已连接", "关闭", "关闭", "亮度 60%", "默认", "70%",
                            "东八区", "32 GB", "PIN", "v1.0.0"};
    list_page(c, labels, values, 10, true);
}

void toggle_page(Canvas& c, const char* page_title, const char* first)
{
    c.clear(Ink::White);
    title(c, page_title);
    const char* labels[] = {"启用", "状态", "高级选项"};
    const char* values[] = {first, "可用", "查看"};
    list_page(c, labels, values, 3, false);
}

void page_wifi(Canvas& c) { toggle_page(c, "Wi-Fi", "已开启"); }
void page_bluetooth(Canvas& c) { toggle_page(c, "蓝牙", "已开启"); }
void page_hotspot(Canvas& c) { toggle_page(c, "热点", "已关闭"); }
void page_display(Canvas& c) { toggle_page(c, "显示", "亮度 60%"); }
void page_keys(Canvas& c) { toggle_page(c, "按键", "Reader"); }
void page_sound(Canvas& c) { toggle_page(c, "声音", "音量 70%"); }
void page_time(Canvas& c) { toggle_page(c, "时间", "自动同步"); }
void page_storage(Canvas& c) { toggle_page(c, "存储", "32 GB 可用"); }
void page_security(Canvas& c) { toggle_page(c, "安全", "PIN 已开启"); }
void page_about(Canvas& c) { toggle_page(c, "关于", "Ebook-S3"); }

void page_keyboard(Canvas& c)
{
    c.clear(Ink::White);
    app::ebook::ui::widgets::label(c, 6, 4, "输入：hello", Theme::kFontSmall);
    const char* rows[] = {"qwertyuiop", "asdfghjkl", "zxcvbnm"};
    for (int r = 0; r < 3; ++r)
    {
        const int count = static_cast<int>(std::char_traits<char>::length(rows[r]));
        for (int i = 0; i < count; ++i)
        {
            const char key[2] = {rows[r][i], '\0'};
            const Rect cell = app::ebook::ui::widgets::grid_cell(
                Rect{4, static_cast<int16_t>(70 + r * 34), 168, 28}, 10, 1,
                static_cast<uint8_t>(i), 2);
            app::ebook::ui::widgets::label_in(c, cell, key, HAlign::Center, VAlign::Middle,
                                              Theme::kFontSmall);
        }
    }
    app::ebook::ui::widgets::label_in(c, Rect{4, 176, 168, 28}, "空格    退格    完成",
                                      HAlign::Center, VAlign::Middle, Theme::kFontSmall);
}

void page_control_center(Canvas& c)
{
    c.clear(Ink::White);
    app::ebook::ui::widgets::label(c, 8, 8, "控制中心", Theme::kFontTitle);
    const char* labels[] = {"Wi-Fi", "蓝牙", "夜间", "音量", "亮度"};
    const char* values[] = {"开", "关", "关", "70%", "60%"};
    list_page(c, labels, values, 5, false);
}

void page_confirm(Canvas& c)
{
    c.clear(Ink::White);
    title(c, "设置");
    app::ebook::ui::widgets::card_border(c, Rect{18, 92, 140, 90});
    app::ebook::ui::widgets::label_in(c, Rect{24, 102, 128, 28}, "确认删除？", HAlign::Center,
                                      VAlign::Middle, Theme::kFontTitle);
    app::ebook::ui::widgets::label_in(c, Rect{24, 136, 128, 16}, "删除后无法恢复", HAlign::Center,
                                      VAlign::Top, Theme::kFontSmall);
    app::ebook::ui::widgets::label_in(c, Rect{28, 156, 54, 20}, "取消", HAlign::Center,
                                      VAlign::Middle, Theme::kFontSmall);
    app::ebook::ui::widgets::label_in(c, Rect{94, 156, 54, 20}, "删除", HAlign::Center,
                                      VAlign::Middle, Theme::kFontSmall);
}

bool is_black(const uint8_t* fb, int x, int y)
{
    const uint32_t index = static_cast<uint32_t>(y) * Panel::STRIDE + (static_cast<uint32_t>(x) / 8U);
    const uint8_t mask = static_cast<uint8_t>(0x80U >> (static_cast<uint32_t>(x) & 7U));
    return (fb[index] & mask) == 0;
}

void write_bmp(const std::filesystem::path& path, const uint8_t* fb)
{
    constexpr int width = Panel::WIDTH;
    constexpr int height = Panel::HEIGHT;
    constexpr int row_bytes = (width * 3 + 3) & ~3;
    const int image_bytes = row_bytes * height;
    const int file_bytes = 54 + image_bytes;

    std::ofstream out(path, std::ios::binary);
    uint8_t header[54]{};
    header[0] = 'B';
    header[1] = 'M';
    *reinterpret_cast<uint32_t*>(&header[2]) = static_cast<uint32_t>(file_bytes);
    *reinterpret_cast<uint32_t*>(&header[10]) = 54;
    *reinterpret_cast<uint32_t*>(&header[14]) = 40;
    *reinterpret_cast<int32_t*>(&header[18]) = width;
    *reinterpret_cast<int32_t*>(&header[22]) = height;
    *reinterpret_cast<uint16_t*>(&header[26]) = 1;
    *reinterpret_cast<uint16_t*>(&header[28]) = 24;
    out.write(reinterpret_cast<const char*>(header), sizeof(header));

    std::vector<uint8_t> row(static_cast<size_t>(row_bytes), 0xFF);
    for (int y = height - 1; y >= 0; --y)
    {
        std::fill(row.begin(), row.end(), 0xFF);
        for (int x = 0; x < width; ++x)
        {
            const uint8_t value = is_black(fb, x, y) ? 0 : 255;
            row[static_cast<size_t>(x * 3)] = value;
            row[static_cast<size_t>(x * 3 + 1)] = value;
            row[static_cast<size_t>(x * 3 + 2)] = value;
        }
        out.write(reinterpret_cast<const char*>(row.data()), row.size());
    }
}

std::vector<PreviewPage> pages()
{
    return {
        {"01-lock", "锁屏", page_lock},
        {"02-home", "主页", page_home},
        {"03-app-grid", "应用网格", page_app_grid},
        {"04-reader", "阅读器", page_reader},
        {"05-notepad", "记事本", page_notepad},
        {"06-gallery", "相册", page_gallery},
        {"07-drawing", "画板", page_drawing},
        {"08-wooden-fish", "电子木鱼", page_wooden_fish},
        {"09-music", "音乐", page_music},
        {"10-weather", "天气", page_weather},
        {"11-clock", "时钟", page_clock},
        {"12-calendar", "日历", page_calendar},
        {"13-files", "文件管理", page_files},
        {"14-update", "更新", page_update},
        {"15-settings-menu", "设置菜单", page_settings_menu},
        {"16-settings-wifi", "Wi-Fi 设置", page_wifi},
        {"17-settings-bluetooth", "蓝牙设置", page_bluetooth},
        {"18-settings-hotspot", "热点设置", page_hotspot},
        {"19-settings-display", "显示设置", page_display},
        {"20-settings-keys", "按键设置", page_keys},
        {"21-settings-sound", "声音设置", page_sound},
        {"22-settings-time", "时间设置", page_time},
        {"23-settings-storage", "存储设置", page_storage},
        {"24-settings-security", "安全设置", page_security},
        {"25-settings-about", "关于", page_about},
        {"26-keyboard", "软键盘", page_keyboard},
        {"27-control-center", "控制中心", page_control_center},
        {"28-confirm-dialog", "确认对话框", page_confirm},
    };
}

void write_index(const std::filesystem::path& dir, const std::vector<PreviewPage>& list)
{
    std::ofstream out(dir / "index.html");
    out << "<!doctype html><html lang=\"zh-CN\"><head><meta charset=\"utf-8\">";
    out << "<title>Ebook UI 布局预览</title><style>";
    out << "body{font-family:Microsoft YaHei;background:#f5f4ef;margin:20px;color:#222}";
    out << ".grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(220px,1fr));gap:18px}";
    out << ".card{background:#fff;border:1px solid #ddd;padding:10px;border-radius:6px}";
    out << "img{width:176px;height:264px;image-rendering:pixelated;border:1px solid #aaa}";
    out << "h1{font-size:22px}h2{font-size:15px;margin:8px 0 4px}";
    out << "</style></head><body><h1>Ebook UI 布局预览</h1>";
    out << "<p>每张图都是当前 UI 代码重新编译后生成的布局预览。</p><div class=\"grid\">";
    for (const auto& page : list)
    {
        out << "<div class=\"card\"><img src=\"pages/" << page.id << ".bmp\">";
        out << "<h2>" << page.title << "</h2><code>" << page.id << ".bmp</code></div>";
    }
    out << "</div></body></html>";
}

} // namespace

int main()
{
    const std::filesystem::path root = "tools/host_preview";
    const std::filesystem::path page_dir = root / "pages";
    std::filesystem::create_directories(page_dir);

    const auto list = pages();
    for (const auto& page : list)
    {
        std::vector<uint8_t> fb(Panel::BUFFER_SIZE, 0xFF);
        Canvas canvas = Canvas::full(fb.data());
        page.draw(canvas);
        write_bmp(page_dir / (page.id + ".bmp"), fb.data());
    }
    write_index(root, list);
    return 0;
}
