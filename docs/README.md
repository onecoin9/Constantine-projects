# docs 目录索引

- SoftwareRequirementsDocument.md
  - 主SRD（持续更新版本）
- augustMind.md
  - 进度日志，与个人目录的 augustMind 同步（仓库跟踪版本）
- workflow/
  - 工作流定义JSON（主流程与各温区子流程）
- examples/
  - CSV 示例：`DUT-X-CaliSample.csv`、`DUT-X-TestSample.csv`、`DUT-X-OTPResult.csv`
- guides/
  - 预留：SOP、设计文档
- scripts/
  - `sync_augustMind.ps1`：同步本地 augustMind 到仓库并提交
  - `archive_srd.ps1`：按日期归档SRD并提交
- archive/
  - 历史归档：按日期建立子目录，保存关键版本的SRD/重要文档
  - 示例：archive/2025-08-08/SoftwareRequirementsDocument.md

归档规则
- 每次关键变更或里程碑达成时，将SRD复制到 `archive/YYYY-MM-DD/`
- 命名保持与原文件一致，方便比对
- 仅归档定版或阶段版本，日常小改无需归档

同步建议
- 本地 `d:\ConstantineFiles\...\augustMind.md` 建议以每日任务结束时同步到 `docs/augustMind.md`
- 每日同步命令示例：
  - `powershell -NoProfile -ExecutionPolicy Bypass -File docs/scripts/sync_augustMind.ps1`
  - `powershell -NoProfile -ExecutionPolicy Bypass -File docs/scripts/archive_srd.ps1`
