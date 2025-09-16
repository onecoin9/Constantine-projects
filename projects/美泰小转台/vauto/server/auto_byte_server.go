package server

import (
	"encoding/binary"
	"encoding/json"
	"fmt"
	"log"
	"net"
	"strconv"
	"sync"
	"time"
	"vauto/util"

	"github.com/Unknwon/goconfig"
	"github.com/golang/glog"
)

const (
	CONN_CLIENT_HOST = "localhost"
	CONN_CLIENT_PORT = "64101"
	CONN_SERVER_HOST = "localhost"
	CONN_SERVER_PORT = "64100"
	CONN_AUTH_HOST   = "localhost"
	CONN_AUTH_PORT   = "2020"
	CONN_TYPE        = "tcp"
	MAX_SITE_IDX     = 20
)

// 内部配置结构体
type tAutoByteServerConfig struct {
	Enable               bool
	ServerHost           string
	ServerPort           string
	JsonProtocol         int // 1:byte 2:json
	SpecialSite          int // 特殊站点编号
	SpecialSiteMaxSkt    int // 特殊站点最大SKT数量
	SpecialSiteSleepMsec int // 特殊站点处理间隔时间
	SktCntPerSite        int // 每个站点的Socket数量
}

// 特殊站点处理状态
type tSpecialSiteInfo struct {
	Enabled        bool // 是否启用特殊站点处理
	SiteIdx        int  // 特殊站点编号
	MaxCount       int  // 每次处理的最大数量
	PendingCount   int  // 待处理数量
	ProcessedCount int  // 已处理数量
	SuccessCount   int  // 处理成功数量
	FailureCount   int  // 处理失败数量
	IsProcessing   bool // 是否正在处理
}

// 特殊站点消息类型
type tSpecialSiteMessage struct {
	Type         string // "success_count", "result"
	SiteIdx      int
	SuccessCount int
	FailureCount int
	Data         []byte
}

// 特殊站点处理请求
type tSpecialSiteRequest struct {
	Type  string // "start_processing"
	Count int    // 处理数量
}

const (
	AutoMode_AP8000     = 1
	AutoMode_AP8000V2_2 = 2
	AutoMode_AP8000V2_3 = 2
)

// History 序列化器
type tSiteInfo struct {
	Idx              int    //站点编号1-9
	Enabled          uint64 //某个站点使能情况01-FF
	ElecChk          []byte //某个站点电子检测情况
	RemaChk          []byte //某个站点残料检测情况
	Alias            string //站点别名
	ElecCheckNum     int    //接收E8次数
	SendElecCheckNum int    //发送E8次数
	RemaCheckNum     int    //接收E5次数
	SendRemaCheckNum int    //发送E5次数
	Num              uint
}

type tAuthInfo struct {
	AuthMode      string
	CounterRemain string
	EndTime       string
	FuncRet       string
	GUID          string
	MaxCount      string
	MinCount      string
	StartTime     string
	TimeRemain    string
}

type tPrintResult struct {
	SiteIdx int
	SKTIdx  int
	Uid     string
	Result  int
}

var gIsRecvCmdTskOK bool
var gIsHaveRecvPdu63 bool
var gPdu63Mutex sync.Mutex // 为gIsHaveRecvPdu63添加互斥锁

var sMaxSktIdx int = 8
var sMaxSiteIdx int = 20

// 要烧录数量
var sStopQuantity int

// 已经生产OK数量
var sProductOKQuantity int

// 已经生产NG数量
var sProductNGQuantity int

// 已经投料数量
var sSupplyQuantity int

var comWriteMutex sync.RWMutex // 读写锁

var bRecvCmd4LotEnd bool
var bDoingExit bool
var bRecvCmd4Supply bool
var bLotEnd bool
var gAutoMode = AutoMode_AP8000 // auto mode

var SitesInfo [MAX_SITE_IDX]tSiteInfo
var configSystem tConfigSystem
var gSpecialSiteInfo tSpecialSiteInfo

// 【新增】站点9芯片放置完成标志，用于轴移动时序控制
var gSite9ChipPlacementCompleted = false

// 特殊站点处理通道
var gSpecialSiteMessageChan chan *tSpecialSiteMessage
var gSpecialSiteRequestChan chan *tSpecialSiteRequest

// 特殊站点信息保护锁
var gSpecialSiteInfoMutex sync.RWMutex

type LotDataResponse struct {
	ErrCode int      `json:"ErrCode"`
	ErrMsg  string   `json:"ErrMsg"`
	LotData *LotData `json:"LotData"`
}

type LotData struct {
	LotStart     string `json:"LotStart"`
	LotEnd       string `json:"LotEnd"`
	TotalCnt     int    `json:"TotalCnt"`
	PassCnt      int    `json:"PassCnt"`
	FailCnt      int    `json:"FailCnt"`
	RemoveCnt    int    `json:"RemoveCnt"`
	AlarmTimes   int    `json:"AlarmTimes"`
	SuspendTimes int    `json:"SuspendTimes"`
	Item1        string `json:"Item1"`
	TimeRun      string `json:"TimeRun"`
	TimeSuspend  string `json:"TimeSuspend"`
	UPH          string `json:"UPH"`
	Effectivity  string `json:"Effectivity"`
}

// 全局配置实例
var autoByteServerConfig *tAutoByteServerConfig

// 从配置文件加载自动化字节服务器配置
func loadAutoByteServerConfig() error {
	cfg, err := goconfig.LoadConfigFile("config.ini")
	if err != nil {
		glog.Errorf("加载配置文件失败: %v", err)
		// 使用默认配置
		initAutoByteServerConfig()
		return err
	}

	autoByteServerConfig = &tAutoByteServerConfig{
		Enable:               cfg.MustBool("auto_byte", "enable", true),
		ServerHost:           cfg.MustValue("auto_byte", "server_host", "localhost"),
		ServerPort:           cfg.MustValue("auto_byte", "server_port", "64100"),
		JsonProtocol:         cfg.MustInt("auto_byte", "json_protocol", 1),
		SpecialSite:          cfg.MustInt("auto_byte", "special_site", 0),
		SpecialSiteMaxSkt:    cfg.MustInt("auto_byte", "special_site_max_skt", 4),
		SpecialSiteSleepMsec: cfg.MustInt("auto_byte", "special_site_sleep_msec", 1000),
		SktCntPerSite:        cfg.MustInt("auto_byte", "skt_cnt_per_site", 16),
	}

	glog.Infof("自动化字节服务器配置已加载: Enable=%t, Host=%s, Port=%s, JsonProtocol=%d, SpecialSite=%d, SpecialSiteMaxSkt=%d, SktCntPerSite=%d",
		autoByteServerConfig.Enable, autoByteServerConfig.ServerHost, autoByteServerConfig.ServerPort, autoByteServerConfig.JsonProtocol, autoByteServerConfig.SpecialSite, autoByteServerConfig.SpecialSiteMaxSkt, autoByteServerConfig.SktCntPerSite)

	// 初始化特殊站点信息
	initSpecialSiteInfo()

	return nil
}

// 初始化默认配置
func initAutoByteServerConfig() {
	autoByteServerConfig = &tAutoByteServerConfig{
		Enable:            true,
		ServerHost:        "localhost",
		ServerPort:        "64100",
		JsonProtocol:      1,
		SpecialSite:       0,
		SpecialSiteMaxSkt: 4,
		SktCntPerSite:     16,
	}
	glog.Info("使用默认自动化字节服务器配置")

	// 初始化特殊站点信息
	initSpecialSiteInfo()
}

// 初始化特殊站点信息
func initSpecialSiteInfo() {
	// 加锁保护初始化过程，防止并发初始化导致的问题
	gSpecialSiteInfoMutex.Lock()
	defer gSpecialSiteInfoMutex.Unlock()

	// 增加错误恢复机制
	defer func() {
		if r := recover(); r != nil {
			log.Printf("[特殊站点] 初始化过程发生panic，已恢复: %v", r)
			// 确保失败时禁用特殊站点处理
			gSpecialSiteInfo.Enabled = false
			if gSpecialSiteMessageChan != nil {
				close(gSpecialSiteMessageChan)
				gSpecialSiteMessageChan = nil
			}
			if gSpecialSiteRequestChan != nil {
				close(gSpecialSiteRequestChan)
				gSpecialSiteRequestChan = nil
			}
		}
	}()

	// 检查配置有效性
	if autoByteServerConfig == nil {
		log.Printf("[特殊站点] 自动化字节服务器配置为空，禁用特殊站点处理")
		gSpecialSiteInfo.Enabled = false
		return
	}

	if autoByteServerConfig.SpecialSite > 0 {
		// 先清理可能存在的旧资源
		if gSpecialSiteMessageChan != nil {
			close(gSpecialSiteMessageChan)
		}
		if gSpecialSiteRequestChan != nil {
			close(gSpecialSiteRequestChan)
		}

		// 初始化特殊站点信息
		gSpecialSiteInfo = tSpecialSiteInfo{
			Enabled:        true,
			SiteIdx:        autoByteServerConfig.SpecialSite,
			MaxCount:       autoByteServerConfig.SpecialSiteMaxSkt,
			PendingCount:   0,
			ProcessedCount: 0,
			SuccessCount:   0,
			FailureCount:   0,
			IsProcessing:   false,
		}

		// 验证配置参数有效性
		if gSpecialSiteInfo.SiteIdx <= 0 || gSpecialSiteInfo.SiteIdx > 9 {
			log.Printf("[特殊站点] 无效的特殊站点编号: %d，禁用特殊站点处理", gSpecialSiteInfo.SiteIdx)
			gSpecialSiteInfo.Enabled = false
			return
		}

		if gSpecialSiteInfo.MaxCount <= 0 || gSpecialSiteInfo.MaxCount > 64 {
			log.Printf("[特殊站点] 无效的最大Socket数量: %d，使用默认值4", gSpecialSiteInfo.MaxCount)
			gSpecialSiteInfo.MaxCount = 4
		}

		// 初始化通道，使用合理的缓冲区大小
		gSpecialSiteMessageChan = make(chan *tSpecialSiteMessage, 100)
		gSpecialSiteRequestChan = make(chan *tSpecialSiteRequest, 10)

		// 启动特殊站点处理goroutine
		go specialSiteProcessor()

		log.Printf("[特殊站点] 特殊站点处理已启用: Site=%d, MaxCount=%d", gSpecialSiteInfo.SiteIdx, gSpecialSiteInfo.MaxCount)
	} else {
		// 禁用特殊站点处理
		gSpecialSiteInfo.Enabled = false

		// 清理可能存在的旧资源
		if gSpecialSiteMessageChan != nil {
			close(gSpecialSiteMessageChan)
			gSpecialSiteMessageChan = nil
		}
		if gSpecialSiteRequestChan != nil {
			close(gSpecialSiteRequestChan)
			gSpecialSiteRequestChan = nil
		}

		log.Printf("[特殊站点] 特殊站点处理未启用 (special_site=%d)", autoByteServerConfig.SpecialSite)
	}
}

// 检查特殊站点是否在使能列表中
func checkSpecialSiteEnabled() {
	if !gSpecialSiteInfo.Enabled {
		return
	}

	// 检查特殊站点是否在使能的站点列表中
	specialSiteIdx := gSpecialSiteInfo.SiteIdx - 1 // 转换为0-based索引
	if specialSiteIdx >= 0 && specialSiteIdx < sMaxSiteIdx && SitesInfo[specialSiteIdx].Enabled != 0 {
		glog.Infof("特殊站点 %d 已在使能列表中，将进行二次处理", gSpecialSiteInfo.SiteIdx)
	} else {
		glog.Warningf("特殊站点 %d 未在使能列表中，将禁用特殊站点处理", gSpecialSiteInfo.SiteIdx)
		gSpecialSiteInfo.Enabled = false
	}
}

