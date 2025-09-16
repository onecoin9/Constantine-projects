package server

import (
	"encoding/binary"
	"net"
	"strconv"
	"time"
	"github.com/golang/glog"
	"github.com/Unknwon/goconfig"
)

const (
	TEMP_HOST                 = "localhost"
	TEMP_CONN_TYPE            = "tcp"
	TEMP_CONN_SERVER_PORT     = 64200
	DEFAULT_MAX_SITE_IDX      = 24  // 默认SITE数量
	DEFAULT_MAX_FAN_IDX       = 16  // 默认风扇数量
)

// 命令ID常量定义
const (
	CMD_TEMP_QUERY_TEMP           = 0x01   // 查询温度
	CMD_TEMP_QUERY_SPEED          = 0x02   // 查询转速
	CMD_TEMP_SET_TEMP             = 0x03   // 设定温度
	CMD_TEMP_SET_SPEED            = 0x04   // 设定转速
	CMD_TEMP_START_STOP           = 0x05   // 启停指令
	CMD_TEMP_SET_PID              = 0x06   // PID参数设置
	CMD_TEMP_QUERY_FAULT          = 0x07   // 故障状态
	CMD_TEMP_QUERY_VOLTAGE        = 0x08   // 电压电流状态
	CMD_TEMP_5V_SWITCH            = 0x09   // 5V开关指令
	CMD_TEMP_SET_SITE_TEMP        = 0x0B   // 设定SITE控温温度
	CMD_TEMP_QUERY_SITE_TEMP      = 0x0C   // 查询SITE设定温度
	CMD_TEMP_SET_MAX_DUTY         = 0x10   // 最大占空比设置
	CMD_TEMP_QUERY_PID            = 0x11   // PID参数设置查询
	CMD_TEMP_QUERY_MAX_DUTY       = 0x12   // 最大占空比参数查询
	CMD_TEMP_SYSTEM_RESET         = 0x1FFF // 系统复位指令
	CMD_TEMP_UPGRADE              = 0x3FFF // 升级指令
)

// 状态码常量
const (
	STATUS_SUCCESS = 0x01
	STATUS_FAILED  = 0x00
)

// 内部配置结构体
type tTempControlConfig struct {
	SiteNum         int
	Enable      	bool
	ServerPort      int
	ServerAddress   string
	ServerType      string
	MaxSiteIdx      int
	MaxFanIdx       int
}

// 全局配置实例
var tempControlConfig *tTempControlConfig

// 从配置文件加载温控配置
func loadTempControlConfig() error {
	cfg, err := goconfig.LoadConfigFile("config.ini")
	if err != nil {
		glog.Errorf("加载配置文件失败: %v", err)
		// 使用默认配置
		initTempControlConfig()
		return err
	}
	
	tempControlConfig = &tTempControlConfig{
		SiteNum:       cfg.MustInt("temperature", "site_num", 1),
		Enable:        cfg.MustBool("temperature", "enable_temperature", false),
		ServerPort:    cfg.MustInt("temperature", "server_port", TEMP_CONN_SERVER_PORT),
		ServerAddress: cfg.MustValue("temperature", "server_address", TEMP_HOST),
		ServerType:    cfg.MustValue("temperature", "server_type", TEMP_CONN_TYPE),
		MaxSiteIdx:    cfg.MustInt("temperature", "max_site_idx", DEFAULT_MAX_SITE_IDX),
		MaxFanIdx:     cfg.MustInt("temperature", "max_fan_idx", DEFAULT_MAX_FAN_IDX),
	}
	
	glog.Infof("温控配置已加载: Enable=%t, Port=%d, SiteNum=%d, MaxSiteIdx=%d, MaxFanIdx=%d", 
		tempControlConfig.Enable, tempControlConfig.ServerPort, tempControlConfig.SiteNum,
		tempControlConfig.MaxSiteIdx, tempControlConfig.MaxFanIdx)
	
	return nil
}

// 初始化默认配置
func initTempControlConfig() {
	tempControlConfig = &tTempControlConfig{
		SiteNum:       1,
		Enable:        false,
		ServerPort:    TEMP_CONN_SERVER_PORT,
		ServerAddress: TEMP_HOST,
		ServerType:    TEMP_CONN_TYPE,
		MaxSiteIdx:    DEFAULT_MAX_SITE_IDX,
		MaxFanIdx:     DEFAULT_MAX_FAN_IDX,
	}
	glog.Info("使用默认温控配置")
}

