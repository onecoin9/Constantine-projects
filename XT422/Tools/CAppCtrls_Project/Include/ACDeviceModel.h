#ifndef _ACDEVICE_H_
#define _ACDEVICE_H_

#include "ACTypes.h"
#include <QObject>
#include <QHash>
#include <QVariant>
#include <QQueue>

/// <summary>
/// 从设备信息响应包中得到的原始的设备信息
/// </summary>
#pragma pack(push, 1)

typedef struct _tDevInfoRaw {
	/// <summary>
	/// 翻译之后得到 XXX_YYYYMMDDSSSS,其中XXX为DevType对应的机型名称，YYYY为年占4个字符，MM为月占2个字符，DD为天占2个字符，SSSS为序列号占4个字符
	/// </summary>
	quint32 Serial : 10; //序列号占10个bit，可以表示0-1023
	quint32 Day : 5;   ///天占5个bit，可以表示0-31
	quint32 Month : 4; ///月占4个bit，可以表示0-15
	quint32 Year : 7;  ///年占7个bit，从2020年开始，可以表示2020+0---2020+127
	quint32 DevType : 6; ///机型占6个Bit可以表示0-63共64种机型
	quint16 SFVersion; ///固件版本号 100为1.00
	quint16 HWVersion; ///硬件版本号 100为1.00
	quint32 Reserved[6];
}tDeviceInfoRaw;

typedef struct _tDeviceEvent {
	uint8_t EventData[64];
}tDeviceEvent;

#pragma pack(pop)



//属性的访问方法
//DeviceModel.setHopNum(100);
//DeviceModel.setDevName("112233");
//Log.PrintLog(0, "HopNum:%d ,DevName:%s\r\n", DeviceModel.HopNum(), DeviceModel.DevName().toStdString().c_str());
//
//DeviceModel.setHopNum(123); 直接设置
//Log.PrintLog(0, "HopNum:%d \r\n", DeviceModel.property("HopNum").toInt());
//
//pObj->setProperty("HopNum", 1000); //通过属性访问
//Log.PrintLog(0, "HopNum1:%d \r\n", DeviceModel.HopNum());


class CACDeviceModel : public QObject
{
	Q_OBJECT
	DefineProperty(QString, DevName)
	DefineProperty(uint32_t, HopNum)
	DefineProperty(uint32_t, isValid)
	DefineProperty(uint32_t, isLastHop)
	DefineProperty(uint32_t, LinkStatus)
	DefineProperty(uint64_t, SSDCapacity)
	DefineProperty(uint64_t, DDRCapacity)
	DefineProperty(uint64_t, SKTCapacity)
	//包序号，通信时用来检查丢包使用
	DefineProperty(uint32_t, SeqNum)

	DefineProperty(uint64_t, PacketLost)
	DefineProperty(uint64_t, PacketTotal)

public:
	CACDeviceModel() = default;
	void setDevInfoRaw(tDeviceInfoRaw* pDevInfoRaw);
	tDeviceInfoRaw getDevInfoRaw();

	//保存和获取Device事件，成功返回0，失败返回负数
	int32_t enqueueDeviceEvent(tDeviceEvent);
	int32_t dequeueDeviceEvent(tDeviceEvent& DeviceEvent);
public:
	void resetProperties()
	{
		m_DevName = "";
		m_HopNum = 0;
		m_isValid = 0;
		m_LinkStatus = 0;
		m_SSDCapacity = 0;
		m_DDRCapacity = 0;
		m_SKTCapacity = 0;
		m_SeqNum = 0;
		m_PacketLost = 0;
		m_PacketTotal = 0;
		m_isLastHop = 0;
	};

private:
	tDeviceInfoRaw m_DevInfoRaw;
	QQueue<tDeviceEvent> m_DeviceEnvents;
};

typedef QHash<uint32_t, CACDeviceModel*> tDeviceModelHash;
typedef QHash<uint32_t, CACDeviceModel*>::iterator tDeviceModelHashItr;
#endif 