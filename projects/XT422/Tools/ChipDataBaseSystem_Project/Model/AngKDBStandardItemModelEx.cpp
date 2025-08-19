#include "AngKDBStandardItemModelEx.h"

AngKDBStandardItemModelEx::AngKDBStandardItemModelEx(QObject *parent)
	: QAbstractTableModel(parent)
{
}

AngKDBStandardItemModelEx::~AngKDBStandardItemModelEx()
{
}

void AngKDBStandardItemModelEx::SetData(const QVariantMap& map)
{
	beginResetModel();
	m_hor_hedlbls = map["headerlabel"].toStringList();
	m_table_map = map;
	m_vec_hedlbls = map.keys();
	endResetModel();
}

Qt::ItemFlags AngKDBStandardItemModelEx::flags(const QModelIndex& index) const
{
	Qt::ItemFlags flags = QAbstractItemModel::flags(index);
	flags |= Qt::ItemIsEditable;
	return flags;
}

QVariant AngKDBStandardItemModelEx::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal)
	{
		if (role == Qt::DisplayRole)    
			return m_hor_hedlbls.at(section);
		else    
			return QVariant();
	}

	return QAbstractTableModel::headerData(section, orientation, role);
}

int AngKDBStandardItemModelEx::rowCount(const QModelIndex& parent) const
{
	if (parent.isValid())   
		return 0;
	else    
		return m_table_map.size() - 1;
}

int AngKDBStandardItemModelEx::columnCount(const QModelIndex& parent) const
{
	if (parent.isValid())
		return 0;
	else    
		return m_hor_hedlbls.size();
}

QVariant AngKDBStandardItemModelEx::data(const QModelIndex& _index, int role) const
{

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		QVariantMap* _map = (QVariantMap*)(m_table_map[m_vec_hedlbls[_index.row()]].data());
		return (*_map)[m_hor_hedlbls[_index.column()]];
	}
	else if (role == Qt::TextAlignmentRole)
	{
		return Qt::AlignCenter; // 文字居中
	}

	return QVariant();
}

bool AngKDBStandardItemModelEx::setData(const QModelIndex& _index, const QVariant& value, int role)
{
	if (_index.isValid() && role == Qt::EditRole)
	{
		QVariantMap* _map = (QVariantMap*)(m_table_map[m_vec_hedlbls[_index.row()]].data());
		if ((*_map)[m_hor_hedlbls[_index.column()]] != value)
		{
			(*_map)[m_hor_hedlbls[_index.column()]] = value;
			emit dataChanged(_index, _index);
			emit itemChanged(_index, value);

			return true;
		}
	}
	return false;
}


QString AngKDBStandardItemModelEx::itemText(int row, int column) const
{
	QVariantMap* _map = (QVariantMap*)(m_table_map[m_vec_hedlbls[row]].data());
	return (*_map)[m_hor_hedlbls[column]].toString();
}

QString AngKDBStandardItemModelEx::itemText(const QModelIndex& index) const
{
	QVariantMap* _map = (QVariantMap*)(m_table_map[m_vec_hedlbls[index.row()]].data());
	return (*_map)[m_hor_hedlbls[index.column()]].toString();
}

void AngKDBStandardItemModelEx::setItemText(const QModelIndex& index, const QString& str)
{
	QVariantMap* _map = (QVariantMap*)(m_table_map[m_vec_hedlbls[index.row()]].data());
	(*_map)[m_hor_hedlbls[index.column()]] = str;
}

void AngKDBStandardItemModelEx::setItemText(int row, int column, const QString& str)
{
	QVariantMap* _map = (QVariantMap*)(m_table_map[m_vec_hedlbls[row]].data());
	(*_map)[m_hor_hedlbls[column]] = str;
}

QVariant AngKDBStandardItemModelEx::ItemData(const QModelIndex& index) const
{
	QVariantMap* _map = (QVariantMap*)(m_data_map[m_vec_hedlbls[index.row()]].data());
	return (*_map)[m_hor_hedlbls[index.column()]];
}

QVariant AngKDBStandardItemModelEx::ItemData(int row, int column) const
{
	QVariantMap* _map = (QVariantMap*)(m_data_map[m_vec_hedlbls[row]].data());
	return (*_map)[m_hor_hedlbls[column]];
}

void AngKDBStandardItemModelEx::SetItemData(const QModelIndex& index, const QVariant& data)
{
	QString vec_lbl = m_vec_hedlbls[index.row()];
	if (!m_data_map.contains(vec_lbl))   m_data_map[vec_lbl] = QVariantMap();

	QVariantMap* _map = (QVariantMap*)(m_data_map[vec_lbl].data());
	(*_map)[m_hor_hedlbls[index.column()]] = data;
}

void AngKDBStandardItemModelEx::SetItemData(int row, int column, const QVariant& data)
{
	QString vec_lbl = m_vec_hedlbls[row];
	if (!m_data_map.contains(vec_lbl))   m_data_map[vec_lbl] = QVariantMap();

	QVariantMap* _map = (QVariantMap*)(m_data_map[vec_lbl].data());
	(*_map)[m_hor_hedlbls[column]] = data;
}

void AngKDBStandardItemModelEx::clear()
{
	m_table_map.clear();
	m_data_map.clear();
	m_hor_hedlbls.clear();
	m_vec_hedlbls.clear();
}
