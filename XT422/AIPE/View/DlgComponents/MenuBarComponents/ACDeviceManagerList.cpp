#include "ACDeviceManagerList.h"
#include "ui_ACDeviceManagerList.h"
#include "StyleInit.h"
#include "ACDeviceManager.h"
#include "AngKMessageHandler.h"
#include "AngKCheckBoxHeader.h"

QString offlineIcon = ":/Skin/Light/Common/Device-offline.png";
QString onlineIcon = ":/Skin/Light/Common/Device-online.png";

ACDeviceManagerList::ACDeviceManagerList(QWidget *parent)
	: AngKDialog(parent)
	, ui(new Ui::ACDeviceManagerList())
	, m_pVarManager(std::make_unique<QtVariantPropertyManager>(ui->DeviceProperty))
	, m_pVarFactory(std::make_unique<QtVariantEditorFactory>(ui->DeviceProperty))
	, m_pStrManager(std::make_unique<QtStringPropertyManager>(ui->DeviceProperty))
	, m_pLineFactory(std::make_unique<QtLineEditFactory>(ui->DeviceProperty))
	, m_pRootItem(std::make_unique<QTreeWidgetItem>(ui->deviceTreeWidget))
{
	this->setObjectName("ACDeviceManagerList");
	QT_SET_STYLE_SHEET(objectName());

	//ui = new Ui::ACDeviceManagerList();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(700, 500);
	this->SetTitle(tr("Device Information"));

	connect(this, &ACDeviceManagerList::sgnClose, this, &ACDeviceManagerList::close);
	connect(ui->okButton, &QPushButton::clicked, this, &ACDeviceManagerList::close);
	connect(ui->modifyButton, &QPushButton::clicked, this, &ACDeviceManagerList::onSlotDeviceModify);
	connect(ui->editCheck, &QCheckBox::clicked, this, &ACDeviceManagerList::onSlotPropertyEdit);
	//TODO 接收设备管理的动态变化信号槽
	ui->modifyButton->setEnabled(false);

	InitDeviceTree();
	InitDeviceProperty();
}

ACDeviceManagerList::~ACDeviceManagerList()
{
	delete ui;
}

void ACDeviceManagerList::InitDeviceTree()
{
	ui->deviceTreeWidget->clear();
	ui->deviceTreeWidget->setColumnCount(1);
	ui->deviceTreeWidget->setHeaderLabels(QStringList(tr("DeviceChain")));

	//默认有一条链，创建根节点
	m_pRootItem->setText(0, tr("DeviceTree"));
	ui->deviceTreeWidget->addTopLevelItem(m_pRootItem.get());

	// 测试
	//QTreeWidgetItem* item = new QTreeWidgetItem(m_pRootItem.get());
	//item->setText(0, "192.168.1.1:0");
	//item->setIcon(0, QIcon(onlineIcon));
	//m_pRootItem->addChild(item);
	//QTreeWidgetItem* item1 = new QTreeWidgetItem(m_pRootItem.get());
	//item1->setText(0, "192.168.1.1:1");
	//item1->setIcon(0, QIcon(offlineIcon));
	//m_pRootItem->addChild(item1);

	// 调用设备管理
	QList<QString> devCHainNum = ACDeviceManager::instance().getChainIpList();
	for (int i = 0; i < devCHainNum.size(); ++i) {//创建设备树节点

		if (i == 0) {
			m_pRootItem->setText(0, devCHainNum[i]);
		}
		else {
			QTreeWidgetItem* devTreeSubNode = new QTreeWidgetItem(ui->deviceTreeWidget);
			devTreeSubNode->setText(0, devCHainNum[i]);
		}

		QMap<int, DeviceStu> devChain = ACDeviceManager::instance().getChainDevInfoByIp(devCHainNum[i]);

		for (auto chainInfo : devChain) {//创建设备树子节点
			QTreeWidgetItem* item = new QTreeWidgetItem(ui->deviceTreeWidget->topLevelItem(i));//设置父节点就可以直接添加为子节点
			QString strIPHop = QString::fromStdString(chainInfo.strIP) + ":" + QString::number(chainInfo.nHopNum);
			item->setText(0, strIPHop);
			chainInfo.bOnLine ? item->setIcon(0, QIcon(onlineIcon)) : item->setIcon(0, QIcon(offlineIcon));
		}
	}

	connect(ui->deviceTreeWidget, &QTreeWidget::itemClicked, this, &ACDeviceManagerList::onSlotShowDevProperty);
}

