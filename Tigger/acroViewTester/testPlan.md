TestPlan/
├── include/                     # 头文件
│   ├── core/                    # 核心引擎
│   │   ├── TestEngine.h         # 测试引擎
│   │   ├── TestScheduler.h      # 测试调度器
│   │   └── TestEventBus.h       # 事件总线
│   ├── domain/                  # 领域层
│   │   ├── devices/             # 设备抽象和实现
│   │   │   ├── IDevice.h        # 设备接口
│   │   │   ├── TemperatureDevice.h  # 温控设备
│   │   │   └── UdpDevice.h      # UDP通信设备
│   │   └── models/              # 领域模型
│   │       ├── TestCase.h       # 测试用例
│   │       ├── TestSuite.h      # 测试套件
│   │       └── TestResult.h     # 测试结果
│   ├── infrastructure/          # 基础设施层
│   │   ├── communications/      # 通信模块
│   │   │   ├── ICommChannel.h   # 通信通道接口
│   │   │   ├── SerialChannel.h  # 串口通信
│   │   │   ├── UdpChannel.h     # UDP通信
│   │   │   └── TcpChannel.h     # TCP通信
│   │   ├── data/                # 数据处理
│   │   │   ├── DataStorage.h    # 数据存储
│   │   │   └── DataExporter.h   # 数据导出
│   │   └── logging/             # 日志
│   │       └── LogManager.h     # 日志管理
│   ├── services/                # 服务层
│   │   ├── DeviceManager.h      # 设备管理服务
│   │   ├── TestPlanService.h    # 测试计划服务
│   │   └── ReportService.h      # 报告服务
│   └── presentation/            # 表现层
│       ├── QmlBridge.h          # QML桥接
│       └── ViewModel.h          # 视图模型
├── src/                         # 源代码
├── resources/                   # 资源文件
├── config/                      # 配置文件
│   ├── devices.json            # 设备配置
│   ├── testplan_template.json  # 测试计划模板
│   └── app_settings.json       # 应用程序设置
└── ui/                          # QML/UI文件