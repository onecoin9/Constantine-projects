#include "ACProjManager.h"
#include "ACChipManager.h"
#include "ACEventLogger.h"
#include "AngKProjFile.h"
#include "AngkLogger.h"
#include "AngKCommonTools.h"
#include "AngKGlobalInstance.h"
#include "AngkLogger.h"
#include "ACMessageBox.h"
#include "ACDeviceManager.h"
#include "AngKPathResolve.h"
#include "AngKMessageHandler.h"
#include "MessageNotify/notifymanager.h"
#include "CustomMessageHandler.h"
#include "ProgressDialogSingleton.h"
#include "ChkSum.h"
#include "json.hpp"
#include "Tag_Proj.h"
#include "eMMCCom.h"
#include "CRC/Crc32_Comm.h"
#include "GlobalDefine.h"
#include <QDir>
#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>
#include <QUuid>
#include <QHostInfo>

#define VERSION_1 (0x01)
#define VERSION_2 (0x02)
#define VERSION_3 (0x03)

int EMMC_COMMON_BOOT_SIZE = 128;//单位kb
int EMMC_COMMON_BLOCK_SIZE = 512;
int EMMC_SSD_MULTIPLE = 4096;

extern UserMode curUserMode;

int calc_crc16sum(unsigned char* buf, unsigned int size, unsigned short* pCRC16Sum);

ACProjManager::ACProjManager(QObject *parent, AngKDataBuffer* _pDataBuf)
	: QObject(parent)
	, m_pProjDataset(std::make_unique<AngKProjDataset>(this))
	, m_pDataBuffer(std::make_unique<AngKDataBuffer>(this))
	, m_pProjFile(std::make_unique<AngKProjFile>(this))
	, m_pAnalyzeMgr(std::make_unique<ACeMMCAnalyzeManager>(this))
{
	m_mapReadyCheckFile.clear();
}

ACProjManager::~ACProjManager()
{
}

void ACProjManager::InitManager()
{
	//初始化芯片保存，第一次创建一定是先选择了芯片
	//ACChipManager chipMgr;
	//ChipDataJsonSerial chipJson;
	//chipJson.serialize(chipMgr.GetChipData());
	ChipDataJsonSerial chipJson = m_pProjDataset->getChipData();

	std::string deviceDrvFilePath;
	std::string masterDrvFilePath;
	QStringList deviceDrvList;
	QStringList masterDrvList;
	Utils::AngKCommonTools::GetDeviceDrvPath(chipJson.getJsonValue<std::string>("chipAlgoFile"), deviceDrvFilePath);
	Utils::AngKCommonTools::GetMasterDrvPath(chipJson.getJsonValue<std::string>("chipMstkoFile"), masterDrvFilePath);

	Utils::AngKCommonTools::DepressBinFile(deviceDrvFilePath, deviceDrvList);
	Utils::AngKCommonTools::DepressBinFile(masterDrvFilePath, masterDrvList);

	m_pDataBuffer->SetChipConfig(chipJson.getJsonValue<ulong>("chipDrvParam"), masterDrvList, deviceDrvList, uchar());

	ALOG_INFO("Initialize buffer.", "CU", "--");
	m_pDataBuffer->InitBuffer(chipJson.getJsonValue<ulong>("chipBufferSize"), 0);
	ALOG_INFO("Initialize buffer complete.", "CU", "--");
}

