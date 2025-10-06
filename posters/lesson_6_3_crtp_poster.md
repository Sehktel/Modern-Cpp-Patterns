# 🔄 CRTP (Curiously Recurring Template Pattern) - Плакат

## 📋 Обзор паттерна

**CRTP (Curiously Recurring Template Pattern)** - это идиома C++, где класс наследуется от шаблона, параметризованного самим этим классом. Это позволяет достичь статического полиморфизма без накладных расходов на виртуальные функции.

## 🎯 Назначение

- **Статический полиморфизм**: Полиморфизм на этапе компиляции без виртуальных функций
- **Производительность**: Отсутствие накладных расходов на виртуальные вызовы
- **Переиспользование кода**: Базовый класс может вызывать методы производного класса
- **Типобезопасность**: Компилятор проверяет типы на этапе компиляции

## 🏗️ Структура паттерна

```
┌─────────────────────────────────┐
│   Base<T>                       │
│   (базовый шаблон)              │
├─────────────────────────────────┤
│ + commonMethod()                │
│ + polymorphicMethod()           │
│   └─ static_cast<T*>(this)      │
└─────────────────────────────────┘
         ▲
         │
    ┌────┴────┐
    │         │
┌───▼───┐ ┌──▼───┐
│Derived│ │Derived│
│<Self> │ │<Self> │
├───────┤ ├───────┤
│+method│ │+method│
└───────┘ └───────┘
```

## 💡 Ключевые компоненты

### 1. Базовый шаблонный класс
```cpp
// Базовый класс с CRTP
template<typename Derived>
class Base {
public:
    // Общие методы
    void commonMethod() {
        std::cout << "Общий метод из базового класса" << std::endl;
    }
    
    // Полиморфный метод, который вызывает метод производного класса
    void polymorphicMethod() {
        static_cast<Derived*>(this)->derivedMethod();
    }
    
    // Метод, который может быть переопределен
    void virtualMethod() {
        static_cast<Derived*>(this)->customImplementation();
    }
    
    // Получение ссылки на производный класс
    Derived& asDerived() {
        return *static_cast<Derived*>(this);
    }
    
    const Derived& asDerived() const {
        return *static_cast<const Derived*>(this);
    }
};
```

### 2. Производные классы
```cpp
// Производный класс, наследующийся от себя же
class ConcreteClass1 : public Base<ConcreteClass1> {
public:
    void derivedMethod() {
        std::cout << "Реализация 1 из ConcreteClass1" << std::endl;
    }
    
    void customImplementation() {
        std::cout << "Кастомная реализация 1" << std::endl;
    }
};

class ConcreteClass2 : public Base<ConcreteClass2> {
public:
    void derivedMethod() {
        std::cout << "Реализация 2 из ConcreteClass2" << std::endl;
    }
    
    void customImplementation() {
        std::cout << "Кастомная реализация 2" << std::endl;
    }
};
```

## 🔧 Практический пример: Система сериализации

