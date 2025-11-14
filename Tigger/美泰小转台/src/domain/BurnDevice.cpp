#include "domain/BurnDevice.h"
#include "Ag06DoCustomProtocol.h"
#include "core/Logger.h"
#include "DutManager.h"
#include <QFileInfo>
#include <QProcess>
#include <QTimer>
#include <QThread>
#ifdef Q_OS_WIN
#include <Windows.h>
#endif

namespace Domain {

BurnDevice::BurnDevice(QObject *parent)
    : IDevice(parent)
    , m_serverHost("127.0.0.1")
    , m_serverPort(12345)
    , m_aprogPath("") // 默认路径
    , m_jsonRpcClient(nullptr)
    , m_atomicStatus(IDevice::DeviceStatus::Disconnected)
    , m_aprogProcess(nullptr)
    , m_isAprogRunning(false)
{
    m_name = "BurnDevice";
    m_type = DeviceType::Burn;

    // 创建JsonRpcClient实例
    m_jsonRpcClient = new JsonRpcClient(this);
    
    // 连接JsonRpcClient信号
    connect(m_jsonRpcClient, &JsonRpcClient::connectionStateChanged,
            this, &BurnDevice::onConnectionStateChanged);
    connect(m_jsonRpcClient, &JsonRpcClient::notificationReceived, this,
        [this](const QString& method, const QJsonObject& params) { handleNotification(method, params); });
    connect(m_jsonRpcClient, &JsonRpcClient::doJobCompleted, this, &BurnDevice::doJobCompleted);

    // 创建QProcess
    m_aprogProcess = new QProcess(this);
    connect(m_aprogProcess, &QProcess::started, this, &BurnDevice::onAprogStarted);
    connect(m_aprogProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &BurnDevice::onAprogFinished);
    connect(m_aprogProcess, &QProcess::errorOccurred, this, &BurnDevice::onAprogError);
    connect(m_aprogProcess, &QProcess::readyReadStandardOutput, this, &BurnDevice::onAprogReadyReadStandardOutput);
    connect(m_aprogProcess, &QProcess::readyReadStandardError, this, &BurnDevice::onAprogReadyReadStandardError);

    LOG_MODULE_INFO("BurnDevice", "BurnDevice created with JsonRpcClient integration");
}

BurnDevice::~BurnDevice()
{
    release();
    stopAprog();
    LOG_MODULE_INFO("BurnDevice", "BurnDevice destroyed");
}

bool BurnDevice::initialize()
{
    LOG_MODULE_INFO("BurnDevice", "Initializing BurnDevice...");

    if (m_atomicStatus.load() == IDevice::DeviceStatus::Ready) {
        LOG_MODULE_WARNING("BurnDevice", "Device already initialized");
        return true;
    }

    m_atomicStatus.store(IDevice::DeviceStatus::Initializing);
    emit statusChanged(IDevice::DeviceStatus::Initializing);

    // 设备初始化完成，但不自动设置为Ready状态
    // Ready状态将在JsonRPC连接建立后设置
    LOG_MODULE_INFO("BurnDevice", "BurnDevice initialization completed");

    return true;
}

bool BurnDevice::release()
{
    LOG_MODULE_INFO("BurnDevice", "Releasing BurnDevice...");

    if (m_jsonRpcClient) {
        m_jsonRpcClient->disconnectFromServer();
    }
    
    stopAprog();

    m_atomicStatus.store(IDevice::DeviceStatus::Disconnected);
    emit statusChanged(IDevice::DeviceStatus::Disconnected);

    LOG_MODULE_INFO("BurnDevice", "BurnDevice released");
    return true;
}

IDevice::DeviceStatus BurnDevice::getStatus() const
{
    return m_atomicStatus.load();
}

QString BurnDevice::getName() const
{
    return m_name;
}

IDevice::DeviceType BurnDevice::getType() const
{
    return m_type;
}

QString BurnDevice::getDescription() const
{
    return QString("JSON-RPC烧录设备 - %1:%2").arg(m_serverHost).arg(m_serverPort);
}

bool BurnDevice::isConnected() const
{
    return m_jsonRpcClient && 
           m_jsonRpcClient->getConnectionState() == JsonRpcClient::Connected;
}

QJsonObject BurnDevice::executeCommand(const QString &command, const QJsonObject &params)
{
    QMutexLocker cmdLocker(&m_cmdMutex);
    if (!isConnected()){
        QString error = QString("Burn device is not connected");
        LOG_MODULE_WARNING("BurnDevice", error.toStdString());
        return {{"success", false}, {"error", error}};
    }
    
    bool success = true;
    try {
        if (command.compare("SiteScanAndConnect", Qt::CaseInsensitive) == 0){
            m_jsonRpcClient->siteScanAndConnect("",
                [this](bool success, const QJsonObject& /*result*/, const QString& error) {
                    if (success) {
                        LOG_MODULE_INFO("BurnDevice", "SiteScanAndConnect successful");
                        // 处理结果...
                    }
                    else {
                        LOG_MODULE_ERROR("BurnDevice", QString("SiteScanAndConnect failed: %1").arg(error).toStdString());
                    }
                }
            );
        }
        else if (command.compare("LoadProject", Qt::CaseInsensitive) == 0) {
            // 优先从params中获取taskFileName，如果没有则使用配置中的值
            QString taskFileFullPath = params.value("taskFileName").toString();
            if (taskFileFullPath.isEmpty()) {
                // 从配置中获取taskFileName
                QMutexLocker locker(&m_configMutex);
                taskFileFullPath = m_config.value("taskFileName").toString();
                locker.unlock();
                
                if (taskFileFullPath.isEmpty()) {
                    LOG_MODULE_ERROR("BurnDevice", "LoadProject command requires 'taskFileName' parameter or configured taskFileName");
                    return {{"success", false}, {"error", "LoadProject command requires 'taskFileName' parameter or configured taskFileName"}};
                }
                
                LOG_MODULE_INFO("BurnDevice", QString("Using configured taskFileName: %1").arg(taskFileFullPath).toStdString());
            } else {
                LOG_MODULE_INFO("BurnDevice", QString("Using provided taskFileName: %1").arg(taskFileFullPath).toStdString());
            }
            
            // 从完整路径中提取项目目录和文件名
            QFileInfo fileInfo(taskFileFullPath);
            QString projectPath = params.value("projectPath").toString();
            if (projectPath.isEmpty()) {
                // 如果没有指定projectPath，使用taskFileName的父目录
                projectPath = fileInfo.absolutePath();
                LOG_MODULE_INFO("BurnDevice", QString("Using derived projectPath: %1").arg(projectPath).toStdString());
            } else {
                LOG_MODULE_INFO("BurnDevice", QString("Using provided projectPath: %1").arg(projectPath).toStdString());
            }
            
            // 提取纯文件名（不包含路径）
            QString taskFileName = fileInfo.fileName();
            
            LOG_MODULE_INFO("BurnDevice", QString("Loading project - Path: %1, TaskFile: %2").arg(projectPath).arg(taskFileName).toStdString());
            
            m_jsonRpcClient->loadProject(projectPath, taskFileName,
                [this](bool success, const QJsonObject& /*result*/, const QString& error) {
                    if (success) {
                        LOG_MODULE_INFO("BurnDevice", "LoadProject successful");
                        // 处理结果...
                    }
                    else {
                        LOG_MODULE_ERROR("BurnDevice", QString("LoadProject failed: %1").arg(error).toStdString());
                    }
                }
            );
        }
        else if (command.compare("DoCustom", Qt::CaseInsensitive) == 0) {
            Ag06DoCustomProtocol protocol;
            protocol.setClient(m_jsonRpcClient);
            if (!protocol.executeCommand(params)) {
                LOG_MODULE_ERROR("BurnDevice", QString("Execute command %1 failed.").arg(command).toStdString());
                return { {"success", false}, {"error", QString("Execute command %1 failed.").arg(command)} };
            }
        }
        else if (command.compare("DoJob", Qt::CaseInsensitive) == 0) {
            QString ip = params.value("strIp").toString();
            int sktEn = params.value("sktEn").toInt();
            QString cmdSeqName = params.value("command").toString();
            QJsonObject cmdSeq = m_cmdSequences[cmdSeqName];

            m_jsonRpcClient->doJob(ip, sktEn, cmdSeq,
                [this](bool success, const QJsonObject& /*result*/, const QString& error) {
                    if (success) {
                        LOG_MODULE_INFO("BurnDevice", "DoJob successful");
                        // 处理结果...
                    }
                    else {
                        LOG_MODULE_ERROR("BurnDevice", QString("DoJob failed: %1").arg(error).toStdString());
                    }
                });
        }
    } catch (const std::exception& e) {
        success = false;
        LOG_MODULE_ERROR("BurnDevice", QString("Exception in executeCommand: %1").arg(e.what()).toStdString());
        return {{"success", false}, {"error", QString("Exception: %1").arg(e.what())}};
    }
    LOG_MODULE_INFO("BurnDevice", QString("Executing command: %1").arg(command).toStdString());

    QJsonObject result;
    result["success"] = success;
    result["error"] = "";

    emit commandFinished(result);

    return result;
}

void BurnDevice::setCommunicationChannel(std::shared_ptr<Infrastructure::ICommunicationChannel> channel)
{
    m_commChannel = channel;
}

std::shared_ptr<Infrastructure::ICommunicationChannel> BurnDevice::getCommunicationChannel() const
{
    return m_commChannel;
}

void BurnDevice::setConfiguration(const QJsonObject &config)
{
    QMutexLocker locker(&m_configMutex);
    m_config = config;

    if (config.contains("serverHost")) {
        m_serverHost = config["serverHost"].toString();
    }
    if (config.contains("serverPort")) {
        m_serverPort = static_cast<quint16>(config["serverPort"].toInt());
    }
    if (config.contains("name")) {
        m_name = config["name"].toString();
    }
    if (config.contains("aprogPath")) {
        m_aprogPath = config["aprogPath"].toString();
        LOG_MODULE_INFO("BurnDevice", QString("Aprog path loaded from config: %1").arg(m_aprogPath).toStdString());

        // 验证文件是否存在
        QFileInfo fileInfo(m_aprogPath);
        if (!fileInfo.exists()) {
            LOG_MODULE_WARNING("BurnDevice", QString("Aprog.exe not found at configured path: %1").arg(m_aprogPath).toStdString());
        } else if (!fileInfo.isExecutable()) {
            LOG_MODULE_WARNING("BurnDevice", QString("Configured Aprog path is not executable: %1").arg(m_aprogPath).toStdString());
        } else {
            LOG_MODULE_INFO("BurnDevice", "Aprog.exe path validation successful");
        }
    } else {
        LOG_MODULE_WARNING("BurnDevice", "No aprogPath specified in configuration, manual selection will be required");
    }

    LOG_MODULE_INFO("BurnDevice", QString("Configuration set - Server: %1:%2, Aprog: %3")
                   .arg(m_serverHost).arg(m_serverPort).arg(m_aprogPath).toStdString());
}

QJsonObject BurnDevice::getConfiguration() const
{
    QMutexLocker locker(&m_configMutex);
    return m_config;
}

bool BurnDevice::selfTest()
{
    if (!isConnected()) {
        LOG_MODULE_WARNING("BurnDevice", "Self test failed - not connected");
        return false;
    }
    
    // 使用JsonRpcClient进行自检
    if (m_jsonRpcClient) {
        m_jsonRpcClient->getProjectInfo();
        LOG_MODULE_INFO("BurnDevice", "Self test request sent");
        return true;
    }
    
    return false;
}

QString BurnDevice::getServerHost() const
{
    QMutexLocker locker(&m_configMutex);
    return m_serverHost;
}

quint16 BurnDevice::getServerPort() const
{
    QMutexLocker locker(&m_configMutex);
    return m_serverPort;
}

void BurnDevice::setServerInfo(const QString &host, quint16 port)
{
    {
        QMutexLocker locker(&m_configMutex);
        m_serverHost = host;
        m_serverPort = port;
    }

    LOG_MODULE_INFO("BurnDevice", QString("Server info updated: %1:%2").arg(host).arg(port).toStdString());

    // 如果当前已连接，重新连接到新的服务器
    if (isConnected() && m_jsonRpcClient) {
        m_jsonRpcClient->disconnectFromServer();
        m_jsonRpcClient->connectToServer(m_serverHost, m_serverPort, true);
    }
}

// Aprog.exe程序管理方法
QString BurnDevice::getAprogPath() const
{
    QMutexLocker locker(&m_configMutex);
    return m_aprogPath;
}

void BurnDevice::setAprogPath(const QString &path)
{
    {
        QMutexLocker locker(&m_configMutex);
        m_aprogPath = path;
        // 同时更新配置对象
        m_config["aprogPath"] = path;
    }

    LOG_MODULE_INFO("BurnDevice", QString("Aprog path updated: %1").arg(path).toStdString());
}

bool BurnDevice::isAprogRunning() const
{
    return m_isAprogRunning && m_aprogProcess &&
           (m_aprogProcess->state() == QProcess::Running);
}

void BurnDevice::startAprog()
{
    if (isAprogRunning()) {
        LOG_MODULE_INFO("BurnDevice", "Aprog.exe is already running, stopping it first...");
        stopAprog();
        
        // 等待进程完全停止
        QThread::msleep(100);  // 给进程一点时间完全退出
    }

    QString aprogPath;
    {
        QMutexLocker locker(&m_configMutex);
        aprogPath = m_aprogPath;
    }

    if (aprogPath.isEmpty()) {
        LOG_MODULE_ERROR("BurnDevice", "Aprog path is empty");
        QTimer::singleShot(0, this, [this]() {
            emit aprogError(QProcess::FailedToStart);
        });
        return;
    }

    QFileInfo fileInfo(aprogPath);
    if (!fileInfo.exists()) {
        LOG_MODULE_ERROR("BurnDevice", QString("Aprog.exe not found at: %1").arg(aprogPath).toStdString());
        QTimer::singleShot(0, this, [this]() {
            emit aprogError(QProcess::FailedToStart);
        });
        return;
    }

    LOG_MODULE_INFO("BurnDevice", QString("Starting Aprog.exe: %1 -r").arg(aprogPath).toStdString());

    // 设置工作目录为Aprog.exe所在目录
    m_aprogProcess->setWorkingDirectory(fileInfo.absolutePath());

#ifdef Q_OS_WIN
    // Windows-specific: Set process creation flags to prevent window from appearing on top
    // This will create the process in the background without showing its window
    m_aprogProcess->setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments *args) {
        args->flags |= CREATE_NO_WINDOW;
        args->startupInfo->dwFlags |= STARTF_USESHOWWINDOW;
        args->startupInfo->wShowWindow = SW_HIDE;
    });
