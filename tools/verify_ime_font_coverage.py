#!/usr/bin/env python3
"""Verify zh_pinyin_decoder output chars exist in simhei bitmap font LUT."""

from __future__ import annotations

import json
import os
import re
import struct
import sys


ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DECODER_DIR = os.path.join(ROOT, "others", "zh_pinyin_decoder", "zh_pinyin_decoder")
PINYIN_BIN = os.path.join(DECODER_DIR, "bin", "zh_pinyin.bin")
WORD_JSON = os.path.join(DECODER_DIR, "bin", "zh_word_dict.json")
CODE_TABLE_C = os.path.join(DECODER_DIR, "zh_code_table.c")
FONT_BIN = os.path.join(ROOT, "assets", "littlefs_assets", "fonts", "simhei_16.bin")


def utf8_to_cp(data: bytes) -> int | None:
    if len(data) != 3 or data[0] < 0xE0:
        return None
    return ((data[0] & 0x0F) << 12) | ((data[1] & 0x3F) << 6) | (data[2] & 0x3F)


def load_font_lut(path: str) -> set[int]:
    with open(path, "rb") as f:
        hdr = f.read(28)
        if len(hdr) != 28:
            raise RuntimeError(f"font header too small: {path}")
        magic = hdr[:4]
        if magic != b"EBF1":
            raise RuntimeError(f"not EBF1 font: {path}")
        glyph_count = struct.unpack_from("<H", hdr, 10)[0]
        lut = f.read(65536 * 2)
    present: set[int] = set()
    for cp in range(65536):
        gid = struct.unpack_from("<H", lut, cp * 2)[0]
        if gid != 0:
            present.add(cp)
    print(f"  font glyphs (header): {glyph_count}")
    print(f"  font LUT entries:       {len(present)}")
    return present


def parse_code_table_c(path: str) -> list[tuple[int, list[int]]]:
    text = open(path, encoding="utf-8").read()
    index_rows = re.findall(
        r"\{\s*(\d+),\s*0x([0-9A-Fa-f]+),\s*code_table_(\w)",
        text,
    )
    num_tables: dict[str, list[int]] = {}
    for m in re.finditer(
        r"const uint8_t\s+code_table_(\w)_num\[\]\s*=\s*\{([^}]+)\}",
        text,
    ):
        letter = m.group(1)
        nums = [int(x.strip()) for x in m.group(2).split(",") if x.strip()]
        num_tables[letter] = nums

    letters = "abcdefghijklmnopqrstuvwxyz"
    per_letter: list[tuple[int, list[int]]] = []
    for ch in letters:
        row = next((r for r in index_rows if r[2] == ch), None)
        if row is None:
            per_letter.append((0, []))
            continue
        length = int(row[0])
        nums = num_tables.get(ch, [])[:length]
        per_letter.append((int(row[1], 16), nums))
    return per_letter


def extract_pinyin_bin_chars(bin_path: str, code_table_path: str) -> set[int]:
    per_letter = parse_code_table_c(code_table_path)
    data = open(bin_path, "rb").read()
    chars: set[int] = set()
    total_slots = 0

    with open(code_table_path, encoding="utf-8") as f:
        ctext = f.read()
    off_tables: dict[str, list[int]] = {}
    for m in re.finditer(
        r"const uint16_t\s+code_table_(\w)_off\[\]\s*=\s*\{([^}]+)\}",
        ctext,
    ):
        letter = m.group(1)
        offs = [int(x.strip(), 0) for x in m.group(2).split(",") if x.strip()]
        off_tables[letter] = offs

    letters = "abcdefghijklmnopqrstuvwxyz"
    for i, ch in enumerate(letters):
        char_start, nums = per_letter[i]
        if char_start == 0 or not nums:
            continue
        offs = off_tables.get(ch, [])
        for j, count in enumerate(nums):
            if j >= len(offs) or count == 0:
                continue
            loc = char_start + offs[j]
            total_slots += count
            chunk = data[loc : loc + count * 3]
            for k in range(count):
                piece = chunk[k * 3 : k * 3 + 3]
                cp = utf8_to_cp(piece)
                if cp is not None:
                    chars.add(cp)
    print(f"  pinyin syllable slots:  {total_slots}")
    print(f"  unique hanzi (bin):     {len(chars)}")
    return chars


def extract_word_dict_chars(json_path: str) -> set[int]:
    with open(json_path, encoding="utf-8") as f:
        obj = json.load(f)
    chars: set[int] = set()
    for values in obj.values():
        for s in values:
            for ch in s:
                cp = ord(ch)
                if cp >= 0x80:
                    chars.add(cp)
    print(f"  unique hanzi (json):    {len(chars)}")
    return chars


def main() -> int:
    print("=== IME / Font coverage verification ===\n")

    for label, path in [
        ("pinyin bin", PINYIN_BIN),
        ("word dict", WORD_JSON),
        ("code table", CODE_TABLE_C),
        ("font bin", FONT_BIN),
    ]:
        if not os.path.isfile(path):
            print(f"ERROR: missing {label}: {path}")
            return 1

    print("[1] Load simhei_16.bin LUT")
    font_cps = load_font_lut(FONT_BIN)

    print("\n[2] Extract hanzi from zh_pinyin.bin (via zh_code_table.c)")
    pinyin_cps = extract_pinyin_bin_chars(PINYIN_BIN, CODE_TABLE_C)

    print("\n[3] Extract hanzi from zh_word_dict.json")
    word_cps = extract_word_dict_chars(WORD_JSON)

    all_ime = pinyin_cps | word_cps
    print(f"\n[4] Combined unique hanzi: {len(all_ime)}")

    miss_pinyin = sorted(pinyin_cps - font_cps)
    miss_word = sorted(word_cps - font_cps)
    miss_all = sorted(all_ime - font_cps)

    print("\n=== Result ===")
    print(f"  zh_pinyin.bin missing:      {len(miss_pinyin)}")
    print(f"  zh_word_dict.json missing:  {len(miss_word)}")
    print(f"  combined missing:           {len(miss_all)}")

    if miss_all:
        sample = "".join(chr(cp) for cp in miss_all[:40])
        print(f"  sample missing chars: {sample}")
        out = os.path.join(ROOT, "tools", "ime_font_missing.txt")
        with open(out, "w", encoding="utf-8") as f:
            for cp in miss_all:
                f.write(f"U+{cp:04X}\t{chr(cp)}\n")
        print(f"  full list written to: {out}")
        return 1

    print("\nPASS: all IME hanzi are present in simhei_16.bin")
    return 0


if __name__ == "__main__":
    sys.exit(main())
