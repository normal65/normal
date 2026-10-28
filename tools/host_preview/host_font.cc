#include <windows.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/driver/gdey027t91/framebuffer.hpp"
#include "gfx/font.hpp"
#include "gfx/utf8.hpp"

namespace app::ebook::gfx {

namespace {

constexpr int kMaskSize = 160;

std::wstring utf8_to_wide(const char* text)
{
    if (text == nullptr || *text == '\0')
        return {};
    const int count = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
    if (count <= 1)
        return {};
    std::wstring result(static_cast<size_t>(count - 1), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text, -1, result.data(), count);
    return result;
}

struct Mask
{
    int width{0};
    int height{0};
    std::vector<uint8_t> pixels;
};

Mask render_mask(const std::wstring& text, uint8_t size_px)
{
    Mask result;
    if (text.empty() || size_px == 0)
        return result;

    HDC screen = GetDC(nullptr);
    HDC dc = CreateCompatibleDC(screen);

    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = kMaskSize;
    info.bmiHeader.biHeight = -kMaskSize;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;

    void* raw = nullptr;
    HBITMAP bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &raw, nullptr, 0);
    HGDIOBJ old_bitmap = SelectObject(dc, bitmap);

    std::fill(static_cast<uint8_t*>(raw), static_cast<uint8_t*>(raw) + kMaskSize * kMaskSize * 4, 0);
    HFONT font = CreateFontW(-static_cast<int>(size_px), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei");
    HGDIOBJ old_font = SelectObject(dc, font);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(255, 255, 255));
    TextOutW(dc, 0, 0, text.c_str(), static_cast<int>(text.size()));

    SIZE extent{};
    GetTextExtentPoint32W(dc, text.c_str(), static_cast<int>(text.size()), &extent);
    result.width = std::clamp<int>(extent.cx, 1, kMaskSize);
    result.height = std::clamp<int>(extent.cy, 1, kMaskSize);
    result.pixels.assign(static_cast<size_t>(result.width * result.height), 0);

    const uint8_t* pixels = static_cast<const uint8_t*>(raw);
    for (int y = 0; y < result.height; ++y)
    {
        for (int x = 0; x < result.width; ++x)
        {
            const uint8_t* p = pixels + (static_cast<size_t>(y) * kMaskSize + x) * 4;
            result.pixels[static_cast<size_t>(y) * result.width + x] = p[0] > 80 ? 1 : 0;
        }
    }

    SelectObject(dc, old_font);
    DeleteObject(font);
    SelectObject(dc, old_bitmap);
    DeleteObject(bitmap);
    DeleteDC(dc);
    ReleaseDC(nullptr, screen);
    return result;
}

void blit_mask(uint8_t* fb, int16_t x, int16_t y_top, const Mask& mask, Ink ink,
               const core::Rect& clip)
{
    const bool black = ink == Ink::Black;
    for (int row = 0; row < mask.height; ++row)
    {
        for (int col = 0; col < mask.width; ++col)
        {
            if (mask.pixels[static_cast<size_t>(row) * mask.width + col] == 0)
                continue;
            const int16_t px = static_cast<int16_t>(x + col);
            const int16_t py = static_cast<int16_t>(y_top + row);
            if (clip.contains(px, py))
                ::app::bsp::driver::gdey027t91::Framebuffer::set_pixel(fb, px, py, black);
        }
    }
}

} // namespace

Font& Font::get_instance()
{
    static Font instance;
    return instance;
}

bool Font::init()
{
    return true;
}

void Font::deinit()
{
}

uint16_t Font::draw_glyph(uint8_t* fb, int16_t x, int16_t y_top, uint32_t cp, uint8_t size_px,
                          FontFace, Ink ink, const core::Rect& clip)
{
    wchar_t text[2] = {static_cast<wchar_t>(cp), L'\0'};
    const Mask mask = render_mask(text, size_px);
    blit_mask(fb, x, y_top, mask, ink, clip);
    return static_cast<uint16_t>(mask.width > 0 ? mask.width : size_px);
}

uint16_t Font::draw_text(uint8_t* fb, int16_t x, int16_t y_top, const char* utf8, uint8_t size_px,
                         FontFace face, Ink ink, const core::Rect& clip)
{
    if (utf8 == nullptr)
        return 0;
    int16_t cursor = x;
    const char* p = utf8;
    uint32_t cp = 0;
    while (utf8_next(p, cp))
        cursor = static_cast<int16_t>(cursor + draw_glyph(fb, cursor, y_top, cp, size_px, face, ink, clip));
    return static_cast<uint16_t>(cursor - x);
}

uint16_t Font::advance(uint32_t cp, uint8_t size_px, FontFace)
{
    wchar_t text[2] = {static_cast<wchar_t>(cp), L'\0'};
    const Mask mask = render_mask(text, size_px);
    return static_cast<uint16_t>(mask.width > 0 ? mask.width : size_px);
}

uint16_t Font::measure(const char* utf8, uint8_t size_px, FontFace)
{
    const std::wstring text = utf8_to_wide(utf8);
    if (text.empty() || size_px == 0)
        return 0;

    HDC screen = GetDC(nullptr);
    HDC dc = CreateCompatibleDC(screen);
    HFONT font = CreateFontW(-static_cast<int>(size_px), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei");
    HGDIOBJ old_font = SelectObject(dc, font);
    SIZE extent{};
    GetTextExtentPoint32W(dc, text.c_str(), static_cast<int>(text.size()), &extent);
    SelectObject(dc, old_font);
    DeleteObject(font);
    DeleteDC(dc);
    ReleaseDC(nullptr, screen);
    return static_cast<uint16_t>(extent.cx);
}

uint16_t Font::line_height(uint8_t size_px, FontFace)
{
    return static_cast<uint16_t>(size_px + 4);
}

uint16_t Font::ascent(uint8_t size_px, FontFace)
{
    return static_cast<uint16_t>(size_px > 2 ? size_px - 2 : size_px);
}

} // namespace app::ebook::gfx
