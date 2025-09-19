#include "TaskManagerSingleton.h"
#include <QDebug>

// Initialize the static instance to nullptr
TaskManagerSingleton* TaskManagerSingleton::m_instance = nullptr;

// Implementation of getInstance()
TaskManagerSingleton& TaskManagerSingleton::getInstance() {
    if (m_instance == nullptr) {
        m_instance = new TaskManagerSingleton();
    }
    return *m_instance;
}

// Implementation of setTaskManager()
void TaskManagerSingleton::setTaskManager(ACTaskManager* manager) {
    m_taskManager = manager;
}

// Implementation of getProjDataset()
AngKProjDataset* TaskManagerSingleton::getProjDataset(const QString& key) {
    if (m_taskManager) {
        QMap<QString, QPair<QString, ACProjManager*>>& allProjInfo = m_taskManager->GetAllProjInfo();
        if (allProjInfo.contains(key)) {
            ACProjManager* projManager = allProjInfo.value(key).second;
            if (projManager) {
                return projManager->GetProjData();
            }
        }
    }
    return nullptr; // Return nullptr if the manager is not set, key not found, or projManager is nullptr
}

// Implementation of getAllProjInfo()
QMap<QString, QPair<QString, ACProjManager*>>* TaskManagerSingleton::getAllProjInfo() {
    if (m_taskManager) {
        return &m_taskManager->GetAllProjInfo();
    }
    return nullptr;
} 