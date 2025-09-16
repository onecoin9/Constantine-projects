package server

import (
	"encoding/json"
	"fmt"
	"log"
	"net"
	"os"
	"strconv"
	"time"
	"vauto/util"

	"github.com/Unknwon/goconfig"
	"github.com/golang/glog"
)

/*
// 自动化JSON命令结构体
type AutoJsonCmd struct {
	Command     string `json:"Command"`
	AxisSelect  string `json:"AxisSelect"`
	SiteIdx     string `json:"SiteIdx"`
	TargetAngle string `json:"TargetAngle"`
}

// JSON响应结构体
type AutoJsonResponse struct {
	Command      string `json:"Command"`
	AxisSelect   string `json:"AxisSelect,omitempty"`
	SiteIdx      string `json:"SiteIdx,omitempty"`
	CurrentAngle string `json:"CurrentAngle,omitempty"`
	Result       string `json:"Result,omitempty"`
	ErrMsg       string `json:"ErrMsg,omitempty"`
}
*/

const (
	JSON_AUTO_PROTCOL_VERSION = "1.0.0.0"
)

// 内部配置结构体
type tAutoJsonServerConfig struct {
	Enable       bool
	ServerHost   string
	ServerPort   string
	JsonProtocol int // 1:byte 2:json
}

type tJsonRecvCmd struct {
	Method string          `json:"method"`
	Data   json.RawMessage `json:"data"`
}

type tJsonResponseCmd struct {
	Method string `json:"method"`
	Status string `json:"status"`
	Data   string `json:"data"`
}

type tEnableInfoCmd struct {
	Bibid  int    `json:"bibid"`
	Enable string `json:"enable"`
}

// Added structs for testend data
type tSiteResult struct {
	SiteID int `json:"siteid"`
	Result int `json:"result"`
}

type tTestEndData struct {
	BibID int           `json:"bibid"`
	Sites []tSiteResult `json:"sites"`
}

// Structs for the new "teststart" command payload
type tTestStartSiteDetail struct {
	SiteID int    `json:"siteid"`
	Enable bool   `json:"enable"`
	SiteSN string `json:"sitesn"`
}

type tTestStartDataPayload struct {
	BibID int                    `json:"bibid"`
	Sites []tTestStartSiteDetail `json:"sites"`
}

type tTestStartCommandMessage struct {
	Method string                `json:"method"`
	Data   tTestStartDataPayload `json:"data"`
}

