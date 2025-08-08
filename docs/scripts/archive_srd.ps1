param(
  [string]$Srd = "docs/SoftwareRequirementsDocument.md",
  [string]$ArchiveRoot = "docs/archive"
)

if (!(Test-Path $Srd)) { Write-Error "SRD not found: $Srd"; exit 1 }
$today = Get-Date -Format "yyyy-MM-dd"
$destDir = Join-Path $ArchiveRoot $today
New-Item -ItemType Directory -Force -Path $destDir | Out-Null
Copy-Item -Path $Srd -Destination (Join-Path $destDir "SoftwareRequirementsDocument.md") -Force

git add $destDir | Out-Null
git commit -m "chore(docs): archive SRD $today" | Out-Null
Write-Host "Archived SRD to $destDir"