int ACProjManager::SaveProject(QString& _projPath, QString& _projPwd, QString& _chkFile, int _nClearBufferType, std::string& _ImageFileFormat
	, std::string& _eMMCOption, std::string& _eMMCExtCSD, std::string& _eMMCSSDHeaderData, eMMCTableHeader& _eMMCSSDTableHeader
	, std::string& _drvCommon, std::string& _drvSelf, std::string& _chipOperate, std::string& _chipBufferChk, std::string& _description)
{
	int ret = -1;
	//AngKProjFile pFile(this, 0);
	ret = m_pProjFile->CreateProjFile(_projPath, MAX_TAGCOUNT, 0, 0, "Proj", "1000", _projPwd);
	if (ret != 0) {
		return ret;
	}
	std::string jsonStr;

	{//--多工程加载需要给每个工程一个UUID
		QUuid guid = QUuid::createUuid();
		ret = m_pProjFile->AddTag(TAG_FILEUUID, (uchar*)guid.toString().toStdString().c_str(), guid.toString().size());
	}

	//--TAG_PROJVERSION工程创建的版本信息
	{
		QString strBuildVersion = AngKGlobalInstance::instance()->ReadValue("Version", "BuildVer").toString();
		ret = m_pProjFile->AddTag(TAG_PROJVERSION, (uchar*)strBuildVersion.toStdString().c_str(), strBuildVersion.size());
	}

	{//---芯片配置TAG设置
		jsonStr = m_pProjDataset->getChipData().json2String();
		m_pProjFile->encodeBase64_Json(jsonStr);//	进行base64加密 防止从工程文件看出
		ret = m_pProjFile->AddTag(TAG_CHIPDATA, (uchar*)jsonStr.c_str(), jsonStr.size());
	}

	{//--烧录档案计算整体Bytesum
		jsonStr = _chkFile.toStdString();
		ret = m_pProjFile->AddTag(TAG_CHECKSUMFILE, (uchar*)jsonStr.c_str(), jsonStr.size());
	}

	//FPGA\ALGO\APP 三个外置文件需要直接加入工程proj中
	//驱动文件分为MasterDriver和DeviceDriver分别保存

	{//--TAG_PROJALLFILE 新加标签，将相关bin档压缩包直接写入工程，这里可以取代FPGA\ALGO\APP相关标签
		nlohmann::json chipJson = m_pProjDataset->getChipData().DataJsonSerial();
		if (!chipJson.is_null()) {
			//添加DeviceDriver
			std::string chipName = chipJson["chipAlgoFile"];
			chipName = chipName.substr(0, chipName.find_first_of("."));
			m_pProjFile->AddBinFileTag(TAG_CHIPALGOFILE, Utils::AngKPathResolve::localDeviceDrvFilePath(QString::fromStdString(chipName)));

			//添加MasterDriver
			std::string chipMasterName = chipJson["chipMstkoFile"];
			chipMasterName = chipMasterName.substr(0, chipMasterName.find_first_of("."));
			m_pProjFile->AddBinFileTag(TAG_CHIPMASTERALGOFILE, Utils::AngKPathResolve::localMasterDrvFilePath(QString::fromStdString(chipMasterName)));

			QStringList deviceDrvList, masterDrvList;
			QString deviceDrvFilePath = Utils::AngKPathResolve::localDeviceDrvFilePath(QString::fromStdString(chipName));
			QString masterDrvFilePath = Utils::AngKPathResolve::localMasterDrvFilePath(QString::fromStdString(chipMasterName));
			Utils::AngKCommonTools::DepressBinFile(deviceDrvFilePath.toStdString(), deviceDrvList);
			QString devVersion = Utils::AngKCommonTools::GetDesFileValue(Utils::AngKPathResolve::localTempFolderPath() + "Version.txt", "Version");
			Utils::AngKCommonTools::DepressBinFile(masterDrvFilePath.toStdString(), masterDrvList);
			QString mstVersion = Utils::AngKCommonTools::GetDesFileValue(Utils::AngKPathResolve::localTempFolderPath() + "Version.txt", "Version");
			ALOG_INFO("Save project dev driver version:%s", "CU", "--", devVersion.toStdString().c_str());
			ALOG_INFO("Save project mst driver version:%s", "CU", "--", mstVersion.toStdString().c_str());

			//添加FPGAFile
			//TODO 暂时没有对应版本，先不添加到工程中
			//std::string chipFPGAName = chipJson["chipFPGAFile"];
			//chipFPGAName = chipFPGAName.substr(0, chipFPGAName.find_first_of("."));
			//pFile.AddBinFileTag(TAG_CHIPFPGAFILE, Utils::AngKPathResolve::localDriverFilePath(QString::fromStdString(chipFPGAName)));
		}
	}

	{//TAG_CHIPSPBIT
		//TODO,当前emmc不支持
	}

	{//烧录档案加载配置TAG设置
		std::vector<FileDataJsonSerial> fileSerial = m_pProjDataset->getFileData();
		if (!fileSerial.empty()) {//通用芯片类型如果没有档案，可以继续判断emmc。两个页面是互斥的，只能有一个show
			FileImportDataJsonSerial fileImportSerial;
			fileImportSerial.serialize(fileSerial, VERSION_3, _nClearBufferType);

			jsonStr = fileImportSerial.json2String();
			m_pProjFile->encodeBase64_Json(jsonStr);
			ret = m_pProjFile->AddTag(TAG_FILEIMPORT, (uchar*)jsonStr.c_str(), jsonStr.size());
		}
	}

	{//烧录档案加载eMMC档案Tag
		std::vector<eMMCFileDataJsonSerial> eMMCFileSerial = m_pProjDataset->geteMMCFileData();
		if (!eMMCFileSerial.empty()) {
			eMMCFileImportDataJsonSerial eFileImportSerial;
			eFileImportSerial.serialize(eMMCFileSerial, _ImageFileFormat, _projPath);
			jsonStr = eFileImportSerial.json2String();
			ret = m_pProjFile->AddTag(TAG_EMMCFILEIMPORT, (uchar*)jsonStr.c_str(), jsonStr.size());
		}
	}

	{//eMMC配置信息保存
		if (!_eMMCOption.empty()) {
			ret = m_pProjFile->AddTag(TAG_EMMCOPTION, (uchar*)_eMMCOption.c_str(), _eMMCOption.size());
		}
	}

	{//eMMC寄存器ExtCSD信息保存
		if (!_eMMCExtCSD.empty()) {
			ret = m_pProjFile->AddTag(TAG_EMMCIGNOREEXTREG, (uchar*)_eMMCExtCSD.c_str(), _eMMCExtCSD.size());
		}
	}

	{//eMMC智能分析信息保存
		QString strIntelligentJson = m_pProjDataset->GeteMMCIntelligentJson();
		if (!strIntelligentJson.isEmpty()) {
			ret = m_pProjFile->AddTag(TAG_EMMCINTELLIGENT, (uchar*)strIntelligentJson.toStdString().c_str(), strIntelligentJson.toStdString().size());
		}
	}

	{//eMMC的TableHeader保存
		ret = m_pProjFile->AddTag(TAG_EMMCHEADER, (uchar*)(&_eMMCSSDTableHeader), sizeof(_eMMCSSDTableHeader));
	}

	{//eMMC的TableHeaderJson保存
		if (!_eMMCSSDHeaderData.empty()) {
			ret = m_pProjFile->AddTag(TAG_EMMCHEADERTABLE, (uchar*)_eMMCSSDHeaderData.c_str(), _eMMCSSDHeaderData.size());
		}
	}

	{//通用驱动参数保存TAG
		if (!_drvCommon.empty()) {
			ret = m_pProjFile->AddTag(TAG_DrvCommonPara, (uchar*)_drvCommon.c_str(), _drvCommon.size());
		}
	}

	{//自定义驱动参数保存TAG
		if (!_drvSelf.empty()) {
			ret = m_pProjFile->AddTag(TAG_DrvSelfPara, (uchar*)_drvSelf.c_str(), _drvSelf.size());
		}
	}

	{// 烧录执行操作命令保存TAG
		ret = m_pProjFile->AddTag(TAG_CHIPOPERATE, (uchar*)_chipOperate.c_str(), _chipOperate.size());
	}

	{// 保存Project _description
		if (!_description.empty())
			ret = m_pProjFile->AddTag(TAG_ProDescriptionPara, (uchar*)_description.c_str(), _description.size());
	}
	{// 烧录执行buffer检查命令保存TAG
		ret = m_pProjFile->AddTag(TAG_CHIPOPCFG, (uchar*)_chipBufferChk.c_str(), _chipBufferChk.size());
	}

	{//--bufMap加载到工程中 TAG_MULTIBUFMAP
		pugi::xml_document doc;
		pugi::xml_node* bufferMap_node = Utils::AngKCommonXMLParser::findNode(doc, XML_NODE_CHIPDATA_BUFFERMAPSET);
		if (bufferMap_node != nullptr && !bufferMap_node->empty())
		{
			//获取DataRemap节点的值
			nlohmann::json cmdJson;

			ulong uAlgo = m_pProjDataset->getChipData().getJsonValue<ulong>("chipDrvParam");
			Utils::AngKCommonXMLParser::BufferMapInfo_Json(*bufferMap_node, uAlgo, cmdJson);

			ret = m_pProjFile->AddTag(TAG_MULTIBUFMAP, (uchar*)cmdJson.dump().c_str(), cmdJson.dump().size());
		}
	}

	{//--databufReMap加载到工程中 TAG_DATAREMAP
		pugi::xml_document doc;
		pugi::xml_node* remapData_node = Utils::AngKCommonXMLParser::findNode(doc, XML_NODE_CHIPDATA_DATAREMAPSET);
		if (remapData_node != nullptr && !remapData_node->empty())
		{
			//获取DataRemap节点的值
			nlohmann::json cmdJson;

			ulong uAlgo = m_pProjDataset->getChipData().getJsonValue<ulong>("chipDrvParam");
			Utils::AngKCommonXMLParser::DataRemapInfo_Json(*remapData_node, uAlgo, cmdJson);

			ret = m_pProjFile->AddTag(TAG_DATAREMAP, (uchar*)cmdJson.dump().c_str(), cmdJson.dump().size());
		}
	}

	{//--pinMap加载到工程中 TAG_PINMAP
		pugi::xml_document doc;
		pugi::xml_node* pinMap_node = Utils::AngKCommonXMLParser::findNode(doc, XML_NODE_CHIPDATA_PINMAPSET);
		if (pinMap_node != nullptr && !pinMap_node->empty())
		{
			//获取PINMAP节点的值
			nlohmann::json cmdJson;

			ulong uAlgo = m_pProjDataset->getChipData().getJsonValue<ulong>("chipDrvParam");
			Utils::AngKCommonXMLParser::DriverPinMap_Json(*pinMap_node, uAlgo, cmdJson);

			ret = m_pProjFile->AddTag(TAG_PINMAP, (uchar*)cmdJson.dump().c_str(), cmdJson.dump().size());
		}
	}

	{//--block加载到工程中 TAG_BLOCK
		pugi::xml_document doc;
		pugi::xml_node* block_node = Utils::AngKCommonXMLParser::findNode(doc, XML_NODE_CHIPDATA_BLOCKSET);
		if (block_node != nullptr && !block_node->empty())
		{
			//获取BLOCK节点的值
			nlohmann::json cmdJson;

			ulong uAlgo = m_pProjDataset->getChipData().getJsonValue<ulong>("chipDrvParam");
			Utils::AngKCommonXMLParser::PartitionBlock_Json(*block_node, uAlgo, cmdJson);

			ret = m_pProjFile->AddTag(TAG_BLOCK, (uchar*)cmdJson.dump().c_str(), cmdJson.dump().size());
		}
	}

	{//--block加载到工程中 TAG_DRVINFO
		pugi::xml_document doc;
		pugi::xml_node* drvfeature_node = Utils::AngKCommonXMLParser::findNode(doc, XML_NODE_CHIPDATA_DRIVERFEATURESET);
		if (drvfeature_node != nullptr && !drvfeature_node->empty())
		{
			//获取DRIVERFEATURE节点的值
			nlohmann::json cmdJson;

			ulong uAlgo = m_pProjDataset->getChipData().getJsonValue<ulong>("chipDrvParam");
			Utils::AngKCommonXMLParser::DriverFeature_Json(*drvfeature_node, uAlgo, cmdJson);
			ALOG_INFO("Save project driver info: %s", "CU", "--", cmdJson.dump().c_str());
			ret = m_pProjFile->AddTag(TAG_DRVINFO, (uchar*)cmdJson.dump().c_str(), cmdJson.dump().size());
		}
	}

	{//--bufMap数据加载到工程中 TAG_CHIPBUFF
		uchar byBuf[BUFFER_RW_SIZE];
		ADR  adrOff = 0;
		uint uiReadLen;
		ADR BufTotalSize = m_pDataBuffer->GetSize();
		uint64_t BytesRead;
		uint64_t BufCondenseSize = 0;
		CHKINFO DataBuffChecksum;
		memset(&DataBuffChecksum, 0, sizeof(DataBuffChecksum));
		BufCondenseSize = m_pDataBuffer->GetCondenseSize();
		ret = m_pProjFile->AddTagHeader(TAG_CHIPBUFF, BufCondenseSize, 0);	//特别要注意，工程文件实际存在的是Buffer的大小之和，其他不可见区域不能算入

		int partCnt = m_pDataBuffer->GetPartitionCount();
		for (int partIdx = 0; partIdx < partCnt; ++partIdx)
		{
			const tPartitionInfo* partInfo;
			partInfo = m_pDataBuffer->GetPartitionInfo(partIdx);

			if (!partInfo->PartitionShow)
				continue;

			int bufCnt = m_pDataBuffer->GetBufferCount(partIdx);
			for (int bufIdx = 0; bufIdx < bufCnt; ++bufIdx) {
				const tBufInfo* BufInfo;
				BufInfo = m_pDataBuffer->GetBufferInfo(bufIdx, partIdx);

				if (!BufInfo->m_bBufferShow)
					continue;

				adrOff = BufInfo->llBufStart;
				while (adrOff <= BufInfo->llBufEnd) {//压缩存储
					BytesRead = (BufInfo->llBufEnd + 1 - adrOff) > BUFFER_RW_SIZE ? BUFFER_RW_SIZE : (BufInfo->llBufEnd + 1 - adrOff);
					uiReadLen = m_pDataBuffer->BufferRead(adrOff, byBuf, BytesRead);
					if (uiReadLen <= 0)
						break;

					//#ifdef SECURITY_ENABLE
					//					Security.EncryptData(adrOff, byBuf, uiReadLen);///在这个地方先进行加密处理
					//#endif
					ret = m_pProjFile->WriteTagBuf(byBuf, uiReadLen);

					if (ret != E_FMTFILE_OK) {
						ALOG_FATAL("Create project WriteTagBuf:TAG_CHIPBUFF failed.", "CU", "--");
						return ret;
					}

					adrOff += uiReadLen;

					//计算数据校验值 Crc32
					Crc32CalcSubRoutine(&DataBuffChecksum, byBuf, uiReadLen);
				}
			}
		}
		Crc32GetChkSum(&DataBuffChecksum);
		ret = m_pProjFile->UpdateExistTag(TAG_CHIPBUFF, BufCondenseSize, 0, NULL, (uint)DataBuffChecksum.chksum);//更新烧录数据的校验值
	}

	//std::string buf_String;
	//int retred = pFile.GetTagData(TAG_FILEIMPORT, buf_String);
	//pFile.dencodeBase64_Json(buf_String);

	m_pProjFile->CloseFile();

	nlohmann::json SaveProjectJson;
	SaveProjectJson["FilePath"] = _projPath.toStdString();
	SaveProjectJson["RetCode"] = ret;
	SaveProjectJson["RetInfo"] = ret != 0 ? "Save Project Failed" : "Save Project Success";
	EventLogger->SendEvent(EventBuilder->GetSaveProject(SaveProjectJson));

	return ret;
}

