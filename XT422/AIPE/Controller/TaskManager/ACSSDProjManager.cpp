#include "ACSSDProjManager.h"
#include "ACDeviceManager.h"
#include "AngKGlobalInstance.h"
#include "AngKMessageHandler.h"
#include "ACEventLogger.h"

extern int calc_crc16sum(unsigned char* buf, unsigned int size, unsigned short* pCRC16Sum);

extern int EMMC_COMMON_BLOCK_SIZE;
extern int EMMC_SSD_MULTIPLE;

static bool CommandReadDataFromSSD(std::string devIP, QString Type, uint32_t HopNum, uint32_t PortID, uint64_t SSDAddr, int32_t BytesToRead, QByteArray& DataBytes, std::string strProgSN, std::string strDataType) {
	
	int nTryReSend = AngKGlobalInstance::instance()->ReadValue("DeviceComm", "Retransmission").toInt();
	//nTryReSend = 3
	while (nTryReSend != 0)
	{
		int nRet = AngKMessageHandler::instance().Command_ReadDataFromSSD(devIP, Type, HopNum, PortID, SSDAddr, BytesToRead, DataBytes, strProgSN, strDataType);
		QThread::msleep(10);// 提高UDP的稳定性，每发送一次命令包，停10ms
		if (nRet == 0) {
			return true;
		}
		else { // 返回错误需要加重传机制
			nTryReSend--;
			ALOG_FATAL("Dev %s:%d read SSD data failed(errorCode: %d), try to retrans, %d remaining times", "CU", "FP", devIP.c_str(), HopNum, nRet, nTryReSend);
		}
	}
	
	return false;
}
static bool CommandWriteDataToSSD(std::string devIP, QString Type, uint32_t HopNum, uint32_t PortID, uint64_t SSDAddr, QByteArray& DataBytes, std::string strProgSN, std::string strDataType) {
	
	DeviceStu devInfo = ACDeviceManager::instance().getDevInfo(devIP.c_str(), HopNum);
	int nTryReSend = AngKGlobalInstance::instance()->ReadValue("DeviceComm", "Retransmission").toInt();

	while (nTryReSend != 0)
	{
		int nRet = AngKMessageHandler::instance().Command_StoreDataToSSDorDDR(devIP, Type, HopNum, 0, SSDAddr, DataBytes, devInfo.tMainBoardInfo.strHardwareSN, "eMMCFileData");
		QThread::msleep(10);// 提高UDP的稳定性，每发送一次命令包，停10ms
		if (nRet == 0) {
			return true;
		}
		else { // 返回错误需要加重传机制
			nTryReSend--;
			ALOG_FATAL("Send eMMC data to %s:%d SSD failed(errorCode: %d), try to retrans, %d remaining times", "CU", "FP", devIP.c_str(), HopNum, nRet, nTryReSend);
		}
	}
	
	return false;
}

static void CalBinFilesByteSum(const std::vector<tSeptBineMMCFileMap>& vecBinFile, nlohmann::json& _bytesumJson)
{
	QElapsedTimer totalTimer; // 整个函数的计时器
	totalTimer.start();
	std::string curParttion = "";
	std::map<std::string, uint64_t> partByteSumMap;//用于统计整体bytesum
	for (int i = 0; i < vecBinFile.size(); ++i)
	{
		tSeptBineMMCFileMap tBinFile = vecBinFile[i];
		if (tBinFile.PartName == "acBinXml")
			continue;
		nlohmann::json calByteSum;
		calByteSum["Partition"] = tBinFile.PartName;
		calByteSum["Sum"] = QString("0x%1").arg(tBinFile.CheckSum, 8, 16, QLatin1Char('0')).toStdString();
		_bytesumJson.push_back(calByteSum);
	}
	return;

	for (int i = 0; i < vecBinFile.size(); ++i)
	{
		uint64_t nBytesum = 0;
		tSeptBineMMCFileMap tBinFile = vecBinFile[i];
		QFile file(QString::fromStdString(tBinFile.strFileName));
		if (!file.open(QIODevice::ReadOnly)) {
			continue;
		}
		curParttion = tBinFile.PartName;
		uint64_t remainderNum = tBinFile.FileSize % EMMC_COMMON_BLOCK_SIZE;

		for (int j = 0; j < tBinFile.vFiles.size(); ++j) {
			teMMCVFile tVFile = tBinFile.vFiles[j];
			uint64_t vfileSize = (uint64_t)tVFile.BlockNum * EMMC_COMMON_BLOCK_SIZE;

			//边界判断
			if (j == 0 && tBinFile.vFiles.size() == 1) {
				if (vfileSize > file.size()) {
					vfileSize = file.size();
				}
			}
			else if (j == tBinFile.vFiles.size() - 1 && remainderNum != 0) {
				if (tVFile.FileBlkPos + vfileSize > file.size()) {
					vfileSize = ((uint64_t)tVFile.BlockNum - 1) * EMMC_COMMON_BLOCK_SIZE + remainderNum;
				}
			}

			uchar* fileBytesumData = file.map(tVFile.FileBlkPos, vfileSize); // 将整个文件映射到内存
			if (fileBytesumData == nullptr)
			{
				ALOG_ERROR("Calculate BinFiles ByteSum error. current binFile Name : %s", "CU", "--", tBinFile.strFileName.c_str());
				break;
			}

			nBytesum += Utils::AngKCommonTools::GetByteSum(fileBytesumData, vfileSize);
			file.unmap(fileBytesumData);
		}

		if (partByteSumMap.find(curParttion) == partByteSumMap.end()) {
			partByteSumMap[curParttion] = nBytesum;
		}
		else {
			uint64_t tempByteSum = partByteSumMap[curParttion] + nBytesum;
			partByteSumMap[curParttion] = tempByteSum;
		}

		file.close();
	}

	for (auto& bytemap : partByteSumMap) {
		nlohmann::json calByteSum;
		calByteSum["Partition"] = bytemap.first;
		calByteSum["Sum"] = QString("0x%1").arg(bytemap.second, 8, 16, QLatin1Char('0')).toStdString();
		_bytesumJson.push_back(calByteSum);
	}

}


