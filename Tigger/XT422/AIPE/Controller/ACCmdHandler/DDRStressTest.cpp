// DDR 80路高速数据处理压力测试
// 专门测试921600波特率 x 80通道的极限性能场景

#include "CustomMessageHandler.h"
#include "DDRPerformanceMonitor.h"
#include "AngkLogger.h"
#include <QCoreApplication>
#include <QTimer>
#include <QThread>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <atomic>

class DDRStressTest : public QObject
{
    Q_OBJECT

public:
    DDRStressTest(QObject* parent = nullptr) : QObject(parent), m_totalMessages(0), m_running(false)
    {
        m_handler = CustomMessageHandler::instance();
        setupStressTest();
    }

    void startStressTest()
    {
        ALOG_INFO("Starting DDR 80-Channel 921600 Baud Stress Test", "STRESS", "--");
        
        // 极限性能配置
        m_handler->configureDDRProcessor(
            1,     // 重试次数：1次 (极限快速失败)
            1,     // 基础延迟：1ms (最小延迟)
            5,     // 重试延迟：5ms
            2000   // 统计间隔：2秒 (密集监控)
        );
        
        // 启动高频性能监控
        m_handler->startPerformanceMonitoring(500); // 500ms监控间隔
        
        // 配置告警阈值为更严格的标准
        if (m_handler->getPerformanceMonitor()) {
            m_handler->getPerformanceMonitor()->setLatencyThreshold(10);    // 10ms延迟告警
            m_handler->getPerformanceMonitor()->setThroughputThreshold(230400); // 230.4KB/s (921600*2/8)
            m_handler->getPerformanceMonitor()->setErrorRateThreshold(1.0);     // 1%错误率告警
        }
        
        m_running = true;
        m_testStartTime.start();
        
        // 启动80个并发数据生成器
        startHighSpeedDataGenerators();
        
        // 启动监控定时器
        startMonitoringTimers();
    }

    void stopStressTest()
    {
        ALOG_INFO("Stopping DDR Stress Test", "STRESS", "--");
        m_running = false;
        
        // 停止所有定时器
        for (auto* timer : m_dataTimers) {
            timer->stop();
            timer->deleteLater();
        }
        m_dataTimers.clear();
        
        // 等待所有任务完成
        QThread::sleep(3);
        
        // 生成最终报告
        generateFinalReport();
        
        QCoreApplication::quit();
    }

private slots:
    void generatePeriodicReport()
    {
        qint64 elapsed = m_testStartTime.elapsed();
        double elapsedSeconds = elapsed / 1000.0;
        double messageRate = m_totalMessages.load() / elapsedSeconds;
        
        ALOG_INFO("=== Stress Test Report (%.1fs) ===", "STRESS", "--", elapsedSeconds);
        ALOG_INFO("Total Messages: %d, Rate: %.1f msg/s", "STRESS", "--", 
                  m_totalMessages.load(), messageRate);
        ALOG_INFO("Expected Rate: %.1f msg/s (921600*80/8/1024)", "STRESS", "--", 
                  921600.0 * 80 / 8 / 1024); // 理论最大包率
        
        m_handler->printDDRStats();
        m_handler->printPerformanceReport();
    }

private:
    void setupStressTest()
    {
        // 连接性能告警信号
        if (m_handler->getPerformanceMonitor()) {
            connect(m_handler->getPerformanceMonitor(), &DDRPerformanceMonitor::highLatencyDetected,
                    [](uint8_t port, qint64 latency) {
                        ALOG_ERROR("CRITICAL: High latency on port %d: %lld ms", "STRESS", "--", port, latency);
                    });
            
            connect(m_handler->getPerformanceMonitor(), &DDRPerformanceMonitor::lowThroughputDetected,
                    [](uint8_t port, qint64 throughput) {
                        ALOG_ERROR("CRITICAL: Low throughput on port %d: %lld bytes/s", "STRESS", "--", port, throughput);
                    });
            
            connect(m_handler->getPerformanceMonitor(), &DDRPerformanceMonitor::highErrorRateDetected,
                    [](uint8_t port, double errorRate) {
                        ALOG_ERROR("CRITICAL: High error rate on port %d: %.2f%%", "STRESS", "--", port, errorRate);
                    });
        }
    }
    
    void startHighSpeedDataGenerators()
    {
        ALOG_INFO("Starting 80 high-speed data generators", "STRESS", "--");
        
        // 为每个端口创建高频数据生成器
        for (int port = 0; port < 80; ++port) {
            QTimer* timer = new QTimer(this);
            
            // 计算每端口的理论间隔
            // 921600 bps = 115200 bytes/s per port
            // 假设平均包大小1KB，每端口约115 packages/s
            // 间隔约 8.7ms，设置为5-10ms随机间隔模拟真实网络抖动
            int baseInterval = 7; // 基础7ms间隔
            int jitter = port % 3; // 0-2ms抖动
            timer->setInterval(baseInterval + jitter);
            
            connect(timer, &QTimer::timeout, [this, port]() {
                if (m_running) {
                    generateHighSpeedData(port);
                }
            });
            
            timer->start();
            m_dataTimers.append(timer);
        }
    }
    