```cpp
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <type_traits>

// Базовый класс для сериализации с CRTP
template<typename Derived>
class Serializable {
public:
    // Общие методы сериализации
    std::string serialize() {
        std::string result = "{\n";
        result += "  \"type\": \"" + getTypeName() + "\",\n";
        result += "  \"data\": " + static_cast<Derived*>(this)->serializeData() + "\n";
        result += "}";
        return result;
    }
    
    void deserialize(const std::string& data) {
        static_cast<Derived*>(this)->deserializeData(data);
    }
    
    // Получение имени типа
    std::string getTypeName() const {
        return static_cast<const Derived*>(this)->getClassName();
    }
    
    // Общий метод для валидации
    bool validate() {
        return static_cast<Derived*>(this)->validateData();
    }
    
    // Метод для получения размера данных
    size_t getDataSize() const {
        return static_cast<const Derived*>(this)->calculateSize();
    }
};

// Класс пользователя с CRTP
class User : public Serializable<User> {
private:
    std::string name;
    int age;
    std::string email;
    
public:
    User(const std::string& n = "", int a = 0, const std::string& e = "")
        : name(n), age(a), email(e) {}
    
    // Методы, требуемые базовым классом
    std::string serializeData() const {
        return "{\n"
               "    \"name\": \"" + name + "\",\n"
               "    \"age\": " + std::to_string(age) + ",\n"
               "    \"email\": \"" + email + "\"\n"
               "  }";
    }
    
    void deserializeData(const std::string& data) {
        // Упрощенная десериализация
        std::cout << "Десериализация пользователя из: " << data << std::endl;
        // В реальной реализации здесь был бы парсинг JSON
    }
    
    std::string getClassName() const {
        return "User";
    }
    
    bool validateData() const {
        return !name.empty() && age > 0 && !email.empty();
    }
    
    size_t calculateSize() const {
        return name.size() + email.size() + sizeof(age);
    }
    
    // Специфичные методы пользователя
    void setName(const std::string& n) { name = n; }
    void setAge(int a) { age = a; }
    void setEmail(const std::string& e) { email = e; }
    
    std::string getName() const { return name; }
    int getAge() const { return age; }
    std::string getEmail() const { return email; }
};

// Класс продукта с CRTP
class Product : public Serializable<Product> {
private:
    std::string title;
    double price;
    int stock;
    std::vector<std::string> categories;
    
public:
    Product(const std::string& t = "", double p = 0.0, int s = 0)
        : title(t), price(p), stock(s) {}
    
    // Методы, требуемые базовым классом
    std::string serializeData() const {
        std::string result = "{\n"
                            "    \"title\": \"" + title + "\",\n"
                            "    \"price\": " + std::to_string(price) + ",\n"
                            "    \"stock\": " + std::to_string(stock) + ",\n"
                            "    \"categories\": [";
        
        for (size_t i = 0; i < categories.size(); ++i) {
            result += "\"" + categories[i] + "\"";
            if (i < categories.size() - 1) result += ", ";
        }
        result += "]\n  }";
        return result;
    }
    
    void deserializeData(const std::string& data) {
        std::cout << "Десериализация продукта из: " << data << std::endl;
    }
    
    std::string getClassName() const {
        return "Product";
    }
    
    bool validateData() const {
        return !title.empty() && price >= 0 && stock >= 0;
    }
    
    size_t calculateSize() const {
        size_t size = title.size() + sizeof(price) + sizeof(stock);
        for (const auto& cat : categories) {
            size += cat.size();
        }
        return size;
    }
    
    // Специфичные методы продукта
    void setTitle(const std::string& t) { title = t; }
    void setPrice(double p) { price = p; }
    void setStock(int s) { stock = s; }
    void addCategory(const std::string& category) {
        categories.push_back(category);
    }
    
    std::string getTitle() const { return title; }
    double getPrice() const { return price; }
    int getStock() const { return stock; }
    const std::vector<std::string>& getCategories() const { return categories; }
};

// Класс заказа с CRTP
class Order : public Serializable<Order> {
private:
    std::string orderId;
    std::string customerId;
    std::vector<std::string> productIds;
    double totalAmount;
    std::string status;
    
public:
    Order(const std::string& id = "", const std::string& customer = "")
        : orderId(id), customerId(customer), totalAmount(0.0), status("pending") {}
    
    // Методы, требуемые базовым классом
    std::string serializeData() const {
        std::string result = "{\n"
                            "    \"orderId\": \"" + orderId + "\",\n"
                            "    \"customerId\": \"" + customerId + "\",\n"
                            "    \"totalAmount\": " + std::to_string(totalAmount) + ",\n"
                            "    \"status\": \"" + status + "\",\n"
                            "    \"productIds\": [";
        
        for (size_t i = 0; i < productIds.size(); ++i) {
            result += "\"" + productIds[i] + "\"";
            if (i < productIds.size() - 1) result += ", ";
        }
        result += "]\n  }";
        return result;
    }
    
    void deserializeData(const std::string& data) {
        std::cout << "Десериализация заказа из: " << data << std::endl;
    }
    
    std::string getClassName() const {
        return "Order";
    }
    
    bool validateData() const {
        return !orderId.empty() && !customerId.empty() && totalAmount >= 0;
    }
    
    size_t calculateSize() const {
        size_t size = orderId.size() + customerId.size() + status.size() + sizeof(totalAmount);
        for (const auto& id : productIds) {
            size += id.size();
        }
        return size;
    }
    
    // Специфичные методы заказа
    void addProduct(const std::string& productId) {
        productIds.push_back(productId);
    }
    
    void setTotalAmount(double amount) {
        totalAmount = amount;
    }
    
    void setStatus(const std::string& newStatus) {
        status = newStatus;
    }
    
    std::string getOrderId() const { return orderId; }
    std::string getCustomerId() const { return customerId; }
    double getTotalAmount() const { return totalAmount; }
    std::string getStatus() const { return status; }
};

// Менеджер сериализации с поддержкой CRTP
template<typename T>
class SerializationManager {
public:
    static void saveToFile(const T& obj, const std::string& filename) {
        std::cout << "💾 Сохранение " << obj.getTypeName() << " в файл " << filename << std::endl;
        std::string data = obj.serialize();
        std::cout << "Данные: " << data << std::endl;
    }
    
    static T loadFromFile(const std::string& filename) {
        std::cout << "📖 Загрузка из файла " << filename << std::endl;
        T obj;
        // В реальной реализации здесь был бы чтение из файла
        obj.deserialize("{\"type\":\"" + obj.getTypeName() + "\",\"data\":{}}");
        return obj;
    }
    
    static void validateAndSave(const T& obj, const std::string& filename) {
        if (obj.validate()) {
            saveToFile(obj, filename);
        } else {
            std::cout << "❌ Ошибка валидации " << obj.getTypeName() << std::endl;
        }
    }
};

// Утилиты для работы с CRTP
template<typename T>
void printObjectInfo(const T& obj) {
    std::cout << "📊 Информация об объекте:" << std::endl;
    std::cout << "   Тип: " << obj.getTypeName() << std::endl;
    std::cout << "   Размер данных: " << obj.getDataSize() << " байт" << std::endl;
    std::cout << "   Валидность: " << (obj.validate() ? "✅" : "❌") << std::endl;
}

// Демонстрация использования
int main() {
    std::cout << "=== Демонстрация CRTP (Curiously Recurring Template Pattern) ===" << std::endl;
    
    // Создание объектов с CRTP
    User user("Иван Петров", 30, "ivan@example.com");
    Product product("Ноутбук", 999.99, 5);
    Order order("ORD-001", "CUST-001");
    
    product.addCategory("Электроника");
    product.addCategory("Компьютеры");
    
    order.addProduct("PROD-001");
    order.addProduct("PROD-002");
    order.setTotalAmount(1999.98);
    order.setStatus("confirmed");
    
    // Демонстрация полиморфизма на этапе компиляции
    std::cout << "\n🔄 Демонстрация статического полиморфизма:" << std::endl;
    
    printObjectInfo(user);
    printObjectInfo(product);
    printObjectInfo(order);
    
    // Сериализация объектов
    std::cout << "\n📝 Сериализация объектов:" << std::endl;
    
    std::cout << "\n👤 Пользователь:" << std::endl;
    std::cout << user.serialize() << std::endl;
    
    std::cout << "\n🛍️ Продукт:" << std::endl;
    std::cout << product.serialize() << std::endl;
    
    std::cout << "\n📦 Заказ:" << std::endl;
    std::cout << order.serialize() << std::endl;
    
    // Использование менеджера сериализации
    std::cout << "\n💾 Работа с менеджером сериализации:" << std::endl;
    
    SerializationManager<User>::saveToFile(user, "user.json");
    SerializationManager<Product>::validateAndSave(product, "product.json");
    SerializationManager<Order>::validateAndSave(order, "order.json");
    
    // Демонстрация загрузки
    std::cout << "\n📖 Загрузка объектов:" << std::endl;
    
    auto loadedUser = SerializationManager<User>::loadFromFile("user.json");
    auto loadedProduct = SerializationManager<Product>::loadFromFile("product.json");
    auto loadedOrder = SerializationManager<Order>::loadFromFile("order.json");
    
    // Демонстрация работы с базовыми методами
    std::cout << "\n🔧 Работа с базовыми методами:" << std::endl;
    
    user.commonMethod();
    product.commonMethod();
    order.commonMethod();
    
    return 0;
}
```