// 检查是否需要进行特殊站点处理
func checkSpecialSiteProcessing(siteidx int, currentSuccessCount int, currentFailureCount int, pData []byte) {
	// 增加错误恢复机制，防止panic导致整个程序崩溃
	defer func() {
		if r := recover(); r != nil {
			log.Printf("[特殊站点] 特殊站点处理发生panic，已恢复: %v", r)
			// 尝试重新初始化特殊站点处理
			if autoByteServerConfig != nil && autoByteServerConfig.SpecialSite > 0 {
				log.Printf("[特殊站点] 尝试重新初始化特殊站点处理器")
				initSpecialSiteInfo()
			}
		}
	}()

	// 加锁保护并发访问
	gSpecialSiteInfoMutex.RLock()
	enabled := gSpecialSiteInfo.Enabled
	siteIdx := gSpecialSiteInfo.SiteIdx
	gSpecialSiteInfoMutex.RUnlock()

	if !enabled {
		return
	}

	// 检查channel是否有效
	if gSpecialSiteMessageChan == nil {
		log.Printf("[特殊站点] 消息通道未初始化，尝试重新初始化")
		// 尝试重新初始化
		if autoByteServerConfig != nil && autoByteServerConfig.SpecialSite > 0 {
			initSpecialSiteInfo()
			// 再次检查
			if gSpecialSiteMessageChan == nil {
				log.Printf("[特殊站点] 重新初始化失败，禁用特殊站点处理")
				gSpecialSiteInfoMutex.Lock()
				gSpecialSiteInfo.Enabled = false
				gSpecialSiteInfoMutex.Unlock()
				return
			}
		} else {
			log.Printf("[特殊站点] 配置无效，禁用特殊站点处理")
			gSpecialSiteInfoMutex.Lock()
			gSpecialSiteInfo.Enabled = false
			gSpecialSiteInfoMutex.Unlock()
			return
		}
	}

	// 构建消息
	message := &tSpecialSiteMessage{
		SiteIdx:      siteidx,
		SuccessCount: currentSuccessCount,
		FailureCount: currentFailureCount,
		Data:         make([]byte, len(pData)), // 创建数据副本，避免并发访问问题
	}
	copy(message.Data, pData)

	if siteidx != siteIdx {
		message.Type = "success_count"
	} else {
		message.Type = "result"
	}

	// 安全的非阻塞发送消息，使用timeout防止阻塞
	select {
	case gSpecialSiteMessageChan <- message:
		// 消息发送成功
		log.Printf("[特殊站点] 消息发送成功: 站点%d, 类型%s, 成功%d, 失败%d",
			siteidx, message.Type, currentSuccessCount, currentFailureCount)
	case <-time.After(time.Millisecond * 100): // 100ms超时
		log.Printf("[特殊站点] 消息发送超时，可能处理器goroutine异常")
		// 检查并尝试重启处理器
		checkAndRestartSpecialSiteProcessor()
	default:
		log.Printf("[特殊站点] 消息队列已满，丢弃消息")
	}
}

// 检查并重启特殊站点处理器
func checkAndRestartSpecialSiteProcessor() {
	gSpecialSiteInfoMutex.Lock()
	defer gSpecialSiteInfoMutex.Unlock()

	if !gSpecialSiteInfo.Enabled {
		return
	}

	// 检查channel状态
	if gSpecialSiteMessageChan == nil || gSpecialSiteRequestChan == nil {
		log.Printf("[特殊站点] 检测到channel异常，重新初始化处理器")

		// 清理旧的channel
		if gSpecialSiteMessageChan != nil {
			close(gSpecialSiteMessageChan)
		}
		if gSpecialSiteRequestChan != nil {
			close(gSpecialSiteRequestChan)
		}

		// 重新创建channel
		gSpecialSiteMessageChan = make(chan *tSpecialSiteMessage, 100)
		gSpecialSiteRequestChan = make(chan *tSpecialSiteRequest, 10)

		// 重新启动处理器goroutine
		go specialSiteProcessor()

		log.Printf("[特殊站点] 特殊站点处理器已重新启动")
	}
}

// 特殊站点处理器 - 独立的goroutine
func specialSiteProcessor() {
	// 增加错误恢复机制，防止goroutine意外退出
	defer func() {
		if r := recover(); r != nil {
			log.Printf("[特殊站点] 特殊站点处理器发生panic，已恢复: %v", r)
			log.Printf("[特殊站点] 处理器将在3秒后自动重启")

			// 等待一段时间后重启
			time.Sleep(3 * time.Second)

			// 检查是否仍需要运行
			gSpecialSiteInfoMutex.RLock()
			enabled := gSpecialSiteInfo.Enabled
			gSpecialSiteInfoMutex.RUnlock()

			if enabled {
				log.Printf("[特殊站点] 重新启动特殊站点处理器")
				go specialSiteProcessor()
			}
		}
	}()

	log.Printf("[特殊站点] 特殊站点处理器已启动")

	for {
		// 使用select with timeout，避免永久阻塞
		select {
		case message, ok := <-gSpecialSiteMessageChan:
			if !ok {
				log.Printf("[特殊站点] 消息通道已关闭，退出处理器")
				return
			}
			if message == nil {
				log.Printf("[特殊站点] 收到空消息，继续处理")
				continue
			}

			// 安全处理消息，避免单个消息处理错误导致整个处理器退出
			func() {
				defer func() {
					if r := recover(); r != nil {
						log.Printf("[特殊站点] 消息处理发生panic，已恢复: %v", r)
					}
				}()

				switch message.Type {
				case "success_count":
					handleSuccessCountMessage(message)
				case "result":
					handleSpecialSiteResultMessage(message)
				default:
					log.Printf("[特殊站点] 未知消息类型: %s", message.Type)
				}
			}()

		case request, ok := <-gSpecialSiteRequestChan:
			if !ok {
				log.Printf("[特殊站点] 请求通道已关闭，退出处理器")
				return
			}
			if request == nil {
				log.Printf("[特殊站点] 收到空请求，继续处理")
				continue
			}

			// 安全处理请求
			func() {
				defer func() {
					if r := recover(); r != nil {
						log.Printf("[特殊站点] 请求处理发生panic，已恢复: %v", r)
					}
				}()

				switch request.Type {
				case "start_processing":
					executeSpecialSiteProcessing(request.Count)
				default:
					log.Printf("[特殊站点] 未知请求类型: %s", request.Type)
				}
			}()

		case <-time.After(time.Minute * 5): // 5分钟心跳检查
			log.Printf("[特殊站点] 处理器运行正常，待处理消息队列长度: %d", len(gSpecialSiteMessageChan))
		}
	}
}

// 处理成功数量消息
func handleSuccessCountMessage(message *tSpecialSiteMessage) {
	gSpecialSiteInfoMutex.Lock()
	defer gSpecialSiteInfoMutex.Unlock()

	// 始终累加成功数量作为待处理数量，不管是否正在处理中
	gSpecialSiteInfo.PendingCount += message.SuccessCount
	log.Printf("[特殊站点] 站点 %d 成功数量 %d，累计待处理数量: %d",
		message.SiteIdx, message.SuccessCount, gSpecialSiteInfo.PendingCount)

	// 如果正在处理中，只累积数量，不启动新的处理
	if gSpecialSiteInfo.IsProcessing {
		log.Printf("[特殊站点] 正在处理中，累积待处理数量，当前处理完成后会自动检查")
		return
	}

	// 检查是否达到处理条件
	if gSpecialSiteInfo.PendingCount >= gSpecialSiteInfo.MaxCount {
		// 计算本次实际处理数量
		actualCount := gSpecialSiteInfo.MaxCount
		if gSpecialSiteInfo.PendingCount < gSpecialSiteInfo.MaxCount {
			actualCount = gSpecialSiteInfo.PendingCount
		}

		// 发送处理请求
		request := &tSpecialSiteRequest{
			Type:  "start_processing",
			Count: actualCount,
		}

		select {
		case gSpecialSiteRequestChan <- request:
			// 请求发送成功
		default:
			log.Printf("[特殊站点] 请求队列已满")
		}
	}
}

// 处理特殊站点结果消息
func handleSpecialSiteResultMessage(message *tSpecialSiteMessage) {
	gSpecialSiteInfoMutex.Lock()
	defer gSpecialSiteInfoMutex.Unlock()

	if !gSpecialSiteInfo.IsProcessing {
		log.Printf("[特殊站点] 当前不在处理中，退出处理结果处理")
		return
	}

	log.Printf("[特殊站点] 收到特殊站点 %d 处理结果", message.SiteIdx)

	// 使用实际的成功和失败数量
	successCount := message.SuccessCount
	failureCount := message.FailureCount

	gSpecialSiteInfo.ProcessedCount += (successCount + failureCount)
	gSpecialSiteInfo.SuccessCount += successCount
	gSpecialSiteInfo.FailureCount += failureCount
	gSpecialSiteInfo.IsProcessing = false

	log.Printf("[特殊站点] 特殊站点处理完成 - 本次成功: %d, 本次失败: %d, 累计成功: %d, 累计失败: %d, 累计处理: %d",
		successCount, failureCount, gSpecialSiteInfo.SuccessCount, gSpecialSiteInfo.FailureCount, gSpecialSiteInfo.ProcessedCount)

	// 检查是否还有待处理的数量
	if gSpecialSiteInfo.PendingCount > 0 {
		// 计算本次实际处理数量
		actualCount := gSpecialSiteInfo.MaxCount
		if gSpecialSiteInfo.PendingCount < gSpecialSiteInfo.MaxCount {
			actualCount = gSpecialSiteInfo.PendingCount
		}

		log.Printf("[特殊站点] 还有 %d 个待处理，继续启动特殊站点处理，本次处理 %d 个",
			gSpecialSiteInfo.PendingCount, actualCount)

		// 发送处理请求
		request := &tSpecialSiteRequest{
			Type:  "start_processing",
			Count: actualCount,
		}

		select {
		case gSpecialSiteRequestChan <- request:
			// 请求发送成功
		default:
			log.Printf("[特殊站点] 请求队列已满")
		}
	}
}

// 执行特殊站点处理
func executeSpecialSiteProcessing(actualCount int) {
	gSpecialSiteInfoMutex.Lock()
	defer gSpecialSiteInfoMutex.Unlock()

	if gSpecialSiteInfo.IsProcessing {
		log.Printf("[特殊站点] 特殊站点正在处理中，忽略重复请求")
		return
	}

	log.Printf("[特殊站点] 开始特殊站点 %d 处理，处理数量: %d", gSpecialSiteInfo.SiteIdx, actualCount)
	gSpecialSiteInfo.IsProcessing = true

	// 减少待处理数量
	gSpecialSiteInfo.PendingCount -= actualCount
	if gSpecialSiteInfo.PendingCount < 0 {
		gSpecialSiteInfo.PendingCount = 0
	}

	// 异步执行特殊站点处理流程
	go executeSpecialSiteProcessingFlow(actualCount)
}

