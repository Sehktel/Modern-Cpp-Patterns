#!/bin/bash
# Скрипт для создания заглушек безопасности для всех модулей курса

# Массив всех модулей курса
modules=(
    "01-basics"
    "02-principles" 
    "03-creational"
    "04-structural"
    "05-behavioral"
    "06-modern-cpp"
    "07-concurrency"
    "08-high-load"
    "09-performance"
)

# Функция для создания заглушек безопасности для модуля
create_security_stubs() {
    local module=$1
    echo "Создание заглушек безопасности для модуля: $module"
    
    # Получаем список всех уроков в модуле
    local lessons=$(find "$module" -maxdepth 1 -type d -name "lesson_*" | sort)
    
    for lesson in $lessons; do
        if [ -d "$lesson" ]; then
            local lesson_name=$(basename "$lesson")
            local pattern_name=$(echo "$lesson_name" | sed 's/lesson_[0-9]*_[0-9]*_//')
            
            echo "  Обработка урока: $lesson_name (паттерн: $pattern_name)"
            
            # Создаем директорию exploits если её нет
            mkdir -p "$lesson/exploits"
            
            # Создаем SECURITY_ANALYSIS.md если его нет
            if [ ! -f "$lesson/SECURITY_ANALYSIS.md" ]; then
                sed "s/{PATTERN_NAME}/$pattern_name/g" SECURITY_ANALYSIS_TEMPLATE.md > "$lesson/SECURITY_ANALYSIS.md"
                echo "    Создан SECURITY_ANALYSIS.md"
            fi
            
            # Создаем {pattern}_vulnerabilities.cpp если его нет
            local vulnerabilities_file="${lesson}/${pattern_name}_vulnerabilities.cpp"
            if [ ! -f "$vulnerabilities_file" ]; then
                sed "s/{PATTERN_NAME}/$pattern_name/g; s/{pattern}/$pattern_name/g" VULNERABILITIES_TEMPLATE.cpp > "$vulnerabilities_file"
                echo "    Создан $vulnerabilities_file"
            fi
            
            # Создаем secure_{pattern}_alternatives.cpp если его нет
            local secure_file="${lesson}/secure_${pattern_name}_alternatives.cpp"
            if [ ! -f "$secure_file" ]; then
                sed "s/{PATTERN_NAME}/$pattern_name/g; s/{pattern}/$pattern_name/g" SECURE_ALTERNATIVES_TEMPLATE.cpp > "$secure_file"
                echo "    Создан $secure_file"
            fi
            
            # Создаем exploits/{pattern}_exploits.cpp если его нет
            local exploits_file="${lesson}/exploits/${pattern_name}_exploits.cpp"
            if [ ! -f "$exploits_file" ]; then
                sed "s/{PATTERN_NAME}/$pattern_name/g; s/{pattern}/$pattern_name/g" EXPLOITS_TEMPLATE.cpp > "$exploits_file"
                echo "    Создан $exploits_file"
            fi
            
            # Создаем CMakeLists.txt если его нет
            if [ ! -f "$lesson/CMakeLists.txt" ]; then
                sed "s/{PATTERN_NAME}/$pattern_name/g; s/{pattern}/$pattern_name/g" CMAKE_TEMPLATE.txt > "$lesson/CMakeLists.txt"
                echo "    Создан CMakeLists.txt"
            fi
            
            # Создаем SECURITY_README.md если его нет
            if [ ! -f "$lesson/SECURITY_README.md" ]; then
                cat > "$lesson/SECURITY_README.md" << EOF
# {PATTERN_NAME} Pattern - Анализ безопасности

## Обзор

Этот модуль демонстрирует паттерн {PATTERN_NAME} с фокусом на анализе безопасности.

## Структура модуля

### Основные файлы
- \`{pattern}_pattern.cpp\` - Классическая реализация {PATTERN_NAME}
- \`modern_{pattern}.cpp\` - Современная реализация {PATTERN_NAME}
- \`{pattern}_vulnerabilities.cpp\` - Уязвимые реализации для анализа
- \`secure_{pattern}_alternatives.cpp\` - Безопасные альтернативы
- \`exploits/{pattern}_exploits.cpp\` - Практические эксплоиты

### Документация
- \`README.md\` - Основное описание паттерна
- \`SECURITY_ANALYSIS.md\` - Детальный анализ уязвимостей

## TODO: Добавить конкретные уязвимости для данного паттерна

## Сборка и запуск

### Основные цели
\`\`\`bash
make {pattern}_pattern          # Классический {PATTERN_NAME}
make modern_{pattern}           # Современный {PATTERN_NAME}
\`\`\`

### Анализ безопасности
\`\`\`bash
make {pattern}_vulnerabilities  # Уязвимые реализации
make {pattern}_vulnerabilities_tsan  # С ThreadSanitizer
make {pattern}_vulnerabilities_asan   # С AddressSanitizer
make {pattern}_vulnerabilities_ubsan # С UndefinedBehaviorSanitizer
\`\`\`

## Предупреждение

⚠️ **ВНИМАНИЕ**: Код в этом модуле содержит уязвимости и эксплоиты для образовательных целей. Не используйте его в продакшене!
EOF
                sed -i "s/{PATTERN_NAME}/$pattern_name/g; s/{pattern}/$pattern_name/g" "$lesson/SECURITY_README.md"
                echo "    Создан SECURITY_README.md"
            fi
        fi
    done
}

# Основная функция
main() {
    echo "=== СОЗДАНИЕ ЗАГЛУШЕК БЕЗОПАСНОСТИ ДЛЯ ВСЕХ МОДУЛЕЙ ==="
    echo ""
    
    # Проверяем наличие шаблонов
    if [ ! -f "SECURITY_ANALYSIS_TEMPLATE.md" ]; then
        echo "ОШИБКА: Файл SECURITY_ANALYSIS_TEMPLATE.md не найден!"
        exit 1
    fi
    
    if [ ! -f "VULNERABILITIES_TEMPLATE.cpp" ]; then
        echo "ОШИБКА: Файл VULNERABILITIES_TEMPLATE.cpp не найден!"
        exit 1
    fi
    
    # Создаем заглушки для каждого модуля
    for module in "${modules[@]}"; do
        if [ -d "$module" ]; then
            create_security_stubs "$module"
            echo ""
        else
            echo "ПРЕДУПРЕЖДЕНИЕ: Модуль $module не найден, пропускаем"
        fi
    done
    
    echo "=== ЗАГЛУШКИ БЕЗОПАСНОСТИ СОЗДАНЫ ДЛЯ ВСЕХ МОДУЛЕЙ ==="
    echo ""
    echo "Следующие шаги:"
    echo "1. Детализировать каждый модуль по очереди"
    echo "2. Добавить конкретные уязвимости для каждого паттерна"
    echo "3. Создать практические эксплоиты"
    echo "4. Реализовать безопасные альтернативы"
    echo "5. Добавить тесты безопасности"
}

# Запуск скрипта
main "$@"