#endif

    // 启动程序，使用-r参数
    QStringList arguments;
    arguments << "-r";

    // 指定端口号
    arguments << "-p" << QString::number(m_serverPort);

    // 异步启动，避免阻塞
    m_aprogProcess->start(aprogPath, arguments);
    
    // 异步等待3秒后尝试连接到服务器
    QTimer::singleShot(3000, this, [this]() {
        if (m_jsonRpcClient) {
            m_jsonRpcClient->connectToServer(m_serverHost, m_serverPort, true);
            LOG_MODULE_INFO("BurnDevice", "尝试连接到JsonRpc服务器");
        }
    });
}

void BurnDevice::stopAprog()
{
    if (!m_aprogProcess || m_aprogProcess->state() == QProcess::NotRunning) {
        m_isAprogRunning = false;
        return;
    }

    LOG_MODULE_INFO("BurnDevice", "Stopping Aprog.exe...");

    // 先尝试正常终止
    m_aprogProcess->terminate();

    // 等待5秒让程序正常退出
    if (!m_aprogProcess->waitForFinished(5000)) {
        LOG_MODULE_WARNING("BurnDevice", "Aprog.exe did not terminate gracefully, killing...");
        m_aprogProcess->kill();
        m_aprogProcess->waitForFinished(2000);
    }

    m_isAprogRunning = false;
    LOG_MODULE_INFO("BurnDevice", "Aprog.exe stopped");
}

