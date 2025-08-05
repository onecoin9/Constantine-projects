#pragma once

#include <QStyledItemDelegate>

enum EItemValue { eIV_Mininum, eIV_Maxinum, eIV_Value, eIV_ProgressType };

class AngKProgressDelegate : public QStyledItemDelegate
{
	Q_OBJECT


public:
	AngKProgressDelegate(QObject *parent = nullptr);
	~AngKProgressDelegate();

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

	void setProgressIndex(int nProgress);

	void UpdateProgressColor(QColor progColor);
private:
	int nProgressIdx;
	QColor m_color;
};
