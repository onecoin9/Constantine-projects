package server

import (
	"fmt"
	"log"
	"encoding/hex"
	"vauto/util"
	"io"
	"github.com/jacobsa/go-serial/serial"
	"github.com/golang/glog"
	"github.com/Unknwon/goconfig"
)

// 内部配置结构体
type tConfigFProtocal struct {
	Enable          bool
	UartPort        string
	BaudRate        int
	ProtocalVersion int
	Quantity        int
	SiteNum         int
	SiteEnable      string
}

// 全局配置实例
var configFProtocal *tConfigFProtocal

// 从配置文件加载F协议配置
func loadFProtocalConfig() error {
	cfg, err := goconfig.LoadConfigFile("config.ini")
	if err != nil {
		glog.Errorf("加载配置文件失败: %v", err)
		// 使用默认配置
		initFProtocalConfig()
		return err
	}
	
	configFProtocal = &tConfigFProtocal{
		Enable:          cfg.MustBool("fprotocal", "enable", false),
		UartPort:        cfg.MustValue("fprotocal", "uart_port", "COM6"),
		BaudRate:        cfg.MustInt("fprotocal", "baud_rate", 115200),
		ProtocalVersion: cfg.MustInt("fprotocal", "protocal_version", 1),
		Quantity:        cfg.MustInt("fprotocal", "quantity", 1),
		SiteNum:         cfg.MustInt("fprotocal", "site_num", 1),
		SiteEnable:      cfg.MustValue("fprotocal", "site_enable", "01"),
	}
	
	// 验证站点使能配置
	SiteEnable, err := hex.DecodeString(configFProtocal.SiteEnable)
	if err != nil {
		glog.Errorf("DecodeString Read: %s", err.Error())
		return err
	}
	if len(SiteEnable) != configFProtocal.SiteNum {
		glog.Errorf("使能与站点数量不匹配: 使能长度=%d, 站点数量=%d", len(SiteEnable), configFProtocal.SiteNum)
		return fmt.Errorf("使能与站点数量不匹配")
	}
	
	glog.Infof("F协议配置已加载: UartPort=%s, BaudRate=%d, SiteNum=%d, Quantity=%d", 
		configFProtocal.UartPort, configFProtocal.BaudRate, configFProtocal.SiteNum, configFProtocal.Quantity)
	
	return nil
}

// 初始化默认配置
func initFProtocalConfig() {
	configFProtocal = &tConfigFProtocal{
		UartPort:        "COM6",
		BaudRate:        115200,
		ProtocalVersion: 1,
		Quantity:        1,
		SiteNum:         1,
		SiteEnable:      "01",
	}
	glog.Info("使用默认F协议配置")
}

// 设置配置参数的公共方法（可选）
func SetFProtocalConfig(uartPort string, baudRate int, protocalVersion int, quantity int, siteNum int, siteEnable string) {
	if configFProtocal == nil {
		initFProtocalConfig()
	}
	
	if uartPort != "" {
		configFProtocal.UartPort = uartPort
	}
	if baudRate > 0 {
		configFProtocal.BaudRate = baudRate
	}
	if protocalVersion > 0 {
		configFProtocal.ProtocalVersion = protocalVersion
	}
	if quantity > 0 {
		configFProtocal.Quantity = quantity
	}
	if siteNum > 0 {
		configFProtocal.SiteNum = siteNum
	}
	if siteEnable != "" {
		configFProtocal.SiteEnable = siteEnable
	}
}

// 启动F协议服务器 - 简化的对外接口
func DoFServer() {
	glog.Info("正在启动F协议模块...")
	
	// 加载配置文件
	if err := loadFProtocalConfig(); err != nil {
		glog.Errorf("加载F协议配置失败: %v", err)
		return
	}

	if configFProtocal.Enable {
		glog.Infof("F协议模块启动成功，串口: %s, 波特率: %d, 站点数: %d, 目标数量: %d", 
			configFProtocal.UartPort, configFProtocal.BaudRate, configFProtocal.SiteNum, configFProtocal.Quantity)
	}else {
		glog.Info("F协议模块未启用")
		return
	}

	// 设置全局变量（为了兼容现有逻辑）
	sStopQuantity = configFProtocal.Quantity
	
	// 启动F协议服务器
	doFServer()
}

