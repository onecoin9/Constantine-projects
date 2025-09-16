package server

import (
	"encoding/binary"
	"fmt"
	"math"
	"math/rand"
	"strconv"
	"strings"
	"time"
	"vauto/util"

	"github.com/Unknwon/goconfig"
	"github.com/golang/glog"
	"go.bug.st/serial"
)

// 命令ID定义
const (
	// 接收的命令ID（主机发送给采集设备）
	CMD_COLLECTION_START_STOP        = 0x01 // 启停指令
	CMD_COLLECTION_DUT_POWER_CTRL    = 0x02 // DUT电源控制
	CMD_COLLECTION_DUT_VOLTAGE_QUERY = 0x03 // DUT电压电流查询
	CMD_COLLECTION_FAULT_STATUS      = 0x04 // 测试板故障状态查询
	CMD_COLLECTION_TEST_CALIBRATION  = 0x05 // 测试标定指令
	CMD_COLLECTION_REG_WRITE         = 0x06 // 寄存器写入指令
	CMD_COLLECTION_REG_READ          = 0x07 // 寄存器查询指令
	CMD_COLLECTION_SET_CHIP_TYPE     = 0x08 // 设置产品类型

	// 发送的反馈命令ID（采集设备发送给主机）
	CMD_COLLECTION_FEEDBACK_DATA          = 0x8001 // 测试板反馈数据（主动上报）
	CMD_COLLECTION_POWER_FEEDBACK         = 0x8002 // DUT电源控制反馈
	CMD_COLLECTION_VOLTAGE_FEEDBACK       = 0x8003 // DUT电压电流查询反馈
	CMD_COLLECTION_FAULT_FEEDBACK         = 0x8004 // 测试板故障状态反馈
	CMD_COLLECTION_CALIBRATION_FEEDBACK   = 0x8005 // 测试标定反馈
	CMD_COLLECTION_REG_WR_FEEDBACK        = 0x8006 // 寄存器写入反馈
	CMD_COLLECTION_REG_RD_FEEDBACK        = 0x8007 // 寄存器查询反馈
	CMD_COLLECTION_SET_CHIP_TYPE_FEEDBACK = 0x8008 // 设置产品类型反馈
)

// XTKZHeader 新协议包头
type XTKZHeader struct {
	CmdFlag     uint32 // 命令标志, 固定为 0x58544B5A
	CmdID       uint16 // 命令码
	CmdDataSize uint16 // 命令数据长度
}

// XTKZPacket 新协议完整包
type XTKZPacket struct {
	Header XTKZHeader
	Data   []byte
	CRC8   uint8
}

// DUTSampleData 新协议中单个DUT的采样数据结构 (33 bytes)
type DUTSampleData struct {
	Start           uint8  // 固定为 0xBC
	ChipIndex       uint8  // 芯片类型
	GyroX           uint32 // X轴陀螺数据
	GyroY           uint32 // Y轴陀螺数据
	GyroZ           uint32 // Z轴陀螺数据
	GyroAccX        uint32 // X轴加表数据
	GyroAccY        uint32 // Y轴加表数据
	GyroAccZ        uint32 // Z轴加表数据
	GyroMix         uint32 // 正交耦合数据
	GyroTemperature uint16 // 温度数据
	SampleCRC       uint8  // 采样校验值
}

// ExternalGyroData 新协议中外挂陀螺仪的数据结构 (32 bytes without start)
type ExternalGyroData struct {
	Start           uint8  // 固定为 0xBC
	GyroX           uint32 // X轴陀螺数据
	GyroY           uint32 // Y轴陀螺数据
	GyroZ           uint32 // Z轴陀螺数据
	GyroAccX        uint32 // X轴加表数据
	GyroAccY        uint32 // Y轴加表数据
	GyroAccZ        uint32 // Z轴加表数据
	GyroMix         uint32 // 正交耦合数据
	GyroTemperature uint16 // 温度数据
	SampleCRC       uint8  // 样本校验值
	GyroCounter     uint16 // 计数器
}

// 序列化新协议包为字节数组
func (packet *XTKZPacket) ToBytes() []byte {
	headerSize := 8                                // XTKZHeader 大小
	totalSize := headerSize + len(packet.Data) + 1 // 包含1字节CRC
	buffer := make([]byte, totalSize)

	// 写入包头 (小端)
	binary.LittleEndian.PutUint32(buffer[0:4], packet.Header.CmdFlag)
	binary.LittleEndian.PutUint16(buffer[4:6], packet.Header.CmdID)
	binary.LittleEndian.PutUint16(buffer[6:8], packet.Header.CmdDataSize)

	// 写入数据
	copy(buffer[8:], packet.Data)

	// 计算并写入CRC8
	packet.CRC8 = util.CheckSumCRC8(buffer[:totalSize-1])
	buffer[totalSize-1] = packet.CRC8

	return buffer
}

// 从字节数组反序列化新协议包
func bytesToXTKZPacket(data []byte) (*XTKZPacket, error) {
	if len(data) < 9 { // 8字节头 + 1字节CRC
		return nil, fmt.Errorf("数据长度不足，至少需要9字节")
	}

	// 验证CmdFlag
	if binary.LittleEndian.Uint32(data[0:4]) != 0x58544B5A {
		return nil, fmt.Errorf("无效的命令标志 (CmdFlag)")
	}

	packet := &XTKZPacket{}

	// 验证CRC
	receivedCRC := data[len(data)-1]
	calculatedCRC := util.CheckSumCRC8(data[:len(data)-1])
	if receivedCRC != calculatedCRC {
		return nil, fmt.Errorf("CRC校验失败: 收到 0x%X, 计算为 0x%X", receivedCRC, calculatedCRC)
	}
	packet.CRC8 = receivedCRC

	// 读取包头
	packet.Header.CmdFlag = binary.LittleEndian.Uint32(data[0:4])
	packet.Header.CmdID = binary.LittleEndian.Uint16(data[4:6])
	packet.Header.CmdDataSize = binary.LittleEndian.Uint16(data[6:8])

	// 检查数据长度是否匹配
	if len(data) != 8+int(packet.Header.CmdDataSize)+1 {
		return nil, fmt.Errorf("数据长度不匹配: 期望 %d, 实际 %d", 8+int(packet.Header.CmdDataSize)+1, len(data))
	}

	// 读取数据
	packet.Data = make([]byte, packet.Header.CmdDataSize)
	copy(packet.Data, data[8:8+packet.Header.CmdDataSize])

	return packet, nil
}

