@echo off
setlocal
REM Normalize console to UTF-8 and disable ANSI to avoid stream UTF-8 issues
chcp 65001 >nul
set "NO_COLOR=1"
set "TERM=dumb"

REM Always point CODEX_HOME to this project's .codex to avoid mixing with global configs
set "CODEX_HOME=%~dp0.codex"

REM Start in project root and force non-streaming chat wire to avoid UTF-8 SSE issues
cd /d "%~dp0"
codex -c model_providers.anyrouter.wire_api="chat" %*
endlocal
