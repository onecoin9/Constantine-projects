#pragma once

#include <QAbstractTableModel>
#include <QObject>
#include <QStringList>
#include <QVector>



class AngKDBStandardItemModelEx : public QAbstractTableModel
{
	Q_OBJECT

public:
	AngKDBStandardItemModelEx(QObject *parent = nullptr);
	~AngKDBStandardItemModelEx();

	// 自定义的函数，为整个model设置数据
	void SetData(const QVariantMap& map);

	virtual Qt::ItemFlags flags(const QModelIndex& index) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

	virtual int rowCount(const QModelIndex& parent) const;
	virtual int columnCount(const QModelIndex& parent) const;

	virtual QVariant data(const QModelIndex& _index, int role = Qt::DisplayRole) const;
	virtual bool setData(const QModelIndex& _index, const QVariant& value, int role);

	QString itemText(int row, int column) const;
	QString itemText(const QModelIndex& index) const;
	void setItemText(const QModelIndex& index, const QString& str);
	void setItemText(int row, int column, const QString& str);

	QVariant ItemData(const QModelIndex& index) const;
	QVariant ItemData(int row, int column) const;
	void SetItemData(const QModelIndex& index, const QVariant& data);
	void SetItemData(int row, int column, const QVariant& data);

	void clear();

signals:
	void itemChanged(const QModelIndex& index, const QVariant& value);


private:
	QStringList		m_hor_hedlbls;         // headerlabels
	QStringList		m_vec_hedlbls;         // oids - map.keys()
	QVariantMap		m_table_map, m_data_map;
};
