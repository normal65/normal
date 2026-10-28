# -*- coding: utf-8 -*-
"""生成《Ebook 项目架构与工作链路》图解 PNG。

运行：
    python tools/build_diagrams.py

产出：
    docs/diagrams/01-项目全景.png ... 09-存储分区.png
"""

import os

from PIL import Image, ImageDraw, ImageFont

SCALE = 2

BG = (246, 248, 251)
CARD = (255, 255, 255)
INK = (27, 36, 48)
MUTED = (92, 107, 122)
LINE = (201, 212, 223)
DARK = (16, 26, 38)

TEAL = (15, 118, 110)
TEAL_BG = (228, 242, 239)
TEAL_LN = (138, 196, 184)
ORANGE = (180, 74, 30)
ORANGE_BG = (252, 238, 230)
ORANGE_LN = (226, 174, 150)
BLUE = (30, 63, 143)
BLUE_BG = (232, 238, 251)
BLUE_LN = (157, 176, 223)
PURPLE = (76, 58, 135)
PURPLE_BG = (240, 236, 250)
PURPLE_LN = (184, 172, 224)
GRAY_BG = (238, 242, 246)
GRAY_LN = (194, 205, 216)
RED = (155, 34, 38)
RED_BG = (251, 234, 234)
RED_LN = (220, 169, 169)

FONT_DIR = r"C:\Windows\Fonts"
F_REG = os.path.join(FONT_DIR, "msyh.ttc")
F_BOLD = os.path.join(FONT_DIR, "msyhbd.ttc")
F_MONO = os.path.join(FONT_DIR, "consola.ttf")
F_MONO_B = os.path.join(FONT_DIR, "consolab.ttf")

_cache = {}


def font(size, bold=False, mono=False):
    key = (size, bold, mono)
    if key in _cache:
        return _cache[key]
    if mono:
        path = F_MONO_B if bold else F_MONO
    else:
        path = F_BOLD if bold else F_REG
    f = ImageFont.truetype(path, int(size * SCALE))
    _cache[key] = f
    return f


def is_cjk(ch):
    return ord(ch) > 0x2E7F


def ascii_only(s):
    """Consolas 没有中文字形，含中文时自动回落到雅黑"""
    return all(ord(ch) < 128 for ch in s)


def wrap(text, fnt, max_w):
    """按字符/单词混合换行，返回行列表。max_w 为逻辑像素。"""
    limit = max_w * SCALE
    lines = []
    for para in text.split("\n"):
        if not para:
            lines.append("")
            continue
        cur = ""
        word = ""
        for ch in para:
            if ch == " " or is_cjk(ch):
                # 先处理待定单词
                if word:
                    if fnt.getlength(cur + word) <= limit:
                        cur += word
                    else:
                        if cur:
                            lines.append(cur)
                        cur = word
                    word = ""
                trial = cur + ch
                if fnt.getlength(trial) <= limit:
                    cur = trial
                else:
                    if cur:
                        lines.append(cur)
                    cur = "" if ch == " " else ch
            else:
                word += ch
                if fnt.getlength(cur + word) > limit and cur:
                    lines.append(cur)
                    cur = ""
                    word = ""
        if word:
            if fnt.getlength(cur + word) <= limit:
                cur += word
            else:
                lines.append(cur)
                cur = word
        if cur:
            lines.append(cur)
    return lines


class Canvas:
    def __init__(self, w, h, bg=BG):
        self.w, self.h = w, h
        self.img = Image.new("RGB", (w * SCALE, h * SCALE), bg)
        self.d = ImageDraw.Draw(self.img)

    # ---------------- 基础图形
    def box(self, x, y, w, h, fill=CARD, line=LINE, lw=1.2, r=9):
        self.d.rounded_rectangle(
            [x * SCALE, y * SCALE, (x + w) * SCALE, (y + h) * SCALE],
            radius=r * SCALE, fill=fill, outline=line, width=max(1, int(lw * SCALE)))

    def band(self, x, y, w, h, fill, line=None):
        self.box(x, y, w, h, fill=fill, line=line, lw=1.2, r=8)

    def line(self, x1, y1, x2, y2, color=LINE, width=1.4, dash=None):
        if dash:
            self._dashed(x1, y1, x2, y2, color, width, dash)
            return
        self.d.line([x1 * SCALE, y1 * SCALE, x2 * SCALE, y2 * SCALE],
                    fill=color, width=max(1, int(width * SCALE)))

    def _dashed(self, x1, y1, x2, y2, color, width, dash):
        import math
        dx, dy = x2 - x1, y2 - y1
        dist = math.hypot(dx, dy)
        if dist == 0:
            return
        ux, uy = dx / dist, dy / dist
        on, off = dash
        t = 0.0
        while t < dist:
            t2 = min(t + on, dist)
            self.d.line([(x1 + ux * t) * SCALE, (y1 + uy * t) * SCALE,
                         (x1 + ux * t2) * SCALE, (y1 + uy * t2) * SCALE],
                        fill=color, width=max(1, int(width * SCALE)))
            t = t2 + off

    def arrow(self, x1, y1, x2, y2, color=TEAL, width=1.6, dash=None, head=9):
        import math
        self.line(x1, y1, x2, y2, color=color, width=width, dash=dash)
        ang = math.atan2(y2 - y1, x2 - x1)
        hx, hy = x2, y2
        p1 = (hx - head * math.cos(ang - 0.45), hy - head * math.sin(ang - 0.45))
        p2 = (hx - head * math.cos(ang + 0.45), hy - head * math.sin(ang + 0.45))
        self.d.polygon([(hx * SCALE, hy * SCALE), (p1[0] * SCALE, p1[1] * SCALE),
                        (p2[0] * SCALE, p2[1] * SCALE)], fill=color)

    def dot(self, x, y, r, fill, line=None):
        self.d.ellipse([(x - r) * SCALE, (y - r) * SCALE, (x + r) * SCALE, (y + r) * SCALE],
                       fill=fill, outline=line, width=max(1, int(1.2 * SCALE)))

    # ---------------- 文本
    def text(self, x, y, s, size=13, color=INK, bold=False, mono=False, anchor="lt"):
        if mono and not ascii_only(s):
            mono = False
        f = font(size, bold, mono)
        self.d.text((x * SCALE, y * SCALE), s, font=f, fill=color, anchor=anchor)

    def text_c(self, cx, y, s, size=13, color=INK, bold=False, mono=False):
        if mono and not ascii_only(s):
            mono = False
        f = font(size, bold, mono)
        self.d.text((cx * SCALE, y * SCALE), s, font=f, fill=color, anchor="mt")

    def para(self, x, y, w, s, size=12.5, color=INK, bold=False, leading=1.45,
             mono=False, align="left"):
        if mono and not ascii_only(s):
            mono = False
        f = font(size, bold, mono)
        lines = wrap(s, f, w)
        lh = size * leading
        for i, ln in enumerate(lines):
            if align == "center":
                self.text_c(x + w / 2, y + i * lh, ln, size, color, bold, mono)
            else:
                self.text(x, y + i * lh, ln, size, color, bold, mono)
        return len(lines) * lh

    def bullets(self, x, y, w, items, size=11.5, color=MUTED, leading=1.5, gap=4,
                dot_color=TEAL, dot="·"):
        cy = y
        for it in items:
            f = font(size)
            lines = wrap(it, f, w - 14)
            self.text(x, cy, dot, size + 2, dot_color, True)
            for j, ln in enumerate(lines):
                self.text(x + 14, cy, ln, size, color)
                cy += size * leading
            cy += gap
        return cy - y

    # ---------------- 复合件
    def chip(self, x, y, label, size=11.5, fill=CARD, line=LINE, color=INK,
             h=30, pad=12, bold=False, mono=False):
        if mono and not ascii_only(label):
            mono = False
        f = font(size, bold, mono)
        w = f.getlength(label) / SCALE + pad * 2
        self.box(x, y, w, h, fill=fill, line=line, lw=1.0, r=h / 2)
        self.d.text(((x + w / 2) * SCALE, (y + h / 2) * SCALE), label, font=f,
                    fill=color, anchor="mm")
        return w

    def title(self, kicker, main, subtitle=None):
        self.text(40, 26, kicker, 11.5, TEAL, True, mono=True)
        self.text(40, 48, main, 27, INK, True)
        self.box(40, 90, 74, 3.5, fill=ORANGE, line=ORANGE, r=2)
        if subtitle:
            self.text(40, 102, subtitle, 12.5, MUTED)
            return 134
        return 116

    def save(self, path):
        os.makedirs(os.path.dirname(path), exist_ok=True)
        self.img.save(path, optimize=True)
        print("saved", path, self.img.size)


