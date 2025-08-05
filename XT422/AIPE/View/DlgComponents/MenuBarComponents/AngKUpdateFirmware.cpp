#include "AngKUpdateFirmware.h"
#include "ui_AngKUpdateFirmware.h"
#include "../View/GlobalInit/StyleInit.h"
#include "AngKProgressDelegate.h"
#include "ACMessageBox.h"

#include "AngKCheckBoxHeader.h"
#include <QFileDialog>
#include <QHeaderView>
#include <QRandomGenerator>
#include <QStandardItemModel>
#include <QThread>
#include <QMessageBox>
#include <QTimer>


Q_DECLARE_METATYPE(DeviceStu);

AngKUpdateFirmware::AngKUpdateFirmware(QWidget *parent, int nHopNumUse)
	: AngKDialog(parent)
	, m_ProgressTableModel(nullptr)
{
	this->setObjectName("AngKUpdateFirmware");
	QT_SET_STYLE_SHEET(objectName());
	
	ui = new Ui::AngKUpdateFirmware();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(650, 480);
	this->SetTitle(tr("UpdateFirmware"));

	InitText();
	InitButton();
	InitTable(nHopNumUse);
}

AngKUpdateFirmware::~AngKUpdateFirmware()
{
	delete ui;
}

void AngKUpdateFirmware::InitText()
{
	ui->fileText->setText(tr("File:"));
	ui->okButton->setText(tr("Ok"));
}

void AngKUpdateFirmware::InitButton()
{
	connect(ui->startButton, &QPushButton::clicked, this, &AngKUpdateFirmware::onSlotUpdateFirmware);
	connect(ui->okButton, &QPushButton::clicked, this, &AngKUpdateFirmware::onSlotCheckClose);
	connect(ui->fileButton, &QPushButton::clicked, this, [=](){
		QString filePath = QFileDialog::getOpenFileName(this, tr("Select UpdateFile..."), QCoreApplication::applicationDirPath(), tr("efwm Files(*.efwm)"));

		ui->fileEdit->setText(filePath);
		});
	connect(this, &AngKUpdateFirmware::sgnClose, this, &AngKUpdateFirmware::onSlotCheckClose);
}

void AngKUpdateFirmware::InitTable(int _UseHop)
{

	std::map<std::string, DeviceStu> insertDev;
	AngKDeviceModel::instance().GetConnetDevMap(insertDev);


	m_ProgressTableModel = new QStandardItemModel(insertDev.size(), 4, this);
	AngKProgressDelegate* progressDelegate = new AngKProgressDelegate();
	progressDelegate->UpdateProgressColor("#b3d9ff");
	progressDelegate->setProgressIndex(2);

	// 隐藏水平表头
	ui->progressView->verticalHeader()->setVisible(false);
	ui->progressView->setMouseTracking(true);
	ui->progressView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	//QStringList headList;
	//headList << tr("") << tr("ProgrammerName") << tr("Update Progress") << tr("Status");

	//m_ProgressTableModel->setHorizontalHeaderLabels(headList);

	m_ProgressTableModel->setHeaderData(0, Qt::Horizontal, "");
	m_ProgressTableModel->setHeaderData(1, Qt::Horizontal, tr("ProgrammerName"));
	m_ProgressTableModel->setHeaderData(2, Qt::Horizontal, tr("Update Progress"));
	m_ProgressTableModel->setHeaderData(3, Qt::Horizontal, tr("Status"));
	AngKCheckBoxHeader* header = new AngKCheckBoxHeader(Qt::Horizontal, ui->progressView);
	header->setCheckBoxModel(m_ProgressTableModel, 0, "");
	ui->progressView->setHorizontalHeader(header);

	ui->progressView->setItemDelegateForColumn(2, progressDelegate);
	ui->progressView->setModel(m_ProgressTableModel);
	ui->progressView->setAlternatingRowColors(true);
	ui->progressView->horizontalHeader()->setHighlightSections(false);
	ui->progressView->horizontalHeader()->setStretchLastSection(true);
	ui->progressView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

	QHeaderView* manuHead = ui->progressView->horizontalHeader();

	manuHead->setSectionResizeMode(QHeaderView::Fixed);

	ui->progressView->setColumnWidth(0, 25);
	ui->progressView->setColumnWidth(1, 135);
	ui->progressView->setColumnWidth(2, 350);
	ui->progressView->setColumnWidth(3, 70);

	int increate = 0;
	for (auto iter : insertDev) {
		//m_ProgressTableModel->insertRow(increate);
		
		m_ProgressTableModel->setData(m_ProgressTableModel->index(increate, 0), QVariant::fromValue(iter.second), Qt::UserRole);
		m_ProgressTableModel->item(increate, 0)->setCheckable(true);
		QString devChain = QString::fromStdString(iter.second.strSiteAlias) + "[" + QString::number(iter.second.nChainID) + ":" + QString::number(iter.second.nHopNum) + "]";
		m_ProgressTableModel->setData(m_ProgressTableModel->index(increate, 1), devChain);
		m_ProgressTableModel->setData(m_ProgressTableModel->index(increate, 2), 0, Qt::UserRole + eIV_Value);
		m_ProgressTableModel->setData(m_ProgressTableModel->index(increate, 3), tr("Idle"));
		increate++;
	}
}

