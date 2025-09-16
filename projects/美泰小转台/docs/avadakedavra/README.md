### 简要说明（PostProcess 与 RunProcess 增强）

#### 1) PostProcess 归档结构
- 最终目录：`archiveRoot/chipType/timestamp/testType-siteId/文件名文件夹/文件名`
- 示例：`C:/TestArchive/270/20241201_14_30_25/calibration-B01/Pitch_0deg_static/Pitch_0deg_static.txt`
- 从配置读取：`archiveRoot`, `archiveBy(=chipType)`, `testType`, `siteId`；`operation` 支持 `move/copy`。
- 额外：创建 `testType-siteId` 目录后，会将该绝对路径保存到全局变量 `archiveBaseDir`。

最小配置片段（工作流步骤）：
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

#### 2) RunProcess 外部进程调用
- 新增步骤类型：`RunProcess`，用于调用外部 exe，并按输出关键字或退出码判定成功。
- 常用参数：
  - `executable`（必填）
  - `args`（支持 `${global.archiveBaseDir}` 占位符）
  - `workingDirectory`, `timeoutMs`
  - `successPatterns`, `failurePatterns`（如：OK/FAILED）
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


