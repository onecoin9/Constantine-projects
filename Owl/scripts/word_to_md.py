#!/usr/bin/env python3
"""
Word文档转Markdown工具
专为JSON-RPC接口文档设计，支持批量转换和图片提取
"""
import os
import sys
import argparse
from pathlib import Path

try:
    import mammoth
    import html2text
except ImportError:
    print("缺少依赖库，请运行: pip install python-mammoth html2text")
    sys.exit(1)


class WordToMarkdownConverter:
    def __init__(self, output_dir="converted", images_dir="images"):
        self.output_dir = Path(output_dir)
        self.images_dir = images_dir
        self.h = html2text.HTML2Text()
        self.h.ignore_links = False
        self.h.ignore_emphasis = False
        
    def ensure_dirs(self, output_path):
        """确保输出目录存在"""
        output_path.parent.mkdir(parents=True, exist_ok=True)
        images_path = output_path.parent / self.images_dir
        images_path.mkdir(parents=True, exist_ok=True)
        return images_path
        
    def save_image(self, images_path, image_bytes, ext, index):
        """保存图片并返回相对路径"""
        filename = f"image_{index:03d}.{ext}"
        image_path = images_path / filename
        
        with open(image_path, "wb") as f:
            f.write(image_bytes)
        
        return f"{self.images_dir}/{filename}"
        
    def convert_single(self, docx_path, output_path=None):
        """转换单个Word文档"""
        docx_path = Path(docx_path)
        
        if not docx_path.exists():
            raise FileNotFoundError(f"文件不存在: {docx_path}")
            
        if output_path is None:
            output_path = self.output_dir / f"{docx_path.stem}.md"
        else:
            output_path = Path(output_path)
            
        images_path = self.ensure_dirs(output_path)
        
        print(f"转换: {docx_path} -> {output_path}")
        
        with open(docx_path, "rb") as f:
            image_index = 1
            
            def handle_image(image):
                nonlocal image_index
                try:
                    content_type = image.content_type
                    ext = content_type.split('/')[-1] if '/' in content_type else 'png'
                    
                    # 使用with_content_type方法获取图片数据
                    if hasattr(image, 'open'):
                        with image.open() as img_data:
                            image_bytes = img_data.read()
                    else:
                        # 备用方法
                        image_bytes = getattr(image, '_content', None)
                    
                    if image_bytes:
                        rel_path = self.save_image(images_path, image_bytes, ext, image_index)
                        image_index += 1
                        return {"src": rel_path}
                    else:
                        return {"alt": image.alt_text or "image"}
                except Exception as e:
                    return {"alt": image.alt_text or "image"}
                    
            # 使用mammoth转换
            result = mammoth.convert_to_html(
                f, 
                convert_image=mammoth.images.inline(handle_image)
            )
            
            if result.messages:
                print("转换警告:")
                for msg in result.messages:
                    print(f"  - {msg}")
                    
            # 转换HTML到Markdown
            markdown = self.h.handle(result.value)
            
        # 写入Markdown文件
        with open(output_path, "w", encoding="utf-8") as f:
            f.write(markdown)
            
        return output_path
        
    def convert_batch(self, input_patterns, output_dir=None):
        """批量转换"""
        if output_dir:
            self.output_dir = Path(output_dir)
            
        converted_files = []
        
        for pattern in input_patterns:
            # 支持通配符
            if '*' in pattern or '?' in pattern:
                import glob
                files = glob.glob(pattern)
            else:
                files = [pattern]
                
            for file_path in files:
                try:
                    output_path = self.convert_single(file_path)
                    converted_files.append(output_path)
                    print(f"[OK] 转换成功: {output_path}")
                except Exception as e:
                    print(f"[ERROR] 转换失败 {file_path}: {e}")
                    
        return converted_files


def main():
    parser = argparse.ArgumentParser(description="Word文档转Markdown工具")
    parser.add_argument("files", nargs="+", help="Word文档路径（支持通配符）")
    parser.add_argument("-o", "--output", help="输出目录", default="converted")
    parser.add_argument("--images-dir", help="图片子目录名", default="images")
    
    args = parser.parse_args()
    
    converter = WordToMarkdownConverter(
        output_dir=args.output,
        images_dir=args.images_dir
    )
    
    print("开始转换Word文档...")
    converted = converter.convert_batch(args.files, args.output)
    
    print(f"\n转换完成！共转换 {len(converted)} 个文件:")
    for file in converted:
        print(f"  - {file}")


if __name__ == "__main__":
    # 如果没有命令行参数，转换JSON-RPC文档
    if len(sys.argv) == 1:
        converter = WordToMarkdownConverter()
        
        # 查找JSON-RPC文档
        owl_dir = Path(__file__).resolve().parents[1]
        jsonrpc_doc = owl_dir / "docs" / "PLT_RD_COG-APS软件JsonRPC接口文档.docx"
        
        if jsonrpc_doc.exists():
            try:
                output_md = owl_dir / "docs" / "api" / "JsonRPC_API.md"
                output = converter.convert_single(jsonrpc_doc, output_md)
                print(f"[OK] JSON-RPC文档转换完成: {output}")
            except Exception as e:
                print(f"[ERROR] 转换失败: {e}")
        else:
            print("未找到JSON-RPC文档，请指定文件路径")
            print("用法: python word_to_md.py <文件路径>")
    else:
        main()