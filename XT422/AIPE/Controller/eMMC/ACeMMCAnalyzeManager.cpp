#include "ACeMMCAnalyzeManager.h"
#include <vector>
#include <fstream>
#include <QDir>
#include <QElapsedTimer>
#include "json.hpp"
#include "CAcMbn.h"
#include "AngkLogger.h"
#include "ProgressDialogSingleton.h"
#include "Thread/ThreadPool.h"

extern Acro::Thread::ThreadPool g_ThreadPool;

int calc_crc16sum(unsigned char* buf, unsigned int size, unsigned short* pCRC16Sum);
void Crc32CalcSubRoutine(CHKINFO* pChkInfo, uint8_t* buf, uint32_t size);
void Crc32GetChkSum(CHKINFO* pChkInfo);

constexpr int HUGE_BUF_SIZE = 1024 * 1024;
constexpr int useAcMbnMode = 0;

static unsigned long int __crc32_reflect(unsigned long int ref, char ch)
{
	int i;
	unsigned long int value = 0;
	// 交换bit0和bit7，bit1和bit6，类推
	for (i = 1; i < (ch + 1); i++) {
		if (ref & 1)
			value |= 1 << (ch - i);
		ref >>= 1;
	}
	return value;
}

int MMCCalcCRC16(EMMCCRC* pCRC, uint8_t* buf, int size)
{
	return calc_crc16sum(buf, size, (unsigned short*)pCRC->CRCValue);
}

void BytesumCalc(CHKINFO* pChkInfo, uint8_t* buf, uint32_t size)
{
	uint32_t i;
	for (i = 0; i < size; ++i) {
		pChkInfo->chksum += buf[i];
	}
	pChkInfo->sumlen += size;
}

void __MakeCard_Crc32CalcSubRoutine(CHKINFO* pChkInfo, uint8_t* buf, uint32_t size)
{
	tMakeCRC32Info* pCRC32Info = (tMakeCRC32Info*)pChkInfo->PrivData;
	Crc32CalcSubRoutine(&pCRC32Info->CRC32ChkInfo, buf, size);//只是个中间步骤
	BytesumCalc(pChkInfo, buf, size);///还计算个Bytesum
}

void __MakeCard_Crc32GetChkSum(CHKINFO* pChkInfo)
{
	tMakeCRC32Info* pCRC32Info = (tMakeCRC32Info*)pChkInfo->PrivData;
	Crc32GetChkSum(&pCRC32Info->CRC32ChkInfo);
}

ACeMMCAnalyzeManager::ACeMMCAnalyzeManager(QObject *parent)
	: QObject(parent)
{
	mSemaphore = new QSemaphore(0);
}

ACeMMCAnalyzeManager::~ACeMMCAnalyzeManager()
{
}

