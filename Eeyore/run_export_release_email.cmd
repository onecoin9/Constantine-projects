@echo off
setlocal

cd /d "%~dp0"

set "PS_EXE=pwsh.exe"
where %PS_EXE% >nul 2>&1 || set "PS_EXE=powershell.exe"

set "SRC=releaseVersion.md"
set "OUT=releaseVersion_email.html"

"%PS_EXE%" -ExecutionPolicy Bypass -NoProfile -File "%~dp0export_email.ps1" -Source "%SRC%" -Out "%OUT%"

endlocal
exit /b 0
