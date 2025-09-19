# JsonRpcClient - Qt JSON-RPC 2.0 客户端

这是一个为 `JsonRpcServer` 专门设计的 Qt5.15 JSON-RPC 2.0 客户端库，实现了完整的通信协议和业务接口。

## 🚀 特性

- ✅ **完整的协议支持**：实现32字节二进制头部 + JSON-RPC 2.0规范
- ✅ **TCP长连接**：支持稳定的长连接通信
- ✅ **自动重连**：网络断开时自动重连
- ✅ **异步回调**：支持异步调用和回调处理
- ✅ **服务器通知**：接收服务器主动发送的通知消息
- ✅ **错误处理**：完善的错误处理和状态管理
- ✅ **线程安全**：在Qt事件循环中安全使用
- 🆕 **图形化测试工具**：提供完整的GUI界面进行接口测试
- 🆕 **可扩展设计**：支持轻松添加新的RPC方法
- 🆕 **多种视图**：文本、结构化树形、日志等多种结果显示方式

## 📁 文件结构

```
JsonRpcClient/
├── JsonRpcClient.h              # 客户端头文件
├── JsonRpcClient.cpp            # 客户端实现文件
├── JsonRpcTestWidget.h          # GUI测试工具头文件
├── JsonRpcTestWidget.cpp        # GUI测试工具实现文件
├── gui_main.cpp                 # GUI应用程序主函数
├── example_main.cpp             # 控制台示例程序
├── JsonRpcClient.pro            # qmake项目文件
├── CMakeLists.txt               # CMake项目文件
├── JsonRpcClient接口说明文档.md   # 详细API文档
└── README.md                    # 项目说明
```

## 🛠️ 编译要求

### 依赖项
- **Qt 5.15** 或更高版本
- **C++11** 或更高标准
- **QtNetwork** 模块（必需）
- **QtWidgets** 模块（GUI版本需要）

### 系统要求
- Windows 10+
- macOS 10.14+
- Linux (Ubuntu 18.04+)

## 📦 编译安装

### 方法1：使用 qmake

**编译GUI版本（推荐）：**
```bash
qmake JsonRpcClient.pro
make          # Linux/macOS
nmake         # Windows (Visual Studio)
mingw32-make  # Windows (MinGW)
```

**编译控制台版本：**
```bash
qmake JsonRpcClient.pro CONFIG+=console
make          # Linux/macOS
nmake         # Windows (Visual Studio)
mingw32-make  # Windows (MinGW)
```

### 方法2：使用 CMake
```bash
mkdir build
cd build
cmake ..
cmake --build .

# 这将生成两个可执行文件：
# - JsonRpcClientGUI      (图形界面版本)
# - JsonRpcClientConsole  (控制台版本)
```

### 方法3：集成到现有项目
直接将 `JsonRpcClient.h` 和 `JsonRpcClient.cpp` 复制到您的项目中，并在 `.pro` 文件中添加：
```pro
QT += network
HEADERS += JsonRpcClient.h
SOURCES += JsonRpcClient.cpp
```

## 🚦 快速开始

### 🖥️ GUI测试工具（推荐）

1. **启动应用程序**
   ```bash
   ./JsonRpcClientGUI      # Linux/macOS
   JsonRpcClientGUI.exe    # Windows
   ```

2. **连接到服务器**
   - 在左侧"连接设置"区域输入服务器地址和端口
   - 点击"连接"按钮

3. **测试RPC方法**
   - 连接成功后，左侧的RPC方法按钮会激活
   - 选择要测试的方法，修改参数（如果需要）
   - 点击对应的按钮执行测试

4. **查看结果**
   - 右侧有四个标签页：
     - **响应结果**：显示格式化的JSON响应
     - **结构化视图**：树形结构显示JSON数据
     - **通知消息**：显示服务器主动发送的通知
     - **详细日志**：显示所有操作的详细日志

### 💻 编程接口使用

