#include <memory>  
#include <mutex>  
#include <QPluginLoader>
#include "automaticplugin_global.h"


class ACAutomaticManager {
public:
    static ACAutomaticManager* instance() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!m_instance) {
            m_instance = new ACAutomaticManager();
        }
        return m_instance;
    }

    IAutomatic* GetAutomaticPlugin()
    {
        return mAutoMatic;
    }

    bool setAutomaticModeEn(bool enable);
    bool isEnable() { return mEnable; }
private:
    ACAutomaticManager();
    ~ACAutomaticManager() {};


    bool automaticInit();

    static ACAutomaticManager* m_instance;
    static std::mutex mutex_;

    IAutomatic* mAutoMatic;
    AutomaticPluginInterface* parser;
    QPluginLoader* pluginloader; // 插件位置？
    QObject* plugin;

    bool mEnable;
};