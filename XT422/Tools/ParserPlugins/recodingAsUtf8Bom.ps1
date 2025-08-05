# ConvertToUtf8Bom.ps1

param (
    [string]$path = "."
)

function IsUtf8Bom($byteArray) {
    return ($byteArray.Length -gt 2 -and 
            $byteArray[0] -eq 0xEF -and 
            $byteArray[1] -eq 0xBB -and 
            $byteArray[2] -eq 0xBF)
}

$files = Get-ChildItem -Path $path -Recurse -Include *.h, *.c, *.hpp, *.cpp

foreach ($file in $files) {
    # 读取文件字节内容
    $byteContent = [System.IO.File]::ReadAllBytes($file.FullName)
    
    # 检查文件是否为UTF-8 BOM格式
    if (-not (IsUtf8Bom $byteContent)) {
        # 尝试使用GB2312编码解码文件内容，如果失败则使用默认编码重试
        try {
            $gb2312Encoding = [System.Text.Encoding]::GetEncoding('GB2312')
            $content = $gb2312Encoding.GetString($byteContent)
        }
        catch {
            # 如果GB2312解码失败，使用默认编码
            $defaultEncoding = [System.Text.Encoding]::Default
            $content = $defaultEncoding.GetString($byteContent)
        }
        
        # 写入文件内容并指定UTF-8带BOM编码
        $utf8BomEncoding = New-Object System.Text.UTF8Encoding $true
        [System.IO.File]::WriteAllText($file.FullName, $content, $utf8BomEncoding)
        
        Write-Output "Converted $($file.FullName) to UTF-8 with BOM"
    } else {
        Write-Output "$($file.FullName) is already in UTF-8 with BOM"
    }
}