```cpp
#include "JsonRpcClient.h"

// 创建客户端
JsonRpcClient* client = new JsonRpcClient(this);

// 连接到服务器
client->connectToServer("127.0.0.1", 8080);

// 监听连接状态
connect(client, &JsonRpcClient::connectionStateChanged,
        [](JsonRpcClient::ConnectionState state) {
    if (state == JsonRpcClient::Connected) {
        qDebug() << "已连接到服务器";
    }
});

// 调用RPC方法
client->loadProject("C:/project", "task.json", 
    [](bool success, const QJsonObject& result, const QString& error) {
        if (success) {
            qDebug() << "项目加载成功";
        } else {
            qWarning() << "加载失败:" << error;
        }
    });
```

### 接收服务器通知

```cpp
// 方法1：使用信号
connect(client, &JsonRpcClient::notificationReceived,
        [](const QString& method, const QJsonObject& params) {
    qDebug() << "收到通知:" << method;
});

// 方法2：使用回调函数
client->setNotificationCallback([](const QString& method, const QJsonObject& params) {
    if (method == "setLoadProjectResult") {
        bool success = params["result"].toBool();
        qDebug() << "项目加载结果:" << success;
    }
});
```

## 🔧 支持的RPC方法

| 方法名 | 说明 | 参数 |
|--------|------|------|
| `LoadProject` | 加载项目文件 | path, taskFileName |
| `SiteScanAndConnect` | 站点扫描和连接 | 无 |
| `GetProjectInfo` | 获取项目信息 | 无 |
| `GetProjectInfoExt` | 获取项目扩展信息 | 无 |
| `GetSKTInfo` | 获取SKT信息 | 无 |
| `SendUUID` | 发送UUID | uuid, strIP, nHopNum, sktEnable |
| `DoJob` | 执行作业任务 | strIP, sktEn, docmdSeqJson |
| `DoCustom` | 执行自定义命令 | 自定义参数 |

## 📋 运行示例

### GUI测试工具
1. 确保服务器在 `127.0.0.1:8080` 上运行
2. 编译并运行GUI程序：
   ```bash
   ./JsonRpcClientGUI      # Linux/macOS
   JsonRpcClientGUI.exe    # Windows
   ```

### 控制台版本
1. 确保服务器在 `127.0.0.1:8080` 上运行
2. 编译并运行控制台程序：
   ```bash
   ./JsonRpcClientConsole  # Linux/macOS
   JsonRpcClientConsole.exe # Windows
   ```

控制台程序将自动：
- 连接到服务器
- 依次测试所有RPC方法
- 显示服务器通知
- 15秒后自动断开连接

## 🔧 添加新接口

### 1. 在JsonRpcClient中添加新方法

在 `JsonRpcClient.h` 中添加新方法声明：
```cpp
void newMethod(const QString& param1, int param2, const RpcCallback& callback = nullptr);
```

在 `JsonRpcClient.cpp` 中实现：
```cpp
void JsonRpcClient::newMethod(const QString& param1, int param2, const RpcCallback& callback)
{
    QJsonObject params;
    params["param1"] = param1;
    params["param2"] = param2;
    
    sendJsonRpcRequest("NewMethod", params, callback);
}
```

### 2. 在GUI测试工具中添加支持

在 `JsonRpcTestWidget.cpp` 的 `initializeMethodConfigs()` 函数中添加：
```cpp
MethodConfig newMethod;
newMethod.name = "NewMethod";
newMethod.description = "新方法测试";
newMethod.parameters = {"param1", "param2"};
newMethod.parameterTypes["param1"] = "string";
newMethod.parameterTypes["param2"] = "number";
newMethod.defaultValues["param1"] = "default_value";
newMethod.defaultValues["param2"] = 100;
m_methodConfigs.append(newMethod);
```

然后在 `callRpcMethod()` 函数中添加调用逻辑：
```cpp
else if (methodName == "NewMethod") {
    m_client->newMethod(params["param1"].toString(), params["param2"].toInt(), callback);
}
```

