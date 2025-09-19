#!/usr/bin/env python3
# 转换 docx -> markdown，提取内嵌图片到指定 media 文件夹
import os
import sys
import mammoth
import html2text

# 要转换的文件（相对仓库根）
INPUT = [
    r"specs\技术研发流程说明V1.0 --Jony.docx",
    r"specs\XXX项目软件开发概要设计V1.0.docx",
    r"specs\XXX项目系统架构设计V1.1.docx",
    r"docs\specs\软件需求文档.docx",
]

OUT_DIR = r"specs\imported"
os.makedirs(OUT_DIR, exist_ok=True)

h = html2text.HTML2Text()
h.ignore_links = False

def save_image_bytes(media_dir, image_bytes, ext, idx):
    filename = f"image_{idx}.{ext}"
    path = os.path.join(media_dir, filename)
    with open(path, "wb") as f:
        f.write(image_bytes)
    return filename


def convert(docx_path):
    base = os.path.splitext(os.path.basename(docx_path))[0]
    media_dir_name = f"media_{base}"
    media_dir = os.path.join(OUT_DIR, media_dir_name)
    os.makedirs(media_dir, exist_ok=True)
    out_md = os.path.join(OUT_DIR, base + ".md")

    with open(docx_path, "rb") as f:
        image_index = 1
        def handle_image(image):
            nonlocal image_index
            content_type = image.content_type  # e.g. 'image/png'
            ext = content_type.split('/')[-1]
            try:
                image_bytes = image.read()
            except Exception:
                image_bytes = None
            if image_bytes:
                filename = save_image_bytes(media_dir, image_bytes, ext, image_index)
                image_index += 1
                # 返回 HTML img src 相对路径（相对于生成的 md 所在目录）
                return {"src": os.path.join("imported", media_dir_name, filename)}
            else:
                return {"alt": image.alt_text or "image"}

        result = mammoth.convert_to_html(f, convert_image=mammoth.images.inline(handle_image))
        html = result.value
        md = h.handle(html)

    with open(out_md, "w", encoding="utf-8") as out:
        out.write(md)
    return out_md

if __name__ == '__main__':
    converted = []
    for p in INPUT:
        if os.path.exists(p):
            try:
                print("Converting:", p)
                md = convert(p)
                print("->", md)
                converted.append(md)
            except Exception as e:
                print("Error converting", p, e)
        else:
            print("Not found:", p)
    print("\nConverted files:")
    for c in converted:
        print(c)
    sys.exit(0)
