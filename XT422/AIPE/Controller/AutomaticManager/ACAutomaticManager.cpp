
#include "ACAutomaticManager.h"
#include "AngKPathResolve.h"
#include <QPluginLoader>
#include <QCoreApplication>
#include "AngkLogger.h"

ACAutomaticManager* ACAutomaticManager::m_instance = nullptr;
std::mutex ACAutomaticManager::mutex_;

ACAutomaticManager::ACAutomaticManager()
	: mAutoMatic(nullptr)
	, parser(nullptr)
	, pluginloader(nullptr)
	, plugin(nullptr)
{
	QString pluginPath = Utils::AngKPathResolve::localAutomaticPluginPath() + "/AutomaticPlugin.dll";
	QCoreApplication::addLibraryPath(pluginPath);
	pluginloader = new QPluginLoader(pluginPath); // 插件位置？
	plugin = pluginloader->instance();

	if (!automaticInit()) {
		return;
	}
}

bool ACAutomaticManager::automaticInit() {

	bool ret = true;
	if (plugin) {
		parser = qobject_cast<AutomaticPluginInterface*>(plugin);
		if (parser) {
			parser->CreateAutomatic("SProtocal");
			mAutoMatic = parser->GetAutomaticPtr();
			if (!mAutoMatic) {

				ALOG_INFO("ACAutomaticManager Init Fail. %s", "CU", "--", parser->GetErrorStr().c_str());
				ret = false;
				return ret;
			}

			mAutoMatic->QuerySiteEn();
			ALOG_INFO("ACAutomaticManager Init OK.", "CU", "--");
		}
		else {
			ALOG_INFO("ACAutomaticManager Init Fail.", "CU", "--");
			ret = false;
		}
	}
	else {
		ret = false;
	}
	return ret;
}


bool ACAutomaticManager::setAutomaticModeEn(bool enable) { 
	mEnable = false;
	if (enable && automaticInit())
		mEnable = enable;

	return mEnable == enable;
}