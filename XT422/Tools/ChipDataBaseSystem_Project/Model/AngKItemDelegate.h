#pragma once

#include <QtWidgets/QStyledItemDelegate>
#include <QTableView>
class AngKItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	explicit AngKItemDelegate(QObject* parent = nullptr);
	~AngKItemDelegate();
	bool setHoverRow(int row);
	int  getHoverRow();

protected:
	void paint(QPainter* painter, const QStyleOptionViewItem& option,
		const QModelIndex& index) const;

private:
	int m_hovered_row;
};
