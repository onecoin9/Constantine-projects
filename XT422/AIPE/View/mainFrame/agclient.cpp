#include "agclient.h"
#include "AngKHelper.h"
#include "StyleInit.h"
#include "AngKAboutDlg.h"
#include "AngKDebugSetting.h"
#include "ACChipManager.h"
#include "AngKMainFrame.h"
#include "AngKSettingDlg.h"
#include "AngKConnetDialog.h"
#include "GlobalDefine.h"
#include "AngkLogger.h"
#include "ACEventLogger.h"
#include "AngKMisc.h"
#include "ACAdapterCountInfomation.h"
#include "ACProgrammerTestDlg.h"
#include "AngKCommonTools.h"
#include "AngKDeviceInfoDlg.h"
#include "AngKDeviceInfoTable.h"
#include "AngKPathResolve.h"
#include "AngKMessageQueue.h"
#include "AngKTranslateSelect.h"
#include "AngKGlobalInstance.h"
#include "MessageType.h"
#include "AngKUpdateFirmware.h"
#include "AngKNandImageDlg.h"
#include "AngKProgrammerInfoDlg.h"
#include "AngKCodeGeneratorDlg.h"
#include "AngKAdapterInformation.h"
#include "AngKSNCreatorDlg.h"
#include "AngKSerialNumberDlg.h"
#include "AngKTransmitSignals.h"
#include "AngKMessageHandler.h"
#include "ACMessageBox.h"
#include "ACDeviceManagerList.h"
#include "ACDeviceManager.h"
#include "ACProjectCreateDlg.h"
#include "ACTaskCreateDlg.h"
#include "ACTaskDownload.h"
#include "ACAutomaticSetting.h"
#include "ACMasterChipAnalyzeDlg.h"
#include <QPushButton>
#include <QBitmap>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QStorageInfo>
#include <QtConcurrent>
#include "../RemoteServer/JsonRpcServer.h"
namespace AcroView
{


	QMap<QString, int> UartComboboxIndexMap;
	QMap<QString, int> LogLevelComboboxIndexMap;

	AGClient::AGClient(QWidget* parent)
		: AngKShadowWindow<QMainWindow>(parent, true)
		, m_clientWgt(nullptr)
		, m_menuBar(nullptr)
		, m_AngKSettingDlg(nullptr)
		, m_transSelect(nullptr)
		, m_pDataset(nullptr)
		, m_pLogManager(nullptr)
		, m_pRemoteCmdManager(nullptr)
		, m_bExitFlag(false)
		, m_bInitFlag(false)
		, m_DataBuffer(std::make_shared<AngKDataBuffer>(this))
		, m_pTaskManager(std::make_shared<ACTaskManager>(this))
	{
		AngKGlobalInstance::setMainWindow(this);
		
		//InitLogManager();

		setCentralWidget(rootWidget());
		clientWidget()->setObjectName("rootWidget");
		this->resize(1680, 1050);

		this->setObjectName("AGClient");
		QT_SET_STYLE_SHEET(objectName());

		m_pLogManager = AngKLogManager::instancePtr();
		m_pRemoteCmdManager = new AngKRemoteCmdManager();
		m_pRemoteCmdManager->SetCmdInterval(1000);

		auto initUartComboboxIndexMap = []() {
			QMap<QString, int> tmpMap;
			for (auto iter : ACDeviceManager::instance().getAllDevInfo()) {
				if (!iter.bOnLine)
					continue;
				QString ipHop = QString::fromStdString(iter.strIP) + ":" + QString::number(iter.nHopNum);
				int idx = UartComboboxIndexMap.find(ipHop) == UartComboboxIndexMap.end() ? -1 : UartComboboxIndexMap.find(ipHop).value();
				tmpMap.insert(ipHop, idx);
			}
			UartComboboxIndexMap = tmpMap;
		};
		initUartComboboxIndexMap();
		QObject::connect(&ACDeviceManager::instance(), &ACDeviceManager::devStateChanged, initUartComboboxIndexMap);
		QObject::connect(&ACDeviceManager::instance(), &ACDeviceManager::devOffLine, initUartComboboxIndexMap);
		QObject::connect(&ACDeviceManager::instance(), &ACDeviceManager::devOnLine, initUartComboboxIndexMap);
		QObject::connect(JsonRpcServer::Instance(),&JsonRpcServer::sgnLoadProjectFile2UI,this,&AGClient::onSlotActionDownloadTaskJsonRpc);


	}

	AGClient::~AGClient()
	{
		qDebug() << "is First? ~AGClient()";
		//使用完成删除Temp文件夹
		QString tempPath = Utils::AngKPathResolve::localTempFolderPath();
		Utils::AngKPathResolve::DeleteDirectory(tempPath);

		SAFEDEL(m_pDataset);
		SAFEDEL(m_transSelect);
		SAFEDEL(m_AngKSettingDlg);
		SAFEDEL(m_menuBar);
		SAFEDEL(m_clientWgt);

		//ExitLogWrite();
	}

	void AGClient::InitTitleBar()
	{
		layoutTitleBar();

		AngKWindowTitleBar* title = titleBar();
		title->setFixedHeight(50);
		//title->setPalette(QPalette(QColor(255, 255, 255)));
		title->setContentsMargins(QMargins(0, 0, 0, 0));
		title->setShowTitleComponent(true);

		QDesktopWidget* m = QApplication::desktop();
		QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
		this->move(desk_rect.width() / 2 - this->width() / 2 + desk_rect.left(), desk_rect.height() / 2 - this->height() / 2 + desk_rect.top());
		show();
	}

