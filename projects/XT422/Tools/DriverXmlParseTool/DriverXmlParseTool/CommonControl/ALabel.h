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

	int CalCursorCol(QPoint pt);    //�������X��λ�� 
	int CalCursorPos(QPoint pt, int colPos);    //��������λ��
	void setCursorShape(int CalPos);    //��������Ӧλ�õ���״

	int GetCursorPos();	//�༭����ȡ���λ�ã���ֹ�϶���Сʱ����

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
