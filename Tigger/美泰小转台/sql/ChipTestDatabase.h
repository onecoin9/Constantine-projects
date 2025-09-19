#ifndef CHIPTESTDATABASE_H
#define CHIPTESTDATABASE_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QString>
#include <QVariant>
#include <QDateTime>
#include <QDebug>

// 芯片测试数据结构体
struct ChipTestData {
    QString uid;                              // 芯片uid (复合主键一部分)
    QString chipModel;                        // 芯片型号
    QString lotid;                            // 批次ID (复合主键一部分)
    QString activationTime;                   // 激活时间
    QString markingNumber;                    // 打标号
    
    // 工作电流
    double workingCurrentMeasured = 0.0;
    double workingCurrentReference = 0.0;
    
    // 锁相环直流电压
    double pllDcVoltageMeasured = 0.0;
    double pllDcVoltageReference = 0.0;
    
    // 驱动直流电压
    double driveDcVoltageMeasured = 0.0;
    double driveDcVoltageReference = 0.0;
    
    // 驱动直流电压峰峰值
    double driveDcPeakVoltageMeasured = 0.0;
    double driveDcPeakVoltageReference = 0.0;
    
    // 方波频率
    double squareWaveFreqMeasured = 0.0;
    double squareWaveFreqReference = 0.0;
    
    // 正弦波电压均值
    double sineWaveVoltageAvgMeasured = 0.0;
    double sineWaveVoltageAvgReference = 0.0;
    
    // 正弦波电压峰峰值
    double sineWavePeakVoltageMeasured = 0.0;
    double sineWavePeakVoltageReference = 0.0;
    
    // 正弦波频率
    double sineWaveFreqMeasured = 0.0;
    double sineWaveFreqReference = 0.0;
    
    // 测试结果
    bool activationOkNg = false;              // 激活ok/ng
    bool turntableCalibrationOkNg = false;    // 小转台标定ok/ng
    
    // 设备和文件信息
    QString machineNumber;                    // 机器号
    QString workflowFile;                     // 工作流文件
    QString automationTaskFile;               // 自动化任务文件
    QString burnTaskFile;                     // 烧录任务文件
    
    // OS测试相关
    bool osTestResult = false;                // OS测试结果
    QString osTestDetails;                    // OS测试详细信息
    QString calibrationParams;                // 标定参数
    
    // 预留参数字段
    QString param1;                           // 预留参数1
    QString param2;                           // 预留参数2
    QString param3;                           // 预留参数3
    QString param4;                           // 预留参数4
    QString param5;                           // 预留参数5
    QString param6;                           // 预留参数6
    QString param7;                           // 预留参数7
    QString param8;                           // 预留参数8
    QString param9;                           // 预留参数9
    QString param10;                          // 预留参数10
    
    // 时间戳
    QDateTime createdAt;
    QDateTime updatedAt;
    
    ChipTestData() = default;
};

class ChipTestDatabase
{
public:
    explicit ChipTestDatabase(const QString &databasePath = "chip_test.db");
    ~ChipTestDatabase();
    
    // 数据库操作
    bool initializeDatabase();
    bool isConnected() const;
    void closeDatabase();
    
    // CRUD 操作
    bool insertChipData(const ChipTestData &data);
    bool updateChipData(const ChipTestData &data);
    bool deleteChipData(const QString &uid, const QString &lotid);
    ChipTestData getChipData(const QString &uid, const QString &lotid);
    QList<ChipTestData> getAllChipData();
    
    // 查询操作
    QList<ChipTestData> getChipDataByModel(const QString &chipModel);
    QList<ChipTestData> getChipDataByLot(const QString &lotid);
    QList<ChipTestData> getChipDataByDateRange(const QDateTime &startTime, const QDateTime &endTime);
    
    // 统计操作
    int getChipCount();
    int getChipCountByModel(const QString &chipModel);
    QStringList getAllChipModels();
    QStringList getAllLotIds();
    
    // 工具方法
    QString getLastError() const;
    bool executeSqlFile(const QString &sqlFilePath);
    
private:
    QString m_databasePath;
    QSqlDatabase m_database;
    QString m_lastError;
    
    void setLastError(const QString &error);
    bool createTables();
    ChipTestData queryToChipData(const QSqlQuery &query);
};

#endif // CHIPTESTDATABASE_H