	void AGClient::InitClient()
	{
		InitTitleBar();
		m_pDataset = new AngKProjDataset(this);
		//GetBPUInfo();
		//TestJson();

		QStorageInfo stoInfo(Utils::AngkLogger::logger()->logFileName());
		quint64 bytefree = stoInfo.bytesFree();
		if (stoInfo.bytesFree() < AngKGlobalInstance::ReadValue("LogFile", "size").toLongLong() * 1024) {
			ACMessageBox::showWarning(this, tr("Notify"), tr("The current drive letter space is insufficient to record logs."));
		}

		AngKConnetDialog conDlg(this);
		//该信号每次选择芯片之后每次会更新pDataset数据集
		connect(&conDlg, &AngKConnetDialog::sgnCloseWindow, this, &AGClient::onChildWindowClosed);
		conDlg.exec();

		ALOG_INFO("Client select %s mode.", "CU", "--", Utils::AngKCommonTools::GetModeStr(g_AppMode).c_str());

		QWidget* main = clientWidget();
		//ALOG_INFO("<================ AngKMainFrame Init ================>", "CU");
		m_clientWgt = new AngKMainFrame(main, m_pDataset, m_pTaskManager);
		m_clientWgt->setDataBuffer(m_DataBuffer.get());
		m_clientWgt->SetRemoteManager(m_pRemoteCmdManager);
		m_clientWgt->SetDeviceModel();
		InitMenuBar();
		//ALOG_INFO("<================ AngKMainFrame End ================>", "CU");

		m_AngKSettingDlg = new AngKSettingDlg(this);
		connect(m_clientWgt, &AngKMainFrame::sgnProgramSetting, m_AngKSettingDlg, &AngKSettingDlg::onSlotProgramSetting);
		clientLayout()->addWidget(m_menuBar);
		clientLayout()->addWidget(m_clientWgt);

		m_clientWgt->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

		bool bDark = AngKGlobalInstance::ReadValue("Skin", "mode").toInt() == (int)ViewMode::Dark ? true : false;
		m_transSelect = new AngKTranslateSelect(m_menuBar, bDark);
		m_transSelect->move(m_menuBar->width() - 225, 2);
		m_transSelect->setCheckState((TranslateLanguage)AngKGlobalInstance::ReadValue("Language", "mode").toInt());

		connect(m_pLogManager, &AngKLogManager::readyWrite, m_clientWgt, &AngKMainFrame::onSlotLogWriteUI, Qt::QueuedConnection);
		AngKLogManager::instancePtr()->setBufferConnection();


		// 隐藏Programmer、Site
		m_clientWgt->setPropetryAreaShow(WinActionType::Programmer, false);
		m_clientWgt->setPropetryAreaShow(WinActionType::Site, false);
	}

	void AGClient::InitMenuBar()
	{
		m_menuBar = new QMenuBar(this);
		m_menuBar->setObjectName("AGClientMenuBar");
		m_menuBar->setFixedHeight(37);
		m_menuBar->setMinimumWidth(800);
		setMenuBar(m_menuBar);

		InitTaskMenu();
		InitProjectMenu();
		InitSettingMenu();
		InitAutomaticMenu();
		InitUtilitiesMenu();
		InitWindowMenu();
		InitViewMenu();
		InitHelpMenu();

		if (AngKGlobalInstance::ReadValue("LoginPwd", "userMode").toInt() >= (int)UserMode::Developer) {
			InitDebugMenu();
		}
	}


	void AGClient::InitProjectMenu()
	{
		QMenu* projMenu = new QMenu(ANGKACTION_MENU_PROJ, m_menuBar);
		projMenu->setObjectName("ProjectMenu");
		SetMenuBackGround(projMenu);
		projMenu->setProperty("customProperty", "custom_1");

		QAction* createAction = new QAction(tr("Create..."), projMenu);
		createAction->setObjectName("createProjectAction");
		projMenu->addAction(createAction);
		connect(createAction, &QAction::triggered, this, &AGClient::onSlotActionCreate);

		//QAction* OpenAction = new QAction(tr("Open..."), projMenu);
		//projMenu->addAction(OpenAction);
		//connect(OpenAction, &QAction::triggered, m_clientWgt, &AngKMainFrame::onSlotActionOpenProj);

		//QAction* SaveAction = new QAction(tr("Save..."), projMenu);
		//projMenu->addAction(SaveAction);

		//QAction* DownLoadAction = new QAction(tr("Download..."), projMenu);
		//projMenu->addAction(DownLoadAction);

		m_menuBar->addMenu(projMenu);
	}

	void AGClient::InitTaskMenu()
	{
		QMenu* taskMenu = new QMenu(ANGKACTION_MENU_FILE, m_menuBar);
		taskMenu->setObjectName("TaskMenu");
		SetMenuBackGround(taskMenu);
		taskMenu->setProperty("customProperty", "custom_1");

		QAction* createTaskAction = new QAction(tr("Create..."), taskMenu);
		createTaskAction->setObjectName("createTaskAction");
		taskMenu->addAction(createTaskAction);
		connect(createTaskAction, &QAction::triggered, this, &AGClient::onSlotActionCreateTask);

		QAction* openAction = new QAction(tr("Download..."), taskMenu);
		openAction->setObjectName("downloadTaskAction");
		taskMenu->addAction(openAction);
		connect(openAction, &QAction::triggered, this, &AGClient::onSlotActionDownloadTask);

		m_menuBar->addMenu(taskMenu);
	}

