# JsonRpcClient 接口说明文档

## 概述

`JsonRpcClient` 是一个基于 Qt5.15 开发的 JSON-RPC 2.0 客户端类，专门用于与 `JsonRpcServer` 服务器进行通信。该客户端实现了服务器所要求的32字节二进制头部协议，支持 TCP 长连接、自动重连、异步回调等功能。

## 主要特性

- ✅ **TCP长连接通信**：支持与服务器建立稳定的TCP连接
- ✅ **自动重连机制**：连接断开时自动重新连接
- ✅ **异步回调处理**：支持异步调用和回调函数
- ✅ **服务器通知接收**：能够接收服务器主动发送的通知消息
- ✅ **完整的错误处理**：提供详细的错误信息和状态管理
- ✅ **线程安全**：可在Qt事件循环中安全使用

## 协议规范

### 二进制头部格式 (32字节)

| 字段 | 长度(字节) | 类型 | 说明 |
|------|------------|------|------|
| Magic Number | 4 | uint32 | 固定值: 0x4150524F ("APRO") |
| Header Version | 2 | uint16 | 协议版本: 1 |
| Payload Length | 4 | uint32 | JSON载荷长度 |
| Reserved | 22 | bytes | 保留字段，填充为0 |

**注意**：所有数值字段均采用大端序(Big-Endian)格式。

### JSON-RPC 2.0 格式

请求格式：
```json
{
    "jsonrpc": "2.0",
    "method": "方法名",
    "params": { /* 参数对象 */ },
    "id": "请求ID"
}
```

响应格式：
```json
{
    "jsonrpc": "2.0",
    "result": { /* 结果对象 */ },
    "id": "请求ID"
}
```

通知格式（服务器主动发送）：
```json
{
    "jsonrpc": "2.0",
    "method": "通知方法名",
    "result": { /* 通知数据 */ }
}
```

## 类定义

### 枚举类型

#### ConnectionState
连接状态枚举：
```cpp
enum ConnectionState {
    Disconnected,   // 未连接
    Connecting,     // 连接中
    Connected,      // 已连接
    Reconnecting    // 重连中
};
```

### 回调函数类型

#### RpcCallback
RPC调用结果回调函数：
```cpp
using RpcCallback = std::function<void(bool success, const QJsonObject& result, const QString& errorMessage)>;
```

**参数说明**：
- `success`：调用是否成功
- `result`：返回的JSON数据（成功时有效）
- `errorMessage`：错误信息（失败时有效）

#### NotificationCallback
服务器通知回调函数：
```cpp
using NotificationCallback = std::function<void(const QString& method, const QJsonObject& params)>;
```

**参数说明**：
- `method`：通知方法名
- `params`：通知参数

## 公共接口

### 构造函数和析构函数

```cpp
explicit JsonRpcClient(QObject *parent = nullptr);
virtual ~JsonRpcClient();
```

### 连接管理

#### connectToServer()
连接到服务器
```cpp
void connectToServer(const QString& host, quint16 port, bool autoReconnect = true);
```

**参数**：
- `host`：服务器主机地址（如 "127.0.0.1"）
- `port`：服务器端口号
- `autoReconnect`：是否启用自动重连（默认true）

**示例**：
```cpp
client->connectToServer("192.168.1.100", 8080, true);
```

#### disconnectFromServer()
断开与服务器的连接
```cpp
void disconnectFromServer();
```

#### getConnectionState()
获取当前连接状态
```cpp
ConnectionState getConnectionState() const;
```

#### setReconnectInterval()
设置自动重连间隔
```cpp
void setReconnectInterval(int milliseconds);
```

**参数**：
- `milliseconds`：重连间隔时间（毫秒），默认5000ms

### 通知回调设置

#### setNotificationCallback()
设置服务器通知回调函数
```cpp
void setNotificationCallback(const NotificationCallback& callback);
```

**示例**：
```cpp
client->setNotificationCallback([](const QString& method, const QJsonObject& params) {
    qDebug() << "收到通知:" << method;
    // 处理通知数据
});
```

## RPC方法接口

### loadProject()
加载项目文件
```cpp
void loadProject(const QString& projectPath, const QString& taskFileName, const RpcCallback& callback = nullptr);
```

**参数**：
- `projectPath`：项目文件路径
- `taskFileName`：任务文件名
- `callback`：结果回调函数（可选）

