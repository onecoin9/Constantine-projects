# -*- coding: utf-8 -*-
"""
简易温控板协议模拟器（TCP 服务器）- 支持多温区
author: you
date  : 2025-05-xx
"""

import argparse
import random
import socket
import struct
import threading
import time
import json

FLAG_BYTES = b'\x57\x44\x4B\x5A'          # "WDKZ"
HEAD_FMT   = '<4sHH'                      # Flag, CmdID, Size
HEAD_LEN   = 8                            # 4+2+2

# 温区配置
ZONE_CONFIGS = {
    1: {"name": "温区1温度控制", "description": "门压温区1的温度控制", "port": 6001, "channels": 9},
    2: {"name": "温区2温度控制", "description": "门压温区2的温度控制", "port": 6002, "channels": 9},
    3: {"name": "温区3温度控制", "description": "门压温区3的温度控制", "port": 6003, "channels": 9},
    4: {"name": "温区4温度控制", "description": "门压温区4的温度控制", "port": 6004, "channels": 9}
}

# 每个温区的状态数据（用于模拟持久化状态）
zone_states = {
    zone_id: {
        "temperatures": [25.0] * config["channels"],  # 当前温度
        "target_temps": [25.0] * config["channels"],  # 目标温度
        "fan_speeds": [0] * config["channels"],       # 风扇转速
        "pid_params": {"kp": 100, "ki": 50, "kd": 25}, # PID参数
        "max_duty": 800,                              # 最大占空比
        "enabled": True,                              # 启停状态
        "site_pressure": [100.0] * 16,               # SITE压力数据 (16个站点)
        "pogo_pin_high": [0] * 8,                    # Pogo pin高电平状态 (8个)
        "pogo_pin_low": [0] * 8,                     # Pogo pin低电平状态 (8个)
        "power_status": 0                            # 上下电状态 (01上电, 00下电, 02进行中)
    } for zone_id, config in ZONE_CONFIGS.items()
}

CMD_DESCRIPTIONS = {
    0x01: "查询温度",
    0x02: "查询转速",
    0x03: "设置温度",
    0x04: "设置转速",
    0x05: "启停控制",
    0x06: "设置PID参数",
    0x07: "查询故障",
    0x08: "查询电压电流",
    0x09: "控制5V开关",
    0x0A: "启停并设置温度和风扇指令",
    0x0B: "设置16站目标温度",
    0x0C: "查询16站当前温度",
    0x0D: "查询SITE压力",
    0x10: "设置最大占空比",
    0x11: "查询PID参数",
    0x12: "查询最大占空比",
    0x13: "POGO PIN检查",
    0x14: "上下电状态检查",
    0x1FFF: "系统复位",
    0x3FFF: "升级数据块"
}

def crc8(data: bytes) -> int:
    """协议的简单累加 CRC8（低 8 位）"""
    return sum(data) & 0xFF

def build_pkt(cmd_id: int, cmd_data: bytes) -> bytes:
    head = FLAG_BYTES + struct.pack('<HH', cmd_id, len(cmd_data))
    body = head + cmd_data
    body += struct.pack('B', crc8(body))
    return body

def parse_temperature_data(temp_data: bytes) -> list:
    """解析温度数据，返回摄氏度列表"""
    temperatures = []
    for i in range(0, len(temp_data), 2):
        if i + 1 < len(temp_data):
            raw_value = struct.unpack('<H', temp_data[i:i+2])[0]
            celsius = (raw_value - 400) / 10.0  # 协议公式: 温度 = (原始值 - 400) / 10
            temperatures.append(celsius)
    return temperatures

def rand_temp_bytes() -> bytes:
    """生成 32 字节 16 站温度数据，小端 uint16"""
    arr = bytearray()
    temps = []
    for i in range(16):
        c = random.uniform(20.0, 85.0)      # 随机 20~85℃
        raw = int(c * 10 + 400)             # 按协议公式: 原始值 = 温度*10 + 400
        temps.append(c)
        arr += struct.pack('<H', raw)
    
    # 显示生成的温度值
    print(f"  生成的16站温度: {[f'{t:.1f}℃' for t in temps]}")
    return bytes(arr)

def rand_speed_bytes() -> bytes:
    """生成 32 字节 16 风扇转速，小端 uint16 (mrp)"""
    arr = bytearray()
    for _ in range(16):
        rpm = random.randint(0, 6000)
        arr += struct.pack('<H', rpm)
    return bytes(arr)

def rand_pid_bytes() -> bytes:
    """生成 6 字节 KP,KI,KD (uint16, *100)"""
    kp = random.randint(10, 500)
    ki = random.randint(1, 100)
    kd = random.randint(1, 100)
    return struct.pack('<HHH', kp, ki, kd)

