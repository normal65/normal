# 文档目录

本目录存放硬件设计、产品图片与各芯片数据手册。

软件侧用户文档见仓库根目录：

- [README.md](../README.md) — 功能、烧录、分区、用户内容目录
- [coding_style.md](coding_style.md) — 代码格式与命名规范
- [assets/fat_userdata/README.txt](../assets/fat_userdata/README.txt) — `/int` 与 `/sd` 文件布局
- [main/app/ebook/README.md](../main/app/ebook/README.md) — UI 框架与刷新边表
- [patches/README.md](../patches/README.md) — managed 组件补丁

[English](#directory-layout)

## 目录结构

```
docs/
├── hardware/                 # 硬件设计（原理图 + EDA 工程）
│   ├── schematic.pdf
│   └── ProDoc_Board1_1_2026-07-02.epro2
├── images/                   # 产品示意图
│   ├── hardware-structure.png
│   └── pcb-top.png
└── datasheets/               # 数据手册
    ├── esp32-s3/
    ├── ft6336u/
    └── gdey027t91/
```

## 文件清单

### hardware — 硬件设计

| 文件 | 说明 |
|------|------|
| [schematic.pdf](hardware/schematic.pdf) | 主板原理图 |
| [ProDoc_Board1_1_2026-07-02.epro2](hardware/ProDoc_Board1_1_2026-07-02.epro2) | 嘉立创 EDA 工程源文件（Board1 v1.1） |

### images — 产品图

| 文件 | 说明 |
|------|------|
| [hardware-structure.png](images/hardware-structure.png) | 整机结构示意 |
| [pcb-top.png](images/pcb-top.png) | 主板俯视图 |

### datasheets — 数据手册

| 目录 | 文件 | 说明 |
|------|------|------|
| `esp32-s3/` | `C2913192_*.PDF` | ESP32-S3 核心板 / 芯片规格书（立创 C2913192） |
| `ft6336u/` | [D-FT6336U-DataSheet-V1.0.pdf](datasheets/ft6336u/D-FT6336U-DataSheet-V1.0.pdf) | FT6336U 数据手册 |
| `ft6336u/` | `Ft6336GU_Firmware 外部寄存器_*.xlsm` | FT6336 系列固件 / 外部寄存器说明 |
| `gdey027t91/` | [GDEY027T91-FT11.pdf](datasheets/gdey027t91/GDEY027T91-FT11.pdf) | GDEY027T91 墨水屏模组规格 |

---

## Directory layout

```
docs/
├── hardware/                 # Schematics & EDA project
├── images/                   # Product photos (used in README)
└── datasheets/               # Component datasheets
    ├── esp32-s3/
    ├── ft6336u/
    └── gdey027t91/
```

| Path | Description |
|------|-------------|
| `hardware/schematic.pdf` | Main board schematic |
| `hardware/ProDoc_Board1_1_2026-07-02.epro2` | JLC EDA project source (Board1 v1.1) |
| `images/` | Assembly and PCB photos for README |
| `datasheets/` | Official datasheets grouped by part |

Related software docs: root [README.md](../README.md), [coding style](coding_style.md), [userdata layout](../assets/fat_userdata/README.txt), [framework](../main/app/ebook/README.md).
