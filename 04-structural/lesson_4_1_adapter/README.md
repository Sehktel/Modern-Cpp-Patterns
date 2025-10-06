# Урок 4.1: Adapter Pattern (Адаптер)

## 🎯 Цель урока
Изучить паттерн Adapter - один из наиболее практичных структурных паттернов. Понять, как интегрировать несовместимые интерфейсы и как адаптировать существующий код для работы с новыми системами.

## 📚 Что изучаем

### 1. Паттерн Adapter
- **Определение**: Позволяет объектам с несовместимыми интерфейсами работать вместе
- **Назначение**: Интеграция legacy кода, работа с внешними библиотеками, унификация интерфейсов
- **Применение**: Интеграция с API, работа с базами данных, адаптация библиотек

### 2. Типы адаптеров
- **Object Adapter**: Композиция (предпочтительный подход)
- **Class Adapter**: Множественное наследование (только в языках с множественным наследованием)
- **Two-way Adapter**: Двунаправленная адаптация

### 3. Проблемы и решения
- **Legacy Integration**: Интеграция старого кода
- **Third-party Libraries**: Работа с внешними библиотеками
- **Interface Mismatch**: Несовместимость интерфейсов
- **Version Compatibility**: Совместимость версий

## 🔍 Ключевые концепции

### Проблема несовместимых интерфейсов
```cpp
// Старая система - ожидает конкретный формат
class OldSystem {
public:
    void processData(const std::string& oldFormat) {
        // Обрабатывает данные в старом формате
    }
};

// Новая система - использует другой формат
class NewSystem {
public:
    void processData(const std::vector<int>& newFormat) {
        // Обрабатывает данные в новом формате
    }
};

// Проблема: Как заставить их работать вместе?
```

### Решение с Adapter
```cpp
// Адаптер для интеграции систем
class SystemAdapter {
private:
    OldSystem& oldSystem_;
    
public:
    explicit SystemAdapter(OldSystem& oldSystem) : oldSystem_(oldSystem) {}
    
    void processData(const std::vector<int>& newFormat) {
        // Преобразуем новый формат в старый
        std::string oldFormat = convertToOldFormat(newFormat);
        oldSystem_.processData(oldFormat);
    }
    
private:
    std::string convertToOldFormat(const std::vector<int>& data) {
        // Логика преобразования
        std::string result;
        for (int value : data) {
            result += std::to_string(value) + ",";
        }
        return result;
    }
};
```

### Object Adapter (Композиция)
```cpp
// Целевой интерфейс
class Target {
public:
    virtual ~Target() = default;
    virtual void request() = 0;
};

// Адаптируемый класс
class Adaptee {
public:
    void specificRequest() {
        std::cout << "Specific request from Adaptee" << std::endl;
    }
};

// Object Adapter
class ObjectAdapter : public Target {
private:
    std::unique_ptr<Adaptee> adaptee_;
    
public:
    ObjectAdapter() : adaptee_(std::make_unique<Adaptee>()) {}
    
    void request() override {
        adaptee_->specificRequest();
    }
};
```

## 🤔 Вопросы для размышления

### 1. Когда использовать Adapter?
**✅ Хорошие случаи:**
- Интеграция с legacy системами
- Работа с внешними библиотеками
- Унификация различных API
- Адаптация интерфейсов баз данных
- Интеграция с третьими сторонами

**❌ Плохие случаи:**
- Когда можно изменить исходный код
- Когда адаптер становится слишком сложным
- Когда есть более простые решения
- Когда нарушается принцип единственной ответственности

### 2. Преимущества Adapter
- **Интеграция**: Позволяет интегрировать несовместимые системы
- **Переиспользование**: Переиспользует существующий код
- **Инкапсуляция**: Скрывает сложность преобразования
- **Гибкость**: Легко добавлять новые адаптеры

### 3. Недостатки Adapter
- **Сложность**: Добавляет дополнительные классы
- **Производительность**: Накладные расходы на преобразование
- **Понимание**: Может усложнить понимание кода
- **Тестирование**: Нужно тестировать как адаптер, так и адаптируемый класс

## 🛠️ Практические примеры

### Адаптация API для работы с базами данных
```cpp
// Интерфейс для работы с БД
class Database {
public:
    virtual ~Database() = default;
    virtual void connect() = 0;
    virtual void executeQuery(const std::string& query) = 0;
    virtual void disconnect() = 0;
};

// MySQL драйвер (старый интерфейс)
class MySQLDriver {
public:
    void openConnection() {
        std::cout << "MySQL: Открываем соединение" << std::endl;
    }
    
    void runQuery(const std::string& sql) {
        std::cout << "MySQL: Выполняем запрос: " << sql << std::endl;
    }
    
    void closeConnection() {
        std::cout << "MySQL: Закрываем соединение" << std::endl;
    }
};

// Адаптер для MySQL
class MySQLAdapter : public Database {
private:
    std::unique_ptr<MySQLDriver> driver_;
    
public:
    MySQLAdapter() : driver_(std::make_unique<MySQLDriver>()) {}
    
    void connect() override {
        driver_->openConnection();
    }
    
    void executeQuery(const std::string& query) override {
        driver_->runQuery(query);
    }
    
    void disconnect() override {
        driver_->closeConnection();
    }
};
```