## 🎯 Применение в реальных проектах

### 1. **Библиотеки сериализации**
```cpp
// Boost.Serialization использует CRTP
template<typename Derived>
class Serializable {
public:
    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version) {
        static_cast<Derived*>(this)->serializeImpl(ar, version);
    }
};

class MyClass : public Serializable<MyClass> {
public:
    template<typename Archive>
    void serializeImpl(Archive& ar, const unsigned int version) {
        ar & data;
    }
    
private:
    int data;
};
```

### 2. **Системы логирования**
```cpp
// CRTP для различных типов логгеров
template<typename Derived>
class Logger {
public:
    void log(const std::string& message) {
        static_cast<Derived*>(this)->writeLog(message);
    }
    
    void logError(const std::string& error) {
        static_cast<Derived*>(this)->writeError(error);
    }
};

class FileLogger : public Logger<FileLogger> {
public:
    void writeLog(const std::string& message) {
        // Запись в файл
    }
    
    void writeError(const std::string& error) {
        // Запись ошибки в файл
    }
};
```

### 3. **Системы кэширования**
```cpp
// CRTP для различных стратегий кэширования
template<typename Derived>
class Cache {
public:
    template<typename Key, typename Value>
    void put(const Key& key, const Value& value) {
        static_cast<Derived*>(this)->store(key, value);
    }
    
    template<typename Key>
    auto get(const Key& key) -> decltype(static_cast<Derived*>(this)->retrieve(key)) {
        return static_cast<Derived*>(this)->retrieve(key);
    }
};

class LRUCache : public Cache<LRUCache> {
public:
    template<typename Key, typename Value>
    void store(const Key& key, const Value& value) {
        // Реализация LRU
    }
    
    template<typename Key>
    auto retrieve(const Key& key) -> decltype(values[key]) {
        // Получение из LRU
        return values[key];
    }
    
private:
    std::unordered_map<Key, Value> values;
};
```

