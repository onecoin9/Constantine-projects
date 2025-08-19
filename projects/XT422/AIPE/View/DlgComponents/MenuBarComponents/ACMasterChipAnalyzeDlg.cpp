#include "ACMasterChipAnalyzeDlg.h"
#include "ui_ACMasterChipAnalyzeDlg.h"
#include "StyleInit.h"
#include "ACDeviceManager.h"
#include "GlobalDefine.h"
#include "Thread/ThreadPool.h"
#include "ACMessageBox.h"
#include "AngKGlobalInstance.h"

#include "ACProjectCreateDlg.h"
#include "ACTaskCreateDlg.h"

#include <QFileDialog>
#include <QListWidgetItem>
#include <QScrollBar>

using namespace Utils;
extern Acro::Thread::ThreadPool g_ThreadPool;

int calc_crc16sum(unsigned char* buf, unsigned int size, unsigned short* pCRC16Sum);


ACMasterChipAnalyzeDlg::ACMasterChipAnalyzeDlg(QWidget *parent)
	: AngKDialog(parent)
	, ui(new Ui::ACMasterChipAnalyzeDlg())
	, m_strRecordPath(QCoreApplication::applicationDirPath())
	, m_pListWidget(std::make_unique<QListWidget>())
	, m_pChipAnalyzeManager(std::make_unique<ACMasterChipAnalyzeManager>(this))
	, m_nDevHop(-1)
	, bFirstAnalyze(true)
	, m_pTtimer(std::make_unique<QTimer>(this))
	, m_RecordPartitionVFileCount(0)
	, m_RecordPartitionVFileSum(0)
	, m_nCountPartition(0)
{
	this->setObjectName("ACMasterChipAnalyzeDlg");
	QT_SET_STYLE_SHEET(objectName());

	ui->setupUi(setCentralWidget());

	this->setFixedSize(900, 530);
	this->SetTitle(tr("MasterChip Analyze"));

	InitText();
	InitButton();
	InitComboBox();
}

ACMasterChipAnalyzeDlg::~ACMasterChipAnalyzeDlg()
{
	delete ui;
}

void ACMasterChipAnalyzeDlg::InitText()
{
	ui->partitionText->setText(tr("Partition:"));
	ui->analyzeSizeText->setText(tr("Analyze Size(MB):"));
	ui->analyzeGrainText->setText(tr("Analyze Grain:"));
	ui->defaultValueText->setText(tr("Default Value:"));
	ui->saveACxmlCheck->setText(tr("Acxml Path:"));
	ui->startAnalyzeButton->setText(tr("Start"));
	ui->stopAnalyzeButton->setText(tr("Stop"));
	ui->progressText->setText(tr("Current Progress:"));

	ui->analyzeProgressBar->setValue(0);
	ui->analyzeSizeTips->setText(tr("(0 mean Analyze all partitions)"));
	QRegExp hexRegexADDR("^[0-9]{1,8}");
	ui->analyzeSizeEdit->setText("0");
	ui->analyzeSizeEdit->setValidator(new QRegExpValidator(hexRegexADDR, ui->analyzeSizeEdit));

	ui->analyzeLogEdit->moveCursor(QTextCursor::End);
	ui->analyzeLogEdit->setReadOnly(true);
	ui->analyzeLogEdit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
	ui->analyzeLogEdit->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	ui->saveACxmlEdit->setReadOnly(true);
}

void ACMasterChipAnalyzeDlg::InitButton()
{
	ui->stopAnalyzeButton->hide();
	SetACXMLPathEnable(false);
	ui->saveACxmlCheck->setChecked(false);
	connect(this, &ACMasterChipAnalyzeDlg::sgnClose, this, [=]() { 
		if (!ui->startAnalyzeButton->isEnabled()) {
			ACMessageBox::showWarning(this, tr("Warning"), tr("Please wait for the analyze to complete."));
		}
		else {
			accept();
		}
		});
	connect(ui->acxmlPathButton, &QPushButton::clicked, this, &ACMasterChipAnalyzeDlg::onSlotSelectACxmlPath);
	connect(ui->startAnalyzeButton, &QPushButton::clicked, this, &ACMasterChipAnalyzeDlg::onSlotExecuteChipAnaylze);
	connect(ui->stopAnalyzeButton, &QPushButton::clicked, this, &ACMasterChipAnalyzeDlg::onSlotStopChipAnalyze);
	connect(m_pChipAnalyzeManager.get(), &ACMasterChipAnalyzeManager::sgnSendWriteProgressValue, this, &ACMasterChipAnalyzeDlg::onSlotGetProgress);
	connect(this, &ACMasterChipAnalyzeDlg::sgnFinishAnalyze, this, &ACMasterChipAnalyzeDlg::onSlotFinishAnalyze);
	connect(m_pTtimer.get(), &QTimer::timeout, this, &ACMasterChipAnalyzeDlg::onSlotUpdateTime);
	connect(ui->saveACxmlCheck, &QCheckBox::stateChanged, this, [=](int state) {
		if (state == Qt::CheckState::Checked)
		{
			SetACXMLPathEnable(true);
		}
		else if (state == Qt::CheckState::Unchecked)
		{
			SetACXMLPathEnable(false);
		}
		});

	connect(this, &ACMasterChipAnalyzeDlg::sgnACImageCRCCheckProgress, this, &ACMasterChipAnalyzeDlg::onSlotACImageCRCCheckProgress);
	connect(this, &ACMasterChipAnalyzeDlg::sgnPrintCRCMessage, this, &ACMasterChipAnalyzeDlg::onSlotPrintCRCMessage);
}

