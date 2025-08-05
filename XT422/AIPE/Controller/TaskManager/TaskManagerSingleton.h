#pragma once

#include "ACTaskManager.h" // Include the header for ACTaskManager
#include "AngKProjDataset.h" // Include the header for AngKProjDataset

class TaskManagerSingleton {
private:
    // Private constructor to prevent instantiation
    TaskManagerSingleton() : m_taskManager(nullptr) {}

    // Delete copy constructor and assignment operator
    TaskManagerSingleton(const TaskManagerSingleton&) = delete;
    TaskManagerSingleton& operator=(const TaskManagerSingleton&) = delete;

    // Static instance of the singleton
    static TaskManagerSingleton* m_instance;

    // Pointer to the ACTaskManager instance
    ACTaskManager* m_taskManager;

public:
    // Static method to get the singleton instance
    static TaskManagerSingleton& getInstance();

    // Method to set the ACTaskManager instance
    void setTaskManager(ACTaskManager* manager);

    // Method to get the AngKProjDataset for a specific project manager by key
    AngKProjDataset* getProjDataset(const QString& key);

    // Optional: Method to get the entire map of project managers
    QMap<QString, QPair<QString, ACProjManager*>>* getAllProjInfo();
}; 