	void AGClient::InitUtilitiesMenu()
	{
		QMenu* UtilitiesMenu = new QMenu(ANGKACTION_MENU_UTIL, m_menuBar);
		UtilitiesMenu->setObjectName("UtilitiesMenu");
		SetMenuBackGround(UtilitiesMenu);
		UtilitiesMenu->setProperty("customProperty", "custom_1");

		//QAction* serialAction = new QAction(tr("Serial Number"), UtilitiesMenu);
		//UtilitiesMenu->addAction(serialAction);
		//connect(serialAction, &QAction::triggered, this, [=]()
		//	{
		//		AngKSerialNumberDlg dlg(this);
		//		dlg.exec();
		//	});

		QAction* firmwareAction = new QAction(tr("Update Firmware"), UtilitiesMenu);
		firmwareAction->setObjectName("firmwareAction");
		UtilitiesMenu->addAction(firmwareAction);
		connect(firmwareAction, &QAction::triggered, this, &AGClient::onSlotActionUpdateFirmware);

		//QAction* factoryAction = new QAction(tr("Factory mode unprotect"), UtilitiesMenu);
		//UtilitiesMenu->addAction(factoryAction);

		//QAction* generatorAction = new QAction(tr("Check Code Generator"), UtilitiesMenu);
		//UtilitiesMenu->addAction(generatorAction);
		//connect(generatorAction, &QAction::triggered, this, [=]()
		//	{
		//		AngKCodeGeneratorDlg dlg(this);
		//		dlg.exec();
		//	});

		QAction* masterChipAction = new QAction(tr("MasterChip Analyze"), UtilitiesMenu);
		masterChipAction->setObjectName("masterChipAction");
		UtilitiesMenu->addAction(masterChipAction);
		connect(m_clientWgt, &AngKMainFrame::sgnHandleEventTransmitChipIDFetched,
			&AngKTransmitSignals::GetInstance(), &AngKTransmitSignals::sigHandleEventTransmitChipIDFetched);
		connect(m_clientWgt, &AngKMainFrame::sgnHandleEventTransmitExtCSDFetched,
			&AngKTransmitSignals::GetInstance(), &AngKTransmitSignals::sigHandleEventTransmitExtCSDFetched);
		connect(masterChipAction, &QAction::triggered, this, [=]()
			{
				ACMasterChipAnalyzeDlg dlg(this); 
				connect(m_clientWgt, &AngKMainFrame::sgnMasterChipAnalyzeResult, &dlg, &ACMasterChipAnalyzeDlg::onSlotMasterChipAnalyzeResult);
				connect(m_clientWgt, &AngKMainFrame::sgnHandleEventAnalyzeResult, &dlg, &ACMasterChipAnalyzeDlg::onSlotHandleEventAnalyzeResult);
				connect(m_clientWgt, &AngKMainFrame::sgnHandleEventExtCSDFetched, &dlg, &ACMasterChipAnalyzeDlg::onSlotHandleEventExtCSDFetched);
				connect(m_clientWgt, &AngKMainFrame::sgnHandleEventAnalyzeInfo, &dlg, &ACMasterChipAnalyzeDlg::onSlotHandleEventAnalyzeInfo);
				connect(m_clientWgt, &AngKMainFrame::sgnHandleEventAnalyzeStatusChange, &dlg, &ACMasterChipAnalyzeDlg::onSlotHandleEventAnalyzeStatusChange);
				connect(m_clientWgt, &AngKMainFrame::sgnHandleEventUIDFetched, &dlg, &ACMasterChipAnalyzeDlg::onSlotHandleEventUIDFetched);
				connect(m_clientWgt, &AngKMainFrame::sgnHandleEventChipIDFetched, &dlg, &ACMasterChipAnalyzeDlg::onSlotHandleEventChipIDFetched);
				connect(m_clientWgt, &AngKMainFrame::sgnUpdateAnalyzeValue, &dlg, &ACMasterChipAnalyzeDlg::onSlotGetProgress);
				connect(&dlg, &ACMasterChipAnalyzeDlg::sgnSetAnalyzeFlag, this, [=](bool bFlag) {
					m_clientWgt->SetAnalyzeFlag(bFlag);
				});
				dlg.exec();
			});

		UtilitiesMenu->addSeparator();

		//QAction* progInfoAction = new QAction(tr("Programmer Info"), UtilitiesMenu);
		//UtilitiesMenu->addAction(progInfoAction);
		//connect(progInfoAction, &QAction::triggered, this, [=]()
		//	{
		//		AngKProgrammerInfoDlg dlg(this);
		//		dlg.exec();
		//	});

		QAction* progTestAction = new QAction(tr("Programmer Test"), UtilitiesMenu);
		progTestAction->setObjectName("progTestAction");
		UtilitiesMenu->addAction(progTestAction);
		connect(progTestAction, &QAction::triggered, this, [=]()
			{
				ACProgrammerTestDlg dlg(this);
				connect(&dlg, &ACProgrammerTestDlg::sgnProgrammgerTest, m_clientWgt, &AngKMainFrame::onSlotProgrammgerTest);
				connect(m_clientWgt, &AngKMainFrame::sgnProgramSelfTestResult, &dlg, &ACProgrammerTestDlg::onSlotProgramSelfTestResult);
				dlg.exec();
			});

		//UtilitiesMenu->addSeparator();

		//QAction* loaderAction = new QAction(tr("Nand Partition Loader"), UtilitiesMenu);
		//UtilitiesMenu->addAction(loaderAction);

		//QAction* imageAction = new QAction(tr("Nand Image"), UtilitiesMenu);
		//UtilitiesMenu->addAction(imageAction);
		//connect(imageAction, &QAction::triggered, this, [=]()
		//	{
		//		AngKNandImageDlg dlg(this);
		//		dlg.exec();
		//	});

		//UtilitiesMenu->addSeparator();

		//QAction* SNCAction = new QAction(tr("SNC Creator"), UtilitiesMenu);
		//UtilitiesMenu->addAction(SNCAction);
		//connect(SNCAction, &QAction::triggered, this, [=]()
		//	{
		//		AngKSNCreatorDlg dlg(this);
		//		dlg.exec();
		//	});

		UtilitiesMenu->addSeparator();

		QAction* restartAction = new QAction(tr("Restart Software"), UtilitiesMenu);
		restartAction->setObjectName("restartAction");
		UtilitiesMenu->addAction(restartAction);
		connect(restartAction, &QAction::triggered, this, [=]()
			{
				auto ret = ACMessageBox::showWarning(this, tr("Warning"), tr("Do you want to restart now ?"),
					ACMessageBox::ACMsgButton::MSG_OK_CANCEL_BTN);

				if (ret == ACMessageBox::ACMsgType::MSG_OK)
				{
					qApp->exit(MessageType::MESSAGE_RESTART);
				}
			});

		QAction* ExitAction = new QAction(tr("Exit"), UtilitiesMenu);
		ExitAction->setObjectName("ExitAction");
		UtilitiesMenu->addAction(ExitAction);
		connect(ExitAction, &QAction::triggered, this, [=]()
			{
				auto ret = ACMessageBox::showWarning(this, tr("Warning"), tr("Do you want to Exit now ?"),
					ACMessageBox::ACMsgButton::MSG_OK_CANCEL_BTN);

				if (ret == ACMessageBox::ACMsgType::MSG_OK)
				{
					qApp->exit(MessageType::MESSAGE_EXIT);
				}
			});



		m_menuBar->addMenu(UtilitiesMenu);
	}

