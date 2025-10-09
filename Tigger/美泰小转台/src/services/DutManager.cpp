#include "services/DutManager.h"
#include "domain/Dut.h"
//#include <QDebug>
#include <QVariant>
#include <QMetaType>
#include <QMutexLocker>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "core/Logger.h"

namespace Services {
// 初始化静态成员
QMutex DutManager::s_mutex;
QAtomicPointer<DutManager> DutManager::s_instance = nullptr;

// 单例获取方法实现
DutManager* DutManager::instance()
{
    // 第一次检查 - 不加锁
    if (s_instance.loadAcquire() == nullptr) {
        // 加锁
        QMutexLocker locker(&s_mutex);
        // 第二次检查 - 加锁后再次检查
        if (s_instance.loadAcquire() == nullptr) {
            // 创建实例
            DutManager* newInstance = new DutManager(nullptr);
            // 使用原子操作设置实例指针
            s_instance.storeRelease(newInstance);
        }
    }
    return s_instance.loadAcquire();
}

DutManager::DutManager(QObject *parent)
    : QObject(parent)
{
    LOG_MODULE_DEBUG("DutManager", "DutManager constructed.");
    qRegisterMetaType<Domain::Dut::State>("Domain::Dut::State");
}

DutManager::~DutManager()
{
}

Domain::Dut* DutManager::registerDut(const QString &dutId)
{
    return registerDutInternal(dutId, true, true);
}

Domain::Dut* DutManager::registerDutInternal(const QString &dutId, bool emitSignal, bool connectSignals)
{
    if (dutId.isEmpty()) {
        return nullptr;
    }
    if (m_duts.contains(dutId)) {
        auto dut = m_duts.value(dutId);
        if (connectSignals) {
            connect(dut, &Domain::Dut::stateChanged, this, &DutManager::onDutStateChanged);
            connect(dut, &Domain::Dut::siteChanged, this, &DutManager::onDutSiteChanged);
        }

        if (emitSignal) {
            emit dutRegistered(dutId);
        }
        return dut;
    }
    auto dut = new Domain::Dut(dutId);
    m_duts.insert(dutId, dut);  
    return dut;
}

bool DutManager::unregisterDut(const QString &dutId)
{
    {
        QMutexLocker locker(&m_mutex);
        if (!m_duts.contains(dutId)) {
            return false;
        }
        m_duts.remove(dutId);
    }
    
    emit dutUnregistered(dutId);
    return true;
}

Domain::Dut* DutManager::getDut(const QString &dutId) const
{
    QMutexLocker locker(&m_mutex);
    return m_duts.value(dutId, nullptr);
}

QStringList DutManager::getAllDutIds() const
{
    QMutexLocker locker(&m_mutex);
    return m_duts.keys();
}


void DutManager::resetAllDuts()
{
    QList<Domain::Dut*> duts;
    {
        QMutexLocker locker(&m_mutex);
        duts = m_duts.values();
    }
    for (auto* dut : duts) {
        if (dut) {
            dut->setState(Domain::Dut::State::Idle);
        }
    }
}

bool DutManager::setState(const QString &dutId, Domain::Dut::State newState)
{

    Domain::Dut* dut = nullptr;
    {
        QMutexLocker locker(&m_mutex);
        dut = m_duts.value(dutId);
        if (!dut) return false;
        // 在锁的作用域内获取DUT指针，然后释放锁
    } // 锁在这里自动释放
    
    // 锁释放后再调用setState，避免在持有锁的情况下触发信号
    dut->setState(newState); // Dut 会自己发 stateChanged 信号，onDutStateChanged 已连接
    return true;
}

bool DutManager::setTestData(const QString &dutId, const QVariantMap &data)
{
    Domain::Dut* dut = nullptr;
    {
        QMutexLocker locker(&m_mutex);
        dut = m_duts.value(dutId);
    }
    if (!dut) return false;
    qDebug()<<"66666setTestData"<<data;
    dut->setTestData(data);
    return true;
}

bool DutManager::setSite(const QString &dutId, const QString &site)
{
    Domain::Dut* dut = nullptr;
    {
        QMutexLocker locker(&m_mutex);
        dut = m_duts.value(dutId);
    }
    if (!dut) return false;
    dut->setCurrentSite(site);
    return true;
}

void DutManager::onDutStateChanged(const QString& dutId, Domain::Dut::State state)
{
    emit dutStateChanged(dutId, state);
}

void DutManager::onDutSiteChanged(const QString &dutId, const QString &newSite)
{
    emit dutSiteChanged(dutId, newSite);
}

bool DutManager::loadSiteConfiguration(const QString& siteConfigFilePath)
{
    QMutexLocker locker(&m_mutex);
    
    // 清空现有站点信息
    m_siteInfoMap.clear();
    
    // 收集新创建的DUT，稍后批量发射信号
    QStringList newDutIds;
    
    QFile file(siteConfigFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOG_ERROR(QString("Cannot open site config file: %1").arg(siteConfigFilePath).toStdString());
        emit siteConfigurationFailed(QString("无法打开站点配置文件: %1").arg(siteConfigFilePath));
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        LOG_ERROR(QString("Site config JSON parse error: %1").arg(error.errorString()).toStdString());
        emit siteConfigurationFailed(QString("站点配置文件JSON解析错误: %1").arg(error.errorString()));
        return false;
    }  
    QJsonObject siteConfiguration = doc.object();
    
    // 添加详细的调试信息
    LOG_MODULE_INFO("DutManager", QString("JSON object keys: %1").arg(siteConfiguration.keys().join(", ")).toStdString());
    LOG_MODULE_INFO("DutManager", QString("Contains 'sites' key: %1").arg(siteConfiguration.contains("sites") ? "true" : "false").toStdString());
    
    if (!siteConfiguration.contains("sites")) {
        LOG_ERROR("No sites section found in site config file");
        emit siteConfigurationFailed("站点配置文件中没有找到 sites 节");
        return false;
    }
    
    LOG_MODULE_INFO("DutManager", "Successfully found 'sites' section in configuration");
    QJsonArray sites = siteConfiguration["sites"].toArray();
    
    // 解析每个站点
    struct DutInit { Domain::Dut* dut; QString siteAlias; QVariantMap dutData; };
    QList<DutInit> dutInits;
    for (const QJsonValue& siteValue : sites) {
        QJsonObject siteObj = siteValue.toObject();
        
        SiteInfo siteInfo;
        siteInfo.siteIndex = siteObj["siteIndex"].toInt();
        siteInfo.siteAlias = siteObj["siteAlias"].toString();
        
        siteInfo.siteSN = siteObj["siteSN"].toString();
        siteInfo.autoAlias = siteObj["autoAlias"].toString();
        siteInfo.socketCount = siteObj["socketCount"].toInt();
        siteInfo.siteEnvInit = siteObj["siteEnvInit"].toBool();
        siteInfo.chipReady = siteObj["chipReady"].toBool();
        siteInfo.description = siteObj["description"].toString();
        siteInfo.ip = siteObj["ip"].toString();
        
        // 解析十六进制字符串
        QString socketEnableStr = siteObj["socketEnable"].toString();
        QString socketReadyStr = siteObj["socketReady"].toString();
        QString chipEnableStr = siteObj["chipEnable"].toString();
        
        bool ok;
        siteInfo.socketEnable = socketEnableStr.toULongLong(&ok, 16);
        if (!ok) siteInfo.socketEnable = 0;
        
        siteInfo.socketReady = socketReadyStr.toULongLong(&ok, 16);
        if (!ok) siteInfo.socketReady = 0;
        
        siteInfo.chipEnable = chipEnableStr.toULongLong(&ok, 16);
        if (!ok) siteInfo.chipEnable = 0;
        
        // 存储站点信息
        m_siteInfoMap[siteInfo.siteAlias] = siteInfo;
        
        // 为站点创建对应的DUT
        for(int i = 1 ; i <= siteInfo.socketCount ; i++){
            QString dutId = QString("%1_Dut_%2").arg(siteInfo.siteAlias).arg(i);
            
            auto dut = registerDutInternal(dutId, false, false);  // 不立即发射信号，不连接信号
            if (dut) {
                newDutIds.append(dutId);  // 收集新DUT ID
                // 准备在解锁后设置站点与测试数据，避免持锁调用对象方法
                QVariantMap dutData;
                dutData["siteIndex"] = siteInfo.siteIndex;
                dutData["siteAlias"] = siteInfo.siteAlias;
                dutData["siteSN"] = siteInfo.siteSN;
                dutData["socketCount"] = siteInfo.socketCount;
                dutData["socketEnable"] = QString("0x%1").arg(siteInfo.socketEnable, 0, 16);
                dutData["description"] = siteInfo.description;
                dutInits.append(DutInit{dut, siteInfo.siteAlias, dutData});
            }
        }
    }
    // 统计启用的站点数量
    int enabledSiteCount = 0;
    for (const auto& siteInfo : m_siteInfoMap) {
        if (siteInfo.siteEnvInit) {
            enabledSiteCount++;
        }
    }
    // 解锁后批量设置DUT信息并发射信号，避免死锁
    locker.unlock();
    for (const auto &init : dutInits) {
        if (init.dut) {
            init.dut->setCurrentSite(init.siteAlias);
            init.dut->setTestData(init.dutData);
        }
    }
    emit siteConfigurationLoaded(m_siteInfoMap.size(), enabledSiteCount);
    
    return true;
}
bool DutManager::updateSktFromDownLoadProject(const QString& data)
{//data为qstring的格式,"BPUEn:63 recvIP:192.168.10.103 nHop:0;BPUEn:65280 recvIP:192.168.10.102 nHop:0;BPUEn:65343 recvIP:192.168.10.108 nHop:0"

    if (data.isEmpty()) {
        LOG_MODULE_WARNING("DutManager", "updateSktFromDownLoadProject: data is empty");
        return false;
    }
    
    // 按分号分割每个站点的数据
    QStringList siteDataList = data.split(';', Qt::SkipEmptyParts);
    
    for (const QString& siteDataStr : siteDataList) {
        // 解析每个站点的数据: "BPUEn:63 recvIP:192.168.10.103 nHop:0"
        QStringList parts = siteDataStr.split(' ', Qt::SkipEmptyParts);
        
        QString bpuEnStr;
        QString recvIP;
        
        // 提取BPUEn和recvIP
        for (const QString& part : parts) {
            if (part.startsWith("BPUEn:")) {
                bpuEnStr = part.mid(6); // 去掉"BPUEn:"前缀
            } else if (part.startsWith("recvIP:")) {
                recvIP = part.mid(7); // 去掉"recvIP:"前缀
            }
        }
        
        if (bpuEnStr.isEmpty() || recvIP.isEmpty()) {
            LOG_MODULE_WARNING("DutManager", QString("Invalid site data format: %1").arg(siteDataStr).toStdString());
            continue;
        }
        
        // 将BPUEn值从十进制转换为整数
        bool ok;
        quint64 bpuEnValue = bpuEnStr.toULongLong(&ok);
        if (!ok) {
            LOG_MODULE_WARNING("DutManager", QString("Invalid BPUEn value: %1").arg(bpuEnStr).toStdString());
            continue;
        }
        
        // 根据IP地址找到对应的站点
        QString targetSiteAlias;
        for (auto it = m_siteInfoMap.begin(); it != m_siteInfoMap.end(); ++it) {
            if (it.value().ip == recvIP) {
                targetSiteAlias = it.key();
                break;
            }
        }
        
        if (targetSiteAlias.isEmpty()) {
            LOG_MODULE_WARNING("DutManager", QString("Site with IP %1 not found").arg(recvIP).toStdString());
            continue;
        }
        
        SiteInfo& siteInfo = m_siteInfoMap[targetSiteAlias];
        
        // 更新站点的socket使能状态
        siteInfo.socketEnable = bpuEnValue;
        
        LOG_MODULE_INFO("DutManager", QString("Updated socket enable for site %1 (IP: %2): 0x%3")
                        .arg(targetSiteAlias).arg(recvIP).arg(bpuEnValue, 0, 16).toStdString());
        
        // 更新对应DUT的状态为SKTEnable
        for (int socketIndex = 1; socketIndex <= siteInfo.socketCount; ++socketIndex) {
            // 检查该socket是否被使能 (二进制位检查，从右边第0位开始)
            bool socketEnabled = (bpuEnValue & (1ULL << (socketIndex - 1))) != 0;
            
            if (socketEnabled) {
                QString dutId = QString("%1_Dut_%2").arg(targetSiteAlias).arg(socketIndex);
                auto dut = registerDut(dutId);
                if (dut) {
                    dut->setState(Domain::Dut::State::SKTEnable);
                    LOG_MODULE_DEBUG("DutManager", QString("Set DUT %1 state to SKTEnable").arg(dutId).toStdString());
                }
            }
        }
    }
    return true;
}
bool DutManager::updateSiteFromScan(const QJsonObject &siteData)
{
    QMutexLocker locker(&m_mutex);

    QString siteAlias = siteData.value("siteAlias").toString();
    if (siteAlias.isEmpty()) {
        LOG_MODULE_WARNING("DutManager", "Site data from scan is missing 'siteAlias'");
        return false;
    }

    if (!m_siteInfoMap.contains(siteAlias)) {
        LOG_MODULE_WARNING("DutManager", QString("Received scan data for an unknown site: %1").arg(siteAlias).toStdString());
        return false;
    }

    LOG_MODULE_INFO("DutManager", QString("Updating site '%1' from scan data...").arg(siteAlias).toStdString());

    SiteInfo& siteInfo = m_siteInfoMap[siteAlias];

    // 更新扫描状态和时间
    siteInfo.isScanned = true;
    siteInfo.scanTime = QDateTime::currentDateTime();

    // 更新所有从JSON中获取的字段
    if (siteData.contains("chainID")) siteInfo.chainID = siteData["chainID"].toInt();
    if (siteData.contains("dpsFpgaVersion")) siteInfo.dpsFpgaVersion = siteData["dpsFpgaVersion"].toString();
    if (siteData.contains("dpsFwVersion")) siteInfo.dpsFwVersion = siteData["dpsFwVersion"].toString();
    if (siteData.contains("firmwareVersion")) siteInfo.firmwareVersion = siteData["firmwareVersion"].toString();
    if (siteData.contains("firmwareVersionDate")) siteInfo.firmwareVersionDate = siteData["firmwareVersionDate"].toString();
    if (siteData.contains("fpgaLocation")) siteInfo.fpgaLocation = siteData["fpgaLocation"].toString();
    if (siteData.contains("fpgaVersion")) siteInfo.fpgaVersion = siteData["fpgaVersion"].toString();
    if (siteData.contains("hopNum")) siteInfo.hopNum = siteData["hopNum"].toInt();
    if (siteData.contains("ip")) siteInfo.ip = siteData["ip"].toString();
    if (siteData.contains("isLastHop")) siteInfo.isLastHop = siteData["isLastHop"].toBool();
    if (siteData.contains("linkNum")) siteInfo.linkNum = siteData["linkNum"].toInt();
    if (siteData.contains("mac")) siteInfo.mac = siteData["mac"].toString();
    if (siteData.contains("muAppVersion")) siteInfo.muAppVersion = siteData["muAppVersion"].toString();
    if (siteData.contains("muAppVersionDate")) siteInfo.muAppVersionDate = siteData["muAppVersionDate"].toString();
    if (siteData.contains("muLocation")) siteInfo.muLocation = siteData["muLocation"].toString();
    if (siteData.contains("port")) siteInfo.port = siteData["port"].toString();
    if (siteData.contains("mainBoardInfo") && siteData["mainBoardInfo"].isObject()) {
        siteInfo.mainBoardInfo = siteData["mainBoardInfo"].toObject();
    }
    
    // 释放锁后发射信号
    locker.unlock();
    emit siteUpdated(siteAlias,siteInfo.ip);

    return true;
}

QMap<QString, DutManager::SiteInfo> DutManager::getAllSiteInfo() const
{
    QMutexLocker locker(&m_mutex);
    return m_siteInfoMap;
}

DutManager::SiteInfo DutManager::getSiteInfo(const QString& siteAlias) const
{
    QMutexLocker locker(&m_mutex);
    return m_siteInfoMap.value(siteAlias, SiteInfo());
}

DutManager::SiteInfo DutManager::getSiteInfoByIndex(int siteIndex) const
{
    QMutexLocker locker(&m_mutex);
    for (const auto& siteInfo : m_siteInfoMap) {
        if (siteInfo.siteIndex == siteIndex) {
            return siteInfo;
        }
    }
    return SiteInfo();
}


bool DutManager::updateSiteChipStatusByIndex(int siteId, const QByteArray& chipStatus)
{
    QMutexLocker locker(&m_mutex);

    // 查找对应的站点
    QString targetSiteAlias;
    for (auto it = m_siteInfoMap.begin(); it != m_siteInfoMap.end(); ++it) {
        if (it.value().siteIndex == siteId) {
            targetSiteAlias = it.key();
            break;
        }
    }

    if (targetSiteAlias.isEmpty()) {
        LOG_MODULE_WARNING("DutManager", QString("Site with index %1 not found").arg(siteId).toStdString());
        return false;
    }

    SiteInfo& siteInfo = m_siteInfoMap[targetSiteAlias];
    siteInfo.currentChipStatus = chipStatus;
    locker.unlock();
    return true;
}

bool DutManager::updateSiteChipStatus(const QString& ip, const QByteArray& chipStatus)
{
    QMutexLocker locker(&m_mutex);

    // 查找对应的站点
    QString targetSiteAlias;
    for (auto it = m_siteInfoMap.begin(); it != m_siteInfoMap.end(); ++it) {
        if (it.value().ip == ip) {
            targetSiteAlias = it.key();
            break;
        }
    }

    if (targetSiteAlias.isEmpty()) {
        LOG_MODULE_WARNING("DutManager", QString("Site with index %1 not found").arg(ip).toStdString());
        return false;
    }

    SiteInfo& siteInfo = m_siteInfoMap[targetSiteAlias];
    siteInfo.currentChipStatus = chipStatus;
    locker.unlock();
    return true;
}

bool DutManager::updateSiteChipPlacement(int siteIndex, quint64 chipMask, const QString& siteSn)
{
    QMutexLocker locker(&m_mutex);
    
    // 查找对应的站点
    QString targetSiteAlias;
    for (auto it = m_siteInfoMap.begin(); it != m_siteInfoMap.end(); ++it) {
        if (it.value().siteIndex == siteIndex) {
            targetSiteAlias = it.key();
            break;
        }
    }
    
    if (targetSiteAlias.isEmpty()) {
        LOG_MODULE_WARNING("DutManager", QString("Site with index %1 not found").arg(siteIndex).toStdString());
        return false;
    }
    
    SiteInfo& siteInfo = m_siteInfoMap[targetSiteAlias];
    siteInfo.hasChips = (chipMask != 0);
    siteInfo.currentChipMask = chipMask;
    siteInfo.lastPlacementTime = QDateTime::currentDateTime();
    
    if (!siteSn.isEmpty() && siteInfo.siteSN != siteSn) {
        siteInfo.siteSN = siteSn;
    }
    
    LOG_MODULE_INFO("DutManager", QString("Updated chip placement for site %1 (index %2): mask=0x%3")
                    .arg(targetSiteAlias).arg(siteIndex).arg(chipMask, 0, 16).toStdString());
    
    // 释放锁后发射信号
    locker.unlock();
    emit siteChipPlacementChanged(siteIndex, chipMask);
    
    return true;
}

bool DutManager::updateSiteTestResult(int siteIndex, bool success)
{
    QMutexLocker locker(&m_mutex);
    
    // 查找对应的站点
    QString targetSiteAlias;
    for (auto it = m_siteInfoMap.begin(); it != m_siteInfoMap.end(); ++it) {
        if (it.value().siteIndex == siteIndex) {
            targetSiteAlias = it.key();
            break;
        }
    }
    
    if (targetSiteAlias.isEmpty()) {
        LOG_MODULE_WARNING("DutManager", QString("Site with index %1 not found").arg(siteIndex).toStdString());
        return false;
    }
    
    SiteInfo& siteInfo = m_siteInfoMap[targetSiteAlias];
    if (success) {
        siteInfo.successCount++;
    } else {
        siteInfo.failCount++;
    }
    
    LOG_MODULE_INFO("DutManager", QString("Updated test result for site %1 (index %2): %3 (total: %4 success, %5 fail)")
                    .arg(targetSiteAlias).arg(siteIndex).arg(success ? "SUCCESS" : "FAIL")
                    .arg(siteInfo.successCount).arg(siteInfo.failCount).toStdString());
    
    // 释放锁后发射信号
    locker.unlock();
    emit siteTestResultUpdated(siteIndex, success);
    
    return true;
}

bool DutManager::clearSiteChips(int siteIndex)
{
    QMutexLocker locker(&m_mutex);
    
    // 查找对应的站点
    QString targetSiteAlias;
    for (auto it = m_siteInfoMap.begin(); it != m_siteInfoMap.end(); ++it) {
        if (it.value().siteIndex == siteIndex) {
            targetSiteAlias = it.key();
            break;
        }
    }
    
    if (targetSiteAlias.isEmpty()) {
        LOG_MODULE_WARNING("DutManager", QString("Site with index %1 not found").arg(siteIndex).toStdString());
        return false;
    }
    
    SiteInfo& siteInfo = m_siteInfoMap[targetSiteAlias];
    siteInfo.hasChips = false;
    siteInfo.currentChipMask = 0;
    
    LOG_MODULE_INFO("DutManager", QString("Cleared chips for site %1 (index %2)")
                    .arg(targetSiteAlias).arg(siteIndex).toStdString());
    
    // 释放锁后发射信号
    locker.unlock();
    emit siteChipPlacementChanged(siteIndex, 0);
    
    return true;
}

QVariantMap DutManager::getSiteStatus(int siteIndex) const
{
    QMutexLocker locker(&m_mutex);
    
    QVariantMap status;
    
    // 查找对应的站点
    for (const auto& siteInfo : m_siteInfoMap) {
        if (siteInfo.siteIndex == siteIndex) {
            status["siteIndex"] = siteInfo.siteIndex;
            status["siteAlias"] = siteInfo.siteAlias;
            status["siteSN"] = siteInfo.siteSN;
            status["hasChips"] = siteInfo.hasChips;
            status["chipMask"] = QString("0x%1").arg(siteInfo.currentChipMask, 0, 16);
            status["socketCount"] = siteInfo.socketCount;
            status["socketEnable"] = QString("0x%1").arg(siteInfo.socketEnable, 0, 16);
            status["successCount"] = siteInfo.successCount;
            status["failCount"] = siteInfo.failCount;
            status["lastPlacementTime"] = siteInfo.lastPlacementTime.toString(Qt::ISODate);
            status["currentWorkflowId"] = siteInfo.currentWorkflowId;
            status["found"] = true;
            break;
        }
    }
    
    if (!status.contains("found")) {
        status["found"] = false;
        status["siteIndex"] = siteIndex;
        LOG_MODULE_DEBUG("DutManager", QString("Site status query for index %1: not found").arg(siteIndex).toStdString());
    }
    
    return status;
}

} // namespace Services