这样新接口就会自动出现在GUI界面中，支持参数输入和测试！

## ✨ 主要功能

### 1. 连接管理
- **服务器连接**：支持连接到指定IP和端口的JSON-RPC服务器
- **自动重连**：网络中断时自动尝试重连
- **连接状态监控**：实时显示连接状态

### 2. RPC方法测试
- **预定义方法**：支持所有标准RPC方法（LoadProject、SiteScanAndConnect等）
- **参数配置**：动态生成参数输入表单
- **自定义调用**：支持调用任意RPC方法

### 3. 站点作业管理
- **站点选择**：选择已连接并加载项目的站点
- **座子选择**：可视化选择16个座子（根据BPU使能状态自动设置可用性）
- **命令选择**：选择要执行的命令序列（Program、Erase、Verify等）
- **执行作业**：发送DoJob命令到指定站点和座子

### 4. 自定义JSON编辑与发送
- **JSON编辑器**：支持编辑任意JSON数据（对象、数组、基本类型）
- **格式化功能**：自动格式化JSON数据，验证语法正确性
- **自定义命令**：通过DoCustom命令发送自定义数据到硬件
- **实时验证**：输入时实时检查JSON格式和连接状态

### 5. 结果显示
- **实时日志**：显示所有RPC请求和响应
- **通知接收**：实时接收服务器推送的通知消息
- **详细信息**：树形显示JSON数据结构

## 🖼️ GUI界面介绍

### 主界面布局
```
┌─────────────────────────────────────────────────────────────────┐
│ JSON-RPC 客户端测试工具 v1.0                                      │
├──────────────────┬──────────────────────────────────────────────┤
│ 左侧面板         │ 右侧面板（标签页）                           │
│                  │                                              │
│ ┌──────────────┐ │ ┌──────────────────────────────────────────┐ │
│ │ 连接设置     │ │ │ [响应][结构化][通知][日志][站点][JSON]   │ │
│ │ 服务器地址   │ │ │                                          │ │
│ │ 端口号       │ │ │  根据标签页显示不同内容：                │ │
│ │ [连接][断开] │ │ │  • 响应结果/结构化视图/通知/日志         │ │
│ └──────────────┘ │ │  • 站点作业管理                         │ │
│                  │ │  • 自定义JSON编辑                       │ │
│ ┌──────────────┐ │ │                                          │ │
│ │ RPC方法测试  │ │ │  站点作业管理标签页：                    │ │
│ │ 方法选择     │ │ │  ┌─站点选择[Site02(IP)]                │ │
│ │ 参数输入表单 │ │ │  ├─站点信息显示区域                     │ │
│ │ [加载项目]   │ │ │  ├─座子选择(4x4网格)                    │ │
│ │ [站点扫描]   │ │ │  │ [S0][S1][S2][S3]                   │ │
│ │ [发送UUID]   │ │ │  │ [S4][S5][S6][S7]                   │ │
│ │ [执行作业]   │ │ │  ├─命令序列选择                          │ │
│ │ ...更多按钮  │ │ │  └─[执行作业]                          │ │
│ └──────────────┘ │ │                                          │ │
│                  │ │  自定义JSON编辑标签页：                  │ │
│                  │ │  ┌─JSON编辑器                            │ │
│                  │ │  ├─状态显示                             │ │
│                  │ │  └─[格式化][清空][执行自定义命令]       │ │
│                  │ └──────────────────────────────────────────┘ │
└──────────────────┴──────────────────────────────────────────────┘
```

### 主要功能区域

1. **连接管理区域**
   - 服务器地址和端口配置
   - 连接状态显示（彩色指示）
   - 自动重连设置

2. **RPC方法测试区域**
   - 支持所有现有RPC方法
   - 动态参数表单（根据选择的方法自动生成）
   - 一键调用按钮