	void AGClient::InitWindowMenu()
	{
		QMenu* windowMenu = new QMenu(ANGKACTION_MENU_WINDOW, m_menuBar);
		windowMenu->setObjectName("WindowMenu");
		SetMenuBackGround(windowMenu);
		windowMenu->setProperty("customProperty", "custom_1");

		QAction* workAction = new QAction(tr("Work Space"), windowMenu);
		workAction->setObjectName("windowWorkAction");
		windowMenu->addAction(workAction);

		QAction* toolAction = new QAction(tr("Toolbar"), windowMenu);
		toolAction->setObjectName("windowToolAction");
		windowMenu->addAction(toolAction);

		windowMenu->addSeparator();

		QAction* winLogAction = new QAction(tr("Log"), windowMenu);
		winLogAction->setObjectName("windowLogAction");
		winLogAction->setCheckable(true);
		winLogAction->setChecked(AngKGlobalInstance::ReadValue("Windows", "Log", false).toBool());
		winLogAction->setProperty("winType", (int)WinActionType::Log);
		windowMenu->addAction(winLogAction);

		QAction* projAction = new QAction(tr("Task"), windowMenu);
		projAction->setObjectName("windowTaskAction");
		projAction->setCheckable(true);
		projAction->setChecked(AngKGlobalInstance::ReadValue("Windows", "Project", true).toBool());
		projAction->setProperty("winType", (int)WinActionType::Project);
		windowMenu->addAction(projAction);

		//QAction* progAction = new QAction(tr("Programmer"), windowMenu);
		//progAction->setCheckable(true);
		//progAction->setChecked(AngKGlobalInstance::ReadValue("Windows", "Programmer", true).toBool());
		//progAction->setProperty("winType", (int)WinActionType::Programmer);
		//windowMenu->addAction(progAction);

		//QAction* siteAction = new QAction(tr("Site"), windowMenu);
		//siteAction->setCheckable(true);
		//siteAction->setChecked(AngKGlobalInstance::ReadValue("Windows", "Site", true).toBool());
		//siteAction->setProperty("winType", (int)WinActionType::Site);
		//windowMenu->addAction(siteAction);

		connect(winLogAction, &QAction::triggered, this, &AGClient::onSlotActionWindow);
		connect(projAction, &QAction::triggered, this, &AGClient::onSlotActionWindow);
		//connect(progAction, &QAction::triggered, this, &AGClient::onSlotActionWindow);
		//connect(siteAction, &QAction::triggered, this, &AGClient::onSlotActionWindow);

		QTimer::singleShot(500, this, [this, winLogAction, projAction/*, progAction, siteAction*/]() {
			m_clientWgt->showLogArea(winLogAction->isChecked());
			//m_clientWgt->setPropetryAreaShow(WinActionType::Programmer, progAction->isChecked());
			//m_clientWgt->setPropetryAreaShow(WinActionType::Site, siteAction->isChecked()); 
			m_clientWgt->setPropetryAreaShow(WinActionType::Project, projAction->isChecked());
			});

		m_menuBar->addMenu(windowMenu);
	}

	void AGClient::InitViewMenu()
	{
		return;
		QMenu* viewMenu = new QMenu(ANGKACTION_MENU_VIEW, m_menuBar);
		viewMenu->setObjectName("ViewMenu");
		SetMenuBackGround(viewMenu); 
		viewMenu->setProperty("customProperty", "custom_1");

		QActionGroup* actGroup = new QActionGroup(this);

		QAction* lightAction = new QAction(tr("Light Mode"), viewMenu);
		lightAction->setObjectName("lightAction");
		lightAction->setProperty("Mode", (int)ViewMode::Light);
		//QAction* darkAction = new QAction(tr("Dark Mode"), viewMenu);
		//darkAction->setObjectName("darkAction");
		//darkAction->setProperty("Mode", (int)ViewMode::Dark);

		viewMenu->addAction(actGroup->addAction(lightAction));
		//viewMenu->addAction(actGroup->addAction(darkAction));

		lightAction->setCheckable(true);
		//darkAction->setCheckable(true);
		//bool bDark = AngKGlobalInstance::ReadValue("Skin", "mode").toInt() == (int)ViewMode::Dark ? true : false;
		//bDark ? darkAction->setChecked(true) : lightAction->setChecked(true);

		connect(lightAction, &QAction::triggered, this, &AGClient::onSlotActionView);
		//connect(darkAction, &QAction::triggered, this, &AGClient::onSlotActionView);

		m_menuBar->addMenu(viewMenu);
	}

