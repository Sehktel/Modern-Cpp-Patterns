# Скрипт для запуска Cppcheck на файлах проекта
# Использование: .\run_cppcheck.ps1 [-Pattern <pattern_name>] [-Xml]

param(
    [string]$Pattern = "",
    [switch]$Xml = $false,
    [string]$OutputDir = "cppcheck_results"
)

$ErrorActionPreference = "Continue"

# Цвета для вывода
function Write-Success { param([string]$Message) Write-Host "✅ $Message" -ForegroundColor Green }
function Write-Error-Msg { param([string]$Message) Write-Host "❌ $Message" -ForegroundColor Red }
function Write-Warning-Msg { param([string]$Message) Write-Host "⚠️  $Message" -ForegroundColor Yellow }
function Write-Info { param([string]$Message) Write-Host "ℹ️  $Message" -ForegroundColor Cyan }

Write-Host "`n=== Cppcheck Static Analysis ===" -ForegroundColor Magenta
Write-Host "Дата: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')`n" -ForegroundColor Gray

# Проверка наличия cppcheck
$cppcheckPath = Get-Command cppcheck -ErrorAction SilentlyContinue
if (-not $cppcheckPath) {
    Write-Error-Msg "cppcheck не найден!"
    Write-Info "Установите Cppcheck: https://cppcheck.sourceforge.io/"
    exit 1
}

Write-Success "Найден: $($cppcheckPath.Source)"

# Создание директории для результатов
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir | Out-Null
    Write-Info "Создана директория: $OutputDir"
}

# Определение директорий для анализа
$dirsToAnalyze = @()

if ($Pattern) {
    $dirsToAnalyze = Get-ChildItem -Path "." -Directory -Recurse -Filter "*$Pattern*" | 
                     Where-Object { $_.FullName -notmatch "\\build\\" -and $_.FullName -notmatch "\\\.git\\" } |
                     Select-Object -First 1
    
    if (-not $dirsToAnalyze) {
        Write-Error-Msg "Паттерн '$Pattern' не найден!"
        exit 1
    }
    
    Write-Info "Анализ паттерна: $($dirsToAnalyze.FullName)"
} else {
    # Анализ всего проекта
    Write-Info "Анализ всего проекта"
    $dirsToAnalyze = Get-Item "."
}

# Параметры cppcheck
$cppcheckArgs = @(
    "--enable=all",
    "--inconclusive",
    "--std=c++23",
    "--platform=win64",
    "--suppress=missingIncludeSystem",
    "--suppress=unmatchedSuppression",
    "-I.",
    "-I./common",
    "--inline-suppr"
)

if ($Xml) {
    $cppcheckArgs += "--xml"
    $cppcheckArgs += "--xml-version=2"
}

# Категории проблем
$categories = @{
    "error" = 0
    "warning" = 0
    "style" = 0
    "performance" = 0
    "portability" = 0
    "information" = 0
}

# Запуск анализа
Write-Host "`nЗапуск анализа..." -ForegroundColor Yellow
Write-Host "Это может занять несколько минут...`n"

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$outputFile = Join-Path $OutputDir "cppcheck_$timestamp"

if ($Xml) {
    $outputFile += ".xml"
    $output = & cppcheck $cppcheckArgs $dirsToAnalyze.FullName 2>&1
    $output | Out-File -FilePath $outputFile -Encoding UTF8
} else {
    $outputFile += ".txt"
    $output = & cppcheck $cppcheckArgs $dirsToAnalyze.FullName 2>&1
    $output | Out-File -FilePath $outputFile -Encoding UTF8
}

# Анализ результатов
Write-Host "`n=== Анализ результатов ===" -ForegroundColor Magenta

foreach ($line in $output) {
    if ($line -match "\[(error|warning|style|performance|portability|information)\]") {
        $severity = $matches[1]
        $categories[$severity]++
        
        switch ($severity) {
            "error" { Write-Error-Msg $line }
            "warning" { Write-Warning-Msg $line }
            default { Write-Host $line -ForegroundColor Gray }
        }
    }
}

# Итоговый отчет
Write-Host "`n=== Итоговый отчет ===" -ForegroundColor Magenta
Write-Host "Файл результатов: $outputFile`n"

$totalIssues = 0
foreach ($category in $categories.Keys | Sort-Object) {
    $count = $categories[$category]
    $totalIssues += $count
    
    $color = switch ($category) {
        "error" { "Red" }
        "warning" { "Yellow" }
        "performance" { "Cyan" }
        default { "Gray" }
    }
    
    Write-Host "$($category.PadRight(15)): $count" -ForegroundColor $color
}

Write-Host "`nВсего проблем: $totalIssues" -ForegroundColor $(if ($totalIssues -eq 0) { "Green" } else { "Yellow" })

# Рекомендации
if ($categories["error"] -gt 0) {
    Write-Host "`n⚠️  КРИТИЧНО: Обнаружены ошибки уровня 'error'!" -ForegroundColor Red
    Write-Host "   Проверьте файл отчета для деталей." -ForegroundColor Red
}

if ($totalIssues -eq 0) {
    Write-Success "`nСтатический анализ не выявил проблем!"
} else {
    Write-Info "`nОбнаружены проблемы. Рекомендуется проверка."
}

# Создание HTML отчета (если установлен cppcheck-htmlreport)
if ($Xml) {
    Write-Info "`nПопытка создать HTML отчет..."
    $htmlReportCmd = Get-Command cppcheck-htmlreport -ErrorAction SilentlyContinue
    
    if ($htmlReportCmd) {
        $htmlDir = Join-Path $OutputDir "html_$timestamp"
        & cppcheck-htmlreport --file=$outputFile --report-dir=$htmlDir --source-dir=.
        
        if ($?) {
            Write-Success "HTML отчет создан: $htmlDir\index.html"
        }
    } else {
        Write-Warning-Msg "cppcheck-htmlreport не найден. HTML отчет не создан."
    }
}

exit $(if ($categories["error"] -gt 0) { 1 } else { 0 })

