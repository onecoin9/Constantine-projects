#include <QCoreApplication>
#include <QDebug>
#include "ChipTestDatabase.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 创建数据库实例
    ChipTestDatabase db("chip_test_example.db");
    
    // 初始化数据库
    if (!db.initializeDatabase()) {
        qDebug() << "Failed to initialize database:" << db.getLastError();
        return -1;
    }
    
    qDebug() << "Database initialized successfully!";
    
    // 示例数据
    ChipTestData testData;
    testData.uid = "CHIP_001_20240829_001";
    testData.chipModel = "MTI270";
    testData.batchNumber = "BATCH_20240829_001";
    testData.activationTime = "2024-08-29 10:30:00";
    testData.markingNumber = "MRK_001";
    
    // 设置测量数据
    testData.workingCurrentMeasured = 3.2;
    testData.workingCurrentReference = 3.0;
    testData.pllDcVoltageMeasured = 5.1;
    testData.pllDcVoltageReference = 5.0;
    testData.driveDcVoltageMeasured = 12.1;
    testData.driveDcVoltageReference = 12.0;
    testData.driveDcPeakVoltageMeasured = 24.2;
    testData.driveDcPeakVoltageReference = 24.0;
    testData.squareWaveFreqMeasured = 1000.5;
    testData.squareWaveFreqReference = 1000.0;
    testData.sineWaveVoltageAvgMeasured = 15.1;
    testData.sineWaveVoltageAvgReference = 15.0;
    testData.sineWavePeakVoltageMeasured = 30.2;
    testData.sineWavePeakVoltageReference = 30.0;
    testData.sineWaveFreqMeasured = 50.1;
    testData.sineWaveFreqReference = 50.0;
    testData.activationOkNg = true;
    testData.turntableCalibrationOkNg = true;
    
    // 1. 插入数据测试
    qDebug() << "=== Insert Test ===";
    if (db.insertChipData(testData)) {
        qDebug() << "Data inserted successfully!";
    } else {
        qDebug() << "Failed to insert data:" << db.getLastError();
    }
    
    // 2. 查询单个数据测试
    qDebug() << "\n=== Query Single Data Test ===";
    ChipTestData retrievedData = db.getChipData("CHIP_001_20240829_001");
    if (!retrievedData.uid.isEmpty()) {
        qDebug() << "Retrieved data:";
        qDebug() << "  UID:" << retrievedData.uid;
        qDebug() << "  Chip Model:" << retrievedData.chipModel;
        qDebug() << "  Batch Number:" << retrievedData.batchNumber;
        qDebug() << "  Working Current (Measured/Reference):" 
                 << retrievedData.workingCurrentMeasured << "/" 
                 << retrievedData.workingCurrentReference;
        qDebug() << "  Activation OK:" << (retrievedData.activationOkNg ? "YES" : "NO");
        qDebug() << "  Turntable Calibration OK:" << (retrievedData.turntableCalibrationOkNg ? "YES" : "NO");
        qDebug() << "  Created At:" << retrievedData.createdAt;
    } else {
        qDebug() << "No data found or error:" << db.getLastError();
    }
    
    // 3. 插入更多测试数据
    qDebug() << "\n=== Insert More Test Data ===";
    QStringList chipModels = {"MTI300D", "MSG300D", "MSA300D"};
    for (int i = 0; i < 3; ++i) {
        ChipTestData moreData = testData;
        moreData.uid = QString("CHIP_00%1_20240829_00%1").arg(i + 2);
        moreData.chipModel = chipModels[i];
        moreData.batchNumber = QString("BATCH_20240829_00%1").arg(i + 2);
        moreData.workingCurrentMeasured += i * 0.1;
        moreData.activationOkNg = (i % 2 == 0);
        
        if (db.insertChipData(moreData)) {
            qDebug() << "Inserted data for UID:" << moreData.uid;
        }
    }
    
    // 4. 查询所有数据测试
    qDebug() << "\n=== Query All Data Test ===";
    QList<ChipTestData> allData = db.getAllChipData();
    qDebug() << "Total records found:" << allData.size();
    for (const ChipTestData &data : allData) {
        qDebug() << QString("  UID: %1, Model: %2, Activation: %3")
                    .arg(data.uid)
                    .arg(data.chipModel)
                    .arg(data.activationOkNg ? "OK" : "NG");
    }
    
    // 5. 按芯片型号查询测试
    qDebug() << "\n=== Query By Chip Model Test ===";
    QList<ChipTestData> mti270Data = db.getChipDataByModel("MTI270");
    qDebug() << "MTI270 records found:" << mti270Data.size();
    
    // 6. 更新数据测试
    qDebug() << "\n=== Update Data Test ===";
    testData.workingCurrentMeasured = 3.5;  // 修改测量值
    testData.activationOkNg = false;        // 修改测试结果
    if (db.updateChipData(testData)) {
        qDebug() << "Data updated successfully!";
        
        // 验证更新结果
        ChipTestData updatedData = db.getChipData(testData.uid);
        qDebug() << "Updated working current:" << updatedData.workingCurrentMeasured;
        qDebug() << "Updated activation status:" << (updatedData.activationOkNg ? "OK" : "NG");
    } else {
        qDebug() << "Failed to update data:" << db.getLastError();
    }
    
    // 7. 统计信息测试
    qDebug() << "\n=== Statistics Test ===";
    qDebug() << "Total chip count:" << db.getChipCount();
    qDebug() << "MTI270 chip count:" << db.getChipCountByModel("MTI270");
    
    QStringList models = db.getAllChipModels();
    qDebug() << "All chip models:" << models;
    
    QStringList batches = db.getAllBatchNumbers();
    qDebug() << "All batch numbers:" << batches;
    
    // 8. 日期范围查询测试
    qDebug() << "\n=== Date Range Query Test ===";
    QDateTime startTime = QDateTime::currentDateTime().addDays(-1);
    QDateTime endTime = QDateTime::currentDateTime().addDays(1);
    QList<ChipTestData> dateRangeData = db.getChipDataByDateRange(startTime, endTime);
    qDebug() << "Records in date range:" << dateRangeData.size();
    
    // 9. 删除数据测试
    qDebug() << "\n=== Delete Data Test ===";
    if (db.deleteChipData("CHIP_002_20240829_002")) {
        qDebug() << "Data deleted successfully!";
        qDebug() << "Remaining records:" << db.getChipCount();
    } else {
        qDebug() << "Failed to delete data:" << db.getLastError();
    }
    
    // 10. 执行SQL文件测试 (可选)
    qDebug() << "\n=== Execute SQL File Test ===";
    // 如果有SQL文件，可以这样执行：
    // if (db.executeSqlFile("chip_test_database.sql")) {
    //     qDebug() << "SQL file executed successfully!";
    // }
    
    qDebug() << "\n=== All Tests Completed ===";
    
    // 关闭数据库连接
    db.closeDatabase();
    
    return 0;
}