def rand_duty_bytes() -> bytes:
    """生成占空比 0-1000 (uint16)"""
    duty = random.randint(0, 1000)
    return struct.pack('<H', duty)

def rand_pressure_bytes() -> bytes:
    """生成16个站点的压力数据 (32字节)"""
    arr = bytearray()
    for _ in range(16):
        pressure = random.uniform(50.0, 200.0)  # 随机压力50-200
        raw = int(pressure * 10)  # 精度0.1
        arr += struct.pack('<H', raw)
    return bytes(arr)

def rand_pogo_pin_bytes() -> bytes:
    """生成Pogo pin状态数据 (高电平16字节 + 低电平16字节)"""
    arr = bytearray()
    # 高电平状态 (8个uint16)
    for _ in range(8):
        val = random.randint(0, 1)  # 0或1
        arr += struct.pack('<H', val)
    # 低电平状态 (8个uint16)  
    for _ in range(8):
        val = random.randint(0, 1)  # 0或1
        arr += struct.pack('<H', val)
    return bytes(arr)

def process(cmd_id: int, cmd_data: bytes, zone_id: int = 1) -> bytes:
    """处理命令，支持多温区"""
    zone_state = zone_states.get(zone_id, zone_states[1])
    
    if cmd_id == 0x01:               # 查询温度
        data = b'\x01' + rand_temp_bytes()
        return build_pkt(cmd_id, data)

    elif cmd_id == 0x02:             # 查询转速
        data = b'\x01' + rand_speed_bytes()
        return build_pkt(cmd_id, data)

    elif cmd_id in (0x03, 0x04, 0x05):  # 设温 / 设转速 / 启停
        if cmd_id == 0x03 and len(cmd_data) >= 2:  # 设置温度
            temp_raw = struct.unpack('<H', cmd_data[:2])[0]
            temp_celsius = (temp_raw - 400) / 10.0  # 协议公式转换
            # 更新第一个通道的目标温度
            if len(zone_state["target_temps"]) > 0:
                zone_state["target_temps"][0] = temp_celsius
            print(f"[Zone {zone_id}] 收到设置温度指令: {temp_celsius:.1f}℃ (原始值: {temp_raw})")
        elif cmd_id == 0x04 and len(cmd_data) >= 2:  # 设置转速
            speed_raw = struct.unpack('<H', cmd_data[:2])[0]
            if len(zone_state["fan_speeds"]) > 0:
                zone_state["fan_speeds"][0] = speed_raw
            print(f"[Zone {zone_id}] 收到设置转速指令: {speed_raw} RPM")
        elif cmd_id == 0x05 and len(cmd_data) >= 1:  # 启停控制
            enable_state = cmd_data[0]
            zone_state["enabled"] = bool(enable_state)
            print(f"[Zone {zone_id}] 收到启停控制指令: {'启动' if enable_state else '停止'}")
        return build_pkt(cmd_id, b'\x01')   # 1=成功

    elif cmd_id == 0x06:             # 设置 PID
        if len(cmd_data) >= 6:
            # 解析PID参数并更新状态
            kp, ki, kd = struct.unpack('<HHH', cmd_data[:6])
            zone_state["pid_params"] = {"kp": kp, "ki": ki, "kd": kd}
        return build_pkt(cmd_id, b'\x01')

    elif cmd_id == 0x09:             # 控制 5V 打开/关闭
        return build_pkt(cmd_id, b'\x01')

    elif cmd_id == 0x0B:             # 设置 16 站目标温度
        if len(cmd_data) >= 32:  # 16站 * 2字节
            temps = []
            for i in range(16):
                temp_raw = struct.unpack('<H', cmd_data[i*2:(i+1)*2])[0]
                temp_celsius = (temp_raw - 400) / 10.0  # 协议公式转换
                temps.append(temp_celsius)
            # 更新前9个通道的目标温度（对应当前温区）
            for i in range(min(9, len(temps))):
                if i < len(zone_state["target_temps"]):
                    zone_state["target_temps"][i] = temps[i]
            # 显示设置的温度值
            temp_str = ", ".join([f"站点{i+1}:{t:.1f}℃" for i, t in enumerate(temps)])
            print(f"[Zone {zone_id}] 设置16站目标温度: {temp_str}")
        return build_pkt(cmd_id, b'\x01')

    elif cmd_id == 0x0C:             # 查询 16 站当前温度
        data = b'\x01' + rand_temp_bytes()   # status + 32B
        return build_pkt(cmd_id, data)

    elif cmd_id == 0x10:             # 设置最大占空比 duty
        if len(cmd_data) >= 2:
            duty = struct.unpack('<H', cmd_data[:2])[0]
            zone_state["max_duty"] = duty
        return build_pkt(cmd_id, b'\x01')

    elif cmd_id == 0x11:             # 查询 PID
        pid_params = zone_state["pid_params"]
        pid_data = struct.pack('<HHH', pid_params["kp"], pid_params["ki"], pid_params["kd"])
        data = b'\x01' + pid_data    # 1 + 6
        return build_pkt(cmd_id, data)

    elif cmd_id == 0x0A:             # 启停并设置温度和风扇指令
        if len(cmd_data) >= 4:  # State(1) + WindSpeedData(1) + TempData(2)
            state = cmd_data[0]  # Bit7:加热, Bit0:开启/停止
            wind_speed = cmd_data[1]  # 风扇转速 0-100
            temp_raw = struct.unpack('<H', cmd_data[2:4])[0]  # 温度数据
            temp_celsius = (temp_raw - 2000) / 100.0  # 温度转换公式
            
            # 更新状态
            zone_state["enabled"] = bool(state & 0x01)  # Bit0: 启停
            if len(zone_state["target_temps"]) > 0:
                zone_state["target_temps"][0] = temp_celsius  # 设置第一个通道目标温度
            if len(zone_state["fan_speeds"]) > 0:
                zone_state["fan_speeds"][0] = wind_speed  # 设置第一个通道风扇转速
        return build_pkt(cmd_id, b'\x01')  # 成功

    elif cmd_id == 0x0D:             # 查询SITE压力
        # 返回16个站点的压力数据 (StatusCode + 32字节压力数据)
        status = b'\x01'  # 成功
        pressure_data = bytearray()
        for pressure in zone_state["site_pressure"]:
            # 压力值转换为 uint16，单位可能是克或其他
            pressure_raw = int(pressure * 10)  # 假设精度为0.1
            pressure_data += struct.pack('<H', pressure_raw)
        data = status + bytes(pressure_data)  # 1 + 32 = 33字节
        return build_pkt(cmd_id, data)

    elif cmd_id == 0x13:             # POGO PIN检查
        # 返回状态码 + Pogo pin高低电平状态
        status = b'\x01'  # 成功
        # Pogo pin高电平参数 (8个uint16，共16字节)
        pogo_high_data = bytearray()
        for i in range(8):
            pogo_val = zone_state["pogo_pin_high"][i]
            pogo_high_data += struct.pack('<H', pogo_val)
        
        # Pogo pin低电平参数 (8个uint16，共16字节)  
        pogo_low_data = bytearray()
        for i in range(8):
            pogo_val = zone_state["pogo_pin_low"][i]
            pogo_low_data += struct.pack('<H', pogo_val)
        
        data = status + bytes(pogo_high_data) + bytes(pogo_low_data)  # 1 + 16 + 16 = 33字节
        return build_pkt(cmd_id, data)

    elif cmd_id == 0x14:             # 上下电状态检查
        # 返回上下电状态 (StatusCode: 01上电, 00下电, 02上电或下电进行中)
        status_code = zone_state["power_status"]
        data = struct.pack('B', status_code)  # 1字节状态码
        return build_pkt(cmd_id, data)

    elif cmd_id == 0x12:             # 查询最大占空比
        duty_data = struct.pack('<H', zone_state["max_duty"])
        data = b'\x01' + duty_data   # 1 + 2
        return build_pkt(cmd_id, data)

    elif cmd_id == 0x1FFF:           # 系统复位
        # 重置当前温区状态
        zone_state["temperatures"] = [25.0] * ZONE_CONFIGS[zone_id]["channels"]
        zone_state["target_temps"] = [25.0] * ZONE_CONFIGS[zone_id]["channels"]
        zone_state["fan_speeds"] = [0] * ZONE_CONFIGS[zone_id]["channels"]
        zone_state["enabled"] = True
        zone_state["site_pressure"] = [100.0] * 16
        zone_state["pogo_pin_high"] = [0] * 8
        zone_state["pogo_pin_low"] = [0] * 8
        zone_state["power_status"] = 1  # 默认上电状态
        return build_pkt(cmd_id, b'\x01')

    elif cmd_id == 0x3FFF:           # 升级数据块 (不实际处理)
        return build_pkt(cmd_id, b'\x01')

    elif cmd_id == 0x07:             # 查询故障
        dummy = bytes([0]*60)        # 1 + 60 = 61 字节
        return build_pkt(cmd_id, b'\x01' + dummy)

    elif cmd_id == 0x08:             # 查询电压电流
        dummy = bytes([0]*25)        # 1 + 25 = 26
        return build_pkt(cmd_id, b'\x01' + dummy)

    else:                            # 未知命令
        return build_pkt(cmd_id, b'\x00')   # 0=失败