QStringList AngKUpdateFirmware::GetChainIDandHop(QString strChain)
{
	QStringList numbersList;
	int colonIndex = strChain.indexOf(":");
	if (colonIndex != -1)
	{
		// 提取冒号前后的字符串
		QString beforeColon = strChain.mid(strChain.indexOf("[") + 1, colonIndex - strChain.indexOf("[") - 1).trimmed(); // 提取冒号前的数字
		QString afterColon = strChain.mid(colonIndex + 1, strChain.indexOf("]") - colonIndex - 1).trimmed(); // 提取冒号后的数字

		// 将提取的数字保存在QStringList中
		
		numbersList << beforeColon << afterColon;
	}
	return numbersList;
}

bool AngKUpdateFirmware::CheckUpdating()
{
	bool bUpdate = false;
	for (int i = 0; i < m_ProgressTableModel->rowCount(); ++i) {
		QString curStatus = m_ProgressTableModel->data(m_ProgressTableModel->index(i, 3)).toString();
		if (curStatus == "Upgrading") {
			bUpdate = true;
			break;
		}
	}

	return bUpdate;
}

bool AngKUpdateFirmware::CheckFirmwareVersion(std::vector<int>& selectDevice, std::string& toVer)
{
	tFirmwareHeader fwHeader;
	memset(&fwHeader, 0, sizeof(tFirmwareHeader));
	QFile fwFile(ui->fileEdit->text());
	if (fwFile.open(QIODevice::ReadOnly)) {
		QByteArray byteArray = fwFile.read(256);
		if (byteArray.size() == 256) {
			// 将QByteArray转换为char数组并复制到结构体中
			memcpy(&fwHeader, byteArray.data(), 256);
		}
	}
	fwFile.close();
	for (int i = 0; i < 3; i++) {
		toVer += std::to_string(fwHeader.Version[i]);
		if (i != 2) {
			toVer += ".";
		}
	}

	QStringList fwToVersion = QString::fromStdString(toVer).split(".");
	for (int i = 0; i < selectDevice.size(); ++i) {
		DeviceStu seDev = m_ProgressTableModel->data(m_ProgressTableModel->index(selectDevice[i], 0), Qt::UserRole).value<DeviceStu>();
		QStringList fwVersion = QString::fromStdString(seDev.strFirmwareVersion).split(".");

		// 比较每个部分
		for (int i = 0; i < qMax(fwVersion.size(), fwToVersion.size()); ++i) {
			int num1 = i < fwVersion.size() ? fwVersion.at(i).toInt() : 0;
			int num2 = i < fwToVersion.size() ? fwToVersion.at(i).toInt() : 0;

			if (num1 > num2) {
				return true; // version1 更大
			}
			else if (num1 < num2) {
				return false; // version2 更大
			}
		}
	}

	return false; 
}

void AngKUpdateFirmware::onSlotUpdateFPGAValue(std::string recvIP, int nHopNum, int nValue)
{
	bool bCheckFinish = false;
	for (int i = 0; i < m_ProgressTableModel->rowCount(); ++i) {
		QString strIP = QString::fromStdString(m_ProgressTableModel->data(m_ProgressTableModel->index(i, 0), Qt::UserRole).value<DeviceStu>().strIP);
		QStringList devInfo = GetChainIDandHop(m_ProgressTableModel->data(m_ProgressTableModel->index(i, 1)).toString());
		if (devInfo.size() > 1 && strIP.toStdString() == recvIP && devInfo[1].toInt() == nHopNum) {
			//当进度为100时，改变状态
			if (nValue >= 100) {
				m_ProgressTableModel->setData(m_ProgressTableModel->index(i, 3), tr("Finish"));
				m_ProgressTableModel->setData(m_ProgressTableModel->index(i, 2), tr("Success"), Qt::UserRole + eIV_ProgressType);
				bCheckFinish = true;
			}
			m_ProgressTableModel->setData(m_ProgressTableModel->index(i, 2), nValue, Qt::UserRole + eIV_Value);
		}
	}

	return; // 修改软件重启至给所有设备下发reboot之后
	if (bCheckFinish) {
		int countSelect = 0;
		int countFinish = 0;
		for (int i = 0; i < m_ProgressTableModel->rowCount(); ++i) {
			if (m_ProgressTableModel->item(i, 0)->checkState() == Qt::CheckState::Checked) {
				countSelect++;
			}
		}

		for (int i = 0; i < m_ProgressTableModel->rowCount(); ++i) {
			if (m_ProgressTableModel->data(m_ProgressTableModel->index(i, 3)).toString() == "Finish") {
				countFinish++;
			}
		}

		if (countSelect == countFinish) {
			QTimer::singleShot(10000, this, &AngKUpdateFirmware::onSlotTimerRebootFinish);
			ACMessageBox::showInformation(this, tr("Restart"), tr("The software will restart..."), ACMessageBox::ACMsgButton::MSG_NO_BTN);
		}
	}
		
}

