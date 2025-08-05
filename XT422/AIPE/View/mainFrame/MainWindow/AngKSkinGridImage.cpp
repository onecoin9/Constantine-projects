#include "AngKSkinGridImage.h"

#include <QPainter>
#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>

bool AngKSkinGridImage::clear()
{
	if (!m_img.isNull())
	{
		m_img = QImage();
	}
	return true;
}

bool AngKSkinGridImage::splitRect(const QRect& rcSrc, QPoint ptTopLeft, QRect* parrayRect, int nArrayCount)
{
	//ATLASSERT(nArrayCount == 9);
	//
	QRect* arrayRect = parrayRect;
	//
	int nWidth = rcSrc.width();
	int nHeight = rcSrc.height();
	//
	if (ptTopLeft.x() <= 0)
		return false;
	if (ptTopLeft.y() <= 0)
		return false;
	if (ptTopLeft.x() >= nWidth / 2)
		return false;
	if (ptTopLeft.y() >= nHeight / 2)
		return false;
	//
	int x1 = rcSrc.left() + 0;
	int x2 = rcSrc.left() + ptTopLeft.x();
	int x3 = rcSrc.left() + nWidth - ptTopLeft.x();
	int x4 = rcSrc.left() + nWidth;
	//
	int y1 = rcSrc.top() + 0;
	int y2 = rcSrc.top() + ptTopLeft.y();
	int y3 = rcSrc.top() + nHeight - ptTopLeft.y();
	int y4 = rcSrc.top() + nHeight;
	//
	arrayRect[0] = QRect(QPoint(x1, y1), QPoint(x2, y2));
	arrayRect[1] = QRect(QPoint(x2 + 1, y1), QPoint(x3, y2));
	arrayRect[2] = QRect(QPoint(x3 + 1, y1), QPoint(x4, y2));

	arrayRect[3] = QRect(QPoint(x1, y2 + 1), QPoint(x2, y3));
	arrayRect[4] = QRect(QPoint(x2 + 1, y2 + 1), QPoint(x3, y3));
	arrayRect[5] = QRect(QPoint(x3 + 1, y2 + 1), QPoint(x4, y3));

	arrayRect[6] = QRect(QPoint(x1, y3 + 1), QPoint(x2, y4));
	arrayRect[7] = QRect(QPoint(x2 + 1, y3 + 1), QPoint(x3, y4));
	arrayRect[8] = QRect(QPoint(x3 + 1, y3 + 1), QPoint(x4, y4));
	//
	return true;
}

bool AngKSkinGridImage::setImage(const CString& strImageFileName, QPoint ptTopLeft, QColor darkColor)
{
	clear();
	//
	if (!m_img.load(strImageFileName))
		return false;

	int nImageWidth = m_img.width();
	int nImageHeight = m_img.height();
	//
	return splitRect(QRect(0, 0, nImageWidth, nImageHeight), ptTopLeft, m_arrayImageGrid, 9);
}

bool AngKSkinGridImage::setImage(const QImage& image, QPoint ptTopLeft, QColor darkColor)
{
	clear();
	//
	m_img = image;
	//
	int nImageWidth = m_img.width();
	int nImageHeight = m_img.height();
	//
	return splitRect(QRect(0, 0, nImageWidth, nImageHeight), ptTopLeft, m_arrayImageGrid, 9);
}

void AngKSkinGridImage::draw(QPainter* p, QRect rc, int nAlpha) const
{
	QRect arrayDest[9];
	//
	splitRect(rc, m_arrayImageGrid[0].bottomRight(), arrayDest, 9);
	//
	for (int i = 0; i < 9; i++)
	{
		const QRect& rcSrc = m_arrayImageGrid[i];
		const QRect& rcDest = arrayDest[i];
		//
		if (nAlpha > 0 && nAlpha <= 255)
		{
			p->drawImage(rcDest, m_img, rcSrc);
		}
		else
		{
			p->drawImage(rcDest, m_img, rcSrc);
		}
	}
}

void AngKSkinGridImage::drawBorder(QPainter* p, QRect rc) const
{
	QRect arrayDest[9];
	//
	splitRect(rc, m_arrayImageGrid[0].bottomRight(), arrayDest, 9);
	//
	for (int i = 0; i < 9; i++)
	{
		if (i == 4)
			continue;
		//
		const QRect& rcSrc = m_arrayImageGrid[i];
		const QRect& rcDest = arrayDest[i];
		//
		p->drawImage(rcDest, m_img, rcSrc);
	}
}

bool AngKSkinGridImage::valid() const
{
	return m_img.width() > 0 && m_img.height() > 0;
}
