#include "AngKExtendCSD.h"
#include "ui_AngKExtendCSD.h"
#include "StyleInit.h"
#include <QToolTip>
#include <QStandardItemModel>
#include <QFileDialog>
#include "ACMessageBox.h"
#include "AngKCustomDelegate.h"
#include "AngKCommonTools.h"

int EXT_COMMON_BOOT_SIZE = 128;//单位kb

AngKExtendCSD::AngKExtendCSD(QWidget *parent)
	: QWidget(parent)
	, m_eCSDDataTableModel(nullptr)
{

	ui = new Ui::AngKExtendCSD();
	ui->setupUi(this);

	//this->setFixedSize(640, 750);
	//this->SetTitle(tr("Eextended CSD"));
	setAttribute(Qt::WA_TranslucentBackground, true);

	InitText();
	InitTable();
	InitButton();
	InitCombox();

	this->setObjectName("AngKExtendCSD");
	QT_SET_STYLE_SHEET(objectName());
}

AngKExtendCSD::~AngKExtendCSD()
{
	delete ui;
}

void AngKExtendCSD::InitText()
{
	ui->startHexText->setText(tr("Start Addr(Hex):"));
	ui->ENH_START_ADDR->setText(tr("ENH_START_ADDR(139-136):"));
	ui->sizeMBytesText->setText(tr("Size(MBytes):"));
	ui->ENH_SIZE_MULT->setText(tr("ENH_SIZE_MULT(142-140):"));

	ui->GPP1ModifyCheck->setText(tr("GPP1 Modify Enable"));
	ui->GPP2ModifyCheck->setText(tr("GPP2 Modify Enable"));
	ui->GPP3ModifyCheck->setText(tr("GPP3 Modify Enable"));
	ui->GPP4ModifyCheck->setText(tr("GPP4 Modify Enable"));

	ui->BootResizeCheck->setText(tr("Boot Size (KBytes):"));
	ui->RPMBResizeCheck->setText(tr("RPMB Size (kBytes):"));

	QRegExp hexRegexADDR("[0-9A-Fa-f]{1,8}");
	ui->ENH_START_ADDR_Edit->setValidator(new QRegExpValidator(hexRegexADDR, ui->ENH_START_ADDR_Edit));
	ui->ENH_START_ADDR_Edit->setMaxLength(8);

	QRegExp hexRegexMULT("[0-9A-Fa-f]{1,6}");
	ui->ENH_SIZE_MULT_Edit->setValidator(new QRegExpValidator(hexRegexMULT, ui->ENH_SIZE_MULT_Edit));
	ui->ENH_SIZE_MULT_Edit->setMaxLength(6);

	ui->widget_5->hide();
	ui->widget_6->hide();
}

void AngKExtendCSD::InitTable()
{
	m_eCSDDataTableModel = new QStandardItemModel(this);
	AngKCustomDelegate* customDelegate = new AngKCustomDelegate();
	customDelegate->setCheckBoxColumn(0);
	customDelegate->setEditColumn(2);
	customDelegate->setExtCSDFlag(true);

	// 隐藏水平表头
	ui->fileTableView->verticalHeader()->setVisible(false);
	ui->fileTableView->setMouseTracking(true);
	connect(ui->fileTableView, &AngKTableView::entered, this, [=](QModelIndex modelIdx) {
		if (!modelIdx.isValid()) {
			return;

		}
		QToolTip::showText(QCursor::pos(), modelIdx.data().toString());

		});

	QStringList headList;
	headList << tr("Modify") << tr("ByteAddr") << tr("Value(h)") << tr("Attibute") << tr("Description");

	m_eCSDDataTableModel->setHorizontalHeaderLabels(headList);
	m_eCSDDataTableModel->setProperty("ExtCSDData", "ExtCSDData");
	//ui->fileTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui->fileTableView->setItemDelegate(customDelegate);
	ui->fileTableView->setModel(m_eCSDDataTableModel);
	ui->fileTableView->setAlternatingRowColors(true);
	ui->fileTableView->horizontalHeader()->setHighlightSections(false);
	ui->fileTableView->horizontalHeader()->setStretchLastSection(true);
	ui->fileTableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	//ui->fileTableView->horizontalHeader()->setMinimumSectionSize(110);

	QHeaderView* manuHead = ui->fileTableView->horizontalHeader();

	manuHead->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui->fileTableView->setColumnWidth(0, 60);
	InsertFixedData();
}

void AngKExtendCSD::InitButton()
{
	//ui->okButton->setText("OK");
	//ui->cancelButton->setText("Cancel");
	//ui->readButton->setText("Read");
	//ui->saveExtcsdCfgButton->setText("Save ExtCfg");

	//connect(ui->okButton, &QPushButton::clicked, this, [=]() {
	//	//Todo 发送ExtendCSD 设置
	//	eMMCOPTION_Modify emcModify;
	//	GetExtendCSD2Send(emcModify);
	//	emit sgnEXTCSDConfig(emcModify);
	//	close();
	//	});

	ui->GPP1_ComboBox->setEnabled(false);
	ui->GPP2_ComboBox->setEnabled(false);
	ui->GPP3_ComboBox->setEnabled(false);
	ui->GPP4_ComboBox->setEnabled(false);
	ui->Boot_SpinBox->setEnabled(false);
	ui->RPMB_SpinBox->setEnabled(false);
	connect(ui->GPP1ModifyCheck, &QCheckBox::clicked, this, [=](bool bCheck) { 
		if (CheckGPPSetting()) { ui->GPP1_ComboBox->setEnabled(bCheck); }
		else { ui->GPP1ModifyCheck->setChecked(!bCheck); } });

	connect(ui->GPP2ModifyCheck, &QCheckBox::clicked, this, [=](bool bCheck) { 
		if (CheckGPPSetting()) { ui->GPP2_ComboBox->setEnabled(bCheck); }
		else { ui->GPP2ModifyCheck->setChecked(!bCheck); } });

	connect(ui->GPP3ModifyCheck, &QCheckBox::clicked, this, [=](bool bCheck) { 
		if (CheckGPPSetting()) { ui->GPP3_ComboBox->setEnabled(bCheck); }
		else { ui->GPP3ModifyCheck->setChecked(!bCheck); } });

	connect(ui->GPP4ModifyCheck, &QCheckBox::clicked, this, [=](bool bCheck) { 
		if (CheckGPPSetting()) { ui->GPP4_ComboBox->setEnabled(bCheck); }
		else { ui->GPP4ModifyCheck->setChecked(!bCheck); } });

	connect(ui->BootResizeCheck, &QCheckBox::clicked, this, [=](bool bCheck) { ui->Boot_SpinBox->setEnabled(bCheck); });
	connect(ui->RPMBResizeCheck, &QCheckBox::clicked, this, [=](bool bCheck) { ui->RPMB_SpinBox->setEnabled(bCheck); });

	connect(ui->Boot_SpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int nValue) {
		if (nValue % 128 == 0) {
			ui->Boot_SpinBox->blockSignals(true);
			ui->Boot_SpinBox->setValue(nValue);
			ui->Boot_SpinBox->blockSignals(false);
		}
		else {

			int nValueSize = nValue / 128;

			ui->Boot_SpinBox->blockSignals(true);
			if (nValue != 0)
				ui->Boot_SpinBox->setValue((nValueSize + 1) * 128);
			else
				ui->Boot_SpinBox->setValue(nValueSize * 128);
			ui->Boot_SpinBox->blockSignals(false);
		}
	});

	connect(ui->RPMB_SpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int nValue) {
		if (nValue % 128 == 0) {
			ui->RPMB_SpinBox->blockSignals(true);
			ui->RPMB_SpinBox->setValue(nValue);
			ui->RPMB_SpinBox->blockSignals(false);
		}
		else {

			int nValueSize = nValue / 128;

			ui->RPMB_SpinBox->blockSignals(true);
			if (nValue != 0)
				ui->RPMB_SpinBox->setValue((nValueSize + 1) * 128);
			else
				ui->RPMB_SpinBox->setValue(nValueSize * 128);
			ui->RPMB_SpinBox->blockSignals(false);
		}
		});
}

