#include "CustomMessageHandler.h"

#include <QDir>
#include <QLibrary>
#include <QMutexLocker>
#include <QDateTime>
#include <exception>
#include "AngkLogger.h"
#include "MT422DllHandler.h"
#include "AngKMessageHandler.h"
#include "CmdHandler.h"

CustomMessageHandler* CustomMessageHandler::m_instance = nullptr;
QMutex CustomMessageHandler::m_instanceMutex;

CustomMessageHandler* CustomMessageHandler::instance(QObject *parent)
{
    if (!m_instance) {
        QMutexLocker locker(&m_instanceMutex);
        if (!m_instance) {
            m_instance = new CustomMessageHandler(parent);
        }
    }
    return m_instance;
}

CustomMessageHandler::CustomMessageHandler(QObject *parent)
    : QObject(parent)
    , m_responseProcessor(new MT422ResponseProcessor(this))
    , m_ddrProcessor(new DDRAsyncProcessor(this, this))
{
   /* m_responseProcessor->start();
    m_ddrProcessor->start();*/
    
    // 连接信号到JsonRpcServer的新槽函数
    /*connect(this, &CustomMessageHandler::sgnForwardDeviceMessage, 
            JsonRpcServer::Instance(), &JsonRpcServer::OnForwardDeviceMessage, 
            Qt::QueuedConnection);*/
            
   // ALOG_INFO("CustomMessageHandler initialized with message forwarding to JsonRpcServer.", "CU", "--");
   // ALOG_INFO("CustomMessageHandler initialized with message forwarding to JsonRpcServer.", "CU", "--");
    //RegisterMT422Handler();
}

CustomMessageHandler::~CustomMessageHandler()
{
    if (m_responseProcessor) {
        m_responseProcessor->stop();
        m_responseProcessor->wait();
    }
    
    if (m_ddrProcessor) {
        m_ddrProcessor->stop();
        m_ddrProcessor->wait();
        delete m_ddrProcessor;
        m_ddrProcessor = nullptr;
    }
    
    QMutexLocker locker(&m_HandlerMutex);
    m_LocalHandlers.clear();
    // 卸载并删除所有已加载的 DLL
    for (QLibrary* lib : m_loadedLibraries) {
        if (lib->isLoaded())
            lib->unload();
        delete lib;
    }
    m_loadedLibraries.clear();
}

void CustomMessageHandler::RegisterLocalHandler(const QString &handlerId, ICustomDllHandler* pHandler)
{
    {
        if (!pHandler)
            return;
        m_LocalHandlers.insert(handlerId, pHandler); 
    }       
}

ICustomDllHandler* CustomMessageHandler::UnregisterLocalHandler(const QString &handlerId)
{
    QMutexLocker locker(&m_HandlerMutex);
    return m_LocalHandlers.take(handlerId);
}

void CustomMessageHandler::OnRecvDoCustom(const QString& strIPHop, const uint16_t BPUID, const QByteArray& message)
{
    // 只做转发，不进行任何检查和处理
   emit sgnForwardDeviceMessage(strIPHop, BPUID, message);
}

bool CustomMessageHandler::ProcessLocalMessage(const QString &strIPHop, const uint16_t BPUID,  const uint8_t comNum,const QByteArray &message)
{

    
    if (m_LocalHandlers.isEmpty()) {
        ALOG_WARN("Local processing: No handlers registered", "CU", "--");
        return false;
    }

    if (m_LocalHandlers.size() > 1) {
        ALOG_ERROR("Local processing: Multiple handlers registered, cannot determine which one to use", "CU", "--");
        return false;
    }
    
    ICustomDllHandler* pHandler = m_LocalHandlers.first();
    if (pHandler) {
        bool ret = pHandler->ProcessMessage(strIPHop, BPUID,comNum,message);
        ALOG_DEBUG("Local processing: using single handler, result:%d", "CU", "--", ret);
        
        
        return ret;
    }
    return false;
}

