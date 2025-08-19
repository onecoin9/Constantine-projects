#include "AngKChipSelectWidget.h"
#include "ui_AngKChipSelectWidget.h"
#include "StyleInit.h"
#include "AngKGlobalInstance.h"
#include "AngKDBStandardItemModel.h"
#include "AngKDBStandardItemModelEx.h"
#include "AngKSortFilterProxyModel.h"
#include "GlobalDefine.h"
#include "ACMessageBox.h"
#include <QFileDialog>
#include <QMouseEvent>
#include <QtConcurrent>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMetaType>
#include <QMessageBox>
#include <QRegularExpression>

Q_DECLARE_METATYPE(std::vector<chip>);

AngKChipSelectWidget::AngKChipSelectWidget(QWidget *parent)
	: QWidget(parent)
	, m_manufactureTableModel(nullptr)
	, m_manufactureListModel(nullptr)
	, m_manufactureFilterTableModel(nullptr)
	, m_DBStandardItemModelEx(nullptr)
	, m_pChipManager(nullptr)
{
	ui = new Ui::AngKChipSelectWidget();
	ui->setupUi(this);

	InitText();
	InitListView();
	InitChipTable();

	ui->adapterComboBox->setView(new QListView());
	ui->typeComboBox->setView(new QListView());

	connect(ui->selectPushButton, &QPushButton::clicked, this, &AngKChipSelectWidget::onSlotSelectChip);
	connect(ui->cancelPushButton, &QPushButton::clicked, this, &AngKChipSelectWidget::sgnCancel);
	connect(ui->exportButton, &QPushButton::clicked, this, &AngKChipSelectWidget::onSlotExportTable);
	connect(ui->searchLabelEdit, &QLineEdit::textEdited, this, &AngKChipSelectWidget::onSlotSearchText);
	connect(ui->adapterComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AngKChipSelectWidget::onSlotAdapterClick);
	connect(ui->typeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AngKChipSelectWidget::onSlotChipTypeClick);

	ui->BeginningRadio->hide();
	ui->ExactRadio->hide();
	ui->SubstringRadio->hide();

	this->setObjectName("AngKChipSelectWidget");
	QT_SET_STYLE_SHEET(objectName());
}

AngKChipSelectWidget::~AngKChipSelectWidget()
{
	if (m_manufactureTableModel != nullptr)
	{
		m_manufactureTableModel->clear();
		SAFEDEL(m_manufactureTableModel);
	}

	if (m_manufactureListModel != nullptr)
	{
		m_manufactureListModel->removeRows(0, m_manufactureListModel->rowCount());
		SAFEDEL(m_manufactureListModel);
	}

	if (m_manufactureFilterTableModel != nullptr)
	{
		m_manufactureFilterTableModel->clear();
		SAFEDEL(m_manufactureFilterTableModel);
	}

	if (m_DBStandardItemModelEx != nullptr)
	{
		m_DBStandardItemModelEx->clear();
		SAFEDEL(m_DBStandardItemModelEx);
	}

	m_varDataModelMap.clear();

	delete ui;
}

void AngKChipSelectWidget::InitText()
{
	ui->searchLabel->setText(tr("Search"));
	ui->SubstringRadio->setText(tr("Substring"));
	ui->ExactRadio->setText(tr("Exact"));
	ui->BeginningRadio->setText(tr("Beginning"));
	ui->adapterLabel->setText(tr("Adapter"));
	ui->typeLabel->setText(tr("Type"));
	ui->manufactureLabel->setText(tr("Manufacture"));
	ui->selectPushButton->setText(tr("Select"));
	ui->cancelPushButton->setText(tr("Cancel"));
	ui->exportButton->setText(tr("Export"));

	ui->SubstringRadio->setChecked(true);
}

