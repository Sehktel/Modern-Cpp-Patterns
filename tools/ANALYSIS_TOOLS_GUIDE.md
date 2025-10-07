# Инструменты анализа безопасности C++

## Обзор инструментов

Этот раздел содержит практические примеры использования инструментов анализа безопасности для обнаружения уязвимостей в паттернах проектирования.

## Статический анализ

### Clang Static Analyzer
```bash
# Базовый анализ
clang --analyze source.cpp

# Анализ с дополнительными проверками
clang --analyze -Xanalyzer -analyzer-checker=core source.cpp

# Анализ всего проекта
scan-build make
```

### Cppcheck
```bash
# Базовый анализ
cppcheck source.cpp

# Расширенный анализ
cppcheck --enable=all --inconclusive source.cpp

# Анализ с отчетами
cppcheck --enable=all --xml --xml-version=2 source.cpp 2> report.xml
```

### PVS-Studio
```bash
# Анализ проекта
pvs-studio-analyzer trace -- make
pvs-studio-analyzer analyze
plog-converter -a GA:1,2,3 -t tasklist -o report.tasks PVS-Studio.log
```

## Динамический анализ

### AddressSanitizer
```bash
# Компиляция с AddressSanitizer
g++ -fsanitize=address -fno-omit-frame-pointer -g source.cpp -o program

# Запуск с дополнительными опциями
ASAN_OPTIONS=detect_leaks=1:abort_on_error=1 ./program
```

### ThreadSanitizer
```bash
# Компиляция с ThreadSanitizer
g++ -fsanitize=thread -fno-omit-frame-pointer -g source.cpp -o program

# Запуск
./program
```

### MemorySanitizer
```bash
# Компиляция с MemorySanitizer
clang++ -fsanitize=memory -fno-omit-frame-pointer -g source.cpp -o program

# Запуск
./program
```

### UndefinedBehaviorSanitizer
```bash
# Компиляция с UBSan
g++ -fsanitize=undefined -fno-omit-frame-pointer -g source.cpp -o program

# Запуск с дополнительными проверками
UBSAN_OPTIONS=print_stacktrace=1:abort_on_error=1 ./program
```

## Valgrind

### Memcheck (обнаружение ошибок памяти)
```bash
# Базовый анализ
valgrind --tool=memcheck ./program

# Анализ с детальным отчетом об утечках
valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./program

# Анализ с подавлением ложных срабатываний
valgrind --tool=memcheck --suppressions=suppressions.txt ./program
```

### Helgrind (обнаружение race conditions)
```bash
# Анализ многопоточности
valgrind --tool=helgrind ./program

# Анализ с дополнительными проверками
valgrind --tool=helgrind --history-level=full ./program
```

### DRD (обнаружение race conditions)
```bash
# Альтернативный инструмент для race conditions
valgrind --tool=drd ./program
```

## Фаззинг

### AFL++
```bash
# Компиляция для фаззинга
afl-gcc -g source.c -o program

# Запуск фаззинга
afl-fuzz -i input_dir -o output_dir ./program @@
```

### libFuzzer
```bash
# Компиляция с libFuzzer
clang++ -fsanitize=fuzzer,address source.cpp -o fuzzer

# Запуск фаззинга
./fuzzer corpus_dir
```

## Инструменты эксплуатации

### GDB с GEF
```bash
# Установка GEF
wget -q -O- https://github.com/hugsy/gef/raw/master/scripts/gef.sh | sh

# Запуск отладки
gdb ./program
```

### pwntools
```python
# Пример использования pwntools
from pwn import *

# Подключение к удаленному сервису
r = remote('target.com', 1337)

# Отправка данных
r.sendline(b'A' * 100)

# Получение ответа
response = r.recvline()
```

## Интеграция с CMake

