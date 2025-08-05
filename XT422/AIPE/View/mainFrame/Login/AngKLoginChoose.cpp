#include "AngKLoginChoose.h"
#include "ui_AngKLoginChoose.h"
#include "../View/GlobalInit/StyleInit.h"
#include <QStandardItemModel>
#include <QGraphicsDropShadowEffect>
AngKLoginChoose::AngKLoginChoose(QWidget *parent)
	: QWidget(parent)
	, ishover(false)
{
	ui = new Ui::AngKLoginChoose();
	ui->setupUi(this);

	setIntroProperty("minWgtFont");

	//设置comboboxItem的样式，必须添加
	ui->comboBoxModels->setView(new QListView());
	m_pComboBoxModel = new QStandardItemModel(this);

	setWidgetShadow();

	this->setAttribute(Qt::WA_Hover, true);//开启悬停事件
	this->setAttribute(Qt::WA_MouseTracking, true);
	this->installEventFilter(this);       //安装事件过滤器
	this->setFixedSize(691, 491);

	this->setObjectName("AngKLoginChoose");
	QT_SET_STYLE_SHEET(objectName());
}

AngKLoginChoose::~AngKLoginChoose()
{
	delete ui;
}

void AngKLoginChoose::setBackgroudProperty(QString bgName)
{
	ui->backgroundWidget->setProperty("customProperty", bgName);
}

void AngKLoginChoose::setIntroProperty(QString introName)
{
	//第一种实时修改样式保留
	//style()->unpolish(ui->title);
	//style()->unpolish(ui->introLabel1);
	//style()->unpolish(ui->introLabel2);
	ui->title->setProperty("customProperty", introName);
	ui->introLabel1->setProperty("customProperty", introName);
	ui->introLabel2->setProperty("customProperty", introName);
	//style()->polish(ui->title);
	//style()->polish(ui->introLabel1);
	//style()->polish(ui->introLabel2);
	//update();
	this->setStyle(QApplication::style());
}

void AngKLoginChoose::setTitle(QString titleName)
{
	ui->title->setText(titleName);
}

void AngKLoginChoose::setWidgetShadow()
{
	//给顶层widget设置背景颜色，不然看不见，因为底层widget已经透明了
	//this->setStyleSheet("background-color:rgb(255, 254, 253)");
	//Qt窗口阴影类
	QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);

	//设置阴影距离
	shadow->setOffset(0, 0);

	//设置阴影颜色  686868
	shadow->setColor(QColor("#e2e2e2"));

	//设置阴影区域
	shadow->setBlurRadius(10);

	//给顶层QWidget设置阴影
	this->setGraphicsEffect(shadow);
}

void AngKLoginChoose::AppendItem(QString strName)
{
	QStandardItem* item = new QStandardItem(strName);
	item->setFont(QFont("Microsoft Yi JhengHei", 12, QFont::Light));
	m_pComboBoxModel->appendRow(item);
	ui->comboBoxModels->setModel(m_pComboBoxModel);
}

bool AngKLoginChoose::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == QEvent::HoverEnter)
	{
		ishover = true;
	}
	else if (event->type() == QEvent::HoverLeave)
	{
		ishover = false;
	}
	else if (event->type() == QEvent::KeyPress) {
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
			// 处理 Enter 键事件
			return true; // 事件已处理，不再传递
		}
	}
	return QWidget::eventFilter(watched, event);
}

void AngKLoginChoose::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	if(ishover)
		painter.setPen(QPen(QColor(9, 151, 247), 2, Qt::SolidLine));
	else
		painter.setPen(QPen(Qt::transparent, 2, Qt::SolidLine));
	painter.drawRect(this->rect());
}

void AngKLoginChoose::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::MouseButton::LeftButton)
	{
		qDebug() << "click";
		emit sgnOpenLoginPage(this);
	}
}
