#include "AngKLoginDialog.h"
#include "ui_AngKLoginDialog.h"
#include "AngKHelper.h"
#include "StyleInit.h"
#include "AngKLoginInput.h"
#include "ACEventLogger.h"
#include "AngKGlobalInstance.h"
#include "ACMessageBox.h"
#include <QScreen>
#include <QDesktopWidget>
#include <QMessageBox>
AngKLoginDialog::AngKLoginDialog(QWidget *parent)
	: AngKShadowWindow<QDialog>(parent, true)
{
	ui = new Ui::AngKLoginDialog();

	layoutTitleBar();
	
	QWidget* uiWidget = new QWidget(clientWidget());
	clientLayout()->addWidget(uiWidget);
	ui->setupUi(uiWidget);

	setMinimumSize(1920, 1080);

	AngKWindowTitleBar* title = titleBar();
	title->setMinimumHeight(45);
	title->setPalette(QPalette(QColor("#E8EAED")));
	title->setContentsMargins(QMargins(0, 0, 0, 0));
	title->setShowTitleComponent(false);

	connect(title, &AngKWindowTitleBar::sgnIntroPropetry, this, &AngKLoginDialog::onSlotSetChooseIntroLabel);
	connect(ui->manualWidget, &AngKLoginChoose::sgnOpenLoginPage, this, &AngKLoginDialog::onSlotOpenManualLoginPage);
	connect(ui->loginInput, &AngKLoginInput::sgnRequstLogin, this, &AngKLoginDialog::onSlotRequstLogin);

	InitChooseWgt();

	this->setFixedSize(900, 730);

	ui->chooseWidget->hide();
	this->setObjectName("AngKLoginDialog");
	QT_SET_STYLE_SHEET(objectName());
}

AngKLoginDialog::~AngKLoginDialog()
{
	delete ui;
}

void AngKLoginDialog::InitChooseWgt()
{
	ui->manualWidget->setBackgroudProperty("AP9900");
	ui->autoWidget->setBackgroudProperty("IPS3000");

	ui->manualWidget->setTitle(tr("Manual"));
	ui->autoWidget->setTitle(tr("Automatic"));

	ui->manualWidget->AppendItem("AP9900");
	ui->autoWidget->AppendItem("IPS3000");
}

void AngKLoginDialog::onSlotOpenManualLoginPage(QObject* obj)
{
	emit accept();
}

void AngKLoginDialog::onSlotRequstLogin(int userMode, QString strID, QString strPwd)
{
	//没有密码登录鉴权服务器判断，此处先注释
	//if (AngKGlobalInstance::ReadValue("LoginPwd", "loginPwd").toString().compare(strPwd, Qt::CaseSensitivity::CaseSensitive) != 0) {
	//	QMessageBox::warning(this, tr("Warning"), tr("Incorrect username or password"), QMessageBox::Ok);
	//	return;
	//}
	bool bIncorrect = false;
	if (strID != "User") {
		if (strID != "Developer") {
			bIncorrect = true;
		}
	}
	else if (strPwd != "123456") {
		bIncorrect = true;
	}

	if (bIncorrect) {
		ACMessageBox::showWarning(this, tr("Warning"), tr("Incorrect username or password"));
		return;
	}

	AngKGlobalInstance::WriteValue("LoginPwd", "userMode", userMode);
	AngKGlobalInstance::WriteValue("LoginPwd", "userID", strID);
	AngKGlobalInstance::WriteValue("LoginPwd", "loginPwd", strPwd);

	std::string strRole = userMode == 1 ? "User" : "Developer";
	EventLogger->SendEvent(EventBuilder->GetLogIn(strID.toStdString(), "", strRole));

	ui->chooseWidget->show();
	ui->loginWidget->hide();
	this->setFixedSize(1920, 1080);

	//移动窗体到屏幕中央

	QDesktopWidget* m = QApplication::desktop();
	QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
	this->move(desk_rect.width() / 2 - this->width() / 2 + desk_rect.left(), desk_rect.height() / 2 - this->height() / 2 + desk_rect.top());

	titleBar()->setShowTitleComponent(true);
}

void AngKLoginDialog::onSlotSetChooseIntroLabel(QString introName)
{
	ui->manualWidget->setIntroProperty(introName);
	ui->autoWidget->setIntroProperty(introName);
}