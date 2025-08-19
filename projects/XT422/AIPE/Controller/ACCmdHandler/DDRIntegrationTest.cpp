// DDR性能监控集成测试
// 用于验证DDRPerformanceMonitor与CustomMessageHandler集成效果

#include "CustomMessageHandler.h"
#include "DDRPerformanceMonitor.h"
#include "AngkLogger.h"
#include <QApplication>
#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include <QThread>

class DDRIntegrationTester : public QObject
{
    Q_OBJECT

public:
    DDRIntegrationTester(QObject* parent = nullptr) : QObject(parent)
    {
        m_handler = CustomMessageHandler::instance();
        setupTest();
    }

private slots:
    void runPerformanceTest()
    {
        ALOG_INFO("Starting DDR Integration Performance Test", "TEST", "--");
        
        // 配置DDR处理器为高性能模式
        m_handler->configureDDRProcessor(
            2,     // 重试次数：2次 (快速失败)
            5,     // 基础延迟：5ms
            10,    // 重试延迟：10ms
            5000   // 统计间隔：5秒
        );
        
        // 启动性能监控（1秒间隔，密集监控）
        m_handler->startPerformanceMonitoring(1000);
        
        // 模拟80路高速数据处理
        simulateHighSpeedDataProcessing();
        
        // 启动定期报告定时器
        QTimer::singleShot(10000, this, &DDRIntegrationTester::printFirstReport);
        QTimer::singleShot(20000, this, &DDRIntegrationTester::printSecondReport);
        QTimer::singleShot(30000, this, &DDRIntegrationTester::stopTest);
    }
    
    void printFirstReport()
    {
        ALOG_INFO("=== DDR Performance Report (10s mark) ===", "TEST", "--");
        m_handler->printDDRStats();
        m_handler->printPerformanceReport();
    }
    
    void printSecondReport()
    {
        ALOG_INFO("=== DDR Performance Report (20s mark) ===", "TEST", "--");
        m_handler->printDDRStats();
        m_handler->printPerformanceReport();
    }
    
    void stopTest()
    {
        ALOG_INFO("=== Final DDR Performance Report ===", "TEST", "--");
        m_handler->printDDRStats();
        m_handler->printPerformanceReport();
        
        m_handler->stopPerformanceMonitoring();
        ALOG_INFO("DDR Integration Test Completed", "TEST", "--");
        
        QApplication::quit();
    }

private:
    void setupTest()
    {
        // 设置性能监控告警阈值
        if (m_handler->getPerformanceMonitor()) {
            m_handler->getPerformanceMonitor()->setLatencyThreshold(50);  // 50ms延迟告警
            m_handler->getPerformanceMonitor()->setThroughputThreshold(115200); // 115.2KB/s吞吐量
            m_handler->getPerformanceMonitor()->setErrorRateThreshold(3.0);     // 3%错误率告警
            
            // 连接告警信号
            connect(m_handler->getPerformanceMonitor(), &DDRPerformanceMonitor::highLatencyDetected,
                    this, &DDRIntegrationTester::onHighLatencyDetected);
            connect(m_handler->getPerformanceMonitor(), &DDRPerformanceMonitor::lowThroughputDetected,
                    this, &DDRIntegrationTester::onLowThroughputDetected);
            connect(m_handler->getPerformanceMonitor(), &DDRPerformanceMonitor::highErrorRateDetected,
                    this, &DDRIntegrationTester::onHighErrorRateDetected);
        }
    }
    
    void simulateHighSpeedDataProcessing()
    {
        ALOG_INFO("Simulating 80-channel 921600 baud high-speed data processing", "TEST", "--");
        
        // 模拟80路并发数据处理
        for (int port = 0; port < 80; ++port) {
            QTimer* timer = new QTimer(this);
            timer->setInterval(10 + (port % 5)); // 10-14ms间隔，模拟不同端口速度
            
            connect(timer, &QTimer::timeout, [this, port]() {
                simulatePortDataProcessing(port);
            });
            
            timer->start();
            m_portTimers.append(timer);
        }
    }
    
    void simulatePortDataProcessing(int port)
    {
        // 模拟DDR2FIBER协议数据包
        QByteArray message;
        message.resize(10); // AG06 DDR2FIBER协议：10字节
        
        unsigned char* data = reinterpret_cast<unsigned char*>(message.data());
        data[0] = 0x07;  // 命令ID
        data[1] = 0x00;  // 数据长度低位
        data[2] = 0x04;  // 数据长度高位 (1024字节)
        data[3] = port;  // 端口索引
        
        // DDR地址 (模拟4K对齐地址)
        uint32_t ddrAddr = 0x10000000 + (port * 0x1000) + (m_messageCounter % 1000) * 0x1000;
        data[4] = ddrAddr & 0xFF;
        data[5] = (ddrAddr >> 8) & 0xFF;
        data[6] = (ddrAddr >> 16) & 0xFF;
        data[7] = (ddrAddr >> 24) & 0xFF;
        
        // 数据长度 (1024字节)
        data[8] = 0x00;
        data[9] = 0x04;
        
        // 模拟网络传输
        QString strIPHop = QString("192.168.1.%1:0").arg(100 + port % 20);
        uint16_t BPUID = 0x1000 + port;
        
        // 发送到DDR处理器
        m_handler->OnRecvDoCustom(strIPHop, BPUID, message);
        
        m_messageCounter++;
        
        // 每1000个消息停止一些端口模拟网络波动
        if (m_messageCounter % 1000 == 0 && port % 10 == 0) {
            if (m_portTimers[port]->isActive()) {
                m_portTimers[port]->stop();
                QTimer::singleShot(100, m_portTimers[port], [this, port]() {
                    m_portTimers[port]->start();
                });
            }
        }
    }

private slots:
    void onHighLatencyDetected(uint8_t portIndex, qint64 latencyMs)
    {
        ALOG_WARN("HIGH LATENCY ALERT: Port %d, Latency: %lld ms", "TEST", "--", portIndex, latencyMs);
    }
    
    void onLowThroughputDetected(uint8_t portIndex, qint64 currentThroughput)
    {
        ALOG_WARN("LOW THROUGHPUT ALERT: Port %d, Throughput: %lld bytes/s", "TEST", "--", portIndex, currentThroughput);
    }
    
    void onHighErrorRateDetected(uint8_t portIndex, double errorRate)
    {
        ALOG_WARN("HIGH ERROR RATE ALERT: Port %d, Error Rate: %.2f%%", "TEST", "--", portIndex, errorRate);
    }

private:
    CustomMessageHandler* m_handler;
    QList<QTimer*> m_portTimers;
    std::atomic<int> m_messageCounter{0};
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    ALOG_INFO("Starting DDR Performance Integration Test", "MAIN", "--");
    
    DDRIntegrationTester tester;
    
    // 延迟启动测试，确保系统初始化完成
    QTimer::singleShot(1000, &tester, &DDRIntegrationTester::runPerformanceTest);
    
    return app.exec();
}

#include "DDRIntegrationTest.moc"
