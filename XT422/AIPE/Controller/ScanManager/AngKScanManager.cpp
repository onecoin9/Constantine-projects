
#include "AngKScanManager.h"

#include "ACError.h"
#include "ACEventLogger.h"
#include <ws2tcpip.h>
#include <QUdpSocket>
#include <QThread>
#include <QNetworkDatagram>
#include <QNetworkInterface>
#include <QProcess>
#include <QTextCodec>
#include "AngkLogger.h"
Q_DECLARE_METATYPE(QHostAddress);
// **** 静态成员初始化 ****
AngKScanManager* AngKScanManager::s_instance = nullptr;
std::once_flag AngKScanManager::once_flag;
// **** 构造函数修改 ****
AngKScanManager::AngKScanManager(QObject *parent) // parent 通常为 nullptr 用于单例
    : QObject(parent) // 注意：单例通常不设置 parent
    , m_nBordcastPort(0)
    , m_nRecvPort(0)
    , m_nScanNetLength(24) // 提供一个默认值
    , m_ScanLocalSocket(INVALID_SOCKET)
    , m_bStopThead(false) // 初始化 m_bStopThead
	, m_scanCompletionTimer(new QTimer(this))
{
     connect(this, &AngKScanManager::sgnSendLinkScan, this, &AngKScanManager::onSlotSendLinkScan);
	 m_scanCompletionTimer->setSingleShot(true);
    // 线程相关的连接移到 instance() 方法中，确保在对象创建后、移动到线程前完成
    // QObject::connect(&m_scanThread, &QThread::started, this, &AngKScanManager::onSlotThreadScanHandler); // 改为在 onThreadStarted 中启动监听循环
    // QObject::connect(this, &AngKScanManager::sgnWorkFinished, this, &AngKScanManager::ThreadExit, Qt::DirectConnection);
    // QObject::connect(this, &AngKScanManager::sgnWorkFinished, &m_scanThread, &QThread::exit, Qt::DirectConnection);

    // this->moveToThread(&m_scanThread); // 移动到线程的操作也移到 instance() 中
}

AngKScanManager::~AngKScanManager()
{
	if (m_scanCompletionTimer && m_scanCompletionTimer->isActive()) {
        m_scanCompletionTimer->stop(); 
    }
    // 确保线程停止
    CloseSokcet(); // 调用 CloseSocket 来停止线程和清理资源
}