// 设置配置参数的公共方法（可选）
func SetTempControlConfig(siteNum int, fanNum int, port int, address string) {
	if tempControlConfig == nil {
		initTempControlConfig()
	}
	
	if siteNum > 0 {
		tempControlConfig.MaxSiteIdx = siteNum
		tempControlConfig.SiteNum = siteNum
	}
	if fanNum > 0 {
		tempControlConfig.MaxFanIdx = fanNum
	}
	if port > 1024 {
		tempControlConfig.ServerPort = port
	}
	if address != "" {
		tempControlConfig.ServerAddress = address
	}
}

// 温控系统状态结构
type TempControlState struct {
	// 配置参数
	MaxSiteIdx    int                     // SITE数量
	MaxFanIdx     int                     // 风扇数量
	
	// 温度相关
	CurrentTemps  []uint16                // 当前温度 (实际温度*10)
	TargetTemps   []uint16                // 目标温度 (实际温度*10)
	GlobalTarget  uint16                  // 全局目标温度
	
	// 转速相关
	FanSpeeds     []uint16                // 风扇转速 (RPM)
	FanSpeedLevel uint8                   // 风扇转速等级 (0-100)
	
	// 系统状态
	IsRunning     bool                    // 系统运行状态
	Power5VOn     bool                    // 5V电源状态
	
	// PID参数
	PIDParams     [3]uint16               // KP, KI, KD (实际值*100)
	
	// 占空比
	MaxDuty       []uint16                // 最大占空比 (0-1000)
	GlobalMaxDuty uint16                  // 全局最大占空比
	
	// 故障状态
	DutFaults     []uint16                // DUT内部故障
	CommFault     uint32                  // 通讯故障 (位图)
	NetworkFault  uint16                  // 网络故障
	RestartCount  uint16                  // 重启计数
	FanFaults     [4]uint8                // 风扇控制板故障
	
	// 电压电流
	Voltages      [6]uint16               // 电压值 (mV)
	Currents      [6]uint16               // 电流值 (uV)
}

// 全局温控状态
var tempState *TempControlState

// 初始化温控状态
func initTempState(maxSiteIdx, maxFanIdx int) {
	tempState = &TempControlState{
		MaxSiteIdx:    maxSiteIdx,
		MaxFanIdx:     maxFanIdx,
		IsRunning:     false,
		Power5VOn:     false,
		GlobalTarget:  0,
		FanSpeedLevel: 0,
		GlobalMaxDuty: 800, // 默认最大占空比
		PIDParams:     [3]uint16{5000, 2000, 0}, // 默认PID参数 (50.00, 20.00, 0.00)
		
		// 初始化切片
		CurrentTemps:  make([]uint16, maxSiteIdx),
		TargetTemps:   make([]uint16, maxSiteIdx),
		FanSpeeds:     make([]uint16, maxFanIdx),
		MaxDuty:       make([]uint16, maxSiteIdx),
		DutFaults:     make([]uint16, maxSiteIdx),
	}
	
	// 初始化默认值
	for i := 0; i < maxSiteIdx; i++ {
		tempState.MaxDuty[i] = 800 // 默认最大占空比
	}
	
	glog.Infof("温控状态已初始化: SITE数量=%d, 风扇数量=%d", maxSiteIdx, maxFanIdx)
}

// NewCmdPacket 创建新的命令包
func NewCmdPacket(cmdID uint16, data []byte) *CmdPacket {
	return &CmdPacket{
		Header: CmdPackHeader{
			CmdFlag:     0,
			CmdID:       cmdID,
			CmdDataSize: uint16(len(data)),
		},
		Data: data,
	}
}

// 指令处理函数

// 1.1 查询温度
func handleQueryTemp() *CmdPacket {
	dataSize := 1 + tempState.MaxSiteIdx*2 // 1字节状态码 + N个SITE*2字节温度数据
	data := make([]byte, dataSize)
	data[0] = STATUS_SUCCESS
	
	// 填充SITE的温度数据，每个SITE 2字节
	for i := 0; i < tempState.MaxSiteIdx; i++ {
		temp := tempState.CurrentTemps[i]
		if temp == 0 {
			temp = 400 // 默认-40度 对应数据400
		}
		binary.LittleEndian.PutUint16(data[1+i*2:3+i*2], temp)
	}
	
	return NewCmdPacket(CMD_TEMP_QUERY_TEMP, data)
}

