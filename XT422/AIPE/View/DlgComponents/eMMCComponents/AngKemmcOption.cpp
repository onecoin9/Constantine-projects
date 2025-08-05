#include "AngKemmcOption.h"
#include "ui_AngKemmcOption.h"
#include "StyleInit.h"
#include "AngKAddeMMCFile.h"
#include "AngKAddeMMCHuaweiFile.h"
#include "AngKCommonTools.h"
#include "AngKProjDataset.h"
#include "AngKExtendCSD.h"
#include "AngKMessageHandler.h"
#include "ProgressDialogSingleton.h"
#include "MessageNotify/notifymanager.h"
#include "AngKTransmitSignals.h"
#include "XmlDefine.h"
#include "AngKemmCDriverPara.h"
#include "AngKDeviceModel.h"
#include "ACEventLogger.h"
#include "AngKGlobalInstance.h"
#include "ACMessageBox.h"
#include <QStandardItem>
#include <QFileDialog>
#include <QToolTip>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QSemaphore>
#include "Thread/ThreadPool.h"

extern Acro::Thread::ThreadPool g_ThreadPool;

int COMMON_BOOT_SIZE = 128;//单位kb
int COMMON_BLOCK_SIZE = 512;
int SSD_MULTIPLE = 4096;

int calc_crc16sum(unsigned char* buf, unsigned int size, unsigned short* pCRC16Sum);

// 在头文件中定义
#define LOG_WARN(msg, ...) ALOG_WARN(msg, "CU", "--", ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) ALOG_ERROR(msg, "CU", "--", ##__VA_ARGS__)
#define LOG_FATAL(msg, ...) ALOG_FATAL(msg, "CU", "--", ##__VA_ARGS__)
#define LOG_INFO(msg, ...) ALOG_INFO(msg, "CU", "--", ##__VA_ARGS__)

AngKemmcOption::AngKemmcOption(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::AngKemmcOption())
	, m_fileTableModel(nullptr)
	, m_sortFilter(nullptr)
	, m_driverParaSettings(nullptr)
	, m_nImageFileType(BinFiles)
	, m_pAnalyzeMgr(std::make_unique<ACeMMCAnalyzeManager>(this))
{
	
	ui->setupUi(this);

	InitText();
	InitButton();
	InitTable();
	InitSetting();

	m_vecFileReocrd.clear();

	this->setObjectName("AngKemmcOption");
	QT_SET_STYLE_SHEET(objectName());
}

AngKemmcOption::~AngKemmcOption()
{
	if (m_fileTableModel) {
		m_fileTableModel->removeRows(0, m_fileTableModel->rowCount());
	}
	m_vecFileReocrd.clear();
	delete ui;
}

void AngKemmcOption::InitText()
{
	ui->addButton->setText(tr("Add"));
	ui->modifyButton->setText(tr("Modify"));
	ui->deleteButton->setText(tr("Delete"));
	ui->batchAddButton->setText(tr("Batch Add"));

	ui->BasicSetting->setTitle(tr("Basic Setting"));
}

void AngKemmcOption::InitButton()
{
	connect(ui->addButton, &QPushButton::clicked, this, &AngKemmcOption::onSlotAddFile);
	connect(ui->modifyButton, &QPushButton::clicked, this, &AngKemmcOption::onSlotModifyFile);
	connect(ui->deleteButton, &QPushButton::clicked, this, &AngKemmcOption::onSlotDeleteFile);
	connect(ui->batchAddButton, &QPushButton::clicked, this, &AngKemmcOption::onSlotBatchAddFile);

	connect(this, &AngKemmcOption::sigACXMLChipIDUpdated,
		&AngKTransmitSignals::GetInstance(), &AngKTransmitSignals::sigOption2PropertyACXMLChipID);
	//connect(ui->ExtendedCSDButton, &QPushButton::clicked, this, &AngKemmcOption::onSlotExtendedCSDFile);
}

void AngKemmcOption::InitTable()
{
	m_fileTableModel = new QStandardItemModel(this);

	// 创建一个QSortFilterProxyModel并设置排序规则
	//QSortFilterProxyModel proxyModel;
	m_sortFilter = new QSortFilterProxyModel(this);
	m_sortFilter->setSourceModel(m_fileTableModel);
	m_sortFilter->setSortRole(Qt::UserRole);
	m_sortFilter->setDynamicSortFilter(true);
	m_sortFilter->sort(1);

	// 隐藏水平表头
	ui->fileTableView->verticalHeader()->setVisible(false);
	ui->fileTableView->setMouseTracking(true);
	connect(ui->fileTableView, &AngKTableView::entered, this, [=](QModelIndex modelIdx) {
		if (!modelIdx.isValid()) {
			return;

		}
		QToolTip::showText(QCursor::pos(), modelIdx.data().toString());

		});

	QStringList headList;
	headList << tr("No.") << tr("File Full Path") << tr("Partition Name") << tr("Start Address") << tr("File Size") << tr("Sector Align") << tr("CheckSum");

	m_fileTableModel->setHorizontalHeaderLabels(headList);
	ui->fileTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui->fileTableView->setModel(m_sortFilter);
	ui->fileTableView->setSortingEnabled(true);
	ui->fileTableView->setAlternatingRowColors(true);
	ui->fileTableView->horizontalHeader()->setHighlightSections(false);
	ui->fileTableView->horizontalHeader()->setStretchLastSection(true);
	ui->fileTableView->horizontalHeader()->setSortIndicator(1, Qt::AscendingOrder);
	ui->fileTableView->horizontalHeader()->setSortIndicatorShown(true);
	ui->fileTableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	ui->fileTableView->horizontalHeader()->setMinimumSectionSize(110);

	QHeaderView* manuHead = ui->fileTableView->horizontalHeader();

	manuHead->setSectionResizeMode(QHeaderView::Interactive);
	ui->fileTableView->setColumnWidth(0, 10);  // 修改 No. 列宽度为 10
	ui->fileTableView->setColumnWidth(1, 300);  // File Full Path 列宽度保持 300

}

void AngKemmcOption::InitSetting()
{
	m_driverParaSettings = new QSettings(Utils::AngKPathResolve::localReportTempFilePath() + "eMMCDrvPara.ini", QSettings::IniFormat);
	m_driverParaSettings->setIniCodec("utf-8");
}

void AngKemmcOption::InsertTable(eMMCFileInfo& efInfo)
{
	if (efInfo.sFilePath.empty())
		return;

	int row = m_fileTableModel->rowCount();
	m_fileTableModel->insertRow(row);
    
    // 添加序号列
    m_fileTableModel->setData(m_fileTableModel->index(row, eFileIndex), row + 1);
	m_fileTableModel->setData(m_fileTableModel->index(row, eFilePath), QString::fromStdString(efInfo.sFilePath));
	m_fileTableModel->setData(m_fileTableModel->index(row, eFilePartitionName), QString::fromStdString(efInfo.sPartitionName));
	m_fileTableModel->setData(m_fileTableModel->index(row, eFilePartitionName), TranslatePartType(QString::fromStdString(efInfo.sPartitionName)), Qt::UserRole);
	m_fileTableModel->setData(m_fileTableModel->index(row, eFileStartAddress), QString("0x%1").arg(efInfo.nStartAddr, 10, 16, QLatin1Char('0')));
	m_fileTableModel->setData(m_fileTableModel->index(row, eFileSize), QString("0x%1").arg(efInfo.nFileSize, 10, 16, QLatin1Char('0')));
	QString sectorAlign = efInfo.bSectorAlign ? "YES" : "NO";
	m_fileTableModel->setData(m_fileTableModel->index(row, eFileSectorAlign), sectorAlign);
	m_fileTableModel->setData(m_fileTableModel->index(row, eFileCheckSum), QString("0x%1").arg(efInfo.nCheckSum, 8, 16, QLatin1Char('0')));

	//记录选择的文件
	eMMCFileRecord eFileRecord;
	eFileRecord.fileArea = QString::fromStdString(efInfo.sPartitionName);
	eFileRecord.fileSize = efInfo.nFileSize;
	eFileRecord.StartAddr = efInfo.nStartAddr;
	m_vecFileReocrd.push_back(eFileRecord);
}

void AngKemmcOption::setProjDataset(AngKProjDataset* _projData)
{
	m_projDataset = _projData;
}

void AngKemmcOption::ModifyTable(eMMCFileInfo& efInfo)
{
	QModelIndex modIdx = ui->fileTableView->currentIndex();
	m_fileTableModel->setData(m_fileTableModel->index(modIdx.row(), eFilePath), QString::fromStdString(efInfo.sFilePath));
	m_fileTableModel->setData(m_fileTableModel->index(modIdx.row(), eFilePartitionName), QString::fromStdString(efInfo.sPartitionName));
	m_fileTableModel->setData(m_fileTableModel->index(modIdx.row(), eFileStartAddress), QString("0x%1").arg(efInfo.nStartAddr, 10, 16, QLatin1Char('0')));
	m_fileTableModel->setData(m_fileTableModel->index(modIdx.row(), eFileSize), QString("0x%1").arg(efInfo.nFileSize, 10, 16, QLatin1Char('0')));
	QString sectorAlign = efInfo.bSectorAlign ? "YES" : "NO";
	m_fileTableModel->setData(m_fileTableModel->index(modIdx.row(), eFileSectorAlign), sectorAlign);
	m_fileTableModel->setData(m_fileTableModel->index(modIdx.row(), eFileCheckSum), QString("0x%1").arg(efInfo.nCheckSum, 8, 16, QLatin1Char('0')));

	//recordVec需要同步修改
	eMMCFileRecord eFileRecord;
	eFileRecord.fileArea = QString::fromStdString(efInfo.sPartitionName);
	eFileRecord.fileSize = efInfo.nFileSize;
	eFileRecord.StartAddr = efInfo.nStartAddr;
	m_vecFileReocrd[modIdx.row()] = eFileRecord;
	//bool bOK = false;
	//int64_t delStartAddr = m_sortFilter->data(m_sortFilter->index(modIdx.row(), eFileStartAddress)).toString().toLongLong(&bOK, 16);
	//QString delFileArea = m_sortFilter->data(m_sortFilter->index(modIdx.row(), eFilePartitionName)).toString();
	//int64_t delFileSize = m_sortFilter->data(m_sortFilter->index(modIdx.row(), eFileSize)).toString().toLongLong(&bOK, 16);
	//for (int i = 0; i < m_vecFileReocrd.size(); ++i) {
	//	if (m_vecFileReocrd[i].StartAddr == delStartAddr
	//		&& m_vecFileReocrd[i].fileArea == delFileArea
	//		&& m_vecFileReocrd[i].fileSize == delFileSize) {
	//		break;
	//	}
	//}

	eMMCFileDataJsonSerial efileJson;
	efileJson.serialize(efInfo);
	int vecSize = m_projDataset->geteMMCFileData().size();
	if(modIdx.row() < vecSize)
		m_projDataset->geteMMCFileData()[modIdx.row()] = efileJson;
}

