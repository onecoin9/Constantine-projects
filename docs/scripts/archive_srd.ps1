param(
  [string]$Srd = "docs/SoftwareRequirementsDocument.md",
  [string]$ArchiveRoot = "docs/archive"
)

# 切换到仓库根目录（脚本位于 docs/scripts 下）
$repoRoot = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
Set-Location $repoRoot

if (!(Test-Path $Srd)) { Write-Error "SRD not found: $Srd"; exit 1 }
$today = Get-Date -Format "yyyy-MM-dd"
$destDir = Join-Path $ArchiveRoot $today
New-Item -ItemType Directory -Force -Path $destDir | Out-Null
Copy-Item -Path $Srd -Destination (Join-Path $destDir "SoftwareRequirementsDocument.md") -Force

try {
  git add -- "$destDir" | Out-Null
  git commit -m "chore(docs): archive SRD $today" | Out-Null
  Write-Host "Archived SRD to $destDir"
} catch {
  Write-Warning $_
  exit 1
}
