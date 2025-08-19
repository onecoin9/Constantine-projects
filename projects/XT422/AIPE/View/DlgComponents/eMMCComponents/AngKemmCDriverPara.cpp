#include "AngKemmCDriverPara.h"
#include "ui_AngKemmCDriverPara.h"
#include "StyleInit.h"
#include <QHeaderView>
#include <QPushButton>
#include <QStandardItemModel>
#include <QSettings>
#include <QToolTip>
#include "AngKCustomDelegate.h"
#include "AngKTableView.h"
AngKemmCDriverPara::AngKemmCDriverPara(QWidget* parent)
	: AngKDialog(parent)
	, m_DriverParaTableModel(nullptr)
	, m_DriverParaSetting(nullptr)
{
	this->setObjectName("AngKemmCDriverPara");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::AngKemmCDriverPara();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(640, 750);
	this->SetTitle(tr("eMMC Driver Parameters"));
	setAttribute(Qt::WA_TranslucentBackground, true);

	InitTable();
	InitButton();
}

AngKemmCDriverPara::~AngKemmCDriverPara()
{
	if (m_DriverParaTableModel)
	{
		m_DriverParaTableModel = nullptr;
		delete m_DriverParaTableModel;
	}
	delete ui;
}

void AngKemmCDriverPara::InitTable()
{
	m_DriverParaTableModel = new QStandardItemModel(this);
	AngKCustomDelegate* customDelegate = new AngKCustomDelegate();
	customDelegate->setEditColumn(2);
	customDelegate->setCheckEnable(false);

	// 隐藏水平表头
	ui->DriverTableView->verticalHeader()->setVisible(false);
	ui->DriverTableView->setMouseTracking(true);
	connect(ui->DriverTableView, &AngKTableView::entered, this, [=](QModelIndex modelIdx) {
		if (!modelIdx.isValid()) {
			return;

		}
		QToolTip::showText(QCursor::pos(), modelIdx.data().toString());

		});

	QStringList headList;
	headList << tr("Name") << tr("Range") << tr("Value") << tr("Description");

	m_DriverParaTableModel->setHorizontalHeaderLabels(headList);

	ui->DriverTableView->setItemDelegate(customDelegate);
	ui->DriverTableView->setModel(m_DriverParaTableModel);
	ui->DriverTableView->setAlternatingRowColors(true);
	ui->DriverTableView->horizontalHeader()->setHighlightSections(false);
	ui->DriverTableView->horizontalHeader()->setStretchLastSection(true);
	ui->DriverTableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

	QHeaderView* manuHead = ui->DriverTableView->horizontalHeader();

	manuHead->setSectionResizeMode(QHeaderView::ResizeToContents);
	//InsertFixedData();
}

void AngKemmCDriverPara::InsertFixedData()
{
	m_DriverParaTableModel->insertRows(m_DriverParaTableModel->rowCount(), 25);
	int count = m_DriverParaTableModel->rowCount();
	//固定第三列初始化为0
	for (int i = 0; i < count; ++i)
	{
		m_DriverParaTableModel->setData(m_DriverParaTableModel->index(i, 1), 0);
	}

	//Special Bit
	QVector<QString> spBitVec = { 
	tr("NAC Cycle"),
	tr("BlockRead Timeout Cycle"),
	tr("Erase size one Time(MBytes)"),
	tr("Timeout Write 32MBytes Data"),
	tr("Phase Tuning"),
	tr("Copy Between Different Device"),
	tr("Not strictly Match MCard"),
	tr("How many times to retry when all failed"),
	tr("Print CID Register Enable"),
	tr("Don't ReScan Before Verify"),
	tr("Chip Overlap Check"),
	tr("Use Open-ended method when writing data"),
	tr("Checksum Mode"),
	tr("Ignore ExtCsD[163] when copying"),
	tr("Ingore all ExtCsD Copying"),
	tr("Erase after Programmed"),
	tr("Set the default value when analyse"),
	tr("Phison Checksum Calcuation"),
	tr("Phison eMMC Preformat"),
	tr("UserArea Blankcheck size"),
	tr("Use Data Compression when Making MCard"),
	tr("Product State Awareness Enablement Value"),
	tr("Pre Loading Data Size"),
	tr("SLC Life time check"),
	tr("MLC Life time check") };
	for (int i = 0; i < count; ++i)
	{
		m_DriverParaTableModel->setData(m_DriverParaTableModel->index(i, 0), spBitVec[i]);
	}

	//Description
	QVector<QString> DescriptionVec = {
	tr("0 means use default"),
	tr("0 means use default"),
	tr("0 means use default"),
	tr("0 means use default"),
	tr("0: Disable,1: Enable"),
	tr("0: Disable,1: Enable"),
	tr("0: Disable,1: Enable"),
	tr("0:means not retry"),
	tr("0: Disable, 1: Enable"),
	tr("0:No,1:Yes"),
	tr("0:Disable,1-10:checkTimes, Enable Print CID First"),
	tr("0:No,1:Yes"),
	tr("Bito: 0: Normal, 1: ACD; Bit1: 1:FORCEN"),
	tr("0:No,1:Yes"),
	tr("0:No,1:Yes"),
	tr("0: Disable,1: Enable"),
	tr("0:Use the value from chip, 1:0x00,2: 0xFF)"),
	tr("0: from databuffer, 1: from chip"),
	tr("0: Disable,1: Enable"),
	tr("0:Disable,others: the MegaBytes to Blankcheck"),
	tr("0: Disable,1: Enable"),
	tr("1Byte, only Bit4,Bit5 Used"),
	tr("4Bytes Not Larger Than ExtCsD[21-18]"),
	tr("1-10 dec, 0: Disable"),
	tr("1-10 dec, 0: Disable")};
	for (int i = 0; i < count; ++i)
	{
		m_DriverParaTableModel->setData(m_DriverParaTableModel->index(i, 2), DescriptionVec[i]);
	}
}