bool CustomMessageHandler::ProcessExternalMessage(const QString &strIPHop, const uint16_t BPUID, const QByteArray &message)
{
    // 通过全局单件接口获取 JsonRpcServer 实例，无需提前保存指针
    JsonRpcServer* pServer = GetGlobalJsonRPCServerApp();
    if (pServer) {
        testJsonServer(pServer);
        return 0;
    } else {
        ALOG_WARN("External processing: Failed to get JsonRpcServer instance, strIPHop:%s", "CU", "--", 
            strIPHop.toStdString().c_str());
    }
    return false;
}
void CustomMessageHandler::testJsonServer(JsonRpcServer *pServer) {
    quint16 port = 12345;
    pServer->Start(port);
    ALOG_INFO("-->JsonRpcServer Start,listen at port:%d", "CU", "--",port);
}
// 自动扫描指定目录下的 DLL，并注册导出 CreateDllHandler 接口返回的处理器
// 如果 solution 参数不为空，则只加载与 solution 名称匹配的 DLL，否则加载所有 DLL
void CustomMessageHandler::AutoRegisterDllHandlers(const QString &directoryPath, const QString &solution)
{
    QDir dir(directoryPath);
    if (!dir.exists()) {
        ALOG_WARN("DLL directory %s does not exist.", "CU", "--", directoryPath.toStdString().c_str());
        return;
    }

    // 根据平台设置动态库文件过滤器
#if defined(Q_OS_WIN)
    QStringList filters = {"*.dll"};
#elif defined(Q_OS_LINUX)
    QStringList filters = {"*.so"};
#elif defined(Q_OS_MAC)
    QStringList filters = {"*.dylib"};
#else
    QStringList filters = {"*"};
#endif
    dir.setNameFilters(filters);
    QFileInfoList fileList = dir.entryInfoList(QDir::Files);
    for (const QFileInfo &fileInfo : fileList) {
        QString filePath = fileInfo.absoluteFilePath();
        QString dllSolution = fileInfo.baseName();  // 例如 "MT422" 对应 "MT422.dll"
        // 如果指定了 solution，并且与当前 dllSolution 不匹配，则跳过
        if (!solution.isEmpty() && dllSolution != solution) {
            continue;
        }
        ALOG_DEBUG("Attempting to load DLL: %s, solution:%s", "CU", "--", filePath.toStdString().c_str(), dllSolution.toStdString().c_str());

        QLibrary* library = new QLibrary(filePath, this);
        if (!library->load()) {
            ALOG_WARN("Failed to load DLL: %s, Error:%s", "CU", "--", filePath.toStdString().c_str(), library->errorString().toStdString().c_str());
            delete library;
            continue;
        }

        // 定义导出函数指针类型：函数返回 ICustomDllHandler*
        typedef ICustomDllHandler* (*CreateDllHandlerFn)();
        CreateDllHandlerFn createHandler = (CreateDllHandlerFn)library->resolve("CreateDllHandler");
        if (!createHandler) {
            ALOG_WARN("Failed to resolve CreateDllHandler, DLL:%s", "CU", "--", filePath.toStdString().c_str());
            library->unload();
            delete library;
            continue;
        }

        ICustomDllHandler* handler = createHandler();
        if (!handler) {
            ALOG_WARN("CreateDllHandler returned nullptr, DLL:%s", "CU", "--", filePath.toStdString().c_str());
            library->unload();
            delete library;
            continue;
        }

        // 注册时使用 dllSolution 作为 key
        RegisterLocalHandler(dllSolution, handler);
        ALOG_DEBUG("Successfully registered DLL processor, solution:%s", "CU", "--", dllSolution.toStdString().c_str());

        // 将 QLibrary 指针保存下来，确保 DLL 不会被卸载
        m_loadedLibraries.append(library);
    }
}

