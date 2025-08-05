#include "ACeMMCSolution.h"
#include "ui_ACeMMCSolution.h"
#include "AngKMessageHandler.h"
#include "StyleInit.h"
#include "AngKCustomTab.h"
#include "ACDeviceManager.h"
#include "AngKProjDataset.h"
#include "AngKTransmitSignals.h"
#include "ACMessageBox.h"
#include "AngKGlobalInstance.h"
int calc_crc16sum(unsigned char* buf, unsigned int size, unsigned short* pCRC16Sum);


ACeMMCSolution::ACeMMCSolution(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::ACeMMCSolution())
	, m_pButtonGroup(nullptr)
	, m_pProjData(nullptr)
{
	ui->setupUi(this);

	InitText();
	InitButton();
	InitComboBox();

	this->setObjectName("ACeMMCSolution");
	QT_SET_STYLE_SHEET(objectName());

	// 打开读扩展寄存器功能
	ui->widget_2->setEnabled(true);
}

ACeMMCSolution::~ACeMMCSolution()
{
	delete ui;
}

void ACeMMCSolution::InitText()
{
	ui->ImageFileFormatText->setText(tr("File Format Select:"));
	ui->checksumText->setText(tr("File Checksum Calc:"));
	ui->importExtCSDConfigPushButton->setText(tr("Import ExtCSD Config"));
	ui->readChipPushButton->setText(tr("Read Chip ExtCSD"));
	ui->configTextLabel->setText(tr("Machine and site position:"));
	ui->saveExtCSDConfigButton->setText(tr("Save ExtCSD Config"));

	//设置tabBar样式
	ui->emmcTabWidget->tabBar()->setStyle(new AngKCustomTab(nullptr, 110));
}

void ACeMMCSolution::InitButton()
{
	m_pButtonGroup = new QButtonGroup(this);

	connect(m_pButtonGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), [&](int index) {
		qDebug() << "输入方式为：" << index;
		if (m_pButtonGroup->checkedId() == 0) {
			ui->importExtCSDConfigPushButton->setEnabled(true);
			ui->readChipPushButton->setEnabled(false);
			ui->devInfoWidget->setEnabled(false);
		}
		else {
			ui->importExtCSDConfigPushButton->setEnabled(false);
			ui->readChipPushButton->setEnabled(true);
			ui->devInfoWidget->setEnabled(true);
		}
	});

	m_pButtonGroup->addButton(ui->importExtCSDConfigButton, 0);
	m_pButtonGroup->addButton(ui->readChipButton, 1);

	ui->importExtCSDConfigButton->setChecked(true);
	ui->readChipPushButton->setEnabled(false);
	ui->devInfoWidget->setEnabled(false);

	connect(ui->binFilesPage, &AngKemmcOption::sgnDealEXTCSDConfig, ui->extCSDWidget, &AngKExtendCSD::onSlotDealEXTCSDConfig);
	connect(ui->binFilesPage, &AngKemmcOption::sgnParserACXML, ui->extCSDWidget, &AngKExtendCSD::onSlotParserACXML);
	connect(ui->binFilesPage, &AngKemmcOption::sgnRestExtCSDInfo, ui->extCSDWidget, &AngKExtendCSD::onSlotRestExtCSDInfo);
	connect(ui->binFilesPage, &AngKemmcOption::sgnSetCheckExtCSD, this, [=](bool bCheck) {
		ui->extCSDCheckbox->setChecked(bCheck);
		});
	connect(ui->importExtCSDConfigPushButton, &QPushButton::clicked, ui->extCSDWidget, &AngKExtendCSD::onSlotImportExtCSDConfig);
	connect(ui->readChipPushButton, &QPushButton::clicked, this, &ACeMMCSolution::onSlotReadChipExtCSD);
	connect(ui->saveExtCSDConfigButton, &QPushButton::clicked, ui->extCSDWidget, &AngKExtendCSD::onSlotSaveExtendCSDConfig);
	connect(this, &ACeMMCSolution::sgnFetchedExtCSD, ui->extCSDWidget, &AngKExtendCSD::onSlotExtCSDInfoFetched);
}

