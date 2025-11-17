#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
TCP 客户端 - 与远程设备通信
"""

import socket
import time

def send_command(ip, port, command, wait_response=False, timeout=5):
    """
    发送 TCP 命令并接收返回数据
    
    Args:
        ip: 目标设备 IP
        port: 目标端口号
        command: 要发送的命令字符串
        wait_response: 是否等待返回数据（True 等待，False 不等待）
        timeout: 超时时间（秒）
    
    Returns:
        返回的数据字符串，如果失败返回 None
    """
    try:
        # 创建 TCP socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(timeout)
        
        print(f"[*] 正在连接到 {ip}:{port}...")
        sock.connect((ip, port))
        print(f"[✓] 连接成功")
        
        # 发送命令
        print(f"[*] 发送命令: {command}")
        print(f"[*] 命令字节: {command.encode('utf-8')}")
        bytes_sent = sock.sendall(command.encode('utf-8'))
        print(f"[✓] 命令已发送")
        
        # 根据 wait_response 决定是否接收返回数据
        if wait_response:
            print(f"[*] 等待返回数据 (超时: {timeout}秒)...")
            response = b''
            bytes_received = 0
            
            sock.setblocking(False)  # 改为非阻塞模式
            import select
            
            end_time = time.time() + timeout
            while time.time() < end_time:
                try:
                    # 使用 select 检查是否有数据
                    ready = select.select([sock], [], [], 0.5)
                    if ready[0]:
                        chunk = sock.recv(4096)
                        if chunk:
                            response += chunk
                            bytes_received += len(chunk)
                            print(f"[*] 接收到 {len(chunk)} 字节数据")
                        else:
                            print(f"[*] 连接被对端关闭")
                            break
                except BlockingIOError:
                    continue
                except Exception as e:
                    print(f"[!] 接收过程异常: {e}")
                    break
            
            if response:
                response_str = response.decode('utf-8', errors='ignore')
                print(f"[✓] 总共接收到 {bytes_received} 字节")
                print(f"[✓] 收到返回数据: {repr(response_str)}")
            else:
                print(f"[!] 没有接收到任何数据")
                response_str = ""
        else:
            print(f"[*] 不等待返回数据")
            response_str = None
        
        sock.close()
        print(f"[✓] 连接已关闭\n")
        
        return response_str
        
    except socket.timeout:
        print(f"[✗] 连接超时")
        return None
    except ConnectionRefusedError:
        print(f"[✗] 连接被拒绝，请检查 IP 和端口号是否正确")
        return None
    except Exception as e:
        print(f"[✗] 错误: {e}")
        import traceback
        traceback.print_exc()
        return None


def show_menu():
    """显示命令菜单"""
    print("\n" + "=" * 60)
    print("TCP 客户端 - 设备通信工具")
    print("=" * 60)
    print("\n请选择要发送的指令:")
    print("  1 - OUTP 1")
    print("  2 - SOUR (自定义数字)")
    print("  3 - SENS?")
    print("  4 - OUTP 0")
    print("  5 - SENS:PRES:RANG\"71.00bara\"")
    print("  6 - SOUR:PRES:RANG\"71.00bara\"")
    print("  0 - 退出程序")
    print("=" * 60)


def get_command(choice):
    """根据选择返回对应的命令和是否需要回复"""
    commands = {
        '1': ('OUTP 1', False),
        '2': None,  # 需要特殊处理
        '3': ('SENS?', True),
        '4': ('OUTP 0', False),
        '5': ('SENS:PRES:RANG"71.00bara"', True),
        '6': ('SOUR:PRES:RANG"71.00bara"', False)
    }
    return commands.get(choice)


if __name__ == "__main__":
    # 配置参数
    TARGET_IP = "192.16.11.125"
    TARGET_PORT = 8000
    
    print("\n" + "=" * 60)
    print("设备通信工具 - 交互式菜单")
    print("=" * 60)
    print(f"目标 IP: {TARGET_IP}")
    print(f"目标端口: {TARGET_PORT}")
    print("=" * 60)
    
    try:
        while True:
            show_menu()
            
            user_input = input("\n请输入选项 (0-6): ").strip()
            
            if user_input == '0':
                print("[*] 退出程序")
                break
            
            if user_input not in ['1', '2', '3', '4', '5', '6']:
                print("[!] 无效的选项，请重新输入")
                continue
            
            # 特殊处理 SOUR 命令
            if user_input == '2':
                try:
                    custom_value = input("请输入 SOUR 命令的数字: ").strip()
                    if not custom_value:
                        print("[!] 输入不能为空，请重新输入")
                        continue
                    command = f"SOUR {custom_value}"
                    wait_response = False
                except ValueError:
                    print("[!] 输入无效，请重新输入")
                    continue
            else:
                result = get_command(user_input)
                if result is None:
                    print("[!] 无法获取命令")
                    continue
                command, wait_response = result
            
            print(f"\n[*] 选择的命令: {command}")
            
            # 发送命令
            response = send_command(TARGET_IP, TARGET_PORT, command, wait_response=wait_response)
            
            if wait_response and response:
                print(f"\n最终返回数据:")
                print(f"类型: {type(response)}")
                print(f"内容: {repr(response)}")
            elif wait_response and not response:
                print("\n[!] 未收到返回数据")
            else:
                print("\n[✓] 命令已发送")
            
            print("\n" + "-" * 60)
            input("按 Enter 键继续...")
    
    except KeyboardInterrupt:
        print("\n\n[*] 程序被中断")
    except Exception as e:
        print(f"\n[✗] 程序出现异常: {e}")
        import traceback
        traceback.print_exc()
    
    finally:
        print("\n[✓] 程序已关闭")
        input("按 Enter 键退出...")