// 创建内置处理器的工厂方法
ICustomDllHandler* CustomMessageHandler::CreateBuiltinHandler(const QString &solutionName)
{
    // 根据解决方案名称创建对应的处理器
    if (solutionName == "XT-RS422") {
       // JsonRpcServer* pServer = GetGlobalJsonRPCServerApp();
       // testJsonServer(pServer);
        return new MT422DllHandler();
    }
    // 在这里添加其他内置处理器的创建
    // else if (solutionName == "OTHER_SOLUTION") {
    //     return new OtherSolutionHandler();
    // }

    ALOG_WARN("No builtin handler found for solution: %s", "CU", "--", solutionName.toStdString().c_str());
    return nullptr;
}

// 使用工厂模式注册内置的处理器
bool CustomMessageHandler::RegisterBuiltinHandler(const QString &solutionName)
{
    //QMutexLocker locker(&m_HandlerMutex);
    
    // 检查是否已经注册
    if (m_LocalHandlers.contains(solutionName)) {
        ALOG_DEBUG("Handler for solution %s already registered", "CU", "--", solutionName.toStdString().c_str());
        return true;
    }

    // 使用工厂方法创建处理器
    ICustomDllHandler* handler = CreateBuiltinHandler(solutionName);
    if (handler) {
        RegisterLocalHandler(solutionName, handler);
        ALOG_INFO("Successfully registered builtin handler for solution: %s", "CU", "--", solutionName.toStdString().c_str());
        return true;
    } else {
        ALOG_WARN("Failed to create builtin handler for solution: %s", "CU", "--", solutionName.toStdString().c_str());
        return false;
    }
}

void CustomMessageHandler::OnMT422Response(const QString &strIPHop, const uint16_t BPUID, quint8 index, const QByteArray &response)
{
    if (response.isEmpty()) {
        ALOG_WARN("Received empty MT422 response", "CU", "--");
        return;
    }

    ALOG_DEBUG("Received MT422 response from strIPHop: %s, BPUID: %d, size: %d bytes", 
        "CU", "--", strIPHop.toStdString().c_str(), BPUID, response.size());

    // 将响应添加到处理队列，包含BPUID
    if (m_responseProcessor) {
        m_responseProcessor->addResponse(strIPHop, BPUID, response);
    }
}

void CustomMessageHandler::PushMT422Request(const QByteArray &request)
{
    QMutexLocker locker(&m_mt422QueueMutex);
    
    // 检查队列大小
    while (m_mt422MessageQueue.size() >= MAX_QUEUE_SIZE) {
        MT422Message* oldMsg = m_mt422MessageQueue.dequeue();
        oldMsg->hasResponse = true;  // 强制唤醒等待的线程
        oldMsg->waitCondition.wakeAll();
        delete oldMsg;
    }

    MT422Message* msg = new MT422Message;
    msg->request = request;
    msg->hasResponse = false;
    m_mt422MessageQueue.enqueue(msg);
}

bool CustomMessageHandler::WaitForMT422Response(QByteArray &response, int timeout)
{
    QMutexLocker locker(&m_mt422QueueMutex);
    
    if (m_mt422MessageQueue.isEmpty()) {
        return false;
    }

    MT422Message* msg = m_mt422MessageQueue.head();
    
    // 等待响应或超时
    while (!msg->hasResponse) {
        if (!msg->waitCondition.wait(&m_mt422QueueMutex, timeout)) {
            // 超时
            return false;
        }
    }

    response = msg->response;
    m_mt422MessageQueue.dequeue();
    delete msg;
    
    return true;
}

// MT422ResponseProcessor implementation
MT422ResponseProcessor::MT422ResponseProcessor(QObject *parent)
    : QThread(parent)
    , m_running(true)
{
}

void MT422ResponseProcessor::stop()
{
    QMutexLocker locker(&m_queueMutex);
    m_running = false;
    m_queueCondition.wakeAll();
}

