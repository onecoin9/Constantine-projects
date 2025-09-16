package server

import (
	"encoding/json"
	"fmt"
	"log"
	"net"
	"os"
	"strconv"
	"strings"
	"time"

	"github.com/Unknwon/goconfig"
	"github.com/golang/glog"
)

const (
	CONN_SERVER_CMD_PORT = "1000"
)

// 内部配置结构体
type tAutoCmdServerConfig struct {
	Enable     bool
	ServerHost string
	ServerPort string
}

// 全局配置实例
var autoCmdServerConfig *tAutoCmdServerConfig

// 从配置文件加载自动化命令服务器配置
func loadAutoCmdServerConfig() error {
	cfg, err := goconfig.LoadConfigFile("config.ini")
	if err != nil {
		glog.Errorf("加载配置文件失败: %v", err)
		// 使用默认配置
		initAutoCmdServerConfig()
		return err
	}

	autoCmdServerConfig = &tAutoCmdServerConfig{
		Enable:     cfg.MustBool("auto_cmd", "enable", true),
		ServerHost: cfg.MustValue("auto_cmd", "server_host", "localhost"),
		ServerPort: cfg.MustValue("auto_cmd", "server_port", "1000"),
	}

	glog.Infof("自动化命令服务器配置已加载: Enable=%t, Host=%s, Port=%s",
		autoCmdServerConfig.Enable, autoCmdServerConfig.ServerHost, autoCmdServerConfig.ServerPort)

	return nil
}

// 初始化默认配置
func initAutoCmdServerConfig() {
	autoCmdServerConfig = &tAutoCmdServerConfig{
		Enable:     true,
		ServerHost: "localhost",
		ServerPort: "1000",
	}
	glog.Info("使用默认自动化命令服务器配置")
}

// 设置配置参数的公共方法（可选）
func SetAutoCmdServerConfig(enable bool, serverHost string, serverPort string) {
	if autoCmdServerConfig == nil {
		initAutoCmdServerConfig()
	}

	autoCmdServerConfig.Enable = enable
	if serverHost != "" {
		autoCmdServerConfig.ServerHost = serverHost
	}
	if serverPort != "" {
		autoCmdServerConfig.ServerPort = serverPort
	}
}

// 启动自动化命令服务器 - 简化的对外接口
func DoAutoCmdServer() {
	glog.Info("正在启动自动化命令服务器模块...")

	// 加载配置文件
	if err := loadAutoCmdServerConfig(); err != nil {
		glog.Warningf("加载自动化命令服务器配置失败，使用默认配置: %v", err)
	}

	// 检查是否启用
	if !autoCmdServerConfig.Enable {
		glog.Info("自动化命令服务器模块未启用，跳过启动")
		return
	}

	glog.Infof("自动化命令服务器模块启动成功，监听地址: %s:%s",
		autoCmdServerConfig.ServerHost, autoCmdServerConfig.ServerPort)

	// 启动CMD服务器
	doCmdServer()
}

func doCmdServer() {
	// Listen for incoming connections.
	l, err := net.Listen(CONN_TYPE, autoCmdServerConfig.ServerHost+":"+autoCmdServerConfig.ServerPort)
	if err != nil {
		log.Printf("Error listening:%s\n", err.Error())
		os.Exit(1)
	}
	// Close the listener when the application closes.
	defer l.Close()
	log.Printf("Listening on " + autoCmdServerConfig.ServerHost + ":" + autoCmdServerConfig.ServerPort)
	for {
		// Listen for an incoming connection.
		conn, err := l.Accept()
		if err != nil {
			log.Printf("Error accepting: %s\n", err.Error())
			os.Exit(1)
		}
		// Handle connections in a new goroutine.
		go handleCmdRequest(conn)
	}
}

