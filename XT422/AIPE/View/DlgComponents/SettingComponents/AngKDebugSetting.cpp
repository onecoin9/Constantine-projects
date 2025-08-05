#include "AngKDebugSetting.h"
#include "ui_AngKDebugSetting.h"
#include "AngKDeviceModel.h"
#include "AngKMessageHandler.h"
#include "StyleInit.h"
#include "ACDeviceManager.h"
#include "ACMessageBox.h"
#include <QStandardItemModel>
#include <QHeaderView>
#include <QPushButton>
#include <QComboBox>

AngKDebugSetting::AngKDebugSetting(QMap<QString, int>* uartCbIdxMap, QMap<QString, int>* logCbIdxMap, QWidget* parent)
	: AngKDialog(parent)
	, m_pUartComboboxIndexMap(uartCbIdxMap)
	, m_pDebugProgTableModel(nullptr)
	, m_pLogLevelComboboxIndexMap(logCbIdxMap)
{
	this->setObjectName("AngKDebugSetting");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::AngKDebugSetting();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(760, 480);
	this->SetTitle(tr("Debug Setting"));
	setAttribute(Qt::WA_TranslucentBackground, true);

	InitTable();

	connect(this, &AngKDebugSetting::sgnClose, this, &AngKDebugSetting::close);
}

AngKDebugSetting::~AngKDebugSetting()
{
	delete ui;
}

void AngKDebugSetting::InitTable()
{
	m_pDebugProgTableModel = new QStandardItemModel(this);

	// 隐藏水平表头
	ui->debugProgramView->verticalHeader()->setVisible(false);
	ui->debugProgramView->setMouseTracking(true);
	ui->debugProgramView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QStringList headList;
	headList << tr("ProgrammerName") << tr("Switch UART") << tr("DevLogLevel") << tr("DebugAction");

	m_pDebugProgTableModel->setHorizontalHeaderLabels(headList);

	ui->debugProgramView->setModel(m_pDebugProgTableModel);
	ui->debugProgramView->setAlternatingRowColors(true);
	ui->debugProgramView->horizontalHeader()->setHighlightSections(false);
	ui->debugProgramView->horizontalHeader()->setStretchLastSection(true);
	ui->debugProgramView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

	QHeaderView* manuHead = ui->debugProgramView->horizontalHeader();

	manuHead->setSectionResizeMode(QHeaderView::Fixed);

	ui->debugProgramView->setColumnWidth(0, 360);
	ui->debugProgramView->setColumnWidth(1, 110);
	ui->debugProgramView->setColumnWidth(2, 110);
	ui->debugProgramView->setColumnWidth(3, 90);

	InitDevice();
}

void AngKDebugSetting::InitDevice()
{
	QString _path = Utils::AngKPathResolve::localSkinFile(objectName(), true);
	QFile f(_path);
	f.open(QFile::ReadOnly);
	if (f.isOpen())
	{
		
	}

	// set click event
	int increate = 0;
	//std::map<std::string, DeviceStu> insertDev;
	//
	//AngKDeviceModel::instance().GetConnetDevMap(insertDev);

	auto insertDev = ACDeviceManager::instance().getAllDevInfo();
	for (auto iter : insertDev) {
		if (!iter.bOnLine)
			continue;
		m_pDebugProgTableModel->insertRow(increate);

		//QString devChain = QString::fromStdString(iter.second.strSiteAlias) + "[" + QString::number(iter.second.nChainID) + ":" + QString::number(iter.second.nHopNum) + "]";
		m_pDebugProgTableModel->setData(m_pDebugProgTableModel->index(increate, 0), QString::fromStdString(iter.strIP) + ":" + QString::number(iter.nHopNum));

		QComboBox* uartCombox = new QComboBox(this);
		QComboBox* logLevelCombox = new QComboBox(this);
		
		uartCombox->setStyleSheet(QString(f.readAll()));
		InitUARTCombobox(uartCombox, iter.strIP, iter.nHopNum);
		connect(uartCombox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AngKDebugSetting::onTableSwitchUART);
		ui->debugProgramView->setIndexWidget(m_pDebugProgTableModel->index(increate, 1), uartCombox);

		logLevelCombox->setStyleSheet(QString(f.readAll()));
		InitLogLevelCombobox(logLevelCombox, iter.strIP, iter.nHopNum);
		connect(logLevelCombox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AngKDebugSetting::onTableSwitchLogLevel);
		ui->debugProgramView->setIndexWidget(m_pDebugProgTableModel->index(increate, 2), logLevelCombox);

		QPushButton* button = new QPushButton(tr("Reboot"), this);
		button->setStyleSheet(QString(f.readAll()));
		connect(button, SIGNAL(clicked()), this, SLOT(onTableRebootBtnClicked()));
		ui->debugProgramView->setIndexWidget(m_pDebugProgTableModel->index(increate, 3), button);
		increate++;
	}

	f.close();
}