int ACProjManager::ImportProject(QString _projPath)
{
	int ret = 0;
	if (E_FMTFILE_ERRER == m_pProjFile->LoadFile(_projPath)) {
		ALOG_FATAL("LoadProjectFile error, strFile Name: %s.", "CU", "--", _projPath.toStdString().c_str());
		return E_FMTFILE_ERRER;
	}

	if (m_pProjFile->HasPasswd()) {
		bool ok;
		QString text = QInputDialog::getText(qobject_cast<QWidget*>(this), QObject::tr("Input Password"), QObject::tr("Please Input Password:"), QLineEdit::Normal, "", &ok);

		// 如果用户点击了OK按钮，则显示用户输入的内容
		if (ok) {
			if (!m_pProjFile->VerifyPasswd(text.toStdString().c_str())) {
				//ACMessageBox::showError(qobject_cast<QWidget*>(this), QObject::tr("Message"), QObject::tr("Verify password error!"));
				return E_PWD_ERROR;
			}
		}
		else {
			return E_PWD_CANCEL;
		}
	}

	stuProjProperty& sProjPro = m_pProjDataset->GetProjProperty();
	sProjPro.Clear();

	{
		std::string fileUUID;
		if (E_FMTFILE_OK == m_pProjFile->GetTagData(TAG_FILEUUID, fileUUID)) {
			if (sProjPro.projUUID == fileUUID) {
				ALOG_WARN("TAG_FILEUUID duplicate, project loaded.", "CU", "--");
				return E_TAG_EXIST;
			}

			QFileInfo fileInfo(_projPath);
			sProjPro.name = _projPath.toStdString();
			sProjPro.projUUID = fileUUID;
			m_sProjUUID = QString::fromStdString(fileUUID);
			m_sProjName = _projPath;
		}
		else {
			ALOG_FATAL("Load Project File error, Get TAG_FILEUUID failed.", "CU", "--");
			return E_FMTFILE_ERRER;
		}
	}

	//--TAG_PROJVERSION工程创建的版本信息
	{
		std::string strBuildVersion;
		if (E_FMTFILE_OK == m_pProjFile->GetTagData(TAG_PROJVERSION, strBuildVersion)) {
			sProjPro.projVersion = strBuildVersion;
		}
	}

	PrintOpenProjLog(_projPath);

	//加载TAG_CHIPDATA，芯片数据。
	{
		std::string chipInfo;
		if (E_FMTFILE_OK == m_pProjFile->GetTagData(TAG_CHIPDATA, chipInfo)) {
			m_pProjFile->dencodeBase64_Json(chipInfo);//base64需要解码

			try {//nlohmann解析失败会报异常需要捕获一下
				auto j3 = nlohmann::json::parse(chipInfo);
				ChipDataJsonSerial chipDataSer;
				chipDataSer.copyJson(j3);
				m_pProjDataset->setChipData(chipDataSer);
				//ui->propetryWidget->setProjDataset(m_projDataset);

				//根据芯片信息进行buffer创建
				std::string chipName = j3["chipAlgoFile"];
				chipName = chipName.substr(0, chipName.find_first_of("."));
				std::string driverFile = Utils::AngKPathResolve::localDeviceDrvFilePath(QString::fromStdString(chipName)).toStdString();

				std::string deviceDrvFilePath;
				std::string masterDrvFilePath;
				QStringList deviceDrvList;
				QStringList masterDrvList;
				if (curUserMode == UserMode::Developer) {
					// 开发者模式，驱动工程从本地文件夹获取
					Utils::AngKCommonTools::GetDeviceDrvPath(j3["chipAlgoFile"].get<std::string>(), deviceDrvFilePath);
					Utils::AngKCommonTools::GetMasterDrvPath(j3["chipMstkoFile"].get<std::string>(), masterDrvFilePath);
				}
				else {
					// 非开发者模式，驱动工程从工程文件获取
					std::string devDriveStr;
					std::string mstDriveStr;
					if (E_FMTFILE_OK == m_pProjFile->GetTagData(TAG_CHIPALGOFILE, devDriveStr) && E_FMTFILE_OK == m_pProjFile->GetTagData(TAG_CHIPMASTERALGOFILE, mstDriveStr)) {
						QFile devDriverFile(Utils::AngKPathResolve::localTempFolderPath() + j3["chipAlgoFile"].get<std::string>().c_str());
						QFile mstDriverFile(Utils::AngKPathResolve::localTempFolderPath() + j3["chipMstkoFile"].get<std::string>().c_str());
						if (!devDriverFile.open(QIODevice::WriteOnly | QIODevice::Truncate) || !mstDriverFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
							ALOG_FATAL("Get tag drv file failed.", "CU", "--");
							return E_FMTFILE_ERRER;
						}

						devDriverFile.write(devDriveStr.c_str(), devDriveStr.size());
						mstDriverFile.write(mstDriveStr.c_str(), mstDriveStr.size());

						devDriverFile.flush();
						mstDriverFile.flush();

						deviceDrvFilePath = Utils::AngKPathResolve::localTempFolderPath().toStdString() + j3["chipAlgoFile"].get<std::string>();
						masterDrvFilePath = Utils::AngKPathResolve::localTempFolderPath().toStdString() + j3["chipMstkoFile"].get<std::string>();
					}

				}

				Utils::AngKCommonTools::DepressBinFile(deviceDrvFilePath, deviceDrvList);
				QString devVersion = Utils::AngKCommonTools::GetDesFileValue(Utils::AngKPathResolve::localTempFolderPath() + "Version.txt", "Version");
				Utils::AngKCommonTools::DepressBinFile(masterDrvFilePath, masterDrvList);
				QString mstVersion = Utils::AngKCommonTools::GetDesFileValue(Utils::AngKPathResolve::localTempFolderPath() + "Version.txt", "Version");
				ALOG_INFO("Import dev driver version:%s", "CU", "--", devVersion.toStdString().c_str());
				ALOG_INFO("Import mst driver version:%s", "CU", "--", mstVersion.toStdString().c_str());

				sProjPro.devDriverVer = devVersion.toStdString();
				sProjPro.mstDriverVer = mstVersion.toStdString();


				m_pDataBuffer->SetChipConfig(j3["chipDrvParam"], masterDrvList, deviceDrvList, uchar());

				if (m_pDataBuffer->InitBuffer(j3["chipBufferSize"], 0) != 0) {
					ALOG_FATAL("chipName : [%s] Initialize buffer failed.", "CU", "--", chipName.c_str());
					return E_FMTFILE_ERRER;
				}

				sProjPro.chipName = j3["chipName"];
				sProjPro.adapterName = j3["chipAdapter"];
				sProjPro.typeName = j3["chipType"];
				sProjPro.manufacturerName = j3["manufacture"];
				sProjPro.packageName = j3["chipPackage"];

				ALOG_INFO("Select Device : %s %s %s", "CU", "--", sProjPro.manufacturerName.c_str(), sProjPro.chipName.c_str(), sProjPro.adapterName.c_str());
			}
			catch (const nlohmann::json::exception& e) {
				ALOG_FATAL("TAG_CHIPDATA Json parse failed : %s.", "CU", "--", e.what());
			}
		}
	}

	//加载TAG_FILEIMPORT，档案数据
	{
		std::string fileInfo;
		if (E_FMTFILE_OK == m_pProjFile->GetTagData(TAG_FILEIMPORT, fileInfo)) {
			m_pProjFile->dencodeBase64_Json(fileInfo);//base64需要解码
			std::vector<FileDataJsonSerial> fDataVec;
			try {//nlohmann解析失败会报异常需要捕获一下
				auto j3 = nlohmann::json::parse(fileInfo);
				for (int i = 0; i < j3["fileNum"]; ++i)
				{
					FileDataJsonSerial fDataSerial;
					fDataSerial.copyJson(j3["fileData"][i]);
					if (!fDataSerial.getJsonValue<std::string>("fileName").empty()) {
						fDataVec.push_back(fDataSerial);
						sProjPro.binFile.push_back(fDataSerial.getJsonValue<std::string>("fileName"));
					}
				}

				FileImportDataJsonSerial fImportDataSerial;
				fImportDataSerial.serialize(fDataVec, j3["version"], j3["clearBufferID"]);
				//clearBufferID 如何使用需考虑
				m_pProjDataset->setFileData(fDataVec);
			}
			catch (const nlohmann::json::exception& e) {
				ALOG_FATAL("TAG_FILEIMPORT Json parse failed : %s.", "CU", "--", e.what());
			}
		}
	}

	int chkTypeIdx = 0;
	QMap<QString, uint64_t> fileChkMap;

	//加载TAG_EMMCOPTION，获取eMMCExtendCSD记录
	{
		std::string emmOptionInfo;
		if (E_FMTFILE_OK == m_pProjFile->GetTagData(TAG_EMMCOPTION, emmOptionInfo)) {
			m_pProjDataset->SeteMMCOptionJson(QString::fromStdString(emmOptionInfo));
			try {
				auto j3 = nlohmann::json::parse(emmOptionInfo);
				chkTypeIdx = j3["imageFileChecksumType"];
			}
			catch (const nlohmann::json::exception& e) {
				ALOG_FATAL("TAG_EMMCOPTION Json parse failed : %s.", "CU", "--", e.what());
			}
		}
	}


	//加载TAG_EMMCFILEIMPORT，档案数据
	{
		std::string emmcFileInfo;
		if (E_FMTFILE_OK == m_pProjFile->GetTagData(TAG_EMMCFILEIMPORT, emmcFileInfo)) {
			try {//nlohmann解析失败会报异常需要捕获一下
				auto j3 = nlohmann::json::parse(emmcFileInfo);

				eMMCFileImportDataJsonSerial femmcImportDataSerial;
				femmcImportDataSerial.copyJson(j3);

				std::string img_file_fmt;
				std::vector<eMMCFileDataJsonSerial> femmcDataVec;
				femmcImportDataSerial.deserialize(femmcDataVec, img_file_fmt, _projPath);

				ALOG_INFO("------------------------------eMMC File------------------------------", "CU", "--");
				for (std::size_t i = 0; i < femmcDataVec.size(); ++i)
				{
					std::string emmcFilePath = femmcDataVec[i].getJsonValue<std::string>("fileFullPath");
					sProjPro.binFile.push_back(emmcFilePath);
					ALOG_INFO("emmcFile[%d] : %s", "CU", "--", i, emmcFilePath.c_str());
					fileChkMap.insert(femmcDataVec[i].getJsonValue<std::string>("fileFullPath").c_str(), femmcDataVec[i].getJsonValue<uint64_t>("checkSum"));
					m_mapReadyCheckFile[chkTypeIdx].insert(femmcDataVec[i].getJsonValue<std::string>("fileFullPath").c_str(), femmcDataVec[i].getJsonValue<uint64_t>("checkSum"));
				}
				m_pProjDataset->geteMMCFileData().clear();
				m_pProjDataset->geteMMCFileImportData();
				m_pProjDataset->seteMMCFileData(femmcDataVec);
				m_pProjDataset->seteMMCFileImportData(femmcImportDataSerial);

			}
			catch (const nlohmann::json::exception& e) {
				ALOG_FATAL("TAG_FILEIMPORT Json parse failed : %s.", "CU", "--", e.what());
			}
		}
	}

	{//--烧录档案计算整体Bytesum
		std::string binFileChk;
		if (E_FMTFILE_OK == m_pProjFile->GetTagData(TAG_CHECKSUMFILE, binFileChk)) {
			std::transform(binFileChk.begin(), binFileChk.end(), binFileChk.begin(), std::toupper);
			sProjPro.checksum = binFileChk;
		}
		ALOG_INFO("Total Data Bytesum: %s", "CU", "--", binFileChk.c_str());
	}

	// chk校验
	//{
	//	for (auto iter = fileChkMap.begin(); iter != fileChkMap.end(); iter++) {
	//		QFile file(iter.key());
	//		if (!file.open(QIODevice::ReadOnly)) {
	//			ALOG_FATAL("Load Project File error, open image file:\"%s\" failed.", "CU", "--", iter.key().toLocal8Bit().data());
	//			return E_TAG_CHECKSUM;
	//		}


	//		CHKINFO DataBuffChecksum;
	//		memset(&DataBuffChecksum, 0, sizeof(DataBuffChecksum));
	//		uint64_t fileCurChk = 0;
	//		while (!file.atEnd())
	//		{
	//			QByteArray contStr = file.read(1024 * 1024);
	//			if (chkTypeIdx == 0) {
	//				Crc32CalcSubRoutine(&DataBuffChecksum, (unsigned char*)(contStr.data()), contStr.size());
	//			}
	//			else {
	//				fileCurChk += Utils::AngKCommonTools::GetByteSum((unsigned char*)(contStr.data()), contStr.size());
	//			}
	//		}

	//		if (chkTypeIdx == 0) {
	//			Crc32GetChkSum(&DataBuffChecksum);
	//			fileCurChk = DataBuffChecksum.chksum;
	//		}

	//		if (fileCurChk != iter.value()) {
	//			ALOG_FATAL("Failed to verify the project file, please check if the file is valid, file:\"%s\".", "CU", "--", iter.key().toLocal8Bit().data());
	//			return E_TAG_CHECKSUM;
	//		}

	//	}
	//}


	//加载TAG_EMMCIGNOREEXTREG，获取eMMCExtendCSD记录
	{
		std::string emmExtendCSDInfo;
		if (E_FMTFILE_OK == m_pProjFile->GetTagData(TAG_EMMCIGNOREEXTREG, emmExtendCSDInfo)) {
			m_pProjDataset->SeteMMCExtCSDParaJson(QString::fromStdString(emmExtendCSDInfo));
			PrintExtCSDLog(emmExtendCSDInfo);
		}
	}

	{//加载TAG_EMMCINTELLIGENT，获取eMMC智能分析信息
		std::string emmIntelligentInfo;
		if (E_FMTFILE_OK == m_pProjFile->GetTagData(TAG_EMMCINTELLIGENT, emmIntelligentInfo)) {
			m_pProjDataset->SeteMMCIntelligentJson(QString::fromStdString(emmIntelligentInfo));
		}
	}

	{//加载TAG_EMMCHEADER，获取eMMC写入SSD的TableHeader
		std::string stuTableHeaderChar;
		if (E_FMTFILE_OK == m_pProjFile->GetTagData(TAG_EMMCHEADER, stuTableHeaderChar)) {

			eMMCTableHeader stuTableHeader;
			memset(&stuTableHeader, 0, sizeof(eMMCTableHeader));
			memcpy(&stuTableHeader, stuTableHeaderChar.data(), sizeof(eMMCTableHeader));
			m_pProjDataset->SeteMMCeMMCTableHeader(stuTableHeader);
		}
	}

	{//eMMC的TableHeaderJson保存
		std::string stuTableHeaderJson;
		if (E_FMTFILE_OK == m_pProjFile->GetTagData(TAG_EMMCHEADERTABLE, stuTableHeaderJson)) {
			m_sTableHeaderJsonStr = QString::fromStdString(stuTableHeaderJson);
			m_pProjDataset->SeteMMCTableHeaderJson(m_sTableHeaderJsonStr);
		}
	}

	//加载TAG_CHIPOPERATE，获取芯片操作设置
	{
		std::string strChipOperInfo;
		if (E_FMTFILE_OK == m_pProjFile->GetTagData(TAG_CHIPOPERATE, strChipOperInfo)) {
			sProjPro.chipOperJson = strChipOperInfo;
			SwitchOperInfoSave();
		}
	}

	//加载TAG_CHIPOPCFG，获取芯片操作设置
	{
		std::string strChipBufChkInfo;
		if (E_FMTFILE_OK == m_pProjFile->GetTagData(TAG_CHIPOPCFG, strChipBufChkInfo)) {
			sProjPro.chipBufChkJson = strChipBufChkInfo;
			// print Option Configuration info
			{
				ALOG_INFO("<================Option Configuration Begin ==============>", "CU", "--");
				nlohmann::json json = nlohmann::json::parse(strChipBufChkInfo);
				std::string checkIDOpt = json["CheckID"].get<bool>() ? "Enabled" : "Disabled";
				std::string pinCheckOpt = json["PinCheck"].get<bool>() ? "Enabled" : "Disabled";
				std::string chipOverLapCheckOpt = json["ChipOverLap"].get<bool>() ? "Enabled" : "Disabled";
				int insertionMode = json["InsertionMode"].get<int>();
				std::string insertionModeOpt;
				switch (insertionMode)
				{
				case 0: insertionModeOpt = "Disable"; break;
				case 1: insertionModeOpt = "Insetion Check"; break;
				case 2: insertionModeOpt = "Auto_Sensing"; break;

				default:
					break;
				}
				std::string readUIDOpt = json["ReadUID"].get<bool>() ? "Enabled" : "Disabled";

				ALOG_INFO("Option : Check ID %s", "CU", "--", checkIDOpt.c_str());
				ALOG_INFO("Option : Pin Check %s", "CU", "--", pinCheckOpt.c_str());
				ALOG_INFO("Option : Chip OverLap %s", "CU", "--", chipOverLapCheckOpt.c_str());
				ALOG_INFO("Option : Insertion Mode %s", "CU", "--", insertionModeOpt.c_str());
				ALOG_INFO("Option : Read UID %s", "CU", "--", readUIDOpt.c_str());
				ALOG_INFO("<================Option Configuration End ==============>", "CU", "--");

			
			}
		}
	}

	//加载TAG_DrvCommonPara，获取芯片操作设置
	{
		std::string strDrvCommonPara;
		if (E_FMTFILE_OK == m_pProjFile->GetTagData(TAG_DrvCommonPara, strDrvCommonPara)) {
			m_pProjDataset->SetCommonDrvParaJson(QString::fromStdString(strDrvCommonPara));
		}
	}
	//加载project description
	{
		std::string strProDescription;
		if (E_FMTFILE_OK == m_pProjFile->GetTagData(TAG_ProDescriptionPara, strProDescription)) {
			sProjPro.projDescription = strProDescription.c_str();
			ALOG_INFO("Project Description : %s", "CU", "--", strProDescription.c_str());
		}
	}

	//加载TAG_DrvSelfPara，获取芯片操作设置
	{
		std::string strDrvSelfPara;
		if (E_FMTFILE_OK == m_pProjFile->GetTagData(TAG_DrvSelfPara, strDrvSelfPara)) {
			m_pProjDataset->SetSelfDrvParaJson(QString::fromStdString(strDrvSelfPara));
		}
	}

	//加载TAG_MULTIBUFMAP，获取bufMap数量与地址区间
	std::vector<tPartitionInfo*> vecPartMap;
	{
		std::string bufMapInfo;
		if (E_FMTFILE_OK == m_pProjFile->GetTagData(TAG_MULTIBUFMAP, bufMapInfo)) {
			//pFile.dencodeBase64_Json(bufMapInfo);//base64需要解码
			try {//nlohmann解析失败会报异常需要捕获一下
				auto j3 = nlohmann::json::parse(bufMapInfo);
				m_pDataBuffer->GetBufferMapByJson(j3, vecPartMap);


			}
			catch (const nlohmann::json::exception& e) {
				ALOG_FATAL("TAG_MULTIBUFMAP Json parse failed : %s.", "CU", "--", e.what());
			}
		}
		else {
			vecPartMap.clear();
			m_pDataBuffer->GetPartitionInfo(vecPartMap);
		}
	}

	//加载TAG_DRVINFO，读取到信息
	std::string DriverInfo;
	int getTagDataReturn = m_pProjFile->GetTagData(TAG_DRVINFO, DriverInfo);
	if (E_FMTFILE_OK == getTagDataReturn) {
		try {//nlohmann解析失败会报异常需要捕获一下
			auto j3 = nlohmann::json::parse(DriverInfo);
			if (j3.contains(XML_NODE_CHIPDATA_SECURITYSOLUTION)) {
				std::string securitySolution = j3[XML_NODE_CHIPDATA_SECURITYSOLUTION];
				ALOG_INFO("TAG_DRVINFO Json parse : %s.", "CU", "--", securitySolution.c_str());
				if (securitySolution == "XT-RS422") {
					CustomMessageHandler::instance()->RegisterBuiltinHandler("XT-RS422");
				}
			}
		}
		catch (const nlohmann::json::exception& e) {
			ALOG_FATAL("TAG_DRVINFO Json parse failed : %s.", "CU", "--", e.what());
		}
	}

	//加载TAG_CHIPBUFF，将数据写入内存地址映射
	{
		PTAG_t pTag;
		uchar* byBuf = new uchar[BUFFER_RW_SIZE];
		ADR  adrOff = 0;
		uint uiReadLen = 0;
		bool IsDataCondense = false;
		uint64_t MaxSize;
		uint BytesToRead;
		CHKINFO Chkinfo;
		memset(&Chkinfo, 0, sizeof(Chkinfo));
		if (m_pProjFile->GetTagHeader(TAG_CHIPBUFF, pTag) != E_FMTFILE_OK) {
			ALOG_FATAL("Get project tag TAG_CHIPBUFF error.", "CU", "--");
			//m_IsLoadPrjErr = TRUE;
			return E_TAG_UNEOUGH;
		}

		MaxSize = m_pDataBuffer->GetSize();
		if (MaxSize > pTag.dwLength) {////采用的是压缩映射
			IsDataCondense = true;
		}

		int partCnt = vecPartMap.size();

		for (int partIdx = 0; partIdx < partCnt; ++partIdx) {

			if (!vecPartMap[partIdx]->PartitionShow)
				continue;

			int bufCnt = vecPartMap[partIdx]->vecSubView.size();

			for (int bufIdx = 0; bufIdx < bufCnt; ++bufIdx)
			{
				tBufInfo* bufInfo = vecPartMap[partIdx]->vecSubView[bufIdx];

				if (!bufInfo->m_bBufferShow)
					continue;

				if (IsDataCondense == false) {///没有压缩保存，那么就需要进行偏移，读取真正需要的位置
					m_pProjFile->SeekTagBuf(pTag, bufInfo->llBufStart, 0);
				}
				for (adrOff = bufInfo->llBufStart; adrOff <= bufInfo->llBufEnd; adrOff += BUFFER_RW_SIZE) {
					BytesToRead = bufInfo->llBufEnd + 1 - adrOff;
					if (BytesToRead > BUFFER_RW_SIZE) {
						BytesToRead = BUFFER_RW_SIZE;
					}
					uiReadLen = m_pProjFile->ReadTagBuf(byBuf, BytesToRead);
					Crc32CalcSubRoutine(&Chkinfo, byBuf, uiReadLen);
					m_pDataBuffer->BufferWrite(adrOff, byBuf, uiReadLen);
				}
			}
		}

		Crc32GetChkSum(&Chkinfo);
		if (pTag.dwCRC != 0) {
			if ((uint)Chkinfo.chksum != pTag.dwCRC) {///进行数据校验值的比对
				ALOG_FATAL("Data Buffer Internal Checksum Compare Error, Desired:0x%08X, Really:0x%08X.", "CU", "--", pTag.dwCRC, (uint)Chkinfo.chksum);
			}
		}

		delete[] byBuf;
	}

	m_pProjFile->CloseFile();


	if (!m_pProjFile->TagHandleCheck()) {
		// 存在必须处理但未处理的TAG
	}

	nlohmann::json LoadProjectJson;
	LoadProjectJson["FilePath"] = _projPath.toStdString();
	LoadProjectJson["RetCode"] = ret;
	LoadProjectJson["RetInfo"] = ret != 0 ? "Load Project Failed" : "Load Project Success";
	EventLogger->SendEvent(EventBuilder->GetLoadProject(LoadProjectJson));

	return ret;
}

