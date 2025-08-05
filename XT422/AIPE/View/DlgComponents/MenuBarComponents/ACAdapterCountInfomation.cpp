#include "ACAdapterCountInfomation.h"
#include "ui_ACAdapterCountInfomation.h"
#include "StyleInit.h"
#include "AngKDeviceModel.h"

Q_DECLARE_METATYPE(DeviceStu);

ACAdapterCountInfomation::ACAdapterCountInfomation(QWidget *parent)
	: AngKDialog(parent)
{
	this->setObjectName("ACAdapterCountInfomation");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::ACAdapterCountInfomation();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(900, 495);
	this->SetTitle(tr("Adapter Information Inquiry"));

	connect(this, &ACAdapterCountInfomation::sgnClose, this, &ACAdapterCountInfomation::close);

	InitText();
	InitButton();
}

ACAdapterCountInfomation::~ACAdapterCountInfomation()
{
	delete ui;
}

void ACAdapterCountInfomation::InitText()
{
	ui->tabWidget->setTabText(0, tr("AdapterUseInfo"));
	ui->tabWidget->setTabText(1, tr("AdapterInfo"));
	ui->queryButton->setText(tr("Query"));
	ui->allCheck->setText(tr("Get all adapter information"));

	//用户视角来看是SKT1-SKT8
	ui->checkBPU_0->setText("SKT1");
	ui->checkBPU_1->setText("SKT2");
	ui->checkBPU_2->setText("SKT3");
	ui->checkBPU_3->setText("SKT4");
	ui->checkBPU_4->setText("SKT5");
	ui->checkBPU_5->setText("SKT6");
	ui->checkBPU_6->setText("SKT7");
	ui->checkBPU_7->setText("SKT8");

	ui->checkBPU_0->setProperty("SKT1", 1);
	ui->checkBPU_1->setProperty("SKT2", 2);
	ui->checkBPU_2->setProperty("SKT3", 4);
	ui->checkBPU_3->setProperty("SKT4", 8);
	ui->checkBPU_4->setProperty("SKT5", 16);
	ui->checkBPU_5->setProperty("SKT6", 32);
	ui->checkBPU_6->setProperty("SKT7", 64);
	ui->checkBPU_7->setProperty("SKT8", 128);

	std::map<std::string, DeviceStu> insertDev;
	AngKDeviceModel::instance().GetConnetDevMap(insertDev);
	for (auto iter : insertDev) {
		QString strDevName = QString::fromStdString(iter.second.tMainBoardInfo.strHardwareSN + "-" + iter.second.strSiteAlias);
		ui->deviceComboBox->addItem(strDevName, QVariant::fromValue(iter.second));
		ui->deviceComboBox2->addItem(strDevName, QVariant::fromValue(iter.second));
	}

	//用户视角来看是SKT1-SKT8
	ui->sktComboBox->addItem("SKT1", 1);
	ui->sktComboBox->addItem("SKT2", 2);
	ui->sktComboBox->addItem("SKT3", 4);
	ui->sktComboBox->addItem("SKT4", 8);
	ui->sktComboBox->addItem("SKT5", 16);
	ui->sktComboBox->addItem("SKT6", 32);
	ui->sktComboBox->addItem("SKT7", 64);
	ui->sktComboBox->addItem("SKT8", 128);

	ui->adapterInfoEdit->setReadOnly(true);
	ui->adapterUseInfoEdit->setReadOnly(true);
}

void ACAdapterCountInfomation::InitButton()
{
	SetAdaUseTab();
	connect(ui->tabWidget, &QTabWidget::tabBarClicked, this, &ACAdapterCountInfomation::onSlotSwitchTabBar);
	connect(ui->allCheck, &QCheckBox::clicked, this, &ACAdapterCountInfomation::onSlotSetBtnState);
	connect(ui->queryButton, &QPushButton::clicked, this, &ACAdapterCountInfomation::onSlotQueryAdapterInfo);
	connect(ui->queryButton2, &QPushButton::clicked, this, &ACAdapterCountInfomation::onSlotQueryAdapterInfo);
}