void ACMasterChipAnalyzeDlg::InitComboBox()
{
	//初始化partition
	m_pListWidget->setObjectName("PartitionList");
	std::vector<QString> partVec{ "USER", "BOOT1", "BOOT2", "RPMB","GPP1","GPP2" ,"GPP3" ,"GPP4"};
	for (int i = 0; i < partVec.size(); i++)
	{
		QListWidgetItem* item = new QListWidgetItem(m_pListWidget.get());
		QCheckBox* chkBox = new QCheckBox(m_pListWidget.get());
		chkBox->setObjectName(partVec[i] + "Check");
		chkBox->setChecked(true);
		chkBox->setText(partVec[i]);

		m_pListWidget->addItem(item);
		m_pListWidget->setItemWidget(item, chkBox);
		//connect(chkBox, SIGNAL(stateChanged(int)), this, SLOT(slot_stateChanged()));
	}
	ui->partitionComboBox->setModel(m_pListWidget->model());
	ui->partitionComboBox->setView(m_pListWidget.get());

	//初始化Grain
	ui->analyzeGrainComboBox->addItem("1M", 1);
	ui->analyzeGrainComboBox->addItem("2M", 2);
	ui->analyzeGrainComboBox->addItem("4M", 4);
	ui->analyzeGrainComboBox->addItem("8M", 8);
	ui->analyzeGrainComboBox->addItem("16M", 16);
	ui->analyzeGrainComboBox->addItem("32M", 32);

	//初始化默认分析值
	ui->defaultValueComboBox->addItem("0x0", 0x0);
	ui->defaultValueComboBox->addItem("0xFF", 0xFF);

	//初始化设备列表
	QVector<DeviceStu> vecDev = ACDeviceManager::instance().getAllDevInfo();
	for (auto dev : vecDev) {
		//离线设备要过滤掉不会在显示
		if (dev.bOnLine) {
			QString devName = QString::fromStdString(dev.strSiteAlias) + "-" + QString::fromStdString(dev.strIP);
			QString IPHop = QString::fromStdString(dev.strIP) + ":" + QString::number(dev.nHopNum);
			ui->deviceInfoComboBox->addItem(devName, IPHop);
		}
	}

	ui->deviceSktComboBox->addItem("SKT[1A]", 1);
	ui->deviceSktComboBox->addItem("SKT[1B]", 2);
	ui->deviceSktComboBox->addItem("SKT[2A]", 4);
	ui->deviceSktComboBox->addItem("SKT[2B]", 8);
	ui->deviceSktComboBox->addItem("SKT[3A]", 16);
	ui->deviceSktComboBox->addItem("SKT[3B]", 32);
	ui->deviceSktComboBox->addItem("SKT[4A]", 64);
	ui->deviceSktComboBox->addItem("SKT[4B]", 128);
	ui->deviceSktComboBox->addItem("SKT[5A]", 256);
	ui->deviceSktComboBox->addItem("SKT[5B]", 512);
	ui->deviceSktComboBox->addItem("SKT[6A]", 1024);
	ui->deviceSktComboBox->addItem("SKT[6B]", 2048);
	ui->deviceSktComboBox->addItem("SKT[7A]", 4096);
	ui->deviceSktComboBox->addItem("SKT[7B]", 8192);
	ui->deviceSktComboBox->addItem("SKT[8A]", 16384);
	ui->deviceSktComboBox->addItem("SKT[8B]", 32768);
}

void ACMasterChipAnalyzeDlg::SetACXMLPathEnable(bool bEnable)
{
	ui->saveACxmlEdit->setEnabled(bEnable);
	ui->acxmlPathButton->setEnabled(bEnable);

	if (!bEnable) {
		ui->saveACxmlEdit->setText("");
	}
}

void ACMasterChipAnalyzeDlg::SetAnalyzeLogText(QString text)
{
	bool bFlag = false;
	QScrollBar* vScrollBar = ui->analyzeLogEdit->verticalScrollBar();
	if (vScrollBar->value() == vScrollBar->maximum()) {
		bFlag = true;
	}

	QTextCursor cursor = ui->analyzeLogEdit->textCursor();
	cursor.movePosition(QTextCursor::End);
	text += "\r\n";
	cursor.insertText(text);

	if (bFlag)
		vScrollBar->setValue(vScrollBar->maximum());
}