// 智能分析出的offset值和SSD中位置不对应，计算CRC时需要统一
static QString exchangeOffsetJsonString(const QString& jsonString) {
	QString tmpStr = jsonString;
	QRegularExpression reDataOffset("\"DataOffsetInSSD\"\\s*:\\s*\"[^\"]*\"");
	tmpStr.replace(reDataOffset, "\"DataOffsetInSSD\": \"0\"");

	QRegularExpression reSSDOffset("\"SSDOffset\"\\s*:\\s*\"[^\"]*\"");
	tmpStr.replace(reSSDOffset, "\"SSDOffset\": \"0\"");
	return tmpStr;
}


QString formatFileSize(qint64 size)
{
	const QStringList units = { "B", "K", "M", "G" };
	int unitIndex = 0;
	double displaySize = size;

	// Keep dividing by 1024 until we get to an appropriate unit or reach the largest unit
	while (displaySize >= 1024.0 && unitIndex < units.size() - 1) {
		displaySize /= 1024.0;
		unitIndex++;
	}

	// Format with at most 1 decimal place and pad with spaces
	QString result;
	if (displaySize < 10.0) {
		result = QString("   %1").arg(displaySize, 0, 'f', 1);
	}
	else if (displaySize < 100.0) {
		result = QString("  %1").arg(displaySize, 0, 'f', 1);
	}
	else if (displaySize < 1000.0) {
		result = QString(" %1").arg(displaySize, 0, 'f', 1);
	}
	else {
		result = QString("%1").arg(qRound(displaySize));
	}

	// Append the unit
	result += units[unitIndex];

	return result;
}


QString truncateFileNameWithExtension(const QString& filePath) {
	QString name = filePath.mid(0, filePath.lastIndexOf("."));
	QString suffix = filePath.mid(filePath.lastIndexOf("."));

	if (filePath.length() > 32) {
		name = name.left(name.length() - (filePath.length() - 32) - 3);
		name += "...";
	}

	QString truncatedName = name + suffix;
	truncatedName = truncatedName.leftJustified(32, ' ');

	return truncatedName;
}

struct Interval {
	uint64_t startAddr;
	uint64_t endAddr;
	int idx;
};


ACSSDProjManager::ACSSDProjManager(const QString& ipStr, int nHop) {
	mDevIp = ipStr;
	mHop = nHop;
}

ACSSDProjManager::~ACSSDProjManager() {}


bool ACSSDProjManager::cacheTableHeadList() {
	ALOG_INFO("ACSSDProjManager::cacheTableHeadList start", "CU", "--");
	mCacheTableHeadMap.clear();
	mCacheTableJsonMap.clear();

	DeviceStu devInfo = ACDeviceManager::instance().getDevInfo(mDevIp, mHop);

	int i = 0;
	for (; i < MAX_EMMC_PARTION_NUM; i++) {
		QByteArray tmpReadTableHead;
		bool ret = CommandReadDataFromSSD(mDevIp.toStdString(), "SSD2FIBER", mHop, 0, ADDR_EMMC_PARTION_HEADER_LIST + i * EMMC_PARTION_HEADER_SIZE, EMMC_PARTION_HEADER_SIZE, tmpReadTableHead, devInfo.tMainBoardInfo.strHardwareSN, "eMMCHeaderData");
		if (!ret)
		{
			mCacheTableHeadMap.clear();
			mCacheTableJsonMap.clear();
			return false;
		}

		eMMCTableHeader* tableHeadPtr = (eMMCTableHeader*)tmpReadTableHead.data();
		uint16_t tmpcrc16 = 0;
		calc_crc16sum((unsigned char*)(tableHeadPtr)+2, sizeof(eMMCTableHeader) - 2, &tmpcrc16);
		// 表头CRC有误，认为分区无效，为空闲分区
		if (tmpcrc16 != tableHeadPtr->HeaderCRC || tmpcrc16 == 0) {
			continue;
		}

		mCacheTableHeadMap.insert(i, *(eMMCTableHeader*)tmpReadTableHead.data());
		mCacheTableJsonMap.insert(i, tmpReadTableHead.data() + sizeof(eMMCTableHeader));
	}
	ALOG_INFO("ACSSDProjManager::cacheTableHeadList end", "CU", "--");
	return true;
}
//
//bool ACSSDProjManager::isProjExist(ACProjManager* projManagerPtr) {
//	QString loaclTableHeadJsonStr = projManagerPtr->GetProjTableHeaderJson();
//	loaclTableHeadJsonStr.remove(QChar(0));
//
//
//	int i = 0;
//	for (; i < MAX_EMMC_PARTION_NUM; i++) {
//		QByteArray tmpReadTableHead;
//		bool ret = CommandReadDataFromSSD(mDevIp.toStdString(), "SSD2FIBER", mHop, 0, ADDR_EMMC_PARTION_HEADER_LIST + i * EMMC_PARTION_HEADER_SIZE, EMMC_PARTION_HEADER_SIZE, tmpReadTableHead, devInfo.tMainBoardInfo.strHardwareSN, "eMMCHeaderData");
//		if (!ret)
//		{
//			return false;
//		}
//		
//		QString readTableHeadJsonStr = tmpReadTableHead.mid(sizeof(eMMCTableHeader));
//		readTableHeadJsonStr.remove(QChar(0));
//
//		if (readTableHeadJsonStr == loaclTableHeadJsonStr) {
//			return true;
//		}
//	}
//	
//	return false;
//}

uint64_t ACSSDProjManager::getSSDMaxIdelSize(bool bNeedUpdateTableHeadList) {
	ALOG_INFO("ACSSDProjManager::getSSDMaxIdelSize start", "CU", "--");
	if (bNeedUpdateTableHeadList && !cacheTableHeadList())
		return false;

	QList<Interval> intervals;

	intervals.append({ ADDR_EMMC_PARTION_DATA_LIST, ADDR_EMMC_PARTION_DATA_LIST, -1 });
	intervals.append({ ADDR_EMMC_PARTION_DATA_LIST + SIZE_EMMC_PARTION_DATA_LIST, ADDR_EMMC_PARTION_DATA_LIST + SIZE_EMMC_PARTION_DATA_LIST, 64 });

	for (auto it = mCacheTableHeadMap.begin(); it != mCacheTableHeadMap.end(); it++)
	{
		intervals.append({ it.value().SSDStartOffset, it.value().SSDEndOffset, it.key() });
	}

	// 按起点排序  
	std::sort(intervals.begin(), intervals.end(), [](const Interval& a, const Interval& b) {
		return a.startAddr < b.startAddr;
		});

	uint64_t maxIdelSize = 0;
	for (size_t j = 1; j < intervals.size(); j++)
	{
		if (maxIdelSize < intervals[j].startAddr - intervals[j - 1].endAddr && intervals[j].startAddr > intervals[j - 1].endAddr)
		{
			maxIdelSize = intervals[j].startAddr - intervals[j - 1].endAddr;
		}
	}

	return maxIdelSize;
	ALOG_INFO("ACSSDProjManager::getSSDMaxIdelSize end", "CU", "--");
}