void ACAdapterCountInfomation::SetAdaUseTab()
{
	////ui->sktComboBox->hide();
	//ui->widget_2->show();
	//ui->allCheck->show();
}

void ACAdapterCountInfomation::SetAdaInfoTab()
{
	//ui->sktComboBox->show();
	//ui->widget_2->hide();
	//ui->allCheck->hide();
}

int ACAdapterCountInfomation::GetUseBPUEn()
{
	int nBPUEn = 0;
	if (ui->checkBPU_0->isChecked()) {
		nBPUEn += ui->checkBPU_0->property("SKT1").toInt();
	}
	if (ui->checkBPU_1->isChecked()) {
		nBPUEn += ui->checkBPU_1->property("SKT2").toInt();
	}
	if (ui->checkBPU_2->isChecked()) {
		nBPUEn += ui->checkBPU_2->property("SKT3").toInt();
	}
	if (ui->checkBPU_3->isChecked()) {
		nBPUEn += ui->checkBPU_3->property("SKT4").toInt();
	}
	if (ui->checkBPU_4->isChecked()) {
		nBPUEn += ui->checkBPU_4->property("SKT5").toInt();
	}
	if (ui->checkBPU_5->isChecked()) {
		nBPUEn += ui->checkBPU_5->property("SKT6").toInt();
	}
	if (ui->checkBPU_6->isChecked()) {
		nBPUEn += ui->checkBPU_6->property("SKT7").toInt();
	}
	if (ui->checkBPU_7->isChecked()) {
		nBPUEn += ui->checkBPU_7->property("SKT8").toInt();
	}
	return nBPUEn;
}

void ACAdapterCountInfomation::TextFormat(QTextStream& textStream, QString strBPUID, QString strUID, int InstCnt, int nFailedCnt, int nLifeCycle)
{
	textStream << "\t" << strBPUID << " ";
	QString sktInfo = "UID:" + strUID + ", ";
	int currentCnt = InstCnt > nLifeCycle ? nLifeCycle : InstCnt;
	sktInfo += "Current:" + QString::number(currentCnt).leftJustified(6, ' ') + ", ";
	sktInfo += "FailCnt:" + QString::number(nFailedCnt).leftJustified(6, ' ') + ", ";
	sktInfo += "LifeCycle:" + QString::number(nLifeCycle).leftJustified(6, ' ');
	textStream << sktInfo << endl;
}

void ACAdapterCountInfomation::TextDetailFormat(QTextStream& textStream, QString strBPUID, int InstCnt, int nFailedCnt, int nLifeCycle)
{
	textStream << "\t" << strBPUID << " ";
	QString strFormat;
	int currentCnt = InstCnt > nLifeCycle ? nLifeCycle : InstCnt;
	strFormat += "Current:" + QString::number(currentCnt).leftJustified(6, ' ') + ", ";
	strFormat += "FailCnt:" + QString::number(nFailedCnt).leftJustified(6, ' ') + ", ";
	textStream << strFormat << endl;
}

void ACAdapterCountInfomation::onSlotSetBtnState(bool bCheck)
{
	ui->checkBPU_0->setEnabled(!bCheck);
	ui->checkBPU_1->setEnabled(!bCheck);
	ui->checkBPU_2->setEnabled(!bCheck);
	ui->checkBPU_3->setEnabled(!bCheck);
	ui->checkBPU_4->setEnabled(!bCheck);
	ui->checkBPU_5->setEnabled(!bCheck);
	ui->checkBPU_6->setEnabled(!bCheck);
	ui->checkBPU_7->setEnabled(!bCheck);
	ui->deviceComboBox->setEnabled(!bCheck);
}