// 执行特殊站点处理流程
func executeSpecialSiteProcessingFlow(actualCount int) {
	specialSiteIdx := gSpecialSiteInfo.SiteIdx - 1
	if specialSiteIdx < 0 || specialSiteIdx >= sMaxSiteIdx {
		log.Printf("[特殊站点] 无效的特殊站点索引: %d", specialSiteIdx)
		gSpecialSiteInfo.IsProcessing = false
		return
	}

	// 设置特殊站点的使能情况，根据实际处理数量设置
	if getAutoMode() == AutoMode_AP8000 {
		SitesInfo[specialSiteIdx].Enabled = (1 << actualCount) - 1
	} else if getAutoMode() == AutoMode_AP8000V2_2 {
		// 设置前actualCount个SKT使能
		enabledBytesLen := (actualCount + 7) / 8 // 向上取整
		enabledBytes := make([]byte, enabledBytesLen)
		for i := 0; i < actualCount && i < sMaxSktIdx; i++ {
			enabledBytes[i/8] |= 1 << (i % 8)
		}
		SitesInfo[specialSiteIdx].Enabled = binary.BigEndian.Uint64(enabledBytes)
	}

	log.Printf("[特殊站点] 设置特殊站点 %d 使能状态: 0x%X (处理数量: %d)",
		gSpecialSiteInfo.SiteIdx, SitesInfo[specialSiteIdx].Enabled, actualCount)

	// 等待配置的延迟时间
	time.Sleep(time.Millisecond * time.Duration(configSystem.EnabledWaitMsec))

	// 发送0xE6命令开始处理
	log.Printf("[特殊站点] 发送0xE6命令到特殊站点 %d", gSpecialSiteInfo.SiteIdx)
	sendSpecialSiteAutoMessage(specialSiteIdx, 0xE6)
}

// 发送特殊站点自动消息
func sendSpecialSiteAutoMessage(siteidx int, cmd byte) {
	conn, err := net.Dial(CONN_TYPE, CONN_CLIENT_HOST+":"+CONN_CLIENT_PORT)
	if err != nil {
		log.Print("dial failed:", err)
		return
	}
	defer conn.Close()

	write_buffer := make([]byte, 128)
	for i, _ := range write_buffer {
		write_buffer[i] = 0
	}
	binary.BigEndian.PutUint16(write_buffer[0:2], 0x4153)
	var sendLen int
	var cal_crc byte

	if cmd == 0xE6 {
		// 【协议重构】修正PDU 0xE6的包格式
		sktBytes := (sMaxSktIdx + 7) / 8 // 计算Socket位图所需的字节数
		pLen := sktBytes + 1             // PDATA长度 = 位图字节数 + 1个站点索引字节

		write_buffer[2] = 0xE6                         // PDU: AutoApp告诉App芯片放置情况
		write_buffer[3] = byte(pLen)                   // PLEN
		write_buffer[4] = byte(SitesInfo[siteidx].Idx) // PDATA: SiteIdx

		// PDATA: SKTEn - 获取最新的使能位图
		// 注意：这里的GetNewEnable需要确保返回正确长度的字节数组
		realSupplyNum, newEnabled := GetNewEnable(getAutoMode(), sMaxSktIdx,
			SitesInfo[siteidx].Enabled, -1) // -1表示不限制数量

		// 填充SKTEn位图
		copy(write_buffer[5:], newEnabled)

		// 计算CRC和总长度
		sendLen = 5 + pLen // 总包长 = PFLAG(2) + PDU(1) + PLEN(1) + PDATA(pLen) + CRC(1)
		cal_crc = util.CheckSumCRC8(write_buffer[0 : 4+pLen])
		write_buffer[4+pLen] = cal_crc

		log.Printf("[CMD]<====VAuto告诉App Site:%d 芯片放置, PLEN=%d, SKTEn=%X",
			SitesInfo[siteidx].Idx, pLen, newEnabled)
		sSupplyQuantity += realSupplyNum

		// 【新增】在发送0xE6后，立即发送ProductInfo命令
		// 只有当这是针对特殊站点（转台）的操作时才执行
		if gSpecialSiteInfo.Enabled && SitesInfo[siteidx].Idx == gSpecialSiteInfo.SiteIdx {
			go sendProductInfoAfterPlacement(siteidx, newEnabled)
		}

	} else if cmd == 0xE4 {
		write_buffer[2] = 0xE4                             //AutoApp告诉App站点设置使能情况
		write_buffer[3] = 2                                //data Len
		write_buffer[4] = byte(SitesInfo[siteidx].Idx)     //站点OK
		write_buffer[5] = byte(SitesInfo[siteidx].Enabled) //SKT1-8使能情况
		write_buffer[6] = util.CheckSumCRC8(write_buffer[0:6])
		sendLen = 7
		cal_crc = write_buffer[6]
		log.Printf("[CMD]<====VAuto告诉App站点设置使能情况")
	} else if cmd == 0xE2 {
		write_buffer[2] = 0xE2 //AutoApp告知App当前支持的协议版本号
		write_buffer[3] = 2    //data Len
		write_buffer[4] = 0x02 //
		write_buffer[5] = 0x07 //V2.7
		write_buffer[6] = util.CheckSumCRC8(write_buffer[0:6])
		sendLen = 7
		cal_crc = write_buffer[6]
		log.Printf("[CMD]<====VAuto告知App当前支持的协议版本号")
	} else if cmd == 0xE1 {
		write_buffer[2] = 0xE1 //AutoApp请求StdMes软件版本信息
		write_buffer[3] = 0    //data Len
		write_buffer[4] = util.CheckSumCRC8(write_buffer[0:6])
		sendLen = 5
		cal_crc = write_buffer[4]
		log.Printf("[CMD]<====VAuto请求App软件协议版本号")
	} else if cmd == 0xE8 || cmd == 0xE5 {
		pdu_name := "接触检查"
		if cmd == 0xE5 {
			pdu_name = "残料检查"
		}
		// 【协议重构】修正PDU 0xE8/E5的包格式
		sktBytes := (sMaxSktIdx + 7) / 8
		pLen := sktBytes + 1

		write_buffer[2] = cmd
		write_buffer[3] = byte(pLen)
		write_buffer[4] = byte(SitesInfo[siteidx].Idx)

		_, newEnabled := GetNewEnable(getAutoMode(), sMaxSktIdx, SitesInfo[siteidx].Enabled, -1)
		copy(write_buffer[5:], newEnabled)

		sendLen = 5 + pLen
		cal_crc = util.CheckSumCRC8(write_buffer[0 : 4+pLen])
		write_buffer[4+pLen] = cal_crc

		log.Printf("[CMD]<====VAuto请求App执行%s, Site:%d, PLEN=%d, SKTEn=%X",
			pdu_name, SitesInfo[siteidx].Idx, pLen, newEnabled)

		if cmd == 0xE8 {
			SitesInfo[siteidx].SendElecCheckNum++
		} else {
			SitesInfo[siteidx].SendRemaCheckNum++
		}
	} else if cmd == 0xF0 {
		result_json := `{"Result":3,"Text":"1234","Uid":"890490321234512345678901235"}`
		result_json_len := len(result_json) + 1
		write_buffer[2] = 0xF0                  //AutoApp告诉App镭雕结果
		write_buffer[3] = byte(result_json_len) //data Len
		copy(write_buffer[4:], result_json)
		write_buffer[4+result_json_len] = util.CheckSumCRC8(write_buffer[0 : 4+result_json_len])
		sendLen = (5 + result_json_len)
		write_buffer[sendLen] = 0
		cal_crc = write_buffer[4+result_json_len]
	}

	n, err2 := conn.Write(write_buffer[0:sendLen])
	if err2 != nil {
		log.Printf("cli send error:%s\n", err2)
		// os.Exit(1) // 【问题修复】移除强制退出，避免闪退
		return // 替换为正常返回
	}

	log.Printf("[特殊站点]VAuto Send Cmd:0x%X to App len:%d Cmd:0x%X CRC:0x%X\n", cmd, n, write_buffer[0:22], cal_crc)

	// Make a buffer to hold incoming data.
	read_buffer := make([]byte, 128)
	// Read the incoming connection into the buffer.
	n, err = conn.Read(read_buffer)
	if err != nil {
		log.Printf("Error reading:%s\n", err.Error())
	}
	log.Printf("VAuto Read Cmd:0x%X to App len:%d Cmd: 0x%X\n", cmd, n, read_buffer[0:n])
	log.Printf("")

	// 【站点9芯片放置完成标志】如果是站点9的0xE6命令且收到正确ACK，设置完成标志
	if cmd == 0xE6 && SitesInfo[siteidx].Idx == 9 && n > 0 {
		// 检查ACK响应格式：应该是 5341E60100XX（PFLAG=SA, PDU=E6, PLEN=01, DATA=00, CRC）
		if n >= 6 && read_buffer[0] == 0x53 && read_buffer[1] == 0x41 &&
			read_buffer[2] == 0xE6 && read_buffer[3] == 0x01 && read_buffer[4] == 0x00 {
			gSite9ChipPlacementCompleted = true
			log.Printf("[AxisMove] 站点9芯片放置完成标志已设置 - 收到正确的0xE6 ACK响应")

			// 【修复】在确认0xE6 ACK后，在一个新的goroutine中独立发送ProductInfo命令
			go sendProductInfoCommand(SitesInfo[siteidx].Idx)

		} else {
			log.Printf("[AxisMove] 站点9的0xE6 ACK响应格式不正确: %02X %02X %02X %02X %02X",
				read_buffer[0], read_buffer[1], read_buffer[2], read_buffer[3], read_buffer[4])
		}
	}
}

// 【新增】在现有连接上发送ProductInfo的辅助函数
func sendProductInfoOnConnection(conn net.Conn, siteIdx int) {
	log.Printf("[ProductInfo] 开始为站点 %d 构建ProductInfo命令", siteIdx)

	var rotateInfo []RotateItem
	turntableInfo := SitesInfo[siteIdx-1]
	enabledMask := turntableInfo.Enabled

	for i := 0; i < sMaxSktIdx; i++ {
		var item RotateItem
		if (enabledMask>>i)&0x1 == 1 {
			// TODO: 实现动态来源追踪
			item = RotateItem{
				RotateIdx:   fmt.Sprintf("1-%d", i+1),
				FromUnitIdx: "1",
				FromSktIdx:  fmt.Sprintf("%d", i%4+1),
			}
		} else {
			item = RotateItem{
				RotateIdx:   fmt.Sprintf("1-%d", i+1),
				FromUnitIdx: "0",
				FromSktIdx:  "0",
			}
		}
		rotateInfo = append(rotateInfo, item)
	}

	productInfoCmd := ProductInfo{
		Command:    "ProductInfo",
		RotateInfo: rotateInfo,
	}

	jsonData, err := json.Marshal(productInfoCmd)
	if err != nil {
		log.Printf("[ProductInfo] JSON序列化失败: %v", err)
		return
	}

	log.Printf("[ProductInfo] 在现有连接上发送ProductInfo: %s", string(jsonData))
	_, err = conn.Write(jsonData)
	if err != nil {
		log.Printf("[ProductInfo] 发送ProductInfo失败: %v", err)
		return
	}

	// 等待并读取Tester的响应
	read_buffer := make([]byte, 256)
	n, err := conn.Read(read_buffer)
	if err != nil {
		log.Printf("[ProductInfo] 读取响应失败: %v", err)
		return
	}
	log.Printf("[ProductInfo] 收到ProductInfo响应: %s", string(read_buffer[:n]))
}

// 设置配置参数的公共方法（可选）
func SetAutoByteServerConfig(enable bool, serverHost string, serverPort string, jsonProtocol int) {
	if autoByteServerConfig == nil {
		initAutoByteServerConfig()
	}

	autoByteServerConfig.Enable = enable
	if serverHost != "" {
		autoByteServerConfig.ServerHost = serverHost
	}
	if serverPort != "" {
		autoByteServerConfig.ServerPort = serverPort
	}
	autoByteServerConfig.JsonProtocol = jsonProtocol
}

