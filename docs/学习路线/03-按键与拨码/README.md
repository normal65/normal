# 03 · 按键与拨码

## 这个模块学什么

- 一个 ADC 引脚怎么通过分压区分三档
- ADC 多次采样、平均、校准
- 阈值分类（UP / MID / DOWN / NONE）
- 软件去抖（连续 3 次一致）
- Press / Release 边沿检测
- 事件通过 UiBus 交给 UI 线程

## 文档

- [按键与拨码流程详解.md](按键与拨码流程详解.md)

## 手册

这个模块没有独立芯片手册：

- 分压网络和阈值在本板原理图与 `main/bsp/config/config.hpp`
- ADC 是 ESP32-S3 内置外设

## 关键代码

- `main/bsp/driver/button/button.cc`
- `main/bsp/driver/button/button.hpp`
- `main/app/ebook/input/physical_input.cc`
- `main/app/ebook/input/physical_types.hpp`
- `main/app/ebook/input/input_dispatch.cc`

## 学完自测

1. 为什么一个引脚能表示三个按键？
2. UP / MID / DOWN 的实测电压是多少？
3. 为什么阈值取中间值？
4. 为什么要连续 3 次一致才确认？
5. Press 和 Release 分别在什么时候发？
