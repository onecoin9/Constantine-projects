#ifndef RECIPESTARTDIALOG_H
#define RECIPESTARTDIALOG_H

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
class RecipeStartDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RecipeStartDialog(QWidget *parent = nullptr);
    ~RecipeStartDialog();

    /**
     * @brief 加载配方参数
     */
    void loadRecipeConfig();

    /**
     * @brief 获取配置的参数
     */
    QJsonObject getConfigParameters();
private slots:
    void onBrowseTaskFileName();
    void onCancelClicked();
    void onApplyClicked();

private:
    void setupUi();
    void connectSignals();
    void loadCurrentConfig();
    bool updateRecipeFile();
    bool validateInputs();

    void loadParametersForChip(const QString& chipType);
    void addParameterRow(const QJsonObject& param);

    void saveLastSelectedConfig(const QString& configGroup);
    QString getLastSelectedConfig();
    void loadRecipeNameFromFile(const QString& filePath);

    
    // UI组件
    QLineEdit* m_recipeNameDisplayEdit;
    QLineEdit* m_actestFieNameEdit;
    QLineEdit* m_batchNumberEdit;
    QSpinBox* m_productionQuantitySpinBox;
    
    // 扩展参数输入字段
    QLineEdit* m_param1Edit;
    QLineEdit* m_param2Edit;
    QLineEdit* m_param3Edit;
    
    // 激活参数配置控件
    QTabWidget* m_tabWidget;
    QTableWidget* m_activationParamsTable;
    

    QPushButton* m_taskFileBrowseButton;
    QPushButton* m_cancelButton;
    QPushButton* m_applyButton;
    
    
    // 数据
    QJsonObject m_recipeData;
    QJsonObject m_chipTemplates;  // 存储不同芯片的参数模板
};

} // namespace Presentation

#endif // RECIPESTARTDIALOG_H 