// 1.2 查询转速
func handleQuerySpeed() *CmdPacket {
	dataSize := 1 + tempState.MaxFanIdx*2 // 1字节状态码 + N个风扇*2字节转速数据
	data := make([]byte, dataSize)
	data[0] = STATUS_SUCCESS
	
	// 填充风扇的转速数据，每个风扇 2字节
	for i := 0; i < tempState.MaxFanIdx; i++ {
		speed := tempState.FanSpeeds[i]
		binary.LittleEndian.PutUint16(data[1+i*2:3+i*2], speed)
	}
	
	return NewCmdPacket(CMD_TEMP_QUERY_SPEED, data)
}

// 1.3 设定温度
func handleSetTemp(cmdData []byte) *CmdPacket {
	if len(cmdData) < 2 {
		return NewCmdPacket(CMD_TEMP_SET_TEMP, []byte{STATUS_FAILED})
	}
	
	targetTemp := binary.LittleEndian.Uint16(cmdData[0:2])
	tempState.GlobalTarget = targetTemp
	
	// 设置所有SITE的目标温度
	for i := 0; i < tempState.MaxSiteIdx; i++ {
		tempState.TargetTemps[i] = targetTemp
	}
	
	glog.Infof("设定全局目标温度: %.1f°C", float64(targetTemp)/10.0)
	return NewCmdPacket(CMD_TEMP_SET_TEMP, []byte{STATUS_SUCCESS})
}

// 1.4 设定转速
func handleSetSpeed(cmdData []byte) *CmdPacket {
	if len(cmdData) < 1 {
		return NewCmdPacket(CMD_TEMP_SET_SPEED, []byte{STATUS_FAILED})
	}
	
	speedLevel := cmdData[0]
	if speedLevel > 100 {
		return NewCmdPacket(CMD_TEMP_SET_SPEED, []byte{STATUS_FAILED})
	}
	
	tempState.FanSpeedLevel = speedLevel
	
	// 根据等级设置实际转速 (假设最大转速6000 RPM)
	actualSpeed := uint16(speedLevel) * 60
	for i := 0; i < tempState.MaxFanIdx; i++ {
		tempState.FanSpeeds[i] = actualSpeed
	}
	
	glog.Infof("设定风扇转速等级: %d%%", speedLevel)
	return NewCmdPacket(CMD_TEMP_SET_SPEED, []byte{STATUS_SUCCESS})
}

// 1.5 启停指令
func handleStartStop(cmdData []byte) *CmdPacket {
	if len(cmdData) < 1 {
		return NewCmdPacket(CMD_TEMP_START_STOP, []byte{STATUS_FAILED})
	}
	
	state := cmdData[0]
	tempState.IsRunning = (state == 0x01)
	
	if tempState.IsRunning {
		glog.Info("温控系统已启动")
	} else {
		glog.Info("温控系统已停止")
	}
	
	return NewCmdPacket(CMD_TEMP_START_STOP, []byte{STATUS_SUCCESS})
}

// 1.6 PID参数设置
func handleSetPID(cmdData []byte) *CmdPacket {
	if len(cmdData) < 6 {
		return NewCmdPacket(CMD_TEMP_SET_PID, []byte{STATUS_FAILED})
	}
	
	kp := binary.LittleEndian.Uint16(cmdData[0:2])
	ki := binary.LittleEndian.Uint16(cmdData[2:4])
	kd := binary.LittleEndian.Uint16(cmdData[4:6])
	
	tempState.PIDParams[0] = kp
	tempState.PIDParams[1] = ki
	tempState.PIDParams[2] = kd
	
	glog.Infof("设定PID参数: KP=%.2f, KI=%.2f, KD=%.2f", 
		float64(kp)/100.0, float64(ki)/100.0, float64(kd)/100.0)
	
	return NewCmdPacket(CMD_TEMP_SET_PID, []byte{STATUS_SUCCESS})
}