// Handles incoming requests.
func handleJsonRequest(conn net.Conn) {
	// Make a buffer to hold incoming data.
	buf := make([]byte, 4096)
	// Read the incoming connection into the buffer.
	n, err := conn.Read(buf)
	if err != nil {
		log.Printf("Error reading:%s\n", err.Error())
	}

	var jsoncommnad tJsonRecvCmd
	err1 := json.Unmarshal(buf[:n], &jsoncommnad)
	if err1 != nil {
		log.Printf("Unmarshal error %s\n", err1.Error())
		return
	}
	cmd := jsoncommnad.Method

	log.Printf("handleJsonRequest cmd:%s\n", jsoncommnad.Method)

	//
	var ResponseCmd tJsonResponseCmd
	ResponseCmd.Data = ""
	ResponseCmd.Status = "success"
	ResponseCmd.Method = cmd

	if cmd == "switch" { //Tester切换模式
		log.Printf("App Ask Switch Mode.")
	} else if cmd == "getversion" { //Tester查询当前协议版本号
		ResponseCmd.Data = `{"version":"` + JSON_AUTO_PROTCOL_VERSION + `"}`
	} else if cmd == "setenable" { //App告知AutoApp站点初始化/使能情况，必须有
		log.Printf("[CMD]====>App Tell Site Init Status.")
		var enableInfos []tEnableInfoCmd
		err := json.Unmarshal(jsoncommnad.Data, &enableInfos)
		if err != nil {
			log.Printf("Error unmarshalling 'setenable' data: %s\n", err.Error())
			ResponseCmd.Status = "error"
			ResponseCmd.Data = `{"message":"failed to parse setenable data"}`
			// Consider sending the error response back to the client here
			// For now, let's prepare the response and let the existing logic send it.
		} else {
			sMaxSiteIdx = len(enableInfos)
			for _, info := range enableInfos {
				// Assuming info.Bibid is 1-indexed and SitesInfo is 0-indexed.
				// Add boundary checks for safety.
				if info.Bibid <= 0 { // Bibid should be positive
					log.Printf("Invalid Bibid %d in 'setenable' data. Skipping.\n", info.Bibid)
					continue
				}
				siteIndex := info.Bibid - 1

				// Ensure siteIndex is within bounds of SitesInfo
				// This part depends on how SitesInfo is sized/managed.
				// For example: if siteIndex >= len(SitesInfo) { log.Printf("Bibid %d out of bounds", info.Bibid); continue }

				enableValue, parseErr := strconv.ParseUint(info.Enable, 2, 64)
				if parseErr != nil {
					log.Printf("Error converting enable string '%s' to uint64 for Bibid %d: %s. Skipping.\n", info.Enable, info.Bibid, parseErr.Error())
					continue
				}

				// Assuming SitesInfo is an array/slice accessible here
				// and can hold at least up to the max Bibid.
				// You might need to adjust this part based on SitesInfo's actual structure and size.
				if siteIndex < len(SitesInfo) { // Basic check
					SitesInfo[siteIndex].Idx = info.Bibid // Or siteIndex + 1 if Idx should be 1-based like Bibid
					SitesInfo[siteIndex].Enabled = enableValue
					log.Printf("SiteIdx:0x%02x (Bibid: %d) Enabled:0x%016X Init OK.\r\n", byte(SitesInfo[siteIndex].Idx), info.Bibid, SitesInfo[siteIndex].Enabled)
				} else {
					log.Printf("Bibid %d (index %d) is out of bounds for SitesInfo (len %d). Skipping.\n", info.Bibid, siteIndex, len(SitesInfo))
				}
			}
			// Assuming the overall operation is successful if we processed some entries,
			// or if individual errors are just logged.
			// If any error during parsing individual items should make the whole command fail,
			// then Sstatus should be set to "error" accordingly.
		}
		gIsHaveRecvPdu63 = true
	} else if cmd == "testend" { //App告知AutoApp芯片烧录结果
		log.Printf("[CMD]====>Received 'testend' command. Raw Data: %s", string(jsoncommnad.Data))
		var testEndPayload tTestEndData
		err := json.Unmarshal(jsoncommnad.Data, &testEndPayload)
		if err != nil {
			log.Printf("Error unmarshalling 'testend' data: %s\n", err.Error())
			ResponseCmd.Status = "error"
			ResponseCmd.Data = `{"message":"failed to parse testend data"}`
		} else {
			siteBibID := testEndPayload.BibID
			foundSiteInfoIndex := -1
			for i := 0; i < len(SitesInfo); i++ { // Assuming SitesInfo is a slice or array and accessible
				if SitesInfo[i].Idx == siteBibID {
					foundSiteInfoIndex = i
					break
				}
			}

			if foundSiteInfoIndex == -1 {
				log.Printf("Error: BibID %d from 'testend' data not found in SitesInfo.\n", siteBibID)
				ResponseCmd.Status = "error"
				ResponseCmd.Data = fmt.Sprintf("{\"message\":\"BibID %d not found\"}", siteBibID)
			} else {
				currentSiteInfo := SitesInfo[foundSiteInfoIndex]
				tempProductOKQuantity := 0
				tempProductNGQuantity := 0

				for _, siteResultItem := range testEndPayload.Sites {
					// Assuming siteResultItem.SiteID is 1-based, like sktIdx in previous logic
					sktIdx := siteResultItem.SiteID - 1

					if sktIdx < 0 { // Basic validation for sktIdx
						log.Printf("Invalid siteid %d for BibID %d. Skipping.\n", siteResultItem.SiteID, siteBibID)
						continue
					}

					// Add check for sktIdx against max slots if necessary, e.g. sktIdx >= MAX_SOCKETS_PER_SITE

					isSocketEnabled := (currentSiteInfo.Enabled>>uint(sktIdx))&0x1 == 1

					// Updated logic for siteResultItem.Result:
					// 0: Disabled
					// 1: OK/Good product
					// 2: NG/Bad product
					// Other: Extendable
					switch siteResultItem.Result {
					case 1: // OK/Good product
						sProductOKQuantity++
						tempProductOKQuantity++
						log.Printf("SiteBibID: %d, Socket (SiteID): %d, Result: %d (OK), Enabled: %t", siteBibID, siteResultItem.SiteID, siteResultItem.Result, isSocketEnabled)
					case 2: // NG/Bad product
						if isSocketEnabled {
							sProductNGQuantity++
							tempProductNGQuantity++
							log.Printf("SiteBibID: %d, Socket (SiteID): %d, Result: %d (NG), Enabled: %t - COUNTED as NG", siteBibID, siteResultItem.SiteID, siteResultItem.Result, isSocketEnabled)
						} else {
							log.Printf("SiteBibID: %d, Socket (SiteID): %d, Result: %d (NG), Socket NOT Enabled: %t - NOT COUNTED as NG", siteBibID, siteResultItem.SiteID, siteResultItem.Result, isSocketEnabled)
						}
					case 0: // Disabled
						log.Printf("SiteBibID: %d, Socket (SiteID): %d, Result: %d (Disabled), Enabled: %t", siteBibID, siteResultItem.SiteID, siteResultItem.Result, isSocketEnabled)
						// If a socket was enabled but now reported as disabled, you might want specific logic here.
						// For now, it's just logged and not counted as OK/NG.
					default: // Extendable / Other values
						log.Printf("SiteBibID: %d, Socket (SiteID): %d, Result: %d (Unknown/Extendable), Enabled: %t", siteBibID, siteResultItem.SiteID, siteResultItem.Result, isSocketEnabled)
					}
				}
				log.Printf("[CMD]====>App通知IC烧录结果 BibID:%d (Site Idx:%d) - OK an diesem Standort: %d, NG an diesem Standort (wenn aktiviert): %d. Gesamt OK: %d, Gesamt NG: %d",
					siteBibID, currentSiteInfo.Idx, tempProductOKQuantity, tempProductNGQuantity, sProductOKQuantity, sProductNGQuantity)
				ResponseCmd.Status = "success" // Ensure status is success if processing is fine
				ResponseCmd.Data = `{}`
			}
		}
		// The old code had: ResponseCmd.Command = "ProgramResultBack"
		// tJsonResponseCmd does not have a "Command" field.
		// The Method field of the response is already set to the incoming cmd ("testend").
		// Data field is set above.
	} else {
		log.Printf("Unknown Cmd: 0x%x.\n", cmd)
	}

	sendbuf := make([]byte, 4096)
	responsebyte, err := json.Marshal(ResponseCmd)
	if err != nil {
		fmt.Println("结构体生成json字符串错误")
		return
	}

	// 将responsebyte放入sendbuf
	copy(sendbuf, responsebyte)
	datalen := len(responsebyte)

	conn.Write(sendbuf[0 : datalen+1])
	log.Printf("Send App Cmd %s Len:%d", string(responsebyte), datalen+1)
	log.Printf("\r\n")
	if cmd == "setenable" {
		go func() {
			time.Sleep(time.Millisecond * time.Duration(configSystem.EnabledWaitMsec))
			num := 0
			if configSystem.DoElecCheck {
				for num < sMaxSiteIdx {
					if SitesInfo[num].Enabled != 0x0 {
						sendJsonAutoMessage(num, "teststart")
					}
					num++
				}
			}

			log.Printf("Wait Millisecond %d\r\n", configSystem.EnabledWaitMsec)
			if configSystem.WaitCmd3 {
				waitTimes := 20
				for waitTimes > 0 {
					if gIsRecvCmdTskOK {
						break
					}
					waitTimes--
					time.Sleep(time.Millisecond * time.Duration(1000))
				}
				if waitTimes <= 0 {
					log.Printf("No CMD3 tsk file cmd recv, quit.\n")
					log.Printf("")
				} else {
					//等待接收0x68完成
					//log.Printf("AutoApp等待接收App指令0x68\r\n")
					time.Sleep(time.Millisecond * 3000)
					num = 0
					for num < sMaxSiteIdx {
						if SitesInfo[num].Enabled != 0x0 {
							sendJsonAutoMessage(num, "teststart")
						}
						num++
					}
				}
			} else if !configSystem.WaitCmd3 {
				//等待接收0x68完成
				//log.Printf("AutoApp等待接收App指令0x68\r\n")
				time.Sleep(time.Millisecond * 3000)
				num = 0
				for num < sMaxSiteIdx {
					if SitesInfo[num].Enabled != 0x0 {
						sendJsonAutoMessage(num, "teststart")
					}
					num++
				}
			}

		}()
	}

	// Close the connection when you're done with it.
	conn.Close()
}

