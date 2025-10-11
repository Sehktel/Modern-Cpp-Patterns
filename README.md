# 🎓 Современные Паттерны Проектирования в C++

<div align="center">

![C++](https://img.shields.io/badge/C%2B%2B-17%2F20%2F23-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![License](https://img.shields.io/badge/license-MIT-green?style=for-the-badge)
![Completion](https://img.shields.io/badge/completion-100%25-brightgreen?style=for-the-badge)
![Patterns](https://img.shields.io/badge/patterns-35-blue?style=for-the-badge)
![Modules](https://img.shields.io/badge/modules-9-orange?style=for-the-badge)

**Полный курс по паттернам проектирования с фокусом на безопасность, производительность и современные подходы**

[Быстрый старт](#-быстрый-старт) •
[Структура курса](#-структура-курса) •
[Особенности](#-особенности-курса) •
[Документация](#-документация) •
[Contributing](#-contributing)

</div>

---

## 🎯 О Курсе

Это **комплексный практический курс** по паттернам проектирования в современном C++, охватывающий:

- 📚 **35 паттернов** - от базовых до продвинутых
- 🛡️ **Анализ безопасности** - уязвимости, эксплоиты, защита
- ⚡ **Производительность** - оптимизации и бенчмарки
- 🚀 **C++17/20/23** - современные стандарты и возможности
- 🎨 **Визуализация** - UML диаграммы и схемы
- 💻 **Практика** - рабочий код и упражнения

### ✅ Статус Проекта: **ЗАВЕРШЕН 100%**

```
Всего паттернов:    35/35  ✅ 100%
README файлов:      35/35  ✅ 100%
Security анализ:    35/35  ✅ 100%
C++23 сравнения:    35/35  ✅ 100%
Эксплоиты:          35/35  ✅ 100%
Защищенные версии:  35/35  ✅ 100%
```

---

## 🚀 Быстрый Старт

### Предварительные требования

```bash
# Компилятор с поддержкой C++17 или новее
- GCC 7+ / Clang 5+ / MSVC 2017+
- CMake 3.10+
- Git
```

### Клонирование и сборка

```bash
# Клонируем репозиторий
git clone https://github.com/yourusername/cpp-patterns.git
cd cpp-patterns

# Создаем директорию для сборки
mkdir build && cd build

# Конфигурируем проект
cmake ..

# Собираем все примеры
cmake --build .

# Или собираем конкретный модуль
cmake --build . --target lesson_3_1_singleton
```

### Запуск примеров

```bash
# Запускаем конкретный паттерн
./build/03-creational/lesson_3_1_singleton/singleton_pattern

# Запускаем с security анализом
./build/03-creational/lesson_3_1_singleton/singleton_vulnerabilities
```

---

## 📚 Структура Курса

Курс состоит из **9 модулей**, охватывающих все аспекты паттернов проектирования:

### 📖 Модуль 1: Основы Современного C++ (4 урока)

Фундаментальные концепции для понимания паттернов:

- **1.1 Hello World** - Введение в современный C++17/20/23
- **1.2 RAII** - Resource Acquisition Is Initialization
- **1.3 Smart Pointers** - `unique_ptr`, `shared_ptr`, `weak_ptr`
- **1.4 Move Semantics** - Эффективная передача владения ресурсами

### 🏗️ Модуль 2: Принципы Проектирования (4 урока)

Фундаментальные принципы качественного кода:

- **2.1 SOLID** - Single Responsibility, Open/Closed, Liskov Substitution, Interface Segregation, Dependency Inversion
- **2.2 DRY, KISS, YAGNI** - Классические принципы разработки
- **2.3 Композиция vs Наследование** - Выбор правильного подхода
- **2.4 Dependency Injection** - Управление зависимостями

### 🏭 Модуль 3: Порождающие Паттерны (4 урока)

Паттерны создания объектов:

- **3.1 Singleton** - Единственный экземпляр класса
- **3.2 Factory Method** - Делегирование создания объектов
- **3.3 Abstract Factory** - Семейства связанных объектов
- **3.4 Builder** - Пошаговое построение сложных объектов

### 🔧 Модуль 4: Структурные Паттерны (4 урока)

Паттерны композиции классов и объектов:

- **4.1 Adapter** - Адаптация несовместимых интерфейсов
- **4.2 Decorator** - Динамическое добавление функциональности
- **4.3 Facade** - Упрощенный интерфейс к подсистеме
- **4.4 Proxy** - Контроль доступа к объекту

### 🎭 Модуль 5: Поведенческие Паттерны (4 урока)

Паттерны взаимодействия объектов:

- **5.1 Observer** - Подписка на изменения объекта
- **5.2 Strategy** - Семейство взаимозаменяемых алгоритмов
- **5.3 Command** - Инкапсуляция запросов как объектов
- **5.4 State** - Изменение поведения в зависимости от состояния

### ⚡ Модуль 6: Современный C++ (4 урока)

Продвинутые техники современного C++:

- **6.1 Smart Pointers** - Углубленное изучение управления памятью
- **6.2 Move Semantics** - Оптимизация производительности
- **6.3 CRTP** - Curiously Recurring Template Pattern
- **6.4 Type Erasure** - Стирание типов для гибкости

### 🧵 Модуль 7: Многопоточность (4 урока)

Паттерны для concurrent программирования:

- **7.1 Producer-Consumer** - Асинхронная обработка данных
- **7.2 Thread Pool** - Управление пулом потоков
- **7.3 Actor Model** - Изолированные вычислительные единицы
- **7.4 Reactor** - Event-driven архитектура

### 🚄 Модуль 8: Высоконагруженные Системы (4 урока)

Паттерны для масштабируемых приложений:

- **8.1 Cache-Aside** - Стратегии кэширования
- **8.2 Circuit Breaker** - Защита от каскадных отказов
- **8.3 Bulkhead** - Изоляция ресурсов
- **8.4 Saga** - Распределенные транзакции

### 🔥 Модуль 9: Производительность (3 урока)

Паттерны оптимизации:

- **9.1 Object Pool** - Переиспользование объектов
- **9.2 Flyweight** - Разделение состояния для экономии памяти
- **9.3 Command Queue** - Пакетная обработка команд

---

## 🎨 Особенности Курса

### 🛡️ Фокус на Безопасности

Каждый паттерн включает:

```
📁 lesson_X_Y_pattern/
├── README.md                              # Теория и примеры
├── pattern_name.cpp                       # Правильная реализация
├── pattern_name_vulnerabilities.cpp       # ❌ Уязвимая версия
├── secure_pattern_name_alternatives.cpp   # ✅ Безопасная версия
├── pattern_name_cpp23_comparison.cpp      # 🚀 C++23 подход
├── exploits/
│   └── pattern_name_exploits.cpp          # 💣 Примеры эксплоитов
├── SECURITY_ANALYSIS.md                   # Детальный анализ
└── PATTERN_SECURITY_POSTER.md             # Визуальный плакат
```

### 📊 Сравнение Стандартов

Каждый паттерн показывает эволюцию от C++17 к C++23:

```cpp
// C++17
auto factory = std::make_unique<ConcreteFactory>();
auto product = factory->createProduct();

// C++20 с Concepts
template<Factory T>
auto product = createProduct<T>();

// C++23 с std::expected
std::expected<Product, Error> product = tryCreateProduct();
```

### 🎯 Практический Подход

- ✅ **Реальные примеры** из production кода
- ✅ **Упражнения** для закрепления
- ✅ **Бенчмарки** производительности
- ✅ **Альтернативные решения** и их анализ
- ✅ **Когда НЕ использовать** паттерн

### 📈 Прогрессивное Обучение

```
Основы C++ → Принципы → Классические паттерны → 
Современный C++ → Concurrency → High-Load → Performance
```

---

## 🛠️ Инструменты и Утилиты

### Автоматическая Проверка

```bash
# Проверка структуры всего курса
python tools/sync_course_structure.py

# Проверка только статуса (без записи)
python tools/sync_course_structure.py --check-only

# Вывод в JSON
python tools/sync_course_structure.py --json
```

### Анализ Безопасности

```bash
# Статический анализ
clang-tidy lesson_*.cpp

# AddressSanitizer
g++ -fsanitize=address -g vulnerabilities.cpp

# ThreadSanitizer
g++ -fsanitize=thread -g concurrent_pattern.cpp
```

### Генерация Документации

```bash
# Doxygen документация
doxygen Doxyfile

# Markdown документация
python tools/generate_docs.py
```

---

## 📖 Документация

### Для Каждого Паттерна

- **README.md** - Теория, примеры, упражнения
- **SECURITY_ANALYSIS.md** - Анализ уязвимостей
- **POSTER.md** - Визуальное представление

### Общая Документация

- [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md) - Инструкции по сборке
- [COURSE_STRUCTURE_COMPLETE.md](COURSE_STRUCTURE_COMPLETE.md) - Полная структура курса
- [TECH_DEBT_TODO.md](TECH_DEBT_TODO.md) - Технический долг
- [CPP23_MIGRATION_GUIDE.md](CPP23_MIGRATION_GUIDE.md) - Миграция на C++23
- [tools/README.md](tools/README.md) - Инструменты разработки

### 🆕 Валидация AI-кода

- **[VALIDATION_QUICKSTART.md](VALIDATION_QUICKSTART.md)** - 🚀 Быстрый старт за 5 минут
- [tools/VERIFICATION_STRATEGY.md](tools/VERIFICATION_STRATEGY.md) - Полная стратегия верификации
- [tools/PRACTICAL_VALIDATION_GUIDE.md](tools/PRACTICAL_VALIDATION_GUIDE.md) - Практические примеры

**Проблема**: Как проверить, что AI-генерированный код не "врет"?

**Решение**: Автоматическая валидация с санитайзерами (ASan, TSan) и статическим анализом (Clang-Tidy, Cppcheck)

```powershell
# Проверить один паттерн
.\tools\validate_pattern.ps1 -Pattern "singleton" -Full

# Проверить всё (35 паттернов)
.\tools\validate_all_patterns.ps1 -Full
```

---

## 💡 Философия Курса

> **"Паттерны - это не решение, а инструмент для мышления о проблемах"**

Каждый паттерн рассматривается с точки зрения:

### 🤔 Критического Мышления

- **Когда использовать?** - Контекст применения
- **Альтернативы?** - Другие подходы к решению
- **Компромиссы?** - Trade-offs и ограничения
- **Современный подход?** - C++20/23 возможности

### 🔬 Научный Подход

- **Математическая формализация** - Теория типов и алгоритмов
- **Complexity analysis** - O-нотация и производительность
- **Эмпирические данные** - Бенчмарки и измерения
- **Теория и практика** - Связь концепций с реализацией

### 👨‍🎓 Как PhD в Computer Science

Объяснения на уровне PhD:
- Теория типов и категорий
- Формальные методы
- Паттерны как алгебраические структуры
- Связь с функциональным программированием

---

## 🎓 Для Кого Этот Курс

### ✅ Идеально подходит для:

- **Middle/Senior разработчиков** желающих углубить знания
- **Архитекторов ПО** проектирующих сложные системы
- **Security инженеров** анализирующих уязвимости
- **Преподавателей** ищущих качественный материал
- **Студентов CS** стремящихся к глубокому пониманию

### 📋 Требуемые знания:

- Базовые знания C++ (классы, шаблоны, наследование)
- Понимание принципов ООП
- Опыт разработки (желательно, но не обязательно)

---

## 🔧 Технологический Стек

```
Языки:       C++17, C++20, C++23
Build:       CMake 3.10+
Компиляторы: GCC 7+, Clang 5+, MSVC 2017+
Инструменты: Git, Python 3.8+, Doxygen
Анализ:      clang-tidy, cppcheck, AddressSanitizer
Тестинг:     Google Test, Catch2 (опционально)
```

---

## 📊 Статистика Проекта

```
📁 Общее количество файлов:     400+
📝 Строк кода:                   50,000+
📚 Паттернов реализовано:        35
🛡️ Security анализов:            35
📖 README документов:            35
🎨 Визуальных плакатов:          35
💣 Примеров эксплоитов:          35
✅ Безопасных альтернатив:       35
🚀 C++23 сравнений:              35
```

---

## 🤝 Contributing

Приветствуются contributions! Вот как вы можете помочь:

### Как внести вклад

1. **Fork** репозиторий
2. Создайте **feature branch** (`git checkout -b feature/AmazingFeature`)
3. **Commit** изменения (`git commit -m 'Add some AmazingFeature'`)
4. **Push** в branch (`git push origin feature/AmazingFeature`)
5. Откройте **Pull Request**

### Области для улучшения

- 📝 Дополнительные примеры и упражнения
- 🌐 Переводы на другие языки
- 🎨 Улучшение визуализаций
- 🔍 Новые security анализы
- ⚡ Бенчмарки производительности
- 📚 Дополнительная документация

### Стиль кода

- Следуйте **C++ Core Guidelines**
- Комментарии на **русском языке**
- Код в стиле **Senior Developer**
- Объяснения как **PhD уровень**

---

## 🐛 Нашли Ошибку?

Создайте issue с описанием:
- Что ожидалось
- Что произошло
- Шаги для воспроизведения
- Версия компилятора и ОС

---

## 📜 Лицензия

Этот проект распространяется под лицензией **MIT License**.

```
MIT License

Copyright (c) 2025 Sehktel

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
```

См. [LICENSE](LICENSE) для полного текста.

---

## 👨‍💻 Автор

**Sehktel**

- 💼 Senior C++ Developer
- 🎓 Специалист по паттернам проектирования
- 🔒 Security-focused разработчик
- 📚 Автор образовательных материалов

---

## 🌟 Поддержите Проект

Если курс оказался полезным:

- ⭐ Поставьте **Star** на GitHub
- 🔄 Поделитесь с коллегами
- 💬 Оставьте feedback в Issues
- 🤝 Внесите свой вклад через PR

---

## 📚 Рекомендуемая Литература

- **"Design Patterns"** - Gang of Four (GoF)
- **"Effective Modern C++"** - Scott Meyers
- **"C++ Concurrency in Action"** - Anthony Williams
- **"C++ Templates: The Complete Guide"** - David Vandevoorde
- **"Large-Scale C++"** - John Lakos

---

## 🔗 Полезные Ссылки

- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [cppreference.com](https://en.cppreference.com/)
- [C++ Standards Committee](https://isocpp.org/)
- [Compiler Explorer (Godbolt)](https://godbolt.org/)
- [Quick C++ Benchmark](https://quick-bench.com/)

---

<div align="center">

**⭐ Если вам понравился курс, поставьте звезду! ⭐**

Made with ❤️ by [Sehktel](https://github.com/sehktel)

© 2025 Sehktel. All rights reserved.

</div>
