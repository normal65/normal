#!/usr/bin/env python3
"""Rewrite generic file-banner purposes with file-specific descriptions."""

from __future__ import annotations

import re
from pathlib import Path
from typing import List, Tuple


ROOT = Path(__file__).resolve().parents[1]
MAIN = ROOT / "main"
SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".h", ".hpp"}
SKIP_PARTS = {"third_party", "test"}

RULES: List[Tuple[str, str]] = [
    ("bsp/driver/gdey027t91/backlight", "控制墨水屏背光/前光亮度，使用 LEDC PWM 调节照明强度。"),
    ("bsp/driver/gdey027t91/ssd1680", "定义 SSD1680 控制器命令、刷新序列和面板初始化参数。"),
    ("bsp/driver/gdey027t91/panel", "定义 GDEY027T91 面板尺寸、RAM 地址范围和 BUSY 超时。"),
    ("bsp/driver/gdey027t91/framebuffer", "操作 1-bit 墨水屏 framebuffer 的像素、矩形填充和位图写入。"),
    ("bsp/driver/gdey027t91/gdey027t91", "实现 GDEY027T91 墨水屏的 SPI 通信、复位、刷新、局刷和休眠。"),
    ("bsp/driver/ft6336u/ft6336u", "实现 FT6336U 触摸控制器初始化、I2C 读点和中断控制。"),
    ("bsp/driver/ft6336u/touch_gesture", "把触摸点转换成 Tap、Swipe、LongPress 等手势事件。"),
    ("bsp/driver/qmi8658a", "实现 QMI8658A 六轴传感器读取、姿态角计算和运动检测。"),
    ("bsp/driver/audio/es8311", "控制 ES8311 音频 Codec 的初始化、采样格式、音量和播放通路。"),
    ("bsp/driver/button", "读取拨码/物理按键的 ADC 电压并产生按键事件。"),
    ("bsp/battery/battery", "读取电池 ADC 电压，并按分压和放电曲线换算电量百分比。"),
    ("bsp/i2c/i2c", "初始化和管理 I2C 总线，为触摸、传感器等设备提供通信。"),
    ("bsp/config/config", "集中定义开发板 GPIO、外设总线和硬件参数。"),
    ("app/ebook/ui/ui_bus", "创建 UI 事件队列，负责普通任务和中断安全的事件投递与接收。"),
    ("app/ebook/ui/ui_loop", "运行 UI 主循环，消费 UiBus 事件并分发给当前页面。"),
    ("app/ebook/ui/list_view", "实现通用列表控件的绘制、滚动、命中测试和行提供者回调。"),
    ("app/ebook/ui/widgets", "提供工具栏、列表行、电池、滚动条、网格等通用绘制控件。"),
    ("app/ebook/ui/dialog", "实现通用对话框、按钮布局和关闭按钮绘制。"),
    ("app/ebook/ui/theme", "定义界面尺寸、字体大小、间距和布局 token。"),
    ("app/ebook/ui/strings", "集中存放界面文字、提示语和按钮标签。"),
    ("app/ebook/ui/layer", "定义可绘制覆盖层的公共接口。"),
    ("app/ebook/router/router", "管理 Shell、App、覆盖层之间的跳转、返回和页面栈变化。"),
    ("app/ebook/router/route_stack", "保存 Shell、App 和 overlay 的页面历史，决定返回目标。"),
    ("app/ebook/router/refresh_edges", "根据页面跳转类型决定使用哪种刷新波形。"),
    ("app/ebook/router/refresh_intent", "定义 Partial、Fast、Full 刷新意图和波形名称。"),
    ("app/ebook/router/transition", "描述一次页面跳转的来源、目标和导航动作。"),
    ("app/ebook/router/page_registry", "维护页面名称和 ID 的映射。"),
    ("app/ebook/presenter/presenter", "调度墨水屏刷新请求、合成 framebuffer 并等待显示任务完成。"),
    ("app/ebook/presenter/present_plan", "把刷新波形转换成墨水屏驱动的具体刷新步骤。"),
    ("app/ebook/presenter/frame_request", "定义一次待刷新请求的页面、动作、波形和序号。"),
    ("app/ebook/composer/composer", "按 Shell、页面、覆盖层顺序合成一帧画面。"),
    ("app/ebook/display/display_port", "把上层刷新意图转换成墨水屏的 Full、Fast、Partial 操作。"),
    ("app/ebook/input/input_router", "从触摸驱动读取原始点，并推进手势状态机。"),
    ("app/ebook/input/input_dispatch", "把物理按键和语义动作分发到 Router、App 或全局功能。"),
    ("app/ebook/input/key_bindings", "保存和查询物理按键到语义动作的映射。"),
    ("app/ebook/input/input_bindings", "提供物理按键、上下文的检测和动作解析。"),
    ("app/ebook/input/physical_input", "运行物理按键任务并产生按键事件。"),
    ("app/ebook/input/physical_types", "定义物理按键、动作和事件的基础类型。"),
    ("app/ebook/input/gesture_tuner", "提供触摸手势参数的调试和调优。"),
    ("app/ebook/data/persist", "封装 NVS 的初始化、读写、提交和命名空间管理。"),
    ("app/ebook/data/system_state", "保存亮度、音量、Wi-Fi、蓝牙、热点等全局运行时状态。"),
    ("app/ebook/data/clock_provider", "提供当前时间和时间字符串。"),
    ("app/ebook/platform/boot", "按依赖顺序初始化 NVS、显示、触摸、音频、数据、电池和任务。"),
    ("app/ebook/platform/housekeeper", "周期更新时钟、电池、自动锁屏和存储热插拔状态。"),
    ("app/ebook/platform/battery_sampler", "周期采样电池电压，并对电量进行平滑和状态广播。"),
    ("app/ebook/platform/audio_codec", "把系统音量、静音和音频格式应用到 ES8311/I2S 通路。"),
    ("app/ebook/platform/music_player", "执行本地音乐播放、解码、写入音频缓冲和控制播放状态。"),
    ("app/ebook/platform/wooden_fish_sound", "加载并播放电子木鱼音效。"),
    ("app/ebook/apps/files/files_app", "实现文件管理界面、目录浏览、打开、删除确认和加载状态。"),
    ("app/ebook/apps/files/vfs_browser", "实现受控目录扫描、排序、路径校验和文件删除。"),
    ("app/ebook/apps/app_registry", "登记所有 App，并负责打开、关闭和查找应用。"),
    ("app/ebook/apps/app", "定义所有 App 的公共生命周期、绘制、输入和刷新接口。"),
    ("app/ebook/apps/stub_app", "实现尚未完成功能的占位 App。"),
    ("app/ebook/apps/reader/reader_app", "实现阅读器页面、分页、阅读进度和阅读菜单入口。"),
    ("app/ebook/apps/reader/book_shelf", "扫描书架目录并管理书籍列表。"),
    ("app/ebook/apps/reader/txt_book", "读取和解析 TXT 文本，处理编码、分页和内容缓存。"),
    ("app/ebook/apps/reader/reading_store", "保存阅读进度、最近阅读和阅读器偏好。"),
    ("app/ebook/apps/reader/txt_index_cache", "建立和缓存 TXT 分页索引。"),
    ("app/ebook/apps/reader/reader_menu", "绘制和处理阅读器菜单。"),
    ("app/ebook/apps/notepad/notepad_app", "实现记事本编辑、保存、列表和键盘输入。"),
    ("app/ebook/apps/gallery/gallery_app", "实现相册列表、图片解码、缩放和查看。"),
    ("app/ebook/apps/drawing/drawing_app", "实现画板绘制、笔刷、撤销和位图保存。"),
    ("app/ebook/apps/wooden_fish/wooden_fish_app", "实现电子木鱼点击、计数和音效播放。"),
    ("app/ebook/apps/music/music_app", "实现音乐列表、播放控制、进度和歌曲信息显示。"),
    ("app/ebook/apps/music/music_library", "扫描音乐目录并维护歌曲元数据。"),
    ("app/ebook/apps/music/music_store", "保存音乐播放列表和播放偏好。"),
    ("app/ebook/apps/weather/weather_app", "获取并显示城市天气，处理城市选择和刷新状态。"),
    ("app/ebook/apps/weather/weather_cities", "保存和查询天气城市码。"),
    ("app/ebook/apps/clock/clock_app", "实现时钟界面、闹钟显示和闹钟编辑。"),
    ("app/ebook/apps/clock/clock_store", "保存和读取闹钟配置。"),
    ("app/ebook/apps/calendar/calendar_app", "实现日历月视图、日期选择和月份切换。"),
    ("app/ebook/apps/settings/settings_app", "实现设置菜单、页面切换和设置页生命周期。"),
    ("app/ebook/apps/settings/settings_common", "提供设置页公共的标题、复制、滑条和列表辅助函数。"),
    ("app/ebook/apps/settings/settings_wifi", "实现 Wi-Fi 设置页的开关、扫描、连接、保存网络和状态显示。"),
    ("app/ebook/apps/settings/settings_bluetooth", "实现蓝牙设置页的开关、名称修改和连接数显示。"),
    ("app/ebook/apps/settings/settings_hotspot", "实现热点设置页的名称、密码、AP 模式和文件服务。"),
    ("app/ebook/apps/settings/settings_display", "实现亮度、夜间模式和自动锁屏设置。"),
    ("app/ebook/apps/settings/settings_key_bindings", "实现拨码按键映射设置页。"),
    ("app/ebook/apps/settings/settings_sound", "实现音量和静音设置页。"),
    ("app/ebook/apps/settings/settings_time", "实现时区、NTP 自动同步和手动同步设置。"),
    ("app/ebook/apps/settings/settings_storage", "显示内部存储、assets 和 SD 卡容量。"),
    ("app/ebook/apps/settings/settings_security", "实现锁屏 PIN 设置、修改、验证和清除流程。"),
    ("app/ebook/apps/settings/settings_about", "显示设备名、MAC、IP、固件版本和内存信息。"),
    ("app/ebook/apps/settings/auto_lock", "根据空闲时间判断何时发出自动锁屏提示。"),
    ("app/ebook/apps/settings/lock_password", "保存和验证锁屏 PIN 的 SHA-256 摘要。"),
    ("app/ebook/apps/settings/power_save", "实现省电开关、背光限制和无线功能关闭/恢复。"),
    ("app/network/wifi/wifi", "初始化 Wi-Fi、扫描 AP、连接网络、保存凭据并广播状态。"),
    ("app/network/bluetooth/bluetooth", "初始化和控制 BLE 广播、连接、设备名和连接数。"),
    ("app/network/bluetooth/gatt/gatt", "实现 BLE GATT 服务和特征值读写、通知。"),
    ("app/protocol/http/http", "实现 HTTP 请求、响应缓冲和流式下载。"),
    ("app/protocol/ntp/ntp", "初始化 NTP、同步网络时间并广播同步状态。"),
    ("app/protocol/websocket/websocket", "实现 WebSocket 连接、发送、接收和事件回调。"),
    ("app/protocol/file_server/file_server", "启动局域网文件服务，让电脑或手机访问设备文件。"),
    ("app/system/task/task", "封装 FreeRTOS 任务创建、栈配置、优先级和销毁。"),
    ("app/system/event/event", "封装 ESP-IDF 事件循环、事件注册和投递。"),
    ("app/system/power/power", "管理轻睡眠、深睡眠和唤醒后的恢复。"),
    ("app/system/info/info", "读取 CPU、内存、PSRAM 和系统信息。"),
    ("common/ota/ota", "实现 OTA 检查更新、下载固件、MD5 校验、切换分区和状态上报。"),
    ("common/storage/storage_usb_msc", "实现 USB MSC 对内部存储的挂载和恢复。"),
    ("common/storage/storage", "管理内部 Flash、assets、SD 卡和 USB 文件系统挂载状态。"),
    ("common/file/file", "提供文件读取、写入和大小查询辅助函数。"),
    ("common/memory/memory", "实现内存池、RAII 内存管理和内存信息统计。"),
    ("common/time/time", "提供系统时间、时间戳和格式化函数。"),
    ("common/uuid/uuid", "生成和处理 UUID。"),
    ("app/ebook/core/geometry", "定义 Point、Size、Rect 等基础几何类型。"),
    ("app/ebook/gfx/canvas", "向 framebuffer 绘制文字、线条、矩形和位图。"),
    ("app/ebook/gfx/font", "管理字体加载、字形缓存、文本测量和绘制。"),
    ("app/ebook/gfx/bitmap_font", "加载和绘制预烘焙点阵字体。"),
    ("app/ebook/gfx/text_layout", "实现文本换行、截断和排版计算。"),
    ("app/ebook/gfx/utf8", "实现 UTF-8 编码判断、解码和遍历。"),
    ("app/ebook/ime/ime_engine", "实现拼音输入法候选查询和候选字管理。"),
    ("app/ebook/text/path_encoding", "处理 UTF-8、GBK 和文件路径显示编码。"),
]