### Адаптация форматов данных
```cpp
// Новый формат данных
struct NewDataFormat {
    std::vector<std::string> fields;
    std::map<std::string, std::string> metadata;
};

// Старый формат данных
struct OldDataFormat {
    std::string data;
    int type;
};

// Старая система
class LegacyProcessor {
public:
    void process(const OldDataFormat& data) {
        std::cout << "Обрабатываем старые данные: " << data.data << std::endl;
    }
};

// Адаптер для новых данных
class DataAdapter {
private:
    LegacyProcessor& processor_;
    
public:
    explicit DataAdapter(LegacyProcessor& processor) : processor_(processor) {}
    
    void process(const NewDataFormat& newData) {
        // Преобразуем новые данные в старый формат
        OldDataFormat oldData;
        oldData.data = joinFields(newData.fields);
        oldData.type = determineType(newData.metadata);
        
        processor_.process(oldData);
    }
    
private:
    std::string joinFields(const std::vector<std::string>& fields) {
        std::string result;
        for (const auto& field : fields) {
            result += field + "|";
        }
        return result;
    }
    
    int determineType(const std::map<std::string, std::string>& metadata) {
        auto it = metadata.find("type");
        return (it != metadata.end()) ? std::stoi(it->second) : 0;
    }
};
```

## 🎨 Современные подходы в C++

### Template Adapter
```cpp
template<typename AdapteeType>
class TemplateAdapter {
private:
    AdapteeType adaptee_;
    
public:
    template<typename... Args>
    explicit TemplateAdapter(Args&&... args) 
        : adaptee_(std::forward<Args>(args)...) {}
    
    template<typename... Args>
    void adaptedMethod(Args&&... args) {
        adaptee_.originalMethod(std::forward<Args>(args)...);
    }
};

// Использование
class LegacyClass {
public:
    void originalMethod(int value) {
        std::cout << "Legacy method with value: " << value << std::endl;
    }
};

// Создание адаптера
TemplateAdapter<LegacyClass> adapter;
adapter.adaptedMethod(42);
```

### Function Adapter
```cpp
class FunctionAdapter {
public:
    // Адаптация функции с одним параметром
    template<typename Func>
    static auto adapt(Func func) {
        return [func](const std::vector<int>& args) {
            if (!args.empty()) {
                func(args[0]);
            }
        };
    }
    
    // Адаптация функции с двумя параметрами
    template<typename Func>
    static auto adaptTwo(Func func) {
        return [func](const std::vector<int>& args) {
            if (args.size() >= 2) {
                func(args[0], args[1]);
            }
        };
    }
};

// Использование
void legacyFunction(int x, int y) {
    std::cout << "Legacy function: " << x << ", " << y << std::endl;
}

auto adapted = FunctionAdapter::adaptTwo(legacyFunction);
adapted({10, 20});
```

### Smart Adapter with Caching
```cpp
class SmartAdapter {
private:
    std::unique_ptr<LegacySystem> legacySystem_;
    std::map<std::string, std::string> cache_;
    
public:
    SmartAdapter() : legacySystem_(std::make_unique<LegacySystem>()) {}
    
    std::string processRequest(const std::string& request) {
        // Проверяем кэш
        auto it = cache_.find(request);
        if (it != cache_.end()) {
            std::cout << "Возвращаем из кэша: " << it->second << std::endl;
            return it->second;
        }
        
        // Обрабатываем запрос
        std::string result = legacySystem_->process(request);
        
        // Сохраняем в кэш
        cache_[request] = result;
        
        return result;
    }
    
    void clearCache() {
        cache_.clear();
    }
};
```

## 🧪 Тестирование Adapter

### Создание моков для тестирования
```cpp
class MockLegacySystem {
public:
    MOCK_METHOD(void, legacyMethod, (const std::string&), ());
};

class Adapter {
private:
    MockLegacySystem& legacySystem_;
    
public:
    explicit Adapter(MockLegacySystem& legacy) : legacySystem_(legacy) {}
    
    void newMethod(const std::vector<int>& data) {
        std::string converted = convertData(data);
        legacySystem_.legacyMethod(converted);
    }
    
private:
    std::string convertData(const std::vector<int>& data) {
        std::string result;
        for (int value : data) {
            result += std::to_string(value) + ",";
        }
        return result;
    }
};

// Тест
TEST(AdapterTest, ConvertAndCall) {
    MockLegacySystem mockLegacy;
    Adapter adapter(mockLegacy);
    
    EXPECT_CALL(mockLegacy, legacyMethod("1,2,3,"))
        .Times(1);
    
    adapter.newMethod({1, 2, 3});
}
```

## 🎯 Практические упражнения

### Упражнение 1: Адаптация API
Создайте адаптер для интеграции с внешним API.

### Упражнение 2: Формат данных
Адаптируйте различные форматы данных для работы с единой системой.

### Упражнение 3: Legacy интеграция
Интегрируйте старую систему с новой архитектурой.

### Упражнение 4: Двунаправленный адаптер
Создайте адаптер, который работает в обе стороны.

## 📈 Связь с другими паттернами

### Adapter vs Decorator
- **Adapter**: Изменяет интерфейс объекта
- **Decorator**: Добавляет функциональность, сохраняя интерфейс

### Adapter vs Facade
- **Adapter**: Адаптирует один интерфейс к другому
- **Facade**: Предоставляет упрощенный интерфейс к подсистеме

### Adapter vs Proxy
- **Adapter**: Изменяет интерфейс
- **Proxy**: Контролирует доступ к объекту

## 📈 Следующие шаги
После изучения Adapter вы будете готовы к:
- Уроку 4.2: Decorator Pattern
- Пониманию структурных паттернов
- Интеграции legacy систем
- Созданию адаптируемых архитектур

## 💡 Важные принципы

1. **Инкапсуляция преобразования**: Скрывайте сложность адаптации
2. **Единственная ответственность**: Адаптер должен делать только адаптацию
3. **Обратная совместимость**: Сохраняйте совместимость с существующим кодом
4. **Производительность**: Учитывайте накладные расходы на преобразование
5. **Тестируемость**: Легко тестируйте адаптер и адаптируемый код