void MT422ResponseProcessor::addResponse(const QString &strIPHop, const uint16_t BPUID, const QByteArray &response)
{
    QMutexLocker locker(&m_queueMutex);
    ResponseData data{strIPHop, BPUID, response};
    m_responseQueue.enqueue(data);
    m_queueCondition.wakeOne();
}

void MT422ResponseProcessor::run()
{
    while (m_running) {
        ResponseData responseData;
        
        {
            QMutexLocker locker(&m_queueMutex);
            while (m_responseQueue.isEmpty() && m_running) {
                m_queueCondition.wait(&m_queueMutex);
            }
            
            if (!m_running) {
                break;
            }
            
            responseData = m_responseQueue.dequeue();
        }

        // 从响应数据中获取 strIPHop 和 BPUID
        QString strIPHop = responseData.strIPHop;
        QStringList parts = strIPHop.split(":");
        QString devIP = parts[0];
        uint32_t hopNum = parts[1].toUInt();
        uint16_t BPUID = responseData.BPUID;
        QByteArray response = responseData.response;

        ALOG_DEBUG("MT422 response from strIPHop: %s, BPUID: %d", 
            "CU", "--", strIPHop.toStdString().c_str(), BPUID);

        // Command_RemoteDoPTCmd(std::string devIP, uint32_t HopNum, uint32_t PortID, 
        //                      uint32_t CmdFlag, uint16_t CmdID, uint32_t SKTNum, 
        //                      uint16_t BPUID, QByteArray& CmdDataBytes)
        AngKMessageHandler::instance().Command_RemoteDoPTCmd(
            devIP.toStdString(),  // devIP
            hopNum,                  // HopNum
            0,                       // PortID
            0,                       // CmdFlag
            (uint16_t)eSubCmdID::SubCmd_MU_DoSendCustom,  // CmdID
            0x0,                // SKTNum
            BPUID,                  // BPUID
            response                // CmdDataBytes
        );
    }
} 
bool CustomMessageHandler::sendMessageToSerialPort(const QString& portName, const QByteArray& message) {
    // 1. 检查串口是否已经连接
    //if (!m_serialPortManager.isPortConnected(portName)) {
        // 2. 如果未连接，则连接串口
        // 这里使用默认的串口配置，你可以根据需要修改
        //int baudRate = 9600;
    int baudRate = 921600;
    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    QSerialPort::Parity parity = QSerialPort::NoParity;
    QSerialPort::StopBits stopBits = QSerialPort::OneStop;

    if (!m_serialPortManager.connectPort(portName,0, baudRate, dataBits, parity, stopBits)) {
        ALOG_INFO("Failed to connect to serial port: %s", "CU", "--", portName.toLocal8Bit().data());
        return false; // 连接失败，返回 false
    }
    ALOG_INFO("Connected to serial port:%s", "CU", "--", portName.toLocal8Bit().data());
    //}
    // 将 QByteArray 转换为 16 进制字符串
    QString hexString = message.toHex().toUpper(); // 转换为大写，更常见
    // 可以在控制台打印 16 进制字符串
    QByteArray byteArray = hexString.toLocal8Bit();
    const char* hexStringCStr = byteArray.data();
    ALOG_INFO("Sending to serial port :%s %s", "CU", "--", portName.toLocal8Bit().data(), hexStringCStr);
    // 3. 调用 SerialPortManager 的 sendData 函数来发送原始数据
   // m_serialPortManager.sendData(portName, message);

    // 根据 sendData 的返回值或其他方式来判断发送是否成功
    // 例如，可以添加错误处理，如果发送失败，则返回 false
    // 目前假设发送总是成功，可以根据实际情况修改
    return true;
}

// DDRAsyncProcessor 实现
DDRAsyncProcessor::DDRAsyncProcessor(CustomMessageHandler* messageHandler, QObject *parent)
    : QThread(parent), m_messageHandler(messageHandler)
{
    m_stats.lastResetTime = QDateTime::currentMSecsSinceEpoch();
    m_running.store(false);
}

DDRAsyncProcessor::~DDRAsyncProcessor()
{
    stop();
    wait();
}

