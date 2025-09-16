package server

import (
	"encoding/binary"
	"fmt"
)

// CmdPackHeader 命令包头结构体
type CmdPackHeader struct {
	CmdFlag     uint32 // 命令标记，预留使用，默认为0
	CmdID       uint16 // 不同的命令码有不同的功能
	CmdDataSize uint16 // 不同的命令码有不同的命令数据长度
}

// CmdPacket 完整的命令包结构
type CmdPacket struct {
	Header CmdPackHeader
	Data   []byte // 实际的命令数据
}

// 序列化命令包为字节数组
func (packet *CmdPacket) ToBytes() []byte {
	headerSize := 8 // CmdPackHeader 大小
	totalSize := headerSize + len(packet.Data)

	buffer := make([]byte, totalSize)

	// 写入包头
	binary.LittleEndian.PutUint32(buffer[0:4], packet.Header.CmdFlag)
	binary.LittleEndian.PutUint16(buffer[4:6], packet.Header.CmdID)
	binary.LittleEndian.PutUint16(buffer[6:8], packet.Header.CmdDataSize)

	// 写入数据
	copy(buffer[8:], packet.Data)

	return buffer
}

// 从字节数组反序列化命令包
func FromBytes(data []byte) (*CmdPacket, error) {
	if len(data) < 8 {
		return nil, fmt.Errorf("数据长度不足，至少需要8字节")
	}

	packet := &CmdPacket{}

	// 读取包头
	packet.Header.CmdFlag = binary.LittleEndian.Uint32(data[0:4])
	packet.Header.CmdID = binary.LittleEndian.Uint16(data[4:6])
	packet.Header.CmdDataSize = binary.LittleEndian.Uint16(data[6:8])

	// 读取数据
	if len(data) >= 8+int(packet.Header.CmdDataSize) {
		packet.Data = make([]byte, packet.Header.CmdDataSize)
		copy(packet.Data, data[8:8+packet.Header.CmdDataSize])
	}

	return packet, nil
}

// History序列化器
type THistory struct {
	Magic    uint32
	Version  uint32
	TotalCnt uint32
	PassCnt  uint32
	FailCnt  uint16 // Changed from uint2_tool_16393f47_33c1_458b_b2da_d1975d6b5 to uint16
}

// tCmd4 结构体定义
type tCmd4 struct {
	Method    string `json:"Method"`
	NewRealId string `json:"NewRealId"`
	OldRealId string `json:"OldRealId"`
}

// ProductInfo 命令结构
type ProductInfo struct {
	Command    string       `json:"Command"`
	RotateInfo []RotateItem `json:"RotateInfo"`
}

// RotateItem 转台项目结构
type RotateItem struct {
	RotateIdx   string `json:"RotateIdx"`   // 转台插座编号，例如 "1-1"
	FromUnitIdx string `json:"FromUnitIdx"` // 产品来源单元号（站点号）
	FromSktIdx  string `json:"FromSktIdx"`  // 产品来源插座号
}

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
