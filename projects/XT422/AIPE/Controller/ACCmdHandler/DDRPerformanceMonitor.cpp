#include "DDRPerformanceMonitor.h"
#include "AngkLogger.h"
#include <QDebug>

DDRPerformanceMonitor::DDRPerformanceMonitor(QObject *parent)
    : QObject(parent)
    , m_monitorTimer(new QTimer(this))
    , m_startTime(QDateTime::currentMSecsSinceEpoch())
{
    connect(m_monitorTimer, &QTimer::timeout, this, &DDRPerformanceMonitor::onMonitoringTimer);
}

DDRPerformanceMonitor::~DDRPerformanceMonitor()
{
    stopMonitoring();
}

void DDRPerformanceMonitor::recordTaskReceived(uint8_t portIndex, qint64 timestamp)
{
    if (portIndex >= MAX_PORTS) return;
    
    m_portStats[portIndex].tasksReceived.fetch_add(1, std::memory_order_relaxed);
    m_globalTasksReceived.fetch_add(1, std::memory_order_relaxed);
}

void DDRPerformanceMonitor::recordTaskProcessed(uint8_t portIndex, qint64 timestamp, bool success)
{
    if (portIndex >= MAX_PORTS) return;
    
    m_portStats[portIndex].tasksProcessed.fetch_add(1, std::memory_order_relaxed);
    if (success) {
        m_portStats[portIndex].tasksSuccessful.fetch_add(1, std::memory_order_relaxed);
    }
    m_globalTasksProcessed.fetch_add(1, std::memory_order_relaxed);
}

void DDRPerformanceMonitor::recordDataThroughput(uint8_t portIndex, uint32_t dataSize)
{
    if (portIndex >= MAX_PORTS) return;
    
    m_portStats[portIndex].totalDataBytes.fetch_add(dataSize, std::memory_order_relaxed);
    m_globalDataBytes.fetch_add(dataSize, std::memory_order_relaxed);
}

void DDRPerformanceMonitor::recordProcessingLatency(uint8_t portIndex, qint64 latencyMs)
{
    if (portIndex >= MAX_PORTS) return;
    
    auto& stats = m_portStats[portIndex];
    stats.totalLatencyMs.fetch_add(latencyMs, std::memory_order_relaxed);
    
    // 更新最大/最小延迟（使用compare_exchange_weak实现原子更新）
    qint64 currentMax = stats.maxLatencyMs.load(std::memory_order_relaxed);
    while (latencyMs > currentMax && 
           !stats.maxLatencyMs.compare_exchange_weak(currentMax, latencyMs, std::memory_order_relaxed)) {
        // 重试直到成功
    }
    
    qint64 currentMin = stats.minLatencyMs.load(std::memory_order_relaxed);
    while (latencyMs < currentMin && 
           !stats.minLatencyMs.compare_exchange_weak(currentMin, latencyMs, std::memory_order_relaxed)) {
        // 重试直到成功
    }
}

void DDRPerformanceMonitor::startMonitoring(int intervalMs)
{
    if (!m_monitoring.load(std::memory_order_relaxed)) {
        m_monitoring.store(true, std::memory_order_relaxed);
        m_startTime = QDateTime::currentMSecsSinceEpoch();
        m_monitorTimer->start(intervalMs);
        
        ALOG_INFO("DDR Performance monitoring started with %dms interval", "DDR_PERF", "--", intervalMs);
    }
}

void DDRPerformanceMonitor::stopMonitoring()
{
    if (m_monitoring.load(std::memory_order_relaxed)) {
        m_monitoring.store(false, std::memory_order_relaxed);
        m_monitorTimer->stop();
        
        ALOG_INFO("DDR Performance monitoring stopped", "DDR_PERF", "--");
        printPerformanceReport(); // 停止时打印最终报告
    }
}

void DDRPerformanceMonitor::resetCounters()
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    
    for (int i = 0; i < MAX_PORTS; ++i) {
        m_portStats[i].tasksReceived.store(0, std::memory_order_relaxed);
        m_portStats[i].tasksProcessed.store(0, std::memory_order_relaxed);
        m_portStats[i].tasksSuccessful.store(0, std::memory_order_relaxed);
        m_portStats[i].totalDataBytes.store(0, std::memory_order_relaxed);
        m_portStats[i].totalLatencyMs.store(0, std::memory_order_relaxed);
        m_portStats[i].maxLatencyMs.store(0, std::memory_order_relaxed);
        m_portStats[i].minLatencyMs.store(LLONG_MAX, std::memory_order_relaxed);
        m_portStats[i].lastResetTime = currentTime;
    }
    
    m_globalTasksReceived.store(0, std::memory_order_relaxed);
    m_globalTasksProcessed.store(0, std::memory_order_relaxed);
    m_globalDataBytes.store(0, std::memory_order_relaxed);
    m_startTime = currentTime;
    
    ALOG_INFO("DDR Performance counters reset", "DDR_PERF", "--");
}

void DDRPerformanceMonitor::printPerformanceReport()
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    double runtimeSeconds = (currentTime - m_startTime) / 1000.0;
    
    qint64 globalReceived = m_globalTasksReceived.load(std::memory_order_relaxed);
    qint64 globalProcessed = m_globalTasksProcessed.load(std::memory_order_relaxed);
    qint64 globalDataBytes = m_globalDataBytes.load(std::memory_order_relaxed);
    
    ALOG_INFO("=== DDR Performance Report (Runtime: %.2f seconds) ===", "DDR_PERF", "--", runtimeSeconds);
    ALOG_INFO("Global: Received: %lld, Processed: %lld, Data: %.2f MB", 
        "DDR_PERF", "--", globalReceived, globalProcessed, globalDataBytes / (1024.0 * 1024.0));
    ALOG_INFO("Global throughput: %.2f tasks/s, %.2f MB/s", 
        "DDR_PERF", "--", globalReceived / runtimeSeconds, (globalDataBytes / (1024.0 * 1024.0)) / runtimeSeconds);
    
    // 统计活跃端口
    int activePorts = 0;
    for (int i = 0; i < MAX_PORTS; ++i) {
        if (m_portStats[i].tasksReceived.load(std::memory_order_relaxed) > 0) {
            activePorts++;
        }
    }
    ALOG_INFO("Active ports: %d", "DDR_PERF", "--", activePorts);
}