int AngKemmcOption::ParserACXml(QString acFile, QString binFile)
{
	if (acFile.isEmpty() || binFile.isEmpty()) {
		LOG_FATAL("Parser ACXml failed : acxml or acimg is empty.");
		return XMLMESSAGE_LOAD_FAILED;
	}

	pugi::xml_document doc;
	const wchar_t* encodedName = reinterpret_cast<const wchar_t*>(acFile.utf16());
	pugi::xml_parse_result result = doc.load_file(encodedName);

	if (!result)
		return XMLMESSAGE_LOAD_FAILED;

	//解析<ExtCSD>
	pugi::xml_node root_node = doc.child(XML_ROOTNODE_EMMC);
	pugi::xml_node extCSD_Node = root_node.child(XML_NODE_EMMC_EXTCSD);
	int extCSDCount = std::distance(extCSD_Node.begin(), extCSD_Node.end());
	pugi::xml_node extCSD_SubNode = extCSD_Node.child(XML_NODE_EMMC_PARTITIONSIZE);
	for (int i = 0; i < extCSDCount; ++i)
	{
		if (strcmp(extCSD_SubNode.name(), XML_NODE_EMMC_PARTITIONSIZE) == 0) {
			QString strName = extCSD_SubNode.attribute("Name").as_string();
			QString strSize = extCSD_SubNode.attribute("Size").as_string();
			QString strUnit = extCSD_SubNode.attribute("Unit").as_string();
			GetPartitionSize(strName, strSize, strUnit, m_configExtCSD);
		}
		else if (strcmp(extCSD_SubNode.name(), XML_NODE_EMMC_Reg) == 0) {
			QString strAddr = extCSD_SubNode.attribute("Addr").as_string();
			QString strValue = extCSD_SubNode.attribute("Value").as_string();
			QString strName = extCSD_SubNode.attribute("Name").as_string();
			GetExtCSDReg(strAddr, strValue, strName, m_configExtCSD);
		}

		extCSD_SubNode = extCSD_SubNode.next_sibling();
	}

	//解析<Partitions>
	pugi::xml_node partitions_Node = root_node.child(XML_NODE_EMMC_PARTITIONS);
	int partitionsCount = std::distance(partitions_Node.begin(), partitions_Node.end());
	pugi::xml_node entry_Node = partitions_Node.child(XML_NODE_EMMC_ENTRY);
	std::vector<tHuaweiACFile> hwACFileVec;
	for (int i = 0; i < partitionsCount; ++i) {

		tHuaweiACFile hwEntry;
		bool bOK = false;
		hwEntry.FileBlockPos = QString::fromStdString(entry_Node.attribute("FileBlockPos").as_string()).toInt(&bOK, 16);
		hwEntry.dwBlkStart = QString::fromStdString(entry_Node.attribute("ChipBlockPos").as_string()).toInt(&bOK, 16);
		hwEntry.dwBlkNum = QString::fromStdString(entry_Node.attribute("BlockNum").as_string()).toInt(&bOK, 16);
		hwEntry.CRC16 = QString::fromStdString(entry_Node.attribute("CRC16").as_string()).toUShort(&bOK, 16);
		hwEntry.PartName = entry_Node.attribute("Partition").as_string();
		hwACFileVec.push_back(hwEntry);

		entry_Node = entry_Node.next_sibling();
	}
	//解析分区byteSum
	pugi::xml_node checkSum_Node = root_node.child(XML_NODE_EMMC_CHECKSUM);
	pugi::xml_node byteSum_Node = checkSum_Node.child(XML_NODE_EMMC_BYTESUM);
	int byteSumCount = std::distance(byteSum_Node.begin(), byteSum_Node.end());
	pugi::xml_node partition_Node = byteSum_Node.child(XML_NODE_EMMC_PARTITION);
	std::map<std::string, std::string> byteSumMap;
	for (int i = 0; i < byteSumCount; ++i){

		byteSumMap[partition_Node.attribute("Name").as_string()] = partition_Node.attribute("Sum").as_string();
		partition_Node = partition_Node.next_sibling();
	}

	return DealhwACXML(hwACFileVec, m_configExtCSD, byteSumMap);
}

int AngKemmcOption::ParserACXml(const QString& proj_path, std::vector<MMAKEFILE>& makeFiles, std::string& strImgType, std::string& IntelligentJson, uint64_t& nBytesum)
{
	int ret = 0;
	QString acXmlFile;
	QString acBinFile;
	if (makeFiles.size() == 2) {
		acXmlFile = QString::fromStdString(makeFiles[0].strFileName);
		acBinFile = QString::fromStdString(makeFiles[1].strFileName);
	}
	else {
		LOG_ERROR("Invalid makeFiles size");
		ret = -1;
		return ret;
	}

	bool bOK = false;

	int nVFileIndex = 1;
	int nBlockByte = 512;
	//将智能分析完的文件重新构成统一的Json，用于直接下发到eMMC中
	nlohmann::json eMMCHeaderPartitionJson;
	eMMCHeaderPartitionJson["PlatformType"] = strImgType;

	//用户添加的xxx.bin文件
	nlohmann::json importFilesJson = nlohmann::json::array();

	//该文件智能分析后的文件Entry
	nlohmann::json vFilemapsJson = nlohmann::json::array();

	pugi::xml_document doc;
	const wchar_t* encodedName = reinterpret_cast<const wchar_t*>(acXmlFile.utf16());
	pugi::xml_parse_result result = doc.load_file(encodedName);

	if (!result)
		return XMLMESSAGE_LOAD_FAILED;

	//对烧录档案的acbin进行设置保存
	for (int i = 0; i < makeFiles.size(); ++i) {
		QFile binFile(QString::fromStdString(makeFiles[i].strFileName));
		if (!binFile.open(QIODevice::ReadOnly)) {
			ret = -1;
			return ret;
		}
		nlohmann::json _imFileJson;
		_imFileJson["Files"] = Utils::AngKCommonTools::Full2RelativePath(QString::fromStdString(makeFiles[i].strFileName), proj_path);
		_imFileJson["LastModify"] = makeFiles[i].lastModifyTime;
		_imFileJson["PartIndex"] = 0;
		_imFileJson["FileSize"] = binFile.size();
		_imFileJson["VFileIndex"] = i + 1;
		_imFileJson["SectorAlign"] = false;
		_imFileJson["StartAddress"] = 0;	//acxml已经是只能分析后到每个分区的，不再需要记录对应分区的起始地址，VFileEntry中都已经包含
		if (i == 0) {
			_imFileJson["PartName"] = "acBinXml";
			_imFileJson["EntryCnt"] = 0;
			_imFileJson["CheckSum"] = 0;
		}
		else if (i == 1) {
			_imFileJson["PartName"] = "acBinFile";

			//解析分区byteSum
			pugi::xml_node root_node = doc.child(XML_ROOTNODE_EMMC);
			pugi::xml_node checkSum_Node = root_node.child(XML_NODE_EMMC_CHECKSUM);
			pugi::xml_node byteSum_Node = checkSum_Node.child(XML_NODE_EMMC_BYTESUM);
			int byteSumCount = std::distance(byteSum_Node.begin(), byteSum_Node.end());
			pugi::xml_node partition_Node = byteSum_Node.child(XML_NODE_EMMC_PARTITION);
			std::map<std::string, std::string> byteSumMap;
			for (int j = 0; j < byteSumCount; ++j) {
				std::string strSumValue = partition_Node.attribute("Sum").as_string();
				nBytesum += QString::fromStdString(strSumValue).toUInt(&bOK, 16);
				partition_Node = partition_Node.next_sibling();
			}
			_imFileJson["CheckSum"] = nBytesum;

			//解析<Partitions>
			pugi::xml_node partitions_Node = root_node.child(XML_NODE_EMMC_PARTITIONS);
			int partitionsCount = std::distance(partitions_Node.begin(), partitions_Node.end());
			pugi::xml_node entry_Node = partitions_Node.child(XML_NODE_EMMC_ENTRY);
			_imFileJson["EntryCnt"] = partitionsCount;
			for (int k = 0; k < partitionsCount; ++k) {
				nlohmann::json _vFileJson;
				_vFileJson["Entry"] = entry_Node.attribute("Name").as_string();
				_vFileJson["FileBlockPos"] = entry_Node.attribute("FileBlockPos").as_string();
				qint64 nFileBolckPos = QString::fromStdString(entry_Node.attribute("FileBlockPos").as_string()).toUInt(&bOK, 16);
				_vFileJson["ChipBlockPos"] = entry_Node.attribute("ChipBlockPos").as_string();
				_vFileJson["BlockNum"] = QString::fromStdString(entry_Node.attribute("BlockNum").as_string()).toUInt(&bOK, 16);
				_vFileJson["CRCType"] = 1;
				_vFileJson["CRC16"] = QString::fromStdString(entry_Node.attribute("CRC16").as_string()).toUShort(&bOK, 16);
				_vFileJson["PartName"] = entry_Node.attribute("Partition").as_string();
				_vFileJson["VFileIndex"] = i + 1;
				_vFileJson["PartIndex"] = (int)TranslatePartType(QString::fromStdString(entry_Node.attribute("Partition").as_string()));
				binFile.seek(nFileBolckPos * 512);
				QByteArray readSrc_1K = binFile.read(1024);
				ushort srcFeture = 0;
				calc_crc16sum((uchar*)readSrc_1K.constData(), readSrc_1K.size(), &srcFeture);
				_vFileJson["Feature"] = srcFeture;

				vFilemapsJson.push_back(_vFileJson);
				entry_Node = entry_Node.next_sibling();

			}

		}

		importFilesJson.push_back(_imFileJson);
		binFile.close();
	}

	eMMCHeaderPartitionJson["ImportFiles"] = importFilesJson;
	eMMCHeaderPartitionJson["VFilemap"] = vFilemapsJson;

	IntelligentJson = eMMCHeaderPartitionJson.dump();
	return ret;
}

