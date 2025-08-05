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
     * @brief 设置当前配方文件路径
     */
    void setRecipeFilePath(const QString& filePath);

    /**
     * @brief 加载配方参数
     */
    void loadRecipeConfig();

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
    void onSaveClicked();
    void onCancelClicked();
    void onApplyClicked();
    void onChipTypeChanged(const QString& chipType);
    void onAddParameter();
    void onRemoveParameter();

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
    
    // UI组件
    QLineEdit* m_cmd3PathEdit;
    QLineEdit* m_taskFileNameEdit;
    QLineEdit* m_batchNumberEdit;
    QSpinBox* m_productionQuantitySpinBox;
    
    // 激活参数配置控件
    QTabWidget* m_tabWidget;
    QComboBox* m_chipTypeComboBox;
    QTableWidget* m_activationParamsTable;
    QPushButton* m_addParamButton;
    QPushButton* m_removeParamButton;
    
    QPushButton* m_cmd3BrowseButton;
    QPushButton* m_taskFileBrowseButton;
    QPushButton* m_saveButton;
    QPushButton* m_cancelButton;
    QPushButton* m_applyButton;
    
    QLabel* m_recipePathLabel;
    
    // 数据
    QString m_recipeFilePath;
    QJsonObject m_recipeData;
    QJsonObject m_chipTemplates;  // 存储不同芯片的参数模板
};

} // namespace Presentation

#endif // RECIPECONFIGDIALOG_H 