def recv_exact(conn, length):
    """确保接收到指定长度的数据"""
    data = b''
    while len(data) < length:
        chunk = conn.recv(length - len(data))
        if not chunk:
            raise ConnectionError("Connection closed unexpectedly")
        data += chunk
    return data

def handle_client(conn, addr, zone_id: int = 1):
    zone_config = ZONE_CONFIGS.get(zone_id, ZONE_CONFIGS[1])
    print(f'client {addr} connected to {zone_config["name"]} (Zone {zone_id})')
    try:
        while True:
            # 确保接收完整的协议头
            head = recv_exact(conn, HEAD_LEN)
            if len(head) < HEAD_LEN:
                break

            flag, cmd_id, size = struct.unpack(HEAD_FMT, head)
            if flag != FLAG_BYTES:
                print(f'[Zone {zone_id}] bad flag: {flag.hex()}, expected: {FLAG_BYTES.hex()}')
                break

            # 获取命令描述
            cmd_desc = CMD_DESCRIPTIONS.get(cmd_id, "未知命令")
            
            # 打印收到的命令信息及功能
            print(f"[Zone {zone_id}] 收到命令: 0x{cmd_id:04X} ({cmd_desc}), 数据长度: {size}字节")

            # 接收payload（如果有的话）
            payload = b''
            if size > 0:
                payload = recv_exact(conn, size)
            
            # 接收CRC
            crc_recv = recv_exact(conn, 1)
            
            # 验证CRC
            frame = head + payload
            calculated_crc = crc8(frame)
            received_crc = crc_recv[0]
            
            if calculated_crc != received_crc:
                print(f'[Zone {zone_id}] CRC mismatch: calculated={calculated_crc:02X}, received={received_crc:02X}')
                break

            # 打印收到的完整数据
            full_packet = frame + crc_recv
            print(f"[Zone {zone_id}] 收到数据: {full_packet.hex()}")

            # 处理命令并生成响应
            rsp = process(cmd_id, payload, zone_id)
            
            # 打印回复的数据与命令描述
            print(f"[Zone {zone_id}] 回复数据 ({cmd_desc}响应): {rsp.hex()}")
            
            # 如果是查询温度命令，解析并显示温度值
            if cmd_id == 0x01 and len(rsp) >= 42:  # 协议头8字节 + 状态1字节 + 温度32字节 + CRC1字节
                temp_data = rsp[9:41]  # 提取温度数据部分 (16站 * 2字节 = 32字节)
                temperatures = parse_temperature_data(temp_data)
                temp_str = ", ".join([f"站点{i+1}:{t:.1f}℃" for i, t in enumerate(temperatures)])
                print(f"[Zone {zone_id}] 温度解析: {temp_str}")
            
            # 发送响应
            conn.sendall(rsp)
            
    except ConnectionError as e:
        print(f'[Zone {zone_id}] Connection error: {e}')
    except Exception as e:
        print(f'[Zone {zone_id}] error:', e)
    finally:
        conn.close()
        print(f'[Zone {zone_id}] client {addr} closed')

