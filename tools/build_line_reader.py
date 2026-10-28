#!/usr/bin/env python3
"""Build a self-contained line-by-line source reader for first-party code."""

from __future__ import annotations

import html
import json
import re
from pathlib import Path
from typing import Dict, List


ROOT = Path(__file__).resolve().parents[1]
MAIN = ROOT / "main"
OUTPUT = ROOT / "docs" / "逐行代码注释阅读器.html"
MARKER = "[初学者导读]"

SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".h", ".hpp"}
SKIP_PARTS = {"third_party", "test"}

EXACT_NOTES: Dict[str, str] = {
    "using apps::AppId;": "类型别名引入：把 apps::AppId 直接导入当前命名空间，后面写代码不用每次写完整长名字。",
    "using apps::AppRegistry;": "类型别名引入：把 apps::AppRegistry 导入当前作用域，后面直接写 AppRegistry。",
    "using namespace ...;": "命名空间引入：把某个命名空间的成员展开到当前作用域。实际项目里通常少用，避免名称冲突。",
}


def note_for(line: str) -> str:
    raw = line.rstrip("\r\n")
    text = raw.strip()
    if not text:
        return "空行：用于分隔逻辑块，方便阅读。"
    if text in EXACT_NOTES:
        return EXACT_NOTES[text]
    if text.startswith(MARKER):
        return text.replace(MARKER, "项目导读：")
    if text.startswith("//"):
        return "注释：" + text[2:].strip()
    if text.startswith("/*") or text.startswith("*") or text.startswith("*/"):
        cleaned = text.strip("/* ")
        return "注释：" + cleaned if cleaned else "注释块的一部分。"
    if text.startswith("#pragma once"):
        return "头文件保护：保证这个头文件在一次编译单元里只被包含一次。"
    if text.startswith("#include"):
        return "引入依赖：把头文件内容交给编译器解析，使用其中的类型、函数和宏。"
    if text.startswith("#define") or text.startswith("#if") or text.startswith("#endif") or text.startswith("#else"):
        return "预处理指令：由编译器在正式编译前处理，常用于条件编译、宏和平台差异。"
    if text.startswith("namespace ") and text.endswith("{"):
        return "打开命名空间：把下面的类型和函数放进这个命名空间，避免全局名称冲突。"
    if text.startswith("namespace ") or text.endswith("// namespace"):
        return "命名空间边界：标记项目模块的作用域范围。"
    if text.startswith("using ") and text.endswith(";"):
        return "名称或类型引入：把某个名称、类型或命名空间成员导入当前作用域，简化后续写法。"
    if text.startswith("typedef "):
        return "定义类型别名：给已有类型起一个更容易理解的名字。"
    if text.startswith("enum class") or text.startswith("enum "):
        return "枚举定义：用有名字的常量表示有限状态，避免直接写魔法数字。"
    if text.startswith("struct "):
        return "结构体定义：把一组相关数据放在一起，默认成员通常公开。"
    if text.startswith("class "):
        return "类定义：把数据和操作这些数据的方法绑定在一起。"
    if text in {"public:", "private:", "protected:"}:
        return "访问控制：决定后面的成员能否被外部代码访问。"
    if text.startswith("template"):
        return "模板声明：让同一段代码适配不同类型。"
    if text.startswith("if (") or text.startswith("if("):
        return "条件判断：条件成立时执行下面的代码块。"
    if text.startswith("else if") or text.startswith("else"):
        return "条件分支：前面条件不成立时继续判断或执行备用逻辑。"
    if text.startswith("for (") or text.startswith("for("):
        return "循环：按初始化、条件、步进规则重复执行代码块。"
    if text.startswith("while (") or text.startswith("while("):
        return "循环：条件为真时重复执行代码块。"
    if text.startswith("switch (") or text.startswith("switch("):
        return "多分支选择：根据一个值选择不同 case。"
    if text.startswith("case "):
        return "分支标签：当前 switch 值匹配时从这里开始执行。"
    if text.startswith("return "):
        return "返回：把结果交还给调用者，或提前结束当前函数。"
    if text == "return;":
        return "提前返回：结束当前函数，不返回具体值。"
    if text == "break;":
        return "跳出：结束当前循环或 switch 分支。"
    if text == "continue;":
        return "继续下一轮：跳过本次循环剩余代码。"
    if text.startswith("static "):
        return "静态声明：限制作用域，或让变量只初始化一次并保持状态。"
    if text.startswith("constexpr "):
        return "编译期常量：值应在编译阶段确定，运行时不能修改。"
    if text.startswith("const "):
        return "只读约束：这个值或对象在当前位置不能被修改。"
    if text.startswith("volatile "):
        return "易变变量：告诉编译器该值可能被中断或硬件改变，不能随意优化读取。"
    if re.match(r'^[A-Za-z_~][\w:~<>]*\s*::\s*[A-Za-z_~][\w~]*\s*\(', text) or re.match(
        r'^[A-Za-z_][\w:<>,*&\s]*\s+[A-Za-z_]\w*\s*\(', text
    ):
        return "函数定义或声明：描述它的输入、返回值和职责，具体约束看上方函数导读。"
    if text.endswith("{"):
        return "开始代码块：下面是这个函数、分支或循环的主体。"
    if text == "}":
        return "结束当前代码块。"
    if "=" in text and text.endswith(";"):
        return "赋值或初始化：把右侧结果保存到左侧变量。"
    if text.endswith(";"):
        return "语句结束：执行一次声明、调用或状态修改。"
    if text.endswith("(") or text.endswith(","):
        return "续行：这条语句还没有结束，下一行继续。"
    return "语法行：结合上方函数注释和 docs/代码详解-初学者版.md 一起阅读。"