void AngKExtendCSD::InitCombox()
{
	ui->GPP1_ComboBox->setEditable(true);
	ui->GPP2_ComboBox->setEditable(true);
	ui->GPP3_ComboBox->setEditable(true);
	ui->GPP4_ComboBox->setEditable(true);
	ui->Boot_SpinBox->setValue(0);
	ui->RPMB_SpinBox->setValue(0);
	ui->sizeMBytesComboBox->setEditable(true);
	ui->startHexComboBox->setEditable(true);
}

void AngKExtendCSD::GetExtendCSD2Send(eMMCOPTION_Modify& emcModify)
{
	//eMMCOPTION_Modify emcModify;
	// EXTCSD判断是否修改
	for (int i = 0; i < m_eCSDDataTableModel->rowCount(); ++i)
	{
		if (m_eCSDDataTableModel->data(m_eCSDDataTableModel->index(i, 2)).toString() != "0")
		{
			SwitchModifyValue(emcModify.modify_extcsd, MyEnum(i), m_eCSDDataTableModel->data(m_eCSDDataTableModel->index(i, 2)).toString());
		}
	}

	// Enhanced User判断是否修改
	if (!ui->ENH_START_ADDR_Edit->text().isEmpty())
	{
		SwitchModifyValue(emcModify.modify_extcsd, MyEnum(ENH_START_ADDR), ui->ENH_START_ADDR_Edit->text());
	}
	if (!ui->ENH_SIZE_MULT_Edit->text().isEmpty())
	{
		SwitchModifyValue(emcModify.modify_extcsd, MyEnum(ENH_SIZE_MULT), ui->ENH_SIZE_MULT_Edit->text());
	}
	if (!ui->sizeMBytesComboBox->currentText().isEmpty())
	{
		emcModify.modify_part.EnhancedUserSize = ui->sizeMBytesComboBox->currentText().toInt();
	}

	// GPP Setting 判断是否修改
	if (!ui->GPP1_ComboBox->currentText().isEmpty())
	{
		emcModify.modify_part.GPP1Size = ui->GPP1_ComboBox->currentText().toInt();
	}
	if (!ui->GPP2_ComboBox->currentText().isEmpty())
	{
		emcModify.modify_part.GPP2Size = ui->GPP2_ComboBox->currentText().toInt();
	}
	if (!ui->GPP3_ComboBox->currentText().isEmpty())
	{
		emcModify.modify_part.GPP3Size = ui->GPP3_ComboBox->currentText().toInt();
	}
	if (!ui->GPP4_ComboBox->currentText().isEmpty())
	{
		emcModify.modify_part.GPP4Size = ui->GPP4_ComboBox->currentText().toInt();
	}
	// BOOT/RPMB 判断是否修改
	if (ui->Boot_SpinBox->value() != 0)
	{
		emcModify.modify_part.BootSize = ui->Boot_SpinBox->value();
	}
	if (ui->RPMB_SpinBox->value() != 0)
	{
		emcModify.modify_part.RPMBSize = ui->RPMB_SpinBox->value();
	}

}

