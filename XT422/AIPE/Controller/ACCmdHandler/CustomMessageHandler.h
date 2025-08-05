#ifndef CUSTOMMESSAGEHANDLER_H
#define CUSTOMMESSAGEHANDLER_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QSemaphore>
#include <QMutex>
#include <QWaitCondition>
#include <QMap>
#include <QList>
#include <QQueue>
#include <QThread>
#include <QLibrary>
#include <QDateTime>
#include <atomic>        // 高速优化：原子操作支持
#include <memory>        // 高速优化：智能指针支持
#include "ICustomDllHandler.h"  // 引入独立的 ICustomDllHandler 声明
#include "SerialPortManager.h"
#include "SerialPortWorker.h"
#include "../Controller/RemoteServer/JsonRpcServer.h"

// 前向声明
class CustomMessageHandler;

// CustomMessageHandler 单件用于处理 DoCustom 消息，
// 首先根据消息中的路由信息尝试使用本地 DLL 处理器进行处理（工厂模式扩展）；
// 如果本地处理失效，则直接调用全局 JsonRpcServer 单件进行外部处理。

// MT422消息请求-响应对
struct MT422Message {
    QByteArray request;
    QByteArray response;
    bool hasResponse;
    QWaitCondition waitCondition;
};

// MT422响应处理线程
class MT422ResponseProcessor : public QThread
{
    Q_OBJECT
public:
    explicit MT422ResponseProcessor(QObject *parent = nullptr);
    void stop();
    void addResponse(const QString &strIPHop, const uint16_t BPUID, const QByteArray &response);

protected:
    void run() override;

private:
    struct ResponseData {
        QString strIPHop;
        uint16_t BPUID;
        QByteArray response;
    };
    QQueue<ResponseData> m_responseQueue;
    QMutex m_queueMutex;
    QWaitCondition m_queueCondition;
    bool m_running;
};

// DDR异步处理器 - 用于处理0x07协议DDR读取请求
class DDRAsyncProcessor : public QThread
{
    Q_OBJECT
public:
    explicit DDRAsyncProcessor(CustomMessageHandler* messageHandler, QObject *parent = nullptr);
    ~DDRAsyncProcessor();
    void stop();
      // 添加DDR读取任务到队列
    void addDDRTask(const QString &strIPHop, const uint16_t BPUID, 
                    uint8_t portIndex, uint32_t ddrAddress, uint16_t dataLength);
    
    // 配置参数设置函数
    void setRetryCount(int retryCount) { m_retryCount = retryCount; }
    void setBaseDelay(int delayMs) { m_baseDelayMs = delayMs; }
    void setRetryDelay(int delayMs) { m_retryDelayMs = delayMs; }
    void setStatsInterval(int intervalMs) { m_statsIntervalMs = intervalMs; }
    
    // 统计信息接口
    void printStats();
    void resetStats();

protected:
    void run() override;

private:
    // 高速优化：紧凑的任务结构，减少内存占用和缓存未命中
    struct alignas(32) DDRTask { // 32字节对齐，优化缓存行利用
        QString strIPHop;
        uint32_t ddrAddress;
        uint16_t dataLength;
        uint16_t BPUID;
        uint8_t portIndex;
        // 使用更轻量的时间戳
        qint64 createTimestamp; // 毫秒时间戳，比QDateTime更高效
        
        DDRTask() = default;
        DDRTask(const QString& ip, uint16_t bpu, uint8_t port, uint32_t addr, uint16_t len)
            : strIPHop(ip), ddrAddress(addr), dataLength(len), BPUID(bpu), portIndex(port)
            , createTimestamp(QDateTime::currentMSecsSinceEpoch()) {}
    };
    
    // 高速优化：原子化统计结构，减少锁竞争
    struct alignas(64) DDRStats { // 64字节对齐，避免伪共享
        std::atomic<qint64> totalTasks{0};
        std::atomic<qint64> successfulTasks{0};
        std::atomic<qint64> failedTasks{0};
        std::atomic<qint64> timeoutTasks{0};
        std::atomic<qint64> duplicateTasks{0};
        std::atomic<qint64> droppedTasks{0};
        qint64 lastResetTime;
        
        DDRStats() : lastResetTime(QDateTime::currentMSecsSinceEpoch()) {}
    };    
    // 高速优化：使用更高效的容器和同步原语
    QQueue<DDRTask> m_taskQueue;
    mutable QMutex m_queueMutex;     // mutable for const methods
    QWaitCondition m_queueCondition;
    std::atomic<bool> m_running{false}; // 原子布尔值，减少锁开销
    DDRStats m_stats;    // 极致性能优化：专为921600波特率×80通道超高速数据流设计
    static constexpr int MAX_DDR_QUEUE_SIZE = 200000;   // 进一步增大队列：200K任务缓冲
    static constexpr int DDR_TASK_TIMEOUT_MS = 1000;    // 极短超时：1秒立即失败
    static constexpr int BATCH_PROCESS_SIZE = 2000;     // 超大批处理：2000任务/批次
    static constexpr int ALIGNMENT_MASK = 4095;         // 4K对齐掩码 (4096-1)
    
