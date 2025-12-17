@echo off
setlocal EnableExtensions EnableDelayedExpansion

REM Ensure UTF-8 output for Chinese logs
chcp 65001 >nul

REM Run from the script directory (can be a folder containing multiple repos)
set "ROOT=%~dp0"
cd /d "%ROOT%" || (
  echo [ERROR] 无法切换到脚本目录：%ROOT%
  pause
  exit /b 1
)

where git >nul 2>&1 || (
  echo [ERROR] 未找到 git，请先安装 Git 并确保已加入 PATH。
  pause
  exit /b 1
)

echo.
echo ===== 选择需要 push 的文件夹（脚本所在目录的子目录） =====

set /a COUNT=0
for /d %%D in ("%ROOT%*") do (
  set /a COUNT+=1
  set "DIR[!COUNT!]=%%~nxD"
  echo  !COUNT!^) %%~nxD
)

if %COUNT%==0 (
  echo [ERROR] 当前目录下没有任何子文件夹：%ROOT%
  pause
  exit /b 1
)

echo.
set "SEL="
set /p "SEL=请输入序号(1-%COUNT%)："

if not defined SEL (
  echo [ERROR] 未输入序号。
  pause
  exit /b 1
)

REM validate numeric
for /f "delims=0123456789" %%A in ("%SEL%") do (
  echo [ERROR] 请输入纯数字序号。
  pause
  exit /b 1
)

if %SEL% LSS 1 (
  echo [ERROR] 序号超出范围。
  pause
  exit /b 1
)
if %SEL% GTR %COUNT% (
  echo [ERROR] 序号超出范围。
  pause
  exit /b 1
)

set "TARGET=!DIR[%SEL%]!"
if not defined TARGET (
  echo [ERROR] 选择无效。
  pause
  exit /b 1
)

pushd "%ROOT%!TARGET!" >nul || (
  echo [ERROR] 无法进入目录：%ROOT%!TARGET!
  pause
  exit /b 1
)

git rev-parse --is-inside-work-tree >nul 2>&1 || (
  echo [ERROR] 该目录不是 Git 仓库：%CD%
  popd >nul
  pause
  exit /b 1
)

for /f "delims=" %%B in ('git rev-parse --abbrev-ref HEAD 2^>nul') do set "BRANCH=%%B"
if "%BRANCH%"=="" (
  echo [ERROR] 获取当前分支失败。
  popd >nul
  pause
  exit /b 1
)
if /i "%BRANCH%"=="HEAD" (
  echo [ERROR] 当前处于 detached HEAD 状态，请先切换到分支再 push。
  popd >nul
  pause
  exit /b 1
)

echo.
echo ===== Git Status (%BRANCH%) =====
git status
echo.
echo ===== Last Commit =====
git --no-pager log -1 --oneline
echo.

REM Check whether upstream is set
git rev-parse --abbrev-ref --symbolic-full-name @{u} >nul 2>&1
if errorlevel 1 (
  echo [INFO] 当前分支尚未设置 upstream，将执行：git push -u origin %BRANCH%
  git push -u origin "%BRANCH%"
) else (
  echo [INFO] 执行：git push
  git push
)

if errorlevel 1 (
  echo.
  echo [ERROR] push 失败。请检查网络/权限/远端分支保护策略。
  popd >nul
  pause
  exit /b 1
)

echo.
echo [OK] push 完成。

popd >nul
pause
exit /b 0
