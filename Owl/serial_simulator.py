#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
串口模拟器工具
模拟串口2和串口4的通信功能

功能：
1. 收到 <<start>> 后回复 <<initialok>>
2. 收到 <<next>> 后回复 <<ERRCODE:00,00,01,00,00,00,00,00>>
"""

import serial
import threading
import time
import sys
import logging

# 配置日志
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

class SerialSimulator:
    def __init__(self, port, baudrate=9600, timeout=1, response_delay=0.1, response_port=None):
        """
        初始化串口模拟器
        
        Args:
            port (str): 串口号，如 'COM2', 'COM4'
            baudrate (int): 波特率，默认9600
            timeout (float): 超时时间，默认1秒
            response_delay (float): 响应延时，默认0.1秒
            response_port (str): 响应发送端口，如果不指定则发送到同一端口
        """
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.response_delay = response_delay
        self.response_port = response_port or port  # 如果没指定响应端口，使用同一端口
        self.serial_conn = None
        self.response_conn = None
        self.running = False
        self.thread = None
        
        # 定义命令和响应映射（支持大小写）
        self.command_responses = {
            b'<<start>>': b'<<INITIALOK>>',
            b'<<START>>': b'<<INITIALOK>>',
            b'<<next>>': b'<<ERRCODE:00,00,01,00,00,00,00,00>>',
            b'<<NEXT>>': b'<<ERRCODE:00,00,01,00,00,00,00,00>>'
        }
    
    def connect(self):
        """连接串口"""
        try:
            # 连接监听端口
            self.serial_conn = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=self.timeout,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE
            )
            logger.info(f"成功连接到监听端口 {self.port}")
            
            # 如果响应端口不同，连接响应端口
            if self.response_port != self.port:
                self.response_conn = serial.Serial(
                    port=self.response_port,
                    baudrate=self.baudrate,
                    timeout=self.timeout,
                    bytesize=serial.EIGHTBITS,
                    parity=serial.PARITY_NONE,
                    stopbits=serial.STOPBITS_ONE
                )
                logger.info(f"成功连接到响应端口 {self.response_port}")
            else:
                self.response_conn = self.serial_conn  # 使用同一端口
                
            return True
        except serial.SerialException as e:
            logger.error(f"连接串口失败: {e}")
            return False
    
    def disconnect(self):
        """断开串口连接"""
        if self.serial_conn and self.serial_conn.is_open:
            self.serial_conn.close()
            logger.info(f"已断开监听端口 {self.port}")
            
        if self.response_conn and self.response_conn != self.serial_conn and self.response_conn.is_open:
            self.response_conn.close()
            logger.info(f"已断开响应端口 {self.response_port}")
    
    def start_simulation(self):
        """启动串口模拟"""
        if not self.connect():
            return False
        
        self.running = True
        self.thread = threading.Thread(target=self._listen_and_respond, daemon=True)
        self.thread.start()
        logger.info(f"串口 {self.port} 模拟器已启动")
        return True
    
    def stop_simulation(self):
        """停止串口模拟"""
        self.running = False
        if self.thread:
            self.thread.join(timeout=2)
        self.disconnect()
        logger.info(f"串口 {self.port} 模拟器已停止")
    
    def _listen_and_respond(self):
        """监听串口数据并响应"""
        buffer = b''
        
        while self.running and self.serial_conn and self.serial_conn.is_open:
            try:
                # 读取数据
                if self.serial_conn.in_waiting > 0:
                    data = self.serial_conn.read(self.serial_conn.in_waiting)
                    buffer += data
                    logger.info(f"串口 {self.port} 接收到原始数据: {data}")
                    logger.info(f"串口 {self.port} 当前缓冲区内容: {buffer}")
                    
                    # 检查缓冲区中是否包含完整的命令
                    buffer = self._process_buffer(buffer)
                    
                    # 清理缓冲区（保留最近的数据，防止命令跨包）
                    if len(buffer) > 1024:  # 防止缓冲区过大
                        buffer = buffer[-512:]
                
                time.sleep(0.01)  # 短暂休眠，避免CPU占用过高
                
            except serial.SerialException as e:
                logger.error(f"串口 {self.port} 读取错误: {e}")
                break
            except Exception as e:
                logger.error(f"串口 {self.port} 处理异常: {e}")
    
    def _process_buffer(self, buffer):
        """处理接收缓冲区中的数据"""
        # 转换缓冲区为小写进行匹配
        buffer_lower = buffer.lower()
        
        # 基础命令定义（小写）
        base_commands = {
            b'<<start>>': b'<<INITIALOK>>',
            b'<<next>>': b'<<ERRCODE:00,00,01,00,00,00,00,00>>'
        }
        
        for command, response in base_commands.items():
            if command in buffer_lower:
                logger.info(f"串口 {self.port} 收到命令: {command.decode('utf-8', errors='ignore')}")
                self._send_response(response)
                
                # 找到原始缓冲区中对应的位置并移除
                # 先找到小写版本的位置
                pos = buffer_lower.find(command)
                if pos != -1:
                    # 从原始缓冲区中移除对应位置的数据
                    buffer = buffer[:pos] + buffer[pos + len(command):]
                    # 同时更新小写版本
                    buffer_lower = buffer_lower[:pos] + buffer_lower[pos + len(command):]
        
        return buffer
    
    def _send_response(self, response):
        """发送响应数据"""
        try:
            if self.response_conn and self.response_conn.is_open:
                # 添加响应延时
                logger.info(f"串口 {self.port} 等待 {self.response_delay} 秒后发送响应...")
                time.sleep(self.response_delay)
                
                self.response_conn.write(response)
                self.response_conn.flush()
                logger.info(f"串口 {self.port} 通过端口 {self.response_port} 发送响应: {response.decode('utf-8', errors='ignore')}")
        except serial.SerialException as e:
            logger.error(f"串口 {self.port} 发送响应失败: {e}")

def main():
    """主函数"""
    # 串口配置
    response_delay = 1.0  # 响应延时（秒），可以根据需要调整
    
    print("=" * 60)
    print("串口模拟器工具")
    print("=" * 60)
    print("功能说明:")
    print("1. 收到 <<start>> 或 <<START>> 后回复 <<INITIALOK>>")
    print("2. 收到 <<next>> 或 <<NEXT>> 后回复 <<ERRCODE:00,00,01,00,00,00,00,00>>")
    print(f"3. 响应延时: {response_delay} 秒")
    print("=" * 60)
    
    # 询问用户选择模式
    print("\n请选择工作模式:")
    print("1. 同端口模式 - 在接收命令的同一端口发送响应")
    print("2. 交叉模式 - COM2接收命令，COM4发送响应；COM4接收命令，COM2发送响应")
    
    while True:
        try:
            choice = input("请输入选择 (1 或 2，直接回车默认选择1): ").strip()
            if choice == "" or choice == "1":
                mode = "same"
                break
            elif choice == "2":
                mode = "cross"
                break
            else:
                print("请输入 1 或 2")
        except KeyboardInterrupt:
            print("\n程序退出")
            return
    
    # 创建串口模拟器实例
    simulators = []
    
    if mode == "same":
        # 同端口模式
        print("\n使用同端口模式")
        ports_config = [
            {'port': 'COM2', 'baudrate': 9600, 'response_delay': response_delay},
            {'port': 'COM4', 'baudrate': 9600, 'response_delay': response_delay}
        ]
    else:
        # 交叉模式
        print("\n使用交叉模式 - COM2↔COM4")
        ports_config = [
            {'port': 'COM2', 'baudrate': 9600, 'response_delay': response_delay, 'response_port': 'COM4'},
            {'port': 'COM4', 'baudrate': 9600, 'response_delay': response_delay, 'response_port': 'COM2'}
        ]
    
    # 启动所有串口模拟器
    for config in ports_config:
        simulator = SerialSimulator(**config)
        if simulator.start_simulation():
            simulators.append(simulator)
        else:
            logger.warning(f"跳过串口 {config['port']}")
    
    if not simulators:
        logger.error("没有成功启动任何串口模拟器")
        return
    
    print(f"\n已启动 {len(simulators)} 个串口模拟器")
    print("按 Ctrl+C 退出程序")
    
    try:
        # 保持程序运行
        while True:
            time.sleep(1)
            # 检查所有模拟器是否还在运行
            active_simulators = [s for s in simulators if s.running]
            if not active_simulators:
                logger.info("所有串口模拟器已停止")
                break
                
    except KeyboardInterrupt:
        print("\n正在停止串口模拟器...")
        
    finally:
        # 停止所有模拟器
        for simulator in simulators:
            simulator.stop_simulation()
        print("程序已退出")

if __name__ == "__main__":
    main()