# docs 目录索引（已整理）

- specs/
  - SoftwareRequirementsDocument.md（主SRD，持续更新版本）
  - images/（SRD 使用的配图与流程图）
- progress/
  - augustMind.md（进度日志，与个人目录的 augustMind 同步）
- workflow/
  - 工作流定义JSON（主流程与各温区子流程）
- scripts/
  - `sync_augustMind.ps1`：同步本地 augustMind 到仓库并提交
  - `archive_srd.ps1`：按日期归档SRD并提交
  - `cleanup_empty_dirs.ps1`：清理空目录
- templates/
  - 软件需求文档模板.md（SRD 模板）
- assets/
  - wallpaper/（壁纸与背景图片）
- misc/
  - chewingGum/（临时个人资料、小技巧与设置）

清理策略

- 禁止保留空目录；必要目录在首次加入内容后再保留
- 定期检查 `misc/`，将过期内容移出或归档至 `specs/archive/`

同步建议

- 本地 `d:\ConstantineFiles\...\augustMind.md` 建议以每日任务结束时同步到 `docs/progress/augustMind.md`
- 每日同步命令示例：
  - `powershell -NoProfile -ExecutionPolicy Bypass -File docs/scripts/sync_augustMind.ps1`
  - `powershell -NoProfile -ExecutionPolicy Bypass -File docs/scripts/archive_srd.ps1`
