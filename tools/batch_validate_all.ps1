# Массовая валидация всех паттернов проекта
# Использует simple_validate_pattern.ps1 для каждого паттерна
param(
    [switch]$WithSanitizers = $false,
    [switch]$WithUBSan = $false,
    [switch]$WithCppcheck = $false,
    [switch]$FullAnalysis = $false,
    [string]$OutputFile = "validation_batch_report.md"
)

$ErrorActionPreference = "Continue"

Write-Host "`n╔════════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║           МАССОВАЯ ВАЛИДАЦИЯ ВСЕХ ПАТТЕРНОВ                    ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════════╝`n" -ForegroundColor Cyan

$startTime = Get-Date

# Список всех директорий с паттернами
$patternDirs = @(
    # Module 01: Basics
    "01-basics\lesson_1_1_hello_world",
    "01-basics\lesson_1_2_raii",
    "01-basics\lesson_1_3_smart_pointers",
    "01-basics\lesson_1_4_move_semantics",
    
    # Module 02: Principles  
    "02-principles\lesson_2_1_solid",
    "02-principles\lesson_2_2_dry_kiss_yagni",
    "02-principles\lesson_2_3_composition_inheritance",
    "02-principles\lesson_2_4_dependency_injection",
    
    # Module 03: Creational
    "03-creational\lesson_3_1_singleton",
    "03-creational\lesson_3_2_factory_method",
    "03-creational\lesson_3_3_abstract_factory",
    "03-creational\lesson_3_4_builder",
    
    # Module 04: Structural
    "04-structural\lesson_4_1_adapter",
    "04-structural\lesson_4_2_decorator",
    "04-structural\lesson_4_3_facade",
    "04-structural\lesson_4_4_proxy",
    
    # Module 05: Behavioral
    "05-behavioral\lesson_5_1_observer",
    "05-behavioral\lesson_5_2_strategy",
    "05-behavioral\lesson_5_3_command",
    "05-behavioral\lesson_5_4_state",
    
    # Module 06: Modern C++
    "06-modern-cpp\lesson_6_1_smart_pointers",
    "06-modern-cpp\lesson_6_2_move_semantics",
    "06-modern-cpp\lesson_6_3_crtp",
    "06-modern-cpp\lesson_6_4_type_erasure",
    
    # Module 07: Concurrency
    "07-concurrency\lesson_7_1_producer_consumer",
    "07-concurrency\lesson_7_2_thread_pool",
    "07-concurrency\lesson_7_3_actor_model",
    "07-concurrency\lesson_7_4_reactor",
    
    # Module 08: High-Load
    "08-high-load\lesson_8_1_cache_aside",
    "08-high-load\lesson_8_2_circuit_breaker",
    "08-high-load\lesson_8_3_bulkhead",
    "08-high-load\lesson_8_4_saga",
    
    # Module 09: Performance
    "09-performance\lesson_9_1_object_pool",
    "09-performance\lesson_9_2_flyweight",
    "09-performance\lesson_9_3_command_queue"
)

$results = @()
$totalPatterns = $patternDirs.Count
$current = 0

Write-Host "Найдено паттернов для валидации: $totalPatterns`n" -ForegroundColor Yellow

foreach ($dir in $patternDirs) {
    $current++
    $patternName = Split-Path $dir -Leaf
    
    Write-Host "[$current/$totalPatterns] Валидация: $patternName" -ForegroundColor Cyan
    Write-Host ("─" * 80) -ForegroundColor Gray
    
    if (-not (Test-Path $dir)) {
        Write-Host "⚠️  Директория не найдена: $dir" -ForegroundColor Yellow
        $results += [PSCustomObject]@{
            Pattern = $patternName
            Path = $dir
            Success = $false
            SuccessRate = 0
            Status = "NOT_FOUND"
            Details = "Директория не существует"
        }
        continue
    }
    
    # Запуск валидации с выбранными опциями
    $output = ""
    $params = @{PatternPath = $dir}
    
    if ($FullAnalysis) {
        $params.FullAnalysis = $true
    } else {
        if ($WithSanitizers) { $params.WithSanitizers = $true }
        if ($WithUBSan) { $params.WithUBSan = $true }
        if ($WithCppcheck) { $params.WithCppcheck = $true }
    }
    
    $output = & ".\tools\simple_validate_pattern.ps1" @params 2>&1 | Out-String
    
    $exitCode = $LASTEXITCODE
    
    # Парсим результат
    $successRate = 0
    if ($output -match "Процент успеха:\s+(\d+\.?\d*)%") {
        $successRate = [double]$matches[1]
    }
    
    $passed = 0
    $failed = 0
    if ($output -match "Пройдено:\s+(\d+)") {
        $passed = [int]$matches[1]
    }
    if ($output -match "Провалено:\s+(\d+)") {
        $failed = [int]$matches[1]
    }
    
    $status = if ($exitCode -eq 0) { "SUCCESS" } else { "FAILED" }
    
    $results += [PSCustomObject]@{
        Pattern = $patternName
        Path = $dir
        Success = ($exitCode -eq 0)
        SuccessRate = $successRate
        Passed = $passed
        Failed = $failed
        Status = $status
        Details = $output
    }
    
    # Краткий вывод
    $color = if ($successRate -ge 80) { "Green" } elseif ($successRate -ge 50) { "Yellow" } else { "Red" }
    Write-Host "  Результат: $successRate% ($passed/$($passed+$failed) тестов)" -ForegroundColor $color
    Write-Host ""
}

# Генерация отчета
$endTime = Get-Date
$duration = $endTime - $startTime

Write-Host "`n╔════════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║                      ИТОГОВЫЙ ОТЧЕТ                            ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════════╝`n" -ForegroundColor Cyan

