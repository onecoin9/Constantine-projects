#pragma once

#include <QtCore/qglobal.h>
#include <QObject>
#include "AutoInterface.h"
#ifndef BUILD_STATIC
# if defined(AUTOMATICPLUGIN_LIB)
#  define AUTOMATICPLUGIN_EXPORT Q_DECL_EXPORT
# else
#  define AUTOMATICPLUGIN_EXPORT Q_DECL_IMPORT
# endif
#else
# define AUTOMATICPLUGIN_EXPORT
#endif


class AutomaticPluginInterface : public QObject
{
	Q_OBJECT

public:
	virtual ~AutomaticPluginInterface() {}
	virtual QString PluginName() = 0;
	virtual void CreateAutomatic(QString ProtocalType) = 0;

	virtual IAutomatic* GetAutomaticPtr() = 0;
	virtual std::string GetErrorStr() = 0;
};

//封装成插件需要在原本封装dll的基础上添加以下语句
QT_BEGIN_NAMESPACE
#define AutomaticPlugin_IID "org.acroview.plugin.Automatic"
Q_DECLARE_INTERFACE(AutomaticPluginInterface, AutomaticPlugin_IID)
QT_END_NAMESPACE