// tSampleDataHeader 采样数据头部结构
type tSampleDataHeader struct {
	Version          uint8    // 版本信息
	ChipIdx          uint8    // 1表示A300, 2表示G300, 3表示270...
	DTUActive        uint16   // DTU Active信息, bit0-7表示DTU1-8, Bit15表示313标准标定数据
	TotalNum         uint16   // 采样数据SampleData长度 = header + 总共有多少采样数据包
	EachDTUSampleLen uint16   // 每个DTU采样的数据包长度
	EachSampleLen    uint16   // 一个DTU里有多少样样数据,每个采样数据包的总长度
	DataCRC8         uint8    // 数据的CRC8
	Reserved         [4]uint8 // 保留
	HeaderCRC8       uint8    // 头部的CRC8
} // 总共16个字节

// 采样数据结构
type SampleData struct {
	GyroX           uint32 // X轴陀螺数据
	GyroY           uint32 // Y轴陀螺数据
	GyroZ           uint32 // Z轴陀螺数据
	GyroAccX        uint32 // X轴加表数据
	GyroAccY        uint32 // Y轴加表数据
	GyroAccZ        uint32 // Z轴加表数据
	GyroMix         uint32 // 正交耦合数据
	GyroTemperature uint16 // 温度数据
}

// 采集设备状态
type CollectionDeviceState struct {
	TestState   uint8  // 测试状态：01测试中，00停止，02故障
	SN          uint32 // 主板序列号
	PowerStates uint16 // DUT电源状态
	FaultStates uint32 // 故障状态
	IsRunning   bool   // 设备运行状态
	DUTActive   uint16 // DUT Active信息, bit0-7表示DUT1-8, Bit15表示313标准标定数据
	ChipIndex   uint8  // 芯片类型
}

// 内部配置结构体
type tCollectionConfig struct {
	SiteNum              int
	Enable               bool
	UartPort             string
	BaudRate             int
	DeviceSN             uint32 // 设备序列号
	ChipIdx              uint8  // 芯片类型索引, 1表示A300, 2表示G300, 3表示270...
	DUTCount             int    // DUT数量
	DUTActive            uint16 // DUT Active信息, bit0-7表示DUT1-8, Bit15表示313标准标定数据
	LogIntervalMsec      int    // 日志打印间隔
	HighSpeedSimulation  bool   // 是否启用高速模拟
	SendIntervalMicrosec int    // 高速模拟下的发送间隔
}

// 全局配置实例和串口连接
var collectionConfig *tCollectionConfig
var serialPort serial.Port
var stopChannel chan bool
var deviceState *CollectionDeviceState
var gSimulationCounter int64 // 用于生成动态数据的全局计数器
var r *rand.Rand             // 全局随机数生成器

func init() {
	// 初始化并播种随机数生成器，以确保每次运行的随机序列都不同
	source := rand.NewSource(time.Now().UnixNano())
	r = rand.New(source)
}

// 从配置文件加载采集配置
func loadCollectionConfig() error {
	cfg, err := goconfig.LoadConfigFile("config.ini")
	if err != nil {
		glog.Errorf("加载配置文件失败: %v", err)
		initCollectionConfig()
		return err
	}

	// 读取dut_active，支持16进制格式（如0xFF）
	dutActiveStr := cfg.MustValue("collection", "dut_active", "255")
	var dutActive uint16
	if strings.HasPrefix(dutActiveStr, "0x") || strings.HasPrefix(dutActiveStr, "0X") {
		// 16进制格式
		val, err := strconv.ParseUint(dutActiveStr[2:], 16, 16)
		if err != nil {
			glog.Errorf("解析dut_active失败: %v, 使用默认值0xFF", err)
			dutActive = 0x00FF
		} else {
			dutActive = uint16(val)
		}
	} else {
		// 10进制格式
		val, err := strconv.ParseUint(dutActiveStr, 10, 16)
		if err != nil {
			glog.Errorf("解析dut_active失败: %v, 使用默认值255", err)
			dutActive = 0x00FF
		} else {
			dutActive = uint16(val)
		}
	}

	collectionConfig = &tCollectionConfig{
		UartPort:             cfg.MustValue("collection", "uart_port", "COM6"),
		BaudRate:             cfg.MustInt("collection", "baud_rate", 1382400),
		SiteNum:              cfg.MustInt("collection", "site_num", 1),
		Enable:               cfg.MustBool("collection", "enable_collection", false),
		DeviceSN:             uint32(cfg.MustInt("collection", "device_sn", 0x12345678)),
		ChipIdx:              uint8(cfg.MustInt("collection", "chip_idx", 1)),
		DUTCount:             cfg.MustInt("collection", "dut_count", 8),
		DUTActive:            dutActive,
		LogIntervalMsec:      cfg.MustInt("collection", "log_interval_msec", 1000),
		HighSpeedSimulation:  cfg.MustBool("collection", "high_speed_simulation", false),
		SendIntervalMicrosec: cfg.MustInt("collection", "send_interval_microsec", 1000),
	}

	glog.Infof("采集控制配置已加载: Enable=%t, SiteNum=%d, UartPort=%s, BaudRate=%d, DeviceSN=0x%08X, ChipIdx=%d, DUTCount=%d, DUTActive=0x%04X, LogInterval=%dms, HighSpeed=%t, SendInterval=%dus",
		collectionConfig.Enable, collectionConfig.SiteNum,
		collectionConfig.UartPort, collectionConfig.BaudRate, collectionConfig.DeviceSN,
		collectionConfig.ChipIdx, collectionConfig.DUTCount, collectionConfig.DUTActive, collectionConfig.LogIntervalMsec,
		collectionConfig.HighSpeedSimulation, collectionConfig.SendIntervalMicrosec)

	return nil
}

// 初始化默认配置
func initCollectionConfig() {
	collectionConfig = &tCollectionConfig{
		SiteNum:              1,
		Enable:               false,
		UartPort:             "COM6",
		BaudRate:             115200,
		DeviceSN:             0x12345678,
		ChipIdx:              1,
		DUTCount:             8,
		DUTActive:            0x00FF, // DUT1-8启用，313标准标定数据关闭
		LogIntervalMsec:      1000,
		HighSpeedSimulation:  false,
		SendIntervalMicrosec: 1000,
	}
	glog.Info("使用默认采集控制配置")
}

