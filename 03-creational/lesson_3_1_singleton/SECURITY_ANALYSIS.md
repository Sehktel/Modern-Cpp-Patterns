# Анализ безопасности паттерна Singleton

## Уязвимости в классическом Singleton

### 1. Race Condition при инициализации
```cpp
// TODO: Показать небезопасную реализацию Singleton
// Проблема: Два потока могут одновременно создать объект
```

### 2. Memory Leak при исключениях
```cpp
// TODO: Показать утечку памяти при исключении в конструкторе
// Проблема: Если конструктор выбрасывает исключение, память не освобождается
```

### 3. Use-After-Free при уничтожении
```cpp
// TODO: Показать использование после уничтожения
// Проблема: Объект может быть использован после вызова деструктора
```

## Инструменты анализа

### Статический анализ
- **Clang Static Analyzer**: `clang --analyze singleton_pattern.cpp`
- **PVS-Studio**: Анализ на предмет race conditions
- **Cppcheck**: `cppcheck --enable=all singleton_pattern.cpp`

### Динамический анализ
- **ThreadSanitizer**: Обнаружение race conditions
- **AddressSanitizer**: Обнаружение use-after-free
- **Valgrind**: Анализ памяти и многопоточности

## Практические задания

1. **Обнаружение race condition**:
   ```bash
   # Сборка с ThreadSanitizer
   g++ -fsanitize=thread -g singleton_pattern.cpp -o singleton_tsan
   ./singleton_tsan
   ```

2. **Анализ с Valgrind**:
   ```bash
   valgrind --tool=helgrind ./singleton_pattern
   ```

3. **Создание эксплоита**:
   - TODO: Показать как использовать race condition для получения привилегий

## Безопасные альтернативы

### 1. Thread-safe Singleton (C++11)
```cpp
// TODO: Показать безопасную реализацию с std::call_once
```

### 2. Singleton с RAII
```cpp
// TODO: Показать реализацию с автоматическим управлением памятью
```

### 3. Dependency Injection вместо Singleton
```cpp
// TODO: Показать как избежать Singleton через DI
```

## Эксплоиты и атаки

### 1. Race Condition Exploit
```cpp
// TODO: Создать эксплоит для race condition
// Цель: Получить контроль над критическими ресурсами
```

### 2. Memory Corruption
```cpp
// TODO: Показать как использовать утечки памяти для атаки
```

### 3. Side Channel Attack
```cpp
// TODO: Показать timing attack на Singleton
```

## Защитные меры

1. **Использование std::call_once**
2. **Правильная обработка исключений**
3. **Избегание глобального состояния**
4. **Использование Dependency Injection**
5. **Статический анализ в CI/CD**

## Связь с другими паттернами

- **Factory**: Может создавать небезопасные Singleton
- **Observer**: Singleton может быть небезопасным субъектом
- **Proxy**: Может скрывать проблемы безопасности Singleton