void ACDeviceManagerList::InitDeviceProperty()
{

	
	QTreeView* treeView = ui->DeviceProperty->findChild<QTreeView*>();
	if (treeView) {
		QAbstractItemModel* model = treeView->model();
		if (model) {
			model->setHeaderData(0, Qt::Horizontal, tr("Property")); // 设置第一列表头  
			model->setHeaderData(1, Qt::Horizontal, tr("Value")); // 设置第二列表头  
		}

	}


	ui->DeviceProperty->setHeaderVisible(true);
	ui->editCheck->setText(tr("DeviceInfo Edit Enable"));
	ui->DeviceProperty->clear();
	ui->DeviceProperty->setFactoryForManager(m_pVarManager.get(), m_pVarFactory.get());
	ui->DeviceProperty->setFactoryForManager(m_pStrManager.get(), m_pLineFactory.get());

	//设备基础信息
	QtProperty* item = m_pStrManager->addProperty(tr("Device IP"));
	m_pStrManager->setReadOnly(item, true);
	ui->DeviceProperty->addProperty(item);

	item = m_pStrManager->addProperty(tr("Device Port"));
	m_pStrManager->setReadOnly(item, true);
	ui->DeviceProperty->addProperty(item);

	item = m_pStrManager->addProperty(tr("HopNum"));
	m_pStrManager->setReadOnly(item, true);
	ui->DeviceProperty->addProperty(item);

	item = m_pStrManager->addProperty(tr("Device Mac"));
	m_pStrManager->setReadOnly(item, true);
	ui->DeviceProperty->addProperty(item);

	item = m_pStrManager->addProperty(tr("Site Alias"));
	m_pStrManager->setReadOnly(item, true);
	ui->DeviceProperty->addProperty(item);

	item = m_pStrManager->addProperty(tr("Firmware Version"));
	m_pStrManager->setReadOnly(item, true);
	ui->DeviceProperty->addProperty(item);

	item = m_pStrManager->addProperty(tr("Firmware Version Date"));
	m_pStrManager->setReadOnly(item, true);
	ui->DeviceProperty->addProperty(item);

	item = m_pStrManager->addProperty(tr("MUAPP Version"));
	m_pStrManager->setReadOnly(item, true);
	ui->DeviceProperty->addProperty(item);

	item = m_pStrManager->addProperty(tr("MUAPP Version Date"));
	m_pStrManager->setReadOnly(item, true);
	ui->DeviceProperty->addProperty(item);

	item = m_pStrManager->addProperty(tr("MUAPP Location"));
	m_pStrManager->setReadOnly(item, true);
	ui->DeviceProperty->addProperty(item);

	item = m_pStrManager->addProperty(tr("FPGA Version"));
	m_pStrManager->setReadOnly(item, true);
	ui->DeviceProperty->addProperty(item);

	item = m_pStrManager->addProperty(tr("FPGA Location"));
	m_pStrManager->setReadOnly(item, true);
	ui->DeviceProperty->addProperty(item);

	item = m_pStrManager->addProperty(tr("DPS Firmware Version"));
	m_pStrManager->setReadOnly(item, true);
	ui->DeviceProperty->addProperty(item);

	item = m_pStrManager->addProperty(tr("DPS FPGA Version"));
	m_pStrManager->setReadOnly(item, true);
	ui->DeviceProperty->addProperty(item);

	//设备主板信息
	QtProperty* mainBoardGroup = m_pVarManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("MainBoardInfo"));

	//设备主板添加子属性
	QtProperty* mainItem = m_pStrManager->addProperty(tr("HardwareUID"));
	m_pStrManager->setReadOnly(mainItem, true);
	mainBoardGroup->addSubProperty(mainItem);

	mainItem = m_pStrManager->addProperty(tr("HardwareSN"));
	m_pStrManager->setReadOnly(mainItem, true);
	mainBoardGroup->addSubProperty(mainItem);

	mainItem = m_pStrManager->addProperty(tr("HardwareVersion"));
	m_pStrManager->setReadOnly(mainItem, true);
	mainBoardGroup->addSubProperty(mainItem);

	mainItem = m_pStrManager->addProperty(tr("HardwareOEM"));
	m_pStrManager->setReadOnly(mainItem, true);
	mainBoardGroup->addSubProperty(mainItem);

	ui->DeviceProperty->addProperty(mainBoardGroup);
}

