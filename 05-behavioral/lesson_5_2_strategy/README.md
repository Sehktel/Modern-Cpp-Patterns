# Урок 5.2: Strategy Pattern (Стратегия)

## 🎯 Цель урока
Изучить паттерн Strategy - один из наиболее гибких поведенческих паттернов. Понять, как инкапсулировать алгоритмы и как обеспечить возможность их динамической замены во время выполнения.

## 📚 Что изучаем

### 1. Паттерн Strategy
- **Определение**: Определяет семейство алгоритмов, инкапсулирует каждый из них и делает их взаимозаменяемыми
- **Назначение**: Позволяет выбирать алгоритм во время выполнения
- **Применение**: Сортировка, валидация, расчеты, обработка платежей, игровые стратегии

### 2. Компоненты паттерна
- **Strategy (Стратегия)**: Интерфейс для всех конкретных стратегий
- **ConcreteStrategy**: Конкретная реализация стратегии
- **Context (Контекст)**: Класс, использующий стратегию
- **Client**: Клиент, который выбирает стратегию

### 3. Современные подходы
- **std::function**: Функциональный подход к стратегиям
- **Lambda expressions**: Встроенные стратегии
- **Template strategies**: Стратегии на основе шаблонов
- **Policy-based design**: Дизайн на основе политик

## 🔍 Ключевые концепции

### Классический Strategy
```cpp
// Интерфейс стратегии
class SortingStrategy {
public:
    virtual ~SortingStrategy() = default;
    virtual void sort(std::vector<int>& data) = 0;
};

// Конкретные стратегии
class BubbleSort : public SortingStrategy {
public:
    void sort(std::vector<int>& data) override {
        // Реализация пузырьковой сортировки
    }
};

class QuickSort : public SortingStrategy {
public:
    void sort(std::vector<int>& data) override {
        // Реализация быстрой сортировки
    }
};

// Контекст
class Sorter {
private:
    std::unique_ptr<SortingStrategy> strategy_;
    
public:
    void setStrategy(std::unique_ptr<SortingStrategy> strategy) {
        strategy_ = std::move(strategy);
    }
    
    void sortData(std::vector<int>& data) {
        if (strategy_) {
            strategy_->sort(data);
        }
    }
};
```

### Современный Strategy с std::function
```cpp
class ModernSorter {
private:
    std::function<void(std::vector<int>&)> sortFunction_;
    
public:
    void setSortStrategy(std::function<void(std::vector<int>&)> strategy) {
        sortFunction_ = strategy;
    }
    
    void sortData(std::vector<int>& data) {
        if (sortFunction_) {
            sortFunction_(data);
        }
    }
};

// Использование
ModernSorter sorter;
sorter.setSortStrategy([](std::vector<int>& data) {
    std::sort(data.begin(), data.end());
});
```

## 🤔 Вопросы для размышления

### 1. Когда использовать Strategy?
**✅ Хорошие случаи:**
- Несколько способов выполнения одной задачи
- Нужно выбирать алгоритм во время выполнения
- Хотите избежать условных операторов
- Алгоритмы должны быть взаимозаменяемыми
- Нужно инкапсулировать сложную логику

**❌ Плохие случаи:**
- Только один способ выполнения задачи
- Алгоритмы не изменяются
- Простые операции, не требующие инкапсуляции
- Когда добавляется ненужная сложность

### 2. Преимущества Strategy
- **Гибкость**: Легко добавлять новые алгоритмы
- **Инкапсуляция**: Каждый алгоритм изолирован
- **Открытость для расширения**: Новые стратегии без изменения кода
- **Устранение условных операторов**: Более чистый код

### 3. Недостатки Strategy
- **Сложность**: Добавляет дополнительные классы
- **Производительность**: Накладные расходы на виртуальные вызовы
- **Понимание**: Может усложнить понимание кода
- **Количество классов**: Увеличивает количество классов в системе

## 🛠️ Практические примеры

### Система валидации
```cpp
class ValidationStrategy {
public:
    virtual ~ValidationStrategy() = default;
    virtual bool validate(const std::string& data) = 0;
};

class EmailValidation : public ValidationStrategy {
public:
    bool validate(const std::string& data) override {
        return data.find("@") != std::string::npos;
    }
};

class PhoneValidation : public ValidationStrategy {
public:
    bool validate(const std::string& data) override {
        return data.length() >= 10 && std::all_of(data.begin(), data.end(), ::isdigit);
    }
};

class Validator {
private:
    std::unique_ptr<ValidationStrategy> strategy_;
    
public:
    void setStrategy(std::unique_ptr<ValidationStrategy> strategy) {
        strategy_ = std::move(strategy);
    }
    
    bool validate(const std::string& data) {
        return strategy_ ? strategy_->validate(data) : false;
    }
};
```

