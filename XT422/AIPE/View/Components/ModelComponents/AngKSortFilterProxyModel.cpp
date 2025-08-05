#include "AngKSortFilterProxyModel.h"
#include "GlobalDefine.h"

AngKSortFilterProxyModel::AngKSortFilterProxyModel(QObject *parent)
	: QSortFilterProxyModel(parent)
{
}

AngKSortFilterProxyModel::~AngKSortFilterProxyModel()
{
}

bool AngKSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
	if (m_column == -1){
		for (int i = 0; i < sourceModel()->columnCount(); ++i) {
			QString dataColumn = this->sourceModel()->index(source_row, i, source_parent).data(Qt::DisplayRole).toString();
			if (dataColumn.contains(m_searchStr, Qt::CaseInsensitive)) {
				return true;
			}
		}
	}
	else if (m_column == ChipAdapter || m_column == ChipType)
	{
		bool isPass = false;
		QString dataColumn1 = this->sourceModel()->index(source_row, ChipAdapter, source_parent).data(Qt::DisplayRole).toString();
		QString dataColumn2 = this->sourceModel()->index(source_row, ChipType, source_parent).data(Qt::DisplayRole).toString();

		if(m_adapterCombox.isEmpty() && m_typeCombox.isEmpty()){
			//return true;
			isPass = true;
		}
		else if(m_adapterCombox.isEmpty() && !m_typeCombox.isEmpty()){
			if (dataColumn2.compare(m_typeCombox) == 0) {
				//return true;
				isPass = true;
			}
		}
		else if (!m_adapterCombox.isEmpty() && m_typeCombox.isEmpty()) {
			if (dataColumn1.compare(m_adapterCombox) == 0) {
				//return true;
				isPass = true;
			}
		}
		else {
			if (dataColumn1.compare(m_adapterCombox) == 0 && dataColumn2.compare(m_typeCombox) == 0) {
				//return true;
				isPass = true;
			}
		}
		
		if (isPass) {
			for (int i = 0; i < sourceModel()->columnCount(); ++i) {
				QString dataColumn = this->sourceModel()->index(source_row, i, source_parent).data(Qt::DisplayRole).toString();
				if (dataColumn.contains(m_searchStr, Qt::CaseInsensitive)) {
					return true;
				}
			}
		}
	}

	return false;
}

void AngKSortFilterProxyModel::setFilterKeyColumn(int column)
{
	m_column = column;
	QSortFilterProxyModel::setFilterKeyColumn(column);
}

void AngKSortFilterProxyModel::setSearchStr(QString _str)
{
	m_searchStr = _str;
}

void AngKSortFilterProxyModel::setAdapterAndType(QString adapterStr, QString typeStr)
{
	m_adapterCombox = adapterStr;
	m_typeCombox = typeStr;

	if (m_adapterCombox == "ALL") {
		m_adapterCombox.clear();
	}

	if (m_typeCombox == "ALL") {
		m_typeCombox.clear();
	}
}