func sendJsonAutoMessage(siteidx int, cmd string) {
	conn, err := net.Dial(CONN_TYPE, CONN_CLIENT_HOST+":"+CONN_CLIENT_PORT)
	if err != nil {
		log.Print("dial failed:", err)
		return
	}
	defer conn.Close()

	write_buffer := make([]byte, 4096)
	for i, _ := range write_buffer {
		write_buffer[i] = 0
	}
	sendLen := 0

	if cmd == "teststart" {
		var writeEnable uint64 = 0
		if configSystem.DoSupplyCheck {
			sktIdx := 0
			remainNum := sStopQuantity - sSupplyQuantity
			if remainNum <= 0 || (isSupplyProgramFinished() && bRecvCmd4LotEnd) {
				log.Printf("完成此次批量 SupplyQuantity:%d StopQuantity:%d RecvCmd4LotEnd:%t\n", sSupplyQuantity, sStopQuantity, bRecvCmd4LotEnd)
				go doExitTask()
				return
			}
			curSupplyNum := util.GetByteEnableCnt(SitesInfo[siteidx].Enabled, sMaxSktIdx)
			log.Printf("[CMD]<====VAuto告诉App芯片放置情况开始 HaveSupply:%d Remain:%d PrepareSupply:%X",
				sSupplyQuantity, remainNum, curSupplyNum)
			if curSupplyNum > remainNum {
				var newEnabled uint64 = 0
				for sktIdx < sMaxSktIdx {
					if SitesInfo[siteidx].Enabled>>sktIdx&0x1 == 1 {
						newEnabled |= 1 << sktIdx
						remainNum--
						if remainNum == 0 {
							break
						}
					}
					sktIdx++
				}
				curSupplyNum = util.GetByteEnableCnt(newEnabled, sMaxSktIdx)
				writeEnable = newEnabled
				log.Printf("[CMD]<====VAuto告诉App芯片放置情况开始 Enabled:0x%X", writeEnable)
			} else {
				writeEnable = SitesInfo[siteidx].Enabled
			}
			sSupplyQuantity += curSupplyNum
			log.Printf("[CMD]<====VAuto告诉App芯片放置情况调整 HaveSupply:%d Remain:%d PrepareSupply:%d 放置情况:0x%x",
				sSupplyQuantity, remainNum, curSupplyNum, writeEnable)
		} else {
			writeEnable = SitesInfo[siteidx].Enabled //SKT1-16使能情况
			log.Printf("[CMD]<====VAuto告诉App芯片放置情况")
		}
		if bRecvCmd4Supply {
			log.Printf("收到供给停止，不再放置芯片\n")
			log.Printf("")
			return
		}
		//返回数据包
		// 1. Construct the Data part of the payload
		var sitesDetails []tTestStartSiteDetail
		if siteidx < len(SitesInfo) && sMaxSktIdx > 0 { // Basic safety checks
			currentSiteEnabledFlags := SitesInfo[siteidx].Enabled
			for skt := 0; skt < sMaxSktIdx; skt++ {
				isEnabled := (currentSiteEnabledFlags>>uint(skt))&0x1 == 1
				siteSN := ""
				sitesDetails = append(sitesDetails, tTestStartSiteDetail{
					SiteID: skt + 1, // siteid is 1-based
					Enable: isEnabled,
					SiteSN: siteSN,
				})
			}
		} else {
			log.Printf("Error: siteidx %d out of bounds or sMaxSktIdx not positive for 'teststart'", siteidx)
			// Decide how to handle: return, send error, or send empty sites list
		}

		payloadData := tTestStartDataPayload{
			BibID: SitesInfo[siteidx].Idx, // Assuming BibID comes from SitesInfo.Idx
			Sites: sitesDetails,
		}

		// 2. Construct the full command message
		message := tTestStartCommandMessage{
			Method: "teststart",
			Data:   payloadData,
		}

		// 3. Marshal to JSON
		responsebyte, err := json.Marshal(message)
		if err != nil {
			log.Printf("Error marshalling 'teststart' command: %s\n", err.Error())
			return // Or handle error appropriately
		}
		copy(write_buffer, responsebyte)
		sendLen = len(responsebyte) + 1
	} else if cmd == "getversion" {
		write_json := `{"data":{}, "method":"getversion"}`
		write_buffer = []byte(write_json)
		sendLen = len(write_buffer) + 1
		log.Printf("[CMD]<====VAuto告知App当前支持的协议版本号")
	} else if cmd == "lotstart" {
		log.Printf("Current SiteIdx: 0x%x sendLen:%d\r\n", byte(SitesInfo[siteidx].Idx), sendLen)
	}

	n, err2 := conn.Write(write_buffer[0:sendLen])
	if err2 != nil {
		log.Printf("cli send error:%s\n", err2)
		os.Exit(1)
	}
	log.Printf("VAuto Send Pdu:0x%X From port %s to App len:%d Cmd:%s\n", cmd, CONN_CLIENT_PORT, n, string(write_buffer[5:n]))

	// Make a buffer to hold incoming data.
	read_buffer := make([]byte, 4096)
	// Read the incoming connection into the buffer.
	n, err = conn.Read(read_buffer)
	if err != nil {
		log.Printf("Error reading:%s\n", err.Error())
	}
	log.Printf("VAuto Read Cmd:0x%X to App len:%d\n", cmd, n)
	log.Printf("")
}

