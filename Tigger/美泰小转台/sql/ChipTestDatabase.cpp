#include "ChipTestDatabase.h"
#include <QtSql/QSqlRecord>
#include <QDir>
#include <QFile>
#include <QTextStream>

ChipTestDatabase::ChipTestDatabase(const QString &databasePath)
    : m_databasePath(databasePath)
{
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(m_databasePath);
}

ChipTestDatabase::~ChipTestDatabase()
{
    closeDatabase();
}

bool ChipTestDatabase::initializeDatabase()
{
    if (!m_database.open()) {
        setLastError(QString("Failed to open database: %1").arg(m_database.lastError().text()));
        return false;
    }
    
    return createTables();
}

bool ChipTestDatabase::isConnected() const
{
    return m_database.isOpen();
}

void ChipTestDatabase::closeDatabase()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

bool ChipTestDatabase::createTables()
{
    QSqlQuery query(m_database);
    
    QString createTableSql = R"(
        CREATE TABLE IF NOT EXISTS chip_test_data (
            uid TEXT NOT NULL,
            chip_model TEXT NOT NULL,
            lotid TEXT NOT NULL,
            activation_time TEXT,
            marking_number TEXT,
            
            working_current_measured REAL,
            working_current_reference REAL,
            
            pll_dc_voltage_measured REAL,
            pll_dc_voltage_reference REAL,
            
            drive_dc_voltage_measured REAL,
            drive_dc_voltage_reference REAL,
            
            drive_dc_peak_voltage_measured REAL,
            drive_dc_peak_voltage_reference REAL,
            
            square_wave_freq_measured REAL,
            square_wave_freq_reference REAL,
            
            sine_wave_voltage_avg_measured REAL,
            sine_wave_voltage_avg_reference REAL,
            
            sine_wave_peak_voltage_measured REAL,
            sine_wave_peak_voltage_reference REAL,
            
            sine_wave_freq_measured REAL,
            sine_wave_freq_reference REAL,
            
            activation_ok_ng INTEGER DEFAULT 0,
            turntable_calibration_ok_ng INTEGER DEFAULT 0,
            
            machine_number TEXT,
            workflow_file TEXT,
            automation_task_file TEXT,
            burn_task_file TEXT,
            
            os_test_result INTEGER DEFAULT 0,
            os_test_details TEXT,
            calibration_params TEXT,
            
            param1 TEXT,
            param2 TEXT,
            param3 TEXT,
            param4 TEXT,
            param5 TEXT,
            param6 TEXT,
            param7 TEXT,
            param8 TEXT,
            param9 TEXT,
            param10 TEXT,
            
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            
            PRIMARY KEY (uid, lotid)
        )
    )";
    
    if (!query.exec(createTableSql)) {
        setLastError(QString("Failed to create table: %1").arg(query.lastError().text()));
        return false;
    }
    
    // 创建索引
    QStringList indexes = {
        "CREATE INDEX IF NOT EXISTS idx_chip_model ON chip_test_data(chip_model)",
        "CREATE INDEX IF NOT EXISTS idx_lotid ON chip_test_data(lotid)",
        "CREATE INDEX IF NOT EXISTS idx_activation_time ON chip_test_data(activation_time)",
        "CREATE INDEX IF NOT EXISTS idx_created_at ON chip_test_data(created_at)"
    };
    
    for (const QString &indexSql : indexes) {
        if (!query.exec(indexSql)) {
            qWarning() << "Failed to create index:" << query.lastError().text();
        }
    }
    
    // 创建更新触发器
    QString triggerSql = R"(
        CREATE TRIGGER IF NOT EXISTS update_chip_test_data_updated_at
            AFTER UPDATE ON chip_test_data
            FOR EACH ROW
        BEGIN
            UPDATE chip_test_data 
            SET updated_at = CURRENT_TIMESTAMP 
            WHERE uid = NEW.uid AND lotid = NEW.lotid;
        END
    )";
    
    if (!query.exec(triggerSql)) {
        qWarning() << "Failed to create trigger:" << query.lastError().text();
    }
    
    return true;
}