void AngKUpdateFirmware::onSlotCheckClose()
{
	bool isUpgrading = false;
	//遍历列表整体的状态
	isUpgrading = CheckUpdating();

	if (isUpgrading) {
		ACMessageBox::showWarning(this, tr("Warning"), tr("The firmware is currently being upgraded and must not be shut down."));
		return;
	}

	close();
}

void AngKUpdateFirmware::onSlotUpdateFwStatus(std::string recvIp, int nHopNum)
{
	for (int i = 0; i < m_ProgressTableModel->rowCount(); ++i) {
		QString strIP = QString::fromStdString(m_ProgressTableModel->data(m_ProgressTableModel->index(i, 0), Qt::UserRole).value<DeviceStu>().strIP);
		QStringList devInfo = GetChainIDandHop(m_ProgressTableModel->data(m_ProgressTableModel->index(i, 1)).toString());
		if (devInfo.size() > 1 && strIP.toStdString() == recvIp && devInfo[1].toInt() == nHopNum) {
			m_ProgressTableModel->setData(m_ProgressTableModel->index(i, 3), tr("Failed"));
			m_ProgressTableModel->setData(m_ProgressTableModel->index(i, 2), tr("Failed"), Qt::UserRole + eIV_ProgressType);
			m_ProgressTableModel->setData(m_ProgressTableModel->index(i, 2), 100, Qt::UserRole + eIV_Value);
			AngKUpdateFirmware::m_UpdateFwNum--;
		}
	}
}

void AngKUpdateFirmware::onSlotTimerRebootFinish()
{
	qApp->exit(MessageType::MESSAGE_RESTART);
}

void  AngKUpdateFirmware::onSlotUpdateFirmware()
{
	//检查选择了哪些设备
	std::vector<int> selectVec;
	for (int i = 0; i < m_ProgressTableModel->rowCount(); ++i){
		if (m_ProgressTableModel->item(i, 0)->checkState() == Qt::CheckState::Checked) {
			selectVec.push_back(i);
		}
	}

	if (selectVec.empty()) {
		ACMessageBox::showWarning(this, tr("SelectWarning"), tr("Please select which device to upgrade."));
		return;
	}

	if(ui->fileEdit->text().isEmpty()){
		ACMessageBox::showWarning(this, tr("Warning"), tr("Firmware upgrade file not selected."));
		return;
	}

	//文件名称校验，因为允许直接拷贝到输入栏
	{
		QFileInfo fInfo(ui->fileEdit->text());
		if (fInfo.suffix() != "efwm") {
			ACMessageBox::showWarning(this, tr("Warning"), tr("Input Firmware upgrade file is error"));
			return;
		}
	}

	//检验选择的固件版本文件是否比设备固件版本要低，低的话需要提示
	std::string toVer;
	if (CheckFirmwareVersion(selectVec, toVer)) {
		auto ret = ACMessageBox::showWarning(this, tr("Warning"), tr("The selected firmware file version is lower than the current version. Do you want to continue upgrading ?"), ACMessageBox::ACMsgButton::MSG_OK_CANCEL_BTN);
		if (ret != ACMessageBox::ACMsgType::MSG_OK) {
			return;
		}
	}

	//遍历列表整体的状态
	for (int i = 0; i < selectVec.size(); ++i) {
		QString curStatus = m_ProgressTableModel->data(m_ProgressTableModel->index(selectVec[i], 3)).toString();
		if (curStatus == "Upgrading") {
			ACMessageBox::showWarning(this, tr("Warning"), tr("The selected device is currently being upgraded , cannot undergo another upgrade."), ACMessageBox::ACMsgButton::MSG_OK_BTN);
			return;
		}
	}

	for (int i = 0; i < selectVec.size(); ++i) {
		QString strIP = QString::fromStdString(m_ProgressTableModel->data(m_ProgressTableModel->index(selectVec[i], 0), Qt::UserRole).value<DeviceStu>().strIP);
		QString fromVer = QString::fromStdString(m_ProgressTableModel->data(m_ProgressTableModel->index(selectVec[i], 0), Qt::UserRole).value<DeviceStu>().strFirmwareVersion);
		QStringList devInfo = GetChainIDandHop(m_ProgressTableModel->data(m_ProgressTableModel->index(selectVec[i], 1)).toString());
		m_ProgressTableModel->setData(m_ProgressTableModel->index(i, 2), tr("Success"), Qt::UserRole + eIV_ProgressType);
		m_ProgressTableModel->setData(m_ProgressTableModel->index(i, 2), 0, Qt::UserRole + eIV_Value);
		m_ProgressTableModel->setData(m_ProgressTableModel->index(selectVec[i], 3), tr("Upgrading"));
		emit sgnUpdateFirmwareFile(strIP, devInfo[1].toInt(), ui->fileEdit->text(), fromVer, QString::fromStdString(toVer));
	}

	m_UpdateFwNum = selectVec.size();
	//close();
}

int AngKUpdateFirmware::m_UpdateFwNum = 0;
