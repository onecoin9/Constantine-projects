#pragma once

#include <QObject>
#include <QMetaType>
#include <QTimer>
#include <string>
#include "GlobalDefine.h"


typedef struct  tagSktInfo {
	uint32_t CurCnt;
	uint32_t LimitCnt;
	uint32_t FailCnt;
	uchar LicenseFlag;
	QString UID;
}tSktInfo;

struct MainBoardInfo
{
	std::string strHardwareUID;
	std::string strHardwareSN;
	std::string strHardwareVersion;
	std::string strHardwareOEM;

	MainBoardInfo() {
		clear();
	}

	void clear() {
		strHardwareUID = "";
		strHardwareSN = "";
		strHardwareVersion = "";
		strHardwareOEM = "";
	}
};

//设备状态：0:Idle 1:Success 2:Failed 3:Abnormal 4:Success but curTsk changed
//使用Q_ENUM宏注册枚举，使其支持Qt元对象系统
//封装在QObject派生类中以便支持Qt的信号槽等特性
class ProEnvStatus : public QObject
{
	Q_OBJECT
public:
	explicit ProEnvStatus(QObject* parent = nullptr) : QObject(parent) {}

	enum Status {
		Idle = 0,                      // 空闲状态
		Success = 1,                   // 执行成功
		Failed = 2,                    // 执行失败
		Abnormal = 3,                  // 异常状态
		SuccessButCurTskChanged = 4    // 成功但当前任务已变更
	};
	Q_ENUM(Status)
};

struct DeviceStu
{
	int nChainID;
	std::string strIP;
	std::string strPort;
	std::string strSiteAlias;
	std::string strMac;
	std::string strFirmwareVersion;
	std::string strFirmwareVersionDate;
	std::string strMUAPPVersion;
	std::string strMUAPPVersionDate;
	std::string strFPGAVersion;
	std::string strFPGALocation;
	std::string strDPSFwVersion;
	std::string strDPSFPGAVersion;
	std::string strMULocation;
	int	nHopNum;
	int nLinkNum;	//判断串联设备的数量
	bool getLastHop;
	bool recvPT;	// 判断是收到PT包之后的设备，不是设备默认一直发送的
	ProEnvStatus::Status ProgEnvReady;			//0:idle 1:success 2:failed 3:abnormal 4:success but curTsk changed
	std::vector<int> vecLinkDev;//选择连接的设备，即在链上的第几个设备
	MainBoardInfo tMainBoardInfo;

	int nSingleBpuSiteNum;
	std::vector<SingleBPU> bpuInfoArr;

	bool bOnLine;
	DeviceStu() {
		clear();
	}
	void clear() {
		nChainID = -1;
		strIP = "";
		strPort = "";
		strSiteAlias = "";
		strMac = "";
		strFirmwareVersion = "";
		strFirmwareVersionDate = "";
		strMUAPPVersion = "";
		strMUAPPVersionDate = "";
		strFPGAVersion = "";
		strFPGALocation = "";
		strDPSFwVersion = "";
		strDPSFPGAVersion = "";
		strMULocation = "";
		nHopNum = 0;
		nLinkNum = -1;
		getLastHop = false;
		recvPT = false;
		ProgEnvReady = ProEnvStatus::Idle;
		tMainBoardInfo.clear();

		nSingleBpuSiteNum = 2;
		bOnLine = true;
	}
};