bool ChipTestDatabase::insertChipData(const ChipTestData &data)
{
    QSqlQuery query(m_database);
    
    QString sql = R"(
        INSERT INTO chip_test_data (
            uid, chip_model, lotid, activation_time, marking_number,
            working_current_measured, working_current_reference,
            pll_dc_voltage_measured, pll_dc_voltage_reference,
            drive_dc_voltage_measured, drive_dc_voltage_reference,
            drive_dc_peak_voltage_measured, drive_dc_peak_voltage_reference,
            square_wave_freq_measured, square_wave_freq_reference,
            sine_wave_voltage_avg_measured, sine_wave_voltage_avg_reference,
            sine_wave_peak_voltage_measured, sine_wave_peak_voltage_reference,
            sine_wave_freq_measured, sine_wave_freq_reference,
            activation_ok_ng, turntable_calibration_ok_ng,
            machine_number, workflow_file, automation_task_file, burn_task_file,
            os_test_result, os_test_details, calibration_params,
            param1, param2, param3, param4, param5, param6, param7, param8, param9, param10
        ) VALUES (
            :uid, :chip_model, :lotid, :activation_time, :marking_number,
            :working_current_measured, :working_current_reference,
            :pll_dc_voltage_measured, :pll_dc_voltage_reference,
            :drive_dc_voltage_measured, :drive_dc_voltage_reference,
            :drive_dc_peak_voltage_measured, :drive_dc_peak_voltage_reference,
            :square_wave_freq_measured, :square_wave_freq_reference,
            :sine_wave_voltage_avg_measured, :sine_wave_voltage_avg_reference,
            :sine_wave_peak_voltage_measured, :sine_wave_peak_voltage_reference,
            :sine_wave_freq_measured, :sine_wave_freq_reference,
            :activation_ok_ng, :turntable_calibration_ok_ng,
            :machine_number, :workflow_file, :automation_task_file, :burn_task_file,
            :os_test_result, :os_test_details, :calibration_params,
            :param1, :param2, :param3, :param4, :param5, :param6, :param7, :param8, :param9, :param10
        )
    )";
    
    query.prepare(sql);
    query.bindValue(":uid", data.uid);
    query.bindValue(":chip_model", data.chipModel);
    query.bindValue(":lotid", data.lotid);
    query.bindValue(":activation_time", data.activationTime);
    query.bindValue(":marking_number", data.markingNumber);
    
    query.bindValue(":working_current_measured", data.workingCurrentMeasured);
    query.bindValue(":working_current_reference", data.workingCurrentReference);
    query.bindValue(":pll_dc_voltage_measured", data.pllDcVoltageMeasured);
    query.bindValue(":pll_dc_voltage_reference", data.pllDcVoltageReference);
    query.bindValue(":drive_dc_voltage_measured", data.driveDcVoltageMeasured);
    query.bindValue(":drive_dc_voltage_reference", data.driveDcVoltageReference);
    query.bindValue(":drive_dc_peak_voltage_measured", data.driveDcPeakVoltageMeasured);
    query.bindValue(":drive_dc_peak_voltage_reference", data.driveDcPeakVoltageReference);
    query.bindValue(":square_wave_freq_measured", data.squareWaveFreqMeasured);
    query.bindValue(":square_wave_freq_reference", data.squareWaveFreqReference);
    query.bindValue(":sine_wave_voltage_avg_measured", data.sineWaveVoltageAvgMeasured);
    query.bindValue(":sine_wave_voltage_avg_reference", data.sineWaveVoltageAvgReference);
    query.bindValue(":sine_wave_peak_voltage_measured", data.sineWavePeakVoltageMeasured);
    query.bindValue(":sine_wave_peak_voltage_reference", data.sineWavePeakVoltageReference);
    query.bindValue(":sine_wave_freq_measured", data.sineWaveFreqMeasured);
    query.bindValue(":sine_wave_freq_reference", data.sineWaveFreqReference);
    
    query.bindValue(":activation_ok_ng", data.activationOkNg ? 1 : 0);
    query.bindValue(":turntable_calibration_ok_ng", data.turntableCalibrationOkNg ? 1 : 0);
    
    query.bindValue(":machine_number", data.machineNumber);
    query.bindValue(":workflow_file", data.workflowFile);
    query.bindValue(":automation_task_file", data.automationTaskFile);
    query.bindValue(":burn_task_file", data.burnTaskFile);
    
    query.bindValue(":os_test_result", data.osTestResult ? 1 : 0);
    query.bindValue(":os_test_details", data.osTestDetails);
    query.bindValue(":calibration_params", data.calibrationParams);
    
    query.bindValue(":param1", data.param1);
    query.bindValue(":param2", data.param2);
    query.bindValue(":param3", data.param3);
    query.bindValue(":param4", data.param4);
    query.bindValue(":param5", data.param5);
    query.bindValue(":param6", data.param6);
    query.bindValue(":param7", data.param7);
    query.bindValue(":param8", data.param8);
    query.bindValue(":param9", data.param9);
    query.bindValue(":param10", data.param10);
    
    if (!query.exec()) {
        setLastError(QString("Failed to insert data: %1").arg(query.lastError().text()));
        return false;
    }
    
    return true;
}