// 设置特殊站点配置的公共方法
func SetSpecialSiteConfig(specialSite int, specialSiteMaxSkt int) {
	if autoByteServerConfig == nil {
		initAutoByteServerConfig()
	}

	autoByteServerConfig.SpecialSite = specialSite
	autoByteServerConfig.SpecialSiteMaxSkt = specialSiteMaxSkt

	// 重新初始化特殊站点信息
	initSpecialSiteInfo()
}

func sendOneSyslogMessageContent(syslog_data []byte) {
	conn, err := net.Dial(CONN_TYPE, CONN_SYSLOG_HOST+":"+CONN_SYSLOG_PORT)
	if err != nil {
		//log.Printf("dial failed:", err)
		return
	}
	defer conn.Close()

	_, err = conn.Write(syslog_data)
	if err != nil {
		log.Printf("cli send syslog_data %s error:%s\n", string(syslog_data), err)
		return
	}
}

func sendAuthMessage(command string) bool {
	conn, err := net.Dial(CONN_TYPE, CONN_AUTH_HOST+":"+CONN_AUTH_PORT)
	if err != nil {
		log.Printf("dial failed:", err)
		return false
	}
	defer conn.Close()

	write_len := 20
	var write_buffer = make([]byte, write_len)
	copy(write_buffer[:], command)
	log.Printf("VAuto Auth Check Write Cmd: %s 0x%x\n", command, write_buffer)
	pKey := []byte{0x1B, 0xF1, 0xE3, 0x70, 0x66, 0x80, 0x5A, 0x01, 0x9A, 0xDE, 0x1F, 0x23, 0xCD, 0xA5, 0x37, 0x42, 0xE4, 0xC6, 0xCC, 0xB4}
	i := 0
	for i = 0; i < write_len; i++ {
		if i%2 == 0 {
			write_buffer[i] = (byte)(int(write_buffer[i]^pKey[i%20]) ^ int(i&0xFF) ^ int(i>>(i%4)))
		} else {
			write_buffer[i] = (byte)(int(write_buffer[i]^pKey[i%20]) ^ int(i&0xFF) ^ int(i<<(i%3)))
		}
	}
	_, err2 := conn.Write(write_buffer)
	if err2 != nil {
		log.Printf("cli send error:%s\n", err2)
		return false
	}
	// Make a buffer to hold incoming data.
	read_buffer := make([]byte, 512)
	// Read the incoming connection into the buffer.
	read_n, err := conn.Read(read_buffer)
	if err != nil {
		log.Printf("Error reading:%s\n", err.Error())
		return false
	}
	for i = 0; i < read_n; i++ {
		if i%2 == 0 {
			read_buffer[i] = (byte)(int(read_buffer[i]^pKey[i%20]) ^ int(i&0xFF) ^ int(i>>(i%4)))
		} else {
			read_buffer[i] = (byte)(int(read_buffer[i]^pKey[i%20]) ^ int(i&0xFF) ^ int(i<<(i%3)))
		}
	}
	if read_n > 4 {
		log.Printf("VAuto Auth Check Read len:%d Cmd: %s\n", read_n, string(read_buffer[4:read_n]))
	}
	if command == "GetReport" {
		log.Printf("VAuto Auth Check Read Cmd: %s Response OK\n", command)
		return true
	} else if command == "GetInfo" {
		var authInfo tAuthInfo
		err = json.Unmarshal([]byte(string(read_buffer[4:read_n-1])), &authInfo)
		if err != nil {
			log.Print(err)
			return false
		}
		if authInfo.FuncRet == "1" {
			log.Printf("VAuto Auth Check OK\n")
			return true
		} else {
			log.Printf("VAuto Auth Check Fail\n")
			return false
		}
	} else {
		log.Printf("UnSupport Auth Check Command\n")
		return false
	}
}

func isSupplyProgramFinished() bool {
	return sSupplyQuantity == (sProductOKQuantity + sProductNGQuantity)
}
func doExitTask() {
	if bLotEnd {
		log.Printf("烧录批次已经完成，不用再退出\n")
		return
	}
	if bDoingExit {
		log.Printf("批量正在结束过程中...\n")
		return
	}
	bDoingExit = true

	num := 0
	maxnum := configSystem.LotEndWaitMsec / 1000
	for num < maxnum {
		num++
		log.Printf("批量结束中%d-%d\n", num, maxnum)
		time.Sleep(time.Millisecond * time.Duration(1000))
	}

	if configSystem.DoRemingCheck {
		num = 0
		for {
			log.Printf("等待残料检测完成...\n")
			if num > 30 {
				break
			}
			num++
			time.Sleep(time.Millisecond * time.Duration(2000))

			bAllDone := true
			for siteidx := 0; siteidx < MAX_SITE_IDX; siteidx++ {
				if SitesInfo[siteidx].SendRemaCheckNum != SitesInfo[siteidx].RemaCheckNum {
					bAllDone = false
					continue
				}
			}
			if bAllDone {
				break
			}
		}
	}

	// 处理特殊站点剩余数量
	gSpecialSiteInfoMutex.RLock()
	enabled := gSpecialSiteInfo.Enabled
	pendingCount := gSpecialSiteInfo.PendingCount
	gSpecialSiteInfoMutex.RUnlock()

	if enabled && pendingCount > 0 {
		log.Printf("[特殊站点] 批次结束，处理剩余 %d 个特殊站点数量", pendingCount)
		ForceProcessSpecialSiteRemaining()

		// 等待特殊站点处理完成
		maxWaitTime := 30 // 最多等待30秒
		for maxWaitTime > 0 {
			gSpecialSiteInfoMutex.RLock()
			isProcessing := gSpecialSiteInfo.IsProcessing
			gSpecialSiteInfoMutex.RUnlock()

			if !isProcessing {
				break
			}

			log.Printf("[特殊站点] 等待特殊站点处理完成...")
			time.Sleep(time.Second)
			maxWaitTime--
		}
	}

	sendAuthMessage("GetReport")

	//写文件
	DumpFile()

	bDoingExit = false
	bRecvCmd4Supply = false
	bLotEnd = true
	gIsRecvCmdTskOK = false
	gIsHaveRecvPdu63 = false
	log.Printf("烧录结批完成\n")
	log.Printf("\n")
}

func getAutoMode() int {
	return gAutoMode
}

func dumpSocketPlacement(Idx int, newEnabled []byte) {
	if sMaxSktIdx == 8 {
		log.Printf("[CMD]<====VAuto告诉App Site:%d 芯片SKT01-8放置调整: 0x%X", Idx, newEnabled[0:8])
		return
	}
	if sMaxSktIdx >= 16 {
		log.Printf("[CMD]<====VAuto告诉App Site:%d 芯片SKT01-16放置调整: 0x%X", Idx, newEnabled[0:16])
	}
	if sMaxSktIdx >= 32 {
		log.Printf("[CMD]<====VAuto告诉App Site:%d 芯片SKT17-32放置调整: 0x%X", Idx, newEnabled[16:32])
	}
	if sMaxSktIdx >= 48 {
		log.Printf("[CMD]<====VAuto告诉App Site:%d 芯片SKT33-48放置调整: 0x%X", Idx, newEnabled[32:48])
	}
	if sMaxSktIdx >= 64 {
		log.Printf("[CMD]<====VAuto告诉App Site:%d 芯片SKT49-64放置调整: 0x%X", Idx, newEnabled[48:64])
	}
}

func sendAutoJsonMessage(siteidx int, json string) {
	conn, err := net.Dial(CONN_TYPE, CONN_CLIENT_HOST+":"+CONN_CLIENT_PORT)
	if err != nil {
		log.Printf("dial failed: %v", err)
		return
	}
	defer conn.Close()

	// 设置连接和读写超时，避免长时间阻塞
	conn.SetDeadline(time.Now().Add(2 * time.Second)) // 增加到2秒

	// 发送JSON数据
	jsonData := []byte(json)
	n, err := conn.Write(jsonData)
	if err != nil {
		log.Printf("cli send error: %v", err)
		return
	}

	log.Printf("VAuto Send JSON to Site:%d len:%d Cmd:%s", siteidx, n, json)

	// 【修复】改为同步读取响应。
	// 此函数通常在自己的goroutine中被调用，因此在这里进行同步读取是安全的，
	// 并且可以确保在读取完成前连接不会被关闭。
	read_buffer := make([]byte, 128)
	n, err = conn.Read(read_buffer)
	if err != nil {
		// 如果对方服务器没有按预期回复，这里会打印错误，例如 "read: i/o timeout"
		log.Printf("Error reading response: %v", err)
		return
	}

	log.Printf("VAuto Read JSON Response from Site:%d len:%d Data: %s", siteidx, n, string(read_buffer[:n]))
}