OUT = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "图解")


# ============================================================ 01 项目全景

def d01_overview():
    c = Canvas(1200, 700)
    c.title("01 / ARCHITECTURE", "项目全景：五层结构，一层只干一类事",
            "从“用户看到什么”到“硬件怎么动”，每层只依赖它下面一层")

    bands = [
        ("① 应用层", BLUE, BLUE_BG, BLUE_LN,
         ["阅读", "记事", "相册", "画板", "音乐", "天气", "时钟", "日历", "木鱼", "文件", "设置", "更新"]),
        ("② UI 框架层", TEAL, TEAL_BG, TEAL_LN,
         ["Router 路由", "Composer 合成", "Presenter 上屏", "Shell + Overlays",
          "Canvas / Widgets", "IME 输入法", "InputRouter", "UiBus / UiLoop"]),
        ("③ 平台服务层", ORANGE, ORANGE_BG, ORANGE_LN,
         ["数据 持久化 / 状态", "启动 / 管家任务", "HTTP · NTP · WS", "Wi-Fi · BLE",
          "Task · Event · Power", "存储挂载", "音频播放"]),
        ("④ 驱动适配层", PURPLE, PURPLE_BG, PURPLE_LN,
         ["SSD1680 屏幕", "FT6336U 触摸", "ES8311 Codec", "QMI8658A", "ADC / 电池", "I2C / SPI",
          "LittleFS / FAT"]),
        ("⑤ 内核与硬件", (51, 65, 80), GRAY_BG, GRAY_LN,
         ["FreeRTOS", "ESP-IDF v5.4", "ESP32-S3", "墨水屏 · 触摸 · Codec · TF · 电池"]),
    ]

    y = 146
    bh = 88
    for name, color, bg, ln, items in bands:
        c.box(40, y, 1120, bh, fill=bg, line=ln)
        c.box(40, y, 152, bh, fill=color, line=color)
        c.text_c(116, y + 30, name, 13, CARD, True)
        cx = 212
        for it in items:
            w = c.chip(cx, y + (bh - 30) / 2, it, size=11, fill=CARD, line=ln, color=color, pad=10)
            if cx + w > 1136:
                break
            cx += w + 9
        y += bh + 12

    c.box(40, 640, 1120, 44, fill=CARD, line=LINE, r=22)
    c.text(62, 653, "反向通路只有一条：后台结果（网络 / 解码 / 存储）post 事件到 UiBus，"
                    "再由 UI 线程统一改状态并重绘。", 11.5, MUTED)
    c.save(os.path.join(OUT, "01-项目全景.png"))


# ============================================================ 02 硬件

def d02_hardware():
    c = Canvas(1200, 700)
    c.title("02 / HARDWARE", "硬件连接：谁挂在哪个总线上",
            "7 个外设 + 1 个主控；面试问“用了什么”，顺着总线讲下去")

    mcu_x, mcu_y, mcu_w, mcu_h = 450, 250, 300, 120
    c.box(mcu_x, mcu_y, mcu_w, mcu_h, fill=TEAL_BG, line=TEAL, lw=2.0, r=12)
    c.text_c(mcu_x + mcu_w / 2, mcu_y + 18, "ESP32-S3", 20, TEAL, True)
    c.text_c(mcu_x + mcu_w / 2, mcu_y + 52, "240 MHz · 8MB PSRAM · 16MB Flash", 11, MUTED)
    c.text_c(mcu_x + mcu_w / 2, mcu_y + 76, "FreeRTOS + ESP-IDF v5.4", 11, MUTED, mono=True)

    left = [("FT6336U 电容触摸", "I2C + INT 中断 · 手势识别", "I2C"),
            ("三段拨码开关", "ADC · 上 / 中 / 下 → 语义动作", "ADC"),
            ("QMI8658A 六轴", "I2C · 加速度 + 陀螺仪", "I2C")]
    right = [("GDEY027T91 墨水屏", "SPI · 176×264 · 1-bit · SSD1680", "SPI"),
             ("前光背光", "PWM 调光（不影响画面灰度）", "PWM"),
             ("ES8311 Codec + 功放", "I2S · MP3 / WAV 解码播放", "I2S")]
    bottom = [("16MB Flash 分区", "boot / OTA×2 / assets / userdata"),
              ("TF 卡", "SDMMC 4-bit · FAT32 → /sd"),
              ("电池 + USB", "ADC 采电量 · MSC 当 U 盘")]

    ys = [160, 290, 420]
    for (t, d, bus), yy in zip(left, ys):
        c.box(60, yy, 290, 84, fill=CARD, line=LINE, r=10)
        c.text(78, yy + 16, t, 13, INK, True)
        c.text(78, yy + 44, d, 10.5, MUTED)
        c.arrow(356, yy + 42, mcu_x - 4, yy + 42, color=TEAL_LN, width=1.4)
        c.text(368, yy + 16, bus, 9.5, TEAL, True, mono=True)

    for (t, d, bus), yy in zip(right, ys):
        c.box(850, yy, 290, 84, fill=CARD, line=LINE, r=10)
        c.text(868, yy + 16, t, 13, INK, True)
        c.text(868, yy + 44, d, 10.5, MUTED)
        c.arrow(mcu_x + mcu_w + 4, yy + 42, 844, yy + 42, color=TEAL_LN, width=1.4)
        c.text(832, yy + 16, bus, 9.5, TEAL, True, mono=True, anchor="rt")

    bx = [60, 430, 800]
    for (t, d), xx in zip(bottom, bx):
        c.box(xx, 536, 330, 84, fill=GRAY_BG, line=GRAY_LN, r=10)
        c.text(xx + 18, 552, t, 12.5, INK, True)
        c.text(xx + 18, 580, d, 10.5, MUTED)
    c.arrow(600, 532, 600, mcu_y + mcu_h + 6, color=ORANGE_LN, width=1.6)
    c.text(612, 512, "STORAGE / POWER", 9.5, MUTED, mono=True)

    c.box(40, 640, 1120, 44, fill=ORANGE_BG, line=ORANGE_LN, r=22)
    c.text(62, 653, "为什么这样选：墨水屏要连续写几 KB 显存 → SPI；触摸数据量小 → I2C 省引脚；"
                    "三段开关 → 一个 ADC 脚；电池必须电阻分压后再采样。", 11.2, MUTED)
    c.save(os.path.join(OUT, "02-硬件连接.png"))


