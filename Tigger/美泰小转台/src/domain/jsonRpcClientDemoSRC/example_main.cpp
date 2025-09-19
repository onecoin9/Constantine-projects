#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include "JsonRpcClient.h"

class ClientExample : public QObject
{
    Q_OBJECT

public:
    explicit ClientExample(QObject *parent = nullptr) : QObject(parent)
    {
        // 创建客户端实例
        m_client = new JsonRpcClient(this);
        
        // 连接信号
        connect(m_client, &JsonRpcClient::connectionStateChanged,
                this, &ClientExample::onConnectionStateChanged);
        connect(m_client, &JsonRpcClient::notificationReceived,
                this, &ClientExample::onNotificationReceived);
        connect(m_client, &JsonRpcClient::errorOccurred,
                this, &ClientExample::onErrorOccurred);
        
        // 设置通知回调
        m_client->setNotificationCallback([this](const QString& method, const QJsonObject& params) {
            qDebug() << "通知回调被调用:" << method;
            qDebug() << "参数:" << QJsonDocument(params).toJson();
        });
    }

public slots:
    void connectToServer()
    {
        qDebug() << "=== 连接到服务器 ===";
        m_client->connectToServer("127.0.0.1", 8080, true);
    }
    
    void testLoadProject()
    {
        qDebug() << "=== 测试加载项目 ===";
        m_client->loadProject("C:/projects/test_project", "task.json", 
            [](bool success, const QJsonObject& result, const QString& error) {
                if (success) {
                    qDebug() << "加载项目成功:" << QJsonDocument(result).toJson();
                } else {
                    qWarning() << "加载项目失败:" << error;
                }
            });
    }
    
    void testSiteScanAndConnect()
    {
        qDebug() << "=== 测试站点扫描 ===";
        m_client->siteScanAndConnect([](bool success, const QJsonObject& result, const QString& error) {
            if (success) {
                qDebug() << "站点扫描启动成功:" << QJsonDocument(result).toJson();
            } else {
                qWarning() << "站点扫描失败:" << error;
            }
        });
    }
    
    void testSendUUID()
    {
        qDebug() << "=== 测试发送UUID ===";
        QString uuid = "12345678-1234-5678-9abc-123456789def";
        m_client->sendUUID(uuid, "192.168.1.100", 1, 0xFF, 
            [](bool success, const QJsonObject& result, const QString& error) {
                if (success) {
                    qDebug() << "UUID发送成功:" << QJsonDocument(result).toJson();
                } else {
                    qWarning() << "UUID发送失败:" << error;
                }
            });
    }
    
    void testDoJob()
    {
        qDebug() << "=== 测试执行作业 ===";
        
        // 构造命令序列
        QJsonObject cmdSequence;
        cmdSequence["command"] = "program";
        cmdSequence["data"] = "test_data";
        
        m_client->doJob("192.168.1.100", 0xFF, cmdSequence,
            [](bool success, const QJsonObject& result, const QString& error) {
                if (success) {
                    qDebug() << "作业执行成功:" << QJsonDocument(result).toJson();
                } else {
                    qWarning() << "作业执行失败:" << error;
                }
            });
    }
    
    void testGetProjectInfo()
    {
        qDebug() << "=== 测试获取项目信息 ===";
        m_client->getProjectInfo([](bool success, const QJsonObject& result, const QString& error) {
            if (success) {
                qDebug() << "项目信息获取成功:" << QJsonDocument(result).toJson();
            } else {
                qWarning() << "项目信息获取失败:" << error;
            }
        });
    }
    
    void testCustomCommand()
    {
        qDebug() << "=== 测试自定义命令 ===";
        QJsonObject params;
        params["custom_param"] = "test_value";
        params["number"] = 123;
        
        m_client->doCustom(params, [](bool success, const QJsonObject& result, const QString& error) {
            if (success) {
                qDebug() << "自定义命令执行成功:" << QJsonDocument(result).toJson();
            } else {
                qWarning() << "自定义命令执行失败:" << error;
            }
        });
    }
    
    void runSequentialTests()
    {
        qDebug() << "\n=== 开始顺序测试 ===";
        
        // 使用定时器来顺序执行测试
        QTimer::singleShot(1000, this, &ClientExample::testLoadProject);
        QTimer::singleShot(3000, this, &ClientExample::testSiteScanAndConnect);
        QTimer::singleShot(5000, this, &ClientExample::testGetProjectInfo);
        QTimer::singleShot(7000, this, &ClientExample::testSendUUID);
        QTimer::singleShot(9000, this, &ClientExample::testDoJob);
        QTimer::singleShot(11000, this, &ClientExample::testCustomCommand);
        
        // 15秒后断开连接
        QTimer::singleShot(15000, [this]() {
            qDebug() << "=== 断开连接 ===";
            m_client->disconnectFromServer();
        });
        
        // 20秒后退出程序
        QTimer::singleShot(20000, []() {
            qDebug() << "=== 测试完成，退出程序 ===";
            QCoreApplication::quit();
        });
    }

private slots:
    void onConnectionStateChanged(JsonRpcClient::ConnectionState state)
    {
        QString stateStr;
        switch (state) {
        case JsonRpcClient::Disconnected:
            stateStr = "已断开";
            break;
        case JsonRpcClient::Connecting:
            stateStr = "连接中";
            break;
        case JsonRpcClient::Connected:
            stateStr = "已连接";
            // 连接成功后开始测试
            QTimer::singleShot(500, this, &ClientExample::runSequentialTests);
            break;
        case JsonRpcClient::Reconnecting:
            stateStr = "重连中";
            break;
        }
        qDebug() << "连接状态变更:" << stateStr;
    }
    
    void onNotificationReceived(const QString& method, const QJsonObject& params)
    {
        qDebug() << "收到服务器通知:";
        qDebug() << "  方法:" << method;
        qDebug() << "  参数:" << QJsonDocument(params).toJson();
        
        // 处理特定的通知
        if (method == "setLoadProjectResult") {
            qDebug() << "项目加载结果通知";
        } else if (method == "SendUUIDResult") {
            qDebug() << "UUID发送结果通知";
        } else if (method == "Error") {
            qDebug() << "服务器错误通知";
        }
    }
    
    void onErrorOccurred(const QString& errorMessage)
    {
        qWarning() << "客户端错误:" << errorMessage;
    }

private:
    JsonRpcClient* m_client;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== JsonRpcClient 示例程序 ===";
    qDebug() << "本程序将演示如何使用 JsonRpcClient 与服务器通信";
    qDebug() << "确保服务器正在 127.0.0.1:8080 上运行";
    
    ClientExample example;
    
    // 1秒后开始连接
    QTimer::singleShot(1000, &example, &ClientExample::connectToServer);
    
    return app.exec();
}

#include "example_main.moc" 