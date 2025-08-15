// SCS_SN_YCHUANGCfg.cpp : 实现文件
//

#include "stdafx.h"
#include "SCS_SN_YCHUANGCfg.h"
#include <fstream>

// CSCS_SN_YCHUANGCfg

CSCS_SN_YCHUANGCfg::CSCS_SN_YCHUANGCfg()
{
	SetDefaultConfig();
}

CSCS_SN_YCHUANGCfg::~CSCS_SN_YCHUANGCfg()
{
}

BOOL CSCS_SN_YCHUANGCfg::LoadConfig(const CString& strConfigFile)
{
	// TODO: 在此添加配置文件加载代码
	// 这里可以实现从INI文件或其他格式加载配置
	return TRUE;
}

BOOL CSCS_SN_YCHUANGCfg::SaveConfig(const CString& strConfigFile)
{
	// TODO: 在此添加配置文件保存代码
	// 这里可以实现保存配置到INI文件或其他格式
	return TRUE;
}

BOOL CSCS_SN_YCHUANGCfg::SetDefaultConfig()
{
	m_strDeviceModel = _T("YCHUANG-001");
	m_strSerialPrefix = _T("YC");
	m_nSerialLength = 12;
	m_bEnableChecksum = TRUE;
	m_strOutputFormat = _T("HEX");
	return TRUE;
}

CString CSCS_SN_YCHUANGCfg::GenerateSerialNumber()
{
	// TODO: 在此添加序列号生成代码
	// 这里可以实现具体的序列号生成算法
	CString strSerial;
	strSerial.Format(_T("%s%08X"), m_strSerialPrefix, GetTickCount());
	
	if (m_bEnableChecksum)
	{
		CString strChecksum = CalculateChecksum(strSerial);
		strSerial += strChecksum;
	}
	
	return strSerial;
}

BOOL CSCS_SN_YCHUANGCfg::ValidateSerialNumber(const CString& strSerial)
{
	// TODO: 在此添加序列号验证代码
	// 这里可以实现序列号格式和校验和的验证
	if (strSerial.GetLength() < m_strSerialPrefix.GetLength())
		return FALSE;
	
	if (strSerial.Left(m_strSerialPrefix.GetLength()) != m_strSerialPrefix)
		return FALSE;
	
	return TRUE;
}

BOOL CSCS_SN_YCHUANGCfg::ParseConfigLine(const CString& strLine)
{
	// TODO: 在此添加配置行解析代码
	return TRUE;
}

CString CSCS_SN_YCHUANGCfg::CalculateChecksum(const CString& strData)
{
	// TODO: 在此添加校验和计算代码
	// 这里可以实现简单的校验和算法
	BYTE checksum = 0;
	for (int i = 0; i < strData.GetLength(); i++)
	{
		checksum ^= (BYTE)strData[i];
	}
	
	CString strChecksum;
	strChecksum.Format(_T("%02X"), checksum);
	return strChecksum;
}