# ============================================================ 03 启动链路

def d03_boot():
    c = Canvas(1200, 706)
    c.title("03 / BOOT", "启动链路：9 步有序初始化",
            "boot() 是一张步骤表，每步带 required 标记：关键步骤失败才终止，其余降级继续")

    steps = [
        ("app_main()", "main.cc 唯一职责：把控制权交给 ebook_app_run()", "entry"),
        ("ebook_app_run()", "调用 platform::boot()，随后 main 任务进入 delay 空闲循环", "entry"),
        ("1 · system", "NVS · EventMgr · HttpMgr · NtpMgr · OTA · 存储默认挂载", "opt"),
        ("2 · persist", "打开 NVS 命名空间，供后续 Store 读写", "opt"),
        ("3 · display", "DisplayPort + 字体 + 输入法候选表（失败即终止）", "req"),
        ("4 · touch", "I2C 总线 + FT6336U + InputRouter 手势状态机（失败即终止）", "req"),
        ("5 · audio", "ES8311 + MusicPlayer + 木鱼音效（失败仅降级为无声）", "opt"),
        ("6 · data", "SystemState / 锁屏 / 省电 / 键位 / 阅读进度 / 闹钟", "opt"),
        ("7–8 · battery / dip", "电池采样器 + 拨码 ADC 驱动", "opt"),
        ("9 · tasks", "Presenter → UiBus → 注册 12 App → UiLoop → 输入任务 → 管家", "req"),
    ]
    y = 150
    for i, (name, desc, kind) in enumerate(steps):
        fill, ln, dotc = CARD, LINE, GRAY_LN
        txtc = INK
        if kind == "req":
            fill, ln, dotc, txtc = TEAL_BG, TEAL_LN, TEAL, TEAL
        elif kind == "entry":
            fill, ln, dotc, txtc = BLUE_BG, BLUE_LN, BLUE, BLUE
        c.box(40, y, 800, 44, fill=fill, line=ln if kind != "opt" else (240, 244, 248))
        c.dot(60, y + 22, 6, dotc)
        c.text(80, y + 12, name, 11.5, txtc, True, mono=True)
        c.text(280, y + 13, desc, 10.8, MUTED)
        if kind == "req":
            c.text(806, y + 12, "★", 12, ORANGE, True, anchor="rt")
        y += 50

    # 右侧说明
    notes = [
        ("★ 两种失败策略", TEAL, TEAL_BG, [
            ("display / touch / tasks 失败 → 直接返回错误", INK),
            ("音频、拨码、NTP 失败只打 WARN，继续启动", MUTED)]),
        ("启动完成后发生什么", BLUE, BLUE_BG, [
            ("UiLoop 一启动就 replace_shell(Lock)", MUTED),
            ("提交第一帧 → 锁屏界面亮起", MUTED)]),
        ("顺序为什么不能乱", ORANGE, ORANGE_BG, [
            ("显示先于 UI 合成；触摸先于 InputRouter", MUTED),
            ("存储挂载先于读取阅读进度", MUTED)]),
    ]
    ny = 150
    for title_, color, bg, lines in notes:
        c.box(870, ny, 290, 118, fill=bg, line=LINE, r=10)
        c.box(870, ny, 6, 118, fill=color, line=color, r=3)
        c.text(892, ny + 14, title_, 12, color, True)
        yy = ny + 44
        for ln_text, col in lines:
            yy += c.para(892, yy, 248, ln_text, 10.5, col, leading=1.35) + 4
        ny += 132

    c.text(40, 668, "启动完成后 main 任务只剩每 5 秒一次 delay，业务都在各自任务里跑 —— "
                    "没有“一个大 while 循环做完所有事”的写法。", 11, MUTED)
    c.save(os.path.join(OUT, "03-启动链路.png"))


# ============================================================ 04 上屏管线