void ACeMMCSolution::InitComboBox()
{
	ui->ImageFileFormatComboBox->addItem("Separated Bin Files", ImageFileType::BinFiles);
	ui->ImageFileFormatComboBox->addItem("MTK", ImageFileType::MTK);
	ui->ImageFileFormatComboBox->addItem("CIMG", ImageFileType::CIMG);
	ui->ImageFileFormatComboBox->addItem("Huawei", ImageFileType::Huawei);
	ui->ImageFileFormatComboBox->addItem("ACIMG", ImageFileType::ACIMG);

	//设备座子选项
	InitSKTComboBox(true);

	//设备信息选项
	QVector<DeviceStu> vecDev = ACDeviceManager::instance().getAllDevInfo();
	for (auto dev : vecDev) {
		QString IPHop = QString::fromStdString(dev.strIP) + "-" + QString::number(dev.nHopNum);
		QString devName = IPHop + "-" + QString::fromStdString(dev.strSiteAlias);
		ui->deviceInfoComboBox->addItem(devName, IPHop);
	}

	//校验值类型
	ui->checksumComboBox->addItem("CRC32", 0);
	ui->checksumComboBox->addItem("ByteSum", 1);
	ui->checksumComboBox->setCurrentIndex(1);

	connect(&AngKTransmitSignals::GetInstance(), &AngKTransmitSignals::sigHandleEventTransmitChipIDFetched,
		this, &ACeMMCSolution::onSlotChipIDFetched);
	connect(&AngKTransmitSignals::GetInstance(), &AngKTransmitSignals::sigHandleEventTransmitExtCSDFetched,
		this, &ACeMMCSolution::onSlotExtCSDFetched);

	connect(ui->ImageFileFormatComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ACeMMCSolution::onSlotImageFileSelect);
	connect(ui->extCSDCheckbox, &QCheckBox::stateChanged, [this]() {
		if (!ui->extCSDCheckbox->isChecked()) {
			ui->emmcTabWidget->setTabVisible(1, false);
			ui->emmcTabWidget->setCurrentIndex(0);
		}
		else {
			ui->emmcTabWidget->setTabVisible(1, true);
		}
		});

	ui->emmcTabWidget->setTabVisible(1, false);
	ui->emmcTabWidget->setCurrentIndex(0);
}

void ACeMMCSolution::InitSKTComboBox(bool bDouble)
{
	if (bDouble) {
		ui->deviceSktComboBox->addItem("SKT1.A", 1);
		ui->deviceSktComboBox->addItem("SKT1.B", 2);
		ui->deviceSktComboBox->addItem("SKT2.A", 4);
		ui->deviceSktComboBox->addItem("SKT2.B", 8);
		ui->deviceSktComboBox->addItem("SKT3.A", 16);
		ui->deviceSktComboBox->addItem("SKT3.B", 32);
		ui->deviceSktComboBox->addItem("SKT4.A", 64);
		ui->deviceSktComboBox->addItem("SKT4.B", 128);
		ui->deviceSktComboBox->addItem("SKT5.A", 256);
		ui->deviceSktComboBox->addItem("SKT5.B", 512);
		ui->deviceSktComboBox->addItem("SKT6.A", 1024);
		ui->deviceSktComboBox->addItem("SKT6.B", 2048);
		ui->deviceSktComboBox->addItem("SKT7.A", 4096);
		ui->deviceSktComboBox->addItem("SKT7.B", 8192);
		ui->deviceSktComboBox->addItem("SKT8.A", 16384);
		ui->deviceSktComboBox->addItem("SKT8.B", 32768);
	}
}

int ACeMMCSolution::GetFileCount()
{
	return ui->binFilesPage->GetFileCount();
}

void ACeMMCSolution::SetProjectData(AngKProjDataset* _projData)
{
	ui->binFilesPage->setProjDataset(_projData);
	m_pProjData = _projData;
}

std::string ACeMMCSolution::GeteMMCOption()
{
	nlohmann::json optionJson;

	optionJson["imageFileFormat"] = ui->ImageFileFormatComboBox->currentIndex();
	optionJson["imageFileChecksumType"] = ui->checksumComboBox->currentIndex();
	optionJson["imageFileExtCSDEnable"] = ui->extCSDCheckbox->isChecked();

	return optionJson.dump();
}

