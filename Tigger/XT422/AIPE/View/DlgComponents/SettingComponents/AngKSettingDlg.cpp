#include "AngKSettingDlg.h"
#include "ui_AngKSettingDlg.h"
#include "../View/GlobalInit/StyleInit.h"
#include "AngKGlobalInstance.h"
#include "GlobalDefine.h"
#include "sqlite3.h"
#include "MessageType.h"
#include "AngKDeviceModel.h"
#include "AngKMessageHandler.h"
#include "ACMessageBox.h"
#include <QButtonGroup>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>


extern UserMode curUserMode;

namespace AcroView
{
	AngKSettingDlg::AngKSettingDlg(QWidget* parent)
		: QDialog(parent)
		, m_curClickButton(0)
		, m_screenGroup(nullptr)
		, m_languageGroup(nullptr)
		, m_curSktMode(0)
		, m_curSktValue(0)
		, m_bDragging(false)
		, m_nDraggableHeight(50)
	{
		this->setObjectName("AngKSettingDlg");
		QT_SET_STYLE_SHEET(objectName());

		ui = new Ui::AngKSettingDlg();
		ui->setupUi(this);

		this->setWindowFlag(Qt::FramelessWindowHint);
		setAttribute(Qt::WA_TranslucentBackground, true);
		connect(ui->closeButton, &QPushButton::clicked, this, &AngKSettingDlg::close);

		InitText();
		InitButton();
		InitPage();
		hide();
	}

	AngKSettingDlg::~AngKSettingDlg()
	{
		delete ui;
	}

	void AngKSettingDlg::InitText()
	{
		ui->Title->setText(tr("Setting"));
		ui->logButton->setText(tr(" Log"));
		ui->bufferButton->setText(tr(" Buffer"));
		ui->programmerButton->setText(tr(" Programmer"));
		ui->factoryModeButton->setText(tr(" Factory Mode"));
		ui->languageButton->setText(tr(" Language"));
		ui->loginPwdButton->setText(tr(" Login Password"));
		ui->projectButton->setText(tr(" Task/Project"));
		ui->skinButton->setText(tr(" Skin"));
		ui->usrButton->setText(tr(" UsrMode"));
		ui->pageTitle->setText(ui->logButton->text());
	}

	void AngKSettingDlg::InitButton()
	{
		ui->factoryModeButton->setVisible(false);
		ui->loginPwdButton->setVisible(false);
		m_vecToolButton.push_back(ui->logButton);
		m_vecToolButton.push_back(ui->bufferButton);
		m_vecToolButton.push_back(ui->programmerButton);
		m_vecToolButton.push_back(ui->factoryModeButton);
		m_vecToolButton.push_back(ui->languageButton);
		m_vecToolButton.push_back(ui->loginPwdButton);
		m_vecToolButton.push_back(ui->projectButton);
		ui->skinButton->hide();
		//m_vecToolButton.push_back(ui->skinButton);
		m_vecToolButton.push_back(ui->usrButton);

		//设置button属性index对应每一个页面
		for (int i = 0; i < m_vecToolButton.size(); ++i)
		{
			m_vecToolButton[i]->setProperty("index", i + 1);
			connect(m_vecToolButton[i], &QToolButton::clicked, this, &AngKSettingDlg::onSlotToolButtonClick);
		}

		m_vecToolButton[m_curClickButton]->setChecked(true);

		ui->saveButton->setText(tr("Ok"));

		connect(ui->saveButton, &QPushButton::clicked, this, &AngKSettingDlg::onSlotSaveConfig);
	}

