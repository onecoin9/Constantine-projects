#include "ACMessageBox.h"
#include "ui_ACMessageBox.h"
#include "AngKHelper.h"
#include "StyleInit.h"

ACMessageBox::ACMessageBox(QWidget* parent,
						   const QString& title,
						   const QString& text,
						   ACMsgButton defaultButton)
	: AngKDialog(parent)
	, m_clickButton(ACMsgType::MSG_CANCEL)
{

	this->setObjectName("ACMessageBox");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::ACMessageBox();
	ui->setupUi(setCentralWidget());

	connect(this, &ACMessageBox::sgnClose, this, &ACMessageBox::close);

	this->SetTitle(title);
	ui->infoLabel->setText(text);

	InitButton();
	ShowButton(defaultButton);

	this->setFixedSize(610,260);

	ui->okButton->setFocus();
}

ACMessageBox::~ACMessageBox()
{
	delete ui;
}

void ACMessageBox::InitButton()
{
	connect(ui->okButton, &QPushButton::clicked, this, &ACMessageBox::onSlotClickOKButton);
	connect(ui->cancelButton, &QPushButton::clicked, this, &ACMessageBox::onSlotClickCancelButton);

	ui->okButton->hide();
	ui->cancelButton->hide();
}

void ACMessageBox::ShowButton(ACMsgButton defaultButton)
{
	if ((int)defaultButton & (int)ACMsgButton::MSG_CANCEL_BTN){
		ui->cancelButton->show();
	}
	
	if ((int)defaultButton & (int)ACMsgButton::MSG_OK_BTN) {
		ui->okButton->show();
	}

	if ((int)defaultButton == (int)ACMsgButton::MSG_NO_BTN) {
		ui->okButton->hide();
		ui->cancelButton->hide();
	}
}

ACMessageBox::ACMsgType ACMessageBox::showInformation(QWidget* parent, const QString& title, const QString& text, ACMsgButton buttons)
{
	ACMessageBox msgBox(parent, title, text, buttons);
	msgBox.setLabelProperty("customProperty", "INFO");
	if (msgBox.exec() == -1) {
		return ACMsgType::MSG_CANCEL;
	}
	return msgBox.m_clickButton;
}

ACMessageBox::ACMsgType ACMessageBox::showWarning(QWidget* parent, const QString& title, const QString& text, ACMsgButton buttons)
{
	ACMessageBox msgBox(parent, title, text, buttons);
	msgBox.setLabelProperty("customProperty", "WARN");
	if (msgBox.exec() == -1) {
		return ACMsgType::MSG_CANCEL;
	}
	return msgBox.m_clickButton;
}

ACMessageBox::ACMsgType ACMessageBox::showError(QWidget* parent, const QString& title, const QString& text, ACMsgButton buttons)
{
	ACMessageBox msgBox(parent, title, text, buttons);
	msgBox.setLabelProperty("customProperty", "ERROR");
	if (msgBox.exec() == -1) {
		return ACMsgType::MSG_CANCEL;
	}
	return msgBox.m_clickButton;
}

void ACMessageBox::setLabelProperty(const char* name, const QVariant& value)
{
	ui->iconLabel->setProperty(name, value);
}

void ACMessageBox::onSlotClickOKButton()
{
	m_clickButton = ACMsgType::MSG_OK;
	accept();
}

void ACMessageBox::onSlotClickCancelButton()
{
	m_clickButton = ACMsgType::MSG_CANCEL;
	reject();
}
