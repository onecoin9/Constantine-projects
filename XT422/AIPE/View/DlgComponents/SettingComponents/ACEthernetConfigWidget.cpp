#include "ACEthernetConfigWidget.h"
#include "ui_ACEthernetConfigWidget.h"
#include "../View/GlobalInit/StyleInit.h"
#include "GlobalDefine.h"
#include "AngKItemDelegate.h"
#include "AngKScanManager.h"
#include "AngKChainSelectDlg.h"
#include "AngKGlobalInstance.h"
#include "ACEventLogger.h"
#include "ACDeviceManager.h"
#include "ACMessageBox.h"
#include <QAbstractItemView>
#include <QHeaderView>
#include <QScrollBar>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QNetworkInterface>

Q_DECLARE_METATYPE(EthernetInfo)

ACEthernetConfigWidget::ACEthernetConfigWidget(QWidget *parent)
	: QWidget(parent)
	, m_netModel(nullptr)
	, m_connectedModel(nullptr)
	//, m_pScanManager(nullptr)
	, m_nDefaultNetLength(24)
{
	ui = new Ui::ACEthernetConfigWidget();
	ui->setupUi(this);
	InitScan();
	InitText();
	InitTable();
	InitButton();
	QT_SET_STYLE_SHEET(objectName());
}

ACEthernetConfigWidget::~ACEthernetConfigWidget()
{
	m_netModel->clear();
	m_connectedModel->clear();
	SAFEDEL(m_netModel);
	SAFEDEL(m_connectedModel);
	//SAFEDEL(m_pScanManager);
	delete ui;
}