int AngKemmcOption::ParserACXmlChipID(QString acFile)
{
	LOG_FATAL("ParserChipID ACXml acFile: %s.", acFile.toStdString().c_str());
	if (acFile.isEmpty()) {
		LOG_FATAL("ParserChipID ACXml failed : acxml or acimg is empty.");
		return XMLMESSAGE_LOAD_FAILED;
	}

	pugi::xml_document doc;
	const wchar_t* encodedName = reinterpret_cast<const wchar_t*>(acFile.utf16());
	pugi::xml_parse_result result = doc.load_file(encodedName);

	if (!result)
		return XMLMESSAGE_LOAD_FAILED;

	//解析<ExtCSD>
	pugi::xml_node root_node = doc.child(XML_ROOTNODE_EMMC);
	pugi::xml_node extCSD_Node = root_node.child(XML_NODE_EMMC_EXTCSD);
	int extCSDCount = std::distance(extCSD_Node.begin(), extCSD_Node.end());

	//解析<ChipID>
	pugi::xml_node chipID_Node = root_node.child(XML_NODE_EMMC_CHIP);
	QString strACXMLChipID = chipID_Node.attribute("Value").as_string();
	std::string strDBChipID = m_projDataset->getChipData().getJsonValue<std::string>("chipId");
	if (strACXMLChipID.toStdString() != strDBChipID && strDBChipID != DEFAULT_CHIPID && strACXMLChipID != "") {
		QString tips = tr("The ChipID in ACXML (%1) does not match the database ChipID (%2). Do you want to continue to use (%3)?")
			.arg(strACXMLChipID)
			.arg(QString::fromStdString(strDBChipID))
			.arg(QString::fromStdString(strDBChipID));
		ACMessageBox::ACMsgType ret = ACMessageBox::showWarning(this, tr("Warning"), tips,
			ACMessageBox::ACMsgButton::MSG_OK_CANCEL_BTN);
		if (ret == ACMessageBox::ACMsgType::MSG_CANCEL)
		{
			QString logTips = tr("The ChipID in ACXML (%1) does not match the database ChipID (%2).User choose exit.")
				.arg(strACXMLChipID)
				.arg(QString::fromStdString(strDBChipID));
			LOG_WARN(logTips.toStdString().c_str());
			return XMLMESSAGE_LOAD_FAILED;
		}
		else {
			emit sigACXMLChipIDUpdated(strACXMLChipID.toStdString());
			QString logTips = tr("The ChipID in ACXML (%1) does not match the database ChipID (%2).User choose use (%3).")
				.arg(strACXMLChipID)
				.arg(QString::fromStdString(strDBChipID)).arg(QString::fromStdString(strDBChipID));
			LOG_INFO(logTips.toStdString().c_str());
		}
	}
	else {
		emit sigACXMLChipIDUpdated(strACXMLChipID.toStdString());
	}

	return XMLMESSAGE_SUCCESS;
}

int AngKemmcOption::ParserExtCSD(QString acFile)
{
	if (acFile.isEmpty()) {
		LOG_FATAL("Parser ACXml failed : acxml or acimg is empty.");
		return XMLMESSAGE_LOAD_FAILED;
	}

	pugi::xml_document doc;
	const wchar_t* encodedName = reinterpret_cast<const wchar_t*>(acFile.utf16());
	pugi::xml_parse_result result = doc.load_file(encodedName);

	if (!result)
		return XMLMESSAGE_LOAD_FAILED;

	//解析<ExtCSD>
	pugi::xml_node root_node = doc.child(XML_ROOTNODE_EMMC);
	pugi::xml_node extCSD_Node = root_node.child(XML_NODE_EMMC_EXTCSD);
	int extCSDCount = std::distance(extCSD_Node.begin(), extCSD_Node.end());

	if (extCSDCount != 0) {
		emit sgnSetCheckExtCSD(true);
	}

	emit sgnParserACXML(acFile);

	return XMLMESSAGE_SUCCESS;
}

void AngKemmcOption::SaveCurProgCheckState(std::map<std::string, uint16_t> progCheck)
{
	m_progCheck = progCheck;
}

int AngKemmcOption::GetFileCount()
{
	return m_fileTableModel->rowCount();
}

void AngKemmcOption::SetArchivesFile()
{
	std::vector<eMMCFileDataJsonSerial>& veceMMCFile = m_projDataset->geteMMCFileData();

	m_vecFileReocrd.clear();
	m_fileTableModel->removeRows(0, m_fileTableModel->rowCount());
	for (auto eMMCFileJson : veceMMCFile) {

		eMMCFileInfo efInfo;
		eMMCFileJson.deserialize(efInfo);

		if (efInfo.sFilePath.empty())
			return;

		int row = m_fileTableModel->rowCount();
		m_fileTableModel->insertRow(row);
		
		// 添加序号列
		m_fileTableModel->setData(m_fileTableModel->index(row, eFileIndex), row + 1);
		m_fileTableModel->setData(m_fileTableModel->index(row, eFilePath), QString::fromStdString(efInfo.sFilePath));
		m_fileTableModel->setData(m_fileTableModel->index(row, eFilePartitionName), QString::fromStdString(efInfo.sPartitionName));
		m_fileTableModel->setData(m_fileTableModel->index(row, eFilePartitionName), TranslatePartType(QString::fromStdString(efInfo.sPartitionName)), Qt::UserRole);
		m_fileTableModel->setData(m_fileTableModel->index(row, eFileStartAddress), QString("0x%1").arg(efInfo.nStartAddr, 10, 16, QLatin1Char('0')));
		m_fileTableModel->setData(m_fileTableModel->index(row, eFileSize), QString("0x%1").arg(efInfo.nFileSize, 10, 16, QLatin1Char('0')));
		QString sectorAlign = efInfo.bSectorAlign ? "YES" : "NO";
		m_fileTableModel->setData(m_fileTableModel->index(row, eFileSectorAlign), sectorAlign);
		m_fileTableModel->setData(m_fileTableModel->index(row, eFileCheckSum), QString("0x%1").arg(efInfo.nCheckSum, 8, 16, QLatin1Char('0')));

		//记录选择的文件
		eMMCFileRecord eFileRecord;
		eFileRecord.fileArea = QString::fromStdString(efInfo.sPartitionName);
		eFileRecord.fileSize = efInfo.nFileSize;
		eFileRecord.StartAddr = efInfo.nStartAddr;
		m_vecFileReocrd.push_back(eFileRecord);
	}

	//加载eMMCOption的配置信息
	std::string extCSDOptionJson = m_projDataset->GeteMMCOptionJson().toStdString();
	try {
		auto extJson = nlohmann::json::parse(extCSDOptionJson); 
		//ui->ImageFileFormatComboBox->setCurrentIndex(extJson["imageFileFormat"].get<int>());
	}
	catch (const nlohmann::json::exception& e) {
		LOG_FATAL("eMMCOption Json parse failed : %s.", e.what());
	}

	//加载eMMCExtCSD的信息
	std::string extCSDJson = m_projDataset->GeteMMCExtCSDParaJson().toStdString();
	try {
		if (!extCSDJson.empty()) {
			auto extJson = nlohmann::json::parse(extCSDJson);
			nlohmann::json regConfigJson = extJson["RegConfig"];
			nlohmann::json partitonSizeJson = extJson["PartitionSize"];

			for (int i = 0; i < regConfigJson.size(); ++i)
			{
				int nAddr = regConfigJson[i]["Addr"];
				std::string strName = regConfigJson[i]["Name"];
				std::string strValue = regConfigJson[i]["Value"];
				GetExtCSDReg(QString::number(nAddr), QString::fromStdString(strValue), QString::fromStdString(strName), m_configExtCSD);
			}

			for (int j = 0; j < partitonSizeJson.size(); ++j)
			{
				uint32_t nSize = partitonSizeJson[j]["Size"];
				std::string strName = partitonSizeJson[j]["Name"];
				std::string strUnit = partitonSizeJson[j]["Unit"];
				GetPartitionSize(QString::fromStdString(strName), QString::number(nSize), QString::fromStdString(strUnit), m_configExtCSD);
			}
		}
	}
	catch (const nlohmann::json::exception& e) {
			LOG_FATAL("eMMCExtCSDPara Json parse failed : %s.", e.what());
	}
}