	void AGClient::InitSettingMenu()
	{
		QMenu* settingMenu = new QMenu(ANGKACTION_MENU_SETTING, m_menuBar);
		settingMenu->setObjectName("SettingMenu");
		SetMenuBackGround(settingMenu); 
		settingMenu->setProperty("customProperty", "custom_1");

		QAction* settingLogAction = new QAction(tr("Log"), settingMenu);
		settingLogAction->setObjectName("settingLogAction");
		settingLogAction->setProperty("index", 1);
		settingMenu->addAction(settingLogAction);

		QAction* bufferAction = new QAction(tr("Buffer"), settingMenu);
		bufferAction->setObjectName("bufferAction");
		bufferAction->setProperty("index", 2);
		settingMenu->addAction(bufferAction);

		QAction* settingProgAction = new QAction(tr("Programmer"), settingMenu);
		settingProgAction->setObjectName("settingProgAction");
		settingProgAction->setProperty("index", 3);
		settingMenu->addAction(settingProgAction);

		QAction* factoryAction = new QAction(tr("Factory Mode"), settingMenu);
		factoryAction->setObjectName("factoryAction");
		factoryAction->setProperty("index", 4);
		//settingMenu->addAction(factoryAction);

		QAction* languageAction = new QAction(tr("Language"), settingMenu);
		languageAction->setObjectName("languageAction");
		languageAction->setProperty("index", 5);
		settingMenu->addAction(languageAction);

		QAction* pwdAction = new QAction(tr("Login Password"), settingMenu);
		pwdAction->setObjectName("pwdAction");
		pwdAction->setProperty("index", 6);
		//settingMenu->addAction(pwdAction);

		QAction* projAction = new QAction(tr("Task/Project Setting"), settingMenu);
		projAction->setObjectName("projAction");
		projAction->setProperty("index", 7);
		settingMenu->addAction(projAction);

		//QAction* skinAction = new QAction(tr("Skin"), settingMenu);
		//skinAction->setObjectName("skinAction");
		//skinAction->setProperty("index", 8);
		//settingMenu->addAction(skinAction);

		QAction* usrModeAction = new QAction(tr("UsrMode"), settingMenu);
		usrModeAction->setObjectName("usrModeAction");
		usrModeAction->setProperty("index", 8);
		settingMenu->addAction(usrModeAction);

		connect(settingLogAction, &QAction::triggered, this, &AGClient::onSlotActionSetting);
		connect(bufferAction, &QAction::triggered, this, &AGClient::onSlotActionSetting);
		connect(settingProgAction, &QAction::triggered, this, &AGClient::onSlotActionSetting);
		connect(factoryAction, &QAction::triggered, this, &AGClient::onSlotActionSetting);
		connect(languageAction, &QAction::triggered, this, &AGClient::onSlotActionSetting);
		connect(pwdAction, &QAction::triggered, this, &AGClient::onSlotActionSetting);
		connect(projAction, &QAction::triggered, this, &AGClient::onSlotActionSetting);
		//connect(skinAction, &QAction::triggered, this, &AGClient::onSlotActionSetting);
		connect(usrModeAction, &QAction::triggered, this, &AGClient::onSlotActionSetting);

		m_menuBar->addMenu(settingMenu);
	}

	void AGClient::InitAutomaticMenu()
	{
		QMenu* autoMenu = new QMenu(ANGKACTION_MENU_AUTOMATIC, m_menuBar);
		autoMenu->setObjectName("AutoMenu");
		SetMenuBackGround(autoMenu);
		autoMenu->setProperty("customProperty", "custom_1");

		QAction* connectAction = new QAction(tr("Connect"), autoMenu);
		connectAction->setObjectName("connectAutoAction");
		autoMenu->addAction(connectAction);

		connect(connectAction, &QAction::triggered, this, [=]() {
			ACAutomaticSetting automaticDlg(this);
			//connect(&automaticDlg, &ACAutomaticSetting::sgnAutoTellDevReady, m_clientWgt, &AngKMainFrame::onSlotAutoTellDevReady);
			connect(&automaticDlg, &ACAutomaticSetting::sgnAutoTellDevReady, this, [this](int nPassLot) {
				QtConcurrent::run([this, nPassLot]() {
					this->m_clientWgt->onSlotAutoTellDevReady(nPassLot);
					});
				});
			connect(&automaticDlg, &ACAutomaticSetting::sgnClearLotData, m_clientWgt, &AngKMainFrame::onSlotClearLotData);
			connect(m_clientWgt, &AngKMainFrame::sgnAutomicOver, &automaticDlg, &ACAutomaticSetting::onSlotAutomicOver);


			connect(&automaticDlg, &ACAutomaticSetting::sgnCheckSitesTskStatus, m_clientWgt, &AngKMainFrame::onSlotAutomaticCheckSitesTask, Qt::DirectConnection);
			connect(m_clientWgt, &AngKMainFrame::sgnContinueRunAutomatic, &automaticDlg, &ACAutomaticSetting::SetSitesTskPass, Qt::DirectConnection);

			if (automaticDlg.IsPluginLoad())
				automaticDlg.exec();
			});




		QAction* disconnectAction = new QAction(tr("Disconnect"), autoMenu);
		disconnectAction->setObjectName("disconnectAutoAction");
		autoMenu->addAction(disconnectAction);

		m_menuBar->addMenu(autoMenu);
	}