void ACEthernetConfigWidget::InitText()
{
	ui->netManagerText->setText(tr("Network Manager"));
	ui->netDetailText->setText(tr("Detailed"));
	ui->netDetailText->hide();
	ui->scanButton->setText(tr("Scan"));
	ui->selectCheckBox->setText(tr("Select All"));
	ui->connectButton->setText(tr("Connect"));
	ui->connectText->setText(tr("Connected"));
	ui->conDetailText->setText(tr("Detailed"));
	ui->conDetailText->hide();
	ui->disSelectCheckBox->setText(tr("Select All"));
	ui->disConnectButton->setText(tr("Disconnect"));
	ui->okButton->setText(tr("Ok"));

	//设置默认IP
	QRegExpValidator regExpVal(QRegExp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b"));
	ui->ipEdit->setValidator(&regExpVal);
	// 用于占位
	ui->ipEdit->setInputMask("000.000.000.000");
	connect(ui->ipEdit, &QLineEdit::cursorPositionChanged, this, [=](int a, int b) {
		auto& scanManager = AngKScanManager::instance();
		scanManager.SetScanNetSegment(ui->ipEdit->text());
		});
	//ui->ipEdit->setText(AngKGlobalInstance::ReadValue("ScanIP", "IPSegment").toString());
	//设置默认子网掩码
	QRegExpValidator regExpValMask(QRegExp("^(0?[1-9]|[1-2][0-9]|31)$"));
	auto& scanManager = AngKScanManager::instance();
	scanManager.SetSubnetMask(m_nDefaultNetLength);
	ui->scanButton->setObjectName("ACEthernetConfigWidget_scanButton");
}

void ACEthernetConfigWidget::InitTable()
{
	//connect(ui->netWorkTableView, &AngKTableView::doubleClicked, this, &ACEthernetConfigWidget::onSlotSelectChainDev);

	m_netModel = new QStandardItemModel(this);
	m_connectedModel = new QStandardItemModel(this);

	// 隐藏水平表头
	ui->netWorkTableView->verticalHeader()->setVisible(false);
	ui->connectedTableView->verticalHeader()->setVisible(false);
	QStringList headList;
    headList << tr("IP Address") 
             << tr("HopNum") 
             << tr("SiteAlias") 
             << tr("SiteSN") 
             << tr("ChainID")
             << tr("Status");

	m_netModel->setHorizontalHeaderLabels(headList);
	m_connectedModel->setHorizontalHeaderLabels(headList);
	ui->netWorkTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->connectedTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui->netWorkTableView->setModel(m_netModel);
	ui->connectedTableView->setModel(m_connectedModel);
	ui->netWorkTableView->setAlternatingRowColors(true);
	ui->connectedTableView->setAlternatingRowColors(true);
	ui->netWorkTableView->horizontalHeader()->setHighlightSections(false);
	ui->connectedTableView->horizontalHeader()->setHighlightSections(false);
	ui->netWorkTableView->horizontalHeader()->setStretchLastSection(true);
	ui->connectedTableView->horizontalHeader()->setStretchLastSection(true);
	ui->netWorkTableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	ui->connectedTableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

	QHeaderView* netHead = ui->netWorkTableView->horizontalHeader();
	QHeaderView* conHead = ui->connectedTableView->horizontalHeader();

	// 列宽度自适应
	netHead->setSectionResizeMode(QHeaderView::Fixed);
	conHead->setSectionResizeMode(QHeaderView::Fixed);

	//网络表格固定，根据UI保持固定
    // 使用枚举值设置列宽
    ui->netWorkTableView->setColumnWidth(IPAddress, 140);  // IP Address
    ui->netWorkTableView->setColumnWidth(HopIndex, 75);    // HopNum
    ui->netWorkTableView->setColumnWidth(SiteAlias, 95);   // SiteAlias
    ui->netWorkTableView->setColumnWidth(SiteSN, 175);     // SiteSN
    ui->netWorkTableView->setColumnWidth(ChainID, 70);     // ChainID
    ui->netWorkTableView->setColumnWidth(Status, 70);      // Status

    // 为连接表设置相同的列宽
    ui->connectedTableView->setColumnWidth(IPAddress, 140);  // IP Address
    ui->connectedTableView->setColumnWidth(HopIndex, 75);    // HopNum
    ui->connectedTableView->setColumnWidth(SiteAlias, 95);   // SiteAlias
    ui->connectedTableView->setColumnWidth(SiteSN, 175);     // SiteSN
    ui->connectedTableView->setColumnWidth(ChainID, 70);     // ChainID
    ui->connectedTableView->setColumnWidth(Status, 70);      // Status

	//InsertData(m_netModel, "AU8fuoaue","Aprog25", "192.168.11.1", tr("Connected"), "0", "0", true);
}

void ACEthernetConfigWidget::InsertData(QStandardItemModel* model, QString sn, QString alias, QString strIP, QString strStatus, QString strHop, QString strChainID, bool testCheck)
{
	int row = model->rowCount();
	model->insertRow(model->rowCount());
	model->setData(model->index(row, IPAddress), strIP);
	model->item(row, IPAddress)->setCheckable(testCheck);

	model->setData(model->index(row, HopIndex), strHop);
	model->setData(model->index(row, SiteSN), sn);
	model->setData(model->index(row, SiteAlias), alias);
	model->setData(model->index(row, ChainID), strChainID);
	model->setData(model->index(row, Status), strStatus);
}

void ACEthernetConfigWidget::InitButton()
{
	connect(ui->scanButton, &QPushButton::clicked, this, &ACEthernetConfigWidget::onSlotScanLoaclNetWork);
	connect(ui->connectButton, &QPushButton::clicked, this, &ACEthernetConfigWidget::onSlotConnectNet);
	connect(ui->disConnectButton, &QPushButton::clicked, this, &ACEthernetConfigWidget::onSlotDisConnectNet);
	connect(ui->okButton, &QPushButton::clicked, this, &ACEthernetConfigWidget::sgnConnectFinish);

	connect(ui->selectCheckBox, &QCheckBox::stateChanged, this, &ACEthernetConfigWidget::onSlotNetWorkSelectAllCheck);
	connect(ui->disSelectCheckBox, &QCheckBox::stateChanged, this, &ACEthernetConfigWidget::onSlotConnectSelectAllCheck);
}

void ACEthernetConfigWidget::InitScan()
{
	// Read the default network prefix from the configuration file.
	// The configuration is expected to have a section "ScanIP" with key "IPSegment".
	QString defaultScanIPSegment = AngKGlobalInstance::ReadValue("ScanIP", "IPSegment").toString();
	if (defaultScanIPSegment.isEmpty()) {
		defaultScanIPSegment = "192.168.11.1";  // Fallback default value.
	}

	// **** 使用单例 ****
    // m_pScanManager = new AngKScanManager(); // 删除这行
    auto& scanManager = AngKScanManager::instance(); // 获取单例引用
	connect(&scanManager, &AngKScanManager::sgnUpdateDevIPScan, this, &ACEthernetConfigWidget::onSlotUpdateDevIPScan, Qt::QueuedConnection);
	connect(&scanManager, &AngKScanManager::sgnScanDeviceError, this, [=](QString recvDevIP, int nHop) {
		ACMessageBox::showError(this, tr("Scan Error"),
								tr("Device %1:%2 Link Error, Please Check Device Link Status.").arg(recvDevIP).arg(nHop));
	});
	connect(&scanManager, &AngKScanManager::sgnScanPortError, this, [=](int nPort) {
		ACMessageBox::showError(this, tr("Scan Error"),
								tr("The local port 8080 is occupied, Please check the port before reconnecting.").arg(nPort));
	});

	connect(ui->networkCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &ACEthernetConfigWidget::onSlotChangedIPScan);

	// Get all available network interfaces.
	QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
	QList<EthernetInfo> netInfos;
	EthernetInfo preferredNetInfo;
	bool foundPreferred = false;

	// Iterate over each network interface and collect IPv4 information.
	for (const QNetworkInterface &netInter : interfaces)
	{
		if (netInter.hardwareAddress().isEmpty())
			continue;

		EthernetInfo netInfo;
		netInfo.strMac = netInter.hardwareAddress();
		netInfo.strMacName = netInter.humanReadableName();

		// Check all IPv4 addresses for the interface.
		for (const QNetworkAddressEntry &entry : netInter.addressEntries())
		{
			if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
			{
				netInfo.strIPv4 = entry.ip().toString();
				netInfo.strNetMask = entry.netmask().toString();
				// Determine if the current interface is in the same network segment as the default IP.
				QStringList defaultParts = defaultScanIPSegment.split('.', QString::SkipEmptyParts);
				QStringList ipParts = netInfo.strIPv4.split('.', QString::SkipEmptyParts);
				if (defaultParts.size() >= 3 && ipParts.size() >= 3) {
					if (defaultParts[0] == ipParts[0] &&
						defaultParts[1] == ipParts[1] &&
						defaultParts[2] == ipParts[2]) {
						// It is in the same preferred subnet
						if (!foundPreferred)
						{
							preferredNetInfo = netInfo;
							foundPreferred = true;
						}
					}
				}
			}
		}
		netInfos.append(netInfo);
	}

	// Clear the combo box to repopulate.
	ui->networkCombo->clear();

	if (foundPreferred)
	{
		// Preferred NIC found: add it first and update ipEdit accordingly.
		ui->networkCombo->addItem(preferredNetInfo.strMacName, QVariant::fromValue(preferredNetInfo));
		ui->ipEdit->setText(preferredNetInfo.strIPv4);

		// Add all remaining interfaces excluding the preferred one.
		for (const EthernetInfo &netInfo : netInfos)
		{
			if (netInfo.strMac == preferredNetInfo.strMac && netInfo.strIPv4 == preferredNetInfo.strIPv4)
				continue;
			ui->networkCombo->addItem(netInfo.strMacName, QVariant::fromValue(netInfo));
		}
		ui->networkCombo->setCurrentIndex(0);
	}
	else
	{
		// No preferred NIC was found.
		QString lastSelectedIP = AngKGlobalInstance::ReadValue("ScanIP", "LastSelectedIP").toString();
		bool foundLastSelected = false;
		EthernetInfo lastSelectedInfo;

		// Try to locate the previously selected interface.
		for (const EthernetInfo &netInfo : netInfos)
		{
			if (netInfo.strIPv4 == lastSelectedIP)
			{
				lastSelectedInfo = netInfo;
				foundLastSelected = true;
				break;
			}
		}

		if (foundLastSelected)
		{
			ui->networkCombo->addItem(lastSelectedInfo.strMacName, QVariant::fromValue(lastSelectedInfo));
			ui->ipEdit->setText(lastSelectedInfo.strIPv4);
			for (const EthernetInfo &netInfo : netInfos)
			{
				if (netInfo.strMac == lastSelectedInfo.strMac && netInfo.strIPv4 == lastSelectedInfo.strIPv4)
					continue;
				ui->networkCombo->addItem(netInfo.strMacName, QVariant::fromValue(netInfo));
			}
			ui->networkCombo->setCurrentIndex(0);
		}
		else
		{
			// Neither a preferred NIC nor a last selected IP is available.
			for (const EthernetInfo &netInfo : netInfos)
			{
				ui->networkCombo->addItem(netInfo.strMacName, QVariant::fromValue(netInfo));
			}
			ui->networkCombo->setCurrentIndex(0);
			EthernetInfo currentInfo = ui->networkCombo->currentData().value<EthernetInfo>();
			ui->ipEdit->setText(currentInfo.strIPv4);
		}
	}
}

void ACEthernetConfigWidget::CloseSocket()
{
	// m_pScanManager->CloseSokcet(); // 改为调用单例
    AngKScanManager::instance().CloseSokcet();
}

void ACEthernetConfigWidget::CloseWidgetInfo()
{
	 // m_pScanManager->CloseSokcet(); // 改为调用单例
	 AngKScanManager::instance().CloseSokcet();
	 // SAFEDEL(m_pScanManager); // 不再需要
}

bool ACEthernetConfigWidget::CheckInsertORUpdate(QString strIP, int& row)
{
	bool bUpdate = false;

	for (int idx = 0; idx < m_netModel->rowCount(); ++idx) {
		if (strIP == m_netModel->data(m_netModel->index(idx, IPAddress)).toString()) {
			bUpdate = true;
			row = idx;
			break;
		}
	}

	return bUpdate;
}

int ACEthernetConfigWidget::CalNetMaskLength(QString strNetmask)
{
	QStringList parts = strNetmask.split('.'); // 将子网掩码按"."进行分割

	int subnetMaskLength = 0;
	foreach(QString part, parts) {
		uint num = part.toUInt(); // 将每部分转换为无符号整数
		while (num != 0) {
			++subnetMaskLength;
			num >>= 1; // 右移一位
		}
	}
	return subnetMaskLength;
}

void ACEthernetConfigWidget::FormatConnectEvent()
{
	std::map<std::string, DeviceStu> insertDev;
	AngKDeviceModel::instance().GetConnetDevMap(insertDev);
	nlohmann::json ProgConnectJson;
	ProgConnectJson["DeviceInfo"] = nlohmann::json::array();
	for (auto conDev : insertDev) {
		nlohmann::json devInfoJson;

		devInfoJson["ProgSN"] = conDev.second.tMainBoardInfo.strHardwareSN;
		devInfoJson["ProgType"] = conDev.second.tMainBoardInfo.strHardwareSN;
		devInfoJson["Alias"] = conDev.second.strSiteAlias;
		devInfoJson["FWVersion"] = conDev.second.strFirmwareVersion;
		devInfoJson["FWDate"] = conDev.second.strFirmwareVersionDate;

		ProgConnectJson["DeviceInfo"].push_back(devInfoJson);
	}

	EventLogger->SendEvent(EventBuilder->GetProgConnect(ProgConnectJson));
}

void ACEthernetConfigWidget::onSlotConnectNet()
{
	// m_pScanManager->blockSignals(true); // 考虑替代方案
    bool wasBlocked = AngKScanManager::instance().signalsBlocked(); // 记录原始状态
    AngKScanManager::instance().blockSignals(true); // 暂时阻塞
	AngKScanManager::instance().CloseSokcet();
	for (int i = m_netModel->rowCount() - 1; i >= 0; --i) {
		if (m_netModel->item(i, 0)->checkState() == Qt::CheckState::Checked) {
			InsertData(m_connectedModel, m_netModel->data(m_netModel->index(i, SiteSN)).toString(), m_netModel->data(m_netModel->index(i, SiteAlias)).toString()
				, m_netModel->data(m_netModel->index(i, IPAddress)).toString(), tr("Connected")
				, m_netModel->data(m_netModel->index(i, HopIndex)).toString(), m_netModel->data(m_netModel->index(i, ChainID)).toString(), true);
			m_netModel->removeRow(i);
		}
	}
	AngKScanManager::instance().blockSignals(wasBlocked); // 恢复原始状态
	// 2. 更新 AngKDeviceModel 的连接列表
    std::map<std::string, DeviceStu> insertDev;
    AngKDeviceModel::instance().GetScanDevMap(insertDev); // 获取扫描列表 Model 中的数据
    for (int idx = 0; idx < m_connectedModel->rowCount(); ++idx) { // 遍历 UI 连接列表中的项
        QString strIP = m_connectedModel->data(m_connectedModel->index(idx, IPAddress)).toString();
        int hop = m_connectedModel->data(m_connectedModel->index(idx, HopIndex)).toInt();
        std::string strIPHop = strIP.toStdString() + ":" + std::to_string(hop);

        // 检查设备是否存在于 Model 的扫描列表中
        if (AngKDeviceModel::instance().FindScanDev(strIPHop)) {
            AngKDeviceModel::instance().AddConnetDevIP(strIPHop); // 添加 Key 到连接列表 Model

            // **** 添加日志打印 ****
            const DeviceStu& devInfoToSet = insertDev[strIPHop]; // 获取要设置的 DeviceStu
            ALOG_INFO("Attempting SetConnetDevInfo for %s in onSlotConnectNet.", "CU", "--", strIPHop.c_str());
            ALOG_INFO(" -> Device Info from Scan Map: IP=%s, Hop=%d, MAC=%s, Alias=%s, FW=%s, recvPT=%s", "CU", "--",
                       devInfoToSet.strIP.c_str(), devInfoToSet.nHopNum, devInfoToSet.strMac.c_str(),
                       devInfoToSet.strSiteAlias.c_str(), devInfoToSet.strFirmwareVersion.c_str(),
                       devInfoToSet.recvPT ? "true" : "false");
            // **** 结束日志打印 ****

            // 将扫描列表 Model 中的 DeviceStu 复制到连接列表 Model
            bool setResult = AngKDeviceModel::instance().SetConnetDevInfo(strIPHop, devInfoToSet);
            if (!setResult) {
                 ALOG_WARN(" -> SetConnetDevInfo FAILED in onSlotConnectNet for %s.", "CU", "--", strIPHop.c_str());
            } else {
                 ALOG_DEBUG(" -> SetConnetDevInfo SUCCEEDED in onSlotConnectNet for %s.", "CU", "--", strIPHop.c_str());
            }
            //ACDeviceManager::instance().addDev(strIP, hop, insertDev[strIPHop]);
        } else {
             ALOG_WARN("Device %s selected in UI but not found in ScanDevMap during onSlotConnectNet.", "CU", "--", strIPHop.c_str());
        }
    }


	FormatConnectEvent();
}

void ACEthernetConfigWidget::onSlotDisConnectNet()
{
	for (int i = m_connectedModel->rowCount() - 1; i >= 0; --i) {
		if (m_connectedModel->item(i, 0)->checkState() == Qt::CheckState::Checked) {
			std::string strIP = m_connectedModel->data(m_connectedModel->index(i, IPAddress)).toString().toStdString();
			std::string strHopIndex = m_connectedModel->data(m_connectedModel->index(i, HopIndex)).toString().toStdString();
			std::string strIPHop = strIP + ":" + strHopIndex;
			m_connectedModel->removeRow(i);
			AngKDeviceModel::instance().ClearConnetDev(strIPHop);
			//ACDeviceManager::instance().delDev(QString::fromStdString(strIP), m_connectedModel->data(m_connectedModel->index(i, HopIndex)).toInt());
		}
	}
}

void ACEthernetConfigWidget::onSlotNetWorkSelectAllCheck(int state)
{
	switch (state)
	{
		case Qt::CheckState::Unchecked:
		{
			for (int i = 0; i < m_netModel->rowCount(); ++i)
			{
				m_netModel->item(i, 0)->setCheckState(Qt::CheckState::Unchecked);
			}
		}
		break;
		case Qt::CheckState::Checked:
		{
			for (int i = 0; i < m_netModel->rowCount(); ++i)
			{
				m_netModel->item(i, 0)->setCheckState(Qt::CheckState::Checked);
			}
		}
		break;
	default:
		break;
	}
}

void ACEthernetConfigWidget::onSlotConnectSelectAllCheck(int state)
{
	for (int i = 0; i < m_connectedModel->rowCount(); ++i)
	{
		m_connectedModel->item(i, 0)->setCheckState(Qt::CheckState(state));
	}
}

void ACEthernetConfigWidget::onSlotUpdateDevIPScan(QString strIPHop)
{
	std::map<std::string, DeviceStu> insertDev;
	AngKDeviceModel::instance().GetScanDevMap(insertDev);
	DeviceStu devStu;
	if(AngKDeviceModel::instance().FindScanDevByIPHop(strIPHop.toStdString(), devStu)){
	//插入数据表格
	/*for (auto iter : insertDev)*/ 
		int getRow = -1;
		if (CheckInsertORUpdate(QString::fromStdString(devStu.strIP), getRow)) {
			//m_netModel->setData(m_netModel->index(getRow, HopIndex), QString::number(iter.second.nHopNum));
			// TODO
			// 检查相同IP 说明是同一个链上的设备，不在进行更新列表。只显示头链设备
			// 双击列表中的某一项，可以直接打开需要连接的链上设备数量。
		}
		else {
			InsertData(m_netModel, QString::fromStdString(devStu.tMainBoardInfo.strHardwareSN), QString::fromStdString(devStu.strSiteAlias)
				, QString::fromStdString(devStu.strIP), tr("Idle"), QString::number(devStu.nHopNum), QString::number(devStu.nChainID), true);
		}
	}
}

void ACEthernetConfigWidget::onSlotSelectChainDev(const QModelIndex& chainDev)
{
	if (m_netModel->data(m_netModel->index(chainDev.row(), ChainID)).toInt() < 1)//说明链上没有设备
		return;

	std::map<std::string, DeviceStu> insertDev;
	AngKDeviceModel::instance().GetScanDevMap(insertDev);
	DeviceStu chainDevInfo;
	for (auto iter : insertDev) {
		if (iter.first == m_netModel->data(m_netModel->index(chainDev.row(), IPAddress)).toString().toStdString()) {
			chainDevInfo = iter.second;
		}
	}

	AngKChainSelectDlg chainDlg;
	chainDlg.SetChainInfo(chainDevInfo);
	connect(&chainDlg, &AngKChainSelectDlg::sgnSelectChain, this, [=](std::string strIP, std::vector<int> linkVec) {
		std::map<std::string, DeviceStu> insertDev;
		AngKDeviceModel::instance().GetScanDevMap(insertDev);
		insertDev[strIP].vecLinkDev = linkVec;
		AngKDeviceModel::instance().SetScanDevInfo(strIP, insertDev[strIP]);
		});
	chainDlg.exec();
}

void ACEthernetConfigWidget::onSlotChangedIPScan(int curIndex)
{
	EthernetInfo etherNetInfo = ui->networkCombo->itemData(curIndex).value<EthernetInfo>();

	ui->ipEdit->setText(etherNetInfo.strIPv4);
	AngKGlobalInstance::WriteValue("DeviceComm", "LocalIP", etherNetInfo.strIPv4);
	auto& scanManager = AngKScanManager::instance();
	scanManager.SetScanNetSegment(ui->ipEdit->text());

	scanManager.SetSubnetMask(CalNetMaskLength(etherNetInfo.strNetMask));
}

void ACEthernetConfigWidget::onSlotSendProgScanEvent()
{
	std::map<std::string, DeviceStu> insertDev;
	AngKDeviceModel::instance().GetScanDevMap(insertDev);
	nlohmann::json ProgScanJson;
	ProgScanJson["DeviceInfo"] = nlohmann::json::array();
	for (auto scanDev : insertDev) {
		nlohmann::json devInfoJson;

		devInfoJson["ProgSN"] = scanDev.second.tMainBoardInfo.strHardwareSN;
		devInfoJson["ProgType"] = scanDev.second.tMainBoardInfo.strHardwareSN;
		devInfoJson["Alias"] = scanDev.second.strSiteAlias;
		devInfoJson["FWVersion"] = scanDev.second.strFirmwareVersion;
		devInfoJson["FWDate"] = scanDev.second.strFirmwareVersionDate;

		ProgScanJson["DeviceInfo"].push_back(devInfoJson);
	}

	EventLogger->SendEvent(EventBuilder->GetProgScan(ProgScanJson));

	// Only if there is at least one device detected, automatically select all devices.
	if (!insertDev.empty()) {
		// Automatically select all devices in the network table.
		for (int i = 0; i < m_netModel->rowCount(); ++i) {
			m_netModel->item(i, 0)->setCheckState(Qt::Checked);
		}
		// Also mark the "Select All" checkbox as checked.
		ui->selectCheckBox->setChecked(true);
	}
}

void ACEthernetConfigWidget::onSlotScanLoaclNetWork()
{
	// 记录当前IP到LastSelectedIP
	AngKGlobalInstance::WriteValue("ScanIP", "LastSelectedIP", ui->ipEdit->text());
	auto& scanner = AngKScanManager::instance();
	scanner.blockSignals(false);
	AngKDeviceModel::instance().ClearScanDev();
	// 每次切换网卡时，清空扫描的设备列表
	if (m_netModel != nullptr) {
		scanner.CloseSokcet();
		m_netModel->removeRows(0, m_netModel->rowCount());
	}
	// m_connectedModel->removeRows(0, m_connectedModel->rowCount());

	if(scanner.SetScanComm())
		scanner.StartScan();

	QTimer::singleShot(2000, this, &ACEthernetConfigWidget::onSlotSendProgScanEvent);
}
