#!/usr/bin/env python3
"""Add beginner-oriented comments without changing program behavior."""

from __future__ import annotations

import re
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Tuple


ROOT = Path(__file__).resolve().parents[1]
MAIN = ROOT / "main"
DOC = ROOT / "docs" / "代码详解-初学者版.md"
MARKER = "[初学者导读]"

SOURCE_SUFFIXES = {".c", ".cc", ".cpp"}
HEADER_SUFFIXES = {".h", ".hpp"}
SKIP_PARTS = {"third_party", "test"}

CATEGORY_RULES: List[Tuple[str, str, str]] = [
    ("app/ebook/ui", "UI 事件、绘制、列表和页面循环", "UI 状态只能在 UiLoop/UI 线程修改，跨任务结果通过 UiBus 回 UI。"),
    ("app/ebook/router", "页面路由、导航动作和刷新边界", "路由先改页面栈，再向 Presenter 提交刷新请求；提交是异步的。"),
    ("app/ebook/presenter", "刷新计划、合成与任务调度", "Presenter 队列有容量限制，提交失败要调用方处理。"),
    ("app/ebook/input", "触摸、拨码和语义输入分发", "中断和输入任务不能直接修改页面，只能投递结果。"),
    ("app/ebook/display", "显示接口和背光控制", "墨水屏刷新是异步且耗时的，要区分 Full、Fast、Partial。"),
    ("app/ebook/data", "持久化、系统状态和时钟", "小配置写 NVS，运行时共享状态放 SystemState。"),
    ("app/ebook/gfx", "Canvas、字体、文本布局和位图", "绘制只改 framebuffer，不直接操作 SPI。"),
    ("app/ebook/composer", "页面分层合成", "合成顺序决定覆盖层和页面谁先画。"),
    ("app/ebook/platform", "启动、周期任务、电池、音频和平台服务", "阻塞工作放独立任务，周期小检查放 Housekeeper。"),
    ("app/ebook/apps", "业务 App 和具体功能", "App 生命周期由 AppRegistry 管理，阻塞工作不能放 UI 回调。"),
    ("app/ebook/overlays", "全局覆盖层", "覆盖层优先接收输入，打开和关闭会改变路由栈。"),
    ("app/ebook/shell", "锁屏、主页、应用网格和应用宿主", "Shell 是 App 之外的外壳页面，负责统一返回和输入兜底。"),
    ("app/ebook", "电子书应用层", "上层通过 Router、UiBus 和 SystemState 协作，不直接操作硬件。"),
    ("app/network", "Wi-Fi、BLE 和网络连接", "网络回调不在 UI 线程，状态变化通常通过 UiBus 回到页面。"),
    ("app/protocol", "HTTP、NTP、WebSocket 和文件服务", "网络请求可能阻塞，必须在后台任务或专门任务中执行。"),
    ("app/system/task", "FreeRTOS 任务封装", "任务栈、优先级和销毁顺序是主要风险点。"),
    ("app/system/event", "系统事件循环", "事件 base 的身份必须长期稳定，不要指向临时字符串。"),
    ("app/system/power", "轻睡眠和深睡眠", "进入低功耗前要确认外设和内存状态，唤醒后需要重新校准。"),
    ("app/system/info", "内存、CPU 和系统信息", "采样结果反映调用时刻，不是持续实时监控。"),
    ("common/storage", "内部 Flash、assets、SD 卡和 USB MSC", "挂载状态有锁和热插拔风险，慢文件操作应放后台任务。"),
    ("common/ota", "OTA 下载、校验、分区和状态上报", "OTA 回调不在 UI 线程，UI 更新必须通过 UiBus。"),
    ("common/memory", "PSRAM、内存池、双缓冲和环形缓冲", "PSRAM 能缓解内部内存压力，但不能无视 DMA 和中断约束。"),
    ("common/time", "系统时间和时区", "本地时间依赖时区和同步状态，不要把它当成单调计时器。"),
    ("common/file", "文件读写辅助", "文件可能很大或位于 SD 卡，调用者要限制单次内存占用。"),
    ("common/uuid", "UUID 生成和格式处理", "UUID 只表示唯一标识，不承担认证或安全职责。"),
    ("bsp/driver/gdey027t91", "GDEY027T91 墨水屏和 SSD1680 兼容控制", "刷新要等待 BUSY，访问 SPI 前必须考虑并发保护。"),
    ("bsp/driver/ft6336u", "FT6336U 触摸和手势识别", "中断只通知事件，I2C 读取和坐标变换应在安全任务上下文执行。"),
    ("bsp/driver/qmi8658a", "QMI8658A 六轴传感器", "自动旋转是否启用取决于系统配置，不能只凭驱动存在推断行为。"),
    ("bsp/driver/audio", "ES8311 音频 Codec 和 I2S 链路", "音频使用 DMA 和环形缓冲，启动停止顺序会影响杂音和尾音。"),
    ("bsp/driver/button", "DIP 拨码和物理输入", "ADC 采样有抖动和区间误差，需要去抖和边界判断。"),
    ("bsp/battery", "电池 ADC 采样", "电压需要按分压和标定换算，不能直接当百分比。"),
    ("bsp/i2c", "I2C 总线", "总线访问可能失败或超时，调用者不能假设设备一定在线。"),
    ("bsp/config", "板级引脚和硬件参数", "引脚修改必须同时核对原理图、复用、总线和 Kconfig。"),
    ("main", "固件入口和启动", "app_main 只负责把控制权交给项目入口，随后由常驻任务运行。"),
]


