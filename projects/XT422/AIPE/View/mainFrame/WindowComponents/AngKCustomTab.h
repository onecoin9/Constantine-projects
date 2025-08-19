#pragma once
#ifndef ANGKCUSTOMTAB_H
#define ANGKCUSTOMTAB_H
#include <QProxyStyle>

class AngKCustomTab : public QProxyStyle
{
	Q_OBJECT

public:
	AngKCustomTab(QStyle* parent = nullptr, int width = 90, int height = 30);
	~AngKCustomTab();

	void drawControl(QStyle::ControlElement element, const QStyleOption* option,
		QPainter* painter, const QWidget* widget = nullptr) const;

	void drawItemText(QPainter* painter, const QRect& rect, int flags, const QPalette& pal,
		bool enabled, const QString& text, QPalette::ColorRole textRole = QPalette::NoRole) const;

	// 控制CE_TabBarTabLabel的尺寸
	QSize sizeFromContents(QStyle::ContentsType type, const QStyleOption* option,
		const QSize& contentsSize, const QWidget* widget = nullptr) const;

private:
	Qt::Orientation m_orientation;	// 文本方向
	int				m_nWidth;
	int				m_nHeight;
};

#endif // ANGKCUSTOMTAB_H