// **** 添加 onThreadStarted ****
void AngKScanManager::onThreadStarted()
{
    ALOG_DEBUG("Scan manager thread started (ID: %p).", "CU", "--", QThread::currentThreadId());
    // 可以在这里启动持续的监听，如果需要的话
    // 例如，如果 onSlotThreadScanHandler 是一个循环，可以在这里触发它
    QMetaObject::invokeMethod(this, "onSlotThreadScanHandler", Qt::QueuedConnection);
}
bool AngKScanManager::SetScanComm()
{
	ushort uLocalPort = AngKGlobalInstance::ReadValue("DeviceComm", "LocalPort").value<ushort>();

	//if (!IsUdpPortAvailable(uLocalPort)) {
	//	emit sgnScanPortError(uLocalPort);
	//	return false;
	//}

	if (m_ScanLocalSocket != INVALID_SOCKET) {
		closesocket(m_ScanLocalSocket);
		//WSACleanup();
		m_ScanLocalSocket = INVALID_SOCKET;
	}

	tSocketOpts* pDestSocketOpts = &optRecord;
	pDestSocketOpts->RecvBufSize = 128 * 1024 * 1024;
	pDestSocketOpts->SendBufSize = 128 * 1024 * 1024;
	pDestSocketOpts->RecvTimeoutms = 5000;
	pDestSocketOpts->SendTimeoutms = 5000;
	pDestSocketOpts->bReuseaddr = true;
	pDestSocketOpts->ZeroCopy = 0;

	int32_t Ret = 0;
	int32_t SktErr = 0;
	//初始化网络环境
	/*WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		ALOG_FATAL("WSAStartup failed.", "CU", "--");
		Ret = ERR_NETCOMM_WSAStartup;
		goto __end;
	}*/

	//建立一个UDP的socket
	m_ScanLocalSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_ScanLocalSocket == SOCKET_ERROR) {
		ALOG_FATAL("Create socket failed.", "CU", "--");
		Ret = ERR_NETCOMM_CreateSocket;
		goto __end;
	}
	//绑定地址信息
	m_ScanLocalAddr.sin_family = AF_INET;
	//ushort uLocalPort = AngKGlobalInstance::ReadValue("DeviceComm", "LocalPort").value<ushort>();
	m_ScanLocalAddr.sin_port = htons(uLocalPort);
	//m_ScanLocalAddr.sin_addr.S_un.S_addr = inet_addr(AngKGlobalInstance::ReadValue("DeviceComm", "LocalIP").toString().toStdString().c_str());
	//ALOG_INFO("m_scanHostSegment = %s", "CU", m_scanHostSegment.toString().toStdString().c_str());
	m_ScanLocalAddr.sin_addr.S_un.S_addr = inet_addr(m_scanHostSegment.toString().toStdString().c_str());

	m_ScanRemoteAddr.sin_family = AF_INET;
	ushort uRemotePort = AngKGlobalInstance::ReadValue("DeviceComm", "RemotePort").value<ushort>();
	m_ScanRemoteAddr.sin_port = htons(uRemotePort);

	SktErr = bind(m_ScanLocalSocket, (struct sockaddr*)&m_ScanLocalAddr, sizeof(struct sockaddr));
	if (SktErr != 0) {
		ALOG_FATAL("Bind Socket Error, IP = %s.", "CU", "--", m_scanHostSegment.toString().toStdString().c_str());
		Ret = ERR_NETCOMM_BindSocket;
		goto __end;
	}
	/*在send()的时候，返回的是实际发送出去的字节(同步)或发送到socket缓冲区的字节
	(异步);系统默认的状态发送和接收一次为8688字节(约为8.5K)；在实际的过程中发送数据
	和接收数据量比较大，可以设置socket缓冲区，而避免了send(),recv()不断的循环收发：*/
	SktErr = setsockopt(m_ScanLocalSocket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*>(&pDestSocketOpts->RecvBufSize), sizeof(int32_t));
	if (SktErr != 0) {
		ALOG_FATAL("SetSockOpt SO_RCVBUF Error, IP = %s.", "CU", "--", m_scanHostSegment.toString().toStdString().c_str());
		Ret = ERR_NETCOMM_SetSktOpts;
		goto __end;
	}
	SktErr = setsockopt(m_ScanLocalSocket, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char*>(&pDestSocketOpts->SendBufSize), sizeof(int32_t));
	if (SktErr != 0) {
		ALOG_FATAL("SetSockOpt SO_SNDBUF Error, IP = %s.", "CU", "--", m_scanHostSegment.toString().toStdString().c_str());
		Ret = ERR_NETCOMM_SetSktOpts;
		goto __end;
	}
	//closesocket（一般不会立即关闭而经历TIME_WAIT的过程）后想继续重用该socket：
	SktErr = setsockopt(m_ScanLocalSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)(&pDestSocketOpts->bReuseaddr), sizeof(bool));
	if (SktErr != 0) {
		ALOG_FATAL("SetSockOpt SO_REUSEADDR Error, IP = %s.", "CU", "--", m_scanHostSegment.toString().toStdString().c_str());
		Ret = ERR_NETCOMM_SetSktOpts;
		goto __end;
	}
	//在send(), recv()过程中有时由于网络状况等原因，发收不能预期进行, 而设置收发时限：
	SktErr = setsockopt(m_ScanLocalSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)(&pDestSocketOpts->RecvTimeoutms), sizeof(int32_t)); //发送时限
	if (SktErr != 0) {
		ALOG_FATAL("SetSockOpt SO_SNDTIMEO Error, IP = %s.", "CU", "--", m_scanHostSegment.toString().toStdString().c_str());
		Ret = ERR_NETCOMM_SetSktOpts;
		goto __end;
	}

	SktErr = setsockopt(m_ScanLocalSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)(&pDestSocketOpts->SendTimeoutms), sizeof(int32_t)); //接收时限
	if (SktErr != 0) {
		ALOG_FATAL("SetSockOpt SO_RCVTIMEO Error, IP = %s.", "CU", "--", m_scanHostSegment.toString().toStdString().c_str());
		Ret = ERR_NETCOMM_SetSktOpts;
		goto __end;
	}