void AngKemmCDriverPara::InitButton()
{
	connect(ui->okButton, &QPushButton::clicked, this, &AngKemmCDriverPara::onSlotOK);

	connect(ui->saveConfigButton, &QPushButton::clicked, this, &AngKemmCDriverPara::onSlotSaveConfigDriverPara);

	connect(this, &AngKemmCDriverPara::sgnClose, this, &AngKemmCDriverPara::close);
}

void AngKemmCDriverPara::SetLocfgIni(QSettings* _setting)
{
	m_DriverParaSetting = _setting;
}

void AngKemmCDriverPara::ReadLocfgIni()
{
	if (m_DriverParaSetting == nullptr)
		return;

	m_DriverParaTableModel->setData(m_DriverParaTableModel->index(5, 1), settingReadValue("eMMCDrvPara", "CopyBetweenDiffDevice").toInt());
	m_DriverParaTableModel->setData(m_DriverParaTableModel->index(6, 1), settingReadValue("eMMCDrvPara", "NoStrictlyMatchMcard").toInt());
	m_DriverParaTableModel->setData(m_DriverParaTableModel->index(7, 1), settingReadValue("eMMCDrvPara", "TimesRetry").toInt());
	m_DriverParaTableModel->setData(m_DriverParaTableModel->index(8, 1), settingReadValue("eMMCDrvPara", "PrintCID").toInt());
	m_DriverParaTableModel->setData(m_DriverParaTableModel->index(10, 1), settingReadValue("eMMCDrvPara", "ChipOverLap").toInt());
	m_DriverParaTableModel->setData(m_DriverParaTableModel->index(11, 1), settingReadValue("eMMCDrvPara", "OpenEndModeEn").toInt());
	m_DriverParaTableModel->setData(m_DriverParaTableModel->index(12, 1), settingReadValue("eMMCDrvPara", "ChecksumMode").toInt());
	m_DriverParaTableModel->setData(m_DriverParaTableModel->index(14, 1), settingReadValue("eMMCDrvPara", "IgnoreExtCSDCopy").toInt());
	m_DriverParaTableModel->setData(m_DriverParaTableModel->index(4, 1), settingReadValue("eMMCDrvPara", "PhaseTuningEn").toInt());
	m_DriverParaTableModel->setData(m_DriverParaTableModel->index(15, 1), settingReadValue("eMMCDrvPara", "EraseAfterProgram").toInt());
	m_DriverParaTableModel->setData(m_DriverParaTableModel->index(19, 1), settingReadValue("eMMCDrvPara", "UserAreaBlankCheck").toInt());
	m_DriverParaTableModel->setData(m_DriverParaTableModel->index(9, 1), settingReadValue("eMMCDrvPara", "DontRescanBeforeVerify").toInt());
	m_DriverParaTableModel->setData(m_DriverParaTableModel->index(13, 1), settingReadValue("eMMCDrvPara", "IgnoreExtCSD163").toInt());
	m_DriverParaTableModel->setData(m_DriverParaTableModel->index(16, 1), settingReadValue("eMMCDrvPara", "AnalyseUserValue").toInt());
	//m_DriverParaTableModel->setData(m_DriverParaTableModel->index(5, 1), settingReadValue("eMMCDrvPara", "PhisoneChecksumCalcMode").toInt());
	//m_DriverParaTableModel->setData(m_DriverParaTableModel->index(5, 1), settingReadValue("eMMCDrvPara", "LifetimeFilter").toInt());
	//m_DriverParaTableModel->setData(m_DriverParaTableModel->index(5, 1), settingReadValue("eMMCDrvPara", "EraseTimes").toInt());
	//m_DriverParaTableModel->setData(m_DriverParaTableModel->index(5, 1), settingReadValue("eMMCDrvPara", "RecycleSpecialOperation").toInt());
}

