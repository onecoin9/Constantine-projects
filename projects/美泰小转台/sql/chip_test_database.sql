-- 芯片测试数据库表结构定义
-- Chip Test Database Schema Definition

CREATE TABLE IF NOT EXISTS chip_test_data (
    uid TEXT NOT NULL,                                -- 芯片uid (复合主键一部分)
    chip_model TEXT NOT NULL,                         -- 芯片型号
    lotid TEXT NOT NULL,                              -- 批次ID (复合主键一部分)  
    activation_time TEXT,                             -- 激活时间
    marking_number TEXT,                              -- 打标号
    
    -- 工作电流 Working Current
    working_current_measured REAL,                    -- 工作电流测量值
    working_current_reference REAL,                   -- 工作电流参考阈值
    
    -- 锁相环直流电压 Phase Locked Loop DC Voltage  
    pll_dc_voltage_measured REAL,                    -- 锁相环直流电压测量值
    pll_dc_voltage_reference REAL,                   -- 锁相环直流电压参考阈值
    
    -- 驱动直流电压 Drive DC Voltage
    drive_dc_voltage_measured REAL,                  -- 驱动直流电压测量值
    drive_dc_voltage_reference REAL,                 -- 驱动直流电压参考阈值
    
    -- 驱动直流电压峰峰值 Drive DC Peak Voltage
    drive_dc_peak_voltage_measured REAL,             -- 驱动直流电压峰峰值测量值
    drive_dc_peak_voltage_reference REAL,            -- 驱动直流电压峰峰值参考阈值
    
    -- 方波频率 Square Wave Frequency
    square_wave_freq_measured REAL,                  -- 方波频率测量值
    square_wave_freq_reference REAL,                 -- 方波频率参考阈值
    
    -- 正弦波电压均值 Sine Wave Voltage Average
    sine_wave_voltage_avg_measured REAL,             -- 正弦波电压均值测量值
    sine_wave_voltage_avg_reference REAL,            -- 正弦波电压均值参考阈值
    
    -- 正弦波电压峰峰值 Sine Wave Peak Voltage
    sine_wave_peak_voltage_measured REAL,            -- 正弦波电压峰峰值测量值
    sine_wave_peak_voltage_reference REAL,           -- 正弦波电压峰峰值参考阈值
    
    -- 正弦波频率 Sine Wave Frequency
    sine_wave_freq_measured REAL,                    -- 正弦波频率测量值
    sine_wave_freq_reference REAL,                   -- 正弦波频率参考阈值
    
    -- 测试结果 Test Results
    activation_ok_ng INTEGER DEFAULT 0,              -- 激活ok/ng? (0=NG, 1=OK)
    turntable_calibration_ok_ng INTEGER DEFAULT 0,   -- 小转台标定ok/ng? (0=NG, 1=OK)
    
    -- 设备和文件信息 Device and File Information
    machine_number TEXT,                              -- 机器号
    workflow_file TEXT,                               -- 工作流文件
    automation_task_file TEXT,                        -- 自动化任务文件
    burn_task_file TEXT,                              -- 烧录任务文件
    
    -- OS测试相关 OS Test Related
    os_test_result INTEGER DEFAULT 0,                -- OS测试结果 (0=NG, 1=OK)
    os_test_details TEXT,                             -- OS测试详细信息
    calibration_params TEXT,                          -- 标定参数
    
    -- 预留参数字段 Reserved Parameter Fields
    param1 TEXT,                                      -- 预留参数1
    param2 TEXT,                                      -- 预留参数2
    param3 TEXT,                                      -- 预留参数3
    param4 TEXT,                                      -- 预留参数4
    param5 TEXT,                                      -- 预留参数5
    param6 TEXT,                                      -- 预留参数6
    param7 TEXT,                                      -- 预留参数7
    param8 TEXT,                                      -- 预留参数8
    param9 TEXT,                                      -- 预留参数9
    param10 TEXT,                                     -- 预留参数10
    
    -- 记录时间戳 Record Timestamp
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    
    -- 复合主键约束
    PRIMARY KEY (uid, lotid)
);

-- 创建索引以提升查询性能
CREATE INDEX IF NOT EXISTS idx_chip_model ON chip_test_data(chip_model);
CREATE INDEX IF NOT EXISTS idx_lotid ON chip_test_data(lotid);
CREATE INDEX IF NOT EXISTS idx_activation_time ON chip_test_data(activation_time);
CREATE INDEX IF NOT EXISTS idx_created_at ON chip_test_data(created_at);

-- 创建触发器自动更新updated_at字段
CREATE TRIGGER IF NOT EXISTS update_chip_test_data_updated_at
    AFTER UPDATE ON chip_test_data
    FOR EACH ROW
BEGIN
    UPDATE chip_test_data 
    SET updated_at = CURRENT_TIMESTAMP 
    WHERE uid = NEW.uid AND lotid = NEW.lotid;
END;