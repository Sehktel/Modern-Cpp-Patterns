# Observer Pattern - Security Analysis

## 📋 Обзор

Этот модуль содержит полный анализ безопасности паттерна Observer, включая:
- Уязвимые реализации для обучения
- Практические эксплойты
- Безопасные альтернативы
- Инструменты анализа

## 🗂️ Структура файлов

```
lesson_5_1_observer/
├── observer_pattern.cpp              # Классическая реализация
├── modern_observer.cpp                # Современные подходы
├── observer_vulnerabilities.cpp       # 🔴 УЯЗВИМЫЕ реализации
├── secure_observer_alternatives.cpp   # ✅ Безопасные альтернативы
├── exploits/
│   └── observer_exploits.cpp         # ⚠️  Практические эксплойты
├── SECURITY_ANALYSIS.md               # Детальный анализ
└── CMakeLists.txt                     # Сборка с санитайзерами
```

## 🐛 Уязвимости

### 1. Use-After-Free (UAF)
**Файл**: `observer_vulnerabilities.cpp` - функция `demonstrateUseAfterFree()`

**Проблема**: Subject хранит сырые указатели на Observers
```cpp
Observer* obs = new ConcreteObserver();
subject.attach(obs);
delete obs;
subject.notify();  // ❌ UAF!
```

**Детектирование**:
```bash
cmake --build . --target observer_vulnerabilities_asan
./observer_vulnerabilities_asan
# Output: heap-use-after-free
```

### 2. Race Condition
**Файл**: `observer_vulnerabilities.cpp` - функция `demonstrateRaceCondition()`

**Проблема**: Неатомарный доступ к списку observers
```cpp
// Поток 1
subject.attach(observer);

// Поток 2 (одновременно)
subject.notify();  // ❌ Data race!
```

**Детектирование**:
```bash
cmake --build . --target observer_vulnerabilities_tsan
./observer_vulnerabilities_tsan
# Output: WARNING: ThreadSanitizer: data race
```

### 3. Memory Leak
**Файл**: `observer_vulnerabilities.cpp` - функция `demonstrateMemoryLeak()`

**Проблема**: Циклические ссылки shared_ptr

**Детектирование**:
```bash
cmake --build . --target observer_vulnerabilities
valgrind --leak-check=full ./observer_vulnerabilities
```

### 4. Iterator Invalidation
**Файл**: `observer_vulnerabilities.cpp` - функция `demonstrateIteratorInvalidation()`

**Проблема**: Observer отписывается во время notify

### 5. Dangling Weak Pointer
**Файл**: `observer_vulnerabilities.cpp` - функция `demonstrateDanglingWeakPtr()`

**Проблема**: Накопление expired weak_ptr

## ⚡ Эксплойты

### Эксплойт 1: UAF для контроля выполнения
```bash
cmake --build . --target observer_exploits_asan
./observer_exploits_asan
```

### Эксплойт 2: TOCTOU через Race Condition
Обход проверки permissions через состояние гонки

### Эксплойт 3: Iterator Invalidation
Манипуляция списком observers во время notify

### Эксплойт 4: Memory Exhaustion (DoS)
Исчерпание памяти через подписку

### Эксплойт 5: Timing Attack
Извлечение информации через анализ времени

## ✅ Безопасные реализации

**Файл**: `secure_observer_alternatives.cpp`

### 1. SafeSubject с weak_ptr
```cpp
std::vector<std::weak_ptr<Observer>> observers_;  // ✅
```

### 2. RAII Subscription
Автоматическая отписка через RAII

### 3. Thread-Safe Subject
Использование `std::shared_mutex`

### 4. Type-Safe Signal
Compile-time проверка типов

## 🔧 Сборка и запуск

### Базовая сборка
```bash
mkdir build && cd build
cmake ..
make
```

### Сборка с санитайзерами
```bash
# AddressSanitizer (UAF, buffer overflow)
make observer_vulnerabilities_asan
./observer_vulnerabilities_asan

# ThreadSanitizer (race conditions)
make observer_vulnerabilities_tsan
./observer_vulnerabilities_tsan

# UndefinedBehaviorSanitizer
make observer_vulnerabilities_ubsan
./observer_vulnerabilities_ubsan

# MemorySanitizer (Clang only)
make observer_vulnerabilities_msan
./observer_vulnerabilities_msan
```

### Статический анализ
```bash
# Clang Static Analyzer
make analyze_observer

# Cppcheck
make cppcheck_observer
```

### Динамический анализ с Valgrind
```bash
# Memory leaks
make valgrind_observer

# Thread errors
make helgrind_observer
```

### Запуск всех санитайзеров сразу
```bash
make run_all_sanitizers
```

## 📊 Результаты анализа

