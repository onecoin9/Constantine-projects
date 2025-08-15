#pragma once

// SCS_SN_YCHUANG 配置类
// 用于管理 YCHUANG 序列号生成器的配置参数

class CSCS_SN_YCHUANGCfg
{
public:
	CSCS_SN_YCHUANGCfg();
	virtual ~CSCS_SN_YCHUANGCfg();

	// 配置参数
	CString m_strDeviceModel;		// 设备型号
	CString m_strSerialPrefix;		// 序列号前缀
	int m_nSerialLength;			// 序列号长度
	BOOL m_bEnableChecksum;			// 是否启用校验和
	CString m_strOutputFormat;		// 输出格式

	// 配置方法
	BOOL LoadConfig(const CString& strConfigFile);
	BOOL SaveConfig(const CString& strConfigFile);
	BOOL SetDefaultConfig();
	
	// 序列号生成方法
	CString GenerateSerialNumber();
	BOOL ValidateSerialNumber(const CString& strSerial);
	
private:
	// 私有辅助方法
	BOOL ParseConfigLine(const CString& strLine);
	CString CalculateChecksum(const CString& strData);
};
