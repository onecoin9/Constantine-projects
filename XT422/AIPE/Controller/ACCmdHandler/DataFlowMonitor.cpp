#include "DataFlowMonitor.h"
#include <QDebug>
#include <QMutexLocker>
#include <QThread>
#include <AngkLogger.h>

DataFlowMonitor* DataFlowMonitor::s_instance = nullptr;
QMutex DataFlowMonitor::s_instanceMutex;

DataFlowMonitor* DataFlowMonitor::instance()
{
    if (!s_instance) {
        QMutexLocker locker(&s_instanceMutex);
        if (!s_instance) {
            s_instance = new DataFlowMonitor();
        }
    }
    return s_instance;
}

void DataFlowMonitor::destroyInstance()
{
    QMutexLocker locker(&s_instanceMutex);
    if (s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }
}

DataFlowMonitor::DataFlowMonitor(QObject* parent)
    : QObject(parent)
    , m_reportTimer(new QTimer(this))
    , m_cleanupTimer(new QTimer(this))
{
    // 设置定时器
    m_reportTimer->setInterval(m_reportInterval);
    m_cleanupTimer->setInterval(60000); // 1分钟清理一次
    
    connect(m_reportTimer, &QTimer::timeout, this, &DataFlowMonitor::generateReport);
    connect(m_cleanupTimer, &QTimer::timeout, this, &DataFlowMonitor::cleanupOldTraces);
    
    ALOG_INFO("DataFlowMonitor initialized", "CU", "--");
}

DataFlowMonitor::~DataFlowMonitor()
{
    stopMonitoring();
    ALOG_INFO("DataFlowMonitor destroyed", "CU", "--");
}

void DataFlowMonitor::startMonitoring()
{
    if (!m_monitoring) {
        m_monitoring = true;
        m_reportTimer->start();
        m_cleanupTimer->start();
        resetStats();
        ALOG_INFO("DataFlowMonitor started", "CU", "--");
    }
}

void DataFlowMonitor::stopMonitoring()
{
    if (m_monitoring) {
        m_monitoring = false;
        m_reportTimer->stop();
        m_cleanupTimer->stop();
        ALOG_INFO("DataFlowMonitor stopped", "CU", "--");
    }
}

void DataFlowMonitor::resetStats()
{
    QMutexLocker locker(&m_mutex);
    m_overallStats.reset();
    
    for (auto& portStat : m_portStats) {
        portStat.stats.reset();
        portStat.recentLatencies.clear();
    }
    
    {
        QMutexLocker throughputLocker(&m_throughputMutex);
        m_throughputHistory.clear();
    }
    
    ALOG_INFO("DataFlowMonitor stats reset", "CU", "--");
}

quint64 DataFlowMonitor::tracePacketEntry(const QString& strIPHop, const uint16_t BPUID, 
                                          const uint8_t comNum, int dataSize)
{
    if (!m_monitoring) return 0;
    
    quint64 id = m_nextId.fetchAndAddOrdered(1);
    auto trace = std::make_shared<DataPacketTrace>();
    
    trace->id = id;
    trace->entryTime = QDateTime::currentDateTime();
    trace->strIPHop = strIPHop;
    trace->BPUID = BPUID;
    trace->comNum = comNum;
    trace->dataSize = dataSize;
    
    {
        QMutexLocker locker(&m_mutex);
        m_traces[id] = trace;
        
        // 更新统计
        m_overallStats.totalPackets.fetchAndAddOrdered(1);
        m_overallStats.totalBytes.fetchAndAddOrdered(dataSize);
    }
    
    // 记录吞吐量数据
    {
        QMutexLocker throughputLocker(&m_throughputMutex);
        m_throughputHistory.enqueue(qMakePair(QDateTime::currentDateTime(), dataSize));
    }
    
    if (m_detailedLogging) {
        ALOG_DEBUG("Packet %llu entered: %s:%d:%d, size=%d", "CU", "--", 
                   id, strIPHop.toStdString().c_str(), BPUID, comNum, dataSize);
    }
    
    return id;
}