std::string ACeMMCSolution::GeteMMCExtCSDOption()
{
	return ui->extCSDWidget->GetExtendCSDJson();
}

void ACeMMCSolution::SetArchivesFile()
{
	ui->binFilesPage->SetArchivesFile();

	//加载eMMCOption的配置信息
	std::string extCSDOptionJson = m_pProjData->GeteMMCOptionJson().toStdString();
	try {
		auto extJson = nlohmann::json::parse(extCSDOptionJson);
		ui->ImageFileFormatComboBox->setCurrentIndex(extJson["imageFileFormat"].get<int>());
		ui->checksumComboBox->setCurrentIndex(extJson["imageFileChecksumType"].get<int>());
		ui->extCSDCheckbox->setChecked(extJson["imageFileExtCSDEnable"].get<bool>());
	}
	catch (const nlohmann::json::exception& e) {
		ALOG_FATAL("eMMCOption Json parse failed : %s.", "CU", "--", e.what());
	}

	std::string extCSDJson = m_pProjData->GeteMMCExtCSDParaJson().toStdString();
	ui->extCSDWidget->SeteMMCExtCSDPara(extCSDJson);
}

bool ACeMMCSolution::isExtCSDChecked() {
	return ui->extCSDCheckbox->isChecked();
}


std::string ACeMMCSolution::GetImageFileFormat()
{
	return ui->ImageFileFormatComboBox->currentText().toStdString();
}

int ACeMMCSolution::StartIntelligentAnalyze(const QString& proj_path, QString& strBytesum)
{
	return ui->binFilesPage->StartIntelligentAnalyze(proj_path, (ImageFileType)ui->ImageFileFormatComboBox->currentData().toInt(), ui->checksumComboBox->currentIndex(), strBytesum);
}

void ACeMMCSolution::GetEMMCHeaderInfo(const QString& proj_path, eMMCTableHeader& tableHeader, std::string& tableJson, std::string& emmcOptonInfo)
{
	ui->binFilesPage->GetEMMCHeaderInfo(proj_path, tableHeader, tableJson, emmcOptonInfo);
}

void ACeMMCSolution::onSlotImageFileSelect(int nIndex)
{
	ui->binFilesPage->SetImageFileType((ImageFileType)ui->ImageFileFormatComboBox->currentData().toInt());
}

void ACeMMCSolution::onSlotReadChipExtCSD()
{
	if (g_AppMode == ConnectType::Demo) {
		QString tips = tr("Run In Demo Mode");
		ACMessageBox::showWarning(this, tr("Warning"), tips, ACMessageBox::ACMsgButton::MSG_OK_BTN);
		return;
	}

	// Get selected IP-Hop from deviceInfoComboBox
	QString selectedIPHop = ui->deviceInfoComboBox->currentData().toString();
	QStringList progIP_Hop = selectedIPHop.split("-");
	
	// Get selected socket value from deviceSktComboBox 
	int SKTNum = ui->deviceSktComboBox->currentData().toInt();

	if (!ACDeviceManager::instance().isDevieOnline(progIP_Hop[0], progIP_Hop[1].toInt())) {
		ACMessageBox::showError(this, tr("Error"), tr("The selected device is offline or abnormal, please select again."));
		return;
	}


	// Send command
	AngKMessageHandler::instance().Command_RemoteDoPTCmd(
		progIP_Hop[0].toStdString(),
		progIP_Hop[1].toInt(), 
		0, 
		0, 
		(uint16_t)eSubCmdID::SubCmd_MU_ReadChipExtcsd, 
		SKTNum,
		8,
		QByteArray()
	);
}

