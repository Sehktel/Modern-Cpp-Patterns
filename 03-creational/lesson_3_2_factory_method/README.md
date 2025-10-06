# Урок 3.2: Factory Method Pattern (Фабричный метод)

## 🎯 Цель урока
Изучить паттерн Factory Method - один из наиболее полезных креационных паттернов. Понять, как создавать объекты без привязки к конкретным классам и как обеспечить расширяемость системы.

## 📚 Что изучаем

### 1. Паттерн Factory Method
- **Определение**: Определяет интерфейс для создания объектов, но позволяет подклассам решать, какой класс инстанцировать
- **Назначение**: Инкапсуляция создания объектов и обеспечение гибкости
- **Применение**: Создание UI элементов, документов, игровых объектов

### 2. Проблемы и решения
- **Tight Coupling**: Связывание с конкретными классами
- **Open/Closed Principle**: Открыт для расширения, закрыт для модификации
- **Polymorphism**: Использование полиморфизма для создания объектов
- **Dependency Inversion**: Зависимость от абстракций, а не от конкретных классов

### 3. Варианты реализации
- **Simple Factory**: Простая фабрика (не является паттерном GoF)
- **Factory Method**: Абстрактный фабричный метод
- **Static Factory**: Статические фабричные методы
- **Modern C++**: Использование современных возможностей C++

## 🔍 Ключевые концепции

### Проблема без Factory Method
```cpp
// Плохо - жесткая связь с конкретными классами
class DocumentProcessor {
public:
    void processDocument(const std::string& type) {
        if (type == "PDF") {
            PDFDocument doc;
            doc.open();
            doc.process();
        } else if (type == "Word") {
            WordDocument doc;
            doc.open();
            doc.process();
        }
        // Проблемы:
        // - Нарушение Open/Closed Principle
        // - Жесткая связь с конкретными классами
        // - Сложно добавлять новые типы
    }
};
```

### Решение с Factory Method
```cpp
// Хорошо - использование Factory Method
class DocumentProcessor {
private:
    std::unique_ptr<DocumentFactory> factory_;
    
public:
    explicit DocumentProcessor(std::unique_ptr<DocumentFactory> factory)
        : factory_(std::move(factory)) {}
    
    void processDocument(const std::string& type) {
        auto doc = factory_->createDocument(type);
        doc->open();
        doc->process();
    }
};
```

### Абстрактная фабрика
```cpp
class DocumentFactory {
public:
    virtual ~DocumentFactory() = default;
    virtual std::unique_ptr<Document> createDocument(const std::string& type) = 0;
};

class ModernDocumentFactory : public DocumentFactory {
public:
    std::unique_ptr<Document> createDocument(const std::string& type) override {
        if (type == "PDF") {
            return std::make_unique<ModernPDFDocument>();
        } else if (type == "Word") {
            return std::make_unique<ModernWordDocument>();
        }
        throw std::invalid_argument("Unknown document type: " + type);
    }
};
```

## 🤔 Вопросы для размышления

### 1. Когда использовать Factory Method?
**✅ Хорошие случаи:**
- Когда класс не знает заранее, какие объекты ему нужно создавать
- Когда класс хочет, чтобы его подклассы определяли тип создаваемых объектов
- Когда нужно инкапсулировать логику создания объектов
- Когда система должна быть расширяемой без модификации

**❌ Плохие случаи:**
- Когда создание объектов простое и не изменяется
- Когда нет необходимости в полиморфизме
- Когда добавляется ненужная сложность

### 2. Преимущества Factory Method
- **Инкапсуляция**: Скрывает детали создания объектов
- **Полиморфизм**: Использует полиморфизм для создания объектов
- **Расширяемость**: Легко добавлять новые типы объектов
- **Тестируемость**: Легко создавать моки для тестирования

### 3. Недостатки Factory Method
- **Сложность**: Добавляет дополнительные классы и интерфейсы
- **Производительность**: Небольшие накладные расходы на виртуальные вызовы
- **Понимание**: Может усложнить понимание кода для новичков

## 🛠️ Практические примеры

### Создание UI элементов
```cpp
class Widget {
public:
    virtual ~Widget() = default;
    virtual void render() = 0;
    virtual void onClick() = 0;
};

class Button : public Widget {
public:
    void render() override {
        std::cout << "Rendering button" << std::endl;
    }
    
    void onClick() override {
        std::cout << "Button clicked" << std::endl;
    }
};

class TextBox : public Widget {
public:
    void render() override {
        std::cout << "Rendering text box" << std::endl;
    }
    
    void onClick() override {
        std::cout << "Text box clicked" << std::endl;
    }
};

class WidgetFactory {
public:
    virtual ~WidgetFactory() = default;
    virtual std::unique_ptr<Widget> createWidget(const std::string& type) = 0;
};

class UIFactory : public WidgetFactory {
public:
    std::unique_ptr<Widget> createWidget(const std::string& type) override {
        if (type == "button") {
            return std::make_unique<Button>();
        } else if (type == "textbox") {
            return std::make_unique<TextBox>();
        }
        throw std::invalid_argument("Unknown widget type: " + type);
    }
};
```