int ACeMMCAnalyzeManager::ImgFmtSPBinParserFiles(std::vector<MMAKEFILE>& Files, int ChkSumType, const std::string& strAcMbnFile, QWidget* call_wgt)
{
	uint8_t* pBuf = NULL;
	ERROR_TYPE ret = E_OK;

	EMMCCRC StdCRCVirgin;
	uint8_t Virgin = 0x00;
	unsigned long long BytesRead = 0, BytesToRead;
	unsigned long long FileLen = 0, Offset = 0;

	HUGEBLKINFO CurHugeBlk, PreHugeBlk;
	unsigned int CurPartIdx = 0;
	eMMCVFILE* pVFile = NULL;
	CHKINFO ChkInfo;
	uint8_t TotalSum = 0;
	CHKINFO TmpCRC32ChkInfo;

	CAcMbn AcMbn;
	FuChksumCalc fnChksumCalc = NULL;
	if (ChkSumType == 1) {
		fnChksumCalc = BytesumCalc;
	}
	else {
		fnChksumCalc = __MakeCard_Crc32CalcSubRoutine;
	}

	QElapsedTimer timer;
	//ProgressDialogSingleton dlgpro(qobject_cast<QWidget*>(this));
	ProgressDialogSingleton dlgpro(call_wgt);
	bool bStop = false;

	connect(this, &ACeMMCAnalyzeManager::updateProgress, &dlgpro, &ProgressDialogSingleton::updateProgress);
	connect(&dlgpro, &ProgressDialogSingleton::canceled, [&bStop]() {bStop = true; });
	connect(this, &ACeMMCAnalyzeManager::sgnCLoseProgressDialog, &dlgpro, &ProgressDialogSingleton::onSlotCLoseProgressDialog);

	pBuf = new uint8_t[HUGE_BUF_SIZE];
	if (!pBuf) {
		ret = E_ALLOC_MEM;
		goto __end;
	}

	InitCrcModule(&StdCRCVirgin);
	memset(pBuf, Virgin, HUGE_BUF_SIZE);
	StdCRCVirgin.CalcCRC(&StdCRCVirgin, pBuf, HUGE_BUF_SIZE);


	if (useAcMbnMode == 1) {

		ALOG_INFO("Save ACMBN File For Future Use: %s", "CU", "--", strAcMbnFile.c_str());
		int RetCallAcMbn = AcMbn.OpenFile(strAcMbnFile, std::ios::in | std::ios::out | std::ios::binary);
		if (RetCallAcMbn != 0) {
			ret = E_FIALED; goto __end;
		}
		RetCallAcMbn = AcMbn.AddHeader(Files.size());
		if (RetCallAcMbn != 0) {
			ret = E_FIALED; goto __end;
		}
	}

	float nProgress = 0;
	timer.start();

	bool bAnalizeErr = false;

	g_ThreadPool.PushTask([&]() {

		auto endFunc = [&](bool bAnaErr) {
			bAnalizeErr = bAnaErr;
			mSemaphore->release();
		};

		uint64_t allFilesSize = 0;
		uint64_t curReadSize = 0;
		for (unsigned int i = 0; i < Files.size(); ++i) {
			allFilesSize += Files[i].Size;
		}

		for (unsigned int i = 0; i < Files.size(); ++i) {
			tAcMbnFileInfo MbnFileInfo;
			QString parseFile = QDir::toNativeSeparators(QString::fromStdString(Files[i].strFileName));
			ALOG_INFO("Parsing File: %s", "CU", "--", QDir::fromNativeSeparators(parseFile).toStdString().c_str());
			memset(&CurHugeBlk, 0, sizeof(HUGEBLKINFO));
			memset(&PreHugeBlk, 0, sizeof(HUGEBLKINFO));
			memset(&ChkInfo, 0, sizeof(CHKINFO));
			CurHugeBlk.wVVirgin = Virgin;
			InitCrcModule(&CurHugeBlk.eMMCCrc);

			if (fnChksumCalc == __MakeCard_Crc32CalcSubRoutine) {
				memset(&TmpCRC32ChkInfo, 0, sizeof(CHKINFO));
				ChkInfo.PrivData = &TmpCRC32ChkInfo;
			}

			PreHugeBlk.bIsVirgin = 1;///初始化为空白
				//Virgin=0x0;
			Offset = 0;
			CurPartIdx = Files[i].PartIndex;
			Files[i].VFileCnt = 0;
			Files[i].pVFileHead = NULL;


			std::ifstream inFile;
			inFile.open(parseFile.toStdWString(), std::ios::binary | std::ios::ate);
			if (!inFile.is_open()) {
				ALOG_FATAL("Intelligent analysis failed to open document", "CU", "--");
				ret = E_OPEN_FILE;
				endFunc(true);
				emit sgnCLoseProgressDialog();
				return;
			}
			FileLen = inFile.tellg();


			if (useAcMbnMode == 1) {
				memset(&MbnFileInfo.Header, 0, sizeof(tAcMbnFileInfoHeader));
				memcpy(MbnFileInfo.Header.FilePath, Files[i].strFileName.c_str(), Files[i].strFileName.length());

				MbnFileInfo.Header.FileLocationAddr = Files[i].StartAddr;
				MbnFileInfo.Header.FileSize = FileLen;
				MbnFileInfo.Header.FileChecksumType = ChkSumType;
				MbnFileInfo.Header.FileChkSum = 0; ///Need To Be Modified Later
				memset(MbnFileInfo.Header.FileMD5, 0, 16);
				MbnFileInfo.Header.PartitionIndex = CurPartIdx;
				MbnFileInfo.Header.VFileNum = 0; ///Need To Be Modified Later
				MbnFileInfo.vVFiles.clear();
			}

			inFile.seekg(0, std::ios::beg);
			Offset = 0;
			int nCurrentProgress = (float)i / Files.size() * 100;
			float mgaProgress = 1.0 / Files.size();
			while (Offset < FileLen) {
				if (bStop == true) {
					ALOG_INFO("Parsing files had been stopped", "CU", "--");
					ret = E_CANCEL;
					endFunc(true);
					emit sgnCLoseProgressDialog();
					return;
				}
				BytesToRead = FileLen - Offset;
				if (BytesToRead > HUGE_BUF_SIZE)
					BytesToRead = HUGE_BUF_SIZE;
				memset(pBuf, Virgin, HUGE_BUF_SIZE);
				inFile.read((char*)pBuf, BytesToRead);
				BytesRead = inFile.gcount();
				fnChksumCalc(&ChkInfo, (uint8_t*)pBuf, BytesRead);///计算CRC32

				BytesRead = (BytesRead + 511) & (~0x1FF);////如果文件的大小不是512的整数倍，需要将其变为512的整数倍进行分析
				CurHugeBlk.bIsVirgin = 1;
				CurHugeBlk.dwBlkNum = BytesRead >> 9;
				CurHugeBlk.dwBlkStart = Offset >> 9;

				calc_crc16sum_checkvirgin(&CurHugeBlk.eMMCCrc, pBuf, BytesRead, Virgin, &CurHugeBlk.bIsVirgin);
				if (CurHugeBlk.bIsVirgin == 0 && PreHugeBlk.bIsVirgin == 1) {//当前非空白，前一个空白,一个新的文件开始
					if (pVFile != NULL) {
						ret = E_FIALED;
						endFunc(true);
						return;
					}
					pVFile = MallocVFile();
					if (pVFile == NULL) {
						ret = E_ALLOC_MEM;
						endFunc(true);
						return;
					}
					pVFile->dwBlkStart = CurHugeBlk.dwBlkStart;
					pVFile->dwBlkNum = CurHugeBlk.dwBlkNum;
					pVFile->CRCType = CurHugeBlk.eMMCCrc.CRCType;
					memcpy(pVFile->CRCValue, CurHugeBlk.eMMCCrc.CRCValue, CRC_LEN);
				}
				else if (CurHugeBlk.bIsVirgin == 1 && PreHugeBlk.bIsVirgin == 0) {//当前空白，前一个非空白，一个文件结束
					if (pVFile == NULL) {
						ret = E_FIALED;
						endFunc(true);
						return;
					}
					CalcVFileFeature(pVFile, inFile, Virgin);///计算虚拟文件的特征值
					PrintFindFile(pVFile);
					AttachVFile(Files[i], pVFile);
					memset(CurHugeBlk.eMMCCrc.CRCValue, 0, CRC_LEN);
					pVFile = NULL;
				}
				else if (CurHugeBlk.bIsVirgin == 0 && PreHugeBlk.bIsVirgin == 0) {////当前非空白，前一个非空白，需要合并
					if (pVFile == NULL) {
						ret = E_FIALED;
						endFunc(true);
						return;
					}
					pVFile->dwBlkNum += CurHugeBlk.dwBlkNum;
					memcpy(pVFile->CRCValue, CurHugeBlk.eMMCCrc.CRCValue, CRC_LEN);
				}
				else if (CurHugeBlk.bIsVirgin == 1 && PreHugeBlk.bIsVirgin == 1) {//当前空白，前一个也空白
					memset(CurHugeBlk.eMMCCrc.CRCValue, 0, CRC_LEN);
				}

				memcpy(&PreHugeBlk, &CurHugeBlk, sizeof(HUGEBLKINFO));
				Offset += BytesRead;
				curReadSize += BytesRead;
				nProgress = (float)SetProgress(Offset, FileLen);
				float nCurProgress = (float)curReadSize / allFilesSize * 100;

				emit updateProgress(nCurProgress);

			}
			if (pVFile != NULL) {
				CalcVFileFeature(pVFile, inFile, Virgin);///计算虚拟文件的特征值
				AttachVFile(Files[i], pVFile);
				memset(CurHugeBlk.eMMCCrc.CRCValue, 0, CRC_LEN);
				PrintFindFile(pVFile);
				pVFile = NULL;
			}

			if (fnChksumCalc == __MakeCard_Crc32CalcSubRoutine) {
				__MakeCard_Crc32GetChkSum(&ChkInfo);
			}
			Files[i].CheckSum = ChkInfo.chksum;
			///进行分区Bytesum值的累加
			TotalSum += Files[i].CheckSum;
			PrintTotal(Files[i], i, &ChkInfo, ChkSumType);


			if (useAcMbnMode == 1) {
				MbnFileInfo.Header.FileChkSum = Files[i].CheckSum;
				eMMCVFILE* pCurVFile = Files[i].pVFileHead;
				while (pCurVFile) {
					tAcMbnVFile MbnVFile;
					memset(&MbnVFile, 0, sizeof(tAcMbnVFile));
					MbnVFile.VFileBlockStart = pCurVFile->dwBlkStart;
					MbnVFile.VFileBlockNum = pCurVFile->dwBlkNum;
					MbnVFile.CRC16 = *(unsigned short*)pCurVFile->CRCValue;
					MbnVFile.Feature = pCurVFile->Feature;
					pCurVFile = pCurVFile->pNext;
					MbnFileInfo.vVFiles.push_back(MbnVFile);
				}
				MbnFileInfo.Header.VFileNum = MbnFileInfo.vVFiles.size();
				AcMbn.AddFileInfo(&MbnFileInfo);
			}


			inFile.close();
		}

		endFunc(false);
		emit updateProgress(100);
		});



	dlgpro.showProgressDialog(100, tr("Intelligent analysis in progress, please wait..."), tr("Intelligent Analysis"), ProgressDialogSingleton::DLG_EXEC);

	mSemaphore->acquire();
	if (bAnalizeErr)
		goto __end;
	
	qint64 elapsedTime = timer.elapsed();  // 获取经过的时间（毫秒）
	// 将毫秒转换成秒
	qint64 totalSeconds = elapsedTime / 1000;
	// 计算小时数
	qint64 hours = totalSeconds / 3600;
	// 计算剩余的秒数（去除小时后的秒数）
	qint64 remainingSeconds = totalSeconds % 3600;
	// 获取完整的分钟数
	int minutes = static_cast<int>(remainingSeconds / 60);
	// 获取剩余的秒数
	int seconds = static_cast<int>(remainingSeconds % 60);
	uint64_t totalBlocks = 0;
	for (const auto& file : Files) {
		totalBlocks += file.TotalBlk;
	}
	if (totalBlocks / (1024*2)) {
		ALOG_INFO("Total File Size=%.2f MB", "CU", "--", (float(totalBlocks)) / (1024 * 2));
	}
	else {
		ALOG_INFO("Total File Size=%.2f KB", "CU", "--", (float(totalBlocks)) / 2);
	}

	ALOG_INFO("Parsing File Total Time : %d hour %d min %d seconds.", "CU", "--", hours, minutes, seconds);

	if (useAcMbnMode == 1) {
		AcMbn.CloseFile();
		ALOG_INFO("Save ACMBN Successfully : %s", "CU", "--", strAcMbnFile.c_str());
	}

__end:
	if (pBuf) {
		delete[] pBuf;
	}
	if (ret == E_OK) {
		ALOG_INFO("Parser Files SUCCESS.", "CU", "--");
		ALOG_INFO("%s", "CU", "--", CCreatFileInfoJson(Files, &ChkInfo).c_str());
	}
	else {
		ALOG_INFO("Parser Files FAILED.", "CU", "--");
	}
	dlgpro.closeProgressDialog();
	return ret;
}

