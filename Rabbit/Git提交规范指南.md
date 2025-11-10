# Git 提交规范指南

> **版本**: 1.0  
> **更新日期**: 2025-11-07  
> **目的**: 规范项目代码提交流程，提高代码管理效率和可维护性

---

## 目录

1. [快速开始](#快速开始)
2. [提交信息格式](#提交信息格式)
3. [提交类型说明](#提交类型说明)
4. [作用域（Scope）](#作用域scope)
5. [主题（Subject）](#主题subject)
6. [正文（Body）](#正文body)
7. [页脚（Footer）](#页脚footer)
8. [完整示例](#完整示例)
9. [最佳实践](#最佳实践)
10. [常见问题](#常见问题)

---

## 快速开始

### 标准提交格式

```
<type>(<scope>): <subject>

<body>

<footer>
```

### 简单提交示例

```bash
git commit -m "feat(auth): 添加用户登录功能"
```

### 完整提交示例

```bash
git commit -m "fix(database): 修复连接池泄漏问题

修改连接池配置，限制最大连接数为 100
添加连接超时检测机制
实现自动重连逻辑

Fixes #123
Closes #456"
```

---

## 提交信息格式

提交信息由三个部分组成：

### 格式结构

```
<type>(<scope>): <subject>     <- 标题行（必需）
                               <- 空行
<body>                         <- 详细说明（可选）
                               <- 空行
<footer>                       <- 页脚信息（可选）
```

### 格式要求

| 部分 | 要求 | 字符数 |
|------|------|--------|
| **类型** | 必需，使用小写 | - |
| **作用域** | 可选，使用小括号包含 | - |
| **主题** | 必需，使用祈使句 | ≤ 50字 |
| **正文** | 可选，解释 what 和 why | 每行 ≤ 72字 |
| **页脚** | 可选，关闭 Issue 等 | - |

---

## 提交类型说明

### 主要类型

| 类型 | 说明 | 示例 |
|------|------|------|
| **feat** | 新增功能 | `feat(user): 添加用户注册功能` |
| **fix** | 修复 Bug | `fix(login): 修复登录超时问题` |
| **docs** | 文档更新 | `docs(readme): 更新安装指南` |
| **style** | 代码风格（不改逻辑） | `style(format): 调整代码缩进` |
| **refactor** | 代码重构 | `refactor(auth): 优化认证模块` |
| **perf** | 性能优化 | `perf(query): 优化数据库查询性能` |
| **test** | 测试相关 | `test(login): 添加登录单元测试` |
| **chore** | 构建、依赖等 | `chore(deps): 更新 npm 依赖` |
| **ci** | CI/CD 配置 | `ci(github): 配置自动化测试` |
| **revert** | 撤销提交 | `revert: 撤销上一次提交` |

### 类型详解

#### feat - 新增功能
```bash
git commit -m "feat(gui): 实现50个DUT动态网格显示

- 支持动态创建 DUT 网格布局
- 实现实时状态更新机制
- 添加状态颜色高亮显示"
```

#### fix - 修复 Bug
```bash
git commit -m "fix(tcp): 修复 TCP2COM 连接状态处理异常

问题：停止监听后出现内存泄漏
解决方案：
- 添加信号槽正确释放逻辑
- 完善线程退出机制
- 清理事件循环

Fixes #234"
```

#### refactor - 代码重构
```bash
git commit -m "refactor(handler): 拆分 HandlerDevice 模块

- 提取消息解析为独立类
- 降低模块耦合度
- 提高代码可测试性"
```

#### docs - 文档更新
```bash
git commit -m "docs(sop): 编写 Tester 安装部署文档"
```

---

## 作用域（Scope）

作用域指定提交影响的模块或组件范围。

### 作用域命名规则

1. **使用小写** - 全部小写字母
2. **使用连字符** - 多个单词用 `-` 分隔
3. **保持简洁** - 2-3 个单词最佳

### 常见作用域示例

```
auth          # 认证模块
database      # 数据库模块
api           # API 接口
ui            # 用户界面
config        # 配置管理
build         # 构建系统
deps          # 依赖管理
docs          # 文档
```

### 项目专用作用域示例

对于 JingCun SLT Tester 项目：

```
tester        # Tester 主程序
gui           # GUI 界面
logger        # 日志系统
dut           # DUT 设备管理
temperature   # 温度控制
workflow      # 工作流引擎
ftp           # FTP 传输
```

---

## 主题（Subject）

主题是提交的简要描述。

### 主题要求

1. **使用祈使句** - "添加"而非"已添加"
2. **不超过 50 字** - 首行字符数限制
3. **不以句号结尾** - 避免末尾标点
4. **首字母大写** - 保持格式一致

### ✅ 正确示例

```
feat(gui): 实现 50 个 DUT 的动态网格显示
fix(temp): 修复温度监测异常
docs(readme): 更新安装说明
refactor(logger): 优化日志输出性能
```

### ❌ 错误示例

```
gui: 实现了50个DUT的动态网格显示。        # 句号、过长、"已"字
temp: Fixed temperature monitoring issue   # 英文、被动语态
update docs.                               # 没有类型和作用域
```

---

## 正文（Body）

正文提供提交的详细说明，解释 **what** 和 **why**，不要说 **how**。

### 正文要求

1. **分段清晰** - 使用空行分隔逻辑段落
2. **每行不超过 72 字** - 便于阅读和 Git 显示
3. **说明动机** - 解释为什么做这个改动
4. **列出关键改动** - 使用 bullet points

### 正文格式示例

```
# 格式 1：段落式

修改连接池配置以解决内存泄漏问题。
配置最大连接数为 100，设置连接超时为 30 秒。
实现了自动重连机制，确保连接可靠性。

# 格式 2：要点式

- 限制连接池最大连接数
- 添加连接超时检测机制
- 实现自动重连逻辑
- 改进错误日志记录
```

### 完整正文示例

```
feat(dut): 实现 DUT 日志存储功能

按照测试批次、Rack 和 Slot 层级组织日志文件夹结构：
- JC App 安装目录/Logs/批次号/Rack/Slot/DUT编号/

支持 16 个 DUT 对应的独立测试日志文件：
- 日志文件命名格式：DUT_编号_时间戳.log
- 包含测试步骤、参数、结果等完整信息
- 支持日志压缩和自动清理

此改动为后续的测试数据分析奠定基础。
```

---

## 页脚（Footer）

页脚用于关闭 Issue、提及相关问题或说明破坏性改动。

### 页脚格式

```
Fixes #123
Closes #456
Refs #789
BREAKING CHANGE: 描述破坏性改动
```

### 页脚示例

#### 关闭 Issue

```
git commit -m "fix(auth): 修复登录超时问题

添加自动重连机制，增加超时时间到 60 秒。

Fixes #123"
```

#### 多个 Issue

```
Fixes #123
Closes #456
Refs #789"
```

#### 破坏性改动

```
BREAKING CHANGE: 修改了 API 接口格式

GET /api/dut/{id} 返回值结构已更改：
- 旧格式：{id, name, status}
- 新格式：{id, name, state, timestamp}

用户需要更新调用代码。
```

---

## 完整示例

### 示例 1：新增功能

```
feat(gui): 实现温度统计 Tooltip 显示

优化用户界面的温度显示方式：

- 完善 Tooltip 格式化显示
  - 包含温度值、阈值、状态等信息
  - 实现温度变化趋势指示（↑/↓/→）
  - 改进 Tooltip 颜色方案

- UI/UX 改进
  - 优化字体大小和间距
  - 提高对比度，便于识别
  - 异常温度高亮显示

此改动提升了用户体验，便于快速了解温度状态。

Refs #456
```

### 示例 2：Bug 修复

```
fix(tcp2com): 修复停止监听后连接状态异常

问题描述：
停止监听后会出现逻辑处理错误，导致内存泄漏。
表现为：客户端连接状态处理异常，可能导致事件未正确清理。

修复措施：
- 验证信号槽连接正确释放
- 改进线程退出机制
- 完善事件循环清理逻辑
- 添加详细的调试日志

Fixes #234
```

### 示例 3：代码重构

```
refactor(handler): 拆分 HandlerDevice 模块并降低耦合度

模块拆分：
- 提取 Lotend 消息解析为独立的 LotendMessageParser 类
- 提取设备状态管理为 DeviceStateManager 类
- 分离异常恢复逻辑到 ExceptionRecoveryHandler 类

质量提升：
- 模块耦合度显著降低，单一职责原则更清晰
- 代码可测试性提高 40%
- 新增单元测试 15 个，集成测试 5 个
- 代码可维护性提升 30%

验证：
所有现有测试通过，无 Regression。
```

### 示例 4：文档更新

```
docs(sop): 编写 Tester 系统安装部署文档

新增文档内容：
1. 系统要求和硬件配置
2. 环境变量和依赖安装步骤
3. 应用配置和数据库初始化
4. 常见问题与故障排查
5. 维护和升级指南

文档包含详细的图解和步骤说明，便于新用户快速上手。

Related to #789
```

---

## 最佳实践

### ✅ DO - 应该做

1. **原子性提交** - 每个提交完成一个独立功能
   ```bash
   git commit -m "feat(auth): 添加用户登录功能"
   git commit -m "test(auth): 添加登录单元测试"
   git commit -m "docs(auth): 更新认证 API 文档"
   ```

2. **清晰的提交历史** - 便于代码审查和问题追踪
   ```bash
   # 好的历史
   - feat: 添加用户注册
   - fix: 修复邮件发送问题
   - docs: 更新 README
   
   # 避免
   - wip
   - 修复
   - asdf123
   ```

3. **经常提交** - 每个完整功能单元进行一次提交
   ```bash
   每天应该有 2-5 个有意义的提交
   ```

4. **提交前检查** - 使用 `git diff` 检查改动
   ```bash
   git diff           # 查看未暂存的改动
   git diff --staged  # 查看已暂存的改动
   ```

### ❌ DON'T - 不应该做

1. **不要混合无关的改动**
   ```bash
   # ❌ 错误：一个提交包含多个无关功能
   git commit -m "fix: 修复各种问题"
   
   # ✅ 正确：分别提交
   git commit -m "fix(auth): 修复登录问题"
   git commit -m "fix(api): 修复接口超时"
   ```

2. **不要提交不完整的代码**
   ```bash
   # ❌ 避免提交未测试的代码
   # ✅ 提交前：运行测试，确保代码可用
   ```

3. **不要过度详细的标题**
   ```bash
   # ❌ 太长
   git commit -m "feat(gui): 实现了用户界面的所有功能包括网格显示、状态更新、颜色高亮等"
   
   # ✅ 简洁
   git commit -m "feat(gui): 实现 50 个 DUT 的动态网格显示"
   ```

4. **不要使用含糊的类型**
   ```bash
   # ❌ 避免
   update
   fix stuff
   changes
   
   # ✅ 使用标准类型
   feat, fix, refactor, docs
   ```

---

## 最佳实践操作流程

### 标准开发流程

```bash
# 1. 创建特性分支
git checkout -b feature/user-login

# 2. 开发功能
# ... 编辑代码 ...

# 3. 检查改动
git status
git diff

# 4. 暂存改动
git add src/auth/login.ts

# 5. 提交改动
git commit -m "feat(auth): 实现用户登录功能

- 支持邮箱和用户名登录
- 实现记住我功能
- 添加登录失败重试限制"

# 6. 查看提交历史
git log --oneline

# 7. 推送到远程
git push origin feature/user-login

# 8. 创建 Pull Request 进行代码审查
```

### 修改最后一次提交

```bash
# 修改提交信息
git commit --amend -m "新的提交信息"

# 添加遗漏的文件
git add forgotten_file.js
git commit --amend --no-edit

# 修改提交者信息
git commit --amend --author="John Doe <john@example.com>"
```

### 交互式变基（Rebase）

```bash
# 合并最后 3 个提交
git rebase -i HEAD~3

# 在编辑器中将后续 pick 改为 squash (s)
# pick abc123 feat: 功能 1
# s def456 feat: 功能 2
# s ghi789 feat: 功能 3
```

---

## 常见问题

### Q1：提交信息过长怎么办？

**A:** 使用多行提交：

```bash
# 使用 git commit 打开编辑器（不带 -m）
git commit

# 编辑器会打开，按照格式填写多行
# 保存后自动提交
```

### Q2：忘记了提交格式怎么办？

**A:** 查看项目历史提交：

```bash
# 查看最近 10 个提交
git log --oneline -10

# 查看某个提交的详细内容
git show abc123
```

### Q3：提交错了怎么办？

**A:** 根据情况选择：

```bash
# 未推送到远程 - 修改本地提交
git commit --amend

# 已推送到远程 - 创建撤销提交
git revert abc123

# 完全撤销改动（谨慎使用）
git reset --hard HEAD~1
```

### Q4：什么时候应该提交？

**A:** 提交的黄金法则：

- ✅ 功能完整可用
- ✅ 代码已测试
- ✅ 不存在 WIP（Work In Progress）代码
- ✅ 一个提交对应一个逻辑改动

### Q5：提交信息应该用中文还是英文？

**A:** 建议：

- **统一语言** - 同一项目保持一致
- **优先英文** - 开源项目推荐使用英文
- **允许中文** - 内部项目可使用中文（更清晰）
- **本项目** - 推荐使用中文，便于团队沟通

---

## 快速参考表

### 类型速查表

```
feat      新增功能
fix       修复 Bug
refactor  代码重构
perf      性能优化
docs      文档更新
style     代码风格
test      测试相关
chore     依赖更新
ci        CI/CD 配置
revert    撤销提交
```

### 命令速查表

```bash
# 查看状态
git status

# 查看改动
git diff
git diff --staged

# 暂存文件
git add <file>
git add .

# 提交改动
git commit -m "message"
git commit -am "message"  # 跳过 git add

# 修改提交
git commit --amend

# 查看历史
git log
git log --oneline
git log --graph --oneline --all

# 撤销改动
git reset HEAD <file>
git revert <commit>
```

---

## 总结

遵循 Git 提交规范的好处：

- 📝 **清晰的提交历史** - 便于追踪代码变更
- 🔍 **快速定位问题** - 通过提交信息快速找到相关改动
- 👥 **提高团队效率** - 统一格式便于团队协作
- 📊 **自动化工具** - 支持基于提交类型生成更新日志
- 🎯 **代码审查** - 规范的提交信息便于 Code Review

---

## 相关资源

- [Conventional Commits](https://www.conventionalcommits.org/)
- [Angular Commit Guidelines](https://github.com/angular/angular/blob/master/CONTRIBUTING.md#commit)
- [Git Book - Recording Changes](https://git-scm.com/book/en/v2/Git-Basics-Recording-Changes-to-the-Repository)

---

**文档维护者**: 项目管理团队  
**最后更新**: 2025-11-07  
**版本**: 1.0
