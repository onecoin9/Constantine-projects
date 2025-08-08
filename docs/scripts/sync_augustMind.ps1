param(
  [string]$Source = "D:/ConstantineFiles/03-Telemetry of Legends/Track Whisper Logs/augustMind.md",
  [string]$Target = "docs/augustMind.md"
)

if (!(Test-Path $Source)) { Write-Error "Source not found: $Source"; exit 1 }
Copy-Item -Path $Source -Destination $Target -Force

git add $Target | Out-Null
$now = Get-Date -Format "yyyy-MM-dd"
git commit -m "chore(log): sync augustMind $now" | Out-Null
Write-Host "Synced and committed augustMind for $now"
