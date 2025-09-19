package server

import (
	"encoding/csv"
	"encoding/json"

	"fmt"
	"io/ioutil"
	"log"
	"net"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
	"sync/atomic"
	"time"
	"vauto/util"

	"github.com/Unknwon/goconfig"
	"github.com/golang/glog"
	"github.com/google/uuid"
)

const (
	CONN_SYSLOG_HOST = "localhost"
	CONN_SYSLOG_PORT = "61000"
)

// 内部配置结构体
type tSyslogConfig struct {
	EnableSyslog    bool
	TestSyslogTimer bool
	SyslogTimerMsec int
}

// 全局配置实例
var syslogConfig *tSyslogConfig

type EData struct {
	ProductVID string          `json:"ProductVID"`
	LotID      string          `json:"LotID"`
	PRODUCT    string          `json:"PRODUCT"`
	Operator   string          `json:"Operator"`
	Storage    json.RawMessage `json:"Storage"`
	ScanCode   json.RawMessage `json:"ScanCode"`
}

type EventMsgData struct {
	EventSender string          `json:"ESender"`
	EventName   string          `json:"EName"`
	EventTime   string          `json:"ETime"`
	EventID     int             `json:"EventID"`
	ICCount     uint64          `json:"ICCount"`
	EventMsg    json.RawMessage `json:"EData"`
}

type SendData struct {
	Path        string
	SendOneTime int
	SendTotal   int
	SendedTotal int
}

type SendConfig struct {
	Lotid    string
	Product  string
	Sender   string
	SendTime string
	SendData []SendData
}

var send_config SendConfig
var syslog_counter uint64
var syslog_lock sync.Mutex
var vid_list = make(map[string]int)
var be_parsed_config = false

// 从配置文件加载syslog配置
func loadSyslogConfig() error {
	cfg, err := goconfig.LoadConfigFile("config.ini")
	if err != nil {
		glog.Errorf("加载配置文件失败: %v", err)
		// 使用默认配置
		initSyslogConfig()
		return err
	}

	syslogConfig = &tSyslogConfig{
		EnableSyslog:    cfg.MustBool("syslog", "enable_syslog", true),
		TestSyslogTimer: cfg.MustBool("syslog", "test_syslog", false),
		SyslogTimerMsec: cfg.MustInt("syslog", "syslog_timer_msec", 10000),
	}

	glog.Infof("Syslog配置已加载: EnableSyslog=%t, TestSyslogTimer=%t, TimerMsec=%d",
		syslogConfig.EnableSyslog, syslogConfig.TestSyslogTimer, syslogConfig.SyslogTimerMsec)

	return nil
}

// 初始化默认配置
func initSyslogConfig() {
	syslogConfig = &tSyslogConfig{
		EnableSyslog:    true,
		TestSyslogTimer: false,
		SyslogTimerMsec: 10000,
	}
	glog.Info("使用默认Syslog配置")
}

// 设置配置参数的公共方法（可选）
func SetSyslogConfig(enableSyslog bool, testSyslogTimer bool, syslogTimerMsec int) {
	if syslogConfig == nil {
		initSyslogConfig()
	}

	syslogConfig.EnableSyslog = enableSyslog
	syslogConfig.TestSyslogTimer = testSyslogTimer
	if syslogTimerMsec > 0 {
		syslogConfig.SyslogTimerMsec = syslogTimerMsec
	}
}

// 启动Syslog服务 - 简化的对外接口
func DoSyslog() {
	glog.Info("正在启动Syslog模块...")

	// 加载配置文件
	if err := loadSyslogConfig(); err != nil {
		glog.Warningf("加载Syslog配置失败，使用默认配置: %v", err)
	}

	// 检查是否启用
	if !syslogConfig.EnableSyslog {
		glog.Info("Syslog模块未启用，跳过启动")
		return
	}

	glog.Infof("Syslog模块启动成功，定时器间隔: %d ms", syslogConfig.SyslogTimerMsec)

	// 启动定时发送goroutine
	go func() {
		d := time.Duration(syslogConfig.SyslogTimerMsec) * time.Millisecond
		t := time.NewTicker(d)
		defer t.Stop()
		Reset()
		for {
			<-t.C
			if gIsRecvCmdTskOK || syslogConfig.TestSyslogTimer {
				SendSyslogMessage()
			}
		}
	}()
}

func Reset() {
	for k := range vid_list {
		delete(vid_list, k)
	}
	syslog_counter = 0
	for i := 0; i < len(send_config.SendData); i++ {
		send_config.SendData[i].SendedTotal = 0
	}
	glog.Info("SendSyslogMessage Reset\n")
}

