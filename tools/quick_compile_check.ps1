# Быстрая проверка компиляции всех паттернов
param([switch]$Verbose = $false)

$ErrorActionPreference = "Continue"
$clang = "C:/Program Files/LLVM/bin/clang++.exe"

if (-not (Test-Path $clang)) {
    Write-Host "❌ Clang++ не найден!" -ForegroundColor Red
    exit 1
}

Write-Host "`n╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║       БЫСТРАЯ ПРОВЕРКА КОМПИЛЯЦИИ ВСЕХ ПАТТЕРНОВ          ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan

# Модули для проверки
$modules = @(
    "01-basics",
    "02-principles", 
    "03-creational",
    "04-structural",
    "05-behavioral",
    "06-modern-cpp",
    "07-concurrency",
    "08-high-load",
    "09-performance"
)

$totalPatterns = 0
$successPatterns = 0
$failedPatterns = 0
$results = @()

foreach ($module in $modules) {
    if (-not (Test-Path $module)) { continue }
    
    $lessons = Get-ChildItem -Path $module -Directory | Where-Object { $_.Name -match "^lesson_" }
    
    foreach ($lesson in $lessons) {
        $totalPatterns++
        $patternName = "$module/$($lesson.Name)"
        
        Write-Host "`n[$totalPatterns] Проверка: $patternName" -ForegroundColor Yellow
        
        # Ищем vulnerable и secure файлы
        $vulnFiles = Get-ChildItem -Path $lesson.FullName -Filter "*vulnerabilities.cpp" -ErrorAction SilentlyContinue
        $secureFiles = Get-ChildItem -Path $lesson.FullName -Filter "*secure*.cpp" -ErrorAction SilentlyContinue
        
        $vulnOk = $true
        $secureOk = $true
        $errorMsg = ""
        
        # Компилируем vulnerable
        foreach ($file in $vulnFiles) {
            Write-Host "  └─ Компиляция vulnerable: $($file.Name)" -NoNewline
            $output = & $clang -std=c++17 -fsyntax-only $file.FullName 2>&1
            if ($LASTEXITCODE -eq 0) {
                Write-Host " ✅" -ForegroundColor Green
            } else {
                Write-Host " ❌" -ForegroundColor Red
                $vulnOk = $false
                if ($Verbose) {
                    Write-Host $output -ForegroundColor DarkRed
                }
                $errorMsg += "Vuln: " + ($output | Select-String "error:" | Select-Object -First 1) + "; "
            }
        }
        
        # Компилируем secure
        foreach ($file in $secureFiles) {
            Write-Host "  └─ Компиляция secure: $($file.Name)" -NoNewline
            $output = & $clang -std=c++17 -fsyntax-only $file.FullName 2>&1
            if ($LASTEXITCODE -eq 0) {
                Write-Host " ✅" -ForegroundColor Green
            } else {
                Write-Host " ❌" -ForegroundColor Red
                $secureOk = $false
                if ($Verbose) {
                    Write-Host $output -ForegroundColor DarkRed
                }
                $errorMsg += "Secure: " + ($output | Select-String "error:" | Select-Object -First 1)
            }
        }
        
        # Результат
        if ($vulnOk -and $secureOk) {
            Write-Host "  ✅ УСПЕХ" -ForegroundColor Green
            $successPatterns++
            $results += [PSCustomObject]@{
                Pattern = $patternName
                Status = "✅ OK"
                Error = ""
            }
        } else {
            Write-Host "  ❌ ПРОВАЛ" -ForegroundColor Red
            $failedPatterns++
            $results += [PSCustomObject]@{
                Pattern = $patternName
                Status = "❌ FAIL"
                Error = $errorMsg
            }
        }
    }
}

# Итоговый отчет
Write-Host "`n╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║                    ИТОГОВЫЙ ОТЧЕТ                          ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host "`nВсего паттернов:   $totalPatterns" -ForegroundColor White
Write-Host "✅ Успешных:       $successPatterns" -ForegroundColor Green
Write-Host "❌ С ошибками:     $failedPatterns" -ForegroundColor Red
Write-Host "📊 Процент успеха: $([math]::Round($successPatterns * 100.0 / $totalPatterns, 1))%" -ForegroundColor Cyan

# Проблемные паттерны
if ($failedPatterns -gt 0) {
    Write-Host "`n⚠️  ПРОБЛЕМНЫЕ ПАТТЕРНЫ:" -ForegroundColor Yellow
    $results | Where-Object { $_.Status -eq "❌ FAIL" } | ForEach-Object {
        Write-Host "  • $($_.Pattern)" -ForegroundColor Red
        if ($_.Error -and $Verbose) {
            Write-Host "    $($_.Error)" -ForegroundColor DarkRed
        }
    }
}

# Сохраняем детальный отчет
$reportPath = "quick_compile_report.md"
$timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"

$report = @"
# Отчет быстрой проверки компиляции

**Дата**: $timestamp  
**Результаты**: $successPatterns/$totalPatterns успешных ($([math]::Round($successPatterns * 100.0 / $totalPatterns, 1))%)

## Детализация

| Паттерн | Статус | Ошибка |
|---------|--------|--------|
"@

foreach ($result in $results) {
    $report += "`n| $($result.Pattern) | $($result.Status) | $($result.Error) |"
}

$report | Out-File -FilePath $reportPath -Encoding UTF8
Write-Host "`n📄 Детальный отчет сохранен: $reportPath" -ForegroundColor Cyan