// Handles incoming requests.
func handleCmdRequest(conn net.Conn) {
	// Make a buffer to hold incoming data.
	buf := make([]byte, 1024)
	// Read the incoming connection into the buffer.
	read_len, err := conn.Read(buf)
	if err != nil {
		log.Printf("Error Cmd reading: %s\n", err.Error())
	}

	// Close the connection when you're done with it.
	defer conn.Close()

	if read_len > 0 && buf[0] == '3' {
		cmdStr := string(buf[0:read_len])
		result_string := "0"

		gPdu63Mutex.Lock()
		isHaveRecvPdu63 := gIsHaveRecvPdu63
		gPdu63Mutex.Unlock()

		if !isHaveRecvPdu63 {
			result_string = "1"
			log.Printf("[CMD]Error, Please Send PDU 0x63 First")
		} else {
			idList := strings.Split(cmdStr, ",")
			length := len(idList)
			if length != 20 {
				gIsRecvCmdTskOK = false
				log.Printf("[CMD]Error, Check Cmd3 Format First, Need 20 Variable")
				result_string = "99"
			} else {
				sStopQuantity, _ = strconv.Atoi(idList[1])
				sProductOKQuantity = 0
				sProductNGQuantity = 0
				sSupplyQuantity = 0
				bRecvCmd4LotEnd = false
				bLotEnd = false
				bDoingExit = false
				bRecvCmd4Supply = false
				log.Printf("[CMD]====>Recv Cmd: %s len:%d 生产数量:%d\n", cmdStr, read_len, sStopQuantity)
				Reset()
				if configSystem.DoAuth {
					if sendAuthMessage("GetInfo") {
						gIsRecvCmdTskOK = true
						log.Printf("[CMD]====>Recv Auth Check OK")
					} else {
						result_string = "8"
						gIsRecvCmdTskOK = false
						log.Printf("[CMD]====>Recv Auth Check Failure")
					}
				} else {
					gIsRecvCmdTskOK = true
				}
			}
		}
		conn.Write([]byte(result_string))
		log.Printf("")
	} else if read_len > 0 && buf[0] == '4' {
		var commnad4 tCmd4
		err1 := json.Unmarshal(buf[2:read_len-1], &commnad4)
		if err1 != nil {
			log.Printf("Unmarshal error %s\n", err1.Error())
			return
		}

		cmd4writejson := `{"ErrCode":0, "ErrMsg":""}`
		log.Printf("[CMD]<====Recv CMD4 Request %s", string(commnad4.Method))
		if commnad4.Method == "LotEndReq" {
			if !bRecvCmd4LotEnd {
				bRecvCmd4LotEnd = true
				if isSupplyProgramFinished() {
					log.Printf("所有供给的芯片都已经烧录完成，准备结批...")
					go func() {
						time.Sleep(time.Millisecond * time.Duration(1000))
						doExitTask()
					}()
				} else {
					log.Printf("还有供给的芯片在烧录过程中，稍后退出...")
				}
			}
		} else if commnad4.Method == "SupplyStopSWReq" {
			//供给停止
			bRecvCmd4Supply = true
			go func() {
				time.Sleep(time.Millisecond * time.Duration(3000))
				var write_buffer []byte = []byte(`{"ESender": "KWAT-YanFa","EName": "Opecall","ETime": "2021-11-06-11-49-10-326","EData": {"Code":"MSG_O09000","Info": "供给停止","ETimeType": "OpeCallStart"}}`)
				sendOneSyslogMessageContent(write_buffer)
			}()
		} else if commnad4.Method == "StopSWReq" {
		} else if commnad4.Method == "GetLotData" {
			response := LotDataResponse{
				ErrCode: 0,
				ErrMsg:  "",
			}
			//指针变量
			lotdata := new(LotData)
			lotdata.LotStart = "2021/12/22 14:18:40"
			lotdata.LotEnd = "2021/12/22 16:18:40"
			lotdata.TotalCnt = sStopQuantity
			lotdata.PassCnt = sProductOKQuantity
			lotdata.FailCnt = sProductNGQuantity
			lotdata.RemoveCnt = 0
			lotdata.AlarmTimes = 3
			lotdata.SuspendTimes = 0
			lotdata.TimeRun = "12:34:56"
			lotdata.TimeSuspend = "01:01:01"
			lotdata.UPH = "2800.00"
			lotdata.Effectivity = "100%"
			response.LotData = lotdata
			responsebyte, err := json.Marshal(response)
			if err != nil {
				fmt.Println("结构体生成json字符串错误")
				cmd4writejson = `{"ErrCode":0, "ErrMsg":""}`
			} else {
				cmd4writejson = string(responsebyte)
			}

		} else if commnad4.Method == "RollInfoCode" {

		} else if commnad4.Method == "SetSysCounts" {
			cmd4writejson = "0"
		}
		var cmd4_write_buffer []byte = []byte(cmd4writejson)
		conn.Write(cmd4_write_buffer)
		log.Printf("[CMD]====>Send CMD4 Response OK %s", string(cmd4_write_buffer))
		log.Printf("")
	}
}

// 可选的配置接口，用于在启动前设置参数
func ConfigureAutoCmdServer(enable bool, serverHost string, serverPort string) {
	SetAutoCmdServerConfig(enable, serverHost, serverPort)
}

// 获取当前配置信息
func GetAutoCmdServerConfig() map[string]interface{} {
	if autoCmdServerConfig == nil {
		loadAutoCmdServerConfig()
	}

	return map[string]interface{}{
		"enable":      autoCmdServerConfig.Enable,
		"server_host": autoCmdServerConfig.ServerHost,
		"server_port": autoCmdServerConfig.ServerPort,
	}
}

// 停止自动化命令服务器（如果需要优雅停止）
func StopAutoCmdServer() {
	glog.Info("停止自动化命令服务器模块")
	// TODO: 添加停止逻辑
}
