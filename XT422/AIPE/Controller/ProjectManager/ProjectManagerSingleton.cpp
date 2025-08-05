#include "ProjectManagerSingleton.h"

// Initialize the static instance to nullptr
ProjectManagerSingleton* ProjectManagerSingleton::m_instance = nullptr;

// Implementation of getInstance()
ProjectManagerSingleton& ProjectManagerSingleton::getInstance() {
    if (m_instance == nullptr) {
        m_instance = new ProjectManagerSingleton();
    }
    return *m_instance;
}

// Implementation of setProjectManager()
void ProjectManagerSingleton::setProjectManager(ACProjManager* manager) {
    m_projectManager = manager;
}

// Implementation of getProjDataset()
AngKProjDataset* ProjectManagerSingleton::getProjDataset() {
    if (m_projectManager) {
        return m_projectManager->GetProjData();
    }
    return nullptr; // Return nullptr if the manager is not set
} 