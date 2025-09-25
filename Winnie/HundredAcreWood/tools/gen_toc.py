#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Generate / update TOC for a Markdown file.
Usage:
  python Winnie/HundredAcreWood/tools/gen_toc.py Winnie/HundredAcreWood/progress/septemberMind.md

It finds headings and replaces the block between:
  <!-- TOC-BEGIN -->
  ...
  <!-- TOC-END -->
If not found, inserts after the front-matter/title area.

Design goals:
- Stable slug generation for Chinese + ASCII
- Ignore the main H1 and any heading containing '目录'
- Limit depth to H4 (configurable)
"""
from __future__ import annotations
import re
import sys
import unicodedata
from pathlib import Path
from typing import List, Tuple

TOC_BEGIN = "<!-- TOC-BEGIN -->"
TOC_END = "<!-- TOC-END -->"
MAX_DEPTH = 4  # include up to ####

HEADING_RE = re.compile(r'^(#{1,6})\s+(.+?)\s*$')

REMOVE_CHARS_RE = re.compile(r'[\\\\/`"'"'"()\[\]{}<>:：,，。.!！？?；;“”‘’]' )
MULTI_DASH_RE = re.compile(r'-{2,}')


def slugify(text: str) -> str:
    # Normalize width / compatibility forms
    text = unicodedata.normalize('NFKC', text.strip().lower())
    # Remove markdown link patterns like [text](url) inside heading just in case
    text = re.sub(r'\[[^\]]+\]\([^)]*\)', lambda m: m.group(0).split(']')[0][1:], text)
    text = REMOVE_CHARS_RE.sub('', text)
    text = re.sub(r'\s+', '-', text)
    text = MULTI_DASH_RE.sub('-', text).strip('-')
    return text

def extract_headings(md: str) -> List[Tuple[int, str]]:
    headings: List[Tuple[int, str]] = []
    for line in md.splitlines():
        m = HEADING_RE.match(line)
        if not m:
            continue
        level = len(m.group(1))
        title = m.group(2).strip()
        # Skip main title (H1) and headings that are exactly or contain '目录'
        if level == 1:
            continue
        if '目录' in title:
            continue
        headings.append((level, title))
    return headings

def build_toc(headings: List[Tuple[int, str]]) -> str:
    lines = ["## 目录", ""]
    for level, title in headings:
        if level > MAX_DEPTH:
            continue
        slug = slugify(title)
        indent = '  ' * (level - 2) if level >= 2 else ''
        if level == 1:
            continue
        lines.append(f"{indent}- [{title}](#{slug})")
    return '\n'.join(lines) + '\n'

def replace_toc(original: str, toc_block: str) -> str:
    if TOC_BEGIN in original and TOC_END in original:
        pattern = re.compile(re.escape(TOC_BEGIN) + r'.*?' + re.escape(TOC_END), re.DOTALL)
        return pattern.sub(TOC_BEGIN + '\n' + toc_block + TOC_END, original)
    # Insert after first block of metadata or after first heading
    lines = original.splitlines()
    insert_index = 0
    for i, line in enumerate(lines[:50]):  # heuristic scan early part
        if line.startswith('# '):
            insert_index = i + 1
            break
    new_lines = lines[:insert_index] + ['', TOC_BEGIN, toc_block.rstrip(), TOC_END, ''] + lines[insert_index:]
    return '\n'.join(new_lines)

def process(path: Path) -> bool:
    content = path.read_text(encoding='utf-8')
    headings = extract_headings(content)
    toc = build_toc(headings)
    updated = replace_toc(content, toc)
    if updated != content:
        path.write_text(updated, encoding='utf-8')
        return True
    return False

def main():
    if len(sys.argv) < 2:
        print("Usage: python gen_toc.py <markdown_file>")
        return
    md_file = Path(sys.argv[1])
    if not md_file.exists():
        print(f"File not found: {md_file}")
        return
    changed = process(md_file)
    print("TOC updated" if changed else "No change")

if __name__ == '__main__':
    main()
