#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QWidget> // Include QWidget base class
#include <QString>
#include <QList>
#include <QComboBox>
#include <QThread>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QRandomGenerator>
#include <QDebug>
#include <QList>
#include <QString>
#include <QVariantMap>
#include <QDateTime>
#include <QPair>

namespace Ui {
    class ConfigDialog;
}

// 测试项数据结构
struct TestItem {
    QString id;
    QString className;
    QString alias;
    QString testMode;
    QString loopCount;
    QString passDo;
    QString failDo;
    QString comment;
};

// Spec项数据结构
struct SpecItem {
    QString id;
    QString testItemName;
    QString aliasName;
    QString params;
    QString value;
    QString lowerLimit;
    QString upperLimit;
    QString binName;
    QString comment;
};

// 新增: 测试计划项数据结构
struct TestPlanItem {
    QString testItem;        // 测试项
    QString isLowTemp;       // 低温
    QString powerSupply;     // 电源供电
    QString voltage;         // 电压
    QString powerStableTime; // 电源稳定时间
    QString chamberMode;     // 温箱(定值/斜坡/不控制)
    QString temperature;     // 温度
    QString tempStableTime;  // 温度稳定时间
    QString funcTest;        // 是否进行功能测试
    QString dataAcq;         // 是否采集
    QString acqFrequency;    // 采集频率s
    QString acqTime;         // 采集时间
};

// 集成了Flow和Spec的测试项结构体
struct IntegratedTestItem {
    // Flow部分
    QString id;
    QString className;
    QString alias;
    QString testMode;
    QString loopCount;
    QString passDo;
    QString failDo;
    QString comment;
    
    // Spec部分 (一个Flow项可能对应多个Spec项)
    QList<SpecItem> specItems;
};

// 整体测试流程结构体
struct ChipTestFlow {
    QList<IntegratedTestItem> testItems;
    QString flowName;
    QString description;
    // 可以添加其他需要的属性
};

class ConfigDialog : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget* parent = nullptr);
    ~ConfigDialog();

private slots:
    // 现有的槽函数
    void on_btnInport_clicked(const QString& buttonId);
    void on_tableFlowCellChanged(int row, int column);
    void on_tableSpecCellChanged(int row, int column);
    void on_tableFlowTestModeChanged(int row, const QString& mode);
    void on_btnExportFlow_clicked();
    //void on_btnExportSpec_clicked();
    void on_btnAddFlow_clicked();
    void on_btnInsertFlow_clicked();
    void on_btnDeleteFlow_clicked();
    void on_btnMoveUpFlow_clicked();
    void on_btnMoveDownFlow_clicked();

    // 新增的测试计划相关槽函数
    void on_btnImportTestPlan_clicked();
    void on_btnExportTestPlan_clicked();
    void on_btnAddTestPlan_clicked();
    void on_btnInsertTestPlan_clicked();
    void on_btnDeleteTestPlan_clicked();
    void on_chamberModeChanged(int row, const QString& mode);
    void on_funcTestChanged(int row, const QString& value);
    void on_dataAcqChanged(int row, const QString& value);
    void on_tableTestPlanCellChanged(int row, int column);
    void on_isLowTempChanged(int row, const QString& value);
private:
    Ui::ConfigDialog* ui;
    QString tableStyleSheet;

    // 现有的数据列表
    QList<TestItem> testItems;
    QList<SpecItem> specItems;

    // 新增的测试计划数据列表
    QList<TestPlanItem> testPlanItems;

    // 现有的初始化函数
    void initTableFlow();
    void initTableSpec();
    void initTableBin();
    void initTablePin();
    void updateTableFlow();
    void updateTableSpec();
    void loadFlowConfig(const QJsonObject& flowConfig);
    void populateTableSpecRow(int row, const SpecItem& item);
    void updateSpecFromFlow(const TestItem& flowItem);
    QStringList parseCommentParams(const QString& comment);
    void generateSpecItems(const TestItem& flowItem, const QStringList& params);
    QString generateSpecItemID(const QString& baseID, int index);
    void syncSpecWithFlow();

    // 新增的测试计划相关函数
    void initTableTestPlan();
    void updateTableTestPlan();
    void populateTableTestPlanRow(int row, const TestPlanItem& item);
    void setupComboBoxForTestPlanTable(int row);

public slots:
    // 主流程处理函数
    void executeTestPlan();
    void processTestFlowItem(int testIndex);

private:
    // 各个模块实现函数
    void openColdWaterValve();
    void setupPowerSupply(const QString& supplyType);
    void setVoltage(double voltage);
    void waitForPowerStable(int seconds);
    void setFixedTemperature(double temperature);
    void setRampTemperature(double temperature);
    void waitForTemperatureStable(int seconds);
    void waitForTemperatureReach(int seconds);
    QString readDeviceID();
    bool executeFunctionTest(int testIndex, const QString& testName);
    void startDataAcquisition(int frequency, int duration);
    void saveAcquisitionData(const QList<QPair<QDateTime, QVariantMap>>& data);

    // ChipTest Flow相关方法
    ChipTestFlow generateChipTestFlow();
    void executeChipTestFlow();
    void prepareForTest(const IntegratedTestItem& item);
    void setupTestParam(const SpecItem& spec);
    bool executeTest(const IntegratedTestItem& item);
    double simulateMeasurement(const SpecItem& spec);
    void executePassDo(const IntegratedTestItem& item);
    void executeFailDo(const IntegratedTestItem& item);

private:
    // 测试环境设置相关方法
    void setupTestEnvironment(const TestPlanItem& item);
    void handleTemperatureSettings(const TestPlanItem& item);
    void handleDataAcquisition(const TestPlanItem& item);
};

#endif // CONFIGDIALOG_H