// JsonRpcClient连接状态变化槽函数
void BurnDevice::onConnectionStateChanged(JsonRpcClient::ConnectionState state)
{
    IDevice::DeviceStatus newStatus = IDevice::DeviceStatus::Disconnected;
    
    switch (state) {
        case JsonRpcClient::Connected:
            newStatus = IDevice::DeviceStatus::Ready;
            break;
        case JsonRpcClient::Connecting:
        case JsonRpcClient::Reconnecting:
            newStatus = IDevice::DeviceStatus::Initializing;
            break;
        case JsonRpcClient::Disconnected:
            newStatus = IDevice::DeviceStatus::Disconnected;
            break;
    }
    
    m_atomicStatus.store(newStatus);
    emit statusChanged(newStatus);
}

// Aprog.exe进程事件处理槽函数
void BurnDevice::onAprogStarted()
{
    m_isAprogRunning = true;
    LOG_MODULE_INFO("BurnDevice", "Aprog.exe started successfully");
    QTimer::singleShot(0, this, [this]() {
        emit aprogStarted();
    });
}

void BurnDevice::onAprogFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_isAprogRunning = false;

    QString statusStr = (exitStatus == QProcess::NormalExit) ? "normally" : "crashed";
    LOG_MODULE_INFO("BurnDevice", QString("Aprog.exe finished %1 with exit code %2").arg(statusStr).arg(exitCode).toStdString());

    QTimer::singleShot(0, this, [this, exitCode, exitStatus]() {
        emit aprogFinished(exitCode, exitStatus);
    });

    // 如果程序异常退出，断开连接
    if (exitStatus == QProcess::CrashExit && m_jsonRpcClient) {
        LOG_MODULE_WARNING("BurnDevice", "Aprog.exe crashed, disconnecting from server");
        m_jsonRpcClient->disconnectFromServer();
        m_atomicStatus.store(IDevice::DeviceStatus::Error);
        QTimer::singleShot(0, this, [this]() {
            emit statusChanged(IDevice::DeviceStatus::Error);
        });
    }
}