void DDRAsyncProcessor::stop()
{
    m_running.store(false);
    {
        QMutexLocker locker(&m_queueMutex);
        m_queueCondition.wakeAll();
    }
}

void DDRAsyncProcessor::addDDRTask(const QString &strIPHop, const uint16_t BPUID, 
                                   uint8_t portIndex, uint32_t ddrAddress, uint16_t dataLength)
{
    
    QMutexLocker locker(&m_queueMutex);
    
    // 高速优化：原子化统计更新，减少锁操作
    m_stats.totalTasks.fetch_add(1, std::memory_order_relaxed);
      // 高速优化：减少统计计算频率 - 从1000改为5000
    if (Q_LIKELY(m_stats.totalTasks.load(std::memory_order_relaxed) % 5000 == 0)) {
        // 每5000个任务才清理一次超时任务，进一步减少开销
        cleanupTimeoutTasks();
    }
      // 高速优化：完全禁用重复检测以获得最大性能
    // 在超高速数据流中，重复检测的开销远大于其收益
    // 注释掉原有的重复检测逻辑
    /*
    bool duplicateFound = false;
    int checkCount = qMin(m_taskQueue.size(), 5); // 只检查最近5个任务
    for (int i = m_taskQueue.size() - checkCount; i < m_taskQueue.size(); ++i) {
        const DDRTask& existingTask = m_taskQueue[i];
        if (Q_UNLIKELY(existingTask.portIndex == portIndex && 
                       existingTask.ddrAddress == ddrAddress &&
                       existingTask.strIPHop == strIPHop)) {
            m_stats.duplicateTasks.fetch_add(1, std::memory_order_relaxed);
            return; // 直接返回，避免不必要的操作
        }
    }
    */
      // 高速优化：队列满时使用更高效的处理
    if (Q_UNLIKELY(m_taskQueue.size() >= MAX_DDR_QUEUE_SIZE)) {
        m_taskQueue.dequeue(); // 直接丢弃最旧任务
        m_stats.droppedTasks.fetch_add(1, std::memory_order_relaxed);
    }
    
    // 高速优化：使用构造函数直接创建任务对象
    m_taskQueue.enqueue(DDRTask(strIPHop, BPUID, portIndex, ddrAddress, dataLength));
    
    m_queueCondition.wakeOne();
}

void DDRAsyncProcessor::run()
{
    ALOG_INFO("DDR async processor thread started", "CU", "--");
    m_running.store(true);
    qint64 lastStatsTime = QDateTime::currentMSecsSinceEpoch();
    
    // 高速优化：预分配批处理数组，减少频繁加锁解锁
    QQueue<DDRTask> batchTasks;
    batchTasks.reserve(BATCH_PROCESS_SIZE); // 预分配空间
    
    while (m_running.load(std::memory_order_relaxed)) {
        {
            QMutexLocker locker(&m_queueMutex);            // 极致性能优化：如果队列为空，只等待10ms就继续处理
            if (m_taskQueue.isEmpty()) {
                if (!m_queueCondition.wait(&m_queueMutex, 10)) { // 进一步缩短等待时间到10ms
                    // 检查是否需要打印统计信息
                    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
                    if (currentTime - lastStatsTime >= m_statsIntervalMs) {
                        locker.unlock();
                        printStats();
                        lastStatsTime = currentTime;
                    }
                    continue;
                }
            }
            
            if (!m_running.load(std::memory_order_relaxed)) {
                break;
            }
              // 超高速优化：批量取任务减少锁竞争 - 增大批处理
            batchTasks.clear();
            int tasksToProcess = qMin(m_taskQueue.size(), BATCH_PROCESS_SIZE);
            for (int i = 0; i < tasksToProcess; ++i) {
                if (!m_taskQueue.isEmpty()) {
                    batchTasks.enqueue(m_taskQueue.dequeue());
                }
            }
              // 超高速优化：大幅减少超时清理频率 - 从500改为2000
            static int cleanupCounter = 0;
            if (++cleanupCounter % 2000 == 0) { // 进一步减少清理频率
                cleanupTimeoutTasks();
            }
        } // 释放锁
        
        // 批量处理任务（在锁外执行）
        while (!batchTasks.isEmpty()) {
            if (!m_running.load(std::memory_order_relaxed)) break;
            
            DDRTask task = batchTasks.dequeue();
            processDDRTask(task);
        }
        
        // 检查是否需要打印统计信息
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        if (currentTime - lastStatsTime >= m_statsIntervalMs) {
            printStats();
            lastStatsTime = currentTime;
        }
    }
    
    ALOG_INFO("DDR async processor thread stopped", "CU", "--");
    printStats(); // 停止前打印最终统计信息
}