/*******************************************
@brief 确认是否存在文件相互覆盖，不允许文件相互覆盖
@param[in] efInfo		需要新加入的文件
@param[in] Idx			新加入的文件的索引，如果是Add，则传入-1.如果是Modify，传入被改变时的index
@return
	TRUE:有文件相互覆盖
	FALSE:没有文件相互覆盖
*********************************************/
bool AngKemmcOption::CheckOverlapped(eMMCFileInfo& efInfo, int Idx)
{
	// 这段代码用于检查文件是否有地址重叠的情况
	// 遍历文件表格中的所有行
	for (int i = 0; i < m_fileTableModel->rowCount(); ++i) {
		// 不和指定的Idx比对
		if (i == Idx) {
			continue;
		}

		// 获取当前行的分区名称
		QString checkFileArea = m_sortFilter->data(m_sortFilter->index(i, eFilePartitionName)).toString();

		// 如果分区名称不同,跳过此次比较
		if (checkFileArea.toStdString() != efInfo.sPartitionName) {
			continue;
		}

		bool bOk;
		// 获取当前行的起始地址和文件大小
		int64_t checkStartAddr = m_sortFilter->data(m_sortFilter->index(i, eFileStartAddress)).toString().toLongLong(&bOk, 16);
		int64_t checkFileSize = m_sortFilter->data(m_sortFilter->index(i, eFileSize)).toString().toLongLong(&bOk, 16);

		// 检查地址范围是否重叠
		// 如果新文件的结束地址小于等于已有文件的起始地址,或新文件的起始地址大于等于已有文件的结束地址,则不重叠
		if (efInfo.nStartAddr + efInfo.nFileSize <= checkStartAddr || efInfo.nStartAddr >= checkStartAddr + checkFileSize) {
			continue;
		}
		else {
			ACMessageBox::showWarning(this, QObject::tr("Warning"), QObject::tr("Sorry, Find the range of file is overlapped with the File: %1").arg(QString::fromStdString(efInfo.sFilePath)));
			return true;
		}
	}
	return false;
}

std::string AngKemmcOption::GetExtendCSDJson()
{
	//EXTCSD 部分
	nlohmann::json regConfigJson = nlohmann::json::array();
	nlohmann::json partitonSizeJson = nlohmann::json::array();
	nlohmann::json EXTCSDJson;
	SwitchRegConfigJson(m_configExtCSD.modify_extcsd, regConfigJson);
	SwitchPartitionSizeJson(m_configExtCSD.modify_part, partitonSizeJson);

	EXTCSDJson["RegConfig"] = regConfigJson;
	EXTCSDJson["PartitionSize"] = partitonSizeJson;

	return EXTCSDJson.dump();
}

std::string AngKemmcOption::GetOptionJson()
{
	nlohmann::json optionJson;

	//optionJson["imageFileFormat"] = ui->ImageFileFormatComboBox->currentIndex();

	return optionJson.dump();
}

void AngKemmcOption::SetImageFileType(ImageFileType imageType)
{
	if (imageType != ImageFileType::ACIMG) {
		emit sigACXMLChipIDUpdated("");
	}
	//if (m_nImageFileType != imageType) {
		//m_fileTableModel->removeRows(0, m_fileTableModel->rowCount());
	//}
	m_nImageFileType = imageType;
}

int AngKemmcOption::StartIntelligentAnalyze(const QString& proj_path, ImageFileType _imgType, int _chkType, QString& strBytesum)
{
	int ret = 0;
	std::vector<MMAKEFILE> vecMAKEFile;
	GetMMAKEFile(vecMAKEFile, _imgType);
	if (vecMAKEFile.empty())//没选择档案也允许保存工程
		return ret;

	LOG_INFO("Start Intelligent Analyze.");
	QString imgName;
	std::string IntelligentJson;
	uint64_t nBytesum = 0;
	switch (_imgType)
	{
	case BinFiles:
		imgName = "BinFiles";
		ret = m_pAnalyzeMgr->ImgFmtSPBinParserFiles(vecMAKEFile, _chkType, "", this);
		if (ret == 0) {
			m_projDataset->geteMMCFileData().clear();
			for (int i = 0; i < vecMAKEFile.size(); ++i) {
				//校验值回填
				eMMCFileInfo efInfo;
				efInfo.nCheckSum = vecMAKEFile[i].CheckSum;
				efInfo.sFilePath = vecMAKEFile[i].strFileName;
				efInfo.sPartitionName = vecMAKEFile[i].strPartName;
				efInfo.nStartAddr = vecMAKEFile[i].StartAddr;
				efInfo.nFileSize = vecMAKEFile[i].Size;
				efInfo.bSectorAlign = vecMAKEFile[i].IsSectorAlign;

				eMMCFileDataJsonSerial efileJson;
				efileJson.serialize(efInfo);
				m_projDataset->geteMMCFileData().push_back(efileJson);
				if (_imgType == BinFiles)
					m_fileTableModel->setData(m_fileTableModel->index(i, eFileCheckSum), QString("0x%1").arg(vecMAKEFile[i].CheckSum, 8, 16, QLatin1Char('0')));

				nBytesum += efInfo.nCheckSum;
			}
			m_pAnalyzeMgr->SerialvFiletoJson(proj_path, vecMAKEFile, imgName, IntelligentJson);
		}
		break;
	case MTK:
		imgName = "MTK";
		break;
	case Ambarella:
		imgName = "Ambarella";
		break;
	case Phison:
		imgName = "Phison";
		break;
	case CIMG:
		imgName = "CIMG";
		break;
	case Huawei:
		imgName = "Huawei";
		break;
	case ACIMG:
		imgName = "ACIMG";
		if (vecMAKEFile.size() == 2) {
			ParserACXml(proj_path, vecMAKEFile, imgName.toStdString(), IntelligentJson, nBytesum);
			ret = CalOtherTypeImageFileChecksum(_chkType, vecMAKEFile);
		}
		break;
	default:
		break;
	}

	if (ret == 0) {
		m_projDataset->SeteMMCIntelligentJson(QString::fromStdString(IntelligentJson));
		strBytesum = QString("0x%1").arg(nBytesum, 8, 16, QLatin1Char('0')).toUpper();
	}
	LOG_INFO("Intelligent Analyze End.");
	return ret;
}