func sendAutoMessage(siteidx int, cmd byte) {
	conn, err := net.Dial(CONN_TYPE, CONN_CLIENT_HOST+":"+CONN_CLIENT_PORT)
	if err != nil {
		log.Print("dial failed:", err)
		return
	}
	defer conn.Close()

	write_buffer := make([]byte, 128)
	for i, _ := range write_buffer {
		write_buffer[i] = 0
	}
	binary.BigEndian.PutUint16(write_buffer[0:2], 0x4153)
	sendLen := 7
	var cal_crc byte = 0
	if cmd == 0xE6 {
		write_buffer[2] = 0xE6 //AutoApp告诉App芯片放置情况
		// 【协议标准化修复】按照标准协议规范，PLEN = 单个站点最大Socket数 + 1
		write_buffer[3] = byte(sMaxSktIdx + 1)         //data Len = Socket数 + 1个站点索引字节
		write_buffer[4] = byte(SitesInfo[siteidx].Idx) //站点
		addtional_offset := 0
		if getAutoMode() == AutoMode_AP8000V2_2 {
			addtional_offset = sMaxSktIdx - 1
			// V2.2模式PLEN已经在上面设置正确
		} else if getAutoMode() == AutoMode_AP8000 {
			// AP8000模式：8个Socket + 1个站点索引 = 9字节数据
			// PLEN已经在上面设置为sMaxSktIdx + 1 = 8 + 1 = 9
			addtional_offset = sMaxSktIdx - 1 // 7个额外的Socket字节
		}
		realSupplyNum := 0
		if configSystem.DoSupplyCheck {
			remainNum := sStopQuantity - sSupplyQuantity
			if remainNum <= 0 || (bRecvCmd4LotEnd && isSupplyProgramFinished()) {
				log.Printf("完成此次批量 SupplyQuantity:%d StopQuantity:%d RecvCmd4LotEnd:%t\n", sSupplyQuantity, sStopQuantity, bRecvCmd4LotEnd)
				go doExitTask()
				return
			}
			curSupplyNum := util.GetByteEnableCnt(SitesInfo[siteidx].Enabled, sMaxSktIdx)
			log.Printf("[CMD]<====VAuto告诉App Site:%d 芯片放置开始 TotalSupply:%d Remain:%d PrepareSupply:%d", SitesInfo[siteidx].Idx,
				sSupplyQuantity, remainNum, curSupplyNum)
			if curSupplyNum > remainNum {
				realSupplyNum = curSupplyNum
				supply_count, newEnabled := GetNewEnable(getAutoMode(), sMaxSktIdx, SitesInfo[siteidx].Enabled, remainNum)
				realSupplyNum = supply_count
				if getAutoMode() == AutoMode_AP8000V2_2 {
					copy(write_buffer[5:], newEnabled)
					dumpSocketPlacement(SitesInfo[siteidx].Idx, newEnabled)
				} else if getAutoMode() == AutoMode_AP8000 {
					// 【协议标准化修复】填充8个Socket字节，符合协议规范
					for i := 0; i < sMaxSktIdx; i++ {
						if i < len(newEnabled) {
							write_buffer[5+i] = newEnabled[i]
						} else {
							write_buffer[5+i] = 0x00
						}
					}
					log.Printf("[CMD]<====VAuto告诉App Site:%d 芯片SKT1-8放置: %02X %02X %02X %02X %02X %02X %02X %02X",
						SitesInfo[siteidx].Idx,
						write_buffer[5], write_buffer[6], write_buffer[7], write_buffer[8],
						write_buffer[9], write_buffer[10], write_buffer[11], write_buffer[12])
				} else {
					log.Printf("[CMD]vauto不支持此座子数%d", sMaxSktIdx)
				}
			} else {
				supply_count, newEnabled := GetNewEnable(getAutoMode(), sMaxSktIdx, SitesInfo[siteidx].Enabled, -1)
				realSupplyNum = supply_count
				if getAutoMode() == AutoMode_AP8000V2_2 {
					copy(write_buffer[5:], newEnabled)
					dumpSocketPlacement(SitesInfo[siteidx].Idx, newEnabled)
				} else if getAutoMode() == AutoMode_AP8000 {
					write_buffer[5] = byte(SitesInfo[siteidx].Enabled)
					log.Printf("[CMD]<====VAuto告诉App Site:%d 芯片SKT1-8放置: 0x%X", SitesInfo[siteidx].Idx, write_buffer[5])
				} else {
					log.Printf("[CMD]vauto不支持此座子数%d", sMaxSktIdx)
				}

			}
			sSupplyQuantity += realSupplyNum
			remainNum -= realSupplyNum
			log.Printf("[CMD]<====VAuto告诉App Site:%d 芯片放置结果 TotalSupply:%d Remain:%d PrepareSupply:%d",
				SitesInfo[siteidx].Idx, sSupplyQuantity, remainNum, curSupplyNum)

		} else {
			supply_count, newEnabled := GetNewEnable(getAutoMode(), sMaxSktIdx, SitesInfo[siteidx].Enabled, -1)
			realSupplyNum = supply_count
			if getAutoMode() == AutoMode_AP8000V2_2 {
				copy(write_buffer[5:], newEnabled)
			} else if getAutoMode() == AutoMode_AP8000 {
				// 【协议标准化修复】填充8个Socket字节，符合协议规范
				for i := 0; i < sMaxSktIdx; i++ {
					if i < len(newEnabled) {
						write_buffer[5+i] = newEnabled[i]
					} else {
						write_buffer[5+i] = 0x00
					}
				}
			} else {
				write_buffer[5+addtional_offset] = byte(SitesInfo[siteidx].Enabled) //SKT1-8使能情况
			}
			sSupplyQuantity += realSupplyNum
			log.Printf("[CMD]<====VAuto告诉App Site:%d 芯片放置结果 TotalSupply:%d", SitesInfo[siteidx].Idx, sSupplyQuantity)
		}

		write_buffer[6+addtional_offset] = util.CheckSumCRC8(write_buffer[0 : 6+addtional_offset])
		sendLen = 7 + addtional_offset

		cal_crc = write_buffer[6+addtional_offset]
	} else if cmd == 0xE4 {
		write_buffer[2] = 0xE4                             //AutoApp告诉App站点设置使能情况
		write_buffer[3] = 2                                //data Len
		write_buffer[4] = byte(SitesInfo[siteidx].Idx)     //站点OK
		write_buffer[5] = byte(SitesInfo[siteidx].Enabled) //SKT1-8使能情况
		write_buffer[6] = util.CheckSumCRC8(write_buffer[0:6])
		sendLen = 7
		cal_crc = write_buffer[6]
		log.Printf("[CMD]<====VAuto告诉App站点设置使能情况")
	} else if cmd == 0xE2 {
		write_buffer[2] = 0xE2 //AutoApp告知App当前支持的协议版本号
		write_buffer[3] = 2    //data Len
		write_buffer[4] = 0x02 //
		write_buffer[5] = 0x07 //V2.7
		write_buffer[6] = util.CheckSumCRC8(write_buffer[0:6])
		sendLen = 7
		cal_crc = write_buffer[6]
		log.Printf("[CMD]<====VAuto告知App当前支持的协议版本号")
	} else if cmd == 0xE1 {
		write_buffer[2] = 0xE1 //AutoApp请求StdMes软件版本信息
		write_buffer[3] = 0    //data Len
		write_buffer[4] = util.CheckSumCRC8(write_buffer[0:6])
		sendLen = 5
		cal_crc = write_buffer[4]
		log.Printf("[CMD]<====VAuto请求App软件协议版本号")
	} else if cmd == 0xE8 || cmd == 0xE5 {
		pdu_name := "接触检查"
		if cmd == 0xE5 {
			pdu_name = "残料检查"
		}
		// 【协议重构】修正PDU 0xE8/E5的包格式
		sktBytes := (sMaxSktIdx + 7) / 8
		pLen := sktBytes + 1

		write_buffer[2] = cmd
		write_buffer[3] = byte(pLen)
		write_buffer[4] = byte(SitesInfo[siteidx].Idx)

		_, newEnabled := GetNewEnable(getAutoMode(), sMaxSktIdx, SitesInfo[siteidx].Enabled, -1)
		copy(write_buffer[5:], newEnabled)

		sendLen = 5 + pLen
		cal_crc = util.CheckSumCRC8(write_buffer[0 : 4+pLen])
		write_buffer[4+pLen] = cal_crc

		log.Printf("[CMD]<====VAuto请求App执行%s, Site:%d, PLEN=%d, SKTEn=%X",
			pdu_name, SitesInfo[siteidx].Idx, pLen, newEnabled)

		if cmd == 0xE8 {
			SitesInfo[siteidx].SendElecCheckNum++
		} else {
			SitesInfo[siteidx].SendRemaCheckNum++
		}
	} else if cmd == 0xF0 {
		result_json := `{"Result":3,"Text":"1234","Uid":"890490321234512345678901235"}`
		result_json_len := len(result_json) + 1
		write_buffer[2] = 0xF0                  //AutoApp告诉App镭雕结果
		write_buffer[3] = byte(result_json_len) //data Len
		copy(write_buffer[4:], result_json)
		write_buffer[4+result_json_len] = util.CheckSumCRC8(write_buffer[0 : 4+result_json_len])
		sendLen = (5 + result_json_len)
		write_buffer[sendLen] = 0
		cal_crc = write_buffer[4+result_json_len]
	}
	//log.Printf("Current Site:%d sendLen:%d\r\n", byte(SitesInfo[siteidx].Idx), sendLen)

	n, err2 := conn.Write(write_buffer[0:sendLen])
	if err2 != nil {
		log.Printf("cli send error:%s\n", err2)
		// os.Exit(1) // 【问题修复】移除强制退出，避免闪退
		return // 替换为正常返回
	}
	if cmd == 0xF0 {
		log.Printf("VAuto Send Cmd:0x%X to App len:%d Cmd:%s\n", cmd, n, string(write_buffer[4:n]))
	} else {
		log.Printf("VAuto Send Cmd:0x%X to App len:%d Cmd:0x%X CRC:0x%X\n", cmd, n, write_buffer[0:22], cal_crc)
	}

	// Make a buffer to hold incoming data.
	read_buffer := make([]byte, 128)
	// Read the incoming connection into the buffer.
	n, err = conn.Read(read_buffer)
	if err != nil {
		log.Printf("Error reading:%s\n", err.Error())
	}
	log.Printf("VAuto Read Cmd:0x%X to App len:%d Cmd: 0x%X\n", cmd, n, read_buffer[0:n])
	log.Printf("")

	// 【站点9芯片放置完成标志】如果是站点9的0xE6命令且收到正确ACK，设置完成标志
	if cmd == 0xE6 && SitesInfo[siteidx].Idx == 9 && n > 0 {
		// 检查ACK响应格式：应该是 5341E60100XX（PFLAG=SA, PDU=E6, PLEN=01, DATA=00, CRC）
		if n >= 6 && read_buffer[0] == 0x53 && read_buffer[1] == 0x41 &&
			read_buffer[2] == 0xE6 && read_buffer[3] == 0x01 && read_buffer[4] == 0x00 {
			gSite9ChipPlacementCompleted = true
			log.Printf("[AxisMove] 站点9芯片放置完成标志已设置 - 收到正确的0xE6 ACK响应")

			// 【修复】在确认0xE6 ACK后，在一个新的goroutine中独立发送ProductInfo命令
			go sendProductInfoCommand(SitesInfo[siteidx].Idx)

		} else {
			log.Printf("[AxisMove] 站点9的0xE6 ACK响应格式不正确: %02X %02X %02X %02X %02X",
				read_buffer[0], read_buffer[1], read_buffer[2], read_buffer[3], read_buffer[4])
		}
	}
}

// 【新增】发送ProductInfo的辅助函数
func sendProductInfoAfterPlacement(turntableSiteIdx int, placementMask []byte) {
	log.Printf("[ProductInfo] 开始为站点 %d 构建ProductInfo命令", turntableSiteIdx+1)

	// 这里需要一个逻辑来确定每个新放置到转台上的产品的来源。
	// 这个逻辑比较复杂，因为它依赖于之前各个站点的状态。
	// 目前，我们先实现一个简化的版本，发送一个固定的、用于演示的ProductInfo。
	// TODO: 实现根据vauto内部状态动态构建ProductInfo的逻辑。

	var rotateInfo []RotateItem
	// 假设转台有16个插槽
	for i := 0; i < 16; i++ {
		// 检查placementMask的第i位是否为1
		isPlaced := false
		if i/8 < len(placementMask) {
			if (placementMask[i/8]>>(i%8))&0x1 == 1 {
				isPlaced = true
			}
		}

		var item RotateItem
		if isPlaced {
			// 这是一个示例，实际来源需要从其他地方获取
			item = RotateItem{
				RotateIdx:   fmt.Sprintf("1-%d", i+1),
				FromUnitIdx: "1",                      // 示例：假设都来自1号站点
				FromSktIdx:  fmt.Sprintf("%d", i%4+1), // 示例：循环使用1-4号插座
			}
		} else {
			item = RotateItem{
				RotateIdx:   fmt.Sprintf("1-%d", i+1),
				FromUnitIdx: "0",
				FromSktIdx:  "0",
			}
		}
		rotateInfo = append(rotateInfo, item)
	}

	productInfoCmd := ProductInfo{
		Command:    "ProductInfo",
		RotateInfo: rotateInfo,
	}

	jsonData, err := json.Marshal(productInfoCmd)
	if err != nil {
		log.Printf("[ProductInfo] JSON序列化失败: %v", err)
		return
	}

	log.Printf("[ProductInfo] 发送ProductInfo命令: %s", string(jsonData))
	// 使用sendAutoJsonMessage发送，因为它会连接到正确的端口
	sendAutoJsonMessage(turntableSiteIdx+1, string(jsonData))
}

// 【新增】构建并独立发送ProductInfo命令的辅助函数
func sendProductInfoCommand(siteIdx int) {
	log.Printf("[ProductInfo] 开始为站点 %d 构建ProductInfo命令", siteIdx)

	var rotateInfo []RotateItem
	turntableInfo := SitesInfo[siteIdx-1]
	enabledMask := turntableInfo.Enabled

	for i := 0; i < sMaxSktIdx; i++ {
		var item RotateItem
		if (enabledMask>>i)&0x1 == 1 {
			// TODO: 实现动态来源追踪
			item = RotateItem{
				RotateIdx:   fmt.Sprintf("1-%d", i+1),
				FromUnitIdx: "1",
				FromSktIdx:  fmt.Sprintf("%d", i%4+1),
			}
		} else {
			item = RotateItem{
				RotateIdx:   fmt.Sprintf("1-%d", i+1),
				FromUnitIdx: "0",
				FromSktIdx:  "0",
			}
		}
		rotateInfo = append(rotateInfo, item)
	}

	productInfoCmd := ProductInfo{
		Command:    "ProductInfo",
		RotateInfo: rotateInfo,
	}

	jsonData, err := json.Marshal(productInfoCmd)
	if err != nil {
		log.Printf("[ProductInfo] JSON序列化失败: %v", err)
		return
	}

	log.Printf("[ProductInfo] 准备发送ProductInfo命令...")
	// 此函数会处理独立的“连接-发送-接收-关闭”流程
	sendAutoJsonMessage(siteIdx, string(jsonData))
}