// 启动自动化Json服务器 - 简化的对外接口
func DoAutoJsonServer() {
	glog.Info("正在启动自动化Json服务器模块...")

	gIsRecvCmdTskOK = false
	// 加载配置文件
	if err := loadAutoJsonServerConfig(); err != nil {
		glog.Warningf("加载自动化Json服务器配置失败，使用默认配置: %v", err)
	}

	// 检查是否启用
	if !autoJsonServerConfig.Enable {
		glog.Info("自动化Json服务器模块未启用，跳过启动")
		return
	}

	glog.Infof("自动化Json服务器模块启动成功，监听地址: %s:%s",
		autoJsonServerConfig.ServerHost, autoJsonServerConfig.ServerPort)

	// 启动TCP服务器
	doTcpJsonServer()
}

func doTcpJsonServer() {
	// Listen for incoming connections.
	l, err := net.Listen(CONN_TYPE, autoJsonServerConfig.ServerHost+":"+autoJsonServerConfig.ServerPort)
	if err != nil {
		log.Printf("Error listening:%s\n", err.Error())
		os.Exit(1)
	}
	// Close the listener when the application closes.
	defer l.Close()
	log.Printf("Listening on " + autoJsonServerConfig.ServerHost + ":" + autoJsonServerConfig.ServerPort)
	log.Printf("")
	for {
		// Listen for an incoming connection.
		conn, err := l.Accept()
		if err != nil {
			log.Printf("Error accepting: %s\n", err.Error())
			os.Exit(1)
		}
		go handleJsonRequest(conn)
	}
}

