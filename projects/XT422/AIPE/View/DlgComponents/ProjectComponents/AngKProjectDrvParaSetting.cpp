#include "AngKProjectDrvParaSetting.h"
#include "ui_AngKProjectDrvParaSetting.h"
#include "AngKCustomDelegate.h"
#include "AngKTableView.h"
#include "json.hpp"
#include "StyleInit.h"
#include <QStandardItemModel>
#include <QToolTip>
AngKProjectDrvParaSetting::AngKProjectDrvParaSetting(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::AngKProjectDrvParaSetting();
	ui->setupUi(this);

	InitTable();

	this->setObjectName("AngKProjectDrvParaSetting");
	QT_SET_STYLE_SHEET(objectName());
}

AngKProjectDrvParaSetting::~AngKProjectDrvParaSetting()
{
	delete ui;
}

void AngKProjectDrvParaSetting::InitTable()
{
	//CommonTable
	m_DriverCommonParaTableModel = new QStandardItemModel(this);
	AngKCustomDelegate* customCommonDelegate = new AngKCustomDelegate();
	customCommonDelegate->setEditColumn(2);
	customCommonDelegate->setCheckEnable(false);

	// 隐藏水平表头
	ui->CommonTable->verticalHeader()->setVisible(false);
	ui->CommonTable->setMouseTracking(true);
	connect(ui->CommonTable, &AngKTableView::entered, this, [=](QModelIndex modelIdx) {
		if (!modelIdx.isValid()) {
			return;

		}
		QToolTip::showText(QCursor::pos(), modelIdx.data().toString());

		});

	QStringList headList;
	headList << tr("Name") << tr("Range") << tr("Value") << tr("Description");

	m_DriverCommonParaTableModel->setHorizontalHeaderLabels(headList);
	m_DriverCommonParaTableModel->setProperty("CommonPara", "CommonPara");

	ui->CommonTable->setItemDelegate(customCommonDelegate);
	ui->CommonTable->setModel(m_DriverCommonParaTableModel);
	ui->CommonTable->setAlternatingRowColors(true);
	ui->CommonTable->horizontalHeader()->setHighlightSections(false);
	ui->CommonTable->horizontalHeader()->setStretchLastSection(true);
	ui->CommonTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

	QHeaderView* manuCommonHead = ui->CommonTable->horizontalHeader();

	manuCommonHead->setSectionResizeMode(QHeaderView::Stretch);

	//SelfTable
	m_DriverSelfParaTableModel = new QStandardItemModel(this);
	AngKCustomDelegate* customSelfDelegate = new AngKCustomDelegate();
	customSelfDelegate->setEditColumn(2);
	customSelfDelegate->setCheckEnable(false);

	// 隐藏水平表头
	ui->SelfTable->verticalHeader()->setVisible(false);
	ui->SelfTable->setMouseTracking(true);
	connect(ui->SelfTable, &AngKTableView::entered, this, [=](QModelIndex modelIdx) {
		if (!modelIdx.isValid()) {
			return;

		}
		QToolTip::showText(QCursor::pos(), modelIdx.data().toString());

		});

	m_DriverSelfParaTableModel->setHorizontalHeaderLabels(headList);
	m_DriverSelfParaTableModel->setProperty("SelfPara", "SelfPara");

	ui->SelfTable->setItemDelegate(customSelfDelegate);
	ui->SelfTable->setModel(m_DriverSelfParaTableModel);
	ui->SelfTable->setAlternatingRowColors(true);
	ui->SelfTable->horizontalHeader()->setHighlightSections(false);
	ui->SelfTable->horizontalHeader()->setStretchLastSection(true);
	ui->SelfTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

	QHeaderView* manuSelfHead = ui->SelfTable->horizontalHeader();

	manuSelfHead->setSectionResizeMode(QHeaderView::Stretch);
}

