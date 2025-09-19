#include "AngKItemDelegate.h"


AngKItemDelegate::AngKItemDelegate(QObject* parent)
	:QStyledItemDelegate(parent)
	, m_hovered_row(-1)
{

}

AngKItemDelegate::~AngKItemDelegate()
{
}

bool AngKItemDelegate::setHoverRow(int row)
{
	if (row != m_hovered_row)
	{
		m_hovered_row = row;
		return true;
	}
	return false;
}

int AngKItemDelegate::getHoverRow()
{
	return m_hovered_row;
}

void AngKItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QStyleOptionViewItem opt(option);
	opt.displayAlignment = Qt::AlignCenter;
	opt.state &= ~QStyle::State_HasFocus;
	if (index.row() == m_hovered_row)
	{
		opt.state |= QStyle::State_MouseOver;
	}
	else
	{
		opt.state &= ~QStyle::State_MouseOver;
	}

	QStyledItemDelegate::paint(painter, opt, index);
}
