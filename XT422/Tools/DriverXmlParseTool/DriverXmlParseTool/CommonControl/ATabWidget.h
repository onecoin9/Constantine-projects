#pragma once

#include <QtWidgets/QTabWidget>
#include <QtCore/QDebug>
#include <QtGui/QEvent.h>
#include "../GlobalDefine.h"

class ATabWidget : public QTabWidget
{
	Q_OBJECT

public:
	ATabWidget(QWidget *parent);
	~ATabWidget();

	int CalCursorCol(QPoint pt);    //�������X��λ�� 
	int CalCursorPos(QPoint pt, int colPos);    //��������λ��
	void setCursorShape(int CalPos);    //��������Ӧλ�õ���״

	int GetCursorPos();	//�༭����ȡ���λ�ã���ֹ�϶���Сʱ����

	void SetView(bool v);
signals:
	void sgnTabWidget(QObject*);
	void sgnShowPropetry(QObject*);
	void sgnRClickTabWidget(QObject*);

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