def has_skipped_part(path: Path) -> bool:
    return any(part in SKIP_PARTS for part in path.parts)


def iter_first_party_files() -> Iterable[Path]:
    for path in sorted(MAIN.rglob("*")):
        if not path.is_file():
            continue
        if path.suffix.lower() not in SOURCE_SUFFIXES | HEADER_SUFFIXES:
            continue
        if has_skipped_part(path.relative_to(MAIN)):
            continue
        yield path


def category_for(path: Path) -> Tuple[str, str]:
    rel = path.relative_to(ROOT).as_posix()
    for prefix, purpose, constraint in CATEGORY_RULES:
        if prefix in rel:
            return purpose, constraint
    return "项目源码的一部分", "修改前先确认调用者、线程上下文和失败路径。"


def read_text(path: Path) -> str:
    with path.open("r", encoding="utf-8-sig", newline="") as handle:
        return handle.read()


def write_text(path: Path, text: str) -> None:
    with path.open("w", encoding="utf-8", newline="") as handle:
        handle.write(text)


def line_ending(text: str) -> str:
    return "\r\n" if "\r\n" in text else "\n"


def insert_file_banner(path: Path) -> bool:
    text = read_text(path)
    if MARKER in text:
        return False

    rel = path.relative_to(ROOT).as_posix()
    purpose, constraint = category_for(path)
    nl = line_ending(text)
    banner = (
        "/**" + nl
        + f" * {MARKER} 文件：{rel}" + nl
        + f" * 作用：{purpose}。" + nl
        + " * 阅读顺序：先看同名头文件的公开接口，再按本文件函数定义顺序阅读。" + nl
        + f" * 约束：{constraint}" + nl
        + " * 详细解释：docs/代码详解-初学者版.md。" + nl
        + " */" + nl + nl
    )

    if path.suffix.lower() in HEADER_SUFFIXES and text.startswith("#pragma once"):
        prefix = "#pragma once"
        rest = text[len(prefix):]
        text = prefix + nl + nl + banner.rstrip(nl) + nl + rest.lstrip("\r\n")
    else:
        text = banner + text
    write_text(path, text)
    return True


def normalize_heading(text: str) -> str:
    text = text.replace("`", "").replace("**", "").strip()
    text = re.sub(r"\s+", " ", text)
    return text


def extract_functions(heading: str) -> List[str]:
    pattern = re.compile(r"(?:[A-Za-z_]\w*::)*[A-Za-z_]\w*\s*\([^()]*\)")
    result: List[str] = []
    for match in pattern.finditer(heading):
        name = re.sub(r"\s+", "", match.group(0))
        if name not in result:
            result.append(name)
    return result


