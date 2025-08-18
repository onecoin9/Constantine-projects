# Convert releaseVersion.md (markdown tables) to an email-friendly HTML file
# Usage:
#   powershell -ExecutionPolicy Bypass -File .\qc需要文件\export_release_email.ps1
#   powershell -ExecutionPolicy Bypass -File .\qc需要文件\export_release_email.ps1 -Source "d:\Constantine-Projects\qc需要文件\releaseVersion.md" -Out "d:\Constantine-Projects\qc需要文件\releaseVersion_email.html"

[CmdletBinding()]
param(
    [string]$Source = (Join-Path $PSScriptRoot "releaseVersion.md"),
    [string]$Out    = (Join-Path $PSScriptRoot "releaseVersion_email.html")
)

function Write-Info($m){ Write-Host "[INFO] $m" -ForegroundColor Cyan }
function HtmlEscape([string]$s) {
    if ($null -eq $s) { return "" }
    return $s -replace '&','&amp;' -replace '<','&lt;' -replace '>','&gt;'
}

function ConvertMarkdownLinks([string]$s){
    # Convert [text](url) to <a href="url">text</a>
    return [regex]::Replace($s, '\[([^\]]+)\]\(([^)]+)\)', '<a href="$2">$1</a>')
}

function Get-MimeType([string]$path) {
    $ext = [System.IO.Path]::GetExtension($path)
    if (-not $ext) { $ext = '' }
    $ext = $ext.ToLowerInvariant()
    switch ($ext) {
        '.png'  { 'image/png' }
        '.jpg'  { 'image/jpeg' }
        '.jpeg' { 'image/jpeg' }
        '.gif'  { 'image/gif' }
        '.bmp'  { 'image/bmp' }
        '.svg'  { 'image/svg+xml' }
        default { 'application/octet-stream' }
    }
}

function ConvertMarkdownImage([string]$line, [string]$mdPath) {
    # Manual parse for ![alt](src)
    if (-not $line.StartsWith('![')) { return $null }
    $startAlt = 2
    $endAlt = $line.IndexOf(']')
    if ($endAlt -lt 0) { return $null }
    $alt = $line.Substring($startAlt, $endAlt - $startAlt)
    if ($endAlt + 1 -ge $line.Length) { return $null }
    if ($line[$endAlt + 1] -ne '(') { return $null }
    $startSrc = $endAlt + 2
    $endSrc = $line.LastIndexOf(')')
    if ($endSrc -lt 0 -or $endSrc -le $startSrc) { return $null }
    $src = $line.Substring($startSrc, $endSrc - $startSrc)

    if ($src -match '^(http|https)://') {
        return "<img alt='$(HtmlEscape $alt)' src='$src' style='max-width:100%;border:1px solid #ddd;' />"
    }
    # Resolve relative path: try md directory, then repo root (one level up)
    $baseDir = Split-Path -Parent $mdPath
    $tryPaths = @()
    if ([System.IO.Path]::IsPathRooted($src)) {
        $tryPaths += $src
    } else {
        $tryPaths += (Join-Path $baseDir $src)
        $repoRoot = Split-Path -Parent $baseDir
        $tryPaths += (Join-Path $repoRoot $src)
    }
    $abs = $null
    foreach ($p in $tryPaths) { if (Test-Path -LiteralPath $p) { $abs = $p; break } }
    if ($abs) {
        try {
            $bytes = [System.IO.File]::ReadAllBytes($abs)
            $b64 = [System.Convert]::ToBase64String($bytes)
            $mime = Get-MimeType $abs
            return "<img alt='$(HtmlEscape $alt)' src='data:$mime;base64,$b64' style='max-width:100%;border:1px solid #ddd;' />"
        } catch {
            return "<div style='color:#a00;'>[图片读取失败] $(HtmlEscape $src)</div>"
        }
    } else {
        return "<div style='color:#a00;'>[图片不存在] $(HtmlEscape $src)</div>"
    }
}