void AngKProjectDrvParaSetting::InsertCommonDrvPara(QString commonJson)
{
	if (commonJson.isEmpty())
		return;

	m_DriverCommonParaTableModel->removeRows(0, m_DriverCommonParaTableModel->rowCount());
	try {//nlohmann解析失败会报异常需要捕获一下

		nlohmann::json commonDrvJson = nlohmann::json::parse(commonJson.toStdString());

		int DrvCommonParaGroupCnt = commonDrvJson["DrvCommonParaGroupCnt"];
		for (int i = 0; i < DrvCommonParaGroupCnt; ++i) {
			nlohmann::json DrvCommonParasJson = commonDrvJson["DrvCommonParas"][i];

			int MembersGroupCnt = DrvCommonParasJson["MembersGroupCnt"];
			for (int mIdx = 0; mIdx < MembersGroupCnt; ++mIdx) {
				nlohmann::json MembersJson = DrvCommonParasJson["Members"][mIdx];

				int row = m_DriverCommonParaTableModel->rowCount();
				m_DriverCommonParaTableModel->insertRow(m_DriverCommonParaTableModel->rowCount());
				m_DriverCommonParaTableModel->setData(m_DriverCommonParaTableModel->index(row, 0), QString::fromStdString(MembersJson["Name"]));
				QString strRange = QString::fromStdString(MembersJson["Min"].get<std::string>() + "-" + MembersJson["Max"].get<std::string>());
				m_DriverCommonParaTableModel->setData(m_DriverCommonParaTableModel->index(row, 1), strRange);
				m_DriverCommonParaTableModel->setData(m_DriverCommonParaTableModel->index(row, 2), QString::fromStdString(MembersJson["Value"]));
				//std::string descriptionInfo = MembersJson["Description"].is_null() ? MembersJson["Description"] : MembersJson["Unit"];
				m_DriverCommonParaTableModel->setData(m_DriverCommonParaTableModel->index(row, 3), QString::fromStdString(MembersJson["Unit"]));
				//m_DriverCommonParaTableModel->setData(m_DriverCommonParaTableModel->index(row, 3), QString::fromStdString(MembersJson["Description"]), Qt::UserRole);

				if (mIdx + 1 == MembersGroupCnt) {
					QStandardItem* item = m_DriverCommonParaTableModel->item(row);
					item->setData(QString::fromStdString(DrvCommonParasJson["Struct"]), Qt::UserRole);
				}
			}
		}
	}
	catch (const nlohmann::json::exception& e) {
		ALOG_FATAL("Insert CommonDrvPara Json parse failed : %s.", "CU", "--", e.what());
	}
}

void AngKProjectDrvParaSetting::InsertSelfDrvPara(QString selfJson)
{
	if (selfJson.isEmpty())
		return;

	m_DriverSelfParaTableModel->removeRows(0, m_DriverSelfParaTableModel->rowCount());
	try {//nlohmann解析失败会报异常需要捕获一下

		nlohmann::json selfDrvJson = nlohmann::json::parse(selfJson.toStdString());
		int DrvSelfParaGroupCnt = selfDrvJson["DrvSelfParaGroupCnt"];
		for (int i = 0; i < DrvSelfParaGroupCnt; ++i) {
			nlohmann::json DrvSelfParasJson = selfDrvJson["DrvSelfParas"][i];

			int MembersGroupCnt = DrvSelfParasJson["MembersGroupCnt"];
			for (int mIdx = 0; mIdx < MembersGroupCnt; ++mIdx) {

				nlohmann::json MembersJson = DrvSelfParasJson["Members"][mIdx];

				int row = m_DriverSelfParaTableModel->rowCount();
				m_DriverSelfParaTableModel->insertRow(m_DriverSelfParaTableModel->rowCount());
				m_DriverSelfParaTableModel->setData(m_DriverSelfParaTableModel->index(row, 0), QString::fromStdString(MembersJson["Name"]));
				QString strRange = QString::fromStdString(MembersJson["Min"].get<std::string>() + "-" + MembersJson["Max"].get<std::string>());
				m_DriverSelfParaTableModel->setData(m_DriverSelfParaTableModel->index(row, 1), strRange);
				m_DriverSelfParaTableModel->setData(m_DriverSelfParaTableModel->index(row, 2), QString::fromStdString(MembersJson["Value"]));
				//std::string descriptionInfo = MembersJson["Description"].is_null() ? MembersJson["Description"] : MembersJson["Unit"];
				m_DriverSelfParaTableModel->setData(m_DriverSelfParaTableModel->index(row, 3), QString::fromStdString(MembersJson["Unit"]));
				//m_DriverSelfParaTableModel->setData(m_DriverSelfParaTableModel->index(row, 3), QString::fromStdString(MembersJson["Description"]), Qt::UserRole);

				if (mIdx + 1 == MembersGroupCnt) {
					QStandardItem* item = m_DriverSelfParaTableModel->item(row);
					item->setData(QString::fromStdString(DrvSelfParasJson["Struct"]), Qt::UserRole);
				}
			}
		}

	}
	catch (const nlohmann::json::exception& e) {
		ALOG_FATAL("InsertSelfDrvPara Json parse failed : %s.", "CU", "--", e.what());
	}
}

