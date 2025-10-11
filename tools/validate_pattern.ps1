# Комплексная валидация паттерна
# Использование: .\tools\validate_pattern.ps1 -Pattern "producer_consumer" [-Full]
#
# Проверяет:
# 1. Компиляцию всех файлов
# 2. Детектирование уязвимостей санитайзерами
# 3. Чистоту безопасных версий
# 4. Статический анализ

param(
    [Parameter(Mandatory=$true)]
    [string]$Pattern,
    
    [switch]$Full = $false,
    [switch]$SkipBuild = $false,
    [string]$BuildDir = "build"
)

$ErrorActionPreference = "Continue"

# Цвета для вывода
function Write-Success { param([string]$Message) Write-Host "✅ $Message" -ForegroundColor Green }
function Write-Failure { param([string]$Message) Write-Host "❌ $Message" -ForegroundColor Red }
function Write-Warning-Msg { param([string]$Message) Write-Host "⚠️  $Message" -ForegroundColor Yellow }
function Write-Info { param([string]$Message) Write-Host "ℹ️  $Message" -ForegroundColor Cyan }
function Write-Step { param([string]$Message) Write-Host "`n=== $Message ===" -ForegroundColor Magenta }

# Начало валидации
Write-Host "`n╔═══════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║     КОМПЛЕКСНАЯ ВАЛИДАЦИЯ ПАТТЕРНА                         ║" -ForegroundColor Cyan
Write-Host "╚═══════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""
Write-Info "Паттерн: $Pattern"
Write-Info "Режим: $(if ($Full) { 'Полная проверка' } else { 'Быстрая проверка' })"
Write-Info "Дата: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
Write-Host ""

# Поиск директории паттерна
$patternDirs = Get-ChildItem -Path "." -Directory -Recurse -Filter "*$Pattern*" | 
               Where-Object { $_.FullName -notmatch "\\build\\" -and $_.FullName -notmatch "\\\.git\\" }

if ($patternDirs.Count -eq 0) {
    Write-Failure "Паттерн '$Pattern' не найден!"
    exit 1
}

if ($patternDirs.Count -gt 1) {
    Write-Warning-Msg "Найдено несколько директорий с '$Pattern'. Используется первая."
}

$patternDir = $patternDirs[0]
Write-Success "Найдена директория: $($patternDir.FullName)"

# Извлечение базового имени паттерна
$baseName = $patternDir.Name

# Счетчики результатов
$totalTests = 0
$passedTests = 0
$failedTests = 0
$skippedTests = 0

function Run-Test {
    param([string]$Name, [ScriptBlock]$Test)
    
    $script:totalTests++
    Write-Host "`n[TEST $script:totalTests] $Name" -ForegroundColor Yellow
    
    try {
        $result = & $Test
        if ($result) {
            Write-Success "PASS"
            $script:passedTests++
            return $true
        } else {
            Write-Failure "FAIL"
            $script:failedTests++
            return $false
        }
    } catch {
        Write-Failure "FAIL: $_"
        $script:failedTests++
        return $false
    }
}

# ============================================================================
# ФАЗА 1: ПРОВЕРКА СТРУКТУРЫ ФАЙЛОВ
# ============================================================================

Write-Step "ФАЗА 1: Проверка структуры файлов"

$expectedFiles = @(
    "CMakeLists.txt",
    "README.md",
    "SECURITY_ANALYSIS.md",
    "*_vulnerabilities.cpp",
    "secure_*_alternatives.cpp"
)

foreach ($filePattern in $expectedFiles) {
    Run-Test "Существование файла: $filePattern" {
        $files = Get-ChildItem -Path $patternDir.FullName -Filter $filePattern -File
        if ($files) {
            Write-Info "  Найдено: $($files[0].Name)"
            return $true
        }
        Write-Warning-Msg "  Не найдено: $filePattern"
        return $false
    }
}

# ============================================================================
# ФАЗА 2: КОМПИЛЯЦИЯ
# ============================================================================

