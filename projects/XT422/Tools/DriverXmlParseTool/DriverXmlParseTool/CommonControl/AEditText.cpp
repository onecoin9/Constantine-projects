#include "AEditText.h"

AEditText::AEditText(QWidget *parent)
	: QTextEdit(parent)
	, m_bView(false)
{
	this->setTextInteractionFlags(Qt::NoTextInteraction);
	this->setMinimumSize(QTextEdit::size());
	this->setMouseTracking(true);
}

AEditText::~AEditText()
{
}

int AEditText::CalCursorCol(QPoint pt)
{
	return (pt.x() < FRAMESHAPE ? 1 : ((pt.x() > this->width() - FRAMESHAPE) ? 3 : 2));
}

int AEditText::CalCursorPos(QPoint pt, int colPos)
{
	return ((pt.y() < FRAMESHAPE ? 10 : ((pt.y() > this->height() - FRAMESHAPE) ? 30 : 20)) + colPos);
}

void AEditText::setCursorShape(int CalPos)
{
	Qt::CursorShape cursor;
	switch (CalPos)
	{
	case TOPLEFT:
	case BUTTOMRIGHT:
		cursor = Qt::SizeFDiagCursor;
		break;
	case TOPRIGHT:
	case BUTTOMLEFT:
		cursor = Qt::SizeBDiagCursor;
		break;
	case TOP:
	case BUTTOM:
		cursor = Qt::SizeVerCursor;
		break;
	case LEFT:
	case RIGHT:
		cursor = Qt::SizeHorCursor;
		break;
	default:
		cursor = Qt::ArrowCursor;
		break;
	}
	viewport()->setCursor(cursor);
}

int AEditText::GetCursorPos()
{
	return m_iCalCursorPos;
}

void AEditText::SetView(bool v)
{
	m_bView = v;
}

void AEditText::mousePressEvent(QMouseEvent* event)
{
	//this->viewport()->setCursor(Qt::PointingHandCursor);
	this->setTextInteractionFlags(Qt::NoTextInteraction);
	m_iCalCursorPos = CalCursorPos(event->pos(), CalCursorCol(event->pos()));
	if (event->buttons() == Qt::LeftButton)
	{
		qDebug() << QString::fromLocal8Bit("点击AEditText窗口");

		emit sgnEditText(this);
		emit sgnShowPropetry(this);

		if (m_iCalCursorPos != CENTER)
		{
			m_bLeftPress = true;
			setCursorShape(m_iCalCursorPos);
		}
	}
	else if (event->buttons() == Qt::RightButton)
	{
		qDebug() << QString::fromLocal8Bit("右键点击AEditText窗口");

		emit sgnRClickEdit(this);
	}

	m_rtPreGeometry = geometry();
	m_ptViewMousePos = event->globalPos();
	//QTextEdit::mousePressEvent(event);
}

void AEditText::mouseDoubleClickEvent(QMouseEvent* e)
{
	this->viewport()->setCursor(Qt::IBeamCursor);
	this->setTextInteractionFlags(Qt::TextEditorInteraction);

	QTextEdit::mouseDoubleClickEvent(e);
}

void AEditText::mouseMoveEvent(QMouseEvent* event)
{
	if (m_bView)
		return;
	//窗体不是最大的话就改变鼠标的形状
	if (Qt::WindowMaximized != windowState())
	{
		setCursorShape(CalCursorPos(event->pos(), CalCursorCol(event->pos())));
	}
	//获取当前的点，这个点是全局的
	QPoint ptCurrentPos = QCursor::pos();
	//计算出移动的位置，当前点 - 鼠标左键按下的点
	QPoint ptMoveSize = ptCurrentPos - m_ptViewMousePos;
	QRect rtTempGeometry = m_rtPreGeometry;
	if (m_bLeftPress)
	{
		switch (m_iCalCursorPos)
		{
		case TOPLEFT:
			rtTempGeometry.setTopLeft(m_rtPreGeometry.topLeft() + ptMoveSize);
			break;
		case TOP:
			rtTempGeometry.setTop(m_rtPreGeometry.top() + ptMoveSize.y());
			break;
		case TOPRIGHT:
			rtTempGeometry.setTopRight(m_rtPreGeometry.topRight() + ptMoveSize);
			break;
		case LEFT:
			rtTempGeometry.setLeft(m_rtPreGeometry.left() + ptMoveSize.x());
			break;
		case RIGHT:
			rtTempGeometry.setRight(m_rtPreGeometry.right() + ptMoveSize.x());
			break;
		case BUTTOMLEFT:
			rtTempGeometry.setBottomLeft(m_rtPreGeometry.bottomLeft() + ptMoveSize);
			break;
		case BUTTOM:
			rtTempGeometry.setBottom(m_rtPreGeometry.bottom() + ptMoveSize.y());
			break;
		case BUTTOMRIGHT:
			rtTempGeometry.setBottomRight(m_rtPreGeometry.bottomRight() + ptMoveSize);
			break;
		default:
			break;
		}
		//移动窗体，如果比最小窗体大，就移动
		if (rtTempGeometry.width() > this->minimumWidth() || rtTempGeometry.height() > this->minimumHeight())
			setGeometry(rtTempGeometry);

	}
	//父类发生改变时，需要对自身属性重新赋值
	QRect labelRect = this->property("wrect").toRect();
	this->setProperty("wrect", QRect(geometry().x(), geometry().y(), this->width(), this->height()));
	QTextEdit::mouseMoveEvent(event);
}

void AEditText::mouseReleaseEvent(QMouseEvent* event)
{
	m_bLeftPress = false;
	QApplication::restoreOverrideCursor();
	emit sgnShowPropetry(this);
	QTextEdit::mouseReleaseEvent(event);
}
