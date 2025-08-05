#pragma once

#include <QtWidgets/QWidget>

class AngKRoundProgressBarEx : public QWidget
{
	Q_OBJECT

public:
	AngKRoundProgressBarEx(QWidget *parent);
	~AngKRoundProgressBarEx();
	//设置范围
	void setRange(int min, int max);

	//设置当前值
	bool setValue(int nValue);

	//设置最小值
	void setMinimum(int min);

	//设置最大值
	void setMaximum(int max);
protected:
	void paintEvent(QPaintEvent* event);

	//绘画进度条外圆
	void GradientArc(QPainter& painter, int nRadius, QRgb qColor);
	
	//绘画进度条开始和结束位置圆弧
	void DrawTwoSmallCircle(QPainter& painter, int nType);

	//绘画渐变圆
	void DrawGradientCircle(QPainter& painter, int radius);

	//渐变覆盖圆
	void GradientFullArc(QPainter& painter, QPointF& point, int radius, QRgb color);

	//设置当前角度
	void setCurrentAngle(int nAngle);

	//画中心文字
	void drawText(QPainter& p, const QRectF& innerRect, double value);
private:
	int m_min;						//最小值
	int m_max;						//最大值
	int m_value;					//设置值
	int m_startAngle;				//开始角度
	int m_endAngle;					//结束角度
	int m_ringWidth;				//进度圆弧宽度
	int m_maxRadius;				//进度圆弧外圆半径
	int m_currentAngle;				//当前角度
	QRgb m_circularBorderColor;		//进度条边框颜色
	QRgb m_insideMaskColor;			//进度条内遮罩颜色
	QRgb m_startGradientColor;		//开始渐变颜色
	QRgb m_endGradientColor;		//结束渐变颜色
};
