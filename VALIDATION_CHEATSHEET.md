# 📋 Шпаргалка по валидации проекта

**Быстрый справочник команд**

---

## 🚀 Быстрый старт

### Проверить один паттерн:
```powershell
.\tools\simple_validate_pattern.ps1 -PatternPath "03-creational\lesson_3_1_singleton"
```

### Проверить все 35 паттернов:
```powershell
.\tools\batch_validate_all.ps1
```

---

## 🔬 С санитайзерами

### AddressSanitizer (memory safety):
```powershell
# Один паттерн
.\tools\simple_validate_pattern.ps1 -PatternPath "путь" -WithSanitizers

# Все паттерны (~2 минуты)
.\tools\batch_validate_all.ps1 -WithSanitizers
```

### ✨ Полный анализ (всё что есть):
```powershell
# Один паттерн: ASan + UBSan + Cppcheck
.\tools\simple_validate_pattern.ps1 -PatternPath "путь" -FullAnalysis

# Все паттерны
.\tools\batch_validate_all.ps1 -FullAnalysis
```

---

## 📊 Что проверяется

| Флаг | Что проверяет | Время (1 паттерн) | Работает на Windows |
|------|---------------|-------------------|---------------------|
| (без флагов) | Компиляция + документация | ~2 сек | ✅ |
| `-WithSanitizers` | + ASan (memory) | ~5 сек | ✅ |
| `-WithUBSan` | + UBSan (undefined behavior) | ~5 сек | ❌ linker |
| `-WithCppcheck` | + Cppcheck анализ | ~10 сек | ⚠️ если установлен |
| `-FullAnalysis` | Всё вместе | ~15 сек | ⚠️ частично |

---

## 📁 Отчёты

### Автоматические отчёты:
```powershell
cat validation_batch_report.md    # Массовая валидация
cat clang_tidy_report_*.txt        # Clang-Tidy анализ
```

### Ручные сводки:
```
FINAL_VALIDATION_SUMMARY.md        # Итоговая сводка
COMPREHENSIVE_ANALYSIS_SUMMARY.md  # Анализ всех инструментов
CLANG_TIDY_ANALYSIS.md             # Разбор Clang-Tidy результатов
```

---

## ✅ Критерии успеха

| Критерий | Целевое | Текущее |
|----------|---------|---------|
| Компиляция | 100% | ✅ 100% (35/35) |
| ASan чисто (secure) | 100% | ✅ 100% (35/35) |
| CWE/CVE в docs | 95%+ | ✅ 100% (35/35) |
| Security warnings (secure) | 0 | ✅ 0 |

---

## 🎯 Быстрая диагностика

### Если что-то не компилируется:
```powershell
# Проверка одного проблемного паттерна
.\tools\simple_validate_pattern.ps1 -PatternPath "проблемный_путь"

# Быстрая проверка всех
.\tools\quick_compile_check.ps1
```

### Если нужна детальная статистика:
```powershell
# С ASan
.\tools\batch_validate_all.ps1 -WithSanitizers

# Результат в:
cat validation_batch_report.md
```

---

## 📚 Полезные команды

### Clang версия:
```powershell
& "C:\Program Files\LLVM\bin\clang++.exe" --version
```

### Проверка доступных санитайзеров:
```powershell
& "C:\Program Files\LLVM\bin\clang++.exe" -fsanitize=help
```

### Ручная компиляция с ASan:
```powershell
& "C:\Program Files\LLVM\bin\clang++.exe" -std=c++17 -fsanitize=address -g file.cpp -o file.exe
```

---

## 🎓 Результат

**Проект полностью валидирован:**
- ✅ 100% компиляция
- ✅ 100% память чиста (ASan)
- ✅ 100% документация (CWE/CVE)
- ✅ 0 security проблем в secure файлах

**Готов к использованию в курсе!** 🚀

