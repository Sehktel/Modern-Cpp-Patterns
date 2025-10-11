# 🤖 Руководство по автоматизации курса

## 📋 Обзор

Данное руководство описывает систему автоматизации для поддержания актуальности метаданных курса "Design Patterns in Modern C++".

### Проблема

В процессе разработки курса **структура проекта постоянно меняется**:
- Добавляются новые файлы (security analysis, exploits, C++23 comparisons)
- Реализуются паттерны
- Обновляется документация

**Manual tracking** такого объёма изменений приводит к:
- ❌ Устаревшей документации (TODO маркеры не синхронизированы)
- ❌ Потере времени на ручное обновление
- ❌ Ошибкам и пропущенным файлам

### Решение

**Автоматическое сканирование** структуры проекта и **генерация отчётов**.

---

## 🛠️ Инструменты

### 1. Python Скрипт (кроссплатформенный)

**Файл**: `tools/sync_course_structure.py`

**Возможности**:
- ✅ Сканирование всей структуры проекта
- ✅ Проверка наличия обязательных компонентов для каждого паттерна
- ✅ Вычисление процента завершённости
- ✅ Генерация markdown отчёта
- ✅ Вывод статистики в консоль
- ✅ JSON export для интеграций

**Использование**:

```bash
# Базовое использование - генерирует COURSE_STRUCTURE_COMPLETE.md
python tools/sync_course_structure.py

# Только проверить статус без изменения файлов
python tools/sync_course_structure.py --check-only

# Указать свой файл вывода
python tools/sync_course_structure.py --output custom_report.md

# Детальный вывод
python tools/sync_course_structure.py --verbose

# JSON вывод (для CI/CD интеграций)
python tools/sync_course_structure.py --json > report.json

# Помощь
python tools/sync_course_structure.py --help
```

---

### 2. PowerShell Скрипт (Windows native)

**Файл**: `tools/sync_course_structure.ps1`

**Возможности**:
- ✅ Те же возможности, что и Python версия
- ✅ Native PowerShell объекты
- ✅ Цветной вывод в консоль
- ✅ Интеграция с Windows workflows

**Использование**:

```powershell
# Базовое использование - генерирует COURSE_STRUCTURE_COMPLETE.md
.\tools\sync_course_structure.ps1

# Только проверить статус без изменения файлов
.\tools\sync_course_structure.ps1 -CheckOnly

# Указать свой файл вывода
.\tools\sync_course_structure.ps1 -OutputFile "custom_report.md"

# Детальный вывод
.\tools\sync_course_structure.ps1 -VerboseOutput

# Помощь
Get-Help .\tools\sync_course_structure.ps1 -Detailed
```

---

## 📊 Что проверяют скрипты

Для каждого паттерна проверяется наличие следующих компонентов:

### Основные файлы
| Файл | Вес | Описание |
|------|-----|----------|
| `README.md` | 10% | Документация урока |
| `CMakeLists.txt` | 10% | Build configuration |

### Security компоненты
| Файл | Вес | Описание |
|------|-----|----------|
| `SECURITY_ANALYSIS.md` | 15% | Детальный анализ уязвимостей |
| `*_SECURITY_POSTER.md` | 15% | Визуальный security плакат |
| `*_vulnerabilities.cpp` | 15% | Уязвимые реализации |
| `exploits/*_exploits.cpp` | 10% | Рабочие эксплоиты |
| `secure_*_alternatives.cpp` | 15% | Безопасные альтернативы |

### Современный C++
| Файл | Вес | Описание |
|------|-----|----------|
| `*_cpp23_comparison.cpp` | 10% | C++23 vs C++17/20 сравнение |

**Общий вес**: 100%

### Статусы завершённости

Скрипты автоматически определяют статус паттерна:

| Статус | Завершённость | Emoji | Описание |
|--------|---------------|-------|----------|
| **ПОЛНОСТЬЮ РЕАЛИЗОВАН** | ≥90% | ✅ | Все компоненты на месте |
| **ПРАКТИЧЕСКИ ЗАВЕРШЁН** | 70-89% | 🟢 | Большинство компонентов готово |
| **ЧАСТИЧНО РЕАЛИЗОВАН** | 50-69% | 🟡 | Половина компонентов |
| **БАЗОВАЯ СТРУКТУРА** | 30-49% | 🟠 | Минимальная структура |
| **МИНИМАЛЬНАЯ СТРУКТУРА** | <30% | 🔴 | Только базовые файлы |