## ⚡ Преимущества паттерна

### ✅ **Плюсы:**
- **Производительность**: Отсутствие накладных расходов на виртуальные функции
- **Статический полиморфизм**: Полиморфизм на этапе компиляции
- **Типобезопасность**: Компилятор проверяет типы
- **Переиспользование кода**: Базовый класс может вызывать методы производного
- **Отсутствие RTTI**: Не требует информации о типах во время выполнения

### ❌ **Минусы:**
- **Сложность**: Сложнее понять и отладить
- **Ограниченность**: Работает только с известными типами на этапе компиляции
- **Шаблонная магия**: Много шаблонного кода
- **Ошибки компиляции**: Сложные сообщения об ошибках

## 🔄 Альтернативы и связанные паттерны

### **Альтернативы:**
- **Виртуальные функции**: Для динамического полиморфизма
- **std::variant**: Для type-safe union
- **std::any**: Для type erasure

### **Связанные паттерны:**
- **Template Method**: CRTP может реализовывать Template Method
- **Strategy**: CRTP может использоваться для стратегий
- **Type Erasure**: Альтернативный подход к полиморфизму

## 🎨 Современные улучшения в C++

### 1. **Использование концептов (C++20)**
```cpp
#include <concepts>

// Концепт для CRTP
template<typename T, typename Base>
concept CRTPDerived = std::derived_from<T, Base<T>>;

// Безопасный CRTP с концептами
template<typename Derived>
class SafeBase {
    static_assert(CRTPDerived<Derived, SafeBase>);
    
public:
    void method() {
        static_cast<Derived*>(this)->implementation();
    }
};
```

### 2. **Использование requires (C++20)**
```cpp
// Требования для производного класса
template<typename Derived>
class AdvancedBase {
public:
    void process() {
        static_cast<Derived*>(this)->processImpl();
    }
    
private:
    // Требование наличия метода
    static_assert(requires(Derived d) {
        d.processImpl();
    });
};
```

## 🚀 Лучшие практики

### 1. **Правильное использование CRTP**
```cpp
// ✅ Хорошо: Четкое разделение ответственности
template<typename Derived>
class Drawable {
public:
    void draw() {
        static_cast<Derived*>(this)->drawImpl();
    }
    
    void drawWithBorder() {
        drawBorder();
        draw();
    }
    
private:
    void drawBorder() {
        // Общая логика для всех производных классов
    }
};

class Circle : public Drawable<Circle> {
public:
    void drawImpl() {
        // Специфичная реализация для круга
    }
};
```

### 2. **Избегание проблем**
```cpp
// ❌ Плохо: Неправильное наследование
class WrongDerived : public Base<OtherClass> {  // Ошибка!
    // Это не CRTP!
};

// ✅ Хорошо: Правильное наследование
class CorrectDerived : public Base<CorrectDerived> {  // Правильно!
    // Это настоящий CRTP
};
```

### 3. **Безопасность типов**
```cpp
// ✅ Хорошо: Проверка типов на этапе компиляции
template<typename Derived>
class TypeSafeBase {
    static_assert(std::is_base_of_v<TypeSafeBase<Derived>, Derived>);
    
public:
    void safeMethod() {
        static_cast<Derived*>(this)->derivedMethod();
    }
};
```

## 📊 Сравнение с другими подходами

| Аспект | CRTP | Виртуальные функции | std::variant |
|--------|------|---------------------|--------------|
| **Производительность** | Высокая | Средняя | Высокая |
| **Полиморфизм** | Статический | Динамический | Статический |
| **Гибкость** | Низкая | Высокая | Средняя |
| **Сложность** | Высокая | Низкая | Средняя |
| **Типобезопасность** | Высокая | Средняя | Высокая |
| **Применение** | Известные типы | Неизвестные типы | Ограниченный набор типов |

## 🎯 Заключение

**CRTP** - это мощная идиома C++ для достижения статического полиморфизма без накладных расходов на виртуальные функции. Она особенно полезна в библиотеках и системах, где производительность критична.

### **Когда использовать:**
- ✅ Нужен статический полиморфизм
- ✅ Производительность критична
- ✅ Типы известны на этапе компиляции
- ✅ Нужно избежать накладных расходов на виртуальные функции

### **Когда НЕ использовать:**
- ❌ Нужен динамический полиморфизм
- ❌ Типы неизвестны на этапе компиляции
- ❌ Сложность кода критична
- ❌ Команда не готова к шаблонной магии

**CRTP** - это продвинутая техника C++, которая позволяет создавать высокопроизводительные и типобезопасные системы! 🔄✨
