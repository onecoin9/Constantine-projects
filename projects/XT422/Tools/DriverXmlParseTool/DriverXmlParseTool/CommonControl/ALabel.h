#pragma once

#include <QtWidgets/QLabel>
#include <QtCore/QDebug>
#include <QtGui/QMouseEvent>
#include "../GlobalDefine.h"
class ALabel : public QLabel
{
	Q_OBJECT

public:
	ALabel(QWidget* parent);
	~ALabel();

	int CalCursorCol(QPoint pt);    //计算鼠标X的位置 
	int CalCursorPos(QPoint pt, int colPos);    //计算鼠标的位置
	void setCursorShape(int CalPos);    //设置鼠标对应位置的形状

	int GetCursorPos();	//编辑区获取鼠标位置，防止拖动大小时抖动

	void SetView(bool v);
signals:
	void sgnLabel(QObject*);
	void sgnRClickLabel(QObject*);
	void sgnShowPropetry(QObject*);

protected:
	virtual void mousePressEvent(QMouseEvent* event);

	virtual void mouseMoveEvent(QMouseEvent* event);

	virtual void mouseReleaseEvent(QMouseEvent* event);

private:
	int			m_iCalCursorPos;
	bool		m_bLeftPress;
	QRect		m_rtPreGeometry;
	QPoint		m_ptViewMousePos;
	bool		m_bView;
};