// 1.7 故障状态
func handleQueryFault() *CmdPacket {
	// 1字节状态码 + N*2字节DUT故障 + 4字节通讯故障 + 2字节网络故障 + 2字节重启计数 + 4字节风扇故障
	dataSize := 1 + tempState.MaxSiteIdx*2 + 4 + 2 + 2 + 4
	data := make([]byte, dataSize)
	data[0] = STATUS_SUCCESS
	
	offset := 1
	
	// DUT内部故障
	for i := 0; i < tempState.MaxSiteIdx; i++ {
		binary.LittleEndian.PutUint16(data[offset+i*2:offset+i*2+2], tempState.DutFaults[i])
	}
	offset += tempState.MaxSiteIdx * 2
	
	// DUT通讯故障 (4字节)
	binary.LittleEndian.PutUint32(data[offset:offset+4], tempState.CommFault)
	offset += 4
	
	// 网络故障状态 (2字节)
	binary.LittleEndian.PutUint16(data[offset:offset+2], tempState.NetworkFault)
	offset += 2
	
	// 网络异常重启计数 (2字节)
	binary.LittleEndian.PutUint16(data[offset:offset+2], tempState.RestartCount)
	offset += 2
	
	// 风扇控制板故障 (4字节)
	copy(data[offset:offset+4], tempState.FanFaults[:])
	
	return NewCmdPacket(CMD_TEMP_QUERY_FAULT, data)
}

// 1.8 电压电流状态
func handleQueryVoltage() *CmdPacket {
	data := make([]byte, 25) // 1字节状态码 + 24字节电压电流数据
	data[0] = STATUS_SUCCESS
	
	// 6个电压值 (12字节)
	for i := 0; i < 6; i++ {
		binary.LittleEndian.PutUint16(data[1+i*2:3+i*2], tempState.Voltages[i])
	}
	
	// 6个电流值 (12字节)
	for i := 0; i < 6; i++ {
		binary.LittleEndian.PutUint16(data[13+i*2:15+i*2], tempState.Currents[i])
	}
	
	return NewCmdPacket(CMD_TEMP_QUERY_VOLTAGE, data)
}

// 1.9 5V开关指令
func handlePower5V(cmdData []byte) *CmdPacket {
	if len(cmdData) < 1 {
		return NewCmdPacket(CMD_TEMP_5V_SWITCH, []byte{STATUS_FAILED})
	}
	
	state := cmdData[0]
	tempState.Power5VOn = (state == 0x01)
	
	if tempState.Power5VOn {
		glog.Info("5V电源已开启")
	} else {
		glog.Info("5V电源已关闭")
	}
	
	return NewCmdPacket(CMD_TEMP_5V_SWITCH, []byte{STATUS_SUCCESS})
}

// 1.10 设定SITE控温温度
func handleSetSiteTemp(cmdData []byte) *CmdPacket {
	expectedSize := tempState.MaxSiteIdx * 2
	if len(cmdData) < expectedSize {
		return NewCmdPacket(CMD_TEMP_SET_SITE_TEMP, []byte{STATUS_FAILED})
	}
	
	// 设置各个SITE的目标温度
	for i := 0; i < tempState.MaxSiteIdx; i++ {
		temp := binary.LittleEndian.Uint16(cmdData[i*2:i*2+2])
		tempState.TargetTemps[i] = temp
	}
	
	glog.Info("已设定各SITE目标温度")
	return NewCmdPacket(CMD_TEMP_SET_SITE_TEMP, []byte{STATUS_SUCCESS})
}

// 1.11 查询SITE设定温度
func handleQuerySiteTemp() *CmdPacket {
	dataSize := 1 + tempState.MaxSiteIdx*2 // 1字节状态码 + N个SITE*2字节温度数据
	data := make([]byte, dataSize)
	data[0] = STATUS_SUCCESS
	
	// 填充各个SITE的设定温度
	for i := 0; i < tempState.MaxSiteIdx; i++ {
		binary.LittleEndian.PutUint16(data[1+i*2:3+i*2], tempState.TargetTemps[i])
	}
	
	return NewCmdPacket(CMD_TEMP_QUERY_SITE_TEMP, data)
}