### AddressSanitizer (UAF)
```
==12345==ERROR: AddressSanitizer: heap-use-after-free
READ of size 8 at 0x60300000eff0 thread T0
    #0 0x... in VulnerableSubject::notify()
    #1 0x... in demonstrateUseAfterFree()
```

### ThreadSanitizer (Race)
```
WARNING: ThreadSanitizer: data race (pid=12345)
  Write of size 8 at 0x7b0400000000 by thread T1:
    #0 std::vector::push_back()
  Previous read of size 8 at 0x7b0400000000 by main thread:
    #0 std::vector::begin()
```

### Valgrind (Memory Leak)
```
LEAK SUMMARY:
   definitely lost: 128 bytes in 2 blocks
   indirectly lost: 256 bytes in 4 blocks
   still reachable: 1,024 bytes in 8 blocks (circular references)
```

## 🎯 Практические задания

### Задание 1: Найти UAF
1. Скомпилируйте с ASan: `make observer_vulnerabilities_asan`
2. Запустите: `./observer_vulnerabilities_asan`
3. Найдите строку с ошибкой в выводе
4. Исправьте используя `secure_observer_alternatives.cpp`

### Задание 2: Обнаружить Race Condition
1. Скомпилируйте с TSan: `make observer_vulnerabilities_tsan`
2. Запустите несколько раз: `for i in {1..10}; do ./observer_vulnerabilities_tsan; done`
3. Изучите вывод TSan
4. Исправьте используя `std::mutex`

### Задание 3: Анализ Memory Leak
1. Запустите Valgrind: `make valgrind_observer`
2. Найдите "still reachable" блоки (циклические ссылки)
3. Используйте `weak_ptr` для исправления

### Задание 4: Создать эксплойт
1. Изучите `exploits/observer_exploits.cpp`
2. Запустите: `./observer_exploits`
3. Попробуйте модифицировать эксплойт для другой уязвимости

## 📚 Рекомендации по безопасности

### ✅ DO (Делайте так)
- Используйте `std::weak_ptr` для хранения observers
- Применяйте RAII для автоматической отписки
- Защищайте shared state с помощью мьютексов
- Копируйте список observers перед итерацией
- Ограничивайте количество подписок
- Обрабатывайте исключения в callbacks
- Регулярно очищайте expired weak_ptr

### ❌ DON'T (Не делайте так)
- ❌ Не используйте сырые указатели для observers
- ❌ Не забывайте о thread safety в многопоточной среде
- ❌ Не создавайте циклические ссылки с `shared_ptr`
- ❌ Не модифицируйте список во время итерации
- ❌ Не игнорируйте исключения в callbacks
- ❌ Не храните expired weak_ptr бесконечно

## 🔗 Связанные паттерны

- **Mediator**: Альтернатива Observer для сложных взаимодействий
- **Command**: Command objects могут быть observers
- **Strategy**: Strategies могут использовать Observer для уведомлений
- **Singleton**: Subject часто является Singleton (добавляет уязвимости)

## 📖 Дополнительные ресурсы

### Документация
- [SECURITY_ANALYSIS.md](SECURITY_ANALYSIS.md) - Детальный анализ
- [CWE-416: Use After Free](https://cwe.mitre.org/data/definitions/416.html)
- [CWE-362: Race Condition](https://cwe.mitre.org/data/definitions/362.html)

### Инструменты
- [AddressSanitizer](https://github.com/google/sanitizers/wiki/AddressSanitizer)
- [ThreadSanitizer](https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual)
- [Valgrind](https://valgrind.org/docs/manual/quick-start.html)

### Книги
- "Secure Coding in C and C++" - Robert Seacord
- "The Art of Software Security Assessment" - Mark Dowd

## 🚀 Быстрый старт

```bash
# 1. Сборка
mkdir build && cd build
cmake ..

# 2. Найти Use-After-Free
make observer_vulnerabilities_asan
./observer_vulnerabilities_asan

# 3. Найти Race Condition  
make observer_vulnerabilities_tsan
./observer_vulnerabilities_tsan

# 4. Запустить эксплойты
make observer_exploits
./observer_exploits

# 5. Изучить безопасные альтернативы
make secure_observer_alternatives
./secure_observer_alternatives
```

## 📝 Чек-лист безопасности

При реализации Observer проверьте:

- [ ] Используются `weak_ptr` вместо сырых указателей
- [ ] Есть защита от race conditions (мьютексы)
- [ ] Реализована очистка expired weak_ptr
- [ ] Копируется список observers перед notify
- [ ] Есть лимит на количество подписок
- [ ] Обрабатываются исключения в callbacks
- [ ] Применяется RAII для автоматической отписки
- [ ] Протестировано с ASan/TSan
- [ ] Нет циклических ссылок shared_ptr
- [ ] Документированы потокобезопасные методы

---

**⚠️  ВАЖНО**: Уязвимый код предназначен ТОЛЬКО для обучения!

