# МИНИМАЛЬНЫЙ PowerShell-скрипт генерации заглушек безопасности для всех уроков

$ErrorActionPreference = 'Stop'

$modules = @(
    '01-basics',
    '02-principles',
    '03-creational',
    '04-structural',
    '05-behavioral',
    '06-modern-cpp',
    '07-concurrency',
    '08-high-load',
    '09-performance'
)

function Ensure-Stub {
    param(
        [string]$LessonPath,
        [string]$Pattern
    )

    # exploits dir
    $exploitsDir = Join-Path $LessonPath 'exploits'
    if (-not (Test-Path $exploitsDir)) { New-Item -ItemType Directory -Path $exploitsDir | Out-Null }

    # SECURITY_ANALYSIS.md
    $secFile = Join-Path $LessonPath 'SECURITY_ANALYSIS.md'
    if (-not (Test-Path $secFile) -and (Test-Path 'SECURITY_ANALYSIS_TEMPLATE.md')) {
        (Get-Content 'SECURITY_ANALYSIS_TEMPLATE.md' -Raw).
            Replace('{PATTERN_NAME}',$Pattern).
            Replace('{pattern}',$Pattern) | Out-File -FilePath $secFile -Encoding UTF8
    }

    # {pattern}_vulnerabilities.cpp
    $vulFile = Join-Path $LessonPath ("$Pattern`_vulnerabilities.cpp")
    if (-not (Test-Path $vulFile) -and (Test-Path 'VULNERABILITIES_TEMPLATE.cpp')) {
        (Get-Content 'VULNERABILITIES_TEMPLATE.cpp' -Raw).
            Replace('{PATTERN_NAME}',$Pattern).
            Replace('{pattern}',$Pattern) | Out-File -FilePath $vulFile -Encoding UTF8
    }

    # secure_{pattern}_alternatives.cpp
    $secureFile = Join-Path $LessonPath ("secure_${Pattern}_alternatives.cpp")
    if (-not (Test-Path $secureFile) -and (Test-Path 'SECURE_ALTERNATIVES_TEMPLATE.cpp')) {
        (Get-Content 'SECURE_ALTERNATIVES_TEMPLATE.cpp' -Raw).
            Replace('{PATTERN_NAME}',$Pattern).
            Replace('{pattern}',$Pattern) | Out-File -FilePath $secureFile -Encoding UTF8
    }

    # exploits/{pattern}_exploits.cpp
    $expFile = Join-Path $exploitsDir ("${Pattern}_exploits.cpp")
    if (-not (Test-Path $expFile) -and (Test-Path 'EXPLOITS_TEMPLATE.cpp')) {
        (Get-Content 'EXPLOITS_TEMPLATE.cpp' -Raw).
            Replace('{PATTERN_NAME}',$Pattern).
            Replace('{pattern}',$Pattern) | Out-File -FilePath $expFile -Encoding UTF8
    }

    # CMakeLists.txt
    $cmakeFile = Join-Path $LessonPath 'CMakeLists.txt'
    if (-not (Test-Path $cmakeFile) -and (Test-Path 'CMAKE_TEMPLATE.txt')) {
        (Get-Content 'CMAKE_TEMPLATE.txt' -Raw).
            Replace('{PATTERN_NAME}',$Pattern).
            Replace('{pattern}',$Pattern) | Out-File -FilePath $cmakeFile -Encoding UTF8
    }
}

foreach ($m in $modules) {
    if (-not (Test-Path $m)) { continue }
    Get-ChildItem -Path $m -Directory -Name 'lesson_*' | ForEach-Object {
        $lessonPath = Join-Path $m $_
        $pattern = ($_ -replace 'lesson_\d+_\d+_', '')
        Ensure-Stub -LessonPath $lessonPath -Pattern $pattern
    }
}

Write-Host 'Генерация заглушек завершена.'