	void AngKSettingDlg::InitPage()
	{
		//LogPage
		ui->logPathText->setText(tr("Log Path"));
		ui->fileSizeText->setText(tr("Max Log File Size"));
		ui->keepTime->setText(tr("Log Keep Time"));
		ui->kbText->setText("KB");
		ui->keepTimeDayText->setText(tr("day (0 to keep log files forever)"));
		connect(ui->logPathButton, &QPushButton::clicked, this, [=]() {
			QString filePath = QFileDialog::getExistingDirectory(this, tr("Select File Directory..."), QCoreApplication::applicationDirPath(), QFileDialog::ShowDirsOnly);
			if (filePath.isEmpty())
				return;
			ui->logPathEdit->setText(filePath);
			ui->logPathEdit->setCursorPosition(0);
			ui->logPathEdit->setToolTip(filePath);
			});

		ui->logPathEdit->setText(AngKGlobalInstance::ReadValue("LogFile", "path").toString());
		ui->fileSizeEdit->setText(AngKGlobalInstance::ReadValue("LogFile", "size").toString());
		ui->keepTimeEdit->setText(AngKGlobalInstance::ReadValue("LogFile", "keepTime").toString());
		ui->logPathEdit->setReadOnly(true);

		//BufferPage
		ui->bufferPathText->setText(tr("Buffer Path"));
		ui->checkDataBox->setText(tr("Check Customer's Data File Loaded?"));
		ui->bufferPathEdit->setText(AngKGlobalInstance::ReadValue("Buffer", "path").toString());
		ui->checkDataBox->setChecked(AngKGlobalInstance::ReadValue("Buffer", "dataFileCheck").toBool());
		connect(ui->bufferPathButton, &QPushButton::clicked, this, [=]() {
			QString filePath = QFileDialog::getExistingDirectory(this, tr("Select File Directory..."), QCoreApplication::applicationDirPath(), QFileDialog::ShowDirsOnly);
			if (filePath.isEmpty())
				return;
			ui->bufferPathEdit->setText(filePath);
			ui->bufferPathEdit->setCursorPosition(0);
			ui->bufferPathEdit->setToolTip(filePath);
			});
		ui->bufferPathEdit->setReadOnly(true);

		//Programmer
		InitProgramPage();

		//Factory Mode
		InitFactoryMode();

		//Language
		InitLanguage();

		//LoginPassword
		InitLoginPassword();

		//Project
		InitTaskProjectPage();


		ui->usrModePwdText->setVisible(false);
		ui->usrModePwdEdit->setVisible(false);
		ui->usrModePwdEdit->clear();
	}


	void AngKSettingDlg::saveProgramInfo() {
		m_programInfo.sktModeIdx = ui->sktModeCombo->currentIndex();
		m_programInfo.sktAbsolute = ui->sktAbsoluteEdit->text();
		m_programInfo.sktPercent = ui->sktPercentEdit->text();
	}

	void AngKSettingDlg::OpenActionPage(int nIndex)
	{
		if (nIndex == 0)
			return;

		m_vecToolButton[nIndex - 1]->click();

		ReBackfillSetting();
		saveProgramInfo();

	}

	void AngKSettingDlg::InitProgramPage()
	{
		ui->buzzerGroup->setTitle(tr(" Buzzer Setting "));
		ui->buzzerCheck->setText(tr("Enable Buzzer"));
		ui->buzzerGroup->hide();
		ui->screenGroup->setTitle(tr(" Screen Setting "));
		ui->screenGroup->hide();
		ui->showtimeGroup->hide();
		m_screenGroup = new QButtonGroup(ui->screenGroup);

		connect(m_screenGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), [&](int index) {
			qDebug() << "单击编号为 " << index;
			});

		ui->radioButton_1->setText(tr("Keep Screen Backlight Always Off"));
		ui->radioButton_2->setText(tr("Keep Screen Backlight 5s"));
		ui->radioButton_3->setText(tr("Keep Screen Backlight 10s"));
		ui->radioButton_4->setText(tr("Keep Screen Backlight 30s"));
		ui->radioButton_5->setText(tr("Keep Screen Backlight 60s"));
		ui->radioButton_6->setText(tr("Keep Screen Backlight Always On"));

		m_screenGroup->addButton(ui->radioButton_1, 1);
		m_screenGroup->addButton(ui->radioButton_2, 2);
		m_screenGroup->addButton(ui->radioButton_3, 3);
		m_screenGroup->addButton(ui->radioButton_4, 4);
		m_screenGroup->addButton(ui->radioButton_5, 5);
		m_screenGroup->addButton(ui->radioButton_6, 6);
		m_screenGroup->button(1)->setChecked(true);

		ui->showtimeGroup->setTitle(tr(" Show Time Setting "));
		ui->timeCheck->setText(tr("Show Time Tag"));

		ui->socketGroup->setTitle(tr(" Socket Lifetime Notification "));
		ui->modeLabel->setText(tr("Mode:"));
		ui->percentLabel->setText(tr("Percent:"));
		ui->percentSymbol->setText("%");
		ui->absoluteLabel->setText(tr("Absolute:"));
		ui->sktPercentEdit->setText("0");
		ui->sktAbsoluteEdit->setText("0");

