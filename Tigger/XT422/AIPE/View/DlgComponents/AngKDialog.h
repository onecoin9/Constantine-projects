#pragma once

#include <QtWidgets/QDialog>
namespace Ui { class AngKDialog; };

class AngKDialog : public QDialog
{
	Q_OBJECT

public:
	AngKDialog(QWidget *parent = Q_NULLPTR);
	~AngKDialog();

	QWidget* setCentralWidget();

	void SetTitle(QString title);
protected:
	virtual void mousePressEvent(QMouseEvent* event);

	virtual void mouseMoveEvent(QMouseEvent* event);

	virtual void mouseReleaseEvent(QMouseEvent* event);
signals:
	void sgnClose();
private:
	Ui::AngKDialog *ui;
	QPoint	clickPos;
	int		m_nDraggableHeight;   // 可拖动区域的高度
	bool	m_bDragging; // 拖动状态标志
};