def d04_pipeline():
    c = Canvas(1200, 690)
    c.title("04 / RENDER PIPELINE", "一帧画面怎么上屏：四个角色，职责不重叠",
            "UI 线程只负责合成，Presenter 只负责搬运和刷屏 —— 这是整个项目最值得讲的设计")

    stages = [
        (40, "① UI 任务  ebook_ui", "24KB 栈 · NORMAL 优先级", TEAL, TEAL_BG, TEAL_LN, [
            "从 UiBus 取出事件（输入 / Tick / 网络状态）",
            "分发给当前页面：on_input / on_ui_event",
            "页面里调用 navigate() 或 request_repaint()",
            "Router 查刷新边表 → 得到 RefreshIntent",
            "Composer.paint：清屏 → Shell → App → Overlay",
        ]),
        (425, "② Presenter 任务  ebook_present", "6KB 栈 · HIGH 优先级", ORANGE, ORANGE_BG, ORANGE_LN, [
            "拿 FrameRequest 队列（深度 4）",
            "丢弃旧请求，只执行最新一帧",
            "加 fb 互斥锁 → copy_fb() 到驱动帧",
            "make_plan()：波形 → 刷新步骤",
            "调 DisplayPort，完成后置 idle 位",
        ]),
        (810, "③ DisplayPort + SSD1680", "bsp 驱动层", PURPLE, PURPLE_BG, PURPLE_LN, [
            "bootstrap：上电时序 + 写两次 RAM",
            "partial：窗口刷（8 像素对齐）",
            "fast / full：整屏快刷 / 全刷",
            "发命令 → 写显存 → 等 BUSY 结束",
            "失败时 invalidate_session() 自愈",
        ]),
    ]
    for x, head, sub, color, bg, ln, items in stages:
        c.box(x, 150, 350, 262, fill=bg, line=ln)
        c.box(x, 150, 350, 62, fill=color, line=color)
        c.text(x + 18, 162, head, 12.5, CARD, True)
        c.text(x + 18, 186, sub, 9.5, CARD)
        c.bullets(x + 20, 228, 312, items, size=11, color=INK, leading=1.6, gap=6, dot_color=color)

    c.arrow(392, 286, 421, 286, color=MUTED, width=2.2, head=10)
    c.arrow(777, 286, 806, 286, color=MUTED, width=2.2, head=10)
    c.text(392, 262, "请求", 9.5, MUTED)
    c.text(778, 262, "步骤", 9.5, MUTED)

    keys = [
        ("双 framebuffer", "back_fb（PSRAM 合成目标）与驱动帧分开，拷贝在锁内完成",
         "防止“画一半被刷上去”的撕裂", TEAL),
        ("队列合并只刷最新", "深度 4；取出一帧后把队列里剩余全部丢弃",
         "墨水屏慢，追最新状态而不是补旧帧", TEAL),
        ("idle 事件组", "submit() 先等 kBitIdle，刷完再置位",
         "高频重绘不堆积、不互相踩", TEAL),
        ("失败自愈", "局刷失败 → 作废会话 → bootstrap + partial 重来",
         "偶发 BUSY 超时也能恢复", TEAL),
    ]
    x = 40
    for title_, desc, why, color in keys:
        c.box(x, 444, 272, 150, fill=CARD, line=LINE, r=10)
        c.box(x, 444, 272, 5, fill=color, line=color, r=2)
        c.text(x + 18, 460, title_, 12, color, True)
        c.para(x + 18, 486, 236, desc, 10.5, INK, leading=1.45)
        c.text(x + 18, 560, "→", 10.5, ORANGE, True)
        c.para(x + 34, 561, 220, why, 10, MUTED, leading=1.35)
        x += 288

    c.box(40, 620, 1120, 48, fill=ORANGE_BG, line=ORANGE_LN, r=24)
    c.text(62, 634, "讲解重点：“合成”发生在 UI 线程，“拷贝 + 等待 BUSY”发生在 Presenter 线程 —— "
                    "把慢操作和 UI 解耦，界面才不会僵住。", 11.5, MUTED)
    c.save(os.path.join(OUT, "04-上屏管线.png"))


# ============================================================ 05 点击时序

def d05_sequence():
    c = Canvas(1200, 750)
    c.title("05 / INPUT TO SCREEN", "一次点击的完整时序：从手指到屏幕",
            "一共 8 跳，中间任何一跳都不允许阻塞 UI")

    lifes = [
        ("手指 / 触摸 IC", "FT6336U 中断", 40, 150),
        ("InputRouter 任务", "读坐标 → 手势", 210, 150),
        ("UiBus 队列（32）", "跨任务投递", 380, 150),
        ("UiLoop 线程", "分发 + 改状态", 550, 150),
        ("App / Router", "业务 + 刷新边表", 720, 150),
        ("Presenter 任务", "搬帧 + 刷屏", 890, 150),
        ("墨水屏", "SSD1680", 1060, 100),
    ]
    top = 150
    centers = []
    for title_, sub, x, w in lifes:
        c.box(x, top, w, 58, fill=CARD, line=TEAL_LN, r=14)
        c.text_c(x + w / 2, top + 10, title_, 11, TEAL, True)
        c.text_c(x + w / 2, top + 33, sub, 9, MUTED)
        cx = x + w / 2
        centers.append(cx)
        c.line(cx, top + 60, cx, 690, color=(214, 223, 232), width=1.2, dash=(6, 6))

    msgs = [
        (0, 1, "按下 / 抬起 → 拉 INT", 232),
        (1, 2, "post_input(ev)", 296),
        (2, 3, "recv() 唤醒 UI 线程", 360),
        (3, 4, "页面 on_input(ev)", 424),
        (3, 4, None, 0),
    ]
    msgs = [
        (0, 1, "按下 / 抬起 → 拉 INT", 232),
        (1, 2, "post_input(ev)", 296),
        (2, 3, "recv() 唤醒 UI 线程", 360),
        (3, 4, "页面 on_input(ev)", 424),
        (4, 5, "submit(FrameRequest)", 500),
        (5, 6, "copy_fb → 刷屏", 588),
    ]
    for a, b, label, y in msgs:
        x1, x2 = centers[a], centers[b]
        c.arrow(x1, y, x2, y, color=TEAL, width=1.8)
        f = font(10.5)
        mid = (x1 + x2) / 2
        c.d.rectangle([(mid - f.getlength(label) / SCALE / 2 - 6) * SCALE, (y - 20) * SCALE,
                       (mid + f.getlength(label) / SCALE / 2 + 6) * SCALE, (y - 2) * SCALE],
                      fill=BG)
        c.text_c(mid, y - 20, label, 10.5, INK)

    # 自调用：合成
    c.box(centers[4] - 3, 452, 22, 34, fill=TEAL_BG, line=TEAL_LN, r=6)
    c.text(centers[4] + 30, 450, "Composer.paint() 合成 back_fb", 10.5, INK)

    # 自调用：队列合并
    c.box(centers[5] - 3, 526, 22, 34, fill=ORANGE_BG, line=ORANGE_LN, r=6)
    c.text(centers[5] + 30, 524, "队列合并：只留最新一帧", 10.5, INK)

    # 回执
    c.arrow(centers[6], 648, centers[5], 648, color=MUTED, width=1.4, dash=(5, 4))
    c.text_c((centers[5] + centers[6]) / 2, 622, "BUSY 结束 → idle 置位", 10.5, MUTED)

    c.box(40, 700, 1120, 44, fill=CARD, line=LINE, r=22)
    c.text(62, 713, "纪律：触摸任务只生产事件；UI 状态只在 UiLoop 里改；合成在 UI 线程；"
                    "拷贝与 BUSY 等待在 Presenter 线程。", 11, MUTED)
    c.save(os.path.join(OUT, "05-点击时序.png"))


# ============================================================ 06 刷新策略

