#include "DragListWidget.h"

DragListWidget::DragListWidget(QWidget *parent)
	: QListWidget(parent)
	, m_sitem(nullptr)
{
	setDragEnabled(true); //启用拖放
	setAcceptDrops(true); //设置拖放
}

DragListWidget::~DragListWidget()
{

}

void DragListWidget::mousePressEvent(QMouseEvent* event)
{
	m_startPos = event->pos();
	m_sitem = this->itemAt(event->pos());

	QListWidget::mousePressEvent(event);
}

void DragListWidget::mouseMoveEvent(QMouseEvent* event)
{
	// 只允许左键拖动
	if (!(event->buttons() & Qt::LeftButton))
	{
		return;
	}
	// 移动一定距离后才算是开始拖动
	if ((event->pos() - m_startPos).manhattanLength() < QApplication::startDragDistance())
	{
		qDebug() << QString("event->pos().x = %1, event->pos().y = %2, manhattanLength = %3, QApplication::startDragDistance() = %4").arg(event->pos().x()).arg(event->pos().y()).arg((event->pos() - m_startPos).manhattanLength()).arg(QApplication::startDragDistance());
		return;
	}

	// 找到拖动的项ComboBox
	if (m_sitem == NULL)
	{
		return;
	}
	QString itemText = m_sitem->text();
	QByteArray byteData;
	// 创建数据
	QDrag* drag = new QDrag(this);
	QMimeData* mimeData = new QMimeData();
	mimeData->setData(itemText, byteData);
	mimeData->setText(itemText);
	drag->setMimeData(mimeData);
	// 设置拖动时的图像显示
	QPixmap drag_img(120, 18);
	QPainter painter(&drag_img);
	painter.drawText(QRectF(20, 0, 120, 18), itemText, QTextOption(Qt::AlignVCenter));
	drag->setPixmap(drag_img);
	// 启动拖放 start a drag
	Qt::DropAction result = drag->exec(Qt::CopyAction | Qt::MoveAction);
	// 检查操作有没有成功，有没有被取消
	if (Qt::IgnoreAction != result)
	{
		qDebug() << QString::fromLocal8Bit("成功完成拖拽");
	}
}