void AngKExtendCSD::SetHuaweiExtCSD(eMMCOPTION_Modify hwExtCSD)
{
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(WR_REL_SET, 2), QString("%1").arg(hwExtCSD.modify_extcsd.wr_rel_set, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(FW_CONFIG, 2), QString("%1").arg(hwExtCSD.modify_extcsd.fw_config, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(BOOT_WP, 2), QString("%1").arg(hwExtCSD.modify_extcsd.boot_wp, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(PARTITION_CFG, 2), QString("%1").arg(hwExtCSD.modify_extcsd.partition_config, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(BOOT_CONFIG_PROT, 2), QString("%1").arg(hwExtCSD.modify_extcsd.boot_config_prot, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(BOOT_BUS_CONDITIONS, 2), QString("%1").arg(hwExtCSD.modify_extcsd.boot_bus_conditions, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(RST_n_FUNCTION, 2), QString("%1").arg(hwExtCSD.modify_extcsd.rst_n_function, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(BKOPS_EN, 2), QString("%1").arg(hwExtCSD.modify_extcsd.bkops_en, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(PARTITION_SETTING_COMPLETED, 2), QString("%1").arg(hwExtCSD.modify_extcsd.partition_setting_completed, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(PARTITIONS_ATTRIBUTE, 2), QString("%1").arg(hwExtCSD.modify_extcsd.partitions_attribute, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(ERASE_GROUP_DEF, 2), QString("%1").arg(hwExtCSD.modify_extcsd.erase_group_def, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(USER_WP, 2), QString("%1").arg(hwExtCSD.modify_extcsd.user_wp, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(SEC_BAD_BLK_MGMNT, 2), QString("%1").arg(hwExtCSD.modify_extcsd.sec_bad_blk_mgmnt, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(SECURE_REMOVAL_TYPE, 2), QString("%1").arg(hwExtCSD.modify_extcsd.secure_removal_type, 0, 16, QLatin1Char('0')));

	int nSize = sizeof(hwExtCSD.modify_extcsd.ext_partitions_attribute) / sizeof(unsigned char);
	QString dataString;
	for (int i = 0; i < nSize; ++i)
	{
		dataString += QString("%1").arg(hwExtCSD.modify_extcsd.ext_partitions_attribute[i], 2, 16, QLatin1Char('0'));
	}
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(EXT_PARTITIONS_ATTRIBUITE, 2), dataString);

	nSize = sizeof(hwExtCSD.modify_extcsd.enh_start_addr) / sizeof(unsigned char);
	dataString.clear();
	for (int i = 0; i < nSize; ++i)
	{
		dataString += QString("%1").arg(hwExtCSD.modify_extcsd.enh_start_addr[i], 2, 16, QLatin1Char('0'));
	}
	ui->ENH_START_ADDR_Edit->setText(dataString);
	ui->startHexComboBox->setEditText(dataString);

	nSize = sizeof(hwExtCSD.modify_extcsd.enh_size_mult) / sizeof(unsigned char);
	dataString.clear();
	for (int i = 0; i < nSize; ++i)
	{
		dataString += QString("%1").arg(hwExtCSD.modify_extcsd.enh_size_mult[i], 2, 16, QLatin1Char('0'));
	}

	ui->ENH_SIZE_MULT_Edit->setText(dataString);
	ui->sizeMBytesComboBox->setEditText(QString::number(hwExtCSD.modify_part.EnhancedUserSize));

	//设置分区大小
	ui->GPP1_ComboBox->setEditText(QString::number(hwExtCSD.modify_part.GPP1Size));
	ui->GPP2_ComboBox->setEditText(QString::number(hwExtCSD.modify_part.GPP2Size));
	ui->GPP3_ComboBox->setEditText(QString::number(hwExtCSD.modify_part.GPP3Size));
	ui->GPP4_ComboBox->setEditText(QString::number(hwExtCSD.modify_part.GPP4Size));
	ui->Boot_SpinBox->setValue(hwExtCSD.modify_part.BootSize);
	ui->RPMB_SpinBox->setValue(hwExtCSD.modify_part.RPMBSize);
}

int AngKExtendCSD::ParserExtCSDXml(QString sfilePath, bool bACXML)
{
	pugi::xml_document doc;
	const wchar_t* encodedName = reinterpret_cast<const wchar_t*>(sfilePath.utf16());
	pugi::xml_parse_result result = doc.load_file(encodedName);

	if (!result)
		return XMLMESSAGE_LOAD_FAILED;

	if (!bACXML) {
		pugi::xml_node extCSD_Node = doc.child(XML_NODE_EMMC_EXTCSD);

		for (pugi::xml_node node : extCSD_Node.children()) {
			if (strcmp(node.name(), XML_NODE_EMMC_PARTITIONSIZE) == 0) {
				QString strName = node.attribute("Name").as_string();
				QString strSize = node.attribute("Size").as_string();
				QString strUnit = node.attribute("Unit").as_string();
				GetPartitionSize(strName, strSize, strUnit, m_readExtCSD);
			}
			else if (strcmp(node.name(), XML_NODE_EMMC_Reg) == 0) {
				QString strAddr = node.attribute("Addr").as_string();
				QString strValue = node.attribute("Value").as_string();
				QString strName = node.attribute("Name").as_string();
				GetExtCSDReg(strAddr, strValue, strName, m_readExtCSD);
			}
		}
	}
	else {
		pugi::xml_node root_node = doc.child(XML_ROOTNODE_EMMC);
		pugi::xml_node extCSD_Node = root_node.child(XML_NODE_EMMC_EXTCSD);

		for (pugi::xml_node node : extCSD_Node.children()) {
			if (strcmp(node.name(), XML_NODE_EMMC_PARTITIONSIZE) == 0) {
				QString strName = node.attribute("Name").as_string();
				QString strSize = node.attribute("Size").as_string();
				QString strUnit = node.attribute("Unit").as_string();
				GetPartitionSize(strName, strSize, strUnit, m_readExtCSD);
			}
			else if (strcmp(node.name(), XML_NODE_EMMC_Reg) == 0) {
				QString strAddr = node.attribute("Addr").as_string();
				QString strValue = node.attribute("Value").as_string();
				QString strName = node.attribute("Name").as_string();
				GetExtCSDReg(strAddr, strValue, strName, m_readExtCSD);
			}
		}
	}

	SetHuaweiExtCSD(m_readExtCSD);//改变寄存器中的值

	return 0;
}

void AngKExtendCSD::GetPartitionSize(QString partName, QString partSize, QString UnitSize, eMMCOPTION_Modify& csdInfo)
{
	int nPartSize = partSize.toInt();
	//if (UnitSize != "MBytes") {// 如果是KB，那么一定是1024 kb * x的整数倍，出错由底层返回
	//	nPartSize %= 1024;

	//	if (nPartSize == 0) {
	//		nPartSize = 1;
	//	}
	//}

	if (partName == "GPP1") {
		csdInfo.modify_part.GPP1Size = nPartSize;
		ui->GPP1ModifyCheck->setChecked(true);
		ui->GPP1_ComboBox->setEnabled(true);
	}
	else if (partName == "GPP2") {
		csdInfo.modify_part.GPP2Size = nPartSize;
		ui->GPP2ModifyCheck->setChecked(true);
		ui->GPP2_ComboBox->setEnabled(true);
	}
	else if (partName == "GPP3") {
		csdInfo.modify_part.GPP3Size = nPartSize;
		ui->GPP3ModifyCheck->setChecked(true);
		ui->GPP3_ComboBox->setEnabled(true);
	}
	else if (partName == "GPP4") {
		csdInfo.modify_part.GPP4Size = nPartSize;
		ui->GPP4ModifyCheck->setChecked(true);
		ui->GPP4_ComboBox->setEnabled(true);
	}
	else if (partName == "BOOT") {
		if (UnitSize == "KBytes") {
			csdInfo.modify_part.BootSize = nPartSize;
		}
		else {
			int kbSize = partSize.toInt();
			kbSize *= 1024;	//转为KB
			int nNum = 0;
			if (kbSize % EXT_COMMON_BOOT_SIZE == 0)
				nNum = kbSize / EXT_COMMON_BOOT_SIZE;
			else
				nNum = (kbSize / EXT_COMMON_BOOT_SIZE) + 1;

			csdInfo.modify_part.BootSize = nNum * EXT_COMMON_BOOT_SIZE;
		}

		ui->BootResizeCheck->setChecked(true);
		ui->Boot_SpinBox->setEnabled(true);
	}
	else if (partName == "RPMB") {
		if (UnitSize == "KBytes") {
			csdInfo.modify_part.RPMBSize = nPartSize;
		}
		else {
			int kbSize = partSize.toInt();
			kbSize *= 1024;	//转为KB
			int nNum = 0;
			if (kbSize % EXT_COMMON_BOOT_SIZE == 0)
				nNum = kbSize / EXT_COMMON_BOOT_SIZE;
			else
				nNum = (kbSize / EXT_COMMON_BOOT_SIZE) + 1;

			csdInfo.modify_part.RPMBSize = nNum * EXT_COMMON_BOOT_SIZE;
		}
		ui->RPMBResizeCheck->setChecked(true);
		ui->RPMB_SpinBox->setEnabled(true);
	}
	//else if (partName == "ENHANCED") {
	//	csdInfo.modify_part.EnhancedUserSize = nPartSize;
	//	ui->startaddrGroup->setChecked(true);
	//}
}

void AngKExtendCSD::GetExtCSDReg(QString regAddr, QString regValue, QString regName, eMMCOPTION_Modify& csdInfo)
{
	bool bOk = false;
	if (regAddr == "167") {//WR_REL_SET
		csdInfo.modify_extcsd.wr_rel_set = regValue.toInt(&bOk, 16);
		m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(WR_REL_SET, 0), true, Qt::UserRole);
	}
	else if (regAddr == "169") {//FW_CONFIG
		csdInfo.modify_extcsd.fw_config = regValue.toInt(&bOk, 16);
		m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(FW_CONFIG, 0), true, Qt::UserRole);
	}
	else if (regAddr == "173") {//BOOT_WP
		csdInfo.modify_extcsd.boot_wp = regValue.toInt(&bOk, 16);
		m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(BOOT_WP, 0), true, Qt::UserRole);
	}
	else if (regAddr == "179") {//PARTITION_CFG
		csdInfo.modify_extcsd.partition_config = regValue.toInt(&bOk, 16);
		m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(PARTITION_CFG, 0), true, Qt::UserRole);
	}
	else if (regAddr == "178") {//BOOT_CONFIG_PROT
		csdInfo.modify_extcsd.boot_config_prot = regValue.toInt(&bOk, 16);
		m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(BOOT_CONFIG_PROT, 0), true, Qt::UserRole);
	}
	else if (regAddr == "177") {//BOOT_BUS_CONDITIONS
		csdInfo.modify_extcsd.boot_bus_conditions = regValue.toInt(&bOk, 16);
		m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(BOOT_BUS_CONDITIONS, 0), true, Qt::UserRole);
	}
	else if (regAddr == "162") {//RST_n_FUNCTION
		csdInfo.modify_extcsd.rst_n_function = regValue.toInt(&bOk, 16);
		m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(RST_n_FUNCTION, 0), true, Qt::UserRole);
	}
	else if (regAddr == "163") {//BKOPS_EN
		csdInfo.modify_extcsd.bkops_en = regValue.toInt(&bOk, 16);
		m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(BKOPS_EN, 0), true, Qt::UserRole);
	}
	else if (regAddr == "155") {//PARTITION_SETTING_COMPLETED
		csdInfo.modify_extcsd.partition_setting_completed = regValue.toInt(&bOk, 16);
		m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(PARTITION_SETTING_COMPLETED, 0), true, Qt::UserRole);
	}
	else if (regAddr == "156") {//PARTITIONS_ATTRIBUTE
		csdInfo.modify_extcsd.partitions_attribute = regValue.toInt(&bOk, 16);
		m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(PARTITIONS_ATTRIBUTE, 0), true, Qt::UserRole);
	}
	else if (regAddr == "175") {//ERASE_GROUP_DEF
		csdInfo.modify_extcsd.erase_group_def = regValue.toInt(&bOk, 16);
		m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(ERASE_GROUP_DEF, 0), true, Qt::UserRole);
	}
	else if (regAddr == "171") {//USER_WP
		csdInfo.modify_extcsd.user_wp = regValue.toInt(&bOk, 16);
		m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(USER_WP, 0), true, Qt::UserRole);
	}
	else if (regAddr == "53:52" || regAddr == "52") {//EXT_PARTITIONS_ATTRIBUITE
		for (int i = 0; i < regValue.size() / 2; i++) {
			bool bOK;
			QString twoDigits = regValue.mid(i * 2, 2); // 获取两位数字的子字符串
			csdInfo.modify_extcsd.ext_partitions_attribute[i] = twoDigits.toUInt(&bOK, 16); // 将子字符串转换为整数并存储
		}
		m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(EXT_PARTITIONS_ATTRIBUITE, 0), true, Qt::UserRole);
	}
	else if (regAddr == "134") {//SEC_BAD_BLK_MGMNT
		csdInfo.modify_extcsd.sec_bad_blk_mgmnt = regValue.toInt(&bOk, 16);
		m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(SEC_BAD_BLK_MGMNT, 0), true, Qt::UserRole);
	}
	else if (regAddr == "16") {//SECURE_REMOVAL_TYPE
		csdInfo.modify_extcsd.secure_removal_type = regValue.toInt(&bOk, 16);
		m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(SECURE_REMOVAL_TYPE, 0), true, Qt::UserRole);
	}
	else if (regAddr == "139:136" || regAddr == "136") {//ENH_START_ADDR
		for (int i = 0; i < regValue.size() / 2; ++i) {
			bool bOK;
			QString twoDigits = regValue.mid(i * 2, 2); // 获取两位数字的子字符串
			csdInfo.modify_extcsd.enh_start_addr[i] = twoDigits.toUInt(&bOK, 16); // 将子字符串转换为整数并存储
		}
		ui->modifyGroup->setChecked(true);
	}
	else if (regAddr == "142:140" || regAddr == "140") {//ENH_SIZE_MULT
		for (int i = 0; i < regValue.size() / 2; ++i) {
			bool bOK;
			QString twoDigits = regValue.mid(i * 2, 2); // 获取两位数字的子字符串
			csdInfo.modify_extcsd.enh_size_mult[i] = twoDigits.toUInt(&bOK, 16); // 将子字符串转换为整数并存储
		}
		ui->startaddrGroup->setChecked(true);
	}
}

