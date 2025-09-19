
#include "ACAutomaticManager.h"
#include <QPluginLoader>
#include <QCoreApplication>
#include <QFile>
// #include "AngkLogger.h"

ACAutomaticManager *ACAutomaticManager::m_instance = nullptr;
std::mutex ACAutomaticManager::mutex_;

ACAutomaticManager::ACAutomaticManager()
	: mAutoMatic(nullptr), parser(nullptr), pluginloader(nullptr), plugin(nullptr)
{
	QString pluginPath = QCoreApplication::applicationDirPath() +
						 "/Plugins/AutoMatic/AutomaticPlugin.dll";

	// 将路径中的斜杠替换为双反斜杠
	pluginPath.replace("/", "\\");

	// 检查路径是否有效
	if (!QFile::exists(pluginPath))
	{
		qDebug() << "Plugin not found at:" << pluginPath;
		pluginPath = "D:\\qtProject\\acroViewTester\\acroViewTester\\acroViewTester\\Plugins\\AutoMatic\\AutomaticPlugin.dll";
		qDebug() << "Using fallback path:" << pluginPath;
	}

	qDebug() << "Plugin Path:" << pluginPath;

	QCoreApplication::addLibraryPath(pluginPath);
	pluginloader = new QPluginLoader(pluginPath);
	plugin = pluginloader->instance();

	if (!automaticInit())
	{
		return;
	}
}

bool ACAutomaticManager::automaticInit()
{

	bool ret = true;
	if (plugin)
	{
		parser = qobject_cast<AutomaticPluginInterface *>(plugin);
		if (parser)
		{
			parser->CreateAutomatic("SProtocal");
			mAutoMatic = parser->GetAutomaticPtr();
			if (!mAutoMatic)
			{

				////ALOG_INFO("ACAutomaticManager Init Fail. %s", "CU", "--", parser->GetErrorStr().c_str());
				ret = false;
				return ret;
			}

			mAutoMatic->QuerySiteEn();
			////ALOG_INFO("ACAutomaticManager Init OK.", "CU", "--");
		}
		else
		{
			// ALOG_INFO("ACAutomaticManager Init Fail.", "CU", "--");
			ret = false;
		}
	}
	else
	{
		ret = false;
	}
	return ret;
}

bool ACAutomaticManager::setAutomaticModeEn(bool enable)
{
	mEnable = false;
	if (enable && automaticInit())
		mEnable = enable;

	return mEnable == enable;
}