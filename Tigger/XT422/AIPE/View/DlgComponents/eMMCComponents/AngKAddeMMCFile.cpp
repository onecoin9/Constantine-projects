#include "AngKAddeMMCFile.h"
#include "ui_AngKAddeMMCFile.h"
#include "AngKPathResolve.h"
#include "ProgressDialogSingleton.h"
#include "StyleInit.h"
#include "ACMessageBox.h"
#include <QFileDialog>

AngKAddeMMCFile::AngKAddeMMCFile(QWidget *parent)
	: AngKDialog(parent)
{
	this->setObjectName("AngKAddeMMCFile");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::AngKAddeMMCFile();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(550, 260);
	this->SetTitle(tr("Program On Partition"));
	setAttribute(Qt::WA_TranslucentBackground, true);

	InitText();
	InitButton();

	m_TempVecRecord.clear();
}

AngKAddeMMCFile::~AngKAddeMMCFile()
{
	m_TempVecRecord.clear();
	delete ui;
}

void AngKAddeMMCFile::InitText()
{
	ui->binFileText->setText(tr("Bin File:"));
	ui->progOnPartitionText->setText(tr("Program On Partition:"));
	ui->startAddressText->setText(tr("Start Address:"));
	ui->startAddrText->setText(tr("StartAddr=0x0，Size=0x0 Bytes, EndAddr=0x0"));
	ui->userPartitionText->setText(tr("USER Partition Size = 0 MBytes, Range(0-0xFFFFFFFFFFFFFFFF)"));

	ui->browseButton->setText(tr(""));
	ui->okButton->setText(tr("OK"));
	ui->cancelButton->setText(tr("Cancel"));

	stuEFileInfo.clear();
}

void AngKAddeMMCFile::InitButton()
{
	connect(ui->browseButton, &QPushButton::clicked, this, [=]() {
		QString filePath = QFileDialog::getOpenFileName(this, 
			"Select File...", 
			"",
			tr(" bin Files(*.bin);; All Files(*.*)"));
			
		if (!filePath.isEmpty()) {
			stuEFileInfo.sFilePath = filePath.toStdString();
			ui->binFileEdit->setText(filePath);
		}
	});

	connect(ui->okButton, &QPushButton::clicked, this, &AngKAddeMMCFile::onSlotAddeMMCFile);

	connect(ui->cancelButton, &QPushButton::clicked, this, &AngKAddeMMCFile::close);

	ui->partitionComboBox->addItem("USER", (int)ProgramOnPartition::USER);
	ui->partitionComboBox->addItem("BOOT1", (int)ProgramOnPartition::BOOT1);
	ui->partitionComboBox->addItem("BOOT2", (int)ProgramOnPartition::BOOT2);
	ui->partitionComboBox->addItem("RPMB", (int)ProgramOnPartition::RPMB);
	ui->partitionComboBox->addItem("GPP1", (int)ProgramOnPartition::GPP1);
	ui->partitionComboBox->addItem("GPP2", (int)ProgramOnPartition::GPP2);
	ui->partitionComboBox->addItem("GPP3", (int)ProgramOnPartition::GPP3);
	ui->partitionComboBox->addItem("GPP4", (int)ProgramOnPartition::GPP4);

	connect(ui->partitionComboBox, &QComboBox::currentTextChanged, this, &AngKAddeMMCFile::onAutoCalPartitionPos);

	connect(this, &AngKAddeMMCFile::sgnClose, this, &AngKAddeMMCFile::close);
}

void AngKAddeMMCFile::seteMMCParams(eMMCFileInfo eInfo)
{
	ui->binFileEdit->setText(QString::fromStdString(eInfo.sFilePath));
	ui->partitionComboBox->setCurrentText(QString::fromStdString(eInfo.sPartitionName));
	ui->startAddrEdit->setText(QString::number(eInfo.nStartAddr, 16));
	stuEFileInfo = eInfo;
}

void AngKAddeMMCFile::setFileRecord(QVector<eMMCFileRecord> vecFileReocrd)
{
	m_TempVecRecord = vecFileReocrd;
	onAutoCalPartitionPos("USER");
}

void AngKAddeMMCFile::onAutoCalPartitionPos(QString selText)
{
	int recordNum = 0;
	bool bFind = false;
	ui->startAddrEdit->clear();
	for (int i = m_TempVecRecord.size() - 1;i >= 0; i--)
	{
		if (m_TempVecRecord[i].fileArea == selText)
		{
			recordNum = i;
			bFind = true;
			break;
		}
	}

	if (bFind)
	{
		uint64_t newStartAddr = m_TempVecRecord[recordNum].StartAddr + m_TempVecRecord[recordNum].fileSize;
		if (newStartAddr % 512 != 0) {
			uint64_t nPart = newStartAddr / 512;

			newStartAddr = (nPart + 1) * 512;
		}

		ui->startAddrEdit->setText(QString::number(newStartAddr, 16));
	}
}

int calc_crc16sum(unsigned char* buf, unsigned int size, unsigned short* pCRC16Sum);

void AngKAddeMMCFile::onSlotAddeMMCFile()
{
	//先计算当前插入的地址是否是512Byte的整数倍
	bool bOk = false;
	uint64_t strAddr = ui->startAddrEdit->text().toULongLong(&bOk, 16);

	if (ui->binFileEdit->text().isEmpty()) {
		ACMessageBox::showError(this, QObject::tr("Warning"), QObject::tr("Please select the file first."));
		return;
	}

	if (strAddr % 512 != 0) {
		ACMessageBox::showError(this, QObject::tr("Warning"), QObject::tr("Sorry, the start address of the file must be 512 Bytes aligned."));
		return;
	}

	uint64_t fileSize = 0;
	ushort crc16 = 0;
	QFile file(QString::fromStdString(stuEFileInfo.sFilePath));

	if (file.open(QIODevice::ReadOnly)) {
		qDebug() << "File size: " << file.size();
		fileSize = file.size();
		file.close();
	}

	bool ok;
	stuEFileInfo.bSectorAlign = false;
	stuEFileInfo.nCheckSum = crc16;
	stuEFileInfo.sPartitionName = ui->partitionComboBox->currentText().toStdString();
	stuEFileInfo.nFileSize = fileSize;
	stuEFileInfo.nStartAddr = ui->startAddrEdit->text().toULongLong(&ok, 16);
	emit sgnAddeMMCFile(stuEFileInfo);

	close();
}