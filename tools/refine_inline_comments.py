#!/usr/bin/env python3
"""Replace generic inline comments with line-specific explanations."""

from __future__ import annotations

import re
from pathlib import Path
from typing import Dict, List, Optional

from annotate_sources import extract_functions, normalize_heading


ROOT = Path(__file__).resolve().parents[1]
MAIN = ROOT / "main"
DOC = ROOT / "docs" / "代码详解-初学者版.md"
SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".h", ".hpp"}
SKIP_PARTS = {"third_party", "test"}

GENERIC_PREFIXES = (
    "调用或定义 ",
    "执行这条代码：",
    "语法行：",
    "语句结束：",
    "赋值或初始化：",
    "续行：",
    "函数定义或声明：",
    "返回：",
    "条件判断：",
    "条件分支：",
    "循环：",
    "多分支选择：",
    "分支标签：",
    "跳出：",
)


def clean_markdown(text: str) -> str:
    text = re.sub(r"`([^`]*)`", r"\1", text)
    text = re.sub(r"\[([^\]]+)\]\([^)]+\)", r"\1", text)
    text = re.sub(r"[*_>#-]+", "", text)
    text = re.sub(r"\s+", " ", text)
    return text.strip()


def is_generic_description(description: str) -> bool:
    return "详解" in description or "实现流程" in description or "对应小节" in description


def build_doc_notes() -> Dict[str, str]:
    lines = DOC.read_text(encoding="utf-8").splitlines()
    result: Dict[str, str] = {}

    for index, line in enumerate(lines):
        if not line.startswith("#"):
            continue
        heading = normalize_heading(line.lstrip("#").strip())
        names = extract_functions(heading)
        if not names:
            continue

        description = ""
        if "：" in heading:
            description = clean_markdown(heading.split("：", 1)[1])

        if not description or is_generic_description(description):
            paragraphs = []
            for next_line in lines[index + 1:]:
                if next_line.startswith("#"):
                    break
                stripped = next_line.strip()
                if not stripped:
                    if paragraphs and not paragraphs[-1].endswith(("：", ":")):
                        break
                    continue
                paragraphs.append(stripped)
            description = clean_markdown(" ".join(paragraphs))

        if not description or is_generic_description(description):
            continue
        description = description.strip()
        if len(description) > 180:
            description = description[:177].rstrip() + "..."
        for name in names:
            result.setdefault(name, description)
    return result


def build_function_maps():
    documented = build_doc_notes()
    exact: Dict[str, str] = {}
    candidates: Dict[str, List[str]] = {}
    for full_name, desc in documented.items():
        exact[full_name] = desc
        base = full_name.split("::")[-1]
        base = base.split("(", 1)[0]
        candidates.setdefault(base, []).append(desc)

    base_notes: Dict[str, str] = {}
    for base, values in candidates.items():
        unique = []
        for value in values:
            if value not in unique:
                unique.append(value)
        if len(unique) == 1:
            base_notes[base] = unique[0]
    return exact, base_notes


FUNCTION_NOTES, BASE_FUNCTION_NOTES = build_function_maps()


def call_name(text: str) -> Optional[str]:
    for name in extract_functions(text):
        bare = name.split("::")[-1]
        bare_base = bare.split("(", 1)[0]
        if name in FUNCTION_NOTES or bare_base in BASE_FUNCTION_NOTES:
            return name
    return None


def function_purpose(text: str) -> Optional[str]:
    for name in extract_functions(text):
        if name in FUNCTION_NOTES:
            return f"调用 {name}：{FUNCTION_NOTES[name]}"
        bare = name.split("::")[-1]
        bare_base = bare.split("(", 1)[0]
        if bare_base in BASE_FUNCTION_NOTES:
            return f"调用 {name}：{BASE_FUNCTION_NOTES[bare_base]}"
    return None


