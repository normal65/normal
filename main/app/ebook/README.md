# Ebook 框架说明

## 目录

```
app/ebook/
├── router/           # 路由 + 刷新边表（改刷新策略改 refresh_edges.cc）
├── presenter/        # 上屏管线
├── composer/         # 帧合成
├── display/          # DisplayPort 驱动适配
├── shell/            # 系统页面 Lock/Home/AppGrid/AppHost
├── apps/             # Reader / Settings / Files 等内置应用
├── overlays/         # StatusBar / Toast / Keyboard / ControlCenter / ConfirmPrompt
├── ui/               # 控件、dialog、ui_loop
├── ime/              # 输入法引擎
├── gfx/              # Canvas / Font / Icon
├── input/            # 触摸 / 拨码
├── data/             # Persist / SystemState
├── platform/         # boot / 电量 / 音频
└── text/             # 编码与路径
```

## 指定跳转刷新

编辑 `router/refresh_edges.cc` 中 `kEdges[]`，为每条 `from → to` 配置 `RefreshIntent`。

调用方也可显式覆盖：

```cpp
router::RefreshIntent force = router::intent_fast_full();
router::Router::instance().navigate(target, router::NavAction::Forward, &force);
```

## App 内刷新与跳转

```cpp
request_repaint();                           // 局刷（默认 intent_partial_full）
request_repaint(router::intent_fast_full()); // 快刷
navigate_page(sub_page_id);                  // App 子页跳转，走 refresh_edges 边表
```

## 用户内容路径

各内容类 App 只扫描固定子目录（`/int/Ebook/...` 与 `/sd/Ebook/...`），约定与说明见仓库根 [README.md](../../../README.md) 与 [assets/fat_userdata/README.txt](../../../assets/fat_userdata/README.txt)。
