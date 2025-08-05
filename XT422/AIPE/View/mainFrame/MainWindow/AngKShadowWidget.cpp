#include "AngKShadowWidget.h"
#include "AngKSkinGridImage.h"
#include "AngKPathResolve.h"
#include <QPainter>
#include <QPaintEngine>
#include <QMouseEvent>
#include <math.h>
inline unsigned char AngKMakeAlpha(int i, double f, int nSize)
{
	if (i == nSize)
		f *= 1.2;
	//

	double f2 = 1 - cos((double)i / nSize * 3.14 / 2);
	//
	return int(fabs((i * f) * f2));
}

QImage AngKMakeShadowImage(int shadowSize, bool activated)
{
	int size = shadowSize * 2 + 10;
	QImage image(size, size, QImage::Format_ARGB32);
	image.fill(QColor(Qt::black));
	//
	double f = activated ? 1.6 : 1.0;
	//
	QSize szImage = image.size();
	//
	//left
	for (int y = shadowSize; y < szImage.height() - shadowSize; y++)
	{
		for (int x = 0; x < shadowSize; x++)
		{
			int i = x + 1;
			int alpha = AngKMakeAlpha(i, f, shadowSize);
			image.setPixelColor(x, y, QColor(0, 0, 0, alpha));
		}
	}
	//right
	for (int y = shadowSize; y < szImage.height() - shadowSize; y++)
	{
		for (int x = szImage.width() - shadowSize - 1; x < szImage.width(); x++)
		{
			int i = szImage.width() - x;
			int alpha = AngKMakeAlpha(i, f, shadowSize);
			image.setPixelColor(x, y, QColor(0, 0, 0, alpha));
		}
	}
	//top
	for (int y = 0; y < shadowSize; y++)
	{
		int i = y + 1;
		for (int x = shadowSize; x < szImage.width() - shadowSize; x++)
		{
			int alpha = AngKMakeAlpha(i, f, shadowSize);
			image.setPixelColor(x, y, QColor(0, 0, 0, alpha));
		}
		//
	}
	//bottom
	for (int y = szImage.height() - shadowSize - 1; y < szImage.height(); y++)
	{
		int i = szImage.height() - y;
		for (int x = shadowSize; x < szImage.width() - shadowSize; x++)
		{
			int alpha = AngKMakeAlpha(i, f, shadowSize);
			image.setPixelColor(x, y, QColor(0, 0, 0, alpha));
		}
	}
	//
	int parentRoundSize = 3;
	//
	for (int x = 0; x < shadowSize + parentRoundSize; x++)
	{
		for (int y = 0; y < shadowSize + parentRoundSize; y++)
		{
			int xx = (shadowSize + parentRoundSize) - x;
			int yy = (shadowSize + parentRoundSize) - y;
			int i = int(sqrt(double(xx * xx + yy * yy)));
			i = std::min<int>(shadowSize + parentRoundSize, i);
			i -= parentRoundSize;
			i = shadowSize - i;
			//
			int alpha = AngKMakeAlpha(i, f, shadowSize);
			image.setPixelColor(x, y, QColor(0, 0, 0, alpha));
		}
	}
	//
	for (int x = szImage.width() - shadowSize - parentRoundSize; x < szImage.width(); x++)
	{
		for (int y = 0; y < shadowSize + parentRoundSize; y++)
		{
			int xx = (shadowSize + parentRoundSize) - (szImage.width() - x);
			int yy = (shadowSize + parentRoundSize) - y;
			int i = int(sqrt(double(xx * xx + yy * yy)));
			i = std::min<int>(shadowSize + parentRoundSize, i);
			i -= parentRoundSize;
			i = shadowSize - i;
			//
			int alpha = AngKMakeAlpha(i, f, shadowSize);
			image.setPixelColor(x, y, QColor(0, 0, 0, alpha));
		}
	}
	//
	for (int x = 0; x < shadowSize + parentRoundSize; x++)
	{
		for (int y = szImage.height() - shadowSize - parentRoundSize; y < szImage.height(); y++)
		{
			int xx = (shadowSize + parentRoundSize) - x;
			int yy = (shadowSize + parentRoundSize) - (szImage.height() - y);
			int i = int(sqrt(double(xx * xx + yy * yy)));
			i = std::min<int>(shadowSize + parentRoundSize, i);
			i -= parentRoundSize;
			i = shadowSize - i;
			//
			int alpha = AngKMakeAlpha(i, f, shadowSize);
			image.setPixelColor(x, y, QColor(0, 0, 0, alpha));
		}
	}
	//
	for (int x = szImage.width() - shadowSize - parentRoundSize; x < szImage.width(); x++)
	{
		for (int y = szImage.height() - shadowSize - parentRoundSize; y < szImage.height(); y++)
		{
			int xx = (shadowSize + parentRoundSize) - (szImage.width() - x);
			int yy = (shadowSize + parentRoundSize) - (szImage.height() - y);
			int i = int(sqrt(double(xx * xx + yy * yy)));
			i = std::min<int>(shadowSize + parentRoundSize, i);
			i -= parentRoundSize;
			i = shadowSize - i;
			//
			int alpha = AngKMakeAlpha(i, f, shadowSize);
			image.setPixelColor(x, y, QColor(0, 0, 0, alpha));
		}
	}
	//
	int borderR = 165;
	int borderG = 165;
	int borderB = 165;
	//
	if (activated)
	{
		borderR = 68;
		borderG = 138;
		borderB = 255;
	}
	//
	int borderSize = ::AngKSmartScaleUI(1);
	//left
	for (int i = 0; i < borderSize; i++)
	{
		for (int y = shadowSize - 1; y < szImage.height() - shadowSize + 1; y++)
		{
			int x = shadowSize - i - 1;
			image.setPixelColor(x, y, QColor(borderR, borderG, borderB, 255));
		}
	}
	//right
	for (int i = 0; i < borderSize; i++)
	{
		for (int y = shadowSize - 1; y < szImage.height() - shadowSize + 1; y++)
		{
			int x = szImage.width() - shadowSize - 1 + i + 1;
			image.setPixelColor(x, y, QColor(borderR, borderG, borderB, 255));
		}
	}
	//top
	for (int i = 0; i < borderSize; i++)
	{
		for (int x = shadowSize; x < szImage.width() - shadowSize; x++)
		{
			int y = shadowSize - i - 1;
			image.setPixelColor(x, y, QColor(borderR, borderG, borderB, 255));
		}
	}
	//bottom
	for (int i = 0; i < borderSize; i++)
	{
		for (int x = shadowSize; x < szImage.width() - shadowSize; x++)
		{
			int y = szImage.height() - shadowSize - 1 + i + 1;
			image.setPixelColor(x, y, QColor(borderR, borderG, borderB, 255));
		}
	}
	//
	return image;
}

