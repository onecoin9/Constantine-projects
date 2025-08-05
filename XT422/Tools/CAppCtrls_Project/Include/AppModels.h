#ifndef _APPMODELS_H_
#define _APPMODELS_H_

#include "ACError.h"
#include "ACDeviceModel.h"
#include "BufferModel.h"
#include "AppConfigModel.h"
#include "ILog.h"



typedef enum class _eCapacityType {
	CAPACITY_NONE = 0,
	CAPACITY_SSD=1,
	CAPACITY_DDR=2,
	CAPACITY_SKT=3
}eCapacityType;

Q_DECLARE_METATYPE(eCapacityType)

class CAppModels :public QObject
{
	Q_OBJECT
public:
	CAppModels();
	~CAppModels();
	void AttachILog(ILog* pILog) { m_pILog = pILog; }
	tDeviceModelHash* GetDeviceModelList(){
		return &m_DeviceModelHash;
	}

	static QString GetCapacityTypeName(eCapacityType Type);

	int32_t UpdateLinkStatus(quint32 HopNum, quint16 LinkStatus,bool IsLastHop);
	//设置设备信息，DevInfo为8个quint32，从Device端获得，具体还未定义，成功返回0，失败返回负数，GetDeviceInfoRaw的DeviceInfoRaw返回器件信息
	int32_t SetDeviceInfoRaw(quint32 HopNum, quint32 DevInfo[]);
	int32_t GetDeviceInfoRaw(quint32 HopNum, tDeviceInfoRaw& DeviceInfoRaw);

	//设置对应容量，成功返回0，失败返回负数，GetCapacity中的Capacity返回保存的容量
	int32_t SetCapacity(quint32 HopNum,eCapacityType CapacityType, quint64 Capacity);
	int32_t GetCapacity(quint32 HopNum, eCapacityType CapacityType, quint64& Capacity);

	//增加指定HopNum的SeqNum，PreSeqNum返回增加前的SeqNum,成功返回0，失败返回负数，
	int32_t IncreaseSeqNum(quint32 HopNum,quint16& PreSeqNum);
	//增加指定HopNum的PackToal，当bPackLost为true时，表示有丢包，则PackLost增加1
	int32_t IncreasePackTotal(quint32 HopNum, bool bPackLost);
	int32_t WriteDatatoBuffer();
	

	CAppConfigModel m_AppConfigModel;

protected:
	int32_t InsertNewHop(quint32 HopNum);

signals:
	void sigLinkStatusChanged(quint32 HopNum, quint16 LinkStatus, quint32 IsLastHop);
	void sigDeviceInfoRawChanged(quint32 HopNum);
	void sigCapacityReceived(quint32 HopNum, qint32 CapacityType);

private:
	ILog* m_pILog;
	tDeviceModelHash m_DeviceModelHash;
	tBufferModelHash m_BufferModeHash;
	
};

#endif 