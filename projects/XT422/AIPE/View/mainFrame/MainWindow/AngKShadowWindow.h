#ifndef IF_ANGK_SHADOW_WINDOW_
#define IF_ANGK_SHADOW_WINDOW_
#pragma once

#include <QWidget>
#include <QPainter>
#include <QPaintEngine>
#include <QMouseEvent>
#include <QDebug>
#include <QBoxLayout>
#include <QTimer>
#include "AngKShadowWidget.h"
#include "AngKWindowTitleBar.h"

template <class Base>
class AngKShadowWindow : public Base
{
	//Q_OBJECT

public:
	AngKShadowWindow(QWidget* parent, bool canResize)
		: Base(parent)
		, m_shadowWidget(NULL)
		, m_clientWidget(NULL)
		, m_clientLayout(NULL)
	{
		Base* pT = this;
		//
		pT->setAttribute(Qt::WA_TranslucentBackground); //enable MainWindow to be transparent
		//pT->setWindowFlags(Qt::FramelessWindowHint);
		pT->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

		pT->setContentsMargins(0, 0, 0, 0);
		pT->setFocusPolicy(Qt::FocusPolicy::NoFocus);
		//
		QLayout* windowLayout = new QBoxLayout(QBoxLayout::TopToBottom);
		pT->setLayout(windowLayout);
		windowLayout->setContentsMargins(0, 0, 0, 0);
		windowLayout->setSpacing(0);
		//
		int shadowSize = AngKSmartScaleUI(20);
		m_shadowWidget = new AngKShadowWidget(this, shadowSize, canResize);
		m_shadowWidget->setContentsMargins(shadowSize, shadowSize, shadowSize, shadowSize);
		windowLayout->addWidget(m_shadowWidget);
		//
		QLayout* rootLayout = new QBoxLayout(QBoxLayout::TopToBottom);

		m_shadowWidget->setObjectName("ShadowWidget");
		m_shadowWidget->setLayout(rootLayout);
		rootLayout->setContentsMargins(0, 0, 0, 0);
		rootLayout->setSpacing(0);

		QWidget* shadowClientWidget = new QWidget(m_shadowWidget);
		shadowClientWidget->setFocusPolicy(Qt::FocusPolicy::NoFocus);
		shadowClientWidget->setObjectName("ShadowClientWidget");
		rootLayout->addWidget(shadowClientWidget);
		//
		QLayout* shadowClientLayout = new QBoxLayout(QBoxLayout::TopToBottom);
		shadowClientLayout->setObjectName("ShadowClientLayout");
		shadowClientLayout->setContentsMargins(0, 0, 0, 0);
		shadowClientLayout->setSpacing(0);
		shadowClientWidget->setLayout(shadowClientLayout);
		shadowClientWidget->setAutoFillBackground(true);
		shadowClientWidget->setCursor(QCursor(Qt::ArrowCursor));
		//
		m_titleBar = new AngKWindowTitleBar(shadowClientWidget, this, m_shadowWidget, canResize);
		m_titleBar->setObjectName("TitleBar");
		shadowClientLayout->addWidget(m_titleBar);
		//
		m_clientWidget = new QWidget(shadowClientWidget);
		m_clientWidget->setObjectName("ClientWidget");
		shadowClientLayout->addWidget(m_clientWidget);

		//
		m_clientLayout = new QBoxLayout(QBoxLayout::TopToBottom);
		m_clientLayout->setObjectName("ClientLayout");
		m_clientWidget->setLayout(m_clientLayout);
		//
		m_clientLayout->setSpacing(0);
		m_clientLayout->setContentsMargins(0, 0, 0, 0);
	}

	~AngKShadowWindow() {};

public:
	/// <summary>
	/// 阴影窗口指针，用于窗口拉伸
	/// </summary>
	/// <returns>阴影窗口指针</returns>
	QWidget* rootWidget() const { return m_shadowWidget; }
	/// <summary>
	/// 主窗口，整个主界面父类
	/// </summary>
	/// <returns>主窗口指针</returns>
	QWidget* clientWidget() const { return m_clientWidget; }
	QLayout* clientLayout() const { return m_clientLayout; }
	/// <summary>
	/// 窗口标题栏
	/// </summary>
	/// <returns>标题栏指针</returns>
	AngKWindowTitleBar* titleBar() const { return m_titleBar; }
	/// <summary>
	/// 设置标题栏名称
	/// </summary>
	/// <param name="title">标题栏名称</param>
	void setTitleText(QString title) { m_titleBar->setText(title); }
private:
	AngKShadowWidget* m_shadowWidget;
	QWidget* m_clientWidget;
	QLayout* m_clientLayout;
	AngKWindowTitleBar* m_titleBar;
protected:
	virtual void changeEvent(QEvent* event)
	{
		if (event->type() == QEvent::WindowStateChange)
		{
			m_titleBar->windowStateChanged();
		}
		
		Base::changeEvent(event);
	}
	
	virtual void layoutTitleBar()
	{
		m_titleBar->layoutTitleBar();
	}
};

#endif //IF_ANGK_SHADOW_WINDOW_