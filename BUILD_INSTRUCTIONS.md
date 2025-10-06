# 🛠️ Инструкции по сборке и запуску курса

## 📋 Требования

### Системные требования
- **ОС**: Windows 10/11, Linux, macOS
- **Компилятор**: GCC 7+, Clang 5+, MSVC 2017+ (с поддержкой C++17)
- **CMake**: версия 3.10 или выше
- **Git**: для клонирования репозитория

### Проверка требований
```bash
# Проверка версии компилятора
g++ --version
# или
clang++ --version
# или
cl /?

# Проверка версии CMake
cmake --version
```

## 🔨 Сборка проекта

### 1. Клонирование репозитория
```bash
git clone <repository-url>
cd Patterns
```

### 2. Создание директории для сборки
```bash
mkdir build
cd build
```

### 3. Конфигурация проекта
```bash
# Linux/macOS
cmake ..

# Windows (Visual Studio)
cmake .. -G "Visual Studio 16 2019"

# Windows (MinGW)
cmake .. -G "MinGW Makefiles"
```

### 4. Сборка
```bash
# Linux/macOS
make -j$(nproc)

# Windows (Visual Studio)
cmake --build . --config Release

# Windows (MinGW)
cmake --build .
```

## 🚀 Запуск примеров

### Модуль 1: Основы современного C++

#### Урок 1.1: Hello World в современном C++
```bash
# Linux/macOS
./01-basics/hello_world_modern

# Windows
01-basics\hello_world_modern.exe
```

#### Урок 1.2: RAII и управление ресурсами
```bash
# Linux/macOS
./01-basics/raii_demo

# Windows
01-basics\raii_demo.exe
```

#### Урок 1.3: Smart Pointers
```bash
# Linux/macOS
./01-basics/smart_pointers_demo

# Windows
01-basics\smart_pointers_demo.exe
```

#### Урок 1.4: Move Semantics
```bash
# Linux/macOS
./01-basics/move_semantics_demo

# Windows
01-basics\move_semantics_demo.exe
```

## 🧪 Тестирование

### Запуск всех тестов
```bash
# Linux/macOS
make test

# Windows
cmake --build . --target test
```

### Запуск конкретного теста
```bash
# Linux/macOS
ctest -R test_name

# Windows
ctest -R test_name -C Release
```

## 📚 Изучение материалов

### Структура курса
```
Patterns/
├── 01-basics/              # Модуль 1: Основы современного C++
│   ├── lesson_1_1_hello_world/
│   ├── lesson_1_2_raii/
│   ├── lesson_1_3_smart_pointers/
│   └── lesson_1_4_move_semantics/
├── 02-principles/          # Модуль 2: Принципы проектирования
├── 03-creational/          # Модуль 3: Креационные паттерны
├── 04-structural/          # Модуль 4: Структурные паттерны
├── 05-behavioral/          # Модуль 5: Поведенческие паттерны
├── 06-modern-cpp/          # Модуль 6: Современные C++ паттерны
├── common/                 # Общие утилиты
├── exercises/              # Практические упражнения
└── posters/                # Плакаты и схемы
```

### Порядок изучения
1. **Начните с README.md** - общий обзор курса
2. **Изучите плакаты** - визуальные схемы в папке `posters/`
3. **Читайте уроки** - подробные объяснения в каждой папке `lesson_X_X_`
4. **Запускайте примеры** - компилируйте и запускайте код
5. **Выполняйте упражнения** - практические задания в `exercises/`

## 🔧 Настройка IDE

### Visual Studio Code
1. Установите расширение C/C++
2. Установите расширение CMake Tools
3. Откройте папку проекта
4. Выберите компилятор в Command Palette (Ctrl+Shift+P) → "C/C++: Select a Configuration"

### Visual Studio
1. Откройте файл `CMakeLists.txt` в корне проекта
2. Visual Studio автоматически настроит проект
3. Выберите конфигурацию сборки (Debug/Release)

### CLion
1. Откройте папку проекта
2. CLion автоматически обнаружит CMake
3. Настройте профиль сборки в Settings → Build, Execution, Deployment → CMake

## 🐛 Решение проблем

### Ошибки компиляции

#### "C++17 features not supported"
```bash
# Убедитесь, что используете современный компилятор
g++ --version  # Должен быть 7.0+
```

#### "CMake not found"
```bash
# Установите CMake
# Ubuntu/Debian
sudo apt-get install cmake

# Windows
# Скачайте с https://cmake.org/download/
```

#### "Permission denied" (Linux/macOS)
```bash
# Дайте права на выполнение
chmod +x build/01-basics/hello_world_modern
```

### Ошибки времени выполнения

#### "File not found"
- Убедитесь, что запускаете из правильной директории
- Проверьте, что файлы были созданы при сборке

#### "Library not found"
- Проверьте, что все зависимости установлены
- Убедитесь, что библиотеки в PATH

## 📖 Дополнительные ресурсы

### Документация
- [cppreference.com](https://en.cppreference.com/) - справочник по C++
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/) - рекомендации по C++
- [Modern C++ Features](https://github.com/AnthonyCalandra/modern-cpp-features) - обзор возможностей

### Книги
- "Effective Modern C++" by Scott Meyers
- "C++17 - The Complete Guide" by Nicolai M. Josuttis
- "Design Patterns" by Gang of Four

### Онлайн курсы
- [Coursera C++ Specialization](https://www.coursera.org/specializations/c-programming)
- [edX C++ Programming](https://www.edx.org/learn/c-plus-plus)

## 🤝 Поддержка

### Получение помощи
1. **Изучите код** - многие вопросы решаются внимательным чтением
2. **Запустите примеры** - они демонстрируют правильные подходы
3. **Читайте комментарии** - код хорошо документирован
4. **Экспериментируйте** - изменяйте код и смотрите результаты

### Сообщение об ошибках
При обнаружении ошибок:
1. Опишите проблему подробно
2. Укажите версию компилятора и ОС
3. Приложите сообщение об ошибке
4. Опишите шаги для воспроизведения

---
*Удачного изучения паттернов программирования в современном C++! 🚀*
