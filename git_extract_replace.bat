@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo === Git文件提取和替换工具 ===
echo 此工具将从git HEAD版本提取指定文件/文件夹并覆盖本地版本
echo.

:: 扫描当前目录下的所有文件夹和文件
echo 正在扫描当前目录...
echo.

set count=0
set "items="

:: 先列出文件夹
for /d %%D in (*) do (
    set /a count+=1
    set "item[!count!]=%%D"
    set "items=!items! !count!"
    echo !count!. [文件夹] %%D
)

:: 再列出文件
for %%F in (*) do (
    if not "%%~xF"==".bat" (
        set /a count+=1
        set "item[!count!]=%%F"
        set "items=!items! !count!"
        echo !count!. [文件] %%F
    )
)

if !count! equ 0 (
    echo 错误：当前目录下没有可选择的文件或文件夹
    pause
    exit /b 1
)

echo.
echo 0. 手动输入路径
echo.

:: 获取用户选择
set /p user_choice="请输入序号（多个序号用空格分隔，或输入0手动输入路径）: "

if "%user_choice%"=="" (
    echo 错误：请输入至少一个选项
    pause
    exit /b 1
)

:: 处理用户选择
set "user_input="

for %%C in (%user_choice%) do (
    if "%%C"=="0" (
        set /p manual_input="请输入要提取的文件或文件夹路径（多个路径用空格分隔）: "
        set "user_input=!user_input! !manual_input!"
    ) else (
        if defined item[%%C] (
            set "user_input=!user_input! !item[%%C]!"
        ) else (
            echo 警告：序号 %%C 无效，已跳过
        )
    )
)

:: 去除首尾空格
for /f "tokens=* delims= " %%A in ("!user_input!") do set "user_input=%%A"

if "%user_input%"=="" (
    echo 错误：没有有效的路径
    pause
    exit /b 1
)

echo.
echo 将要提取的路径: %user_input%
echo.

:: 设置变量
set "zip_filename=gitSource.zip"
set "temp_dir=temp_extract"

echo.
echo 当前工作目录: %CD%
echo 压缩包文件: %zip_filename%
echo.

:: 删除已存在的zip文件
if exist "%zip_filename%" (
    echo 删除已存在的压缩包: %zip_filename%
    del "%zip_filename%"
)

:: 删除临时目录
if exist "%temp_dir%" (
    rmdir /s /q "%temp_dir%"
)

:: 执行git archive命令
set "git_command=git archive --format=zip --output=%zip_filename% HEAD %user_input%"
echo 执行命令: %git_command%

%git_command%

if %errorlevel% neq 0 (
    echo git archive 执行失败
    pause
    exit /b 1
)

:: 检查zip文件是否生成
if not exist "%zip_filename%" (
    echo 错误：压缩包文件未生成: %zip_filename%
    pause
    exit /b 1
)

echo ✓ 成功创建压缩包: %zip_filename%

:: 使用WinRAR解压文件
echo.
echo 开始使用WinRAR解压并覆盖本地文件...

:: 查找WinRAR路径
set "winrar_exe="
if exist "C:\Program Files\WinRAR\WinRAR.exe" (
    set "winrar_exe=C:\Program Files\WinRAR\WinRAR.exe"
) else if exist "C:\Program Files (x86)\WinRAR\WinRAR.exe" (
    set "winrar_exe=C:\Program Files (x86)\WinRAR\WinRAR.exe"
) else if exist "C:\WinRAR\WinRAR.exe" (
    set "winrar_exe=C:\WinRAR\WinRAR.exe"
)

if not defined winrar_exe (
    echo 错误：未找到WinRAR.exe
    echo 请确保WinRAR已安装
    pause
    exit /b 1
)

:: 使用WinRAR直接解压到当前目录
echo 使用WinRAR解压: %winrar_exe%
"%winrar_exe%" x -y -o+ "%zip_filename%"

if %errorlevel% neq 0 (
    echo WinRAR解压失败
    pause
    exit /b 1
)

echo ✓ 成功使用WinRAR解压并覆盖文件

:: 清理压缩包文件
echo.
echo 清理压缩包文件...

if exist "%zip_filename%" (
    del "%zip_filename%"
    echo ✓ 已清理压缩包文件
)

echo.
echo === 操作完成 ===
echo 已成功从git HEAD版本提取并更新了指定的文件/文件夹

pause