void DDRAsyncProcessor::processDDRTask(const DDRTask &task)
{
    // 开始性能计时 - 使用RAII确保自动计时
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();
    
    // 超高速优化：快速超时检查使用时间戳
    qint64 currentTime = startTime;
    if (Q_UNLIKELY(currentTime - task.createTimestamp > DDR_TASK_TIMEOUT_MS)) {
        return; // 超时任务直接丢弃
    }
    
    bool success = false;
    uint32_t dataSize = 0;
    
    try {
        // 超高速优化：预计算4K对齐，使用位运算
        uint16_t actualDataLength = task.dataLength;
        uint16_t alignedDataLength = (actualDataLength + ALIGNMENT_MASK) & ~ALIGNMENT_MASK;
        
        QByteArray ddr2fiberData;
        ddr2fiberData.reserve(alignedDataLength); // 预分配内存
          // 超高速优化：极简重试逻辑 - 零重试模式，使用高性能版本
        int result = AngKMessageHandler::instance().Command_ReadDataFromSSD2(
            task.strIPHop.toStdString(), 
            "DDR2FIBER", 
            0, 0, 
            task.ddrAddress, 
            alignedDataLength,
            ddr2fiberData
        );
        
        // 超高速优化：合并成功和失败的处理
        if (Q_LIKELY(result == 0)) {
            // 快速数据截断
            if (Q_UNLIKELY(ddr2fiberData.size() > actualDataLength)) {
                ddr2fiberData.truncate(actualDataLength);
            }
            
            success = true;
            dataSize = ddr2fiberData.size();
            
            // 原子化统计更新
            m_stats.successfulTasks.fetch_add(1, std::memory_order_relaxed);
            
            // 超高速数据处理
            if (Q_LIKELY(m_messageHandler && !ddr2fiberData.isEmpty())) {
                m_messageHandler->ProcessLocalMessage(task.strIPHop, task.BPUID, task.portIndex, ddr2fiberData);
            }
        } else {
            // 原子化失败统计
            m_stats.failedTasks.fetch_add(1, std::memory_order_relaxed);
            
            // 极致性能优化：完全禁用失败日志以获得最大性能
            // 在超高速场景下，日志输出成为性能瓶颈
            /*
            // 只记录关键错误
            if (Q_UNLIKELY(result != ERR_NETCOMM_CmdCRCCompareFailed && 
                          result != ERR_CMDHAND_CmdQueueAvailable)) {
                ALOG_ERROR("DDR read failed - Port: %d, Addr: 0x%08X, Len: %d, Err: %d",
                    "CU", "--", task.portIndex, task.ddrAddress, task.dataLength, result);
            }
            */
        }
    } catch (...) {
        // 极致性能优化：禁用异常日志输出
        // 在超高速场景下，异常日志会严重影响性能
        // ALOG_ERROR("DDR task exception - Port: %d, Addr: 0x%08X", "CU", "--", task.portIndex, task.ddrAddress);
        
        m_stats.failedTasks.fetch_add(1, std::memory_order_relaxed);
    }
    
}