void AngKExtendCSD::SwitchRegConfigJson(UI_CFG_EXTCSD cfgEXTCSD, nlohmann::json& regCfgJson)
{
	size_t arrayLength = 0;
	for (int i = 0; i < m_eCSDDataTableModel->rowCount(); ++i) {
		nlohmann::json partCfgJson;
		if (m_eCSDDataTableModel->data(m_eCSDDataTableModel->index(i, 0), Qt::UserRole).toBool()) {
			QString spAddr = m_eCSDDataTableModel->data(m_eCSDDataTableModel->index(i, 1)).toString();
			if (spAddr == "53:52") {
				arrayLength = sizeof(cfgEXTCSD.ext_partitions_attribute) / sizeof(cfgEXTCSD.ext_partitions_attribute[0]);
				QString ext_partitions_attributeSTR = QString::fromUtf8(reinterpret_cast<const char*>(cfgEXTCSD.ext_partitions_attribute), static_cast<int>(arrayLength));
				//if (!isAllZeros(ext_partitions_attributeSTR))
				{
					// 将数组中的字节顺序反转
					INT8U reversed_addr[2] = { cfgEXTCSD.ext_partitions_attribute[1], cfgEXTCSD.ext_partitions_attribute[0] };

					// 将反转后的数据转换为字符串
					QString reversed_str = QString("%1%2").arg(cfgEXTCSD.ext_partitions_attribute[0], 2, 16, QChar('0'))
						.arg(cfgEXTCSD.ext_partitions_attribute[1], 2, 16, QChar('0'));

					nlohmann::json partCfgJson;
					partCfgJson["Addr"] = 52;
					partCfgJson["Value"] = reversed_str.toStdString();
					partCfgJson["Name"] = "EXT_PARTITIONS_ATTRIBUTE";
					regCfgJson.push_back(partCfgJson);
				}
				continue;
			}
			int nAddr = m_eCSDDataTableModel->data(m_eCSDDataTableModel->index(i, 1)).toInt();
			partCfgJson["Addr"] = nAddr;
			partCfgJson["Value"] = GetEXTCSDValue(cfgEXTCSD, nAddr);
			partCfgJson["Name"] = m_eCSDDataTableModel->data(m_eCSDDataTableModel->index(i, 4)).toString().toStdString();
			regCfgJson.push_back(partCfgJson);
		}
	}

	if (ui->modifyGroup->isChecked()) {
		arrayLength = sizeof(cfgEXTCSD.enh_start_addr) / sizeof(cfgEXTCSD.enh_start_addr[0]);
		QString enh_start_addrSTR = QString::fromUtf8(reinterpret_cast<const char*>(cfgEXTCSD.enh_start_addr), static_cast<int>(arrayLength));
		//if (!isAllZeros(enh_start_addrSTR))
		{
			// 将数组中的字节顺序反转
			INT8U reversed_addr[4] = { cfgEXTCSD.enh_start_addr[3], cfgEXTCSD.enh_start_addr[2], cfgEXTCSD.enh_start_addr[1], cfgEXTCSD.enh_start_addr[0] };

			// 将反转后的数据转换为字符串
			QString reversed_str = QString("%1%2%3%4").arg(cfgEXTCSD.enh_start_addr[0], 2, 16, QChar('0'))
				.arg(cfgEXTCSD.enh_start_addr[1], 2, 16, QChar('0'))
				.arg(cfgEXTCSD.enh_start_addr[2], 2, 16, QChar('0'))
				.arg(cfgEXTCSD.enh_start_addr[3], 2, 16, QChar('0'));

			nlohmann::json partCfgJson;
			partCfgJson["Addr"] = 136;
			partCfgJson["Value"] = reversed_str.toStdString();
			partCfgJson["Name"] = "ENH_START_ADDR";
			regCfgJson.push_back(partCfgJson);
		}
	}

	if (ui->startaddrGroup->isChecked()) {
		arrayLength = sizeof(cfgEXTCSD.enh_size_mult) / sizeof(cfgEXTCSD.enh_size_mult[0]);
		QString enh_size_multSTR = QString::fromUtf8(reinterpret_cast<const char*>(cfgEXTCSD.enh_size_mult), static_cast<int>(arrayLength));
		//if (!isAllZeros(enh_size_multSTR))
		{
			// 将数组中的字节顺序反转
			INT8U reversed_addr[3] = { cfgEXTCSD.enh_size_mult[2], cfgEXTCSD.enh_size_mult[1], cfgEXTCSD.enh_size_mult[0] };

			// 将反转后的数据转换为字符串
			QString reversed_str = QString("%1%2%3").arg(cfgEXTCSD.enh_size_mult[0], 2, 16, QChar('0'))
				.arg(cfgEXTCSD.enh_size_mult[1], 2, 16, QChar('0'))
				.arg(cfgEXTCSD.enh_size_mult[2], 2, 16, QChar('0'));

			nlohmann::json partCfgJson;
			partCfgJson["Addr"] = 140;
			partCfgJson["Value"] = reversed_str.toStdString();
			partCfgJson["Name"] = "ENH_SIZE_MULT";
			regCfgJson.push_back(partCfgJson);
		}
	}
}

void AngKExtendCSD::SwitchPartitionSizeJson(PartitionSizeModify cfgEXTCSD, nlohmann::json& partSizeJson)
{
	//if (ui->startaddrGroup->isChecked() && !ui->sizeMBytesComboBox->currentText().isEmpty())
	//{
	//	nlohmann::json partJson;
	//	partJson["Name"] = "ENHANCED";
	//	partJson["Size"] = cfgEXTCSD.EnhancedUserSize;
	//	partJson["Unit"] = "MBytes";
	//	partSizeJson.push_back(partJson);
	//}
	if (ui->GPP1ModifyCheck->isChecked())
	{
		nlohmann::json partJson;
		partJson["Name"] = "GPP1";
		partJson["Size"] = cfgEXTCSD.GPP1Size;
		partJson["Unit"] = "MBytes";
		partSizeJson.push_back(partJson);
	}
	if (ui->GPP2ModifyCheck->isChecked())
	{
		nlohmann::json partJson;
		partJson["Name"] = "GPP2";
		partJson["Size"] = cfgEXTCSD.GPP2Size;
		partJson["Unit"] = "MBytes";
		partSizeJson.push_back(partJson);
	}
	if (ui->GPP3ModifyCheck->isChecked())
	{
		nlohmann::json partJson;
		partJson["Name"] = "GPP3";
		partJson["Size"] = cfgEXTCSD.GPP3Size;
		partJson["Unit"] = "MBytes";
		partSizeJson.push_back(partJson);
	}
	if (ui->GPP4ModifyCheck->isChecked())
	{
		nlohmann::json partJson;
		partJson["Name"] = "GPP4";
		partJson["Size"] = cfgEXTCSD.GPP4Size;
		partJson["Unit"] = "MBytes";
		partSizeJson.push_back(partJson);
	}
	if (ui->BootResizeCheck->isChecked())
	{
		nlohmann::json partJson;
		partJson["Name"] = "BOOT";
		partJson["Size"] = cfgEXTCSD.BootSize;
		partJson["Unit"] = "KBytes";
		partSizeJson.push_back(partJson);
	}
	if (ui->RPMBResizeCheck->isChecked())
	{
		nlohmann::json partJson;
		partJson["Name"] = "RPMB";
		partJson["Size"] = cfgEXTCSD.RPMBSize;
		partJson["Unit"] = "KBytes";
		partSizeJson.push_back(partJson);
	}
}

