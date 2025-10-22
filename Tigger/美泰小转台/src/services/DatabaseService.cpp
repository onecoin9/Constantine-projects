#include "services/DatabaseService.h"
#include "sql/ChipTestDatabase.h"
#include "core/Logger.h"
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>

namespace Services {

DatabaseService& DatabaseService::getInstance()
{
    static DatabaseService instance;
    return instance;
}

DatabaseService::DatabaseService(QObject *parent)
    : QObject(parent)
    , m_database(nullptr)
    , m_isInitialized(false)
{
}

DatabaseService::~DatabaseService()
{
    closeDatabase();
}

bool DatabaseService::initializeDatabase(const QString &databasePath)
{
    try {
        // 确保数据库目录存在
        QFileInfo fileInfo(databasePath);
        QDir dir = fileInfo.absoluteDir();
        if (!dir.exists()) {
            if (!dir.mkpath(".")) {
                setLastError(QString("无法创建数据库目录: %1").arg(dir.absolutePath()));
                return false;
            }
        }
        
        m_databasePath = databasePath;
        m_database = std::make_unique<ChipTestDatabase>(databasePath);
        
        if (!m_database->initializeDatabase()) {
            setLastError(m_database->getLastError());
            return false;
        }
        
        m_isInitialized = true;
        
        emit databaseInitialized(databasePath);
        LOG_MODULE_INFO("DatabaseService", QString("数据库服务初始化成功: %1").arg(databasePath).toStdString());
        
        return true;
        
    } catch (const std::exception &e) {
        setLastError(QString("数据库初始化异常: %1").arg(e.what()));
        emit databaseError(m_lastError);
        return false;
    }
}

void DatabaseService::closeDatabase()
{
    if (m_database) {
        m_database->closeDatabase();
        m_database.reset();
    }
    m_isInitialized = false;
}

bool DatabaseService::insertChipTestData(const ChipTestData &data)
{
    if (!m_isInitialized || !m_database) {
        setLastError("数据库未初始化");
        return false;
    }
    
    if (!validateChipData(data)) {
        setLastError("数据验证失败");
        return false;
    }
    
    bool success = m_database->insertChipData(data);
    if (success) {
        emit dataInserted(data.uid, data.lotid);
        LOG_MODULE_INFO("DatabaseService", QString("成功插入芯片数据: %1 (批次: %2)").arg(data.uid).arg(data.lotid).toStdString());
    } else {
        setLastError(m_database->getLastError());
        LOG_MODULE_ERROR("DatabaseService", QString("插入芯片数据失败: %1").arg(m_lastError).toStdString());
    }
    
    return success;
}

bool DatabaseService::insertChipTestDataFromJson(const QJsonObject &jsonData)
{
    try {
        ChipTestData data = jsonToChipData(jsonData);
        return insertChipTestData(data);
    } catch (const std::exception &e) {
        setLastError(QString("JSON数据解析失败: %1").arg(e.what()));
        return false;
    }
}

bool DatabaseService::insertChipTestDataFromCsv(const QString &csvFilePath)
{
    if (!m_isInitialized || !m_database) {
        setLastError("数据库未初始化");
        return false;
    }
    
    QFile file(csvFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setLastError(QString("无法打开CSV文件: %1").arg(csvFilePath));
        return false;
    }
    
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    
    QList<ChipTestData> dataList;
    bool isFirstLine = true;
    int lineNumber = 0;
    int successCount = 0;
    int failCount = 0;
    
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        lineNumber++;
        
        if (line.isEmpty()) {
            continue;
        }
        
        // 跳过表头
        if (isFirstLine) {
            isFirstLine = false;
            continue;
        }
        
        try {
            QStringList fields = line.split(',');
            ChipTestData data = parseCsvLine(fields);
            
            if (validateChipData(data)) {
                if (m_database->insertChipData(data)) {
                    successCount++;
                    emit dataInserted(data.uid, data.lotid);
                } else {
                    failCount++;
                    LOG_MODULE_WARNING("DatabaseService", 
                        QString("CSV第%1行插入失败: %2").arg(lineNumber).arg(m_database->getLastError()).toStdString());
                }
            } else {
                failCount++;
                LOG_MODULE_WARNING("DatabaseService", 
                    QString("CSV第%1行数据验证失败").arg(lineNumber).toStdString());
            }
        } catch (const std::exception &e) {
            failCount++;
            LOG_MODULE_WARNING("DatabaseService", 
                QString("CSV第%1行解析失败: %2").arg(lineNumber).arg(e.what()).toStdString());
        }
    }
    