3. **右侧标签页区域**
   - **响应结果**：格式化的JSON文本显示
   - **结构化视图**：树形结构，便于查看复杂JSON
   - **通知消息**：服务器主动推送的通知
   - **详细日志**：包含时间戳的完整操作日志
   - **站点作业管理**：站点选择、座子管理、作业执行
   - **自定义JSON编辑**：自由编辑JSON数据并发送

4. **控制功能**
   - 清除日志按钮
   - 自动滚动开关
   - JSON格式化开关

### 📋 推荐使用流程

#### 1. 基础连接和测试
1. **连接服务器** → 输入IP和端口 → 点击"连接"
2. **测试基础功能** → 使用左侧RPC方法按钮进行基础测试

#### 2. 站点作业管理流程
1. **站点扫描** → 点击"站点扫描连接"获取站点信息
2. **加载项目** → 点击"加载项目"并选择项目文件
3. **切换到"站点作业管理"标签页**：
   - 选择要操作的站点
   - 查看站点详细信息（IP、固件版本等）
   - 选择要使用的座子（根据BPU使能状态）
   - 选择要执行的命令序列
   - 点击"执行作业"

#### 3. 自定义JSON命令流程
1. **完成站点和座子选择**（在"站点作业管理"标签页）
2. **切换到"自定义JSON编辑"标签页**：
   - 编辑自定义JSON数据
   - 使用"格式化JSON"验证语法
   - 查看状态提示确认准备就绪
   - 点击"执行自定义命令"

> **💡 提示**：站点和座子的选择在两个功能间是共享的，您只需要在"站点作业管理"标签页中设置一次即可。

## 🐛 故障排除

### 常见问题

**Q: 连接失败怎么办？**
A: 检查服务器是否运行，防火墙设置，网络连接是否正常。

**Q: 收不到服务器通知？**
A: 确保已设置通知回调函数或连接了相应的信号。

**Q: 编译错误？**
A: 检查Qt版本是否为5.15+，是否包含了QtNetwork和QtWidgets模块。

**Q: 如何只编译控制台版本？**
A: 使用 `qmake JsonRpcClient.pro CONFIG+=console` 或在CMake中只构建JsonRpcClientConsole目标。

**Q: GUI界面显示不正常？**
A: 确保系统支持Qt Widgets，检查是否缺少GUI相关的系统库。

### 日志调试

**正常模式（默认）**：只显示应用程序相关的日志，过滤Qt框架内部信息
```bash
./JsonRpcClientGUI.exe
```

**详细日志模式**：显示所有调试信息（包括Qt内部日志）
```bash
# Windows
set JSONRPC_VERBOSE_LOG=1
JsonRpcClientGUI.exe

# Linux/macOS  
export JSONRPC_VERBOSE_LOG=1
./JsonRpcClientGUI
```

**控制台日志说明**：
- ✅ 默认会过滤掉Qt手势识别、鼠标事件、文本绘制等内部日志
- ✅ 只显示JSON-RPC请求、响应、连接状态等有用信息
- ✅ 通过环境变量可以启用完整的调试模式

## 📚 文档

- **[完整API文档](JsonRpcClient接口说明文档.md)** - 详细的接口说明和使用指南
- **[示例代码](example_main.cpp)** - 完整的使用示例

## 🔄 协议规范

### 数据包格式
```
+------------------+------------------+
|   32字节头部      |    JSON载荷       |
+------------------+------------------+
| Magic | Ver | Len | Reserved | JSON |
|   4   |  2  |  4  |    22    |  ... |
+------------------+------------------+
```

### JSON-RPC 请求
```json
{
    "jsonrpc": "2.0",
    "method": "LoadProject",
    "params": {
        "path": "C:/project",
        "taskFileName": "task.json"
    },
    "id": "1"
}
```

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

## 📄 许可证

本项目遵循项目许可证条款。

## 📞 支持

如有问题或建议，请联系开发团队。

---

**Happy Coding! 🎉** 