void ACeMMCAnalyzeManager::SerialvFiletoJson(const QString& proj_path, std::vector<MMAKEFILE>& vecMAKEFile, QString strImgType, std::string& intelligentJson)
{
	int nVFileIndex = 1;
	int nBlockByte = 512;
	//将智能分析完的文件重新构成统一的Json，用于直接下发到eMMC中
	nlohmann::json eMMCHeaderPartitionJson;
	eMMCHeaderPartitionJson["PlatformType"] = strImgType.toStdString();

	//用户添加的xxx.bin文件
	nlohmann::json importFilesJson = nlohmann::json::array();

	//该文件智能分析后的文件Entry
	nlohmann::json vFilemapsJson = nlohmann::json::array();

	for (int i = 0; i < vecMAKEFile.size(); ++i) {
		int nVFileCnt = vecMAKEFile[i].VFileCnt;
		nlohmann::json _imFileJson;
		_imFileJson["Files"] = Utils::AngKCommonTools::Full2RelativePath(QString::fromStdString(vecMAKEFile[i].strFileName), proj_path);
		_imFileJson["LastModify"] = vecMAKEFile[i].lastModifyTime;
		_imFileJson["PartIndex"] = vecMAKEFile[i].PartIndex;
		_imFileJson["PartName"] = vecMAKEFile[i].strPartName;
		_imFileJson["FileSize"] = vecMAKEFile[i].Size;
		_imFileJson["EntryCnt"] = vecMAKEFile[i].VFileCnt;
		_imFileJson["VFileIndex"] = i + 1;
		_imFileJson["CheckSum"] = vecMAKEFile[i].CheckSum;
		_imFileJson["SectorAlign"] = vecMAKEFile[i].IsSectorAlign;
		_imFileJson["StartAddress"] = vecMAKEFile[i].StartAddr;

		eMMCVFILE* pVF_Head = vecMAKEFile[i].pVFileHead;
		uint64_t nChipBlockPos = vecMAKEFile[i].StartAddr / 512;
		uint64_t nFileBlcckPos = 0;
		bool bOk = false;
		while (pVF_Head) {
			nlohmann::json _vFileJson;
			QString entryName = "VFile[" + QString("%1").arg(nVFileIndex, 4, 10, QLatin1Char('0')) + "]";
			_vFileJson["Entry"] = entryName.toStdString();
			nFileBlcckPos = pVF_Head->dwBlkStart;//算下一段文件的起始位置
			_vFileJson["FileBlockPos"] = QString("0x%1").arg(nFileBlcckPos, 8, 16, QLatin1Char('0')).toStdString();
			_vFileJson["ChipBlockPos"] = QString("0x%1").arg(nChipBlockPos, 8, 16, QLatin1Char('0')).toStdString();
			nChipBlockPos = nChipBlockPos + (pVF_Head->dwBlkNum);//算前一段的chip位置
			_vFileJson["BlockNum"] = pVF_Head->dwBlkNum;
			_vFileJson["Feature"] = pVF_Head->Feature;
			_vFileJson["CRCType"] = 1;
			_vFileJson["CRC16"] = *(ushort*)pVF_Head->CRCValue;
			_vFileJson["VFileIndex"] = i + 1;
			_vFileJson["PartIndex"] = vecMAKEFile[i].PartIndex;
			_vFileJson["PartName"] = vecMAKEFile[i].strPartName;
			vFilemapsJson.push_back(_vFileJson);

			pVF_Head = pVF_Head->pNext;
			nVFileIndex++;
		}

		importFilesJson.push_back(_imFileJson);
	}

	eMMCHeaderPartitionJson["ImportFiles"] = importFilesJson;
	eMMCHeaderPartitionJson["VFilemap"] = vFilemapsJson;

	intelligentJson = eMMCHeaderPartitionJson.dump();
}

