#include "ACProgrammerTestDlg.h"
#include "ACDeviceManager.h"
#include "ui_ACProgrammerTestDlg.h"
#include "StyleInit.h"
#include "MessageNotify/notifyManager.h"

ACProgrammerTestDlg::ACProgrammerTestDlg(QWidget *parent)
	: AngKDialog(parent)
	, m_nAllDevNums(0)
	, bStartTest(false)
{
	this->setObjectName("ACProgrammerTestDlg");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::ACProgrammerTestDlg();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(636, 300);
	this->SetTitle(tr("Programmer Test"));

	InitText();

	connect(this, &ACProgrammerTestDlg::sgnClose, this, &ACProgrammerTestDlg::close);
	connect(ui->testButton, &QPushButton::clicked, this, &ACProgrammerTestDlg::onSlotProgrammgerTest);
}

ACProgrammerTestDlg::~ACProgrammerTestDlg()
{
	delete ui;
}

void ACProgrammerTestDlg::InitText()
{
	ui->testButton->setText(tr("Test"));
	ui->tipLabel->setText(tr("Please select the part to be tested"));
	ui->DDR_Check->setText(tr("DDR_Test"));
	ui->SPIFlash_Check->setText(tr("SPIFlash_Test"));
	ui->SSD_Check->setText(tr("SSD_Test"));
	ui->DPS_Check->setText(tr("DPS_Test"));
}

void ACProgrammerTestDlg::onSlotProgramSelfTestResult(QString testResult, QString ipStr, int nHopNum)
{
	m_nAllDevNums--;
	bool bOk;
	int nResult = testResult.toInt(&bOk, 16);
	QVector<QString> successVec;
	QVector<QString> failedVec;

	if (ui->DDR_Check->isChecked()) {
		(nResult & 1) == 0 ? successVec.push_back("DDR") : failedVec.push_back("DDR");
	}
	if (ui->SPIFlash_Check->isChecked()) {
		(nResult & (1 << 1)) == 0 ? successVec.push_back("SPIFlash") : failedVec.push_back("SPIFlash");
	}
	if (ui->SSD_Check->isChecked()) {
		(nResult & (1 << 2)) == 0 ? successVec.push_back("SSD") : failedVec.push_back("SSD");
	}
	if (ui->DPS_Check->isChecked()) {
		(nResult & (1 << 8)) == 0 ? successVec.push_back("DPS") : failedVec.push_back("DPS");
	}

	QString successMsg, failedMsg;
	for (int i = 0; i < successVec.size(); ++i) {
		if (i != successVec.size() - 1)
		{
			successMsg += successVec[i] + ",";
		}

		successMsg += successVec[i];
	}

	for (int i = 0; i < failedVec.size(); ++i) {
		if (i != failedVec.size() - 1)
		{
			failedMsg += failedVec[i] + ",";
		}

		failedMsg += failedVec[i];
	}

	//if(!successMsg.isEmpty())
	//	NotifyManager::instance().notify(tr("Notify"), tr("Device %1 TestCheck Result : %2, <font color=\"green\">successfully</font>.").arg(ipStr + ":" + QString::number(nHopNum)).arg(successMsg));

	//if (!failedMsg.isEmpty())
	//	NotifyManager::instance().notify(tr("Notify"), tr("Device %1 TestCheck Result : %2, <font color=\"red\">failed</font>.").arg(ipStr + ":" + QString::number(nHopNum)).arg(failedMsg));

	if (!failedMsg.isEmpty())
		ui->tipLabel->setText(tr("Device %1 TestCheck Result : %2, <font color=\"red\">failed</font>.").arg(ipStr + ":" + QString::number(nHopNum)).arg(failedMsg) + "<br>" + ui->tipLabel->text());

	if (m_nAllDevNums == 0) {
		bStartTest = false;
		QString tmpStr = ui->tipLabel->text();
		ui->tipLabel->setText("");
		QString resultStr = "";
		for (auto it : tmpStr.split("<br>"))
		{
			if (it.indexOf("failed") > 0){
				if (resultStr.size() > 0)
					resultStr += "<br>" + it;
				else
					resultStr = it;
			}
		}

		if (resultStr.size() > 0)
			ui->tipLabel->setText(tr("<font color=\"red\">Errors occurred</font>") + "<br>" + resultStr + "<br>");
		else
			ui->tipLabel->setText(tr("All device self checked <font color=\"green\">successfully</font>") + "<br>");
		ui->testButton->setEnabled(true);
	}
}

void ACProgrammerTestDlg::onSlotTestTimeout()
{
	if (bStartTest)
	{
		bStartTest = false;
		ui->tipLabel->setText(tr("The device self-test timed out, please try again!"));
		ui->testButton->setEnabled(true);
	}
}

void ACProgrammerTestDlg::onSlotProgrammgerTest()
{
	int TestEnable = 0;
	m_nAllDevNums = ACDeviceManager::instance().getAllDevNum();
	if (ui->DDR_Check->isChecked()) {
		TestEnable |= 1;
	}
	if (ui->SPIFlash_Check->isChecked()) {
		TestEnable |= (1 << 1);
	}
	if (ui->SSD_Check->isChecked()) {
		TestEnable |= (1 << 2);
	}
	if (ui->DPS_Check->isChecked()) {
		TestEnable |= (1 << 3);
	}

	if (TestEnable == 0) {
		ui->tipLabel->setText(tr("Please select a program test item."));
		return;
	}

	int inputEnable = TestEnable;
	emit sgnProgrammgerTest(TestEnable);

	QTimer::singleShot(1000 * 60, this, &ACProgrammerTestDlg::onSlotTestTimeout);
	bStartTest = true;

	ui->tipLabel->setText(tr("All devices are self checking, please do not operate..."));
	ui->testButton->setEnabled(false);
}