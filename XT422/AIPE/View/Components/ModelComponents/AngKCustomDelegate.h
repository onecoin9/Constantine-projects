#pragma once

#include <QStyledItemDelegate>

class AngKCustomDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	AngKCustomDelegate(QObject *parent = nullptr);
	~AngKCustomDelegate();

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

	void setEditorData(QWidget* editor, const QModelIndex& index) const override;

	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

	void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

	bool editorEvent(QEvent* event, QAbstractItemModel* model,
		const QStyleOptionViewItem& option, const QModelIndex& index) override;

	void setCheckBoxColumn(int _column);

	void setEditColumn(int _column);

	void setCheckEnable(bool _check);

	void setExtCSDFlag(bool bFlag);
private:
	int checkboxColumn;
	int editColumn;
	bool bCheck;
	bool extFlag;
};
