#ifndef DATABASESERVICE_H
#define DATABASESERVICE_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QDateTime>
#include <memory>

// 前向声明
class ChipTestDatabase;
struct ChipTestData;

namespace Services {

/**
 * @brief 数据库服务类
 * 提供数据库操作的统一接口，供其他模块调用
 */
class DatabaseService : public QObject
{
    Q_OBJECT

public:
    static DatabaseService& getInstance();
    
    // 数据库初始化
    bool initializeDatabase(const QString &databasePath = "data/chip_test.db");
    void closeDatabase();
    bool isInitialized() const { return m_isInitialized; }
    
    // 数据插入接口
    bool insertChipTestData(const ChipTestData &data);
    bool insertChipTestDataFromJson(const QJsonObject &jsonData);
    bool insertChipTestDataFromCsv(const QString &csvFilePath);
    
    // 批量插入
    bool insertBatchChipTestData(const QList<ChipTestData> &dataList);
    
    // 查询接口
    QList<ChipTestData> getAllChipData();
    QList<ChipTestData> getChipDataByModel(const QString &chipModel);
    QList<ChipTestData> getChipDataByLot(const QString &lotid);
    QList<ChipTestData> getChipDataByDateRange(const QDateTime &startTime, const QDateTime &endTime);
    ChipTestData getChipData(const QString &uid, const QString &lotid);
    
    // 统计接口
    int getTotalChipCount();
    int getChipCountByModel(const QString &chipModel);
    QStringList getAllChipModels();
    QStringList getAllLotIds();
    
    // 工具方法
    QString getLastError() const;

signals:
    // 数据变更信号
    void dataInserted(const QString &uid, const QString &lotid);
    void dataUpdated(const QString &uid, const QString &lotid);
    void dataDeleted(const QString &uid, const QString &lotid);
    void batchDataInserted(int count);
    
    // 状态信号
    void databaseInitialized(const QString &path);
    void databaseError(const QString &error);

public slots:
    // 外部调用接口
    void onTestDataAvailable(const QJsonObject &testResult);
    void onCsvDataImport(const QString &csvFilePath);
    void onActivationDataReceived(const QJsonObject &activationData);
    void onCalibrationDataReceived(const QJsonObject &calibrationData);

private:
    explicit DatabaseService(QObject *parent = nullptr);
    ~DatabaseService();
    
    // 禁用拷贝构造和赋值
    DatabaseService(const DatabaseService&) = delete;
    DatabaseService& operator=(const DatabaseService&) = delete;
    
    // 数据转换
    ChipTestData jsonToChipData(const QJsonObject &jsonData);
    QJsonObject chipDataToJson(const ChipTestData &data);
    ChipTestData parseCsvLine(const QStringList &fields);
    
    // 数据验证
    bool validateChipData(const ChipTestData &data);
    
    // 错误处理
    void setLastError(const QString &error);

private:
    std::unique_ptr<ChipTestDatabase> m_database;
    bool m_isInitialized;
    QString m_lastError;
    QString m_databasePath;
};

} // namespace Services

#endif // DATABASESERVICE_H
