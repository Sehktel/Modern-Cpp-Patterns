# Валидация всех паттернов в проекте
# Использование: .\tools\validate_all_patterns.ps1 [-Full] [-Parallel]

param(
    [switch]$Full = $false,
    [switch]$Parallel = $false,
    [string]$ReportFile = "validation_report.md"
)

$ErrorActionPreference = "Continue"

Write-Host @"

╔════════════════════════════════════════════════════════════════╗
║                                                                ║
║     КОМПЛЕКСНАЯ ВАЛИДАЦИЯ ВСЕХ ПАТТЕРНОВ                       ║
║     Design Patterns in Modern C++ Security Course              ║
║                                                                ║
╚════════════════════════════════════════════════════════════════╝

"@ -ForegroundColor Cyan

Write-Host "Дата: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Gray
Write-Host "Режим: $(if ($Full) { 'Полная проверка' } else { 'Быстрая проверка' })" -ForegroundColor Gray
Write-Host ""

# Поиск всех директорий паттернов
$patternDirs = @(
    Get-ChildItem -Path "01-basics" -Directory -Recurse -Filter "lesson_*",
    Get-ChildItem -Path "02-principles" -Directory -Recurse -Filter "lesson_*",
    Get-ChildItem -Path "03-creational" -Directory -Recurse -Filter "lesson_*",
    Get-ChildItem -Path "04-structural" -Directory -Recurse -Filter "lesson_*",
    Get-ChildItem -Path "05-behavioral" -Directory -Recurse -Filter "lesson_*",
    Get-ChildItem -Path "06-modern-cpp" -Directory -Recurse -Filter "lesson_*",
    Get-ChildItem -Path "07-concurrency" -Directory -Recurse -Filter "lesson_*",
    Get-ChildItem -Path "08-high-load" -Directory -Recurse -Filter "lesson_*",
    Get-ChildItem -Path "09-performance" -Directory -Recurse -Filter "lesson_*"
) | Where-Object { $_ -ne $null }

Write-Host "Найдено паттернов для валидации: $($patternDirs.Count)" -ForegroundColor Yellow
Write-Host ""

# Результаты валидации
$results = @()

# Счетчики
$totalPatterns = $patternDirs.Count
$passedPatterns = 0
$failedPatterns = 0
$current = 0

# Валидация каждого паттерна
foreach ($dir in $patternDirs) {
    $current++
    $patternName = $dir.Name
    
    Write-Host "[$current/$totalPatterns] Валидация: $patternName" -ForegroundColor Cyan
    Write-Host ("─" * 80) -ForegroundColor Gray
    
    # Извлечение краткого имени для скрипта валидации
    $shortName = $patternName -replace "lesson_\d+_\d+_", ""
    
    try {
        # Запуск валидации паттерна
        $output = & ".\tools\validate_pattern.ps1" -Pattern $shortName -Full:$Full 2>&1
        $exitCode = $LASTEXITCODE
        
        $result = @{
            Pattern = $patternName
            Directory = $dir.FullName
            Success = ($exitCode -eq 0)
            Output = $output -join "`n"
        }
        
        $results += [PSCustomObject]$result
        
        if ($exitCode -eq 0) {
            Write-Host "✅ PASS: $patternName" -ForegroundColor Green
            $passedPatterns++
        } else {
            Write-Host "❌ FAIL: $patternName" -ForegroundColor Red
            $failedPatterns++
        }
    } catch {
        Write-Host "❌ ERROR: $patternName - $_" -ForegroundColor Red
        $failedPatterns++
        
        $result = @{
            Pattern = $patternName
            Directory = $dir.FullName
            Success = $false
            Output = "Exception: $_"
        }
        
        $results += [PSCustomObject]$result
    }
    
    Write-Host ""
}

# ============================================================================
# ГЕНЕРАЦИЯ ОТЧЕТА
# ============================================================================

Write-Host "`n╔════════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host   "║                      ИТОГОВЫЙ ОТЧЕТ                            ║" -ForegroundColor Cyan
Write-Host   "╚════════════════════════════════════════════════════════════════╝`n" -ForegroundColor Cyan

$successRate = if ($totalPatterns -gt 0) { [math]::Round(($passedPatterns / $totalPatterns) * 100, 1) } else { 0 }

Write-Host "Всего паттернов:    $totalPatterns"
Write-Host "Пройдено:           $passedPatterns" -ForegroundColor Green
Write-Host "Провалено:          $failedPatterns" -ForegroundColor $(if ($failedPatterns -gt 0) { "Red" } else { "Green" })
Write-Host "Процент успеха:     $successRate%" -ForegroundColor $(
    if ($successRate -eq 100) { "Green" }
    elseif ($successRate -ge 80) { "Yellow" }
    else { "Red" }
)

# Сохранение markdown отчета
Write-Host "`nГенерация отчета: $ReportFile" -ForegroundColor Yellow

$reportContent = @"
# Отчет валидации курса "Design Patterns in Modern C++"

**Дата**: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')  
**Режим**: $(if ($Full) { 'Полная проверка (с санитайзерами и статическим анализом)' } else { 'Быстрая проверка (компиляция и структура)' })

---

## 📊 Общая статистика

| Метрика | Значение |
|---------|----------|
| Всего паттернов | $totalPatterns |
| ✅ Пройдено | $passedPatterns |
| ❌ Провалено | $failedPatterns |
| **Процент успеха** | **$successRate%** |

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
    $moduleName = ($result.Directory -split '\\')[1]
    if ($modules.ContainsKey($moduleName)) {
        $modules[$moduleName] += $result
    }
}