int ACMasterChipAnalyzeDlg::SaveACIMGFile(uint64_t nTotalBlkNum)
{
	if (!ui->saveACxmlCheck->isChecked()) {
		if (m_pTtimer->isActive()) {
			m_pTtimer->stop();
		}
		ui->startAnalyzeButton->setEnabled(true);
		return -1;
	}

	//重新置位进度条
	ui->progressText->setText(tr("[2/2]MasterChip WriteLocal:"));
	ui->analyzeProgressBar->setValue(0);
	int ret = 0;
	QString IPHop = ui->deviceInfoComboBox->currentData().toString();
	QStringList IPHopList = IPHop.split(":");
	uint64_t alignSize = nTotalBlkNum * 512;
	qDebug() << "alignSize" << alignSize;
	g_ThreadPool.PushTask([this](QString& strDevIP, int nHop, QString& strTaskPath, uint64_t uAlignSize) {
		int ret = m_pChipAnalyzeManager->ExecuteReadDataAndSaveToFile(strDevIP.toStdString(), m_strACIMGPath, "SSD2FIBER", nHop, SSD_READ_MASTERCHIP_ANALYZE, uAlignSize);

		if (ret == 0) {
			QString successLog = QString("MasterChip analyze save ACIMG file %1 successfully.").arg(m_strACIMGPath);
			RecordPrintLog(successLog);
		}
		else {
			QString failedLog = QString("MasterChip analyze save ACIMG file %1 failed.").arg(m_strACIMGPath);
			RecordPrintLog(failedLog);
		}
		m_pChipAnalyzeManager->SaveACXMLFile(m_strACXMLPath);
		emit sgnFinishAnalyze(ret);
		}, std::move(IPHopList[0]), IPHopList[1].toInt(), m_strACXMLPath, alignSize);

	return ret;
}

void ACMasterChipAnalyzeDlg::RecordPrintLog(QString text, QString fromDev, Utils::AngkLogger::LogLevel logLevel)
{
	if (logLevel == Utils::AngkLogger::LogLevel::INFO) {
		ALOG_INFO("%s", "CU", fromDev.toStdString().c_str(), text.toStdString().c_str());
	}
	else if (logLevel == Utils::AngkLogger::LogLevel::FATAL) {
		ALOG_FATAL("%s", "CU", fromDev.toStdString().c_str(), text.toStdString().c_str());
	}
	else if (logLevel == Utils::AngkLogger::LogLevel::WARN) {
		ALOG_WARN("%s", "CU", fromDev.toStdString().c_str(), text.toStdString().c_str());
	}
	else if (logLevel == Utils::AngkLogger::LogLevel::Error) {
		ALOG_ERROR("%s", "CU", fromDev.toStdString().c_str(), text.toStdString().c_str());
	}
	SetAnalyzeLogText(text);
}

void ACMasterChipAnalyzeDlg::PrintBasicInfo(std::string& devSN, std::string& devAlias)
{
	RecordPrintLog("======================== Start to Analyse Card ==========================");
	QString analyzeSnInfo = QString("Device SN : %1").arg(QString::fromStdString(devSN));
	QString analyzeAliasInfo = QString("Device Alias : %1").arg(QString::fromStdString(devAlias));
	QString analyzeSktInfo = QString("Device SKT : %1").arg(ui->deviceSktComboBox->currentText());
	QString analyzeGrainInfo = QString("Device Analyze Grain : %1").arg(ui->analyzeGrainComboBox->currentText());
	QString analyzeDefaulteValue = QString("Device Analyze DefaultValue : %1").arg(ui->defaultValueComboBox->currentText());
	QString analyzeSize = QString("Device Analyze Size : %1 MB").arg(ui->analyzeSizeEdit->text());
	RecordPrintLog(analyzeSnInfo);
	RecordPrintLog(analyzeAliasInfo);
	RecordPrintLog(analyzeSktInfo);
	RecordPrintLog(analyzeGrainInfo);
	RecordPrintLog(analyzeDefaulteValue);
	RecordPrintLog(analyzeSize);
	//RecordPrintLog("=========================================================================");
}

void ACMasterChipAnalyzeDlg::PrintEntryInfo(nlohmann::json& vectorInfo)
{
	bool bOk = false;
	int64_t nCalBlockLength = QString::fromStdString(vectorInfo["BlockNum"].get<std::string>()).toLongLong(&bOk, 16);
	nCalBlockLength = (nCalBlockLength * 512) / 1024 / 1024;
	QString strCalBlock = QString("%1").arg(nCalBlockLength).leftJustified(8, ' ');
	QString entryInfo = QString("%1 Partition Get Entry %2, Length = %3 MBytes, ChipBlockPos = %4, CRC16 = %5")
		.arg(vectorInfo["Partition"].get<std::string>().c_str())
		.arg(vectorInfo["Name"].get<std::string>().c_str())
		.arg(strCalBlock)
		.arg(vectorInfo["ChipBlockPos"].get<std::string>().c_str())
		.arg(vectorInfo["CRC16"].get<std::string>().c_str());
	
	RecordPrintLog(entryInfo);
}

