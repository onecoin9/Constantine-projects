#pragma once
#include <QFrame>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QResizeEvent>
#include <QtWidgets/QMainWindow>
#include "ui_agclient.h"
#include "ACTaskManager.h"
#include "AngKDataBuffer.h"
#include "AngKDeviceModel.h"
#include "AngKLogManager.h"
#include "AngKProjDataset.h"
#include "AngKShadowWindow.h"
#include "AngKRemoteCmdManager.h"

namespace AcroView 
{
	class AngKMainFrame;
	class AngKSettingDlg;
	class AngKTranslateSelect;
	class AGClient : public AngKShadowWindow<QMainWindow>
	{
		Q_OBJECT

	public:
		AGClient(QWidget* parent = Q_NULLPTR);
		~AGClient();

		/// <summary>
		/// 初始化标题栏
		/// </summary>
		void InitTitleBar();
		/// <summary>
		/// 初始化界面相关启动配置
		/// </summary>
		void InitClient();
		/// <summary>
		/// 初始化菜单栏
		/// </summary>
		void InitMenuBar();
		/// <summary>
		/// 初始化日志管理模块
		/// </summary>
		//void InitLogManager();

		//初始化各种菜单
		void InitProjectMenu();
		void InitTaskMenu();
		void InitUtilitiesMenu();
		void InitWindowMenu();
		void InitViewMenu();
		void InitSettingMenu();
		void InitAutomaticMenu();
		void InitHelpMenu();
		void InitDebugMenu();

		void setTranslate()
		{
			QString(tr("Project"));
			QString(tr("Task"));
			QString(tr("Utilities"));
			QString(tr("Window"));
			QString(tr("View"));
			QString(tr("Setting"));
			QString(tr("Help"));
			QString(tr("Debug"));
		}

		/// <summary>
		/// 登录检查
		/// </summary>
		/// <returns>返回登录标记</returns>
		bool LoginCheck() { return m_bExitFlag; }
		/// <summary>
		/// 获取BPU信息
		/// </summary>
		void GetBPUInfo();
		/// <summary>
		/// 退出日志读写线程
		/// </summary>
		//void ExitLogWrite();
	protected:
		virtual void resizeEvent(QResizeEvent* event);

	private:
		void SetMenuBackGround(QMenu* _menu);
		void TestJson();
	signals:
		void sgnGetBPUInfoFinished();

	public slots:
		void onSlotActionCreate();

		void onSlotActionWindow(bool);

		void onSlotActionView();

		void onSlotActionSetting();

		void onSlotAGClientRemoteQueryDoCmd(QString, uint32_t, uint32_t, uint32_t, uint16_t, uint32_t, uint16_t, QByteArray, uint32_t);

		void onChildWindowClosed();

		void onSlotActionCreateTask();

		void onSlotActionDownloadTask();

		void onSlotActionDownloadTaskJsonRpc(QString filePath);

		void onSlotActionUpdateFirmware();
	private:
		Ui::AGClientClass ui;
		AngKMainFrame*			m_clientWgt;
		QMenuBar*				m_menuBar;
		QMap<QString, QAction*> m_mapAction;
		AngKSettingDlg*			m_AngKSettingDlg;
		AngKTranslateSelect*	m_transSelect;
		AngKProjDataset*		m_pDataset;
		std::shared_ptr<AngKDataBuffer>	m_DataBuffer;
		AngKLogManager*			m_pLogManager;
		AngKRemoteCmdManager*	m_pRemoteCmdManager;
		bool					m_bExitFlag;
		bool					m_bInitFlag;
		std::shared_ptr<ACTaskManager> m_pTaskManager;
	};
}