void ACeMMCAnalyzeManager::UnSerialvFiletoJson(const QString& proj_path, std::vector<tSeptBineMMCFileMap>& _binFileInfo, std::string& intelligentJson)
{
	if (intelligentJson.empty())
		return;
	
	try {//nlohmann解析失败会报异常需要捕获一下
		auto allFileInfoJson = nlohmann::json::parse(intelligentJson);
		for (int i = 0; i < allFileInfoJson["ImportFiles"].size(); ++i) {
			nlohmann::json importFileVec =  allFileInfoJson["ImportFiles"][i];
			tSeptBineMMCFileMap tfileMap;
			tfileMap.CheckSum = importFileVec["CheckSum"];
			tfileMap.EntryCnt = importFileVec["EntryCnt"];
			tfileMap.FileSize = importFileVec["FileSize"];
			tfileMap.strFileName = Utils::AngKCommonTools::Relative2FullPath(QString::fromStdString(importFileVec["Files"]), proj_path);
			tfileMap.PartIndex = importFileVec["PartIndex"];
			tfileMap.PartName = importFileVec["PartName"];
			tfileMap.bSector = importFileVec["SectorAlign"];
			tfileMap.ChipBlkOrgStart = importFileVec["StartAddress"];
			tfileMap.vFileIdx = importFileVec["VFileIndex"];
			tfileMap.lastModifyTime = importFileVec["LastModify"];
			_binFileInfo.push_back(tfileMap);
		}

		bool bOk;
		for (int j = 0; j < allFileInfoJson["VFilemap"].size(); ++j) {
			nlohmann::json vFilemapVec = allFileInfoJson["VFilemap"][j];
			teMMCVFile tVFile;
			tVFile.EntryIndex = j + 1;
			tVFile.EntryName = vFilemapVec["Entry"];
			tVFile.BlockNum = vFilemapVec["BlockNum"];
			tVFile.CRCValue = vFilemapVec["CRC16"];
			tVFile.CRCType = vFilemapVec["CRCType"];
			tVFile.ChipBlkPos = QString::fromStdString(vFilemapVec["ChipBlockPos"]).toLongLong(&bOk, 16);
			tVFile.FileBlkPos = QString::fromStdString(vFilemapVec["FileBlockPos"]).toLongLong(&bOk, 16);
			tVFile.vFileIdx = vFilemapVec["VFileIndex"];
			tVFile.Feature = vFilemapVec["Feature"];
			tVFile.PartIndex = vFilemapVec["PartIndex"];
			tVFile.PartName = vFilemapVec["PartName"];
			_binFileInfo[tVFile.vFileIdx - 1].vFiles.push_back(tVFile);
		}
	}
	catch (const nlohmann::json::exception& e) {
		ALOG_FATAL("Intelligent Json parse failed : %s.", "CU", "--", e.what());
	}
}

