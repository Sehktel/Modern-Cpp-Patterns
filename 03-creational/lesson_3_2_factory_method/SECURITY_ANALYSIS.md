# Анализ безопасности паттерна Factory Method

## Обзор уязвимостей

### Основные типы уязвимостей в Factory Method:
- **Buffer Overflow** - переполнение буфера при создании объектов
- **Memory Leak** - утечки памяти при исключениях в конструкторе
- **Type Confusion** - проблемы с небезопасным приведением типов
- **Integer Overflow** - переполнение при вычислении размеров объектов

## Инструменты анализа

### Статический анализ
- **Clang Static Analyzer**: `clang --analyze factory_method_vulnerabilities.cpp`
- **Cppcheck**: `cppcheck --enable=all factory_method_vulnerabilities.cpp`
- **PVS-Studio**: Анализ на предмет buffer overflow и memory leaks

### Динамический анализ
- **AddressSanitizer**: Обнаружение buffer overflow и use-after-free
- **MemorySanitizer**: Обнаружение неинициализированной памяти
- **Valgrind**: Анализ памяти и обнаружение leaks

## Практические задания

1. **Обнаружение buffer overflow**:
   ```bash
   g++ -fsanitize=address -g factory_method_vulnerabilities.cpp -o factory_method_asan
   ./factory_method_asan
   ```

2. **Анализ memory leaks**:
   ```bash
   valgrind --tool=memcheck --leak-check=full ./factory_method_vulnerabilities
   ```

3. **Создание эксплоита**:
   - TODO: Показать как использовать buffer overflow для ROP

## Безопасные альтернативы

### 1. Factory с RAII
```cpp
// TODO: Показать безопасную реализацию с автоматическим управлением памятью
```

### 2. Factory с проверкой типов
```cpp
// TODO: Показать реализацию с безопасным приведением типов
```

### 3. Factory с ограничениями размера
```cpp
// TODO: Показать реализацию с проверкой размеров
```

## Эксплоиты и атаки

### 1. Buffer Overflow Exploit
```cpp
// TODO: Создать эксплоит для buffer overflow
// Цель: Получить контроль над выполнением программы
```

### 2. Memory Corruption
```cpp
// TODO: Показать как использовать type confusion для атаки
```

### 3. Heap Spraying
```cpp
// TODO: Показать heap spraying через Factory
```

## Защитные меры

1. **Использование std::unique_ptr и std::shared_ptr**
2. **Проверка размеров перед выделением памяти**
3. **Безопасное приведение типов**
4. **RAII для управления ресурсами**
5. **Статический анализ в CI/CD**

## Связь с другими паттернами

- **Abstract Factory**: Может создавать небезопасные объекты
- **Builder**: Может иметь проблемы с валидацией
- **Prototype**: Может копировать уязвимые объекты