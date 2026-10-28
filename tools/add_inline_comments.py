#!/usr/bin/env python3
"""Insert beginner-oriented // comments above first-party source lines."""

from __future__ import annotations

import re
from pathlib import Path
from typing import Dict, List, Optional, Tuple

from build_line_reader import note_for
from annotate_sources import extract_functions, parse_doc_functions


ROOT = Path(__file__).resolve().parents[1]
MAIN = ROOT / "main"
MARKER = "[初学者导读]"
SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".h", ".hpp"}
SKIP_PARTS = {"third_party", "test"}


def build_function_notes() -> Tuple[Dict[str, str], Dict[str, str]]:
    exact: Dict[str, str] = {}
    bare_candidates: Dict[str, List[Tuple[str, str]]] = {}
    try:
        documented = parse_doc_functions()
    except Exception:
        documented = {}

    for name, desc in documented.items():
        exact[name] = desc
        bare = name.split("::")[-1]
        bare_candidates.setdefault(bare, []).append((name, desc))

    bare: Dict[str, str] = {}
    for name, candidates in bare_candidates.items():
        unique = {}
        for full_name, desc in candidates:
            unique[full_name] = desc
        if len(unique) == 1:
            bare[name] = next(iter(unique.values()))
    return exact, bare


FUNCTION_NOTES, BARE_FUNCTION_NOTES = build_function_notes()


def detailed_note(text: str) -> str:
    for name in extract_functions(text):
        if name in FUNCTION_NOTES:
            return f"调用或定义 {name}：{FUNCTION_NOTES[name]}。"
        bare = name.split("::")[-1]
        if bare in BARE_FUNCTION_NOTES:
            return f"调用或定义 {name}：{BARE_FUNCTION_NOTES[bare]}。"
    return note_for(text)


def should_skip_line(text: str, in_block_comment: bool, in_macro: bool) -> bool:
    if in_block_comment or in_macro:
        return True
    stripped = text.strip()
    if not stripped:
        return True
    if stripped in {"{", "}", "};", "{}", "{},"}:
        return True
    if stripped.startswith("//"):
        return True
    if stripped.startswith("/*") or stripped.startswith("*") or stripped.startswith("*/"):
        return True
    return False


def update_block_comment_state(text: str, in_block_comment: bool) -> bool:
    stripped = text.strip()
    if in_block_comment:
        return False if "*/" in stripped else True
    if stripped.startswith("/*") and "*/" not in stripped:
        return True
    return False


def update_macro_state(text: str, in_macro: bool) -> bool:
    stripped = text.rstrip()
    if in_macro:
        return stripped.endswith("\\")
    if text.lstrip().startswith("#") and stripped.endswith("\\"):
        return True
    return False


def file_note(path: Path) -> str:
    return f"文件：{path.relative_to(ROOT).as_posix()}；{note_for('')}"


def add_comments(path: Path) -> int:
    try:
        with path.open("r", encoding="utf-8-sig", newline="") as handle:
            lines = handle.read().splitlines(keepends=True)
    except UnicodeDecodeError:
        return 0

    output: List[str] = []
    inserted = 0
    in_block_comment = False
    in_macro = False
    previous_was_comment = False

    for raw in lines:
        text = raw.rstrip("\r\n")
        newline = raw[len(text):] or ("\r\n" if "\r\n" in raw else "\n")

        line_is_comment = text.strip().startswith("//")
        if (
            not should_skip_line(text, in_block_comment, in_macro)
            and not previous_was_comment
            and text.strip() not in {"", "{" , "}", "};"}
        ):
            note = detailed_note(text)
            indent = re.match(r"\s*", text).group(0)
            output.append(f"{indent}// {note}{newline}")
            inserted += 1

        output.append(raw)
        in_block_comment = update_block_comment_state(text, in_block_comment)
        in_macro = update_macro_state(text, in_macro)
        previous_was_comment = line_is_comment

    if inserted:
        with path.open("w", encoding="utf-8", newline="") as handle:
            handle.write("".join(output))
    return inserted


def iter_files():
    for path in sorted(MAIN.rglob("*")):
        if not path.is_file() or path.suffix.lower() not in SOURCE_SUFFIXES:
            continue
        rel_parts = path.relative_to(MAIN).parts
        if any(part in SKIP_PARTS for part in rel_parts):
            continue
        yield path


def main() -> None:
    changed_files = 0
    inserted = 0
    for path in iter_files():
        count = add_comments(path)
        if count:
            changed_files += 1
            inserted += count
    print(f"changed_files={changed_files}")
    print(f"inserted_comments={inserted}")


if __name__ == "__main__":
    main()
