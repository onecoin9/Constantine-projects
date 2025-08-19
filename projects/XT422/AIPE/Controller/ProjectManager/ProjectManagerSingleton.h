#pragma once

#include "ACProjManager.h" // Include the header for ACProjManager

class ProjectManagerSingleton {
private:
    // Private constructor to prevent instantiation
    ProjectManagerSingleton() : m_projectManager(nullptr) {}

    // Delete copy constructor and assignment operator
    ProjectManagerSingleton(const ProjectManagerSingleton&) = delete;
    ProjectManagerSingleton& operator=(const ProjectManagerSingleton&) = delete;

    // Static instance of the singleton
    static ProjectManagerSingleton* m_instance;

    // Pointer to the ACProjManager instance
    ACProjManager* m_projectManager;

public:
    // Static method to get the singleton instance
    static ProjectManagerSingleton& getInstance();

    // Method to set the ACProjManager instance
    void setProjectManager(ACProjManager* manager);

    // Method to get the AngKProjDataset
    AngKProjDataset* getProjDataset();
}; 