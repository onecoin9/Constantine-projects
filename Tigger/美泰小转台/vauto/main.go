package main

import (
	"flag"
	"fmt"
	"io"
	"log"
	"os"
	"path/filepath"
	"time"
	"vauto/server"
)

func setupLogger() (*os.File, error) {
	// 创建日志目录
	today := time.Now().Format("2006-01-02")
	logDir := filepath.Join("log", today)
	err := os.MkdirAll(logDir, 0755)
	if err != nil {
		return nil, fmt.Errorf("failed to create log directory: %v", err)
	}

	// 打开日志文件
	logFile, err := os.OpenFile(filepath.Join(logDir, "vauto.log"), os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0644)
	if err != nil {
		return nil, fmt.Errorf("failed to open log file: %v", err)
	}

	// 设置日志输出到文件和标准输出
	multiWriter := io.MultiWriter(os.Stdout, logFile)
	log.SetOutput(multiWriter)

	// 设置日志格式，只包含日期和时间（精确到秒）
	log.SetFlags(log.Ldate | log.Ltime)

	return logFile, nil
}

const (
	VAUTO_VERSION = "V1.2.6_250630" // 协议标准化版本：修复0xE6 PLEN，完善ACK机制
)

func main() {
	// 为glog设置参数，确保日志输出到控制台
	flag.Set("logtostderr", "true")
	// 设置日志详细程度，V(2)及以上的日志也会被打印
	flag.Set("v", "3")
	// 解析命令行参数以使glog配置生效
	flag.Parse()

	fmt.Printf("=== vauto 自动化模拟器启动 ===\n")
	fmt.Printf("版本: %s\n", VAUTO_VERSION)
	fmt.Printf("主要更新:\n")
	fmt.Printf("  ✓ 修复0xE6协议PLEN计算，符合标准规范\n")
	fmt.Printf("  ✓ 完善Socket数据填充，支持8Socket标准格式\n")
	fmt.Printf("  ✓ 确保ACK响应机制正确运行\n")
	fmt.Printf("================================\n\n")
	// 设置日志
	logFile, err := setupLogger()
	if err != nil {
		log.Fatalf("Failed to set up logger: %v", err)
	}
	defer logFile.Close()

	log.Println("Starting vauto server...")

	server.RunServer()
}
