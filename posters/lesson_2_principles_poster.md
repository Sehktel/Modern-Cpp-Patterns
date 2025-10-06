# 🎯 Плакат: Принципы проектирования программного обеспечения

## 📚 Модуль 2: Принципы проектирования

> **"Принципы - это не догмы, а инструменты для мышления о проблемах проектирования"** - Robert C. Martin

---

## 🔬 SOLID Принципы

### S - Single Responsibility Principle (SRP)
**"Каждый класс должен иметь только одну причину для изменения"**

```cpp
// ❌ Плохо: Множественные ответственности
class BadUserManager {
    void saveUser();
    void sendEmail();
    void validateData();
    void logActivity();
};

// ✅ Хорошо: Единственная ответственность
class User { /* только данные */ };
class UserValidator { /* только валидация */ };
class EmailService { /* только email */ };
class Logger { /* только логирование */ };
```

**Математическая формализация:**
```
∀C ∈ Classes: |Reasons(C)| = 1
где Reasons(C) = {r | изменение r требует модификации C}
```

### O - Open/Closed Principle (OCP)
**"Программные сущности должны быть открыты для расширения, но закрыты для модификации"**

```cpp
// ❌ Плохо: Модификация для добавления функциональности
class BadPaymentProcessor {
    void processPayment(PaymentType type) {
        switch(type) {
            case CREDIT_CARD: /* ... */ break;
            case PAYPAL: /* ... */ break;
            // Новый тип требует модификации!
        }
    }
};

// ✅ Хорошо: Расширение без модификации
class PaymentProcessor {
    std::unique_ptr<PaymentStrategy> strategy;
public:
    void setStrategy(std::unique_ptr<PaymentStrategy> s) {
        strategy = std::move(s);
    }
};
```

### L - Liskov Substitution Principle (LSP)
**"Объекты производного класса должны быть заменяемы объектами базового класса"**

```cpp
// ❌ Нарушение LSP
class BadSquare : public Rectangle {
    void setWidth(int w) override {
        width = w;
        height = w; // Нарушение: изменяет поведение базового класса
    }
};

// ✅ Соблюдение LSP
class Shape {
public:
    virtual double getArea() const = 0;
    virtual double getPerimeter() const = 0;
};
```

### I - Interface Segregation Principle (ISP)
**"Клиенты не должны зависеть от интерфейсов, которые они не используют"**

```cpp
// ❌ Плохо: Слишком большой интерфейс
class BadIDevice {
    virtual void read() = 0;
    virtual void write() = 0;
    virtual void scan() = 0;
    virtual void print() = 0;
    virtual void fax() = 0;
};

// ✅ Хорошо: Разделение интерфейсов
class IReadable { virtual void read() = 0; };
class IWritable { virtual void write() = 0; };
class IPrintable { virtual void print() = 0; };
```

### D - Dependency Inversion Principle (DIP)
**"Модули высокого уровня не должны зависеть от модулей низкого уровня. Оба должны зависеть от абстракций"**

```cpp
// ❌ Плохо: Зависимость от конкретных реализаций
class BadBusinessLogic {
    FileLogger logger; // Прямая зависимость
};

// ✅ Хорошо: Зависимость от абстракций
class GoodBusinessLogic {
    std::unique_ptr<ILogger> logger; // Зависимость от интерфейса
public:
    GoodBusinessLogic(std::unique_ptr<ILogger> l) : logger(std::move(l)) {}
};
```

---

## 🎨 DRY, KISS, YAGNI

### DRY - Don't Repeat Yourself
**"Каждый фрагмент знания должен иметь единственное, однозначное, авторитетное представление"**

```cpp
// ❌ Дублирование логики
bool validateEmail1(const std::string& email) { /* ... */ }
bool validateEmail2(const std::string& email) { /* ... */ }

// ✅ Единая система валидации
class EmailValidator {
public:
    static bool isValid(const std::string& email) { /* ... */ }
};
```

### KISS - Keep It Simple Stupid
**"Простота должна быть ключевой целью, и ненужная сложность должна быть устранена"**

```cpp
// ❌ Избыточная сложность
class OverEngineeredConfig {
    std::map<std::string, std::unique_ptr<ConfigNode>> nodes;
    // Сложная иерархия для простой задачи
};

// ✅ Простое решение
class SimpleConfig {
    std::map<std::string, std::string> values;
public:
    void set(const std::string& key, const std::string& value) {
        values[key] = value;
    }
};
```

### YAGNI - You Aren't Gonna Need It
**"Не добавляйте функциональность, пока она не понадобится"**

```cpp
// ❌ Преждевременная оптимизация
class OverEngineeredNotificationSystem {
    // Поддержка всех возможных типов уведомлений заранее
    void sendSlackNotification();
    void sendDiscordNotification();
    void sendTelegramNotification();
};

// ✅ Только необходимое
class SimpleNotificationService {
public:
    void sendEmail(const std::string& message) { /* ... */ }
    void sendSMS(const std::string& message) { /* ... */ }
};
```

---

## 🔄 Композиция vs Наследование

### Наследование (Inheritance)
**Отношение "is-a"**

```cpp
// ✅ Хорошее использование наследования
class Shape {
public:
    virtual double getArea() const = 0;
    virtual double getPerimeter() const = 0;
};

class Circle : public Shape {
    double radius;
public:
    double getArea() const override {
        return 3.14159 * radius * radius;
    }
};
```