QVariant AngKemmCDriverPara::settingReadValue(QString strGroup, QString strValue)
{
	return m_DriverParaSetting->value(strGroup + "/" + strValue);
}

void AngKemmCDriverPara::settingWriteValue(QString strGroup, QString strName, QVariant strValue)
{
	m_DriverParaSetting->beginGroup(strGroup);
	m_DriverParaSetting->setValue(strName, strValue);
	m_DriverParaSetting->endGroup();
}

void AngKemmCDriverPara::onSlotSaveConfigDriverPara()
{
	settingWriteValue("eMMCDrvPara", "CopyBetweenDiffDevice", m_DriverParaTableModel->data(m_DriverParaTableModel->index(5, 1)).toInt());
	settingWriteValue("eMMCDrvPara", "NoStrictlyMatchMcard", m_DriverParaTableModel->data(m_DriverParaTableModel->index(6, 1)).toInt());
	settingWriteValue("eMMCDrvPara", "TimesRetry", m_DriverParaTableModel->data(m_DriverParaTableModel->index(7, 1)).toInt());
	settingWriteValue("eMMCDrvPara", "PrintCID", m_DriverParaTableModel->data(m_DriverParaTableModel->index(8, 1)).toInt());
	settingWriteValue("eMMCDrvPara", "ChipOverLap", m_DriverParaTableModel->data(m_DriverParaTableModel->index(10, 1)).toInt());
	settingWriteValue("eMMCDrvPara", "OpenEndModeEn", m_DriverParaTableModel->data(m_DriverParaTableModel->index(11, 1)).toInt());
	settingWriteValue("eMMCDrvPara", "ChecksumMode", m_DriverParaTableModel->data(m_DriverParaTableModel->index(12, 1)).toInt());
	settingWriteValue("eMMCDrvPara", "IgnoreExtCSDCopy", m_DriverParaTableModel->data(m_DriverParaTableModel->index(14, 1)).toInt());
	settingWriteValue("eMMCDrvPara", "PhaseTuningEn", m_DriverParaTableModel->data(m_DriverParaTableModel->index(4, 1)).toInt());
	settingWriteValue("eMMCDrvPara", "EraseAfterProgram", m_DriverParaTableModel->data(m_DriverParaTableModel->index(15, 1)).toInt());
	settingWriteValue("eMMCDrvPara", "UserAreaBlankCheck", m_DriverParaTableModel->data(m_DriverParaTableModel->index(19, 1)).toInt());
	settingWriteValue("eMMCDrvPara", "DontRescanBeforeVerify", m_DriverParaTableModel->data(m_DriverParaTableModel->index(9, 1)).toInt());
	settingWriteValue("eMMCDrvPara", "IgnoreExtCSD163", m_DriverParaTableModel->data(m_DriverParaTableModel->index(13, 1)).toInt());
	settingWriteValue("eMMCDrvPara", "AnalyseUserValue", m_DriverParaTableModel->data(m_DriverParaTableModel->index(16, 1)).toInt());
	//settingWriteValue("eMMCDrvPara", "PhisoneChecksumCalcMode", m_DriverParaTableModel->data(m_DriverParaTableModel->index(5, 1)).toInt());
	//settingWriteValue("eMMCDrvPara", "LifetimeFilter", m_DriverParaTableModel->data(m_DriverParaTableModel->index(5, 1)).toInt());
	//settingWriteValue("eMMCDrvPara", "EraseTimes", m_DriverParaTableModel->data(m_DriverParaTableModel->index(5, 1)).toInt());
	//settingWriteValue("eMMCDrvPara", "RecycleSpecialOperation", m_DriverParaTableModel->data(m_DriverParaTableModel->index(5, 1)).toInt());
}

void AngKemmCDriverPara::onSlotOK()
{

	close();
}
