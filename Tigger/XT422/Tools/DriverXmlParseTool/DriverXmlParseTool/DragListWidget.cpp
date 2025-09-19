#include "DragListWidget.h"

DragListWidget::DragListWidget(QWidget *parent)
	: QListWidget(parent)
	, m_sitem(nullptr)
{
	setDragEnabled(true); //�����Ϸ�
	setAcceptDrops(true); //�����Ϸ�
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
	// ֻ��������϶�
	if (!(event->buttons() & Qt::LeftButton))
	{
		return;
	}
	// �ƶ�һ�����������ǿ�ʼ�϶�
	if ((event->pos() - m_startPos).manhattanLength() < QApplication::startDragDistance())
	{
		qDebug() << QString("event->pos().x = %1, event->pos().y = %2, manhattanLength = %3, QApplication::startDragDistance() = %4").arg(event->pos().x()).arg(event->pos().y()).arg((event->pos() - m_startPos).manhattanLength()).arg(QApplication::startDragDistance());
		return;
	}

	// �ҵ��϶�����ComboBox
	if (m_sitem == NULL)
	{
		return;
	}
	QString itemText = m_sitem->text();
	QByteArray byteData;
	// ��������
	QDrag* drag = new QDrag(this);
	QMimeData* mimeData = new QMimeData();
	mimeData->setData(itemText, byteData);
	mimeData->setText(itemText);
	drag->setMimeData(mimeData);
	// �����϶�ʱ��ͼ����ʾ
	QPixmap drag_img(120, 18);
	QPainter painter(&drag_img);
	painter.drawText(QRectF(20, 0, 120, 18), itemText, QTextOption(Qt::AlignVCenter));
	drag->setPixmap(drag_img);
	// �����Ϸ� start a drag
	Qt::DropAction result = drag->exec(Qt::CopyAction | Qt::MoveAction);
	// ��������û�гɹ�����û�б�ȡ��
	if (Qt::IgnoreAction != result)
	{
		qDebug() << QString::fromLocal8Bit("�ɹ������ק");
	}
}