// 1.12 最大占空比设置
func handleSetMaxDuty(cmdData []byte) *CmdPacket {
	if len(cmdData) < 2 {
		return NewCmdPacket(CMD_TEMP_SET_MAX_DUTY, []byte{STATUS_FAILED})
	}
	
	maxDuty := binary.LittleEndian.Uint16(cmdData[0:2])
	if maxDuty > 1000 {
		return NewCmdPacket(CMD_TEMP_SET_MAX_DUTY, []byte{STATUS_FAILED})
	}
	
	tempState.GlobalMaxDuty = maxDuty
	// 设置所有SITE的最大占空比
	for i := 0; i < tempState.MaxSiteIdx; i++ {
		tempState.MaxDuty[i] = maxDuty
	}
	
	glog.Infof("设定最大占空比: %d", maxDuty)
	return NewCmdPacket(CMD_TEMP_SET_MAX_DUTY, []byte{STATUS_SUCCESS})
}

// 1.13 PID参数查询
func handleQueryPID() *CmdPacket {
	data := make([]byte, 7) // 1字节状态码 + 6字节PID参数
	data[0] = STATUS_SUCCESS
	
	binary.LittleEndian.PutUint16(data[1:3], tempState.PIDParams[0])
	binary.LittleEndian.PutUint16(data[3:5], tempState.PIDParams[1])
	binary.LittleEndian.PutUint16(data[5:7], tempState.PIDParams[2])
	
	return NewCmdPacket(CMD_TEMP_QUERY_PID, data)
}

// 1.14 最大占空比参数查询
func handleQueryMaxDuty() *CmdPacket {
	dataSize := 1 + tempState.MaxSiteIdx*2 // 1字节状态码 + N个SITE*2字节占空比数据
	data := make([]byte, dataSize)
	data[0] = STATUS_SUCCESS
	
	// 填充各个SITE的最大占空比
	for i := 0; i < tempState.MaxSiteIdx; i++ {
		binary.LittleEndian.PutUint16(data[1+i*2:3+i*2], tempState.MaxDuty[i])
	}
	
	return NewCmdPacket(CMD_TEMP_QUERY_MAX_DUTY, data)
}

// 1.15 系统复位指令
func handleSystemReset() {
	glog.Info("执行系统复位")
	// 这里可以添加实际的复位逻辑
	// 注意：这个命令没有回复
}

// 1.16 升级指令
func handleUpgrade(cmdData []byte) *CmdPacket {
	if len(cmdData) < 8 {
		return NewCmdPacket(CMD_TEMP_UPGRADE, []byte{STATUS_FAILED})
	}
	
	glog.Info("收到升级包")
	// 这里应该实现实际的升级逻辑
	// 暂时直接回复相同的数据
	return NewCmdPacket(CMD_TEMP_UPGRADE, cmdData)
}

// 处理命令包
func ProcessCommand(packet *CmdPacket) *CmdPacket {
	switch packet.Header.CmdID {
	case CMD_TEMP_QUERY_TEMP:
		return handleQueryTemp()
	case CMD_TEMP_QUERY_SPEED:
		return handleQuerySpeed()
	case CMD_TEMP_SET_TEMP:
		return handleSetTemp(packet.Data)
	case CMD_TEMP_SET_SPEED:
		return handleSetSpeed(packet.Data)
	case CMD_TEMP_START_STOP:
		return handleStartStop(packet.Data)
	case CMD_TEMP_SET_PID:
		return handleSetPID(packet.Data)
	case CMD_TEMP_QUERY_FAULT:
		return handleQueryFault()
	case CMD_TEMP_QUERY_VOLTAGE:
		return handleQueryVoltage()
	case CMD_TEMP_5V_SWITCH:
		return handlePower5V(packet.Data)
	case CMD_TEMP_SET_SITE_TEMP:
		return handleSetSiteTemp(packet.Data)
	case CMD_TEMP_QUERY_SITE_TEMP:
		return handleQuerySiteTemp()
	case CMD_TEMP_SET_MAX_DUTY:
		return handleSetMaxDuty(packet.Data)
	case CMD_TEMP_QUERY_PID:
		return handleQueryPID()
	case CMD_TEMP_QUERY_MAX_DUTY:
		return handleQueryMaxDuty()
	case CMD_TEMP_SYSTEM_RESET:
		handleSystemReset()
		return nil // 无回复
	case CMD_TEMP_UPGRADE:
		return handleUpgrade(packet.Data)
	default:
		glog.Warningf("未知命令ID: 0x%04X", packet.Header.CmdID)
		return nil
	}
}