void ACAdapterCountInfomation::onSlotQueryAdapterInfo()
{
	int nIndex = ui->tabWidget->currentIndex();
	int BPUEn = -1;

	if (ui->deviceComboBox->count() < 1 && nIndex == 0) {
		return;
	}
	if (ui->deviceComboBox2->count() < 1 && nIndex == 1) {
		return;
	}

	if (ui->allCheck->isChecked() && nIndex == 0) {
		ui->adapterUseInfoEdit->clear();
		int nDeviceCount = ui->deviceComboBox->count();
		for (int i = 0; i < nDeviceCount; ++i) {
			DeviceStu devInfo = ui->deviceComboBox->itemData(i).value<DeviceStu>();
			BPUEn = 255;
			emit sgnQueryAdapterInfo(nIndex, BPUEn, devInfo.strIP, devInfo.nHopNum);
		}
		return;
	}


	if (nIndex != 0) {
		ui->adapterInfoEdit->clear();
		DeviceStu selDevceInfo = ui->deviceComboBox2->currentData().value<DeviceStu>();
		BPUEn = ui->sktComboBox->currentData().toInt();
		if (BPUEn != -1) {
			emit sgnQueryAdapterInfo(nIndex, BPUEn, selDevceInfo.strIP, selDevceInfo.nHopNum);
		}
	} else {
		ui->adapterUseInfoEdit->clear();
		DeviceStu selDevceInfo = ui->deviceComboBox->currentData().value<DeviceStu>();
		BPUEn = GetUseBPUEn();
		if (BPUEn == 0)
			return;
		emit sgnQueryAdapterInfo(nIndex, BPUEn, selDevceInfo.strIP, selDevceInfo.nHopNum);
	}

}

void ACAdapterCountInfomation::onSlotShowSktInfo(std::string strInfo, std::string strIPHop)
{
	if (strInfo.empty()) {
		ui->adapterInfoEdit->clear();
		return;
	}

	try {
		QString textOut;
		QTextStream stream(&textOut);
		nlohmann::json getSktInfoJson = nlohmann::json::parse(strInfo);
		DeviceStu devShowInfo = AngKDeviceModel::instance().GetConnetDev(strIPHop);
		stream << QString::fromStdString(devShowInfo.tMainBoardInfo.strHardwareSN) << "-" << QString::fromStdString(devShowInfo.strSiteAlias) << "-" << ui->sktComboBox->currentText() << endl;
		int nBPUCount = getSktInfoJson["BPUInfo"].size();
		for (int i = 0; i < nBPUCount; ++i) {
			nlohmann::json BPUInfoJson = getSktInfoJson["BPUInfo"][i];
			int nBPUIdx = BPUInfoJson["BPUIdx"];
			nlohmann::json SKTInfoJson = BPUInfoJson["SKTInfo"];
			std::string strUID = SKTInfoJson["UID"];
			std::string strID = SKTInfoJson["ID"];
			std::string strDate = SKTInfoJson["Date"];
			std::string strSerial = SKTInfoJson["Serial"];
			std::string strCustomName = SKTInfoJson["CustomName"].is_null() ? "" : SKTInfoJson["CustomName"];
			int nLifeCycleCnt = SKTInfoJson["LifeCycleCnt"];
			int nLifeCycleShow = SKTInfoJson["LifeCycleShow"];
			int nHeadCnt = SKTInfoJson["HeadCnt"];
			int nInstCnt0 = SKTInfoJson["InstCnt0"];
			int nFailCnt0 = SKTInfoJson["FailCnt0"];
			int nInstCnt1 = SKTInfoJson["InstCnt1"];
			int nFailCnt1 = SKTInfoJson["FailCnt1"];

			stream << "\t" << "UID: " << QString::fromStdString(strUID) << endl;
			stream << "\t" << "ID: " << QString::fromStdString(strID) << endl;
			stream << "\t" << "Date: " << QString::fromStdString(strDate) << endl;
			stream << "\t" << "Serial: " << QString::fromStdString(strSerial) << endl;
			stream << "\t" << "LifeCycleShow: " << QString::number(nLifeCycleShow) << endl;
			stream << "\t" << "Customer: " << QString::fromStdString(strCustomName) << endl;
			if (nHeadCnt == 1) {
				QString skt0Id = "SKT" + QString::number(nBPUIdx + 1);
				TextDetailFormat(stream, skt0Id, nInstCnt0, nFailCnt0, nLifeCycleShow);
			}
			else {
				QString skt0Id = "SKT" + QString::number(nBPUIdx + 1) + ".A";
				QString skt1Id = "SKT" + QString::number(nBPUIdx + 1) + ".B";
				TextDetailFormat(stream, skt0Id, nInstCnt0, nFailCnt0, nLifeCycleShow);
				TextDetailFormat(stream, skt1Id, nInstCnt1, nFailCnt1, nLifeCycleShow);

			}
		}
		ui->adapterInfoEdit->setText(textOut);
	}
	catch (const nlohmann::json::exception& e) {
		ALOG_FATAL("return GetSktInfo Json parse failed : %s.", "CU", "--", e.what());
	}
}