function Convert-Table {
    param([string[]]$lines, [int]$startIndex)
    $i = $startIndex
    $header = $lines[$i].Trim()
    $i++
    if ($i -ge $lines.Count) { return @{ html=""; next=$i } }
    # alignment row (--- | :--- etc.)
    $i++
    $headers = ($header.Trim('|').Split('|') | ForEach-Object { $_.Trim() })

    $rows = @()
    while ($i -lt $lines.Count -and $lines[$i].Trim().StartsWith('|')) {
        $rowCells = ($lines[$i].Trim().Trim('|').Split('|') | ForEach-Object { $_.Trim() })
        $rows += ,$rowCells
        $i++
    }

    # build HTML
    $sb = New-Object System.Text.StringBuilder
    [void]$sb.AppendLine('<table style="border-collapse:collapse;width:100%;font-family:Segoe UI,Arial;font-size:12.5px;">')
    [void]$sb.AppendLine('<thead>')
    [void]$sb.AppendLine('<tr>')
    foreach ($h in $headers) {
        $h2 = ConvertMarkdownLinks((HtmlEscape $h))
        [void]$sb.AppendLine("<th style='border:1px solid #ccc;background:#f8f8a0;padding:6px;text-align:left;'>$h2</th>")
    }
    [void]$sb.AppendLine('</tr>')
    [void]$sb.AppendLine('</thead>')
    [void]$sb.AppendLine('<tbody>')
    foreach ($r in $rows) {
        [void]$sb.AppendLine('<tr>')
        foreach ($cell in $r) {
            $c = ConvertMarkdownLinks((HtmlEscape $cell))
            [void]$sb.AppendLine("<td style='border:1px solid #ccc;padding:6px;vertical-align:top;'>$c</td>")
        }
        [void]$sb.AppendLine('</tr>')
    }
    [void]$sb.AppendLine('</tbody>')
    [void]$sb.AppendLine('</table>')

    return @{ html=$sb.ToString(); next=$i }
}

if (-not (Test-Path $Source)) {
    Write-Error "Source not found: $Source"; exit 1
}

$lines = Get-Content -LiteralPath $Source -Encoding UTF8

# Sequential parse: headings, images, tables, paragraphs
$html = @()
for ($i = 0; $i -lt $lines.Count; $i++) {
    $t = $lines[$i].Trim()
    if (-not $t) { continue }

    if ($t -match '^# ') {
        $h = $t -replace '^#\s+', ''
        $html += "<h1 style='font-family:Segoe UI,Arial;margin:0 0 8px;'>$((HtmlEscape $h))</h1>"
        continue
    }
    if ($t -match '^## ') {
        $h2 = $t -replace '^##\s+', ''
        $html += "<h2 style='font-family:Segoe UI,Arial;margin:10px 0 6px;'>$((HtmlEscape $h2))</h2>"
        continue
    }
    if ($t -match '^---+$') { $html += '<hr style="border:none;border-top:1px solid #ddd;margin:8px 0;" />'; continue }

    if ($t.StartsWith('|')) {
        $res = Convert-Table -lines $lines -startIndex $i
        $html += $res.html
        $i = $res.next - 1
        continue
    }

    if ($t -match '^!\[') {
        $img = ConvertMarkdownImage -line $t -mdPath $Source
        if ($img) { $html += $img; continue }
    }

    # default paragraph
    $html += "<p style='font-family:Segoe UI,Arial;margin:4px 0;'>$((ConvertMarkdownLinks (HtmlEscape $t)))</p>"
}

$doc = @(
    '<!doctype html>',
    '<html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">',
    '<title>QC 版本发布通知</title></head><body style="margin:12px;">',
    ($html -join "`n"),
    '</body></html>'
) -join "`n"

Set-Content -LiteralPath $Out -Encoding UTF8 -Value $doc
Write-Host "[INFO] Generated: $Out"
