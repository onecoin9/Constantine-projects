#include "AngKPathResolve.h"
#include "GlobalDefine.h"
#include "AngKGlobalInstance.h"
#include <windows.h>
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QCoreApplication>
#include <QString>

namespace Utils
{
	QString AngKPathResolve::m_onlyFile = "";
	QString AngKPathResolve::m_onlyEventFile = "";

	QString AngKPathResolve::logFile()
	{
		if(m_onlyFile.isEmpty())
			m_onlyFile = logFilePath() + "acroview_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".log";

		ensureFileExists(m_onlyFile);

		return m_onlyFile;
	}

	QString AngKPathResolve::eventFile()
	{
		//关键事件记录和日志记录放到一起
		if (m_onlyEventFile.isEmpty())
			m_onlyEventFile = logFilePath() + "acroviewEvent_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".log";

		ensureFileExists(m_onlyEventFile);

		return m_onlyEventFile;
	}

	void AngKPathResolve::ensurePathExists(const QString& path)
	{
		QDir dir;
		dir.mkpath(path);
	}

	void AngKPathResolve::ensureFileExists(const QString& path)
	{
		if (!QFileInfo::exists(path))
		{
			QFile pfile(path);
			pfile.open(QIODevice::WriteOnly | QIODevice::Text);
			pfile.close();
		}
	}

	QString AngKPathResolve::localTranslatePath(int nLanguageMode)
	{
		QString transFile;
		switch ((TranslateLanguage)nLanguageMode)
		{
		case TranslateLanguage::English:
			transFile = "/Translations/agclient_en.qm";
			break;
		case TranslateLanguage::Chinese:
			transFile = "/Translations/agclient_zh.qm";
			break;
		case TranslateLanguage::Janpanese:
			transFile = "/Translations/agclient_ja.qm";
			break;
		default:
			break;
		}
		return QCoreApplication::applicationDirPath() + transFile;
	}

	QString AngKPathResolve::localSkinPath()
	{
		QString skinPath = QCoreApplication::applicationDirPath() + "/Skin/";
		
		switch ((ViewMode)AngKGlobalInstance::ReadValue("Skin", "mode").toInt())
		{
		case ViewMode::Light:
			skinPath += "Light/";
			break;
		case ViewMode::Dark:
			skinPath += "Dark/";
			break;
		default:
			break;
		}
		return skinPath;
	}

	QString AngKPathResolve::localRelativeSkinPath()
	{
		QString skinPath = ":/Skin/";

		switch ((ViewMode)AngKGlobalInstance::GetSkinMode())
		{
		case ViewMode::Light:
			skinPath += "Light/";
			break;
		case ViewMode::Dark:
			skinPath += "Dark/";
			break;
		default:
			break;
		}
		return skinPath;
	}

	QString AngKPathResolve::localSkinFile(QString qssFile, bool bRelative)
	{
		QString skinFile;
		bRelative ? skinFile = localRelativeSkinPath(): skinFile = localSkinPath();

		return skinFile + qssFile + ".qss";
	}

	QString AngKPathResolve::localFontPath()
	{
		return QCoreApplication::applicationDirPath() + "/Fonts/";
	}

	QString AngKPathResolve::localGlobalSettingFile()
	{
		QString settingPath = QCoreApplication::applicationDirPath() + "/Localcfg.ini";
		ensureFileExists(settingPath);

		return settingPath;
	}

	QString AngKPathResolve::localDBPath(QString dbFile)
	{
		return QCoreApplication::applicationDirPath() + "/LocalDB/" + dbFile;
	}

	QString AngKPathResolve::localParsersPath()
	{
		return QCoreApplication::applicationDirPath() + "/Plugins/Parsers";
	}

	QString AngKPathResolve::localReportTempFilePath()
	{
		return QCoreApplication::applicationDirPath() + "/data/";
	}

	QString AngKPathResolve::localProjectPath()
	{
		QString projPath = QCoreApplication::applicationDirPath() + "/project";
		if (!QDir(projPath).exists())
		{
			QDir().mkdir(projPath);
		}

		return projPath;
	}