void ACProjManager::ErrorClearFile()
{
	m_pProjFile->CloseFile();
}

AngKProjDataset* ACProjManager::GetProjData()
{
	return m_pProjDataset.get();
}

AngKDataBuffer* ACProjManager::GetProjDataBuffer()
{
	return m_pDataBuffer.get();
}

AngKProjFile* ACProjManager::GetProjFilePtr()
{
	return m_pProjFile.get();
}

void ACProjManager::SetProjName(QString _sProjName)
{
	m_sProjName = _sProjName;
}

QString& ACProjManager::GetProjName()
{
	return m_sProjName;
}

QString ACProjManager::GetProjDescription() {
	return m_pProjDataset.get()->GetProjProperty().projDescription.c_str();
}

void ACProjManager::SwitchOperInfoSave()
{
	OpInfoList& projOpInfo = m_pProjDataset->GetOperInfoList();
	std::string operInfoJson = m_pProjDataset->GetProjProperty().chipOperJson;
	try {//nlohmann解析失败会报异常需要捕获一下
		auto j3 = nlohmann::json::parse(operInfoJson);

		projOpInfo.clear();

		bool bEraseRight = j3["Erase"]["Enable"].get<bool>();
		bool bBlankRight = j3["Blank"]["Enable"].get<bool>();
		bool bProgRight = j3["Program"]["Enable"].get<bool>();
		bool bVerifyRight = j3["Verify"]["Enable"].get<bool>();
		bool bSecureRight = j3["Secure"]["Enable"].get<bool>();
		bool bReadRight = j3["Read"]["Enable"].get<bool>();
		//bool bMarginVerifyRight = m_stuOperJson.otherInfo.bH_L_VccVerify;
		//bool bChecksumCompareRight = m_stuOperJson.otherInfo.bCompare;

		//erasePage
		if (bEraseRight)
		{
			OperatorInfo eraseInfo;
			eraseInfo.strOpName = "Erase";
			eraseInfo.iOpId = ChipOperCfgCmdID::Erase;
			int eraseData = j3["Erase"]["EraseData"].get<int>();
			int blankData = j3["Erase"]["BlankCheckData"].get<int>();

			if (eraseData > 0)
				eraseInfo.vecOpList.push_back(eraseData);
			if (bBlankRight && blankData > 0)
				eraseInfo.vecOpList.push_back(blankData);

			projOpInfo.push_back(eraseInfo);
		}

		//blankPage
		if (bBlankRight)
		{
			OperatorInfo blankInfo;
			blankInfo.strOpName = "Blank";
			blankInfo.iOpId = ChipOperCfgCmdID::BlankCheck;
			int blankData = j3["Blank"]["BlankCheckData"].get<int>();
			if (blankData > 0)
				blankInfo.vecOpList.push_back(blankData);

			projOpInfo.push_back(blankInfo);
		}

		//programPage
		if (bProgRight)
		{
			OperatorInfo programInfo;
			programInfo.strOpName = "Program";
			programInfo.iOpId = ChipOperCfgCmdID::Program;
			int eraseData = j3["Program"]["EraseData"].get<int>();
			int blankData = j3["Program"]["BlankCheckData"].get<int>();
			int progData = j3["Program"]["ProgramData"].get<int>();
			int verifyData = j3["Program"]["VerifyData"].get<int>();
			int marginVerifyData = j3["Program"]["MarginVerifyData"].get<int>();
			//int compareData = ui->progPage_CompareTagComboBox->currentData().toInt();
			int secureData = j3["Program"]["SecureData"].get<int>();

			if (bEraseRight && eraseData > 0)
				programInfo.vecOpList.push_back(eraseData);
			if (bBlankRight && blankData > 0)
				programInfo.vecOpList.push_back(blankData);
			if (progData > 0)
				programInfo.vecOpList.push_back(progData);
			if (bVerifyRight && verifyData > 0)
				programInfo.vecOpList.push_back(verifyData);
			if (marginVerifyData > 0)
				programInfo.vecOpList.push_back(marginVerifyData);
			//if (bChecksumCompareRight && compareData > 0)
			//	programInfo.vecOpList.push_back(compareData);
			if (bSecureRight && secureData > 0)
				programInfo.vecOpList.push_back(secureData);

			projOpInfo.push_back(programInfo);
		}
		//verifyPage
		if (bVerifyRight)
		{
			OperatorInfo verifyInfo;
			verifyInfo.strOpName = "Verify";
			verifyInfo.iOpId = ChipOperCfgCmdID::Verify;
			int verifyData = j3["Verify"]["VerifyData"].get<int>();
			int marginVerifyData = j3["Verify"]["MarginVerifyData"].get<int>();

			if (verifyData > 0)
				verifyInfo.vecOpList.push_back(verifyData);
			if (marginVerifyData > 0)
				verifyInfo.vecOpList.push_back(marginVerifyData);

			projOpInfo.push_back(verifyInfo);
		}

		//securePage
		if (bSecureRight)
		{
			OperatorInfo secureInfo;
			secureInfo.strOpName = "Secure";
			secureInfo.iOpId = ChipOperCfgCmdID::Secure;
			int verifyData = j3["Secure"]["VerifyData"].get<int>();
			int marginVerifyData = j3["Secure"]["MarginVerifyData"].get<int>();
			int secureData = j3["Secure"]["SecureData"].get<int>();

			if (bVerifyRight && verifyData > 0)
				secureInfo.vecOpList.push_back(verifyData);
			if (marginVerifyData > 0)
				secureInfo.vecOpList.push_back(marginVerifyData);
			if (secureData > 0)
				secureInfo.vecOpList.push_back(secureData);

			projOpInfo.push_back(secureInfo);
		}

		//readPage
		if (bReadRight)
		{
			OperatorInfo readInfo;
			readInfo.strOpName = "Read";
			readInfo.iOpId = ChipOperCfgCmdID::Read;
			int readData = j3["Read"]["ReadData"].get<int>();
			if (readData > 0)
				readInfo.vecOpList.push_back(readData);

			projOpInfo.push_back(readInfo);
		}

		//selfPage
		OperatorInfo selfInfo;
		selfInfo.strOpName = "Self";
		selfInfo.iOpId = ChipOperCfgCmdID::Custom;
		int seqComData_1 = j3["Self"]["selfCmd1Data"].get<int>();
		int seqComData_2 = j3["Self"]["selfCmd2Data"].get<int>();
		int seqComData_3 = j3["Self"]["selfCmd3Data"].get<int>();
		int seqComData_4 = j3["Self"]["selfCmd4Data"].get<int>();
		int seqComData_5 = j3["Self"]["selfCmd5Data"].get<int>();
		int seqComData_6 = j3["Self"]["selfCmd6Data"].get<int>();
		int seqComData_7 = j3["Self"]["selfCmd7Data"].get<int>();

		seqComData_1 > 0 ? selfInfo.vecOpList.push_back(seqComData_1) : false;
		seqComData_2 > 0 ? selfInfo.vecOpList.push_back(seqComData_2) : false;
		seqComData_3 > 0 ? selfInfo.vecOpList.push_back(seqComData_3) : false;
		seqComData_4 > 0 ? selfInfo.vecOpList.push_back(seqComData_4) : false;
		seqComData_5 > 0 ? selfInfo.vecOpList.push_back(seqComData_5) : false;
		seqComData_6 > 0 ? selfInfo.vecOpList.push_back(seqComData_6) : false;
		seqComData_7 > 0 ? selfInfo.vecOpList.push_back(seqComData_7) : false;
		projOpInfo.push_back(selfInfo);
	}
	catch (const nlohmann::json::exception& e) {
		ALOG_FATAL("OperateInfo Json parse failed : %s.", "CU", "--", e.what());
	}
}

