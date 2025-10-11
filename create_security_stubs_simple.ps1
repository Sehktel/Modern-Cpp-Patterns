# PowerShell скрипт для создания заглушек безопасности для всех модулей курса

# Массив всех модулей курса
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

Write-Host "=== СОЗДАНИЕ ЗАГЛУШЕК БЕЗОПАСНОСТИ ДЛЯ ВСЕХ МОДУЛЕЙ ===" -ForegroundColor Magenta
Write-Host ""

# Проверяем наличие шаблонов
$templates = @(
    "SECURITY_ANALYSIS_TEMPLATE.md",
    "VULNERABILITIES_TEMPLATE.cpp",
    "SECURE_ALTERNATIVES_TEMPLATE.cpp",
    "EXPLOITS_TEMPLATE.cpp",
    "CMAKE_TEMPLATE.txt"
)

foreach ($template in $templates) {
    if (-not (Test-Path $template)) {
        Write-Host "ОШИБКА: Файл $template не найден!" -ForegroundColor Red
        exit 1
    }
}

# Создаем заглушки для каждого модуля
foreach ($module in $modules) {
    if (Test-Path $module) {
        Write-Host "Создание заглушек безопасности для модуля: $module" -ForegroundColor Green
        
        # Получаем список всех уроков в модуле
        $lessons = Get-ChildItem -Path $module -Directory -Name "lesson_*" | Sort-Object
        
        foreach ($lesson in $lessons) {
            $lessonPath = Join-Path $module $lesson
            if (Test-Path $lessonPath) {
                $patternName = $lesson -replace 'lesson_\d+_\d+_', ''
                
                Write-Host "  Обработка урока: $lesson (паттерн: $patternName)" -ForegroundColor Yellow
                
                # Создаем директорию exploits если её нет
                $exploitsDir = Join-Path $lessonPath "exploits"
                if (-not (Test-Path $exploitsDir)) {
                    New-Item -ItemType Directory -Path $exploitsDir -Force | Out-Null
                }
                
                # Создаем SECURITY_ANALYSIS.md если его нет
                $securityAnalysisFile = Join-Path $lessonPath "SECURITY_ANALYSIS.md"
                if (-not (Test-Path $securityAnalysisFile)) {
                    $content = Get-Content "SECURITY_ANALYSIS_TEMPLATE.md" -Raw
                    $content = $content -replace '\{PATTERN_NAME\}', $patternName
                    $content | Out-File -FilePath $securityAnalysisFile -Encoding UTF8
                    Write-Host "    Создан SECURITY_ANALYSIS.md" -ForegroundColor Cyan
                }
                
                # Создаем {pattern}_vulnerabilities.cpp если его нет
                $vulnerabilitiesFile = Join-Path $lessonPath "${patternName}_vulnerabilities.cpp"
                if (-not (Test-Path $vulnerabilitiesFile)) {
                    $content = Get-Content "VULNERABILITIES_TEMPLATE.cpp" -Raw
                    $content = $content -replace '\{PATTERN_NAME\}', $patternName
                    $content = $content -replace '\{pattern\}', $patternName
                    $content | Out-File -FilePath $vulnerabilitiesFile -Encoding UTF8
                    Write-Host "    Создан $vulnerabilitiesFile" -ForegroundColor Cyan
                }
                
                # Создаем secure_{pattern}_alternatives.cpp если его нет
                $secureFile = Join-Path $lessonPath "secure_${patternName}_alternatives.cpp"
                if (-not (Test-Path $secureFile)) {
                    $content = Get-Content "SECURE_ALTERNATIVES_TEMPLATE.cpp" -Raw
                    $content = $content -replace '\{PATTERN_NAME\}', $patternName
                    $content = $content -replace '\{pattern\}', $patternName
                    $content | Out-File -FilePath $secureFile -Encoding UTF8
                    Write-Host "    Создан $secureFile" -ForegroundColor Cyan
                }
                
                # Создаем exploits/{pattern}_exploits.cpp если его нет
                $exploitsFile = Join-Path $exploitsDir "${patternName}_exploits.cpp"
                if (-not (Test-Path $exploitsFile)) {
                    $content = Get-Content "EXPLOITS_TEMPLATE.cpp" -Raw
                    $content = $content -replace '\{PATTERN_NAME\}', $patternName
                    $content = $content -replace '\{pattern\}', $patternName
                    $content | Out-File -FilePath $exploitsFile -Encoding UTF8
                    Write-Host "    Создан $exploitsFile" -ForegroundColor Cyan
                }
                
                # Создаем CMakeLists.txt если его нет
                $cmakeFile = Join-Path $lessonPath "CMakeLists.txt"
                if (-not (Test-Path $cmakeFile)) {
                    $content = Get-Content "CMAKE_TEMPLATE.txt" -Raw
                    $content = $content -replace '\{PATTERN_NAME\}', $patternName
                    $content = $content -replace '\{pattern\}', $patternName
                    $content | Out-File -FilePath $cmakeFile -Encoding UTF8
                    Write-Host "    Создан CMakeLists.txt" -ForegroundColor Cyan
                }
                
                # Создаем SECURITY_README.md если его нет
                $securityReadmeFile = Join-Path $lessonPath "SECURITY_README.md"
                if (-not (Test-Path $securityReadmeFile)) {
                    $readmeContent = @"
# $patternName Pattern - Анализ безопасности

## Обзор

Этот модуль демонстрирует паттерн $patternName с фокусом на анализе безопасности.

## Структура модуля

### Основные файлы
- ``${patternName}_pattern.cpp`` - Классическая реализация $patternName
- ``modern_${patternName}.cpp`` - Современная реализация $patternName
- ``${patternName}_vulnerabilities.cpp`` - Уязвимые реализации для анализа
- ``secure_${patternName}_alternatives.cpp`` - Безопасные альтернативы
- ``exploits/${patternName}_exploits.cpp`` - Практические эксплоиты

### Документация
- ``README.md`` - Основное описание паттерна
- ``SECURITY_ANALYSIS.md`` - Детальный анализ уязвимостей

## TODO: Добавить конкретные уязвимости для данного паттерна

## Сборка и запуск

### Основные цели
``````bash
make ${patternName}_pattern          # Классический $patternName
make modern_${patternName}           # Современный $patternName
``````

### Анализ безопасности
``````bash
make ${patternName}_vulnerabilities  # Уязвимые реализации
make ${patternName}_vulnerabilities_tsan  # С ThreadSanitizer
make ${patternName}_vulnerabilities_asan   # С AddressSanitizer
make ${patternName}_vulnerabilities_ubsan # С UndefinedBehaviorSanitizer
``````

## Предупреждение

⚠️ **ВНИМАНИЕ**: Код в этом модуле содержит уязвимости и эксплоиты для образовательных целей. Не используйте его в продакшене!
"@
                    $readmeContent | Out-File -FilePath $securityReadmeFile -Encoding UTF8
                    Write-Host "    Создан SECURITY_README.md" -ForegroundColor Cyan
                }
            }
        }
        Write-Host ""
    } else {
        Write-Host "ПРЕДУПРЕЖДЕНИЕ: Модуль $module не найден, пропускаем" -ForegroundColor Yellow
    }
}

Write-Host "=== ЗАГЛУШКИ БЕЗОПАСНОСТИ СОЗДАНЫ ДЛЯ ВСЕХ МОДУЛЕЙ ===" -ForegroundColor Magenta
Write-Host ""
Write-Host "Следующие шаги:" -ForegroundColor Green
Write-Host "1. Детализировать каждый модуль по очереди"
Write-Host "2. Добавить конкретные уязвимости для каждого паттерна"
Write-Host "3. Создать практические эксплоиты"
Write-Host "4. Реализовать безопасные альтернативы"
Write-Host "5. Добавить тесты безопасности"


