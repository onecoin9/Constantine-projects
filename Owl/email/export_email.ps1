param(
    [Parameter(Mandatory=$false)]
    [string]$Source = "releaseVersion.md",
    
    [Parameter(Mandatory=$false)]
    [string]$Out = "releaseVersion_email.html"
)

$scriptDir = Split-Path -Parent -Path $MyInvocation.MyCommand.Definition
if (-not $scriptDir) { $scriptDir = Get-Location }

$Source = Join-Path $scriptDir $Source
$Out = Join-Path $scriptDir $Out

Write-Host "[INFO] Reading: $Source"
if (-not (Test-Path $Source)) {
    Write-Error "Source file not found: $Source"
    exit 1
}

# ==================== Markdown 转 HTML ====================

function Escape-Html {
    param([string]$s)
    if ($null -eq $s) { return "" }
    return ($s -replace '&','&amp;' -replace '<','&lt;' -replace '>','&gt;')
}

function Convert-InlineMarkdown {
    param([string]$text)
    if ($null -eq $text) { return "" }
    $t = Escape-Html $text
    # 链接
    $t = [Regex]::Replace($t, '\[([^\]]+)\]\(([^\)]+)\)', '<a href="$2">$1</a>')
    # 加粗
    $t = [Regex]::Replace($t, '\*\*([^*]+)\*\*', '<strong>$1</strong>')
    # 斜体
    $t = [Regex]::Replace($t, '(?<!\*)\*([^*]+)\*(?!\*)', '<em>$1</em>')
    # 代码
    $t = [Regex]::Replace($t, '(?<!&lt;)`([^`]+)`(?!;)', '<code>$1</code>')
    return $t
}

$raw = Get-Content -LiteralPath $Source -Raw -Encoding UTF8
$lines = $raw -split "\r?\n"

$htmlLines = @()
$inParagraph = $false
$i = 0

function Close-ParagraphIfOpen {
    if ($script:inParagraph) {
        $script:htmlLines += "</p>"
        $script:inParagraph = $false
    }
}

while ($i -lt $lines.Length) {
    $line = $lines[$i]
    $tline = $line.TrimEnd()

    # 表格
    if ($tline -match '\|' -and ($i+1 -lt $lines.Length) -and ($lines[$i+1] -match '^[\s\|:\-]+$')) {
        Close-ParagraphIfOpen
        $header = $tline.Trim('|').Split('|') | ForEach-Object { $_.Trim() }
        $i++
        $htmlLines += '<table border="1" cellpadding="8" cellspacing="0" style="border-collapse:collapse;width:100%;margin:12px 0">'
        $htmlLines += '<thead>'
        $htmlLines += '<tr style="background-color:#f8f8a0">'
        foreach ($h in $header) { $htmlLines += "<th style='text-align:left;border:1px solid #999'>$(Convert-InlineMarkdown $h)</th>" }
        $htmlLines += '</tr>'
        $htmlLines += '</thead>'
        $htmlLines += '<tbody>'
        
        while (($i+1) -lt $lines.Length -and ($lines[$i+1] -match '\|')) {
            $i++
            $row = $lines[$i].Trim('|').Split('|') | ForEach-Object { $_.Trim() }
            $htmlLines += '<tr>'
            foreach ($c in $row) { $htmlLines += "<td style='border:1px solid #999'>$(Convert-InlineMarkdown $c)</td>" }
            $htmlLines += '</tr>'
        }
        $htmlLines += '</tbody>'
        $htmlLines += '</table>'
        $i++
        continue
    }

    # 标题
    if ($tline -match '^(#{1,6})\s*(.*)$') {
        Close-ParagraphIfOpen
        $level = $matches[1].Length
        $text = $matches[2]
        $htmlLines += "<h$level style='margin:12px 0 8px 0'>$(Convert-InlineMarkdown $text)</h$level>"
        $i++
        continue
    }

    # 空行
    if ($tline -match '^\s*$') {
        Close-ParagraphIfOpen
        $i++
        continue
    }

    # 段落
    if (-not $script:inParagraph) {
        $script:inParagraph = $true
        $script:htmlLines += '<p style="margin:8px 0;line-height:1.6">'
    }

    $script:htmlLines += Convert-InlineMarkdown $tline
    $i++
}

Close-ParagraphIfOpen

$title = 'QC 版本发布通知'
foreach ($l in $htmlLines) { if ($l -match '^<h1[^>]*>(.*)</h1>$') { $title = $matches[1]; break } }

$html = @()
$html += '<!DOCTYPE html>'
$html += '<html>'
$html += '<head>'
$html += '  <meta charset="UTF-8">'
$html += '  <meta name="viewport" content="width=device-width,initial-scale=1">'
$html += "  <title>$title</title>"
$html += '  <style>'
$html += '    body { font-family: "Segoe UI", Arial, SimSun, sans-serif; margin:20px; background:#fff; color:#333; }'
$html += '    a { color:#0066cc; text-decoration:none; }'
$html += '    a:hover { text-decoration:underline; }'
$html += '    table { border-collapse:collapse; width:100%; margin:12px 0; background:#fff; }'
$html += '    th, td { border:1px solid #999; padding:8px; text-align:left; vertical-align:top; }'
$html += '    th { background-color:#f8f8a0; font-weight:bold; }'
$html += '  </style>'
$html += '</head>'
$html += '<body>'
$html += $htmlLines
$html += '</body>'
$html += '</html>'

$utf8BOM = New-Object System.Text.UTF8Encoding $true
[System.IO.File]::WriteAllText($Out, ($html -join "`n"), $utf8BOM)

Write-Host "[INFO] Generated: $Out"
Write-Host ""
Write-Host "=========== INSTRUCTIONS ==========="
Write-Host "1. File opened in browser"
Write-Host "2. Use Ctrl+A to select all content"
Write-Host "3. Use Ctrl+C to copy"
Write-Host "4. Paste into Dingtalk email with Ctrl+V"
Write-Host "=================================="
Write-Host ""

# 打开浏览器
Start-Process $Out

exit 0