__end:
	if (SktErr) {
		ALOG_FATAL("SocketErrorCode:%d.", "CU", "--", WSAGetLastError());
		return false;
	}
	else {
		ALOG_DEBUG("Establishing scan udp socket connection successfully, IP = %s:%d.", "CU", "--", m_scanHostSegment.toString().toStdString().c_str(), m_ScanLocalAddr.sin_port);
	}

	m_scanThread.start();
	m_bStopThead = false;
	return true;
}

void AngKScanManager::StartScan()
{
	// 构造ARP请求数据包
	QByteArray arpRequest;
	tCmdPacketPT CmdPacketPT, * pCmdPacketPT = NULL;
	tACCmdPack* pACCmdPack = NULL;
	pCmdPacketPT = &CmdPacketPT;
	pACCmdPack = (tACCmdPack*)pCmdPacketPT->Data;

	memset(pCmdPacketPT, 0, sizeof(tCmdPacketPT));

	pCmdPacketPT->HopNum = 0;
	pCmdPacketPT->MsgID = ICDMsgID_PT;
	pCmdPacketPT->PortID = 0;
	pCmdPacketPT->MsgSubID = CMDID_PTPACK_SRC;
	pACCmdPack->BPUID = 8;
	pACCmdPack->SKTEn = 0;
	pACCmdPack->CmdFlag = 0;
	//pACCmdPack->CmdID = 0x434;
	//可读性差，改为枚举
	pACCmdPack->CmdID = static_cast<uint16_t>(eSubCmdID::SubCmd_MU_GetDeviceInfo); 
	//pACCmdPack->CmdDataSize = 0;

	arpRequest.append((char*)&pCmdPacketPT);
	int32_t PTPacketRealSize = 8 + sizeof(tACCmdPack) + pACCmdPack->CmdDataSize;

	QHostAddress maskHost(GetDefaultSubnetMask(m_nScanNetLength));
	QHostAddress subIntnet;
	quint32 segIP = m_scanHostSegment.toIPv4Address();
	quint32 maskIP = maskHost.toIPv4Address();
	subIntnet.setAddress(m_scanHostSegment.toIPv4Address() & maskHost.toIPv4Address());
	if (m_scanHostSegment.isNull() || !m_scanHostSegment.isInSubnet(subIntnet, m_nScanNetLength)) {
		ALOG_WARN("Invalid network segment:%s!", "CU", "--", m_scanHostSegment.toString().toStdString().c_str());
	}
	
	QString gateway;
	QProcess process;
	process.start("route print");
	process.waitForFinished();
	QString output = process.readAllStandardOutput();

	// 解析输出  
	QStringList lines = output.split('\n');
	bool found = false;

	for (const QString& line : lines) {
		// 查找目标 IP 地址所在的行  
		if (line.contains(m_scanHostSegment.toString())) {
			// 找到目标 IP 地址后，获取网关地址  
			QStringList parts = line.split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
			if (parts.size() >= 3) {
				gateway = parts[2]; // 网关地址通常在第三列  
				break;
			}
		}
	}

	ALOG_INFO("Send GetDeviceInfo Request for Detect.", "CU", "MU");
	int usermode = AngKGlobalInstance::ReadValue("LoginPwd", "userMode").toInt();
	for (int i = 1; i < 255; ++i) {
		QHostAddress ip;
		ip.setAddress(subIntnet.toIPv4Address() + i);

		if (subIntnet.toIPv4Address() + i == m_scanHostSegment.toIPv4Address() || ip.toString() == gateway) {
			if (usermode != (int)UserMode::Tester) {
				continue;
			}
		}

		if (AngKDeviceModel::instance().FindConnetDev(ip.toString().toStdString())) {
			continue;
		}
		if (ip.toString() == AngKGlobalInstance::ReadValue("DeviceComm", "LocalIP").toString()) {
			if (usermode != (int)UserMode::Tester) {
				continue;
			}
		}
		
		int32_t Ret = 0;
		int32_t BytesSend = 0;
		if (m_ScanLocalSocket == INVALID_SOCKET) {
			Ret = ERR_NETCOMM_SocketInvalid;
			goto __end;
		}
		m_ScanRemoteAddr.sin_addr.S_un.S_addr = inet_addr(ip.toString().toStdString().c_str());
		BytesSend = sendto(m_ScanLocalSocket, (char*)pCmdPacketPT, sizeof(tCmdPacketPT), 0, reinterpret_cast<sockaddr*>(&m_ScanRemoteAddr), sizeof(sockaddr));
		//ALOG_INFO("Send PT Package to %s ,data size = %d", "CU", ip.toString().toStdString().c_str(), BytesSend);
		if (BytesSend <= 0) {
			ALOG_WARN("Sendto %s:%d failed with error: %d.", "CU", "MU", ip.toString().toStdString().c_str(), m_ScanRemoteAddr.sin_port, WSAGetLastError());
			Ret = ERR_NETCOMM_SendData;
			goto __end;
		}

	}
__end:
	//return Ret;
	return;
}