bool ACProjManager::CheckEmmcFile(QString strIP, int nHop) {
	eMMCFileImportDataJsonSerial fileImport = m_pProjDataset->geteMMCFileImportData();
	std::vector<eMMCFileDataJsonSerial>& dataJson = m_pProjDataset->geteMMCFileData();
	std::string imageFileFormat = fileImport.getJsonValue<std::string>("imageFileFormat");

	std::vector<tSeptBineMMCFileMap> tbinFileMapInfoVec;
	m_pAnalyzeMgr->UnSerialvFiletoJson(m_pProjFile->getFilePath(), tbinFileMapInfoVec, m_pProjDataset->GeteMMCIntelligentJson().toStdString());

	DeviceStu devInfo = ACDeviceManager::instance().getDevInfo(strIP, nHop);
	if (CheckSSDTaskDownload(devInfo, tbinFileMapInfoVec))
	{
		ALOG_INFO("Device %s:%d check user files are consistent", "CU", "--", strIP.toStdString().c_str(), nHop);
		return true;
	}
	else {
		ALOG_INFO("Device %s:%d check user files are inconsistent", "CU", "--", strIP.toStdString().c_str(), nHop);
		return false;
	}
}


int ACProjManager::LoadeMMCFile(QString strIP, int nHop)
{
	int nRet = 0;
	eMMCFileImportDataJsonSerial fileImport = m_pProjDataset->geteMMCFileImportData();
	std::vector<eMMCFileDataJsonSerial>& dataJson = m_pProjDataset->geteMMCFileData();
	std::string imageFileFormat = fileImport.getJsonValue<std::string>("imageFileFormat");
	//fileImport.deserialize(dataJson, imageFileFormat);

	int nImageType = QString::fromStdString(imageFileFormat).toInt();

	switch (nImageType)
	{
	case emmc_BinFiles:
		nRet = LoadSeparatedBinFiles(strIP, nHop, dataJson);
		break;
	case emmc_MTK:
		break;
	case emmc_Ambarella:
		break;
	case emmc_Phison:
		break;
	case emmc_CIMG:
		break;
	case emmc_Huawei:
		break;
	case emmc_ACIMG:
		break;
	default:
		break;
	}

	//记录加载档案事件
	nlohmann::json LoadFilesJson;
	LoadFilesJson["filesInfo"] = nlohmann::json::array();

	for (int i = 0; i < dataJson.size(); ++i)
	{
		eMMCFileInfo efInfo;
		dataJson[i].deserialize(efInfo);

		nlohmann::json infoJson;
		infoJson["fileName"] = efInfo.sFilePath;
		infoJson["fileSize"] = efInfo.nFileSize;
		infoJson["fileChecksum"] = efInfo.nCheckSum;

		LoadFilesJson["filesInfo"].push_back(infoJson);
	}

	EventLogger->SendEvent(EventBuilder->GetLoadFiles(LoadFilesJson));

	return nRet;
}

