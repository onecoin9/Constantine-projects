#include "AngKGlobalInstance.h"
#include <QSettings>

static AcroView::AGClient* g_mainWindow = NULL;
static QSettings* g_settings = NULL;
static QSettings* g_globalSettings = NULL;
int AngKGlobalInstance::SkinMode = 0;

AngKGlobalInstance::AngKGlobalInstance()
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
