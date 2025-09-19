package util

import (
	"os"
	"runtime"

	"github.com/golang/glog"
	"golang.org/x/sys/windows"
)

func GetByteEnableCnt(result uint64, MaxSktIdx int) int {
	sktIdx := 0
	count := 0
	for sktIdx < MaxSktIdx {
		if result>>sktIdx&0x1 == 1 {
			count++
		}
		sktIdx++
	}
	return count
}

func ToBytes(num uint64) []byte {
	bytes := make([]byte, 8)
	bytes[0] = byte(num >> 0)
	bytes[1] = byte(num >> 8)
	bytes[2] = byte(num >> 16)
	bytes[3] = byte(num >> 24)
	bytes[4] = byte(num >> 32)
	bytes[5] = byte(num >> 40)
	bytes[6] = byte(num >> 48)
	bytes[7] = byte(num >> 56)
	return bytes
}

// GetDetailedHostname 获取详细的主机名，在 Windows 上会尝试获取完整的主机信息
func GetDetailedHostname() (string, error) {
	if runtime.GOOS == "windows" {
		var n uint32 = 128
		b := make([]uint16, n)
		err := windows.GetComputerNameEx(windows.ComputerNameDnsFullyQualified, &b[0], &n)
		if err != nil {
			glog.Errorf("GetComputerNameEx Hostname fail %s", err.Error())
			// 如果获取详细信息失败，则回退到标准方法
			return os.Hostname()
		}

		return windows.UTF16ToString(b[:n]), nil
	}

	// 对于非 Windows 系统，使用标准的 os.Hostname()
	return os.Hostname()
}