int ACProjManager::LoadSeparatedBinFiles(QString strIP, int nHop, std::vector<eMMCFileDataJsonSerial>& _efileDataJson)
{
	//使用智能分析管理类反序列化档案信息
	std::vector<tSeptBineMMCFileMap> tbinFileMapInfoVec;
	m_pAnalyzeMgr->UnSerialvFiletoJson(m_pProjFile->getFilePath(), tbinFileMapInfoVec, m_pProjDataset->GeteMMCIntelligentJson().toStdString());

	//校验SSD用户档案是否已经下发

	DeviceStu devInfo = ACDeviceManager::instance().getDevInfo(strIP, nHop);
	if (CheckSSDTaskDownload(devInfo, tbinFileMapInfoVec))
	{
		ALOG_INFO("Device %s:%d check user files are consistent，no need download files repeat.", "CU", "--", strIP.toStdString().c_str(), nHop);
		emit sgnUpdateFileDownloadProgress(99, strIP, nHop);
		return 0;
	}
	else {
		ALOG_WARN("Device %s:%d check user files are not consistent，need download files.", "CU", "--", strIP.toStdString().c_str(), nHop);
	}
	//ClearEmmcHeaderData(strIP, nHop, devInfo.tMainBoardInfo.strHardwareSN);

	//先写用户档案数据，失败了则不下发Header，并将头置为空
	//emmc发往SSD的数据非常大，需要分段传送
	QByteArray partitionData;

	ALOG_INFO("Send eMMC file user data to %s:%d SSD", "CU", "FP", strIP.toStdString().c_str(), nHop);
	uint64_t writeAddr = SSD_WRITE_PARTITION_DATA;
	int nRet = 0;
	int allFileSize = tbinFileMapInfoVec.size();
	int currentFileIndex = 0;
	
	uint64_t binFileAllSize = 0;
	uint64_t currentReadByte = 0;
	for (int i = 0; i < tbinFileMapInfoVec.size(); ++i)
	{
		binFileAllSize += tbinFileMapInfoVec[i].FileSize;
	}

	for (int i = 0; i < tbinFileMapInfoVec.size(); ++i)
	{
		tSeptBineMMCFileMap tBinFile = tbinFileMapInfoVec[i];
		if(tBinFile.PartName == "acBinXml")
			continue;

		QString strBinFiles = QString::fromStdString(tBinFile.strFileName);
		QFile file(strBinFiles);
		if (!file.open(QIODevice::ReadOnly)) {
			file.close();
			ALOG_FATAL("open eMMC file %s failed", "CU", "--", strBinFiles.toStdString().c_str());
			goto _end;
		}
		currentFileIndex++;
		int nRecoverTrySend = AngKGlobalInstance::instance()->ReadValue("DeviceComm", "Retransmission").toInt();
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
					//partitionData.append(byteFileSeg);
					int nTryReSend = nRecoverTrySend;
					while (nTryReSend != 0)
					{
						nRet = AngKMessageHandler::instance().Command_StoreDataToSSDorDDR(strIP.toStdString(), "FIBER2SSD", nHop, 0, writeAddr, byteFileSeg, devInfo.tMainBoardInfo.strHardwareSN, "eMMCFileData");
						QThread::msleep(10);//提高UDP的稳定性，每发送一次命令包，停10ms
						if (nRet == 0) {//返回错误需要加重传机制
							writeAddr += last_Blk * EMMC_SSD_MULTIPLE;
							nTryReSend = nRecoverTrySend;
							currentReadByte += bytesToRead;
							int nFileValue = 60 + (static_cast<float>(currentReadByte) / binFileAllSize) * 39;
							emit sgnUpdateFileDownloadProgress(nFileValue, strIP, nHop);
							break;
						}
						else {
							nTryReSend--;
							ALOG_FATAL("Send eMMC file : %s to %s:%d SSD failed(errorCode: %d), Try to Retrans, %d remaining times", "CU", "FP", strBinFiles.toStdString().c_str(), strIP.toStdString().c_str(), nHop, nRet, nTryReSend);
						}
					}

					if (nTryReSend <= 0 && nRet != 0) {
						nRet = -1;
						file.close();
						goto _end;
					}
				}
				else
				{
					//partitionData.append(chunk);
					int nTryReSend = AngKGlobalInstance::instance()->ReadValue("DeviceComm", "Retransmission").toInt();
					while (nTryReSend != 0)
					{
						nRet = AngKMessageHandler::instance().Command_StoreDataToSSDorDDR(strIP.toStdString(), "FIBER2SSD", nHop, 0, writeAddr, chunk, devInfo.tMainBoardInfo.strHardwareSN, "eMMCFileData");
						QThread::msleep(10);//提高UDP的稳定性，每发送一次命令包，停10ms
						if (nRet == 0) {//返回错误需要加重传机制
							writeAddr += bytesToRead;
							fileOffset += bytesToRead;
							nTryReSend = nRecoverTrySend;
							currentReadByte += bytesToRead;
							int nFileValue = 60 + (static_cast<float>(currentReadByte) / binFileAllSize) * 39;
							emit sgnUpdateFileDownloadProgress(nFileValue, strIP, nHop);
							break;
						}
						else {
							nTryReSend--;
							ALOG_FATAL("Send eMMC file : %s to %s:%d SSD Try to Retrans(Result: %d), %d remaining times", "CU", "FP", strBinFiles.toStdString().c_str(), strIP.toStdString().c_str(), nHop, nRet, nTryReSend);
						}
					}

					if (nTryReSend <= 0 && nRet != 0) {
						nRet = -1;
						file.close();
						goto _end;
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
		EventLogger->SendEvent(EventBuilder->GetDataTransfer(dataTransferJson));
		file.close();

	}

	emit sgnUpdateFileDownloadProgress(99, strIP, nHop);
_end:

	if(nRet < 0){
		ClearEmmcHeaderData(strIP, nHop, devInfo.tMainBoardInfo.strHardwareSN);
	}
	else {

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
		featureJson["DataOffsetInSSD"] = "0x14000000";

		//EXTCSD 部分
		std::string projExtCSD = m_pProjDataset->GeteMMCExtCSDParaJson().toStdString();
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
				ALOG_FATAL("LoadSeparatedBinFiles Parse EXTCSD Json error : %s", "CU", "--", e.what());
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
		std::string test = byteSumJson.dump();

		//Files 部分
		nlohmann::json filesJson = nlohmann::json::array();
		uint64_t ssdAddr = SSD_WRITE_PARTITION_DATA;
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

		std::string strMagic = "Json";
		memcpy(eHeader.MagicType, strMagic.c_str(), strMagic.size());
		eHeader.PartitionTableSize = eMMC_Json.dump().size();

		ushort headerChk = 0;
		calc_crc16sum((uchar*)(eMMC_Json.dump().c_str()), eMMC_Json.dump().size(), &headerChk);
		eHeader.PartitionTableCRC = headerChk;

		headerChk = 0;
		calc_crc16sum((uchar*)(&eHeader) + 2, sizeof(eHeader) - 2, &headerChk);
		eHeader.HeaderCRC = headerChk;

		uint64_t nByte = sizeof(eMMCTableHeader) + eMMC_Json.dump().size();
		uint64_t a_Blk = 0;

		if (nByte % EMMC_SSD_MULTIPLE == 0)
			a_Blk = nByte / EMMC_SSD_MULTIPLE;
		else
			a_Blk = (nByte / EMMC_SSD_MULTIPLE) + 1;

		uint64_t realSize = a_Blk * EMMC_SSD_MULTIPLE <= 4096 ? 4096 : a_Blk * EMMC_SSD_MULTIPLE;

		partitionTable.resize(realSize - sizeof(eHeader));
		partitionTable.fill(0);
		partitionTable.replace(0, eMMC_Json.dump().size(), eMMC_Json.dump().c_str());

		QByteArray tableByte;
		tableByte.append((char*)&eHeader, sizeof(eHeader));
		tableByte.append(partitionTable);
		//dataStruct.append(partitionData, partitionData.size());
		dataStruct.append(partitionTable, partitionTable.size());

		//QFile tempFIle(QCoreApplication::applicationDirPath() + "/partitionTableHeader.bin");
		//if (tempFIle.open(QIODevice::ReadWrite | QIODevice::Truncate))
		//{
		//	tempFIle.write(tableByte);
		//}
		//tempFIle.close();

		int nTryReSend = AngKGlobalInstance::instance()->ReadValue("DeviceComm", "Retransmission").toInt();
		ALOG_INFO("Send eMMC file Header data to %s:%d SSD", "CU", "FP", strIP.toStdString().c_str(), nHop);
		int TableRet = 0;
		while (nTryReSend != 0)
		{
			TableRet = AngKMessageHandler::instance().Command_StoreDataToSSDorDDR(strIP.toStdString(), "FIBER2SSD", nHop, 0, SSD_WRITE_PARTITION_TABLE, tableByte, devInfo.tMainBoardInfo.strHardwareSN, "eMMCFileData");
			//TableRet = AngKMessageHandler::instance().Command_StoreDataToSSDorDDR(strIP.toStdString(), "FIBER2SSD", nHop, 0, SSD_WRITE_PARTITION_TABLE, tableByte, devInfo.tMainBoardInfo.strHardwareSN, "eMMCFileData");

			if (TableRet == 0) {//返回错误需要加重传机制
				break;
			}
			else {
				ALOG_FATAL("Send eMMC file Header data to %s:%d SSD failed(errorCode: %d), Try to Retrans, %d remaining times", "CU", "FP", strIP.toStdString().c_str(), nHop, TableRet, nTryReSend);
				nTryReSend--;
			}
		}
		if (nTryReSend <= 0 && TableRet != 0) {
			nRet = -1;
			return nRet;
		}
	}
	return nRet;
}

QByteArray ACProjManager::GetBufferCheckBPUAttribute()
{
	QByteArray _attrBute;

	std::string _bufCheckJson = m_pProjDataset->GetProjProperty().chipBufChkJson;
	uint32_t nBPUAttribute = 0;
	try {
		auto BufChkJson = nlohmann::json::parse(_bufCheckJson);

		if (BufChkJson["CheckID"]) {//驱动还未支持，暂时使用Bit7
			nBPUAttribute |= (1 << 6);
		}

		if (BufChkJson["PinCheck"]) {
			nBPUAttribute |= (1 << 3);
		}

		if (BufChkJson["ChipOverLap"]) {
			nBPUAttribute |= (1 << 5);
		}

		if (BufChkJson["InsertionMode"] == BufferCheckInsetionMode::Insetion_Check) {
			nBPUAttribute |= (1 << 2);
		}
		else if (BufChkJson["InsertionMode"] == BufferCheckInsetionMode::Auto_Sensing) {
			nBPUAttribute |= (1 << 4);
		}

		if (BufChkJson["ReadUID"]) {
			nBPUAttribute |= (1 << 1);
		}
	}
	catch (nlohmann::json::exception& e) {
		ALOG_FATAL("Parse ChipBufChk Json error: ", "CU", "--", e.what());
	}

	QString BPUAttribute = QString("%1").arg(nBPUAttribute, 8, 16, QLatin1Char('0'));
	_attrBute.append(nBPUAttribute);
	return _attrBute;
}

bool ACProjManager::CheckSSDTaskDownload(DeviceStu& devInfo, std::vector<tSeptBineMMCFileMap>& vecBinFile)
{
	//快速比对
	int nVerifyType = AngKGlobalInstance::instance()->ReadValue("DeviceComm", "DoublueVerify").toInt();
	if (nVerifyType == 1) {
		int32_t byteReadSize = 1 * 1024 * 1024;
		QByteArray readSSDByte;
		if (m_pProjDataset->getChipData().getJsonValue<std::string>("chipType") == "eMMC") {
			int nRecoverTrySend = AngKGlobalInstance::instance()->ReadValue("DeviceComm", "Retransmission").toInt();
			int nTryReSend = nRecoverTrySend;
			int ret = 0;
			while (nTryReSend != 0)
			{
				ret = AngKMessageHandler::instance().Command_ReadDataFromSSD(devInfo.strIP, "SSD2FIBER", devInfo.nHopNum, 0, SSD_WRITE_PARTITION_TABLE, byteReadSize, readSSDByte, devInfo.tMainBoardInfo.strHardwareSN, "eMMCHeaderData");
			
				if (ret == 0) {

					eMMCTableHeader eHeader;
					memcpy(&eHeader, readSSDByte.constData(), sizeof(eMMCTableHeader));

					eMMCTableHeader projeHeader = m_pProjDataset->GeteMMCTableHeader();
					if (projeHeader.HeaderCRC == eHeader.HeaderCRC && projeHeader.PartitionTableCRC == eHeader.PartitionTableCRC) {
						for (int i = 0; i < vecBinFile.size(); ++i) {
							QFileInfo pFileInfo(QString::fromStdString(vecBinFile[i].strFileName));
							if (pFileInfo.lastModified().toTime_t() != vecBinFile[i].lastModifyTime) {
								return false;
							}
						}
					}
					else {
						return false;
					}

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
								QByteArray readSrc_1K = entryFile.read(1024);
								if (readSrc_1K.size() < 1024) {
									uchar* pBuf = new uchar[1024];
									memset(pBuf, 0, 1024);
									memcpy(pBuf, (uchar*)readSrc_1K.constData(), readSrc_1K.size());

									readSrc_1K = QByteArray((char*)pBuf, 1024);

									delete[] pBuf;
								}
								ushort srcFeture = 0;
								calc_crc16sum((uchar*)readSrc_1K.constData(), readSrc_1K.size(), &srcFeture);
								if (tVFile.Feature == srcFeture) {
									continue;
								}
								else {
									return false;
								}
							}
						}
					}

					break;
				}
				else {
					nTryReSend--;
					QThread::msleep(100);

					ALOG_FATAL("Device %s:%d Check user bin files Try to Retrans(Result:%d), %d remaining times", "FP", "CU", devInfo.strIP.c_str(), devInfo.nHopNum, ret, nTryReSend);
					//return false;
				}
			}

			if (nTryReSend <= 0 && ret != 0) {
				return false;
			}
		}
	}
	else {
		return false;	
	}

	return true;
}

