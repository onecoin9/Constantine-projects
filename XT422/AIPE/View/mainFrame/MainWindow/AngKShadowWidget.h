#ifndef ANGK_SKIN_GRID_IMAGE_
#define ANGK_SKIN_GRID_IMAGE_
#pragma once

#include <QGraphicsEffect>
#include <QWidget>

class AngKSkinGridImage;

enum AngKWindowHitTestResult { TopLeft, Top, TopRight, Left, angkClient, Right, BottomLeft, Bottom, BottomRight };

class AngKShadowWidget : public QWidget
{
	//Q_OBJECT

public:
	AngKShadowWidget(QWidget* parent, int shadowSize, bool canResize);
	~AngKShadowWidget();

private:
	AngKSkinGridImage* m_shadow;
	int m_shadowSize;
	bool m_canResize;
	//
	AngKWindowHitTestResult m_oldHitCode;
	QPoint m_oldPressPos;
	QRect m_oldGeometry;
	bool m_mousePressed;
protected:
	void paintEvent(QPaintEvent*) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	//
	virtual AngKWindowHitTestResult hitTest(const QPoint& posOfWindow);
public:
	bool canResize() const { return m_canResize; }
	void setCanResize(bool b) { m_canResize = b; }
};

#endif	//ANGK_SKIN_GRID_IMAGE_