bool AngKExtendCSD::isAllZeros(const QString& str)
{
	for (int i = 0; i < str.length(); ++i) {
		if (str[i] != '\0') {
			return false; // 如果存在非零字符则返回false
		}
	}
	return true; // 若没有非零字符则返回true
}

std::string AngKExtendCSD::GetExtendCSDJson()
{
	GetExtendCSD2Send(m_readExtCSD);
	//EXTCSD 部分
	nlohmann::json regConfigJson = nlohmann::json::array();
	nlohmann::json partitonSizeJson = nlohmann::json::array();
	nlohmann::json EXTCSDJson;
	SwitchRegConfigJson(m_readExtCSD.modify_extcsd, regConfigJson);
	SwitchPartitionSizeJson(m_readExtCSD.modify_part, partitonSizeJson);

	EXTCSDJson["RegConfig"] = regConfigJson;
	EXTCSDJson["PartitionSize"] = partitonSizeJson;

	return EXTCSDJson.dump();
}

void AngKExtendCSD::SeteMMCExtCSDPara(std::string& extJson)
{
	if (extJson.empty())
		return;

	//加载eMMCExtCSD的信息
	std::string extCSDJson = extJson;
	try {
		auto extJson = nlohmann::json::parse(extCSDJson);
		nlohmann::json regConfigJson = extJson["RegConfig"];
		nlohmann::json partitonSizeJson = extJson["PartitionSize"];

		for (int i = 0; i < regConfigJson.size(); ++i)
		{
			int nAddr = regConfigJson[i]["Addr"];
			std::string strName = regConfigJson[i]["Name"];
			std::string strValue = regConfigJson[i]["Value"];
			GetExtCSDReg(QString::number(nAddr), QString::fromStdString(strValue), QString::fromStdString(strName), m_readExtCSD);
		}

		for (int j = 0; j < partitonSizeJson.size(); ++j)
		{
			uint32_t nSize = partitonSizeJson[j]["Size"];
			std::string strName = partitonSizeJson[j]["Name"];
			std::string strUnit = partitonSizeJson[j]["Unit"];
			GetPartitionSize(QString::fromStdString(strName), QString::number(nSize), QString::fromStdString(strUnit), m_readExtCSD);
		}
	}
	catch (const nlohmann::json::exception& e) {
		ALOG_FATAL("eMMCExtCSDPara Json parse failed : %s.", "CU", "--", e.what());
	}

	SetHuaweiExtCSD(m_readExtCSD);
}

std::string AngKExtendCSD::GetEXTCSDValue(UI_CFG_EXTCSD& _cfgEXTCSD, int nAddr)
{
	std::string extcsdValue;

	if (nAddr == 179) {
		extcsdValue = QString("%1").arg(_cfgEXTCSD.partition_config, 2, 16, QLatin1Char('0')).toStdString();
	}
	else if (nAddr == 178) {
		extcsdValue = QString("%1").arg(_cfgEXTCSD.boot_config_prot, 2, 16, QLatin1Char('0')).toStdString();
	}
	else if (nAddr == 177) {
		extcsdValue = QString("%1").arg(_cfgEXTCSD.boot_bus_conditions, 2, 16, QLatin1Char('0')).toStdString();
	}
	else if (nAddr == 175) {
		extcsdValue = QString("%1").arg(_cfgEXTCSD.erase_group_def, 2, 16, QLatin1Char('0')).toStdString();
	}
	else if (nAddr == 173) {
		extcsdValue = QString("%1").arg(_cfgEXTCSD.boot_wp, 2, 16, QLatin1Char('0')).toStdString();
	}
	else if (nAddr == 171) {
		extcsdValue = QString("%1").arg(_cfgEXTCSD.user_wp, 2, 16, QLatin1Char('0')).toStdString();
	}
	else if (nAddr == 169) {
		extcsdValue = QString("%1").arg(_cfgEXTCSD.fw_config, 2, 16, QLatin1Char('0')).toStdString();
	}
	else if (nAddr == 167) {
		extcsdValue = QString("%1").arg(_cfgEXTCSD.wr_rel_set, 2, 16, QLatin1Char('0')).toStdString();
	}
	else if (nAddr == 163) {
		extcsdValue = QString("%1").arg(_cfgEXTCSD.bkops_en, 2, 16, QLatin1Char('0')).toStdString();
	}
	else if (nAddr == 162) {
		extcsdValue = QString("%1").arg(_cfgEXTCSD.rst_n_function, 2, 16, QLatin1Char('0')).toStdString();
	}
	else if (nAddr == 156) {
		extcsdValue = QString("%1").arg(_cfgEXTCSD.partitions_attribute, 2, 16, QLatin1Char('0')).toStdString();
	}
	else if (nAddr == 155) {
		extcsdValue = QString("%1").arg(_cfgEXTCSD.partition_setting_completed, 2, 16, QLatin1Char('0')).toStdString();
	}
	else if (nAddr == 134) {
		extcsdValue = QString("%1").arg(_cfgEXTCSD.sec_bad_blk_mgmnt, 2, 16, QLatin1Char('0')).toStdString();
	}
	else if (nAddr == 16) {
		extcsdValue = QString("%1").arg(_cfgEXTCSD.secure_removal_type, 2, 16, QLatin1Char('0')).toStdString();
	}

	return extcsdValue;
}

bool AngKExtendCSD::CheckGPPSetting()
{
	for (int i = 0; i < m_eCSDDataTableModel->rowCount(); ++i) {
		QString spAddr = m_eCSDDataTableModel->data(m_eCSDDataTableModel->index(i, 1)).toString();
		if (spAddr == "155") {
			if (m_eCSDDataTableModel->data(m_eCSDDataTableModel->index(i, 2)).toInt() != 0) {
				return true;
			}
			else {
				ACMessageBox::showWarning(this, tr("GPP Setting Warn"), tr("Setting GPP partition requires setting 155Bit first."));
				return false;
			}
		}
	}
	return false;
}

void AngKExtendCSD::InsertFixedData()
{
	m_eCSDDataTableModel->insertRows(m_eCSDDataTableModel->rowCount(), 15);
	int count = m_eCSDDataTableModel->rowCount();
	//固定第三列初始化为0
	for (int i = 0; i < count; ++i)
	{
		m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(i, 2), 0);
	}

	//ByteAddr
	std::vector<std::string> byteAddrVec = { "167","169","173","179","178","177","162","163","155","156","175","171","53:52" ,"134" ,"16" };
	for (int i = 0; i < count; ++i)
	{
		m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(i, 1), QString::fromStdString(byteAddrVec[i]));
	}

	//Attribute
	std::vector<std::string> AttributeVec = { "R/W(OTP)","R/W(OTP)","R/W(OTP)&R/W/C_P","R/W/E/E_P","R/W(OTP)","R/W/E","R/W(OTP)","R/W(OTP)","R/W(OTP)","R/W(OTP)","R/W/E_P","R/W(OTP)&R/W/C_P","R/W(OTP)" ,"R/W(OTP)" ,"R/W(OTP)" };
	for (int i = 0; i < count; ++i)
	{
		m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(i, 3), QString::fromStdString(AttributeVec[i]));
	}

	//Description
	std::vector<std::string> DescriptionVec = { "WR_REL_SET","FW_CONFIG","BOOT_WP","PARTITION_CFG","BOOT_CONFIG_PROT","BOOT_BUS_CONDITIONS","RST_n_FUNCTION","BKOPS_EN",
		"PARTITION_SETTING_COMPLETED","PARTITIONS_ATTRIBUTE","ERASE_GROUP_DEF","USER_WP","EXT_PARTITIONS_ATTRIBUITE" ,"SEC_BAD_BLK_MGMNT" ,"SECURE_REMOVAL_TYPE" };
	for (int i = 0; i < count; ++i)
	{
		m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(i, 4), QString::fromStdString(DescriptionVec[i]));
	}
}

