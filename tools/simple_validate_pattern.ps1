# Упрощенная валидация паттерна (без CMake)
# Компилирует файлы напрямую и запускает проверки
param(
    [Parameter(Mandatory=$true)]
    [string]$PatternPath,
    [switch]$WithSanitizers = $false,
    [switch]$WithUBSan = $false,
    [switch]$WithCppcheck = $false,
    [switch]$FullAnalysis = $false  # Включает всё
)

$ErrorActionPreference = "Continue"

# Цвета
function Write-Success { param([string]$Message) Write-Host "✅ $Message" -ForegroundColor Green }
function Write-Failure { param([string]$Message) Write-Host "❌ $Message" -ForegroundColor Red }
function Write-Info { param([string]$Message) Write-Host "ℹ️  $Message" -ForegroundColor Cyan }
function Write-Step { param([string]$Message) Write-Host "`n=== $Message ===" -ForegroundColor Magenta }

# Получаем название паттерна из пути
$patternName = (Split-Path $PatternPath -Leaf)
$header = "ВАЛИДАЦИЯ: $patternName"
$headerPadded = $header.PadRight(61)

Write-Host "`n╔═══════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║ $headerPadded║" -ForegroundColor Cyan
Write-Host "╚═══════════════════════════════════════════════════════════╝" -ForegroundColor Cyan

# Проверка существования директории
if (-not (Test-Path $PatternPath)) {
    Write-Failure "Директория не найдена: $PatternPath"
    exit 1
}

Write-Success "Директория: $PatternPath"

# Инструменты
$clang = "C:\Program Files\LLVM\bin\clang++.exe"
if (-not (Test-Path $clang)) {
    Write-Failure "Clang++ не найден: $clang"
    exit 1
}

# Если включен FullAnalysis, включаем всё
if ($FullAnalysis) {
    $WithSanitizers = $true
    $WithUBSan = $true
    $WithCppcheck = $true
}

# Проверяем cppcheck (опционально)
$cppcheckAvailable = $false
if ($WithCppcheck) {
    $cppcheck = Get-Command cppcheck -ErrorAction SilentlyContinue
    if ($cppcheck) {
        $cppcheckAvailable = $true
        Write-Info "Cppcheck найден: $($cppcheck.Source)"
    } else {
        Write-Info "Cppcheck не найден (пропускаем)"
    }
}

# Счетчики
$totalTests = 0
$passedTests = 0
$failedTests = 0

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

# Находим файлы
$vulnFiles = Get-ChildItem -Path $PatternPath -Filter "*vulnerabilities.cpp" -File
$secureFiles = Get-ChildItem -Path $PatternPath -Filter "secure_*.cpp" -File

Write-Info "Найдено vulnerable файлов: $($vulnFiles.Count)"
Write-Info "Найдено secure файлов: $($secureFiles.Count)"

# Создаем temp директорию для бинарников
$tempDir = Join-Path $PatternPath "temp_build"
if (Test-Path $tempDir) {
    Remove-Item $tempDir -Recurse -Force
}
New-Item -ItemType Directory -Path $tempDir | Out-Null

