#pragma once
#include <QImage>
#include <QPushButton>
#include "AngKMisc.h"
class QPainter;
class QLineEdit;
class QLabel;

class AngKSkinGridImage
{
protected:
	QImage m_img;
	QRect m_arrayImageGrid[9];
	//
	bool clear();
public:
	static bool splitRect(const QRect& rcSrc, QPoint ptTopLeft, QRect* parrayRect, int nArrayCount);
	bool setImage(const CString& strImageFileName, QPoint ptTopLeft, QColor darkColor = Qt::transparent);
	bool setImage(const QImage& image, QPoint ptTopLeft, QColor darkColor = Qt::transparent);
	//
	void draw(QPainter* p, QRect rc, int nAlpha) const;
	void drawBorder(QPainter* p, QRect rc) const;
	bool valid() const;
	//
	QSize actualSize() const { return m_img.size(); }
};