void ACProjManager::ClearEmmcHeaderData(QString strIP, int nHop, std::string& devSN)
{
	ALOG_INFO("Clear eMMC file Header data to %s:%d SSD", "CU", "FP", strIP.toStdString().c_str(), nHop);
	const int size = 64 * 1024 * 1024;
	QByteArray tableByte(size, 0); // 使用 0 填充初始化
	//返回值不为0，说明写入档案数据已经失败，需要将HeaderSSD块清空
	AngKMessageHandler::instance().Command_StoreDataToSSDorDDR(strIP.toStdString(), "FIBER2SSD", nHop, 0, SSD_WRITE_PARTITION_TABLE, tableByte, devSN, "eMMCFileData");
	QThread::msleep(10);//提高UDP的稳定性，每发送一次命令包，停10ms
}

int ACProjManager::CheckAllFileChkSum()
{
	// chk校验
	for (auto iterChk = m_mapReadyCheckFile.begin(); iterChk != m_mapReadyCheckFile.end(); iterChk++) {
		int nFileCount = iterChk.value().size();
		int nFileIndex = 0;
		double totalCalValue = 0;
		for (auto iter = iterChk.value().begin(); iter != iterChk.value().end(); iter++) {
			QFile file(iter.key());
			if (!file.open(QIODevice::ReadOnly)) {
				ALOG_FATAL("Load Project File error, open image file:\"%s\" failed.", "CU", "--", iter.key().toLocal8Bit().data());
				return E_FMTFILE_ERRER;
			}
			nFileIndex++;

			CHKINFO DataBuffChecksum;
			memset(&DataBuffChecksum, 0, sizeof(DataBuffChecksum));
			uint64_t fileCurChk = 0;
			uint64_t singleFileSize = file.size();
			uint64_t curFileSize = 0;
			double curCalValue = totalCalValue;
			double RatioValue = (static_cast<double>(1) / nFileCount);
			while (!file.atEnd())
			{
				QByteArray contStr = file.read(1024 * 1024);
				if (iterChk->first() == 0) {
					Crc32CalcSubRoutine(&DataBuffChecksum, (unsigned char*)(contStr.data()), contStr.size());
				}
				else {
					fileCurChk += Utils::AngKCommonTools::GetByteSum((unsigned char*)(contStr.data()), contStr.size());
				}

				curFileSize += contStr.size();
				double fileValue = (static_cast<double>(curFileSize) / singleFileSize) * 100;
				double changeCalValue = curCalValue + (fileValue * RatioValue);
				emit sgnUpdateChkProgress(changeCalValue * 0.5);
			}

			if (iterChk->first() == 0) {
				Crc32GetChkSum(&DataBuffChecksum);
				fileCurChk = DataBuffChecksum.chksum;
			}

			totalCalValue = ((static_cast<double>(nFileIndex) / nFileCount) * 50);
			if (fileCurChk != iter.value()) {
				ALOG_FATAL("Failed to verify the project file, please check if the file is valid, file:\"%s\".", "CU", "--", iter.key().toLocal8Bit().data());
				emit sgnUpdateChkProgress(100 * 0.5);
				return E_TAG_CHECKSUM;
			}

		}

		emit sgnUpdateChkProgress(100 * 0.5);
	}

	return E_FMTFILE_OK;
}

void ACProjManager::CalBinFilesByteSum(std::vector<tSeptBineMMCFileMap>& vecBinFile, nlohmann::json& _bytesumJson)
{
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
			else if(j == tBinFile.vFiles.size() - 1 && remainderNum != 0){
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

void ACProjManager::PrintOpenProjLog(QString _fPath)
{
	ALOG_INFO("Loading Project : %s", "CU", "--", _fPath.toStdString().c_str());
	std::string strVersion = m_pProjDataset->GetProjProperty().projVersion;
	ALOG_INFO("Project Build version : %s", "CU", "--", strVersion.c_str());
	QFileInfo fInfo(_fPath);
	QString timeFormat = fInfo.birthTime().toString("yyyy/MM/dd hh:mm:ss");
	ALOG_INFO("Project Build time : %s", "CU", "--", timeFormat.toStdString().c_str());
	QString hostName = QHostInfo::localHostName();
	ALOG_INFO("Project Author : %s", "CU", "--", hostName.toStdString().c_str());
	ALOG_INFO("Project Build on : AP9900", "CU", "--");
}

void ACProjManager::PrintExtCSDLog(std::string& extcsdJson)
{
	if (extcsdJson.empty())
		return;

	//加载eMMCExtCSD的信息
	std::string extCSDJson = extcsdJson;
	eMMCOPTION_Modify readExtCSD;
	ALOG_INFO("------------------------------ExtCSD------------------------------", "CU", "--");
	try {
		auto extJson = nlohmann::json::parse(extCSDJson);
		nlohmann::json regConfigJson = extJson["RegConfig"];
		nlohmann::json partitonSizeJson = extJson["PartitionSize"];

		for (int i = 0; i < regConfigJson.size(); ++i)
		{
			int nAddr = regConfigJson[i]["Addr"];
			std::string strName = regConfigJson[i]["Name"];
			std::string strValue = regConfigJson[i]["Value"];
			GetExtCSDReg(QString::number(nAddr), QString::fromStdString(strValue), QString::fromStdString(strName), readExtCSD);
		}

		for (int j = 0; j < partitonSizeJson.size(); ++j)
		{
			uint32_t nSize = partitonSizeJson[j]["Size"];
			std::string strName = partitonSizeJson[j]["Name"];
			std::string strUnit = partitonSizeJson[j]["Unit"];
			GetPartitionSize(QString::fromStdString(strName), QString::number(nSize), QString::fromStdString(strUnit), readExtCSD);
		}
	}
	catch (const nlohmann::json::exception& e) {
		ALOG_FATAL("ProjManager eMMCExtCSDPara Json parse failed : %s.", "CU", "--", e.what());
	}

	QString extcsdInfo;
	extcsdInfo = QString("ExtCSD[16] SECURE_REMOVAL_TYPE : %1").arg(readExtCSD.modify_extcsd.secure_removal_type, 2, 16, QLatin1Char('0')).toUpper();
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());
	extcsdInfo = QString("ExtCSD[53:52] EXT_PARTITIONS_ATTRIBUTE : %1").arg(*(uint16_t*)readExtCSD.modify_extcsd.ext_partitions_attribute, 4, 16, QLatin1Char('0')).toUpper();
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());
	extcsdInfo = QString("ExtCSD[134] SEC_BAD_BLK_MGMNT : %1").arg(readExtCSD.modify_extcsd.sec_bad_blk_mgmnt, 2, 16, QLatin1Char('0')).toUpper();
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());
	extcsdInfo = QString("ExtCSD[139:136] ENH_START_ADDR : ");
	for (int i = 0; i < 4; i++) {
		extcsdInfo += QString("%1").arg(readExtCSD.modify_extcsd.enh_start_addr[3 - i], 2, 16, QLatin1Char('0')).toUpper();
	}
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());
	extcsdInfo = QString("ExtCSD[142:140] ENH_SIZE_MULT : ");
	for (int i = 0; i < 3; i++) {
		extcsdInfo += QString("%1").arg(readExtCSD.modify_extcsd.enh_size_mult[2 - i], 2, 16, QLatin1Char('0')).toUpper();
	}
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());
	extcsdInfo = QString("ExtCSD[145:143] GPP1_SIZE_MULT:");
	for (int i = 0; i < 3; i++) {
		extcsdInfo += QString("%1").arg(readExtCSD.modify_extcsd.gp_size_mult[2 - i], 2, 16, QLatin1Char('0')).toUpper();
	}
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());
	extcsdInfo = QString("ExtCSD[148:146] GPP2_SIZE_MULT:");
	for (int i = 0; i < 3; i++) {
		extcsdInfo += QString("%1").arg(readExtCSD.modify_extcsd.gp_size_mult[5 - i], 2, 16, QLatin1Char('0')).toUpper();
	}
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());
	extcsdInfo = QString("ExtCSD[151:149] GPP3_SIZE_MULT:");
	for (int i = 0; i < 3; i++) {
		extcsdInfo += QString("%1").arg(readExtCSD.modify_extcsd.gp_size_mult[8 - i], 2, 16, QLatin1Char('0')).toUpper();
	}
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());
	extcsdInfo = QString("ExtCSD[154:152] GPP4_SIZE_MULT:");
	for (int i = 0; i < 3; i++) {
		extcsdInfo += QString("%1").arg(readExtCSD.modify_extcsd.gp_size_mult[11 - i], 2, 16, QLatin1Char('0')).toUpper();
	}
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());

	extcsdInfo = QString("ExtCSD[155] PARTITION_SETTING_COMPLETED : %1").arg(readExtCSD.modify_extcsd.partition_setting_completed, 2, 16, QLatin1Char('0')).toUpper();
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());
	extcsdInfo = QString("ExtCSD[156] PARTITION_ATTRIBUTE : %1").arg(readExtCSD.modify_extcsd.partitions_attribute, 2, 16, QLatin1Char('0')).toUpper();
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());
	extcsdInfo = QString("ExtCSD[162] RST_n_FUNCTION : %1").arg(readExtCSD.modify_extcsd.rst_n_function, 2, 16, QLatin1Char('0')).toUpper();
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());
	extcsdInfo = QString("ExtCSD[163] BKOPS_EN : %1").arg(readExtCSD.modify_extcsd.bkops_en, 2, 16, QLatin1Char('0')).toUpper();
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());
	extcsdInfo = QString("ExtCSD[167] WR_REL_SET : %1").arg(readExtCSD.modify_extcsd.wr_rel_set, 2, 16, QLatin1Char('0')).toUpper();
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());
	extcsdInfo = QString("ExtCSD[168] RPMB_SIZE_MULT : %1").arg(readExtCSD.modify_extcsd.rpmb_size_mult, 2, 16, QLatin1Char('0')).toUpper();
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());
	extcsdInfo = QString("ExtCSD[169] FW_CONFIG : %1").arg(readExtCSD.modify_extcsd.fw_config, 2, 16, QLatin1Char('0')).toUpper();
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());
	extcsdInfo = QString("ExtCSD[171] USER_WP : %1").arg(readExtCSD.modify_extcsd.user_wp, 2, 16, QLatin1Char('0')).toUpper();
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());
	extcsdInfo = QString("ExtCSD[173] BOOT_WP : %1").arg(readExtCSD.modify_extcsd.boot_wp, 2, 16, QLatin1Char('0')).toUpper();
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());
	extcsdInfo = QString("ExtCSD[175] ERASE_GROUP_DEF : %1").arg(readExtCSD.modify_extcsd.erase_group_def, 2, 16, QLatin1Char('0')).toUpper();
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());
	extcsdInfo = QString("ExtCSD[177] BOOT_BUS_CONDITIONS : %1").arg(readExtCSD.modify_extcsd.boot_bus_conditions, 2, 16, QLatin1Char('0')).toUpper();
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());
	extcsdInfo = QString("ExtCSD[178] BOOT_CONFIG_PORT : %1").arg(readExtCSD.modify_extcsd.boot_config_prot, 2, 16, QLatin1Char('0')).toUpper();
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());
	extcsdInfo = QString("ExtCSD[179] PARTITION_CONFIG : %1").arg(readExtCSD.modify_extcsd.partition_config, 2, 16, QLatin1Char('0')).toUpper();
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());
	extcsdInfo = QString("ExtCSD[226] BOOT_SIZE_MULT : %1").arg(readExtCSD.modify_extcsd.boot_size_mult, 2, 16, QLatin1Char('0')).toUpper();
	ALOG_INFO("%s", "CU", "--", extcsdInfo.toStdString().c_str());
	ALOG_INFO("----------------------------------------------------------------", "CU", "--");
}