void AngKemmcOption::GetEMMCHeaderInfo(const QString& proj_path, eMMCTableHeader& tableHeader, std::string& tableJson, std::string& emmcOptonInfo)
{
	std::vector<tSeptBineMMCFileMap> tbinFileMapInfoVec;
	m_pAnalyzeMgr->UnSerialvFiletoJson(proj_path, tbinFileMapInfoVec, m_projDataset->GeteMMCIntelligentJson().toStdString());

	nlohmann::json eMMC_Json;
	QByteArray dataStruct;//存放所有数据
	QByteArray partitionTable;
	//eMMCTableHeader tableHeader;
	memset(&tableHeader, 0, sizeof(tableHeader));

	//Feature 部分
	nlohmann::json featureJson;
	featureJson["ByteSumEn"] = 1;
	featureJson["CRC16En"] = 1;
	featureJson["DataOffsetInSSD"] = "0x14000000";

	//EXTCSD 部分
	nlohmann::json regConfigJson = nlohmann::json::array();
	nlohmann::json partitonSizeJson = nlohmann::json::array();
	nlohmann::json EXTCSDJson;
	if (!emmcOptonInfo.empty()) {
		try {
			auto eventJson = nlohmann::json::parse(emmcOptonInfo);
			EXTCSDJson["RegConfig"] = eventJson["RegConfig"];
			EXTCSDJson["PartitionSize"] = eventJson["PartitionSize"];
		}
		catch (nlohmann::json::exception& e) {
			EXTCSDJson["RegConfig"] = nlohmann::json::array();
			EXTCSDJson["PartitionSize"] = nlohmann::json::array();
			LOG_FATAL("GetEMMCHeaderInfo Parse EXTCSD Json error : %s", e.what());
		}
	}
	else {
		EXTCSDJson["RegConfig"] = nlohmann::json::array();
		EXTCSDJson["PartitionSize"] = nlohmann::json::array();
	}

	//ByteSum 部分
	nlohmann::json byteSumJson = nlohmann::json::array();
	std::string curParttion = "";
	std::map<std::string, uint64_t> partByteSumMap;//用于统计整体bytesum
	for (int i = 0; i < tbinFileMapInfoVec.size(); ++i)
	{
		tSeptBineMMCFileMap tBinFile = tbinFileMapInfoVec[i];

		if (tBinFile.PartName == "acBinXml")
			continue;

		nlohmann::json calByteSum;
		calByteSum["Partition"] = tBinFile.PartName;
		calByteSum["Sum"] = QString("0x%1").arg(tBinFile.CheckSum, 8, 16, QLatin1Char('0')).toStdString();
		byteSumJson.push_back(calByteSum);
	}
	std::string test = byteSumJson.dump();

	//Files 部分
	nlohmann::json filesJson = nlohmann::json::array();
	uint64_t ssdAddr = 0;// SSD_WRITE_PARTITION_DATA;
	std::vector<uint64_t> vecSSD_Offset;
	uint32_t allFileTotalBlockSize = 0;
	for (int i = 0; i < tbinFileMapInfoVec.size(); ++i)
	{
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
			uint64_t dataRealSize = (uint64_t)tVFile.BlockNum * 512;
			uint64_t last_Blk = 0;
			if (dataRealSize % 4096 == 0)
				last_Blk = dataRealSize / 4096;
			else
				last_Blk = (dataRealSize / 4096) + 1;
			ssdAddr += last_Blk * 4096;
			//ssdAddr += (uint64_t)tVFile.BlockNum * 512;
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
	memcpy(tableHeader.MagicType, strMagic.c_str(), strMagic.size());
	tableHeader.PartitionTableSize = eMMC_Json.dump().size();

	ushort headerChk = 0;
	calc_crc16sum((uchar*)(eMMC_Json.dump().c_str()), eMMC_Json.dump().size(), &headerChk);
	tableHeader.PartitionTableCRC = headerChk;

	headerChk = 0;
	calc_crc16sum((uchar*)(&tableHeader) + 2, sizeof(tableHeader) - 2, &headerChk);
	tableHeader.HeaderCRC = headerChk;

	tableJson = eMMC_Json.dump();
}

uint64_t AngKemmcOption::hexStr2Int(QString strHex)
{
	//当前emmc表格地址的特殊性，所以可以不判断直接删除前两位
	strHex.remove(0, 2);

	bool bOk;
	uint64_t ret = strHex.toULongLong(&bOk, 16);
	return ret;
}

PartitionType AngKemmcOption::TranslatePartType(QString strName)
{
	if (strName == "USER") {
		return PartitionType::USER;
	}
	else if (strName == "BOOT1") {
		return PartitionType::BOOT1;
	}
	else if (strName == "BOOT2") {
		return PartitionType::BOOT2;
	}
	else if (strName == "RPMB") {
		return PartitionType::RPMB;
	}
	else if (strName == "GPP1") {
		return PartitionType::GPP1;
	}
	else if (strName == "GPP2") {
		return PartitionType::GPP2;
	}
	else if (strName == "GPP3") {
		return PartitionType::GPP3;
	}
	else if (strName == "GPP4") {
		return PartitionType::GPP4;
	}

	return PartitionType::None;
}

QString AngKemmcOption::TranslatePartIdx(PartitionType partIdx)
{
	QString partName;
	switch (partIdx)
	{
	case USER:
		partName = "USER";
		break;
	case BOOT1:
		partName = "BOOT1";
		break;
	case BOOT2:
		partName = "BOOT2";
		break;
	case RPMB:
		partName = "RPMB";
		break;
	case GPP1:
		partName = "GPP1";
		break;
	case GPP2:
		partName = "GPP2";
		break;
	case GPP3:
		partName = "GPP3";
		break;
	case GPP4:
		partName = "GPP4";
		break;
	case None:
	default:
		break;
	}
	return partName;
}

void AngKemmcOption::SwitchRegConfigJson(UI_CFG_EXTCSD cfgEXTCSD, nlohmann::json& regCfgJson)
{
	if (cfgEXTCSD.partition_config != 0)
	{
		nlohmann::json partCfgJson;
		partCfgJson["Addr"] = 179;
		partCfgJson["Value"] = QString("%1").arg(cfgEXTCSD.partition_config, 2, 16, QLatin1Char('0')).toStdString();
		partCfgJson["Name"] = "PARTITION_CONFIG";
		regCfgJson.push_back(partCfgJson);
	}
	if (cfgEXTCSD.boot_config_prot != 0)
	{
		nlohmann::json partCfgJson;
		partCfgJson["Addr"] = 178;
		partCfgJson["Value"] = QString("%1").arg(cfgEXTCSD.boot_config_prot, 2, 16, QLatin1Char('0')).toStdString();
		partCfgJson["Name"] = "BOOT_CONFIG_PROT";
		regCfgJson.push_back(partCfgJson);
	}
	if (cfgEXTCSD.boot_bus_conditions != 0)
	{
		nlohmann::json partCfgJson;
		partCfgJson["Addr"] = 177;
		partCfgJson["Value"] = QString("%1").arg(cfgEXTCSD.boot_bus_conditions, 2, 16, QLatin1Char('0')).toStdString();
		partCfgJson["Name"] = "BOOT_BUS_CONDITIONS";
		regCfgJson.push_back(partCfgJson);
	}
	if (cfgEXTCSD.erase_group_def != 0)
	{
		nlohmann::json partCfgJson;
		partCfgJson["Addr"] = 175;
		partCfgJson["Value"] = QString("%1").arg(cfgEXTCSD.erase_group_def, 2, 16, QLatin1Char('0')).toStdString();
		partCfgJson["Name"] = "ERASE_GROUP_DEF";
		regCfgJson.push_back(partCfgJson);
	}
	if (cfgEXTCSD.boot_wp != 0)
	{
		nlohmann::json partCfgJson;
		partCfgJson["Addr"] = 173;
		partCfgJson["Value"] = QString("%1").arg(cfgEXTCSD.boot_wp, 2, 16, QLatin1Char('0')).toStdString();
		partCfgJson["Name"] = "BOOT_WP";
		regCfgJson.push_back(partCfgJson);
	}
	if (cfgEXTCSD.user_wp != 0)
	{
		nlohmann::json partCfgJson;
		partCfgJson["Addr"] = 171;
		partCfgJson["Value"] = QString("%1").arg(cfgEXTCSD.user_wp, 2, 16, QLatin1Char('0')).toStdString();
		partCfgJson["Name"] = "USER_WP";
		regCfgJson.push_back(partCfgJson);
	}
	if (cfgEXTCSD.fw_config != 0)
	{
		nlohmann::json partCfgJson;
		partCfgJson["Addr"] = 169;
		partCfgJson["Value"] = QString("%1").arg(cfgEXTCSD.fw_config, 2, 16, QLatin1Char('0')).toStdString();
		partCfgJson["Name"] = "FW_CONFIG";
		regCfgJson.push_back(partCfgJson);
	}
	if (cfgEXTCSD.wr_rel_set != 0)
	{
		nlohmann::json partCfgJson;
		partCfgJson["Addr"] = 167;
		partCfgJson["Value"] = QString("%1").arg(cfgEXTCSD.wr_rel_set, 2, 16, QLatin1Char('0')).toStdString();
		partCfgJson["Name"] = "WR_REL_SET";
		regCfgJson.push_back(partCfgJson);
	}
	if (cfgEXTCSD.bkops_en != 0)
	{
		nlohmann::json partCfgJson;
		partCfgJson["Addr"] = 163;
		partCfgJson["Value"] = QString("%1").arg(cfgEXTCSD.bkops_en, 2, 16, QLatin1Char('0')).toStdString();
		partCfgJson["Name"] = "BKOPS_EN";
		regCfgJson.push_back(partCfgJson);
	}
	if (cfgEXTCSD.rst_n_function != 0)
	{
		nlohmann::json partCfgJson;
		partCfgJson["Addr"] = 162;
		partCfgJson["Value"] = QString("%1").arg(cfgEXTCSD.rst_n_function, 2, 16, QLatin1Char('0')).toStdString();
		partCfgJson["Name"] = "RST_n_FUNCTION";
		regCfgJson.push_back(partCfgJson);
	}
	if (cfgEXTCSD.partitions_attribute != 0)
	{
		nlohmann::json partCfgJson;
		partCfgJson["Addr"] = 156;
		partCfgJson["Value"] = QString("%1").arg(cfgEXTCSD.partitions_attribute, 2, 16, QLatin1Char('0')).toStdString();
		partCfgJson["Name"] = "PARTITIONS_ATTRIBUTE";
		regCfgJson.push_back(partCfgJson);
	}
	if (cfgEXTCSD.partition_setting_completed != 0)
	{
		nlohmann::json partCfgJson;
		partCfgJson["Addr"] = 155;
		partCfgJson["Value"] = QString("%1").arg(cfgEXTCSD.partition_setting_completed, 2, 16, QLatin1Char('0')).toStdString();
		partCfgJson["Name"] = "PARTITION_SETTING_COMPLETED";
		regCfgJson.push_back(partCfgJson);
	}
	if (cfgEXTCSD.sec_bad_blk_mgmnt != 0)
	{
		nlohmann::json partCfgJson;
		partCfgJson["Addr"] = 134;
		partCfgJson["Value"] = QString("%1").arg(cfgEXTCSD.sec_bad_blk_mgmnt, 2, 16, QLatin1Char('0')).toStdString();
		partCfgJson["Name"] = "SEC_BAD_BLK_MGMNT";
		regCfgJson.push_back(partCfgJson);
	}
	if (cfgEXTCSD.secure_removal_type != 0)
	{
		nlohmann::json partCfgJson;
		partCfgJson["Addr"] = 16;
		partCfgJson["Value"] = QString("%1").arg(cfgEXTCSD.secure_removal_type, 2, 16, QLatin1Char('0')).toStdString();
		partCfgJson["Name"] = "secure_removal_type";
		regCfgJson.push_back(partCfgJson);
	}

	size_t arrayLength = sizeof(cfgEXTCSD.ext_partitions_attribute) / sizeof(cfgEXTCSD.ext_partitions_attribute[0]);
	QString ext_partitions_attributeSTR = QString::fromUtf8(reinterpret_cast<const char*>(cfgEXTCSD.ext_partitions_attribute), static_cast<int>(arrayLength));
	if (!isAllZeros(ext_partitions_attributeSTR))
	{
		// 将数组中的字节顺序反转
		INT8U reversed_addr[2] = { cfgEXTCSD.ext_partitions_attribute[1], cfgEXTCSD.ext_partitions_attribute[0] };

		// 将反转后的数据转换为字符串
		QString reversed_str = QString("%1%2").arg(cfgEXTCSD.ext_partitions_attribute[0], 2, 16, QChar('0'))
			.arg(cfgEXTCSD.ext_partitions_attribute[1], 2, 16, QChar('0'));

		nlohmann::json partCfgJson;
		partCfgJson["Addr"] = 52;
		partCfgJson["Value"] = reversed_str.toStdString();
		partCfgJson["Name"] = "EXT_PARTITIONS_ATTRIBUTE";
		regCfgJson.push_back(partCfgJson);
	}

	arrayLength = sizeof(cfgEXTCSD.enh_start_addr) / sizeof(cfgEXTCSD.enh_start_addr[0]);
	QString enh_start_addrSTR = QString::fromUtf8(reinterpret_cast<const char*>(cfgEXTCSD.enh_start_addr), static_cast<int>(arrayLength));
	if (!isAllZeros(enh_start_addrSTR))
	{
		// 将数组中的字节顺序反转
		INT8U reversed_addr[4] = { cfgEXTCSD.enh_start_addr[3], cfgEXTCSD.enh_start_addr[2], cfgEXTCSD.enh_start_addr[1], cfgEXTCSD.enh_start_addr[0] };

		// 将反转后的数据转换为字符串
		QString reversed_str = QString("%1%2%3%4").arg(cfgEXTCSD.enh_start_addr[0], 2, 16, QChar('0'))
			.arg(cfgEXTCSD.enh_start_addr[1], 2, 16, QChar('0'))
			.arg(cfgEXTCSD.enh_start_addr[2], 2, 16, QChar('0'))
			.arg(cfgEXTCSD.enh_start_addr[3], 2, 16, QChar('0'));

		nlohmann::json partCfgJson;
		partCfgJson["Addr"] = 136;
		partCfgJson["Value"] = reversed_str.toStdString();
		partCfgJson["Name"] = "ENH_START_ADDR";
		regCfgJson.push_back(partCfgJson);
	}

	arrayLength = sizeof(cfgEXTCSD.enh_size_mult) / sizeof(cfgEXTCSD.enh_size_mult[0]);
	QString enh_size_multSTR = QString::fromUtf8(reinterpret_cast<const char*>(cfgEXTCSD.enh_size_mult), static_cast<int>(arrayLength));
	if (!isAllZeros(enh_size_multSTR))
	{
		// 将数组中的字节顺序反转
		INT8U reversed_addr[3] = { cfgEXTCSD.enh_size_mult[2], cfgEXTCSD.enh_size_mult[1], cfgEXTCSD.enh_size_mult[0] };

		// 将反转后的数据转换为字符串
		QString reversed_str = QString("%1%2%3").arg(cfgEXTCSD.enh_size_mult[0], 2, 16, QChar('0'))
			.arg(cfgEXTCSD.enh_size_mult[1], 2, 16, QChar('0'))
			.arg(cfgEXTCSD.enh_size_mult[2], 2, 16, QChar('0'));

		nlohmann::json partCfgJson;
		partCfgJson["Addr"] = 140;
		partCfgJson["Value"] = reversed_str.toStdString();
		partCfgJson["Name"] = "ENH_SIZE_MULT";
		regCfgJson.push_back(partCfgJson);
	}
}

void AngKemmcOption::SwitchPartitionSizeJson(PartitionSizeModify cfgEXTCSD, nlohmann::json& partSizeJson)
{
	if (cfgEXTCSD.EnhancedUserSize != 0)
	{
		nlohmann::json partJson;
		partJson["Name"] = "ENHANCED";
		partJson["Size"] = cfgEXTCSD.EnhancedUserSize;
		partJson["Unit"] = "MBytes";
		partSizeJson.push_back(partJson);
	}
	if (cfgEXTCSD.GPP1Size != 0)
	{
		nlohmann::json partJson;
		partJson["Name"] = "GPP1";
		partJson["Size"] = cfgEXTCSD.GPP1Size;
		partJson["Unit"] = "MBytes";
		partSizeJson.push_back(partJson);
	}
	if (cfgEXTCSD.GPP2Size != 0)
	{
		nlohmann::json partJson;
		partJson["Name"] = "GPP2";
		partJson["Size"] = cfgEXTCSD.GPP2Size;
		partJson["Unit"] = "MBytes";
		partSizeJson.push_back(partJson);
	}
	if (cfgEXTCSD.GPP3Size != 0)
	{
		nlohmann::json partJson;
		partJson["Name"] = "GPP3";
		partJson["Size"] = cfgEXTCSD.GPP3Size;
		partJson["Unit"] = "MBytes";
		partSizeJson.push_back(partJson);
	}
	if (cfgEXTCSD.GPP4Size != 0)
	{
		nlohmann::json partJson;
		partJson["Name"] = "GPP4";
		partJson["Size"] = cfgEXTCSD.GPP4Size;
		partJson["Unit"] = "MBytes";
		partSizeJson.push_back(partJson);
	}
	if (cfgEXTCSD.BootSize != 0)
	{
		nlohmann::json partJson;
		partJson["Name"] = "BOOT";
		partJson["Size"] = cfgEXTCSD.BootSize;
		partJson["Unit"] = "KBytes";
		partSizeJson.push_back(partJson);
	}
	if (cfgEXTCSD.RPMBSize != 0)
	{
		nlohmann::json partJson;
		partJson["Name"] = "RPMB";
		partJson["Size"] = cfgEXTCSD.RPMBSize;
		partJson["Unit"] = "KBytes";
		partSizeJson.push_back(partJson);
	}
}

bool AngKemmcOption::isAllZeros(const QString& str) {
	for (int i = 0; i < str.length(); ++i) {
		if (str[i] != '\0') {
			return false; // 如果存在非零字符则返回false
		}
	}
	return true; // 若没有非零字符则返回true
}

void AngKemmcOption::GetPartitionSize(QString partName, QString partSize, QString UnitSize, eMMCOPTION_Modify& csdInfo)
{
	int nPartSize = partSize.toInt();
	//if (UnitSize != "MBytes") {// 如果是KB，那么一定是1024 kb * x的整数倍，出错由底层返回
	//	nPartSize /= 1024;

	//	if (nPartSize == 0){
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
			if (kbSize % COMMON_BOOT_SIZE == 0)
				nNum = kbSize / COMMON_BOOT_SIZE;
			else
				nNum = (kbSize / COMMON_BOOT_SIZE) + 1;

			csdInfo.modify_part.BootSize = nNum * COMMON_BOOT_SIZE;
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
			if (kbSize % COMMON_BOOT_SIZE == 0)
				nNum = kbSize / COMMON_BOOT_SIZE;
			else
				nNum = (kbSize / COMMON_BOOT_SIZE) + 1;

			csdInfo.modify_part.RPMBSize = nNum * COMMON_BOOT_SIZE;
		}
	}
	//else if (partName == "ENHANCED") {
	//	csdInfo.modify_part.EnhancedUserSize = nPartSize;
	//}
}