	void AGClient::InitHelpMenu()
	{
		QMenu* helpMenu = new QMenu(ANGKACTION_MENU_HELP, m_menuBar);
		helpMenu->setObjectName("HelpMenu");
		SetMenuBackGround(helpMenu);
		helpMenu->setProperty("customProperty", "custom_1");

		QAction* aboutAction = new QAction(tr("About"), helpMenu);
		aboutAction->setObjectName("aboutAction");
		helpMenu->addAction(aboutAction);

		connect(aboutAction, &QAction::triggered, this, [=]() {
			AngKAboutDlg aboutDlg(this);
			aboutDlg.exec();
			});

		QAction* helpAction = new QAction(tr("Chip Help"), helpMenu);
		helpAction->setObjectName("helpAction");
		helpMenu->addAction(helpAction);

		QAction* chipInfoAction = new QAction(tr("Chip Information"), helpMenu);
		chipInfoAction->setObjectName("chipInfoAction");
		helpMenu->addAction(chipInfoAction);

		QAction* devInfoAction = new QAction(tr("Device Information"), helpMenu);
		devInfoAction->setObjectName("devInfoAction");
		helpMenu->addAction(devInfoAction);

		connect(chipInfoAction, &QAction::triggered, this, [=]() {
				if (m_pDataset == NULL || m_pDataset->getChipData().json2String() == "null") {
					return;
				}
				AngKDeviceInfoDlg devDlg(this);
				devDlg.SetChipDeviceInfo(m_pDataset->getChipData());
				devDlg.exec();
			});

		connect(devInfoAction, &QAction::triggered, this, [=]() {
			ACDeviceManagerList devList(this);
			//connect(m_clientWgt, &AngKMainFrame::sgnProgramSettingComplete, &devList , &ACDeviceManagerList::onSlotProgramSettingComplete);
			devList.exec();
			//AngKDeviceInfoTable devTable(this);
			//devTable.exec();
			});

		QAction* adapterAction = new QAction(tr("Adapter Information"), helpMenu);
		adapterAction->setObjectName("adapterAction");
		helpMenu->addAction(adapterAction);
		connect(adapterAction, &QAction::triggered, this, [=]()
			{
				ACAdapterCountInfomation dlg(m_clientWgt);
				connect(&dlg, &ACAdapterCountInfomation::sgnQueryAdapterInfo, m_clientWgt, &AngKMainFrame::onSlotQueryAdapterInfo);
				connect(m_clientWgt, &AngKMainFrame::sgnShowSktInfo, &dlg, &ACAdapterCountInfomation::onSlotShowSktInfo);
				connect(m_clientWgt, &AngKMainFrame::sgnShowSktInfoSimple, &dlg, &ACAdapterCountInfomation::onSlotShowSktInfoSimple);
				dlg.exec();
				disconnect(&dlg, &ACAdapterCountInfomation::sgnQueryAdapterInfo, m_clientWgt, &AngKMainFrame::onSlotQueryAdapterInfo);
				disconnect(m_clientWgt, &AngKMainFrame::sgnShowSktInfo, &dlg, &ACAdapterCountInfomation::onSlotShowSktInfo);
				disconnect(m_clientWgt, &AngKMainFrame::sgnShowSktInfoSimple, &dlg, &ACAdapterCountInfomation::onSlotShowSktInfoSimple);
			});

		m_menuBar->addMenu(helpMenu);
	}

	void AGClient::InitDebugMenu()
	{
		QMenu* debugMenu = new QMenu(ANGKACTION_MENU_DEBUG, m_menuBar);
		debugMenu->setObjectName("DebugMenu");
		SetMenuBackGround(debugMenu);
		debugMenu->setProperty("customProperty", "custom_1");

		QAction* switchUrctAction = new QAction(tr("Debugging setting"), debugMenu);
		switchUrctAction->setObjectName("switchUrctAction");
		debugMenu->addAction(switchUrctAction);

		connect(switchUrctAction, &QAction::triggered, this, [=]() {
			AngKDebugSetting debugDlg(&UartComboboxIndexMap, &LogLevelComboboxIndexMap, this);
			debugDlg.exec();
			});

		m_menuBar->addMenu(debugMenu);
	}

	void AGClient::resizeEvent(QResizeEvent* event)
	{
		if (m_transSelect)
		{
			m_transSelect->move(m_menuBar->width() - 225, 1);
		}

		if (m_clientWgt)
		{
			m_clientWgt->MoveLogArea(event->size());
		}

		QWidget::resizeEvent(event);
	}

	void AGClient::SetMenuBackGround(QMenu* _menu)
	{
		_menu->setWindowFlag(Qt::FramelessWindowHint);
		_menu->setAttribute(Qt::WA_TranslucentBackground);
		_menu->setWindowFlag(Qt::NoDropShadowWindowHint);
	}

	void AGClient::GetBPUInfo()
	{
		//初始化建立连接并获取AG06设备烧录座信息
		if (AngKDeviceModel::instance().GetConnetDevMapSize() > 0 && !m_bInitFlag) {

			std::map<std::string, DeviceStu> insertDev;
			AngKDeviceModel::instance().GetConnetDevMap(insertDev);
			AngKMessageHandler::instance().Command_Initialize(this);

			//QEventLoop loop;
			for (auto iter : insertDev) {
				connect(AngKMessageHandler::instance().GetACCmdHandler(), &CACCmdHandler::sigRemoteCmdComplete, this, &AGClient::onSlotAGClientRemoteQueryDoCmd);
				AngKMessageHandler::instance().Command_RemoteDoPTCmd(iter.second.strIP, iter.second.nHopNum, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_GetBPUInfo, BPU0_SKT0, 8, QByteArray());
			}
			//AngKMessageHandler::instance().Command_LinkScan(0);
			m_bInitFlag = true;
		}
	}


