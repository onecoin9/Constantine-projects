#pragma once

#include <QThread>
#include <QString>
#include <QtWidgets/QWidget>
#include <QMap>
#include "DataJsonSerial.hpp"
#include "ACChipManager.h"

namespace Ui { class AngKChipSelectWidget; };

class QSortFilterProxyModel;
class AngKSortFilterProxyModel;
class QStandardItemModel;
class QStringListModel;
class AngKDBStandardItemModel;
class AngKDBStandardItemModelEx;
class AngKChipSelectWidget : public QWidget
{
	Q_OBJECT

public:
	AngKChipSelectWidget(QWidget *parent = Q_NULLPTR);
	~AngKChipSelectWidget();

	void InitText();

	void InitChipTable();

	void InitListView();

	void InitChipDBData();

	void setManufactureList(QString singleManu);

	void setDBManufactureData(const std::vector<manufacture> manuVec);

	void setDBAdapterData(const std::vector<adapter> adapterVec);

	void setDBChipTypeData(const std::vector<chiptype> chiptypeVec);
private:
	bool ParseUnTestValue(const QString& jsonString);
	void LoadChipData2Table();
	void InsertData(QString name, QString manufact = "", QString adapter = "", QString package = "", QString strType = "", QString status = "");
	void resetView();
	bool FindManuItem(QString strManu, QModelIndex& outIndex);
	void UpdateStringListModel();

signals:
	void sgnCancel();
	void sgnSelectChipDataJson(ChipDataJsonSerial);

public slots:
	void onSlotSelectChip();
	void onSlotClickListView(const QModelIndex&);
	void onSlotSearchText(const QString&);
	void onSlotAdapterClick(int);
	void onSlotChipTypeClick(int);
	void onSlotDoubleSelectChip(const QModelIndex&);
	void onSlotExportTable();
private:
	Ui::AngKChipSelectWidget	*ui;
	QStandardItemModel*			m_manufactureTableModel;
	QStringListModel*			m_manufactureListModel;
	AngKSortFilterProxyModel*	m_manufactureFilterTableModel;
	AngKDBStandardItemModelEx*	m_DBStandardItemModelEx;
	QVariantMap					m_varDataModelMap;
	ACChipManager*				m_pChipManager;
};

Q_DECLARE_METATYPE(chip);