// 初始化设备状态
func initDeviceState() {
	deviceState = &CollectionDeviceState{
		TestState:   0x00, // 初始停止状态
		SN:          collectionConfig.DeviceSN,
		PowerStates: 0x0000,     // 所有DUT电源关闭
		FaultStates: 0x00000000, // 无故障
		IsRunning:   false,
		DUTActive:   collectionConfig.DUTActive, // 从配置读取
		ChipIndex:   collectionConfig.ChipIdx,   // 从配置读取
	}
}

// 初始化串口连接
func initSerialPort() error {
	mode := &serial.Mode{
		BaudRate: collectionConfig.BaudRate,
		Parity:   serial.NoParity,
		DataBits: 8,
		StopBits: serial.OneStopBit,
	}

	var err error
	serialPort, err = serial.Open(collectionConfig.UartPort, mode)
	if err != nil {
		return fmt.Errorf("打开串口失败: %v", err)
	}

	glog.Infof("串口 %s 初始化成功，波特率: %d", collectionConfig.UartPort, collectionConfig.BaudRate)
	return nil
}

// 发送反馈数据
func sendFeedback(cmdID uint16, data []byte) error {
	if serialPort == nil {
		return fmt.Errorf("串口未初始化")
	}

	packet := &XTKZPacket{
		Header: XTKZHeader{
			CmdFlag:     0x58544B5A,
			CmdID:       cmdID,
			CmdDataSize: uint16(len(data)),
		},
		Data: data,
	}

	buffer := packet.ToBytes()

	// 打印详细的发送日志（除了高频的采集数据）
	if cmdID != CMD_COLLECTION_FEEDBACK_DATA {
		logPacketDetails(buffer, "Sent", cmdID)
	} else {
		// 对于高频数据，只在需要时打印
		if shouldLogHighFreqData() {
			logPacketDetails(buffer, "Sent", cmdID)
		}
	}

	n, err := serialPort.Write(buffer)
	if err != nil {
		return fmt.Errorf("发送反馈失败: %v", err)
	}

	// 对于高频数据CMD_COLLECTION_FEEDBACK_DATA，使用时间间隔控制日志输出
	if cmdID != CMD_COLLECTION_FEEDBACK_DATA {
		glog.Infof("发送反馈成功: CmdID=0x%04X, 数据长度=%d, 写入字节数=%d", cmdID, len(data), n)
	} else {
		// 对于高频数据，只在需要时打印
		if shouldLogHighFreqData() {
			glog.Infof("发送反馈成功: CmdID=0x%04X, 数据长度=%d, 写入字节数=%d", cmdID, len(data), n)
		}
	}
	return nil
}