bool ChipTestDatabase::updateChipData(const ChipTestData &data)
{
    QSqlQuery query(m_database);
    
    QString sql = R"(
        UPDATE chip_test_data SET
            chip_model = :chip_model,
            lotid = :lotid,
            activation_time = :activation_time,
            marking_number = :marking_number,
            working_current_measured = :working_current_measured,
            working_current_reference = :working_current_reference,
            pll_dc_voltage_measured = :pll_dc_voltage_measured,
            pll_dc_voltage_reference = :pll_dc_voltage_reference,
            drive_dc_voltage_measured = :drive_dc_voltage_measured,
            drive_dc_voltage_reference = :drive_dc_voltage_reference,
            drive_dc_peak_voltage_measured = :drive_dc_peak_voltage_measured,
            drive_dc_peak_voltage_reference = :drive_dc_peak_voltage_reference,
            square_wave_freq_measured = :square_wave_freq_measured,
            square_wave_freq_reference = :square_wave_freq_reference,
            sine_wave_voltage_avg_measured = :sine_wave_voltage_avg_measured,
            sine_wave_voltage_avg_reference = :sine_wave_voltage_avg_reference,
            sine_wave_peak_voltage_measured = :sine_wave_peak_voltage_measured,
            sine_wave_peak_voltage_reference = :sine_wave_peak_voltage_reference,
            sine_wave_freq_measured = :sine_wave_freq_measured,
            sine_wave_freq_reference = :sine_wave_freq_reference,
            activation_ok_ng = :activation_ok_ng,
            turntable_calibration_ok_ng = :turntable_calibration_ok_ng,
            machine_number = :machine_number,
            workflow_file = :workflow_file,
            automation_task_file = :automation_task_file,
            burn_task_file = :burn_task_file,
            os_test_result = :os_test_result,
            os_test_details = :os_test_details,
            calibration_params = :calibration_params,
            param1 = :param1,
            param2 = :param2,
            param3 = :param3,
            param4 = :param4,
            param5 = :param5,
            param6 = :param6,
            param7 = :param7,
            param8 = :param8,
            param9 = :param9,
            param10 = :param10
        WHERE uid = :uid AND lotid = :lotid
    )";
    
    query.prepare(sql);
    query.bindValue(":uid", data.uid);
    query.bindValue(":chip_model", data.chipModel);
    query.bindValue(":lotid", data.lotid);
    query.bindValue(":activation_time", data.activationTime);
    query.bindValue(":marking_number", data.markingNumber);
    
    query.bindValue(":working_current_measured", data.workingCurrentMeasured);
    query.bindValue(":working_current_reference", data.workingCurrentReference);
    query.bindValue(":pll_dc_voltage_measured", data.pllDcVoltageMeasured);
    query.bindValue(":pll_dc_voltage_reference", data.pllDcVoltageReference);
    query.bindValue(":drive_dc_voltage_measured", data.driveDcVoltageMeasured);
    query.bindValue(":drive_dc_voltage_reference", data.driveDcVoltageReference);
    query.bindValue(":drive_dc_peak_voltage_measured", data.driveDcPeakVoltageMeasured);
    query.bindValue(":drive_dc_peak_voltage_reference", data.driveDcPeakVoltageReference);
    query.bindValue(":square_wave_freq_measured", data.squareWaveFreqMeasured);
    query.bindValue(":square_wave_freq_reference", data.squareWaveFreqReference);
    query.bindValue(":sine_wave_voltage_avg_measured", data.sineWaveVoltageAvgMeasured);
    query.bindValue(":sine_wave_voltage_avg_reference", data.sineWaveVoltageAvgReference);
    query.bindValue(":sine_wave_peak_voltage_measured", data.sineWavePeakVoltageMeasured);
    query.bindValue(":sine_wave_peak_voltage_reference", data.sineWavePeakVoltageReference);
    query.bindValue(":sine_wave_freq_measured", data.sineWaveFreqMeasured);
    query.bindValue(":sine_wave_freq_reference", data.sineWaveFreqReference);
    
    query.bindValue(":activation_ok_ng", data.activationOkNg ? 1 : 0);
    query.bindValue(":turntable_calibration_ok_ng", data.turntableCalibrationOkNg ? 1 : 0);
    
    query.bindValue(":machine_number", data.machineNumber);
    query.bindValue(":workflow_file", data.workflowFile);
    query.bindValue(":automation_task_file", data.automationTaskFile);
    query.bindValue(":burn_task_file", data.burnTaskFile);
    
    query.bindValue(":os_test_result", data.osTestResult ? 1 : 0);
    query.bindValue(":os_test_details", data.osTestDetails);
    query.bindValue(":calibration_params", data.calibrationParams);
    
    query.bindValue(":param1", data.param1);
    query.bindValue(":param2", data.param2);
    query.bindValue(":param3", data.param3);
    query.bindValue(":param4", data.param4);
    query.bindValue(":param5", data.param5);
    query.bindValue(":param6", data.param6);
    query.bindValue(":param7", data.param7);
    query.bindValue(":param8", data.param8);
    query.bindValue(":param9", data.param9);
    query.bindValue(":param10", data.param10);
    
    if (!query.exec()) {
        setLastError(QString("Failed to update data: %1").arg(query.lastError().text()));
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

bool ChipTestDatabase::deleteChipData(const QString &uid, const QString &lotid)
{
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM chip_test_data WHERE uid = :uid AND lotid = :lotid");
    query.bindValue(":uid", uid);
    query.bindValue(":lotid", lotid);
    
    if (!query.exec()) {
        setLastError(QString("Failed to delete data: %1").arg(query.lastError().text()));
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

ChipTestData ChipTestDatabase::getChipData(const QString &uid, const QString &lotid)
{
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM chip_test_data WHERE uid = :uid AND lotid = :lotid");
    query.bindValue(":uid", uid);
    query.bindValue(":lotid", lotid);
    
    if (query.exec() && query.next()) {
        return queryToChipData(query);
    }
    
    setLastError(QString("Chip data not found or query failed: %1").arg(query.lastError().text()));
    return ChipTestData();
}

QList<ChipTestData> ChipTestDatabase::getAllChipData()
{
    QList<ChipTestData> dataList;
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM chip_test_data ORDER BY created_at DESC");
    
    if (query.exec()) {
        while (query.next()) {
            dataList.append(queryToChipData(query));
        }
    } else {
        setLastError(QString("Failed to get all chip data: %1").arg(query.lastError().text()));
    }
    
    return dataList;
}

QList<ChipTestData> ChipTestDatabase::getChipDataByModel(const QString &chipModel)
{
    QList<ChipTestData> dataList;
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM chip_test_data WHERE chip_model = :chip_model ORDER BY created_at DESC");
    query.bindValue(":chip_model", chipModel);
    
    if (query.exec()) {
        while (query.next()) {
            dataList.append(queryToChipData(query));
        }
    } else {
        setLastError(QString("Failed to get chip data by model: %1").arg(query.lastError().text()));
    }
    
    return dataList;
}

QList<ChipTestData> ChipTestDatabase::getChipDataByLot(const QString &lotid)
{
    QList<ChipTestData> dataList;
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM chip_test_data WHERE lotid = :lotid ORDER BY created_at DESC");
    query.bindValue(":lotid", lotid);
    
    if (query.exec()) {
        while (query.next()) {
            dataList.append(queryToChipData(query));
        }
    } else {
        setLastError(QString("Failed to get chip data by batch: %1").arg(query.lastError().text()));
    }
    
    return dataList;
}

QList<ChipTestData> ChipTestDatabase::getChipDataByDateRange(const QDateTime &startTime, const QDateTime &endTime)
{
    QList<ChipTestData> dataList;
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM chip_test_data WHERE created_at BETWEEN :start_time AND :end_time ORDER BY created_at DESC");
    query.bindValue(":start_time", startTime);
    query.bindValue(":end_time", endTime);
    
    if (query.exec()) {
        while (query.next()) {
            dataList.append(queryToChipData(query));
        }
    } else {
        setLastError(QString("Failed to get chip data by date range: %1").arg(query.lastError().text()));
    }
    
    return dataList;
}

int ChipTestDatabase::getChipCount()
{
    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM chip_test_data");
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    
    return 0;
}

int ChipTestDatabase::getChipCountByModel(const QString &chipModel)
{
    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM chip_test_data WHERE chip_model = :chip_model");
    query.bindValue(":chip_model", chipModel);
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    
    return 0;
}

QStringList ChipTestDatabase::getAllChipModels()
{
    QStringList models;
    QSqlQuery query(m_database);
    query.prepare("SELECT DISTINCT chip_model FROM chip_test_data ORDER BY chip_model");
    
    if (query.exec()) {
        while (query.next()) {
            models.append(query.value(0).toString());
        }
    }
    
    return models;
}

QStringList ChipTestDatabase::getAllLotIds()
{
    QStringList lots;
    QSqlQuery query(m_database);
    query.prepare("SELECT DISTINCT lotid FROM chip_test_data ORDER BY lotid");
    
    if (query.exec()) {
        while (query.next()) {
            lots.append(query.value(0).toString());
        }
    }
    
    return lots;
}

QString ChipTestDatabase::getLastError() const
{
    return m_lastError;
}

bool ChipTestDatabase::executeSqlFile(const QString &sqlFilePath)
{
    QFile file(sqlFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setLastError(QString("Cannot open SQL file: %1").arg(sqlFilePath));
        return false;
    }
    
    QTextStream stream(&file);
    QString sqlContent = stream.readAll();
    file.close();
    
    QSqlQuery query(m_database);
    if (!query.exec(sqlContent)) {
        setLastError(QString("Failed to execute SQL file: %1").arg(query.lastError().text()));
        return false;
    }
    
    return true;
}

void ChipTestDatabase::setLastError(const QString &error)
{
    m_lastError = error;
    qDebug() << "ChipTestDatabase Error:" << error;
}

ChipTestData ChipTestDatabase::queryToChipData(const QSqlQuery &query)
{
    ChipTestData data;
    
    data.uid = query.value("uid").toString();
    data.chipModel = query.value("chip_model").toString();
    data.lotid = query.value("lotid").toString();
    data.activationTime = query.value("activation_time").toString();
    data.markingNumber = query.value("marking_number").toString();
    
    data.workingCurrentMeasured = query.value("working_current_measured").toDouble();
    data.workingCurrentReference = query.value("working_current_reference").toDouble();
    data.pllDcVoltageMeasured = query.value("pll_dc_voltage_measured").toDouble();
    data.pllDcVoltageReference = query.value("pll_dc_voltage_reference").toDouble();
    data.driveDcVoltageMeasured = query.value("drive_dc_voltage_measured").toDouble();
    data.driveDcVoltageReference = query.value("drive_dc_voltage_reference").toDouble();
    data.driveDcPeakVoltageMeasured = query.value("drive_dc_peak_voltage_measured").toDouble();
    data.driveDcPeakVoltageReference = query.value("drive_dc_peak_voltage_reference").toDouble();
    data.squareWaveFreqMeasured = query.value("square_wave_freq_measured").toDouble();
    data.squareWaveFreqReference = query.value("square_wave_freq_reference").toDouble();
    data.sineWaveVoltageAvgMeasured = query.value("sine_wave_voltage_avg_measured").toDouble();
    data.sineWaveVoltageAvgReference = query.value("sine_wave_voltage_avg_reference").toDouble();
    data.sineWavePeakVoltageMeasured = query.value("sine_wave_peak_voltage_measured").toDouble();
    data.sineWavePeakVoltageReference = query.value("sine_wave_peak_voltage_reference").toDouble();
    data.sineWaveFreqMeasured = query.value("sine_wave_freq_measured").toDouble();
    data.sineWaveFreqReference = query.value("sine_wave_freq_reference").toDouble();
    
    data.activationOkNg = query.value("activation_ok_ng").toInt() == 1;
    data.turntableCalibrationOkNg = query.value("turntable_calibration_ok_ng").toInt() == 1;
    
    data.machineNumber = query.value("machine_number").toString();
    data.workflowFile = query.value("workflow_file").toString();
    data.automationTaskFile = query.value("automation_task_file").toString();
    data.burnTaskFile = query.value("burn_task_file").toString();
    
    data.osTestResult = query.value("os_test_result").toInt() == 1;
    data.osTestDetails = query.value("os_test_details").toString();
    data.calibrationParams = query.value("calibration_params").toString();
    
    data.param1 = query.value("param1").toString();
    data.param2 = query.value("param2").toString();
    data.param3 = query.value("param3").toString();
    data.param4 = query.value("param4").toString();
    data.param5 = query.value("param5").toString();
    data.param6 = query.value("param6").toString();
    data.param7 = query.value("param7").toString();
    data.param8 = query.value("param8").toString();
    data.param9 = query.value("param9").toString();
    data.param10 = query.value("param10").toString();
    
    data.createdAt = query.value("created_at").toDateTime();
    data.updatedAt = query.value("updated_at").toDateTime();
    
    return data;
}