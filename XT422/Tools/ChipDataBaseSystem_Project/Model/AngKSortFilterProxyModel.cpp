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
	if (m_column == -1)
	{

		return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
	}
    else if (m_column == ChipAdapter1 || m_column == ChipType)
	{
		QString dataColumn1 = this->sourceModel()->index(source_row, m_column, source_parent).data(Qt::DisplayRole).toString();
		if (dataColumn1 == m_searchStr || m_searchStr.isEmpty())
		{
			return true;
		}
		return false;
	}
	else
	{
		QString dataColumn1 = this->sourceModel()->index(source_row, m_column, source_parent).data(Qt::DisplayRole).toString();
		if (dataColumn1.contains(m_searchStr))
		{
			return true;
		}
		return false;
	}
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