### Система расчета стоимости
```cpp
class PricingStrategy {
public:
    virtual ~PricingStrategy() = default;
    virtual double calculatePrice(double basePrice, int quantity) = 0;
};

class RegularPricing : public PricingStrategy {
public:
    double calculatePrice(double basePrice, int quantity) override {
        return basePrice * quantity;
    }
};

class BulkPricing : public PricingStrategy {
public:
    double calculatePrice(double basePrice, int quantity) override {
        if (quantity >= 10) {
            return basePrice * quantity * 0.9; // 10% скидка
        }
        return basePrice * quantity;
    }
};

class PremiumPricing : public PricingStrategy {
public:
    double calculatePrice(double basePrice, int quantity) override {
        return basePrice * quantity * 1.2; // 20% надбавка
    }
};
```

## 🎨 Современные подходы в C++

### Template Strategy
```cpp
template<typename Strategy>
class TemplateContext {
private:
    Strategy strategy_;
    
public:
    template<typename... Args>
    explicit TemplateContext(Args&&... args) 
        : strategy_(std::forward<Args>(args)...) {}
    
    template<typename... Args>
    auto execute(Args&&... args) -> decltype(strategy_.execute(args...)) {
        return strategy_.execute(std::forward<Args>(args)...);
    }
};

// Конкретные стратегии
struct AddStrategy {
    int execute(int a, int b) { return a + b; }
};

struct MultiplyStrategy {
    int execute(int a, int b) { return a * b; }
};
```

### Policy-based Design
```cpp
template<typename SortingPolicy>
class PolicyBasedSorter {
private:
    SortingPolicy sortingPolicy_;
    
public:
    void sort(std::vector<int>& data) {
        sortingPolicy_.sort(data);
    }
};

// Политики сортировки
struct BubbleSortPolicy {
    void sort(std::vector<int>& data) {
        // Реализация пузырьковой сортировки
    }
};

struct QuickSortPolicy {
    void sort(std::vector<int>& data) {
        // Реализация быстрой сортировки
    }
};

// Использование
PolicyBasedSorter<BubbleSortPolicy> bubbleSorter;
PolicyBasedSorter<QuickSortPolicy> quickSorter;
```

### Functional Strategy
```cpp
class FunctionalStrategy {
private:
    std::function<double(double, int)> pricingFunction_;
    
public:
    void setPricingStrategy(std::function<double(double, int)> strategy) {
        pricingFunction_ = strategy;
    }
    
    double calculatePrice(double basePrice, int quantity) {
        return pricingFunction_ ? pricingFunction_(basePrice, quantity) : 0.0;
    }
};

// Использование с lambda
FunctionalStrategy calculator;
calculator.setPricingStrategy([](double basePrice, int quantity) {
    return basePrice * quantity * (quantity >= 10 ? 0.9 : 1.0);
});
```

## 🧪 Тестирование Strategy

### Создание моков для тестирования
```cpp
class MockSortingStrategy : public SortingStrategy {
public:
    MOCK_METHOD(void, sort, (std::vector<int>&), (override));
};

// Тест
TEST(StrategyTest, SortData) {
    auto mockStrategy = std::make_unique<MockSortingStrategy>();
    EXPECT_CALL(*mockStrategy, sort(_)).Times(1);
    
    Sorter sorter;
    sorter.setStrategy(std::move(mockStrategy));
    
    std::vector<int> data = {3, 1, 4, 1, 5};
    sorter.sortData(data);
}
```

## 🎯 Практические упражнения

### Упражнение 1: Система скидок
Создайте систему расчета скидок с различными стратегиями (процентная, фиксированная, накопительная).

### Упражнение 2: Игровые стратегии
Реализуйте систему стратегий для игрового ИИ (агрессивная, защитная, случайная).

### Упражнение 3: Форматирование данных
Создайте систему форматирования данных в различные форматы (JSON, XML, CSV).

### Упражнение 4: Алгоритмы поиска
Реализуйте различные алгоритмы поиска (линейный, бинарный, хеш-поиск).

## 📈 Связь с другими паттернами

### Strategy vs State
- **Strategy**: Выбор алгоритма во время выполнения
- **State**: Изменение поведения объекта в зависимости от состояния

### Strategy vs Template Method
- **Strategy**: Полная замена алгоритма
- **Template Method**: Изменение отдельных шагов алгоритма

### Strategy vs Command
- **Strategy**: Инкапсуляция алгоритма
- **Command**: Инкапсуляция запроса как объекта

## 📈 Следующие шаги
После изучения Strategy вы будете готовы к:
- Уроку 5.3: Command Pattern
- Пониманию поведенческих паттернов
- Созданию гибких систем
- Использованию policy-based design

## 💡 Важные принципы

1. **Инкапсуляция алгоритмов**: Каждая стратегия должна быть изолирована
2. **Взаимозаменяемость**: Стратегии должны быть взаимозаменяемыми
3. **Открытость для расширения**: Легко добавлять новые стратегии
4. **Производительность**: Учитывайте накладные расходы на виртуальные вызовы
5. **Простота**: Не усложняйте без необходимости