def d06_refresh():
    c = Canvas(1200, 690)
    c.title("06 / E-INK REFRESH", "墨水屏刷新：把“页面变化”翻译成“波形”",
            "全刷慢但干净、局刷快但留残影 —— 框架用一张边表统一决策，而不是散落在各页面里")

    waves = [
        ("Full 全刷", "整屏、会闪黑、最慢（BUSY 超时 15s）",
         "彻底清残影：进入阅读、退出 App、长按菜单", RED, RED_BG, RED_LN),
        ("Fast 快刷", "整屏、速度快、对比度略低（超时 6s）",
         "连续翻页等“要快、可接受轻微残影”的场景", ORANGE, ORANGE_BG, ORANGE_LN),
        ("Partial 局刷", "窗口刷、最快（超时 3s）、残影最明显",
         "输入反馈、光标、选中态等小范围变化", TEAL, TEAL_BG, TEAL_LN),
    ]
    y = 150
    for name, desc, use, color, bg, ln in waves:
        c.box(40, y, 560, 118, fill=bg, line=ln)
        c.box(40, y, 7, 118, fill=color, line=color, r=3)
        c.text(66, y + 14, name, 14, color, True)
        c.text(66, y + 44, desc, 11, INK)
        c.text(66, y + 72, "用于：" + use, 10.5, MUTED)
        y += 130

    # 边表
    c.box(630, 150, 530, 226, fill=CARD, line=LINE, r=10)
    c.text(652, 164, "刷新边表：router/refresh_edges.cc", 12, TEAL, True)
    c.text(652, 192, "kEdges[] = { {from, to, action, intent}, ... }", 10.5, INK, True, mono=True)
    c.para(652, 216, 486, "命中 → 用该边的波形；未命中 → fallback Partial 并打 WARN 日志", 10.5, MUTED)
    c.para(652, 242, 486, "调用方也可传 override_intent 临时覆盖（例如翻页强制 Fast）", 10.5, MUTED)
    c.text(652, 274, "例：Home → AppGrid（Forward）  →  Partial", 10.5, TEAL, True)
    c.text(652, 300, "例：任意 App → Home（Back）    →  Partial", 10.5, TEAL, True)
    c.text(652, 332, "好处：新增页面只补一行，不用在 12 个 App 里各写判断", 10.5, ORANGE)

    # 残影 + 现状
    c.box(630, 390, 530, 96, fill=CARD, line=LINE, r=10)
    c.box(630, 390, 6, 96, fill=ORANGE_LN, line=ORANGE_LN, r=3)
    c.text(650, 402, "残影治理", 12, ORANGE, True)
    c.para(650, 428, 490, "阅读 App 统计翻页次数，连续 N 次局刷后插一次全刷；"
                          "进入 / 退出 App、长菜单等“大变化”直接走全刷。", 10.5, MUTED)

    c.box(630, 500, 530, 110, fill=CARD, line=LINE, r=10)
    c.box(630, 500, 6, 110, fill=PURPLE, line=PURPLE, r=3)
    c.text(650, 512, "诚实地说现状与演进", 12, PURPLE, True)
    c.para(650, 538, 490, "现状：Composer 整屏合成，没有脏区裁剪；局刷 = 整屏提交 + Partial 波形。", 10.5, INK)
    c.para(650, 566, 490, "演进：驱动已支持 8 像素对齐窗口，下一步让 Composer 输出脏矩形。", 10.5, MUTED)

    c.text(40, 546, "补充：SSD1680 的 X 地址按字节寻址，1 字节 = 8 像素，", 10.5, MUTED)
    c.text(40, 570, "所以局刷窗口必须 8 像素对齐。", 10.5, MUTED)
    c.save(os.path.join(OUT, "06-刷新策略.png"))


# ============================================================ 07 任务并发

def d07_tasks():
    c = Canvas(1200, 700)
    c.title("07 / CONCURRENCY", "任务与并发：谁在跑，怎么通信",
            "记住一条纪律：只有 UI 线程能改 UI，其他任务只发事件")

    tasks = [
        ("ebook_ui", "24KB · NORMAL", "唯一能改 UI 状态的线程：取事件、分发、合成", TEAL),
        ("ebook_present", "6KB · HIGH", "搬帧 + 驱动刷新，等待 BUSY（慢操作隔离在这里）", ORANGE),
        ("input（触摸）", "4KB", "等中断 → 读坐标 → 手势状态机 → 投 UiBus", BLUE),
        ("dip（拨码）", "25ms 轮询", "读 ADC，三次一致才判定档位变化", BLUE),
        ("housekeeper", "1 秒周期", "时钟 / 电池 / 自动锁屏 / 闹钟 / SD 热插拔", BLUE),
        ("music（播放）", "PSRAM 大栈", "读文件 → esp_audio_simple_dec → I2S 输出", PURPLE),
        ("协议栈任务", "系统默认", "Wi-Fi / BLE / HTTP / WebSocket，结果 post 回 UI", PURPLE),
        ("main", "默认", "启动完成后每 5 秒 delay，不再承担业务", MUTED),
    ]
    x, y = 40, 150
    for i, (name, spec, desc, color) in enumerate(tasks):
        col = i % 2
        row = i // 2
        xx = x + col * 350
        yy = y + row * 96
        c.box(xx, yy, 330, 82, fill=CARD, line=LINE, r=10)
        c.box(xx, yy, 5, 82, fill=color, line=color, r=2)
        c.text(xx + 18, yy + 12, name, 12, INK, True, mono=True)
        c.text(xx + 200, yy + 13, spec, 9.5, color, True, mono=True)
        c.para(xx + 18, yy + 38, 296, desc, 10.5, MUTED, leading=1.35)

    # 右侧通信设施
    c.box(760, 150, 400, 140, fill=TEAL_BG, line=TEAL_LN, r=10)
    c.text(780, 164, "UiBus 队列（深度 32）", 12, TEAL, True)
    c.para(780, 192, 360, "多生产者 → 单消费者；事件按值入队，结构体 ≤32 字节；"
                          "大块数据用指针 / ID 传递。", 10.5, INK)
    c.box(760, 302, 400, 140, fill=ORANGE_BG, line=ORANGE_LN, r=10)
    c.text(780, 316, "Presenter 队列（深度 4）", 12, ORANGE, True)
    c.para(780, 344, 360, "UI → Presenter 单向；配 fb 互斥锁 + idle 事件组；"
                          "队列满时 submit 返回 Timeout。", 10.5, INK)
    c.box(760, 454, 400, 120, fill=GRAY_BG, line=GRAY_LN, r=10)
    c.text(780, 468, "EventMgr 事件总线", 12, PURPLE, True)
    c.para(780, 496, 360, "系统级事件（电源 / 按键 / 网络）的订阅发布；"
                          "与 UI 无关的模块间通知走这里。", 10.5, INK)

    c.arrow(700, 220, 754, 220, color=TEAL_LN, width=1.5)
    c.arrow(700, 372, 754, 372, color=ORANGE_LN, width=1.5)

    c.box(40, 566, 700, 74, fill=ORANGE_BG, line=ORANGE_LN, r=10)
    c.text(60, 578, "为什么不用一个大循环做完所有事？", 11.5, ORANGE, True)
    c.para(60, 602, 660, "墨水屏刷新、解码、联网都会长时间阻塞，拆成任务后 UI 才不会卡。", 10.5, MUTED)

    c.text(760, 598, "三条纪律", 12, INK, True)
    c.text(760, 624, "① UI 状态只在 UI 任务里改", 10.5, MUTED)
    c.text(760, 646, "② 阻塞操作放独立任务（解码 / 网络 / 刷屏）", 10.5, MUTED)
    c.text(760, 668, "③ UI 回调里绝不做 while(BUSY) 这类等待", 10.5, MUTED)
    c.save(os.path.join(OUT, "07-任务并发.png"))


