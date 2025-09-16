package server

import (
	"log"

	"github.com/Unknwon/goconfig"
)

const (
	VERSION = "V1.2.0_250619"
)

type tConfigSystem struct {
	WaitCmd3        bool
	DoAuth          bool
	DoElecCheck     bool
	DoRemingCheck   bool
	DoSupplyCheck   bool
	EnabledWaitMsec int //收到站点使能后，等待X毫秒开始通知放置
	ResultWaitMsec  int //收到烧录结果后，等待X毫秒开始通知放置
	LotEndWaitMsec  int //批量结束后，等待X毫秒开始电子检测
	JsonProtocal    int //使用自动化json协议,1:byte 2:json
}

func loadConfig() {
	cfg, err := goconfig.LoadConfigFile("config.ini")
	if err != nil {
		panic("LoadConfigFile error")
	}
	configSystem.EnabledWaitMsec = cfg.MustInt("system", "enabled_wait_msec", 1000)
	configSystem.ResultWaitMsec = cfg.MustInt("system", "result_wait_msec", 1000)
	configSystem.LotEndWaitMsec = cfg.MustInt("system", "lotend_wait_msec", 1000)
	configSystem.WaitCmd3 = cfg.MustBool("system", "wait_cmd3", true)
	configSystem.DoAuth = cfg.MustBool("system", "do_auth", false)
	configSystem.DoElecCheck = cfg.MustBool("system", "do_electric_check", false)
	configSystem.DoRemingCheck = cfg.MustBool("system", "do_remaining_check", false)
	configSystem.DoSupplyCheck = cfg.MustBool("system", "do_supply_check", true)
	configSystem.JsonProtocal = cfg.MustInt("system", "json_auto_protocal", 1)
}

func RunServer() {
	log.Printf("Version: %s\n", VERSION)
	loadConfig()
	log.Printf("收到站点使能等待毫秒: %d, 收到烧录结果等待毫秒: %d 供给数达成等待毫秒：%d\r\n",
		configSystem.EnabledWaitMsec, configSystem.ResultWaitMsec, configSystem.LotEndWaitMsec)

	// 自动化命令服务器 - 现在由 auto_cmd_server.go 内部处理
	go DoAutoCmdServer()

	// 自动化字节服务器 - 现在由 auto_byte_server.go 内部处理
	go DoAutoByteServer()

	// 自动化Json服务器 - 现在由 auto_json_server.go 内部处理
	go DoAutoJsonServer()

	// F协议 - 现在由 fprot_server.go 内部处理
	go DoFServer()

	// Syslog服务 - 现在由 syslog.go 内部处理
	go DoSyslog()

	// 温度控制 - 现在由 temp_control.go 内部处理
	go DoTempControl()

	// 旋转控制 - 现在由 turn_control.go 内部处理
	go DoTurnControl()

	// 水阀控制 - 现在由 water_control.go 内部处理
	go DoWaterControl()

	// 采集 - 现在由 collection.go 内部处理
	go DoCollection()

	// 主循环保持程序运行
	select {}
}
