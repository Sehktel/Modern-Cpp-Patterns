# Contributing to C++ Patterns Course

Спасибо за интерес к проекту! Мы рады любому вкладу в развитие курса.

## 🎯 Как можно помочь

### 📝 Документация
- Улучшение существующих README
- Добавление примеров и упражнений
- Исправление опечаток и ошибок
- Переводы на другие языки

### 💻 Код
- Новые примеры паттернов
- Оптимизация существующего кода
- Дополнительные security анализы
- Бенчмарки производительности

### 🎨 Визуализация
- UML диаграммы
- Схемы архитектуры
- Инфографика

### 🧪 Тестирование
- Unit тесты
- Integration тесты
- Security тесты

## 🔧 Процесс Contribution

### 1. Fork репозитория

```bash
# Создайте fork на GitHub, затем клонируйте
git clone https://github.com/your-username/cpp-patterns.git
cd cpp-patterns
```

### 2. Создайте feature branch

```bash
git checkout -b feature/your-feature-name
```

**Naming conventions для branches:**
- `feature/` - новая функциональность
- `fix/` - исправление ошибок
- `docs/` - изменения в документации
- `refactor/` - рефакторинг кода
- `security/` - security-related изменения

### 3. Внесите изменения

Следуйте нашим **стандартам кода**:

```cpp
// Комментарии на русском языке
// Код в стиле Senior Developer

// ✅ Хорошо
class PatternExample {
private:
    std::unique_ptr<Resource> resource_;  // RAII
    
public:
    // Явный конструктор с проверкой
    explicit PatternExample(int value) {
        if (value < 0) {
            throw std::invalid_argument("Value must be non-negative");
        }
    }
    
    // Удаляем копирование, разрешаем перемещение
    PatternExample(const PatternExample&) = delete;
    PatternExample& operator=(const PatternExample&) = delete;
    PatternExample(PatternExample&&) noexcept = default;
    PatternExample& operator=(PatternExample&&) noexcept = default;
};
```

### 4. Проверьте код

```bash
# Сборка
mkdir build && cd build
cmake ..
cmake --build .

# Тесты
ctest

# Статический анализ
clang-tidy ../path/to/your/file.cpp

# Форматирование
clang-format -i ../path/to/your/file.cpp
```

### 5. Commit изменения

```bash
git add .
git commit -m "feat: add new example for Observer pattern

- Added real-world example
- Improved documentation
- Added performance benchmarks
"
```

**Commit message format:**
```
<type>: <subject>

<body>

<footer>
```

**Types:**
- `feat`: Новая функциональность
- `fix`: Исправление ошибки
- `docs`: Изменения в документации
- `style`: Форматирование кода
- `refactor`: Рефакторинг
- `test`: Добавление тестов
- `chore`: Обновление конфигурации

### 6. Push и создайте PR

```bash
git push origin feature/your-feature-name
```

Затем создайте Pull Request на GitHub.

## 📋 Требования к Pull Request

### Checklist перед созданием PR:

- [ ] Код компилируется без ошибок
- [ ] Все тесты проходят
- [ ] Добавлена документация
- [ ] Соблюден code style
- [ ] Нет compiler warnings
- [ ] Security анализ выполнен
- [ ] Комментарии на русском языке
- [ ] README обновлен при необходимости

### Описание PR должно включать:

1. **Что изменено** - краткое описание
2. **Зачем** - обоснование изменений
3. **Как протестировано** - шаги проверки
4. **Скриншоты** (если применимо)
5. **Related issues** - ссылки на связанные issues

**Пример PR description:**

```markdown
## Что изменено
Добавлен новый пример Observer pattern с использованием std::observer_ptr из C++23

## Зачем
Показать современный подход к реализации паттерна

## Как протестировано
- Собрано на GCC 13, Clang 16, MSVC 2022
- Проверено с AddressSanitizer
- Добавлены unit тесты

## Related Issues
Closes #42
```

## 🎨 Стандарты Кода

### C++ Style Guide

Следуем **C++ Core Guidelines**:

```cpp
// ✅ ХОРОШО
class GoodExample {
private:
    std::string name_;    // Trailing underscore для member variables
    int value_{0};        // Default initialization
    
public:
    explicit GoodExample(std::string name)  // Pass by value для move
        : name_(std::move(name)) {}
    
    [[nodiscard]] const std::string& getName() const noexcept {
        return name_;
    }
};

// ❌ ПЛОХО
class bad_example {  // Неправильный naming
    string name;      // Нет std::, нет underscore
    int value;        // Не инициализирован
    
    bad_example(string n) { name = n; }  // Не explicit, копирование
    string get_name() { return name; }   // Возврат по значению, не const
};
```

### Naming Conventions

