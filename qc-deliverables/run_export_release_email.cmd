@echo off
setlocal

REM Run from this script's directory
cd /d "%~dp0"

REM Prefer bundled PowerShell (pwsh.exe) if available, else fallback to Windows PowerShell
set "PS_EXE=pwsh.exe"
where %PS_EXE% >nul 2>&1 || set "PS_EXE=powershell.exe"

REM Source/Out defaults
set "SRC=releaseVersion.md"
set "OUT=releaseVersion_email.html"

REM Run converter
"%PS_EXE%" -ExecutionPolicy Bypass -NoProfile -File "%~dp0export_release_email.ps1" -Source "%~dp0%SRC%" -Out "%~dp0%OUT%"
if errorlevel 1 (
  echo [ERROR] 导出失败，详见上方输出。
  pause
  exit /b 1
)

REM Copy result to clipboard (prefer HTML, fallback to plain text)
set "FULL=%~dp0%OUT%"
"%PS_EXE%" -NoProfile -Command "$f=[IO.Path]::GetFullPath('%FULL%'); try { Get-Content -LiteralPath $f -Raw | Set-Clipboard -AsHtml; Write-Host '[INFO] 已复制到剪贴板（HTML）' } catch { try { Get-Content -LiteralPath $f -Raw | Set-Clipboard; Write-Host '[INFO] 已复制到剪贴板（纯文本）' } catch { Write-Warning '未能复制到剪贴板' } }"

REM Open result with default browser
start "" "%~dp0%OUT%"

endlocal
exit /b 0