**Проблемы наследования:**
- ❌ Fragile Base Class Problem
- ❌ Diamond Problem
- ❌ Tight Coupling
- ❌ Violation of LSP

### Композиция (Composition)
**Отношение "has-a"**

```cpp
// ✅ Композиция предпочтительнее
class Vehicle {
    std::unique_ptr<Engine> engine;
    std::vector<std::unique_ptr<Wheel>> wheels;
    std::unique_ptr<FuelTank> fuelTank;
public:
    Vehicle(std::unique_ptr<Engine> e,
           std::unique_ptr<FuelTank> ft)
        : engine(std::move(e)), fuelTank(std::move(ft)) {}
};
```

**Преимущества композиции:**
- ✅ Loose Coupling
- ✅ Flexibility
- ✅ Easier Testing
- ✅ Multiple Inheritance Simulation

---

## 💉 Dependency Injection

### Constructor Injection
```cpp
class UserService {
    std::unique_ptr<IEmailService> emailService;
    std::unique_ptr<IDatabaseService> dbService;
public:
    UserService(std::unique_ptr<IEmailService> email,
               std::unique_ptr<IDatabaseService> db)
        : emailService(std::move(email)), dbService(std::move(db)) {}
};
```

### Setter Injection
```cpp
class FlexibleService {
    std::unique_ptr<ILogger> logger;
public:
    void setLogger(std::unique_ptr<ILogger> l) {
        logger = std::move(l);
    }
};
```

### Interface Injection
```cpp
class IInjectableService {
public:
    virtual void setDependencies(std::unique_ptr<IDependency> dep) = 0;
};
```

---

## 📊 Метрики качества

### Основные метрики:
- **CBO (Coupling Between Objects)** - связанность между объектами
- **LCOM (Lack of Cohesion in Methods)** - недостаток связности
- **DIT (Depth of Inheritance Tree)** - глубина иерархии наследования
- **NOC (Number of Children)** - количество потомков
- **RFC (Response For Class)** - количество методов

### Целевые значения:
- CBO < 7 (низкая связанность)
- LCOM < 1 (высокая связность)
- DIT < 6 (неглубокая иерархия)
- NOC < 7 (разумное количество потомков)

---

## 🎯 Практические рекомендации

### Когда использовать принципы:

#### SOLID:
- **SRP**: Всегда для бизнес-логики
- **OCP**: При частых изменениях требований
- **LSP**: При проектировании иерархий
- **ISP**: При работе с внешними библиотеками
- **DIP**: При необходимости тестирования

#### DRY, KISS, YAGNI:
- **DRY**: При обнаружении дублирования
- **KISS**: При усложнении архитектуры
- **YAGNI**: При планировании новых функций

#### Композиция vs Наследование:
- **Наследование**: Истинные "is-a" отношения
- **Композиция**: "has-a" отношения, гибкость

#### Dependency Injection:
- **Constructor Injection**: Обязательные зависимости
- **Setter Injection**: Опциональные зависимости
- **Interface Injection**: Сложные сценарии

---

## 🔬 Современные подходы

### C++20 возможности:
- **Concepts** для строгой типизации
- **Modules** для улучшения модульности
- **Coroutines** для асинхронной обработки

### Популярные библиотеки:
- **Boost.DI** - мощный DI контейнер
- **Fruit** - легковесный DI фреймворк
- **Hypodermic** - header-only DI контейнер

---

## 📈 Влияние на качество

### Преимущества применения принципов:
- 🧪 **Тестируемость**: +50-70% улучшение
- 🔧 **Сопровождаемость**: +30-50% улучшение
- 🔄 **Гибкость**: +40-60% улучшение
- 🐛 **Качество**: -40-60% снижение багов

### Компромиссы:
- 📈 **Сложность**: Увеличение на 20-30%
- ⏱️ **Время разработки**: +15-25% на начальном этапе
- 🎓 **Кривая обучения**: Требует времени на освоение

---

## 💡 Заключение

### Ключевые выводы:
1. **Принципы взаимосвязаны** и должны применяться комплексно
2. **Контекст определяет приоритеты** - нет универсального решения
3. **Баланс важнее догм** - здравый смысл превыше слепого следования
4. **Современные инструменты** упрощают применение принципов

### Золотые правила:
- 🎯 **Начинайте с простого** (KISS)
- 🔄 **Устраняйте дублирование** (DRY)
- ⚡ **Не добавляйте лишнего** (YAGNI)
- 🏗️ **Предпочитайте композицию** наследованию
- 💉 **Используйте DI** для тестируемости

---

## 🚀 Следующие шаги

### Рекомендуемое изучение:
1. **Модуль 3**: Креационные паттерны
2. **Модуль 4**: Структурные паттерны
3. **Модуль 5**: Поведенческие паттерны
4. **Модуль 6**: Современные C++ паттерны

### Практические задания:
- Рефакторинг существующего кода с применением принципов
- Создание тестов с использованием DI
- Анализ метрик качества кода
- Изучение современных DI фреймворков

---

**🎓 "Принципы - это инструменты для мышления о проблемах проектирования. Используйте их мудро!"**

*Плакат создан для модуля "Принципы проектирования" курса по паттернам программирования в современном C++*
