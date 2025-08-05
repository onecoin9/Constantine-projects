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

	int CalCursorCol(QPoint pt);    //�������X��λ�� 
	int CalCursorPos(QPoint pt, int colPos);    //��������λ��
	void setCursorShape(int CalPos);    //��������Ӧλ�õ���״

	int GetCursorPos();	//�༭����ȡ���λ�ã���ֹ�϶���Сʱ����

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
