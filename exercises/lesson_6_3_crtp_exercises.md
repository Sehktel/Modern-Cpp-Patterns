# 🏋️ Упражнения: CRTP (Curiously Recurring Template Pattern)

## 📋 Задание 1: Modern CRTP с Templates

### Описание
Создайте современную реализацию CRTP с использованием C++17:

### Требования
1. ✅ Используйте `std::enable_if` для SFINAE
2. ✅ Примените `std::is_base_of_v` для проверки наследования
3. ✅ Используйте `auto` и `decltype` для вывода типов
4. ✅ Добавьте поддержку variadic templates
5. ✅ Реализуйте type-safe CRTP

### Современная архитектура
```cpp
#include <memory>
#include <variant>
#include <optional>
#include <unordered_map>
#include <functional>
#include <string>
#include <vector>
#include <type_traits>

// Базовый CRTP класс
template<typename Derived>
class CRTPBase {
public:
    // Получение ссылки на производный класс
    Derived& derived() {
        return static_cast<Derived&>(*this);
    }
    
    const Derived& derived() const {
        return static_cast<const Derived&>(*this);
    }
    
    // Виртуальные методы для переопределения
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual bool isValid() const = 0;
    
    // Методы, использующие CRTP
    void process() {
        if (isValid()) {
            derived().doProcess();
        }
    }
    
    void validate() {
        derived().doValidate();
    }
    
    // Статический метод для создания объекта
    template<typename... Args>
    static std::unique_ptr<Derived> create(Args&&... args) {
        return std::make_unique<Derived>(std::forward<Args>(args)...);
    }
};

// Конкретные CRTP классы
class DataProcessor : public CRTPBase<DataProcessor> {
private:
    std::string name_;
    std::vector<int> data_;
    
public:
    DataProcessor(const std::string& name, const std::vector<int>& data) 
        : name_(name), data_(data) {
        std::cout << "DataProcessor " << name_ << " created with " << data_.size() << " elements" << std::endl;
    }
    
    std::string getName() const override {
        return name_;
    }
    
    std::string getDescription() const override {
        return "Data processor with " + std::to_string(data_.size()) + " elements";
    }
    
    bool isValid() const override {
        return !data_.empty() && !name_.empty();
    }
    
    void doProcess() {
        std::cout << "DataProcessor " << name_ << " processing data..." << std::endl;
        for (const auto& value : data_) {
            std::cout << "  Processing: " << value << std::endl;
        }
    }
    
    void doValidate() {
        std::cout << "DataProcessor " << name_ << " validating data..." << std::endl;
        if (isValid()) {
            std::cout << "  Validation passed" << std::endl;
        } else {
            std::cout << "  Validation failed" << std::endl;
        }
    }
    
    const std::vector<int>& getData() const {
        return data_;
    }
    
    void addData(int value) {
        data_.push_back(value);
    }
};

class StringProcessor : public CRTPBase<StringProcessor> {
private:
    std::string name_;
    std::string text_;
    
public:
    StringProcessor(const std::string& name, const std::string& text) 
        : name_(name), text_(text) {
        std::cout << "StringProcessor " << name_ << " created with " << text_.size() << " characters" << std::endl;
    }
    
    std::string getName() const override {
        return name_;
    }
    
    std::string getDescription() const override {
        return "String processor with " + std::to_string(text_.size()) + " characters";
    }
    
    bool isValid() const override {
        return !text_.empty() && !name_.empty();
    }
    
    void doProcess() {
        std::cout << "StringProcessor " << name_ << " processing text..." << std::endl;
        std::cout << "  Text: " << text_ << std::endl;
    }
    
    void doValidate() {
        std::cout << "StringProcessor " << name_ << " validating text..." << std::endl;
        if (isValid()) {
            std::cout << "  Validation passed" << std::endl;
        } else {
            std::cout << "  Validation failed" << std::endl;
        }
    }
    
    const std::string& getText() const {
        return text_;
    }
    
    void append(const std::string& text) {
        text_ += text;
    }
};

// Современный менеджер CRTP
class ModernCRTPManager {
private:
    std::unordered_map<std::string, std::unique_ptr<CRTPBase<void>>> processors_;
    
public:
    ModernCRTPManager() = default;
    
    // Добавление процессора с perfect forwarding
    template<typename T, typename... Args>
    void addProcessor(const std::string& name, Args&&... args) {
        static_assert(std::is_base_of_v<CRTPBase<T>, T>, "T must be derived from CRTPBase<T>");
        
        auto processor = std::make_unique<T>(std::forward<Args>(args)...);
        processors_[name] = std::move(processor);
    }
    
    // Получение процессора
    template<typename T>
    T* getProcessor(const std::string& name) const {
        auto it = processors_.find(name);
        if (it != processors_.end()) {
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }
    
    // Обработка всех процессоров
    void processAll() {
        std::cout << "\n=== Processing All Processors ===" << std::endl;
        for (auto& pair : processors_) {
            std::cout << "Processing " << pair.first << "..." << std::endl;
            pair.second->process();
        }
    }
    
    // Валидация всех процессоров
    void validateAll() {
        std::cout << "\n=== Validating All Processors ===" << std::endl;
        for (auto& pair : processors_) {
            std::cout << "Validating " << pair.first << "..." << std::endl;
            pair.second->validate();
        }
    }
    
    // Получение информации о процессорах
    std::string getProcessorsInfo() const {
        std::string info = "Processors:\n";
        for (const auto& pair : processors_) {
            info += "  " + pair.first + ": " + pair.second->getName() + 
                   " - " + pair.second->getDescription() + "\n";
        }
        return info;
    }
    
    // Получение количества процессоров
    size_t getProcessorCount() const {
        return processors_.size();
    }
};

// Фабрика CRTP процессоров
class CRTPProcessorFactory {
private:
    using ProcessorVariant = std::variant<
        std::unique_ptr<DataProcessor>,
        std::unique_ptr<StringProcessor>
    >;
    
    using CreatorFunction = std::function<ProcessorVariant(const std::string&, const std::string&)>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerProcessor(const std::string& type) {
        if (type == "data") {
            creators_[type] = [](const std::string& name, const std::string& param) -> ProcessorVariant {
                std::vector<int> data = {1, 2, 3, 4, 5};
                return std::make_unique<DataProcessor>(name, data);
            };
        } else if (type == "string") {
            creators_[type] = [](const std::string& name, const std::string& param) -> ProcessorVariant {
                return std::make_unique<StringProcessor>(name, param);
            };
        }
    }
    
    std::optional<ProcessorVariant> createProcessor(const std::string& type, const std::string& name, const std::string& param) {
        auto it = creators_.find(type);
        if (it != creators_.end()) {
            return it->second(name, param);
        }
        return std::nullopt;
    }
    
    std::vector<std::string> getAvailableTypes() const {
        std::vector<std::string> types;
        for (const auto& pair : creators_) {
            types.push_back(pair.first);
        }
        return types;
    }
};
```

