# Git 更新脚本
Set-Location "D:\Constantine-Projects\Tigger\美泰小转台"

Write-Host "检查 git 状态..." -ForegroundColor Green
git status

Write-Host "`n检查已跟踪的 bmp 和 rtf 文件..." -ForegroundColor Green
$trackedFiles = git ls-files | Where-Object { $_ -match '\.(bmp|rtf)$' }

if ($trackedFiles) {
    Write-Host "找到已跟踪的文件，正在从 git 中移除（保留本地文件）..." -ForegroundColor Yellow
    foreach ($file in $trackedFiles) {
        Write-Host "移除: $file" -ForegroundColor Cyan
        git rm --cached $file
    }
} else {
    Write-Host "没有找到已跟踪的 bmp 或 rtf 文件" -ForegroundColor Green
}

Write-Host "`n添加 .gitignore 到暂存区..." -ForegroundColor Green
git add .gitignore

Write-Host "`n提交更改..." -ForegroundColor Green
git commit -m "同步其他的人的代码崇开始数据库开发"

Write-Host "`n完成！" -ForegroundColor Green
