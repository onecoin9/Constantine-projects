#include <QtWidgets/QApplication>
#include <QSettings>
#include <QTranslator>
#include <QBuffer>
#include <QDebug>
#include <QDir>
#include <QFontDatabase>
#include <QLibrary>
#include <QTemporaryFile>
#include <QSharedMemory>
#include <QFileDevice>
#include <QPainter>
#include <QProcess>
#include <QResource>
#include <QSvgRenderer>
#include <winsock2.h>
#include <QCommandLineParser> // <-- 添加命令行解析器头文件
#include <QCommandLineOption> // <-- 添加命令行选项头文件

#include <Dbghelp.h>
#pragma comment( lib, "Dbghelp.lib" )

#include <windows.h>
#include <winnt.h>
#include "agclient.h"
#include "AngKLoginDialog.h"
#include "AngKGlobalInstance.h"
#include "AppModeType.h"
#include "MessageType.h"
#include "ACEventManager.h"
#include "AngkLogger.h"
#include "AngKPathResolve.h"
#include "AngKMessageHandler.h"
#include "Thread/ThreadPool.h"
#include "../RemoteServer/JsonRpcServer.h"
Acro::Thread::ThreadPool g_ThreadPool;

// global
UserMode curUserMode = UserMode::Operator;
bool bAutoConnect = false;

