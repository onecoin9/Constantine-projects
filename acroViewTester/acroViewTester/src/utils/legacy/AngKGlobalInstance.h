#pragma once

#include <QObject>
#include <atomic>
#include "agclient.h"
class AcroView::AGClient;
class QSettings;

class AngKGlobalInstance : public QObject
{
	Q_OBJECT

public:
	AngKGlobalInstance();
	~AngKGlobalInstance();

public:
	static AngKGlobalInstance* instance();

	static void setMainWindow(AcroView::AGClient* mw);
	static AcroView::AGClient* mainWindow();

	static void setSettings(QSettings* settings);
	static QSettings* settings();

	static void setGlobalSettings(QSettings* settings);
	static QSettings* globalSettings();

	static void WriteValue(QString strGroup, QString strName, QVariant strValue);
	static QVariant ReadValue(QString strGroup, QString strName, const QVariant& default = QVariant());

	static void SetSkinMode(int nMode);
	static int GetSkinMode();

    ApplicationStatus getAppRunStatus();
    bool isAppRunStatusNormal(QString *warnMsg = Q_NULLPTR);
    void setAppRunStatus(ApplicationStatus);
	void setDeviceType(const QString& deviceType) { m_devType = deviceType; }
	QString getDeviceType() { return m_devType; }
	void outputReport();


signals:
    void sgnAppStatusChanged();
	void sgnReportOutput();

private:
	friend class AcroView::AGClient;
	static int SkinMode;
    std::atomic<ApplicationStatus> m_appStatus;
	QString m_devType;
};