// logPacketDetails 详细打印数据包的结构，用于调试
func logPacketDetails(data []byte, direction string, cmdID uint16) {
	if len(data) < 9 {
		glog.Warningf("[%s] 尝试记录的数据包长度不足9字节: %d", direction, len(data))
		return
	}

	cmdFlag := binary.LittleEndian.Uint32(data[0:4])
	cmdDataSize := binary.LittleEndian.Uint16(data[6:8])
	cmdData := data[8 : 8+cmdDataSize]
	crc8 := data[len(data)-1]

	var logBuilder strings.Builder
	logBuilder.WriteString(fmt.Sprintf("\n--- %s Packet Details (CmdID: 0x%04X) ---\n", direction, cmdID))
	logBuilder.WriteString(fmt.Sprintf("Raw Data: %X\n", data))
	logBuilder.WriteString("--------------------------------------------------\n")
	logBuilder.WriteString(fmt.Sprintf("  [0-3]   CmdFlag     : 0x%08X\n", cmdFlag))
	logBuilder.WriteString(fmt.Sprintf("  [4-5]   CmdID       : 0x%04X\n", cmdID))
	logBuilder.WriteString(fmt.Sprintf("  [6-7]   CmdDataSize : %d (0x%04X)\n", cmdDataSize, cmdDataSize))

	// --- CmdData Parsing ---
	logBuilder.WriteString("  CmdData Content:\n")
	if cmdDataSize > 0 {
		switch cmdID {
		case CMD_COLLECTION_START_STOP: // 0x01
			if len(cmdData) >= 7 {
				logBuilder.WriteString(fmt.Sprintf("    [0]     State      : %d (0:Stop, 1:Start)\n", cmdData[0]))
				logBuilder.WriteString(fmt.Sprintf("    [1-2]   Dut_active : 0x%04X\n", binary.LittleEndian.Uint16(cmdData[1:3])))
				logBuilder.WriteString(fmt.Sprintf("    [3-6]   Time       : %d\n", binary.LittleEndian.Uint32(cmdData[3:7])))
			}
		case CMD_COLLECTION_DUT_POWER_CTRL: // 0x02
			if len(cmdData) >= 3 {
				logBuilder.WriteString(fmt.Sprintf("    [0]     State      : %d (0:Stop, 1:Start)\n", cmdData[0]))
				logBuilder.WriteString(fmt.Sprintf("    [1-2]   Dut_Power  : 0x%04X\n", binary.LittleEndian.Uint16(cmdData[1:3])))
			}
		case CMD_COLLECTION_POWER_FEEDBACK: // 0x8002
			if len(cmdData) >= 7 {
				logBuilder.WriteString(fmt.Sprintf("    [0-3]   SN         : 0x%08X\n", binary.LittleEndian.Uint32(cmdData[0:4])))
				logBuilder.WriteString(fmt.Sprintf("    [4]     State      : %d (0:Fail, 1:Success)\n", cmdData[4]))
				logBuilder.WriteString(fmt.Sprintf("    [5-6]   PowerState : 0x%04X\n", binary.LittleEndian.Uint16(cmdData[5:7])))
			}
		case CMD_COLLECTION_DUT_VOLTAGE_QUERY: // 0x03
			logBuilder.WriteString("    (No data)\n")
		case CMD_COLLECTION_VOLTAGE_FEEDBACK: // 0x8003
			if len(cmdData) >= 72 {
				logBuilder.WriteString(fmt.Sprintf("    [0-3]   SN           : 0x%08X\n", binary.LittleEndian.Uint32(cmdData[0:4])))
				logBuilder.WriteString(fmt.Sprintf("    [4-5]   Board Volt   : %d mV\n", binary.LittleEndian.Uint16(cmdData[4:6])))
				logBuilder.WriteString(fmt.Sprintf("    [6-7]   Board Curr   : %d mA\n", binary.LittleEndian.Uint16(cmdData[6:8])))
				// Just show one DUT for brevity
				logBuilder.WriteString(fmt.Sprintf("    [8-9]   DUT1 5V Volt : %d mV\n", binary.LittleEndian.Uint16(cmdData[8:10])))
				logBuilder.WriteString(fmt.Sprintf("    [10-11] DUT1 5V Curr : %d mA\n", binary.LittleEndian.Uint16(cmdData[10:12])))
				logBuilder.WriteString("    (... and so on for all DUTs ...)\n")
			}
		case CMD_COLLECTION_FAULT_STATUS: // 0x04
			logBuilder.WriteString("    (No data)\n")
		case CMD_COLLECTION_FAULT_FEEDBACK: // 0x8004
			if len(cmdData) >= 8 {
				logBuilder.WriteString(fmt.Sprintf("    [0-3]   SN         : 0x%08X\n", binary.LittleEndian.Uint32(cmdData[0:4])))
				logBuilder.WriteString(fmt.Sprintf("    [4-7]   FaultState : 0x%08X\n", binary.LittleEndian.Uint32(cmdData[4:8])))
			}
		case CMD_COLLECTION_TEST_CALIBRATION: // 0x05
			if len(cmdData) >= 2 {
				length := cmdData[1]
				logBuilder.WriteString(fmt.Sprintf("    [0]     Dut_sel    : 0x%02X\n", cmdData[0]))
				logBuilder.WriteString(fmt.Sprintf("    [1]     Length     : %d\n", length))
				if len(cmdData) >= 2+int(length) {
					logBuilder.WriteString(fmt.Sprintf("    [2-%d]   Command    : %X\n", 2+length-1, cmdData[2:2+length]))
				}
			}
		case CMD_COLLECTION_CALIBRATION_FEEDBACK: //0x8005
			if len(cmdData) >= 5 {
				logBuilder.WriteString(fmt.Sprintf("    [0-3]   SN         : 0x%08X\n", binary.LittleEndian.Uint32(cmdData[0:4])))
				logBuilder.WriteString(fmt.Sprintf("    [4]     State      : %d (0:Fail, 1:Success)\n", cmdData[4]))
				logBuilder.WriteString(fmt.Sprintf("    [5->]   Command    : %X\n", cmdData[5:]))
			}
		case CMD_COLLECTION_REG_WRITE: // 0x06
			if len(cmdData) >= 3 {
				length := cmdData[2]
				logBuilder.WriteString(fmt.Sprintf("    [0]     Dut_sel    : 0x%02X\n", cmdData[0]))
				logBuilder.WriteString(fmt.Sprintf("    [1]     Reg_addr   : 0x%02X\n", cmdData[1]))
				logBuilder.WriteString(fmt.Sprintf("    [2]     Length     : %d\n", length))
				if len(cmdData) >= 3+int(length) {
					logBuilder.WriteString(fmt.Sprintf("    [3-%d]   Value      : %X\n", 3+length-1, cmdData[3:3+length]))
				}
			}
		case CMD_COLLECTION_REG_WR_FEEDBACK: // 0x8006
			if len(cmdData) >= 8 {
				logBuilder.WriteString(fmt.Sprintf("    [0-3]   SN         : 0x%08X\n", binary.LittleEndian.Uint32(cmdData[0:4])))
				logBuilder.WriteString(fmt.Sprintf("    [4]     State      : %d (0:Fail, 1:Success)\n", cmdData[4]))
				logBuilder.WriteString(fmt.Sprintf("    [5]     Dut_sel    : 0x%02X\n", cmdData[5]))
				logBuilder.WriteString(fmt.Sprintf("    [6]     Reg_addr   : 0x%02X\n", cmdData[6]))
				logBuilder.WriteString(fmt.Sprintf("    [7]     Length     : %d\n", cmdData[7]))
				logBuilder.WriteString(fmt.Sprintf("    [8->]   Value(s)   : %X\n", cmdData[8:]))
			}
		case CMD_COLLECTION_REG_READ: // 0x07
			if len(cmdData) >= 3 {
				logBuilder.WriteString(fmt.Sprintf("    [0]     Dut_sel    : 0x%02X\n", cmdData[0]))
				logBuilder.WriteString(fmt.Sprintf("    [1]     Reg_addr   : 0x%02X\n", cmdData[1]))
				logBuilder.WriteString(fmt.Sprintf("    [2]     Length     : %d\n", cmdData[2]))
			}
		case CMD_COLLECTION_REG_RD_FEEDBACK: // 0x8007
			if len(cmdData) >= 8 {
				logBuilder.WriteString(fmt.Sprintf("    [0-3]   SN         : 0x%08X\n", binary.LittleEndian.Uint32(cmdData[0:4])))
				logBuilder.WriteString(fmt.Sprintf("    [4]     State      : %d (0:Fail, 1:Success)\n", cmdData[4]))
				logBuilder.WriteString(fmt.Sprintf("    [5]     Dut_sel    : 0x%02X\n", cmdData[5]))
				logBuilder.WriteString(fmt.Sprintf("    [6]     Reg_addr   : 0x%02X\n", cmdData[6]))
				logBuilder.WriteString(fmt.Sprintf("    [7]     Length     : %d\n", cmdData[7]))
				logBuilder.WriteString(fmt.Sprintf("    [8->]   Value(s)   : %X\n", cmdData[8:]))
			}
		case CMD_COLLECTION_SET_CHIP_TYPE: // 0x08
			if len(cmdData) >= 1 {
				logBuilder.WriteString(fmt.Sprintf("    [0]     ChipIndex  : %d\n", cmdData[0]))
			}
		case CMD_COLLECTION_SET_CHIP_TYPE_FEEDBACK: // 0x8008
			if len(cmdData) >= 6 {
				logBuilder.WriteString(fmt.Sprintf("    [0-3]   SN         : 0x%08X\n", binary.LittleEndian.Uint32(cmdData[0:4])))
				logBuilder.WriteString(fmt.Sprintf("    [4]     State      : %d (0:Fail, 1:Success)\n", cmdData[4]))
				logBuilder.WriteString(fmt.Sprintf("    [5]     ChipIndex  : %d\n", cmdData[5]))
			}
		default:
			logBuilder.WriteString(fmt.Sprintf("    (raw)            : %X\n", cmdData))
		}
	} else {
		logBuilder.WriteString("    (empty)\n")
	}

	logBuilder.WriteString(fmt.Sprintf("  [%-7d] CRC8        : 0x%02X\n", len(data)-1, crc8))
	logBuilder.WriteString("--------------------------------------------------")

	glog.Info(logBuilder.String())
}

