#!/usr/bin/env python3
"""Convert Markdown files to DOCX using python-markdown + html2docx."""

import argparse
from io import BytesIO
from pathlib import Path

from docx import Document
from html2docx import html2docx
from markdown import markdown

DEFAULT_EXTENSIONS = [
    "extra",  # includes abbr, attrs, def_list, fenced_code, tables, footnotes
    "sane_lists",
    "toc",
]


def convert_md_to_docx(src: Path, dest: Path) -> None:
    text = src.read_text(encoding="utf-8")
    html = markdown(text, extensions=DEFAULT_EXTENSIONS)

    buffer: BytesIO = html2docx(html, title=src.stem)
    document = Document(buffer)

    document.core_properties.title = src.stem
    document.core_properties.subject = "Converted from Markdown"
    document.core_properties.category = "Documentation"

    dest.parent.mkdir(parents=True, exist_ok=True)
    document.save(dest)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Convert Markdown to DOCX")
    parser.add_argument("source", type=Path, help="Markdown file path")
    parser.add_argument("output", type=Path, nargs="?", help="Output DOCX path")
    args = parser.parse_args()

    src_path: Path = args.source.resolve()
    if args.output:
        out_path = args.output.resolve()
    else:
        out_path = src_path.with_suffix(".docx")

    convert_md_to_docx(src_path, out_path)
