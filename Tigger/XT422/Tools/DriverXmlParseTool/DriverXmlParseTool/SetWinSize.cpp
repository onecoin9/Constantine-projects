#include "SetWinSize.h"
#include "ui_SetWinSize.h"

SetWinSize::SetWinSize(QWidget *parent)
	: QDialog(parent)
{
	ui = new Ui::SetWinSize();
	ui->setupUi(this);

	connect(ui->pushButton, &QPushButton::clicked, this, [&]()
		{
			emit sgnWHSize(ui->lineEdit->text().toInt(), ui->lineEdit2->text().toInt());
		});
	connect(ui->pushButton2, &QPushButton::clicked, this, [&]()
		{
			emit sgnWHSize(ui->lineEdit->text().toInt(), ui->lineEdit2->text().toInt());
		});
}

SetWinSize::~SetWinSize()
{
	delete ui;
}

void SetWinSize::SetWHSize(int w, int h)
{
	ui->lineEdit->setText(QString::number(w));
	ui->lineEdit2->setText(QString::number(h));
}
