#include "AngKAdvanceSetting.h"
#include "ui_AngKAdvanceSetting.h"
#include "StyleInit.h"
#include "ACMessageBox.h"
#include <QMessageBox>

AngKAdvanceSetting::AngKAdvanceSetting(QWidget *parent)
	: AngKDialog(parent)
{
	this->setObjectName("AngKAdvanceSetting");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::AngKAdvanceSetting();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(550, 260);
	this->SetTitle(tr("Advance Setting"));
	setAttribute(Qt::WA_TranslucentBackground, true);

	InitText();
	connect(ui->okButton, &QPushButton::clicked, this, &AngKAdvanceSetting::onSlotSaveAdvanceSetting);
	connect(ui->cancelButton, &QPushButton::clicked, this, &AngKAdvanceSetting::hide);
	connect(this, &AngKAdvanceSetting::sgnClose, this, &AngKAdvanceSetting::hide);
}

AngKAdvanceSetting::~AngKAdvanceSetting()
{
	delete ui;
}

void AngKAdvanceSetting::InitText()
{
	ui->pwdLabel->setText(tr("Password:"));
	ui->pwdTip->setText(tr("(must be 6 numbers)"));
	ui->confirmPwdLabel->setText(tr("Confirm Password:"));
	ui->okButton->setText(tr("OK"));
	ui->cancelButton->setText(tr("Cancel"));
}

void AngKAdvanceSetting::InitSetting(ADVSETTING* _setting)
{
	m_pAdvSetting = _setting;
}

void AngKAdvanceSetting::onSlotSaveAdvanceSetting()
{
	if (ui->groupBox->isChecked()) {
		if (ui->pwdEdit->text().size() < 6 || ui->confirmPwdEdit->text().size() < 6) {
			ACMessageBox::showWarning(this, tr("Warning"), tr("The length of password must be 6 numbers"));
			return;
		}

		if (ui->pwdEdit->text() != ui->confirmPwdEdit->text()) {
			ACMessageBox::showWarning(this, tr("Warning"), tr("The two password are not match"));
			return;
		}

		m_pAdvSetting->bPasswdEn = true;
		m_pAdvSetting->sPasswd = ui->confirmPwdEdit->text();

	}
	this->hide();
}