### Создание игровых объектов
```cpp
class GameObject {
public:
    virtual ~GameObject() = default;
    virtual void update() = 0;
    virtual void render() = 0;
};

class Player : public GameObject {
public:
    void update() override {
        std::cout << "Updating player" << std::endl;
    }
    
    void render() override {
        std::cout << "Rendering player" << std::endl;
    }
};

class Enemy : public GameObject {
public:
    void update() override {
        std::cout << "Updating enemy" << std::endl;
    }
    
    void render() override {
        std::cout << "Rendering enemy" << std::endl;
    }
};

class GameObjectFactory {
public:
    virtual ~GameObjectFactory() = default;
    virtual std::unique_ptr<GameObject> createGameObject(const std::string& type) = 0;
};

class GameFactory : public GameObjectFactory {
public:
    std::unique_ptr<GameObject> createGameObject(const std::string& type) override {
        if (type == "player") {
            return std::make_unique<Player>();
        } else if (type == "enemy") {
            return std::make_unique<Enemy>();
        }
        throw std::invalid_argument("Unknown game object type: " + type);
    }
};
```

## 🎨 Современные подходы в C++

### Static Factory Methods
```cpp
class Point {
private:
    double x_, y_;
    
    Point(double x, double y) : x_(x), y_(y) {}
    
public:
    // Static factory methods
    static Point fromCartesian(double x, double y) {
        return Point(x, y);
    }
    
    static Point fromPolar(double radius, double angle) {
        double x = radius * std::cos(angle);
        double y = radius * std::sin(angle);
        return Point(x, y);
    }
    
    static Point origin() {
        return Point(0, 0);
    }
    
    double getX() const { return x_; }
    double getY() const { return y_; }
};
```

### Template Factory
```cpp
template<typename T>
class Factory {
public:
    template<typename... Args>
    static std::unique_ptr<T> create(Args&&... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
};

// Использование
auto player = Factory<Player>::create("John", 100);
auto enemy = Factory<Enemy>::create("Goblin", 50);
```

### Registry-based Factory
```cpp
template<typename BaseType>
class FactoryRegistry {
private:
    std::map<std::string, std::function<std::unique_ptr<BaseType>()>> creators_;
    
public:
    template<typename DerivedType>
    void registerType(const std::string& name) {
        creators_[name] = []() {
            return std::make_unique<DerivedType>();
        };
    }
    
    std::unique_ptr<BaseType> create(const std::string& name) {
        auto it = creators_.find(name);
        if (it != creators_.end()) {
            return it->second();
        }
        throw std::invalid_argument("Unknown type: " + name);
    }
};

// Использование
FactoryRegistry<GameObject> registry;
registry.registerType<Player>("player");
registry.registerType<Enemy>("enemy");

auto player = registry.create("player");
```

## 🧪 Тестирование Factory Method

### Создание моков для тестирования
```cpp
class MockWidget : public Widget {
public:
    MOCK_METHOD(void, render, (), (override));
    MOCK_METHOD(void, onClick, (), (override));
};

class MockWidgetFactory : public WidgetFactory {
public:
    MOCK_METHOD(std::unique_ptr<Widget>, createWidget, (const std::string&), (override));
};

// Тест
TEST(WidgetTest, CreateWidget) {
    auto mockFactory = std::make_unique<MockWidgetFactory>();
    auto mockWidget = std::make_unique<MockWidget>();
    
    EXPECT_CALL(*mockFactory, createWidget("button"))
        .WillOnce(Return(ByMove(std::move(mockWidget))));
    
    // Тестируем создание виджета
}
```

## 🎯 Практические упражнения

### Упражнение 1: Создание системы документов
Создайте систему обработки документов с использованием Factory Method.

### Упражнение 2: UI Framework
Реализуйте простой UI фреймворк с фабрикой для создания элементов.

### Упражнение 3: Игровые объекты
Создайте систему создания игровых объектов с различными фабриками.

### Упражнение 4: Registry-based Factory
Реализуйте фабрику на основе реестра для динамической регистрации типов.

## 📈 Связь с другими паттернами

### Factory Method vs Abstract Factory
- **Factory Method**: Создает один тип объекта
- **Abstract Factory**: Создает семейства связанных объектов

### Factory Method vs Builder
- **Factory Method**: Создает объект в один шаг
- **Builder**: Создает объект пошагово с возможностью настройки

### Factory Method vs Prototype
- **Factory Method**: Создает новые объекты
- **Prototype**: Клонирует существующие объекты

## 📈 Следующие шаги
После изучения Factory Method вы будете готовы к:
- Уроку 3.3: Abstract Factory Pattern
- Пониманию различий между креационными паттернами
- Изучению принципов SOLID
- Созданию расширяемых систем

## 💡 Важные принципы

1. **Инкапсуляция создания**: Скрывайте детали создания объектов
2. **Полиморфизм**: Используйте полиморфизм для создания объектов
3. **Расширяемость**: Система должна быть открыта для расширения
4. **Тестируемость**: Легко создавать моки для тестирования
5. **Простота**: Не усложняйте без необходимости