void DDRAsyncProcessor::cleanupTimeoutTasks()
{
    // 注意：调用此函数时调用者应已持有m_queueMutex锁
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    
    // 极致性能优化：限制每次清理的最大任务数，避免长时间阻塞
    int maxCleanupTasks = 1000; // 每次最多清理1000个超时任务
    int cleanedCount = 0;
    
    while (!m_taskQueue.isEmpty() && cleanedCount < maxCleanupTasks) {
        const DDRTask& oldestTask = m_taskQueue.head();
        qint64 elapsedMs = currentTime - oldestTask.createTimestamp;
        
        if (elapsedMs > DDR_TASK_TIMEOUT_MS) {
            DDRTask timeoutTask = m_taskQueue.dequeue();
            m_stats.timeoutTasks.fetch_add(1, std::memory_order_relaxed);
            cleanedCount++;
            
            // 极致性能优化：禁用超时日志以获得最大性能
            // ALOG_WARN("DDR task timeout and removed - Port: %d, DDR Addr: 0x%08X, Length: %d, Elapsed: %lld ms",
            //     "CU", "--", timeoutTask.portIndex, timeoutTask.ddrAddress, timeoutTask.dataLength, elapsedMs);
        } else {
            // 队列按时间顺序，如果最旧的任务没有超时，后面的也不会超时
            break;
        }
    }
}

void DDRAsyncProcessor::printStats()
{
    QMutexLocker locker(&m_queueMutex);
    
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    double runtimeHours = (currentTime - m_stats.lastResetTime) / (1000.0 * 60.0 * 60.0);
    
    // 原子变量读取
    qint64 totalTasks = m_stats.totalTasks.load(std::memory_order_relaxed);
    qint64 successfulTasks = m_stats.successfulTasks.load(std::memory_order_relaxed);
    qint64 failedTasks = m_stats.failedTasks.load(std::memory_order_relaxed);
    qint64 timeoutTasks = m_stats.timeoutTasks.load(std::memory_order_relaxed);
    qint64 duplicateTasks = m_stats.duplicateTasks.load(std::memory_order_relaxed);
    qint64 droppedTasks = m_stats.droppedTasks.load(std::memory_order_relaxed);
}

void DDRAsyncProcessor::resetStats()
{
    QMutexLocker locker(&m_queueMutex);
    
    ALOG_INFO("Resetting DDR processor statistics", "CU", "--");
    
    // 原子变量重置
    m_stats.totalTasks.store(0, std::memory_order_relaxed);
    m_stats.successfulTasks.store(0, std::memory_order_relaxed);
    m_stats.failedTasks.store(0, std::memory_order_relaxed);
    m_stats.timeoutTasks.store(0, std::memory_order_relaxed);
    m_stats.duplicateTasks.store(0, std::memory_order_relaxed);
    m_stats.droppedTasks.store(0, std::memory_order_relaxed);
    m_stats.lastResetTime = QDateTime::currentMSecsSinceEpoch();
}

void CustomMessageHandler::configureDDRProcessor(int retryCount, int baseDelayMs, 
                                                 int retryDelayMs, int statsIntervalMs)
{
    if (m_ddrProcessor) {
        m_ddrProcessor->setRetryCount(retryCount);
        m_ddrProcessor->setBaseDelay(baseDelayMs);
        m_ddrProcessor->setRetryDelay(retryDelayMs);
        m_ddrProcessor->setStatsInterval(statsIntervalMs);
        
        ALOG_INFO("DDR processor configured - Retry: %d, BaseDelay: %dms, RetryDelay: %dms, StatsInterval: %dms",
            "CU", "--", retryCount, baseDelayMs, retryDelayMs, statsIntervalMs);
    }
}

void CustomMessageHandler::printDDRStats()
{
    if (m_ddrProcessor) {
        m_ddrProcessor->printStats();
    }
}

void CustomMessageHandler::resetDDRStats()
{
    if (m_ddrProcessor) {
        m_ddrProcessor->resetStats();
    }
}
