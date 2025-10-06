# Анализ безопасности паттерна Builder

## Обзор уязвимостей

### Основные типы уязвимостей в Builder:
- **Buffer Overflow** - переполнение буфера при построении объектов
- **Memory Leak** - утечки памяти при исключениях в процессе построения
- **Use-After-Free** - использование объектов после их сброса
- **Integer Overflow** - переполнение при вычислении размеров компонентов

## Инструменты анализа

### Статический анализ
- **Clang Static Analyzer**: `clang --analyze builder_vulnerabilities.cpp`
- **Cppcheck**: `cppcheck --enable=all builder_vulnerabilities.cpp`
- **PVS-Studio**: Анализ на предмет buffer overflow и memory leaks

### Динамический анализ
- **AddressSanitizer**: Обнаружение buffer overflow и use-after-free
- **MemorySanitizer**: Обнаружение неинициализированной памяти
- **Valgrind**: Анализ памяти и обнаружение leaks

## Практические задания

1. **Обнаружение buffer overflow**:
   ```bash
   g++ -fsanitize=address -g builder_vulnerabilities.cpp -o builder_asan
   ./builder_asan
   ```

2. **Анализ memory leaks**:
   ```bash
   valgrind --tool=memcheck --leak-check=full ./builder_vulnerabilities
   ```

3. **Создание эксплоита**:
   - TODO: Показать как использовать buffer overflow для ROP

## Безопасные альтернативы

### 1. Builder с RAII
```cpp
// TODO: Показать безопасную реализацию с автоматическим управлением памятью
```

### 2. Builder с проверкой размеров
```cpp
// TODO: Показать реализацию с проверкой границ
```

### 3. Builder с валидацией
```cpp
// TODO: Показать реализацию с проверкой входных данных
```

## Эксплоиты и атаки

### 1. Buffer Overflow Exploit
```cpp
// TODO: Создать эксплоит для buffer overflow
// Цель: Получить контроль над выполнением программы
```

### 2. Memory Corruption
```cpp
// TODO: Показать как использовать use-after-free для атаки
```

### 3. Resource Exhaustion
```cpp
// TODO: Показать DoS атаку через исчерпание памяти
```

## Защитные меры

1. **Использование std::vector и std::string**
2. **Проверка размеров перед операциями с памятью**
3. **RAII для управления ресурсами**
4. **Валидация входных данных**
5. **Статический анализ в CI/CD**

## Связь с другими паттернами

- **Factory**: Builder может создавать небезопасные объекты
- **Prototype**: Может копировать уязвимые объекты
- **Composite**: Может иметь проблемы с рекурсивным построением