### Тестовый случай
```cpp
int main() {
    CRTPProcessorFactory factory;
    
    // Регистрируем процессоры
    factory.registerProcessor("data");
    factory.registerProcessor("string");
    
    // Создаем менеджер CRTP
    ModernCRTPManager manager;
    
    // Добавляем процессоры
    manager.addProcessor<DataProcessor>("data1", std::vector<int>{1, 2, 3, 4, 5});
    manager.addProcessor<StringProcessor>("string1", "Hello, World!");
    
    std::cout << "Initial processors:" << std::endl;
    std::cout << manager.getProcessorsInfo() << std::endl;
    
    // Обрабатываем все процессоры
    manager.processAll();
    
    // Валидируем все процессоры
    manager.validateAll();
    
    // Получаем конкретный процессор
    auto dataProcessor = manager.getProcessor<DataProcessor>("data1");
    if (dataProcessor) {
        std::cout << "\nData processor found: " << dataProcessor->getName() << std::endl;
        dataProcessor->addData(6);
        std::cout << "Added data, new size: " << dataProcessor->getData().size() << std::endl;
    }
    
    return 0;
}
```

---

## 📋 Задание 2: Multi-Level CRTP System

### Описание
Создайте систему многоуровневых CRTP для работы с разными типами объектов:

### Требования
1. **Multiple Levels**: Поддержка множественных уровней CRTP
2. **Type Safety**: Проверка типов на этапе компиляции
3. **Error Handling**: Обработка ошибок CRTP
4. **Validation**: Валидация CRTP