	QString AngKPathResolve::localTaskPath()
	{
		QString taskPath = QCoreApplication::applicationDirPath() + "/task";
		if (!QDir(taskPath).exists())
		{
			QDir().mkdir(taskPath);
		}

		return taskPath;
	}

	QString AngKPathResolve::localAutomaticPluginPath()
	{
		return QCoreApplication::applicationDirPath() + "/Plugins/AutoMatic";
	}

	QString AngKPathResolve::logPath()
	{
		QString strPath;

		strPath = AngKGlobalInstance::ReadValue("LogFile", "path").toString();

		QDir dir(strPath);
		if (strPath.isEmpty() || !dir.exists()) {
			strPath = QCoreApplication::applicationDirPath() + "/log";
			AngKGlobalInstance::WriteValue("LogFile", "path", strPath);
		}
		return strPath;
	}

	QString AngKPathResolve::localDeviceDrvFilePath(QString binFile)
	{
		QString strPath;

		strPath = QCoreApplication::applicationDirPath() + "/Drv/" + binFile + ".drv";

		return strPath;
	}

	QString AngKPathResolve::localMasterDrvFilePath(QString binFile)
	{
		QString strPath;

		strPath = QCoreApplication::applicationDirPath() + "/Mst/" + binFile + ".drv";

		return strPath;
	}


	QString AngKPathResolve::localTempBufFolderPath()
	{
		QString strPath;

#ifdef Q_OS_WIN
		strPath = QCoreApplication::applicationDirPath() + "/temp/";
#else
		strPath = QCoreApplication::applicationDirPath() + "/.temp/";
#endif
		if (!QDir(strPath).exists())
		{
			QDir().mkdir(strPath);
		}
#ifdef Q_OS_WIN
		SetFileAttributes((LPCWSTR)strPath.unicode(), FILE_ATTRIBUTE_HIDDEN);        // 设置隐藏文件夹
#endif

		return strPath;
	}

	QString AngKPathResolve::localTempFolderPath()
	{
		QString strPath;

#ifdef Q_OS_WIN
		strPath = QCoreApplication::applicationDirPath() + "/temp/";
#else
		strPath = QCoreApplication::applicationDirPath() + "/.temp/";
#endif
		if (!QDir(strPath).exists())
		{
			QDir().mkdir(strPath);
		}
#ifdef Q_OS_WIN
		SetFileAttributes((LPCWSTR)strPath.unicode(), FILE_ATTRIBUTE_HIDDEN);        // 设置隐藏文件夹
#endif

		return strPath;
	}

	QString AngKPathResolve::localDriverFilePath(QString driverFile)
	{
		QString strPath;

		strPath = localTempFolderPath() + driverFile + ".adrv";

		return strPath;
	}

	QString AngKPathResolve::localFPGAFilePath(QString fpgaFile)
	{
		QString strPath;

		strPath = localTempFolderPath() + fpgaFile + ".bin";

		return strPath;
	}

	void AngKPathResolve::ReCreateLogFile()
	{
		m_onlyFile = "";
	}

	void AngKPathResolve::DeleteDirectory(const QString& path)
	{
		QDir dir(path);

		// 先尝试直接删除文件夹
		if (dir.removeRecursively()) {
			return; // 如果成功，则直接返回
		}

		// 如果文件夹不为空，逐个删除文件和子文件夹
		dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
		QFileInfoList fileList = dir.entryInfoList();
		for (int i = 0; i < fileList.size(); ++i) {
			QFileInfo fileInfo = fileList.at(i);
			if (fileInfo.isFile()) {
				QFile::setPermissions(fileInfo.absoluteFilePath(), QFile::WriteOwner);
				QFile::remove(fileInfo.absoluteFilePath());
			}
			else {
				DeleteDirectory(fileInfo.absoluteFilePath());
			}
		}

		// 最后尝试再次删除文件夹
		dir.setFilter(QDir::NoFilter);
		dir.removeRecursively();
	}

	QString AngKPathResolve::logFilePath()
	{
		QString strPath;

		strPath = logPath() + "/";

		ensurePathExists(strPath);
		return strPath;
	}
}