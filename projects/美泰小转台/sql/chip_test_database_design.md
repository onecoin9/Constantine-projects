# 芯片测试数据库设计

## 数据库概述

本数据库用于存储芯片测试过程中的各项测量数据和测试结果，以芯片UID和批次ID作为复合主键，记录每个芯片在不同批次中的完整测试信息。

## 数据库表结构 (Mermaid ER图)

```mermaid
erDiagram
    chip_test_data {
        TEXT uid PK "芯片UID(复合主键)"
        TEXT chip_model "芯片型号"
        TEXT lotid PK "批次ID(复合主键)"
        TEXT activation_time "激活时间"
        TEXT marking_number "打标号"
        REAL working_current_measured "工作电流测量值"
        REAL working_current_reference "工作电流参考阈值"
        REAL pll_dc_voltage_measured "锁相环直流电压测量值"
        REAL pll_dc_voltage_reference "锁相环直流电压参考阈值"
        REAL drive_dc_voltage_measured "驱动直流电压测量值"
        REAL drive_dc_voltage_reference "驱动直流电压参考阈值"
        REAL drive_dc_peak_voltage_measured "驱动直流电压峰峰值测量值"
        REAL drive_dc_peak_voltage_reference "驱动直流电压峰峰值参考阈值"
        REAL square_wave_freq_measured "方波频率测量值"
        REAL square_wave_freq_reference "方波频率参考阈值"
        REAL sine_wave_voltage_avg_measured "正弦波电压均值测量值"
        REAL sine_wave_voltage_avg_reference "正弦波电压均值参考阈值"
        REAL sine_wave_peak_voltage_measured "正弦波电压峰峰值测量值"
        REAL sine_wave_peak_voltage_reference "正弦波电压峰峰值参考阈值"
        REAL sine_wave_freq_measured "正弦波频率测量值"
        REAL sine_wave_freq_reference "正弦波频率参考阈值"
        INTEGER activation_ok_ng "激活测试结果(0=NG,1=OK)"
        INTEGER turntable_calibration_ok_ng "小转台标定结果(0=NG,1=OK)"
        TEXT machine_number "机器号"
        TEXT workflow_file "工作流文件"
        TEXT automation_task_file "自动化任务文件"
        TEXT burn_task_file "烧录任务文件"
        INTEGER os_test_result "OS测试结果(0=NG,1=OK)"
        TEXT os_test_details "OS测试详细信息"
        TEXT calibration_params "标定参数"
        TEXT param1 "预留参数1"
        TEXT param2 "预留参数2"
        TEXT param3 "预留参数3"
        TEXT param4 "预留参数4"
        TEXT param5 "预留参数5"
        TEXT param6 "预留参数6"
        TEXT param7 "预留参数7"
        TEXT param8 "预留参数8"
        TEXT param9 "预留参数9"
        TEXT param10 "预留参数10"
        DATETIME created_at "创建时间"
        DATETIME updated_at "更新时间"
    }
```

## 数据库索引设计

```mermaid
graph TD
    A[chip_test_data表] --> B[主键索引: uid]
    A --> C[普通索引: chip_model]
    A --> D[普通索引: lotid]
    A --> E[普通索引: activation_time]
    A --> F[普通索引: created_at]
    
    B --> G[唯一性约束<br/>快速主键查找]
    C --> H[按芯片型号查询优化]
    D --> I[按批次ID查询优化]
    E --> J[按激活时间查询优化]
    F --> K[按创建时间排序优化]
```

## 数据流程图

```mermaid
flowchart TD
    A[芯片测试开始] --> B[生成唯一UID]
    B --> C[设置基本信息<br/>型号/批次ID/标号等]
    C --> D[电流测试]
    D --> E[电压测试]
    E --> F[频率测试]
    F --> G[计算测试结果]
    G --> H{所有测试通过?}
    H -->|是| I[标记OK]
    H -->|否| J[标记NG]
    I --> K[写入数据库]
    J --> K
    K --> L[更新时间戳]
    L --> M[测试完成]
```

## 数据表关系说明