var lastLogTime time.Time
var lastGenerateLogTime time.Time

func shouldLogHighFreqData() bool {
	if collectionConfig.LogIntervalMsec == 0 {
		return true // 0表示总是打印
	}
	if time.Since(lastLogTime) > time.Duration(collectionConfig.LogIntervalMsec)*time.Millisecond {
		lastLogTime = time.Now()
		return true
	}
	return false
}

func shouldLogGenerateData() bool {
	if collectionConfig.LogIntervalMsec == 0 {
		return true // 0表示总是打印
	}
	if time.Since(lastGenerateLogTime) > time.Duration(collectionConfig.LogIntervalMsec)*time.Millisecond {
		lastGenerateLogTime = time.Now()
		return true
	}
	return false
}

// 处理接收到的命令
func handleReceivedCommand(data []byte) {
	packet, err := bytesToXTKZPacket(data)
	if err != nil {
		glog.Errorf("解析接收命令失败: %v", err)
		return
	}

	// 打印详细的接收日志
	logPacketDetails(data, "Received", packet.Header.CmdID)

	glog.Infof("收到命令: CmdID=0x%04X, 数据长度=%d", packet.Header.CmdID, packet.Header.CmdDataSize)

	switch packet.Header.CmdID {
	case CMD_COLLECTION_START_STOP:
		handleStartStopCommand(packet.Data)
	case CMD_COLLECTION_DUT_POWER_CTRL:
		handleDUTPowerControl(packet.Data)
	case CMD_COLLECTION_DUT_VOLTAGE_QUERY:
		handleDUTVoltageQuery(packet.Data)
	case CMD_COLLECTION_FAULT_STATUS:
		handleFaultStatusQuery(packet.Data)
	case CMD_COLLECTION_TEST_CALIBRATION:
		handleTestCalibration(packet.Data)
	case CMD_COLLECTION_REG_WRITE:
		handleRegisterWrite(packet.Data)
	case CMD_COLLECTION_REG_READ:
		handleRegisterRead(packet.Data)
	case CMD_COLLECTION_SET_CHIP_TYPE:
		handleSetChipType(packet.Data)
	default:
		glog.Warningf("未知的命令ID: 0x%04X", packet.Header.CmdID)
	}
}

// 处理启停指令（CmdId=0x01）
func handleStartStopCommand(data []byte) {
	if len(data) < 7 {
		glog.Errorf("启停指令数据长度不足")
		return
	}

	state := data[0]
	dutActive := binary.LittleEndian.Uint16(data[1:3])
	timestamp := binary.LittleEndian.Uint32(data[3:7])

	glog.Infof("收到启停指令: 状态=%d, DUT激活=0x%04X, 时间戳=%d", state, dutActive, timestamp)

	// 更新设备状态
	if state == 0x01 {
		deviceState.TestState = 0x01 // 测试中
		deviceState.IsRunning = true
		deviceState.DUTActive = dutActive
	} else {
		deviceState.TestState = 0x00 // 停止
		deviceState.IsRunning = false
	}

	// 启停指令一般不需要单独的反馈，但会影响后续的0x8001主动上报
}

// 处理DUT电源控制（CmdId=0x02）
func handleDUTPowerControl(data []byte) {
	if len(data) < 3 {
		glog.Errorf("DUT电源控制数据长度不足")
		return
	}

	state := data[0]
	powerEnable := binary.LittleEndian.Uint16(data[1:3])

	glog.Infof("收到DUT电源控制: 状态=%d, 电源控制=0x%04X", state, powerEnable)

	// 模拟电源控制操作
	deviceState.PowerStates = powerEnable

	// 发送反馈（CmdId=0x8002）
	feedbackData := make([]byte, 7)
	binary.LittleEndian.PutUint32(feedbackData[0:4], deviceState.SN)
	feedbackData[4] = 0x01 // 成功
	binary.LittleEndian.PutUint16(feedbackData[5:7], deviceState.PowerStates)

	sendFeedback(CMD_COLLECTION_POWER_FEEDBACK, feedbackData)
}

// 处理DUT电压电流查询（CmdId=0x03）
func handleDUTVoltageQuery(data []byte) {
	glog.Info("收到DUT电压电流查询")

	// 发送反馈（CmdId=0x8003）
	feedbackData := make([]byte, 72) // 4+2+2+8*8

	binary.LittleEndian.PutUint32(feedbackData[0:4], deviceState.SN)

	// 模拟电压电流数据
	offset := 4
	// 主板供电
	binary.LittleEndian.PutUint16(feedbackData[offset:offset+2], 12000) // 12V
	binary.LittleEndian.PutUint16(feedbackData[offset+2:offset+4], 500) // 500mA
	offset += 4

	// DUT1-8的5V和3.3V供电
	for i := 0; i < 8; i++ {
		binary.LittleEndian.PutUint16(feedbackData[offset:offset+2], 5000)   // 5V
		binary.LittleEndian.PutUint16(feedbackData[offset+2:offset+4], 100)  // 100mA
		binary.LittleEndian.PutUint16(feedbackData[offset+4:offset+6], 3300) // 3.3V
		binary.LittleEndian.PutUint16(feedbackData[offset+6:offset+8], 50)   // 50mA
		offset += 8
	}

	sendFeedback(CMD_COLLECTION_VOLTAGE_FEEDBACK, feedbackData)
}

// 处理故障状态查询（CmdId=0x04）
func handleFaultStatusQuery(data []byte) {
	glog.Info("收到故障状态查询")

	// 发送反馈（CmdId=0x8004）
	feedbackData := make([]byte, 8)
	binary.LittleEndian.PutUint32(feedbackData[0:4], deviceState.SN)
	binary.LittleEndian.PutUint32(feedbackData[4:8], deviceState.FaultStates)

	sendFeedback(CMD_COLLECTION_FAULT_FEEDBACK, feedbackData)
}

