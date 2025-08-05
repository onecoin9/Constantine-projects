#include "AngKChipDialog.h"
#include "ui_AngKChipDialog.h"
#include "StyleInit.h"
#include "AngKTransmitSignals.h"
#include "ChipModel.h"
#include "GlobalDefine.h"
#include <QMouseEvent>
AngKChipDialog::AngKChipDialog(QWidget *parent)
	: QDialog(parent)
{
	ui = new Ui::AngKChipDialog();
	ui->setupUi(this);

	this->setWindowFlag(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground, true);

	connect(ui->closeButton, &QPushButton::clicked, this, &AngKChipDialog::close);
	connect(ui->chipWidget, &AngKChipSelectWidget::sgnCancel, this, &AngKChipDialog::close);
	//connect(ui->chipWidget, &AngKChipSelectWidget::sgnSelectChipDataJson, &AngKTransmitSignals::GetInstance(), &AngKTransmitSignals::sgnSelectChipDataJson);
	connect(ui->chipWidget, &AngKChipSelectWidget::sgnSelectChipDataJson, this, [=](ChipDataJsonSerial chipJson) {
		emit sgnSelectChipDataJson(chipJson);
		close();
		});

	this->setObjectName("AngKChipDialog");
	QT_SET_STYLE_SHEET(objectName());
}

AngKChipDialog::~AngKChipDialog()
{
	ui->chipWidget->sgnCancel();
	delete ui;
}

void AngKChipDialog::SetTitle(QString strTitle)
{
	ui->Title->setText(strTitle);
}

void AngKChipDialog::InitChipData()
{
	ui->chipWidget->InitChipDBData();
}

void AngKChipDialog::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		clickPos.setX(event->pos().x());
		clickPos.setY(event->pos().y());
	}
}

void AngKChipDialog::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() == Qt::LeftButton)
	{
		this->move(this->pos() + event->pos() - this->clickPos);
	}
}