---

## 🔄 Рабочий процесс

### Ежедневное использование

```bash
# До начала работы - проверяем текущий статус
python tools/sync_course_structure.py --check-only

# Работаем над паттернами...
# Добавляем файлы, реализуем код...

# После завершения работы - обновляем документацию
python tools/sync_course_structure.py

# Коммитим изменения вместе с обновлённым отчётом
git add .
git commit -m "feat: implement Pattern XYZ + update structure"
```

### Интеграция с Git Hooks

Можно настроить **pre-commit hook** для автоматического обновления:

**`.git/hooks/pre-commit`**:
```bash
#!/bin/bash
# Автоматически обновляем COURSE_STRUCTURE_COMPLETE.md перед коммитом

python tools/sync_course_structure.py

# Добавляем изменённый файл в коммит
git add COURSE_STRUCTURE_COMPLETE.md

exit 0
```

**Не забудьте**:
```bash
chmod +x .git/hooks/pre-commit
```

### Интеграция с CI/CD

Пример для **GitHub Actions**:

```yaml
name: Update Course Structure

on:
  push:
    branches: [main, develop]
  pull_request:

jobs:
  update-structure:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Set up Python
        uses: actions/setup-python@v4
        with:
          python-version: '3.11'
      
      - name: Check course structure
        run: |
          python tools/sync_course_structure.py --check-only
          python tools/sync_course_structure.py --json > structure.json
      
      - name: Upload structure report
        uses: actions/upload-artifact@v3
        with:
          name: course-structure
          path: structure.json
      
      - name: Fail if completion < 90%
        run: |
          COMPLETION=$(python -c "import json; data=json.load(open('structure.json')); print(data['stats']['average_completion'])")
          if (( $(echo "$COMPLETION < 90" | bc -l) )); then
            echo "❌ Course completion is below 90%: $COMPLETION%"
            exit 1
          fi
```

---

## 📈 Вывод и интерпретация

### Консольный вывод

```
============================================================
📊 ОБЩАЯ СТАТИСТИКА
============================================================
Всего паттернов: 35
Средняя завершённость: 97.1%
Модулей: 9

Покрытие компонентов:
  ✅ CMakeLists                35/35 (100.0%)
  ✅ Security Analysis         35/35 (100.0%)
  ✅ Security Poster           35/35 (100.0%)
  ✅ Vulnerabilities           35/35 (100.0%)
  ✅ Exploits                  35/35 (100.0%)
  ✅ Secure Alternatives       35/35 (100.0%)
  ✅ C++23 Comparison          35/35 (100.0%)
  🟡 README                    25/35 ( 71.4%)

📝 Генерация отчёта...
✅ Отчёт сохранён: COURSE_STRUCTURE_COMPLETE.md
```

### Интерпретация

- **✅ (зелёная галочка)** - компонент реализован в ≥90% паттернов
- **🟡 (жёлтый круг)** - компонент реализован в 70-89% паттернов
- **🔴 (красный круг)** - компонент реализован в <70% паттернов

**Пример выше**:
- Почти все компоненты на 100% ✅
- README файлы есть только в 71.4% паттернов 🟡
- **Действие**: Нужно добавить README в оставшиеся 10 паттернов

### JSON вывод

```json
{
  "timestamp": "2025-10-11T15:30:00",
  "stats": {
    "total_patterns": 35,
    "average_completion": 97.1,
    "files": {
      "readme": 25,
      "cmakelists": 35,
      "security_analysis": 35,
      ...
    },
    "modules": 9
  },
  "patterns": [
    {
      "module_number": 3,
      "lesson_number": 1,
      "pattern_name": "singleton",
      "has_readme": true,
      "completion_percentage": 100.0,
      ...
    },
    ...
  ]
}
```

Это можно использовать для:
- Dashboard мониторинга
- Автоматических проверок в CI/CD
- Генерации других отчётов

---

## 🎯 Best Practices

### 1. Регулярное обновление

**Рекомендация**: Запускайте скрипт **после каждого значительного изменения** в структуре:
- Добавили новый паттерн
- Реализовали security components
- Создали C++23 comparison файлы

### 2. Используйте --check-only для проверки

Перед началом работы **проверяйте текущий статус**:
```bash
python tools/sync_course_structure.py --check-only
```