void DDRPerformanceMonitor::printPortStatistics(uint8_t portIndex)
{
    if (portIndex >= MAX_PORTS) return;
    
    const auto& stats = m_portStats[portIndex];
    qint64 received = stats.tasksReceived.load(std::memory_order_relaxed);
    qint64 processed = stats.tasksProcessed.load(std::memory_order_relaxed);
    qint64 successful = stats.tasksSuccessful.load(std::memory_order_relaxed);
    qint64 dataBytes = stats.totalDataBytes.load(std::memory_order_relaxed);
    qint64 totalLatency = stats.totalLatencyMs.load(std::memory_order_relaxed);
    qint64 maxLatency = stats.maxLatencyMs.load(std::memory_order_relaxed);
    qint64 minLatency = stats.minLatencyMs.load(std::memory_order_relaxed);
    
    if (received == 0) {
        ALOG_INFO("Port %d: No activity", "DDR_PERF", "--", portIndex);
        return;
    }
    
    double successRate = (double)successful / processed * 100.0;
    double avgLatency = processed > 0 ? (double)totalLatency / processed : 0.0;
    qint64 throughput = calculateThroughput(portIndex);
    
    ALOG_INFO("Port %d: Recv: %lld, Proc: %lld, Success: %.1f%%, Data: %.2f KB", 
        "DDR_PERF", "--", portIndex, received, processed, successRate, dataBytes / 1024.0);
    ALOG_INFO("Port %d: Latency - Avg: %.2fms, Min: %lldms, Max: %lldms, Throughput: %lld B/s", 
        "DDR_PERF", "--", portIndex, avgLatency, minLatency, maxLatency, throughput);
}

void DDRPerformanceMonitor::printThroughputAnalysis()
{
    ALOG_INFO("=== Throughput Analysis ===", "DDR_PERF", "--");
    
    for (int i = 0; i < MAX_PORTS; ++i) {
        qint64 received = m_portStats[i].tasksReceived.load(std::memory_order_relaxed);
        if (received > 0) {
            qint64 throughput = calculateThroughput(i);
            double errorRate = calculateErrorRate(i);
            
            if (throughput < m_throughputThreshold || errorRate > m_errorRateThreshold) {
                ALOG_WARN("Port %d: LOW PERFORMANCE - Throughput: %lld B/s, Error rate: %.2f%%", 
                    "DDR_PERF", "--", i, throughput, errorRate);
            } else {
                ALOG_INFO("Port %d: OK - Throughput: %lld B/s, Error rate: %.2f%%", 
                    "DDR_PERF", "--", i, throughput, errorRate);
            }
        }
    }
}

void DDRPerformanceMonitor::onMonitoringTimer()
{
    if (!m_monitoring.load(std::memory_order_relaxed)) return;
    
    checkAlerts();
    updateThroughputStatistics();
    
    // 每10次监控周期打印一次详细报告
    static int cycleCount = 0;
    if (++cycleCount % 10 == 0) {
        printPerformanceReport();
        printThroughputAnalysis();
    }
}

void DDRPerformanceMonitor::checkAlerts()
{
    for (int i = 0; i < MAX_PORTS; ++i) {
        qint64 received = m_portStats[i].tasksReceived.load(std::memory_order_relaxed);
        if (received == 0) continue;
        
        // 检查延迟告警
        qint64 maxLatency = m_portStats[i].maxLatencyMs.load(std::memory_order_relaxed);
        if (maxLatency > m_latencyThreshold) {
            emit highLatencyDetected(i, maxLatency);
        }
        
        // 检查吞吐量告警
        qint64 throughput = calculateThroughput(i);
        if (throughput < m_throughputThreshold) {
            emit lowThroughputDetected(i, throughput);
        }
        
        // 检查错误率告警
        double errorRate = calculateErrorRate(i);
        if (errorRate > m_errorRateThreshold) {
            emit highErrorRateDetected(i, errorRate);
        }
    }
}

void DDRPerformanceMonitor::updateThroughputStatistics()
{
    // 这里可以添加更复杂的吞吐量统计逻辑
    // 例如计算滑动窗口平均值等
}

double DDRPerformanceMonitor::calculateErrorRate(uint8_t portIndex) const
{
    if (portIndex >= MAX_PORTS) return 0.0;
    
    qint64 processed = m_portStats[portIndex].tasksProcessed.load(std::memory_order_relaxed);
    qint64 successful = m_portStats[portIndex].tasksSuccessful.load(std::memory_order_relaxed);
    
    if (processed == 0) return 0.0;
    
    return ((double)(processed - successful) / processed) * 100.0;
}

qint64 DDRPerformanceMonitor::calculateThroughput(uint8_t portIndex) const
{
    if (portIndex >= MAX_PORTS) return 0;
    
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 dataBytes = m_portStats[portIndex].totalDataBytes.load(std::memory_order_relaxed);
    qint64 elapsedMs = currentTime - m_portStats[portIndex].lastResetTime;
    
    if (elapsedMs == 0) return 0;
    
    return (dataBytes * 1000) / elapsedMs; // bytes per second
}
