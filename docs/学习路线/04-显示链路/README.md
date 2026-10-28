# 04 · 显示链路（待补充）

## 这个模块要学什么

- `UiBus`：跨任务事件队列
- `UiLoop`：UI 线程主循环
- `Router`：页面栈和导航
- `Composer`：把页面、状态栏、弹窗合成到 framebuffer
- `Presenter`：异步上屏、帧合并
- `Canvas`：点、线、矩形、文字绘制

## 前置模块

- 01 墨水屏
- 02 触摸 FT6336U
- 03 按键与拨码

## 关键代码

- `main/app/ebook/ui/ui_bus.cc`
- `main/app/ebook/ui/ui_loop.cc`
- `main/app/ebook/router/router.cc`
- `main/app/ebook/composer/composer.cc`
- `main/app/ebook/presenter/presenter.cc`
- `main/app/ebook/gfx/canvas.hpp`

## 状态

文档待补充。建议先把 01–03 的链路跑通，再看这一层。
