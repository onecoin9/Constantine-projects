@echo off
chcp 65001 >nul
cd /d "%~dp0"

echo 检查 git 状态...
git status

echo.
echo 检查已跟踪的 bmp 和 rtf 文件...
git ls-files | findstr /i "\.bmp$ \.rtf$" > tracked_files.txt
if exist tracked_files.txt (
    for /f "delims=" %%f in (tracked_files.txt) do (
        echo 从 git 中移除: %%f
        git rm --cached "%%f"
    )
    del tracked_files.txt
)

echo.
echo 添加 .gitignore 到暂存区...
git add .gitignore

echo.
echo 提交更改...
git commit -m "同步其他的人的代码崇开始数据库开发"

echo.
echo 完成！
pause