def explain_line(text: str, previous_code: Optional[str]) -> str:
    stripped = text.strip()
    if not stripped:
        return "空行，用来分隔代码块。"
    if stripped.startswith("#include"):
        return "引入依赖：把头文件内容交给编译器，让本文件可以使用其中的类型、函数和宏。"
    if stripped.startswith("#define"):
        return "定义宏：给一个常量或代码片段起名字，预处理阶段会替换它。"
    if stripped.startswith("#pragma once"):
        return "头文件保护：防止同一个头文件被重复包含。"
    if stripped.startswith("using ") and stripped.endswith(";"):
        return "类型或名称引入：把某个完整名字导入当前作用域，后续代码可以写短名字。"
    if stripped.startswith("namespace "):
        return "打开命名空间：把下面的代码放进这个命名空间，避免和别的模块重名。"
    if stripped.startswith("class ") or stripped.startswith("struct "):
        return "定义类型：把一组数据和相关操作放在一起。"
    if stripped.startswith("enum class"):
        return "定义强类型枚举：用有名字的状态值代替难懂的魔法数字。"
    if stripped.startswith("case "):
        return "分支匹配：" + stripped[:-1] + " 成立时执行下面的代码。"
    if stripped.startswith("if (") or stripped.startswith("if("):
        return "判断条件：" + stripped[stripped.find("(") + 1:stripped.rfind(")")] + " 是否成立。"
    if stripped.startswith("else"):
        return "备用分支：前面的条件不成立时执行这里。"
    if stripped.startswith("for (") or stripped.startswith("for("):
        return "循环执行：按括号里的初始化、条件和步进规则重复运行。"
    if stripped.startswith("while (") or stripped.startswith("while("):
        return "循环执行：条件为真时重复运行，条件变化后退出。"
    if stripped.startswith("switch (") or stripped.startswith("switch("):
        return "多分支判断：根据括号里的值选择不同 case。"
    if stripped == "break;":
        return "跳出当前循环或 switch，不再执行剩余分支。"
    if stripped == "continue;":
        return "跳过本次循环剩余代码，直接进入下一轮。"
    if stripped.startswith("return "):
        expr = stripped[7:].rstrip(";")
        purpose = function_purpose(expr)
        if purpose:
            return f"返回结果：{purpose}。"
        return f"返回 {expr} 给调用者，表示当前函数的执行结果。"
    if stripped == "return;":
        return "提前结束当前函数，不返回具体值。"
    if stripped.startswith("static "):
        return "静态声明：限制这个名字只在本文件或本函数内使用，并保持自己的生命周期。"
    if stripped.startswith("constexpr "):
        return "定义编译期常量：值在编译阶段确定，运行时不能修改。"
    if stripped.startswith("const "):
        return "定义只读对象或变量：初始化后不能在当前位置修改。"

    signature = re.search(r"([A-Za-z_~][A-Za-z0-9_:~<>]*)\s*\(", stripped)
    if signature and (stripped.endswith(")") or stripped.endswith(" const") or stripped.endswith(" override")):
        purpose = function_purpose(signature.group(1) + "()")
        if purpose:
            return "函数定义/声明：" + purpose + "。"
        return "定义或声明函数 " + signature.group(1) + "：参数在括号里，返回类型写在函数名前面。"

    if previous_code and previous_code.rstrip().endswith(("(", ",", "&&", "||", "+", "=", "?")):
        if "(" in stripped or ")" in stripped:
            return "函数签名或调用的续行：补充上一行的参数 " + stripped + "。"
        if "&&" in stripped or "||" in stripped:
            return "条件判断的续行：继续补充上一行的条件 " + stripped + "。"
        return "表达式续行：继续完成上一行没有结束的计算 " + stripped + "。"

    if re.match(r"^[A-Za-z_:][\w:<>]*\s+[A-Za-z_]\w*\s*\{\}\s*;", stripped):
        return "声明并默认初始化对象：" + stripped[:-1] + "。"

    if re.match(r"^[A-Za-z_:][\w:<>]*\s+[A-Za-z_]\w*\s*=", stripped):
        declaration = stripped[:-1]
        if "=" not in declaration:
            return "定义变量：" + stripped
        left, right = declaration.split("=", 1)
        purpose = function_purpose(right)
        if purpose:
            return f"定义变量并调用函数初始化：{purpose}，结果保存到 {left.strip()}。"
        return f"定义变量 {left.strip()} 并初始化为 {right.strip()}。"

    purpose = function_purpose(stripped)
    if purpose:
        return purpose + "。"

    if re.search(r"\b(?:static_cast|dynamic_cast|reinterpret_cast|const_cast)\s*<", stripped):
        return "显式类型转换：把值转换成目标类型，明确告诉编译器这里是有意转换。"

    if "&&" in stripped or "||" in stripped:
        return "组合条件：左右两个条件共同决定这一段代码是否继续执行。"

    if any(operator in stripped for operator in ("==", "!=", "<=", ">=", " < ", " > ")):
        return "比较表达式：比较左右两边的值，结果会用于条件判断或返回。"

    if re.match(r"^(?:[A-Za-z_]\w*::)*[A-Za-z_]\w*\s*\(", stripped):
        return "调用 " + stripped + "，执行这个函数并处理它的返回值或状态变化。"

    if "=" in stripped and stripped.endswith(";"):
        statement = stripped[:-1]
        if "=" not in statement:
            return "执行这条语句：" + stripped
        left, right = statement.split("=", 1)
        return f"赋值：把右侧 {right.strip()} 的结果保存到左侧 {left.strip()}。"

    if ("." in stripped or "->" in stripped) and stripped.endswith(";"):
        return "访问对象成员：" + stripped + "；读取或修改这个对象里的成员状态。"

    if re.search(r"[A-Za-z_][A-Za-z0-9_:]*\s*\(", stripped):
        return "函数调用：" + stripped + "；执行该函数并处理它的返回值或副作用。"

    if stripped.endswith(";"):
        return "执行语句：" + stripped + "；它会更新一次变量、对象或程序状态。"

    words = re.findall(r"[A-Za-z_][A-Za-z0-9_]*", stripped)
    if words:
        return "执行这条代码：" + stripped + "。它会更新变量、对象状态或调用链。"
    return "执行这条代码：" + stripped + "。"