void ACeMMCSolution::onSlotChipIDFetched(std::string resultJson) {

	try {
		nlohmann::json IDFetchedJson = nlohmann::json::parse(resultJson);
		nlohmann::json ChipIDInfoJson = IDFetchedJson["ChipIDInfo"];

		int nBPUIdx = IDFetchedJson["BPUIdx"];
		QString strBPU = "B" + QString::number(nBPUIdx);

		int nSKTIdx = -1;
		if (ChipIDInfoJson.contains("SKTIdx")) {
			nSKTIdx = ChipIDInfoJson["SKTIdx"].get<int>();
		}

		if (ChipIDInfoJson.contains("ChipID")) {
			std::string fetchedChipID = ChipIDInfoJson["ChipID"].get<std::string>();
			ALOG_INFO("Recv ChipIDFetched SKTIdx: %d ChipID: %s.", "CU", "--", nSKTIdx, fetchedChipID.c_str());
			QString tips = tr("Read ExtCSD Result: %1").arg(QString::fromStdString(fetchedChipID));
			ACMessageBox::showWarning(this, tr("Info"), tips, ACMessageBox::ACMsgButton::MSG_OK_BTN);
		}
		else {
			ALOG_WARN("ChipIDFetched SKTIdx: %d not found ChipID node %s.", "CU", "--", nSKTIdx, resultJson.c_str());
		}
	}
	catch (const nlohmann::json::exception& e) {
		ALOG_FATAL("Parse ChipIDFetched Json failed : %s.", "CU", "--", e.what());
	}


}

void ACeMMCSolution::onSlotExtCSDFetched(std::string resultJson) {

	try {
		QString extLog;
		nlohmann::json eDataJson = nlohmann::json::parse(resultJson);
		nlohmann::json ExtCSDInfoJson = eDataJson["ExtCSDInfo"];
		bool bOk;
		uint32_t nCrc16 = QString::fromStdString(ExtCSDInfoJson["CRC16"]).toUInt(&bOk, 16);
		// Get selected IP-Hop from deviceInfoComboBox
		QString selectedIPHop = ui->deviceInfoComboBox->currentData().toString();
		QStringList progIP_Hop = selectedIPHop.split("-");

		// Get selected socket value from deviceSktComboBox 
		int SKTNum = ui->deviceSktComboBox->currentData().toInt();
		//GetExtCSDInfo(selectedIPHop.toStdString(), 
			//progIP_Hop[1].toInt(), ExtCSDInfoJson["Size"] * 8, 
			//QString::fromStdString(ExtCSDInfoJson["DDROffset"].get<std::string>()), nCrc16, extLog);
		//SetAnalyzeLogText(extLog);
	}
	catch (const nlohmann::json::exception& e) {
		ALOG_FATAL("Parse ExtCSDFetched Json failed : %s", "CU", "--", e.what());
	}

}


void ACeMMCSolution::GetExtCSDInfo(std::string devIP, uint32_t HopNum, uint32_t uExtSize, QString& strDDROffset, uint32_t uCrc16, QString& extLog)
{
	bool bOk = false;
	QByteArray readExtCSDByte;
	DeviceStu devInfo = ACDeviceManager::instance().getDevInfo(QString::fromStdString(devIP), HopNum);
	int nRecoverTrySend = AngKGlobalInstance::instance()->ReadValue("DeviceComm", "Retransmission").toInt();
	int nTryReSend = nRecoverTrySend;
	int ret = 0;
	while (nTryReSend != 0)
	{
		ret = AngKMessageHandler::instance().Command_ReadDataFromSSD(devIP, "DDR2FIBER", HopNum, 0, strDDROffset.toInt(&bOk, 16), uExtSize, readExtCSDByte, devInfo.tMainBoardInfo.strHardwareSN, "extCSDData");
		if (ret == 0) {
			ALOG_INFO("MasterChip getExtInfo complete.", "CU", "--");
			break;
		}
		else {
			nTryReSend--;
			ALOG_FATAL("MasterChip getExtInfo failed(errcode=%d).", "CU", "--", ret);
		}
	}

	if (nTryReSend <= 0 && ret != 0) {
		return;
	}
	ushort calCRC16 = 0;
	calc_crc16sum((uchar*)readExtCSDByte.constData(), 512, &calCRC16);

	if (calCRC16 != uCrc16) {
		ALOG_FATAL("MasterChip getExtInfo failed : calculate CRC16 different", "CU", "--");
		return;
	}

	//emit sgnFetchedExtCSD(readExtCSDByte);
	//GetExtCSDReg(strAddr, strValue, strName, m_readExtCSD);
}