// 启动自动化字节服务器 - 简化的对外接口
func DoAutoByteServer() {
	glog.Info("正在启动自动化字节服务器模块...")

	gIsRecvCmdTskOK = false
	// 加载配置文件
	if err := loadAutoByteServerConfig(); err != nil {
		glog.Warningf("加载自动化字节服务器配置失败，使用默认配置: %v", err)
	}

	// 检查是否启用
	if !autoByteServerConfig.Enable {
		glog.Info("自动化字节服务器模块未启用，跳过启动")
		return
	}

	glog.Infof("自动化字节服务器模块启动成功，监听地址: %s:%s",
		autoByteServerConfig.ServerHost, autoByteServerConfig.ServerPort)

	// 启动TCP服务器
	doTcpServer()
}

func doTcpServer() {
	// Listen for incoming connections.
	l, err := net.Listen(CONN_TYPE, autoByteServerConfig.ServerHost+":"+autoByteServerConfig.ServerPort)
	if err != nil {
		log.Printf("Error listening:%s\n", err.Error())
		// os.Exit(1) // 【问题修复】移除强制退出，避免闪退
		return // 替换为正常返回
	}
	// Close the listener when the application closes.
	defer l.Close()
	log.Printf("Listening on " + autoByteServerConfig.ServerHost + ":" + autoByteServerConfig.ServerPort)
	log.Printf("")
	for {
		// Listen for an incoming connection.
		conn, err := l.Accept()
		if err != nil {
			log.Printf("Error accepting: %s\n", err.Error())
			// os.Exit(1) // 【问题修复】移除强制退出，避免闪退
			continue // 继续接受下一个连接
		}
		go handleByteRequest(conn)
	}
}