def iter_files():
    for path in sorted(MAIN.rglob("*")):
        if not path.is_file() or path.suffix.lower() not in SOURCE_SUFFIXES:
            continue
        rel_parts = path.relative_to(MAIN).parts
        if any(part in SKIP_PARTS for part in rel_parts):
            continue
        yield path


def description_for(path: Path) -> str:
    rel = path.relative_to(ROOT).as_posix()
    for fragment, description in RULES:
        if fragment in rel:
            return description
    stem = path.stem.replace("_", " ")
    parent = path.parent.name.replace("_", " ")
    return f"实现 {parent} 模块中的 {stem} 功能。"


def refine_file(path: Path) -> bool:
    with path.open("r", encoding="utf-8-sig", newline="") as handle:
        text = handle.read()
    match = re.search(r"(?m)^( \* 作用：).*$", text)
    if not match:
        return False
    replacement = match.group(1) + description_for(path)
    if replacement == match.group(0):
        return False
    text = text[:match.start()] + replacement + text[match.end():]
    with path.open("w", encoding="utf-8", newline="") as handle:
        handle.write(text)
    return True


def main() -> None:
    files = 0
    for path in iter_files():
        if refine_file(path):
            files += 1
    print(f"updated_files={files}")


if __name__ == "__main__":
    main()