bool AngKScanManager::ParserDevPTPackage(tCmdPacketPT* ptPackage, DeviceStu& devStu)
{
	tACCmdPack* pACCmdPack = (tACCmdPack*)ptPackage->Data;
	QByteArray DataSerial;
	QDataStream DataStream(&DataSerial, QIODevice::ReadWrite);//将DataStream与ByteArray绑定，设置为读写，为后续序列化作准备
	QByteArray RespDataBytes;
	int32_t ResultCode;
	int32_t RespDataSize;
	DataStream.writeRawData((char*)pACCmdPack->CmdData, pACCmdPack->CmdDataSize); //将需要反序列化的数据写入到DataStream中
	DataStream.device()->seek(0); //将位置定位到最开始，准备从最开始读取
	DataStream.setByteOrder(QDataStream::LittleEndian);//设置为小端方式
	DataStream >> ResultCode;
	DataStream >> RespDataSize;

	if (RespDataSize > 0) {
		char* pRespData = new char[RespDataSize];
		if (pRespData) {
			DataStream.readRawData(pRespData, RespDataSize);
			RespDataBytes.append(pRespData, RespDataSize);
			if (pRespData) {
				delete[] pRespData;
			}
		}
	}

	std::string strInfo = std::string(RespDataBytes.constData(), RespDataSize);
	try {//nlohmann解析失败会报异常需要捕获一下

		if (strInfo.empty())
			return false;

		nlohmann::json DevInfoJson = nlohmann::json::parse(strInfo);

		//return;
		if (DevInfoJson.is_null() || !DevInfoJson.is_object())
			return false;
		//二次开发部分-把扫描到的站点信息发送出去
		emit SendNotification("setSiteReuslt",DevInfoJson);
		//二次开发部分-把扫描到的站点信息发送出去
		devStu.strSiteAlias = DevInfoJson["SiteAlias"].get<std::string>();
		devStu.strMac = DevInfoJson["MacAddr"].get<std::string>();
		devStu.strPort = DevInfoJson["UDPPort"].get<std::string>();
		devStu.strFirmwareVersion = DevInfoJson["FirmwareVersion"].get<std::string>();
		devStu.strFirmwareVersionDate = DevInfoJson["FirmwareVersionDate"].get<std::string>();
		devStu.strMUAPPVersion = DevInfoJson["MUAPPVersion"].get<std::string>();
		devStu.strMUAPPVersionDate = DevInfoJson["MUAPPVersionDate"].get<std::string>();
		devStu.strFPGAVersion = DevInfoJson["FPGAVersion"].get<std::string>();
		devStu.strFPGALocation = DevInfoJson["FPGALocation"].get<std::string>();
		devStu.strDPSFwVersion = DevInfoJson["DPSFwVersion"].get<std::string>();
		devStu.strDPSFPGAVersion = DevInfoJson["DPSFPGAVersion"].get<std::string>();
		if (DevInfoJson.contains("MULocation")) {
			devStu.strMULocation = DevInfoJson["MULocation"].get<std::string>();
		}
		else {
			devStu.strMULocation = "";
		}


		if (DevInfoJson["MainBoardInfo"].is_object()) {
			nlohmann::json boardInfo = DevInfoJson["MainBoardInfo"];
			devStu.tMainBoardInfo.strHardwareUID = boardInfo["UID"].get<std::string>();
			devStu.tMainBoardInfo.strHardwareSN = boardInfo["SN"].get<std::string>();
			devStu.tMainBoardInfo.strHardwareVersion = boardInfo["HWVersion"].get<std::string>();
			devStu.tMainBoardInfo.strHardwareOEM = boardInfo["OEM"].get<std::string>();
		}

	}
	catch (const nlohmann::json::exception& e) {
		ALOG_FATAL("GetDeviceInfo Json parse failed : %s.", "CU", "--", e.what());
		return false;
	}

	return true;
}