void AngKExtendCSD::SwitchModifyValue(UI_CFG_EXTCSD& uiExtCSD, MyEnum csdType, QString valueStr)
{
	QByteArray u8Str;
	bool bOk = false;
	switch (csdType)
	{
	case WR_REL_SET://167
		uiExtCSD.wr_rel_set = valueStr.toInt(&bOk, 16);
		break;
	case FW_CONFIG://169
		uiExtCSD.fw_config = valueStr.toInt(&bOk, 16);
		break;
	case BOOT_WP://173
		uiExtCSD.boot_wp = valueStr.toInt(&bOk, 16);
		break;
	case PARTITION_CFG://179
		uiExtCSD.partition_config = valueStr.toInt(&bOk, 16);
		break;
	case BOOT_CONFIG_PROT://178
		uiExtCSD.boot_config_prot = valueStr.toInt(&bOk, 16);
		break;
	case BOOT_BUS_CONDITIONS://177
		uiExtCSD.boot_bus_conditions = valueStr.toInt(&bOk, 16);
		break;
	case RST_n_FUNCTION://162
		uiExtCSD.rst_n_function = valueStr.toInt(&bOk, 16);
		break;
	case BKOPS_EN://163
		uiExtCSD.bkops_en = valueStr.toInt(&bOk, 16);
		break;
	case PARTITION_SETTING_COMPLETED://155
		uiExtCSD.partition_setting_completed = valueStr.toInt(&bOk, 16);
		break;
	case PARTITIONS_ATTRIBUTE://156
		uiExtCSD.partitions_attribute = valueStr.toInt(&bOk, 16);
		break;
	case ERASE_GROUP_DEF://175
		uiExtCSD.erase_group_def = valueStr.toInt(&bOk, 16);
		break;
	case USER_WP://171
		uiExtCSD.user_wp = valueStr.toInt(&bOk, 16);
		break;
	case EXT_PARTITIONS_ATTRIBUITE://53:52
	{
		memset(uiExtCSD.ext_partitions_attribute, 0, sizeof(uiExtCSD.ext_partitions_attribute));
		for (int i = 0; i < valueStr.size() / 2; ++i) {
			QString twoDigits = valueStr.mid(i * 2, 2); // 获取两位数字的子字符串
			uiExtCSD.ext_partitions_attribute[i] = twoDigits.toUInt(&bOk, 16); // 将子字符串转换为整数并存储
		}
	}
		break;
	case SEC_BAD_BLK_MGMNT://134
		uiExtCSD.sec_bad_blk_mgmnt = valueStr.toInt(&bOk, 16);
		break;
	case SECURE_REMOVAL_TYPE://16
		uiExtCSD.secure_removal_type = valueStr.toInt(&bOk, 16);
		break;
	case ENH_START_ADDR://139:136
		//先清空在重新赋值
		memset(uiExtCSD.enh_start_addr, 0, sizeof(uiExtCSD.enh_start_addr));
		valueStr = valueStr.rightJustified(8, '0');
		for (int i = 0; i < valueStr.size() / 2; ++i) { 
			QString twoDigits = valueStr.mid(i * 2, 2); // 获取两位数字的子字符串
			uiExtCSD.enh_start_addr[i] = twoDigits.toUInt(&bOk, 16); // 将子字符串转换为整数并存储
		}
		break;
	case ENH_SIZE_MULT://142:140
		memset(uiExtCSD.enh_size_mult, 0, sizeof(uiExtCSD.enh_size_mult));
		valueStr = valueStr.rightJustified(6, '0');
		for (int i = 0; i < valueStr.size() / 2; ++i) {
			QString twoDigits = valueStr.mid(i * 2, 2); // 获取两位数字的子字符串
			uiExtCSD.enh_size_mult[i] = twoDigits.toUInt(&bOk, 16); // 将子字符串转换为整数并存储
		}
		break;
	default:
		break;
	}
}

void AngKExtendCSD::onSlotDealEXTCSDConfig(eMMCOPTION_Modify emmcModify)
{
	SetHuaweiExtCSD(emmcModify);
}

void AngKExtendCSD::onSlotImportExtCSDConfig()
{
	QString filePath = QFileDialog::getOpenFileName(this, "Select File...", QCoreApplication::applicationDirPath(), tr("ExtCSD Config File(*.esdxml);; All Files(*.*)"));

	ParserExtCSDXml(filePath);
}

void AngKExtendCSD::onSlotParserACXML(QString acFilePath)
{
	ParserExtCSDXml(acFilePath, true);
}

void AngKExtendCSD::onSlotRestExtCSDInfo()
{
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(WR_REL_SET, 2), QString("%1").arg(0, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(WR_REL_SET, 0), false, Qt::UserRole);
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(FW_CONFIG, 2), QString("%1").arg(0, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(FW_CONFIG, 0), false, Qt::UserRole);
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(BOOT_WP, 2), QString("%1").arg(0, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(BOOT_WP, 0), false, Qt::UserRole);
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(PARTITION_CFG, 2), QString("%1").arg(0, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(PARTITION_CFG, 0), false, Qt::UserRole);
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(BOOT_CONFIG_PROT, 2), QString("%1").arg(0, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(BOOT_CONFIG_PROT, 0), false, Qt::UserRole);
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(BOOT_BUS_CONDITIONS, 2), QString("%1").arg(0, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(BOOT_BUS_CONDITIONS, 0), false, Qt::UserRole);
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(RST_n_FUNCTION, 2), QString("%1").arg(0, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(RST_n_FUNCTION, 0), false, Qt::UserRole);
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(BKOPS_EN, 2), QString("%1").arg(0, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(BKOPS_EN, 0), false, Qt::UserRole);
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(PARTITION_SETTING_COMPLETED, 2), QString("%1").arg(0, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(PARTITION_SETTING_COMPLETED, 0), false, Qt::UserRole);
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(PARTITIONS_ATTRIBUTE, 2), QString("%1").arg(0, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(PARTITIONS_ATTRIBUTE, 0), false, Qt::UserRole);
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(ERASE_GROUP_DEF, 2), QString("%1").arg(0, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(ERASE_GROUP_DEF, 0), false, Qt::UserRole);
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(USER_WP, 2), QString("%1").arg(0, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(USER_WP, 0), false, Qt::UserRole);
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(SEC_BAD_BLK_MGMNT, 2), QString("%1").arg(0, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(SEC_BAD_BLK_MGMNT, 0), false, Qt::UserRole);
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(SECURE_REMOVAL_TYPE, 2), QString("%1").arg(0, 0, 16, QLatin1Char('0')));
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(SECURE_REMOVAL_TYPE, 0), false, Qt::UserRole);
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(EXT_PARTITIONS_ATTRIBUITE, 2), 0);
	m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(EXT_PARTITIONS_ATTRIBUITE, 0), false, Qt::UserRole);

	ui->modifyGroup->setChecked(false);
	ui->ENH_START_ADDR_Edit->clear();
	ui->startHexComboBox->clearEditText();

	ui->startaddrGroup->setChecked(false);
	ui->ENH_SIZE_MULT_Edit->clear();
	ui->sizeMBytesComboBox->clearEditText();

	//设置分区大小
	ui->GPP1ModifyCheck->setChecked(false);
	ui->GPP2ModifyCheck->setChecked(false);
	ui->GPP3ModifyCheck->setChecked(false);
	ui->GPP4ModifyCheck->setChecked(false);
	ui->BootResizeCheck->setChecked(false);
	ui->RPMBResizeCheck->setChecked(false);
	ui->GPP1_ComboBox->clearEditText();
	ui->GPP2_ComboBox->clearEditText();
	ui->GPP3_ComboBox->clearEditText();
	ui->GPP4_ComboBox->clearEditText();
	ui->Boot_SpinBox->clear();
	ui->RPMB_SpinBox->clear();
}