```cpp
// Классы и типы: PascalCase
class MyClass { };
struct DataStructure { };
using AliasType = int;

// Функции и переменные: camelCase или snake_case
void processData();
int calculateSum();

// Member variables: camelCase с trailing underscore
class Example {
    int value_;
    std::string name_;
};

// Constants: UPPER_CASE
constexpr int MAX_SIZE = 100;
const double PI = 3.14159;

// Namespaces: lowercase
namespace patterns {
namespace creational {
}
}
```

### Комментарии

```cpp
// Комментарии на русском языке для объяснений
// Английские термины можно не переводить

// ============================================================================
// ЗАГОЛОВОК РАЗДЕЛА
// ============================================================================

// Документирующие комментарии для функций
/**
 * Вычисляет сумму элементов вектора.
 * 
 * @param data Вектор целых чисел
 * @return Сумма всех элементов
 * @throws std::overflow_error если происходит переполнение
 */
int calculateSum(const std::vector<int>& data);

// Inline комментарии для сложной логики
if (condition) {
    // Объяснение почему это необходимо
    complexOperation();
}
```

## 🛡️ Security Guidelines

### Все новые примеры должны включать:

1. **Безопасная версия** - правильная реализация
2. **Уязвимая версия** - для обучения
3. **Эксплоиты** - демонстрация атак
4. **Security анализ** - документация

```cpp
// vulnerable_example.cpp
class VulnerableClass {
    char* buffer_;  // ❌ Сырой указатель
    
public:
    void unsafeOperation(const char* input) {
        strcpy(buffer_, input);  // ❌ Buffer overflow!
    }
};

// secure_example.cpp
class SecureClass {
    std::vector<char> buffer_;  // ✅ RAII
    
public:
    void safeOperation(std::string_view input) {
        buffer_.assign(input.begin(), input.end());  // ✅ Безопасно
    }
};
```

## 📝 Документация

### README Structure

Каждый паттерн должен иметь README со структурой:

```markdown
# Паттерн Name

## 🎯 Цель урока
## 📚 Что изучаем
## 🔍 Ключевые концепции
## 🤔 Вопросы для размышления
## 🛠️ Практические примеры
## 🎨 Альтернативные подходы
## 🚀 Современный C++
## 🎯 Практические упражнения
## 📈 Следующие шаги
## ⚠️ Важные предупреждения
```

### Security Analysis Structure

```markdown
# Анализ безопасности паттерна X

## Обзор уязвимостей
## Инструменты анализа
## Практические задания
## Безопасные альтернативы
## Эксплоиты и атаки
## Защитные меры
## Связь с другими паттернами
```

## 🧪 Тестирование

### Требования к тестам

```cpp
#include <gtest/gtest.h>

// Тест должен быть self-descriptive
TEST(PatternTest, CreatesObjectCorrectly) {
    // Arrange
    auto factory = createFactory();
    
    // Act
    auto object = factory->create();
    
    // Assert
    ASSERT_NE(object, nullptr);
    EXPECT_EQ(object->getValue(), 42);
}

// Тест edge cases
TEST(PatternTest, HandlesEmptyInput) {
    // ...
}

// Тест производительности (опционально)
TEST(PatternTest, PerformanceIsAcceptable) {
    // Benchmark
}
```

## 🚀 Процесс Review

### Что проверяет reviewer:

1. **Корректность** - код работает правильно
2. **Style** - соответствие стандартам
3. **Документация** - наличие и полнота
4. **Тесты** - покрытие и качество
5. **Security** - отсутствие уязвимостей
6. **Performance** - нет очевидных проблем

### Timeline

- **Review начнется в течение**: 2-3 дней
- **Feedback**: В течение недели
- **Merge**: После approval от maintainer

## 💬 Коммуникация

### Где задавать вопросы:

- **Issues** - для багов и feature requests
- **Discussions** - для вопросов и обсуждений
- **Pull Requests** - для review комментариев

### Code of Conduct

- Будьте уважительны
- Конструктивная критика
- Помогайте новичкам
- Фокус на коде, а не на людях

## 🎓 Для Новичков

Если вы впервые вносите вклад в open source:

1. Начните с **good first issue**
2. Читайте существующий код
3. Задавайте вопросы
4. Делайте маленькие PR
5. Учитесь на review комментариях

### Good First Issues

Ищите issues с метками:
- `good first issue`
- `help wanted`
- `documentation`
- `beginner friendly`

## 📚 Ресурсы

- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [How to Write a Git Commit Message](https://chris.beams.io/posts/git-commit/)
- [GitHub Flow](https://guides.github.com/introduction/flow/)

---

## 🙏 Спасибо

Спасибо за ваш вклад в развитие курса!

Каждый contribution, большой или маленький, ценится.

---

**Вопросы?** Создайте issue или напишите в Discussions.

