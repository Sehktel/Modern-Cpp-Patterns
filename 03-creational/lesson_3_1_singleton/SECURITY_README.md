# Singleton Pattern - Анализ безопасности

## Обзор

Этот модуль демонстрирует паттерн Singleton с фокусом на анализе безопасности. Мы показываем как классические реализации могут содержать уязвимости, и как их можно использовать для проведения пентестов.

## Структура модуля

### Основные файлы
- `singleton_pattern.cpp` - Классическая реализация Singleton
- `modern_singleton.cpp` - Современная реализация Singleton
- `singleton_vulnerabilities.cpp` - Уязвимые реализации для анализа
- `secure_singleton_alternatives.cpp` - Безопасные альтернативы
- `exploits/singleton_exploits.cpp` - Практические эксплоиты

### Документация
- `README.md` - Основное описание паттерна
- `SECURITY_ANALYSIS.md` - Детальный анализ уязвимостей

## Уязвимости в Singleton

### 1. Race Condition при инициализации
**Проблема**: Два потока могут одновременно создать объект Singleton
**Эксплоит**: Получение контроля над критическими ресурсами
**Защита**: Использование `std::call_once` или мьютексов

### 2. Memory Leak при исключениях
**Проблема**: Если конструктор выбрасывает исключение, память не освобождается
**Эксплоит**: Утечка памяти может привести к DoS
**Защита**: RAII и умные указатели

### 3. Use-After-Free при уничтожении
**Проблема**: Объект может быть использован после вызова деструктора
**Эксплоит**: Crash или выполнение произвольного кода
**Защита**: Правильное управление жизненным циклом

### 4. Integer Overflow в счетчике ссылок
**Проблема**: При большом количестве ссылок может произойти переполнение
**Эксплоит**: Преждевременное уничтожение объекта
**Защита**: Проверка границ и использование атомарных операций

## Инструменты анализа

### Статический анализ
```bash
# Clang Static Analyzer
clang --analyze singleton_vulnerabilities.cpp

# Cppcheck
cppcheck --enable=all singleton_vulnerabilities.cpp

# PVS-Studio
pvs-studio-analyzer trace -- make
```

### Динамический анализ
```bash
# ThreadSanitizer (race conditions)
g++ -fsanitize=thread -g singleton_vulnerabilities.cpp -o singleton_tsan
./singleton_tsan

# AddressSanitizer (use-after-free)
g++ -fsanitize=address -g singleton_vulnerabilities.cpp -o singleton_asan
./singleton_asan

# Valgrind
valgrind --tool=helgrind ./singleton_vulnerabilities
```

## Практические задания

### Базовый уровень
1. **Анализ кода**: Запустите статический анализ на уязвимых реализациях
2. **Динамический анализ**: Используйте санитайзеры для обнаружения проблем
3. **Понимание отчетов**: Научитесь интерпретировать результаты анализа

### Средний уровень
1. **Создание эксплоитов**: Реализуйте race condition exploit
2. **Обход защиты**: Попробуйте обойти базовые защитные механизмы
3. **Реализация защиты**: Создайте безопасную альтернативу

### Продвинутый уровень
1. **Сложные эксплоиты**: Создайте use-after-free exploit
2. **Side channel attacks**: Реализуйте timing attack
3. **Инструменты**: Разработайте собственный анализатор

## Сборка и запуск

### Основные цели
```bash
make singleton_pattern          # Классический Singleton
make modern_singleton           # Современный Singleton
```

### Анализ безопасности
```bash
make singleton_vulnerabilities  # Уязвимые реализации
make singleton_vulnerabilities_tsan  # С ThreadSanitizer
make singleton_vulnerabilities_asan   # С AddressSanitizer
make singleton_vulnerabilities_ubsan # С UndefinedBehaviorSanitizer
```

### Безопасные альтернативы
```bash
make secure_singleton_alternatives  # Безопасные реализации
```

### Эксплоиты
```bash
make singleton_exploits         # Практические эксплоиты
```

## Рекомендации по безопасности

### 1. Thread-Safe инициализация
```cpp
// Используйте std::call_once
static std::once_flag init_flag;
static std::unique_ptr<Singleton> instance;

static Singleton& getInstance() {
    std::call_once(init_flag, []() {
        instance = std::make_unique<Singleton>();
    });
    return *instance;
}
```

### 2. RAII для управления памятью
```cpp
// Используйте умные указатели
static std::shared_ptr<Singleton> instance;
```

### 3. Защита от timing attacks
```cpp
// Постоянное время для криптографических операций
bool checkPassword(const std::string& password) const {
    if (password.length() != storedPassword.length()) {
        return false;
    }
    
    bool result = true;
    for (size_t i = 0; i < password.length(); ++i) {
        result &= (password[i] == storedPassword[i]);
    }
    return result;
}
```

### 4. Безопасная очистка данных
```cpp
~Singleton() {
    // Очистка чувствительных данных
    std::fill(password.begin(), password.end(), 0);
}
```

### 5. Избегание Singleton
```cpp
// Используйте Dependency Injection
class ServiceContainer {
    std::unique_ptr<AdminService> adminService;
public:
    AdminService& getAdminService() {
        if (!adminService) {
            adminService = std::make_unique<AdminService>();
        }
        return *adminService;
    }
};
```

## Связь с другими паттернами

- **Factory**: Может создавать небезопасные Singleton
- **Observer**: Singleton может быть небезопасным субъектом
- **Proxy**: Может скрывать проблемы безопасности Singleton
- **Dependency Injection**: Безопасная альтернатива Singleton

## Дополнительные ресурсы

- [OWASP Secure Coding Practices](https://owasp.org/www-project-secure-coding-practices-quick-reference-guide/)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [Secure Coding in C and C++](https://www.securecoding.cert.org/)

## Предупреждение

⚠️ **ВНИМАНИЕ**: Код в этом модуле содержит уязвимости и эксплоиты для образовательных целей. Не используйте его в продакшене!

## Лицензия

Этот модуль предназначен только для образовательных целей. Используйте на свой страх и риск.