    file.close();
    
    if (successCount > 0) {
        emit batchDataInserted(successCount);
    }
    
    LOG_MODULE_INFO("DatabaseService", 
        QString("CSV导入完成: 成功%1条，失败%2条").arg(successCount).arg(failCount).toStdString());
    
    return failCount == 0;
}

bool DatabaseService::insertBatchChipTestData(const QList<ChipTestData> &dataList)
{
    if (!m_isInitialized || !m_database) {
        setLastError("数据库未初始化");
        return false;
    }
    
    int successCount = 0;
    int failCount = 0;
    
    for (const ChipTestData &data : dataList) {
        if (validateChipData(data)) {
            if (m_database->insertChipData(data)) {
                successCount++;
                emit dataInserted(data.uid, data.lotid);
            } else {
                failCount++;
                LOG_MODULE_WARNING("DatabaseService", 
                    QString("批量插入失败 [%1]: %2").arg(data.uid).arg(m_database->getLastError()).toStdString());
            }
        } else {
            failCount++;
        }
    }
    
    if (successCount > 0) {
        emit batchDataInserted(successCount);
    }
    
    LOG_MODULE_INFO("DatabaseService", 
        QString("批量插入完成: 成功%1条，失败%2条").arg(successCount).arg(failCount).toStdString());
    
    return failCount == 0;
}

QList<ChipTestData> DatabaseService::getAllChipData()
{
    if (!m_isInitialized || !m_database) {
        return QList<ChipTestData>();
    }
    
    return m_database->getAllChipData();
}

QList<ChipTestData> DatabaseService::getChipDataByModel(const QString &chipModel)
{
    if (!m_isInitialized || !m_database) {
        return QList<ChipTestData>();
    }
    
    return m_database->getChipDataByModel(chipModel);
}

QList<ChipTestData> DatabaseService::getChipDataByLot(const QString &lotid)
{
    if (!m_isInitialized || !m_database) {
        return QList<ChipTestData>();
    }
    
    return m_database->getChipDataByLot(lotid);
}

QList<ChipTestData> DatabaseService::getChipDataByDateRange(const QDateTime &startTime, const QDateTime &endTime)
{
    if (!m_isInitialized || !m_database) {
        return QList<ChipTestData>();
    }
    
    return m_database->getChipDataByDateRange(startTime, endTime);
}

ChipTestData DatabaseService::getChipData(const QString &uid, const QString &lotid)
{
    if (!m_isInitialized || !m_database) {
        return ChipTestData();
    }
    
    return m_database->getChipData(uid, lotid);
}

int DatabaseService::getTotalChipCount()
{
    if (!m_isInitialized || !m_database) {
        return 0;
    }
    
    return m_database->getChipCount();
}

int DatabaseService::getChipCountByModel(const QString &chipModel)
{
    if (!m_isInitialized || !m_database) {
        return 0;
    }
    
    return m_database->getChipCountByModel(chipModel);
}

QStringList DatabaseService::getAllChipModels()
{
    if (!m_isInitialized || !m_database) {
        return QStringList();
    }
    
    return m_database->getAllChipModels();
}

QStringList DatabaseService::getAllLotIds()
{
    if (!m_isInitialized || !m_database) {
        return QStringList();
    }
    
    return m_database->getAllLotIds();
}

