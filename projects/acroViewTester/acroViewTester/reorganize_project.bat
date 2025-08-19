@echo off
chcp 65001 > nul
echo 正在优化项目文件结构...

REM 创建新的顶层目录
mkdir third_party
mkdir build
mkdir tools
mkdir db

echo 创建完整的子目录结构...
mkdir resources\data
mkdir resources\qrc
mkdir docs\references
mkdir third_party\plugins
mkdir build\generated
mkdir build\x64

echo 移动文件夹到优化位置...

REM 移动第三方和依赖库
echo 移动第三方库...
xcopy /E /I /Y AutomaticManager third_party\AutomaticManager
rmdir /S /Q AutomaticManager
xcopy /E /I /Y depend third_party\depend
rmdir /S /Q depend
xcopy /E /I /Y Plugins third_party\plugins
rmdir /S /Q Plugins

REM 移动资源和数据
echo 移动资源文件...
xcopy /E /I /Y data resources\data
rmdir /S /Q data
xcopy /E /I /Y qrc resources\qrc
rmdir /S /Q qrc
xcopy /E /I /Y reference docs\references
rmdir /S /Q reference

REM 移动构建和生成文件
echo 移动构建文件...
xcopy /E /I /Y GeneratedFiles build\generated
rmdir /S /Q GeneratedFiles
xcopy /E /I /Y x64 build\x64
rmdir /S /Q x64

REM 移动工具类和数据库文件
echo 移动工具和数据库文件...
xcopy /E /I /Y Utils src\utils\legacy
rmdir /S /Q Utils
xcopy /E /I /Y file src\utils\file\legacy
rmdir /S /Q file
xcopy /E /I /Y sqllite db
rmdir /S /Q sqllite

echo 移动完成，清理临时文件...
if exist acroview_2025_05_07_15_23_54.dmp move acroview_2025_05_07_15_23_54.dmp logs\

echo 创建README文件...
> README.md (
echo # acroViewTester 项目结构
echo.
echo ## 文件夹结构说明
echo.
echo - src/: 所有源代码
echo   - core/: 核心功能
echo   - ui/: 用户界面
echo   - network/: 网络通信
echo   - utils/: 工具类
echo - include/: 公共头文件
echo - resources/: 资源文件
echo   - data/: 数据文件
echo   - qrc/: Qt资源
echo - tests/: 测试用例
echo - docs/: 文档
echo   - references/: 参考资料
echo - third_party/: 第三方库
echo   - AutomaticManager/: 自动化管理器
echo   - depend/: 依赖库
echo   - plugins/: 插件
echo - build/: 构建输出
echo   - generated/: 自动生成的文件
echo   - x64/: x64位构建输出
echo - tools/: 工具脚本
echo - db/: 数据库文件
echo - logs/: 日志文件
echo.
echo ## 注意事项
echo.
echo 项目文件已根据功能和类型进行了重组，请相应更新项目文件和包含路径。
)

echo 优化完成!
echo 请手动更新项目文件和包含路径。