// 处理测试标定指令（CmdId=0x05）
func handleTestCalibration(data []byte) {
	if len(data) < 2 {
		glog.Errorf("测试标定指令数据长度不足")
		return
	}

	dutSel := data[0]
	cmdLength := data[1]

	glog.Infof("收到测试标定指令: DUT选择=0x%02X, 指令长度=%d", dutSel, cmdLength)

	// 发送反馈（CmdId=0x8005）
	feedbackData := make([]byte, 5+int(cmdLength))
	binary.LittleEndian.PutUint32(feedbackData[0:4], deviceState.SN)
	feedbackData[4] = 0x01 // 成功
	if len(data) >= 2+int(cmdLength) {
		copy(feedbackData[5:], data[2:2+cmdLength]) // 回传指令
	}

	sendFeedback(CMD_COLLECTION_CALIBRATION_FEEDBACK, feedbackData)
}

// 处理寄存器写入指令（CmdId=0x06）
func handleRegisterWrite(data []byte) {
	if len(data) < 3 {
		glog.Errorf("寄存器写入指令数据长度不足")
		return
	}

	dutSel := data[0]
	regAddr := data[1]
	length := data[2]
	var writeValue []byte
	if len(data) >= 3+int(length) {
		writeValue = data[3 : 3+int(length)]
	} else {
		glog.Errorf("寄存器写入指令数据值长度不足")
		return
	}

	glog.Infof("收到寄存器写入指令: DUT选择=0x%02X, 寄存器地址=0x%02X, 长度=%d", dutSel, regAddr, length)

	// 根据协议，为每个使能的DUT构建反馈
	var feedbackPayload []byte
	var enabledDutCount int
	for i := 0; i < 8; i++ {
		if (dutSel & (1 << i)) != 0 {
			enabledDutCount++
			// 模拟从该DUT读回写入的值
			feedbackPayload = append(feedbackPayload, writeValue...)
		}
	}

	// 完整反馈包: SN(4) + State(1) + Dut_sel(1) + Reg_addr(1) + length(1) + M*n(data)
	feedbackData := make([]byte, 8+len(feedbackPayload))
	offset := 0
	binary.LittleEndian.PutUint32(feedbackData[offset:offset+4], deviceState.SN)
	offset += 4
	feedbackData[offset] = 0x01 // State: 成功
	offset++
	feedbackData[offset] = dutSel // Dut_sel: 反馈使能位
	offset++
	feedbackData[offset] = regAddr
	offset++
	feedbackData[offset] = length
	offset++
	copy(feedbackData[offset:], feedbackPayload)

	sendFeedback(CMD_COLLECTION_REG_WR_FEEDBACK, feedbackData)
}

// 处理寄存器读取指令（CmdId=0x07）
func handleRegisterRead(data []byte) {
	if len(data) < 3 {
		glog.Errorf("寄存器读取指令数据长度不足")
		return
	}

	dutSel := data[0]
	regAddr := data[1]
	length := data[2]

	glog.Infof("收到寄存器读取指令: DUT选择=0x%02X, 寄存器地址=0x%02X, 长度=%d", dutSel, regAddr, length)

	// 根据协议，为每个使能的DUT构建反馈
	var feedbackPayload []byte
	for i := 0; i < 8; i++ {
		if (dutSel & (1 << i)) != 0 {
			// 模拟从该DUT读取的数据
			dummyData := make([]byte, length)
			for j := 0; j < int(length); j++ {
				dummyData[j] = byte(0xAA + i + j) // 为每个DUT生成不同的模拟数据
			}
			feedbackPayload = append(feedbackPayload, dummyData...)
		}
	}

	// 完整反馈包: SN(4) + State(1) + Dut_sel(1) + Reg_addr(1) + length(1) + M*n(data)
	feedbackData := make([]byte, 8+len(feedbackPayload))
	offset := 0
	binary.LittleEndian.PutUint32(feedbackData[offset:offset+4], deviceState.SN)
	offset += 4
	feedbackData[offset] = 0x01 // State: 成功
	offset++
	feedbackData[offset] = dutSel // Dut_sel: 反馈使能位
	offset++
	feedbackData[offset] = regAddr
	offset++
	feedbackData[offset] = length
	offset++
	copy(feedbackData[offset:], feedbackPayload)

	sendFeedback(CMD_COLLECTION_REG_RD_FEEDBACK, feedbackData)
}

// 处理设置产品类型（CmdId=0x08）
func handleSetChipType(data []byte) {
	if len(data) < 1 {
		glog.Errorf("设置产品类型指令数据长度不足")
		return
	}
	chipIndex := data[0]
	glog.Infof("收到设置产品类型指令: ChipIndex=%d", chipIndex)

	// 更新设备状态中的芯片类型
	deviceState.ChipIndex = chipIndex

	// 发送反馈 (CmdID=0x8008)
	feedbackData := make([]byte, 6)
	binary.LittleEndian.PutUint32(feedbackData[0:4], deviceState.SN)
	feedbackData[4] = 1 // State: 成功
	feedbackData[5] = deviceState.ChipIndex
	sendFeedback(CMD_COLLECTION_SET_CHIP_TYPE_FEEDBACK, feedbackData)
}