    void generateHighSpeedData(int port)
    {
        // 生成真实大小的DDR2FIBER数据包
        QByteArray message;
        message.resize(10); // 协议头10字节
        
        unsigned char* data = reinterpret_cast<unsigned char*>(message.data());
        data[0] = 0x07;  // DDR2FIBER命令ID
        
        // 模拟不同大小的数据传输 (256B - 4KB)
        static uint16_t dataSizes[] = {256, 512, 1024, 2048, 4096};
        uint16_t dataLength = dataSizes[QRandomGenerator::global()->bounded(5)];
        
        data[1] = dataLength & 0xFF;
        data[2] = (dataLength >> 8) & 0xFF;
        data[3] = port;  // 端口索引
        
        // 4K对齐的DDR地址
        uint32_t baseAddr = 0x20000000 + (port * 0x100000); // 每端口1MB基址
        uint32_t offset = (m_totalMessages.load() % 256) * 0x1000; // 4K对齐偏移
        uint32_t ddrAddr = baseAddr + offset;
        
        data[4] = ddrAddr & 0xFF;
        data[5] = (ddrAddr >> 8) & 0xFF;
        data[6] = (ddrAddr >> 16) & 0xFF;
        data[7] = (ddrAddr >> 24) & 0xFF;
        
        data[8] = dataLength & 0xFF;
        data[9] = (dataLength >> 8) & 0xFF;
        
        // 模拟分布式网络环境
        int deviceGroup = port / 20; // 每20个端口一组设备
        QString strIPHop = QString("192.168.%1.%2:0")
                          .arg(deviceGroup + 1)
                          .arg(100 + (port % 20));
        uint16_t BPUID = 0x2000 + port;
        
        // 发送到DDR处理器
        m_handler->OnRecvDoCustom(strIPHop, BPUID, message);
        
        m_totalMessages.fetch_add(1);
        
        // 模拟网络不稳定 (1%概率短暂停止)
        if (QRandomGenerator::global()->bounded(1000) < 10) {
            QTimer::singleShot(50 + QRandomGenerator::global()->bounded(100), [this, port]() {
                // 网络恢复后的突发数据
                for (int i = 0; i < 3; ++i) {
                    generateHighSpeedData(port);
                }
            });
        }
    }
    
    void startMonitoringTimers()
    {
        // 每5秒生成详细报告
        QTimer* reportTimer = new QTimer(this);
        connect(reportTimer, &QTimer::timeout, this, &DDRStressTest::generatePeriodicReport);
        reportTimer->start(5000);
        
        // 60秒后停止测试
        QTimer::singleShot(60000, this, &DDRStressTest::stopStressTest);
    }
    
    void generateFinalReport()
    {
        qint64 totalElapsed = m_testStartTime.elapsed();
        double elapsedSeconds = totalElapsed / 1000.0;
        int totalMessages = m_totalMessages.load();
        double avgMessageRate = totalMessages / elapsedSeconds;
        double theoreticalMaxRate = 921600.0 * 80 / 8 / 1024; // 理论最大包率
        double efficiency = (avgMessageRate / theoreticalMaxRate) * 100.0;
        
        ALOG_INFO("=== FINAL DDR STRESS TEST REPORT ===", "STRESS", "--");
        ALOG_INFO("Test Duration: %.1f seconds", "STRESS", "--", elapsedSeconds);
        ALOG_INFO("Total Messages: %d", "STRESS", "--", totalMessages);
        ALOG_INFO("Average Rate: %.1f msg/s", "STRESS", "--", avgMessageRate);
        ALOG_INFO("Theoretical Max: %.1f msg/s", "STRESS", "--", theoreticalMaxRate);
        ALOG_INFO("System Efficiency: %.1f%%", "STRESS", "--", efficiency);
        
        if (efficiency > 90.0) {
            ALOG_INFO("RESULT: EXCELLENT - System performing at peak efficiency", "STRESS", "--");
        } else if (efficiency > 75.0) {
            ALOG_INFO("RESULT: GOOD - System performing well under high load", "STRESS", "--");
        } else if (efficiency > 50.0) {
            ALOG_WARN("RESULT: MODERATE - System showing performance degradation", "STRESS", "--");
        } else {
            ALOG_ERROR("RESULT: POOR - System unable to handle target load", "STRESS", "--");
        }
        
        // 最终性能报告
        m_handler->printDDRStats();
        m_handler->printPerformanceReport();
        m_handler->stopPerformanceMonitoring();
    }

private:
    CustomMessageHandler* m_handler;
    QList<QTimer*> m_dataTimers;
    QElapsedTimer m_testStartTime;
    std::atomic<int> m_totalMessages;
    std::atomic<bool> m_running;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    ALOG_INFO("DDR 80-Channel 921600 Baud Stress Test Starting", "MAIN", "--");
    ALOG_INFO("Target: 80 channels × 921600 bps = 73,728,000 bps total", "MAIN", "--");
    ALOG_INFO("Expected Peak: ~9.2MB/s data throughput", "MAIN", "--");
    
    DDRStressTest stressTest;
    
    // 延迟2秒启动，确保系统完全初始化
    QTimer::singleShot(2000, &stressTest, &DDRStressTest::startStressTest);
    
    return app.exec();
}

#include "DDRStressTest.moc"
