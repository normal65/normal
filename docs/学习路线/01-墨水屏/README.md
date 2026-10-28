# 01 · 墨水屏

## 这个模块学什么

- 墨水屏怎么上电、复位、初始化
- 4 线 SPI 的 CS / D/C# / SCL / SDA 怎么用
- SSD1680 的命令：0x01、0x11、0x12、0x18、0x20、0x22、0x24、0x26、0x3C、0x44、0x45、0x4E、0x4F
- Full / Fast / Partial 三种刷新波形
- 局刷基线和残影

## 文档

- [墨水屏启动流程详解.md](墨水屏启动流程详解.md)

## 手册

- 面板手册：`docs/datasheets/gdey027t91/GDEY027T91-FT11.pdf`
- 控制器手册：`docs/datasheets/ssd1680/SSD1680.pdf`

## 关键代码

- `main/bsp/driver/gdey027t91/gdey027t91.cc`
- `main/bsp/driver/gdey027t91/gdey027t91.hpp`
- `main/bsp/driver/gdey027t91/ssd1680.hpp`
- `main/bsp/driver/gdey027t91/panel.hpp`
- `main/bsp/driver/gdey027t91/framebuffer.hpp`
- `main/bsp/driver/gdey027t91/backlight.cc`

## 学完自测

1. RST / BUSY / D/C# / CS# 分别是什么？
2. 0x12 是什么命令？
3. Full / Fast / Partial 有什么区别？
4. 为什么第一次刷新要用 Full？
5. 为什么局刷要按 8 像素对齐？