**示例**：
```cpp
client->loadProject("C:/projects/my_project", "task.json", 
    [](bool success, const QJsonObject& result, const QString& error) {
        if (success) {
            qDebug() << "项目加载成功";
        } else {
            qWarning() << "项目加载失败:" << error;
        }
    });
```

### siteScanAndConnect()
执行站点扫描和连接
```cpp
void siteScanAndConnect(const RpcCallback& callback = nullptr);
```

**示例**：
```cpp
client->siteScanAndConnect([](bool success, const QJsonObject& result, const QString& error) {
    if (success) {
        qDebug() << "站点扫描启动成功";
    }
});
```

### getProjectInfo()
获取项目信息
```cpp
void getProjectInfo(const RpcCallback& callback = nullptr);
```

### getProjectInfoExt()
获取项目扩展信息
```cpp
void getProjectInfoExt(const RpcCallback& callback = nullptr);
```

### getSKTInfo()
获取SKT信息
```cpp
void getSKTInfo(const RpcCallback& callback = nullptr);
```

### sendUUID()
发送UUID到硬件
```cpp
void sendUUID(const QString& uuid, const QString& strIP, quint32 hopNum, quint32 sktEnable, const RpcCallback& callback = nullptr);
```

**参数**：
- `uuid`：UUID字符串
- `strIP`：目标IP地址
- `hopNum`：跳数
- `sktEnable`：座子使能位图

**示例**：
```cpp
QString uuid = "12345678-1234-5678-9abc-123456789def";
client->sendUUID(uuid, "192.168.1.100", 1, 0xFF, 
    [](bool success, const QJsonObject& result, const QString& error) {
        qDebug() << "UUID发送结果:" << (success ? "成功" : error);
    });
```

### doJob()
执行作业任务
```cpp
void doJob(const QString& strIP, quint32 sktEnable, const QJsonObject& cmdSequence, const RpcCallback& callback = nullptr);
```

**参数**：
- `strIP`：目标IP地址
- `sktEnable`：座子使能位图
- `cmdSequence`：命令序列JSON对象

**示例**：
```cpp
QJsonObject cmdSeq;
cmdSeq["operation"] = "program";
cmdSeq["data"] = "binary_data_here";

client->doJob("192.168.1.100", 0xFF, cmdSeq, 
    [](bool success, const QJsonObject& result, const QString& error) {
        qDebug() << "作业执行结果:" << (success ? "成功" : error);
    });
```

### doCustom()
执行自定义命令
```cpp
void doCustom(const QJsonObject& params, const RpcCallback& callback = nullptr);
```

### callRpcMethod()
通用RPC方法调用
```cpp
void callRpcMethod(const QString& method, const QJsonObject& params, const RpcCallback& callback = nullptr);
```

**参数**：
- `method`：方法名
- `params`：参数对象
- `callback`：结果回调函数

## 信号

### connectionStateChanged()
连接状态变化信号
```cpp
void connectionStateChanged(ConnectionState state);
```

### notificationReceived()
服务器通知信号
```cpp
void notificationReceived(const QString& method, const QJsonObject& params);
```

### errorOccurred()
错误发生信号
```cpp
void errorOccurred(const QString& errorMessage);
```

## 使用示例

### 基本使用流程

```cpp
#include "JsonRpcClient.h"

class MyApplication : public QObject 
{
    Q_OBJECT
    
public:
    MyApplication() {
        // 创建客户端
        m_client = new JsonRpcClient(this);
        
        // 连接信号
        connect(m_client, &JsonRpcClient::connectionStateChanged,
                this, &MyApplication::onConnectionChanged);
        connect(m_client, &JsonRpcClient::notificationReceived,
                this, &MyApplication::onNotification);
        connect(m_client, &JsonRpcClient::errorOccurred,
                this, &MyApplication::onError);
        
        // 连接到服务器
        m_client->connectToServer("127.0.0.1", 8080);
    }
    
private slots:
    void onConnectionChanged(JsonRpcClient::ConnectionState state) {
        switch (state) {
        case JsonRpcClient::Connected:
            qDebug() << "已连接，开始执行业务逻辑";
            startWork();
            break;
        case JsonRpcClient::Disconnected:
            qDebug() << "连接已断开";
            break;
        }
    }
    
    void onNotification(const QString& method, const QJsonObject& params) {
        qDebug() << "收到通知:" << method;
        
        if (method == "setLoadProjectResult") {
            // 处理项目加载结果
            bool success = params["result"].toBool();
            if (success) {
                // 项目加载成功，可以继续后续操作
                m_client->getProjectInfo();
            }
        }
    }
    
    void onError(const QString& error) {
        qWarning() << "客户端错误:" << error;
    }
    
    void startWork() {
        // 加载项目
        m_client->loadProject("C:/my_project", "task.json",
            [this](bool success, const QJsonObject& result, const QString& error) {
                if (success) {
                    qDebug() << "项目加载请求已发送";
                    // 注意：实际结果会通过通知返回
                }
            });
    }
    
private:
    JsonRpcClient* m_client;
};
```