QString AngKScanManager::GetDefaultSubnetMask(int networkLength)
{
	if (networkLength < 0 || networkLength > 32) {
		return "Invalid network length.";
	}

	quint32 subnetMaskValue = 0xFFFFFFFF << (32 - networkLength);
	QHostAddress subnetMask(subnetMaskValue);

	return subnetMask.toString();
}

void AngKScanManager::CloseSokcet()
{
	//emit sgnWorkFinished(0);
	m_bStopThead = true;
	if (m_ScanLocalSocket != INVALID_SOCKET) {
		closesocket(m_ScanLocalSocket);
		//WSACleanup();
		m_ScanLocalSocket = INVALID_SOCKET;
	}
	m_scanThread.quit();
	m_scanThread.wait();
}

void AngKScanManager::SetScanNetSegment(QString netIP)
{
	QHostAddress testIP(netIP);
	m_scanHostSegment = testIP;
}

void AngKScanManager::SetSubnetMask(int networkLength)
{
	m_nScanNetLength = networkLength;
}

bool AngKScanManager::IsUdpPortAvailable(quint16 nPort)
{
	QUdpSocket socket;
	if (socket.bind(QHostAddress::Any, nPort)) {
		socket.close();
		return true; // 端口未被占用
	}
	else {
		emit 
		return false; // 端口已被占用
	}
}

void AngKScanManager::onSlotSendLinkScan(QHostAddress hostIP)
{
	tLinkScanPacket LinkScanPacket, *pLinkScanPacket = NULL;
	pLinkScanPacket = &LinkScanPacket;
	memset(&LinkScanPacket, 0, sizeof(tLinkScanPacket));
	LinkScanPacket.MsgID = ICDMsgID_LinkScan;
	LinkScanPacket.HopNum = 0;

	//uint8_t* swData = (uint8_t*)pLinkScanPacket;
	//ALOG_INFO("Send LinkScanPackage to %s", "CU", hostIP.toString().toStdString().c_str());
	m_ScanRemoteAddr.sin_addr.S_un.S_addr = inet_addr(hostIP.toString().toStdString().c_str());
	int BytesSend = sendto(m_ScanLocalSocket, (char*)pLinkScanPacket, sizeof(tLinkScanPacket), 0, reinterpret_cast<sockaddr*>(&m_ScanRemoteAddr), sizeof(sockaddr));

	ALOG_INFO("Send LinkScan to %s:%d.", "CU", "MU", hostIP.toString().toStdString().c_str(), LinkScanPacket.HopNum);
}

