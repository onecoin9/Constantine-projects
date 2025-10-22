// 数据库功能测试示例
// 这个文件展示如何使用数据库服务API

#include "services/DatabaseService.h"
#include "sql/ChipTestDatabase.h"
#include "core/Logger.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>
#include <QJsonObject>

void testDatabaseService()
{
    qDebug() << "=== 数据库服务测试开始 ===";
    
    // 1. 初始化数据库服务
    Services::DatabaseService& dbService = Services::DatabaseService::getInstance();
    
    if (!dbService.initializeDatabase("test_chip_data.db")) {
        qDebug() << "数据库初始化失败:" << dbService.getLastError();
        return;
    }
    
    qDebug() << "数据库初始化成功";
    
    // 2. 创建测试数据
    ChipTestData testData;
    testData.uid = "CHIP_TEST_001";
    testData.chipModel = "MTI270";
    testData.lotid = "BATCH_20241022_001";
    testData.activationTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    testData.markingNumber = "MRK_TEST_001";
    
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
    testData.osTestResult = true;
    
    testData.machineNumber = "MT_001";
    testData.workflowFile = "turntable_test.json";
    testData.automationTaskFile = "automation_task.json";
    testData.burnTaskFile = "burn_task.json";
    
    // 3. 插入数据测试
    qDebug() << "\n=== 插入数据测试 ===";
    if (dbService.insertChipTestData(testData)) {
        qDebug() << "数据插入成功!";
    } else {
        qDebug() << "数据插入失败:" << dbService.getLastError();
    }
    
    // 4. 查询数据测试
    qDebug() << "\n=== 查询数据测试 ===";
    ChipTestData retrievedData = dbService.getChipData(testData.uid, testData.lotid);
    if (!retrievedData.uid.isEmpty()) {
        qDebug() << "查询数据成功:";
        qDebug() << "  UID:" << retrievedData.uid;
        qDebug() << "  芯片型号:" << retrievedData.chipModel;
        qDebug() << "  批次ID:" << retrievedData.lotid;
        qDebug() << "  激活时间:" << retrievedData.activationTime;
        qDebug() << "  打标号:" << retrievedData.markingNumber;
        qDebug() << "  工作电流(测):" << retrievedData.workingCurrentMeasured;
        qDebug() << "  激活结果:" << (retrievedData.activationOkNg ? "通过" : "失败");
        qDebug() << "  标定结果:" << (retrievedData.turntableCalibrationOkNg ? "通过" : "失败");
    } else {
        qDebug() << "查询数据失败:" << dbService.getLastError();
    }
    
    // 5. 统计信息测试
    qDebug() << "\n=== 统计信息测试 ===";
    int totalCount = dbService.getTotalChipCount();
    qDebug() << "总记录数:" << totalCount;
    
    QStringList models = dbService.getAllChipModels();
    qDebug() << "芯片型号列表:" << models;
    
    QStringList lots = dbService.getAllLotIds();
    qDebug() << "批次列表:" << lots;
    
    // 6. JSON接口测试
    qDebug() << "\n=== JSON接口测试 ===";
    QJsonObject jsonData;
    jsonData["uid"] = "CHIP_JSON_001";
    jsonData["chip_model"] = "MTI270";
    jsonData["lotid"] = "BATCH_JSON_001";
    jsonData["activation_time"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    jsonData["marking_number"] = "MRK_JSON_001";
    jsonData["working_current_measured"] = 3.5;
    jsonData["working_current_reference"] = 3.0;
    jsonData["activation_ok_ng"] = true;
    jsonData["turntable_calibration_ok_ng"] = true;
    
    if (dbService.insertChipTestDataFromJson(jsonData)) {
        qDebug() << "JSON数据插入成功!";
    } else {
        qDebug() << "JSON数据插入失败:" << dbService.getLastError();
    }
    
    // 7. 批量查询测试
    qDebug() << "\n=== 批量查询测试 ===";
    QList<ChipTestData> allData = dbService.getAllChipData();
    qDebug() << "查询到" << allData.size() << "条记录";
    
    for (int i = 0; i < qMin(3, allData.size()); ++i) {
        const ChipTestData& data = allData[i];
        qDebug() << QString("  记录%1: %2 - %3 (%4)")
                    .arg(i+1)
                    .arg(data.uid)
                    .arg(data.chipModel)
                    .arg(data.activationOkNg ? "通过" : "失败");
    }
    
    qDebug() << "\n=== 数据库服务测试完成 ===";
}

// 注意：这个文件不会被编译到主程序中，仅作为使用示例
// 如果要运行测试，需要创建单独的测试项目或在main函数中调用