void ACProjManager::GetPartitionSize(QString partName, QString partSize, QString UnitSize, eMMCOPTION_Modify& csdInfo)
{
	int nPartSize = partSize.toInt();
	//if (UnitSize != "MBytes") {// 如果是KB，那么一定是1024 kb * x的整数倍，出错由底层返回
	//	nPartSize %= 1024;

	//	if (nPartSize == 0) {
	//		nPartSize = 1;
	//	}
	//}

	if (partName == "GPP1") {
		csdInfo.modify_part.GPP1Size = nPartSize;
	}
	else if (partName == "GPP2") {
		csdInfo.modify_part.GPP2Size = nPartSize;
	}
	else if (partName == "GPP3") {
		csdInfo.modify_part.GPP3Size = nPartSize;
	}
	else if (partName == "GPP4") {
		csdInfo.modify_part.GPP4Size = nPartSize;
	}
	else if (partName == "BOOT") {
		if (UnitSize == "KBytes") {
			csdInfo.modify_part.BootSize = nPartSize;
		}
		else {
			int kbSize = partSize.toInt();
			kbSize *= 1024;	//转为KB
			int nNum = 0;
			if (kbSize % EMMC_COMMON_BOOT_SIZE == 0)
				nNum = kbSize / EMMC_COMMON_BOOT_SIZE;
			else
				nNum = (kbSize / EMMC_COMMON_BOOT_SIZE) + 1;

			csdInfo.modify_part.BootSize = nNum * EMMC_COMMON_BOOT_SIZE;
		}
	}
	else if (partName == "RPMB") {
		if (UnitSize == "KBytes") {
			csdInfo.modify_part.RPMBSize = nPartSize;
		}
		else {
			int kbSize = partSize.toInt();
			kbSize *= 1024;	//转为KB
			int nNum = 0;
			if (kbSize % EMMC_COMMON_BOOT_SIZE == 0)
				nNum = kbSize / EMMC_COMMON_BOOT_SIZE;
			else
				nNum = (kbSize / EMMC_COMMON_BOOT_SIZE) + 1;

			csdInfo.modify_part.RPMBSize = nNum * EMMC_COMMON_BOOT_SIZE;
		}
	}
	//else if (partName == "ENHANCED") {
	//	csdInfo.modify_part.EnhancedUserSize = nPartSize;
	//}
}

void ACProjManager::GetExtCSDReg(QString regAddr, QString regValue, QString regName, eMMCOPTION_Modify& csdInfo)
{
	bool bOk = false;
	if (regAddr == "167") {//WR_REL_SET
		csdInfo.modify_extcsd.wr_rel_set = regValue.toInt(&bOk, 16);
	}
	else if (regAddr == "169") {//FW_CONFIG
		csdInfo.modify_extcsd.fw_config = regValue.toInt(&bOk, 16);
	}
	else if (regAddr == "173") {//BOOT_WP
		csdInfo.modify_extcsd.boot_wp = regValue.toInt(&bOk, 16);
	}
	else if (regAddr == "179") {//PARTITION_CFG
		csdInfo.modify_extcsd.partition_config = regValue.toInt(&bOk, 16);
	}
	else if (regAddr == "178") {//BOOT_CONFIG_PROT
		csdInfo.modify_extcsd.boot_config_prot = regValue.toInt(&bOk, 16);
	}
	else if (regAddr == "177") {//BOOT_BUS_CONDITIONS
		csdInfo.modify_extcsd.boot_bus_conditions = regValue.toInt(&bOk, 16);
	}
	else if (regAddr == "162") {//RST_n_FUNCTION
		csdInfo.modify_extcsd.rst_n_function = regValue.toInt(&bOk, 16);
	}
	else if (regAddr == "163") {//BKOPS_EN
		csdInfo.modify_extcsd.bkops_en = regValue.toInt(&bOk, 16);
	}
	else if (regAddr == "155") {//PARTITION_SETTING_COMPLETED
		csdInfo.modify_extcsd.partition_setting_completed = regValue.toInt(&bOk, 16);
	}
	else if (regAddr == "156") {//PARTITIONS_ATTRIBUTE
		csdInfo.modify_extcsd.partitions_attribute = regValue.toInt(&bOk, 16);
	}
	else if (regAddr == "175") {//ERASE_GROUP_DEF
		csdInfo.modify_extcsd.erase_group_def = regValue.toInt(&bOk, 16);
	}
	else if (regAddr == "171") {//USER_WP
		csdInfo.modify_extcsd.user_wp = regValue.toInt(&bOk, 16);
	}
	else if (regAddr == "53:52" || regAddr == "52") {//EXT_PARTITIONS_ATTRIBUITE
		for (int i = 0; i < regValue.size() / 2; i++) {
			bool bOK;
			QString twoDigits = regValue.mid(i * 2, 2); // 获取两位数字的子字符串
			csdInfo.modify_extcsd.ext_partitions_attribute[i] = twoDigits.toUInt(&bOK, 16); // 将子字符串转换为整数并存储
		}
	}
	else if (regAddr == "134") {//SEC_BAD_BLK_MGMNT
		csdInfo.modify_extcsd.sec_bad_blk_mgmnt = regValue.toInt(&bOk, 16);
	}
	else if (regAddr == "16") {//SECURE_REMOVAL_TYPE
		csdInfo.modify_extcsd.secure_removal_type = regValue.toInt(&bOk, 16);
	}
	else if (regAddr == "139:136" || regAddr == "136") {//ENH_START_ADDR
		for (int i = 0; i < regValue.size() / 2; ++i) {
			bool bOK;
			QString twoDigits = regValue.mid(i * 2, 2); // 获取两位数字的子字符串
			csdInfo.modify_extcsd.enh_start_addr[i] = twoDigits.toUInt(&bOK, 16); // 将子字符串转换为整数并存储
		}
	}
	else if (regAddr == "142:140" || regAddr == "140") {//ENH_SIZE_MULT
		for (int i = 0; i < regValue.size() / 2; ++i) {
			bool bOK;
			QString twoDigits = regValue.mid(i * 2, 2); // 获取两位数字的子字符串
			csdInfo.modify_extcsd.enh_size_mult[i] = twoDigits.toUInt(&bOK, 16); // 将子字符串转换为整数并存储
		}
	}
}


uint64_t ACProjManager::CalcBinFilesSize() {
	std::vector<tSeptBineMMCFileMap> tbinFileMapInfoVec;
	m_pAnalyzeMgr->UnSerialvFiletoJson(m_pProjFile->getFilePath(), tbinFileMapInfoVec, m_pProjDataset->GeteMMCIntelligentJson().toStdString());
	uint64_t projBinSize = 0;
	for (int i = 0; i < tbinFileMapInfoVec.size(); i++) {
		if (tbinFileMapInfoVec[i].PartName == "acBinXml")
			continue;
		for (int j = 0; j < tbinFileMapInfoVec[i].vFiles.size(); j++) {
			projBinSize += static_cast<uint64_t>(tbinFileMapInfoVec[i].vFiles[j].BlockNum) * EMMC_COMMON_BLOCK_SIZE;
		}
	}
	return projBinSize;
}

std::vector<tSeptBineMMCFileMap> ACProjManager::GetBinFileMapInfoVec() {
	std::vector<tSeptBineMMCFileMap> tbinFileMapInfoVec;
	m_pAnalyzeMgr->UnSerialvFiletoJson(m_pProjFile->getFilePath(), tbinFileMapInfoVec, m_pProjDataset->GeteMMCIntelligentJson().toStdString());
	return tbinFileMapInfoVec;
}