    // 极致参数优化 - 无延迟、无重试的极速模式
    int m_retryCount = 0;           // 零重试：一次失败立即放弃
    int m_baseDelayMs = 0;          // 零延迟：无等待时间
    int m_retryDelayMs = 0;         // 零重试延迟
    int m_statsIntervalMs = 300000;  // 统计间隔：5分钟 (减少日志输出)
    
    // 指向CustomMessageHandler的指针，用于调用ProcessLocalMessage
    CustomMessageHandler* m_messageHandler;
    
    // 处理单个DDR任务
    void processDDRTask(const DDRTask &task);
    // 清理超时任务
    void cleanupTimeoutTasks();
};

class CustomMessageHandler : public QObject
{
    Q_OBJECT
public:
    // 获取全局单件实例；可传入 parent 用于生命周期管理（仅在首次调用时生效）
    static CustomMessageHandler* instance(QObject *parent = nullptr);

    // 注册本地处理器，handlerId 用于路由消息到不同的 DLL 模块
    void RegisterLocalHandler(const QString &handlerId, ICustomDllHandler* pHandler);
    ICustomDllHandler* UnregisterLocalHandler(const QString &handlerId);

    // 自动扫描指定目录下的 DLL（文件扩展名根据平台，建议 DLL 文件命名与 solution 名称一致）
    // 如 solution 参数不为空，则只加载该 solution 对应的 DLL，否则加载目录下所有 DLL。
    void AutoRegisterDllHandlers(const QString &directoryPath, const QString &solution = QString());    // 使用工厂模式注册内置的处理器
    bool RegisterBuiltinHandler(const QString &solutionName);    // DDR异步处理器配置接口
    void configureDDRProcessor(int retryCount = 3, int baseDelayMs = 50, 
                              int retryDelayMs = 100, int statsIntervalMs = 60000);
    void printDDRStats();
    void resetDDRStats();
    
    
    // 尝试使用本地 DLL 处理器处理消息，如处理成功则返回 true
    bool ProcessLocalMessage(const QString &strIPHop, const uint16_t BPUID,const uint8_t comNum, const QByteArray &message);

public slots:
    // 接收 DoCustom 消息，参数顺序：strIPHop、BPUID、消息内容
    void OnRecvDoCustom(const QString &strIPHop, const uint16_t BPUID, const QByteArray &message);
    // MT422串口响应处理函数：strIPHop、BPUID、响应内容
    void OnMT422Response(const QString &strIPHop, const uint16_t BPUID, quint8 index, const QByteArray &response);
signals:
    // 将原始设备消息转发到JsonRpcServer进行封装和发送
    void sgnForwardDeviceMessage(const QString& strIPHop, uint16_t BPUID, const QByteArray& message);
private:
    explicit CustomMessageHandler(QObject *parent = nullptr);
    ~CustomMessageHandler();    // 禁止拷贝构造和赋值
    Q_DISABLE_COPY(CustomMessageHandler)

    // 本地处理失败时，交由 JsonRpcServer 单件进行外部处理
    bool ProcessExternalMessage(const QString &strIPHop, const uint16_t BPUID, const QByteArray &message);

    bool sendMessageToSerialPort(const QString& portName, const QByteArray& message);

    void testJsonServer(JsonRpcServer* pServer);

    // 创建内置处理器的工厂方法
    ICustomDllHandler* CreateBuiltinHandler(const QString &solutionName);
public:
    // 新增：MT422消息队列管理
    void PushMT422Request(const QByteArray &request);
    bool WaitForMT422Response(QByteArray &response, int timeout = 5000);
private:
    QMutex m_HandlerMutex;                   // 保护本地处理器集合的互斥锁
    QMap<QString, ICustomDllHandler*> m_LocalHandlers; // 存放本地处理器集合，以处理器标识为 key

    // 保存加载的 QLibrary 指针，防止 DLL 被卸载
    QList<QLibrary*> m_loadedLibraries;

    static CustomMessageHandler* m_instance;
    static QMutex m_instanceMutex;

    // 新增：MT422消息队列相关成员
    QQueue<MT422Message*> m_mt422MessageQueue;
    QMutex m_mt422QueueMutex;
    static const int MAX_QUEUE_SIZE = 100;
    int testComNum = 0;
    uint64_t test = 0;    // 新增：响应处理线程
    MT422ResponseProcessor* m_responseProcessor;
    // 新增：DDR异步处理线程
    DDRAsyncProcessor* m_ddrProcessor;
    SerialPortManager m_serialPortManager; // 串口管理器
};

#endif // CUSTOMMESSAGEHANDLER_H