void DataFlowMonitor::traceProcessLocal(quint64 id)
{
    if (!m_monitoring || id == 0) return;
    
    QMutexLocker locker(&m_mutex);
    auto it = m_traces.find(id);
    if (it != m_traces.end()) {
        (*it)->processLocalTime = QDateTime::currentDateTime();
        
        if (m_detailedLogging) {
            qint64 elapsed = (*it)->entryTime.msecsTo((*it)->processLocalTime);
            ALOG_DEBUG("Packet %llu ProcessLocal: %lld ms", "CU", "--", id, elapsed);
        }
    }
}

void DataFlowMonitor::traceDllHandler(quint64 id)
{
    if (!m_monitoring || id == 0) return;
    
    QMutexLocker locker(&m_mutex);
    auto it = m_traces.find(id);
    if (it != m_traces.end()) {
        (*it)->dllHandlerTime = QDateTime::currentDateTime();
        
        if (m_detailedLogging) {
            qint64 elapsed = (*it)->processLocalTime.msecsTo((*it)->dllHandlerTime);
            ALOG_DEBUG("Packet %llu DllHandler: %lld ms", "CU", "--", id, elapsed);
        }
    }
}

void DataFlowMonitor::traceSerialHandler(quint64 id, const QString& portName)
{
    if (!m_monitoring || id == 0) return;
    
    QMutexLocker locker(&m_mutex);
    auto it = m_traces.find(id);
    if (it != m_traces.end()) {
        (*it)->serialHandlerTime = QDateTime::currentDateTime();
        (*it)->portName = portName;
        
        if (m_detailedLogging) {
            qint64 elapsed = (*it)->dllHandlerTime.msecsTo((*it)->serialHandlerTime);
            ALOG_DEBUG("Packet %llu SerialHandler: %lld ms, port=%s", "CU", "--", 
                       id, elapsed, portName.toStdString().c_str());
        }
    }
}

void DataFlowMonitor::traceManagerSend(quint64 id)
{
    if (!m_monitoring || id == 0) return;
    
    QMutexLocker locker(&m_mutex);
    auto it = m_traces.find(id);
    if (it != m_traces.end()) {
        (*it)->managerSendTime = QDateTime::currentDateTime();
        
        if (m_detailedLogging) {
            qint64 elapsed = (*it)->serialHandlerTime.msecsTo((*it)->managerSendTime);
            ALOG_DEBUG("Packet %llu ManagerSend: %lld ms", "CU", "--", id, elapsed);
        }
    }
}

void DataFlowMonitor::traceWorkerSend(quint64 id)
{
    if (!m_monitoring || id == 0) return;
    
    QMutexLocker locker(&m_mutex);
    auto it = m_traces.find(id);
    if (it != m_traces.end()) {
        (*it)->workerSendTime = QDateTime::currentDateTime();
        
        if (m_detailedLogging) {
            qint64 elapsed = (*it)->managerSendTime.msecsTo((*it)->workerSendTime);
            ALOG_DEBUG("Packet %llu WorkerSend: %lld ms", "CU", "--", id, elapsed);
        }
    }
}

void DataFlowMonitor::traceSerialPortSend(quint64 id, bool success, const QString& errorMsg)
{
    if (!m_monitoring || id == 0) return;
    
    QMutexLocker locker(&m_mutex);
    auto it = m_traces.find(id);
    if (it != m_traces.end()) {
        (*it)->serialPortTime = QDateTime::currentDateTime();
        (*it)->completed = true;
        (*it)->failed = !success;
        (*it)->errorMsg = errorMsg;
        
        // 更新统计
        if (success) {
            m_overallStats.completedPackets.fetchAndAddOrdered(1);
        } else {
            m_overallStats.failedPackets.fetchAndAddOrdered(1);
        }
        
        updateStats(**it);
        detectBottlenecks(**it);
        
        if (m_detailedLogging) {
            qint64 totalElapsed = (*it)->entryTime.msecsTo((*it)->serialPortTime);
            qint64 workerElapsed = (*it)->workerSendTime.msecsTo((*it)->serialPortTime);
            ALOG_DEBUG("Packet %llu SerialPortSend: %lld ms (total: %lld ms), success=%d", 
                       "CU", "--", id, workerElapsed, totalElapsed, success);
        }
    }
}