func doFServer() {
	SiteEnable, err := hex.DecodeString(configFProtocal.SiteEnable)
	if err != nil {
		log.Printf("DecodeString SiteEnable Read: %s\n", err.Error())
	}
	sktIdx := 0
	for sktIdx < configFProtocal.SiteNum {
		one_site_enable := byte(SiteEnable[configFProtocal.SiteNum-sktIdx-1]) //byte((SiteEnable >> (8 * sktIdx)) & 0xFF)
		SitesInfo[sktIdx].Idx = sktIdx + 1
		SitesInfo[sktIdx].Enabled = uint64(one_site_enable)
		sktIdx++
	}

	//设置串口编号
	options := serial.OpenOptions{
		PortName:              configFProtocal.UartPort,
		BaudRate:              uint(configFProtocal.BaudRate),
		DataBits:              8,
		StopBits:              1,
		MinimumReadSize:       2,
		ParityMode:            serial.PARITY_NONE,
		InterCharacterTimeout: 5000,
	}
	log.Printf("Open uart %s\n", configFProtocal.UartPort)
	//打开串口
	port, err := serial.Open(options)
	if err != nil {
		log.Printf("Error OpenPort: %s\n", err.Error())
		return
	}
	defer func() {
		err := port.Close()
		if err != nil {
			log.Fatal(err)
		}
		log.Printf("%s closed\n", configFProtocal.UartPort)
	}()
	//保持数据持续接收
	for {
		buf := make([]byte, 1024)
		lens, err := port.Read(buf)
		if err != nil {
			if err != io.EOF {
				fmt.Println("Error reading from serial port: ", err)
			}
			log.Printf("Error Read: %s\n", err.Error())
			continue
		}
		if lens > 4 {
			//30 31 36 33 34 30 30 39 0D
			revData := buf[:lens-1]
			recvstr := string(revData)
			log.Printf("[CMD]Recv command from AutoApp: %s\n", recvstr)
			cmd, err := hex.DecodeString(recvstr)
			if err != nil {
				log.Printf("DecodeString Read: %s\n", err.Error())
			}
			if cmd[1] == 0x63 {
				//===>01634009 <===
				log.Printf("[CMD]====>请求自动化设备告知当前协议版本")
				writebuf := make([]byte, 1024)
				crcout := make([]byte, 2)
				writebuf[0] = cmd[0]
				writebuf[1] = cmd[1]
				writebuf[2] = 0x1
				writebuf[3] = byte(configFProtocal.ProtocalVersion)
				util.CalcModbusCRC16(writebuf[0:4], crcout)
				writebuf[4] = crcout[1]
				writebuf[5] = crcout[0]
				writestring := hex.EncodeToString(writebuf[0:6])
				log.Printf("[CMD]<====VAuto告诉App协议版本")
				log.Printf("[CMD]Send command to AutoApp: %s\n", writestring)
				var final_write_buffer []byte = []byte(writestring)
				writeComData(port, final_write_buffer)
				writeEndbuf := make([]byte, 1)
				writeEndbuf[0] = 0x0d
				writeComData(port, writeEndbuf)
			} else if cmd[1] == 0x64 {
				// ===>016401CB <===016401FF01D7
				log.Printf("[CMD]====>App请求自动化设备站点使能情况")
				writebuf := make([]byte, 1024)
				crcout := make([]byte, 2)
				writebuf[0] = cmd[0]
				writebuf[1] = cmd[1]
				writebuf[2] = byte(configFProtocal.SiteNum)
				sktIdx := 0
				for sktIdx < configFProtocal.SiteNum {
					one_site_enable := byte(SiteEnable[configFProtocal.SiteNum-sktIdx-1]) //byte((SiteEnable >> (8 * sktIdx)) & 0xFF)
					log.Printf("[CMD]自动化设备站点使能情况SiteEnable:%x Site[%d] Enable:%02X", SiteEnable, sktIdx, one_site_enable)
					writebuf[3+sktIdx] = byte(SitesInfo[sktIdx].Enabled)
					sktIdx++
				}
				util.CalcModbusCRC16(writebuf[0:3+configFProtocal.SiteNum], crcout)
				writebuf[3+configFProtocal.SiteNum] = crcout[1]
				writebuf[4+configFProtocal.SiteNum] = crcout[0]
				writestring := hex.EncodeToString(writebuf[0 : 5+configFProtocal.SiteNum])
				log.Printf("[CMD]<====VAuto告诉自动化设备站点使能情况")
				log.Printf("[CMD]Send command to AutoApp: %s\n", writestring)
				var cmd4_write_buffer []byte = []byte(writestring)
				writeComData(port, cmd4_write_buffer)
				writeEndbuf := make([]byte, 1)
				writeEndbuf[0] = 0x0d
				writeComData(port, writeEndbuf)
			} else if cmd[1] == 0x65 {
				// ===>016501CB <===016401FF01D7
				siteidx := cmd[2] - 0xA
				log.Printf("[CMD]====>App请求自动化设备告知Site:%c: 芯片是否放好", siteidx+'A')
				writebuf := make([]byte, 1024)
				crcout := make([]byte, 2)
				writebuf[0] = cmd[0]
				writebuf[1] = cmd[1]
				writebuf[2] = cmd[2]
				writebuf[3] = 0x01

				remainNum := sStopQuantity - sSupplyQuantity
				if configSystem.DoSupplyCheck && remainNum <= 0 {
					writebuf[3] = 0x02
					log.Printf("[CMD]<====VAuto告诉App站点Site:%c: 芯片数量达到%d，不再放置芯片", siteidx+'A', sStopQuantity)
				} else {
					curSupplyNum := util.GetByteEnableCnt(SitesInfo[siteidx].Enabled, sMaxSktIdx)
					sSupplyQuantity += curSupplyNum
					remainNum -= curSupplyNum
					log.Printf("[CMD]<====VAuto告诉App站点Site:%c: 芯片已经放好，目标数:%d 已供给%d 剩余:%d",
						siteidx+'A', sStopQuantity, sSupplyQuantity, remainNum)
				}

				util.CalcModbusCRC16(writebuf[0:4], crcout)
				writebuf[4] = crcout[1]
				writebuf[5] = crcout[0]

				writestring := hex.EncodeToString(writebuf[0:6])
				log.Printf("[CMD]Send command to AutoApp: %s\n", writestring)
				var cmd4_write_buffer []byte = []byte(writestring)
				writeComData(port, cmd4_write_buffer)
				writeEndbuf := make([]byte, 1)
				writeEndbuf[0] = 0x0d
				writeComData(port, writeEndbuf)
			} else if cmd[1] == 0x66 {
				// ===>01660A0126A7 <===01660a8ba7
				siteidx := cmd[2] - 0xA
				result := cmd[3]
				sktIdx := 0
				for sktIdx < sMaxSktIdx {
					if result>>sktIdx&0x1 == 1 {
						sProductOKQuantity++
					}
					//SKT使能了结果却返回0
					if (result>>sktIdx&0x1) == 0 && (SitesInfo[siteidx-1].Enabled>>sktIdx&0x1 == 1) {
						sProductNGQuantity++
					}
					sktIdx++
				}
				log.Printf("[CMD]====>App通知芯片烧录结果 Site:%c: 结果:0x%02x 烧录成功数量:%d 失败数量:%d", siteidx+'A', result, sProductOKQuantity, sProductNGQuantity)
				writebuf := make([]byte, 1024)
				crcout := make([]byte, 2)
				writebuf[0] = cmd[0]
				writebuf[1] = cmd[1]
				writebuf[2] = cmd[2] //FIXME + 1 测试发送结果失败情况
				util.CalcModbusCRC16(writebuf[0:3], crcout)
				writebuf[3] = crcout[1]
				writebuf[4] = crcout[0]
				writestring := hex.EncodeToString(writebuf[0:5])
				log.Printf("[CMD]<====VAuto告知App收到Site:%c: 芯片烧录结果", siteidx+'A')
				log.Printf("[CMD]Send command to AutoApp: %s\n", writestring)
				var cmd4_write_buffer []byte = []byte(writestring)
				writeComData(port, cmd4_write_buffer)
				writeEndbuf := make([]byte, 1)
				writeEndbuf[0] = 0x0d
				writeComData(port, writeEndbuf)

			} else if cmd[1] == 0x67 {
				// ===>016741CA <===016741CA
				log.Printf("[CMD]====>App告知自动化初始化已经完成,可以进行芯片的取放")
				writebuf := make([]byte, 1024)
				crcout := make([]byte, 2)
				writebuf[0] = cmd[0]
				writebuf[1] = cmd[1]
				sProductOKQuantity = 0
				sProductNGQuantity = 0
				sSupplyQuantity = 0
				util.CalcModbusCRC16(writebuf[0:2], crcout)
				writebuf[2] = crcout[1]
				writebuf[3] = crcout[0]
				writestring := hex.EncodeToString(writebuf[0:4])
				log.Printf("[CMD]<====VAuto告知收到告知App初始化已经完成消息")
				log.Printf("[CMD]Send command to AutoApp: %s\n", writestring)
				var cmd4_write_buffer []byte = []byte(writestring)
				writeComData(port, cmd4_write_buffer)
				writeEndbuf := make([]byte, 1)
				writeEndbuf[0] = 0x0d
				writeComData(port, writeEndbuf)
			} else if cmd[1] == 0x68 {
				// ===>01680B4E07 <===01680b0102f433
				siteidx := cmd[2] - 0xA
				log.Printf("[CMD]====>App请求自动化设备告知Site:%c: 芯片是否放好", siteidx+'A')
				writebuf := make([]byte, 1024)
				crcout := make([]byte, 2)
				writebuf[0] = cmd[0]
				writebuf[1] = cmd[1]
				writebuf[2] = cmd[2]
				writebuf[3] = 0x01
				writebuf[4] = byte(SitesInfo[siteidx].Enabled)

				remainNum := sStopQuantity - sSupplyQuantity
				if configSystem.DoSupplyCheck && remainNum <= 0 {
					writebuf[3] = 0x02
					log.Printf("[CMD]<====VAuto告诉App站点Site:%c: 芯片数量达到%d,不再放置芯片", siteidx+'A', sStopQuantity)
				} else {
					curSupplyNum := util.GetByteEnableCnt(SitesInfo[siteidx].Enabled, sMaxSktIdx)
					sSupplyQuantity += curSupplyNum
					remainNum -= curSupplyNum
					log.Printf("[CMD]<====VAuto告诉App站点Site:%c: 芯片已经放好，目标数:%d 已供给%d 剩余:%d",
						siteidx+'A', sStopQuantity, sSupplyQuantity, remainNum)
				}

				util.CalcModbusCRC16(writebuf[0:5], crcout)
				writebuf[5] = crcout[1]
				writebuf[6] = crcout[0]

				writestring := hex.EncodeToString(writebuf[0:7])
				log.Printf("[CMD]Send command to AutoApp: %s\n", writestring)
				var cmd4_write_buffer []byte = []byte(writestring)
				writeComData(port, cmd4_write_buffer)
				writeEndbuf := make([]byte, 1)
				writeEndbuf[0] = 0x0d
				writeComData(port, writeEndbuf)
			}
		}
	}
}

// 可选的配置接口，用于在启动前设置参数
func ConfigureFProtocal(uartPort string, baudRate int, protocalVersion int, quantity int, siteNum int, siteEnable string) {
	SetFProtocalConfig(uartPort, baudRate, protocalVersion, quantity, siteNum, siteEnable)
}

// 获取当前配置信息
func GetFProtocalConfig() map[string]interface{} {
	if configFProtocal == nil {
		loadFProtocalConfig()
	}
	
	return map[string]interface{}{
		"uart_port":        configFProtocal.UartPort,
		"baud_rate":        configFProtocal.BaudRate,
		"protocal_version": configFProtocal.ProtocalVersion,
		"quantity":         configFProtocal.Quantity,
		"site_num":         configFProtocal.SiteNum,
		"site_enable":      configFProtocal.SiteEnable,
	}
}

func writeComData(port io.ReadWriteCloser, data []byte) {
	comWriteMutex.Lock()
	port.Write(data)
	defer comWriteMutex.Unlock()
}