### Реализация
```cpp
#include <any>
#include <type_traits>
#include <stdexcept>

// Базовый интерфейс для CRTP объекта
class ICRTPObject {
public:
    virtual ~ICRTPObject() = default;
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual bool isValid() const = 0;
    virtual void execute() = 0;
    virtual void validate() = 0;
};

// Конкретные CRTP объекты
class FileHandler : public ICRTPObject {
private:
    std::string name_;
    std::string filePath_;
    
public:
    FileHandler(const std::string& name, const std::string& filePath) 
        : name_(name), filePath_(filePath) {
        std::cout << "FileHandler " << name_ << " created for " << filePath_ << std::endl;
    }
    
    std::string getName() const override {
        return name_;
    }
    
    std::string getDescription() const override {
        return "File handler for " + filePath_;
    }
    
    bool isValid() const override {
        return !filePath_.empty() && !name_.empty();
    }
    
    void execute() override {
        if (!isValid()) {
            throw std::runtime_error("FileHandler is not valid");
        }
        
        std::cout << "FileHandler " << name_ << " executing on " << filePath_ << std::endl;
    }
    
    void validate() override {
        std::cout << "FileHandler " << name_ << " validating..." << std::endl;
        if (isValid()) {
            std::cout << "  Validation passed" << std::endl;
        } else {
            std::cout << "  Validation failed" << std::endl;
        }
    }
    
    std::string getFilePath() const {
        return filePath_;
    }
};

class DatabaseHandler : public ICRTPObject {
private:
    std::string name_;
    std::string tableName_;
    
public:
    DatabaseHandler(const std::string& name, const std::string& tableName) 
        : name_(name), tableName_(tableName) {
        std::cout << "DatabaseHandler " << name_ << " created for table " << tableName_ << std::endl;
    }
    
    std::string getName() const override {
        return name_;
    }
    
    std::string getDescription() const override {
        return "Database handler for table " + tableName_;
    }
    
    bool isValid() const override {
        return !tableName_.empty() && !name_.empty();
    }
    
    void execute() override {
        if (!isValid()) {
            throw std::runtime_error("DatabaseHandler is not valid");
        }
        
        std::cout << "DatabaseHandler " << name_ << " executing on table " << tableName_ << std::endl;
    }
    
    void validate() override {
        std::cout << "DatabaseHandler " << name_ << " validating..." << std::endl;
        if (isValid()) {
            std::cout << "  Validation passed" << std::endl;
        } else {
            std::cout << "  Validation failed" << std::endl;
        }
    }
    
    std::string getTableName() const {
        return tableName_;
    }
};

class NetworkHandler : public ICRTPObject {
private:
    std::string name_;
    std::string url_;
    
public:
    NetworkHandler(const std::string& name, const std::string& url) 
        : name_(name), url_(url) {
        std::cout << "NetworkHandler " << name_ << " created for " << url_ << std::endl;
    }
    
    std::string getName() const override {
        return name_;
    }
    
    std::string getDescription() const override {
        return "Network handler for " + url_;
    }
    
    bool isValid() const override {
        return !url_.empty() && !name_.empty();
    }
    
    void execute() override {
        if (!isValid()) {
            throw std::runtime_error("NetworkHandler is not valid");
        }
        
        std::cout << "NetworkHandler " << name_ << " executing on " << url_ << std::endl;
    }
    
    void validate() override {
        std::cout << "NetworkHandler " << name_ << " validating..." << std::endl;
        if (isValid()) {
            std::cout << "  Validation passed" << std::endl;
        } else {
            std::cout << "  Validation failed" << std::endl;
        }
    }
    
    std::string getUrl() const {
        return url_;
    }
};

// Многоуровневый менеджер CRTP
class MultiLevelCRTPManager {
private:
    std::vector<std::unique_ptr<ICRTPObject>> handlers_;
    
public:
    MultiLevelCRTPManager() = default;
    
    // Добавление обработчика
    void addHandler(std::unique_ptr<ICRTPObject> handler) {
        if (!handler) {
            std::cerr << "Cannot add null handler" << std::endl;
            return;
        }
        
        if (!handler->isValid()) {
            std::cerr << "Handler is not valid" << std::endl;
            return;
        }
        
        handlers_.push_back(std::move(handler));
    }
    
    // Выполнение обработчика по индексу
    bool executeHandler(size_t index) {
        if (index >= handlers_.size()) {
            std::cerr << "Handler index out of range" << std::endl;
            return false;
        }
        
        try {
            handlers_[index]->execute();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error executing handler: " << e.what() << std::endl;
            return false;
        }
    }
    
    // Выполнение всех обработчиков
    bool executeAll() {
        bool allSuccessful = true;
        
        for (size_t i = 0; i < handlers_.size(); ++i) {
            if (!executeHandler(i)) {
                allSuccessful = false;
            }
        }
        
        return allSuccessful;
    }
    
    // Валидация всех обработчиков
    void validateAll() {
        std::cout << "\n=== Validating All Handlers ===" << std::endl;
        for (size_t i = 0; i < handlers_.size(); ++i) {
            std::cout << "Validating handler " << i << "..." << std::endl;
            handlers_[i]->validate();
        }
    }
    
    // Получение информации об обработчиках
    std::string getHandlersInfo() const {
        std::string info = "Handlers:\n";
        for (size_t i = 0; i < handlers_.size(); ++i) {
            info += "  " + std::to_string(i) + ": " + handlers_[i]->getName() + 
                   " - " + handlers_[i]->getDescription() + "\n";
        }
        return info;
    }
    
    // Получение количества обработчиков
    size_t getHandlerCount() const {
        return handlers_.size();
    }
    
    // Очистка обработчиков
    void clearHandlers() {
        handlers_.clear();
    }
};

// Фабрика CRTP обработчиков
class CRTPHandlerFactory {
private:
    using HandlerVariant = std::variant<
        std::unique_ptr<FileHandler>,
        std::unique_ptr<DatabaseHandler>,
        std::unique_ptr<NetworkHandler>
    >;
    
    using CreatorFunction = std::function<HandlerVariant(const std::string&, const std::string&)>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerHandler(const std::string& type) {
        if (type == "file") {
            creators_[type] = [](const std::string& name, const std::string& param) -> HandlerVariant {
                return std::make_unique<FileHandler>(name, param);
            };
        } else if (type == "database") {
            creators_[type] = [](const std::string& name, const std::string& param) -> HandlerVariant {
                return std::make_unique<DatabaseHandler>(name, param);
            };
        } else if (type == "network") {
            creators_[type] = [](const std::string& name, const std::string& param) -> HandlerVariant {
                return std::make_unique<NetworkHandler>(name, param);
            };
        }
    }
    
    std::optional<HandlerVariant> createHandler(const std::string& type, const std::string& name, const std::string& param) {
        auto it = creators_.find(type);
        if (it != creators_.end()) {
            return it->second(name, param);
        }
        return std::nullopt;
    }
    
    std::vector<std::string> getAvailableTypes() const {
        std::vector<std::string> types;
        for (const auto& pair : creators_) {
            types.push_back(pair.first);
        }
        return types;
    }
};
```