def next_code_line(lines: List[str], start: int) -> Optional[str]:
    for index in range(start + 1, len(lines)):
        stripped = lines[index].strip()
        if not stripped or stripped.startswith("//"):
            continue
        return re.sub(r"\s*//.*$", "", stripped).strip()
    return None


def previous_code_line(lines: List[str], start: int) -> Optional[str]:
    for index in range(start - 1, -1, -1):
        stripped = lines[index].strip()
        if not stripped or stripped.startswith("//"):
            continue
        return re.sub(r"\s*//.*$", "", stripped).strip()
    return None


def refine_file(path: Path) -> int:
    with path.open("r", encoding="utf-8-sig", newline="") as handle:
        lines = handle.read().splitlines(keepends=True)

    changed = 0
    for index, raw in enumerate(lines):
        stripped = raw.strip()
        if not stripped.startswith("//"):
            continue
        body = stripped[2:].strip()
        if not body.startswith(GENERIC_PREFIXES):
            continue
        text_lines = [line.rstrip("\r\n") for line in lines]
        code = next_code_line(text_lines, index)
        if not code:
            continue
        replacement = explain_line(code, previous_code_line(text_lines, index))
        indent = raw[: len(raw) - len(raw.lstrip())]
        newline = raw[len(raw.rstrip("\r\n")):]
        lines[index] = f"{indent}// {replacement}{newline}"
        changed += 1

    if changed:
        with path.open("w", encoding="utf-8", newline="") as handle:
            handle.write("".join(lines))
    return changed


def iter_files():
    for path in sorted(MAIN.rglob("*")):
        if not path.is_file() or path.suffix.lower() not in SOURCE_SUFFIXES:
            continue
        rel_parts = path.relative_to(MAIN).parts
        if any(part in SKIP_PARTS for part in rel_parts):
            continue
        yield path


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