void DataFlowMonitor::traceError(quint64 id, const QString& stage, const QString& errorMsg)
{
    if (!m_monitoring || id == 0) return;
    
    QMutexLocker locker(&m_mutex);
    auto it = m_traces.find(id);
    if (it != m_traces.end()) {
        (*it)->failed = true;
        (*it)->errorMsg = QString("%1: %2").arg(stage).arg(errorMsg);
        m_overallStats.failedPackets.fetchAndAddOrdered(1);
        
        ALOG_WARN("Packet %llu error in %s: %s", "CU", "--", 
                  id, stage.toStdString().c_str(), errorMsg.toStdString().c_str());
    }
}

void DataFlowMonitor::updateStats(const DataPacketTrace& trace)
{
    if (!trace.completed) return;
    
    // 计算总时延 (微秒)
    qint64 totalLatencyMs = trace.entryTime.msecsTo(trace.serialPortTime);
    quint64 totalLatencyUs = totalLatencyMs * 1000;
    
    // 更新整体统计
    m_overallStats.totalLatency.fetchAndAddOrdered(totalLatencyUs);
    
    // 更新最大最小时延
    quint64 currentMax = m_overallStats.maxLatency.load();
    while (totalLatencyUs > currentMax && 
           !m_overallStats.maxLatency.testAndSetOrdered(currentMax, totalLatencyUs)) {
        currentMax = m_overallStats.maxLatency.load();
    }
    
    quint64 currentMin = m_overallStats.minLatency.load();
    while (totalLatencyUs < currentMin && 
           !m_overallStats.minLatency.testAndSetOrdered(currentMin, totalLatencyUs)) {
        currentMin = m_overallStats.minLatency.load();
    }
    
    // 计算各阶段时延
    if (!trace.processLocalTime.isNull()) {
        qint64 stageLatency = trace.entryTime.msecsTo(trace.processLocalTime) * 1000;
        m_overallStats.processLocalLatency.fetchAndAddOrdered(stageLatency);
    }
    
    if (!trace.dllHandlerTime.isNull()) {
        qint64 stageLatency = trace.processLocalTime.msecsTo(trace.dllHandlerTime) * 1000;
        m_overallStats.dllHandlerLatency.fetchAndAddOrdered(stageLatency);
    }
    
    if (!trace.serialHandlerTime.isNull()) {
        qint64 stageLatency = trace.dllHandlerTime.msecsTo(trace.serialHandlerTime) * 1000;
        m_overallStats.serialHandlerLatency.fetchAndAddOrdered(stageLatency);
    }
    
    if (!trace.managerSendTime.isNull()) {
        qint64 stageLatency = trace.serialHandlerTime.msecsTo(trace.managerSendTime) * 1000;
        m_overallStats.managerLatency.fetchAndAddOrdered(stageLatency);
    }
    
    if (!trace.workerSendTime.isNull()) {
        qint64 stageLatency = trace.managerSendTime.msecsTo(trace.workerSendTime) * 1000;
        m_overallStats.workerLatency.fetchAndAddOrdered(stageLatency);
    }
    
    // 更新端口统计
    if (!trace.portName.isEmpty()) {
        auto& portStat = m_portStats[trace.portName];
        portStat.portName = trace.portName;
        portStat.stats.totalPackets.fetchAndAddOrdered(1);
        portStat.stats.totalBytes.fetchAndAddOrdered(trace.dataSize);
        portStat.stats.totalLatency.fetchAndAddOrdered(totalLatencyUs);
        
        if (trace.failed) {
            portStat.stats.failedPackets.fetchAndAddOrdered(1);
        } else {
            portStat.stats.completedPackets.fetchAndAddOrdered(1);
        }
        
        portStat.lastActivity = QDateTime::currentDateTime();
        portStat.recentLatencies.enqueue(totalLatencyUs);
        
        // 保持最近100个时延记录
        while (portStat.recentLatencies.size() > 100) {
            portStat.recentLatencies.dequeue();
        }
    }
}