// 全局配置实例
var autoJsonServerConfig *tAutoJsonServerConfig

// 从配置文件加载自动化Json服务器配置
func loadAutoJsonServerConfig() error {
	cfg, err := goconfig.LoadConfigFile("config.ini")
	if err != nil {
		glog.Errorf("加载配置文件失败: %v", err)
		// 使用默认配置
		initAutoJsonServerConfig()
		return err
	}

	autoJsonServerConfig = &tAutoJsonServerConfig{
		Enable:       cfg.MustBool("auto_json", "enable", true),
		ServerHost:   cfg.MustValue("auto_json", "server_host", "localhost"),
		ServerPort:   cfg.MustValue("auto_json", "server_port", "64100"),
		JsonProtocol: cfg.MustInt("auto_json", "json_protocol", 1),
	}

	glog.Infof("自动化Json服务器配置已加载: Enable=%t, Host=%s, Port=%s, JsonProtocol=%d",
		autoJsonServerConfig.Enable, autoJsonServerConfig.ServerHost, autoJsonServerConfig.ServerPort, autoJsonServerConfig.JsonProtocol)

	return nil
}

// 初始化默认配置
func initAutoJsonServerConfig() {
	autoJsonServerConfig = &tAutoJsonServerConfig{
		Enable:       true,
		ServerHost:   "localhost",
		ServerPort:   "64100",
		JsonProtocol: 1,
	}
	glog.Info("使用默认自动化Json服务器配置")
}

// 设置配置参数的公共方法（可选）
func SetAutoJsonServerConfig(enable bool, serverHost string, serverPort string, jsonProtocol int) {
	if autoJsonServerConfig == nil {
		initAutoJsonServerConfig()
	}

	autoJsonServerConfig.Enable = enable
	if serverHost != "" {
		autoJsonServerConfig.ServerHost = serverHost
	}
	if serverPort != "" {
		autoJsonServerConfig.ServerPort = serverPort
	}
	autoJsonServerConfig.JsonProtocol = jsonProtocol
}

// 可选的配置接口，用于在启动前设置参数
func ConfigureAutoJsonServer(enable bool, serverHost string, serverPort string, jsonProtocol int) {
	SetAutoJsonServerConfig(enable, serverHost, serverPort, jsonProtocol)
}

// 获取当前配置信息
func GetAutoJsonServerConfig() map[string]interface{} {
	if autoJsonServerConfig == nil {
		loadAutoJsonServerConfig()
	}

	return map[string]interface{}{
		"enable":        autoByteServerConfig.Enable,
		"server_host":   autoByteServerConfig.ServerHost,
		"server_port":   autoByteServerConfig.ServerPort,
		"json_protocol": autoByteServerConfig.JsonProtocol,
	}
}

// 停止自动化字节服务器（如果需要优雅停止）
func StopAutoJsonServer() {
	glog.Info("停止自动化Json服务器模块")
	// TODO: 添加停止逻辑
}
