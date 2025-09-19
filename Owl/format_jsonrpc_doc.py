#!/usr/bin/env python3
"""
JsonRPC API 文档格式化脚本
自动修复JSON代码块和标题层级
"""
import re
from pathlib import Path

def format_json_rpc_doc(file_path):
    """格式化JsonRPC API文档"""
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 修复标题层级 - API方法应该是### (h3)
    api_methods = [
        'LoadProject', 'GetProjectInfo', 'GetProjectInfoExt', 'GetAllSitesAdpEn',
        'SetAdapterEn', 'DoCustom格式', 'DoCustom \\(CmdID == 1078\\)', 'DoJob',
        'DoJob \\(接触检查\\)', 'SetMissionResult'
    ]
    
    for method in api_methods:
        content = re.sub(f'^## ({method})', r'### \1', content, flags=re.MULTILINE)
    
    # 修复子标题层级
    content = re.sub(r'^### (请求|应答|执行结果)', r'#### \1', content, flags=re.MULTILINE)
    
    # 修复版本记录标题
    content = re.sub(r'^# 版本记录', '## 版本记录', content, flags=re.MULTILINE)
    
    # 修复JSON代码块 - 匹配多行JSON结构
    def format_json_block(match):
        json_text = match.group(1)
        # 清理多余的换行和空格
        lines = [line.strip() for line in json_text.split('\n') if line.strip()]
        
        # 重建JSON结构
        formatted_lines = []
        indent_level = 0
        
        for line in lines:
            # 处理缩进
            if line.endswith(('{', '[')):
                formatted_lines.append('  ' * indent_level + line)
                indent_level += 1
            elif line.startswith(('}', ']')):
                indent_level = max(0, indent_level - 1)
                formatted_lines.append('  ' * indent_level + line)
            else:
                formatted_lines.append('  ' * indent_level + line)
        
        return '```json\n' + '\n'.join(formatted_lines) + '\n```'
    
    # 匹配不规范的JSON块
    json_pattern = r'(?:^{[\s\S]*?^}$|^{\s*$[\s\S]*?^}\s*$)'
    matches = list(re.finditer(json_pattern, content, flags=re.MULTILINE))
    
    # 从后向前替换，避免位置偏移
    for match in reversed(matches):
        start, end = match.span()
        json_content = match.group(0)
        
        # 跳过已经在代码块中的JSON
        before_match = content[:start]
        if before_match.count('```json') > before_match.count('```\n```'):
            continue
            
        # 清理和格式化JSON
        lines = []
        indent = 0
        for line in json_content.split('\n'):
            line = line.strip()
            if not line:
                continue
                
            # 调整缩进
            if line in ['}', '},']: 
                indent = max(0, indent - 1)
            if line in [']', '],']: 
                indent = max(0, indent - 1)
                
            lines.append('  ' * indent + line)
            
            if line.endswith(('{', '[')):
                indent += 1
        
        formatted_json = '```json\n' + '\n'.join(lines) + '\n```'
        content = content[:start] + formatted_json + content[end:]
    
    # 修复表格格式
    content = re.sub(r'^([^|\n]+)\|([^|\n]+)$\n^---\|---$', 
                     r'| \1 | \2 |\n|---|---|', 
                     content, flags=re.MULTILINE)
    
    # 修复特殊格式问题
    content = re.sub(r'<APS\.exe目录>', r'`<APS.exe目录>`', content)
    content = re.sub(r'<端口号>', r'`<端口号>`', content)
    
    # 修复粗体标记中的代码
    content = re.sub(r'\*\*"([^"]+)"\*\*', r'**"\1"**', content)
    
    return content

def main():
    file_path = Path("docs/api/JsonRPC_API.md")
    
    print(f"正在格式化文档: {file_path}")
    
    try:
        formatted_content = format_json_rpc_doc(file_path)
        
        # 写回文件
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(formatted_content)
            
        print("✓ 文档格式化完成!")
        
    except Exception as e:
        print(f"✗ 格式化失败: {e}")

if __name__ == "__main__":
    main()