void AngKScanManager::onSlotThreadScanHandler()
{
	int32_t Ret = 0;
	int32_t CurIdx = 0;
	int32_t TimeRuns = 2;
	int32_t nSize = sizeof(sockaddr_in);
	uint8_t* TmpDataBuf = (uint8_t*)SysMalloc(MaxNetDataSize);
	if (TmpDataBuf == NULL) {
		Ret = ERR_MemoryAlloc;
		goto __end;
	}
	while (!m_bStopThead) {
		if(m_ScanLocalSocket == INVALID_SOCKET)
			continue;

		SOCKADDR clntAddr;  //客户端地址信息
		sockaddr_in addrclnt;
		memset(TmpDataBuf, 0, MaxNetDataSize);

		//UDP协议不需要处理粘包问题，整包不会给你分割
		int32_t BytesRead = recvfrom(m_ScanLocalSocket, (char*)TmpDataBuf, MaxNetDataSize, 0, (SOCKADDR*)&addrclnt, &nSize);
		if (BytesRead >= 0) {
			char clientIP[256];
			// **** 初始化 clientIP 缓冲区 ****
            memset(clientIP, 0, sizeof(clientIP));

            // **** 调用 inet_ntop 并检查返回值 ****
            const char* pClientIP = inet_ntop(AF_INET, &addrclnt.sin_addr, clientIP, sizeof(clientIP)); // 使用 sizeof(clientIP) 更安全

			if (pClientIP != NULL) { // **** 检查 inet_ntop 是否成功 ****
                // 成功，clientIP 包含有效的 IP 地址字符串
                std::string recvDevIP = clientIP; // 或者直接使用 pClientIP: std::string recvDevIP = pClientIP;
                //ALOG_INFO("Recv Package from %s ,data size = %d", "CU", "--", recvDevIP.c_str(), BytesRead); // 将 "to" 改为 "from" 可能更符合语义
			uint8_t* swData = (uint8_t*)TmpDataBuf;
			uint8_t MsgID = swData[0];
			switch (MsgID)
			{
			case ICDMsgID_Heartbeat:
			{
				tHeartbeatPacket* pHeartbeatPck = (tHeartbeatPacket*)(swData);
				std::string recvHeartIPHop = recvDevIP + ":" + QString::number(pHeartbeatPck->HopNum).toStdString();
				std::map<std::string, DeviceStu> insertDev;
				AngKDeviceModel::instance().GetScanDevMap(insertDev);

				if (insertDev[recvHeartIPHop].recvPT) {
					if (AngKDeviceModel::instance().FindScanDev(recvHeartIPHop))
					{
						insertDev[recvHeartIPHop].getLastHop = pHeartbeatPck->LastHopFlag == 1 ? true : false;
						insertDev[recvHeartIPHop].nHopNum = pHeartbeatPck->HopNum;
						AngKDeviceModel::instance().SetScanDevInfo(recvHeartIPHop, insertDev[recvHeartIPHop]);
						emit sgnUpdateDevIPScan(QString::fromStdString(recvHeartIPHop));

					}
					else {//没找到的一定是某条链设备上的
						if (pHeartbeatPck->HopNum != 0) {
							AngKDeviceModel::instance().AddScanDevChain(recvHeartIPHop);
						}
					}
				}
			}
			break;
			case ICDMsgID_PT:
			{
				tCmdPacketPT* pPacketPT = (tCmdPacketPT*)(swData);
				tACCmdPack* pACCmdPack = (tACCmdPack*)pPacketPT->Data;
				std::string cmdName = Utils::AngKCommonTools::TranslateMessageCmdID(pACCmdPack->CmdID);
				if (CMDID_PTPACK_ACK == pPacketPT->MsgSubID) {
					std::string recvAckIPHop = recvDevIP + ":" + QString::number(pPacketPT->HopNum).toStdString();
					ALOG_INFO("Recv %s ACK from %s.", "MU", "CU", cmdName.c_str(), recvAckIPHop.c_str());
					AngKDeviceModel::instance().AddScanDevIP(recvAckIPHop);
					
				}
				else if (CMDID_PTPACK_COMPLETE == pPacketPT->MsgSubID) {
					QByteArray DataSerial;
					QDataStream DataStream(&DataSerial, QIODevice::ReadWrite);//将DataStream与ByteArray绑定，设置为读写，为后续序列化作准备
					QByteArray RespDataBytes;
					int32_t ResultCode;
					DataStream.writeRawData((char*)pACCmdPack->CmdData, pACCmdPack->CmdDataSize);
					DataStream.device()->seek(0); //将位置定位到最开始，准备从最开始读取
					DataStream.setByteOrder(QDataStream::LittleEndian);//设置为小端方式
					DataStream >> ResultCode;
					std::string recvPtIPHop = recvDevIP + ":" + QString::number(pPacketPT->HopNum).toStdString();
					ALOG_INFO("Recv %s Complete(ResultCode:%d) from %s.", "MU", "CU", cmdName.c_str(), ResultCode, recvPtIPHop.c_str());
					DeviceStu devStu;
					devStu.strIP = recvDevIP;
					devStu.nHopNum = pPacketPT->HopNum;
					devStu.recvPT = true;

					if (ParserDevPTPackage(pPacketPT, devStu)) {
						AngKDeviceModel::instance().SetScanDevInfo(recvPtIPHop, devStu);
						emit sgnSendLinkScan(QHostAddress(QString::fromStdString(recvDevIP)));
						//此刻devstu已经有一条来自设备的内容
					}
					else {
						emit sgnScanDeviceError(QString::fromStdString(recvDevIP), pPacketPT->HopNum);
					}
				}
				else if (CMDID_PTPACK_QUERYDOCMD == pPacketPT->MsgSubID) {
					tACCmdPack* pACCmdPack = (tACCmdPack*)pPacketPT->Data;
					std::string cmdName = Utils::AngKCommonTools::TranslateMessageCmdID(pACCmdPack->CmdID);
					if (pACCmdPack->CmdID == (uint16_t)eSubCmdID::SubCmd_MU_SetEvent) {
						QByteArray CmdDataBytes;
						CmdDataBytes.append((char*)pACCmdPack->CmdData, pACCmdPack->CmdDataSize);
						QDataStream DataStream(&CmdDataBytes, QIODevice::ReadOnly);
						DataStream.setByteOrder(QDataStream::LittleEndian);//设置为小端方式
						uint16_t EventMsgSize;
						char EventMsg[1024];

						memset(EventMsg, 0, 1024);
						DataStream >> EventMsgSize;
						DataStream.readRawData(EventMsg, EventMsgSize);
						ALOG_INFO("Recv %s Message from %s:%d", "MU", "CU", cmdName.c_str(), recvDevIP.c_str(), pPacketPT->HopNum);
						std::string eventInfo = EventMsg;
						EventLogger->SendEvent(eventInfo);
					}
				}
			}
			break;
			default:
				break;
			}
		}
		else {
			 // **** inet_ntop 失败 ****
			 int error = WSAGetLastError(); // 获取 Winsock 错误码
			 ALOG_ERROR("inet_ntop failed to convert address. Error: %d. Received %d bytes.", "CU", "--", error, BytesRead);
			 // 这里不应该继续使用 clientIP 或 recvDevIP
		}
	}
	}

__end:
	SysSafeFree(TmpDataBuf);

	//emit sgnWorkFinished(Ret);
}

