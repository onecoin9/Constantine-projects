#include "QtFileChange2Stream.h"
#include <QPushButton>
#include <QFileDialog>
#include <QCoreApplication>
#include <QTextStream>
#include <QByteArray>
#include <QSettings>
#include "json.hpp"

//MU Logical Location
#define MUStartAddr		(0xC0000000)
#define PCMUSectorSize	(0x800000)
#define SMAddr_PC2MU	(MUStartAddr - PCMUSectorSize);

#define DDRMULTIPLE 4096

QtFileChange2Stream::QtFileChange2Stream(QWidget *parent)
	: QMainWindow(parent)
	, m_globalSettings(nullptr)
{
	ui.setupUi(this);

	connect(ui.pushButton, &QPushButton::clicked, this, &QtFileChange2Stream::onSlotFileChange);
	connect(ui.fusionButton, &QPushButton::clicked, this, &QtFileChange2Stream::onSlotDrvFileFusion);

	InitSetting();
}

void QtFileChange2Stream::onSlotFileChange()
{
	QString filePath = QFileDialog::getOpenFileName(this, "Open File...", QCoreApplication::applicationDirPath(), tr("All Files(*.*)"));

	QFile pFile(filePath);
	if (!pFile.open(QIODevice::ReadOnly))
	{
		ui.textEdit->setText("Open File Failed");
		return;
	}

	QTextStream in(&pFile);

	// 读取文件内容
	QString fileContent = in.readAll();

	// 关闭文件
	pFile.close();

	// 将文件内容以hex形式展示在QTextEdit中
	QByteArray byteArray = fileContent.toUtf8();
	QString hexString = byteArray.toHex().toUpper(); // 转换为大写形式

	ui.textEdit->setText(hexString);
}

void QtFileChange2Stream::InitSetting()
{
	QString strAppPath = QCoreApplication::applicationDirPath();
	QSettings::setDefaultFormat(QSettings::IniFormat);
	bool bCreate = true;
	m_globalSettings = new QSettings(localGlobalSettingFile(bCreate), QSettings::IniFormat);
	m_globalSettings->setIniCodec("utf-8");

	if (!bCreate)
	{
		settingWriteValue("FilePath", "DriverFilePath", "");
		settingWriteValue("FilePath", "MatserDriverFilePath", "");
	}

	ui.devFIleEdit->setText(settingReadValue("FilePath", "DriverFilePath").toString());
	ui.masterFIleEdit->setText(settingReadValue("FilePath", "MatserDriverFilePath").toString());
}

QString QtFileChange2Stream::localGlobalSettingFile(bool& isCreate)
{
	QString settingPath = QCoreApplication::applicationDirPath() + "/Drivercfg.ini";
	if (!QFileInfo::exists(settingPath))
	{
		QFile pfile(settingPath);
		pfile.open(QIODevice::WriteOnly | QIODevice::Text);

		isCreate = false;

		pfile.close();
	}

	return settingPath;
}

QVariant QtFileChange2Stream::settingReadValue(QString strGroup, QString strValue)
{
	return m_globalSettings->value(strGroup + "/" + strValue);
}

void QtFileChange2Stream::settingWriteValue(QString strGroup, QString strName, QVariant strValue)
{
	m_globalSettings->beginGroup(strGroup);
	m_globalSettings->setValue(strName, strValue);
	m_globalSettings->endGroup();
}

int calc_crc16sum(unsigned char* buf, unsigned int size, unsigned short* pCRC16Sum);