$successCount = ($results | Where-Object { $_.Success }).Count
$avgSuccessRate = ($results | Measure-Object -Property SuccessRate -Average).Average

Write-Host "Всего паттернов:      $totalPatterns"
Write-Host "Полностью успешных:   $successCount" -ForegroundColor Green
Write-Host "С проблемами:         $($totalPatterns - $successCount)" -ForegroundColor Red
Write-Host "Средний % успеха:     $([math]::Round($avgSuccessRate, 1))%"
Write-Host "Время выполнения:     $($duration.ToString('hh\:mm\:ss'))"

# Сохранение markdown отчета
$reportContent = @"
# Отчет массовой валидации курса "Design Patterns in Modern C++"

**Дата**: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')  
**Режим**: $(if ($WithSanitizers) { 'С санитайзерами (ASan)' } else { 'Быстрая проверка' })  
**Время выполнения**: $($duration.ToString('hh\:mm\:ss'))

---

## 📊 Общая статистика

| Метрика | Значение |
|---------|----------|
| Всего паттернов | $totalPatterns |
| ✅ Полностью успешных | $successCount |
| ❌ С проблемами | $($totalPatterns - $successCount) |
| **Средний % успеха** | **$([math]::Round($avgSuccessRate, 1))%** |

---

## 📈 Детализация по модулям

"@

# Группировка по модулям
$modules = @{
    "01-basics" = @()
    "02-principles" = @()
    "03-creational" = @()
    "04-structural" = @()
    "05-behavioral" = @()
    "06-modern-cpp" = @()
    "07-concurrency" = @()
    "08-high-load" = @()
    "09-performance" = @()
}

foreach ($result in $results) {
    $moduleName = ($result.Path -split '\\')[0]
    if ($modules.ContainsKey($moduleName)) {
        $modules[$moduleName] += $result
    }
}

foreach ($moduleName in $modules.Keys | Sort-Object) {
    $moduleResults = $modules[$moduleName]
    if ($moduleResults.Count -eq 0) { continue }
    
    $moduleAvg = ($moduleResults | Measure-Object -Property SuccessRate -Average).Average
    $moduleSuccess = ($moduleResults | Where-Object { $_.SuccessRate -ge 80 }).Count
    
    $reportContent += @"

### $moduleName ($moduleSuccess/$($moduleResults.Count) успешных, avg: $([math]::Round($moduleAvg, 1))%)

| Паттерн | Успех % | Пройдено | Провалено | Статус |
|---------|---------|----------|-----------|--------|
"@
    
    foreach ($result in $moduleResults | Sort-Object Pattern) {
        $statusIcon = if ($result.SuccessRate -ge 80) { "✅" } elseif ($result.SuccessRate -ge 50) { "⚠️" } else { "❌" }
        $reportContent += "`n| $($result.Pattern) | $($result.SuccessRate)% | $($result.Passed) | $($result.Failed) | $statusIcon |"
    }
    
    $reportContent += "`n"
}

# Проблемные паттерны
$problematic = $results | Where-Object { $_.SuccessRate -lt 80 } | Sort-Object SuccessRate

if ($problematic.Count -gt 0) {
    $reportContent += @"

---

## ⚠️ Проблемные паттерны (< 80% успеха)

| Паттерн | Успех % | Проблемы |
|---------|---------|----------|
"@
    
    foreach ($result in $problematic) {
        $issues = ""
        if ($result.Details -match "Ошибки компиляции") {
            $issues += "Не компилируется; "
        }
        if ($result.Details -match "не детектировал") {
            $issues += "ASan не детектирует; "
        }
        if ($result.Details -match "CWE найдено: False") {
            $issues += "Нет CWE; "
        }
        
        $reportContent += "`n| $($result.Pattern) | $($result.SuccessRate)% | $issues |"
    }
}

$reportContent += @"

---

## 🎯 Рекомендации

"@

$criticalCount = ($results | Where-Object { $_.SuccessRate -lt 50 }).Count
$warningCount = ($results | Where-Object { $_.SuccessRate -ge 50 -and $_.SuccessRate -lt 80 }).Count

if ($criticalCount -gt 0) {
    $reportContent += @"

### 🔴 Критично ($criticalCount паттернов)

Паттерны с успехом < 50% требуют немедленного исправления:
"@
    
    foreach ($result in ($results | Where-Object { $_.SuccessRate -lt 50 } | Sort-Object SuccessRate)) {
        $reportContent += "`n- **$($result.Pattern)** ($($result.SuccessRate)%)"
    }
}

if ($warningCount -gt 0) {
    $reportContent += @"

### 🟡 Важно ($warningCount паттернов)

Паттерны с успехом 50-79% требуют проверки и улучшения:
"@
    
    foreach ($result in ($results | Where-Object { $_.SuccessRate -ge 50 -and $_.SuccessRate -lt 80 } | Sort-Object SuccessRate)) {
        $reportContent += "`n- **$($result.Pattern)** ($($result.SuccessRate)%)"
    }
}

$reportContent += @"

---

## 📚 Следующие шаги

1. **Исправить критичные проблемы** (паттерны < 50%)
2. **Улучшить проблемные паттерны** (50-79%)
3. **Повторить валидацию** после исправлений
4. **Провести полную валидацию с ThreadSanitizer** для concurrency паттернов

---

**Сгенерировано**: batch_validate_all.ps1  
**Версия**: 1.0
"@

# Сохранение отчета
$reportContent | Out-File -FilePath $OutputFile -Encoding UTF8

Write-Host "`n✅ Отчет сохранен: $OutputFile" -ForegroundColor Green
Write-Host "`nДля просмотра: cat $OutputFile`n" -ForegroundColor Cyan

# Возвращаем exit code
if ($successCount -eq $totalPatterns) {
    exit 0
} else {
    exit 1
}

