#pragma once

#include "automaticplugin_global.h"
#include "AutoInterface.h"
//#include "ComStruct/GlbSetting.h"

class AUTOMATICPLUGIN_EXPORT AutomaticPlugin : public AutomaticPluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID AutomaticPlugin_IID)
	Q_INTERFACES(AutomaticPluginInterface)

public:
	AutomaticPlugin();
	~AutomaticPlugin() { };

	virtual QString PluginName() override { return "AutomaticPlugin"; }

	virtual void CreateAutomatic(QString ProtocalType);

	virtual std::string GetErrorStr() { 

		return errStr; 
	};
	virtual IAutomatic* GetAutomaticPtr();

private:
	IAutomatic* mAutomatic;
	std::string errStr;
	//GLBSETTING mGlobalSetting;
};