// 生成测试板反馈数据（CmdId=0x8001）- 主动上报
func generateTestBoardFeedback() {
	if !deviceState.IsRunning {
		return // 只有在运行状态才主动上报
	}

	// --- 动态数据生成参数 ---
	// 使用全局计数器来驱动正弦波，模拟平滑变化
	counter := float64(gSimulationCounter)
	// 基础噪声幅度
	noiseLevel := 10

	// 根据新协议（已移除Header、Start、SampleCRC）计算数据长度
	// 1(TestState) + 4(SN) + 4(Time) + 2(Dut_active) + 1(ChipIndex) + 8*30(DUTs) + 32(External)
	dutCount := 8 // 新协议固定为8个DUT
	dataSize := 1 + 4 + 4 + 2 + 1 + (dutCount * 30) + 32
	feedbackData := make([]byte, dataSize)

	offset := 0
	// Test state
	feedbackData[offset] = deviceState.TestState
	offset++
	// SN
	binary.LittleEndian.PutUint32(feedbackData[offset:offset+4], deviceState.SN)
	offset += 4
	// Time
	binary.LittleEndian.PutUint32(feedbackData[offset:offset+4], uint32(time.Now().Unix()))
	offset += 4
	// Dut_active
	binary.LittleEndian.PutUint16(feedbackData[offset:offset+2], deviceState.DUTActive)
	offset += 2
	// ChipIndex (全局)
	feedbackData[offset] = deviceState.ChipIndex
	offset++

	// 8个DUT的数据块 (每个30字节)
	for i := 0; i < dutCount; i++ {
		// 检查该DUT是否启用
		if (deviceState.DUTActive & (1 << i)) != 0 {
			// --- 为每个通道生成动态数据 ---
			// 每个DUT有不同的相位，使其数据看起来不同
			phase := float64(i * 15)

			// Gyro数据: 基础值 + 较大幅度的正弦波 + 少量噪声
			gyroX := uint32(1000 + 200*math.Sin((counter+phase)/20.0) + float64(r.Intn(noiseLevel*2)-noiseLevel))
			gyroY := uint32(2000 + 250*math.Sin((counter+phase)/25.0) + float64(r.Intn(noiseLevel*2)-noiseLevel))
			gyroZ := uint32(3000 + 300*math.Sin((counter+phase)/30.0) + float64(r.Intn(noiseLevel*2)-noiseLevel))

			// Acc数据: 基础值 + 较小幅度的正弦波 + 少量噪声
			accX := uint32(100 + 20*math.Sin((counter+phase)/15.0) + float64(r.Intn(noiseLevel)-noiseLevel/2))
			accY := uint32(200 + 25*math.Sin((counter+phase)/18.0) + float64(r.Intn(noiseLevel)-noiseLevel/2))
			accZ := uint32(300 + 30*math.Sin((counter+phase)/22.0) + float64(r.Intn(noiseLevel)-noiseLevel/2))

			// Mix数据
			mix := uint32(50 + 15*math.Sin((counter+phase)/35.0) + float64(r.Intn(noiseLevel/2)))

			// 温度数据
			temp := uint16(3500 + 50*math.Sin((counter+phase)/100.0) + float64(r.Intn(20)-10))

			binary.LittleEndian.PutUint32(feedbackData[offset:offset+4], gyroX)
			binary.LittleEndian.PutUint32(feedbackData[offset+4:offset+8], gyroY)
			binary.LittleEndian.PutUint32(feedbackData[offset+8:offset+12], gyroZ)
			binary.LittleEndian.PutUint32(feedbackData[offset+12:offset+16], accX)
			binary.LittleEndian.PutUint32(feedbackData[offset+16:offset+20], accY)
			binary.LittleEndian.PutUint32(feedbackData[offset+20:offset+24], accZ)
			binary.LittleEndian.PutUint32(feedbackData[offset+24:offset+28], mix)
			binary.LittleEndian.PutUint16(feedbackData[offset+28:offset+30], temp)
		} else {
			// 如果DUT未启用，填充0数据
			for j := 0; j < 30; j++ {
				feedbackData[offset+j] = 0
			}
		}
		offset += 30 // 每个DUT采样30字节
	}

	// 外挂陀螺仪数据 (32字节)
	// 检查是否启用313标准标定数据（Bit15）
	if (deviceState.DUTActive & 0x8000) != 0 {
		// 外部陀螺仪数据也进行动态模拟
		gyroX_ext := uint32(9000 + 100*math.Sin(counter/18.0) + float64(r.Intn(noiseLevel*2)-noiseLevel))
		gyroY_ext := uint32(9100 + 120*math.Sin(counter/22.0) + float64(r.Intn(noiseLevel*2)-noiseLevel))
		gyroZ_ext := uint32(9200 + 150*math.Sin(counter/26.0) + float64(r.Intn(noiseLevel*2)-noiseLevel))
		accX_ext := uint32(900 + 10*math.Sin(counter/12.0) + float64(r.Intn(noiseLevel)-noiseLevel/2))
		accY_ext := uint32(910 + 12*math.Sin(counter/15.0) + float64(r.Intn(noiseLevel)-noiseLevel/2))
		accZ_ext := uint32(920 + 15*math.Sin(counter/18.0) + float64(r.Intn(noiseLevel)-noiseLevel/2))
		mix_ext := uint32(950 + 5*math.Sin(counter/30.0) + float64(r.Intn(noiseLevel/2)))
		temp_ext := uint16(3800 + 20*math.Sin(counter/90.0) + float64(r.Intn(10)-5))

		binary.LittleEndian.PutUint32(feedbackData[offset:offset+4], gyroX_ext)
		binary.LittleEndian.PutUint32(feedbackData[offset+4:offset+8], gyroY_ext)
		binary.LittleEndian.PutUint32(feedbackData[offset+8:offset+12], gyroZ_ext)
		binary.LittleEndian.PutUint32(feedbackData[offset+12:offset+16], accX_ext)
		binary.LittleEndian.PutUint32(feedbackData[offset+16:offset+20], accY_ext)
		binary.LittleEndian.PutUint32(feedbackData[offset+20:offset+24], accZ_ext)
		binary.LittleEndian.PutUint32(feedbackData[offset+24:offset+28], mix_ext)
		binary.LittleEndian.PutUint16(feedbackData[offset+28:offset+30], temp_ext)
	} else {
		// 如果313未启用，填充0数据
		for j := 0; j < 30; j++ {
			feedbackData[offset+j] = 0
		}
	}
	offset += 30

	// Gyro_counter
	binary.LittleEndian.PutUint16(feedbackData[offset:offset+2], uint16(time.Now().UnixMilli()%65536))
	offset += 2

	sendFeedback(CMD_COLLECTION_FEEDBACK_DATA, feedbackData)
	
	// 使用独立的时间间隔控制日志输出
	if shouldLogGenerateData() {
		glog.Infof("发送测试板反馈数据（主动上报），DUT数量=%d, DUTActive=0x%04X, 数据长度=%d", dutCount, deviceState.DUTActive, len(feedbackData))
	}

	// 每次生成数据后，递增全局计数器
	gSimulationCounter++
}