if (-not $SkipBuild) {
    Write-Step "ФАЗА 2: Компиляция"
    
    # Проверка наличия build директории
    if (-not (Test-Path $BuildDir)) {
        Write-Info "Создание build директории..."
        New-Item -ItemType Directory -Path $BuildDir | Out-Null
        
        Write-Info "Конфигурация CMake..."
        Push-Location $BuildDir
        cmake .. -DCMAKE_BUILD_TYPE=Debug
        Pop-Location
    }
    
    # Поиск таргетов для компиляции
    $vulnerableFile = Get-ChildItem -Path $patternDir.FullName -Filter "*_vulnerabilities.cpp" -File | Select-Object -First 1
    $secureFile = Get-ChildItem -Path $patternDir.FullName -Filter "secure_*.cpp" -File | Select-Object -First 1
    
    if ($vulnerableFile) {
        $targetName = [System.IO.Path]::GetFileNameWithoutExtension($vulnerableFile.Name)
        
        Run-Test "Компиляция: $targetName" {
            Push-Location $BuildDir
            $output = cmake --build . --target $targetName 2>&1
            $success = $LASTEXITCODE -eq 0
            Pop-Location
            
            if (-not $success) {
                Write-Warning-Msg "  Вывод компиляции:"
                $output | Select-Object -Last 10 | ForEach-Object { Write-Host "    $_" }
            }
            
            return $success
        }
        
        # Компиляция с санитайзерами (если включен Full mode)
        if ($Full) {
            foreach ($sanitizer in @("asan", "tsan", "ubsan")) {
                Run-Test "Компиляция с $sanitizer`: $targetName" {
                    Push-Location $BuildDir
                    $target = "${targetName}_${sanitizer}"
                    $output = cmake --build . --target $target 2>&1
                    $success = $LASTEXITCODE -eq 0
                    Pop-Location
                    return $success
                }
            }
        }
    }
    
    if ($secureFile) {
        $targetName = [System.IO.Path]::GetFileNameWithoutExtension($secureFile.Name)
        
        Run-Test "Компиляция: $targetName" {
            Push-Location $BuildDir
            $output = cmake --build . --target $targetName 2>&1
            $success = $LASTEXITCODE -eq 0
            Pop-Location
            return $success
        }
    }
} else {
    Write-Warning-Msg "Фаза компиляции пропущена (--SkipBuild)"
}

# ============================================================================
# ФАЗА 3: ДИНАМИЧЕСКИЙ АНАЛИЗ (SANITIZERS)
# ============================================================================