// 查找syslog文件夹下的json文件，逐一发送出去
func SendSyslogMessage() {
	if !be_parsed_config {
		be_parsed_config = parseJsonConfig()
	}
	if !be_parsed_config {
		glog.Errorf("be_parsed_config fail\n")
		return
	}
	if send_config.Sender == "" {
		send_config.Sender, _ = util.GetDetailedHostname()
	}

	for i, val := range send_config.SendData {
		var files []string
		root := "./syslog/" + val.Path
		err := filepath.Walk(root, func(path string, info os.FileInfo, err error) error {
			files = append(files, path)
			return nil
		})
		if err != nil {
			panic(err)
		}
		if len(files) == 0 {
			continue
		}
		//同一目录发送多次，使用不同的vid
		for j := 0; j < send_config.SendData[i].SendOneTime; j++ {
			uuidWithHyphen := uuid.New().String()
			uuidWithHyphen = strings.ToUpper(uuidWithHyphen)
			send_ok := false
			if send_config.SendData[i].SendedTotal >= send_config.SendData[i].SendTotal {
				continue
			}
			for _, file := range files {
				//使用同一个vid
				if filepath.Ext(file) == ".json" {
					if sendOneSyslogMessage(file, uuidWithHyphen, send_config) {
						send_ok = true
					}
				}
			}
			if send_ok {
				send_config.SendData[i].SendedTotal++
			}
		}
	}

}

func parseJsonConfig() bool {
	syslog_data, err := ioutil.ReadFile("./syslog/syslog.json")
	if err != nil {
		glog.Errorf("ReadFile error %s.", err.Error())
		return false
	}

	err = json.Unmarshal(syslog_data, &send_config)
	if err != nil {
		glog.Errorf("Unmarshal error %s.", err.Error())
		return false
	}
	return true
}

func sendOneSyslogMessage(path string, vid string, config SendConfig) bool {
	conn, err := net.Dial(CONN_TYPE, CONN_SYSLOG_HOST+":"+CONN_SYSLOG_PORT)
	if err != nil {
		//log.Printf("dial failed:", err)
		return false
	}
	defer conn.Close()
	syslog_data, err := ioutil.ReadFile(path)
	if err != nil {
		fmt.Print(err)
	}

	//replace AAA to lotid, replace vid to uuid
	old_json := string(syslog_data)

	old_json = strings.ReplaceAll(old_json, "ACROVIEW_LOTID", config.Lotid)
	old_json = strings.ReplaceAll(old_json, "ACROVIEW_PRODUCT", config.Product)
	old_json = strings.ReplaceAll(old_json, "ACROVIEW_VID", vid)
	old_json = strings.ReplaceAll(old_json, "ACROVIEW_ESENDER", config.Sender)
	old_json = strings.ReplaceAll(old_json, "ACROVIEW_CODEINFO", vid[0:8])
	diff_time := strings.Split(config.SendTime, ",")
	t := time.Now()
	diff_y, err := strconv.Atoi(diff_time[0])
	if err != nil {
		diff_y = 0
	}
	diff_m, err := strconv.Atoi(diff_time[1])
	if err != nil {
		diff_m = 0
	}
	diff_d, err := strconv.Atoi(diff_time[2])
	if err != nil {
		diff_d = 0
	}
	t = t.AddDate(diff_y, diff_m, diff_d)
	old_json = strings.ReplaceAll(old_json, "ACROVIEW_ETIME", t.Format("2006-01-02-15-04-05-000"))

	var edata EData
	var eventData EventMsgData
	err = json.Unmarshal([]byte(old_json), &eventData)
	if err != nil {
		glog.Errorf("Unmarshal error %s.", err.Error())
		return false
	}
	err = json.Unmarshal(eventData.EventMsg, &edata)
	if err != nil {
		glog.Errorf("Unmarshal error %s.", err.Error())
		return false
	}

	syslog_lock.Lock()
	atomic.AddUint64(&syslog_counter, 1)
	if edata.ProductVID != "" {
		vid_list[edata.ProductVID] = 1
	}
	syslog_lock.Unlock()
	filename := filepath.Base(path)
	log.Printf("sendOneSyslogMessage syslog_count: %d vid_count:%d sender:%s path:%s vid:%s",
		syslog_counter, len(vid_list), config.Sender, filename, edata.ProductVID)

	_, err = conn.Write([]byte(old_json))
	if err != nil {
		log.Printf("cli send syslog_data %s error:%s\n", old_json, err)
		return false
	}
	return true

}

// 可选的配置接口，用于在启动前设置参数
func ConfigureSyslog(enableSyslog bool, testSyslogTimer bool, syslogTimerMsec int) {
	SetSyslogConfig(enableSyslog, testSyslogTimer, syslogTimerMsec)
}

// 获取当前配置信息
func GetSyslogConfig() map[string]interface{} {
	if syslogConfig == nil {
		loadSyslogConfig()
	}

	return map[string]interface{}{
		"enable_socket":     syslogConfig.EnableSyslog,
		"test_syslog":       syslogConfig.TestSyslogTimer,
		"syslog_timer_msec": syslogConfig.SyslogTimerMsec,
	}
}

// 停止Syslog服务（如果需要优雅停止）
func StopSyslog() {
	glog.Info("停止Syslog模块")
	// TODO: 添加停止逻辑
}

func DumpFile() {
	file, err := os.Create("dump.csv")
	if err != nil {
		fmt.Println("Error opening CSV file:", err)
		return
	}
	defer file.Close()

	writer := csv.NewWriter(file)

	defer writer.Flush()
}