void ACMasterChipAnalyzeDlg::PrintPartitionInfo()
{
	if (m_curAnalyzePartition == "ExtCSD")
		return;

	uint32_t curPartSize = 0;
	QString partitionInfo, partitionFileInfo, partitionSum;
	if (m_curAnalyzePartition == "USER") {
		curPartSize = m_pChipAnalyzeManager->GetUserAreaSize();
		partitionInfo = QString("Partition %1 Total Size = %2 MB").arg(m_curAnalyzePartition).arg(curPartSize);
	}
	else if (m_curAnalyzePartition == "BOOT1" || m_curAnalyzePartition == "BOOT2") {
		curPartSize = m_pChipAnalyzeManager->GetBootAreaSize();
		partitionInfo = QString("Partition %1 Total Size = %2 MB").arg(m_curAnalyzePartition).arg(curPartSize / 1024);
	}
	else if (m_curAnalyzePartition == "GPP1") {
		curPartSize = m_pChipAnalyzeManager->GetGPPAreaSize(0);
		partitionInfo = QString("Partition %1 Total Size = %2 MB").arg(m_curAnalyzePartition).arg(curPartSize);
	}
	else if (m_curAnalyzePartition == "GPP2") {
		curPartSize = m_pChipAnalyzeManager->GetGPPAreaSize(1);
		partitionInfo = QString("Partition %1 Total Size = %2 MB").arg(m_curAnalyzePartition).arg(curPartSize);
	}
	else if (m_curAnalyzePartition == "GPP3") {
		curPartSize = m_pChipAnalyzeManager->GetGPPAreaSize(2);
		partitionInfo = QString("Partition %1 Total Size = %2 MB").arg(m_curAnalyzePartition).arg(curPartSize);
	}
	else if (m_curAnalyzePartition == "GPP4") {
		curPartSize = m_pChipAnalyzeManager->GetGPPAreaSize(3);
		partitionInfo = QString("Partition %1 Total Size = %2 MB").arg(m_curAnalyzePartition).arg(curPartSize);
	}

	if (curPartSize == 0)
	{
		partitionInfo = QString("Partition %1 size is zero, ignore it").arg(m_curAnalyzePartition);
		partitionFileInfo = QString("File Counter: 0");
		RecordPrintLog(partitionInfo);
		RecordPrintLog(partitionFileInfo);
		RecordPrintLog(partitionSum);
	}
	else {
		RecordPrintLog(partitionInfo);
	}
}

void ACMasterChipAnalyzeDlg::PrintResultInfo(nlohmann::json& partitionJson, QString& TotalBlkNum, QString& TotalBlkBytesum)
{
	for (int i = 0; i < partitionJson.size(); ++i) {
		nlohmann::json partJson = partitionJson[i];
		std::string BlkNum = "0x" + partJson["BlkNum"].get<std::string>();
		std::string bytesum = "0x" + partJson["ByteSum"].get<std::string>();
		QString partName = QString("%1").arg(partJson["Name"].get<std::string>().c_str()).leftJustified(5, ' ');
		QString partLog = QString("%1 Partition, BlockNum = %2, ByteSum = %3")
			.arg(partName)
			.arg(BlkNum.c_str())
			.arg(bytesum.c_str());
		RecordPrintLog(partLog);
	}

	QString totalLog = QString("Total Partition, BlockNum = 0x%1, Total Data ByteSum = 0x%2").arg(TotalBlkNum).arg(TotalBlkBytesum);
	RecordPrintLog(totalLog);
}

void ACMasterChipAnalyzeDlg::SetAnalyzePartitionCount()
{
	if (m_pChipAnalyzeManager->GetUserAreaSize() > 0) {
		m_nCountPartition++;
	}	
	if (m_pChipAnalyzeManager->GetBootAreaSize() > 0) {
		m_nCountPartition += 2;
	}
	for (int i = 0; i < 4; ++i) {
		if (m_pChipAnalyzeManager->GetGPPAreaSize(i) > 0) {
			m_nCountPartition++;
		}
	}
}

void ACMasterChipAnalyzeDlg::ResetAnalyzeInfo()
{
	ui->analyzeLogEdit->clear();
	m_pChipAnalyzeManager->ResetDocument();
	elapsedTime = 0;
	ui->timeLabel->setText("00:00");
	m_pTtimer->start(1000);
	m_nCountPartition = 0;
	m_fCurrentTotalProgress = 0;
	ui->analyzeProgressBar->setValue(0);
}

void ACMasterChipAnalyzeDlg::PrintAnalyzeTime(QString analyzeLabel)
{
	// 计算小时数
	qint64 hours = elapsedTime / 3600;
	// 计算剩余的秒数（去除小时后的秒数）
	qint64 remainingSeconds = elapsedTime % 3600;
	// 获取完整的分钟数
	int minutes = static_cast<int>(remainingSeconds / 60);
	// 获取剩余的秒数
	int seconds = static_cast<int>(remainingSeconds % 60);
	QString strTimeAnalyze = QString("%1 Time Total Time : %2 hours %3 min %4 seconds.").arg(analyzeLabel).arg(hours).arg(minutes).arg(seconds);
	RecordPrintLog(strTimeAnalyze);
}

