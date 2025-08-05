#pragma once

#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtCore/QDebug>
#include <QtGui/QMouseEvent>
#include "../GlobalDefine.h"

class AComboBox : public QComboBox
{
	Q_OBJECT

public:
	AComboBox(QWidget *parent);
	~AComboBox();

	int CalCursorCol(QPoint pt);    //计算鼠标X的位置 
	int CalCursorPos(QPoint pt, int colPos);    //计算鼠标的位置
	void setCursorShape(int CalPos);    //设置鼠标对应位置的形状

	int GetCursorPos();	//编辑区获取鼠标位置，防止拖动大小时抖动

	void SetView(bool v);
signals:
	void sgnComboBox(QObject*);
	void sgnShowPropetry(QObject*);
	void sgnRClickComboBox(QObject*);

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