// 处理客户端连接
func handleConnection(conn net.Conn) {
	defer conn.Close()
	glog.Infof("新客户端连接: %s", conn.RemoteAddr())
	
	buffer := make([]byte, 1024)
	for {
		n, err := conn.Read(buffer)
		if err != nil {
			glog.Errorf("读取数据失败: %v", err)
			break
		}
		
		// 解析命令包
		packet, err := FromBytes(buffer[:n])
		if err != nil {
			glog.Errorf("解析命令包失败: %v", err)
			continue
		}
		
		glog.Infof("收到命令: ID=0x%04X, Size=%d", packet.Header.CmdID, packet.Header.CmdDataSize)
		
		// 处理命令
		response := ProcessCommand(packet)
		if response != nil {
			responseData := response.ToBytes()
			_, err = conn.Write(responseData)
			if err != nil {
				glog.Errorf("发送响应失败: %v", err)
				break
			}
			glog.Infof("发送响应: ID=0x%04X, Size=%d", response.Header.CmdID, len(responseData))
		}
	}
	
	glog.Infof("客户端断开连接: %s", conn.RemoteAddr())
}

// 模拟温度更新（实际应用中这里应该读取真实的传感器数据）
func simulateTemperatureUpdate() {
	ticker := time.NewTicker(5 * time.Second)
	defer ticker.Stop()
	
	for range ticker.C {
		if tempState != nil && tempState.IsRunning {
			// 模拟温度变化
			for i := 0; i < tempState.MaxSiteIdx; i++ {
				target := tempState.TargetTemps[i]
				current := tempState.CurrentTemps[i]
				
				if target > 0 {
					// 简单的温度模拟：向目标温度靠近
					if current < target {
						tempState.CurrentTemps[i] = current + 5 // 每次增加0.5度
					} else if current > target {
						tempState.CurrentTemps[i] = current - 5 // 每次减少0.5度
					}
				}
			}
		}
	}
}

// 启动温控服务器 - 简化的对外接口
func DoTempControl() {
	glog.Info("正在启动温控模块...")
	
	// 加载配置文件
	if err := loadTempControlConfig(); err != nil {
		glog.Warningf("加载温控配置失败，使用默认配置: %v", err)
	}
	
	// 检查是否启用
	if !tempControlConfig.Enable {
		glog.Info("温控模块未启用，跳过启动")
		return
	}
	
	// 初始化温控状态
	initTempState(tempControlConfig.MaxSiteIdx, tempControlConfig.MaxFanIdx)
	
	// 启动服务器
	listener, err := net.Listen(tempControlConfig.ServerType, 
		tempControlConfig.ServerAddress+":"+strconv.Itoa(tempControlConfig.ServerPort))
	if err != nil {
		glog.Fatalf("启动温控服务器失败: %v", err)
		return
	}
	defer listener.Close()
	
	glog.Infof("温控服务器启动成功")
	glog.Infof("监听地址: %s:%d", tempControlConfig.ServerAddress, tempControlConfig.ServerPort)
	glog.Infof("SITE数量: %d, 风扇数量: %d", tempControlConfig.MaxSiteIdx, tempControlConfig.MaxFanIdx)
	
	// 启动模拟温度更新任务
	go simulateTemperatureUpdate()
	
	// 接受客户端连接
	for {
		conn, err := listener.Accept()
		if err != nil {
			glog.Errorf("接受连接失败: %v", err)
			continue
		}
		
		// 为每个连接启动一个goroutine
		go handleConnection(conn)
	}
}

// 可选的配置接口，用于在启动前设置参数
func ConfigureTempControl(siteNum, fanNum, port int, address string) {
	SetTempControlConfig(siteNum, fanNum, port, address)
}

// 获取当前配置信息
func GetTempControlConfig() map[string]interface{} {
	if tempControlConfig == nil {
		loadTempControlConfig()
	}
	
	return map[string]interface{}{
		"enable":         tempControlConfig.Enable,
		"server_address": tempControlConfig.ServerAddress,
		"server_port":    tempControlConfig.ServerPort,
		"server_type":    tempControlConfig.ServerType,
		"max_site_idx":   tempControlConfig.MaxSiteIdx,
		"max_fan_idx":    tempControlConfig.MaxFanIdx,
		"site_num":       tempControlConfig.SiteNum,
	}
}