### 高级使用：链式调用

```cpp
void performCompleteWorkflow() {
    // 1. 首先加载项目
    m_client->loadProject("C:/project", "task.json", 
        [this](bool success, const QJsonObject&, const QString& error) {
            if (!success) {
                qWarning() << "项目加载失败:" << error;
                return;
            }
            
            // 2. 然后执行站点扫描
            m_client->siteScanAndConnect([this](bool success, const QJsonObject&, const QString& error) {
                if (!success) {
                    qWarning() << "站点扫描失败:" << error;
                    return;
                }
                
                // 3. 最后发送UUID
                QString uuid = generateUUID();
                m_client->sendUUID(uuid, "192.168.1.100", 1, 0xFF,
                    [](bool success, const QJsonObject&, const QString& error) {
                        if (success) {
                            qDebug() << "完整工作流程执行成功";
                        } else {
                            qWarning() << "UUID发送失败:" << error;
                        }
                    });
            });
        });
}
```

## 错误处理

### 常见错误类型

1. **连接错误**
   - 服务器未运行
   - 网络不可达
   - 端口被占用

2. **协议错误**
   - Magic Number不匹配
   - 协议版本不支持
   - 数据包格式错误

3. **RPC错误**
   - 方法不存在
   - 参数格式错误
   - 服务器内部错误

### 错误处理策略

```cpp
// 设置详细的错误处理
m_client->loadProject("path", "file", 
    [](bool success, const QJsonObject& result, const QString& error) {
        if (!success) {
            if (error.contains("连接")) {
                // 连接问题，可能需要重连
                qWarning() << "连接问题，正在重试...";
            } else if (error.contains("参数")) {
                // 参数错误，需要检查调用参数
                qCritical() << "参数错误，请检查输入:" << error;
            } else {
                // 其他错误
                qWarning() << "操作失败:" << error;
            }
        }
    });
```

## 性能优化建议

### 1. 连接管理
- 使用长连接，避免频繁连接/断开
- 启用自动重连功能
- 合理设置重连间隔

### 2. 数据传输
- 避免发送过大的JSON数据
- 使用紧凑的JSON格式
- 批量操作减少网络往返

### 3. 内存管理
- 及时清理不需要的回调函数
- 避免在回调中保存大对象的引用
- 使用智能指针管理对象生命周期

## 注意事项

### 1. 线程安全
- 客户端必须在主线程中使用
- 回调函数在主线程中执行
- 不要在回调中执行耗时操作

### 2. 回调函数
- 回调函数可能不会立即执行
- 确保回调中引用的对象仍然有效
- 避免在回调中抛出异常

### 3. 服务器通知
- 通知消息是异步的
- 通知不保证顺序
- 需要根据业务逻辑处理通知

## 编译要求

### 依赖项
- Qt 5.15 或更高版本
- C++11 或更高标准
- QtNetwork 模块

### 编译配置
在 `.pro` 文件中添加：
```pro
QT += core network
CONFIG += c++11

HEADERS += JsonRpcClient.h
SOURCES += JsonRpcClient.cpp
```

### CMake 配置
```cmake
find_package(Qt5 REQUIRED COMPONENTS Core Network)

add_executable(your_app 
    JsonRpcClient.h 
    JsonRpcClient.cpp 
    your_main.cpp
)

target_link_libraries(your_app Qt5::Core Qt5::Network)
```

## 版本信息

- **当前版本**：1.0.0
- **Qt版本要求**：5.15+
- **协议版本**：1
- **最后更新**：2024年

## 许可证

本代码遵循项目许可证条款。

---

如有问题或建议，请联系开发团队。 