QString DatabaseService::getLastError() const
{
    return m_lastError;
}

void DatabaseService::onTestDataAvailable(const QJsonObject &testResult)
{
    try {
        // 从测试结果JSON中提取数据并插入数据库
        ChipTestData data = jsonToChipData(testResult);
        insertChipTestData(data);
    } catch (const std::exception &e) {
        LOG_MODULE_ERROR("DatabaseService", 
            QString("处理测试数据失败: %1").arg(e.what()).toStdString());
    }
}

void DatabaseService::onCsvDataImport(const QString &csvFilePath)
{
    insertChipTestDataFromCsv(csvFilePath);
}

void DatabaseService::onActivationDataReceived(const QJsonObject &activationData)
{
    try {
        // 处理激活过程数据
        QString uid = activationData["uid"].toString();
        QString lotid = activationData["lotid"].toString();
        
        if (!uid.isEmpty() && !lotid.isEmpty()) {
            // 尝试获取现有数据并更新
            ChipTestData existingData = getChipData(uid, lotid);
            if (existingData.uid.isEmpty()) {
                // 创建新记录
                ChipTestData newData;
                newData.uid = uid;
                newData.lotid = lotid;
                newData.activationTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
                newData.activationOkNg = activationData["result"].toBool();
                
                insertChipTestData(newData);
            } else {
                // 更新现有记录
                existingData.activationOkNg = activationData["result"].toBool();
                m_database->updateChipData(existingData);
                emit dataUpdated(uid, lotid);
            }
        }
    } catch (const std::exception &e) {
        LOG_MODULE_ERROR("DatabaseService", 
            QString("处理激活数据失败: %1").arg(e.what()).toStdString());
    }
}

void DatabaseService::onCalibrationDataReceived(const QJsonObject &calibrationData)
{
    try {
        // 处理标定数据
        QString uid = calibrationData["uid"].toString();
        QString lotid = calibrationData["lotid"].toString();
        
        if (!uid.isEmpty() && !lotid.isEmpty()) {
            ChipTestData existingData = getChipData(uid, lotid);
            if (!existingData.uid.isEmpty()) {
                // 更新标定结果
                existingData.turntableCalibrationOkNg = calibrationData["result"].toBool();
                existingData.calibrationParams = QJsonDocument(calibrationData).toJson(QJsonDocument::Compact);
                
                m_database->updateChipData(existingData);
                emit dataUpdated(uid, lotid);
            }
        }
    } catch (const std::exception &e) {
        LOG_MODULE_ERROR("DatabaseService", 
            QString("处理标定数据失败: %1").arg(e.what()).toStdString());
    }
}