void AngKChipSelectWidget::InitChipTable()
{
	//m_manufactureTableModel = new QStandardItemModel();

	m_DBStandardItemModelEx = new AngKDBStandardItemModelEx(this);
	m_manufactureFilterTableModel = new AngKSortFilterProxyModel(this);
	//m_manufactureFilterTableModel = new QSortFilterProxyModel(this);
	m_manufactureFilterTableModel->setSourceModel(m_DBStandardItemModelEx);

	// 隐藏水平表头
	ui->manufactureTableView->verticalHeader()->setVisible(false);
	
	QStringList headList;
	m_varDataModelMap["headerlabel"] = headList << tr("Manufact") << tr("Name") << tr("Bottom Board") << tr("Adapter") << tr("Package") << tr("Type") << tr("Status") << tr("AllChipInfo");

	m_DBStandardItemModelEx->SetData(m_varDataModelMap);
	//m_DBStandardItemModel->setHorizontalHeaderLabels(headList);
	ui->manufactureTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui->manufactureTableView->setModel(m_manufactureFilterTableModel);
	ui->manufactureTableView->setAlternatingRowColors(true);
	ui->manufactureTableView->horizontalHeader()->setHighlightSections(false);
	ui->manufactureTableView->horizontalHeader()->setStretchLastSection(true);
	ui->manufactureTableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	connect(ui->manufactureTableView, &AngKTableView::doubleClicked, this, &AngKChipSelectWidget::onSlotDoubleSelectChip);

	QHeaderView* manuHead = ui->manufactureTableView->horizontalHeader();

	// 列宽度自适应效果不好，自定宽度并可以拖动
	manuHead->setSectionResizeMode(QHeaderView::Interactive);
	resetView();
}

void AngKChipSelectWidget::InitListView()
{
	//初始化QlistViewde的Model
	m_manufactureListModel = new QStringListModel(ui->manufactureListView);

	ui->manufactureListView->setModel(m_manufactureListModel);
	ui->manufactureListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(ui->manufactureListView, &QListView::clicked, this, &AngKChipSelectWidget::onSlotClickListView);
}

void AngKChipSelectWidget::setManufactureList(QString singleManu)
{
	m_manufactureListModel->insertRow(m_manufactureListModel->rowCount());
	m_manufactureListModel->setData(m_manufactureListModel->index(m_manufactureListModel->rowCount() - 1, 0), singleManu);
}

void AngKChipSelectWidget::InitChipDBData()
{
	if(m_pChipManager == nullptr)
		m_pChipManager = new ACChipManager();

	setDBManufactureData(m_pChipManager->GetALLManufactureData());
	setDBAdapterData(m_pChipManager->GetALLAdapterData());
	setDBChipTypeData(m_pChipManager->GetALLChipTypeData());
	LoadChipData2Table();
}

void AngKChipSelectWidget::setDBManufactureData(const std::vector<manufacture> manuVec)
{
	for (int i = 0; i < manuVec.size(); ++i)
	{
		setManufactureList(QString::fromStdString(manuVec[i].name));
	}
}

void AngKChipSelectWidget::setDBAdapterData(const std::vector<adapter> adapterVec)
{
	ui->adapterComboBox->addItem("ALL");
	for (int i = 0; i < adapterVec.size(); ++i)
	{
		ui->adapterComboBox->addItem(QString::fromStdString(adapterVec[i].name));
	}
}

void AngKChipSelectWidget::setDBChipTypeData(const std::vector<chiptype> chiptypeVec)
{
	ui->typeComboBox->addItem("ALL");
	for (int i = 0; i < chiptypeVec.size(); ++i)
	{
		ui->typeComboBox->addItem(QString::fromStdString(chiptypeVec[i].name));
	}
}

bool AngKChipSelectWidget::ParseUnTestValue(const QString& jsonString) {
	QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());

	if (doc.isNull() || !doc.isObject()) {
		return false;
	}

	QJsonObject rootObj = doc.object();

	if (!rootObj.contains("otherOper") || !rootObj["otherOper"].isObject()) {
		return false;
	}
	QJsonObject otherOperObj = rootObj["otherOper"].toObject();

	if (!otherOperObj.contains("unTest") || !otherOperObj["unTest"].isBool()) {
		return false;
	}

	return otherOperObj["unTest"].toBool();
}