// 串口数据接收循环
func serialReceiveLoop() {
	receiveBuffer := make([]byte, 0, 8192) // 持续存在的缓冲区
	tempBuf := make([]byte, 4096)

	for {
		select {
		case <-stopChannel:
			glog.Info("串口接收循环退出")
			return
		default:
			if serialPort == nil {
				time.Sleep(100 * time.Millisecond) // 避免在串口未准备好时空转
				continue
			}

			n, err := serialPort.Read(tempBuf)
			if err != nil {
				// 在没有数据时，Read可能会返回超时错误，这是正常的，可以忽略
				if !strings.Contains(err.Error(), "timeout") {
					glog.Errorf("串口读取失败: %v", err)
				}
				time.Sleep(20 * time.Millisecond) // 发生错误或超时后稍作等待
				continue
			}

			if n > 0 {
				receiveBuffer = append(receiveBuffer, tempBuf[:n]...)
			}

			// 循环处理缓冲区中的所有完整数据包
			for {
				// 1. 寻找包头
				startIndex := -1
				for i := 0; i <= len(receiveBuffer)-4; i++ {
					if binary.LittleEndian.Uint32(receiveBuffer[i:i+4]) == 0x58544B5A {
						startIndex = i
						break
					}
				}

				if startIndex == -1 {
					// 没有找到包头，等待更多数据。
					// 为防止缓冲区无限增长，如果缓冲区很大但仍未找到包头，则清空。
					if len(receiveBuffer) > 4096 {
						glog.Warningf("在 %d 字节中未找到包头，清空接收缓冲区", len(receiveBuffer))
						receiveBuffer = receiveBuffer[:0]
					}
					break // 退出内层循环，等待下一次串口读取
				}

				// 2. 丢弃包头前的无效数据
				if startIndex > 0 {
					glog.Warningf("丢弃 %d 字节的无效串口数据", startIndex)
					receiveBuffer = receiveBuffer[startIndex:]
				}

				// 3. 检查头部长度是否足够
				if len(receiveBuffer) < 8 {
					break // 数据不足以读取完整头部，等待更多数据
				}

				// 4. 解析数据包总长度
				dataSize := binary.LittleEndian.Uint16(receiveBuffer[6:8])
				totalPacketLen := 8 + int(dataSize) + 1 // Header(8) + Data(N) + CRC(1)

				// 5. 检查整个包的数据是否已完全接收
				if len(receiveBuffer) < totalPacketLen {
					break // 数据包不完整，等待更多数据
				}

				// 6. 提取并处理完整的数据包
				packetBytes := receiveBuffer[:totalPacketLen]
				glog.V(2).Infof("从缓冲区找到完整数据包，长度=%d", len(packetBytes))
				handleReceivedCommand(packetBytes)

				// 7. 从缓冲区移除已处理的数据包
				receiveBuffer = receiveBuffer[totalPacketLen:]
			}
		}
	}
}

// 设置配置参数的公共方法
func SetCollectionConfig(siteNum int, enable bool, uartPort string, baudRate int) {
	if collectionConfig == nil {
		initCollectionConfig()
	}

	if siteNum > 0 {
		collectionConfig.SiteNum = siteNum
	}
	collectionConfig.Enable = enable
	if uartPort != "" {
		collectionConfig.UartPort = uartPort
	}
	if baudRate > 0 {
		collectionConfig.BaudRate = baudRate
	}
}

// 启动采集服务器 - 简化的对外接口
func DoCollection() {
	glog.Info("正在启动采集设备模块...")

	// 加载配置文件
	if err := loadCollectionConfig(); err != nil {
		glog.Warningf("加载采集配置失败，使用默认配置: %v", err)
	}

	// 检查是否启用
	if !collectionConfig.Enable {
		glog.Info("采集设备模块未启用，跳过启动")
		return
	}

	// 初始化设备状态
	initDeviceState()

	// 初始化串口
	if err := initSerialPort(); err != nil {
		glog.Errorf("串口初始化失败: %v", err)
		return
	}

	glog.Infof("采集设备模块启动成功，SITE数量: %d, 串口: %s, 波特率: %d, 设备SN=0x%08X",
		collectionConfig.SiteNum, collectionConfig.UartPort, collectionConfig.BaudRate, collectionConfig.DeviceSN)

	// 初始化停止通道
	stopChannel = make(chan bool, 1)

	// 启动串口接收循环
	go serialReceiveLoop()

	// 根据配置选择工作模式
	if collectionConfig.HighSpeedSimulation {
		glog.Warningf("进入高速数据模拟模式，发送间隔: %d us", collectionConfig.SendIntervalMicrosec)
		// 高速发送循环
		go highSpeedSendLoop()
	} else {
		glog.Info("进入标准（1Hz）数据发送模式")
		// 标准低速循环
		go standardSendLoop()
	}

	// 主循环保持运行，等待停止信号
	<-stopChannel
	glog.Info("采集设备模块主循环已收到停止信号并退出。")
}

// standardSendLoop 标准（1Hz）发送循环
func standardSendLoop() {
	ticker := time.NewTicker(1 * time.Second)
	defer ticker.Stop()

	glog.Info("标准发送循环已启动，定时器间隔: 1秒")

	for {
		select {
		case <-stopChannel:
			glog.Info("标准发送循环退出")
			return
		case <-ticker.C:
			glog.V(1).Info("定时器触发，准备发送数据")
			generateTestBoardFeedback()
		}
	}
}

// highSpeedSendLoop 高速发送循环
func highSpeedSendLoop() {
	// 如果间隔大于0，则创建定时器
	var ticker *time.Ticker
	if collectionConfig.SendIntervalMicrosec > 0 {
		ticker = time.NewTicker(time.Duration(collectionConfig.SendIntervalMicrosec) * time.Microsecond)
		defer ticker.Stop()
	}

	for {
		select {
		case <-stopChannel:
			glog.Info("高速发送循环退出")
			return
		default:
			// 如果间隔为0，则无延迟循环
			if ticker == nil {
				generateTestBoardFeedback()
			} else {
				// 等待下一个tick
				<-ticker.C
				generateTestBoardFeedback()
			}
		}
	}
}

// 可选的配置接口
func ConfigureCollection(siteNum int, enable bool, uartPort string, baudRate int) {
	SetCollectionConfig(siteNum, enable, uartPort, baudRate)
}

// 获取当前配置信息
func GetCollectionConfig() map[string]interface{} {
	if collectionConfig == nil {
		loadCollectionConfig()
	}

	return map[string]interface{}{
		"enable":     collectionConfig.Enable,
		"site_num":   collectionConfig.SiteNum,
		"uart_port":  collectionConfig.UartPort,
		"baud_rate":  collectionConfig.BaudRate,
		"device_sn":  collectionConfig.DeviceSN,
		"chip_idx":   collectionConfig.ChipIdx,
		"dut_count":  collectionConfig.DUTCount,
		"dut_active": collectionConfig.DUTActive,
	}
}

// 停止采集
func StopCollection() {
	glog.Info("停止采集设备模块")

	if stopChannel != nil {
		select {
		case stopChannel <- true:
		default:
		}
	}

	if serialPort != nil {
		serialPort.Close()
		serialPort = nil
	}
}
