### PostProcessStep 归档功能改动与使用说明

#### 1. 改动概述
- 新增归档目录层级：`archiveRoot/chipType/timestamp/testType-siteId/文件名文件夹/文件名`
- 支持从配置读取 `testType` 与 `siteId`，用于构建 `testType-siteId` 目录。
- 为每个测试结果文件（如 `Pitch_0deg_static.txt`）自动创建同名文件夹（去扩展名），并将文件移动/复制至该文件夹下。
- 保留原有 `archiveBy` 机制（如 `chipType` 从上下文读取），并支持 `move/copy` 两种操作。
- 新增将 `testType-siteId` 层级的绝对路径保存到全局变量：`archiveBaseDir`（可供后续步骤或外部模块使用）。

对应代码文件：`src/application/PostProcessStep.cpp`

目录示例：
```
C:/TestArchive/
└── 270/                                  # chipType（来自 setParameters → context）
    └── 20241201_14_30_25/                # 时间戳（yyyyMMdd_HH_mm_ss）
        └── calibration-B01/              # testType-siteId
            ├── Pitch_0deg_static/
            │   └── Pitch_0deg_static.txt
            ├── Pitch_180deg_move/
            │   └── Pitch_180deg_move.txt
            └── ...
```

#### 2. 配置使用
在工作流 `PostProcess` 步骤中配置 `archive` 字段：
```json
{
  "id": "final_archive",
  "type": "PostProcess",
  "config": {
    "setParameters": {
      "chipType": "270"
    },
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

参数说明：
- `sourceDirectory`：源目录（如前置步骤输出 `output`）
- `archiveRoot`：归档根目录（绝对路径）
- `archiveBy`：用于第一级子目录名的上下文键（如 `chipType`）
- `testType`：测试类型（如 `calibration`、`test`、`validation`）
- `siteId`：站点标识（如 `B01`）
- `operation`：`move` 或 `copy`

行为说明：
- 时间戳目录自动生成：`yyyyMMdd_HH_mm_ss`
- `testType-siteId` 目录自动拼接：例如 `calibration-B01`
- 文件名目录自动生成：对每个文件取去扩展名作为子目录名，例如 `Pitch_0deg_static.txt` → `Pitch_0deg_static/`
- 全局变量：在创建 `testType-siteId` 目录后，写入 `GlobalItem::setString("archiveBaseDir", <绝对路径>)`。

#### 3. 依赖与前提
- `archiveBy` 对应的值（如 `chipType`）需已通过 `setParameters` 或其他步骤设置到上下文（`WorkflowContext`）。
- 源目录应存在可归档的文件；若不存在，步骤将完成但会输出提示日志。

#### 4. 日志与错误处理
- 成功示例日志：`文件归档: 'Pitch_0deg_static.txt' -> '.../Pitch_0deg_static/Pitch_0deg_static.txt'`
- 关键错误：
  - 缺少 `archiveRoot`/`archiveBy`
  - 上下文中缺少 `archiveBy` 对应值（如 `chipType`）
  - 目标目录创建失败
  - 文件移动/复制失败

#### 5. 常见问题（FAQ）
- Q：是否必须配置 `testType` 和 `siteId`？
  - A：建议配置。若未配置，内置默认值：`testType = "test"`，`siteId = "A01"`。
- Q：是否支持只复制不移动？
  - A：支持，将 `operation` 设为 `copy` 即可。
- Q：文件夹名从哪里来？
  - A：从文件名去扩展名自动生成（无需额外配置）。
- Q：是否能获取到“testType-siteId”这一层的绝对路径？
  - A：可以，变量键名为 `archiveBaseDir`，通过 `GlobalItem::getInstance().getString("archiveBaseDir")` 读取。

#### 6. 变更记录
- 2025-09-16：
  - 新增 `testType`/`siteId` 支持
  - 文件夹层级扩展至 `archiveRoot/chipType/timestamp/testType-siteId/文件名文件夹/文件名`
  - 为每个结果文件自动创建同名目录