ChipTestData DatabaseService::jsonToChipData(const QJsonObject &jsonData)
{
    ChipTestData data;
    
    // 基本信息
    data.uid = jsonData["uid"].toString();
    data.chipModel = jsonData["chip_model"].toString();
    data.lotid = jsonData["lotid"].toString();
    data.activationTime = jsonData["activation_time"].toString();
    data.markingNumber = jsonData["marking_number"].toString();
    
    // 测量数据
    data.workingCurrentMeasured = jsonData["working_current_measured"].toDouble();
    data.workingCurrentReference = jsonData["working_current_reference"].toDouble();
    data.pllDcVoltageMeasured = jsonData["pll_dc_voltage_measured"].toDouble();
    data.pllDcVoltageReference = jsonData["pll_dc_voltage_reference"].toDouble();
    data.driveDcVoltageMeasured = jsonData["drive_dc_voltage_measured"].toDouble();
    data.driveDcVoltageReference = jsonData["drive_dc_voltage_reference"].toDouble();
    data.driveDcPeakVoltageMeasured = jsonData["drive_dc_peak_voltage_measured"].toDouble();
    data.driveDcPeakVoltageReference = jsonData["drive_dc_peak_voltage_reference"].toDouble();
    data.squareWaveFreqMeasured = jsonData["square_wave_freq_measured"].toDouble();
    data.squareWaveFreqReference = jsonData["square_wave_freq_reference"].toDouble();
    data.sineWaveVoltageAvgMeasured = jsonData["sine_wave_voltage_avg_measured"].toDouble();
    data.sineWaveVoltageAvgReference = jsonData["sine_wave_voltage_avg_reference"].toDouble();
    data.sineWavePeakVoltageMeasured = jsonData["sine_wave_peak_voltage_measured"].toDouble();
    data.sineWavePeakVoltageReference = jsonData["sine_wave_peak_voltage_reference"].toDouble();
    data.sineWaveFreqMeasured = jsonData["sine_wave_freq_measured"].toDouble();
    data.sineWaveFreqReference = jsonData["sine_wave_freq_reference"].toDouble();
    
    // 测试结果
    data.activationOkNg = jsonData["activation_ok_ng"].toBool();
    data.turntableCalibrationOkNg = jsonData["turntable_calibration_ok_ng"].toBool();
    data.osTestResult = jsonData["os_test_result"].toBool();
    
    // 文件信息
    data.machineNumber = jsonData["machine_number"].toString();
    data.workflowFile = jsonData["workflow_file"].toString();
    data.automationTaskFile = jsonData["automation_task_file"].toString();
    data.burnTaskFile = jsonData["burn_task_file"].toString();
    
    // 其他信息
    data.osTestDetails = jsonData["os_test_details"].toString();
    data.calibrationParams = jsonData["calibration_params"].toString();
    
    // 预留参数
    data.param1 = jsonData["param1"].toString();
    data.param2 = jsonData["param2"].toString();
    data.param3 = jsonData["param3"].toString();
    data.param4 = jsonData["param4"].toString();
    data.param5 = jsonData["param5"].toString();
    data.param6 = jsonData["param6"].toString();
    data.param7 = jsonData["param7"].toString();
    data.param8 = jsonData["param8"].toString();
    data.param9 = jsonData["param9"].toString();
    data.param10 = jsonData["param10"].toString();
    
    return data;
}

QJsonObject DatabaseService::chipDataToJson(const ChipTestData &data)
{
    QJsonObject jsonData;
    
    // 基本信息
    jsonData["uid"] = data.uid;
    jsonData["chip_model"] = data.chipModel;
    jsonData["lotid"] = data.lotid;
    jsonData["activation_time"] = data.activationTime;
    jsonData["marking_number"] = data.markingNumber;
    
    // 测量数据
    jsonData["working_current_measured"] = data.workingCurrentMeasured;
    jsonData["working_current_reference"] = data.workingCurrentReference;
    jsonData["pll_dc_voltage_measured"] = data.pllDcVoltageMeasured;
    jsonData["pll_dc_voltage_reference"] = data.pllDcVoltageReference;
    jsonData["drive_dc_voltage_measured"] = data.driveDcVoltageMeasured;
    jsonData["drive_dc_voltage_reference"] = data.driveDcVoltageReference;
    jsonData["drive_dc_peak_voltage_measured"] = data.driveDcPeakVoltageMeasured;
    jsonData["drive_dc_peak_voltage_reference"] = data.driveDcPeakVoltageReference;
    jsonData["square_wave_freq_measured"] = data.squareWaveFreqMeasured;
    jsonData["square_wave_freq_reference"] = data.squareWaveFreqReference;
    jsonData["sine_wave_voltage_avg_measured"] = data.sineWaveVoltageAvgMeasured;
    jsonData["sine_wave_voltage_avg_reference"] = data.sineWaveVoltageAvgReference;
    jsonData["sine_wave_peak_voltage_measured"] = data.sineWavePeakVoltageMeasured;
    jsonData["sine_wave_peak_voltage_reference"] = data.sineWavePeakVoltageReference;
    jsonData["sine_wave_freq_measured"] = data.sineWaveFreqMeasured;
    jsonData["sine_wave_freq_reference"] = data.sineWaveFreqReference;
    
    // 测试结果
    jsonData["activation_ok_ng"] = data.activationOkNg;
    jsonData["turntable_calibration_ok_ng"] = data.turntableCalibrationOkNg;
    jsonData["os_test_result"] = data.osTestResult;
    
    // 文件信息
    jsonData["machine_number"] = data.machineNumber;
    jsonData["workflow_file"] = data.workflowFile;
    jsonData["automation_task_file"] = data.automationTaskFile;
    jsonData["burn_task_file"] = data.burnTaskFile;
    
    // 其他信息
    jsonData["os_test_details"] = data.osTestDetails;
    jsonData["calibration_params"] = data.calibrationParams;
    
    // 预留参数
    jsonData["param1"] = data.param1;
    jsonData["param2"] = data.param2;
    jsonData["param3"] = data.param3;
    jsonData["param4"] = data.param4;
    jsonData["param5"] = data.param5;
    jsonData["param6"] = data.param6;
    jsonData["param7"] = data.param7;
    jsonData["param8"] = data.param8;
    jsonData["param9"] = data.param9;
    jsonData["param10"] = data.param10;
    
    return jsonData;
}

