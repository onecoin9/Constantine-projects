#include "AngKDialog.h"
#include "ui_AngKDialog.h"
#include "StyleInit.h"
#include <QMouseEvent>
#include <QPainter>
#include <QApplication>
#include <qdesktopwidget.h>
#include "AngKGlobalInstance.h"

AngKDialog::AngKDialog(QWidget* parent)
	: QDialog(parent)
	, m_nDraggableHeight(50)
	, m_bDragging(false)
{
	ui = new Ui::AngKDialog();
	ui->setupUi(this);

	this->setWindowFlag(Qt::FramelessWindowHint);
	connect(ui->closeButton, &QPushButton::clicked, this, &AngKDialog::sgnClose);

	ui->titleSymbol->hide();
	QString styleBackground;
	if ((ViewMode)AngKGlobalInstance::GetSkinMode() == ViewMode::Light) {
		styleBackground = "QFrame#bgFrame {background-color: rgba(255, 255, 255, 1);}";
	}
	else {
		styleBackground = "QFrame#bgFrame {background-color: rgba(70, 70, 70, 1);}";
	}
	ui->bgFrame->setStyleSheet(styleBackground);
	this->setObjectName("AngKDialog");
	QT_SET_STYLE_SHEET(objectName());
}

AngKDialog::~AngKDialog()
{
	delete ui;
}

QWidget* AngKDialog::setCentralWidget()
{
	return ui->centralWidget;
}

void AngKDialog::SetTitle(QString title)
{
	ui->Title->setText(title);
}

void AngKDialog::mousePressEvent(QMouseEvent* event)
{
	if (event->y() < m_nDraggableHeight && event->button() == Qt::LeftButton)
	{
		clickPos.setX(event->pos().x());
		clickPos.setY(event->pos().y());
		m_bDragging = true;
	}
}

void AngKDialog::mouseMoveEvent(QMouseEvent* event)
{
	if (m_bDragging && event->buttons() == Qt::LeftButton)
	{

		QPoint globalMousePos = event->globalPos();
		QPoint point(globalMousePos - clickPos);

		QRect screenGeometry = QApplication::desktop()->availableGeometry(this);

		//if (point.x() < screenGeometry.left()) {
		//	point.setX(screenGeometry.left());
		//}
		//if (point.y() < screenGeometry.top()) {
		//	point.setY(screenGeometry.top());
		//}
		//if (point.x() + this->width() > screenGeometry.right()) {
		//	point.setX(screenGeometry.right() - this->width());
		//}
		//if (point.y() + this->height() > screenGeometry.bottom()) {
		//	point.setY(screenGeometry.bottom() - this->height());
		//}

		this->move(point);
	}
}

void AngKDialog::mouseReleaseEvent(QMouseEvent* event)
{
	m_bDragging = false; // 重置拖动状态
	QDialog::mouseReleaseEvent(event);
}