	void AGClient::TestJson()
	{
		QFile jsonFile(QCoreApplication::applicationDirPath() + "/BPUInfo.json");
		if (!jsonFile.open(QIODevice::ReadOnly)) {
			return;
		}

		QByteArray dataInfo = jsonFile.readAll();
		// 去除制表符和换行符
		//dataInfo.replace("\t", ""); dataInfo.replace("\n", "");
		//dataInfo.replace("\r", ""); dataInfo.replace("\\\"", "\"");
		try {//nlohmann解析失败会报异常需要捕获一下
			std::string strInfo = QString::fromUtf8(dataInfo).toStdString();
			nlohmann::json BPUInfoJson = nlohmann::json::parse(dataInfo);
			BPUInfo infos;
			infos.strBPUEn = BPUInfoJson["BPUEn"];
			int bpuCount = BPUInfoJson["BPUInfo"].size();
			for (int i = 0; i < bpuCount; ++i)
			{
				nlohmann::json singleJson = BPUInfoJson["BPUInfo"][i];
				SingleBPU sBPU;
				sBPU.idx = singleJson["BPUIdx"];
				sBPU.SktCnt = singleJson["SKTCnt"];
				sBPU.AdapterID = singleJson["AdapterID"];
				infos.vecInfos.push_back(sBPU);
			}

			m_pDataset->SetBPUInfo("192.168.11.1", infos);

		}
		catch (const nlohmann::json::exception& e) {
			ALOG_FATAL("GetBPUInfo Json parse failed : %s.", "CU", "--", e.what());
		}
	}

	void AGClient::onSlotActionWindow(bool state)
	{
		QAction* clickAction = qobject_cast<QAction*>(sender());

		if (!state)
			clickAction->setChecked(false);
		else
			clickAction->setChecked(true);

		switch ((WinActionType)clickAction->property("winType").toInt())
		{
			case WinActionType::Log:
				AngKGlobalInstance::WriteValue("Windows", "Log", state);
				m_clientWgt->showLogArea(state);
				break;
			case WinActionType::Project:
				AngKGlobalInstance::WriteValue("Windows", "Project", state);
				m_clientWgt->setPropetryAreaShow((WinActionType)clickAction->property("winType").toInt(), state);
				break;
			case WinActionType::Programmer:
				AngKGlobalInstance::WriteValue("Windows", "Programmer", state);
				m_clientWgt->setPropetryAreaShow((WinActionType)clickAction->property("winType").toInt(), state);
				break;
			case WinActionType::Site:
				AngKGlobalInstance::WriteValue("Windows", "Site", state);
				m_clientWgt->setPropetryAreaShow((WinActionType)clickAction->property("winType").toInt(), state);
				break;
			default:
				break;
		}
	}

	void AGClient::onSlotActionView()
	{
		QAction* clickAction = qobject_cast<QAction*>(sender());
		
		switch ((ViewMode)clickAction->property("Mode").toInt())
		{
		case ViewMode::Light:
			AngKGlobalInstance::WriteValue("Skin", "mode", (int)ViewMode::Light);
			break;
		case ViewMode::Dark:
			AngKGlobalInstance::WriteValue("Skin", "mode", (int)ViewMode::Dark);
			break;
		default:
			break;
		}

		auto ret = ACMessageBox::showWarning(this, tr("Warning"), tr("Do you want to restart now ?"),
			ACMessageBox::ACMsgButton::MSG_OK_CANCEL_BTN);

		if (ret == ACMessageBox::ACMsgType::MSG_OK)
		{
			qApp->exit(MessageType::MESSAGE_RESTART);
		}
	}

	void AGClient::onSlotActionSetting()
	{
		QAction* clickAction = qobject_cast<QAction*>(sender());
		//打开设置弹框
		if (m_AngKSettingDlg->isHidden())
		{
			m_AngKSettingDlg->OpenActionPage(clickAction->property("index").toInt());
			m_AngKSettingDlg->show();
			m_AngKSettingDlg->raise();
		}
	}

	void AGClient::onSlotAGClientRemoteQueryDoCmd(QString recvIP, uint32_t HopNum, uint32_t PortID, uint32_t ResultCode, uint16_t CmdID, uint32_t CmdFlag, uint16_t nBPUID, QByteArray CmdDataBytes, uint32_t RespDataSize)
	{
		//ALOG_INFO("Client Recv RemoteQueryDoCmd from %s, HopNum:%d, CmdID:0x%02X, Result:%d", "CU", recvIP.toStdString().c_str(), HopNum, CmdID, ResultCode);
		if ((eSubCmdID)CmdID == eSubCmdID::SubCmd_MU_GetBPUInfo){
			std::string strInfo = std::string(CmdDataBytes.constData(), CmdDataBytes.size());

			try {//nlohmann解析失败会报异常需要捕获一下

				nlohmann::json BPUInfoJson = nlohmann::json::parse(strInfo);
				BPUInfo infos;
				infos.strBPUEn = BPUInfoJson["BPUEn"];
				int bpuCount = BPUInfoJson["BPUInfo"].size();
				for (int i = 0; i < bpuCount; ++i)
				{
					nlohmann::json singleJson = BPUInfoJson["BPUInfo"][i];
					SingleBPU sBPU;
					sBPU.idx = singleJson["BPUIdx"];
					sBPU.SktCnt = singleJson["SKTCnt"];
					sBPU.AdapterID = singleJson["AdapterID"];
					infos.vecInfos.push_back(sBPU);
				}

				m_pDataset->SetBPUInfo(recvIP.toStdString(), infos);
				
			}
			catch (const nlohmann::json::exception& e) {
				ALOG_FATAL("GetBPUInfo Json parse failed : %s.", "CU", "--", e.what());
			}

			QObject::disconnect(AngKMessageHandler::instance().GetACCmdHandler(), &CACCmdHandler::sigRemoteCmdComplete, this, &AGClient::onSlotAGClientRemoteQueryDoCmd);
		}
	}

	void AGClient::onChildWindowClosed()
	{
		m_bExitFlag = true;
	}

	void AGClient::onSlotActionCreateTask()
	{
		ACTaskCreateDlg taskDlg(this, m_pTaskManager);
		taskDlg.exec();
	}

