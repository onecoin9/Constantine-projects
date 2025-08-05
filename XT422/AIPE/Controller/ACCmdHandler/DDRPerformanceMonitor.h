#ifndef DDRPERFORMANCEMONITOR_H
#define DDRPERFORMANCEMONITOR_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QDateTime>
#include <atomic>
#include <memory>

// DDR性能监控器 - 专门用于监控80路921600波特率高速数据处理性能
class DDRPerformanceMonitor : public QObject
{
    Q_OBJECT

public:
    explicit DDRPerformanceMonitor(QObject *parent = nullptr);
    ~DDRPerformanceMonitor();

    // 性能指标记录
    void recordTaskReceived(uint8_t portIndex, qint64 timestamp = 0);
    void recordTaskProcessed(uint8_t portIndex, qint64 timestamp = 0, bool success = true);
    void recordDataThroughput(uint8_t portIndex, uint32_t dataSize);
    void recordProcessingLatency(uint8_t portIndex, qint64 latencyMs);

    // 监控控制
    void startMonitoring(int intervalMs = 5000);
    void stopMonitoring();
    void resetCounters();

    // 性能报告
    void printPerformanceReport();
    void printPortStatistics(uint8_t portIndex);
    void printThroughputAnalysis();

    // 告警系统
    void setLatencyThreshold(qint64 thresholdMs) { m_latencyThreshold = thresholdMs; }
    void setThroughputThreshold(qint64 bytesPerSecond) { m_throughputThreshold = bytesPerSecond; }
    void setErrorRateThreshold(double percentage) { m_errorRateThreshold = percentage; }

signals:
    void highLatencyDetected(uint8_t portIndex, qint64 latencyMs);
    void lowThroughputDetected(uint8_t portIndex, qint64 currentThroughput);
    void highErrorRateDetected(uint8_t portIndex, double errorRate);

private slots:
    void onMonitoringTimer();

private:
    struct PortStatistics {
        std::atomic<qint64> tasksReceived{0};
        std::atomic<qint64> tasksProcessed{0};
        std::atomic<qint64> tasksSuccessful{0};
        std::atomic<qint64> totalDataBytes{0};
        std::atomic<qint64> totalLatencyMs{0};
        std::atomic<qint64> maxLatencyMs{0};
        std::atomic<qint64> minLatencyMs{LLONG_MAX};
        qint64 lastResetTime;
        
        PortStatistics() : lastResetTime(QDateTime::currentMSecsSinceEpoch()) {}
    };

    static constexpr int MAX_PORTS = 80;
    PortStatistics m_portStats[MAX_PORTS];
    
    // 全局统计
    std::atomic<qint64> m_globalTasksReceived{0};
    std::atomic<qint64> m_globalTasksProcessed{0};
    std::atomic<qint64> m_globalDataBytes{0};
    
    // 监控配置
    QTimer* m_monitorTimer;
    std::atomic<bool> m_monitoring{false};
    qint64 m_startTime;
    
    // 告警阈值
    qint64 m_latencyThreshold = 100;      // 100ms延迟告警
    qint64 m_throughputThreshold = 115200; // 115200 bytes/s (921600 bps)
    double m_errorRateThreshold = 5.0;    // 5%错误率告警
    
    // 内部方法
    void checkAlerts();
    void updateThroughputStatistics();
    double calculateErrorRate(uint8_t portIndex) const;
    qint64 calculateThroughput(uint8_t portIndex) const;
};

// 性能测量辅助类 - RAII方式自动测量执行时间
class DDRPerformanceTimer
{
public:
    DDRPerformanceTimer(DDRPerformanceMonitor* monitor, uint8_t portIndex)
        : m_monitor(monitor), m_portIndex(portIndex)
        , m_startTime(QDateTime::currentMSecsSinceEpoch()) {}
    
    ~DDRPerformanceTimer() {
        if (m_monitor) {
            qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - m_startTime;
            m_monitor->recordProcessingLatency(m_portIndex, elapsed);
        }
    }

private:
    DDRPerformanceMonitor* m_monitor;
    uint8_t m_portIndex;
    qint64 m_startTime;
};

// 便利宏定义
#define DDR_PERF_TIMER(monitor, port) DDRPerformanceTimer _perf_timer(monitor, port)

#endif // DDRPERFORMANCEMONITOR_H
