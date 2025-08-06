#ifndef PROJECTSELECTDIALOG_H
#define PROJECTSELECTDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QGroupBox>
#include <QSettings>
#include <QDir>
#include <QJsonObject>

class ProjectSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectSelectDialog(QWidget *parent = nullptr);
    ~ProjectSelectDialog();
    
    // 获取选中项目的路径和名称
    QString getSelectedProjectPath() const;
    QString getSelectedProjectName() const;

private slots:
    void onAddProjectClicked();
    void onEditProjectClicked();
    void onRemoveProjectClicked();
    void onProjectSelectionChanged();
    void onOkButtonClicked();
    void onCancelButtonClicked();
    
private:
    void setupUI();
    void loadProjects();
    void saveProjects();
    void updateProjectDetails();
    void applyStyleSheet();
    
    QListWidget* m_projectList;
    QPushButton* m_addButton;
    QPushButton* m_editButton;
    QPushButton* m_removeButton;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    
    QGroupBox* m_detailsGroup;
    QLabel* m_pathLabel;
    QLineEdit* m_pathEdit;
    QLabel* m_descLabel;
    QTextEdit* m_descEdit;
    
    QString m_configDir;
    QSettings m_settings;
    
    struct ProjectInfo {
        QString name;
        QString path;
        QString description;
    };
    QList<ProjectInfo> m_projects;
    QString m_selectedProjectPath;
};

#endif // PROJECTSELECTDIALOG_H