	void AGClient::onSlotActionDownloadTask()
	{
		if (m_clientWgt->GetBurnStatus()) {
			ACMessageBox::showWarning(this, tr("Warning"), tr("Please try after the programming is complete."));
			return;
		}

		ACTaskDownload taskDownDlg(this, m_pTaskManager);
		connect(&taskDownDlg, &ACTaskDownload::sgnDownloadProject, m_clientWgt, &AngKMainFrame::onSlotDownloadProject, Qt::DirectConnection);
		connect(&taskDownDlg, &ACTaskDownload::sgnCheckDevVFiles, m_clientWgt, &AngKMainFrame::onSlotCheckDevVFiles, Qt::DirectConnection);

		connect(m_clientWgt, &AngKMainFrame::sgnProjectValue, &taskDownDlg, &ACTaskDownload::onSlotDownloadProgress);
		connect(&taskDownDlg, &ACTaskDownload::sgnUpdateTaskInfo, m_clientWgt, &AngKMainFrame::onSlotUpdateTaskInfo);
		connect(&taskDownDlg, &ACTaskDownload::sgnRequestSKTInfo, m_clientWgt, &AngKMainFrame::onSlotRequestSKTInfo);
		taskDownDlg.exec();
		disconnect(&taskDownDlg, &ACTaskDownload::sgnDownloadProject, m_clientWgt, &AngKMainFrame::onSlotDownloadProject);
		disconnect(&taskDownDlg, &ACTaskDownload::sgnCheckDevVFiles, m_clientWgt, &AngKMainFrame::onSlotCheckDevVFiles);

		disconnect(m_clientWgt, &AngKMainFrame::sgnProjectValue, &taskDownDlg, &ACTaskDownload::onSlotDownloadProgress);
		disconnect(&taskDownDlg, &ACTaskDownload::sgnUpdateTaskInfo, m_clientWgt, &AngKMainFrame::onSlotUpdateTaskInfo);
	}
	void AGClient::onSlotActionDownloadTaskJsonRpc(QString filePath)
	{
		if (m_clientWgt->GetBurnStatus()) {
			ACMessageBox::showWarning(this, tr("Warning"), tr("Please try after the programming is complete."));
			return;
		}

		ACTaskDownload taskDownDlg(this, m_pTaskManager);
		taskDownDlg.onSlotSelectTaskPathFromJsonRpc(filePath);
		taskDownDlg.setModelTableSeleted();
		
		connect(&taskDownDlg, &ACTaskDownload::sgnDownloadProject, m_clientWgt, &AngKMainFrame::onSlotDownloadProject, Qt::DirectConnection);
		connect(&taskDownDlg, &ACTaskDownload::sgnCheckDevVFiles, m_clientWgt, &AngKMainFrame::onSlotCheckDevVFiles, Qt::DirectConnection);

		connect(m_clientWgt, &AngKMainFrame::sgnProjectValue, &taskDownDlg, &ACTaskDownload::onSlotDownloadProgress);
		connect(&taskDownDlg, &ACTaskDownload::sgnUpdateTaskInfo, m_clientWgt, &AngKMainFrame::onSlotUpdateTaskInfo);
		connect(&taskDownDlg, &ACTaskDownload::sgnRequestSKTInfo, m_clientWgt, &AngKMainFrame::onSlotRequestSKTInfo);
		taskDownDlg.onSlotDownloadTask();
		taskDownDlg.exec();
		disconnect(&taskDownDlg, &ACTaskDownload::sgnDownloadProject, m_clientWgt, &AngKMainFrame::onSlotDownloadProject);
		disconnect(&taskDownDlg, &ACTaskDownload::sgnCheckDevVFiles, m_clientWgt, &AngKMainFrame::onSlotCheckDevVFiles);

		disconnect(m_clientWgt, &AngKMainFrame::sgnProjectValue, &taskDownDlg, &ACTaskDownload::onSlotDownloadProgress);
		disconnect(&taskDownDlg, &ACTaskDownload::sgnUpdateTaskInfo, m_clientWgt, &AngKMainFrame::onSlotUpdateTaskInfo);
	}

	void AGClient::onSlotActionUpdateFirmware()
	{
		AngKUpdateFirmware dlg(this, m_clientWgt->GetHopNumUse());
		connect(&dlg, &AngKUpdateFirmware::sgnUpdateFirmwareFile, m_clientWgt, &AngKMainFrame::onSlotUpdateFirmwareFile);
		connect(m_clientWgt, &AngKMainFrame::sgnUpdateFPGAValue, &dlg, &AngKUpdateFirmware::onSlotUpdateFPGAValue);
		connect(m_clientWgt, &AngKMainFrame::sgnUpdateFwStatus, &dlg, &AngKUpdateFirmware::onSlotUpdateFwStatus);
		dlg.exec();
		disconnect(&dlg, &AngKUpdateFirmware::sgnUpdateFirmwareFile, m_clientWgt, &AngKMainFrame::onSlotUpdateFirmwareFile);
		disconnect(m_clientWgt, &AngKMainFrame::sgnUpdateFPGAValue, &dlg, &AngKUpdateFirmware::onSlotUpdateFPGAValue);
		disconnect(m_clientWgt, &AngKMainFrame::sgnUpdateFwStatus, &dlg, &AngKUpdateFirmware::onSlotUpdateFwStatus);
	}

	void AGClient::onSlotActionCreate()
	{
		//m_clientWgt->CreateProject();

		//先选择芯片，否则不允许创
		//std::string curChipName = m_pDataset->getChipData().getJsonValue<std::string>("chipName");
		//if (curChipName.empty()) {
		//	ALOG_ERROR("Please choose a chip first!", "CU", "--");
		//	ACMessageBox::showWarning(this, tr("Warning"), tr("Please choose a chip first!"));
		//	return;
		//}

		ACProjectCreateDlg dlg(this, m_DataBuffer);
		dlg.exec();
	}
}