void ACMasterChipAnalyzeDlg::onSlotExecuteChipAnaylze()
{
	ResetAnalyzeInfo();
	if (g_AppMode == ConnectType::Demo) {

		// 先在demo测试创建空task
		//std::shared_ptr<AngKDataBuffer>	m_DataBuffer = std::make_shared<AngKDataBuffer>(this);
		//ACProjectCreateDlg dlg(this, m_DataBuffer);
		//dlg.FTabCreateEmptyEapr();

		//std::shared_ptr<ACTaskManager> m_pTaskManager = std::make_shared<ACTaskManager>(this);
		//ACTaskCreateDlg tskDlg(this, m_pTaskManager);
		//tskDlg.CreateMasterAnalyzeTask();


		//return;
	}

	QString IPHop = ui->deviceInfoComboBox->currentData().toString();
	QStringList IPHopList = IPHop.split(":");

	if (IPHopList.size() < 2) {
		if (m_pTtimer->isActive()) {
			m_pTtimer->stop();
		}
		ACMessageBox::showError(this, tr("Error"), tr("The selected device is offline or abnormal, please select again."));

		return;
	}

	m_strDevIP = IPHopList[0];
	m_nDevHop = IPHopList[1].toInt();
	DeviceStu devInfo = ACDeviceManager::instance().getDevInfo(m_strDevIP, m_nDevHop);
	// 检测设备状态
	//if (devInfo.ProgEnvReady == ProEnvStatus::Failed) {
	//	ACMessageBox::showError(this, tr("Error"), tr("Device %1:%2 load task failed, can't execute MasterChip Analyze.").arg(QString::fromStdString(devInfo.strIP)).arg(QString::number(devInfo.nHopNum)));
	//	return;
	//}
	if (devInfo.ProgEnvReady == ProEnvStatus::Idle) {
		ACMessageBox::showError(this, tr("Error"), tr("Device %1:%2 not loaded task, unable to execute MasterChip Analyze.").arg(QString::fromStdString(devInfo.strIP)).arg(QString::number(devInfo.nHopNum)));
		return;
	}
	//if (devInfo.ProgEnvReady == ProEnvStatus::Abnormal) {
	//	ACMessageBox::showWarning(this, tr("Warn"), tr("Device %1:%2 task is abnormal, unable to execute MasterChip Analyze.").arg(QString::fromStdString(devInfo.strIP)).arg(QString::number(devInfo.nHopNum)));
	//	return;
	//}


	uint32_t nSktEnable = ui->deviceSktComboBox->currentData().toUInt();
	uint32_t nDefaultValue = ui->defaultValueComboBox->currentData().toUInt();
	uint32_t nAnalyzeSize = ui->analyzeSizeEdit->text() == "" ? 0 : ui->analyzeSizeEdit->text().toUInt();
	uint32_t nAnalyzeGrain = ui->analyzeGrainComboBox->currentData().toUInt();

	if(ui->analyzeSizeEdit->text() == "")
		ui->analyzeSizeEdit->setText("0");

	uint32_t nSelectPartition = 0;
	for (int i = 0; i < m_pListWidget->count(); ++i) {
		QListWidgetItem* item = m_pListWidget->item(i);
		QCheckBox* checkBox = qobject_cast<QCheckBox*>(m_pListWidget->itemWidget(item));
		if (checkBox && checkBox->isChecked()) {
			nSelectPartition += (1 << i);
		}
	}

	m_pChipAnalyzeManager->ExecuteChipAnaylze(m_strDevIP.toStdString(), m_nDevHop, nSktEnable, nSelectPartition, nDefaultValue, nAnalyzeSize, nAnalyzeGrain, SSD_READ_MASTERCHIP_ANALYZE);

	ui->startAnalyzeButton->setEnabled(false);
	if (ui->saveACxmlCheck->isChecked()) {
		ui->progressText->setText(tr("[1/2]MasterChip Analyze:"));
	}
	else {
		ui->progressText->setText(tr("MasterChip Analyze:"));
	}
	emit sgnSetAnalyzeFlag(true);

	PrintBasicInfo(devInfo.tMainBoardInfo.strHardwareSN, devInfo.strSiteAlias);
}

void ACMasterChipAnalyzeDlg::onSlotStopChipAnalyze()
{
	ui->startAnalyzeButton->setEnabled(true);
}

void ACMasterChipAnalyzeDlg::onSlotSelectACxmlPath()
{
	QString sfilePath = QFileDialog::getSaveFileName(this, "Select Task File...", m_strRecordPath, tr("eMMC MCard Image(*.acimg)"));
	if (!sfilePath.isEmpty()) {
		m_strRecordPath = sfilePath;
		m_strACIMGPath = sfilePath;


		int lastSlashIndex = m_strACIMGPath.lastIndexOf('\\');
		QString fileName = m_strACIMGPath.mid(lastSlashIndex + 1);
		// 移除文件名中的最后一个后缀（.acimg）
		fileName.chop(6); // 移除 ".acimg" 的 6 个字符

		// 添加新的后缀（.acxml）
		fileName.append(".acxml");

		// 构建新的文件路径
		m_strACXMLPath = m_strACIMGPath.left(lastSlashIndex + 1) + fileName;
		ui->saveACxmlEdit->setText(sfilePath);
	}
}

void ACMasterChipAnalyzeDlg::onSlotGetProgress(std::string recvIP, int nHop, int nValue)
{
	ui->analyzeProgressBar->setValue(nValue);
}

void ACMasterChipAnalyzeDlg::onSlotGetChipAnalyzeInfo(QString strInfo)
{

}

