#pragma once

#include <QtGlobal>
class QString;

namespace Utils
{
	class AngKPathResolve
	{
	public:
		static QString logFile();
		static QString eventFile();
		static void ensurePathExists(const QString& path);
		static void ensureFileExists(const QString& path);

		static QString localTranslatePath(int nLanguageMode);
		static QString localSkinPath();
		static QString localRelativeSkinPath();
		static QString localSkinFile(QString qssFile, bool bRelative = false);
		static QString localFontPath();
		static QString localGlobalSettingFile();
		static QString localDBPath(QString dbFile);
		static QString localParsersPath();
		static QString localReportTempFilePath();
		static QString localProjectPath();
		static QString localTaskPath();
		static QString localAutomaticPluginPath();

		static QString logPath();
		static QString localDeviceDrvFilePath(QString binFile);
		static QString localMasterDrvFilePath(QString binFile);
		static QString localTempFolderPath();
		static QString localTempBufFolderPath();
		static QString localDriverFilePath(QString driverFile);
		static QString localFPGAFilePath(QString fpgaFile);

		//日志文件满了之后，重新创建
		static void ReCreateLogFile();
		static void DeleteDirectory(const QString& path);
	private:
		static QString logFilePath();

		static QString m_onlyFile;
		static QString m_onlyEventFile;
	};

}