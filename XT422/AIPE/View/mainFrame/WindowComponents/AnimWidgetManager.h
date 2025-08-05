#pragma once

#include <QObject>
#include <QtWidgets/QWidget>
#include "GlobalDefine.h"
class AnimWidgetManager : public QObject
{
	Q_OBJECT

public:
	AnimWidgetManager(QWidget* parent = Q_NULLPTR);
	~AnimWidgetManager();

	//其他界面调用此接口来添加子窗口以用来管理
//添加id及widget
	void addWidget(QString id, QWidget* widget);

	//根据id放大收缩对应窗口
	void popupWidget(QString id, bool enable, AnimDirection nDir, int startPos, int endPos);

	//事件过滤器，为了实现父窗口放大缩小时子窗口跟着变化
	//让子窗口来处理父窗口的窗口大小变化事件
	virtual bool eventFilter(QObject* watched, QEvent* event);

private:
	QWidget* parent_;//需要继承父窗口，当前被管理类的子窗口的父窗口
	std::map<QString, QWidget*> id_widget_map_;//用来记录多个子窗口
	QString flag;//用来记录当前哪个窗口被打开
};