void AngKScanManager::ThreadExit(int RetCode)
{
	CloseSokcet();
}

void AngKScanManager::onSlotScanNetSegment(const QString& netSeg)
{
	m_scanHostSegment = netSeg;
}
void AngKScanManager::SiteScanAndConnect() {

	auto& model = AngKDeviceModel::instance();
	// --- 清除旧的扫描结果 ---
	model.ClearScanDev();
	// 可选：通知 UI 清空显示 (如果 UI 连接了 Model 的信号)
	// emit model.scanCleared(); // 假设 Model 有这样的信号
	// --- 确定扫描参数 (使用全局配置) ---
	model.autoAddToConnect = true; // 设置自动添加设备标志
	QString localIP = AngKGlobalInstance::ReadValue("DeviceComm", "LocalIP").toString();
	if (localIP.isEmpty()) {
		// 尝试查找一个合适的本地 IP
		QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
		for (const QNetworkInterface& iface : interfaces) {
			if (!(iface.flags() & QNetworkInterface::IsLoopBack) && (iface.flags() & QNetworkInterface::IsRunning)) {
				for (const QNetworkAddressEntry& entry : iface.addressEntries()) {
					if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
						localIP = entry.ip().toString();
						ALOG_WARN("LocalIP not configured, using found IP: %s", "CU", "--", localIP.toStdString().c_str());
						// 可选：同时设置子网掩码长度
						// scanner.SetSubnetMask(entry.prefixLength());
						goto ip_found; // 找到后跳出
					}
				}
			}
		}
	ip_found:; // 跳转标签
		if (localIP.isEmpty()) {
			ALOG_ERROR("Failed to determine local IP for scanning.", "CU", "--");
			return;
		}
	}
	// 设置扫描网段 (通常是本地 IP) 和子网掩码长度 (假设为 24)
	SetScanNetSegment(localIP);
	// scanner.SetSubnetMask(24); // 或者从配置读取，或者根据 IP 地址自动计算

	// --- 初始化扫描器 Socket ---
	if (!SetScanComm()) {
		ALOG_ERROR("Failed to initialize scan communication socket.", "CU", "--");
		return;
	}

	// --- 启动扫描 ---
	StartScan(); // 这个函数会向网络发送扫描包
	 // --- 启动定时器以在扫描后添加设备 ---
	
	 ALOG_INFO("Site Scan and Connect process finalized.", "CU", "--");
	// emit sgnScanAndConnectComplete();
	 //blockSignals(wasBlocked); // 恢复信号阻塞状态
	// onScanCompletionTimeout();
}
// **** 新增的槽函数，用于处理扫描完成后的操作 ****
//void AngKScanManager::onScanCompletionTimeout()
//{
//    ALOG_INFO("Scan time elapsed. Transferring found devices to connected list.", "CU", "--");
//    auto& model = AngKDeviceModel::instance();
//    std::map<std::string, DeviceStu> scannedDevices;
//
//    // 1. 获取扫描到的设备列表
//    model.GetScanDevMap(scannedDevices);
//
//    if (scannedDevices.empty()) {
//        ALOG_WARN("No devices found during scan.", "CU", "--");
//    } else {
//        ALOG_INFO("Found %d devices during scan. Adding to connected list.", "CU", "--", scannedDevices.size());
//        // 2. 遍历扫描到的设备并添加到连接列表
//        for (const auto& pair : scannedDevices) {
//            const std::string& ipHop = pair.first;
//            const DeviceStu& devInfo = pair.second;
//
//            // 检查设备是否已经因为某些原因被添加（可选，取决于 Add/Set 的实现）
//            if (!model.FindConnetDevByIPHop(ipHop)) {
//                 model.AddConnetDevIP(ipHop); // 添加 IP:Hop 到连接列表
//            }
//            // 设置/更新连接设备的信息，并检查结果
//            bool setResult = model.SetConnetDevInfo(ipHop, devInfo);
//            if (!setResult) {
//                 // SetConnetDevInfo 失败
//                 ALOG_WARN("model.SetConnetDevInfo for device %s FAILED.", "CU", "--", ipHop.c_str());
//                 // 可以选择在这里记录 devInfo 的内容以帮助调试
//                 ALOG_WARN(" -> Device Info attempted: IP=%s, Hop=%d, MAC=%s, Alias=%s, FW=%s", "CU", "--",
//                           devInfo.strIP.c_str(), devInfo.nHopNum, devInfo.strMac.c_str(),
//                           devInfo.strSiteAlias.c_str(), devInfo.strFirmwareVersion.c_str());
//            } else {
//                 // SetConnetDevInfo 成功
//                 ALOG_INFO("model.SetConnetDevInfo for device %s SUCCEEDED.", "CU", "--", ipHop.c_str());
//                 // 记录一些关键信息以确认内容是否符合预期
//                 ALOG_INFO(" -> Device Info set: IP=%s, Hop=%d, MAC=%s, Alias=%s, FW=%s, recvPT=%s", "CU", "--",
//                           devInfo.strIP.c_str(), devInfo.nHopNum, devInfo.strMac.c_str(),
//                           devInfo.strSiteAlias.c_str(), devInfo.strFirmwareVersion.c_str(),
//                           devInfo.recvPT ? "true" : "false");
//                 // 可以根据需要添加更多 devInfo 的字段
//            }
//        }
//    }
//    // 3. 发射完成信号 (可选)
//    ALOG_INFO("Site Scan and Connect process finalized.", "CU", "--");
//    emit sgnScanAndConnectComplete();
//}
