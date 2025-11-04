#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
FTP测试服务器脚本
用于测试FTP文件传输功能

使用方法:
1. 确保已安装 pyftpdlib: pip install pyftpdlib
2. 运行脚本: python ftp_server.py
3. 在FTP客户端中连接: 127.0.0.1:21

支持的登录方式:
- 匿名登录: 用户名和密码留空
- 用户登录: testuser / test123
"""

from pyftpdlib.authorizers import DummyAuthorizer
from pyftpdlib.handlers import FTPHandler
from pyftpdlib.servers import FTPServer
import os
import sys
import threading

# FTP服务器配置（支持多个服务器）
FTP_SERVERS = [
    {
        "host": "127.0.0.1",
        "port": 21,
        "directory": r"D:\testFtp1",
        "name": "FTP服务器1"
    },
    {
        "host": "127.0.0.2",
        "port": 22,
        "directory": r"D:\testFtp2",
        "name": "FTP服务器2"
    }
]

def create_ftp_server(config):
    """创建并配置单个FTP服务器实例"""
    
    host = config["host"]
    port = config["port"]
    directory = config["directory"]
    name = config["name"]
    
    # 检查并创建共享目录
    if not os.path.exists(directory):
        print(f"[{name}] 警告: 目录 {directory} 不存在，正在创建...")
        try:
            os.makedirs(directory, exist_ok=True)
            print(f"[{name}] ✓ 目录创建成功: {directory}")
        except Exception as e:
            print(f"[{name}] ✗ 无法创建目录: {e}")
            return None
    
    if not os.path.isdir(directory):
        print(f"[{name}] 错误: {directory} 不是一个目录")
        return None
    
    # 创建授权器
    authorizer = DummyAuthorizer()
    
    # 添加匿名用户（支持匿名FTP）
    try:
        authorizer.add_anonymous(directory, perm="elr")  # 匿名用户只能列出和下载
        print(f"[{name}] ✓ 匿名FTP已启用")
    except Exception as e:
        print(f"[{name}] 警告: 无法添加匿名用户: {e}")
    
    # 添加认证用户（可选）
    try:
        authorizer.add_user("testuser", "test123", directory, perm="elradfmw")
        print(f"[{name}] ✓ 用户登录已启用: testuser / test123")
    except Exception as e:
        print(f"[{name}] 警告: 无法添加用户: {e}")
    
    # 创建FTP处理器
    handler = FTPHandler
    handler.authorizer = authorizer
    handler.permit_foreign_addresses = True
    handler.max_connections = 256
    handler.max_connections_per_ip = 5
    
    # 为每个服务器设置不同的被动模式端口范围
    if port == 21:
        handler.passive_ports = range(60000, 60050)
    elif port == 22:
        handler.passive_ports = range(60050, 60100)
    else:
        handler.passive_ports = range(60100, 60150)
    
    # 创建FTP服务器
    try:
        server = FTPServer((host, port), handler)
        print(f"[{name}] ✓ 服务器创建成功: {host}:{port}")
        print(f"[{name}]   共享目录: {directory}")
        return server
    except PermissionError:
        print(f"[{name}] ✗ 无法绑定端口 {port}")
        print(f"[{name}]   可能原因: 端口已被占用或需要管理员权限")
        return None
    except Exception as e:
        print(f"[{name}] ✗ 创建服务器失败: {e}")
        return None

def run_server(server, config):
    """在线程中运行FTP服务器"""
    name = config["name"]
    try:
        print(f"[{name}] 服务器启动中...")
        server.serve_forever()
    except Exception as e:
        print(f"[{name}] 服务器运行错误: {e}")

def main():
    """主函数：启动所有FTP服务器"""
    
    print("=" * 60)
    print("FTP多服务器测试系统")
    print("=" * 60)
    print()
    
    servers = []
    threads = []
    
    # 创建所有服务器
    for config in FTP_SERVERS:
        server = create_ftp_server(config)
        if server:
            servers.append((server, config))
    
    if not servers:
        print("错误: 没有成功创建任何服务器")
        sys.exit(1)
    
    print()
    print("=" * 60)
    print(f"成功创建 {len(servers)} 个FTP服务器")
    print("=" * 60)
    print()
    
    # 显示连接信息
    print("连接信息:")
    for i, (server, config) in enumerate(servers, 1):
        print(f"  {i}. {config['name']}")
        print(f"     - 地址: {config['host']}:{config['port']}")
        print(f"     - 目录: {config['directory']}")
        print(f"     - 匿名登录: 用户名和密码留空")
        print(f"     - 用户登录: testuser / test123")
        print()
    
    print("=" * 60)
    print("按 Ctrl+C 停止所有服务器")
    print("=" * 60)
    print()
    
    # 在独立线程中启动每个服务器
    for server, config in servers:
        thread = threading.Thread(target=run_server, args=(server, config), daemon=True)
        thread.start()
        threads.append(thread)
    
    # 主线程等待
    try:
        for thread in threads:
            thread.join()
    except KeyboardInterrupt:
        print("\n")
        print("=" * 60)
        print("正在停止所有服务器...")
        for server, config in servers:
            server.close_all()
        print("所有服务器已停止")
        print("=" * 60)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n")
        print("=" * 60)
        print("服务器已停止")
        print("=" * 60)
        sys.exit(0)

