#include "AngKGlobalInstance.h"
#include <QSettings>

static AcroView::AGClient* g_mainWindow = NULL;
static QSettings* g_settings = NULL;
static QSettings* g_globalSettings = NULL;
int AngKGlobalInstance::SkinMode = 0;

AngKGlobalInstance::AngKGlobalInstance() : m_appStatus(ApplicationStatus::Normal)
{
}

AngKGlobalInstance::~AngKGlobalInstance()
{
	g_mainWindow = NULL;
	g_settings = NULL;
	g_globalSettings = NULL;
}

AngKGlobalInstance* AngKGlobalInstance::instance()
{
	static AngKGlobalInstance instance;
	return &instance;
}

void AngKGlobalInstance::setMainWindow(AcroView::AGClient* mw)
{
	g_mainWindow = mw;
}

AcroView::AGClient* AngKGlobalInstance::mainWindow()
{
	return g_mainWindow;
}

void AngKGlobalInstance::setSettings(QSettings* settings)
{
	if (g_settings)
	{
		delete g_settings;
		g_settings = NULL;
	}
	
	g_settings = settings;
	g_settings->setParent(AngKGlobalInstance::instance());
}

QSettings* AngKGlobalInstance::settings()
{
	return g_settings;
}

void AngKGlobalInstance::setGlobalSettings(QSettings* settings)
{
	if (g_globalSettings)
	{
		delete g_globalSettings;
		g_globalSettings = NULL;
	}
	//
	g_globalSettings = settings;
	g_globalSettings->setParent(AngKGlobalInstance::instance());
}

QSettings* AngKGlobalInstance::globalSettings()
{
	return g_globalSettings;
}

void AngKGlobalInstance::WriteValue(QString strGroup, QString strName, QVariant strValue)
{
	g_globalSettings->beginGroup(strGroup);
	g_globalSettings->setValue(strName, strValue);
	g_globalSettings->endGroup();
}

QVariant AngKGlobalInstance::ReadValue(QString strGroup, QString strName, const QVariant& default)
{
	return g_globalSettings->value(strGroup + "/" + strName, default);
}

void AngKGlobalInstance::SetSkinMode(int nMode)
{
	SkinMode = nMode;
}

int AngKGlobalInstance::GetSkinMode()
{
	return SkinMode;
}


ApplicationStatus AngKGlobalInstance::getAppRunStatus()
{
    return m_appStatus.load();
}

bool AngKGlobalInstance::isAppRunStatusNormal(QString *warnMsg)
{
    auto status = m_appStatus.load();

    if (warnMsg != Q_NULLPTR) {
        switch (status) {

        case ApplicationStatus::Burn:
            *warnMsg = tr("Please try again after the programming is completed.");
			break;
        case ApplicationStatus::DownloadTask:
            *warnMsg = tr("Please try again after the download task is completed.");
			break;
        case ApplicationStatus::MasterChipAnalyze:
            *warnMsg = tr("Please try again after the master chip analysis is completed.");
			break;
        case ApplicationStatus::EmmcAnalyze:
            *warnMsg = tr("Please try again after the emmc analysis is completed.");
			break;
        case ApplicationStatus::AutomaticBurn:
            *warnMsg = tr("Please try again after the burning process of automatic machine id completed.");
			break;
        case ApplicationStatus::FirmwareUpdate:
            *warnMsg = tr("Please try again after the firmware upgrade is completed.");
			break;
        default:
            break;
        }
    }
    

    return (ApplicationStatus::Normal == status);
}

void AngKGlobalInstance::setAppRunStatus(ApplicationStatus status)
{
    m_appStatus.store(status);

    emit sgnAppStatusChanged();
}


void AngKGlobalInstance::outputReport()
{
	emit sgnReportOutput();
}