bool ACSSDProjManager::downLoadProject(ACProjManager* projManagerPtr) {

	//// 重新清理SSD
	//mCacheTableHeadMap.clear();
	//updateTableHeadListToSSD();
	ALOG_INFO("Device %s:%d start downLoad project %s.", "CU", "FP", mDevIp.toStdString().c_str(), mHop, projManagerPtr->GetProjName().toStdString().c_str());

	std::string curManufacture = projManagerPtr->GetProjData()->getChipData().getJsonValue<std::string>("manufacture");
	std::string curChipName = projManagerPtr->GetProjData()->getChipData().getJsonValue<std::string>("chipName");
	bool storeProject2DDR = false;
	if ((curChipName == "MSI313" && curManufacture == "XT") || 
		(curChipName == "XSA300D" && curManufacture == "XT") || 
		(curChipName == "MSI270" && curManufacture == "XT") ||
		(curChipName == "XSG300D" && curManufacture == "XT")){
		storeProject2DDR = true;
		ALOG_INFO("Device %s:%d download project %s to DDR.", "CU", "FP", mDevIp.toStdString().c_str(), mHop, projManagerPtr->GetProjName().toStdString().c_str());
	}
	if (!storeProject2DDR) {
		if (!cacheTableHeadList())
			return false;
	}

	if (mCacheTableHeadMap.size() > 0)
		ALOG_INFO("The following projects are already stored on device %s:%d.", "FP", "CU", mDevIp.toStdString().c_str(), mHop);
	else
		ALOG_INFO("Device %s:%d, No downloaded project exists on the device.", "FP", "CU", mDevIp.toStdString().c_str(), mHop);
	for (auto it = mCacheTableHeadMap.begin(); it != mCacheTableHeadMap.end(); it++) {
		ALOG_INFO("Device:%s:%d, Index:%s, Size:%s, Update time:%s, Name:%s.", "FP", "CU", mDevIp.toStdString().c_str(), mHop, QString::number(it.key()).rightJustified(2, '0').toStdString().c_str()
			, formatFileSize(it.value().SSDEndOffset - it.value().SSDStartOffset).toStdString().c_str()
			, QDateTime::fromSecsSinceEpoch(it.value().SSDUseTime).toString("yyyy-MM-dd hh:mm:ss").toStdString().c_str()
			, (char*)it.value().ProjectName);
	}

	DeviceStu devInfo = ACDeviceManager::instance().getDevInfo(mDevIp, mHop);
	// 检查SSD中是否存在相同工程
	QString loaclTableHeadJsonStr = exchangeOffsetJsonString(projManagerPtr->GetProjTableHeaderJson());
	ushort headerChk = 0;
	calc_crc16sum((uchar*)(loaclTableHeadJsonStr.toLocal8Bit().data()), loaclTableHeadJsonStr.toLocal8Bit().size(), &headerChk);


	for (auto it = mCacheTableJsonMap.begin(); it != mCacheTableJsonMap.end(); it++) {
		QString tmpstr = it.value();
		tmpstr = exchangeOffsetJsonString(tmpstr);
		ushort tmpheadChk = 0;
		calc_crc16sum((uchar*)(tmpstr.toLocal8Bit().data()), tmpstr.toLocal8Bit().size(), &tmpheadChk);

		QString ssdProName((char*)mCacheTableHeadMap[it.key()].ProjectName);
		QString curDownProName = projManagerPtr->GetProjName();
		curDownProName.replace('\\', '/');
		curDownProName = curDownProName.mid(curDownProName.lastIndexOf('/') + 1);


		if (tmpheadChk == headerChk && ssdProName == curDownProName) {
			QByteArray tmpReadTableHead;
			bool ret = CommandReadDataFromSSD(mDevIp.toStdString(), "SSD2FIBER", mHop, 0, ADDR_EMMC_PARTION_HEADER_LIST + it.key() * EMMC_PARTION_HEADER_SIZE, EMMC_PARTION_HEADER_SIZE, tmpReadTableHead, devInfo.tMainBoardInfo.strHardwareSN, "eMMCHeaderData");
			if (!ret)
			{
				return false;
			}
			
			QString readTableHeadJsonStr = tmpReadTableHead.mid(sizeof(eMMCTableHeader));
			readTableHeadJsonStr.remove(QChar(0));
			
			readTableHeadJsonStr = exchangeOffsetJsonString(readTableHeadJsonStr);

			if (readTableHeadJsonStr == loaclTableHeadJsonStr) {

				(*(eMMCTableHeader*)(tmpReadTableHead.data())).SSDUseTime = QDateTime::currentSecsSinceEpoch();

				ushort headerChk = 0;
				calc_crc16sum((uchar*)(tmpReadTableHead.data()) + 2, sizeof(eMMCTableHeader) - 2, &headerChk);
				(*(eMMCTableHeader*)(tmpReadTableHead.data())).HeaderCRC = headerChk;

				if (!CommandWriteDataToSSD(mDevIp.toStdString(), "FIBER2SSD", mHop, 0, ADDR_EMMC_PARTION_HEADER_LIST + it.key() * EMMC_PARTION_HEADER_SIZE, tmpReadTableHead, devInfo.tMainBoardInfo.strHardwareSN, "eMMCFileData")) {
					ALOG_FATAL("Download project table head failed.", "CU", "--");
					return false;
				}

				ALOG_INFO("Device %s:%d check user files are consistent, no need to download.", "CU", "--", mDevIp.toStdString().c_str(), mHop);
				mWritePartitionHeadAddr = ADDR_EMMC_PARTION_HEADER_LIST + it.key() * EMMC_PARTION_HEADER_SIZE;
				//binFilesCompare(it.key(), projManagerPtr);
				return true;
			}
		}
	}


	// 分区表满，需要删除一个最老工程
	if (mCacheTableHeadMap.size() == MAX_EMMC_PARTION_NUM && !freeOldestOneProj(false)) {
		return false;
	}
	uint64_t projBinFilesSize = projManagerPtr->CalcBinFilesSize();
	if (!storeProject2DDR) {
		ALOG_INFO("Device %s:%d check user files are inconsistent, need download files.", "CU", "--", mDevIp.toStdString().c_str(), mHop);
		// 计算空闲空间块
		//此处freeSSDSize耗时1分钟
		if (!freeSSDSize(projBinFilesSize, false)) {//
			ALOG_FATAL("Device %s:%d free SSD failed, download task error!", "CU", "--", mDevIp.toStdString().c_str(), mHop);
			return false;
		}
	}
	//test
	ALOG_INFO("Device %s:%d calculating download location.", "CU", "--", mDevIp.toStdString().c_str(), mHop);
	QList<Interval> intervals;
	intervals.append({ ADDR_EMMC_PARTION_DATA_LIST, ADDR_EMMC_PARTION_DATA_LIST, -1 });
	intervals.append({ ADDR_EMMC_PARTION_DATA_LIST + SIZE_EMMC_PARTION_DATA_LIST, ADDR_EMMC_PARTION_DATA_LIST + SIZE_EMMC_PARTION_DATA_LIST, 64 });
	for (auto it2 = mCacheTableHeadMap.begin(); it2 != mCacheTableHeadMap.end(); it2++)
	{
		intervals.append({ it2.value().SSDStartOffset, it2.value().SSDEndOffset, it2.key() });
	}
	// 按起点排序  
	std::sort(intervals.begin(), intervals.end(), [](const Interval& a, const Interval& b) {
		return a.startAddr < b.startAddr;
		});

	size_t k = 1;
	for (; k < intervals.size(); k++)
	{
		uint64_t idelSize = intervals[k].startAddr - intervals[k - 1].endAddr;
		if (projBinFilesSize <= idelSize)
			break;
	}

	uint64_t downLoadBinFileStartAddr = -1;
	if (k != intervals.size()) {
		downLoadBinFileStartAddr = intervals[k - 1].endAddr;
	}


	ALOG_INFO("Device %s:%d start download user files.", "CU", "--", mDevIp.toStdString().c_str(), mHop);

	// 写入档案
	int nRet = 0;
	int currentFileIndex = 0;
	uint64_t writeAddr = downLoadBinFileStartAddr;
	std::vector<tSeptBineMMCFileMap> tbinFileMapInfoVec = projManagerPtr->GetBinFileMapInfoVec();
	int nRecoverTrySend = AngKGlobalInstance::instance()->ReadValue("DeviceComm", "Retransmission").toInt();
	bool isEmmcChip = projManagerPtr->GetProjData()->getChipData().getJsonValue<std::string>("chipType") == "eMMC";
	if (isEmmcChip) {
		int allFileNum = tbinFileMapInfoVec.size();
		for (int i = 0; i < tbinFileMapInfoVec.size(); ++i) {
			tSeptBineMMCFileMap tBinFile = tbinFileMapInfoVec[i];
			if (tBinFile.PartName == "acBinXml")
				continue;
			QString strBinFiles = QString::fromStdString(tBinFile.strFileName);
			QFile file(strBinFiles);
			if (!file.open(QIODevice::ReadOnly)) {
				file.close();
				ALOG_FATAL("Open eMMC file %s failed.", "CU", "--", strBinFiles.toStdString().c_str());
				return false;
			}
			currentFileIndex++;
			for (int j = 0; j < tBinFile.vFiles.size(); ++j) {
				teMMCVFile tVFile = tBinFile.vFiles[j];
				uint64_t vfileSize = (uint64_t)tVFile.BlockNum * EMMC_COMMON_BLOCK_SIZE;

				uint64_t remainderNum = tBinFile.FileSize % EMMC_COMMON_BLOCK_SIZE;

				//边界判断
				if (j == 0 && tBinFile.vFiles.size() == 1) {
					if (vfileSize > file.size()) {
						vfileSize = file.size();
					}
				}
				else if (j == tBinFile.vFiles.size() - 1 && remainderNum != 0) {
					if (tVFile.FileBlkPos + vfileSize > file.size()) {
						vfileSize = ((uint64_t)tVFile.BlockNum - 1) * EMMC_COMMON_BLOCK_SIZE + remainderNum;
					}
				}
				quint64 fileOffset = (quint64)tVFile.FileBlkPos * EMMC_COMMON_BLOCK_SIZE;
				// 6. 分块写入
				int nSSDWriteSize = AngKGlobalInstance::instance()->ReadValue("DeviceComm", "SSDWriteSize").toInt();
				const int chunkSize = nSSDWriteSize * 1024 * 1024; // 写入SSD每次32M

				for (qint64 k = 0; k < vfileSize; k += chunkSize) {
					qint64 bytesToRead = qMin((uint64_t)chunkSize, vfileSize - k);

					//开启一个32M的buffer，每次从文件中取32M使用，如果不满足在下发时会进行对齐
					//uchar* vEntryfileDataBuffer = new uchar[bytesToRead];
					//memset(vEntryfileDataBuffer, 0, bytesToRead);
					file.seek(fileOffset);
					QByteArray chunk = file.read(bytesToRead);

					if (bytesToRead < chunkSize) {
						//最后一块进行4k补齐

						uint64_t last_Blk = 0;
						if (bytesToRead % EMMC_SSD_MULTIPLE == 0)
							last_Blk = bytesToRead / EMMC_SSD_MULTIPLE;
						else
							last_Blk = (bytesToRead / EMMC_SSD_MULTIPLE) + 1;

						QByteArray byteFileSeg;
						byteFileSeg.resize(last_Blk * EMMC_SSD_MULTIPLE);
						byteFileSeg.fill(0);
						byteFileSeg.replace(0, bytesToRead, chunk);

						int nTryReSend = nRecoverTrySend;
						while (nTryReSend != 0)
						{
							nRet = AngKMessageHandler::instance().Command_StoreDataToSSDorDDR(mDevIp.toStdString(), "FIBER2SSD", mHop, 0, writeAddr, byteFileSeg, devInfo.tMainBoardInfo.strHardwareSN, "eMMCFileData");
							QThread::msleep(10);//提高UDP的稳定性，每发送一次命令包，停10ms
							if (nRet == 0) {//返回错误需要加重传机制
								writeAddr += last_Blk * EMMC_SSD_MULTIPLE;
								nTryReSend = nRecoverTrySend;
								break;
							}
							else {
								nTryReSend--;
								ALOG_FATAL("Send eMMC file : %s to %s:%d SSD failed(errorCode: %d), Try to Retrans, %d remaining times.", "CU", "FP", strBinFiles.toStdString().c_str(), mDevIp.toStdString().c_str(), mHop, nRet, nTryReSend);
							}
						}


						if (nTryReSend <= 0 && nRet != 0) {
							nRet = -1;
							file.close();
							return false;
						}
					}
					else
					{
						int nTryReSend = AngKGlobalInstance::instance()->ReadValue("DeviceComm", "Retransmission").toInt();
						while (nTryReSend != 0)
						{
							nRet = AngKMessageHandler::instance().Command_StoreDataToSSDorDDR(mDevIp.toStdString(), "FIBER2SSD", mHop, 0, writeAddr, chunk, devInfo.tMainBoardInfo.strHardwareSN, "eMMCFileData");
							QThread::msleep(10);//提高UDP的稳定性，每发送一次命令包，停10ms
							if (nRet == 0) {//返回错误需要加重传机制
								writeAddr += bytesToRead;
								fileOffset += bytesToRead;
								nTryReSend = nRecoverTrySend;
								break;
							}
							else {
								nTryReSend--;
								ALOG_FATAL("Send eMMC file : %s to %s:%d SSD failed(errorCode: %d), Try to Retrans, %d remaining times.", "CU", "FP", strBinFiles.toStdString().c_str(), mDevIp.toStdString().c_str(), mHop, nRet, nTryReSend);
							}
						}

						if (nTryReSend <= 0 && nRet != 0) {
							nRet = -1;
							file.close();
							return false;
						}
					}
				}
			}
			//关键事件记录
			nlohmann::json dataTransferJson;
			dataTransferJson["ProgSN"] = devInfo.tMainBoardInfo.strHardwareSN;
			dataTransferJson["DataType"] = "eMMCFileData";
			dataTransferJson["CMD"] = "FIBER2SSD";
			dataTransferJson["RetCode"] = 0;
			dataTransferJson["SrcAddr"] = 0;
			dataTransferJson["DestAddr"] = QString("%1").arg(DDR_SharedMemeroyExchangePC2MUOffset_PL, 8, 16, QLatin1Char('0')).toUpper().toStdString();
			dataTransferJson["DataSize"] = QString("%1").arg(file.size(), 8, 16, QLatin1Char('0')).toUpper().toStdString();
			ACEventLogger::eventLogger()->SendEvent(ACEventBuild::eventBuilder()->GetDataTransfer(dataTransferJson));
			file.close();
			int nFileValue = 60 + (static_cast<float>(currentFileIndex) / allFileNum) * 39;
			emit sgnUpdateProjWriteProcess(nFileValue, mDevIp, mHop);
		}
	}
	else {
		uchar byBuff[BUFFER_RW_SIZE];
		ADR  adrOff = 0;
		uint64_t BytesRead;
		uint64_t BufCondenseSize = 0;
		if (storeProject2DDR) {
			writeAddr = ADDR_DDR_PARTION_DATA_LIST;
		}

		uint uiReadLen;
		int partCnt = projManagerPtr->GetProjDataBuffer()->GetPartitionCount();
		for (int partIdx = 0; partIdx < partCnt; ++partIdx)
		{
			const tPartitionInfo* partInfo;
			partInfo = projManagerPtr->GetProjDataBuffer()->GetPartitionInfo(partIdx);

			if (!partInfo->PartitionShow)
				continue;

			int bufCnt = projManagerPtr->GetProjDataBuffer()->GetBufferCount(partIdx);
			for (int bufIdx = 0; bufIdx < bufCnt; ++bufIdx) {
				const tBufInfo* BufInfo;
				BufInfo = projManagerPtr->GetProjDataBuffer()->GetBufferInfo(bufIdx, partIdx);

				if (!BufInfo->m_bBufferShow)
					continue;

				adrOff = BufInfo->llBufStart;
				while (adrOff <= BufInfo->llBufEnd) {///压缩存储
					BytesRead = (BufInfo->llBufEnd + 1 - adrOff) > BUFFER_RW_SIZE ? BUFFER_RW_SIZE : (BufInfo->llBufEnd + 1 - adrOff);
					uiReadLen = projManagerPtr->GetProjDataBuffer()->BufferRead(adrOff, byBuff, BytesRead);
					if (uiReadLen <= 0) {
						break;
					}
					adrOff += uiReadLen;

					ADR bufferSize = uiReadLen;
					uint64_t vfileSize = bufferSize;
					quint64 fileOffset = (quint64)0;

					int nSSDWriteSize = AngKGlobalInstance::instance()->ReadValue("DeviceComm", "SSDWriteSize").toInt();
					const int chunkSize = nSSDWriteSize * 1024 * 1024; // 写入SSD每次32M

					for (qint64 k = 0; k < vfileSize; k += chunkSize) {
						qint64 bytesToRead = qMin((uint64_t)chunkSize, vfileSize - k);

						//开启一个32M的buffer，每次从文件中取32M使用，如果不满足在下发时会进行对齐
						QByteArray chunk((char*)byBuff, bytesToRead);

						if (bytesToRead < chunkSize) {
							//最后一块进行4k补齐

							uint64_t last_Blk = 0;
							if (bytesToRead % EMMC_SSD_MULTIPLE == 0)
								last_Blk = bytesToRead / EMMC_SSD_MULTIPLE;
							else
								last_Blk = (bytesToRead / EMMC_SSD_MULTIPLE) + 1;

							QByteArray byteFileSeg;
							byteFileSeg.resize(last_Blk * EMMC_SSD_MULTIPLE);
							byteFileSeg.fill(0);
							byteFileSeg.replace(0, bytesToRead, chunk);

							int nTryReSend = nRecoverTrySend;
							while (nTryReSend != 0)
							{
								nRet = AngKMessageHandler::instance().Command_StoreDataToSSDorDDR(mDevIp.toStdString(), "FIBER2DDR", mHop, 
									0, writeAddr, byteFileSeg, devInfo.tMainBoardInfo.strHardwareSN, "eMMCFileData");
								ALOG_INFO("Send buffer to %s:%d DDR 0x%08X Ret: %d, remaining times: %d.", "CU", "FP",
									mDevIp.toStdString().c_str(), mHop, writeAddr, nRet, nTryReSend);
								QThread::msleep(10);//提高UDP的稳定性，每发送一次命令包，停10ms
								if (nRet == 0) {//返回错误需要加重传机制
									writeAddr += last_Blk * EMMC_SSD_MULTIPLE;
									nTryReSend = nRecoverTrySend;
									break;
								}
								else {
									nTryReSend--;
									ALOG_FATAL("Send buffer to %s:%d DDR failed(errorCode: %d), Try to Retrans, %d remaining times.", "CU", "FP",
										mDevIp.toStdString().c_str(), mHop, nRet, nTryReSend);
								}
							}


							if (nTryReSend <= 0 && nRet != 0) {
								nRet = -1;
								return false;
							}
						}
						else
						{
							int nTryReSend = AngKGlobalInstance::instance()->ReadValue("DeviceComm", "Retransmission").toInt();
							while (nTryReSend != 0)
							{
								nRet = AngKMessageHandler::instance().Command_StoreDataToSSDorDDR(mDevIp.toStdString(), "FIBER2DDR", mHop, 0, writeAddr, chunk, devInfo.tMainBoardInfo.strHardwareSN, "eMMCFileData");
								QThread::msleep(10);//提高UDP的稳定性，每发送一次命令包，停10ms
								if (nRet == 0) {//返回错误需要加重传机制
									writeAddr += bytesToRead;
									fileOffset += bytesToRead;
									nTryReSend = nRecoverTrySend;
									break;
								}
								else {
									nTryReSend--;
									ALOG_FATAL("Send buffer to %s:%d DDR failed(errorCode: %d), Try to Retrans, %d remaining times.", "CU", "FP",
										mDevIp.toStdString().c_str(), mHop, nRet, nTryReSend);
								}
							}

							if (nTryReSend <= 0 && nRet != 0) {
								nRet = -1;
								return false;
							}
						}
					}
				}
			}
		}

		//关键事件记录
		nlohmann::json dataTransferJson;
		dataTransferJson["ProgSN"] = devInfo.tMainBoardInfo.strHardwareSN;
		dataTransferJson["DataType"] = "eMMCFileData";
		dataTransferJson["CMD"] = "FIBER2SSD";
		dataTransferJson["RetCode"] = 0;
		dataTransferJson["SrcAddr"] = 0;
		dataTransferJson["DestAddr"] = QString("%1").arg(DDR_SharedMemeroyExchangePC2MUOffset_PL, 8, 16, QLatin1Char('0')).toUpper().toStdString();
		dataTransferJson["DataSize"] = QString("%1").arg(0, 8, 16, QLatin1Char('0')).toUpper().toStdString();
		ACEventLogger::eventLogger()->SendEvent(ACEventBuild::eventBuilder()->GetDataTransfer(dataTransferJson));
		emit sgnUpdateProjWriteProcess(99, mDevIp, mHop);
	}

	// 如果出现错误，直接返回，不用写表头
	if (nRet)
		return false;

	// 写分区表表头
	if (!storeProject2DDR)
	{

		ALOG_INFO("Device %s:%d start write partition table head.", "CU", "--", mDevIp.toStdString().c_str(), mHop);
		int tableHeadIdx = -1;
		for (int i = 0; i < MAX_EMMC_PARTION_NUM; i++) {
			if (mCacheTableHeadMap.find(i) == mCacheTableHeadMap.end()) {
				tableHeadIdx = i;
				break;
			}
		}

		if (tableHeadIdx == -1) {
			if (!freeOldestOneProj(false))
				return false;
			for (int i = 0; i < MAX_EMMC_PARTION_NUM; i++) {
				if (mCacheTableHeadMap.find(i) == mCacheTableHeadMap.end()) {
					tableHeadIdx = i;
					break;
				}
			}
		}

		int ret = 0;

		nlohmann::json eMMC_Json;
		QByteArray dataStruct;//存放所有数据
		QByteArray partitionTable;
		eMMCTableHeader eHeader;
		memset(&eHeader, 0, sizeof(eHeader));

		//Feature 部分
		nlohmann::json featureJson;
		featureJson["ByteSumEn"] = 1;
		featureJson["CRC16En"] = 1;
		featureJson["DataOffsetInSSD"] = "0x" + QString::number(ADDR_EMMC_PARTION_HEADER_LIST + tableHeadIdx * EMMC_PARTION_HEADER_SIZE, 16).toStdString();

		//EXTCSD 部分
		std::string projExtCSD = projManagerPtr->GetEMMCExtCSDParaJson().toStdString();
		nlohmann::json regConfigJson = nlohmann::json::array();
		nlohmann::json partitonSizeJson = nlohmann::json::array();
		nlohmann::json EXTCSDJson;
		if (!projExtCSD.empty()) {
			try {
				auto eventJson = nlohmann::json::parse(projExtCSD);
				EXTCSDJson["RegConfig"] = eventJson["RegConfig"];
				EXTCSDJson["PartitionSize"] = eventJson["PartitionSize"];
			}
			catch (nlohmann::json::exception& e) {
				EXTCSDJson["RegConfig"] = nlohmann::json::array();
				EXTCSDJson["PartitionSize"] = nlohmann::json::array();
				ALOG_FATAL("DownloadBinFiles Parse EXTCSD Json error : %s.", "CU", "--", e.what());
			}
		}
		else {
			EXTCSDJson["RegConfig"] = nlohmann::json::array();
			EXTCSDJson["PartitionSize"] = nlohmann::json::array();
		}

		//ByteSum 部分
		nlohmann::json byteSumJson = nlohmann::json::array();
		std::string curParttion = "";
		uint64_t nByteSum = 0;
		CalBinFilesByteSum(tbinFileMapInfoVec, byteSumJson);


		//Files 部分
		nlohmann::json filesJson = nlohmann::json::array();
		uint64_t ssdAddr = 0;//SSD_WRITE_PARTITION_DATA;
		std::vector<uint64_t> vecSSD_Offset;
		uint32_t allFileTotalBlockSize = 0;
		for (int i = 0; i < tbinFileMapInfoVec.size(); ++i)
		{
			uint64_t nBytesum = 0;
			tSeptBineMMCFileMap tBinFile = tbinFileMapInfoVec[i];
			if (tBinFile.PartName == "acBinXml")
				continue;
			std::string strTPartition = tBinFile.PartName;
			for (int j = 0; j < tBinFile.vFiles.size(); ++j) {
				teMMCVFile tVFile = tBinFile.vFiles[j];
				nlohmann::json entryFile;
				entryFile["Entry"] = tVFile.EntryIndex;
				entryFile["Partition"] = tVFile.PartName;
				entryFile["SSDOffset"] = QString("0x%1").arg(ssdAddr, 8, 16, QLatin1Char('0')).toStdString();

				uint64_t dataRealSize = (uint64_t)tVFile.BlockNum * EMMC_COMMON_BLOCK_SIZE;
				uint64_t last_Blk = 0;
				if (dataRealSize % EMMC_SSD_MULTIPLE == 0)
					last_Blk = dataRealSize / EMMC_SSD_MULTIPLE;
				else
					last_Blk = (dataRealSize / EMMC_SSD_MULTIPLE) + 1;
				//ssdAddr += (uint64_t)tVFile.BlockNum * EMMC_COMMON_BLOCK_SIZE;
				ssdAddr += last_Blk * EMMC_SSD_MULTIPLE;
				allFileTotalBlockSize += tVFile.BlockNum;
				entryFile["ChipBlockPos"] = QString("0x%1").arg(tVFile.ChipBlkPos, 8, 16, QLatin1Char('0')).toStdString();
				entryFile["BlockNum"] = tVFile.BlockNum;
				entryFile["CRC16"] = QString("0x%1").arg(tVFile.CRCValue, 8, 16, QLatin1Char('0')).toStdString();
				filesJson.push_back(entryFile);
			}
		}

		featureJson["TotalBlockNum"] = allFileTotalBlockSize;

		eMMC_Json["Feature"] = featureJson;
		eMMC_Json["EXTCSD"] = EXTCSDJson;
		eMMC_Json["ByteSum"] = byteSumJson;
		eMMC_Json["Files"] = filesJson;

		eHeader.Magic = 0x4150524F;
		eHeader.Version = 2;
		eHeader.HeaderLen = 512;

		std::string strMagic = "Json";
		memcpy(eHeader.MagicType, strMagic.c_str(), strMagic.size());
		eHeader.PartitionTableSize = eMMC_Json.dump().size();

		ushort partitionTableCRC = 0;
		QString tmpstr = eMMC_Json.dump().c_str();

		calc_crc16sum((uchar*)(tmpstr.toUtf8().data()), tmpstr.toLocal8Bit().size(), &partitionTableCRC);
		eHeader.PartitionTableCRC = partitionTableCRC;

		eHeader.SSDStartOffset = downLoadBinFileStartAddr;
		eHeader.SSDEndOffset = writeAddr;

		eHeader.SSDUseTime = QDateTime::currentSecsSinceEpoch();
		eHeader.IsComplete = 1;

		QString progName = projManagerPtr->GetProjName();
		progName.replace('\\', '/');
		progName = progName.mid(progName.lastIndexOf('/') + 1);

		memcpy((char*)eHeader.ProjectName, progName.toStdString().c_str(), min(progName.toUtf8().size(), sizeof(eHeader.ProjectName) - 1));

		ushort headerChk = 0;
		calc_crc16sum((uchar*)(&eHeader) + 2, sizeof(eHeader) - 2, &headerChk);
		eHeader.HeaderCRC = headerChk;



		//uint64_t jsonAddr = ADDR_EMMC_PARTION_HEADER_LIST + tableHeadIdx * EMMC_PARTION_HEADER_SIZE + sizeof(eMMCTableHeader);

		//QByteArray jsonBytes(eMMC_Json.dump().c_str());
		//if (!CommandWriteDataToSSD(mDevIp.toStdString(), "FIBER2SSD", mHop, 0, jsonAddr, jsonBytes, devInfo.tMainBoardInfo.strHardwareSN, "eMMCFileData")) {
		//	ALOG_FATAL("Download project table head json failed", "CU", "--");
		//	return false;
		//}
		QByteArray tmpBytes((int)EMMC_PARTION_HEADER_SIZE, '\0');
		memcpy(tmpBytes.data(), (char*)&eHeader, sizeof(eHeader));
		memcpy(tmpBytes.data() + sizeof(eHeader), eMMC_Json.dump().c_str(), eMMC_Json.dump().size());

		if (!CommandWriteDataToSSD(mDevIp.toStdString(), "FIBER2SSD", mHop, 0, ADDR_EMMC_PARTION_HEADER_LIST + tableHeadIdx * EMMC_PARTION_HEADER_SIZE, tmpBytes, devInfo.tMainBoardInfo.strHardwareSN, "eMMCFileData")) {
			ALOG_FATAL("Download project table head failed.", "CU", "--");
			return false;
		}
		else {
			ALOG_INFO("Send partition header to %s:%d SSD offset: 0x%08X.", "CU", "--",
				mDevIp.toStdString().c_str(), mHop, ADDR_EMMC_PARTION_HEADER_LIST + tableHeadIdx * EMMC_PARTION_HEADER_SIZE, nRet);

			QByteArray tmpReadTableHead;
			bool ret = CommandReadDataFromSSD(mDevIp.toStdString(), "SSD2FIBER", mHop, 0, ADDR_EMMC_PARTION_HEADER_LIST + tableHeadIdx * EMMC_PARTION_HEADER_SIZE, EMMC_PARTION_HEADER_SIZE, tmpReadTableHead, devInfo.tMainBoardInfo.strHardwareSN, "eMMCHeaderData");
			if (!ret)
			{
				return false;
			}
		}

		mWritePartitionHeadAddr = ADDR_EMMC_PARTION_HEADER_LIST + tableHeadIdx * EMMC_PARTION_HEADER_SIZE;
	}


	ALOG_INFO("Device %s:%d download task end.", "CU", "--", mDevIp.toStdString().c_str(), mHop);
	return true;
}

