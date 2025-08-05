#include "AutomaticPlugin.h"
#include "TranslationCtrl.h"
AutomaticPlugin::AutomaticPlugin()
	: mAutomatic(nullptr)
{
}

void AutomaticPlugin::CreateAutomatic(QString ProtocalType)
{
	//AProgSetting::InitGlbSetting(&mGlobalSetting);

	void* Para = NULL;

	/*if (mGlobalSetting.strProtocalType == "KProtocal") {
		Para = (void*)&mGlobalSetting.AutoKProtocal;
	}
	else if (mGlobalSetting.strProtocalType == "FProtocal") {
		Para = (void*)&mGlobalSetting.AutoFProtocal;
	}
	else if (mGlobalSetting.strProtocalType == "EProtocal") {
		Para = (void*)&mGlobalSetting.AutoEProtocal;
	}
	else if (mGlobalSetting.strProtocalType == "SProtocal") {
		Para = (void*)&mGlobalSetting.AutoSProtocal;
	}*/
	//else if (mGlobalSetting.strProtocalType == "ICTNetProtocal") {
	//	Para = (void*)&mGlobalSetting.AutoICTNetProtocal;
	//}

	mAutomatic = GetAutomatic(ProtocalType.toStdString(), errStr);
	InitTranslation("");
	
}

IAutomatic* AutomaticPlugin::GetAutomaticPtr()
{
	return mAutomatic;
}
