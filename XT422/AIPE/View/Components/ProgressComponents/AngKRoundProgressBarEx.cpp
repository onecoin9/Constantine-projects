#include "AngKRoundProgressBarEx.h"
#include "AngKGlobalInstance.h"
#include "GlobalDefine.h"
#include <QPainter>
#include <QPointF>
#include <QtMath>
#include <QPainterPath>

AngKRoundProgressBarEx::AngKRoundProgressBarEx(QWidget* parent)
	: QWidget(parent)
	, m_min(0)
	, m_max(100)
	, m_value(0)
	, m_startAngle(-45)
	, m_endAngle(225)
	, m_ringWidth(10)
	, m_maxRadius(50)
	, m_currentAngle(270)
	, m_circularBorderColor(qRgb(204, 204, 204))
	, m_insideMaskColor(qRgb(247, 247, 247))
	, m_startGradientColor(qRgb(182, 219, 206))
	, m_endGradientColor(qRgb(40, 241, 157))
{
	bool bDark = AngKGlobalInstance::ReadValue("Skin", "mode").toInt() == (int)ViewMode::Dark ? true : false;
	bDark ? m_circularBorderColor = QRgb(qRgb(57, 57, 57)) : m_circularBorderColor = QRgb(qRgb(204, 204, 204));
	bDark ? m_insideMaskColor = QRgb(qRgb(37, 37, 37)) : m_insideMaskColor = QRgb(qRgb(247, 247, 247));
}

AngKRoundProgressBarEx::~AngKRoundProgressBarEx()
{
}

void AngKRoundProgressBarEx::setRange(int min, int max)
{
	if (min < 0)
		min = 0;

	if (max < 0)
		max = 0;

	m_min = min;
	m_max = max;

	if (m_max < m_min)
	{
		qSwap(m_max, m_min);
	}


	update();
}

void AngKRoundProgressBarEx::setCurrentAngle(int nAngle)
{
	if (nAngle != m_startAngle)
	{
		m_currentAngle = nAngle;
	}
	update();
}

bool AngKRoundProgressBarEx::setValue(int nValue)
{
	if (m_max - m_min <= 0)
		return false;

	m_value = nValue;
	m_currentAngle = 270 * (1 - (float)nValue / (m_max - m_min));
	update();

	return true;
}

void AngKRoundProgressBarEx::setMinimum(int min)
{
	setRange(min, m_max);
}

void AngKRoundProgressBarEx::setMaximum(int max)
{
	setRange(m_min, max);
}

void AngKRoundProgressBarEx::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	// 右移1位 相当于width()/2
	painter.translate(width() >> 1, height() >> 1);
	//启动反锯齿
	painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

	GradientArc(painter, m_maxRadius, m_circularBorderColor);

	//DrawTwoSmallCircle(painter, 0);

	GradientArc(painter, m_maxRadius - 1, m_circularBorderColor);

	//DrawTwoSmallCircle(painter, 1);

	DrawGradientCircle(painter, m_maxRadius - 1);

	GradientArc(painter, m_maxRadius - m_ringWidth + 1, m_circularBorderColor);

	QPointF point(0, 0);
	GradientFullArc(painter, point, m_maxRadius - m_ringWidth, m_insideMaskColor);

	//初始坐标求如下，构建等腰直角三角形，求y轴。长宽根据UI固定67，可自行修改
	double nRect = qSqrt(qPow(m_maxRadius, 2) / (double)2);
	QRectF baseRect(-nRect, -nRect, 67, 67);
	drawText(painter, baseRect, m_value);
}

void AngKRoundProgressBarEx::GradientArc(QPainter& painter, int nRadius, QRgb qColor)
{
	QRadialGradient gradient(0, 0, nRadius);
	gradient.setColorAt(0, qColor);
	gradient.setColorAt(1.0, qColor);
	painter.setBrush(gradient);
	QRectF rect(-nRadius, -nRadius, nRadius << 1, nRadius << 1);
	QPainterPath path;
	path.arcTo(rect, m_startAngle, m_endAngle - m_startAngle);
	painter.setPen(Qt::NoPen);
	painter.drawPath(path);
}

void AngKRoundProgressBarEx::DrawTwoSmallCircle(QPainter& painter, int nType)
{
	//计算小圆坐标
	QPoint rightCircle(0, 0);
	QPoint leftCircle(0, 0);
	rightCircle.setY(-(qSin(m_startAngle * M_PI / 180) * (m_maxRadius - m_ringWidth / 2)));
	rightCircle.setX(qCos(m_startAngle * M_PI / 180) * (m_maxRadius - m_ringWidth / 2) + 1);
	leftCircle.setX(-rightCircle.rx());
	leftCircle.setY(rightCircle.ry());

	if (nType == 0) {
		painter.drawEllipse(rightCircle, m_ringWidth / 2, m_ringWidth / 2);
		painter.drawEllipse(leftCircle, m_ringWidth / 2, m_ringWidth / 2);
	}
	else {
		painter.drawEllipse(rightCircle, m_ringWidth / 2 - 1, m_ringWidth / 2 - 1);
		painter.drawEllipse(leftCircle, m_ringWidth / 2 - 1, m_ringWidth / 2 - 1);
	}
}

void AngKRoundProgressBarEx::DrawGradientCircle(QPainter& painter, int radius)
{
	QConicalGradient conicalGradient(QPointF(0, 0), m_endAngle + 1);
	conicalGradient.setColorAt((360 - qAbs(m_endAngle - m_startAngle) + m_currentAngle) / 360.0, m_endGradientColor);
	conicalGradient.setColorAt(1, m_startGradientColor);
	painter.setBrush(conicalGradient);
	QPainterPath path;
	QRectF rect(-radius, -radius, radius << 1, radius << 1);
	path.arcTo(rect, m_startAngle + m_currentAngle, m_endAngle - m_startAngle - m_currentAngle);
	painter.setPen(Qt::NoPen);
	painter.drawPath(path);
}

void AngKRoundProgressBarEx::GradientFullArc(QPainter& painter, QPointF& point, int radius, QRgb color)
{
	//渐变色
	QRadialGradient gradient(0, 0, radius);
	gradient.setColorAt(0, color);
	gradient.setColorAt(1.0, color);
	painter.setBrush(gradient);
	painter.drawEllipse(point, radius, radius);
}



void AngKRoundProgressBarEx::drawText(QPainter& p, const QRectF& innerRect, double value)
{
	QFont f;
	f.setFamily("HarmonyOS Sans SC");
	f.setPixelSize(40);

	p.setFont(f);
	bool bDark = AngKGlobalInstance::ReadValue("Skin", "mode").toInt() == (int)ViewMode::Dark ? true : false;
	QColor skinColor;
	bDark ? skinColor = QColor(227, 227, 227) : skinColor = QColor(95, 99, 104);
	p.setPen(skinColor);

	p.drawText(innerRect, Qt::AlignCenter, QString::number(value));
}