bool ACSSDProjManager::updateTableHeadListToSSD() {
	DeviceStu devInfo = ACDeviceManager::instance().getDevInfo(mDevIp, mHop);
	for (int i = 0; i < MAX_EMMC_PARTION_NUM; i++) {
		eMMCTableHeader tmpTableHead;
		memset((char*)&tmpTableHead, 0, sizeof(tmpTableHead));
		QString jsonStr;
		if (mCacheTableHeadMap.find(i) != mCacheTableHeadMap.end()) {
			memcpy((char*)&tmpTableHead, (char*)&mCacheTableHeadMap.find(i).value(), sizeof(tmpTableHead));
			jsonStr = mCacheTableJsonMap.find(i).value();
		}

		QByteArray tmpBytes((int)EMMC_PARTION_HEADER_SIZE, '\0');
		memcpy(tmpBytes.data(), (char*)&tmpTableHead, sizeof(tmpTableHead));
		memcpy(tmpBytes.data() + sizeof(tmpTableHead), (char*)jsonStr.toStdString().c_str(), jsonStr.toLocal8Bit().size());

		if (!CommandWriteDataToSSD(mDevIp.toStdString(), "FIBER2SSD", mHop, 0, ADDR_EMMC_PARTION_HEADER_LIST + i * EMMC_PARTION_HEADER_SIZE, tmpBytes, devInfo.tMainBoardInfo.strHardwareSN, "eMMCFileData"))
			return false;
	}
	return true;
}

