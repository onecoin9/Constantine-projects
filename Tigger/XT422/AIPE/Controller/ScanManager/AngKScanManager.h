#ifndef ANKSCANMANAGER_H
#define ANKSCANMANAGER_H
#include <QObject>
#include <QHostAddress>
#include <QThread>
#include "AngKDeviceModel.h"
#include "ACCmdPacket.h"
#include "ICD.h"
#include <mutex> 
#include <stdio.h>
#include <winsock2.h>
#include <Windows.h>
#include <QTimer>
#include "AngKGlobalInstance.h"
#pragma comment(lib,"ws2_32.lib")
Q_DECLARE_METATYPE(DeviceStu);
class QThread;
class QUdpSocket;
class AngKScanManager : public QObject
{
	Q_OBJECT

public:
	// AngKScanManager(QObject *parent = nullptr);
	// ~AngKScanManager();
	 // **** 添加单例访问方法 ****
	 static AngKScanManager& instance()
	 {
		 // 使用 std::call_once 保证线程安全的初始化
		 std::call_once(once_flag, []() {
			 s_instance = new AngKScanManager();
			 // 将实例移动到其工作线程，确保槽函数在正确线程执行
			 s_instance->moveToThread(&s_instance->m_scanThread);
			 // 确保线程启动后对象能接收信号/调用槽

			  QObject::connect(&s_instance->m_scanThread, &QThread::started, s_instance, &AngKScanManager::onThreadStarted, Qt::DirectConnection);
			  QObject::connect(s_instance, &AngKScanManager::sgnInternalWorkFinished, s_instance, &AngKScanManager::ThreadExit, Qt::DirectConnection);
			  QObject::connect(s_instance, &AngKScanManager::sgnInternalWorkFinished, &s_instance->m_scanThread, &QThread::quit, Qt::DirectConnection); // 使用 quit 替代 exit
			  qRegisterMetaType<DeviceStu>();
			});
		 return *s_instance;
	 }
 
	 // **** 删除拷贝构造和赋值 ****
	 AngKScanManager(const AngKScanManager&) = delete;
	 AngKScanManager& operator=(const AngKScanManager&) = delete;

	bool SetScanComm(int nMask);

	bool SetScanComm();

	void StartScan();

	bool ParserDevPTPackage(tCmdPacketPT* ptPackage, DeviceStu& devStu);

	QString GetDefaultSubnetMask(int networkLength);

	void CloseSokcet();

	void SetScanNetSegment(QString netIP);

	void SetSubnetMask(int networkLength);

	bool IsUdpPortAvailable(quint16 nPort);

	// **** 新增的公共接口 ****
    Q_INVOKABLE void SiteScanAndConnect(); // 使其可从其他元对象系统调用（如果需要）
signals:
	//void sgnWorkFinished(int);
	void sgnInternalWorkFinished(int);//改为内部信号，用于线程退出
	void sgnSendLinkScan(QHostAddress);
	void SendNotification(const QString& method, const nlohmann::json& result);//发送给jsonrpc客户端的信号
	void sgnUpdateDevIPScan(QString);// View 可以连接这个信号获取更新
	void sgnScanDeviceError(QString, int);
	void sgnScanPortError(int);
	//void SiteScanAndConnect();
	// **** 新增信号，用于发送找到的设备信息 ****
	void sgnDeviceFoundInfo(QString ipHop, DeviceStu deviceInfo);
	void sgnScanAndConnectComplete();

public slots:
	void ThreadExit(int);
	void onSlotScanNetSegment(const QString&);
	void onSlotSendLinkScan(QHostAddress);
	void onSlotThreadScanHandler();
	void onThreadStarted(); // 添加一个槽，在线程启动时调用
	//void onScanCompletionTimeout();
	// **** SiteScanAndConnect 作为槽函数 ****
    // void SiteScanAndConnect(); // 也可以放在这里，如果主要由信号触发
private:
 // **** 构造/析构函数设为私有 ****
	AngKScanManager(QObject *parent = nullptr);
	~AngKScanManager();

	// **** 静态成员 ****
	static AngKScanManager* s_instance;
	static std::once_flag once_flag;

	QThread		m_scanThread;
	QHostAddress m_scanHostSegment;
	int			m_nBordcastPort;
	int			m_nRecvPort;
	int			m_nScanNetLength;
	SOCKET m_ScanLocalSocket;
	SOCKADDR_IN m_ScanLocalAddr;
	SOCKADDR_IN m_ScanRemoteAddr;
	volatile bool m_bStopThead;
	tSocketOpts	optRecord;
	QTimer*     m_scanCompletionTimer; 
};
#endif // !ANKSCANMANAGER_H