#pragma once

#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtCore/QDebug>
#include <QtGui/QDrag>
#include <QtCore/QMimeData>
#include <QtGui/QMouseEvent>
#include "../GlobalDefine.h"

class AGroupBox : public QGroupBox
{
	Q_OBJECT

public:
	AGroupBox(QWidget *parent);
	~AGroupBox();

	void setName(QString str) { m_strName = str; }

	int CalCursorCol(QPoint pt);    //�������X��λ�� 
	int CalCursorPos(QPoint pt, int colPos);    //��������λ��
	void setCursorShape(int CalPos);    //��������Ӧλ�õ���״

	int GetCursorPos();	//�༭����ȡ���λ�ã���ֹ�϶���Сʱ����

	void SetView(bool v);
signals:
	void sgnGroupBox(QObject*);
	void sgnShowPropetry(QObject*);
	void sgnRClickGroupBox(QObject*);

protected:
	virtual void mousePressEvent(QMouseEvent* event);

	virtual void mouseMoveEvent(QMouseEvent* event);

	virtual void mouseReleaseEvent(QMouseEvent* event);
private:
	QString		m_strName;
	int			m_iCalCursorPos;
	bool		m_bLeftPress;
	QRect		m_rtPreGeometry;
	QPoint		m_ptViewMousePos;
	bool		m_bView;
};
