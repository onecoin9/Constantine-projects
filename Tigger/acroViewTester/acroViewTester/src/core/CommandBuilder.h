// CommandBuilder.h
#ifndef COMMANDBUILDER_H
#define COMMANDBUILDER_H

#include "ProtocolStructures.h"
#include <QVariantMap> // 便于传递参数

class CommandBuilder {
public:
    // --- 1.1. 协议头部设置 (通用头部，实际协议中似乎没有单独的命令来设置这个) ---
    // 这个 "协议头部设置" 更像是每个JSON命令内部的 "header" 对象。
    // 我们将它作为构建具体命令时的一个辅助函数。
    static QJsonObject createBaseHeader(const QString& type, int sequence, int version = 2, const QString& magic = "AA55") {
        QJsonObject headerObj;
        headerObj["magic"] = magic;
        headerObj["version"] = version;
        headerObj["sequence"] = sequence;
        headerObj["type"] = type; // "config", "data", "ack"
        return headerObj;
    }

    // --- 1.2. 设备控制段设置 (CMDID=0x01) ---
    // 发送: 温箱配置
    static QByteArray buildDeviceControlCommand(
        int sequence,
        const QString& thermalMode, // "hold", "ramp", "none"
        double targetTemp,          // for hold mode
        int stableTime,             // for hold mode
        double rampRate,            // for ramp mode
        const QVariantMap& powerControl = QVariantMap() // 可选的电源控制
    ) {
        QJsonObject cmdData;
        cmdData["header"] = createBaseHeader("config", sequence, 2, "0x01"); // magic 根据示例是CmdID

        QJsonObject controlObj;
        QJsonObject thermalObj;
        thermalObj["mode"] = thermalMode;

        QJsonObject paramsObj;
        if (thermalMode == "hold") {
            QJsonObject holdObj;
            holdObj["target_temp"] = targetTemp;
            holdObj["stable_time"] = stableTime;
            paramsObj["hold"] = holdObj;
        } else if (thermalMode == "ramp") {
            paramsObj["ramp"] = rampRate; // 协议中 ramp 是 Json Number，但示例中 ramp: null，hold有值
                                          // 假设 ramp 模式下直接是 rampRate
        }
        // 根据示例，即使是 hold 模式，ramp 键也存在且为 null
        if (!paramsObj.contains("ramp") && thermalMode != "ramp") {
             paramsObj["ramp"] = QJsonValue(); // null
        }
        if (!paramsObj.contains("hold") && thermalMode != "hold") {
            // paramsObj["hold"] = QJsonValue(); // 根据示例，如果不是hold，hold对象不一定需要
        }


        thermalObj["params"] = paramsObj;
        controlObj["thermal"] = thermalObj;

        if (!powerControl.isEmpty()) {
            controlObj["power"] = QJsonObject::fromVariantMap(powerControl);
        }

        cmdData["control"] = controlObj;
        cmdData["checksum"] = "xx"; // 协议示例中的校验和

        return ProtocolUtils::buildPacket(0x01, cmdData);
    }

    // 接收: 设备控制响应
    static bool parseDeviceControlResponse(const QJsonObject& jsonData, int& code, int& timestamp) {
        if (!jsonData.contains("header") || !jsonData.contains("status")) {
            qWarning() << "DeviceControlResponse: Missing header or status";
            return false;
        }
        // 可以进一步校验 header 的 type 是否为 "ack" 等

        QJsonObject statusObj = jsonData["status"].toObject();
        if (statusObj.contains("code") && statusObj.contains("timestamp")) {
            code = statusObj["code"].toInt(-1);
            timestamp = statusObj["timestamp"].toInt(-1);
            return true;
        }
        qWarning() << "DeviceControlResponse: Missing code or timestamp in status";
        return false;
    }