def heading_description(heading: str, names: List[str]) -> str:
    text = heading
    description = ""
    if "：" in text:
        description = text.split("：", 1)[1]
    description = normalize_heading(description)
    if not description or description == normalize_heading(heading):
        return "实现流程与设计原因见详解对应小节"
    return description


def parse_doc_functions() -> Dict[str, str]:
    text = DOC.read_text(encoding="utf-8")
    result: Dict[str, str] = {}
    for raw_line in text.splitlines():
        if not raw_line.startswith("#"):
            continue
        heading = normalize_heading(raw_line.lstrip("#").strip())
        names = extract_functions(heading)
        if not names:
            continue
        desc = heading_description(heading, names)
        for name in names:
            result.setdefault(name, desc)
    return result


def likely_definition(line: str, name: str) -> bool:
    stripped = line.strip()
    if not stripped or stripped.startswith("//") or stripped.startswith("*"):
        return False
    if stripped.startswith(("return ", "if ", "while ", "for ", "switch ")):
        return False
    if stripped.endswith(";"):
        return False
    if "=" in stripped and name + "(" in stripped:
        before = stripped.split(name + "(", 1)[0]
        if "=" in before:
            return False
    if "::" not in name:
        bare = name.split("(", 1)[0]
        if len(bare) < 6:
            return False
        if re.search(r"[.\-]\s*" + re.escape(bare) + r"\s*\(", line):
            return False
    return True


def find_definition(name: str, source_files: List[Path]) -> Optional[Tuple[Path, int, str]]:
    pattern = re.compile(r"(?<![A-Za-z0-9_:])" + re.escape(name).replace(r"\(", r"\s*\("))
    candidates: List[Tuple[Path, int, str, int]] = []
    for path in source_files:
        for line_no, line in enumerate(read_text(path).splitlines()):
            if pattern.search(line) and likely_definition(line, name):
                priority = 0 if path.suffix.lower() in SOURCE_SUFFIXES else 1
                candidates.append((path, line_no, line, priority))
    if not candidates:
        return None
    candidates.sort(key=lambda item: (item[3], item[0].as_posix(), item[1]))
    if "::" not in name and len(candidates) != 1:
        return None
    path, line_no, line, _ = candidates[0]
    return path, line_no, line


def add_function_comments(doc_functions: Dict[str, str], source_files: List[Path]) -> int:
    insertions: Dict[Path, List[Tuple[int, str, str]]] = {}
    used_targets = set()

    for name, desc in sorted(doc_functions.items(), key=lambda item: len(item[0]), reverse=True):
        found = find_definition(name, source_files)
        if not found:
            continue
        path, line_no, line = found
        target_key = (path, line_no)
        if target_key in used_targets:
            continue
        used_targets.add(target_key)

        nl = line_ending(read_text(path))
        indent = re.match(r"\s*", line).group(0)
        comment = f"{indent}// {MARKER} {name}：{desc}。详见 docs/代码详解-初学者版.md。{nl}"
        insertions.setdefault(path, []).append((line_no, name, comment))

    inserted = 0
    for path, items in insertions.items():
        lines = read_text(path).splitlines(keepends=True)
        for line_no, name, comment in sorted(items, reverse=True):
            previous_lines = lines[max(0, line_no - 4):line_no]
            already_annotated = any(MARKER in previous and name in previous for previous in previous_lines)
            if already_annotated:
                continue
            lines.insert(line_no, comment)
            inserted += 1
        write_text(path, "".join(lines))
    return inserted


def main() -> None:
    files = list(iter_first_party_files())
    banner_count = 0
    for path in files:
        if insert_file_banner(path):
            banner_count += 1

    source_files = [path for path in files if path.suffix.lower() in SOURCE_SUFFIXES]
    doc_functions = parse_doc_functions()
    function_count = add_function_comments(doc_functions, source_files)

    print(f"file_banners={banner_count}")
    print(f"function_comments={function_count}")
    print(f"source_files={len(source_files)}")
    print(f"documented_functions={len(doc_functions)}")


if __name__ == "__main__":
    main()