bool DatabaseService::validateChipData(const ChipTestData &data)
{
    // 基本验证
    if (data.uid.isEmpty() || data.lotid.isEmpty()) {
        setLastError("UID和批次号不能为空");
        return false;
    }
    
    // 可以添加更多验证逻辑
    return true;
}

void DatabaseService::setLastError(const QString &error)
{
    m_lastError = error;
    emit databaseError(error);
}

ChipTestData DatabaseService::parseCsvLine(const QStringList &fields)
{
    ChipTestData data;
    
    // 根据MT软件的CSV格式进行解析
    // 这里需要根据实际的CSV格式调整
    
    if (fields.size() >= 5) {
        data.uid = fields[0].trimmed();
        data.chipModel = fields[1].trimmed();
        data.lotid = fields[2].trimmed();
        data.activationTime = fields[3].trimmed();
        data.markingNumber = fields[4].trimmed();
    }
    
    // 解析测量数据（需要根据实际CSV格式调整索引）
    if (fields.size() >= 25) {
        bool ok;
        data.workingCurrentMeasured = fields[5].toDouble(&ok);
        if (ok) data.workingCurrentReference = fields[6].toDouble();
        if (ok) data.pllDcVoltageMeasured = fields[7].toDouble();
        if (ok) data.pllDcVoltageReference = fields[8].toDouble();
        if (ok) data.driveDcVoltageMeasured = fields[9].toDouble();
        if (ok) data.driveDcVoltageReference = fields[10].toDouble();
        if (ok) data.driveDcPeakVoltageMeasured = fields[11].toDouble();
        if (ok) data.driveDcPeakVoltageReference = fields[12].toDouble();
        if (ok) data.squareWaveFreqMeasured = fields[13].toDouble();
        if (ok) data.squareWaveFreqReference = fields[14].toDouble();
        if (ok) data.sineWaveVoltageAvgMeasured = fields[15].toDouble();
        if (ok) data.sineWaveVoltageAvgReference = fields[16].toDouble();
        if (ok) data.sineWavePeakVoltageMeasured = fields[17].toDouble();
        if (ok) data.sineWavePeakVoltageReference = fields[18].toDouble();
        if (ok) data.sineWaveFreqMeasured = fields[19].toDouble();
        if (ok) data.sineWaveFreqReference = fields[20].toDouble();
    }
    
    // 设置默认值
    data.activationOkNg = true;
    data.turntableCalibrationOkNg = true;
    data.osTestResult = true;
    
    return data;
}

} // namespace Services
