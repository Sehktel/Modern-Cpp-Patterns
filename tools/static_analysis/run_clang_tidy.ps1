# Скрипт для запуска Clang-Tidy на файлах проекта
# Использование: .\run_clang_tidy.ps1 [-Pattern <pattern_name>] [-Secure] [-Vulnerable]

param(
    [string]$Pattern = "",
    [switch]$Secure = $false,
    [switch]$Vulnerable = $false,
    [switch]$All = $false
)

$ErrorActionPreference = "Continue"

# Цвета для вывода
function Write-Success { param([string]$Message) Write-Host "✅ $Message" -ForegroundColor Green }
function Write-Error-Msg { param([string]$Message) Write-Host "❌ $Message" -ForegroundColor Red }
function Write-Warning-Msg { param([string]$Message) Write-Host "⚠️  $Message" -ForegroundColor Yellow }
function Write-Info { param([string]$Message) Write-Host "ℹ️  $Message" -ForegroundColor Cyan }

Write-Host "`n=== Clang-Tidy Static Analysis ===" -ForegroundColor Magenta
Write-Host "Дата: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')`n" -ForegroundColor Gray

# Проверка наличия clang-tidy
$clangTidy = "C:\Program Files\LLVM\bin\clang-tidy.exe"
if (-not (Test-Path $clangTidy)) {
    $clangTidyPath = Get-Command clang-tidy -ErrorAction SilentlyContinue
    if ($clangTidyPath) {
        $clangTidy = $clangTidyPath.Source
    } else {
        Write-Error-Msg "clang-tidy не найден!"
        Write-Info "Установите LLVM: https://github.com/llvm/llvm-project/releases"
        exit 1
    }
}

Write-Success "Найден: $clangTidy"

# Конфигурация clang-tidy
$checks = @(
    "bugprone-*",
    "cert-*",
    "clang-analyzer-*",
    "cppcoreguidelines-*",
    "modernize-*",
    "performance-*",
    "readability-*",
    "concurrency-*"
) -join ","

$clangTidyArgs = @(
    "--checks=$checks",
    "--warnings-as-errors=",
    "--",
    "-std=c++23",
    "-I.",
    "-I./common"
)

# Функция для анализа файла
function Analyze-File {
    param([string]$FilePath)
    
    Write-Info "Анализ: $FilePath"
    
    $output = & $clangTidy $FilePath $clangTidyArgs 2>&1
    $warningCount = ($output | Select-String "warning:" | Measure-Object).Count
    $errorCount = ($output | Select-String "error:" | Measure-Object).Count
    
    if ($errorCount -gt 0) {
        Write-Error-Msg "  Найдено ошибок: $errorCount"
    }
    
    if ($warningCount -gt 0) {
        Write-Warning-Msg "  Найдено предупреждений: $warningCount"
    } else {
        Write-Success "  Чисто!"
    }
    
    return @{
        File = $FilePath
        Warnings = $warningCount
        Errors = $errorCount
        Output = $output -join "`n"
    }
}

# Поиск файлов для анализа
$filesToAnalyze = @()

if ($Pattern) {
    # Анализ конкретного паттерна
    $patternDirs = Get-ChildItem -Path "." -Directory -Recurse -Filter "*$Pattern*" | 
                   Where-Object { $_.FullName -notmatch "\\build\\" -and $_.FullName -notmatch "\\\.git\\" }
    
    foreach ($dir in $patternDirs) {
        if ($Vulnerable) {
            $filesToAnalyze += Get-ChildItem -Path $dir.FullName -Filter "*vulnerabilities.cpp" -File
        }
        if ($Secure) {
            $filesToAnalyze += Get-ChildItem -Path $dir.FullName -Filter "secure_*.cpp" -File
        }
        if (-not $Vulnerable -and -not $Secure) {
            $filesToAnalyze += Get-ChildItem -Path $dir.FullName -Filter "*.cpp" -File
        }
    }
} else {
    # Анализ всех файлов
    if ($Vulnerable) {
        $filesToAnalyze = Get-ChildItem -Path "." -Filter "*vulnerabilities.cpp" -File -Recurse | 
                         Where-Object { $_.FullName -notmatch "\\build\\" }
    } elseif ($Secure) {
        $filesToAnalyze = Get-ChildItem -Path "." -Filter "secure_*.cpp" -File -Recurse | 
                         Where-Object { $_.FullName -notmatch "\\build\\" }
    } else {
        $filesToAnalyze = Get-ChildItem -Path "." -Filter "*.cpp" -File -Recurse | 
                         Where-Object { $_.FullName -notmatch "\\build\\" -and $_.FullName -notmatch "\\common\\" }
    }
}

if ($filesToAnalyze.Count -eq 0) {
    Write-Warning-Msg "Файлы для анализа не найдены!"
    exit 1
}

Write-Info "Найдено файлов для анализа: $($filesToAnalyze.Count)`n"

# Анализ файлов
$results = @()
$totalWarnings = 0
$totalErrors = 0

foreach ($file in $filesToAnalyze) {
    $result = Analyze-File -FilePath $file.FullName
    $results += $result
    $totalWarnings += $result.Warnings
    $totalErrors += $result.Errors
}

# Генерация отчета
Write-Host "`n=== Итоговый отчет ===" -ForegroundColor Magenta
Write-Host "Проанализировано файлов: $($results.Count)"
Write-Host "Всего предупреждений: $totalWarnings"
Write-Host "Всего ошибок: $totalErrors"

if ($totalErrors -eq 0) {
    Write-Success "`nСтатический анализ завершен успешно!"
} else {
    Write-Error-Msg "`nОбнаружены критические проблемы!"
}

# Сохранение детального отчета
$reportPath = "clang_tidy_report_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"
$results | ForEach-Object {
    "=" * 80 >> $reportPath
    "FILE: $($_.File)" >> $reportPath
    "WARNINGS: $($_.Warnings)" >> $reportPath
    "ERRORS: $($_.Errors)" >> $reportPath
    "=" * 80 >> $reportPath
    $_.Output >> $reportPath
    "" >> $reportPath
} 

Write-Info "Детальный отчет сохранен: $reportPath"

exit $totalErrors

