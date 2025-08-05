@echo off  
chcp 65001 >nul  
REM ================================  
REM 脚本名称: build_solution.bat  
REM 功能: 使用 MSBuild 编译 Visual Studio 解决方案  
REM ================================  

set VS_PATH="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"  
call %VS_PATH%

REM 检查是否加载成功  
if %errorlevel% neq 0 (  
    echo [错误] 无法加载 VS2019 开发者命令提示符环境！  
    exit /b %errorlevel%  
)  

REM 设置解决方案路径  
set SOLUTION_PATH=%cd%\Aprog.sln 
REM 设置默认配置和平台  
set CONFIGURATION=Release  
set PLATFORM=x64 


REM 检查是否提供了解决方案文件路径  
if not "%1"=="" (  
    set SOLUTION_PATH=%1
)

REM 如果提供了配置参数，则覆盖默认值  
if not "%2"=="" (  
    set CONFIGURATION=%2  
)  

REM 如果提供了平台参数，则覆盖默认值  
if not "%3"=="" (  
    set PLATFORM=%3  
)  

REM 输出当前编译信息  
echo ================================  
echo 正在编译解决方案: %SOLUTION_PATH%  
echo 配置: %CONFIGURATION%  
echo 平台: %PLATFORM%  
echo ================================  

REM 调用 MSBuild 进行编译  
msbuild "%SOLUTION_PATH%" /p:Configuration=%CONFIGURATION% -m:2 /p:Platform=%PLATFORM%  

REM 检查编译是否成功  
if %errorlevel% neq 0 (  
    echo [错误] 编译失败！  
    exit /b %errorlevel%  
)  

echo [成功] 编译完成！  
:: pause
exit /b 0