void AngKChipSelectWidget::LoadChipData2Table()
{
	std::vector<chip> chipVec = m_pChipManager->GetALLChipData();
	for (size_t i = 0; i < chipVec.size(); ++i)
	{
		QVariantMap chipMap;
		chipMap[tr("Manufact")] = QString::fromStdString(chipVec[i].strManu);
		chipMap[tr("Name")] = QString::fromStdString(chipVec[i].strName);

		QString strItemValue;
		if (((chipVec[i].nProgType >> 24) & 0xFF) == 0x03) {
			strItemValue = "AP9900";
		}
		else if (((chipVec[i].nProgType >> 24) & 0xFF) == 0x02) {
			strItemValue = "Bottom Board-AG01";
		}
		else if (((chipVec[i].nProgType >> 24) & 0xFF) == 0x05) {
			strItemValue = "Bottom Board-AG06";
		}
		else if (((chipVec[i].nProgType >> 24) & 0xFF) == 0x04) {
			strItemValue = "Bottom Board-AG07";
		}
		else {
			strItemValue = "None";
		}
		chipVec[i].strBottomBoard = strItemValue.toStdString();
		chipMap[tr("Bottom Board")] = strItemValue;
		chipMap[tr("Adapter")] = QString::fromStdString(chipVec[i].strAdapter);
		chipMap[tr("Package")] = QString::fromStdString(chipVec[i].strPack);
		chipMap[tr("Type")] = QString::fromStdString(chipVec[i].strType);
		//chipMap[tr("Status")] = "";
		chipMap[tr("Status")] = ParseUnTestValue(QString::fromStdString(chipVec[i].strOperCfgJson)) ? "Beta" : "Release";
		//chipMap[tr("AllChipInfo")] = QVariant::fromValue(m_chipVec[i]);
		chipMap[tr("AllChipInfo")] = i;
		m_varDataModelMap[QString("%1").arg(i, 8, 10, QLatin1Char('0'))] = chipMap;
	}

	m_DBStandardItemModelEx->SetData(m_varDataModelMap);
	UpdateStringListModel();
}


void AngKChipSelectWidget::onSlotClickListView(const QModelIndex& modelIdx)
{
	QModelIndex outModel;
	if (FindManuItem(m_manufactureListModel->data(modelIdx).toString(), outModel)) {
		if (outModel.isValid()) {
			ui->manufactureTableView->scrollTo(outModel, QAbstractItemView::PositionAtTop);
		}
	}
	ui->manufactureTableView->reset();
	resetView();
}

void AngKChipSelectWidget::onSlotSearchText(const QString& test)
{
	m_manufactureFilterTableModel->setSearchStr(test);

	m_manufactureFilterTableModel->invalidate();
	UpdateStringListModel();
	ui->manufactureTableView->reset();
	resetView();
}

void AngKChipSelectWidget::onSlotAdapterClick(int index)
{
	if (index == 0)
	{
		m_manufactureFilterTableModel->setFilterFixedString("");
	}
	else {
		QString adapterText = ui->adapterComboBox->currentText();

		m_manufactureFilterTableModel->setFilterFixedString(adapterText);
	}
	m_manufactureFilterTableModel->invalidate();
	m_manufactureFilterTableModel->setAdapterAndType(ui->adapterComboBox->currentText(), ui->typeComboBox->currentText());
	m_manufactureFilterTableModel->setFilterKeyColumn(ChipAdapter);
	UpdateStringListModel();
	ui->manufactureTableView->reset();
	resetView();
}

void AngKChipSelectWidget::onSlotChipTypeClick(int index)
{
	if (index == 0)
	{
		m_manufactureFilterTableModel->setFilterFixedString("");
	}
	else {

		m_manufactureFilterTableModel->setFilterFixedString(ui->typeComboBox->currentText());
	}
	m_manufactureFilterTableModel->invalidate();

	m_manufactureFilterTableModel->setAdapterAndType(ui->adapterComboBox->currentText(), ui->typeComboBox->currentText());
	m_manufactureFilterTableModel->setFilterKeyColumn(ChipType);
	UpdateStringListModel();

	ui->manufactureTableView->reset();
	resetView();
}

void AngKChipSelectWidget::onSlotDoubleSelectChip(const QModelIndex& doubleModel)
{
	if (doubleModel.isValid())
	{
		ChipDataJsonSerial chipJson;
		int nChipIndex = m_manufactureFilterTableModel->data(m_manufactureFilterTableModel->index(doubleModel.row(), AllChipInfo)).toInt();
		m_pChipManager->SetSelectChipData(nChipIndex);
		chipJson.serialize(m_pChipManager->GetChipData());

		emit sgnSelectChipDataJson(chipJson);
	}
}