void AngKDebugSetting::InitUARTCombobox(QComboBox* combox, const std::string& ip, int hop)
{
	combox->addItem("BPU0", 0);
	combox->addItem("BPU1", 1);
	combox->addItem("BPU2", 2);
	combox->addItem("BPU3", 3);
	combox->addItem("BPU4", 4);
	combox->addItem("BPU5", 5);
	combox->addItem("BPU6", 6);
	combox->addItem("BPU7", 7);
	combox->addItem("MU", 8);

	QString ipHop = QString::fromStdString(ip) + ":" + QString::number(hop);
	int idx;
	if (!m_pUartComboboxIndexMap || m_pUartComboboxIndexMap->find(ipHop) == m_pUartComboboxIndexMap->end())
		idx = 8;
	else if (m_pUartComboboxIndexMap->find(ipHop).value() == -1)
		idx = 8;
	else
		idx = m_pUartComboboxIndexMap->find(ipHop).value();

	combox->setCurrentIndex(idx);
}

void AngKDebugSetting::InitLogLevelCombobox(QComboBox* combox, const std::string& ip, int hop)
{
	combox->addItem(tr("Debug"), 0);
	combox->addItem(tr("Normal"), 1);
	combox->addItem(tr("Warning"), 2);
	combox->addItem(tr("Error"), 3);

	QString ipHop = QString::fromStdString(ip) + ":" + QString::number(hop);
	int idx;
	if (!m_pLogLevelComboboxIndexMap || m_pLogLevelComboboxIndexMap->find(ipHop) == m_pLogLevelComboboxIndexMap->end())
		idx = 1;
	else if (m_pLogLevelComboboxIndexMap->find(ipHop).value() == -1)
		idx = 1;
	else
		idx = m_pLogLevelComboboxIndexMap->find(ipHop).value();

	combox->setCurrentIndex(idx);
}

void AngKDebugSetting::onTableSwitchUART(int uartData)
{
	QModelIndex curIndex = ui->debugProgramView->currentIndex();
	
	nlohmann::json debugSettingjson;
	debugSettingjson["ChangeUART"] = uartData;

	qDebug() << curIndex;
	QString ipHop = m_pDebugProgTableModel->data(m_pDebugProgTableModel->index(curIndex.row(), 0)).toString();
	if (m_pUartComboboxIndexMap && m_pUartComboboxIndexMap->find(ipHop) != m_pUartComboboxIndexMap->end())
		(*m_pUartComboboxIndexMap)[ipHop] = uartData;

	QStringList progIP_Hop = ipHop.split(":");

	AngKMessageHandler::instance().Command_RemoteDoPTCmd(progIP_Hop[0].toStdString(), progIP_Hop[1].toInt(), 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_DebugSetting, ALL_MU, 8, QByteArray(debugSettingjson.dump().c_str()));

}

void AngKDebugSetting::onTableSwitchLogLevel(int logData)
{
	QModelIndex curIndex = ui->debugProgramView->currentIndex();

	nlohmann::json debugSettingjson;
	debugSettingjson["LogLevelThreshold"] = logData;

	qDebug() << curIndex;
	QString ipHop = m_pDebugProgTableModel->data(m_pDebugProgTableModel->index(curIndex.row(), 0)).toString();
	if (m_pLogLevelComboboxIndexMap && m_pLogLevelComboboxIndexMap->find(ipHop) != m_pLogLevelComboboxIndexMap->end())
		(*m_pLogLevelComboboxIndexMap)[ipHop] = logData;

	QStringList progIP_Hop = ipHop.split(":");

	AngKMessageHandler::instance().Command_RemoteDoPTCmd(progIP_Hop[0].toStdString(), progIP_Hop[1].toInt(), 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_DebugSetting, ALL_MU, 8, QByteArray(debugSettingjson.dump().c_str()));
}

void AngKDebugSetting::onTableRebootBtnClicked()
{
	ACMessageBox::ACMsgType ret = ACMessageBox::showInformation(this, tr("Reboot"), tr("The device will reboot..."), ACMessageBox::ACMsgButton::MSG_OK_CANCEL_BTN);
	if (ret == ACMessageBox::ACMsgType::MSG_CANCEL)
	{
		return;
	}

	QModelIndex curIndex = ui->debugProgramView->currentIndex();

	nlohmann::json rebootMUjson;
	rebootMUjson["ResetCommand"] = 1;
	rebootMUjson["DelayTime"] = 1000;

	QStringList progIP_Hop = m_pDebugProgTableModel->data(m_pDebugProgTableModel->index(curIndex.row(), 0)).toString().split(":");

	AngKMessageHandler::instance().Command_RemoteDoPTCmd(progIP_Hop[0].toStdString(), progIP_Hop[1].toInt(), 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_RebootMU, ALL_MU, 8, QByteArray(rebootMUjson.dump().c_str()));

}