if ($Full -and -not $SkipBuild) {
    Write-Step "ФАЗА 3: Динамический анализ (Sanitizers)"
    
    $vulnerableFile = Get-ChildItem -Path $patternDir.FullName -Filter "*_vulnerabilities.cpp" -File | Select-Object -First 1
    
    if ($vulnerableFile) {
        $baseName = [System.IO.Path]::GetFileNameWithoutExtension($vulnerableFile.Name)
        
        # AddressSanitizer
        Run-Test "ASan детектирует уязвимость" {
            $exePath = Join-Path $BuildDir "${baseName}_asan.exe"
            if (-not (Test-Path $exePath)) {
                $exePath = Join-Path $BuildDir "${baseName}_asan"
            }
            
            if (-not (Test-Path $exePath)) {
                Write-Warning-Msg "  Исполняемый файл не найден: $exePath"
                $script:skippedTests++
                $script:totalTests--
                return $null
            }
            
            Write-Info "  Запуск: $exePath"
            $output = & $exePath 2>&1 | Out-String
            
            # ASan должен детектировать проблему
            $detected = $output -match "ERROR: AddressSanitizer" -or 
                       $output -match "heap-use-after-free" -or 
                       $output -match "heap-buffer-overflow" -or
                       $output -match "SEGV"
            
            if ($detected) {
                Write-Info "  ✓ Уязвимость детектирована ASan"
            } else {
                Write-Warning-Msg "  ✗ ASan не детектировал уязвимость"
                Write-Info "  (Это может быть нормально для некоторых типов уязвимостей)"
            }
            
            return $detected
        }
        
        # ThreadSanitizer (для concurrency паттернов)
        if ($patternDir.FullName -match "concurrency|high-load") {
            Run-Test "TSan детектирует race condition" {
                $exePath = Join-Path $BuildDir "${baseName}_tsan.exe"
                if (-not (Test-Path $exePath)) {
                    $exePath = Join-Path $BuildDir "${baseName}_tsan"
                }
                
                if (-not (Test-Path $exePath)) {
                    Write-Warning-Msg "  Исполняемый файл не найден"
                    $script:skippedTests++
                    $script:totalTests--
                    return $null
                }
                
                Write-Info "  Запуск: $exePath"
                $output = & $exePath 2>&1 | Out-String
                
                $detected = $output -match "WARNING: ThreadSanitizer" -or 
                           $output -match "data race"
                
                if ($detected) {
                    Write-Info "  ✓ Race condition детектирована TSan"
                } else {
                    Write-Warning-Msg "  ✗ TSan не детектировала race condition"
                }
                
                return $detected
            }
        }
    }
    
    # Проверка безопасной версии
    $secureFile = Get-ChildItem -Path $patternDir.FullName -Filter "secure_*.cpp" -File | Select-Object -First 1
    
    if ($secureFile) {
        $baseName = [System.IO.Path]::GetFileNameWithoutExtension($secureFile.Name)
        
        Run-Test "Безопасная версия чиста под ASan" {
            $exePath = Join-Path $BuildDir "${baseName}_asan.exe"
            if (-not (Test-Path $exePath)) {
                $exePath = Join-Path $BuildDir "${baseName}_asan"
            }
            
            if (-not (Test-Path $exePath)) {
                Write-Warning-Msg "  Исполняемый файл не найден"
                $script:skippedTests++
                $script:totalTests--
                return $null
            }
            
            Write-Info "  Запуск: $exePath"
            $output = & $exePath 2>&1 | Out-String
            
            $clean = -not ($output -match "ERROR: AddressSanitizer")
            
            if ($clean) {
                Write-Success "  ✓ Безопасная версия чиста"
            } else {
                Write-Failure "  ✗ ASan нашел проблемы в безопасной версии!"
                Write-Host $output
            }
            
            return $clean
        }
    }
}

# ============================================================================
# ФАЗА 4: СТАТИЧЕСКИЙ АНАЛИЗ
# ============================================================================

if ($Full) {
    Write-Step "ФАЗА 4: Статический анализ"
    
    # Clang-Tidy
    $clangTidy = Get-Command clang-tidy -ErrorAction SilentlyContinue
    if ($clangTidy) {
        Run-Test "Clang-Tidy анализ" {
            Write-Info "  Запуск Clang-Tidy..."
            $cppFiles = Get-ChildItem -Path $patternDir.FullName -Filter "*.cpp" -File
            
            $totalWarnings = 0
            foreach ($file in $cppFiles) {
                $output = & clang-tidy $file.FullName -- -std=c++23 -I. -I./common 2>&1
                $warnings = ($output | Select-String "warning:" | Measure-Object).Count
                $totalWarnings += $warnings
            }
            
            Write-Info "  Найдено предупреждений: $totalWarnings"
            return $true  # Clang-Tidy не должен останавливать валидацию
        }
    } else {
        Write-Warning-Msg "Clang-Tidy не найден, пропуск"
        $skippedTests++
    }
    
    # Cppcheck
    $cppcheck = Get-Command cppcheck -ErrorAction SilentlyContinue
    if ($cppcheck) {
        Run-Test "Cppcheck анализ" {
            Write-Info "  Запуск Cppcheck..."
            $output = & cppcheck --enable=all --inconclusive --std=c++23 `
                      --suppress=missingIncludeSystem `
                      $patternDir.FullName 2>&1 | Out-String
            
            $errors = ([regex]::Matches($output, "\[error\]")).Count
            $warnings = ([regex]::Matches($output, "\[warning\]")).Count
            
            Write-Info "  Ошибок: $errors, Предупреждений: $warnings"
            
            # Для уязвимых файлов warnings это нормально
            return $errors -eq 0
        }
    } else {
        Write-Warning-Msg "Cppcheck не найден, пропуск"
        $skippedTests++
    }
}

