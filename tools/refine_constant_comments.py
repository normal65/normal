#!/usr/bin/env python3
"""Replace generic static/struct comments with name-aware explanations."""

from __future__ import annotations

import re
from pathlib import Path
from typing import Optional


ROOT = Path(__file__).resolve().parents[1]
MAIN = ROOT / "main"
SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".h", ".hpp"}
SKIP_PARTS = {"third_party", "test"}

GENERIC_PREFIXES = (
    "// 静态声明：",
    "// 编译期常量：",
    "// 结构体定义：",
    "// 执行这条代码：}",
)


def iter_files():
    for path in sorted(MAIN.rglob("*")):
        if not path.is_file() or path.suffix.lower() not in SOURCE_SUFFIXES:
            continue
        rel_parts = path.relative_to(MAIN).parts
        if any(part in SKIP_PARTS for part in rel_parts):
            continue
        yield path


def next_code_line(lines, start: int) -> Optional[str]:
    for index in range(start + 1, len(lines)):
        stripped = lines[index].strip()
        if not stripped or stripped.startswith("//"):
            continue
        return stripped
    return None


def constant_description(name: str, value: str) -> str:
    lower = name.lower()
    if name in {"kFull", "kFast", "kPartial"}:
        return {
            "kFull": "全刷波形值：画质最好、残影最少，但最慢且可能明显闪烁。",
            "kFast": "快刷波形值：速度较快，画质不如全刷，残影严重时仍要全刷。",
            "kPartial": "局刷波形值：速度最快、闪烁最少，但依赖基准并会累积残影。",
        }[name]
    if name == "kWhite":
        return "白色像素值：0xFF 表示一字节 8 个 bit 全为 1。"
    if name == "kBlack":
        return "黑色像素值：0x00 表示一字节 8 个 bit 全为 0。"
    if name == "WIDTH":
        return "面板或缓冲区宽度，单位像素。"
    if name == "HEIGHT":
        return "面板或缓冲区高度，单位像素。"
    if name == "STRIDE":
        return "每行数据占用的字节数；1-bit 屏幕通常是宽度除以 8。"
    if name == "BUFFER_SIZE":
        return "整屏数据缓冲区大小，等于 stride 乘以高度。"
    if "timeout" in lower:
        return f"超时上限：超过 {value} 毫秒仍未完成时，代码会按失败处理。"
    if "stack" in lower:
        return f"任务栈大小：{value}，栈太小会崩溃，太大会浪费 RAM。"
    if "queue" in lower and "depth" in lower:
        return f"队列深度：最多缓存 {value} 个事件。"
    if "priority" in lower:
        return "任务优先级：数值或枚举决定它相对其他任务的重要程度。"
    if "freq" in lower:
        return f"硬件频率：{value}。频率越高，速度和抗干扰要求通常越高。"
    if "sample" in lower:
        return f"采样次数或采样参数：{value}，用于降低测量抖动。"
    if "divider" in lower:
        return f"分压比例：{value}；ADC 电压要乘回这个比例才是真实电压。"
    if "scale" in lower:
        return f"原始值换算比例：把传感器读数转换成物理单位。"
    if "threshold" in lower:
        return f"阈值：超过或低于 {value} 时触发对应逻辑。"
    if "chunk" in lower or "buffer" in lower:
        return f"缓冲区或分块大小：{value}，影响单次读写占用和性能。"
    if name.startswith("kFont"):
        return f"字体尺寸：{value} 像素。"
    if name.startswith("kPad") or name.startswith("kGap"):
        return f"界面间距：{value} 像素。"
    if name.startswith("kScreen"):
        return f"屏幕尺寸：{value}。"
    return f"固定参数 {name} = {value}：由当前模块定义，用于控制布局、时序或硬件行为。"


def struct_description(line: str) -> str:
    match = re.match(r"(struct|class)\s+([A-Za-z_]\w*)", line)
    if not match:
        return "类型定义：把相关字段和操作组合成一个整体。"
    kind, name = match.groups()
    known = {
        "Cmd": "SSD1680 命令常量集合。",
        "Seq": "SSD1680 刷新序列值集合。",
        "Param": "SSD1680 初始化参数集合。",
        "Panel": "面板尺寸、地址范围和 BUSY 超时参数。",
        "Window": "局刷矩形窗口。",
        "Framebuffer": "1-bit 屏幕 framebuffer 操作。",
        "SensorData": "QMI8658A 的原始值、物理量和姿态角结果。",
        "AngleData": "设备姿态角：roll、pitch、yaw。",
    }
    if name in known:
        return known[name]
    return f"{kind} {name}：把相关数据字段组合在一起，表示项目中的一个概念。"


