#include "AngKAddeMMCHuaweiFile.h"
#include "ui_AngKAddeMMCHuaweiFile.h"
#include "StyleInit.h"
#include "ACMessageBox.h"
#include <QFileDialog>

AngKAddeMMCHuaweiFile::AngKAddeMMCHuaweiFile(QWidget *parent)
	: AngKDialog(parent)
{
	this->setObjectName("AngKAddeMMCHuaweiFile");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::AngKAddeMMCHuaweiFile();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(650, 200);
	this->SetTitle(tr("Image Files Import"));
	setAttribute(Qt::WA_TranslucentBackground, true);

	InitButton();
}

AngKAddeMMCHuaweiFile::~AngKAddeMMCHuaweiFile()
{
	delete ui;
}

void AngKAddeMMCHuaweiFile::InitButton()
{
	connect(ui->acxmlButton, &QPushButton::clicked, this, [=]() {
		QString filePath = QFileDialog::getOpenFileName(this, "Select File...", QCoreApplication::applicationDirPath()
			, tr("xml Config(*.acxml);; All Files(*.*)"));
		ui->acxmlEdit->setText(filePath);
		});

	connect(ui->binButton, &QPushButton::clicked, this, [=]() {
		QString filePath = QFileDialog::getOpenFileName(this, "Select File...", QCoreApplication::applicationDirPath()
			, tr("acimg Files(*.acimg);; bin Files(*.bin);; All Files(*.*)"));
		ui->binEdit->setText(filePath);
		});

	connect(ui->pushButton, &QPushButton::clicked, this, [=]() {
		if (ui->acxmlEdit->text().isEmpty() || ui->binEdit->text().isEmpty()) {
			ACMessageBox::showError(this, QObject::tr("Error"), QObject::tr("acxml or acimg is empty."));
			return;
		}
		emit sgnAddHuaweiFile(ui->acxmlEdit->text(), ui->binEdit->text());
		close();
		});

	connect(this, &AngKAddeMMCHuaweiFile::sgnClose, this, &AngKAddeMMCHuaweiFile::close);
}
