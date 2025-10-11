# PowerShell скрипт автоматической синхронизации структуры курса
#
# Сканирует проект, проверяет наличие обязательных файлов для каждого паттерна
# и генерирует актуальный COURSE_STRUCTURE_COMPLETE.md
#
# Использование:
#   .\sync_course_structure.ps1 [-OutputFile FILE] [-CheckOnly] [-Verbose]
#
# Параметры:
#   -OutputFile     Путь к выходному файлу (по умолчанию: COURSE_STRUCTURE_COMPLETE.md)
#   -CheckOnly      Только проверить статус, не обновлять файл
#   -Verbose        Детальный вывод

[CmdletBinding()]
param(
    [string]$OutputFile = "COURSE_STRUCTURE_COMPLETE.md",
    [switch]$CheckOnly = $false,
    [switch]$VerboseOutput = $false
)

# Определяем корень проекта
$ProjectRoot = Split-Path -Parent $PSScriptRoot

# Определение модулей курса
$Modules = @{
    "01-basics"       = @{ Number = 1; Name = "Основы C++" }
    "02-principles"   = @{ Number = 2; Name = "Принципы проектирования" }
    "03-creational"   = @{ Number = 3; Name = "Порождающие паттерны" }
    "04-structural"   = @{ Number = 4; Name = "Структурные паттерны" }
    "05-behavioral"   = @{ Number = 5; Name = "Поведенческие паттерны" }
    "06-modern-cpp"   = @{ Number = 6; Name = "Современный C++" }
    "07-concurrency"  = @{ Number = 7; Name = "Паттерны многопоточности" }
    "08-high-load"    = @{ Number = 8; Name = "Паттерны высоконагруженных систем" }
    "09-performance"  = @{ Number = 9; Name = "Паттерны производительности" }
}

# Класс для хранения статуса паттерна
class PatternStatus {
    [int]$ModuleNumber
    [string]$ModuleName
    [int]$LessonNumber
    [string]$LessonName
    [string]$PatternName
    [string]$Directory
    
    # Основные файлы
    [bool]$HasReadme = $false
    [bool]$HasCMakeLists = $false
    
    # Security файлы
    [bool]$HasSecurityAnalysis = $false
    [bool]$HasSecurityPoster = $false
    [bool]$HasVulnerabilities = $false
    [bool]$HasExploits = $false
    [bool]$HasSecureAlternatives = $false
    
    # C++23 файлы
    [bool]$HasCpp23Comparison = $false
    
    # Дополнительные файлы
    [string[]]$MainCppFiles = @()
    [string[]]$OtherFiles = @()
    
    # Вычисляет процент завершённости
    [double] CompletionPercentage() {
        $weights = @{
            HasReadme = 10
            HasCMakeLists = 10
            HasSecurityAnalysis = 15
            HasSecurityPoster = 15
            HasVulnerabilities = 15
            HasExploits = 10
            HasSecureAlternatives = 15
            HasCpp23Comparison = 10
        }
        
        $totalWeight = ($weights.Values | Measure-Object -Sum).Sum
        $achievedWeight = 0
        
        foreach ($key in $weights.Keys) {
            if ($this.$key) {
                $achievedWeight += $weights[$key]
            }
        }
        
        return ($achievedWeight / $totalWeight) * 100
    }
    
    # Возвращает emoji статуса
    [string] GetStatusEmoji() {
        $completion = $this.CompletionPercentage()
        if ($completion -ge 90) { return "✅" }
        if ($completion -ge 70) { return "🟢" }
        if ($completion -ge 50) { return "🟡" }
        if ($completion -ge 30) { return "🟠" }
        return "🔴"
    }
    
    # Возвращает текстовый статус
    [string] GetStatusText() {
        $completion = $this.CompletionPercentage()
        if ($completion -ge 90) { return "ПОЛНОСТЬЮ РЕАЛИЗОВАН" }
        if ($completion -ge 70) { return "ПРАКТИЧЕСКИ ЗАВЕРШЁН" }
        if ($completion -ge 50) { return "ЧАСТИЧНО РЕАЛИЗОВАН" }
        if ($completion -ge 30) { return "БАЗОВАЯ СТРУКТУРА" }
        return "МИНИМАЛЬНАЯ СТРУКТУРА"
    }
}

