param(
  [string]$Source = "D:/ConstantineFiles/03-Telemetry of Legends/Track Whisper Logs/augustMind.md",
  [string]$Target = "docs/augustMind.md"
)

# 切换到仓库根目录（脚本位于 docs/scripts 下）
$repoRoot = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
Set-Location $repoRoot

if (!(Test-Path $Source)) { Write-Error "Source not found: $Source"; exit 1 }
Copy-Item -Path $Source -Destination $Target -Force

# 提交到git
try {
  git add -- "$Target" | Out-Null
  $now = Get-Date -Format "yyyy-MM-dd"
  git commit -m "chore(log): sync augustMind $now" | Out-Null
  Write-Host "Synced and committed augustMind for $now"
} catch {
  Write-Warning $_
  exit 1
}