// Handles incoming requests.
func handleByteRequest(conn net.Conn) {
	// Make a buffer to hold incoming data.
	buf := make([]byte, 1024)
	// Read the incoming connection into the buffer.
	recvLen, err := conn.Read(buf)
	if err != nil {
		log.Printf("Error reading:%s\n", err.Error())
	}

	defer conn.Close()

	// Add a small delay at the end of processing to ensure the client has time to read the response
	// before the deferred conn.Close() is executed. This helps mitigate a race condition.
	defer time.Sleep(10 * time.Millisecond)

	if gSpecialSiteInfo.Enabled && buf[0] == '{' && buf[recvLen-1] == '}' {
		jsonData := string(buf[0:recvLen])
		log.Printf("VAuto Read Json to App len:%d Cmd: %s\n", recvLen, jsonData)

		// 解析JSON命令
		var autoCmd AutoJsonCmd
		err := json.Unmarshal(buf[0:recvLen], &autoCmd)
		if err != nil {
			log.Printf("JSON解析失败: %v, 原始数据: %s", err, jsonData)
		} else {
			log.Printf("解析JSON命令成功: Command=%s, AxisSelect=%s, SiteIdx=%s, TargetAngle=%s",
				autoCmd.Command, autoCmd.AxisSelect, autoCmd.SiteIdx, autoCmd.TargetAngle)
		}
		if autoCmd.Command == "AxisMoveComplete" {
			return
		}
		// 立即发送响应
		response := AutoJsonResponse{
			Command: "AxisMove",
			Result:  "0",
			ErrMsg:  "",
		}
		responseData, err := json.Marshal(response)
		if err != nil {
			log.Printf("JSON序列化失败: %v", err)
			return
		}

		_, err = conn.Write(responseData)
		if err != nil {
			log.Printf("发送响应失败: %v", err)
			return
		}
		log.Printf("VAuto Send JSON Response to App len:%d Cmd: %s\n", len(responseData), string(responseData))

		// 【立即处理轴移动命令】- 确保3秒内完成响应
		go func() {
			if autoCmd.Command == "AxisMove" {
				siteIdx, _ := strconv.Atoi(autoCmd.SiteIdx)
				log.Printf("[AxisMove] 收到轴移动命令: %s, 站点: %d, 目标角度: %s",
					autoCmd.AxisSelect, siteIdx, autoCmd.TargetAngle)

				// 【移除芯片放置等待逻辑】- 立即执行轴移动，不等待芯片放置
				// 原有的芯片放置等待逻辑可能导致30秒延迟，现在直接执行

				log.Printf("[AxisMove] 立即开始执行轴移动: %s, 站点: %d, 目标角度: %s",
					autoCmd.AxisSelect, siteIdx, autoCmd.TargetAngle)

				// 【精确3秒轴移动时间】- 使用独立goroutine和channel确保时序
				done := make(chan bool, 1)

				// 启动独立定时器
				go func() {
					timer := time.NewTimer(3 * time.Second)
					<-timer.C
					done <- true
					timer.Stop()
					log.Printf("[AxisMove] 3秒轴移动时间完成")
				}()

				// 等待3秒完成
				<-done

				// 构建完成响应
				completeResponse := AutoJsonResponse{
					Command:      "AxisMoveComplete",
					AxisSelect:   autoCmd.AxisSelect,
					SiteIdx:      autoCmd.SiteIdx,
					CurrentAngle: autoCmd.TargetAngle,
				}

				log.Printf("[AxisMove] 轴移动执行完成: %s, 站点: %d, 当前角度: %s",
					autoCmd.AxisSelect, siteIdx, autoCmd.TargetAngle)

				completeData, err := json.Marshal(completeResponse)
				if err != nil {
					log.Printf("JSON序列化失败: %v", err)
					return
				}

				// 【异步发送完成信号】避免发送操作阻塞
				go func() {
					startTime := time.Now()
					sendAutoJsonMessage(gSpecialSiteInfo.SiteIdx, string(completeData))
					elapsed := time.Since(startTime)
					log.Printf("[AxisMove] 轴移动完成信号发送耗时: %v", elapsed)
				}()
			}
		}()

		return
	}

	if buf[0] != 0x53 || buf[1] != 0x41 {
		// 旧的、令人困惑的日志，将被移除
		// log.Printf("VAuto Read Cmd:0x%X to App len:%d Cmd: 0x%X\n", buf[0], buf[1], buf[2:10])
		return
	}

	//pFlag := binary.BigEndian.Uint16(buf[0:2])
	pPdu := buf[2]
	var pLen uint16 = 0
	var pCRC byte
	var pData []byte
	if pPdu == 0x70 {
		pLen = binary.LittleEndian.Uint16(buf[3:5])
		pData = buf[5 : 5+pLen]
	} else {
		pLen = uint16(buf[3])
		pData = buf[4 : 4+pLen]
		pCRC = buf[4+pLen]
	}

	// 添加CRC8校验
	expectCRC := util.CheckSumCRC8(buf[0 : 4+pLen])
	if pPdu != 0x70 && expectCRC != pCRC {
		log.Printf("\n")
		log.Printf("***CRC Check Failed. Calculated: %X, Received: %X, please Check Your Data***\n", expectCRC, pCRC)
		return
	}

	if pLen+5 != uint16(recvLen) {
		log.Printf("\n")
		log.Printf("***Data Length Check Failed. Calculated: %d, Received: %d, please Check Your Data***\n", pLen+5, recvLen)
		return
	}

	if pPdu == 0x61 { //App告知AutoApp当前支持的通信协议版本号
		log.Printf("App Tell AutoApp Support Protocal Version: %X.", pData[0])
		if pData[0] < AutoMode_AP8000 && pData[0] > AutoMode_AP8000V2_3 {
			log.Printf("App Tell AutoApp UnSupport Protocal Version: %X.", pData[0])
		} else {
			gAutoMode = int(pData[0])
		}

		// 【协议修复】为PDU 0x61添加缺失的ACK响应
		var write_buffer = make([]byte, 10)
		binary.BigEndian.PutUint16(write_buffer[0:2], 0x4153) // PFLAG "AS"
		write_buffer[2] = 0x61                                // PDU
		write_buffer[3] = 1                                   // PLEN
		write_buffer[4] = 0                                   // ErrCode 0: Success
		write_buffer[5] = util.CheckSumCRC8(write_buffer[0:5])
		_, err2 := conn.Write(write_buffer[0:6])
		if err2 != nil {
			log.Printf("VAuto Send 0x61 Ack error:%s\n", err2)
		} else {
			log.Printf("VAuto Send 0x61 Ack OK")
		}
	} else if pPdu == 0x63 { // App告知AutoApp初始化情况
		handlePDU63(conn, pData)
	} else if pPdu == 0x67 {
		siteIdx := int(pData[0])
		resultData := pData[1:]
		successCount := 0
		failureCount := 0

		log.Printf("[PDU 0x67] 处理站点 %d 的烧录结果，数据长度: %d", siteIdx, len(resultData))

		if getAutoMode() == AutoMode_AP8000 {
			// 【协议标准化更新】支持标准协议格式：每个Socket一个字节的结果码
			// 检查数据长度来判断是新格式还是旧格式
			if len(resultData) == 2 {
				// 旧格式：位掩码格式（兼容性支持）
				if len(resultData) < 2 {
					log.Printf("[PDU 0x67] 旧格式数据不足，需要2字节，实际: %d", len(resultData))
					return
				}

				result := resultData[1]
				sktIdx := 0
				onebyte := byte(SitesInfo[siteIdx-1].Enabled)

				for sktIdx < sMaxSktIdx {
					if result>>sktIdx&0x1 == 1 && (onebyte>>sktIdx&0x1 == 1) {
						sProductOKQuantity++
						successCount++
					}
					//SKT使能了结果却返回0
					if (result>>sktIdx&0x1) == 0 && (onebyte>>sktIdx&0x1 == 1) {
						sProductNGQuantity++
						failureCount++
					}
					sktIdx++
				}
				log.Printf("[PDU 0x67] 旧格式处理完成 Site:%d Result:0x%02x", siteIdx, result)
			} else {
				// 【BUG修复】修正了对新协议格式的处理逻辑，特别是索引和使能掩码的检查
				sktIdx := 0
				// 确定要处理的Socket数量，取站点最大Socket数和实际收到的结果数中的较小者
				maxSktToProcess := sMaxSktIdx
				if len(resultData) < maxSktToProcess {
					maxSktToProcess = len(resultData)
					log.Printf("[PDU 0x67] 接收到的结果数据 %d 少于站点最大Socket数 %d，将按实际数量处理", len(resultData), sMaxSktIdx)
				}

				enabledMask := SitesInfo[siteIdx-1].Enabled

				for sktIdx < maxSktToProcess {
					// 【BUG修复】正确检查64位使能掩码中的对应位
					isSocketEnabled := (enabledMask>>sktIdx)&0x1 == 1

					// 【BUG修复】使用正确的索引访问结果数据
					socketResult := resultData[sktIdx]

					if socketResult == 0x01 && isSocketEnabled { // 0x01 = Bin1(PASS)
						sProductOKQuantity++
						successCount++
					} else if socketResult != 0x01 && socketResult != 0x00 && isSocketEnabled {
						//SKT使能了但结果不是PASS (也不是0x00空槽)
						sProductNGQuantity++
						failureCount++
					}
					sktIdx++
				}

				// 安全地打印结果数据
				endIdx := maxSktToProcess
				if endIdx > len(resultData) {
					endIdx = len(resultData)
				}
				log.Printf("[PDU 0x67] 新格式处理完成 Site:%d Result:0x%x", siteIdx, resultData[:endIdx])
			}
			log.Printf("[PDU 0x67] 站点 %d 统计完成，成功: %d，失败: %d，累计成功: %d，累计失败: %d",
				siteIdx, successCount, failureCount, sProductOKQuantity, sProductNGQuantity)
		} else if getAutoMode() == AutoMode_AP8000V2_2 {
			// V2.2版本已经使用标准格式，需要安全处理
			if len(resultData) < 2 {
				log.Printf("[PDU 0x67] V2.2格式数据不足，至少需要2字节，实际: %d", len(resultData))
				return
			}

			sktIdx := 0
			enabledBytes := util.ToBytes(SitesInfo[siteIdx-1].Enabled)

			for sktIdx < sMaxSktIdx {
				// 检查数据和使能字节数组边界
				if 1+sktIdx >= len(resultData) {
					log.Printf("[PDU 0x67] V2.2数据索引越界，停止处理。sktIdx: %d", sktIdx)
					break
				}
				if sktIdx/8 >= len(enabledBytes) {
					log.Printf("[PDU 0x67] 使能字节数组索引越界，停止处理。sktIdx: %d", sktIdx)
					break
				}

				onebyte := enabledBytes[sktIdx/8]
				result := resultData[1+sktIdx]
				if result == 0x1 && (onebyte>>(sktIdx%8)&0x1 == 1) {
					sProductOKQuantity++
					successCount++
				}
				//SKT使能了结果却返回0
				if result >= 0x2 && onebyte>>(sktIdx%8)&0x1 == 1 {
					sProductNGQuantity++
					failureCount++
				}
				sktIdx++
			}

			// 安全地打印结果
			endIdx := 17
			if endIdx > len(resultData) {
				endIdx = len(resultData)
			}
			log.Printf("[PDU 0x67] V2.2处理完成 Site:%d Result:0x%x", siteIdx, resultData[1:endIdx])
			log.Printf("[PDU 0x67] V2.2站点 %d 统计完成，成功: %d，失败: %d", siteIdx, sProductOKQuantity, sProductNGQuantity)
		} else {
			log.Printf("[PDU 0x67] 不支持的AutoMode: %d，Socket数: %d", getAutoMode(), sMaxSktIdx)
			return
		}

		// 安全地调用特殊站点处理，使用额外的保护层
		func() {
			defer func() {
				if r := recover(); r != nil {
					log.Printf("[PDU 0x67] 特殊站点处理调用发生panic，已恢复: %v", r)
				}
			}()

			log.Printf("[PDU 0x67] 准备调用特殊站点处理，站点: %d，成功: %d，失败: %d",
				siteIdx, successCount, failureCount)
			checkSpecialSiteProcessing(siteIdx, successCount, failureCount, resultData)
		}()

		// 响应ACK
		var write_buffer = make([]byte, 10)
		binary.BigEndian.PutUint16(write_buffer[0:2], 0x4153) // PFLAG "AS"
		write_buffer[2] = 0x67                                // PDU
		write_buffer[3] = 1                                   // PLEN
		write_buffer[4] = 0                                   // ErrCode 0: Success
		write_buffer[5] = util.CheckSumCRC8(write_buffer[0:5])
		_, err2 := conn.Write(write_buffer[0:6])
		if err2 != nil {
			log.Printf("VAuto Send 0x67 Ack error:%s\n", err2)
		} else {
			log.Printf("VAuto Send 0x67 Ack OK")
		}
	} else if pPdu == 0x68 { // App告知InsertionCheck命令执行结果
		siteidx := 0
		SKTCnt := 0
		if getAutoMode() == AutoMode_AP8000V2_2 {
			siteidx = int(pData[0]) - 1
			SKTCnt = int(pData[1])
		} else if getAutoMode() == AutoMode_AP8000 {
			siteidx = int(pData[0]) - 1 //0-8
		}
		SitesInfo[siteidx].ElecChk = pData[2 : 2+SKTCnt]
		SitesInfo[siteidx].ElecCheckNum++
		log.Printf("SiteIdx:0x%02x ElecChk:%x.\r\n", byte(SitesInfo[siteidx].Idx), SitesInfo[siteidx].ElecChk)
		log.Printf("[CMD]====>App Tell IC InsertCheck SiteIdx:%d SKTCnt:%d", byte(SitesInfo[siteidx].Idx), SKTCnt)
		if configSystem.DoElecCheck {
			log.Printf("等待放置芯片中%d毫秒...\r\n", configSystem.ResultWaitMsec)
			log.Printf("\n")
			time.Sleep(time.Millisecond * time.Duration(configSystem.ResultWaitMsec))
			if SitesInfo[siteidx].Enabled != 0x0 {
				sendAutoMessage(siteidx, 0xE6)
			}
		}
	} else if pPdu == 0x65 { // App告知残料检查功能判断是否有IC放入命令执行结果
		siteidx := 0
		SKTCnt := 0
		if getAutoMode() == AutoMode_AP8000V2_2 {
			siteidx = int(pData[0]) - 1
			SKTCnt = int(pData[1])
		} else if getAutoMode() == AutoMode_AP8000 {
			siteidx = int(pData[0]) - 1 //0-8
		}
		SitesInfo[siteidx].RemaChk = pData[2 : 2+SKTCnt]
		SitesInfo[siteidx].RemaCheckNum++
		log.Printf("SiteIdx:0x%02x ElecChk:%x.\r\n", byte(SitesInfo[siteidx].Idx), SitesInfo[siteidx].ElecChk)
		log.Printf("[CMD]====>App Tell IC Remaining Status SiteIdx:%d SKTCnt:%d", byte(SitesInfo[siteidx].Idx), SKTCnt)
		if configSystem.DoRemingCheck {
			log.Printf("等待放置芯片中%d毫秒...\r\n", configSystem.ResultWaitMsec)
			log.Printf("\n")
			time.Sleep(time.Millisecond * time.Duration(configSystem.ResultWaitMsec))
			if SitesInfo[siteidx].Enabled != 0x0 {
				sendAutoMessage(siteidx, 0xE6)
			}
		}
	} else if pPdu == 0x69 { // App告知AutoApp芯片烧录结果带错误码
		siteidx := int(pData[0])
		result := pData[1]
		sktIdx := 0
		for sktIdx < sMaxSktIdx {
			onebyte := util.ToBytes(SitesInfo[siteidx-1].Enabled)[sktIdx/8]
			siteresult := binary.LittleEndian.Uint32(pData[2+4*sktIdx : 6+4*sktIdx])
			log.Printf("[CMD]====>App通知IC烧录结果 siteidx:%d sktIdx:%d 错误码:%x", siteidx, sktIdx+1, siteresult)
			if result>>sktIdx&0x1 == 1 && (onebyte>>sktIdx&0x1 == 1) {
				sProductOKQuantity++
			}
			//SKT使能了结果却返回0
			if (result>>sktIdx&0x1) == 0 && (onebyte>>sktIdx&0x1 == 1) {
				sProductNGQuantity++
			}
			sktIdx++
		}
		log.Printf("[CMD]====>App通知IC烧录结果带错误码 siteidx:%d result:0x%02x 烧录成功数量:%d 失败数量:%d", siteidx, result, sProductOKQuantity, sProductNGQuantity)
	} else if pPdu == 0x70 { //App告知AutoApp需要打印的内容
		log.Printf("[CMD]App Tell AutoApp Laser Print.")
	} else {
		log.Printf("Unknown Cmd: 0x%x.\n", pPdu)
	}
	if pPdu != 0x70 {
		log.Printf("Recv App Cmd Pdu:0x%X Len:%d CRC:0x%X Data:0x%X\n", pPdu, pLen, pCRC, pData[0:18])
	} else {
		log.Printf("Recv App Cmd Pdu:0x%X Len:%d CRC:0x%X Data:%s\n", pPdu, pLen, pCRC, string(pData))
	}

	sendbuf := make([]byte, 128)
	binary.BigEndian.PutUint16(sendbuf[0:2], 0x4153)
	sendbuf[2] = pPdu
	sendbuf[3] = 1    //data Len
	sendbuf[4] = 0x00 //OK
	sendbuf[5] = util.CheckSumCRC8(sendbuf[0:5])
	conn.Write(sendbuf[0:6])
	log.Printf("\r\n")
	if pPdu == 0x63 {
		go func() {
			time.Sleep(time.Millisecond * time.Duration(configSystem.EnabledWaitMsec))
			num := 0
			log.Printf("Wait Cmd3 tsk file to Recv...\n")
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
					have_enabled := false
					for num < sMaxSiteIdx {
						// if site is enabled and do elec check, send 0xE8 first
						if SitesInfo[num].Enabled != 0x0 {
							have_enabled = true
							if configSystem.DoElecCheck {
								for num < sMaxSiteIdx {
									if SitesInfo[num].Enabled != 0x0 {
										sendAutoMessage(num, 0xE8)
									}
									num++
								}
							} else {
								sendAutoMessage(num, 0xE6)
							}

						}
						num++
					}
					if !have_enabled {
						log.Printf("没有找到使能的站点,请检查stdmes.ini站点映射...\r\n")
						log.Printf("\n")
					}
				}
			} else if !configSystem.WaitCmd3 {
				//等待接收0x68完成
				//log.Printf("AutoApp等待接收App指令0x68\r\n")
				time.Sleep(time.Millisecond * 3000)
				num = 0
				have_enabled := false
				for num < sMaxSiteIdx {
					if SitesInfo[num].Enabled != 0x0 {
						have_enabled = true
						sendAutoMessage(num, 0xE6)
					}
					num++
				}
				if !have_enabled {
					log.Printf("没有找到使能的站点,请检查stdmes.ini站点映射...\r\n")
					log.Printf("\n")
				}
			}

		}()
	} else if pPdu == 0x67 || pPdu == 0x69 {
		go func() {
			if configSystem.DoRemingCheck {
				num := 0
				for num < sMaxSiteIdx {
					if SitesInfo[num].Enabled != 0x0 {
						sendAutoMessage(num, 0xE5)
					}
					num++
				}
			} else {
				log.Printf("等待放置芯片中%d毫秒...\r\n", configSystem.ResultWaitMsec)
				log.Printf("\n")
				time.Sleep(time.Millisecond * time.Duration(configSystem.ResultWaitMsec))
				siteidx := int(pData[0]) - 1 //0-8
				if SitesInfo[siteidx].Enabled != 0x0 {
					sendAutoMessage(siteidx, 0xE6)
				}
			}

		}()
	} else if pPdu == 0x70 {
		go func() {
			time.Sleep(time.Millisecond * time.Duration(configSystem.ResultWaitMsec))
			var printResult tPrintResult
			err1 := json.Unmarshal(pData[0:pLen], &printResult)
			if err1 != nil {
				log.Printf("Unmarshal error %s\n", err1.Error())
			}
			siteidx := printResult.SiteIdx - 1
			log.Printf("VAuto Tell Print SiteIdx:%d SKT%d result:%d Uid:%s\n",
				printResult.SiteIdx, printResult.SKTIdx, printResult.Result, printResult.Uid)

			if siteidx >= 0 && siteidx < 8 && SitesInfo[siteidx].Enabled != 0x0 {
				sendAutoMessage(siteidx, 0xF0)
			}

		}()
	}
}