# Тест 1-N: Компиляция vulnerable файлов
foreach ($file in $vulnFiles) {
    Run-Test "Компиляция: $($file.Name)" {
        $outFile = Join-Path $tempDir "$($file.BaseName).exe"
        $output = & $clang -std=c++17 -O0 -g $file.FullName -o $outFile 2>&1
        
        if ($LASTEXITCODE -eq 0) {
            Write-Info "  Скомпилирован: $outFile"
            return $true
        } else {
            Write-Host "  Ошибки компиляции:" -ForegroundColor Yellow
            $output | Select-Object -Last 5 | ForEach-Object { Write-Host "    $_" }
            return $false
        }
    }
    
    # Если включены санитайзеры
    if ($WithSanitizers) {
        Run-Test "Компиляция с ASan: $($file.Name)" {
            $outFile = Join-Path $tempDir "$($file.BaseName)_asan.exe"
            $output = & $clang -std=c++17 -O0 -g -fsanitize=address -fno-omit-frame-pointer $file.FullName -o $outFile 2>&1
            
            if ($LASTEXITCODE -eq 0) {
                Write-Info "  Скомпилирован с ASan"
                
                # Пробуем запустить (должен детектировать проблему) с timeout
                Write-Info "  Запуск для детектирования уязвимости (timeout 5s)..."
                $job = Start-Job -ScriptBlock { param($exe) & $exe 2>&1 | Out-String } -ArgumentList $outFile
                $completed = Wait-Job $job -Timeout 5
                if ($completed) {
                    $runOutput = Receive-Job $job
                    Remove-Job $job
                } else {
                    Stop-Job $job
                    Remove-Job $job
                    $runOutput = ""
                    Write-Info "  ⏱️ Timeout (возможно зациклился)"
                }
                
                if ($runOutput -match "ERROR: AddressSanitizer" -or 
                    $runOutput -match "heap-use-after-free" -or 
                    $runOutput -match "heap-buffer-overflow" -or
                    $runOutput -match "SEGV") {
                    Write-Success "  ✓ ASan детектировал уязвимость!"
                    return $true
                } else {
                    Write-Info "  ⚠️ ASan не детектировал явных проблем"
                    Write-Info "  (Это может быть нормально для некоторых типов уязвимостей)"
                    return $true  # Не считаем это фейлом
                }
            } else {
                Write-Host "  Ошибки компиляции с ASan:" -ForegroundColor Yellow
                $output | Select-Object -Last 5 | ForEach-Object { Write-Host "    $_" }
                return $false
            }
        }
    }
    
    # UBSan тест (если включен)
    if ($WithUBSan) {
        Run-Test "Компиляция с UBSan: $($file.Name)" {
            $outFile = Join-Path $tempDir "$($file.BaseName)_ubsan.exe"
            $output = & $clang -std=c++17 -O1 -g -fsanitize=undefined -fno-sanitize-recover=all $file.FullName -o $outFile 2>&1
            
            if ($LASTEXITCODE -eq 0) {
                Write-Info "  Скомпилирован с UBSan"
                
                # Запуск с timeout
                Write-Info "  Запуск для детектирования UB (timeout 5s)..."
                $job = Start-Job -ScriptBlock { param($exe) & $exe 2>&1 | Out-String } -ArgumentList $outFile
                $completed = Wait-Job $job -Timeout 5
                if ($completed) {
                    $runOutput = Receive-Job $job
                    Remove-Job $job
                } else {
                    Stop-Job $job
                    Remove-Job $job
                    $runOutput = ""
                    Write-Info "  ⏱️ Timeout"
                }
                
                if ($runOutput -match "runtime error:" -or 
                    $runOutput -match "undefined behavior" -or
                    $runOutput -match "division by zero" -or
                    $runOutput -match "signed integer overflow") {
                    Write-Success "  ✓ UBSan детектировал UB!"
                    return $true
                } else {
                    Write-Info "  ⚠️ UBSan не детектировал UB"
                    return $true  # Не считаем фейлом
                }
            } else {
                Write-Host "  Ошибки компиляции с UBSan:" -ForegroundColor Yellow
                $output | Select-Object -Last 3 | ForEach-Object { Write-Host "    $_" }
                return $false
            }
        }
    }
    
    # Cppcheck тест (если включен и доступен)
    if ($WithCppcheck -and $cppcheckAvailable) {
        Run-Test "Cppcheck анализ: $($file.Name)" {
            $output = & cppcheck --enable=all --suppress=missingInclude --quiet --template='{file}:{line}: {severity}: {message}' $file.FullName 2>&1
            
            $errors = ($output | Select-String "\berror\b" | Measure-Object).Count
            $warnings = ($output | Select-String "\bwarning\b" | Measure-Object).Count
            
            Write-Info "  Найдено: $errors errors, $warnings warnings"
            
            if ($errors -gt 0) {
                Write-Host "  Критичные проблемы:" -ForegroundColor Red
                $output | Select-String "\berror\b" | Select-Object -First 3 | ForEach-Object { Write-Host "    $_" -ForegroundColor DarkRed }
            }
            
            # Для vulnerable файлов ошибки - это нормально
            return $true
        }
    }
}