void ACMasterChipAnalyzeDlg::onSlotMasterChipAnalyzeResult(uint32_t nResult, std::string fromDev)
{
	QString strSuccess = "finish";
	QString strFailed = "failed";
	QString alogText = "Device MasterChip Analyze ";
	if (nResult == 0) {
		alogText += strSuccess;
		ALOG_INFO("%s", "CU", fromDev.c_str(), alogText.toStdString().c_str());
	}
	else {
		alogText += strFailed;
		ALOG_FATAL("%s", "CU", fromDev.c_str(), alogText.toStdString().c_str());

		if (m_pTtimer->isActive()) {
			m_pTtimer->stop();
		}
		ui->startAnalyzeButton->setEnabled(true);
	}

	SetAnalyzeLogText(alogText);
	if(!ui->saveACxmlCheck->isChecked())
		ui->startAnalyzeButton->setEnabled(true);

}

void ACMasterChipAnalyzeDlg::onSlotHandleEventAnalyzeResult(std::string eStrDataJson)
{
	try {
		nlohmann::json eDataJson = nlohmann::json::parse(eStrDataJson);
		int nBPUIdx = eDataJson["BPUIdx"];
		std::string strBPU = "B" + QString::number(nBPUIdx).toStdString();
		nlohmann::json resultJson = eDataJson["ResultInfo"];
		int SktIdx = resultJson["SKTIdx"];
		std::string retCode = resultJson["RetCode"];
		std::string ExtMsg = resultJson["ExtMsg"];

		if (retCode != "4000") {
			QString resultLog = QString("MasterChip Analyze %1 Partition failed(errorCode:%2)").arg(QString::fromStdString(ExtMsg)).arg(QString::fromStdString(retCode));
			ALOG_FATAL("%s", "CU", strBPU.c_str(), resultLog.toStdString().c_str());
			SetAnalyzeLogText(resultLog);

			if (m_pTtimer->isActive()) {
				m_pTtimer->stop();
			}
			ui->startAnalyzeButton->setEnabled(true);
			return;
		}
		else {
			bool bOK = false;
			m_pChipAnalyzeManager->AppendNode("Checksum", eDataJson["PartitionInfo"]);

			QString strTotalBlkNum = QString::fromStdString(eDataJson["TotalBlkNum"].get<std::string>());
			qDebug() << "strTotalBlkNum:" << strTotalBlkNum;
			QString strTotalBlkByteSum = QString::fromStdString(eDataJson["TotalByteSum"].get<std::string>());
			PrintResultInfo(eDataJson["PartitionInfo"], strTotalBlkNum, strTotalBlkByteSum);

			uint64_t nTotalBlkNum = strTotalBlkNum.toLongLong(&bOK, 16);

			if (nTotalBlkNum == 0) {
				if (m_pTtimer->isActive()) {
					m_pTtimer->stop();
				}
				ui->startAnalyzeButton->setEnabled(true);
				ALOG_WARN("MasterChip Analyze TotalBlockNum is zero, not save file.", "CU", "--");
				return;
			}

			SaveACIMGFile(nTotalBlkNum);
		}

		//不管成功失败，收到结果时，都需要将设备Task下发状态改为未下发状态，点击Start时，不允许执行烧录

		//DeviceStu devCopy = ACDeviceManager::instance().getDevInfo(m_strDevIP, m_nDevHop);
		//devCopy.ProgEnvReady = ProEnvStatus::Abnormal;
		//ACDeviceManager::instance().setDevInfo(m_strDevIP, m_nDevHop, devCopy);

		//ALOG_WARN("MasterChip analyze finish, Current Device need to download task again.", "CU", "--");
	}
	catch (const nlohmann::json::exception& e) {
		ALOG_FATAL("Parse AnalyzeResult Json failed : %s", "CU", "--", e.what());
	}

	RecordPrintLog("======================== Card Analysis END ==========================");

	PrintAnalyzeTime("Analyze");
}

void ACMasterChipAnalyzeDlg::onSlotHandleEventExtCSDFetched(std::string eStrDataJson)
{
	try {
		QString extLog;
		nlohmann::json eDataJson = nlohmann::json::parse(eStrDataJson);
		nlohmann::json ExtCSDInfoJson = eDataJson["ExtCSDInfo"];
		bool bOk;
		uint32_t nCrc16 = QString::fromStdString(ExtCSDInfoJson["CRC16"]).toUInt(&bOk, 16);
		m_pChipAnalyzeManager->GetExtCSDInfo(m_strDevIP.toStdString(), m_nDevHop, ExtCSDInfoJson["Size"] * 8, QString::fromStdString(ExtCSDInfoJson["DDROffset"].get<std::string>()), nCrc16, extLog);
		extLog = "\n" + extLog;
		RecordPrintLog(extLog);
		//SetAnalyzeLogText(extLog);
	}
	catch (const nlohmann::json::exception& e) {
		ALOG_FATAL("Parse ExtCSDFetched Json failed : %s", "CU", "--", e.what());
	}
}

