@echo off
REM DDR性能监控集成编译测试脚本
echo ==============================================
echo DDR Performance Integration Build Test
echo ==============================================

REM 设置环境变量
set QTDIR=C:\Qt\6.2.4\msvc2019_64
set PATH=%QTDIR%\bin;%PATH%

REM 检查Qt环境
echo Checking Qt environment...
qmake --version
if %ERRORLEVEL% neq 0 (
    echo ERROR: Qt not found in PATH
    echo Please install Qt6 and set QTDIR correctly
    pause
    exit /b 1
)

REM 创建构建目录
if not exist build mkdir build
cd build

echo.
echo ==============================================
echo Configuring CMake...
echo ==============================================
cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_PREFIX_PATH=%QTDIR%
if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake configuration failed
    pause
    exit /b 1
)

echo.
echo ==============================================
echo Building project...
echo ==============================================
cmake --build . --config Release
if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed
    pause
    exit /b 1
)

echo.
echo ==============================================
echo Build completed successfully!
echo ==============================================
echo.
echo Available executables:
if exist bin\Release\DDRIntegrationTest.exe (
    echo   - DDRIntegrationTest.exe
) else (
    echo   - DDRIntegrationTest.exe [NOT FOUND]
)

if exist bin\Release\DDRStressTest.exe (
    echo   - DDRStressTest.exe
) else (
    echo   - DDRStressTest.exe [NOT FOUND]
)

echo.
echo To run tests:
echo   .\bin\Release\DDRIntegrationTest.exe
echo   .\bin\Release\DDRStressTest.exe
echo.

pause
