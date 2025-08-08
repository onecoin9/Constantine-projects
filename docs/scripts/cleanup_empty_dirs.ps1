param([string]$Root = "docs")

$removed = @()
$dirs = Get-ChildItem -Path $Root -Directory -Recurse -Force
foreach ($d in $dirs) {
  if ((Get-ChildItem -Force -Path $d.FullName | Measure-Object).Count -eq 0) {
    Remove-Item -LiteralPath $d.FullName -Force -Recurse
    $removed += $d.FullName
  }
}
if ($removed.Count -gt 0) {
  "Removed empty directories:"
  $removed | ForEach-Object { $_ }
} else {
  "No empty directories found."
}