void AngKExtendCSD::onSlotSaveExtendCSDConfig()
{
	QString cfgPath = QFileDialog::getSaveFileName(this, "Select File...", QCoreApplication::applicationDirPath(), tr("ExtCSD Config File(*.esdxml);; All Files(*.*)"));
	std::string unitMB = "MBytes";
	std::string unitKB = "KBytes";
	// 创建 XML 文档对象
	pugi::xml_document doc;
	pugi::xml_node rootExt = doc.append_child("ExtCSD");

	eMMCOPTION_Modify emcModify;
	GetExtendCSD2Send(emcModify);
	
	//PartitionSizeModify
	if (emcModify.modify_part.BootSize != 0) {
		pugi::xml_node partChild = rootExt.append_child("PartitionSize");
		partChild.append_attribute("Name") = "Boot";
		partChild.append_attribute("Size") = emcModify.modify_part.BootSize;
		partChild.append_attribute("Unit") = unitKB.c_str();
	}
	if (emcModify.modify_part.RPMBSize != 0) {
		pugi::xml_node partChild = rootExt.append_child("PartitionSize");
		partChild.append_attribute("Name") = "RPMB";
		partChild.append_attribute("Size") = emcModify.modify_part.RPMBSize;
		partChild.append_attribute("Unit") = unitKB.c_str();
	}
	//if (emcModify.modify_part.EnhancedUserSize != 0) {
	//	pugi::xml_node partChild = rootExt.append_child("PartitionSize");
	//	partChild.append_attribute("Name") = "ENHANCED";
	//	partChild.append_attribute("Size") = emcModify.modify_part.EnhancedUserSize;
	//	partChild.append_attribute("Unit") = unitMB.c_str();
	//}
	if (emcModify.modify_part.GPP1Size != 0) {
		pugi::xml_node partChild = rootExt.append_child("PartitionSize");
		partChild.append_attribute("Name") = "GPP1";
		partChild.append_attribute("Size") = emcModify.modify_part.GPP1Size;
		partChild.append_attribute("Unit") = unitMB.c_str();
	}
	if (emcModify.modify_part.GPP2Size != 0) {
		pugi::xml_node partChild = rootExt.append_child("PartitionSize");
		partChild.append_attribute("Name") = "GPP2";
		partChild.append_attribute("Size") = emcModify.modify_part.GPP2Size;
		partChild.append_attribute("Unit") = unitMB.c_str();
	}
	if (emcModify.modify_part.GPP3Size != 0) {
		pugi::xml_node partChild = rootExt.append_child("PartitionSize");
		partChild.append_attribute("Name") = "GPP3";
		partChild.append_attribute("Size") = emcModify.modify_part.GPP3Size;
		partChild.append_attribute("Unit") = unitMB.c_str();
	}
	if (emcModify.modify_part.GPP4Size != 0) {
		pugi::xml_node partChild = rootExt.append_child("PartitionSize");
		partChild.append_attribute("Name") = "GPP4";
		partChild.append_attribute("Size") = emcModify.modify_part.GPP4Size;
		partChild.append_attribute("Unit") = unitMB.c_str();
	}

	//UI_Config_EXTCSD
	if (emcModify.modify_extcsd.partition_config != 0) {
		pugi::xml_node regChild = rootExt.append_child("Reg");
		regChild.append_attribute("Addr") = "179";
		regChild.append_attribute("Value") = QString("%1").arg(emcModify.modify_extcsd.partition_config, 2, 16, QLatin1Char('0')).toStdString().c_str();
		regChild.append_attribute("Name") = "PARTITION_CONFIG";
	}
	if (emcModify.modify_extcsd.boot_config_prot != 0) {
		pugi::xml_node regChild = rootExt.append_child("Reg");
		regChild.append_attribute("Addr") = "178";
		regChild.append_attribute("Value") = QString("%1").arg(emcModify.modify_extcsd.boot_config_prot, 2, 16, QLatin1Char('0')).toStdString().c_str();
		regChild.append_attribute("Name") = "BOOT_CONFIG_PROT";
	}
	if (emcModify.modify_extcsd.boot_bus_conditions != 0) {
		pugi::xml_node regChild = rootExt.append_child("Reg");
		regChild.append_attribute("Addr") = "177";
		regChild.append_attribute("Value") = QString("%1").arg(emcModify.modify_extcsd.boot_bus_conditions, 2, 16, QLatin1Char('0')).toStdString().c_str();
		regChild.append_attribute("Name") = "BOOT_BUS_CONDITIONS";
	}
	if (emcModify.modify_extcsd.erase_group_def != 0) {
		pugi::xml_node regChild = rootExt.append_child("Reg");
		regChild.append_attribute("Addr") = "175";
		regChild.append_attribute("Value") = QString("%1").arg(emcModify.modify_extcsd.erase_group_def, 2, 16, QLatin1Char('0')).toStdString().c_str();
		regChild.append_attribute("Name") = "ERASE_GROUP_DEF";
	}
	if (emcModify.modify_extcsd.boot_wp != 0) {
		pugi::xml_node regChild = rootExt.append_child("Reg");
		regChild.append_attribute("Addr") = "173";
		regChild.append_attribute("Value") = QString("%1").arg(emcModify.modify_extcsd.boot_wp, 2, 16, QLatin1Char('0')).toStdString().c_str();
		regChild.append_attribute("Name") = "BOOT_WP";
	}
	if (emcModify.modify_extcsd.user_wp != 0) {
		pugi::xml_node regChild = rootExt.append_child("Reg");
		regChild.append_attribute("Addr") = "171";
		regChild.append_attribute("Value") = QString("%1").arg(emcModify.modify_extcsd.user_wp, 2, 16, QLatin1Char('0')).toStdString().c_str();
		regChild.append_attribute("Name") = "USER_WP";
	}
	if (emcModify.modify_extcsd.fw_config != 0) {
		pugi::xml_node regChild = rootExt.append_child("Reg");
		regChild.append_attribute("Addr") = "169";
		regChild.append_attribute("Value") = QString("%1").arg(emcModify.modify_extcsd.fw_config, 2, 16, QLatin1Char('0')).toStdString().c_str();
		regChild.append_attribute("Name") = "FW_CONFIG";
	}
	if (emcModify.modify_extcsd.wr_rel_set != 0) {
		pugi::xml_node regChild = rootExt.append_child("Reg");
		regChild.append_attribute("Addr") = "167";
		regChild.append_attribute("Value") = QString("%1").arg(emcModify.modify_extcsd.wr_rel_set, 2, 16, QLatin1Char('0')).toStdString().c_str();
		regChild.append_attribute("Name") = "WR_REL_SET";
	}
	if (emcModify.modify_extcsd.bkops_en != 0) {
		pugi::xml_node regChild = rootExt.append_child("Reg");
		regChild.append_attribute("Addr") = "163";
		regChild.append_attribute("Value") = QString("%1").arg(emcModify.modify_extcsd.bkops_en, 2, 16, QLatin1Char('0')).toStdString().c_str();
		regChild.append_attribute("Name") = "BKOPS_EN";
	}
	if (emcModify.modify_extcsd.rst_n_function != 0) {
		pugi::xml_node regChild = rootExt.append_child("Reg");
		regChild.append_attribute("Addr") = "162";
		regChild.append_attribute("Value") = QString("%1").arg(emcModify.modify_extcsd.rst_n_function, 2, 16, QLatin1Char('0')).toStdString().c_str();
		regChild.append_attribute("Name") = "RST_n_FUNCTION";
	}
	if (emcModify.modify_extcsd.partitions_attribute != 0) {
		pugi::xml_node regChild = rootExt.append_child("Reg");
		regChild.append_attribute("Addr") = "156";
		regChild.append_attribute("Value") = QString("%1").arg(emcModify.modify_extcsd.partitions_attribute, 2, 16, QLatin1Char('0')).toStdString().c_str();
		regChild.append_attribute("Name") = "PARTITIONS_ATTRIBUTE";
	}
	if (emcModify.modify_extcsd.partition_setting_completed != 0) {
		pugi::xml_node regChild = rootExt.append_child("Reg");
		regChild.append_attribute("Addr") = "155";
		regChild.append_attribute("Value") = QString("%1").arg(emcModify.modify_extcsd.partition_setting_completed, 2, 16, QLatin1Char('0')).toStdString().c_str();
		regChild.append_attribute("Name") = "PARTITION_SETTING_COMPLETED";
	}
	if (emcModify.modify_extcsd.sec_bad_blk_mgmnt != 0) {
		pugi::xml_node regChild = rootExt.append_child("Reg");
		regChild.append_attribute("Addr") = "134";
		regChild.append_attribute("Value") = QString("%1").arg(emcModify.modify_extcsd.sec_bad_blk_mgmnt, 2, 16, QLatin1Char('0')).toStdString().c_str();
		regChild.append_attribute("Name") = "SEC_BAD_BLK_MGMNT";
	}
	if (emcModify.modify_extcsd.secure_removal_type != 0) {
		pugi::xml_node regChild = rootExt.append_child("Reg");
		regChild.append_attribute("Addr") = "16";
		regChild.append_attribute("Value") = QString("%1").arg(emcModify.modify_extcsd.secure_removal_type, 2, 16, QLatin1Char('0')).toStdString().c_str();
		regChild.append_attribute("Name") = "SEC_REMOVAL_TYPE";
	}
	if (emcModify.modify_extcsd.enh_size_mult != 0) {
		pugi::xml_node regChild = rootExt.append_child("Reg");
		regChild.append_attribute("Addr") = "142:140";
		int nSize = sizeof(emcModify.modify_extcsd.enh_size_mult) / sizeof(unsigned char);
		QString dataString;
		for (int i = 0; i < nSize; ++i)
		{
			dataString += QString("%1").arg(emcModify.modify_extcsd.enh_size_mult[i], 2, 16, QLatin1Char('0'));
		}
		regChild.append_attribute("Value") = dataString.toStdString().c_str();
		regChild.append_attribute("Name") = "ENH_SIZE_MULT";
	}
	if (emcModify.modify_extcsd.enh_start_addr != 0) {
		pugi::xml_node regChild = rootExt.append_child("Reg");
		regChild.append_attribute("Addr") = "139:136";
		int nSize = sizeof(emcModify.modify_extcsd.enh_start_addr) / sizeof(unsigned char);
		QString dataString;
		for (int i = 0; i < nSize; ++i)
		{
			dataString += QString("%1").arg(emcModify.modify_extcsd.enh_start_addr[i], 2, 16, QLatin1Char('0'));
		}
		regChild.append_attribute("Value") = dataString.toStdString().c_str();
		regChild.append_attribute("Name") = "ENH_START_ADDR";
	}
	if (emcModify.modify_extcsd.ext_partitions_attribute != 0) {
		pugi::xml_node regChild = rootExt.append_child("Reg");
		regChild.append_attribute("Addr") = "53:52";
		int nSize = sizeof(emcModify.modify_extcsd.ext_partitions_attribute) / sizeof(unsigned char);
		QString dataString;
		for (int i = 0; i < nSize; ++i)
		{
			dataString += QString("%1").arg(emcModify.modify_extcsd.ext_partitions_attribute[i], 2, 16, QLatin1Char('0'));
		}
		regChild.append_attribute("Value") = dataString.toStdString().c_str();
		regChild.append_attribute("Name") = "EXT_PARTITIONS_ATTRIBUTE";
	}

	// 保存到文件
	doc.save_file(reinterpret_cast<const wchar_t*>(cfgPath.utf16()));
}

