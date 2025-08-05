#include "AngKProgressBar.h"
#include "AngKGlobalInstance.h"
#include "GlobalDefine.h"
#include <QPainter>
#include <QtMath>
#include <QPoint>

AngKProgressBar::AngKProgressBar(QWidget *parent)
	: QWidget(parent)
	, m_nInsidePadding(2.0)
	, m_min(0)
	, m_max(100)
	, m_value(80)
{
}

AngKProgressBar::~AngKProgressBar()
{
}

void AngKProgressBar::setInsidePadding(double valueInside)
{
	m_nInsidePadding = valueInside;
	update();
}

void AngKProgressBar::setRange(int min, int max)
{
	m_min = min;
	m_max = max;

	if (m_max == m_min)
	{
		m_min = 0;
		m_max = 1;
	}

	if (m_max < m_min)
	{
		qSwap(m_max, m_min);
	}

	if (m_value < m_min)
	{
		m_value = m_min;
	}
	else if (m_value > m_max)
	{
		m_value = m_max;
	}

	update();
}

void AngKProgressBar::setValue(int nValue)
{
	setValue(double(nValue));
}

void AngKProgressBar::setValue(double val)
{
	if (m_value != val)
	{
		if (val < m_min)
		{
			m_value = m_min;
		}
		else if (val > m_max)
		{
			m_value = m_max;
		}
		else
		{
			m_value = val;
		}
		update();
	}
}

void AngKProgressBar::setMinimum(int min)
{
	setRange(min, m_max);
}

void AngKProgressBar::setMaximum(int max)
{
	setRange(m_min, max);
}

void AngKProgressBar::paintEvent(QPaintEvent* event)
{
	QPainter p(this);

	double dwidth = width() - 2.0;
	double dheight = height() - 2.0;

	QRectF rect = QRectF(0, 0, dwidth, dheight);
	QRectF insideRect = QRectF(m_nInsidePadding, m_nInsidePadding, dwidth - m_nInsidePadding * 2, dheight - m_nInsidePadding * 2);

	const double k = (double)(m_value - m_min) / (m_max - m_min);
	int x = (int)(insideRect.width() * k);
	QRectF modifyRect = insideRect.adjusted(0, 0, x - insideRect.width(), 0);

	//画进度条
	p.setPen(Qt::NoPen);
	bool bDark = AngKGlobalInstance::ReadValue("Skin", "mode").toInt() == (int)ViewMode::Dark ? true : false;
	QColor skinColor;
	bDark ? skinColor = QColor(50, 50, 50) : skinColor = QColor(204, 204, 204);
	p.setBrush(QBrush(skinColor));
	p.drawRoundedRect(rect, 6, 6);

	QLinearGradient gradient(QPoint(modifyRect.x(), modifyRect.y()), QPoint(modifyRect.x() + modifyRect.width(), modifyRect.y() + modifyRect.height()));
	gradient.setColorAt(0, QColor(193, 216, 227));
	gradient.setColorAt(1.0, QColor(39, 205, 253));
	gradient.setSpread(QGradient::PadSpread);
	p.setBrush(gradient);
	p.drawRoundedRect(modifyRect, 6, 6);


}