def refined_description(code: str) -> str:
    if code.strip().startswith("} // namespace"):
        return "命名空间结束：离开当前模块作用域。"
    if code.strip() in {"},", "})", "}))", "});"}:
        return "结束上一行的 lambda 或初始化列表：把这段匿名代码/数据交给调用者。"
    match = re.match(r"(?:static\s+)?const(?:expr)?\s+[^=;]+\s+([A-Za-z_]\w*)\s*\[", code)
    if match:
        name = match.group(1)
        if "hidden" in name.lower():
            return "静态查表：保存需要在文件列表中隐藏的系统文件名。"
        if name.startswith("k") and "label" in name.lower():
            return "静态查表：保存界面显示用的固定文字标签。"
        if "rate" in name.lower() or name.endswith("Sr") or "sr" in name:
            return "静态查表：保存音频采样率、比特率或格式转换参数。"
        return f"静态查表数组 {name}：运行时复用固定数据，避免每次重新计算。"

    match = re.match(r"static\s+[^=;]+\s+([A-Za-z_]\w*)\s*\{\s*\}", code)
    if match:
        return f"静态空对象 {match.group(1)}：作为默认返回值或占位值，避免返回悬空引用。"

    match = re.match(r"static\s+char\s+([A-Za-z_]\w*)\s*\[", code)
    if match:
        return f"静态字符缓冲区 {match.group(1)}：保存临时文本，跨函数调用复用。"

    match = re.match(r"constexpr\s+([A-Za-z_]\w*)\s*\(", code)
    if match:
        return f"constexpr 构造函数：允许对象在编译期或轻量场景中直接构造。"

    if re.match(r"static\s+std::(unordered_)?map<", code):
        return "模块私有映射表：保存需要跨调用共享的键值数据，通常配合互斥锁保护。"

    match = re.match(r"static\s+([A-Za-z_:<>]+)\s+([A-Za-z_]\w*)\s*;", code)
    if match:
        type_name, variable = match.groups()
        lower_type = type_name.lower()
        if variable in {"s", "instance"} or "mgr" in type_name.lower():
            return "函数内静态单例：第一次调用时创建，之后一直复用同一个对象。"
        if "mutex" in lower_type:
            return "模块私有互斥锁：保护下面这些静态数据不会被多个任务同时修改。"
        if "map" in lower_type or "set" in lower_type:
            return f"模块私有静态表 {variable}：保存该模块需要跨调用共享的数据。"
        return f"模块私有静态对象 {variable}：只在当前文件或函数内共享。"

    match = re.match(r"constexpr\s+[^=]+\s+([A-Za-z_]\w*)\s*\[\s*\]\s*=", code)
    if match:
        name = match.group(1)
        if "curve" in name.lower():
            return "编译期数组：保存电压到百分比的换算曲线。"
        return f"编译期数组 {name}：运行时只读的固定查表数据。"

    match = re.match(r"static\s+[^;()]+\s+([A-Za-z_]\w*)\s*\(", code)
    if match:
        name = match.group(1)
        known = {
            "fill": "静态辅助函数：用指定颜色快速填满整块 framebuffer。",
            "set_pixel": "静态辅助函数：设置 framebuffer 中一个像素的黑白状态。",
            "fill_rect": "静态辅助函数：在 framebuffer 中填充一个矩形区域。",
            "mv_to_percent": "静态辅助函数：把电压值换算成电池百分比。",
        }
        return known.get(name, f"静态函数 {name}：不创建对象也可以调用，属于当前类型或文件的辅助功能。")

    match = re.match(r"constexpr\s+[^;()]+\s+([A-Za-z_]\w*)\s*\(", code)
    if match:
        return f"编译期函数 {match.group(1)}：常用于固定规则计算，能参与编译期优化。"

    match = re.match(r"(?:static\s+)?constexpr\s+[^=]+\s+([A-Za-z_]\w*)\s*=\s*([^;]+);", code)
    if match:
        return constant_description(match.group(1), match.group(2).strip())
    match = re.match(r"static\s+[^;=]+\s+([A-Za-z_]\w*)\s*=", code)
    if match:
        return f"静态值 {match.group(1)}：只属于当前模块或类，并保持固定含义。"
    if code.startswith(("struct ", "class ")):
        return struct_description(code)
    return ""


def refine_file(path: Path) -> int:
    with path.open("r", encoding="utf-8-sig", newline="") as handle:
        lines = handle.read().splitlines(keepends=True)
    plain = [line.rstrip("\r\n") for line in lines]
    changed = 0

    for index, raw in enumerate(lines):
        stripped = raw.strip()
        if not stripped.startswith(GENERIC_PREFIXES):
            continue
        code = next_code_line(plain, index)
        if not code:
            continue
        note = refined_description(code)
        if not note:
            continue
        indent = raw[: len(raw) - len(raw.lstrip())]
        newline = raw[len(raw.rstrip("\r\n")):]
        lines[index] = f"{indent}// {note}{newline}"
        changed += 1

    if changed:
        with path.open("w", encoding="utf-8", newline="") as handle:
            handle.write("".join(lines))
    return changed


def main() -> None:
    files = 0
    changed = 0
    for path in iter_files():
        count = refine_file(path)
        if count:
            files += 1
            changed += count
    print(f"changed_files={files}")
    print(f"refined_comments={changed}")


if __name__ == "__main__":
    main()
