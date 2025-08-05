#pragma once

#include <QSortFilterProxyModel>

class AngKSortFilterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	AngKSortFilterProxyModel(QObject *parent = nullptr);
	~AngKSortFilterProxyModel();

	virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
	void setFilterKeyColumn(int column);

	void setSearchStr(QString _str);
private:
	int m_column;
	QString m_searchStr;
};