void AngKExtendCSD::onSlotExtCSDInfoFetched(QByteArray readExtCSDByte) {
	bool bOk = false;
	uchar* extCSDInfo = new uchar[512];

	// 初始化所有位为 0
	memset(extCSDInfo, 0, 512);
	memcpy(extCSDInfo, readExtCSDByte.constData(), 512);

	for (int i = 0; i < 512; i++) {
		QString regAddr = i;
		QString regValue = extCSDInfo[i];
		if (regAddr == "167") {//WR_REL_SET
			//csdInfo.modify_extcsd.wr_rel_set = regValue.toInt(&bOk, 16);
			m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(WR_REL_SET, 0), true, Qt::UserRole);
		}
		else if (regAddr == "169") {//FW_CONFIG
			//csdInfo.modify_extcsd.fw_config = regValue.toInt(&bOk, 16);
			m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(FW_CONFIG, 0), true, Qt::UserRole);
		}
		else if (regAddr == "173") {//BOOT_WP
			//csdInfo.modify_extcsd.boot_wp = regValue.toInt(&bOk, 16);
			m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(BOOT_WP, 0), true, Qt::UserRole);
		}
		else if (regAddr == "179") {//PARTITION_CFG
			//csdInfo.modify_extcsd.partition_config = regValue.toInt(&bOk, 16);
			m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(PARTITION_CFG, 0), true, Qt::UserRole);
		}
		else if (regAddr == "178") {//BOOT_CONFIG_PROT
			//csdInfo.modify_extcsd.boot_config_prot = regValue.toInt(&bOk, 16);
			m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(BOOT_CONFIG_PROT, 0), true, Qt::UserRole);
		}
		else if (regAddr == "177") {//BOOT_BUS_CONDITIONS
			//csdInfo.modify_extcsd.boot_bus_conditions = regValue.toInt(&bOk, 16);
			m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(BOOT_BUS_CONDITIONS, 0), true, Qt::UserRole);
		}
		else if (regAddr == "162") {//RST_n_FUNCTION
			//csdInfo.modify_extcsd.rst_n_function = regValue.toInt(&bOk, 16);
			m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(RST_n_FUNCTION, 0), true, Qt::UserRole);
		}
		else if (regAddr == "163") {//BKOPS_EN
			//csdInfo.modify_extcsd.bkops_en = regValue.toInt(&bOk, 16);
			m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(BKOPS_EN, 0), true, Qt::UserRole);
		}
		else if (regAddr == "155") {//PARTITION_SETTING_COMPLETED
			//csdInfo.modify_extcsd.partition_setting_completed = regValue.toInt(&bOk, 16);
			m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(PARTITION_SETTING_COMPLETED, 0), true, Qt::UserRole);
		}
		else if (regAddr == "156") {//PARTITIONS_ATTRIBUTE
			//csdInfo.modify_extcsd.partitions_attribute = regValue.toInt(&bOk, 16);
			m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(PARTITIONS_ATTRIBUTE, 0), true, Qt::UserRole);
		}
		else if (regAddr == "175") {//ERASE_GROUP_DEF
			//csdInfo.modify_extcsd.erase_group_def = regValue.toInt(&bOk, 16);
			m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(ERASE_GROUP_DEF, 0), true, Qt::UserRole);
		}
		else if (regAddr == "171") {//USER_WP
			//csdInfo.modify_extcsd.user_wp = regValue.toInt(&bOk, 16);
			m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(USER_WP, 0), true, Qt::UserRole);
		}
		else if (regAddr == "53:52" || regAddr == "52") {//EXT_PARTITIONS_ATTRIBUITE
			for (int i = 0; i < regValue.size() / 2; i++) {
				bool bOK;
				QString twoDigits = regValue.mid(i * 2, 2); // 获取两位数字的子字符串
				//csdInfo.modify_extcsd.ext_partitions_attribute[i] = twoDigits.toUInt(&bOK, 16); // 将子字符串转换为整数并存储
			}
			m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(EXT_PARTITIONS_ATTRIBUITE, 0), true, Qt::UserRole);
		}
		else if (regAddr == "134") {//SEC_BAD_BLK_MGMNT
			//csdInfo.modify_extcsd.sec_bad_blk_mgmnt = regValue.toInt(&bOk, 16);
			m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(SEC_BAD_BLK_MGMNT, 0), true, Qt::UserRole);
		}
		else if (regAddr == "16") {//SECURE_REMOVAL_TYPE
			//csdInfo.modify_extcsd.secure_removal_type = regValue.toInt(&bOk, 16);
			m_eCSDDataTableModel->setData(m_eCSDDataTableModel->index(SECURE_REMOVAL_TYPE, 0), true, Qt::UserRole);
		}
		else if (regAddr == "139:136" || regAddr == "136") {//ENH_START_ADDR
			for (int i = 0; i < regValue.size() / 2; ++i) {
				bool bOK;
				QString twoDigits = regValue.mid(i * 2, 2); // 获取两位数字的子字符串
				//csdInfo.modify_extcsd.enh_start_addr[i] = twoDigits.toUInt(&bOK, 16); // 将子字符串转换为整数并存储
			}
			ui->modifyGroup->setChecked(true);
		}
		else if (regAddr == "142:140" || regAddr == "140") {//ENH_SIZE_MULT
			for (int i = 0; i < regValue.size() / 2; ++i) {
				bool bOK;
				QString twoDigits = regValue.mid(i * 2, 2); // 获取两位数字的子字符串
				//csdInfo.modify_extcsd.enh_size_mult[i] = twoDigits.toUInt(&bOK, 16); // 将子字符串转换为整数并存储
			}
			ui->startaddrGroup->setChecked(true);
		}
	}
	delete[] extCSDInfo;

}