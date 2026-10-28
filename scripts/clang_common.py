#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""format_code.py / check_code.py 共用：路径、源文件收集、LLVM 工具查找。"""

from __future__ import annotations

import os
import shutil
import sys
from pathlib import Path
from typing import Iterable, Optional, Sequence

# ANSI（Windows 需 enable_ansi）
RED = '\033[31m'
GREEN = '\033[32m'
YELLOW = '\033[33m'
NC = '\033[0m'

SOURCE_SUFFIXES = {'.c', '.cc', '.cpp', '.cxx', '.h', '.hpp', '.hxx'}
DEFAULT_SCOPE = ('main',)

SKIP_DIR_NAMES = {
    '.git',
    'build',
    'managed_components',
    'dist',
    'others',
    '__pycache__',
    '.cache',
}


def enable_ansi() -> None:
    """尽量打开 Windows 控制台 ANSI 转义。"""
    if os.name != 'nt':
        return
    try:
        import ctypes

        kernel32 = ctypes.windll.kernel32  # type: ignore[attr-defined]
        handle = kernel32.GetStdHandle(-11)  # STD_OUTPUT_HANDLE
        mode = ctypes.c_uint32()
        if kernel32.GetConsoleMode(handle, ctypes.byref(mode)):
            kernel32.SetConsoleMode(handle, mode.value | 0x0004)  # ENABLE_VIRTUAL_TERMINAL_PROCESSING
    except Exception:
        pass


def project_root() -> Path:
    """仓库根目录（scripts/ 的上一级）。"""
    return Path(__file__).resolve().parent.parent


def scope_label(paths: Optional[Sequence[str]]) -> str:
    if not paths:
        return 'main/'
    return ', '.join(paths)


def print_scope(root: Path, paths: Optional[Sequence[str]], files: Sequence[Path]) -> None:
    print(f'工程: {root}')
    print(f'范围: {scope_label(paths)}  ({len(files)} 个文件)')


def _should_skip_dir(name: str) -> bool:
    return name in SKIP_DIR_NAMES or name.startswith('.')


def _iter_source_files(base: Path) -> Iterable[Path]:
    if base.is_file():
        if base.suffix.lower() in SOURCE_SUFFIXES:
            yield base.resolve()
        return
    if not base.is_dir():
        return
    for dirpath, dirnames, filenames in os.walk(base):
        dirnames[:] = [d for d in dirnames if not _should_skip_dir(d)]
        for name in filenames:
            p = Path(dirpath) / name
            if p.suffix.lower() in SOURCE_SUFFIXES:
                yield p.resolve()


def collect_sources(root: Path, paths: Optional[Sequence[str]] = None) -> list[Path]:
    """收集待处理源文件；paths 为空时默认 main/。"""
    root = root.resolve()
    rels = list(paths) if paths else list(DEFAULT_SCOPE)
    found: set[Path] = set()
    for rel in rels:
        base = (root / rel).resolve()
        if not str(base).startswith(str(root)):
            continue
        for f in _iter_source_files(base):
            found.add(f)
    return sorted(found)


def find_llvm_tool(name: str) -> Optional[str]:
    """
    查找 clang-format / clang-tidy。
    环境变量优先：CLANG_FORMAT_PATH / CLANG_TIDY_PATH（或通用 LLVM_BIN）。
    """
    env_keys = []
    if name == 'clang-format':
        env_keys = ['CLANG_FORMAT_PATH']
    elif name == 'clang-tidy':
        env_keys = ['CLANG_TIDY_PATH']
    env_keys.append('LLVM_BIN')

    for key in env_keys:
        val = os.environ.get(key, '').strip().strip('"')
        if not val:
            continue
        p = Path(val)
        if key == 'LLVM_BIN':
            cand = p / (name + ('.exe' if os.name == 'nt' else ''))
            if cand.is_file():
                return str(cand)
        elif p.is_file():
            return str(p)

    which = shutil.which(name)
    if which:
        return which

    # 常见本机安装位置（Espressif / LLVM）
    home = Path.home()
    candidates: list[Path] = []
    if os.name == 'nt':
        for base in (
            Path(r'C:\Program Files\LLVM\bin'),
            Path(r'C:\LLVM\bin'),
            Path(os.environ.get('ProgramFiles', r'C:\Program Files')) / 'LLVM' / 'bin',
            home / 'scoop' / 'apps' / 'llvm' / 'current' / 'bin',
        ):
            candidates.append(base / f'{name}.exe')
        # Espressif 工具链里偶有 clang
        esp = Path(os.environ.get('IDF_TOOLS_PATH', r'C:\Espressif'))
        if esp.is_dir():
            for p in esp.rglob(f'{name}.exe'):
                candidates.append(p)
                break
    else:
        candidates.extend([
            Path(f'/usr/bin/{name}'),
            Path(f'/usr/local/bin/{name}'),
            Path('/opt/homebrew/bin') / name,
        ])

    for c in candidates:
        if c.is_file():
            return str(c)
    return None


def find_compile_db(root: Path) -> Optional[Path]:
    """查找 compile_commands.json（优先 build/）。"""
    root = root.resolve()
    candidates = [
        root / 'build' / 'compile_commands.json',
        root / 'compile_commands.json',
    ]
    for c in candidates:
        if c.is_file():
            return c
    # 兜底：浅层搜索
    build = root / 'build'
    if build.is_dir():
        hit = next(build.rglob('compile_commands.json'), None)
        if hit is not None and hit.is_file():
            return hit
    return None


def ensure_scripts_on_path() -> None:
    """保证以任意 cwd 运行时也能 import clang_common。"""
    scripts = Path(__file__).resolve().parent
    s = str(scripts)
    if s not in sys.path:
        sys.path.insert(0, s)
