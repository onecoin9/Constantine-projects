@echo off
setlocal

rem 设置要处理的文件夹路径
set folderPath=%~dp0

rem 调用PowerShell脚本
powershell -NoProfile -ExecutionPolicy Bypass -Command "& { .\recodingAsUtf8Bom.ps1 -path '%folderPath%' }"

endlocal