# Функция сканирования урока
function Scan-Lesson {
    param(
        [int]$ModuleNum,
        [string]$ModuleName,
        [System.IO.DirectoryInfo]$LessonDir
    )
    
    # Парсим имя директории: lesson_X_Y_pattern_name
    $parts = $LessonDir.Name -split '_'
    if ($parts.Length -lt 3) {
        return $null
    }
    
    $lessonNum = [int]$parts[1]
    $patternName = ($parts[3..($parts.Length-1)] -join '_')
    if ([string]::IsNullOrEmpty($patternName)) {
        $patternName = $parts[2]
    }
    $lessonName = ($patternName -replace '_', ' ').ToUpper()
    
    $status = [PatternStatus]::new()
    $status.ModuleNumber = $ModuleNum
    $status.ModuleName = $ModuleName
    $status.LessonNumber = $lessonNum
    $status.LessonName = $lessonName
    $status.PatternName = $patternName
    $status.Directory = $LessonDir.FullName.Replace($ProjectRoot + '\', '')
    
    # Проверяем наличие основных файлов
    $status.HasReadme = Test-Path (Join-Path $LessonDir.FullName "README.md")
    $status.HasCMakeLists = Test-Path (Join-Path $LessonDir.FullName "CMakeLists.txt")
    
    # Проверяем security файлы
    $status.HasSecurityAnalysis = Test-Path (Join-Path $LessonDir.FullName "SECURITY_ANALYSIS.md")
    $status.HasVulnerabilities = (Get-ChildItem -Path $LessonDir.FullName -Filter "*_vulnerabilities.cpp" -ErrorAction SilentlyContinue).Count -gt 0
    $status.HasSecureAlternatives = (Get-ChildItem -Path $LessonDir.FullName -Filter "secure_*_alternatives.cpp" -ErrorAction SilentlyContinue).Count -gt 0
    
    # Проверяем exploits директорию
    $exploitsDir = Join-Path $LessonDir.FullName "exploits"
    if (Test-Path $exploitsDir) {
        $status.HasExploits = (Get-ChildItem -Path $exploitsDir -Filter "*.cpp" -ErrorAction SilentlyContinue).Count -gt 0
    }
    
    # Проверяем security poster
    $status.HasSecurityPoster = (Get-ChildItem -Path $LessonDir.FullName -Filter "*SECURITY_POSTER.md" -ErrorAction SilentlyContinue).Count -gt 0
    
    # Проверяем C++23 comparison файлы
    $cpp23Files = Get-ChildItem -Path $LessonDir.FullName -Filter "*_cpp23*.cpp" -ErrorAction SilentlyContinue
    $status.HasCpp23Comparison = $cpp23Files.Count -gt 0
    
    # Собираем список .cpp файлов
    $cppFiles = Get-ChildItem -Path $LessonDir.FullName -Filter "*.cpp" -ErrorAction SilentlyContinue
    foreach ($cppFile in $cppFiles) {
        if ($cppFile.Name -notmatch 'vulnerabilities' -and 
            $cppFile.Name -notmatch '^secure_' -and 
            $cppFile.Name -notmatch 'cpp23') {
            $status.MainCppFiles += $cppFile.Name
        }
    }
    
    # Собираем другие markdown файлы
    $mdFiles = Get-ChildItem -Path $LessonDir.FullName -Filter "*.md" -ErrorAction SilentlyContinue
    foreach ($mdFile in $mdFiles) {
        if ($mdFile.Name -ne 'README.md' -and 
            $mdFile.Name -ne 'SECURITY_ANALYSIS.md' -and 
            $mdFile.Name -notmatch 'SECURITY_POSTER') {
            $status.OtherFiles += $mdFile.Name
        }
    }
    
    return $status
}

# Функция генерации markdown отчёта
function Generate-Markdown {
    param(
        [PatternStatus[]]$Patterns
    )
    
    $lines = @()
    
    # Заголовок
    $lines += "# 📁 Полная структура курса - Автоматически сгенерировано"
    $lines += ""
    $lines += "**Дата генерации**: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
    $lines += "**Генератор**: ``tools/sync_course_structure.ps1``"
    $lines += ""
    $lines += "---"
    $lines += ""
    
    # Общая статистика
    $totalPatterns = $Patterns.Count
    $avgCompletion = ($Patterns | ForEach-Object { $_.CompletionPercentage() } | Measure-Object -Average).Average
    
    $lines += "## 📊 Общая статистика"
    $lines += ""
    $lines += "**Всего паттернов**: $totalPatterns"
    $lines += "**Средняя завершённость**: $($avgCompletion.ToString('F1'))%"
    $lines += ""
    
    # Таблица покрытия
    $lines += "### Покрытие компонентов"
    $lines += ""
    $lines += "| Компонент | Реализовано | Процент |"
    $lines += "|-----------|-------------|---------|"
    
    $components = @(
        @{ Name = "README"; Prop = "HasReadme" },
        @{ Name = "CMakeLists"; Prop = "HasCMakeLists" },
        @{ Name = "Security Analysis"; Prop = "HasSecurityAnalysis" },
        @{ Name = "Security Poster"; Prop = "HasSecurityPoster" },
        @{ Name = "Vulnerabilities"; Prop = "HasVulnerabilities" },
        @{ Name = "Exploits"; Prop = "HasExploits" },
        @{ Name = "Secure Alternatives"; Prop = "HasSecureAlternatives" },
        @{ Name = "C++23 Comparison"; Prop = "HasCpp23Comparison" }
    )
    
    foreach ($component in $components) {
        $count = ($Patterns | Where-Object { $_.$($component.Prop) }).Count
        $percentage = ($count / $totalPatterns) * 100
        $emoji = if ($percentage -ge 90) { "✅" } elseif ($percentage -ge 70) { "🟡" } else { "🔴" }
        $lines += "| $($component.Name) | $count/$totalPatterns | $emoji $($percentage.ToString('F0'))% |"
    }
    
    $lines += ""
    $lines += "---"
    $lines += ""
    
    # Детали по модулям
    $lines += "## 📚 Детали по модулям"
    $lines += ""
    
    $groupedPatterns = $Patterns | Group-Object -Property ModuleNumber | Sort-Object Name
    
    foreach ($group in $groupedPatterns) {
        $modulePatterns = $group.Group | Sort-Object LessonNumber
        $moduleName = $modulePatterns[0].ModuleName
        $moduleNum = $group.Name
        $avgComp = ($modulePatterns | ForEach-Object { $_.CompletionPercentage() } | Measure-Object -Average).Average
        
        $lines += "### Модуль $moduleNum`: $moduleName"
        $lines += ""
        $lines += "**Паттернов**: $($modulePatterns.Count) | **Завершённость**: $($avgComp.ToString('F1'))%"
        $lines += ""
        
        # Таблица паттернов модуля
        $lines += "| # | Паттерн | Статус | Завершено | Компоненты |"
        $lines += "|---|---------|--------|-----------|------------|"
        
        foreach ($pattern in $modulePatterns) {
            $components = @()
            if ($pattern.HasReadme) { $components += "📖" }
            if ($pattern.HasSecurityAnalysis) { $components += "🔒" }
            if ($pattern.HasSecurityPoster) { $components += "📋" }
            if ($pattern.HasVulnerabilities) { $components += "⚠️" }
            if ($pattern.HasExploits) { $components += "💣" }
            if ($pattern.HasSecureAlternatives) { $components += "✅" }
            if ($pattern.HasCpp23Comparison) { $components += "🚀" }
            
            $componentsStr = if ($components.Count -gt 0) { $components -join " " } else { "—" }
            
            $lines += "| $($pattern.LessonNumber) | $($pattern.LessonName) | " +
                     "$($pattern.GetStatusEmoji()) $($pattern.GetStatusText()) | " +
                     "$($pattern.CompletionPercentage().ToString('F0'))% | $componentsStr |"
        }
        
        $lines += ""
        $lines += "---"
        $lines += ""
    }
    
    # Легенда
    $lines += "## 📖 Легенда"
    $lines += ""
    $lines += "### Компоненты"
    $lines += "- 📖 README.md - документация урока"
    $lines += "- 🔒 SECURITY_ANALYSIS.md - анализ безопасности"
    $lines += "- 📋 *_SECURITY_POSTER.md - security плакат"
    $lines += "- ⚠️ *_vulnerabilities.cpp - уязвимые реализации"
    $lines += "- 💣 exploits/*_exploits.cpp - эксплоиты"
    $lines += "- ✅ secure_*_alternatives.cpp - безопасные альтернативы"
    $lines += "- 🚀 *_cpp23_comparison.cpp - C++23 сравнение"
    $lines += ""
    
    $lines += "### Статусы"
    $lines += "- ✅ ПОЛНОСТЬЮ РЕАЛИЗОВАН (≥90%)"
    $lines += "- 🟢 ПРАКТИЧЕСКИ ЗАВЕРШЁН (70-89%)"
    $lines += "- 🟡 ЧАСТИЧНО РЕАЛИЗОВАН (50-69%)"
    $lines += "- 🟠 БАЗОВАЯ СТРУКТУРА (30-49%)"
    $lines += "- 🔴 МИНИМАЛЬНАЯ СТРУКТУРА (<30%)"
    $lines += ""
    
    $lines += "---"
    $lines += ""
    $lines += "*Автоматически сгенерировано: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')*"
    
    return $lines -join "`n"
}

# Главная логика
Write-Host "🔍 Сканирование проекта: $ProjectRoot" -ForegroundColor Cyan
Write-Host ""

$allPatterns = @()

foreach ($moduleDir in $Modules.Keys | Sort-Object) {
    $moduleInfo = $Modules[$moduleDir]
    $modulePath = Join-Path $ProjectRoot $moduleDir
    
    if (-not (Test-Path $modulePath)) {
        Write-Host "⚠️  Модуль не найден: $moduleDir" -ForegroundColor Yellow
        continue
    }
    
    Write-Host "📂 Сканирование модуля $($moduleInfo.Number): $($moduleInfo.Name)" -ForegroundColor Green
    
    # Ищем все директории уроков (lesson_*)
    $lessonDirs = Get-ChildItem -Path $modulePath -Directory -Filter "lesson_*" | Sort-Object Name
    
    foreach ($lessonDir in $lessonDirs) {
        $pattern = Scan-Lesson -ModuleNum $moduleInfo.Number -ModuleName $moduleInfo.Name -LessonDir $lessonDir
        if ($pattern) {
            $allPatterns += $pattern
            if ($VerboseOutput) {
                Write-Host "  └─ $($pattern.LessonName): $($pattern.GetStatusEmoji()) $($pattern.CompletionPercentage().ToString('F0'))%" -ForegroundColor Gray
            }
        }
    }
}

Write-Host ""
Write-Host "✅ Сканирование завершено. Найдено паттернов: $($allPatterns.Count)" -ForegroundColor Green
Write-Host ""

# Статистика
Write-Host ("=" * 60) -ForegroundColor Cyan
Write-Host "📊 ОБЩАЯ СТАТИСТИКА" -ForegroundColor Cyan
Write-Host ("=" * 60) -ForegroundColor Cyan

$totalPatterns = $allPatterns.Count
$avgCompletion = ($allPatterns | ForEach-Object { $_.CompletionPercentage() } | Measure-Object -Average).Average

Write-Host "Всего паттернов: $totalPatterns"
Write-Host "Средняя завершённость: $($avgCompletion.ToString('F1'))%"
Write-Host ""

Write-Host "Покрытие компонентов:"
$components = @(
    @{ Name = "README"; Prop = "HasReadme" },
    @{ Name = "Security Analysis"; Prop = "HasSecurityAnalysis" },
    @{ Name = "Security Poster"; Prop = "HasSecurityPoster" },
    @{ Name = "Vulnerabilities"; Prop = "HasVulnerabilities" },
    @{ Name = "Exploits"; Prop = "HasExploits" },
    @{ Name = "Secure Alternatives"; Prop = "HasSecureAlternatives" },
    @{ Name = "C++23 Comparison"; Prop = "HasCpp23Comparison" }
)

foreach ($component in $components) {
    $count = ($allPatterns | Where-Object { $_.$($component.Prop) }).Count
    $percentage = ($count / $totalPatterns) * 100
    $emoji = if ($percentage -ge 90) { "✅" } elseif ($percentage -ge 70) { "🟡" } else { "🔴" }
    $paddedName = $component.Name.PadRight(25)
    Write-Host "  $emoji $paddedName $count/$totalPatterns ($($percentage.ToString('F1'))%)"
}

# Генерируем и сохраняем markdown
if (-not $CheckOnly) {
    Write-Host ""
    Write-Host "📝 Генерация отчёта..." -ForegroundColor Cyan
    
    $markdown = Generate-Markdown -Patterns $allPatterns
    
    $outputPath = if ([System.IO.Path]::IsPathRooted($OutputFile)) {
        $OutputFile
    } else {
        Join-Path $ProjectRoot $OutputFile
    }
    
    $markdown | Out-File -FilePath $outputPath -Encoding UTF8
    
    Write-Host "✅ Отчёт сохранён: $outputPath" -ForegroundColor Green
    Write-Host "   Размер: $($markdown.Length) символов"
} else {
    Write-Host ""
    Write-Host "⚠️  Режим -CheckOnly: файл не обновлён" -ForegroundColor Yellow
}

