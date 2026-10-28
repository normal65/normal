# 02 · 触摸 FT6336U

## 这个模块学什么

- FT6336U 的 I2C 接口和中断引脚
- 上电 / 复位时序（Trst、Trsi）
- I2C 读触点、解析 x/y
- INT 中断 → 信号量 → 输入任务
- Tap / Swipe / LongPress 手势状态机
- 事件通过 UiBus 交给 UI 线程

## 文档

- [FT6336U触摸流程详解.md](FT6336U触摸流程详解.md)

## 手册

- `docs/datasheets/ft6336u/D-FT6336U-DataSheet-V1.0.pdf`

## 关键代码

- `main/bsp/driver/ft6336u/ft6336u.cc`
- `main/bsp/driver/ft6336u/touch_gesture.cc`
- `main/app/ebook/input/input_router.cc`
- `main/app/ebook/input/gesture_tuner.cc`

## 学完自测

1. INT 中断为什么只给信号量发通知？
2. I2C 地址是多少？
3. 一个触点有几个字节？x/y 怎么拼？
4. Tap / Swipe / LongPress 分别在什么条件下产生？
5. 为什么坐标翻转只能在一个地方做？
