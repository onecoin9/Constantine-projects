# Export selected files for third-party consumption (collect to a clean folder)
# Usage examples (PowerShell):
#   powershell -ExecutionPolicy Bypass -File .\docs\scripts\export_for_third_party.ps1
#   powershell -ExecutionPolicy Bypass -File .\docs\scripts\export_for_third_party.ps1 -SourceRoot "D:\Constantine-Projects" -ExportRoot "D:\export\Constantine-projects" -DryRun
#   powershell -ExecutionPolicy Bypass -File .\docs\scripts\export_for_third_party.ps1 -InitGit -RemoteUrl "https://your.git.server/group/export-repo.git" -Push

[CmdletBinding()]param(
    [string]$SourceRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")),
    [string]$ExportRoot = "D:\\export\\Constantine-projects",
    [string[]]$IncludePaths = @(
        "workflow",
        "MTTurnable\bin",   # runtime binaries
        "MTTurnable\config" # runtime configs
    ),
    [string[]]$IncludeExtensions = @(
        ".exe", ".dll", ".pdb", ".ini", ".json", ".yaml", ".yml",
        ".qrc", ".ui", ".csv", ".txt", ".md", ".pdf",
        ".png", ".ico", ".jpg", ".jpeg", ".svg"
    ),
    [switch]$DryRun,
    [switch]$InitGit,
    [string]$Branch = "export",
    [string]$RemoteUrl,
    [switch]$Push
)

function Write-Info($msg) { Write-Host "[INFO] $msg" -ForegroundColor Cyan }
function Write-Warn($msg) { Write-Host "[WARN] $msg" -ForegroundColor Yellow }
function Write-Err($msg)  { Write-Host "[ERR ] $msg" -ForegroundColor Red }

if (-not (Test-Path $SourceRoot)) { Write-Err "SourceRoot not found: $SourceRoot"; exit 1 }

$timestamp = Get-Date -Format "yyyyMMdd-HHmmss"
$DestRoot = Join-Path $ExportRoot $timestamp

# Directories to exclude (substring match)
$ExcludeDirMarkers = @(
    "\\.git\\", "\\build\\", "\\cmake-build", "\\out\\", "\\node_modules\\",
    "\\.vs\\", "\\.vscode\\", "\\.idea\\", "\\obj\\", "\\Debug\\", "\\Release\\",
    "\\x64\\", "\\x86\\", "\\dist\\", "\\target\\", "\\__pycache__\\"
)

function Test-IsExcludedPath([string]$path) {
    foreach ($mark in $ExcludeDirMarkers) {
        if ($path -imatch [regex]::Escape($mark)) { return $true }
    }
    return $false
}

function Test-IsIncludedFile($file) {
    $ext = [System.IO.Path]::GetExtension($file.FullName)
    return $IncludeExtensions -contains $ext.ToLowerInvariant()
}

# Collect files
$files = @()
foreach ($rel in $IncludePaths) {
    $abs = Join-Path $SourceRoot $rel
    if (Test-IsExcludedPath $abs) { continue }
    if (Test-Path $abs) {
        $items = Get-ChildItem -LiteralPath $abs -Recurse -File -ErrorAction SilentlyContinue |
            Where-Object { -not (Test-IsExcludedPath $_.FullName) } |
            Where-Object { Test-IsIncludedFile $_ }
        $files += $items
    } else {
        Write-Warn "Skip missing path: $rel"
    }
}

# Also include scattered configs at repo level (json/ini/yaml) not inside excluded dirs
$scattered = Get-ChildItem -LiteralPath $SourceRoot -Recurse -File -Include *.json,*.ini,*.yaml,*.yml -ErrorAction SilentlyContinue |
    Where-Object { -not (Test-IsExcludedPath $_.FullName) }
$files += $scattered

# De-duplicate
$files = $files | Sort-Object FullName -Unique

Write-Info "SourceRoot: $SourceRoot"
Write-Info "ExportRoot: $DestRoot"
Write-Info ("Files to export (pre-filtered): {0}" -f $files.Count)

if (-not $DryRun) {
    New-Item -ItemType Directory -Path $DestRoot -Force | Out-Null
}

$copied = 0
foreach ($f in $files) {
    try {
        $relPath = $f.FullName.Substring($SourceRoot.Length).TrimStart('\\','/')
        $dest = Join-Path $DestRoot $relPath
        $destDir = Split-Path $dest -Parent
        if (-not $DryRun) {
            if (-not (Test-Path $destDir)) { New-Item -ItemType Directory -Path $destDir -Force | Out-Null }
            Copy-Item -LiteralPath $f.FullName -Destination $dest -Force -ErrorAction Stop
        }
        Write-Host " + ${relPath}"
        $copied++
    } catch {
        Write-Warn "Failed: $($f.FullName) -> $dest : $($_.Exception.Message)"
    }
}

Write-Info ("Copied files: {0}" -f $copied)
Write-Info "Next: 可对导出目录执行一次‘外发/解密’操作（由公司DLP/加密客户端提供），再交付第三方使用。"

if ($InitGit) {
    Write-Info "Initializing Git repo under: $DestRoot (branch: $Branch)"
    try {
        if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
            Write-Err "git is not in PATH. Skip git steps."; return
        }
        Push-Location $DestRoot
        if (-not (Test-Path ".git")) { git init | Out-Null }
        git checkout -B $Branch | Out-Null
        # Minimal .gitignore / .gitattributes
        if (-not (Test-Path ".gitignore")) {
            @(
                "*.log",
                "*.tmp",
                "Thumbs.db"
            ) | Set-Content -NoNewline:$false -Encoding UTF8 ".gitignore"
        }
        if (-not (Test-Path ".gitattributes")) {
            @(
                "*.exe binary",
                "*.dll binary",
                "*.pdb binary",
                "*.png binary",
                "*.jpg binary",
                "*.ico binary",
                "*.pdf binary",
                "* text=auto eol=crlf"
            ) | Set-Content -NoNewline:$false -Encoding UTF8 ".gitattributes"
        }
        git add -A
        git commit -m "chore(export): third-party delivery ${timestamp}" | Out-Null
        if ($RemoteUrl) {
            # set/update origin
            $hasOrigin = git remote | Select-String -SimpleMatch "origin"
            if ($hasOrigin) { git remote remove origin | Out-Null }
            git remote add origin $RemoteUrl
            if ($Push) {
                git push -u origin $Branch
                Write-Info "Pushed to: $RemoteUrl ($Branch)"
            } else {
                Write-Info "Remote set. Run 'git push -u origin $Branch' when ready to publish."
            }
        } else {
            Write-Info "No RemoteUrl provided. Local export repo ready at: $DestRoot"
        }
    } catch {
        Write-Err "Git step failed: $($_.Exception.Message)"
    } finally {
        Pop-Location | Out-Null
    }
}
