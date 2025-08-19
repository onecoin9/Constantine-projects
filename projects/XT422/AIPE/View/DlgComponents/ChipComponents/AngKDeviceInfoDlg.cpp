#include "AngKDeviceInfoDlg.h"
#include "ui_AngKDeviceInfoDlg.h"
#include "StyleInit.h"
#include "json.hpp"
#include "AngKCommonTools.h"

AngKDeviceInfoDlg::AngKDeviceInfoDlg(QWidget *parent)
	: AngKDialog(parent)
{
	this->setObjectName("AngKDeviceInfoDlg");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::AngKDeviceInfoDlg();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(800, 380);
	this->SetTitle(tr("Chip Information"));

	connect(this, &AngKDeviceInfoDlg::sgnClose, this, &AngKDeviceInfoDlg::close);
	connect(ui->okButton, &QPushButton::clicked, this, &AngKDeviceInfoDlg::close);
}

AngKDeviceInfoDlg::~AngKDeviceInfoDlg()
{
	delete ui;
}

void AngKDeviceInfoDlg::SetChipDeviceInfo(ChipDataJsonSerial chipInfo)
{
	ui->nameEdit->setText(QString::fromStdString(chipInfo.getJsonValue<std::string>("chipName")));
	ui->manuEdit->setText(QString::fromStdString(chipInfo.getJsonValue<std::string>("manufacture")));
	ui->typeEdit->setText(QString::fromStdString(chipInfo.getJsonValue<std::string>("chipType")));
	ui->driverFileEdit->setText(QString::fromStdString(chipInfo.getJsonValue<std::string>("chipAlgoFile")));
	ui->driverVerEdit->setText(QString::fromStdString(chipInfo.getJsonValue<std::string>("chipName")));
	//ui->mainVerEdit->setText(QString::fromStdString(chipInfo.getJsonValue<std::string>("")));
	//ui->svnVerEdit->setText(QString::fromStdString(chipInfo.getJsonValue<std::string>("")));

	ui->mainVerEdit->setText(Utils::AngKCommonTools::GetDesFileValue(Utils::AngKPathResolve::localTempFolderPath() + "Version.txt", "Version"));
	ui->svnVerEdit->setText(Utils::AngKCommonTools::GetDesFileValue(Utils::AngKPathResolve::localTempFolderPath() + "Version.txt", "SVNVersion"));


	ui->packageEdit->setText(QString::fromStdString(chipInfo.getJsonValue<std::string>("chipPackage")));
	ui->adapterEdit->setText(QString::fromStdString(chipInfo.getJsonValue<std::string>("chipAdapter")));
	ui->FPGAFileEdit->setText(QString::fromStdString(chipInfo.getJsonValue<std::string>("chipFPGAFile")));
	ui->FPGAFile2Edit->setText(QString::fromStdString(chipInfo.getJsonValue<std::string>("chipFPGAFile2")));
	ui->algoICEdit->setText(QString::number(chipInfo.getJsonValue<unsigned long>("chipDrvParam")));
	ui->bufferSizeEdit->setText(QString::number(chipInfo.getJsonValue<unsigned long>("chipBufferSize")));
	ui->securitySlnEdit->setText("");

	ui->nameEdit->setCursorPosition(0);
	ui->manuEdit->setCursorPosition(0);
	ui->typeEdit->setCursorPosition(0);
	ui->driverFileEdit->setCursorPosition(0);
	ui->driverVerEdit->setCursorPosition(0);
	ui->mainVerEdit->setCursorPosition(0);
	ui->svnVerEdit->setCursorPosition(0);
	ui->packageEdit->setCursorPosition(0);
	ui->adapterEdit->setCursorPosition(0);
	ui->FPGAFileEdit->setCursorPosition(0);
	ui->FPGAFile2Edit->setCursorPosition(0);
	ui->algoICEdit->setCursorPosition(0);
	ui->bufferSizeEdit->setCursorPosition(0);
	ui->securitySlnEdit->setCursorPosition(0);

	//checkBox根据chipOperCfg来判断
	std::string strChipOper = chipInfo.getJsonValue<std::string>("chipOperCfgJson");

	try {//nlohmann解析失败会报异常需要捕获一下
		auto j3 = nlohmann::json::parse(strChipOper);
		bool bBigEndian = j3["fileLoadOper"]["bigEndian"];
		bool bWordAddress = j3["fileLoadOper"]["wordAddress"];

		ui->BigendianCheck->setChecked(bBigEndian);
		ui->wordaddressCheck->setChecked(bWordAddress);
		ui->UIDSupportCheck->setEnabled(false);
	}
	catch (const nlohmann::json::exception& e) {
		ALOG_FATAL("chipOperCfgJson Json parse failed : %s.", "CU", "--", e.what());
	}
}