		ui->sktModeCombo->addItem(tr("Disable"), (int)progSettingMode::Disabled);
		ui->sktModeCombo->addItem(tr("Percent"), (int)progSettingMode::Percent);
		ui->sktModeCombo->addItem(tr("Absolute"), (int)progSettingMode::Absolute);


		ui->usrModeCombo->addItem(tr("Operator"), (int)UserMode::Operator);
		ui->usrModeCombo->addItem(tr("Engineer"), (int)UserMode::Engineer);
		ui->usrModeCombo->addItem(tr("Developer"), (int)UserMode::Developer);

		ui->usrModeCombo->setCurrentIndex(0);

		if (AngKGlobalInstance::ReadValue("Login", "AuthMode").toInt() == 2)
			ui->usrModeCombo->setEnabled(false);

		ui->sktModeCombo->setCurrentIndex(0);
		connect(ui->sktModeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AngKSettingDlg::onSlotSktComboBoxSwitch);
		connect(ui->usrModeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]() {
			if (ui->usrModeCombo->currentIndex() == 2) {
				ui->usrModePwdText->setVisible(true);
				ui->usrModePwdEdit->setVisible(true);
				ui->usrModePwdEdit->clear();
				//QInputDialog dialog(this);
				//dialog.setWindowTitle(tr("Developer Password"));
				//dialog.setLabelText(tr("Password:"));
				//dialog.setTextValue("");
				//dialog.setInputMode(QInputDialog::TextInput);
				//dialog.setFixedSize(250, 200);
				//dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
				//if (dialog.exec() == QDialog::Accepted) {
				//	QString pwd = dialog.textValue();
				//	if (pwd == "ACROVIEW") {
				//		curUserMode = (UserMode)ui->usrModeCombo->currentData().toInt();

				//		ALOG_INFO("Usr Mode changed, cur mode :%s:", "CU", "--", ui->usrModeCombo->currentText());
				//	}
				//	else {
				//		if (!pwd.isEmpty())
				//			ACMessageBox::showWarning(this, QObject::tr("Warning"), QObject::tr("Password error!"));
				//		ui->usrModeCombo->setCurrentIndex((int)curUserMode - (int)(UserMode::Operator));
				//	}
				//}
				//else {
				//	ui->usrModeCombo->setCurrentIndex((int)curUserMode - (int)(UserMode::Operator));
				//}
			}
			else {
				ui->usrModePwdText->setVisible(false);
				ui->usrModePwdEdit->setVisible(false);
				//curUserMode = (UserMode)ui->usrModeCombo->currentData().toInt();
				//ALOG_INFO("Usr Mode changed, cur mode:%s:", "CU", "--", ui->usrModeCombo->currentText().toLocal8Bit().data());
	
			}
			});
		std::map<std::string, DeviceStu> insertDev;
		AngKDeviceModel::instance().GetConnetDevMap(insertDev);
		for (auto iter : insertDev) {
			AngKMessageHandler::instance().Command_RemoteDoPTCmd(iter.second.strIP, iter.second.nHopNum, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_GetProgramSetting, ALL_MU, 8, QByteArray());
		}
	}

	void AngKSettingDlg::InitFactoryMode()
	{
		ui->FactoryEnableCheck->setText(tr("Factory Mode Enable"));
		ui->pwdLabel->setText(tr("Password:"));
		ui->pwdTip->setText(tr("(must be 6 numbers)"));
		ui->confirmPwdLabel->setText(tr("Confirm Password:"));

		ui->pwdEdit->setText(AngKGlobalInstance::ReadValue("FactoryMode", "passwd").toString());
		ui->confirmPwdEdit->setText(AngKGlobalInstance::ReadValue("FactoryMode", "passwd").toString());

		ui->OperationGroup->setTitle(tr("Operation Enable"));
		ui->programCheck->setText(tr("Enable Program"));
		ui->verifyCheck->setText(tr("Enable Verify"));
		ui->blankCheck->setText(tr("Enable Blank"));
		ui->eraseCheck->setText(tr("Enable Erase"));
		ui->secureCheck->setText(tr("Enable Secure"));
		ui->readCheck->setText(tr("Enable Read"));
		ui->factorySelectAllCheck->setText(tr("Select All"));

		ui->programCheck->setEnabled(false);
		ui->verifyCheck->setEnabled(false);
		ui->blankCheck->setEnabled(false);
		ui->eraseCheck->setEnabled(false);
		ui->secureCheck->setEnabled(false);
		ui->readCheck->setEnabled(false);
		ui->factorySelectAllCheck->setEnabled(false);
		ui->pwdEdit->setEnabled(false);
		ui->confirmPwdEdit->setEnabled(false);

		connect(ui->FactoryEnableCheck, &QCheckBox::clicked, this, [=](bool bCheck)
			{
				ui->programCheck->setEnabled(bCheck);
				ui->verifyCheck->setEnabled(bCheck);
				ui->blankCheck->setEnabled(bCheck);
				ui->eraseCheck->setEnabled(bCheck);
				ui->secureCheck->setEnabled(bCheck);
				ui->readCheck->setEnabled(bCheck);
				ui->factorySelectAllCheck->setEnabled(bCheck);
				ui->pwdEdit->setEnabled(bCheck);
				ui->confirmPwdEdit->setEnabled(bCheck);
			});

		connect(ui->factorySelectAllCheck, &QCheckBox::clicked, this, [=](bool bCheck)
			{
				ui->programCheck->setChecked(bCheck);
				ui->verifyCheck->setChecked(bCheck);
				ui->blankCheck->setChecked(bCheck);
				ui->eraseCheck->setChecked(bCheck);
				ui->secureCheck->setChecked(bCheck);
				ui->readCheck->setChecked(bCheck);
			});

		if (AngKGlobalInstance::ReadValue("FactoryMode", "enable").toBool())
		{
			ui->FactoryEnableCheck->click();
		}

		setEnableFactory(AngKGlobalInstance::ReadValue("FactoryMode", "func").toInt());
	}

	void AngKSettingDlg::InitLanguage()
	{
		m_languageGroup = new QButtonGroup(ui->LanguagePage);

		connect(m_languageGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), [&](int index) {
			});

		ui->enButton->setText(tr("English"));
		ui->cnButton->setText(tr("Chinese Simplified"));
		ui->jpButton->setText(tr("Japanese"));

		m_languageGroup->addButton(ui->enButton, 1);
		m_languageGroup->addButton(ui->cnButton, 2);
		m_languageGroup->addButton(ui->jpButton, 3);

		m_languageGroup->button(AngKGlobalInstance::ReadValue("Language", "mode").toInt())->setChecked(true);
	}

	void AngKSettingDlg::InitLoginPassword()
	{
		ui->pwdEnableGroup->setTitle(tr("PassWord Enable"));

		ui->oldPwd->setText(tr("Old Password:"));
		ui->newPwd->setText(tr("New Password"));
		ui->pwdConfirmButton->setText(tr("Confirm"));

		ui->userMode->setText(tr("UserMode"));
		ui->userConfirmButton->setText(tr("Confirm"));
	}

	void AngKSettingDlg::InitTaskProjectPage()
	{
		ui->projPathText->setText(tr("Project save path"));
		ui->projPathEdit->setReadOnly(true);
		ui->taskPathText->setText(tr("Task save path"));
		ui->taskPathEdit->setReadOnly(true);

		if (ui->projPathEdit->text().isEmpty()) {
			ui->projPathEdit->setText(AngKGlobalInstance::ReadValue("TaskProjectPath", "projPath").toString());
			ui->projPathEdit->setCursorPosition(0);
			ui->projPathEdit->setToolTip(AngKGlobalInstance::ReadValue("TaskProjectPath", "projPath").toString());
		}
		if (ui->taskPathEdit->text().isEmpty()) {
			ui->taskPathEdit->setText(AngKGlobalInstance::ReadValue("TaskProjectPath", "taskPath").toString());
			ui->taskPathEdit->setCursorPosition(0);
			ui->taskPathEdit->setToolTip(AngKGlobalInstance::ReadValue("TaskProjectPath", "taskPath").toString());
		}

		connect(ui->projPathButton, &QPushButton::clicked, this, [=]() {
			QString filePath = QFileDialog::getExistingDirectory(this, tr("Select File Directory..."), QCoreApplication::applicationDirPath(), QFileDialog::ShowDirsOnly);
			if (filePath.isEmpty())
				return;
			ui->projPathEdit->setText(filePath);
			ui->projPathEdit->setCursorPosition(0);
			ui->projPathEdit->setToolTip(filePath);
		});

		connect(ui->taskPathButton, &QPushButton::clicked, this, [=]() {
			QString filePath = QFileDialog::getExistingDirectory(this, tr("Select File Directory..."), QCoreApplication::applicationDirPath(), QFileDialog::ShowDirsOnly);
			if (filePath.isEmpty())
				return;
			ui->taskPathEdit->setText(filePath);
			ui->taskPathEdit->setCursorPosition(0);
			ui->taskPathEdit->setToolTip(filePath);
			});
	}

	void AngKSettingDlg::ReBackfillSetting()
	{
		//Log Path
		ui->logPathEdit->setText(AngKGlobalInstance::ReadValue("LogFile", "path").toString());
		ui->logPathEdit->setCursorPosition(0);
		ui->logPathEdit->setToolTip(AngKGlobalInstance::ReadValue("LogFile", "path").toString());
		ui->fileSizeEdit->setText(AngKGlobalInstance::ReadValue("LogFile", "size").toString());
		ui->keepTimeEdit->setText(AngKGlobalInstance::ReadValue("LogFile", "keepTime").toString());

		//buffer
		ui->bufferPathEdit->setText(AngKGlobalInstance::ReadValue("Buffer", "path").toString());
		ui->bufferPathEdit->setCursorPosition(0);
		ui->bufferPathEdit->setToolTip(AngKGlobalInstance::ReadValue("Buffer", "path").toString());
		ui->checkDataBox->setChecked(AngKGlobalInstance::ReadValue("Buffer", "dataFileCheck").toBool());

		//prog
		ui->sktModeCombo->setCurrentIndex(m_curSktMode);
		if (m_curSktMode == 0)
		{
			ui->sktPercentEdit->setText("0");
			ui->sktAbsoluteEdit->setText("0");
			ui->sktAbsoluteEdit->setEnabled(false);
			ui->sktPercentEdit->setEnabled(false);
		}
		else if (m_curSktMode == 1) {
			ui->sktPercentEdit->setText(QString::number(m_curSktValue));
			ui->sktAbsoluteEdit->setText("0");
			ui->sktAbsoluteEdit->setEnabled(false);
			ui->sktPercentEdit->setEnabled(true);
		}
		else {
			ui->sktPercentEdit->setText("0");
			ui->sktAbsoluteEdit->setText(QString::number(m_curSktValue));
			ui->sktAbsoluteEdit->setEnabled(true);
			ui->sktPercentEdit->setEnabled(false);
		}

		//factory
		ui->pwdEdit->setText(AngKGlobalInstance::ReadValue("FactoryMode", "passwd").toString());
		ui->confirmPwdEdit->setText(AngKGlobalInstance::ReadValue("FactoryMode", "passwd").toString());
		setEnableFactory(AngKGlobalInstance::ReadValue("FactoryMode", "func").toInt());

		//language
		m_languageGroup->button(AngKGlobalInstance::ReadValue("Language", "mode").toInt())->setChecked(true);

		//project
		ui->projPathEdit->setText(AngKGlobalInstance::ReadValue("TaskProjectPath", "projPath").toString());
		ui->projPathEdit->setCursorPosition(0);
		ui->projPathEdit->setToolTip(AngKGlobalInstance::ReadValue("TaskProjectPath", "projPath").toString());
		//Task
		ui->taskPathEdit->setText(AngKGlobalInstance::ReadValue("TaskProjectPath", "taskPath").toString());
		ui->taskPathEdit->setCursorPosition(0);
		ui->taskPathEdit->setToolTip(AngKGlobalInstance::ReadValue("TaskProjectPath", "taskPath").toString());

		//usrMode
		ui->usrModeCombo->setCurrentIndex((int)curUserMode - (int)(UserMode::Operator));
	}

	void AngKSettingDlg::mousePressEvent(QMouseEvent* event)
	{

		if (event->y() < m_nDraggableHeight && event->button() == Qt::LeftButton)
		{
			m_clickPos.setX(event->pos().x());
			m_clickPos.setY(event->pos().y());
			m_bDragging = true;
		}
	}

	void AngKSettingDlg::mouseMoveEvent(QMouseEvent* event)
	{
		if (m_bDragging && event->buttons() == Qt::LeftButton)
		{
			this->move(this->pos() + event->pos() - this->m_clickPos);
		}
	}

	void AngKSettingDlg::mouseReleaseEvent(QMouseEvent* event)
	{
		m_bDragging = false; // 重置拖动状态
		QDialog::mouseReleaseEvent(event);
	}

	int AngKSettingDlg::calEnableFactory()
	{
		int calNum = 0;

		if (ui->programCheck->isChecked())
			calNum += CFG_ENABLE_PROGRAM;
		if (ui->verifyCheck->isChecked())
			calNum += CFG_ENABLE_VERIFY;
		if (ui->blankCheck->isChecked())
			calNum += CFG_ENABLE_BLANK;
		if (ui->eraseCheck->isChecked())
			calNum += CFG_ENABLE_ERASE;
		if (ui->secureCheck->isChecked())
			calNum += CFG_ENABLE_SECURE;
		if (ui->readCheck->isChecked())
			calNum += CFG_ENABLE_READ;

		return calNum;
	}

	void AngKSettingDlg::setEnableFactory(int calValue)
	{
		ui->programCheck->setChecked(calValue & CFG_ENABLE_PROGRAM);
		ui->verifyCheck->setChecked(calValue & CFG_ENABLE_VERIFY);
		ui->blankCheck->setChecked(calValue & CFG_ENABLE_BLANK);
		ui->eraseCheck->setChecked(calValue & CFG_ENABLE_ERASE);
		ui->secureCheck->setChecked(calValue & CFG_ENABLE_SECURE);
		ui->readCheck->setChecked(calValue & CFG_ENABLE_READ);

		if (calValue == (CFG_ENABLE_PROGRAM + CFG_ENABLE_VERIFY + CFG_ENABLE_BLANK + CFG_ENABLE_ERASE + CFG_ENABLE_SECURE + CFG_ENABLE_READ))
			ui->factorySelectAllCheck->setChecked(true);
	}

	void AngKSettingDlg::onSlotToolButtonClick(bool state)
	{
		QToolButton* senderButton = qobject_cast<QToolButton*>(sender());

		//if (m_vecToolButton[m_curClickButton] == senderButton)
		//{
		//	senderButton->setChecked(true);
		//	return;
		//}

		m_vecToolButton[m_curClickButton]->setChecked(false);
		senderButton->setChecked(true);
		m_curClickButton = senderButton->property("index").toInt() - 1;

		ui->pageTitle->setText(senderButton->text());

		ui->stackedWidget->setCurrentIndex(m_curClickButton);
	}

	void AngKSettingDlg::onSlotSaveConfig()
	{


		//prog
		if (ui->sktModeCombo->currentIndex() != m_programInfo.sktModeIdx ||
			(ui->sktModeCombo->currentIndex() == 1 && m_programInfo.sktPercent != ui->sktPercentEdit->text()) ||
			(ui->sktModeCombo->currentIndex() == 2 && m_programInfo.sktAbsolute != ui->sktAbsoluteEdit->text()))
		{

			auto ret = ACMessageBox::showWarning(this, tr("Warning"), tr("After modifying the programmer configuration, you need to restart the entire device and restart the client software to reconnect.Do you want to continue?"),
				ACMessageBox::ACMsgButton::MSG_OK_CANCEL_BTN);

			if (ret == ACMessageBox::ACMsgType::MSG_OK)
			{
				std::map<std::string, DeviceStu> insertDev;
				AngKDeviceModel::instance().GetConnetDevMap(insertDev);
				for (auto iter : insertDev) {
					nlohmann::json progSettingJson;
					progSettingJson["SKTNotifyMode"] = QString::number(ui->sktModeCombo->currentIndex()).toStdString();
					if (ui->sktModeCombo->currentIndex() == 0) {
						progSettingJson["SKTNotifyValue"] = "";
					}
					else if (ui->sktModeCombo->currentIndex() == 1) {
						progSettingJson["SKTNotifyValue"] = ui->sktPercentEdit->text().toStdString();
					}
					else {
						progSettingJson["SKTNotifyValue"] = ui->sktAbsoluteEdit->text().toStdString();
					}
					AngKMessageHandler::instance().Command_RemoteDoPTCmd(iter.second.strIP, iter.second.nHopNum, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_ProgramSetting, ALL_MU, 8, QByteArray(progSettingJson.dump().c_str()));

				}
				qApp->exit(MessageType::MESSAGE_RESTART);
			}
			else {
				return;
			}
		}

		//依次保存到配置文件中
		//Log
		AngKGlobalInstance::WriteValue("LogFile", "path", ui->logPathEdit->text());
		AngKGlobalInstance::WriteValue("LogFile", "size", ui->fileSizeEdit->text().toInt());
		AngKGlobalInstance::WriteValue("LogFile", "keepTime", ui->keepTimeEdit->text().toInt());

		//buffer
		AngKGlobalInstance::WriteValue("Buffer", "path", ui->bufferPathEdit->text());
		AngKGlobalInstance::WriteValue("Buffer", "dataFileCheck", ui->checkDataBox->isChecked());



		//Factory
		AngKGlobalInstance::WriteValue("FactoryMode", "enable", ui->FactoryEnableCheck->isChecked());
		if (ui->pwdEdit->text() == ui->confirmPwdEdit->text())
		{
			AngKGlobalInstance::WriteValue("FactoryMode", "passwd", ui->confirmPwdEdit->text().toInt());
		}
		else
		{
			ACMessageBox::showWarning(this, tr("Input error"), tr("<font size='4' color='black'>The passwords of the two factory modes are inconsistent</font>"));
			return;
		}

		int enableSum = calEnableFactory();
		AngKGlobalInstance::WriteValue("FactoryMode", "func", enableSum);

		//language
		if (m_languageGroup->checkedId() != AngKGlobalInstance::ReadValue("Language", "mode").toInt()) {
			AngKGlobalInstance::WriteValue("Language", "mode", m_languageGroup->checkedId());

			auto ret = ACMessageBox::showInformation(this, tr("Restart"), tr("<font size='4' color='black'>Some settings need to be restarted to take effect. Do you want to restart now?</font>")
				, ACMessageBox::ACMsgButton::MSG_OK_CANCEL_BTN);

			if (ret == ACMessageBox::ACMsgType::MSG_OK)
			{
				qApp->exit(MessageType::MESSAGE_RESTART);
			}
		}

		//Project

		AngKGlobalInstance::WriteValue("TaskProjectPath", "projPath", ui->projPathEdit->text());
		AngKGlobalInstance::WriteValue("TaskProjectPath", "taskPath", ui->taskPathEdit->text());


		ALOG_INFO("Save Setting Successfully!", "CU", "--");
		// usrmode
		QString pwd = ui->usrModePwdEdit->text();
		if (pwd == "ACROVIEW" || ui->usrModeCombo->currentData().toInt() != (int)UserMode::Developer) {
			curUserMode = (UserMode)ui->usrModeCombo->currentData().toInt();
			AngKGlobalInstance::instance()->WriteValue("LoginPwd", "userMode", ui->usrModeCombo->currentData().toInt());
			ALOG_INFO("Usr Mode changed, cur mode :%s", "CU", "--", ui->usrModeCombo->currentText().toStdString().c_str());
		}
		else {
			ACMessageBox::showWarning(this, QObject::tr("Warning"), QObject::tr("UserMode password error! Other config save successfully"));
			ui->usrModeCombo->setCurrentIndex((int)curUserMode - (int)(UserMode::Operator));
			return;
		}

		ACMessageBox::showInformation(this, tr("Info"), tr("Save Setting Successfully"));
		close();

	}

	void AngKSettingDlg::onSlotProgramSetting(int nMode, int nValue)
	{
		ui->sktModeCombo->setCurrentIndex(nMode);
		m_curSktMode = nMode;
		m_curSktValue = nValue;
		if (nMode == 0)
		{
			ui->sktPercentEdit->setText("0");
			ui->sktAbsoluteEdit->setText("0");
		}
		else if (nMode == 1) {
			ui->sktPercentEdit->setText(QString::number(nValue));
			ui->sktAbsoluteEdit->setText("0");
		}
		else {
			ui->sktPercentEdit->setText("0");
			ui->sktAbsoluteEdit->setText(QString::number(nValue));
		}
	}

	void AngKSettingDlg::onSlotSktComboBoxSwitch(int nIndex)
	{
		if (nIndex == 0) {
			ui->sktAbsoluteEdit->setEnabled(false);
			ui->sktPercentEdit->setEnabled(false);
		}
		else if (nIndex == 1) {
			ui->sktAbsoluteEdit->setEnabled(false);
			ui->sktPercentEdit->setEnabled(true);
		}
		else {
			ui->sktAbsoluteEdit->setEnabled(true);
			ui->sktPercentEdit->setEnabled(false);
		}
	}

}