    // --- 1.3. 功能测试设置 (CMDID=0x02) ---
    // 发送: 功能测试配置
    static QByteArray buildFunctionTestCommand(
        int sequence,
        bool enabledTest,
        bool powerOn,
        double targetVoltage,
        int voltageStableTime,
        const QVariant& targetCurrent, // 使用 QVariant 以便可以是数字或 "xx"
        const QVariant& targetDriveFrequency,
        const QVariant& targetDriveForce,
        const QVariant& targetQuadrature,
        const QVariant& targetAmplitude,
        const QVariant& targetClock
    ) {
        QJsonObject cmdData;
        cmdData["header"] = createBaseHeader("config", sequence, 2, "0x02");

        QJsonObject testObj;
        testObj["enabled_test"] = enabledTest ? 1 : 0;
        testObj["on"] = powerOn ? 1 : 0;
        testObj["target_voltage"] = targetVoltage;
        testObj["voltage_stable_time"] = voltageStableTime;

        // 处理可能为 "xx" 的值
        auto setNumericOrString = [&](const QString& key, const QVariant& value) {
            if (value.canConvert<double>()) {
                testObj[key] = value.toDouble();
            } else {
                 testObj[key] = "xx"; 
            }
        };

        setNumericOrString("target_current", targetCurrent);
        setNumericOrString("target_drive_frequency", targetDriveFrequency);
        setNumericOrString("target_drive_force", targetDriveForce);
        setNumericOrString("target_quadrature", targetQuadrature);
        setNumericOrString("target_amplitude", targetAmplitude);
        setNumericOrString("target_clock", targetClock);

        cmdData["test"] = testObj;
        cmdData["checksum"] = "xx";

        return ProtocolUtils::buildPacket(0x02, cmdData);
    }

    // 接收: 功能测试响应 (与设备控制响应结构类似)
    static bool parseFunctionTestResponse(const QJsonObject& jsonData, int& code, int& timestamp) {
        return parseDeviceControlResponse(jsonData, code, timestamp); // 复用解析逻辑
    }


    // --- 1.4. 数据采集设置 (CMDID=0x03) ---
    // 发送: 数据采集配置
    static QByteArray buildDataAcquisitionConfigCommand(
        int sequence,
        bool enabled,
        int durationSec,
        int intervalMs
    ) {
        QJsonObject cmdData;
        cmdData["header"] = createBaseHeader("config", sequence, 2, "0x03");

        QJsonObject acqObj;
        acqObj["enabled"] = enabled;
        acqObj["duration"] = durationSec; // 协议中是 duration，示例是 duration
        acqObj["interval"] = intervalMs;

        cmdData["data_acquisition"] = acqObj;
        cmdData["checksum"] = "xx";

        return ProtocolUtils::buildPacket(0x03, cmdData);
    }

    // 接收: 数据采集响应 (服务端主动上报数据)
    // 这个响应比较特殊，header.type 是 "data"
    struct SensorData {
        double voltage = 0.0;
        double current = 0.0;
        double temperature = 0.0;
        double drive_freq = 0.0;
        QString drive_force; // "xx" or number
        QString quadrature;  // "xx" or number
        QString amplitude;   // "xx" or number
        QString clock;       // "xx" or number
        QString checksum;
    };
    static bool parseDataAcquisitionData(const QJsonObject& jsonData, SensorData& sensorData) {
        if (!jsonData.contains("header") || !jsonData.contains("sensor_data")) {
            qWarning() << "DataAcquisitionData: Missing header or sensor_data";
            return false;
        }
        QJsonObject headerObj = jsonData["header"].toObject();
        if(headerObj["type"].toString() != "data"){
            qWarning() << "DataAcquisitionData: header type is not 'data'";
            return false;
        }

        QJsonObject dataObj = jsonData["sensor_data"].toObject();
        sensorData.voltage = dataObj["voltage"].toDouble();
        sensorData.current = dataObj["current"].toDouble();
        sensorData.temperature = dataObj["temperature"].toDouble();
        sensorData.drive_freq = dataObj["drive_freq"].toDouble();
        sensorData.drive_force = dataObj["drive_force"].toVariant().toString(); // .toVariant().toString() 可以处理数字或字符串"xx"
        sensorData.quadrature = dataObj["quadrature"].toVariant().toString();
        sensorData.amplitude = dataObj["amplitude"].toVariant().toString();
        sensorData.clock = dataObj["clock"].toVariant().toString();
        sensorData.checksum = dataObj["checksum"].toString();

        return true;
    }
};

#endif // COMMANDBUILDER_H