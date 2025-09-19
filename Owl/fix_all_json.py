#!/usr/bin/env python3
"""
批量修复JsonRPC文档中所有未格式化的JSON块
"""
import re
from pathlib import Path

def fix_all_json_blocks(file_path):
    """修复文档中所有的JSON格式问题"""
    
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 找到所有未在代码块中的JSON块
    # 模式：以{开始，以}结束，但不在```json代码块中
    lines = content.split('\n')
    result_lines = []
    i = 0
    in_code_block = False
    
    while i < len(lines):
        line = lines[i]
        
        # 检查是否进入或退出代码块
        if line.strip().startswith('```'):
            in_code_block = not in_code_block
            result_lines.append(line)
            i += 1
            continue
            
        # 如果已经在代码块中，直接添加
        if in_code_block:
            result_lines.append(line)
            i += 1
            continue
            
        # 检查是否是未格式化的JSON块开始
        if line.strip() == '{' and i > 0:
            # 查找JSON块结束位置
            json_start = i
            brace_count = 1
            json_lines = [line.strip()]
            j = i + 1
            
            while j < len(lines) and brace_count > 0:
                current_line = lines[j].strip()
                if current_line:
                    json_lines.append(current_line)
                    # 计算大括号数量
                    brace_count += current_line.count('{') - current_line.count('}')
                j += 1
            
            if brace_count == 0:
                # 找到完整的JSON块，格式化它
                formatted_json = format_json_content(json_lines)
                result_lines.append('```json')
                result_lines.extend(formatted_json)
                result_lines.append('```')
                i = j
            else:
                # 不是完整的JSON块，保持原样
                result_lines.append(line)
                i += 1
        else:
            result_lines.append(line)
            i += 1
    
    return '\n'.join(result_lines)

def format_json_content(json_lines):
    """格式化JSON内容"""
    formatted = []
    indent_level = 0
    
    for line in json_lines:
        line = line.strip()
        if not line:
            continue
            
        # 移除多余的空格和格式化
        line = re.sub(r'\s*:\s*', ': ', line)
        line = re.sub(r'\*\*(\w+)\*\*', r'\1', line)  # 移除粗体标记
        
        # 处理缩进
        if line in ['}', '},', ']', '],']: 
            indent_level = max(0, indent_level - 1)
            
        formatted.append('  ' * indent_level + line)
        
        if line.endswith(('{', '[')):
            indent_level += 1
    
    return formatted

def main():
    file_path = Path("docs/api/JsonRPC_API.md")
    
    print("正在批量修复JSON格式...")
    
    try:
        formatted_content = fix_all_json_blocks(file_path)
        
        # 写回文件
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(formatted_content)
            
        print("[OK] JSON格式修复完成!")
        
    except Exception as e:
        print(f"[ERROR] 修复失败: {e}")

if __name__ == "__main__":
    main()