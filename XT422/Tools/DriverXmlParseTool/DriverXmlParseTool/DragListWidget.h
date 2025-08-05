#pragma once

#include <QtWidgets/QApplication>
#include <QtCore/QDebug>
#include <QtGui/QDrag>
#include <QtWidgets/QListWidget>
#include <QtCore/QMimeData>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

class DragListWidget : public QListWidget
{
	Q_OBJECT

public:
	DragListWidget(QWidget *parent);
	~DragListWidget();

protected:
	virtual void mousePressEvent(QMouseEvent* event);

	virtual void mouseMoveEvent(QMouseEvent* event);

private:
	// �Ϸ����
	QPoint m_startPos;
	// ���Ϸŵ�item
	QListWidgetItem* m_sitem;
};
