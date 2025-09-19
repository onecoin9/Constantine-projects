#include "AngKDeviceInfoTable.h"
#include "ui_AngKDeviceInfoTable.h"
#include "StyleInit.h"
#include "AngKDeviceModel.h"
#include <QHeaderView>
#include <QStandardItemModel>

AngKDeviceInfoTable::AngKDeviceInfoTable(QWidget *parent)
	: AngKDialog(parent)
	, m_DeviceTableModel(nullptr)
{
	this->setObjectName("AngKDeviceInfoTable");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::AngKDeviceInfoTable();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(700, 400);
	this->SetTitle(tr("Device Information"));

	connect(this, &AngKDeviceInfoTable::sgnClose, this, &AngKDeviceInfoTable::close);
	connect(ui->okButton, &QPushButton::clicked, this, &AngKDeviceInfoTable::close);

	InitDevTable();
}

AngKDeviceInfoTable::~AngKDeviceInfoTable()
{
	delete ui;
}

void AngKDeviceInfoTable::InitDevTable()
{
	m_DeviceTableModel = new QStandardItemModel(this);

	// 隐藏水平表头
	ui->DeviceView->verticalHeader()->setVisible(false);
	ui->DeviceView->setMouseTracking(true);
	ui->DeviceView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QStringList headList;
	headList << tr("ProgrammerName") << tr("SiteSN") << tr("SiteAlias") << tr("FirmwareVersion") << tr("HardwareVersion")
		<< tr("MUAPPVersion") << tr("FPGAVersion") << tr("FPGALocation");

	m_DeviceTableModel->setHorizontalHeaderLabels(headList);

	ui->DeviceView->setModel(m_DeviceTableModel);
	ui->DeviceView->setAlternatingRowColors(true);
	ui->DeviceView->horizontalHeader()->setHighlightSections(false);
	ui->DeviceView->horizontalHeader()->setStretchLastSection(true);
	ui->DeviceView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

	QHeaderView* manuHead = ui->DeviceView->horizontalHeader();

	manuHead->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui->DeviceView->setColumnWidth(FPGALocation, 120);

	InitData();
}

void AngKDeviceInfoTable::InitData()
{
	int increate = 0;
	std::map<std::string, DeviceStu> insertDev;
	AngKDeviceModel::instance().GetConnetDevMap(insertDev);
	for (auto iter : insertDev) {
		m_DeviceTableModel->insertRow(increate);

		QString devChain = QString::fromStdString(iter.second.strSiteAlias) + "[" + QString::number(iter.second.nChainID) + ":" + QString::number(iter.second.nHopNum) + "]";
		m_DeviceTableModel->setData(m_DeviceTableModel->index(increate, IPHop), devChain);
		m_DeviceTableModel->setData(m_DeviceTableModel->index(increate, DevSiteSN), QString::fromStdString(iter.second.tMainBoardInfo.strHardwareSN));
		m_DeviceTableModel->setData(m_DeviceTableModel->index(increate, DevSiteAlias), QString::fromStdString(iter.second.strSiteAlias));
		m_DeviceTableModel->setData(m_DeviceTableModel->index(increate, FirmwareVersion), QString::fromStdString(iter.second.strFirmwareVersion));
		m_DeviceTableModel->setData(m_DeviceTableModel->index(increate, HardwareVersion), QString::fromStdString(iter.second.tMainBoardInfo.strHardwareVersion));
		m_DeviceTableModel->setData(m_DeviceTableModel->index(increate, MUAPPVersion), QString::fromStdString(iter.second.strMUAPPVersion));
		m_DeviceTableModel->setData(m_DeviceTableModel->index(increate, FPGAVersion), QString::fromStdString(iter.second.strFPGAVersion));
		m_DeviceTableModel->setData(m_DeviceTableModel->index(increate, FPGALocation), QString::fromStdString(iter.second.strFPGALocation));
	}
}