void BurnDevice::onAprogError(QProcess::ProcessError error)
{
    m_isAprogRunning = false;

    QString errorStr;
    switch (error) {
        case QProcess::FailedToStart:
            errorStr = "Failed to start";
            break;
        case QProcess::Crashed:
            errorStr = "Crashed";
            break;
        case QProcess::Timedout:
            errorStr = "Timed out";
            break;
        case QProcess::WriteError:
            errorStr = "Write error";
            break;
        case QProcess::ReadError:
            errorStr = "Read error";
            break;
        case QProcess::UnknownError:
        default:
            errorStr = "Unknown error";
            break;
    }

    LOG_MODULE_ERROR("BurnDevice", QString("Aprog.exe error: %1").arg(errorStr).toStdString());
    QTimer::singleShot(0, this, [this, error]() {
        emit aprogError(error);
    });

    // 程序启动失败或崩溃时，更新设备状态
    if (error == QProcess::FailedToStart || error == QProcess::Crashed) {
        m_atomicStatus.store(IDevice::DeviceStatus::Error);
        QTimer::singleShot(0, this, [this]() {
            emit statusChanged(IDevice::DeviceStatus::Error);
        });
    }
}

void BurnDevice::onAprogReadyReadStandardOutput()
{
    if (!m_aprogProcess) return;

    QByteArray data = m_aprogProcess->readAllStandardOutput();
    QString output = QString::fromLocal8Bit(data).trimmed();

    if (!output.isEmpty()) {
        LOG_MODULE_DEBUG("BurnDevice", QString("Aprog.exe stdout: %1").arg(output).toStdString());
        QTimer::singleShot(0, this, [this, output]() {
            emit aprogOutputReceived(output);
        });
    }
}