# ============================================================ 08 业务链路

def d08_chains():
    c = Canvas(1200, 760)
    c.title("08 / BUSINESS CHAINS", "三条典型业务链路：数据从哪来、到哪去",
            "面试常问“你最熟的功能怎么跑起来的”—— 选一条能讲到底")

    chains = [
        ("阅读 Reader", TEAL, TEAL_BG, TEAL_LN, [
            "扫描 /int|/sd 的 Ebook/txt → 书架",
            "TxtBook 打开文件（GBK / UTF-8 识别）",
            "TxtIndexCache 建字节索引 + 分页缓存",
            "ReaderLayout 按字号算每页行数",
            "Canvas 绘制 → Presenter 上屏",
            "ReadingStore 存进度（NVS，下次续读）",
        ]),
        ("音乐 Music", ORANGE, ORANGE_BG, ORANGE_LN, [
            "扫描 Ebook/music → 曲库",
            "MusicPlayer.play(path)",
            "独立播放任务：流式读文件",
            "esp_audio_simple_dec 解码 MP3 / WAV",
            "PCM → ES8311（I2S）→ 功放",
            "进度经 UiBus 回到 UI 刷新",
        ]),
        ("天气 Weather", BLUE, BLUE_BG, BLUE_LN, [
            "Wi-Fi 未连接 → 引导去设置",
            "HttpMgr 请求高德 Web 服务",
            "cJSON 解析实况 + 预报",
            "post 事件到 UiBus（跨任务）",
            "UI 线程更新 WeatherApp 状态",
            "request_repaint → 上屏",
        ]),
    ]
    x = 40
    for name, color, bg, ln, steps in chains:
        c.box(x, 150, 350, 490, fill=CARD, line=LINE, r=10)
        c.box(x, 150, 350, 46, fill=color, line=color, r=10)
        c.text(x + 20, 160, name, 13.5, CARD, True)
        yy = 214
        for i, st in enumerate(steps):
            c.box(x + 18, yy, 314, 52, fill=bg, line=ln, r=10)
            c.para(x + 30, yy + 15, 290, st, 10.5, INK, leading=1.3)
            if i < len(steps) - 1:
                c.arrow(x + 175, yy + 54, x + 175, yy + 62, color=ln, width=1.4)
            yy += 72
        x += 380

    c.box(40, 668, 1120, 46, fill=TEAL_BG, line=TEAL_LN, r=23)
    c.text(62, 682, "共同点：扫描固定目录 → 后台做慢活 → 结果 post 回 UI → 一次重绘。"
                    "三个 App 只是“哪个任务慢”不同。", 11.5, MUTED)
    c.save(os.path.join(OUT, "08-业务链路.png"))


# ============================================================ 09 存储分区

def d09_storage():
    c = Canvas(1200, 720)
    c.title("09 / STORAGE", "Flash 分区与挂载点：数据放哪、怎么用",
            "16MB Flash 切 8 块；三种挂载点对应三种“信任级别”：只读资源 / 可写数据 / 外置卡")

    segs = [
        ("bootloader\n0x0", 118, GRAY_BG, GRAY_LN),
        ("分区表\n0x8000", 106, GRAY_BG, GRAY_LN),
        ("nvs\n16KB", 108, BLUE_BG, BLUE_LN),
        ("otadata\n8KB", 92, BLUE_BG, BLUE_LN),
        ("ota_0\n≈2.9MB", 176, TEAL_BG, TEAL_LN),
        ("ota_1\n≈2.9MB", 176, TEAL_BG, TEAL_LN),
        ("assets 4MB\nLittleFS 只读", 176, PURPLE_BG, PURPLE_LN),
        ("userdata 6MB\nFAT + 磨损均衡", 168, ORANGE_BG, ORANGE_LN),
    ]
    x = 40
    for label, w, bg, ln in segs:
        c.box(x, 150, w - 4, 86, fill=bg, line=ln, r=8)
        lines = label.split("\n")
        c.text_c(x + (w - 4) / 2, 164, lines[0], 10.5, INK, True)
        c.text_c(x + (w - 4) / 2, 188, lines[1], 9.5, MUTED)
        x += w
    c.text(40, 244, "（示意，非等比；实际以 partitions/partition_16mb.csv 为准）", 9.5, MUTED)

    mounts = [
        ("/int", "userdata 分区", "FAT + wear leveling", "用户数据：书 / 音乐 / 照片 / 笔记 / 画稿", ORANGE),
        ("/sd", "TF 卡", "FAT32 · SDMMC 4-bit", "大容量内容，支持热插拔", TEAL),
        ("/assets", "assets 分区", "LittleFS · 只读", "字体、图标、拼音表（烧录时写入）", PURPLE),
    ]
    y = 286
    for path, part, fs, use, color in mounts:
        c.box(40, y, 1120, 58, fill=CARD, line=LINE, r=8)
        c.text(60, y + 16, path, 13, color, True, mono=True)
        c.text(160, y + 18, part, 11, INK)
        c.text(340, y + 18, fs, 11, INK, mono=True)
        c.text(600, y + 18, use, 11, MUTED)
        y += 66

    # 目录树
    c.box(40, 500, 470, 150, fill=(22, 30, 40), line=(22, 30, 40), r=10)
    tree = [
        ("/int/Ebook/", " /sd/Ebook/"),
        ("├─ txt/       阅读", "      ├─ music/   音乐"),
        ("├─ photos/    相册", "      ├─ notes/   记事"),
        ("└─ drawings/  画板导出", ""),
    ]
    for i, (a, b) in enumerate(tree):
        c.text(60, 514 + i * 26, a, 10.5, TEAL_LN if i == 0 else (215, 224, 233), i == 0, mono=True)
        if b:
            c.text(240, 514 + i * 26, b, 10.5, (215, 224, 233), False, mono=True)

    c.box(530, 500, 630, 150, fill=CARD, line=LINE, r=10)
    c.text(550, 512, "两条传文件的路", 12, TEAL, True)
    c.para(550, 540, 590, "USB MSC：把 /int 暴露成 U 盘（TinyUSB），拔线前先“收回”给应用。", 10.5, MUTED)
    c.para(550, 566, 590, "局域网 file_server：Wi-Fi 热点 + HTTP 上传 / 下载。", 10.5, MUTED)
    c.para(550, 592, 590, "约定：用户内容只扫描固定子目录，绝不整盘检索 —— 快且行为可预期。", 10.5, INK)

    c.text(40, 668, "NVS 里放什么：Wi-Fi 配置、亮度 / 音量、阅读进度、闹钟、键位映射 —— 小、常改、需掉电保存。",
           10.5, MUTED)
    c.save(os.path.join(OUT, "09-存储分区.png"))


