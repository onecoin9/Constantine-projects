# MT门压芯片自动化测试系统 - 软件需求规格说明书

## 1. 项目概述

### 1.1 项目背景
本项目旨在开发MT门压芯片（SMP475）的自动化测试系统，实现芯片参数的自动检测、校准和验证。

### 1.2 测试芯片规格
- 芯片型号：SMP475
- 通信协议：PSI5（Peripheral Sensor Interface 5）
- 封装形式：标准传感器封装
- 工作温度：-40C to +150C
- 压力检测范围：0-300KPa

### 1.3 测试目标
- 实现芯片寄存器的读写操作
- 完成芯片校准参数的自动设置
- 验证芯片在不同温度和压力下的性能
- 达到UPH1200的测试效率

## 2. 系统架构

### 2.1 硬件架构
`
测试控制器
 AP8000设备1（8通道）
 AP8000设备2（8通道）
 温度控制系统（4个温度点）
 压力控制系统（0-300KPa）
`

### 2.2 软件架构
`
应用层 (Qt界面)
 测试流程控制
 数据采集显示
 结果分析报告

业务逻辑层
 PSI5协议栈
 设备驱动接口
 测试算法引擎

数据层
 测试数据存储
 校准参数管理
 历史记录查询
`

## 3. 功能需求

### 3.1 核心功能

#### 3.1.1 芯片识别与连接
- 自动检测芯片连接状态
- 读取芯片ID和版本信息
- 验证PSI5通信链路

#### 3.1.2 寄存器操作
- 读取所有可访问寄存器
- 写入校准参数到指定寄存器
- 验证写入数据的正确性

#### 3.1.3 校准功能
- 零点校准：在零压力下校准传感器
- 满量程校准：在最大压力下校准
- 温度补偿：不同温度点的补偿参数
- 线性度校准：多点压力校准

#### 3.1.4 测试验证
- 精度测试：测量精度验证
- 重复性测试：多次测量一致性
- 温度特性测试：温度漂移测试
- 响应时间测试：动态响应特性

### 3.2 扩展功能

#### 3.2.1 批量测试
- 同时测试16个芯片（8+8通道）
- 并行测试流程管理
- 测试结果汇总分析

#### 3.2.2 数据管理
- 测试数据实时存储
- 历史数据查询检索
- 数据导出功能（Excel/CSV）
- 统计分析报告

#### 3.2.3 质量控制
- 测试标准配置管理
- 合格率统计分析
- 异常数据预警
- 测试追溯功能

## 4. 测试流程规范

### 4.1 标准测试流程
1. **初始化阶段**
   - 设备自检
   - 通道分配
   - 环境条件确认

2. **芯片检测阶段**
   - PSI5链路建立
   - 芯片ID读取
   - 基本功能验证

3. **校准阶段**
   - 零点校准（0KPa）
   - 中点校准（150KPa）
   - 满量程校准（300KPa）
   - 温度补偿校准

4. **验证测试阶段**
   - 精度验证测试
   - 温度特性测试
   - 重复性验证
   - 稳定性测试

5. **结果评估阶段**
   - 数据分析处理
   - 合格性判定
   - 报告生成
   - 数据存储

### 4.2 特殊测试模式
- **快速测试模式**：仅核心参数测试，UPH优化
- **完整测试模式**：全参数测试，质量保证
- **调试测试模式**：单步测试，问题诊断
- **校准模式**：设备校准，精度维护

## 5. 数据格式规范

### 5.1 寄存器数据格式

#### 5.1.1 基本寄存器定义
`cpp
// 芯片识别寄存器
#define REG_CHIP_ID        0x00   // 芯片ID
#define REG_VERSION        0x01   // 版本信息
#define REG_STATUS         0x02   // 状态寄存器

// 校准参数寄存器
#define REG_ZERO_CAL       0x10   // 零点校准
#define REG_SPAN_CAL       0x11   // 量程校准
#define REG_TEMP_COMP      0x12   // 温度补偿
#define REG_LINEARITY      0x13   // 线性度校准

// 测试数据寄存器
#define REG_PRESSURE_DATA  0x20   // 压力数据
#define REG_TEMP_DATA      0x21   // 温度数据
#define REG_DIAG_DATA      0x22   // 诊断数据
`