void ACDeviceManagerList::UpdateDeviceProperty(DeviceStu devInfo)
{
	QSet<QtProperty*> strPropertyList = m_pStrManager->properties();
	for (auto vat : strPropertyList)
	{
		if (vat->propertyName() == tr("Device IP")) {
			m_pStrManager->setValue(vat, QString::fromStdString(devInfo.strIP));
			vat->setToolTip(QString::fromStdString(devInfo.strIP));
		}
		else if (vat->propertyName() == tr("Device Port")) {
			m_pStrManager->setValue(vat, QString::fromStdString(devInfo.strPort));
			vat->setToolTip(QString::fromStdString(devInfo.strPort));
		}
		else if (vat->propertyName() == tr("HopNum")) {
			m_pStrManager->setValue(vat, QString::number(devInfo.nHopNum));
			vat->setToolTip(QString::number(devInfo.nHopNum));
		}
		else if (vat->propertyName() == tr("Device Mac")) {
			m_pStrManager->setValue(vat, QString::fromStdString(devInfo.strMac));
			vat->setToolTip(QString::fromStdString(devInfo.strMac));
		}
		else if (vat->propertyName() == tr("Site Alias")) {
			m_pStrManager->setValue(vat, QString::fromStdString(devInfo.strSiteAlias));
			vat->setToolTip(QString::fromStdString(devInfo.strSiteAlias));
		}
		else if (vat->propertyName() == tr("Firmware Version")) {
			m_pStrManager->setValue(vat, QString::fromStdString(devInfo.strFirmwareVersion));
			vat->setToolTip(QString::fromStdString(devInfo.strFirmwareVersion));
		}
		else if (vat->propertyName() == tr("Firmware Version Date")) {
			m_pStrManager->setValue(vat, QString::fromStdString(devInfo.strFirmwareVersionDate));
			vat->setToolTip(QString::fromStdString(devInfo.strFirmwareVersionDate));
		}
		else if (vat->propertyName() == tr("MUAPP Version")) {
			m_pStrManager->setValue(vat, QString::fromStdString(devInfo.strMUAPPVersion));
			vat->setToolTip(QString::fromStdString(devInfo.strMUAPPVersion));
		}
		else if (vat->propertyName() == tr("MUAPP Location")) {
			m_pStrManager->setValue(vat, QString::fromStdString(devInfo.strMULocation));
			vat->setToolTip(QString::fromStdString(devInfo.strMULocation));
		}
		else if (vat->propertyName() == tr("MUAPP Version Date")) {
			m_pStrManager->setValue(vat, QString::fromStdString(devInfo.strMUAPPVersionDate));
			vat->setToolTip(QString::fromStdString(devInfo.strMUAPPVersionDate));
		}
		else if (vat->propertyName() == tr("FPGA Version")) {
			m_pStrManager->setValue(vat, QString::fromStdString(devInfo.strFPGAVersion));
			vat->setToolTip(QString::fromStdString(devInfo.strFPGAVersion));
		}
		else if (vat->propertyName() == tr("FPGA Location")) {
			m_pStrManager->setValue(vat, QString::fromStdString(devInfo.strFPGALocation));
			vat->setToolTip(QString::fromStdString(devInfo.strFPGALocation));
		}
		else if (vat->propertyName() == tr("DPS Firmware Version")) {
			m_pStrManager->setValue(vat, QString::fromStdString(devInfo.strDPSFwVersion));
			vat->setToolTip(QString::fromStdString(devInfo.strDPSFwVersion));
		}
		else if (vat->propertyName() == tr("DPS FPGA Version")) {
			m_pStrManager->setValue(vat, QString::fromStdString(devInfo.strDPSFPGAVersion));
			vat->setToolTip(QString::fromStdString(devInfo.strDPSFPGAVersion));
		}
		else if (vat->propertyName() == tr("HardwareUID")) {
			m_pStrManager->setValue(vat, QString::fromStdString(devInfo.tMainBoardInfo.strHardwareUID));
			vat->setToolTip(QString::fromStdString(devInfo.tMainBoardInfo.strHardwareUID));
		}
		else if (vat->propertyName() == tr("HardwareSN")) {
			m_pStrManager->setValue(vat, QString::fromStdString(devInfo.tMainBoardInfo.strHardwareSN));
			vat->setToolTip(QString::fromStdString(devInfo.tMainBoardInfo.strHardwareSN));
		}
		else if (vat->propertyName() == tr("HardwareVersion")) {
			m_pStrManager->setValue(vat, QString::fromStdString(devInfo.tMainBoardInfo.strHardwareVersion));
			vat->setToolTip(QString::fromStdString(devInfo.tMainBoardInfo.strHardwareVersion));
		}
		else if (vat->propertyName() == tr("HardwareOEM")) {
			m_pStrManager->setValue(vat, QString::fromStdString(devInfo.tMainBoardInfo.strHardwareOEM));
			vat->setToolTip(QString::fromStdString(devInfo.tMainBoardInfo.strHardwareOEM));
		}
	}
}