def start_zone_server(zone_id: int):
    """启动单个温区的TCP服务器"""
    zone_config = ZONE_CONFIGS[zone_id]
    port = zone_config["port"]
    
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind(('127.0.0.1', port))
        s.listen()
        print(f'{zone_config["name"]} mock server listening on 127.0.0.1:{port}')
        
        while True:
            try:
                c, a = s.accept()
                threading.Thread(target=handle_client, args=(c, a, zone_id), daemon=True).start()
            except Exception as e:
                print(f'[Zone {zone_id}] Accept error:', e)

def main(port: int = None):
    """主函数：启动所有温区服务器或单个服务器"""
    if port is not None:
        # 兼容模式：单端口服务器
        print("Running in single-port compatibility mode")
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.bind(('127.0.0.1', port))
            s.listen()
            print(f'Temp-Ctrl mock server listening on 127.0.0.1:{port}')
            while True:
                c, a = s.accept()
                threading.Thread(target=handle_client, args=(c, a, 1), daemon=True).start()
    else:
        # 多温区模式：为每个温区启动独立的服务器
        print("Starting multi-zone temperature control servers...")
        print("Zone configurations:")
        for zone_id, config in ZONE_CONFIGS.items():
            print(f"  Zone {zone_id}: {config['name']} on port {config['port']}")
        
        # 为每个温区创建独立的服务器线程
        server_threads = []
        for zone_id in ZONE_CONFIGS.keys():
            thread = threading.Thread(target=start_zone_server, args=(zone_id,), daemon=True)
            thread.start()
            server_threads.append(thread)
        
        print("\nAll zone servers started. Press Ctrl+C to stop.")
        try:
            # 主线程等待，保持程序运行
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            print("\nShutting down all servers...")
            return

if __name__ == '__main__':
    ap = argparse.ArgumentParser(description='Temperature Controller Mock Server')
    ap.add_argument('-p', '--port', type=int, default=None, help='listen port (default: multi-zone mode)')
    args = ap.parse_args()
    
    if args.port:
        main(args.port)
    else:
        main()