### Пример использования
```cpp
int main() {
    CRTPHandlerFactory factory;
    
    // Регистрируем обработчики
    factory.registerHandler("file");
    factory.registerHandler("database");
    factory.registerHandler("network");
    
    // Создаем многоуровневый менеджер CRTP
    MultiLevelCRTPManager manager;
    
    // Добавляем обработчики
    if (auto handler1 = factory.createHandler("file", "file1", "/tmp/file1.txt")) {
        std::visit([&manager](auto& h) {
            manager.addHandler(std::move(h));
        }, *handler1);
    }
    
    if (auto handler2 = factory.createHandler("database", "db1", "users")) {
        std::visit([&manager](auto& h) {
            manager.addHandler(std::move(h));
        }, *handler2);
    }
    
    if (auto handler3 = factory.createHandler("network", "net1", "https://api.example.com")) {
        std::visit([&manager](auto& h) {
            manager.addHandler(std::move(h));
        }, *handler3);
    }
    
    // Показываем информацию об обработчиках
    std::cout << manager.getHandlersInfo() << std::endl;
    
    // Выполняем обработчики по одному
    std::cout << "=== Executing Handlers One by One ===" << std::endl;
    for (size_t i = 0; i < manager.getHandlerCount(); ++i) {
        std::cout << "Executing handler " << i << "..." << std::endl;
        if (manager.executeHandler(i)) {
            std::cout << "Handler " << i << " executed successfully" << std::endl;
        } else {
            std::cout << "Handler " << i << " execution failed" << std::endl;
        }
    }
    
    // Валидируем все обработчики
    manager.validateAll();
    
    return 0;
}
```

---

## 🎯 Критерии оценки

### Отлично (5 баллов)
- ✅ Все требования выполнены
- ✅ Современные C++17 возможности
- ✅ Правильная архитектура
- ✅ Хорошая валидация
- ✅ Полная документация

### Хорошо (4 балла)
- ✅ Большинство требований выполнено
- ✅ Современные возможности использованы
- ✅ Код работает корректно

### Удовлетворительно (3 балла)
- ✅ Основные требования выполнены
- ✅ Код компилируется и работает
- ❌ Не все возможности использованы

### Неудовлетворительно (2 балла)
- ❌ Код не компилируется или работает неправильно
- ❌ Не выполнены основные требования

---

## 💡 Подсказки

1. **CRTP Pattern**: Используйте CRTP для статического полиморфизма
2. **Type Safety**: Обеспечивайте type safety на этапе компиляции
3. **Error Handling**: Обрабатывайте ошибки CRTP
4. **Validation**: Валидируйте CRTP объекты
5. **Testing**: Создавайте тесты для проверки CRTP

---

*Помните: CRTP обеспечивает статический полиморфизм без накладных расходов на виртуальные функции!*
