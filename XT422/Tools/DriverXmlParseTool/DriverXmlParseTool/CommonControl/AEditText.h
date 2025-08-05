#pragma once

#include <QtWidgets/QTextEdit>
#include <QtCore/QDebug>
#include <QtGui/QEvent.h>
#include "../GlobalDefine.h"

class AEditText : public QTextEdit
{
	Q_OBJECT

public:
	AEditText(QWidget *parent);
	~AEditText();

	int CalCursorCol(QPoint pt);    //�������X��λ�� 
	int CalCursorPos(QPoint pt, int colPos);    //��������λ��
	void setCursorShape(int CalPos);    //��������Ӧλ�õ���״

	int GetCursorPos();	//�༭����ȡ���λ�ã���ֹ�϶���Сʱ����

	void SetView(bool v);
signals:
	void sgnEditText(QObject*);
	void sgnRClickEdit(QObject*);
	void sgnShowPropetry(QObject*);

protected:
	virtual void mousePressEvent(QMouseEvent* event);

	virtual void mouseDoubleClickEvent(QMouseEvent* e);

	virtual void mouseMoveEvent(QMouseEvent* event);

	virtual void mouseReleaseEvent(QMouseEvent* event);

private:
	int			m_iCalCursorPos;
	bool		m_bLeftPress;
	QRect		m_rtPreGeometry;
	QPoint		m_ptViewMousePos;
	bool		m_bView;
};