void DataFlowMonitor::detectBottlenecks(const DataPacketTrace& trace)
{
    if (!trace.completed) return;
    
    const qint64 HIGH_LATENCY_THRESHOLD = 100; // 100ms
    const qint64 BOTTLENECK_THRESHOLD = 50;    // 50ms
    
    qint64 totalLatency = trace.entryTime.msecsTo(trace.serialPortTime);
    
    // 检测高时延
    if (totalLatency > HIGH_LATENCY_THRESHOLD) {
        emit highLatencyWarning(trace.id, totalLatency);
    }
    
    // 检测各阶段瓶颈
    if (!trace.processLocalTime.isNull()) {
        qint64 latency = trace.entryTime.msecsTo(trace.processLocalTime);
        if (latency > BOTTLENECK_THRESHOLD) {
            emit bottleneckDetected("ProcessLocal", latency);
        }
    }
    
    if (!trace.dllHandlerTime.isNull()) {
        qint64 latency = trace.processLocalTime.msecsTo(trace.dllHandlerTime);
        if (latency > BOTTLENECK_THRESHOLD) {
            emit bottleneckDetected("DllHandler", latency);
        }
    }
    
    if (!trace.serialHandlerTime.isNull()) {
        qint64 latency = trace.dllHandlerTime.msecsTo(trace.serialHandlerTime);
        if (latency > BOTTLENECK_THRESHOLD) {
            emit bottleneckDetected("SerialHandler", latency);
        }
    }
    
    if (!trace.managerSendTime.isNull()) {
        qint64 latency = trace.serialHandlerTime.msecsTo(trace.managerSendTime);
        if (latency > BOTTLENECK_THRESHOLD) {
            emit bottleneckDetected("Manager", latency);
        }
    }
    
    if (!trace.workerSendTime.isNull()) {
        qint64 latency = trace.managerSendTime.msecsTo(trace.workerSendTime);
        if (latency > BOTTLENECK_THRESHOLD) {
            emit bottleneckDetected("Worker", latency);
        }
    }
    
    if (!trace.serialPortTime.isNull()) {
        qint64 latency = trace.workerSendTime.msecsTo(trace.serialPortTime);
        if (latency > BOTTLENECK_THRESHOLD) {
            emit bottleneckDetected("SerialPort", latency);
        }
    }
}

void DataFlowMonitor::calculateThroughput()
{
    QMutexLocker throughputLocker(&m_throughputMutex);
    
    QDateTime now = QDateTime::currentDateTime();
    QDateTime oneSecondAgo = now.addSecs(-1);
    
    // 清理超过1秒的记录
    while (!m_throughputHistory.isEmpty() && 
           m_throughputHistory.head().first < oneSecondAgo) {
        m_throughputHistory.dequeue();
    }
    
    // 计算1秒内的包数和字节数
    int packetsInLastSecond = m_throughputHistory.size();
    int bytesInLastSecond = 0;
    
    for (const auto& entry : m_throughputHistory) {
        bytesInLastSecond += entry.second;
    }
    
    m_overallStats.packetsPerSecond.store(packetsInLastSecond);
    m_overallStats.bytesPerSecond.store(bytesInLastSecond);
}

PerformanceStats DataFlowMonitor::getOverallStats() const
{
    QMutexLocker locker(&m_mutex);
    return m_overallStats;
}

PerformanceStats DataFlowMonitor::getPortStats(const QString& portName) const
{
    QMutexLocker locker(&m_mutex);
    auto it = m_portStats.find(portName);
    if (it != m_portStats.end()) {
        return it->stats;
    }
    return PerformanceStats();
}