def iter_files():
    for path in sorted(MAIN.rglob("*")):
        if not path.is_file() or path.suffix.lower() not in SOURCE_SUFFIXES:
            continue
        rel_parts = path.relative_to(MAIN).parts
        if any(part in SKIP_PARTS for part in rel_parts):
            continue
        yield path


def build_data():
    files = []
    for path in iter_files():
        rel = path.relative_to(ROOT).as_posix()
        lines = []
        with path.open("r", encoding="utf-8-sig", newline="") as handle:
            for number, line in enumerate(handle.read().splitlines(keepends=False), 1):
                lines.append({
                    "n": number,
                    "code": line.expandtabs(4),
                    "note": note_for(line),
                })
        files.append({"path": rel, "lines": lines})
    return files


def main() -> None:
    data = build_data()
    payload = json.dumps(data, ensure_ascii=False)
    output = f"""<!doctype html>
<html lang="zh-CN">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Ebook 逐行代码注释阅读器</title>
<style>
:root {{
  --bg: #fffdf9;
  --sidebar: #f7f4ed;
  --border: #ddd6c9;
  --text: #23282d;
  --muted: #6f746f;
  --accent: #0f766e;
  --code: #f4f2eb;
}}
* {{ box-sizing: border-box; }}
html, body {{ height: 100%; }}
body {{
  margin: 0;
  background: var(--bg);
  color: var(--text);
  font-family: "Microsoft YaHei", "PingFang SC", "Noto Sans CJK SC", sans-serif;
}}
.layout {{ display: flex; min-height: 100vh; }}
.sidebar {{
  position: fixed;
  inset: 0 auto 0 0;
  width: 360px;
  overflow: auto;
  border-right: 1px solid var(--border);
  background: var(--sidebar);
}}
.sidebar-head {{ padding: 18px 18px 12px; border-bottom: 1px solid var(--border); }}
.sidebar-head h1 {{ margin: 0; font-size: 19px; }}
.sidebar-head p {{ margin: 6px 0 0; color: var(--muted); font-size: 12px; }}
#fileSearch {{
  width: calc(100% - 28px);
  margin: 12px 14px;
  padding: 9px 10px;
  border: 1px solid var(--border);
  border-radius: 6px;
  font: inherit;
}}
.file-list {{ padding: 0 10px 30px; }}
.file-button {{
  display: block;
  width: 100%;
  margin: 3px 0;
  padding: 7px 9px;
  border: 0;
  border-radius: 5px;
  background: transparent;
  color: var(--text);
  text-align: left;
  font: inherit;
  font-size: 13px;
  cursor: pointer;
}}
.file-button:hover, .file-button.active {{ background: #dff3ef; color: #075e56; }}
.content {{ width: calc(100% - 360px); margin-left: 360px; padding: 26px 30px 80px; }}
.content h2 {{ margin: 0 0 8px; font-size: 22px; }}
.content .path {{ color: var(--muted); font-family: Consolas, monospace; font-size: 13px; }}
.code-table {{ width: 100%; margin-top: 18px; border-collapse: collapse; table-layout: fixed; }}
.code-table td {{ vertical-align: top; padding: 1px 8px; border-bottom: 1px solid #f0ece2; }}
.code-table .num {{ width: 50px; color: #999; text-align: right; user-select: none; font-family: Consolas, monospace; font-size: 12px; }}
.code-table .code {{ width: 58%; overflow: hidden; background: var(--code); white-space: pre-wrap; overflow-wrap: anywhere; font-family: Consolas, "Cascadia Mono", monospace; font-size: 13px; }}
.code-table .note {{ color: #4f5d57; font-size: 13px; line-height: 1.55; }}
.code-table tr:hover .code {{ background: #e9f7f4; }}
@media (max-width: 900px) {{
  .sidebar {{ position: relative; width: 100%; max-height: 260px; }}
  .content {{ width: 100%; margin-left: 0; padding: 18px; }}
  .code-table .code {{ width: 48%; }}
}}
</style>
</head>
<body>
<div class="layout">
  <aside class="sidebar">
    <div class="sidebar-head">
      <h1>Ebook 逐行代码注释阅读器</h1>
      <p>左侧选文件，右侧每一行代码对应一条中文解释。</p>
    </div>
    <input id="fileSearch" type="search" placeholder="搜索文件、函数名或路径">
    <div class="file-list" id="fileList"></div>
  </aside>
  <main class="content">
    <h2 id="fileTitle">请选择左侧文件</h2>
    <div class="path" id="filePath"></div>
    <table class="code-table"><tbody id="codeBody"></tbody></table>
  </main>
</div>
<script>
const sourceData = {payload};
const fileList = document.getElementById("fileList");
const codeBody = document.getElementById("codeBody");
const fileTitle = document.getElementById("fileTitle");
const filePath = document.getElementById("filePath");
const fileSearch = document.getElementById("fileSearch");
let currentFile = null;

function escapeText(value) {{
  const div = document.createElement("div");
  div.textContent = value;
  return div.innerHTML;
}}

function showFile(file) {{
  currentFile = file;
  fileTitle.textContent = file.path.split("/").pop();
  filePath.textContent = file.path;
  codeBody.textContent = "";
  file.lines.forEach(function (line) {{
    const tr = document.createElement("tr");
    const num = document.createElement("td");
    num.className = "num";
    num.textContent = line.n;
    const code = document.createElement("td");
    code.className = "code";
    code.textContent = line.code;
    const note = document.createElement("td");
    note.className = "note";
    note.textContent = line.note;
    tr.appendChild(num);
    tr.appendChild(code);
    tr.appendChild(note);
    codeBody.appendChild(tr);
  }});
  document.querySelectorAll(".file-button").forEach(function (button) {{
    button.classList.toggle("active", button.dataset.path === file.path);
  }});
}}

sourceData.forEach(function (file) {{
  const button = document.createElement("button");
  button.type = "button";
  button.className = "file-button";
  button.dataset.path = file.path;
  button.textContent = file.path;
  button.addEventListener("click", function () {{ showFile(file); }});
  fileList.appendChild(button);
}});

fileSearch.addEventListener("input", function () {{
  const query = fileSearch.value.trim().toLowerCase();
  document.querySelectorAll(".file-button").forEach(function (button) {{
    button.hidden = Boolean(query) && !button.dataset.path.toLowerCase().includes(query);
  }});
}});

if (sourceData.length) {{
  showFile(sourceData[0]);
}}
</script>
</body>
</html>
"""
    OUTPUT.write_text(output, encoding="utf-8")
    print(f"generated={OUTPUT}")
    print(f"files={len(data)}")
    print(f"lines={sum(len(item['lines']) for item in data)}")


if __name__ == "__main__":
    main()