func handlePDU63(conn net.Conn, pData []byte) {
	log.Printf("[PDU 0x63] App告知AutoApp初始化情况: pData=%X", pData)

	if len(pData) < 2 {
		log.Printf("[PDU 0x63] Error: pData length is too short (%d bytes)", len(pData))
		return
	}

	siteCnt := int(pData[0])
	sktCntFromPDU := int(pData[1])
	siteEnBitmap := pData[2:]

	// 从配置中获取预期的SKT数量
	expectedSktCnt := autoByteServerConfig.SktCntPerSite
	if sktCntFromPDU != expectedSktCnt {
		log.Printf("[PDU 0x63] Warning: SKT count from PDU (%d) does not match config (%d). Using value from PDU for parsing.", sktCntFromPDU, expectedSktCnt)
	}
	sMaxSktIdx = sktCntFromPDU // 使用PDU中的值进行解析

	sktBytesPerSite := sktCntFromPDU / 8
	expectedBitmapSize := siteCnt * sktBytesPerSite

	if len(siteEnBitmap) != expectedBitmapSize {
		log.Printf("[PDU 0x63] Error: SiteEnBitmap size mismatch. Expected %d, got %d", expectedBitmapSize, len(siteEnBitmap))
		return
	}

	log.Printf("[PDU 0x63] 解析初始化数据: SiteCnt=%d, SKTCnt=%d, SKTBytesPerSite=%d", siteCnt, sktCntFromPDU, sktBytesPerSite)

	// 重置所有站点信息
	for i := range SitesInfo {
		SitesInfo[i].Enabled = 0
		SitesInfo[i].Idx = i + 1
	}

	// 根据位图解析每个站点的使能情况
	for i := 0; i < siteCnt; i++ {
		offset := i * sktBytesPerSite
		siteEnableData := siteEnBitmap[offset : offset+sktBytesPerSite]

		// 假设启用的站点索引是连续的，或者需要从一个映射表中查找
		// 这里我们假设App发送的站点是连续的，并且pData中的顺序对应site 1, 2, 3...
		// 注意：协议中没有明确说明位图中的站点索引如何映射，这里采用最直接的顺序映射
		siteIndex := i + 1
		if siteIndex > MAX_SITE_IDX {
			log.Printf("[PDU 0x63] Warning: Parsed site index %d exceeds MAX_SITE_IDX %d", siteIndex, MAX_SITE_IDX)
			continue
		}

		// 将字节数组转换为uint64
		var enabledValue uint64 = 0
		if len(siteEnableData) <= 8 {
			for j, b := range siteEnableData {
				enabledValue |= uint64(b) << (8 * (len(siteEnableData) - 1 - j))
			}
		} else {
			log.Printf("[PDU 0x63] Error: siteEnableData for site %d is too long (%d bytes)", siteIndex, len(siteEnableData))
			continue
		}

		SitesInfo[siteIndex-1].Enabled = enabledValue
		SitesInfo[siteIndex-1].Idx = siteIndex
		log.Printf("[PDU 0x63] SiteIdx:0x%02X Enabled:0x%X MaxSkt:%d Init OK.", siteIndex, enabledValue, sktCntFromPDU)
	}

	// 触发一次状态重置，特别是用于同步轴移动的时序
	gSite9ChipPlacementCompleted = false
	log.Printf("[AxisMove] 站点9芯片放置完成标志已重置，开始新的测试循环")

	gPdu63Mutex.Lock()
	gIsHaveRecvPdu63 = true
	gPdu63Mutex.Unlock()

	// 响应ACK
	var write_buffer = make([]byte, 10)
	binary.BigEndian.PutUint16(write_buffer[0:2], 0x4153) // PFLAG "AS"
	write_buffer[2] = 0x63                                // PDU
	write_buffer[3] = 1                                   // PLEN
	write_buffer[4] = 0                                   // ErrCode 0: Success
	write_buffer[5] = util.CheckSumCRC8(write_buffer[0:5])
	_, err2 := conn.Write(write_buffer[0:6])
	if err2 != nil {
		log.Printf("VAuto Send 0x63 Ack error:%s\n", err2)
	} else {
		log.Printf("VAuto Send 0x63 Ack OK")
	}
}

// 可选的配置接口，用于在启动前设置参数
func ConfigureAutoByteServer(enable bool, serverHost string, serverPort string, jsonProtocol int) {
	SetAutoByteServerConfig(enable, serverHost, serverPort, jsonProtocol)
}

// 获取当前配置信息
func GetAutoByteServerConfig() map[string]interface{} {
	if autoByteServerConfig == nil {
		loadAutoByteServerConfig()
	}

	return map[string]interface{}{
		"enable":                  autoByteServerConfig.Enable,
		"server_host":             autoByteServerConfig.ServerHost,
		"server_port":             autoByteServerConfig.ServerPort,
		"json_protocol":           autoByteServerConfig.JsonProtocol,
		"special_site":            autoByteServerConfig.SpecialSite,
		"special_site_max_skt":    autoByteServerConfig.SpecialSiteMaxSkt,
		"special_site_sleep_msec": autoByteServerConfig.SpecialSiteSleepMsec,
	}
}

// 获取特殊站点处理状态信息
func GetSpecialSiteStatus() map[string]interface{} {
	gSpecialSiteInfoMutex.RLock()
	defer gSpecialSiteInfoMutex.RUnlock()

	messageChanLen := 0
	requestChanLen := 0

	if gSpecialSiteMessageChan != nil {
		messageChanLen = len(gSpecialSiteMessageChan)
	}

	if gSpecialSiteRequestChan != nil {
		requestChanLen = len(gSpecialSiteRequestChan)
	}

	return map[string]interface{}{
		"enabled":             gSpecialSiteInfo.Enabled,
		"site_idx":            gSpecialSiteInfo.SiteIdx,
		"max_count":           gSpecialSiteInfo.MaxCount,
		"pending_count":       gSpecialSiteInfo.PendingCount,
		"processed_count":     gSpecialSiteInfo.ProcessedCount,
		"success_count":       gSpecialSiteInfo.SuccessCount,
		"failure_count":       gSpecialSiteInfo.FailureCount,
		"is_processing":       gSpecialSiteInfo.IsProcessing,
		"message_chan_length": messageChanLen,
		"request_chan_length": requestChanLen,
	}
}

// 强制处理特殊站点剩余数量（处理尾数）
func ForceProcessSpecialSiteRemaining() {
	gSpecialSiteInfoMutex.RLock()
	enabled := gSpecialSiteInfo.Enabled
	pendingCount := gSpecialSiteInfo.PendingCount
	isProcessing := gSpecialSiteInfo.IsProcessing
	gSpecialSiteInfoMutex.RUnlock()

	if !enabled {
		log.Printf("[特殊站点] 特殊站点处理未启用")
		return
	}

	if pendingCount <= 0 {
		log.Printf("[特殊站点] 没有待处理的数量")
		return
	}

	if isProcessing {
		log.Printf("[特殊站点] 正在处理中，无法强制处理")
		return
	}

	log.Printf("[特殊站点] 强制处理剩余 %d 个特殊站点数量", pendingCount)

	// 发送处理请求
	request := &tSpecialSiteRequest{
		Type:  "start_processing",
		Count: pendingCount,
	}

	select {
	case gSpecialSiteRequestChan <- request:
		log.Printf("[特殊站点] 强制处理请求已发送")
	default:
		log.Printf("[特殊站点] 请求队列已满，无法发送强制处理请求")
	}
}

// 停止自动化字节服务器（如果需要优雅停止）
func StopAutoByteServer() {
	glog.Info("停止自动化字节服务器模块")

	// 停止特殊站点处理器
	if gSpecialSiteInfo.Enabled {
		stopSpecialSiteProcessor()
	}
}

// 停止特殊站点处理器
func stopSpecialSiteProcessor() {
	if gSpecialSiteMessageChan != nil {
		close(gSpecialSiteMessageChan)
		gSpecialSiteMessageChan = nil
	}

	if gSpecialSiteRequestChan != nil {
		close(gSpecialSiteRequestChan)
		gSpecialSiteRequestChan = nil
	}

	glog.Info("特殊站点处理器已停止")
}

// isSite9ChipPlacementCompleted 检查站点9的芯片放置是否完成
// 通过检查站点9是否已经发送过0xE6命令并收到ACK响应来判断
func isSite9ChipPlacementCompleted() bool {
	// 优先检查专门的完成标志
	if gSite9ChipPlacementCompleted {
		log.Printf("[AxisMove] 站点9芯片放置已完成（根据0xE6 ACK标志）")
		return true
	}

	// 站点9的数组索引（0-based）
	site9Idx := 8

	// 检查站点9是否在有效范围内
	if site9Idx >= len(SitesInfo) || SitesInfo[site9Idx].Idx != 9 {
		log.Printf("[AxisMove] 站点9不存在或索引错误")
		return false
	}

	// 检查站点9是否使能
	if SitesInfo[site9Idx].Enabled == 0 {
		log.Printf("[AxisMove] 站点9未使能，视为芯片放置完成")
		return true
	}

	// 备用检查：通过特殊站点处理状态判断
	if gSpecialSiteInfo.Enabled && gSpecialSiteInfo.SiteIdx == 9 {
		// 检查特殊站点是否已经处理过
		if gSpecialSiteInfo.ProcessedCount > 0 {
			log.Printf("[AxisMove] 站点9特殊站点已处理 %d 次，芯片放置完成",
				gSpecialSiteInfo.ProcessedCount)
			return true
		}

		// 如果正在处理中，也认为芯片放置已开始
		if gSpecialSiteInfo.IsProcessing {
			log.Printf("[AxisMove] 站点9特殊站点正在处理中，等待完成...")
			return false
		}
	}

	log.Printf("[AxisMove] 站点9芯片放置尚未完成 - 使能状态: 0x%X, 完成标志: %t",
		SitesInfo[site9Idx].Enabled, gSite9ChipPlacementCompleted)
	return false
}

func GetNewEnable(autoMode int, MaxSktIdx int, Enabled uint64, maxChip int) (int, []byte) {
	//0F00000000000000 第一个SKT1-4使能
	//00000000000000F0 第二个SKT15-1使能
	newEnabled := make([]byte, MaxSktIdx)
	maxSupply := maxChip
	sktIdx := 0
	realSupply := 0
	log.Printf("[GetNewEnable] 开始计算新的使能状态: autoMode=%d, MaxSktIdx=%d, Enabled=0x%X, maxChip=%d",
		autoMode, MaxSktIdx, Enabled, maxChip)

	if autoMode == AutoMode_AP8000V2_3 {
		for sktIdx < MaxSktIdx {
			// Enabled 是uint64类型，需要转换为byte数组
			onebyte := util.ToBytes(Enabled)[sktIdx/8]
			log.Printf("[GetNewEnable] 当前使能状态: onebyte=0x%X sktIdx=%d", onebyte, sktIdx)
			if onebyte>>(sktIdx%8)&0x1 == 1 {
				newEnabled[sktIdx] = 1
				realSupply++
				if maxChip != -1 { //不是尾数，全部放满
					maxSupply--
					if maxSupply == 0 {
						break
					}
				}
			}
			sktIdx++
		}
	} else if autoMode == AutoMode_AP8000 {
		for sktIdx < sMaxSktIdx {
			if Enabled>>sktIdx&0x1 == 1 {
				newEnabled[0] |= 1 << sktIdx
				maxSupply--
				realSupply++
				if maxSupply == 0 {
					break
				}
			}
			sktIdx++
		}
	} else {
		log.Printf("GetNewEnable不支持此座子数%d", sMaxSktIdx)
	}

	return realSupply, newEnabled
}