/// <summary>
/// 程序异常捕获，上位机崩溃的时候产生dump，方面查看
/// </summary>
/// <param name="pException">系统捕获参数</param>
/// <returns></returns>
LONG ExceptionCapture(EXCEPTION_POINTERS* pException)
{
	//当前时间串
	const int TIMESTRLEN = 32;
	WCHAR timeStr[TIMESTRLEN];
	SYSTEMTIME time;
	GetLocalTime(&time);
	swprintf_s(timeStr, TIMESTRLEN, L"%4d_%02d_%02d_%02d_%02d_%02d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
	WCHAR strname[MAX_PATH];
	swprintf_s(strname, MAX_PATH, L"acroview_%s.dmp", timeStr);

	//创建 Dump 文件
	HANDLE hDumpFile = CreateFile(strname, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hDumpFile != INVALID_HANDLE_VALUE)
	{
		//Dump信息
		MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
		dumpInfo.ExceptionPointers = pException;
		dumpInfo.ThreadId = GetCurrentThreadId();
		dumpInfo.ClientPointers = TRUE;
		//写入Dump文件内容
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, nullptr, nullptr);
	}

	//完成一些数据保存工作
	AngKMessageHandler::instance().Command_CloseProgramSocket();

	QDir qdir(Utils::AngKPathResolve::localTempFolderPath());
	qdir.removeRecursively();

	//弹出错误对话框并退出程序
	//QMessageBox::critical(nullptr, "错误提示", QString("当前程序遇到异常.\n  异常文件:%1").arg(QString::fromWCharArray(strname)), QMessageBox::Ok, QMessageBox::Ok);
	EventLogger->SendEvent(EventBuilder->GetSoftStatusChange(false));
	return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	// --- 单例应用程序检查 ---
	QSharedMemory sharedMemory("AcroView_SingleInstance_Guard");
	if (!sharedMemory.create(1)) {
		// 如果共享内存段已存在，则说明已有实例在运行
		// 此处可以添加一个提示框，但为了保持静默退出，我们直接返回
		ALOG_WARN("Application is already running. Exiting.", "CU", "--");
		return 0; // 退出当前实例
	}
	// 如果创建成功，sharedMemory 会在程序退出时自动释放

	// **** 初始化 Winsock ****
	WSADATA wsaData;
	int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsaResult != 0) {
		// 处理初始化失败，可能需要记录日志并退出
		ALOG_FATAL("WSAStartup failed with error: %d", "CU", "--", wsaResult);
		return 1; // 或者其他错误码
	}

	// --- 命令行参数解析 ---
	QCommandLineParser parser;
	parser.setApplicationDescription("AcroView Application");
	parser.addHelpOption();
	parser.addVersionOption(); // 可选

	// 添加一个选项来启用 RPC 服务器
	QCommandLineOption enableRpcOption(QStringList() << "r" << "enable-rpc",
	                                   QCoreApplication::translate("main", "Enable the JSON-RPC server."));
	parser.addOption(enableRpcOption);

    // 添加一个选项来指定 RPC 服务器端口 (可选)
    QCommandLineOption rpcPortOption(QStringList() << "p" << "rpc-port",
                                     QCoreApplication::translate("main", "Specify the JSON-RPC server port (default: 12345)."),
                                     QCoreApplication::translate("main", "port"),
                                     "12345"); // 默认端口
    parser.addOption(rpcPortOption);


	// 处理命令行参数
	parser.process(a);
	// --- 命令行参数解析结束 ---


	//注冊异常捕获函数
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ExceptionCapture);

	//setup settings
	QSettings::setDefaultFormat(QSettings::IniFormat);
	QSettings* globalSettings = new QSettings(Utils::AngKPathResolve::localGlobalSettingFile(), QSettings::IniFormat);
	globalSettings->setIniCodec("utf-8");
	AngKGlobalInstance::setGlobalSettings(globalSettings);
	AngKGlobalInstance::SetSkinMode(AngKGlobalInstance::ReadValue("Skin", "mode").toInt());
	curUserMode = (UserMode)AngKGlobalInstance::instance()->ReadValue("LoginPwd", "userMode", 0).toInt();

	//创建事件管理器
	std::unique_ptr<ACEventManager> eventManager = std::make_unique<ACEventManager>();
	eventManager->start();

	qputenv("QT_ENABLE_HIGHDPI_SCALING", "1");
	QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

	//QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);;

	QResource::registerResource(qApp->applicationDirPath() + "/agclient.rcc");

	//设置Icon
	QSvgRenderer render;
	bool ok = render.load(QCoreApplication::applicationDirPath() + "/icon.svg");
	QSize size = render.defaultSize();
	QPixmap* pix = new QPixmap(30,30);
	pix->fill(Qt::transparent);
	QPainter painter(pix);
	painter.setRenderHints(QPainter::Antialiasing);
	render.render(&painter);
	QIcon icon(*pix);
	QApplication::setWindowIcon(icon);

	//设置日志等级和写入位置、大小、保持时间
	SET_ALOG_LEVEL((Utils::AngkLogger::LogLevel)AngKGlobalInstance::ReadValue("LogLevel", "level").toInt());
	//工程和任务路径设置
	if (AngKGlobalInstance::ReadValue("TaskProjectPath", "projPath").toString().isEmpty()) {
		AngKGlobalInstance::WriteValue("TaskProjectPath", "projPath", Utils::AngKPathResolve::localProjectPath());
	} else {
		QFile file(AngKGlobalInstance::ReadValue("TaskProjectPath", "projPath").toString());
		if (!file.exists()) {
			AngKGlobalInstance::WriteValue("TaskProjectPath", "projPath", Utils::AngKPathResolve::localProjectPath());
		}
	}
	if (AngKGlobalInstance::ReadValue("TaskProjectPath", "taskPath").toString().isEmpty()) {
		AngKGlobalInstance::WriteValue("TaskProjectPath", "taskPath", Utils::AngKPathResolve::localTaskPath());
	}
	else {
		QFile file(AngKGlobalInstance::ReadValue("TaskProjectPath", "taskPath").toString());
		if (!file.exists()) {
			AngKGlobalInstance::WriteValue("TaskProjectPath", "taskPath", Utils::AngKPathResolve::localTaskPath());
		}
	}
	//AngKGlobalInstance::WriteValue("LogFile", "size", 1024);
	//AngKGlobalInstance::WriteValue("LogFile", "keepTime", 0);

	//翻译
	QTranslator translatorQt;
	QString strLocaleFile = Utils::AngKPathResolve::localTranslatePath(AngKGlobalInstance::ReadValue("Language", "mode").toInt());
	translatorQt.load(strLocaleFile);
	a.installTranslator(&translatorQt);

	//加入免费字体包
	QDir fontsDir(Utils::AngKPathResolve::localFontPath());
	QFileInfoList filelist = fontsDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
	for (const auto& fontFileInfo : fontsDir.entryInfoList()) {
		const auto id = QFontDatabase::addApplicationFont(fontFileInfo.absoluteFilePath());
		if (id == -1)
			qWarning() << "Load font file" << fontFileInfo << "failed.";
		else
			qInfo() << "Load font" << QFontDatabase::applicationFontFamilies(id) << "ok.";
	}

	// 登录窗口先打开
	//AngKLoginDialog dlg;
	//int ret = dlg.exec();
	//if (QDialog::Accepted != ret) {
	//	EventLogger->SendEvent(EventBuilder->GetSoftStatusChange(false));
	//	return 0;
	//}
	//else {
	//	EventLogger->SendEvent(EventBuilder->GetSoftStatusChange(true));
	//}
	g_ThreadPool.Start(15);
	AcroView::AGClient w;
	// 根据命令行参数决定是否启动 JsonRpcServer
	if (parser.isSet(enableRpcOption)) {
		bool ok;
		quint16 port = parser.value(rpcPortOption).toUShort(&ok);
		if (!ok) {
			qWarning() << "Invalid RPC port specified:" << parser.value(rpcPortOption) << ". Using default 12345.";
			port = 12345;
		}
		ALOG_INFO("Command-line option --enable-rpc detected. Starting JSON-RPC server on port %u.", "CU", "--", port);

		if (!JsonRpcServer::Instance()->Start(port)) {
			ALOG_ERROR("Failed to start JSON-RPC server on port %u.", "CU", "--", port);
			// 可以考虑在这里退出或给出更明确的错误提示
		}
	}
	else {
		ALOG_INFO("JSON-RPC server is not enabled via command-line option.", "CU", "--");
	}
	bool jsonRpcDebug = false;//true启用jsonRpcDebug，false关闭
	if(jsonRpcDebug){
		JsonRpcServer::Instance()->Start(12345);
	}
	//JsonRpcServer::Instance()->Start(12345); // <-- 注释掉或删除原来的无条件启动
	ALOG_INFO("Client initialing start, Version = %s.", "CU", "--", AngKGlobalInstance::ReadValue("Version", "BuildVer").toString().toStdString().c_str());
	w.InitClient();
	if (w.LoginCheck()) {
		ALOG_WARN("Client exit.", "CU", "--");
		//w.ExitLogWrite();
		//QThread::sleep(2);
		return 0;
	}
	ALOG_INFO("Client initialized successfully.", "CU", "--");
	//w.GetBPUInfo();
	int retExec = a.exec();

	//程序结束关闭程序Socket
	if(g_AppMode != ConnectType::Demo)
		AngKMessageHandler::instance().Command_CloseProgramSocket();

	if (MessageType::MESSAGE_RESTART == retExec) {
		QProcess::startDetached(qApp->applicationFilePath(), QStringList());
	}
	//qApp->exit();
	qDebug() << "is First? exec()";
	EventLogger->SendEvent(EventBuilder->GetSoftStatusChange(false));
	// **** 清理 Winsock ****
	WSACleanup();
	g_ThreadPool.Stop();

	return retExec;
}


