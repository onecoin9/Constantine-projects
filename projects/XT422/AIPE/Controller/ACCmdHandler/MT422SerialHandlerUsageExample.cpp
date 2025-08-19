// MT422SerialHandlerUsageExample.cpp
// 这是一个示例文件，展示如何使用单例模式的 MT422SerialHandler

#include "MT422SerialHandler.h"
#include <QCoreApplication>
#include <QDebug>

// 示例1：基本使用
void basicUsageExample()
{
    // 获取单例实例
    MT422SerialHandler& handler = MT422SerialHandler::instance();
    
    // 加载配置文件
    QString configPath = QCoreApplication::applicationDirPath() + "/422com.json";
    if (handler.loadPortConfiguration(configPath)) {
        qDebug() << "Configuration loaded successfully";
    }
    
    // 发送消息
    QByteArray message = "Hello Serial Port";
    handler.sendMessage("192.168.1.100", 1, 0, message);
}

// 示例2：在不同的类中使用
class SerialCommunicationManager : public QObject
{
    Q_OBJECT
public:
    SerialCommunicationManager()
    {
        // 连接信号
        connect(&MT422SerialHandler::instance(), &MT422SerialHandler::messageReceived,
                this, &SerialCommunicationManager::onMessageReceived);
    }
    
    void sendData(const QString& ipHop, uint16_t bpu, quint8 index, const QByteArray& data)
    {
        // 直接使用单例实例发送数据
        MT422SerialHandler::instance().sendMessage(ipHop, bpu, index, data);
    }
    
private slots:
    void onMessageReceived(const QString& strIPHop, const uint16_t BPUID, 
                          quint8 index, const QByteArray& message)
    {
        qDebug() << "Received message from" << strIPHop << "BPU:" << BPUID 
                 << "Index:" << index << "Data:" << message.toHex();
    }
};

// 示例3：在应用程序生命周期中使用
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 应用程序启动时初始化
    MT422SerialHandler& handler = MT422SerialHandler::instance();
    
    // 配置串口
    QString configPath = app.applicationDirPath() + "/422com.json";
    if (!handler.loadPortConfiguration(configPath)) {
        qDebug() << "Failed to load configuration";
        return -1;
    }
    
    // 创建使用串口的管理器
    SerialCommunicationManager manager;
    
    // 发送测试数据
    QByteArray testData = "Test Message";
    manager.sendData("192.168.1.100:0", 1, 0, testData);
    
    // 运行事件循环
    int result = app.exec();
    
    // 应用程序退出时清理（可选）
    MT422SerialHandler::destroyInstance();
    
    return result;
}

// 示例4：在多线程环境中使用
class WorkerThread : public QThread
{
    Q_OBJECT
protected:
    void run() override
    {
        // 在工作线程中也可以安全地获取单例实例
        MT422SerialHandler& handler = MT422SerialHandler::instance();
        
        // 发送数据
        QByteArray data = "Thread message";
        handler.sendMessage("192.168.1.101:0", 2, 1, data);
    }
};

// 注意事项：
// 1. 单例实例在第一次调用 instance() 时创建
// 2. 使用双重检查锁定确保线程安全
// 3. 可以在程序退出时调用 destroyInstance() 进行清理
// 4. 原有的所有功能保持不变，只是访问方式改为通过 instance() 方法

#include "MT422SerialHandlerUsageExample.moc" 