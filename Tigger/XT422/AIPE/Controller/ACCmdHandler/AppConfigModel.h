#ifndef _APPSETTING_H_
#define _APPSETTING_H_

#include "ACTypes.h"
#include "ACError.h"
#include <QSettings>
#include <QObject>

class CAppConfigModel : public QObject
{
	Q_OBJECT
public:
	int32_t LoadConfig(QString strConfig);
	int32_t SaveConfig(QString strConfig="");
	DefineProperty(QString, RemoteIP)				//AG06的IP
	DefineProperty(quint16, RemotePort)				//AG06的端口
	DefineProperty(QString, LocalIP)				//本地端的IP
	DefineProperty(quint16, LocalPort)				//本地端的端口
	DefineProperty(qint32, NetCommCmdAckTimeoutms)	//通信层命令ACKTimeout设定,单位为ms，-1表示永远等待
	DefineProperty(qint32, CmdQueueAvailableTimeoutms) //等待命令队列能够使用的超时时间，单位为ms，-1表示永远等待
	DefineProperty(qint32, SoftCRC32En)					//数据发送和接收时是否进行软CRC的计算,1表示需要，0表示不需要
	DefineProperty(qint32, LogLevel)					//日志输出等级，只有大于等于这个等级的日志才会被输出
	DefineProperty(QString, DataFilePath)			//数据文件的路径，和json中的文件配合使用
public:
	void resetProperties()
	{
		m_RemoteIP = "192.168.11.1";
		m_RemotePort = 8080;
		m_LocalIP = "192.168.11.2";
		m_LocalPort = 8081;
		m_NetCommCmdAckTimeoutms = -1;
		m_CmdQueueAvailableTimeoutms = -1;
		m_SoftCRC32En = true;
		m_LogLevel = 1;
		m_DataFilePath = "";
	};

private:
	QString m_strConfigPath;
};

#endif 