### Базовый CMakeLists.txt с поддержкой анализа
```cmake
# Настройки для анализа безопасности
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_ASAN "-fsanitize=address -fno-omit-frame-pointer -g")
set(CMAKE_CXX_FLAGS_TSAN "-fsanitize=thread -fno-omit-frame-pointer -g")
set(CMAKE_CXX_FLAGS_UBSAN "-fsanitize=undefined -fno-omit-frame-pointer -g")

# Функция для создания целей с разными санитайзерами
function(add_security_targets target_name source_file)
    # Обычная версия
    add_executable(${target_name} ${source_file})
    
    # Версия с AddressSanitizer
    add_executable(${target_name}_asan ${source_file})
    target_compile_options(${target_name}_asan PRIVATE ${CMAKE_CXX_FLAGS_ASAN})
    target_link_options(${target_name}_asan PRIVATE ${CMAKE_CXX_FLAGS_ASAN})
    
    # Версия с ThreadSanitizer
    add_executable(${target_name}_tsan ${source_file})
    target_compile_options(${target_name}_tsan PRIVATE ${CMAKE_CXX_FLAGS_TSAN})
    target_link_options(${target_name}_tsan PRIVATE ${CMAKE_CXX_FLAGS_TSAN})
    
    # Версия с UndefinedBehaviorSanitizer
    add_executable(${target_name}_ubsan ${source_file})
    target_compile_options(${target_name}_ubsan PRIVATE ${CMAKE_CXX_FLAGS_UBSAN})
    target_link_options(${target_name}_ubsan PRIVATE ${CMAKE_CXX_FLAGS_UBSAN})
endfunction()
```

## Автоматизация анализа

### Скрипт для комплексного анализа
```bash
#!/bin/bash
# comprehensive_analysis.sh

echo "=== Комплексный анализ безопасности ==="

# Статический анализ
echo "1. Статический анализ с Clang..."
clang --analyze source.cpp

echo "2. Статический анализ с Cppcheck..."
cppcheck --enable=all source.cpp

# Динамический анализ
echo "3. Компиляция с AddressSanitizer..."
g++ -fsanitize=address -fno-omit-frame-pointer -g source.cpp -o program_asan

echo "4. Компиляция с ThreadSanitizer..."
g++ -fsanitize=thread -fno-omit-frame-pointer -g source.cpp -o program_tsan

echo "5. Компиляция с UndefinedBehaviorSanitizer..."
g++ -fsanitize=undefined -fno-omit-frame-pointer -g source.cpp -o program_ubsan

# Запуск анализа
echo "6. Запуск AddressSanitizer..."
./program_asan

echo "7. Запуск ThreadSanitizer..."
./program_tsan

echo "8. Запуск UndefinedBehaviorSanitizer..."
./program_ubsan

echo "9. Анализ с Valgrind..."
valgrind --tool=memcheck --leak-check=full ./program_asan

echo "=== Анализ завершен ==="
```

## Интерпретация результатов

### AddressSanitizer
- **ERROR: AddressSanitizer: heap-buffer-overflow** - Переполнение буфера в куче
- **ERROR: AddressSanitizer: use-after-free** - Использование после освобождения
- **ERROR: AddressSanitizer: double-free** - Двойное освобождение памяти

### ThreadSanitizer
- **WARNING: ThreadSanitizer: data race** - Race condition
- **WARNING: ThreadSanitizer: lock-order-inversion** - Инверсия порядка блокировок

### Valgrind
- **Invalid read/write** - Некорректное чтение/запись
- **Use of uninitialised value** - Использование неинициализированного значения
- **Memory leak** - Утечка памяти

## Рекомендации по использованию

1. **Начинайте со статического анализа** - он быстрый и находит много проблем
2. **Используйте динамический анализ** - для обнаружения runtime проблем
3. **Комбинируйте инструменты** - разные инструменты находят разные проблемы
4. **Интегрируйте в CI/CD** - автоматический анализ при каждом коммите
5. **Регулярно обновляйте инструменты** - новые версии находят больше проблем