### 主表：chip_test_data
- **复合主键**: `uid, lotid` - 芯片UID和批次ID的组合
- **基本信息**: 芯片型号、批次ID、激活时间、打标号
- **测量数据**: 各项电压、电流、频率的测量值和参考阈值
- **测试结果**: 激活测试、转台标定和OS测试的OK/NG状态
- **设备和文件信息**: 机器号、工作流文件、自动化任务文件、烧录任务文件、OS测试详细信息、标定参数
- **预留参数**: 10个TEXT类型的预留字段(param1-param10)，用于扩展功能
- **时间戳**: 记录创建和更新时间

### 数据分类

```mermaid
mindmap
  root((芯片测试数据))
    基本信息
      芯片UID
      芯片型号
      批次ID
      激活时间
      打标号
    电流测量
      工作电流
        测量值
        参考阈值
    电压测量
      锁相环直流电压
        测量值
        参考阈值
      驱动直流电压
        测量值
        参考阈值
      驱动直流电压峰峰值
        测量值
        参考阈值
      正弦波电压均值
        测量值
        参考阈值
      正弦波电压峰峰值
        测量值
        参考阈值
    频率测量
      方波频率
        测量值
        参考阈值
      正弦波频率
        测量值
        参考阈值
    测试结果
      激活测试OK/NG
      转台标定OK/NG
      OS测试OK/NG
    设备和文件信息
      机器号
      工作流文件
      自动化任务文件
      烧录任务文件
      OS测试详细信息
      标定参数
    预留参数
      预留参数1-10
    时间信息
      创建时间
      更新时间
```

## 数据库操作流程

```mermaid
sequenceDiagram
    participant App as 应用程序
    participant DB as 数据库
    participant Trigger as 更新触发器

    Note over App,DB: 数据插入流程
    App->>DB: INSERT 芯片测试数据
    DB-->>App: 返回插入结果
    
    Note over App,DB: 数据查询流程
    App->>DB: SELECT 根据UID查询
    DB-->>App: 返回芯片数据
    
    Note over App,DB: 数据更新流程
    App->>DB: UPDATE 修改测试数据
    DB->>Trigger: 触发更新时间戳
    Trigger->>DB: 自动更新updated_at字段
    DB-->>App: 返回更新结果
    
    Note over App,DB: 数据删除流程
    App->>DB: DELETE 根据UID删除
    DB-->>App: 返回删除结果
```

## 查询优化建议

### 常用查询模式
1. **按UID精确查询** - 使用主键索引，性能最优
2. **按芯片型号查询** - 使用chip_model索引
3. **按批次ID查询** - 使用lotid索引
4. **按时间范围查询** - 使用created_at索引
5. **复合查询** - 可结合多个索引条件

### 性能优化策略
```mermaid
graph LR
    A[查询性能优化] --> B[合适的索引设计]
    A --> C[避免全表扫描]
    A --> D[使用预编译语句]
    A --> E[批量操作优化]
    
    B --> B1[主键索引]
    B --> B2[业务字段索引]
    B --> B3[时间字段索引]
    
    C --> C1[WHERE条件使用索引字段]
    C --> C2[避免SELECT *]
    C --> C3[合理使用LIMIT]
    
    D --> D1[QSqlQuery::prepare]
    D --> D2[参数绑定]
    D --> D3[重复执行优化]
    
    E --> E1[事务批处理]
    E --> E2[批量INSERT]
    E --> E3[批量UPDATE]
```

## 数据完整性约束

- **复合主键约束**: UID和批次ID的组合必须唯一
- **非空约束**: 芯片型号必须非空
- **默认值**: 测试结果默认为0(NG状态)
- **自动时间戳**: 创建和更新时间自动维护
- **触发器**: 自动更新updated_at字段

## 扩展建议

1. **数据归档**: 可考虑按时间分表存储历史数据
2. **统计表**: 可增加汇总统计表提升查询性能
3. **日志表**: 可增加操作日志表记录数据变更
4. **配置表**: 可增加参考阈值配置表便于管理
5. **预留字段使用**: param1-param10字段可用于存储自定义测试参数、工艺参数或其他扩展数据