bool ACSSDProjManager::freeSSDSize(uint64_t size, bool bNeedUpdateTableHeadList) {

	if (bNeedUpdateTableHeadList && !cacheTableHeadList())
		return false;

	if (size + ALIGNMENT_4K < getSSDMaxIdelSize(false)) {

		return updateTableHeadListToSSD();
	}
	uint64_t oldestTime = mCacheTableHeadMap.begin().value().SSDUseTime;
	auto oldestItor = mCacheTableHeadMap.begin();
	for (auto it = mCacheTableHeadMap.begin(); it != mCacheTableHeadMap.end(); it++)
	{
		if (oldestTime > it.value().SSDUseTime){
			oldestTime = it.value().SSDUseTime;
			oldestItor = it;
		}
	}

	ALOG_INFO("Device %s:%d has deleted project %s.", "CU", "FP", mDevIp.toStdString().c_str(), mHop, (char*)oldestItor.value().ProjectName);
	mCacheTableJsonMap.remove(oldestItor.key());
	mCacheTableHeadMap.erase(oldestItor);
	return freeSSDSize(size, false);
}

bool ACSSDProjManager::freeOldestOneProj(bool bNeedUpdateTableHeadList) {
	if (bNeedUpdateTableHeadList && !cacheTableHeadList())
		return false;

	uint64_t oldestTime = mCacheTableHeadMap.begin().value().SSDUseTime;
	auto oldestItor = mCacheTableHeadMap.begin();
	for (auto it = mCacheTableHeadMap.begin(); it != mCacheTableHeadMap.end(); it++)
	{
		if (oldestTime > it.value().SSDUseTime) {
			oldestTime = it.value().SSDUseTime;
			oldestItor = it;
		}
	}

	ALOG_FATAL("Device %s:%d has deleted project %s.", "CU", "FP", mDevIp.toStdString().c_str(), mHop, (char*)oldestItor.value().ProjectName);
	mCacheTableJsonMap.remove(oldestItor.key());
	mCacheTableHeadMap.erase(oldestItor);
	return updateTableHeadListToSSD();
}


