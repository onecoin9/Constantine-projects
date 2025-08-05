#include "AngKTitleImg.h"
#include "ui_AngKTitleImg.h"
#include "../View/../View/GlobalInit/StyleInit.h"
#include <QLabel>
AngKTitleImg::AngKTitleImg(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::AngKTitleImg();
	ui->setupUi(this);

	this->setObjectName("AngKTitleImg");
	QT_SET_STYLE_SHEET(objectName());
}

AngKTitleImg::~AngKTitleImg()
{
	delete ui;
}

QLabel* AngKTitleImg::imgLabel()
{
	return ui->personImg;
}

QLabel* AngKTitleImg::nameLabel()
{
	return ui->userName;
}
