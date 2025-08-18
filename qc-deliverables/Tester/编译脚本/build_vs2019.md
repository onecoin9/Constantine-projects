# build_vs2019.py 使用说明

本文档介绍如何使用项目根目录下的 `build_vs2019.py` 脚本，通过 Visual Studio 2019 的 MSBuild（优先）或 `devenv.com` 来编译解决方案 `doorPressureTester/doorPressureTester.sln`。

## 环境要求
- Windows 系统
- 已安装 Visual Studio 2019（16.x），包含 MSBuild 组件
- Python 3.8 及以上（建议）
- 建议安装 `vswhere.exe`（通常随 Visual Studio Installer 一起安装），用于自动定位 VS 安装路径

## 文件位置
- 脚本：`build_vs2019.py`（仓库根目录）
- 默认解决方案：`doorPressureTester/doorPressureTester.sln`

## 快速开始
- PowerShell（推荐）
```powershell
python .\build_vs2019.py
```
- CMD
```bat
python build_vs2019.py
```
以上等价于默认 Debug 配置、x64 平台、执行 Build 目标。

## 常用示例
- 指定 Release/x64，重建（Rebuild），并行 8 任务，输出日志：
```powershell
python .\build_vs2019.py \
  --solution doorPressureTester\doorPressureTester.sln \
  --configuration Release \
  --platform x64 \
  --target Rebuild \
  --parallel 8 \
  --log build.log
```
- 清理：
```powershell
python .\build_vs2019.py --target Clean
```
- 跳过引用项目联编（更快）：
```powershell
python .\build_vs2019.py --no-ref-build
```
- 构建 Win32：
```powershell
python .\build_vs2019.py --platform Win32
```

## 命令行参数
- `--solution`：解决方案路径，默认 `doorPressureTester/doorPressureTester.sln`
- `--configuration`：`Debug`/`Release`，默认 `Debug`
- `--platform`：`x64`/`Win32`，默认 `x64`
- `--target`：`Build`/`Rebuild`/`Clean`，默认 `Build`
- `--parallel`：MSBuild 并行度（等价 `/m`），默认 CPU 核心数
- `--log`：日志文件路径（可选），同时输出到控制台与文件
- `--no-ref-build`：向 MSBuild 传递 `BuildProjectReferences=false`，跳过引用项目联编

## 工作机制
1. 使用 `vswhere.exe` 自动查找 VS2019 安装位置并定位 `MSBuild.exe`
2. 若未找到，尝试常见安装目录；仍未找到则退化为调用 `devenv.com`
3. 将构建过程实时输出到控制台（可选同时写入 `--log`）

> 说明：脚本不修改工程的输出目录设置，最终 exe 路径由各 `*.vcxproj` 的 `OutDir`、`TargetName` 等决定。

## 产物位置
- 受 VS 项目属性“输出目录”等设置影响
- `compile.bat` 的提示可能类似：`build\Debug\x64\doorPressureTester.exe`
- 可在 `doorPressureTester\doorPressureTester.vcxproj` 中查阅 `<OutDir>`/`<TargetName>`

## 常见问题
- 找不到 MSBuild/VS2019：确认 VS2019 与 MSBuild 组件已安装，`vswhere.exe` 可用；或将 `MSBuild.exe`/`devenv.com` 加入 PATH
- 平台/配置不存在：确保解决方案中存在对应 `x64/Win32` 配置
- 非零退出码：表示构建失败；请根据控制台或日志信息排查

## 与 `compile.bat` 的区别
- `compile.bat`：依赖固定路径与环境，适合本机快速构建
- `build_vs2019.py`：自动发现 VS 安装位置、参数更灵活，适合多机器和 CI 环境

## 扩展建议
- 构建成功后自动复制 exe 至 `deploy`、打包、签名、写入版本号等，可按需在 `build_vs2019.py` 中扩展
