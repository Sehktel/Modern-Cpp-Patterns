# 🛠️ Tools Directory

Этот каталог содержит инструменты для автоматизации и анализа курса "Design Patterns in Modern C++".

## 📁 Структура

```
tools/
├── sync_course_structure.py      # Python скрипт синхронизации (кроссплатформенный)
├── sync_course_structure.ps1     # PowerShell скрипт синхронизации (Windows)
├── AUTOMATION_GUIDE.md            # Полное руководство по автоматизации
├── README.md                      # Этот файл
├── static_analysis/               # Инструменты статического анализа
├── dynamic_analysis/              # Инструменты динамического анализа
├── fuzzing/                       # Fuzzing инструменты
└── exploitation/                  # Инструменты для exploitation

```

## 🤖 Автоматизация структуры курса

### Быстрый старт

**Python** (рекомендуется - кроссплатформенный):
```bash
# Обновить COURSE_STRUCTURE_COMPLETE.md
python tools/sync_course_structure.py

# Только проверить статус
python tools/sync_course_structure.py --check-only

# Детальный вывод
python tools/sync_course_structure.py --verbose

# JSON export
python tools/sync_course_structure.py --json > report.json
```

**PowerShell** (Windows native):
```powershell
# Обновить COURSE_STRUCTURE_COMPLETE.md
.\tools\sync_course_structure.ps1

# Только проверить статус
.\tools\sync_course_structure.ps1 -CheckOnly

# Детальный вывод
.\tools\sync_course_structure.ps1 -VerboseOutput
```

### Что делают скрипты?

1. **Сканируют** всю структуру проекта (35 паттернов в 9 модулях)
2. **Проверяют** наличие обязательных компонентов:
   - README.md
   - SECURITY_ANALYSIS.md
   - Security posters
   - Vulnerabilities файлы
   - Exploits
   - Secure alternatives
   - C++23 comparison файлы
3. **Вычисляют** процент завершённости для каждого паттерна
4. **Генерируют** актуальный markdown отчёт с полной статистикой

### Зачем это нужно?

**Проблема**: Manual tracking структуры проекта приводит к устаревшей документации.

**Решение**: Автоматическая генерация документации из структуры проекта.

**Результат**: Документация **всегда актуальна** и синхронизирована с кодом.

### Полное руководство

Для детального описания всех возможностей, примеров интеграции с Git hooks и CI/CD, а также troubleshooting:

👉 **[AUTOMATION_GUIDE.md](AUTOMATION_GUIDE.md)**

## 🔍 Анализ и тестирование

### 🎯 Валидация курса (НОВОЕ!)

**Проблема**: Как проверить, что AI-генерированный код не "врет"?

**Решение**: Комплексная автоматическая валидация

#### Быстрый старт

```powershell
# Валидация одного паттерна
.\tools\validate_pattern.ps1 -Pattern "producer_consumer"

# Полная валидация с санитайзерами
.\tools\validate_pattern.ps1 -Pattern "singleton" -Full

# Валидация всего курса (35 паттернов)
.\tools\validate_all_patterns.ps1

# Полная валидация (займет ~1-2 часа)
.\tools\validate_all_patterns.ps1 -Full
```

#### Что проверяется?

- ✅ **Компиляция** всех файлов
- ✅ **AddressSanitizer детектирует** уязвимости в vulnerable версиях
- ✅ **ThreadSanitizer детектирует** race conditions
- ✅ **Безопасные версии чисты** под всеми санитайзерами
- ✅ **CVE/CWE references корректны**
- ✅ **Статический анализ** (Clang-Tidy, Cppcheck)

#### Документация

| Документ | Описание |
|----------|----------|
| [VERIFICATION_STRATEGY.md](VERIFICATION_STRATEGY.md) | Стратегия верификации AI-кода |
| [PRACTICAL_VALIDATION_GUIDE.md](PRACTICAL_VALIDATION_GUIDE.md) | Практические примеры |

📂 **Скрипты**: `validate_pattern.ps1`, `validate_all_patterns.ps1`

---

### Static Analysis

Инструменты для статического анализа кода:
- Clang-Tidy (скрипт: `static_analysis/run_clang_tidy.ps1`)
- Cppcheck (скрипт: `static_analysis/run_cppcheck.ps1`)
- PVS-Studio

📂 `static_analysis/`

### Dynamic Analysis

Инструменты для динамического анализа:
- AddressSanitizer (ASan)
- ThreadSanitizer (TSan)
- UndefinedBehaviorSanitizer (UBSan)
- Valgrind

📂 `dynamic_analysis/`

### Fuzzing

Fuzzing инструменты для обнаружения уязвимостей:
- libFuzzer
- AFL++

📂 `fuzzing/`

### Exploitation

Инструменты для создания и тестирования эксплоитов:
- GDB scripts
- Exploit templates

📂 `exploitation/`

## 📚 Документация

| Документ | Описание |
|----------|----------|
| [AUTOMATION_GUIDE.md](AUTOMATION_GUIDE.md) | Полное руководство по автоматизации |
| [ANALYSIS_TOOLS_GUIDE.md](ANALYSIS_TOOLS_GUIDE.md) | Руководство по инструментам анализа |
| [VERIFICATION_STRATEGY.md](VERIFICATION_STRATEGY.md) | 🆕 Стратегия верификации AI-кода |
| [PRACTICAL_VALIDATION_GUIDE.md](PRACTICAL_VALIDATION_GUIDE.md) | 🆕 Практическое руководство по валидации |

## 🎯 Best Practices

### 1. Регулярное обновление структуры

После каждого значительного изменения запускайте:
```bash
python tools/sync_course_structure.py
```

### 2. Проверка перед коммитом

Добавьте в `.git/hooks/pre-commit`:
```bash
#!/bin/bash
python tools/sync_course_structure.py
git add COURSE_STRUCTURE_COMPLETE.md
```

### 3. CI/CD интеграция

Интегрируйте проверку в ваш CI/CD pipeline:
```yaml
- name: Check structure
  run: python tools/sync_course_structure.py --check-only
```

## 🤝 Contributing

При добавлении новых инструментов:

1. Создайте соответствующую директорию
2. Добавьте README.md с описанием
3. Обновите этот файл
4. Добавьте примеры использования

## 📞 Помощь

Если скрипты не работают или нашли баг:

1. Проверьте [AUTOMATION_GUIDE.md](AUTOMATION_GUIDE.md) → раздел Troubleshooting
2. Убедитесь, что используете Python 3.7+ или PowerShell 5.1+
3. Проверьте, что запускаете из корня проекта

## 📈 Статистика

По состоянию на последнее сканирование:

```
Паттернов: 35
Средняя завершённость: 97.1%
Модулей: 9

Покрытие:
  ✅ CMakeLists:        35/35 (100%)
  ✅ Security Analysis: 35/35 (100%)
  ✅ Security Posters:  35/35 (100%)
  ✅ Vulnerabilities:   35/35 (100%)
  ✅ Exploits:          35/35 (100%)
  ✅ Secure Alternatives: 35/35 (100%)
  ✅ C++23 Comparison:  35/35 (100%)
  🟡 README:            25/35 (71%)
```

---

**Дата**: 2025-10-11  
**Maintainer**: Development Team

