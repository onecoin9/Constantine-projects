### PostProcess 与 RunProcess 简要说明

#### 1) PostProcess 归档功能（关键点汇总）
- 目录层级：`archiveRoot/chipType/timestamp/testType-siteId/文件名文件夹/文件名`
- 示例：`C:/TestArchive/270/20241201_14_30_25/calibration-B01/Pitch_0deg_static/Pitch_0deg_static.txt`
- 重要配置：
  - `archiveRoot`（根目录，绝对路径）
  - `archiveBy`（上下文键，如 `chipType`，需先在 `setParameters` 写入）
  - `testType`（如 `calibration`/`test`）
  - `siteId`（如 `B01`）
  - `operation`：`move` 或 `copy`
- 行为：
  - 自动生成时间戳目录（`yyyyMMdd_HH_mm_ss`）
  - 自动拼接 `testType-siteId` 目录
  - 为每个结果文件创建“去扩展名”同名文件夹，并将该文件移动/复制进去
  - 将 `testType-siteId` 层目录的绝对路径保存到全局变量 `archiveBaseDir`

最小配置片段：
```json
{
  "type": "PostProcess",
  "config": {
    "setParameters": {"chipType": "270"},
    "archive": {
      "sourceDirectory": "output",
      "archiveRoot": "C:/TestArchive",
      "archiveBy": "chipType",
      "testType": "calibration",
      "siteId": "B01",
      "operation": "move"
    }
  }
}
```

读取全局路径：`GlobalItem::getInstance().getString("archiveBaseDir")`

常见问题：
- 未设置 `archiveRoot`/`archiveBy` → 步骤失败
- 上下文缺少 `archiveBy` 对应值（如 `chipType`）→ 步骤失败
- 源目录为空 → 记录警告并完成（跳过归档）

#### 2) RunProcess 外部进程调用（OK/FAILED 判定）
- 作用：调用外部 exe 并根据输出关键字或退出码判定成功。
- 常用参数：
  - `executable`（必填）
  - `args`（支持占位符 `${global.archiveBaseDir}` / `${context.archiveBaseDir}`）
  - `workingDirectory`, `timeoutMs`
  - `successPatterns`, `failurePatterns`（如 "OK" / "FAILED"）
  - `exitCodeAsSuccess`, `expectedExitCode`

最小配置片段：
```json
{
  "type": "RunProcess",
  "config": {
    "executable": "C:/Users/pc/Downloads/turnable/ChipTest_all.exe",
    "args": ["${global.archiveBaseDir}"],
    "workingDirectory": "C:/Users/pc/Downloads/turnable",
    "timeoutMs": 600000,
    "successPatterns": ["OK"],
    "failurePatterns": ["FAILED"]
  }
}
```

判定优先级：输出关键字 > 退出码（未配置关键字时默认按退出码，0 为成功）。


