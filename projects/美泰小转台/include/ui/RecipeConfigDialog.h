#ifndef RECIPECONFIGDIALOG_H
#define RECIPECONFIGDIALOG_H

#include <QDialog>
#include <QJsonObject>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QSpinBox;
class QPushButton;
class QLabel;
class QComboBox;
class QTableWidget;
class QTabWidget;
class QSettings;
QT_END_NAMESPACE

namespace Presentation {

/**
 * @brief 配方配置对话框
 * 
 * 用于设置生产配方的关键参数，包括：
 * - cmd3TaskPath: 自动机配置文件路径
 * - taskFileName: 烧录软件配置文件路径  
 * - batchNumber: 批次号
 * - productionQuantity: 生产数量
 * - activationParams: 激活参数（扩展备用）
 */
class RecipeConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RecipeConfigDialog(QWidget *parent = nullptr);
    ~RecipeConfigDialog();
    /**
     * @brief 保存配方参数
     */
    bool saveRecipeConfig();

    /**
     * @brief 获取配置的参数
     */
    QJsonObject getConfigParameters() const;

private slots:
    void onBrowseCmd3Path();
    void onBrowseTaskFileName();
    void onBrowseRouterFile();
    void onSaveClicked();
    void onCancelClicked();
    void onChipTypeChanged(const QString& chipType);
    void onAddParameter();
    void onRemoveParameter();
    void onSaveParameters();
    void onLoadActestFile();

private:
    void setupUi();
    void connectSignals();
    void loadCurrentConfig();
    bool updateRecipeFile();
    bool validateInputs();
    void setupActivationParametersTable();
    void loadChipTemplates();
    void loadParametersForChip(const QString& chipType);
    void addParameterRow(const QJsonObject& param);
    void loadConfigTypeOptions();
    void saveLastSelectedConfig(const QString& configGroup);
    QString getLastSelectedConfig();
    void loadConfigByGroup(const QString& selectedGroup);
    bool saveCurrentConfigToSettings(const QString& filePath, const QString& configGroup);
    bool loadActestConfigFile(const QString& filePath);
    
    // UI组件
    QLineEdit* m_recipeNameEdit;
    QLineEdit* m_siteGroupsEdit;
    QLineEdit* m_routerFileEdit;
    QLineEdit* m_cmd3PathEdit;
    QLineEdit* m_taskFileNameEdit;
    
    // 激活参数配置控件
    QTabWidget* m_tabWidget;
    QComboBox* m_chipTypeComboBox;
    QTableWidget* m_activationParamsTable;
    QPushButton* m_addParamButton;
    QPushButton* m_removeParamButton;
    QPushButton* m_saveParamButton;
    
    QPushButton* m_routerBrowseButton;
    QPushButton* m_cmd3BrowseButton;
    QPushButton* m_taskFileBrowseButton;
    QPushButton* m_saveButton;
    QPushButton* m_cancelButton;
    QPushButton* m_loadActestButton;
    
    
    // 数据
    QJsonObject m_recipeData;
    QJsonObject m_chipTemplates;  // 存储不同芯片的参数模板
};

} // namespace Presentation

#endif // RECIPECONFIGDIALOG_H 