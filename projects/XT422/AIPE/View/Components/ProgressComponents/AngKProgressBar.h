#pragma once

#include <QtWidgets/QWidget>

class AngKProgressBar : public QWidget
{
	Q_OBJECT

public:
	AngKProgressBar(QWidget *parent);
	~AngKProgressBar();

	void setInsidePadding(double valueInside);

	//设置范围
	void setRange(int min, int max);

	//设置当前值
	void setValue(int nValue);

	//设置当前值
	void setValue(double val);

	//设置最小值
	void setMinimum(int min);

	//设置最大值
	void setMaximum(int max);

	//获取当前值
	int getValue() { return m_value; }
protected:
	void paintEvent(QPaintEvent* event);

private:
	int m_min;					//最小值
	int m_max;					//最大值
	int m_value;				//设置值
	double	m_nInsidePadding;	//内边距值
};