void ACMasterChipAnalyzeDlg::onSlotHandleEventAnalyzeInfo(std::string eStrDataJson)
{
	try {
		nlohmann::json analyzeInfoJson = nlohmann::json::parse(eStrDataJson);
		nlohmann::json VectorInfo = analyzeInfoJson["VectorInfo"];
		m_pChipAnalyzeManager->AppendNode("Partitions", VectorInfo);

		PrintEntryInfo(VectorInfo);
		m_RecordPartitionVFileCount++;
	}
	catch (const nlohmann::json::exception& e) {
		ALOG_FATAL("Parse AnalyzeInfo Json failed : %s", "CU", "--", e.what());
	}
}

void ACMasterChipAnalyzeDlg::onSlotHandleEventAnalyzeStatusChange(std::string eStrDataJson)
{
	try {
		nlohmann::json analyzeStateJson = nlohmann::json::parse(eStrDataJson);
		if (!bFirstAnalyze) {

			if (m_curAnalyzePartition == "ExtCSD") {
				QString partitionFileInfo = QString("File Counter: %1").arg(m_RecordPartitionVFileCount);
				RecordPrintLog(partitionFileInfo);
			}
			RecordPrintLog("======================== Analysis Partition END ==========================");
			bFirstAnalyze = false;
		}
		int nBPUIdx = analyzeStateJson["BPUIdx"];
		QString strBPU = "B" + QString::number(nBPUIdx);
		m_curAnalyzePartition = QString::fromStdString(analyzeStateJson["Status"]);

		QString statusLog = QString("======================== Analysis %1 Start ==========================").arg(m_curAnalyzePartition);
		RecordPrintLog(statusLog, strBPU);

		PrintPartitionInfo();
		m_RecordPartitionVFileCount = 0;
	}
	catch (const nlohmann::json::exception& e) {
		ALOG_FATAL("Parse AnalyzeInfo Json failed : %s", "CU", "--", e.what());
	}
}

void ACMasterChipAnalyzeDlg::onSlotHandleEventUIDFetched(std::string eStrDataJson)
{
	try {
		nlohmann::json UIDFetchedJson = nlohmann::json::parse(eStrDataJson);
		nlohmann::json UIDInfoJson = UIDFetchedJson["UIDInfo"];

		int nBPUIdx = UIDFetchedJson["BPUIdx"];
		QString strBPU = "B" + QString::number(nBPUIdx);

		for (int infoIdx = 0; infoIdx < UIDInfoJson.size(); ++infoIdx) {
			nlohmann::json infoIdxJson = UIDInfoJson[infoIdx];
			int nSKTIdx = infoIdxJson["SKTIdx"];

			if (((1 << (nBPUIdx * 2)) + nSKTIdx) == ui->deviceSktComboBox->currentData().toInt()) {
				QString UIDLog = QString("%1 UID : %2").arg(ui->deviceSktComboBox->currentText()).arg(infoIdxJson["UID"].get<std::string>().c_str());
				RecordPrintLog(UIDLog, strBPU);
				RecordPrintLog("=========================================================================");
			}
		}
	}
	catch (const nlohmann::json::exception& e) {
		ALOG_FATAL("Parse UIDFetched Json failed : %s", "CU", "--", e.what());
	}
}

void ACMasterChipAnalyzeDlg::onSlotHandleEventChipIDFetched(std::string eStrDataJson)
{
	try {
		nlohmann::json ChipIDFetchedJson = nlohmann::json::parse(eStrDataJson);
		nlohmann::json ChipIDInfoJson = ChipIDFetchedJson["ChipIDInfo"];

		int nBPUIdx = ChipIDFetchedJson["BPUIdx"];
		QString strBPU = "B" + QString::number(nBPUIdx);

		int nSKTIdx = -1;
		if (ChipIDInfoJson.contains("SKTIdx")) {
			nSKTIdx = ChipIDInfoJson["SKTIdx"].get<int>();
		}

		if (ChipIDInfoJson.contains("ChipID")) {
			std::string fetchedChipID = ChipIDInfoJson["ChipID"].get<std::string>();
			ALOG_INFO("ChipIDFetched SKTIdx: %d ID: %s", "CU", "--", nSKTIdx, fetchedChipID.c_str());
			if (((1 << (nBPUIdx * 2)) + nSKTIdx) == ui->deviceSktComboBox->currentData().toInt()) {
				QString ChipIDLog = QString("%1 ChipID : %2").arg(ui->deviceSktComboBox->currentText()).arg(fetchedChipID.c_str());
				RecordPrintLog(ChipIDLog, strBPU);
				RecordPrintLog("=========================================================================");
			}
			m_pChipAnalyzeManager->AppendNode("ChipID", ChipIDInfoJson);
		}
		else {
			ALOG_WARN("ChipIDFetched SKTIdx: %d not found ChipID node %s.", "CU", "--", nSKTIdx, eStrDataJson.c_str());
		}
	}
	catch (const nlohmann::json::exception& e) {
		ALOG_FATAL("Parse ChipIDFetched Json failed : %s", "CU", "--", e.what());
	}
}

void ACMasterChipAnalyzeDlg::onSlotUpdateTime()
{
	elapsedTime++;
	QTime currentTime = QTime(0, 0).addSecs(elapsedTime);
	ui->timeLabel->setText(currentTime.toString("mm:ss"));
}