foreach ($moduleName in $modules.Keys | Sort-Object) {
    $moduleResults = $modules[$moduleName]
    if ($moduleResults.Count -eq 0) { continue }
    
    $modulePassed = ($moduleResults | Where-Object { $_.Success }).Count
    $moduleTotal = $moduleResults.Count
    $moduleRate = if ($moduleTotal -gt 0) { [math]::Round(($modulePassed / $moduleTotal) * 100, 1) } else { 0 }
    
    $reportContent += @"

### $moduleName ($modulePassed/$moduleTotal, $moduleRate%)

| Паттерн | Статус |
|---------|--------|
"@
    
    foreach ($result in $moduleResults | Sort-Object Pattern) {
        $status = if ($result.Success) { "✅ PASS" } else { "❌ FAIL" }
        $reportContent += "`n| $($result.Pattern) | $status |"
    }
    
    $reportContent += "`n"
}

# Проблемные паттерны
$failedResults = $results | Where-Object { -not $_.Success }
if ($failedResults.Count -gt 0) {
    $reportContent += @"

---

## ⚠️ Проблемные паттерны

Следующие паттерны требуют внимания:

"@
    
    foreach ($result in $failedResults) {
        $reportContent += @"

### ❌ $($result.Pattern)

**Директория**: ``$($result.Directory)``

**Вывод**:
``````
$($result.Output | Select-Object -First 50 -join "`n")
``````

"@
    }
}

# Рекомендации
$reportContent += @"

---

## 💡 Рекомендации

"@

if ($successRate -eq 100) {
    $reportContent += @"
✅ **Отлично!** Все паттерны прошли валидацию.

### Дальнейшие шаги:
1. Поддерживать актуальность кода
2. Регулярно запускать валидацию (рекомендуется: еженедельно)
3. Добавить CI/CD интеграцию для автоматической валидации
"@
} elseif ($successRate -ge 80) {
    $reportContent += @"
⚠️ **Хорошо**, но есть проблемы в некоторых паттернах.

### Приоритетные действия:
1. Исправить провалившиеся паттерны (см. раздел "Проблемные паттерны")
2. Запустить повторную валидацию
3. Проверить SECURITY_ANALYSIS.md на корректность CVE/CWE
"@
} else {
    $reportContent += @"
❌ **Требуется внимание!** Значительное количество паттернов не прошло валидацию.

### Критические действия:
1. **Немедленно** проверить все провалившиеся паттерны
2. Убедиться, что код компилируется
3. Проверить, что санитайзеры установлены и работают
4. Пересмотреть уязвимые версии - они должны быть реалистичными
5. Проверить безопасные версии - они должны быть действительно безопасными
"@
}

$reportContent += @"

---

## 🛠️ Инструменты верификации

### Использованные инструменты:
- ✅ **CMake** - компиляция
- $(if ($Full) { '✅' } else { '⏸️' }) **AddressSanitizer** - детектирование memory issues
- $(if ($Full) { '✅' } else { '⏸️' }) **ThreadSanitizer** - детектирование race conditions
- $(if ($Full) { '✅' } else { '⏸️' }) **Clang-Tidy** - статический анализ
- $(if ($Full) { '✅' } else { '⏸️' }) **Cppcheck** - статический анализ

### Для полной валидации:
``````powershell
.\tools\validate_all_patterns.ps1 -Full
``````

---

## 📚 Дополнительные ресурсы

- [VERIFICATION_STRATEGY.md](tools/VERIFICATION_STRATEGY.md) - Полная стратегия верификации
- [ANALYSIS_TOOLS_GUIDE.md](tools/ANALYSIS_TOOLS_GUIDE.md) - Руководство по инструментам
- [validate_pattern.ps1](tools/validate_pattern.ps1) - Валидация одного паттерна

---

**Сгенерировано**: validate_all_patterns.ps1  
**Версия**: 1.0  
**Maintainer**: Security Team
"@

# Сохранение отчета
$reportContent | Out-File -FilePath $ReportFile -Encoding UTF8
Write-Host "✅ Отчет сохранен: $ReportFile" -ForegroundColor Green

# Финальный вердикт
Write-Host ""
if ($failedPatterns -eq 0) {
    Write-Host "╔════════════════════════════════════════════════════════════════╗" -ForegroundColor Green
    Write-Host "║                                                                ║" -ForegroundColor Green
    Write-Host "║              ✅ ВСЕ ПАТТЕРНЫ ВАЛИДНЫ ✅                         ║" -ForegroundColor Green
    Write-Host "║                                                                ║" -ForegroundColor Green
    Write-Host "║  Курс готов к использованию в production!                      ║" -ForegroundColor Green
    Write-Host "║                                                                ║" -ForegroundColor Green
    Write-Host "╚════════════════════════════════════════════════════════════════╝" -ForegroundColor Green
    exit 0
} else {
    Write-Host "╔════════════════════════════════════════════════════════════════╗" -ForegroundColor Red
    Write-Host "║                                                                ║" -ForegroundColor Red
    Write-Host "║           ⚠️  ОБНАРУЖЕНЫ ПРОБЛЕМЫ ⚠️                           ║" -ForegroundColor Red
    Write-Host "║                                                                ║" -ForegroundColor Red
    Write-Host "║  Провалено паттернов: $($failedPatterns.ToString().PadRight(46)) ║" -ForegroundColor Red
    Write-Host "║  См. детали в: $($ReportFile.PadRight(45)) ║" -ForegroundColor Red
    Write-Host "║                                                                ║" -ForegroundColor Red
    Write-Host "╚════════════════════════════════════════════════════════════════╝" -ForegroundColor Red
    exit 1
}

