# Валидация concurrency паттерна с ThreadSanitizer
param(
    [Parameter(Mandatory=$true)]
    [string]$PatternPath
)

$ErrorActionPreference = "Continue"
$clang = "C:\Program Files\LLVM\bin\clang++.exe"

Write-Host "`n╔═══════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║     ВАЛИДАЦИЯ С THREADSANITIZER                           ║" -ForegroundColor Cyan
Write-Host "╚═══════════════════════════════════════════════════════════╝" -ForegroundColor Cyan

if (-not (Test-Path $clang)) {
    Write-Host "❌ Clang++ не найден!" -ForegroundColor Red
    exit 1
}

if (-not (Test-Path $PatternPath)) {
    Write-Host "❌ Паттерн не найден: $PatternPath" -ForegroundColor Red
    exit 1
}

Write-Host "✅ Паттерн: $PatternPath" -ForegroundColor Green

# Находим файлы
$vulnFiles = Get-ChildItem -Path $PatternPath -Filter "*vulnerabilities.cpp" -File
$secureFiles = Get-ChildItem -Path $PatternPath -Filter "secure_*.cpp" -File

# Создаем temp директорию
$tempDir = Join-Path $PatternPath "temp_tsan"
if (Test-Path $tempDir) {
    Remove-Item $tempDir -Recurse -Force
}
New-Item -ItemType Directory -Path $tempDir | Out-Null

$results = @{
    vuln_compiled = $false
    vuln_detected = $false
    secure_compiled = $false
    secure_clean = $false
}

# Компилируем и запускаем vulnerable с TSan
Write-Host "`n[1] Компиляция vulnerable с ThreadSanitizer..." -ForegroundColor Yellow
foreach ($file in $vulnFiles) {
    $outFile = Join-Path $tempDir "$($file.BaseName)_tsan.exe"
    
    Write-Host "  Компиляция: $($file.Name)" -NoNewline
    $compileOutput = & $clang -std=c++17 -O1 -g -fsanitize=thread $file.FullName -o $outFile 2>&1
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host " ✅" -ForegroundColor Green
        $results.vuln_compiled = $true
        
        # Запуск
        Write-Host "  Запуск (timeout 10 сек)..." -NoNewline
        $job = Start-Job -ScriptBlock {
            param($exe)
            & $exe 2>&1 | Out-String
        } -ArgumentList $outFile
        
        $timeout = Wait-Job $job -Timeout 10
        if ($timeout) {
            $output = Receive-Job $job
            Stop-Job $job
            Remove-Job $job
            
            if ($output -match "WARNING: ThreadSanitizer" -or 
                $output -match "data race" -or 
                $output -match "race on") {
                Write-Host " ✅ TSan детектировал race condition!" -ForegroundColor Green
                $results.vuln_detected = $true
                
                # Показываем фрагмент
                $lines = $output -split "`n" | Where-Object { $_ -match "ThreadSanitizer|race|WARNING" } | Select-Object -First 5
                foreach ($line in $lines) {
                    Write-Host "    $line" -ForegroundColor Yellow
                }
            } else {
                Write-Host " ⚠️  TSan не детектировал проблем" -ForegroundColor Yellow
            }
        } else {
            Write-Host " ⏱️  Timeout (возможно зациклилось)" -ForegroundColor Yellow
            Stop-Job $job
            Remove-Job $job
        }
    } else {
        Write-Host " ❌" -ForegroundColor Red
        Write-Host "  Ошибка компиляции:" -ForegroundColor Red
        $compileOutput | Select-Object -Last 3 | ForEach-Object { Write-Host "    $_" -ForegroundColor DarkRed }
    }
}

# Компилируем и запускаем secure с TSan
Write-Host "`n[2] Компиляция secure с ThreadSanitizer..." -ForegroundColor Yellow
foreach ($file in $secureFiles) {
    $outFile = Join-Path $tempDir "$($file.BaseName)_tsan.exe"
    
    Write-Host "  Компиляция: $($file.Name)" -NoNewline
    $compileOutput = & $clang -std=c++17 -O1 -g -fsanitize=thread $file.FullName -o $outFile 2>&1
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host " ✅" -ForegroundColor Green
        $results.secure_compiled = $true
        
        # Запуск
        Write-Host "  Запуск (timeout 10 сек)..." -NoNewline
        $job = Start-Job -ScriptBlock {
            param($exe)
            & $exe 2>&1 | Out-String
        } -ArgumentList $outFile
        
        $timeout = Wait-Job $job -Timeout 10
        if ($timeout) {
            $output = Receive-Job $job
            Stop-Job $job
            Remove-Job $job
            
            if ($output -match "WARNING: ThreadSanitizer" -or 
                $output -match "data race") {
                Write-Host " ❌ TSan детектировал проблемы!" -ForegroundColor Red
                $results.secure_clean = $false
                
                $lines = $output -split "`n" | Where-Object { $_ -match "ThreadSanitizer|race|WARNING" } | Select-Object -First 5
                foreach ($line in $lines) {
                    Write-Host "    $line" -ForegroundColor Red
                }
            } else {
                Write-Host " ✅ Безопасная версия чиста!" -ForegroundColor Green
                $results.secure_clean = $true
            }
        } else {
            Write-Host " ⏱️  Timeout" -ForegroundColor Yellow
            Stop-Job $job
            Remove-Job $job
        }
    } else {
        Write-Host " ❌" -ForegroundColor Red
    }
}

# Очистка
Remove-Item $tempDir -Recurse -Force -ErrorAction SilentlyContinue

# Итоги
Write-Host "`n╔═══════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║                    ИТОГОВЫЙ ОТЧЕТ                          ║" -ForegroundColor Cyan
Write-Host "╚═══════════════════════════════════════════════════════════╝" -ForegroundColor Cyan

Write-Host "`nРезультаты:" -ForegroundColor White
Write-Host "  Vulnerable компилируется:  $(if($results.vuln_compiled){'✅'}else{'❌'})" -ForegroundColor $(if($results.vuln_compiled){'Green'}else{'Red'})
Write-Host "  TSan детектировал уязвимость: $(if($results.vuln_detected){'✅'}else{'⚠️ '})" -ForegroundColor $(if($results.vuln_detected){'Green'}else{'Yellow'})
Write-Host "  Secure компилируется:      $(if($results.secure_compiled){'✅'}else{'❌'})" -ForegroundColor $(if($results.secure_compiled){'Green'}else{'Red'})
Write-Host "  Secure версия чиста:       $(if($results.secure_clean){'✅'}else{'❌'})" -ForegroundColor $(if($results.secure_clean){'Green'}else{'Red'})

if ($results.vuln_compiled -and $results.secure_compiled -and $results.secure_clean) {
    Write-Host "`n✅ ВАЛИДАЦИЯ УСПЕШНА" -ForegroundColor Green
} else {
    Write-Host "`n⚠️  ОБНАРУЖЕНЫ ПРОБЛЕМЫ" -ForegroundColor Yellow
}

