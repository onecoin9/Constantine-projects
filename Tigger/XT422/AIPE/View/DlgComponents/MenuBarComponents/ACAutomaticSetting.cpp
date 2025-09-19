#include "ACAutomaticSetting.h"
#include "ui_ACAutomaticSetting.h"
#include "StyleInit.h"
#include "ACAutomaticManager.h"
#include "ACMessageBox.h"
#include <QFileDialog>
#include <QTimer>

ACAutomaticSetting::ACAutomaticSetting(QWidget *parent)
	: AngKDialog(parent)
	, m_strRecordProjPath(QCoreApplication::applicationDirPath())
	, m_bLoading(false), m_bPluginLoaded(true)
{
	this->setObjectName("ACAutomaticSetting");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::ACAutomaticSetting();
	ui->setupUi(setCentralWidget());

	m_textChangeTimer = new QTimer(this);
	connect(m_textChangeTimer, &QTimer::timeout, [this]() {
		/*if (this->ui->loadButton->text().length() < sizeof("Loading.."))
			this->ui->loadButton->setText(this->ui->loadButton->text() + ".");
		else*/
			this->ui->loadButton->setText(tr("Loading"));
		});


	m_timeOutTimer = new QTimer(this);
	m_timeOutTimer->setInterval(1000 * 60 * 5);
	//FIXME, automatic maybe paused, so do not need to set to false.
	//connect(m_timeOutTimer, &QTimer::timeout, [this]() {setLoadStatus(false); });

	this->setFixedSize(760, 400);
	this->SetTitle(tr("Automatic Setting"));

	InitText();
	InitButton();

	if (!ACAutomaticManager::instance()->GetAutomaticPlugin()) {
		m_bPluginLoaded = false;
		ACMessageBox::showError(this, QObject::tr("Error"), QObject::tr("Loading automatic plugin error"));
	}

	//ui->widget_4->hide();
}

ACAutomaticSetting::~ACAutomaticSetting()
{
	delete ui;
}

void ACAutomaticSetting::InitText()
{
	ui->passlotText->setText(tr("Pass Lot:"));
	ui->tskPathText->setText(tr("Tsk Path:"));
	ui->supplyStartPosCheck->setText(tr("Supply Start Position Set"));
	ui->supplyXPosText->setText(tr("Supply X Position"));
	ui->supplyYPosText->setText(tr("Supply Y Position"));
	ui->okStartPosCheck->setText(tr("OK Start Position Set"));
	ui->okXPosText->setText(tr("OK X Position"));
	ui->okYPosText->setText(tr("OK Y Position"));
	ui->ngStartPosCheck->setText(tr("NG Start Position Set"));
	ui->ngXPosText->setText(tr("NG X Position"));
	ui->ngYPosText->setText(tr("NG Y Position"));
	ui->realStartPosText->setText(tr("Reel Start Position:"));

	ui->loadButton->setText(tr("Load"));

	ui->cmd3GroupSetting->setTitle(tr("Command3 Setting"));
}

void ACAutomaticSetting::InitButton()
{
	connect(this, &AngKDialog::sgnClose, [this]() {
		if (GetLoadStatus()) {
			ACMessageBox::showWarning(this, tr("Tips"), tr("The Automatic programming has not finished yet."));
			return;
		}
		this->close();
		});

	connect(ui->tskPathButton, &QPushButton::clicked, this, &ACAutomaticSetting::onSlotTskPathSetting);
	connect(ui->loadButton, &QPushButton::clicked, this, &ACAutomaticSetting::onSlotLoadAutoSetting);
}

/// <summary>
/// 加载tsk文件和托盘信息
/// </summary>
void ACAutomaticSetting::onSlotLoadAutoSetting()
{
	if (GetLoadStatus())
		return;

	// 检测烧录器的task状态，如果与当前task不一致，需要提醒用户手动确认
	emit sgnCheckSitesTskStatus();

	if (!m_bSitesTskPass)
		return;

	auto autoPlugin = ACAutomaticManager::instance()->GetAutomaticPlugin();
	if (autoPlugin == nullptr)
	{
		ALOG_FATAL("Get Automatic Plugin failed.", "CU", "--");
		return;
	}

	m_timeOutTimer->start();
	setLoadStatus(true);
	//if (!ui->tskPathEdit->isHidden() && ui->tskPathEdit->text().isEmpty()) {
	//	auto ret = ACMessageBox::showWarning(this, tr("Warning"), tr("Automated tsk file not selected, there may be failures. Do you want to continue?"), ACMessageBox::ACMsgButton::MSG_OK_CANCEL_BTN);
	//	if (ret != ACMessageBox::ACMsgType::MSG_CANCEL) {
	//		return;
	//	}
	//}

	emit sgnClearLotData(ui->passlotEdit->text().toInt());

	QString strTaskCmd;
	strTaskCmd = QString("3,%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,,,,,,,").arg(ui->passlotEdit->text().toInt())
		.arg(ui->tskPathEdit->text()).arg(ui->supplyStartPosCheck->isChecked()).arg(ui->supplyXPosEdit->text().toInt())
		.arg(ui->supplyYPosEdit->text().toInt()).arg(ui->okStartPosCheck->isChecked()).arg(ui->okXPosEdit->text().toInt())
		.arg(ui->okYPosEdit->text().toInt()).arg(ui->ngStartPosCheck->isChecked()).arg(ui->ngXPosEdit->text().toInt())
		.arg(ui->ngYPosEdit->text().toInt()).arg(ui->realStartPosEdit->text().toInt());

	ALOG_INFO("SlotLoadAutoSetting.", "CU", "--");
	autoPlugin->SetTask(strTaskCmd.toStdString());

	//下发Cmd3之后，直接下发0x63命令
	emit sgnAutoTellDevReady(ui->passlotEdit->text().toInt());
}

void ACAutomaticSetting::onSlotTskPathSetting()
{
	QString strProjPath = QFileDialog::getOpenFileName(this, "Select Bind Project File...", m_strRecordProjPath, tr("auto task File(*.tsk)"));
	if (!strProjPath.isEmpty()) {
		m_strRecordProjPath = strProjPath;
		ui->tskPathEdit->setText(QDir::fromNativeSeparators(m_strRecordProjPath));
	}
}


void ACAutomaticSetting::onSlotAutomicOver() {
	m_timeOutTimer->stop();
	setLoadStatus(false);
}

void ACAutomaticSetting::setLoadStatus(bool status) {
	m_bLoading = status;
	ui->loadButton->setEnabled(status ? false : true);
	if (status)
		m_textChangeTimer->start(500);
	else {
		m_textChangeTimer->stop();
	}
	ui->loadButton->setText(status ? tr("Loading") : tr("Load"));
}

bool ACAutomaticSetting::GetLoadStatus() {
	return m_bLoading;
}