#### 5.1.2 数据位定义
`cpp
// 状态寄存器位定义
typedef struct {
    uint16_t cal_done    : 1;  // 校准完成标志
    uint16_t temp_ready  : 1;  // 温度数据就绪
    uint16_t press_ready : 1;  // 压力数据就绪
    uint16_t error_flag  : 1;  // 错误标志
    uint16_t reserved    : 12; // 保留位
} StatusRegister;

// 压力数据格式
typedef struct {
    uint16_t pressure_raw : 12; // 原始压力数据
    uint16_t valid_flag   : 1;  // 数据有效标志
    uint16_t reserved     : 3;  // 保留位
} PressureData;
`

### 5.2 测试数据存储格式

#### 5.2.1 数据库表结构
`sql
-- 测试记录主表
CREATE TABLE test_records (
    id              INTEGER PRIMARY KEY,
    chip_id         VARCHAR(20) NOT NULL,
    test_time       DATETIME DEFAULT CURRENT_TIMESTAMP,
    operator        VARCHAR(50),
    test_mode       VARCHAR(20),
    test_result     VARCHAR(10),
    total_time      INTEGER
);

-- 测试数据详表
CREATE TABLE test_data (
    record_id       INTEGER,
    test_step       VARCHAR(50),
    parameter_name  VARCHAR(30),
    measured_value  REAL,
    expected_value  REAL,
    tolerance       REAL,
    pass_fail       VARCHAR(10),
    FOREIGN KEY (record_id) REFERENCES test_records(id)
);

-- 校准参数表
CREATE TABLE calibration_params (
    chip_id         VARCHAR(20),
    param_type      VARCHAR(30),
    param_value     REAL,
    calibration_date DATETIME,
    valid_until     DATETIME
);
`

#### 5.2.2 数据文件格式
`json
{
    "testRecord": {
        "chipId": "SMP475_001",
        "testTime": "2024-01-15T10:30:00Z",
        "operator": "TestOperator",
        "testMode": "FULL_TEST",
        "testResult": "PASS",
        "testDuration": 45,
        "testSteps": [
            {
                "stepName": "ZERO_CALIBRATION",
                "stepResult": "PASS",
                "parameters": [
                    {
                        "name": "ZeroOffset",
                        "measured": 0.05,
                        "expected": 0.00,
                        "tolerance": 0.1,
                        "unit": "KPa"
                    }
                ]
            }
        ]
    }
}
`

## 6. 性能要求

### 6.1 测试效率要求
- **单芯片测试时间**：3秒
- **UPH目标**：1200芯片/小时
- **并行测试能力**：16通道同时测试
- **设备利用率**：95%

### 6.2 测试精度要求
- **压力测试精度**：0.1% FS
- **温度测试精度**：0.5C
- **重复性**：0.05% FS
- **线性度**：0.1% FS

### 6.3 系统性能要求
- **系统响应时间**：100ms
- **数据处理能力**：1000条记录/秒
- **内存占用**：2GB
- **CPU使用率**：70%

### 6.4 可靠性要求
- **系统可用性**：99.9%
- **MTBF**：8760小时
- **数据完整性**：100%
- **错误恢复时间**：10秒

## 7. 接口规范

### 7.1 PSI5通信接口

#### 7.1.1 物理层规范
- **通信速率**：125Kbps
- **电压等级**：12V供电
- **阻抗匹配**：50Ω
- **连接器规格**：标准PSI5连接器

