#ifndef _CLIVIEW_H_
#define _CLIVIEW_H_

#include <QObject>
#include "ILog.h"
#include "APPModels.h"

class CCLIView :public QObject
{
    Q_OBJECT
public:
    void AttachILog(ILog* pILog);
    void AttachAppModel(CAppModels* pAppModels);

public slots:
    //信号和槽定义采用的变量类型需要以Qt认准的，否则跨线程发射信号会产生错误 Make sure 'uint16_t' is registered using qRegisterMetaType().
    void OnDeviceModelLinkStatusChanged(quint32 HopNum, quint16 LinkStatus, quint32 isLastHop);
    void OnDeviceInfoRawChanged(quint32 HopNum);
    void OnCapacityReceived(quint32 HopNum, qint32 CapacityType);
private:
    ILog* m_pILog;
    CAppModels* m_pAppModels;
};

#endif