void AngKProjectDrvParaSetting::GetDrvParaJson(QString& commonJson, QString& selfJson)
{
	//通用驱动数据保存
	nlohmann::json commonDrvJson;
	commonDrvJson["DrvCommonParas"] = nlohmann::json::array();
	int commonRow = m_DriverCommonParaTableModel->rowCount();
	nlohmann::json MembersArray = nlohmann::json::array();
	for (int i = 0; i < commonRow; ++i) {
		nlohmann::json MembersJson;
		MembersJson["Name"] = m_DriverCommonParaTableModel->data(m_DriverCommonParaTableModel->index(i, 0)).toString().toStdString();
		QStringList rangList = m_DriverCommonParaTableModel->data(m_DriverCommonParaTableModel->index(i, 1)).toString().split("-");
		if (!rangList.empty() && rangList.size() == 2) {
			MembersJson["Min"] = rangList[0].toStdString();
			MembersJson["Max"] = rangList[1].toStdString();
		}
		MembersJson["Value"] = m_DriverCommonParaTableModel->data(m_DriverCommonParaTableModel->index(i, 2)).toString().toStdString();
		MembersJson["Unit"] = m_DriverCommonParaTableModel->data(m_DriverCommonParaTableModel->index(i, 3)).toString().toStdString();

		MembersArray.push_back(MembersJson);
		if (!m_DriverCommonParaTableModel->item(i)->data(Qt::UserRole).toString().isEmpty()) {
			nlohmann::json CommonPara;
			CommonPara["Struct"] = m_DriverCommonParaTableModel->item(i)->data(Qt::UserRole).toString().toStdString();
			CommonPara["MembersGroupCnt"] = MembersArray.size();
			CommonPara["Members"] = MembersArray;
			commonDrvJson["DrvCommonParas"].push_back(CommonPara);
			MembersArray.clear();
		}
	}

	commonDrvJson["DrvCommonParaGroupCnt"] = commonDrvJson["DrvCommonParas"].size();

	commonJson = QString::fromStdString(commonDrvJson.dump());

	//自定义驱动数据保存
	nlohmann::json selfDrvJson;
	selfDrvJson["DrvSelfParas"] = nlohmann::json::array();
	int selfRow = m_DriverSelfParaTableModel->rowCount();
	nlohmann::json selfMembersArray = nlohmann::json::array();
	for (int j = 0; j < selfRow; ++j) {
		nlohmann::json MembersJson;
		MembersJson["Name"] = m_DriverSelfParaTableModel->data(m_DriverSelfParaTableModel->index(j, 0)).toString().toStdString();
		QStringList rangList = m_DriverSelfParaTableModel->data(m_DriverSelfParaTableModel->index(j, 1)).toString().split("-");
		if (!rangList.empty() && rangList.size() == 2) {
			MembersJson["Min"] = rangList[0].toStdString();
			MembersJson["Max"] = rangList[1].toStdString();
		}
		MembersJson["Value"] = m_DriverSelfParaTableModel->data(m_DriverSelfParaTableModel->index(j, 2)).toString().toStdString();
		MembersJson["Unit"] = m_DriverSelfParaTableModel->data(m_DriverSelfParaTableModel->index(j, 3)).toString().toStdString();

		selfMembersArray.push_back(MembersJson);
		if (!m_DriverSelfParaTableModel->item(j)->data(Qt::UserRole).toString().isEmpty()) {
			nlohmann::json SelfPara;
			SelfPara["Struct"] = m_DriverSelfParaTableModel->item(j)->data(Qt::UserRole).toString().toStdString();
			SelfPara["MembersGroupCnt"] = selfMembersArray.size();
			SelfPara["Members"] = selfMembersArray;
			selfDrvJson["DrvSelfParas"].push_back(SelfPara);
			selfMembersArray.clear();
		}
	}

	selfDrvJson["DrvSelfParaGroupCnt"] = selfDrvJson["DrvSelfParas"].size();

	selfJson = QString::fromStdString(selfDrvJson.dump());
}