void BurnDevice::onAprogReadyReadStandardError()
{
    if (!m_aprogProcess) return;

    QByteArray data = m_aprogProcess->readAllStandardError();
    QString output = QString::fromLocal8Bit(data).trimmed();

    if (!output.isEmpty()) {
        LOG_MODULE_WARNING("BurnDevice", QString("Aprog.exe stderr: %1").arg(output).toStdString());
        QTimer::singleShot(0, this, [this, output]() {
            emit aprogOutputReceived(QString("[ERROR] %1").arg(output));
        });
    }
}

void BurnDevice::extractCommandSequences(const QJsonObject& jsonObject, const QString& parentKey)
{
    for (auto it = jsonObject.begin(); it != jsonObject.end(); ++it) {
        QString key = parentKey.isEmpty() ? it.key() : parentKey + "." + it.key();

        if (it.value().isObject()) {
            QJsonObject obj = it.value().toObject();

            // 检查是否是命令序列
            if (obj.contains("commands") || obj.contains("sequence")) {
                m_cmdSequences[key] = obj;
            }
            else {
                // 递归搜索
                extractCommandSequences(obj, key);
            }
        }
    }
}

} // namespace Domain

// 新增：处理服务端通知，转发到更上层（DutManager）
namespace Domain {
void BurnDevice::handleNotification(const QString &method, const QJsonObject &params)
{
    // 兜底：参数已是 QJsonObject；若为空则继续走兼容解析（不强制返回）
    // setSiteResult → 发射 deviceDiscovered（保持DutManager::updateSiteFromScan 所需格式）
    if (method == QStringLiteral("setSiteResult")) {
        // 兼容多种字段命名，尽力抽取需要的键
        QJsonObject siteInfo;
        QJsonObject root = params;
        if (root.contains("result") && root.value("result").isObject()) {
            root = root.value("result").toObject();
        }
        if (root.contains("data") && root.value("data").isObject()) {
            root = root.value("data").toObject();
        }
        // 多个可能的键名映射
        auto str = [&](const char* a, const char* b = nullptr){
            if (root.contains(a)) return root.value(a).toString();
            if (b && root.contains(b)) return root.value(b).toString();
            return QString();
        };
        siteInfo["siteAlias"] = str("SiteAlias", "siteAlias");
        siteInfo["mac"] = str("MacAddr", "mac");
        siteInfo["port"] = str("UDPPort", "port");
        siteInfo["dpsFpgaVersion"] = str("DPSFPGAVersion", "dpsFpgaVersion");
        siteInfo["dpsFwVersion"] = str("DPSFwVersion", "dpsFwVersion");
        siteInfo["fpgaLocation"] = str("FPGALocation", "fpgaLocation");
        siteInfo["fpgaVersion"] = str("FPGAVersion", "fpgaVersion");
        siteInfo["firmwareVersion"] = str("FirmwareVersion", "firmwareVersion");
        siteInfo["firmwareVersionDate"] = str("FirmwareVersionDate", "firmwareVersionDate");
        siteInfo["muAppVersion"] = str("MUAPPVersion", "muAppVersion");
        siteInfo["muAppVersionDate"] = str("MUAPPVersionDate", "muAppVersionDate");
        siteInfo["muLocation"] = str("MULocation", "muLocation");
        if (root.contains("MainBoardInfo") && root.value("MainBoardInfo").isObject())
            siteInfo["mainBoardInfo"] = root.value("MainBoardInfo").toObject();
        siteInfo["ip"] = str("ip", "strip");
        emit deviceDiscovered(siteInfo); // 即使部分字段为空，也不会崩溃
        
        return;
    }
    // setSKTEnResult → 发射 getSKTEnable（字符串聚合）
    if (method == QStringLiteral("setSKTEnResult")) {
        QJsonObject data = params.value("data").toObject();
        QJsonArray results = data.value("results").toArray();
        if (!results.isEmpty()) {
            QStringList lines;
            for (const auto &v : results) { lines << v.toString(); }
            emit getSKTEnable(lines.join(';'));
            emit statusChanged(IDevice::DeviceStatus::Loaded);
        }
        return;
    }
    if (method == "setLoadProjectResult") {
        QJsonObject data = params.value("data").toObject();
        QJsonObject proInfo = params.value("proInfo").toObject();

        QJsonArray seqArray = proInfo.value("doCmdSequenceArray").toArray();
        if (!seqArray.isEmpty()) {
            m_cmdSequences.clear();

            for (const auto& v : seqArray) {
                QJsonObject entry = v.toObject();
                QString group = entry.value("CmdRun").toString();
                QJsonArray seqs = entry.value("CmdSequences").toArray();
                if (seqs.isEmpty()) continue;

                QJsonObject groupObj;
                groupObj["CmdRun"] = group;
                groupObj["CmdSequences"] = seqs;
                groupObj["CmdSequencesGroupCnt"] = seqs.size();
                groupObj["CmdID"] = entry.value("CmdID").toString();
                QString key = group.isEmpty() ? QString::number(m_cmdSequences.size()) : group;
                m_cmdSequences[key] = groupObj;
            }
        }

    }
    if (method == "setProjectInfo" || method == "GetProjectInfo" || method == "GetProjectInfoExt") {
        QJsonObject root = params;
        if (root.contains("result") && root.value("result").isObject()) {
            root = root.value("result").toObject();
        }
        QJsonObject proInfo = root.value("proInfo").toObject();
        QJsonObject dataObj = root.value("data").toObject();

        if (!proInfo.isEmpty()) {
            // 清空旧的命令序列
            m_cmdSequences.clear();

            // 直接解析 doCmdSequenceArray
            QJsonArray arr = proInfo.value("doCmdSequenceArray").toArray();
            for (const auto& v : arr) {
                QJsonObject entry = v.toObject();
                QString cmdRun = entry.value("CmdRun").toString();
                QJsonArray seqs = entry.value("CmdSequences").toArray();
                if (!seqs.isEmpty()) {
                    // 用 CmdRun 作为分组名
                    QJsonObject seqGroup;
                    seqGroup["CmdRun"] = cmdRun;
                    seqGroup["CmdSequences"] = seqs;
                    seqGroup["CmdSequencesGroupCnt"] = seqs.size();
                    seqGroup["CmdID"] = entry.value("CmdID").toString();
                    QString key = cmdRun.isEmpty() ? QString::number(m_cmdSequences.size()) : cmdRun;
                    m_cmdSequences[key] = seqGroup;
                }
            }
            // 回退：递归方式挖掘
            if (m_cmdSequences.isEmpty()) {
                extractCommandSequences(proInfo);
            }
        }
    }


    // setDoJobResult → 提取 strip/SKTIdx/result，并发射 doJobResult
    if (method == QStringLiteral("setDoJobResult")) {
        QJsonObject obj = params;
        const QString ip = obj.value("strIp").toString();
        int nHopNum = obj.value("nHopNum").toInt(-1);
        const QString resStr = obj.value("result").toString();
        QJsonArray dutResult = obj.value("dutResult").toArray();
        QByteArray resArray;
        for (const auto& value : dutResult) {
            resArray.append(static_cast<char>(value.toInt()));
        }
        Services::DutManager::instance()->updateSiteChipStatus(ip, resArray);

        LOG_MODULE_INFO("BurnDevice", QString("ip:%1 setDoJobResult - updateSiteChipStatus:").arg(ip).append(resArray.toHex()).toStdString());

        bool success = (resStr.compare("Passed", Qt::CaseInsensitive) == 0 || resStr == "OK" || resStr == "Success");
        Q_UNUSED(success); // Variable used for potential future logic
        if (!ip.isEmpty()) {
            emit doJobResult(ip, nHopNum, obj);
            emit doJobCompleted(obj);
        }
        return;
    }
    else if (method == QStringLiteral("ScanComplete")) {

        // 等待2秒后再发射Scanned状态信号
        QTimer::singleShot(2000, this, [this]() {
            emit statusChanged(IDevice::DeviceStatus::Scanned);
            });
    }
    // setDoJobResult → 可在此解析并上抛，当前暂由更高层监听 serverCommandReceived 后自行处理
}
}