void ACMasterChipAnalyzeDlg::onSlotFinishAnalyze(int nResult)
{
	if(nResult == 0)
		PrintAnalyzeTime("Write ACIMG file to local");

	// 对Entry进行CRC比对
	int nACImageCRCCheck = AngKGlobalInstance::instance()->ReadValue("DeviceComm", "ACImageCRCCheck").toInt();
	if (nACImageCRCCheck == 1 && nResult == 0) {
		//重新置位进度条
		ui->progressText->setText(tr("ACIMG Compare:"));
		ui->analyzeProgressBar->setValue(0);

		g_ThreadPool.PushTask([this]() {
			pugi::xml_document doc;
			pugi::xml_parse_result result = doc.load_file(reinterpret_cast<const wchar_t*>(m_strACXMLPath.utf16()));
			if (!result) {
				QString CRCErrorMsg = QString("CRC check err! Parse %s ACXML failed : %1").arg(m_strACIMGPath);
				emit sgnPrintCRCMessage(CRCErrorMsg);
				emit sgnACImageCRCCheckProgress(100, true);
				return;
			}
			pugi::xml_node emmcNode = doc.child("eMMC");
			pugi::xml_node partitionNodes = emmcNode.child("Partitions");

			QFile acimageFile(m_strACIMGPath);
			if (!acimageFile.open(QIODevice::ReadOnly)) {
				QString CRCErrorMsg = QString("CRC check err! Open %s failed : %1").arg(m_strACIMGPath);
				emit sgnPrintCRCMessage(CRCErrorMsg);
				emit sgnACImageCRCCheckProgress(100, true);
				return;
			}
			int nEntryCount = 0;

			uint64_t allEntryLen = 0;
			uint64_t curReadEntryLen = 0;
			for (pugi::xml_node entry = partitionNodes.child("Entry"); entry; entry = entry.next_sibling("Entry")) {
				allEntryLen += QString(entry.attribute("BlockNum").value()).toLongLong(nullptr, 16) * 512;
			}

			for (pugi::xml_node entry = partitionNodes.child("Entry"); entry; entry = entry.next_sibling("Entry")) {

				QString entryName = entry.attribute("Name").value();
				int64_t startAddr = QString(entry.attribute("FileBlockPos").value()).toLongLong(nullptr, 16) * 512;
				int64_t entryLen = QString(entry.attribute("BlockNum").value()).toLongLong(nullptr, 16) * 512;
				unsigned short acxmlEntryCrc16 = QString(entry.attribute("CRC16").value()).toInt(nullptr, 16);

				int64_t curReadLen = 0;
				int64_t curCRCReadEntryLen = 0;
				int64_t maxOnceReadLen = 1024 * 1024 * 1;
				unsigned short calcEntryCrc16 = 0;


				if (!acimageFile.seek(startAddr)) {
					QString CRCErrorMsg = QString("CRC check err! Seek %s failed : %1").arg(m_strACIMGPath);
					emit sgnPrintCRCMessage(CRCErrorMsg);
					emit sgnACImageCRCCheckProgress(100, true);
					return;
				}
				while (curReadLen < entryLen)
				{
					QByteArray data = acimageFile.read(qMin(maxOnceReadLen, entryLen - curReadLen));
					calc_crc16sum((uchar*)data.constData(), qMin(maxOnceReadLen, entryLen - curReadLen), &calcEntryCrc16);
					curReadLen += qMin(maxOnceReadLen, entryLen - curReadLen);
					
					curCRCReadEntryLen = curReadEntryLen + curReadLen;

					float nValue = static_cast<float>(curCRCReadEntryLen) / allEntryLen * 100;
					emit sgnACImageCRCCheckProgress(nValue, false);
				}

				curReadEntryLen = curCRCReadEntryLen;
				if (acxmlEntryCrc16 != calcEntryCrc16) {
					QString CRCErrorMsg = QString("CRC check err! Entry chk is different from the original value. EntryName:%1 Org CRC16:%2, Calc CRC16:%3").arg(entryName).arg(QString::number(acxmlEntryCrc16, 16)).arg(QString::number(calcEntryCrc16, 16));
					emit sgnPrintCRCMessage(CRCErrorMsg);
					emit sgnACImageCRCCheckProgress(100, true);
					return;
				}
			}
			QString CRCErrorMsg = QString("MasterChip analyze CRC16 check success!");
			emit sgnPrintCRCMessage(CRCErrorMsg);
			emit sgnACImageCRCCheckProgress(100, true);
		});
	}
	else {
		ui->analyzeProgressBar->setValue(100);
		if (m_pTtimer->isActive()) {
			m_pTtimer->stop();
		}

		ui->startAnalyzeButton->setEnabled(true);
	}


}

void ACMasterChipAnalyzeDlg::onSlotACImageCRCCheckProgress(int nProgress, bool bCRCFinish)
{
	ui->analyzeProgressBar->setValue(nProgress);

	if (bCRCFinish) {
		ui->analyzeProgressBar->setValue(100);
		if (m_pTtimer->isActive()) {
			m_pTtimer->stop();
		}

		ui->startAnalyzeButton->setEnabled(true);
	}
}

void ACMasterChipAnalyzeDlg::onSlotPrintCRCMessage(QString strCRCMessage)
{
	RecordPrintLog(strCRCMessage);
}