# ============================================================ 10 功能地图

def d10_features():
    c = Canvas(1200, 600)
    c.title("10 / FEATURES", "功能地图：12 个应用分别在做什么",
            "11 个可用 + 1 个占位；按“内容 / 工具 / 系统”三类看，比逐个背更好记")

    apps = [
        ("阅读", "Reader", "TXT 书架 + 正文；字号 / 进度 / 三档刷新", "内容", BLUE, BLUE_LN),
        ("记事", "Notepad", "TXT 多文件编辑，扫描 notes 目录", "内容", BLUE, BLUE_LN),
        ("相册", "Gallery", "JPEG 浏览，扫描 photos 目录", "内容", BLUE, BLUE_LN),
        ("画板", "Drawing", "1-bit 绘制，导出 BMP 到 drawings", "内容", BLUE, BLUE_LN),
        ("音乐", "Music", "MP3 / WAV 流式播放 + 进度回显", "内容", BLUE, BLUE_LN),
        ("天气", "Weather", "高德 API：实况 + 预报，31 城切换", "联网", ORANGE, ORANGE_LN),
        ("时钟", "Clock", "大字时钟 + 闹钟提醒", "工具", TEAL, TEAL_LN),
        ("日历", "Calendar", "月视图 + 年月选择", "工具", TEAL, TEAL_LN),
        ("木鱼", "WoodenFish", "敲击计数 + 音效", "工具", TEAL, TEAL_LN),
        ("文件", "Files", "/int /sd /assets 浏览、复制、删除", "系统", ORANGE, ORANGE_LN),
        ("设置", "Settings", "网络 / 显示 / 按键 / 声音 / 存储等 10 页", "系统", ORANGE, ORANGE_LN),
        ("更新", "Update", "占位页：OTA 流程待实现", "占位", MUTED, GRAY_LN),
    ]
    for i, (name, en, desc, tag, color, ln) in enumerate(apps):
        col, row = i % 4, i // 4
        x = 40 + col * 288
        y = 150 + row * 118
        c.box(x, y, 272, 106, fill=CARD, line=LINE, r=10)
        c.box(x, y, 5, 106, fill=color, line=color, r=2)
        c.text(x + 20, y + 14, name, 13, INK, True)
        c.text(x + 20, y + 42, en, 9, color, True, mono=True)
        w_en = font(9, True, True).getlength(en) / SCALE
        c.text(x + 24 + w_en, y + 42, "· " + tag, 9, MUTED)
        c.para(x + 20, y + 68, 234, desc, 10, MUTED, leading=1.4)

    c.box(40, 516, 1120, 46, fill=ORANGE_BG, line=ORANGE_LN, r=23)
    c.text(62, 530, "边界说明：更新（OTA 完整流程）、EPUB 阅读仍未实现 —— "
                    "面试里主动讲清“没做什么”，比被追问出来更好。", 11, MUTED)
    c.save(os.path.join(OUT, "10-功能地图.png"))


# ============================================================ 11 开发顺序（从零到整机）