AngKShadowWidget::AngKShadowWidget(QWidget* parent, int shadowSize, bool canResize)
	: QWidget(parent)
	, m_shadow(new AngKSkinGridImage())
	, m_shadowSize(shadowSize)
	, m_canResize(canResize)
	, m_oldHitCode(angkClient)
	, m_mousePressed(false)
{
	if (m_canResize)
	{
		setMouseTracking(true);
	}
	//
	QImage image = AngKMakeShadowImage(shadowSize, true);
	m_shadow->setImage(image, QPoint(shadowSize + 1, shadowSize + 1), "#666666");
	setFocusPolicy(Qt::FocusPolicy::NoFocus);
}

AngKShadowWidget::~AngKShadowWidget()
{
	if (m_shadow)
	{
		m_shadow = nullptr;
		delete m_shadow;
	}
}

void AngKShadowWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	m_shadow->drawBorder(&painter, rect());
}

void AngKShadowWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (m_mousePressed)
	{
		if (m_oldHitCode == angkClient)
			return;
		//
		QPoint pos = event->globalPos();
		int offsetX = pos.x() - m_oldPressPos.x();
		int offsetY = pos.y() - m_oldPressPos.y();
		//
		QRect rc = m_oldGeometry;
		//
		switch (m_oldHitCode)
		{
		case TopLeft:
			rc.adjust(offsetX, offsetY, 0, 0);
			break;
		case Top:
			rc.adjust(0, offsetY, 0, 0);
			break;
		case TopRight:
			rc.adjust(0, offsetY, offsetX, 0);
			break;
		case Left:
			rc.adjust(offsetX, 0, 0, 0);
			break;
		case Right:
			rc.adjust(0, 0, offsetX, 0);
			break;
		case BottomLeft:
			rc.adjust(offsetX, 0, 0, offsetY);
			break;
		case Bottom:
			rc.adjust(0, 0, 0, offsetY);
			break;
		case BottomRight:
			rc.adjust(0, 0, offsetX, offsetY);
			break;
		default:
			Q_ASSERT(false);
			break;
		}
		//
		parentWidget()->setGeometry(rc);
	}
	else
	{
		QPoint pos = event->pos();
		AngKWindowHitTestResult hit = hitTest(pos);
		if (hit != angkClient)
		{
			event->accept();
		}
		//
		switch (hit)
		{
		case TopLeft:
			setCursor(QCursor(Qt::SizeFDiagCursor));
			break;
		case Top:
			setCursor(QCursor(Qt::SizeVerCursor));
			break;
		case TopRight:
			setCursor(QCursor(Qt::SizeBDiagCursor));
			break;
		case Left:
			setCursor(QCursor(Qt::SizeHorCursor));
			break;
		case angkClient:
			setCursor(QCursor(Qt::ArrowCursor));
			break;
		case Right:
			setCursor(QCursor(Qt::SizeHorCursor));
			break;
		case BottomLeft:
			setCursor(QCursor(Qt::SizeBDiagCursor));
			break;
		case Bottom:
			setCursor(QCursor(Qt::SizeVerCursor));
			break;
		case BottomRight:
			setCursor(QCursor(Qt::SizeFDiagCursor));
			break;
		}
	}
}