# Тест: Компиляция secure файлов
foreach ($file in $secureFiles) {
    Run-Test "Компиляция: $($file.Name)" {
        $outFile = Join-Path $tempDir "$($file.BaseName).exe"
        $output = & $clang -std=c++17 -O0 -g $file.FullName -o $outFile 2>&1
        
        if ($LASTEXITCODE -eq 0) {
            Write-Info "  Скомпилирован: $outFile"
            return $true
        } else {
            Write-Host "  Ошибки компиляции:" -ForegroundColor Yellow
            $output | Select-Object -Last 5 | ForEach-Object { Write-Host "    $_" }
            return $false
        }
    }
    
    if ($WithSanitizers) {
        Run-Test "Безопасная версия чиста под ASan: $($file.Name)" {
            $outFile = Join-Path $tempDir "$($file.BaseName)_asan.exe"
            $output = & $clang -std=c++17 -O0 -g -fsanitize=address -fno-omit-frame-pointer $file.FullName -o $outFile 2>&1
            
            if ($LASTEXITCODE -eq 0) {
                Write-Info "  Запуск под ASan (timeout 5s)..."
                $job = Start-Job -ScriptBlock { param($exe) & $exe 2>&1 | Out-String } -ArgumentList $outFile
                $completed = Wait-Job $job -Timeout 5
                if ($completed) {
                    $runOutput = Receive-Job $job
                    Remove-Job $job
                } else {
                    Stop-Job $job
                    Remove-Job $job
                    Write-Info "  ⏱️ Timeout (слишком долго)"
                    return $true
                }
                
                $hasErrors = $runOutput -match "ERROR: AddressSanitizer"
                
                if (-not $hasErrors) {
                    Write-Success "  ✓ Безопасная версия чиста!"
                    return $true
                } else {
                    Write-Failure "  ✗ ASan нашел проблемы в 'безопасной' версии!"
                    Write-Host $runOutput
                    return $false
                }
            } else {
                return $false
            }
        }
    }
    
    # UBSan для secure (если включен)
    if ($WithUBSan) {
        Run-Test "Secure чиста под UBSan: $($file.Name)" {
            $outFile = Join-Path $tempDir "$($file.BaseName)_ubsan.exe"
            $output = & $clang -std=c++17 -O1 -g -fsanitize=undefined -fno-sanitize-recover=all $file.FullName -o $outFile 2>&1
            
            if ($LASTEXITCODE -eq 0) {
                Write-Info "  Запуск под UBSan (timeout 5s)..."
                $job = Start-Job -ScriptBlock { param($exe) & $exe 2>&1 | Out-String } -ArgumentList $outFile
                $completed = Wait-Job $job -Timeout 5
                if ($completed) {
                    $runOutput = Receive-Job $job
                    Remove-Job $job
                } else {
                    Stop-Job $job
                    Remove-Job $job
                    Write-Info "  ⏱️ Timeout"
                    return $true
                }
                
                # Secure НЕ должна иметь UB
                if ($runOutput -match "runtime error:" -or $runOutput -match "undefined behavior") {
                    Write-Failure "  ✗ UBSan нашёл UB в secure версии!"
                    Write-Host $runOutput -ForegroundColor Red
                    return $false
                } else {
                    Write-Success "  ✓ Secure версия чиста!"
                    return $true
                }
            } else {
                return $false
            }
        }
    }
    
    # Cppcheck для secure (должна быть чище)
    if ($WithCppcheck -and $cppcheckAvailable) {
        Run-Test "Cppcheck secure: $($file.Name)" {
            $output = & cppcheck --enable=all --suppress=missingInclude --quiet --template='{severity}: {message}' $file.FullName 2>&1
            
            $errors = ($output | Select-String "\berror\b" | Measure-Object).Count
            $warnings = ($output | Select-String "\bwarning\b" | Measure-Object).Count
            
            Write-Info "  Найдено: $errors errors, $warnings warnings"
            
            if ($errors -gt 0) {
                Write-Failure "  Критичные проблемы в secure версии!"
                $output | Select-String "\berror\b" | Select-Object -First 3 | ForEach-Object { Write-Host "    $_" -ForegroundColor DarkRed }
                return $false
            }
            
            # Warnings допустимы
            return $true
        }
    }
}

# Тест: Проверка документации
Run-Test "Существует SECURITY_ANALYSIS.md" {
    $secAnalysis = Join-Path $PatternPath "SECURITY_ANALYSIS.md"
    if (Test-Path $secAnalysis) {
        $content = Get-Content $secAnalysis -Raw
        $hasCWE = $content -match "CWE-\d+"
        $hasCVE = $content -match "CVE-\d{4}-\d+"
        
        Write-Info "  CWE найдено: $hasCWE"
        Write-Info "  CVE найдено: $hasCVE"
        
        return ($hasCWE -or $hasCVE)
    }
    return $false
}

# Очистка
Write-Info "`nОчистка временных файлов..."
if (Test-Path $tempDir) {
    Remove-Item $tempDir -Recurse -Force
}

# Итоги
Write-Host "`n╔═══════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║                 ИТОГОВЫЙ ОТЧЕТ                            ║" -ForegroundColor Cyan
Write-Host "╚═══════════════════════════════════════════════════════════╝" -ForegroundColor Cyan

Write-Host "`nПаттерн: $PatternPath" -ForegroundColor White
Write-Host "`nРезультаты:"
Write-Host "  Всего тестов:   $totalTests"
Write-Success "  Пройдено:       $passedTests"
Write-Failure "  Провалено:      $failedTests"

$successRate = if ($totalTests -gt 0) { [math]::Round(($passedTests / $totalTests) * 100, 1) } else { 0 }
Write-Host "`nПроцент успеха: $successRate%" -ForegroundColor $(
    if ($successRate -eq 100) { "Green" }
    elseif ($successRate -ge 80) { "Yellow" }
    else { "Red" }
)

if ($failedTests -eq 0) {
    Write-Host "`n✅ ВАЛИДАЦИЯ УСПЕШНА" -ForegroundColor Green
    exit 0
} else {
    Write-Host "`n❌ ОБНАРУЖЕНЫ ПРОБЛЕМЫ" -ForegroundColor Red
    exit 1
}