# ============================================================================
# ФАЗА 5: ПРОВЕРКА ДОКУМЕНТАЦИИ
# ============================================================================

Write-Step "ФАЗА 5: Проверка документации"

Run-Test "SECURITY_ANALYSIS.md содержит CWE" {
    $secAnalysis = Join-Path $patternDir.FullName "SECURITY_ANALYSIS.md"
    if (-not (Test-Path $secAnalysis)) {
        Write-Warning-Msg "  Файл не найден"
        return $false
    }
    
    $content = Get-Content $secAnalysis -Raw
    $hasCWE = $content -match "CWE-\d+"
    
    if ($hasCWE) {
        $cweMatches = [regex]::Matches($content, "CWE-(\d+)")
        Write-Info "  Найдено CWE: $($cweMatches.Count)"
    }
    
    return $hasCWE
}

Run-Test "SECURITY_ANALYSIS.md содержит ссылки на CVE" {
    $secAnalysis = Join-Path $patternDir.FullName "SECURITY_ANALYSIS.md"
    if (-not (Test-Path $secAnalysis)) {
        return $false
    }
    
    $content = Get-Content $secAnalysis -Raw
    $hasCVE = $content -match "CVE-\d{4}-\d+"
    
    if ($hasCVE) {
        $cveMatches = [regex]::Matches($content, "CVE-\d{4}-\d+")
        Write-Info "  Найдено CVE: $($cveMatches.Count)"
    }
    
    return $hasCVE
}

# ============================================================================
# ИТОГОВЫЙ ОТЧЕТ
# ============================================================================

Write-Host "`n"
Write-Host "╔═══════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║                 ИТОГОВЫЙ ОТЧЕТ                            ║" -ForegroundColor Cyan
Write-Host "╚═══════════════════════════════════════════════════════════╝" -ForegroundColor Cyan

Write-Host "`nПаттерн: $Pattern" -ForegroundColor White
Write-Host "Директория: $($patternDir.FullName)" -ForegroundColor Gray

Write-Host "`nРезультаты:" -ForegroundColor White
Write-Host "  Всего тестов:   $totalTests"
Write-Success "  Пройдено:       $passedTests"
Write-Failure "  Провалено:      $failedTests"
if ($skippedTests -gt 0) {
    Write-Warning-Msg "  Пропущено:      $skippedTests"
}

$successRate = if ($totalTests -gt 0) { [math]::Round(($passedTests / $totalTests) * 100, 1) } else { 0 }
Write-Host "`nПроцент успеха: $successRate%" -ForegroundColor $(
    if ($successRate -eq 100) { "Green" }
    elseif ($successRate -ge 80) { "Yellow" }
    else { "Red" }
)

# Финальный вердикт
Write-Host "`n"
if ($failedTests -eq 0) {
    Write-Host "╔═══════════════════════════════════════════════════════════╗" -ForegroundColor Green
    Write-Host "║               ✅ ВАЛИДАЦИЯ УСПЕШНА ✅                      ║" -ForegroundColor Green
    Write-Host "╚═══════════════════════════════════════════════════════════╝" -ForegroundColor Green
    exit 0
} else {
    Write-Host "╔═══════════════════════════════════════════════════════════╗" -ForegroundColor Red
    Write-Host "║            ❌ ОБНАРУЖЕНЫ ПРОБЛЕМЫ ❌                       ║" -ForegroundColor Red
    Write-Host "╚═══════════════════════════════════════════════════════════╝" -ForegroundColor Red
    exit 1
}