Это покажет, какие компоненты отсутствуют, без изменения файлов.

### 3. Коммитьте обновлённый отчёт

**Всегда коммитьте** обновлённый `COURSE_STRUCTURE_COMPLETE.md` вместе с изменениями в коде:
```bash
git add COURSE_STRUCTURE_COMPLETE.md
git commit -m "docs: update course structure"
```

### 4. Настройте Git Hooks (опционально)

Автоматизируйте обновление через pre-commit hook - это гарантирует, что отчёт **никогда не устареет**.

### 5. Мониторинг в CI/CD

Добавьте проверку в CI/CD pipeline:
- Fail билд, если completion < 90%
- Генерируйте artifact с отчётом
- Отслеживайте прогресс в Dashboard

---

## 🔧 Расширение скриптов

### Добавление новых проверок

Чтобы добавить проверку нового типа файла:

**Python** (`tools/sync_course_structure.py`):
```python
# В классе PatternStatus добавляем поле
has_new_component: bool = False

# В методе _scan_lesson добавляем проверку
status.has_new_component = (lesson_dir / 'new_file.md').exists()

# В методе completion_percentage добавляем вес
weights = {
    ...
    'has_new_component': 5,  # Вес компонента
}
```

**PowerShell** (`tools/sync_course_structure.ps1`):
```powershell
# В классе PatternStatus добавляем свойство
[bool]$HasNewComponent = $false

# В функции Scan-Lesson добавляем проверку
$status.HasNewComponent = Test-Path (Join-Path $LessonDir.FullName "new_file.md")

# В методе CompletionPercentage добавляем вес
$weights = @{
    ...
    HasNewComponent = 5
}
```

### Добавление новых выходных форматов

Можно легко добавить export в другие форматы:

**HTML Report**:
```python
def generate_html(self) -> str:
    """Генерирует HTML отчёт."""
    # Используем шаблонизатор (jinja2, etc.)
    ...
```

**CSV Export**:
```python
def export_csv(self) -> str:
    """Экспортирует данные в CSV."""
    import csv
    ...
```

---

## 🐛 Troubleshooting

### Скрипт не находит паттерны

**Проблема**: `Найдено паттернов: 0`

**Решение**:
1. Проверьте, что запускаете скрипт из **корня проекта**
2. Проверьте структуру директорий (должны быть `01-basics`, `02-principles`, и т.д.)
3. Проверьте, что директории уроков называются `lesson_X_Y_*`

### Python ModuleNotFoundError

**Проблема**: `ModuleNotFoundError: No module named 'dataclasses'`

**Решение**: Используйте **Python 3.7+**
```bash
python --version  # Должно быть ≥3.7
```

### PowerShell Execution Policy

**Проблема**: `cannot be loaded because running scripts is disabled`

**Решение**:
```powershell
# Временно разрешить выполнение скриптов
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process
.\tools\sync_course_structure.ps1

# Или постоянно (требует администратора)
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### Кодировка в Windows

**Проблема**: Некорректное отображение emoji/кириллицы

**Решение**:
```powershell
# В PowerShell установите UTF-8
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
```

---

## 📚 Дополнительные ресурсы

- **Технический долг**: `TECH_DEBT_TODO.md` - статус текущих задач
- **Отчёт о завершении**: `TECH_DEBT_COMPLETION_REPORT.md` - история закрытия долга
- **Структура курса**: `COURSE_STRUCTURE_COMPLETE.md` - автогенерируемый отчёт

---

## 🎉 Заключение

Автоматизация синхронизации структуры курса решает проблему **устаревшей документации**:

**До автоматизации**:
- ❌ Manual tracking изменений
- ❌ Устаревшие TODO маркеры
- ❌ Траты времени на обновление
- ❌ Ошибки и пропущенные файлы

**После автоматизации**:
- ✅ Автоматическое сканирование
- ✅ Всегда актуальная статистика
- ✅ Экономия времени
- ✅ Точные данные о прогрессе
- ✅ Интеграция с CI/CD

**Философия**: 
> Single source of truth - это код. Документация должна генерироваться из него автоматически.

---

**Автор**: AI Assistant  
**Дата**: 2025-10-11  
**Версия**: 1.0

**Обратная связь**: Если нашли баги или хотите добавить новые возможности - создайте issue или PR!