void QtFileChange2Stream::onSlotDrvFileFusion()
{
	ui.textEdit->clear();

	if (ui.devFIleEdit->text().isEmpty() || ui.masterFIleEdit->text().isEmpty())
	{
		ui.textEdit->clear();
		ui.textEdit->setText("Driver File Cannot be empty");
		return;
	}

	QByteArray Driver_IC_Data;
	QString DDadrvFile = ui.devFIleEdit->text();//Device Driver
	QString MDadrvFile = ui.masterFIleEdit->text();//Master Driver

	nlohmann::json AllDriverJson;
	AllDriverJson["FileType"] = "Driver";
	AllDriverJson["IsCompress"] = 0;

	QFile ddFile(DDadrvFile);
	if (ddFile.open(QIODevice::ReadOnly)) {

		int nByte = ddFile.size();
		int a_Blk = 0;

		if (nByte % DDRMULTIPLE == 0)
			a_Blk = nByte / DDRMULTIPLE;
		else
			a_Blk = (nByte / DDRMULTIPLE) + 1;

		int driverSize = a_Blk * DDRMULTIPLE;
		ushort crc16 = 0;
		QByteArray filedata = ddFile.readAll();
		calc_crc16sum((uchar*)(filedata.data()), nByte, &crc16);

		QByteArray ddFileByte;
		ddFileByte.resize(driverSize);
		ddFileByte.fill(0);

		ddFileByte.replace(0, nByte, filedata);

		Driver_IC_Data.push_back(ddFileByte);

		AllDriverJson["DevDrvDDRAddr"] = (QString("0x") + QString("%1").arg(MUStartAddr - PCMUSectorSize, 8, 16, QLatin1Char('0')).toUpper()).toStdString();
		AllDriverJson["DevDrvDataLen"] = (QString("0x") + QString("%1").arg(driverSize, 8, 16, QLatin1Char('0')).toUpper()).toStdString();
		AllDriverJson["DevDrvCRC16"] = (QString("0x") + QString("%1").arg(crc16, 4, 16, QLatin1Char('0')).toUpper()).toStdString();
	}
	else {
		return;
	}
	ddFile.close();

	QFile mdFile(MDadrvFile);
	if (mdFile.open(QIODevice::ReadOnly)) {

		AllDriverJson["MstDrvDDRAddr"] = (QString("0x") + QString("%1").arg(MUStartAddr - PCMUSectorSize + Driver_IC_Data.size(), 8, 16, QLatin1Char('0')).toUpper()).toStdString();

		int nByte = mdFile.size();
		int a_Blk = 0;

		if (nByte % DDRMULTIPLE == 0)
			a_Blk = nByte / DDRMULTIPLE;
		else
			a_Blk = (nByte / DDRMULTIPLE) + 1;

		ushort crc16 = 0;
		QByteArray filedata = mdFile.readAll();
		calc_crc16sum((uchar*)(filedata.data()), nByte, &crc16);

		int driverSize = a_Blk * DDRMULTIPLE;
		QByteArray ddFileByte;
		ddFileByte.resize(driverSize);
		ddFileByte.fill(0);

		ddFileByte.replace(0, nByte, filedata);

		Driver_IC_Data.push_back(ddFileByte);

		AllDriverJson["MstDrvDataLen"] = (QString("0x") + QString("%1").arg(driverSize, 8, 16, QLatin1Char('0')).toUpper()).toStdString();
		AllDriverJson["MstDrvCRC16"] = (QString("0x") + QString("%1").arg(crc16, 4, 16, QLatin1Char('0')).toUpper()).toStdString();
	}
	else {
		return;
	}
	mdFile.close();

	QFile tempFIle(QCoreApplication::applicationDirPath() + "/testDriver_IC_Data.bin");
	if (tempFIle.open(QIODevice::ReadWrite | QIODevice::Truncate))
	{
		tempFIle.write(Driver_IC_Data);
	}
	tempFIle.close();

	// 将文件内容以hex形式展示在QTextEdit中
	QByteArray byteArray = QString::fromStdString(AllDriverJson.dump()).toUtf8();
	QString hexString = byteArray.toHex().toUpper(); // 转换为大写形式

	ui.textEdit->setText(hexString);

	settingWriteValue("FilePath", "DriverFilePath", DDadrvFile);
	settingWriteValue("FilePath", "MatserDriverFilePath", MDadrvFile);

	QFile tempdFIle(QCoreApplication::applicationDirPath() + "/InstallDriver.json");
	if (tempdFIle.open(QIODevice::ReadWrite | QIODevice::Truncate))
	{
		QByteArray byteArray = QString::fromStdString(AllDriverJson.dump()).toUtf8();
		tempdFIle.write(byteArray);
	}
	tempdFIle.close();

}