void AngKShadowWidget::mousePressEvent(QMouseEvent* event)
{
	QPoint pos = event->pos();
	AngKWindowHitTestResult hit = hitTest(pos);
	//
	m_oldHitCode = hit;
	m_oldPressPos = event->globalPos();
	m_mousePressed = true;
	m_oldGeometry = parentWidget()->geometry();
	//
	QWidget::mousePressEvent(event);
}

void AngKShadowWidget::mouseReleaseEvent(QMouseEvent* event)
{
	m_mousePressed = false;
	//
	QWidget::mouseReleaseEvent(event);
}

AngKWindowHitTestResult AngKShadowWidget::hitTest(const QPoint& posOfWindow)
{
	if (!m_canResize)
		return angkClient;
	//
	QPoint pos = posOfWindow;
	if (pos.x() < m_shadowSize)
	{
		if (pos.y() < m_shadowSize)
		{
			return TopLeft;
		}
		else if (pos.y() >= height() - m_shadowSize)
		{
			return BottomLeft;
		}
		else
		{
			return Left;
		}
	}
	else if (pos.x() > width() - m_shadowSize)
	{
		if (pos.y() < m_shadowSize)
		{
			return TopRight;
		}
		else if (pos.y() >= height() - m_shadowSize)
		{
			return BottomRight;
		}
		else
		{
			return Right;
		}
	}
	else if (pos.y() < m_shadowSize)
	{
		return Top;
	}
	else if (pos.y() > height() - m_shadowSize)
	{
		return Bottom;
	}
	else
	{
		return angkClient;
	}
}

