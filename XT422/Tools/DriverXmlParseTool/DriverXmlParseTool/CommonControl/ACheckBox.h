#pragma once

#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtCore/QDebug>
#include <QtGui/QEvent.h>
#include "../GlobalDefine.h"

class ACheckBox : public QCheckBox
{
	Q_OBJECT

public:
	ACheckBox(QWidget *parent);
	~ACheckBox();

	int CalCursorCol(QPoint pt);    //�������X��λ�� 
	int CalCursorPos(QPoint pt, int colPos);    //��������λ��
	void setCursorShape(int CalPos);    //��������Ӧλ�õ���״

	int GetCursorPos();	//�༭����ȡ���λ�ã���ֹ�϶���Сʱ����

	void SetView(bool v);
signals:
	void sgnCheckBox(QObject*);
	void sgnShowPropetry(QObject*);
	void sgnRClickCheck(QObject*);

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