#### 7.1.2 协议层规范
`cpp
// PSI5命令格式
typedef struct {
    uint8_t sync_byte;      // 同步字节 0x55
    uint8_t device_addr;    // 设备地址
    uint8_t command;        // 命令字节
    uint8_t data_length;    // 数据长度
    uint8_t data[MAX_DATA_LEN]; // 数据域
    uint16_t crc;           // CRC校验
} PSI5_Frame;

// 命令定义
#define CMD_READ_REG       0x01  // 读寄存器
#define CMD_WRITE_REG      0x02  // 写寄存器
#define CMD_CALIBRATE      0x03  // 执行校准
#define CMD_RESET          0x04  // 复位命令
`

### 7.2 AP8000设备接口

#### 7.2.1 设备控制接口
`cpp
class AP8000Interface {
public:
    bool Initialize(int deviceId);
    bool SelectChannel(int channel);
    bool SetVoltage(double voltage);
    bool SetCurrent(double current);
    bool MeasureVoltage(double* voltage);
    bool MeasureCurrent(double* current);
    void Cleanup();
};
`

#### 7.2.2 通道管理
- **设备1通道**：CH1-CH8 (芯片位置1-8)
- **设备2通道**：CH9-CH16 (芯片位置9-16)
- **通道切换时间**：10ms
- **测量精度**：电压0.1%, 电流0.1%

### 7.3 外部程序接口

#### 7.3.1 MPPTCali.exe接口
`ash
# 校准程序调用格式
MPPTCali.exe -chip <chip_id> -mode <cal_mode> -output <result_file>

# 参数说明：
# chip_id: 芯片标识符
# cal_mode: 校准模式 (ZERO|SPAN|FULL)
# result_file: 结果输出文件
`

#### 7.3.2 MTPTCheck.exe接口
`ash
# 验证程序调用格式
MTPTCheck.exe -chip <chip_id> -test <test_type> -report <report_file>

# 参数说明：
# chip_id: 芯片标识符
# test_type: 测试类型 (ACCURACY|LINEARITY|TEMP)
# report_file: 测试报告文件
`

## 8. 用户界面要求

### 8.1 主界面设计

#### 8.1.1 界面布局
`

  菜单栏: 文件 编辑 测试 工具 帮助                          

  工具栏: [开始] [停止] [暂停] [设置] [导出] [帮助]          

  设备状态                  测试进度                    
  AP8000-1       
   CH1:PASS        当前测试: 芯片ID_001              
   CH2:FAIL        进度: [] 75%         
   CH3:TEST        剩余时间: 00:01:25                
   CH4:IDLE        已完成: 450/600                  
         

  测试结果                  实时数据                    
  总数: 450           
  通过: 445          压力: 150.25 KPa                  
  失败: 5            温度: 25.3 C                     
  合格率: 98.9%      电压: 12.05 V                     
                     电流: 15.2 mA                     
                      

`

#### 8.1.2 状态指示
- **绿色**：测试通过，设备正常
- **红色**：测试失败，设备错误
- **黄色**：测试进行中，设备忙碌
- **灰色**：设备空闲，等待测试

### 8.2 功能界面

#### 8.2.1 测试配置界面
`cpp
class TestConfigDialog {
private:
    QComboBox* testModeCombo;      // 测试模式选择
    QSpinBox* temperaturePoints;   // 温度点数设置
    QDoubleSpinBox* pressureMin;   // 最小压力值
    QDoubleSpinBox* pressureMax;   // 最大压力值
    QCheckBox* enableCalibration;  // 是否执行校准
    QLineEdit* operatorName;       // 操作员姓名
    
public slots:
    void onSaveConfig();
    void onLoadConfig();
    void onResetDefault();
};
`

#### 8.2.2 数据查看界面
`cpp
class DataViewDialog {
private:
    QTableWidget* dataTable;       // 数据表格显示
    QChartView* trendChart;        // 趋势图表显示
    QDateTimeEdit* startTime;      // 开始时间选择
    QDateTimeEdit* endTime;        // 结束时间选择
    QComboBox* chipIdFilter;       // 芯片ID过滤
    
public slots:
    void onRefreshData();
    void onExportData();
    void onPrintReport();
};
`