int ACeMMCAnalyzeManager::calc_crc16sum_checkvirgin(EMMCCRC* pCRC, uint8_t* buf, int size, uint8_t virgin, uint32_t* pIsVirgin)
{
	int i = 0;
	for (; i < size; i++) {
		if (buf[i] != virgin) {
			*pIsVirgin = 0;
			break;
		}
	}
	if (i == size)
		*pIsVirgin = 1;

	return calc_crc16sum(buf, size, (unsigned short*)pCRC->CRCValue);
}

bool ACeMMCAnalyzeManager::InitCrcModule(EMMCCRC* pMCrc)
{
	pMCrc->CRCType = CRCTYPE_CRC16;
	switch (pMCrc->CRCType) {
	case CRCTYPE_CRC16:
		pMCrc->CalcCRC = MMCCalcCRC16;
		return true;
		break;
	default:
		break;
	}
	return false;
}

eMMCVFILE* ACeMMCAnalyzeManager::MallocVFile()
{
	eMMCVFILE* pVFile = new eMMCVFILE;
	memset(pVFile, 0, sizeof(eMMCVFILE));
	return pVFile;
}

int ACeMMCAnalyzeManager::CalcVFileFeature(eMMCVFILE* pVFile, std::ifstream& clsFile, uint8_t byVirgin)
{
	int ret = E_OK;
	uint8_t* pBuf = new uint8_t[1024];
	unsigned long long ullOldPos = clsFile.tellg();
	unsigned long long VFileStart;
	if (pBuf) {
		VFileStart = (unsigned long long)pVFile->dwBlkStart << 9;
		clsFile.seekg(VFileStart, std::ios::beg);
		memset(pBuf, byVirgin, 1024);
		clsFile.read((char*)pBuf, 1024);
		calc_crc16sum(pBuf, 1024, (unsigned short*)&pVFile->Feature);
		clsFile.seekg(ullOldPos);
		if (pBuf)
			delete[] pBuf;
	}
	else {
		ret = E_FIALED;
	}
	return ret;
}