QMap<QString, PerformanceStats> DataFlowMonitor::getAllPortStats() const
{
    QMutexLocker locker(&m_mutex);
    QMap<QString, PerformanceStats> result;
    
    for (auto it = m_portStats.begin(); it != m_portStats.end(); ++it) {
        result[it.key()] = it->stats;
    }
    
    return result;
}

double DataFlowMonitor::getCurrentThroughput() const
{
    return static_cast<double>(m_overallStats.packetsPerSecond.load());
}

double DataFlowMonitor::getCurrentBandwidth() const
{
    return static_cast<double>(m_overallStats.bytesPerSecond.load());
}

double DataFlowMonitor::getAverageLatency() const
{
    quint64 completed = m_overallStats.completedPackets.load();
    if (completed == 0) return 0.0;
    
    quint64 total = m_overallStats.totalLatency.load();
    return static_cast<double>(total) / completed / 1000.0; // 转换为毫秒
}

void DataFlowMonitor::setMaxTraceItems(int maxItems)
{
    QMutexLocker locker(&m_mutex);
    m_maxTraceItems = maxItems;
}

void DataFlowMonitor::setReportInterval(int intervalMs)
{
    m_reportInterval = intervalMs;
    m_reportTimer->setInterval(intervalMs);
}

void DataFlowMonitor::enableDetailedLogging(bool enable)
{
    m_detailedLogging = enable;
}

void DataFlowMonitor::generateReport()
{
    if (!m_monitoring) return;
    
    calculateThroughput();
    
    QMutexLocker locker(&m_mutex);
    
    // 发送整体统计报告
    emit performanceReport(m_overallStats);
    
    // 输出性能报告
    quint64 total = m_overallStats.totalPackets.load();
    quint64 completed = m_overallStats.completedPackets.load();
    quint64 failed = m_overallStats.failedPackets.load();
    double avgLatency = getAverageLatency();
    double throughput = getCurrentThroughput();
    double bandwidth = getCurrentBandwidth();
    
    ALOG_INFO("=== DataFlow Performance Report ===", "CU", "--");
    ALOG_INFO("Total Packets: %llu, Completed: %llu, Failed: %llu", "CU", "--", 
              total, completed, failed);
    ALOG_INFO("Success Rate: %.2f%%, Avg Latency: %.2f ms", "CU", "--", 
              total > 0 ? (double)completed / total * 100 : 0.0, avgLatency);
    ALOG_INFO("Throughput: %.1f pkt/s, Bandwidth: %.1f B/s", "CU", "--", 
              throughput, bandwidth);
    
    // 报告各端口统计
    for (auto it = m_portStats.begin(); it != m_portStats.end(); ++it) {
        const auto& portStat = it.value();
        quint64 portTotal = portStat.stats.totalPackets.load();
        quint64 portCompleted = portStat.stats.completedPackets.load();
        
        if (portTotal > 0) {
            double portAvgLatency = portTotal > 0 ? 
                static_cast<double>(portStat.stats.totalLatency.load()) / portTotal / 1000.0 : 0.0;
            
            ALOG_INFO("Port %s: %llu packets, %.2f ms avg latency", "CU", "--", 
                      it.key().toStdString().c_str(), portTotal, portAvgLatency);
        }
    }
}

void DataFlowMonitor::cleanupOldTraces()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_traces.size() <= m_maxTraceItems) return;
    
    // 按时间排序，移除最旧的项目
    QMultiMap<QDateTime, quint64> timeToId;
    for (auto it = m_traces.begin(); it != m_traces.end(); ++it) {
        timeToId.insert(it.value()->entryTime, it.key());
    }
    
    int toRemove = m_traces.size() - m_maxTraceItems;
    auto removeIt = timeToId.begin();
    
    for (int i = 0; i < toRemove && removeIt != timeToId.end(); ++i, ++removeIt) {
        m_traces.remove(removeIt.value());
    }
    
    if (m_detailedLogging) {
        ALOG_DEBUG("Cleaned up %d old trace items", "CU", "--", toRemove);
    }
}
