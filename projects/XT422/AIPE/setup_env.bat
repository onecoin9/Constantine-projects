@echo off
chcp 65001 >nul

set "ProjPath=%cd%"
set "DestPath=%ProjPath%\Build"
set "SourcePath=%ProjPath%\Aprog"
set "ProjName=%ProjPath%\Build\Aprog.exe"

REM 检查 build 目录是否存在，存在则删除
if exist "%DestPath%" (
    rmdir /s /q "%DestPath%"
    echo 已删除 %DestPath% 目录
) else (
    echo %DestPath% 目录不存在，跳过删除
)

REM 设置Automatic编译目录  
for %%i in ("%~dp0..\Tools\AutomaticPlugin") do set "AutomaticPlugin_DIR=%%~fi"

set "AutomaticPlugin=%AutomaticPlugin_DIR%\build_solution.bat"
call %AutomaticPlugin%

if %ERRORLEVEL% neq 0 (
    echo 第一个构建脚本失败，退出...
    exit /b %ERRORLEVEL%
)

call build_aprog_solution.bat
echo 子脚本的退出码是：%ERRORLEVEL%

if %ERRORLEVEL% equ 0 (
    echo 搭建环境编译执行成功，继续执行...
    rem 此处写后续逻辑
) else (
    echo 搭建环境编译执行失败（错误码: %ERRORLEVEL%），退出！
	pause
    exit /b %ERRORLEVEL%  
	rem 可选：将错误码传递给父级调用者
)

call "%SourcePath%\rccBuild.bat"

rem "创建必要的目录"
echo "创建必要的目录..."
MKDIR "%DestPath%\data"
MKDIR "%DestPath%\Drv"
MKDIR "%DestPath%\Mst"
MKDIR "%DestPath%\LocalDB"
MKDIR "%DestPath%\Fonts"
MKDIR "%DestPath%\Translations"
MKDIR "%DestPath%\Plugins"
MKDIR "%DestPath%\Firmware"
MKDIR "%DestPath%\Tools"
MKDIR "%DestPath%\Lic"

XCOPY  "%SourcePath%\BackupFiles" "%DestPath%" /E
XCOPY  "%SourcePath%\*.rcc" "%DestPath%"
XCOPY  "%SourcePath%\LocalDB" "%DestPath%\LocalDB" /E
XCOPY  "%SourcePath%\data" "%DestPath%\data" /E
XCOPY  "%SourcePath%\Drv" "%DestPath%\Drv" /E
XCOPY  "%SourcePath%\Mst" "%DestPath%\Mst" /E
XCOPY  "%SourcePath%\Fonts" "%DestPath%\Fonts" /E
XCOPY  "%SourcePath%\Translations" "%DestPath%\Translations" /E
robocopy  "%SourcePath%\Plugins" "%DestPath%\Plugins" /IS
XCOPY  "%SourcePath%\Tools" "%DestPath%\Tools" /E
XCOPY  "%SourcePath%\Depend\sqlite3\*.dll" "%DestPath%"
XCOPY  "%SourcePath%\Depend\QtPropertyBrowser\QtSolutions_PropertyBrowser-head.dll" "%DestPath%"
XCOPY  "%SourcePath%\Depend\ACComlib\ACComLib_x64.dll" "%DestPath%"
XCOPY  "%SourcePath%\Lic" "%DestPath%\Lic" /E

rem XCOPY  "%DestPath%" %cd%\Build /E
windeployqt --no-translations %ProjName%

robocopy  "%DestPath%\styles" "%DestPath%\Plugins\styles" /IS
RD /S /Q "%DestPath%\styles"
robocopy  "%DestPath%\platforms" "%DestPath%\Plugins\platforms" /IS
RD /S /Q "%DestPath%\platforms"
robocopy  "%DestPath%\imageformats" "%DestPath%\Plugins\imageformats" /IS
RD /S /Q "%DestPath%\imageformats"
robocopy  "%DestPath%\iconengines" "%DestPath%\Plugins\iconengines" /IS
RD /S /Q "%DestPath%\iconengines"
RD /S /Q "%DestPath%\bearer"

for /f "delims=" %%i in ('svn info ./ ^| findstr "Rev:"') do set svn_version=%%i
set code=%svn_version:~18%

if defined code (
    echo Current SVN version is: %code%
) else (
	echo Failed to Get SVN version
)

:: 设置 Python 脚本路径和参数
set "python_script=%SourcePath%\Depend\scripts\modifyVersion.py"
set "xml_file=%SourcePath%\version.xml"
set "node_path=Client"
set "attribute_name=version"
set "new_value=%code%"

:: 调用 Python 脚本
python %python_script% %xml_file% %node_path% %attribute_name% %new_value%
XCOPY  "%SourcePath%\version.xml" "%DestPath%"

:: 比较拷贝的文件数量是否正确
set "python_compare_script=%SourcePath%\Depend\scripts\compare_files.py"

REM 设置文本文件路径
set FILE_LIST=file_list.txt

REM 调用 Python 脚本
python "%python_compare_script%" "%FILE_LIST%" "%DestPath%"
echo "Build environment completed"
pause