STAGES = [
    ("阶段 0", "选型与硬件打样", "先把“屏幕能亮、串口能出字”拿到手", [
        ("定规格：屏幕 2.7\" 176×264 1-bit · ESP32-S3 · 16MB Flash · 500mAh",
         "README · docs/datasheets", "写下帧大小 5,808B 与三档刷新"),
        ("画原理图 / PCB：SPI 屏 · I2C 触摸+IMU · I2S Codec · SDMMC · ADC · USB",
         "docs/hardware", "引脚表定稿，屏幕排线不发热"),
        ("焊接上电：先量 3.3V / 1.8V，再烧 blink，看串口 log",
         "—", "能烧录、能打印、USB 能被识别"),
    ]),
    ("阶段 1", "板级最小系统（驱动地基）", "每个外设单独跑通，再谈框架", [
        ("工程骨架：CMakeLists + main.cc + boot() 空壳 + Kconfig + 分区表",
         "main/ · partitions/", "idf.py build 通过，能进了 boot()"),
        ("总线与屏幕：bsp/i2c 扫描地址 → SSD1680 初始化 → 全刷白/黑",
         "bsp/driver/gdey027t91", "test_gdey027t91 全刷正常"),
        ("触摸：FT6336U 读坐标 → 手势状态机 → 串口打印 Tap / Swipe",
         "bsp/driver/ft6336u", "test_gdey027t91_touch 手感正常"),
        ("存储：挂载 /int（FAT+WL）与 /assets（LittleFS），插卡挂 /sd",
         "common/storage", "test_storage 三个挂载点都在"),
    ]),
    ("阶段 2", "显示与图形基础", "有了画布，才谈得上界面", [
        ("framebuffer：1-bit 字节布局 · 双缓冲 · 局刷窗口 8 像素对齐",
         "presenter/ · gdey027t91", "memcpy 后整屏与局部都对得上"),
        ("gfx：Canvas 画点线面 · Font 从 /assets 读字库 · Icon",
         "gfx/", "屏幕能画出中文与图标"),
        ("主题与控件：ui/theme + widgets（列表行、按钮、进度条）",
         "ui/", "一屏列表 + 一行按钮画得整齐"),
    ]),
    ("阶段 3", "UI 框架骨架（核心）", "四个角色就位，页面才动得起来", [
        ("页面模型：PageId（Shell / App / Overlay）+ RouteStack 栈操作",
         "router/", "push / pop / 栈满保护都有日志"),
        ("Router：navigate / back / repaint + refresh_edges 刷新边表",
         "router/", "跳转命中边表，未命中打 WARN"),
        ("Composer：按 Shell → App → Overlay 顺序合成一帧",
         "composer/", "四层叠起来顺序正确"),
        ("Presenter + UiBus/UiLoop：队列合并、双缓冲、idle 同步、单线程改 UI",
         "presenter/ · ui/", "连续点击不卡、屏幕追最新状态"),
    ]),
    ("阶段 4", "第一个应用（纵向打通）", "选最复杂的阅读器，把整条链路走穿", [
        ("App 基类 + AppRegistry：id / title / paint / on_input / on_enter",
         "apps/app.hpp · app_registry", "网格里能开、能退、能重进"),
        ("Reader：扫描目录 → 分页索引 → 排版绘制 → 进度保存",
         "apps/reader/", "翻页正常，掉电后能续读"),
        ("输入闭环：InputRouter 手势 + 拨码语义 + 键盘输入法",
         "input/ · ime/", "手势与拨码都能操作阅读器"),
    ]),
    ("阶段 5", "内容类应用批量复制", "同一套控件，换业务逻辑", [
        ("Files：VFS 浏览 / 复制 / 删除，弹窗与加载态",
         "apps/files/", "能删能改名，热插拔不崩"),
        ("Notepad / Gallery / Drawing：复用 ListView + 编辑模式",
         "apps/notepad · gallery · drawing", "三个 App 各自独立跑通"),
        ("Music：解码任务 + ES8311 + 进度回显（异步结果走 UiBus）",
         "apps/music · platform/music_player", "播放/暂停/切歌都正常"),
    ]),
    ("阶段 6", "联网与系统能力", "从“单机玩具”变成“能用的产品”", [
        ("Wi-Fi / BLE + 设置页：连接、扫描、状态回显",
         "network/ · apps/settings", "断网能提示，重连能恢复"),
        ("NTP 校时 → HTTP → 天气 App → 局域网 file_server",
         "protocol/ · apps/weather", "时间准、天气能刷、网页能传文件"),
        ("系统能力：Housekeeper、省电、自动锁屏、电池采样、USB MSC",
         "platform/ · common/storage", "低电量与 1 分钟无操作行为正确"),
    ]),
    ("阶段 7", "打包与交付", "别人拿到包能自己烧", [
        ("assets 生成（字体 / 图标 / 拼音表）→ LittleFS · OTA 分区与回滚",
         "tools/ · partitions/", "资源分区可读、OTA 双分区可切"),
        ("固件打包 + FLASH.md + 版本清单，整机 12 个 App 回归一遍",
         "scripts/package_firmware.py", "发布包按文档烧录即可点亮"),
    ]),
]

INTEG_LAYERS = [
    ("① 应用层", BLUE, "12 个 App（阶段 4~6 逐个长出来）"),
    ("② UI 框架", TEAL, "Router · Composer · Presenter · UiLoop（阶段 2~3 搭好）"),
    ("③ 平台服务", ORANGE, "数据 / 网络 / 协议 / 存储 / 音频（阶段 1、6）"),
    ("④ 驱动适配", PURPLE, "屏幕 / 触摸 / Codec / IMU / ADC（阶段 1）"),
    ("⑤ 内核硬件", (51, 65, 80), "ESP-IDF · FreeRTOS · ESP32-S3 与板卡（阶段 0）"),
]


def d11_dev_order():
    row_h = 62
    head_h = 46
    stage_gap = 24
    top = 158
    total = top
    for _, _, subt, rows in STAGES:
        total += head_h + 10 + len(rows) * row_h + stage_gap
    total += 330

    c = Canvas(1360, total)
    c.title("11 / BUILD ORDER", "从零到整机：这个项目的开发顺序",
            "每一步都能单独验证，验证通过再往上叠 —— 最后用 boot() 串成一个整体")
    c.text(40, 126, "读法：每行三列 = ① 这一步做什么　② 代码落在哪个目录　③ 怎么算做完（验收）", 10.5, MUTED)

    y = top
    body_x = 40
    body_w = 1280
    for si, (tag, name, subt, rows) in enumerate(STAGES):
        color = [BLUE, TEAL, TEAL, PURPLE, ORANGE, ORANGE, TEAL, BLUE][si % 8]
        c.box(body_x, y, body_w, head_h, fill=color, line=color, r=10)
        c.text(body_x + 18, y + 13, tag, 12, CARD, True, mono=True)
        c.text(body_x + 90, y + 12, name, 13.5, CARD, True)
        c.text(body_x + body_w - 18, y + 15, subt, 10.5, CARD, anchor="rt")
        y += head_h + 10

        for ri, (action, where, done) in enumerate(rows):
            c.box(body_x + 16, y, body_w - 32, row_h - 8, fill=CARD, line=LINE, r=8)
            c.dot(body_x + 40, y + 26, 9, color)
            c.text_c(body_x + 40, y + 21, str(ri + 1), 10, CARD, True)
            c.text(body_x + 60, y + 10, action, 11, INK, True)
            c.text(body_x + 640, y + 11, where, 10, TEAL, mono=True)
            c.text(body_x + 930, y + 11, "→ " + done, 10, MUTED)
            y += row_h

        # 阶段之间的小箭头
        c.arrow(body_x + 40, y + 2, body_x + 40, y + stage_gap - 4, color=color, width=1.6, head=7)
        y += stage_gap

    # 整合：五层落位
    c.box(body_x, y + 6, body_w, 300, fill=(248, 250, 252), line=LINE)
    c.text(body_x + 24, y + 22, "把前面 8 个阶段拼成一个整体", 15, INK, True)
    c.text(body_x + 24, y + 50, "boot() 是唯一的组装点：按顺序调用各阶段产出的初始化，"
                               "任何一层没就绪都会在启动日志里暴露。", 10.5, MUTED)
    ly = y + 80
    for lname, lcolor, ltext in INTEG_LAYERS:
        c.box(body_x + 24, ly, 116, 36, fill=lcolor, line=lcolor, r=8)
        c.text_c(body_x + 82, ly + 9, lname, 10.5, CARD, True)
        c.box(body_x + 148, ly, body_w - 172, 36, fill=CARD, line=LINE, r=8)
        c.text(body_x + 166, ly + 9, ltext, 10.5, INK)
        ly += 42
    c.save(os.path.join(OUT, "11-开发顺序.png"))


def main():
    d01_overview()
    d02_hardware()
    d03_boot()
    d04_pipeline()
    d05_sequence()
    d06_refresh()
    d07_tasks()
    d08_chains()
    d09_storage()
    d10_features()
    d11_dev_order()


if __name__ == "__main__":
    main()