### 8.3 交互要求

#### 8.3.1 操作便利性
- **一键开始**：单击开始按钮即可启动测试
- **实时监控**：测试过程实时显示进度和状态
- **异常提醒**：测试异常时自动弹窗提醒
- **快捷操作**：支持键盘快捷键操作

#### 8.3.2 数据可视化
- **实时曲线**：压力、温度、电流等参数实时曲线
- **统计图表**：合格率、效率等统计图表
- **趋势分析**：历史数据趋势分析图
- **3D显示**：温度-压力-精度三维关系图

## 9. 系统集成

### 9.1 开发环境
- **IDE**：Qt Creator 5.15+
- **编译器**：MSVC 2019 / GCC 9.0+
- **构建系统**：CMake 3.16+
- **版本控制**：Git

### 9.2 第三方库依赖
`cmake
# CMakeLists.txt 依赖配置
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Charts SerialPort)
find_package(SQLite3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(Eigen3 REQUIRED)

target_link_libraries(TestFramework
    Qt5::Core
    Qt5::Widgets
    Qt5::Charts
    Qt5::SerialPort
    SQLite::SQLite3
    spdlog::spdlog
    Eigen3::Eigen
)
`

### 9.3 部署要求
- **操作系统**：Windows 10/11 64位
- **最小内存**：8GB RAM
- **存储空间**：100GB可用空间
- **USB接口**：2个USB 3.0接口
- **网络连接**：以太网口（设备通信）

### 9.4 安装包结构
`
TestFramework_Setup.exe
 bin/
    TesterFramework.exe    # 主程序
    MPPTCali.exe          # 校准程序
    MTPTCheck.exe         # 验证程序
    Qt5*.dll              # Qt运行库
 config/
    device.json           # 设备配置
    test_specs.json       # 测试规范
    user_settings.ini     # 用户设置
 drivers/
    AP8000_driver.dll     # AP8000驱动
    PSI5_driver.dll       # PSI5驱动
 docs/
     用户手册.pdf           # 用户手册
     操作指南.pdf           # 操作指南
     API文档.html           # API文档
`

---

## 附录

### 附录A：寄存器映射表

| 地址 | 寄存器名称 | 读写属性 | 位宽 | 描述 |
|------|------------|----------|------|------|
| 0x00 | CHIP_ID | R | 16 | 芯片识别码 |
| 0x01 | VERSION | R | 16 | 版本信息 |
| 0x02 | STATUS | R | 16 | 状态寄存器 |
| 0x10 | ZERO_CAL | R/W | 16 | 零点校准值 |
| 0x11 | SPAN_CAL | R/W | 16 | 量程校准值 |
| 0x12 | TEMP_COMP | R/W | 16 | 温度补偿系数 |
| 0x20 | PRESSURE_DATA | R | 16 | 压力测量数据 |
| 0x21 | TEMP_DATA | R | 16 | 温度测量数据 |

### 附录B：错误代码表

| 错误码 | 错误描述 | 处理建议 |
|--------|----------|----------|
| E001 | PSI5通信超时 | 检查连接线缆 |
| E002 | 寄存器读取失败 | 重新尝试通信 |
| E003 | 校准参数异常 | 重新执行校准 |
| E004 | 温度超出范围 | 检查温控系统 |
| E005 | 压力超出范围 | 检查压力系统 |

### 附录C：测试标准参考

- **ISO 26262**：汽车功能安全标准
- **PSI5 Specification v2.1**：PSI5协议标准
- **IEC 61508**：电气安全标准
- **JEDEC JESD22**：半导体测试标准

---

**文档版本**：v1.0  
**编制日期**：2024-01-15  
**编制人员**：测试团队  
**审核状态**：待审核