bool ACeMMCAnalyzeManager::PrintFindFile(eMMCVFILE* pVFile)
{
	char MsgSend[256];
	char CrcV[64];
	memset(MsgSend, 0, 256);
	memset(CrcV, 0, 64);
	//sprintf(MsgSend,"Find a file: StartBlk=0x%08x, BlkNum=%d, ",pVFile->dwBlkStart,pVFile->dwBlkNum);
	if (pVFile->dwBlkNum / (1024 * 2)) {
		sprintf(MsgSend, "Find a file: StartBlk=0x%08X, BlkNum=0x%08X, Size=%.2f MB ", pVFile->dwBlkStart, pVFile->dwBlkNum, (float(pVFile->dwBlkNum)) / (1024 * 2));
	}
	else {
		if (pVFile->dwBlkNum % 2 == 0)
			sprintf(MsgSend, "Find a file: StartBlk=0x%08X, BlkNum=0x%08X, Size=%d KB ", pVFile->dwBlkStart, pVFile->dwBlkNum, (pVFile->dwBlkNum) / 2);
		else
			sprintf(MsgSend, "Find a file: StartBlk=0x%08X, BlkNum=0x%08X, Size=%d.5 KB ", pVFile->dwBlkStart, pVFile->dwBlkNum, (pVFile->dwBlkNum) / 2);
	}
#if 1
	switch (pVFile->CRCType) {
	case CRCTYPE_CRC16:
		sprintf(CrcV, "VFeature=0x%04X, CRC16=0x%04X", pVFile->Feature, *(unsigned short*)pVFile->CRCValue);
		break;
	default:
		return false;
	}
	strcat(MsgSend, CrcV);
#endif
	return true;
}