void ACDeviceManagerList::onSlotDeviceModify()
{
	//调用网络管理模块处理
	QTreeWidgetItem* curTreeItem = ui->deviceTreeWidget->currentItem();
	if (curTreeItem == nullptr) {
		return;
	}
	QString strIPHop = curTreeItem->text(0);
	QStringList IPHopList = strIPHop.split(":");

	QSet<QtProperty*> strPropertyList = m_pStrManager->properties();
	std::string progIP;
	std::string progAlias;
	std::string progMac;
	int nHop = -1;
	for (auto vat : strPropertyList)
	{
		if (vat->propertyName() == tr("Device IP")) {
			progIP = m_pStrManager->value(vat).toStdString();
		}
		else if (vat->propertyName() == tr("Site Alias")) {
			progAlias = m_pStrManager->value(vat).toStdString();
		}
		else if (vat->propertyName() == tr("HopNum")) {
			nHop = m_pStrManager->value(vat).toInt();
		}
		else if (vat->propertyName() == tr("Device Mac")) {
			progMac = m_pStrManager->value(vat).toStdString();
		}
	}
	nlohmann::json progSettingJson;
	progSettingJson["ProgIP"] = progIP;
	progSettingJson["ProgAlias"] = progAlias;
	progSettingJson["ProgMac"] = progMac;

	if (IPHopList.size() > 1) {
		AngKMessageHandler::instance().Command_RemoteDoPTCmd(IPHopList[0].toStdString(), IPHopList[1].toInt(), 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_ProgramSetting, ALL_MU, 8, QByteArray(progSettingJson.dump().c_str()));
		//curTreeItem->setText(0,)
	}
}

void ACDeviceManagerList::onSlotShowDevProperty(QTreeWidgetItem* item, int nCol)
{
	if (item->parent() == nullptr)
	{
		QSet<QtProperty*> properties = m_pStrManager->properties();
		for (QtProperty* vat : properties) {
			m_pStrManager->setValue(vat, "");
		}
		return;
	}

	QString strIPHop = item->text(0);
	QStringList IPHopList = strIPHop.split(":");

	//调用DeviceManager 获取设备信息
	DeviceStu stu = ACDeviceManager::instance().getDevInfo(IPHopList[0], IPHopList[1].toInt());
	UpdateDeviceProperty(stu);

}

void ACDeviceManagerList::onSlotPropertyEdit(bool bCheck)
{
	//Demo模式直接返回
	//if (g_AppMode == ConnectType::Demo) {
	//	return;
	//}

	QSet<QtProperty*> strPropertyList = m_pStrManager->properties();
	for (auto vat : strPropertyList)
	{
		m_pStrManager->setReadOnly(vat, !bCheck);
	}

	ui->modifyButton->setEnabled(bCheck);
}