void ACAdapterCountInfomation::onSlotShowSktInfoSimple(std::string strInfo, std::string strIPHop)
{
	if (strInfo.empty()) {
		ui->adapterUseInfoEdit->clear();
		return;
	}
	try {
		QString textOut;
		QTextStream stream(&textOut);
		nlohmann::json getSktInfoSimpleJson = nlohmann::json::parse(strInfo);
		DeviceStu devShowInfo = AngKDeviceModel::instance().GetConnetDev(strIPHop);
		stream << QString::fromStdString(devShowInfo.tMainBoardInfo.strHardwareSN) << "-" << QString::fromStdString(devShowInfo.strSiteAlias) << endl;

		if (getSktInfoSimpleJson.is_null() || !getSktInfoSimpleJson.is_object()) {
			stream << "\t" << "Get SktInfo Simple Error";
			ui->adapterUseInfoEdit->setPlainText(textOut);
			return;
		}

		int nBPUCount = getSktInfoSimpleJson["BPUInfo"].size();
		for (int i = 0; i < nBPUCount; ++i) {
			nlohmann::json BPUInfoJson = getSktInfoSimpleJson["BPUInfo"][i];
			int nBPUIdx = BPUInfoJson["BPUIdx"];
			nlohmann::json SKTInfoJson = BPUInfoJson["SKTInfo"];
			std::string strUID = SKTInfoJson["UID"];
			int nLifeCycleShow = SKTInfoJson["LifeCycleShow"];
			int nHeadCnt = SKTInfoJson["HeadCnt"];
			int nInstCnt0 = SKTInfoJson["InstCnt0"];
			int nFailCnt0 = SKTInfoJson["FailCnt0"];
			int nInstCnt1 = SKTInfoJson["InstCnt1"];
			int nFailCnt1 = SKTInfoJson["FailCnt1"];
			if (nHeadCnt == 1) {
				QString skt0Id = "SKT" + QString::number(nBPUIdx + 1);
				TextFormat(stream, skt0Id, QString::fromStdString(strUID), nInstCnt0, nFailCnt0, nLifeCycleShow);
			}
			else
			{
				QString skt0Id = "SKT" + QString::number(nBPUIdx + 1) + ".A";
				QString skt1Id = "SKT" + QString::number(nBPUIdx + 1) + ".B";
				TextFormat(stream, skt0Id, QString::fromStdString(strUID), nInstCnt0, nFailCnt0, nLifeCycleShow);
				TextFormat(stream, skt1Id, QString::fromStdString(strUID), nInstCnt1, nFailCnt1, nLifeCycleShow);

			}
		}
		if (ui->allCheck->isChecked()) {
			ui->adapterUseInfoEdit->append(textOut);
		}
		else {
			ui->adapterUseInfoEdit->setPlainText(textOut);
		}
	}
	catch (const nlohmann::json::exception& e) {
		ALOG_FATAL("return GetSktInfoSimple Json parse failed : %s.", "CU", "--", e.what());
	}
}

void ACAdapterCountInfomation::onSlotSwitchTabBar(int nIndex)
{
	if (nIndex != 0) {//AdapterInfo
		SetAdaInfoTab();
	}
	else {//AdapterUseInfo
		SetAdaUseTab();
	}
}