int ACeMMCAnalyzeManager::AttachVFile(MMAKEFILE& MFile, eMMCVFILE* pNew)
{
	eMMCVFILE* pTmp = MFile.pVFileHead;
	MFile.VFileCnt++;
	if (pTmp == NULL) {
		MFile.pVFileHead = pNew;
	}
	else {
		while (pTmp->pNext) {
			pTmp = pTmp->pNext;
		}
		pTmp->pNext = pNew;
	}
	return E_OK;
}

int ACeMMCAnalyzeManager::SetProgress(unsigned long long Offset, unsigned long long FileLen)
{
	return ((float)Offset / FileLen) * 100;
}

std::string ACeMMCAnalyzeManager::GetSumType(int Type)
{
	std::string strType;
	switch (Type) {
	case 0:
		strType = "CRC32";
		break;
	case 1:
		strType = "Byte";
		break;
	case 2:
		strType = "MD5";
		break;
	default:
		strType = "CRC32";
		break;
	}
	return strType;
}

bool ACeMMCAnalyzeManager::PrintTotal(MMAKEFILE& MFile, int FileIndex, CHKINFO* pChkInfo, int chkSumType)
{
	char MsgSend[256];
	eMMCVFILE* pTmpV = MFile.pVFileHead;
	uint32_t TotalBlkNum = 0;
	int32_t Cnt = 0;
	while (pTmpV) {
		TotalBlkNum += pTmpV->dwBlkNum;
		pTmpV = pTmpV->pNext;
		Cnt++;
	}
	if (Cnt != MFile.VFileCnt)
		return false;
	MFile.TotalBlk = TotalBlkNum;
	if (TotalBlkNum / (1024 * 2)) {
		sprintf(MsgSend, "File(#%d) vfiles count=%d, Size=%.2f MB", FileIndex + 1, Cnt, (float(TotalBlkNum)) / (1024 * 2));
	}
	else {
		sprintf(MsgSend, "File(#%d) vfiles count=%d, Size=%.1f KB", FileIndex + 1, Cnt, (float(TotalBlkNum)) / 2);
	}
	ALOG_INFO("%s", "CU", "--", MsgSend);
	if (GetSumType(chkSumType) == "CRC32") {
		if (pChkInfo) {
			std::string TmpData;
			char CrcValue[100];
			memset(CrcValue, 0, 100);
			tMakeCRC32Info* pCRC32Info = (tMakeCRC32Info*)pChkInfo->PrivData;
			sprintf(CrcValue, "0x%08X", (unsigned int)pCRC32Info->CRC32ChkInfo.chksum);
			sprintf(MsgSend, "File Checksum=%s -%s", CrcValue, GetSumType(chkSumType).c_str());
			ALOG_INFO("%s", "CU", "--", MsgSend);
			sprintf(MsgSend, "File Checksum=0x%llX -%s", MFile.CheckSum, GetSumType(1).c_str());
			ALOG_INFO("%s", "CU", "--", MsgSend);
			MFile.CheckSum = (unsigned int)pCRC32Info->CRC32ChkInfo.chksum;
		}
	}
	else {
		sprintf(MsgSend, "File Checksum=0x%llX -%s", MFile.CheckSum, GetSumType(chkSumType).c_str());
		ALOG_INFO("%s", "CU", "--", MsgSend);
	}
	return true;
}

std::string ACeMMCAnalyzeManager::CCreatFileInfoJson(std::vector<MMAKEFILE>& Files, CHKINFO* pChkInfo)
{
	nlohmann::json root;
	nlohmann::json DataArray;
	nlohmann::json JsonData;
	std::string strFileInfoJson;
	std::string Type, Value;
	std::string TmpData;

	if (Files.size() > 0) {
		root["fileCnt"] = Files.size();
		for (unsigned int i = 0; i < Files.size(); ++i) {

			Type = "Byte";
			char tmpbuf[100];
			memset(tmpbuf, 0, 100);
			sprintf(tmpbuf, "0x%llX", Files[i].CheckSum);
			Value = tmpbuf;

			JsonData["FilePath"] = Files[i].strFileName;
			JsonData["CheckSumType"] = Type;
			JsonData["CheckSum"] = Value;
			DataArray.push_back(JsonData);
		}
		root["fileList"] = DataArray;
	}

	strFileInfoJson = root.dump(4);
	return strFileInfoJson;
}