void AngKChipSelectWidget::onSlotExportTable()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Excel file"), qApp->applicationDirPath(), tr("Files (*.csv)"));
	if (fileName.isEmpty())
		return;

	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qDebug() << "Failed to open file for writing:" << file.errorString();
		return;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");

	//添加导出基本信息
	QString strHead;
	out << "[Header]\n";
	out << "//Chip Information\n";
	out << "//This file is created automatically. Please do not change it manually\n";
	out << "DeviceType : AG06\n";

	QString strVersion = "Version : " + AngKGlobalInstance::ReadValue("Version", "BuildVer").toString() + "\n";
	out << strVersion;
	QString totalCounter = "Total Counter : " + ui->numText->text() + "\n";
	out << totalCounter;
	out << "[END]\n";

	out << "[ChipList]\n";
	// 获取列数
	int columnCount = m_manufactureFilterTableModel->columnCount();

	// 写入表头
	for (int column = 0; column < columnCount - 1; ++column) {
		if (column > 0) {
			out << ",";
		}
		out << m_manufactureFilterTableModel->headerData(column, Qt::Horizontal, Qt::DisplayRole).toString();
	}
	out << "\n";

	// 遍历过滤后的数据并写入到 CSV 文件中
	for (int row = 0; row < m_manufactureFilterTableModel->rowCount(); ++row) {
		for (int column = 0; column < columnCount; ++column) {
			if (column > 0) {
				out << ",";
			}
			QModelIndex index = m_manufactureFilterTableModel->index(row, column);
			out << index.data(Qt::DisplayRole).toString();
		}
		out << "\n";
	}
	out << "[END]\n";
	file.close();
	ACMessageBox::showInformation(this, tr("Save Tips"), tr("Save chip information successfully"));
}

void AngKChipSelectWidget::InsertData(QString name, QString manufact, QString adapter, QString package, QString strType, QString status)
{
	int row = m_manufactureTableModel->rowCount();
	m_manufactureTableModel->insertRow(m_manufactureTableModel->rowCount());
	m_manufactureTableModel->setData(m_manufactureTableModel->index(row, ChipManufact), name);
	m_manufactureTableModel->item(row, ChipManufact)->setCheckable(true);

	m_manufactureTableModel->setData(m_manufactureTableModel->index(row, ChipName), manufact);
	m_manufactureTableModel->setData(m_manufactureTableModel->index(row, ChipAdapter), adapter);
	m_manufactureTableModel->setData(m_manufactureTableModel->index(row, ChipPackage), package);
	m_manufactureTableModel->setData(m_manufactureTableModel->index(row, ChipType), strType);
	m_manufactureTableModel->setData(m_manufactureTableModel->index(row, ChipStatus), status);
}

void AngKChipSelectWidget::resetView()
{
	ui->manufactureTableView->setColumnWidth(0, 155);
	ui->manufactureTableView->setColumnWidth(1, 200);
	ui->manufactureTableView->setColumnWidth(2, 150);
	ui->manufactureTableView->setColumnWidth(3, 150);
	ui->manufactureTableView->setColumnWidth(4, 150);
	ui->manufactureTableView->setColumnWidth(5, 110);
	ui->manufactureTableView->setColumnWidth(6, 100);
	ui->manufactureTableView->setColumnHidden(7, true); // 隐藏第7列
}

bool AngKChipSelectWidget::FindManuItem(QString strManu, QModelIndex& outIndex)
{
	for (int row = 0; row < m_manufactureFilterTableModel->rowCount(); ++row) {
		for (int column = 0; column < m_manufactureFilterTableModel->columnCount(); ++column) {
			QModelIndex index = m_manufactureFilterTableModel->index(row, column);
			if (index.isValid()) {
				if (m_manufactureFilterTableModel->data(index).toString() == strManu) {
					outIndex = index;
					return true; // 找到目标数据返回
				}
			}
		}
	}

	return false;
}

void AngKChipSelectWidget::UpdateStringListModel()
{
	// 清空 QStringListModel 中的数据
	m_manufactureListModel->setStringList(QStringList());

	// 获取过滤后的数据
	QStringList filteredData;

	for (int row = 0; row < m_manufactureFilterTableModel->rowCount(); ++row) {
		// 获取过滤后的每一行数据
		QModelIndex index = m_manufactureFilterTableModel->index(row, ChipManufact);
		QVariant data = index.data(Qt::DisplayRole);
		QVariant editdata = index.data(Qt::EditRole);

		// 将数据添加到 QStringList 中
		filteredData.append(data.toString());
	}

	filteredData.removeDuplicates();
	// 将过滤后的数据设置到 QStringListModel 中
	m_manufactureListModel->setStringList(filteredData);

	ui->numText->setText(QString::number(m_manufactureFilterTableModel->rowCount()));
}

void AngKChipSelectWidget::onSlotSelectChip()
{
	QModelIndex modIdx = ui->manufactureTableView->currentIndex();

	if (modIdx.isValid())
	{
		ChipDataJsonSerial chipJson;
		int nChipIndex = m_manufactureFilterTableModel->data(m_manufactureFilterTableModel->index(modIdx.row(), AllChipInfo)).toInt();
		m_pChipManager->SetSelectChipData(nChipIndex);
		chipJson.serialize(m_pChipManager->GetChipData());

		emit sgnSelectChipDataJson(chipJson);
	}
}