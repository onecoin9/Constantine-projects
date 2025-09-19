#ifndef DATAFLOWMONITOR_H
#define DATAFLOWMONITOR_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QDateTime>
#include <QMap>
#include <QString>
#include <QQueue>
#include <QAtomicInteger>
#include <memory>

// 单个数据包的监控信息
struct DataPacketTrace {
    quint64 id;                        // 唯一标识
    QDateTime entryTime;               // 进入时间
    QDateTime processLocalTime;        // ProcessLocalMessage处理时间
    QDateTime dllHandlerTime;          // DllHandler处理时间
    QDateTime serialHandlerTime;       // SerialHandler处理时间
    QDateTime managerSendTime;         // Manager发送时间
    QDateTime workerSendTime;          // Worker发送时间
    QDateTime serialPortTime;          // 串口发送时间
    
    QString strIPHop;                  // 目标IP:Hop
    uint16_t BPUID;                    // BPU ID
    uint8_t comNum;                    // 通道号
    QString portName;                  // 端口名
    int dataSize;                      // 数据大小
    bool completed;                    // 是否完成发送
    bool failed;                       // 是否失败
    QString errorMsg;                  // 错误信息
    
    DataPacketTrace() : id(0), BPUID(0), comNum(0), dataSize(0), completed(false), failed(false) {}
};

// 性能统计数据
struct PerformanceStats {
    // 计数统计
    QAtomicInteger<quint64> totalPackets{0};      // 总包数
    QAtomicInteger<quint64> completedPackets{0};  // 完成包数
    QAtomicInteger<quint64> failedPackets{0};     // 失败包数
    QAtomicInteger<quint64> totalBytes{0};        // 总字节数
    
    // 时延统计（微秒）
    QAtomicInteger<quint64> totalLatency{0};      // 总时延累计
    QAtomicInteger<quint64> maxLatency{0};        // 最大时延
    QAtomicInteger<quint64> minLatency{UINT64_MAX}; // 最小时延
    
    // 各阶段时延统计
    QAtomicInteger<quint64> processLocalLatency{0};   // ProcessLocal阶段
    QAtomicInteger<quint64> dllHandlerLatency{0};     // DllHandler阶段
    QAtomicInteger<quint64> serialHandlerLatency{0};  // SerialHandler阶段
    QAtomicInteger<quint64> managerLatency{0};        // Manager阶段
    QAtomicInteger<quint64> workerLatency{0};         // Worker阶段
    
    // 吞吐量统计
    QAtomicInteger<quint64> packetsPerSecond{0};  // 每秒包数
    QAtomicInteger<quint64> bytesPerSecond{0};    // 每秒字节数
    
    void reset() {
        totalPackets = 0;
        completedPackets = 0;
        failedPackets = 0;
        totalBytes = 0;
        totalLatency = 0;
        maxLatency = 0;
        minLatency = UINT64_MAX;
        processLocalLatency = 0;
        dllHandlerLatency = 0;
        serialHandlerLatency = 0;
        managerLatency = 0;
        workerLatency = 0;
        packetsPerSecond = 0;
        bytesPerSecond = 0;
    }
};

// 按端口分组的统计
struct PortStats {
    QString portName;
    PerformanceStats stats;
    QQueue<quint64> recentLatencies;  // 最近的时延记录
    QDateTime lastActivity;           // 最后活动时间
};

class DataFlowMonitor : public QObject
{
    Q_OBJECT

public:
    static DataFlowMonitor* instance();
    static void destroyInstance();
    
    // 数据包跟踪方法
    quint64 tracePacketEntry(const QString& strIPHop, const uint16_t BPUID, 
                            const uint8_t comNum, int dataSize);
    void traceProcessLocal(quint64 id);
    void traceDllHandler(quint64 id);
    void traceSerialHandler(quint64 id, const QString& portName);
    void traceManagerSend(quint64 id);
    void traceWorkerSend(quint64 id);
    void traceSerialPortSend(quint64 id, bool success = true, const QString& errorMsg = "");
    
    // 错误跟踪
    void traceError(quint64 id, const QString& stage, const QString& errorMsg);
    
    // 统计查询
    PerformanceStats getOverallStats() const;
    PerformanceStats getPortStats(const QString& portName) const;
    QMap<QString, PerformanceStats> getAllPortStats() const;
    
    // 实时数据
    double getCurrentThroughput() const;  // 当前吞吐量 (包/秒)
    double getCurrentBandwidth() const;   // 当前带宽 (字节/秒)
    double getAverageLatency() const;     // 平均时延 (毫秒)
    
    // 配置
    void setMaxTraceItems(int maxItems);  // 设置最大跟踪项数
    void setReportInterval(int intervalMs); // 设置报告间隔
    void enableDetailedLogging(bool enable); // 启用详细日志
    
    // 控制
    void startMonitoring();
    void stopMonitoring();
    void resetStats();
    
signals:
    void performanceReport(const PerformanceStats& stats);
    void bottleneckDetected(const QString& stage, double latencyMs);
    void highLatencyWarning(quint64 packetId, double latencyMs);

private slots:
    void generateReport();
    void cleanupOldTraces();

private:
    explicit DataFlowMonitor(QObject* parent = nullptr);
    ~DataFlowMonitor();
    Q_DISABLE_COPY(DataFlowMonitor)
    
    void updateStats(const DataPacketTrace& trace);
    void detectBottlenecks(const DataPacketTrace& trace);
    void calculateThroughput();
    
    static DataFlowMonitor* s_instance;
    static QMutex s_instanceMutex;
    
    mutable QMutex m_mutex;
    QMap<quint64, std::shared_ptr<DataPacketTrace>> m_traces;
    QAtomicInteger<quint64> m_nextId{1};
    
    // 统计数据
    PerformanceStats m_overallStats;
    QMap<QString, PortStats> m_portStats;
    
    // 配置参数
    int m_maxTraceItems{10000};
    int m_reportInterval{20000};  // 20秒
    bool m_detailedLogging{false};
    bool m_monitoring{false};
    
    // 定时器
    QTimer* m_reportTimer;
    QTimer* m_cleanupTimer;
    
    // 吞吐量计算
    QQueue<QPair<QDateTime, int>> m_throughputHistory;
    mutable QMutex m_throughputMutex;
};

// 便利宏，用于在代码中插入监控点
#define TRACE_PACKET_ENTRY(strIPHop, BPUID, comNum, dataSize) \
    DataFlowMonitor::instance()->tracePacketEntry(strIPHop, BPUID, comNum, dataSize)

#define TRACE_PROCESS_LOCAL(id) \
    DataFlowMonitor::instance()->traceProcessLocal(id)

#define TRACE_DLL_HANDLER(id) \
    DataFlowMonitor::instance()->traceDllHandler(id)

#define TRACE_SERIAL_HANDLER(id, portName) \
    DataFlowMonitor::instance()->traceSerialHandler(id, portName)

#define TRACE_MANAGER_SEND(id) \
    DataFlowMonitor::instance()->traceManagerSend(id)

#define TRACE_WORKER_SEND(id) \
    DataFlowMonitor::instance()->traceWorkerSend(id)

#define TRACE_SERIAL_PORT_SEND(id, success, errorMsg) \
    DataFlowMonitor::instance()->traceSerialPortSend(id, success, errorMsg)

#define TRACE_ERROR(id, stage, errorMsg) \
    DataFlowMonitor::instance()->traceError(id, stage, errorMsg)

#define TRACE_COMPLETE_SUCCESS(id) \
    DataFlowMonitor::instance()->traceSerialPortSend(id, true, "")

#endif // DATAFLOWMONITOR_H
