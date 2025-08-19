#include "AnimWidgetManager.h"
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QDebug>
AnimWidgetManager::AnimWidgetManager(QWidget* parent)
	: QObject(parent)
{
	parent_ = parent;
}

AnimWidgetManager::~AnimWidgetManager()
{
}

void AnimWidgetManager::addWidget(QString id, QWidget* widget)
{
	if (parent_ == nullptr)
		return;
	id_widget_map_[id] = widget;
	widget->setParent(parent_);
	//刚添加时最好先隐藏而不设置位置
	//因为在多个窗口加载时父窗口还会有变化，需要实时获取父窗口位置
	widget->hide();
}

void AnimWidgetManager::popupWidget(QString id, bool enable, AnimDirection nDir, int startPos, int endPos)
{
	QWidget* widget = id_widget_map_[id];
	if (widget == nullptr)
		return;

	QPropertyAnimation* animation = new QPropertyAnimation(widget, "pos");
	if (enable)
	{
		flag = id;
		switch (nDir)
		{
		case AnimDirection::Left:
			animation->setStartValue(QPoint(startPos, 0));
			animation->setEndValue(QPoint(endPos, 0));
			break;
		case AnimDirection::Top:
			animation->setStartValue(QPoint(0, startPos));
			animation->setEndValue(QPoint(0, endPos));
			break;
		case AnimDirection::Right:
			animation->setStartValue(QPoint(startPos, 0));
			animation->setEndValue(QPoint(0, 0));
			break;
		case AnimDirection::Bottom:
			animation->setStartValue(QPoint(0, startPos));
			animation->setEndValue(QPoint(0, 0));
			break;
		default:
			break;
		}
	}
	else
	{
		switch (nDir)
		{
		case AnimDirection::Left:
			animation->setStartValue(QPoint(startPos, 0));
			animation->setEndValue(QPoint(endPos, 0));
			break;
		case AnimDirection::Top:
			animation->setStartValue(QPoint(0, startPos));
			animation->setEndValue(QPoint(0, endPos));
			break;
		case AnimDirection::Right:
			animation->setStartValue(QPoint(0, 0));
			animation->setEndValue(QPoint(endPos, 0));
			break;
		case AnimDirection::Bottom:
			animation->setStartValue(QPoint(0, 0));
			animation->setEndValue(QPoint(0, endPos));
			break;
		default:
			break;
		}
	}
	animation->setEasingCurve(QEasingCurve::Linear);
	animation->start();
}

bool AnimWidgetManager::eventFilter(QObject* watched, QEvent* event)
{
	//这部分位置移动有很多坑，需要自己多调试注意，可灵活修改。
	if (event->type() == QEvent::Resize) {
		QResizeEvent* resize_event = (QResizeEvent*)event;
		QSize cursize = resize_event->size();
		for (auto a : id_widget_map_) {
			if (a.first == "AngKShowLogArea") {
				a.second->move(a.second->x(), a.second->y());
				a.second->setFixedWidth(parent_->width());
			}
			//else {
			//	//a.second->move(cursize.width(), a.second->y());
			//	a.second->move(a.second->x(), a.second->y());
			//	a.second->setFixedWidth(parent_->width());
			//}
		}
		return true;
	}
	return QObject::eventFilter(watched, event);
}