bool ACSSDProjManager::binFilesCompare(int partionTableHeadIndex, ACProjManager* projManagerPtr) {
	eMMCTableHeader tableHead = mCacheTableHeadMap.find(partionTableHeadIndex).value();
	QString headJsonStr = mCacheTableJsonMap.find(partionTableHeadIndex).value();

	auto headJson = nlohmann::json::parse(headJsonStr.toStdString());
	DeviceStu devInfo = ACDeviceManager::instance().getDevInfo(mDevIp, mHop);


	std::vector<tSeptBineMMCFileMap> vecBinFile = projManagerPtr->GetBinFileMapInfoVec();
	//双端档案和SSD比对
	for (int i = 0; i < vecBinFile.size(); ++i) {

		QString srcBinFile = QString::fromStdString(vecBinFile[i].strFileName);
		QFile entryFile(srcBinFile);
		if (!entryFile.open(QIODevice::ReadOnly)) {
			return false;
		}
		else {
			if (vecBinFile[i].PartName == "acBinXml")
				continue;

			for (int j = 0; j < vecBinFile[i].vFiles.size(); ++j) {
				teMMCVFile tVFile = vecBinFile[i].vFiles[j];
				entryFile.seek(tVFile.FileBlkPos * 512);
				QByteArray readSrc_1K = entryFile.read(1024 * 1024);
				if (readSrc_1K.size() != 1024 * 1024) {
					continue;
				}


				QByteArray ssdBytes;
				if (headJson.contains("Files") && headJson["Files"].is_array()) {
					for (const auto& it : headJson["Files"]) {

						if (it["ChipBlockPos"].get<std::string>() == QString("0x%1").arg(tVFile.ChipBlkPos, 8, 16, QLatin1Char('0')).toStdString()) {
							bool ret = CommandReadDataFromSSD(mDevIp.toStdString(), "SSD2FIBER", mHop, 0, tableHead.SSDStartOffset + std::stoull(it["SSDOffset"].get<std::string>(), nullptr, 16), 1024 * 1024, ssdBytes, devInfo.tMainBoardInfo.strHardwareSN, "eMMCHeaderData");
							if (!ret)
							{
								return false;
							}
						}
					}
				}



				if (ssdBytes == readSrc_1K) {
					continue;
				}
				else {
					return false;
				}
			}
		}
	}

	return true;


}

bool ACSSDProjManager::createSSDJsonFile(const QString& fileName) {

	if (!cacheTableHeadList() || mCacheTableHeadMap.size() == 0)
		return false;


	nlohmann::json ssdJson;
	nlohmann::json emmcTableHeadJsonArr = nlohmann::json::array();
	for (auto it : mCacheTableHeadMap) {
		nlohmann::json tabHeadJson;
		tabHeadJson["SSDOffset"] = it.SSDStartOffset;
		tabHeadJson["ImgSize"] = it.SSDEndOffset - it.SSDStartOffset;
		emmcTableHeadJsonArr.push_back(tabHeadJson);
	}

	ssdJson["EmmcTableHeaders"] = emmcTableHeadJsonArr;

	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly))
	{
		return false;
	}

	file.write(ssdJson.dump().c_str(), ssdJson.dump().length());

	file.close();
	return true;
}