void AngKemmcOption::GetExtCSDReg(QString regAddr, QString regValue, QString regName, eMMCOPTION_Modify& csdInfo)
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
		for (int i = 0; i < regValue.size() / 2; ++i) {
			QString twoDigits = regValue.mid(i * 2, 2); // 获取两位数字的子字符串
			csdInfo.modify_extcsd.ext_partitions_attribute[i] = twoDigits.toUInt(&bOk, 16); // 将子字符串转换为整数并存储
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

int AngKemmcOption::DealhwACXML(std::vector<tHuaweiACFile> hwFileVec, eMMCOPTION_Modify csdInfo, std::map<std::string, std::string> byteMap)
{
	int nRet = 0;
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
	nlohmann::json regConfigJson = nlohmann::json::array();
	nlohmann::json partitonSizeJson = nlohmann::json::array();
	nlohmann::json EXTCSDJson;
	SwitchRegConfigJson(csdInfo.modify_extcsd, regConfigJson);
	SwitchPartitionSizeJson(csdInfo.modify_part, partitonSizeJson);

	EXTCSDJson["RegConfig"] = regConfigJson;
	EXTCSDJson["PartitionSize"] = partitonSizeJson;

	//ByteSum 部分
	nlohmann::json byteSumJson = nlohmann::json::array();
	bool bOk = false;
	for (auto byteIter : byteMap) {
		uint32_t nByteSum = QString::fromStdString(byteIter.second).toInt(&bOk, 16);
		if (nByteSum != 0) {
			nlohmann::json bySum;
			bySum["Partition"] = byteIter.first;
			bySum["Sum"] = byteIter.second;
			byteSumJson.push_back(bySum);
		}
	}

	//Files 部分
	nlohmann::json filesJson = nlohmann::json::array();
	uint64_t ssdAddr = SSD_WRITE_PARTITION_DATA;
	std::vector<uint64_t> vecSSD_Offset;
	int EntryIdx = 1;
	uint32_t allFileTotalBlockSize = 0;
	for (auto hwFile : hwFileVec)
	{
		nlohmann::json entryFile;
		entryFile["BlockNum"] = hwFile.dwBlkNum;
		entryFile["CRC16"] = QString("0x%1").arg(hwFile.CRC16, 8, 16, QLatin1Char('0')).toStdString();
		entryFile["ChipBlockPos"] = hwFile.dwBlkStart;
		entryFile["Entry"] = EntryIdx;
		entryFile["Partition"] = hwFile.PartName;
		entryFile["SSDOffset"] = QString("0x%1").arg(ssdAddr, 8, 16, QLatin1Char('0')).toStdString();
		ssdAddr += hwFile.dwBlkNum * COMMON_BLOCK_SIZE;
		allFileTotalBlockSize += hwFile.dwBlkNum;
		EntryIdx++;
		filesJson.push_back(entryFile);
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

	if (nByte % SSD_MULTIPLE == 0)
		a_Blk = nByte / SSD_MULTIPLE;
	else
		a_Blk = (nByte / SSD_MULTIPLE) + 1;

	uint64_t realSize = a_Blk * SSD_MULTIPLE <= 4096 ? 4096 : a_Blk * SSD_MULTIPLE;

	partitionTable.resize(realSize - sizeof(eHeader));
	partitionTable.fill(0);
	partitionTable.replace(0, eMMC_Json.dump().size(), eMMC_Json.dump().c_str());

	QByteArray tableByte;
	tableByte.append((char*)&eHeader, sizeof(eHeader));
	tableByte.append(partitionTable);

	//QFile tempFIle(QCoreApplication::applicationDirPath() + "/partitionTableHeaderhuawei.bin");
	//if (tempFIle.open(QIODevice::ReadWrite | QIODevice::Truncate))
	//{
	//	tempFIle.write(tableByte);
	//}
	//tempFIle.close();

	std::map<std::string, DeviceStu> insertDev;
	AngKDeviceModel::instance().GetConnetDevMap(insertDev);
	for (auto prog : m_progCheck) {
		QStringList IPHopList = QString::fromStdString(prog.first).split(":");
		int nTryReSend = AngKGlobalInstance::instance()->ReadValue("DeviceComm", "Retransmission").toInt();
		ALOG_INFO("Send eMMC file data to %s:%d SSD", "CU", "FP", IPHopList[0].toStdString().c_str(), IPHopList[1].toInt());
		int TableRet = 0;
		while (nTryReSend != 0)
		{
			TableRet = AngKMessageHandler::instance().Command_StoreDataToSSDorDDR(IPHopList[0].toStdString(), "FIBER2SSD", IPHopList[1].toInt(), 0, SSD_WRITE_PARTITION_TABLE, tableByte, insertDev[prog.first].tMainBoardInfo.strHardwareSN, "eMMCFileData");
			TableRet = AngKMessageHandler::instance().Command_StoreDataToSSDorDDR(IPHopList[0].toStdString(), "FIBER2SSD", IPHopList[1].toInt(), 0, SSD_WRITE_PARTITION_TABLE, tableByte, insertDev[prog.first].tMainBoardInfo.strHardwareSN, "eMMCFileData");

			if (TableRet == 0) {
				NotifyManager::instance().notify(tr("Notify"), tr("Write SSD PARTITION Table Complete"));
				break;
			}
			else {
				ALOG_FATAL("Send eMMC file data to %s:%d SSD failed(errorCode: %d)", "CU", "FP", IPHopList[0].toStdString().c_str(), IPHopList[1].toInt(), TableRet);
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

void AngKemmcOption::GetMMAKEFile(std::vector<MMAKEFILE>& _vecMMAKEFile, ImageFileType _imgType)
{
	switch (_imgType)
	{
	case BinFiles:
		for (int i = 0; i < m_fileTableModel->rowCount(); ++i) {
			MMAKEFILE emcInfo;
			emcInfo.strFileName = m_fileTableModel->data(m_fileTableModel->index(i, eFilePath)).toString().toStdString();
			emcInfo.strPartName = m_fileTableModel->data(m_fileTableModel->index(i, eFilePartitionName)).toString().toStdString();
			emcInfo.PartIndex = m_fileTableModel->data(m_fileTableModel->index(i, eFilePartitionName), Qt::UserRole).toInt();
			emcInfo.Size = hexStr2Int(m_fileTableModel->data(m_fileTableModel->index(i, eFileSize)).toString());
			emcInfo.StartAddr = hexStr2Int(m_fileTableModel->data(m_fileTableModel->index(i, eFileStartAddress)).toString());
			emcInfo.IsSectorAlign = m_fileTableModel->data(m_fileTableModel->index(i, eFileSectorAlign)).toInt();
			QFileInfo fInfo(m_fileTableModel->data(m_fileTableModel->index(i, eFilePath)).toString());
			emcInfo.lastModifyTime = fInfo.lastModified().toTime_t();
			_vecMMAKEFile.push_back(emcInfo);
		}
		break;
	case MTK:
		break;
	case Ambarella:
		break;
	case Phison:
		break;
	case CIMG:
		break;
	case Huawei:
	case ACIMG:
		for (int i = 0; i < m_fileTableModel->rowCount(); ++i) {
			MMAKEFILE emcInfo;
			emcInfo.strFileName = m_fileTableModel->data(m_fileTableModel->index(i, eFilePath)).toString().toStdString();
			QFileInfo fInfo(m_fileTableModel->data(m_fileTableModel->index(i, eFilePath)).toString());
			emcInfo.lastModifyTime = fInfo.lastModified().toTime_t();
			emcInfo.PartIndex = 0;

			if(i == 0)
				emcInfo.strPartName = "acXmlBin";
			else if(i == 1)
				emcInfo.strPartName = "acXml";

			_vecMMAKEFile.push_back(emcInfo);
		}
		break;
	default:
		break;
	}
}

void AngKemmcOption::onSlotAddFile()
{
	if (m_nImageFileType == ImageFileType::Huawei
		|| m_nImageFileType == ImageFileType::ACIMG) {
		AngKAddeMMCHuaweiFile huaweiDlg(this);
		connect(&huaweiDlg, &AngKAddeMMCHuaweiFile::sgnAddHuaweiFile, this, [=](QString acFile, QString binFile) {
			if (m_fileTableModel->rowCount() >= 2) {
				ACMessageBox::showWarning(this, tr("Add Warning"), tr("ACIMG files and ACXML files do not allow duplicate additions."));
				return;
			}

			if (ParserACXmlChipID(acFile) != XMLMESSAGE_SUCCESS){
				return;
			}

			//huawei特殊档案只需要显示acxml和acimg文件就行
			eMMCFileInfo acInfo;
			acInfo.sFilePath = acFile.toStdString();
			InsertTable(acInfo);
			//eMMCFileDataJsonSerial acInfoJson;
			//acInfoJson.serialize(acInfo);
			//m_projDataset->geteMMCFileData().push_back(acInfoJson);

			eMMCFileInfo binInfo;
			binInfo.sFilePath = binFile.toStdString();
			InsertTable(binInfo);
			//eMMCFileDataJsonSerial binInfoJson;
			//binInfoJson.serialize(binInfo);
			//m_projDataset->geteMMCFileData().push_back(binInfoJson);

			emit sgnRestExtCSDInfo();
			ParserExtCSD(acFile);


			});

		huaweiDlg.exec();
	}
	else if (m_nImageFileType == ImageFileType::BinFiles)
	{
		AngKAddeMMCFile dlg(this);
		dlg.setFileRecord(m_vecFileReocrd);
		connect(&dlg, &AngKAddeMMCFile::sgnAddeMMCFile, this, [=](eMMCFileInfo eInfo) {
			if (!CheckOverlapped(eInfo, -1)) {
				InsertTable(eInfo);
			}
		});

		dlg.exec();
	}

}

void AngKemmcOption::onSlotModifyFile()
{
	if (m_nImageFileType == ImageFileType::Huawei
		|| m_nImageFileType == ImageFileType::ACIMG) {
		ACMessageBox::showWarning(this, tr("Modify Warning"), tr("ACIMG files and ACXML files do not support modification."));
		return;
	}

	QModelIndex modIdx = ui->fileTableView->currentIndex();
	AngKAddeMMCFile dlg(this);
	if (modIdx.isValid()){
		eMMCFileInfo eInfo;
		eInfo.sFilePath = m_fileTableModel->data(m_fileTableModel->index(modIdx.row(), eFilePath)).toString().toStdString();
		eInfo.sPartitionName = m_fileTableModel->data(m_fileTableModel->index(modIdx.row(), eFilePartitionName)).toString().toStdString();
		eInfo.nStartAddr = hexStr2Int(m_fileTableModel->data(m_fileTableModel->index(modIdx.row(), eFileStartAddress)).toString());
		eInfo.nFileSize = hexStr2Int(m_fileTableModel->data(m_fileTableModel->index(modIdx.row(), eFileSize)).toString());
		QString sectorAlign = m_fileTableModel->data(m_fileTableModel->index(modIdx.row(), eFileSectorAlign)).toString();
		eInfo.bSectorAlign = sectorAlign == "YES" ? true : false;
		eInfo.nCheckSum = hexStr2Int(m_fileTableModel->data(m_fileTableModel->index(modIdx.row(), eFileCheckSum)).toString());

		dlg.seteMMCParams(eInfo);
	}
	else
		return;

	connect(&dlg, &AngKAddeMMCFile::sgnAddeMMCFile, this, [=](eMMCFileInfo eInfo) {
		if (!CheckOverlapped(eInfo, modIdx.row())) {
			ModifyTable(eInfo);
		}
		});
	dlg.exec();
}

void AngKemmcOption::onSlotDeleteFile()
{
    QModelIndex modIdx = ui->fileTableView->currentIndex();
    if (modIdx.isValid())
    {
		if (m_nImageFileType == ImageFileType::Huawei
			|| m_nImageFileType == ImageFileType::ACIMG) {
			m_fileTableModel->removeRows(0, m_fileTableModel->rowCount());

			emit sgnRestExtCSDInfo();
			emit sigACXMLChipIDUpdated("");
		}
		else {
			int eraseIdx = -1;
			bool bOK;
			int64_t delStartAddr = m_sortFilter->data(m_sortFilter->index(modIdx.row(), eFileStartAddress)).toString().toLongLong(&bOK, 16);
			QString delFileArea = m_sortFilter->data(m_sortFilter->index(modIdx.row(), eFilePartitionName)).toString();
			int64_t delFileSize = m_sortFilter->data(m_sortFilter->index(modIdx.row(), eFileSize)).toString().toLongLong(&bOK, 16);
			for (int i = 0; i < m_vecFileReocrd.size(); ++i) {
				if (m_vecFileReocrd[i].StartAddr == delStartAddr
					&& m_vecFileReocrd[i].fileArea == delFileArea
					&& m_vecFileReocrd[i].fileSize == delFileSize) {
					eraseIdx = i;
					break;
				}
			}
			if (eraseIdx >= 0) {
				m_sortFilter->removeRow(modIdx.row());
				m_vecFileReocrd.erase(m_vecFileReocrd.begin() + eraseIdx);

				// 更新删除行之后的所有行的序号
				for (int row = modIdx.row(); row < m_fileTableModel->rowCount(); ++row) {
					m_fileTableModel->setData(m_fileTableModel->index(row, eFileIndex), row + 1);
				}
			}
			else {
				LOG_ERROR("Delete eMMC File Failed!");
			}
		}
    }
}

void AngKemmcOption::onSlotBatchAddFile()
{
	if (m_nImageFileType == ImageFileType::Huawei
		|| m_nImageFileType == ImageFileType::ACIMG) {
		ACMessageBox::showWarning(this, tr("Batch Add Warning"), tr("ACIMG files and ACXML files do not support batch addition."));
		return;
	}

    QString xmlPath = QFileDialog::getOpenFileName(
        this,
        tr("Select XML File"),
        QString(),
        tr("XML Files (*.xml);;All Files (*.*)")
    );

    if (!xmlPath.isEmpty())
    {
        if (ParseXML(xmlPath))
        {
            
        }
    }
}

void AngKemmcOption::onSlotExtendedCSDFile()
{
	//AngKExtendCSD dlg(this);
	//connect(&dlg, &AngKExtendCSD::sgnEXTCSDConfig, this, &AngKemmcOption::onSlotDealEXTCSDConfig);

	///*if (ui->ImageFileFormatComboBox->currentData().toInt() == ImageFileType::Huawei)*/ {
	//	dlg.SetHuaweiExtCSD(m_configExtCSD);
	//}

	//dlg.exec();
}

void AngKemmcOption::onSlotDealEXTCSDConfig(eMMCOPTION_Modify csdInfo)
{
	m_configExtCSD = csdInfo;
}

// 添加新函数用于从XML导入文件
bool AngKemmcOption::ParseXML(const QString& xmlPath)
{
    try {
        pugi::xml_document doc;
        if (!doc.load_file(xmlPath.toStdString().c_str()))
        {
            ACMessageBox::showError(this, tr("Error"), tr("Failed to load XML file."));
            return false;
        }

        pugi::xml_node root = doc.child("eMMC");
        if (!root)
        {
            ACMessageBox::showError(this, tr("Error"), tr("Invalid XML format: missing root node 'eMMC'."));
            return false;
        }

    pugi::xml_node importElement = root.child("FilesImport");
    if (!importElement)
    {
        LOG_ERROR("Invalid XML format: missing node 'FilesImport'");
        ACMessageBox::showError(this, tr("Error"), tr("Invalid XML format: missing node 'FilesImport'."));
        return false;
    }

        // 检查PathMode
        pugi::xml_node pathModeNode = importElement.child("PathMode");
        if (!pathModeNode || std::string(pathModeNode.child_value()) != "Relative")
        {
            ACMessageBox::showError(this, tr("Error"), tr("Invalid or missing PathMode: must be 'Relative'."));
            return false;
        }

        // 清空现有数据
        m_vecFileReocrd.clear();
        m_fileTableModel->removeRows(0, m_fileTableModel->rowCount());
        
        // 获取XML文件所在目录路径
        QFileInfo xmlFileInfo(xmlPath);
        QString xmlDir = xmlFileInfo.absolutePath();
        
        int row = 0;
        for (pugi::xml_node fileNode = importElement.child("File"); fileNode; fileNode = fileNode.next_sibling("File"))
        {
            eMMCFileInfo efInfo;
            bool ok = false;

            // 获取File节点的子元素
            pugi::xml_node partitionNode = fileNode.child("Partition");
            pugi::xml_node offsetNode = fileNode.child("Offset");
            pugi::xml_node fileNameNode = fileNode.child("FileName");

            if (!partitionNode || !offsetNode || !fileNameNode)
            {
                LOG_ERROR("Missing required Partition/Offset/FileName elements in File node");
                ACMessageBox::showError(this, tr("Error"), tr("Missing required Partition/Offset/FileName elements in File node"));
                return false;
            }

            // 构建完整的文件路径
            QString fileName = QString::fromStdString(fileNameNode.child_value());
            QString fullPath = QDir(xmlDir).filePath(fileName);
            
            // 解析XML节点数据
            efInfo.sFilePath = fullPath.toStdString();
            efInfo.sPartitionName = partitionNode.child_value();
            
            // 处理偏移量，支持十六进制和十进制
            QString offsetStr = QString::fromStdString(offsetNode.child_value());
            if (offsetStr.startsWith("0x", Qt::CaseInsensitive))
            {
                efInfo.nStartAddr = offsetStr.toULongLong(&ok, 16);
            }
            else {
                LOG_ERROR("Offset value  %s must with 0x", offsetStr.toStdString().c_str());
                ACMessageBox::showError(this, tr("Error"), tr("offset value %1 must with 0x").arg(offsetStr));
                return false;
            }

            if (!ok)
            {
                LOG_ERROR("Invalid offset value: %s", offsetStr.toStdString().c_str());
                ACMessageBox::showError(this, tr("Error"), tr("Invalid offset value: %1").arg(offsetStr));
                return false;
            }

            // 验证文件是否存在并获取实际文件大小
            QFile file(fullPath);
            if (!file.exists())
            {
                LOG_ERROR("File not found: %s", efInfo.sFilePath.c_str());
                ACMessageBox::showError(this, tr("Error"), tr("File not found: %1").arg(QString::fromStdString(efInfo.sFilePath)));
                return false;
            }

            // 获取实际文件大小
            efInfo.nFileSize = file.size();

            // 检查地址重叠
            if (!CheckOverlapped(efInfo, -1))
            {
                InsertTable(efInfo);
                
                // 添加到项目数据集
                eMMCFileDataJsonSerial fileJson;
                fileJson.serialize(efInfo);
                m_projDataset->geteMMCFileData().push_back(fileJson);
                
                row++;
            }
            else
            {
                LOG_ERROR("Overlapped address detected for file: %s", efInfo.sFilePath.c_str());
                ACMessageBox::showError(this, tr("Error"), tr("Overlapped address detected for file: %1").arg(QString::fromStdString(efInfo.sFilePath)));
                return false;
            }
        }

        // 强制更新视图
        m_sortFilter->invalidate();
        ui->fileTableView->reset();
        ui->fileTableView->resizeColumnsToContents();

        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception during XML parsing: %s", e.what());
        ACMessageBox::showError(this, tr("Error"), tr("Failed to parse XML file: %1").arg(e.what()));
        return false;
    }
}

int AngKemmcOption::CalOtherTypeImageFileChecksum(int _chkType, std::vector<MMAKEFILE>& _mmakeFileVec)
{
	int ret = E_OK;
	m_projDataset->geteMMCFileData().clear();
	ProgressDialogSingleton dlgpro(this);
	bool bStop = false;
	bool bAnaErr = false;
	QSemaphore semaphore(0);

	connect(this, &AngKemmcOption::sgnAcxmlAnaUpdateProgress, &dlgpro, &ProgressDialogSingleton::updateProgress);
	connect(&dlgpro, &ProgressDialogSingleton::canceled, [&bStop]() {bStop = true; });
	connect(this, &AngKemmcOption::sgnCLoseProgressDialog, &dlgpro, &ProgressDialogSingleton::onSlotCLoseProgressDialog);


	g_ThreadPool.PushTask([&]() {
		auto endFunc = [&](bool bErr) {
			bAnaErr = bErr;
			semaphore.release();
		};

		for (int i = 0; i < _mmakeFileVec.size(); ++i) {
			eMMCFileInfo efInfo;

			efInfo.sFilePath = _mmakeFileVec[i].strFileName;
			efInfo.nFileSize = _mmakeFileVec[i].Size;
			efInfo.bSectorAlign = _mmakeFileVec[i].IsSectorAlign;

			//efInfo.nCheckSum = _mmakeFileVec[i].CheckSum;
			QFile file(QString::fromStdString(efInfo.sFilePath));
			if (!file.open(QIODevice::ReadOnly)) {
				ALOG_FATAL("Load Project File error, open image file:\"%s\" failed.", "CU", "--", efInfo.sFilePath.data());
				ret = E_OPEN_FILE;
				endFunc(true);
				emit sgnCLoseProgressDialog();
				return;
			}


			CHKINFO DataBuffChecksum;
			memset(&DataBuffChecksum, 0, sizeof(DataBuffChecksum));
			uint64_t fileCurChk = 0;
			uint64_t nFileSize = file.size();
			uint64_t nProgress = 0;
			while (!file.atEnd())
			{
				QByteArray contStr = file.read(1024 * 1024);
				nProgress += 1024 * 1024;
				if (_chkType == 0) {//CRC32
					Crc32CalcSubRoutine(&DataBuffChecksum, (unsigned char*)(contStr.data()), contStr.size());
				}
				else {
					fileCurChk += Utils::AngKCommonTools::GetByteSum((unsigned char*)(contStr.data()), contStr.size());
				}
				float nValue = static_cast<float>(nProgress) / nFileSize * 100;

				emit sgnAcxmlAnaUpdateProgress(nValue);
				if (bStop) {
					ret = E_CANCEL;
					endFunc(true);
					emit sgnCLoseProgressDialog();
					return;
				}
			}

			if (_chkType == 0) {//Bytesum
				Crc32GetChkSum(&DataBuffChecksum);
				fileCurChk = DataBuffChecksum.chksum;
			}

			efInfo.nCheckSum = fileCurChk;

			eMMCFileDataJsonSerial efileJson;
			efileJson.serialize(efInfo);
			m_projDataset->geteMMCFileData().push_back(efileJson);

			file.close();
		}

		endFunc(false);
		//emit sgnCLoseProgressDialog();
		});

	dlgpro.showProgressDialog(100, tr("Calculating ACIMG file checksum, please wait..."), tr("Calculate Checksum"), ProgressDialogSingleton::DLG_EXEC);

	semaphore.acquire();
	dlgpro.closeProgressDialog();

	return ret;
}
