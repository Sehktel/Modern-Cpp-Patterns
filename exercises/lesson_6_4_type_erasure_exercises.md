# 🏋️ Упражнения: Type Erasure

## 📋 Задание 1: Modern Type Erasure с Templates

### Описание
Создайте современную реализацию Type Erasure с использованием C++17:

### Требования
1. ✅ Используйте `std::any` для type erasure
2. ✅ Примените `std::variant` для type-safe хранения
3. ✅ Используйте `auto` и `decltype` для вывода типов
4. ✅ Добавьте поддержку variadic templates
5. ✅ Реализуйте type-safe стирание типов

### Современная архитектура
```cpp
#include <memory>
#include <variant>
#include <optional>
#include <unordered_map>
#include <functional>
#include <string>
#include <vector>
#include <any>
#include <type_traits>

// Базовый интерфейс для стирания типов
class ITypeErasure {
public:
    virtual ~ITypeErasure() = default;
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual bool isValid() const = 0;
    virtual void execute() = 0;
    virtual std::any getValue() const = 0;
    virtual void setValue(const std::any& value) = 0;
};

// Конкретные классы для стирания типов
class IntTypeErasure : public ITypeErasure {
private:
    std::string name_;
    int value_;
    
public:
    IntTypeErasure(const std::string& name, int value) : name_(name), value_(value) {
        std::cout << "IntTypeErasure " << name_ << " created with value " << value_ << std::endl;
    }
    
    std::string getName() const override {
        return name_;
    }
    
    std::string getDescription() const override {
        return "Integer type erasure with value " + std::to_string(value_);
    }
    
    bool isValid() const override {
        return !name_.empty();
    }
    
    void execute() override {
        std::cout << "IntTypeErasure " << name_ << " executing with value " << value_ << std::endl;
    }
    
    std::any getValue() const override {
        return value_;
    }
    
    void setValue(const std::any& value) override {
        try {
            value_ = std::any_cast<int>(value);
            std::cout << "IntTypeErasure " << name_ << " value set to " << value_ << std::endl;
        } catch (const std::bad_any_cast&) {
            std::cerr << "Invalid type for IntTypeErasure" << std::endl;
        }
    }
    
    int getIntValue() const {
        return value_;
    }
};

class StringTypeErasure : public ITypeErasure {
private:
    std::string name_;
    std::string value_;
    
public:
    StringTypeErasure(const std::string& name, const std::string& value) : name_(name), value_(value) {
        std::cout << "StringTypeErasure " << name_ << " created with value " << value_ << std::endl;
    }
    
    std::string getName() const override {
        return name_;
    }
    
    std::string getDescription() const override {
        return "String type erasure with value " + value_;
    }
    
    bool isValid() const override {
        return !name_.empty();
    }
    
    void execute() override {
        std::cout << "StringTypeErasure " << name_ << " executing with value " << value_ << std::endl;
    }
    
    std::any getValue() const override {
        return value_;
    }
    
    void setValue(const std::any& value) override {
        try {
            value_ = std::any_cast<std::string>(value);
            std::cout << "StringTypeErasure " << name_ << " value set to " << value_ << std::endl;
        } catch (const std::bad_any_cast&) {
            std::cerr << "Invalid type for StringTypeErasure" << std::endl;
        }
    }
    
    const std::string& getStringValue() const {
        return value_;
    }
};

class DoubleTypeErasure : public ITypeErasure {
private:
    std::string name_;
    double value_;
    
public:
    DoubleTypeErasure(const std::string& name, double value) : name_(name), value_(value) {
        std::cout << "DoubleTypeErasure " << name_ << " created with value " << value_ << std::endl;
    }
    
    std::string getName() const override {
        return name_;
    }
    
    std::string getDescription() const override {
        return "Double type erasure with value " + std::to_string(value_);
    }
    
    bool isValid() const override {
        return !name_.empty();
    }
    
    void execute() override {
        std::cout << "DoubleTypeErasure " << name_ << " executing with value " << value_ << std::endl;
    }
    
    std::any getValue() const override {
        return value_;
    }
    
    void setValue(const std::any& value) override {
        try {
            value_ = std::any_cast<double>(value);
            std::cout << "DoubleTypeErasure " << name_ << " value set to " << value_ << std::endl;
        } catch (const std::bad_any_cast&) {
            std::cerr << "Invalid type for DoubleTypeErasure" << std::endl;
        }
    }
    
    double getDoubleValue() const {
        return value_;
    }
};

// Современный менеджер стирания типов
class ModernTypeErasureManager {
private:
    std::unordered_map<std::string, std::unique_ptr<ITypeErasure>> erasures_;
    
public:
    ModernTypeErasureManager() = default;
    
    // Добавление стирания типа с perfect forwarding
    template<typename T, typename... Args>
    void addTypeErasure(const std::string& name, Args&&... args) {
        static_assert(std::is_base_of_v<ITypeErasure, T>, "T must be derived from ITypeErasure");
        
        auto erasure = std::make_unique<T>(std::forward<Args>(args)...);
        erasures_[name] = std::move(erasure);
    }
    
    // Получение стирания типа
    ITypeErasure* getTypeErasure(const std::string& name) const {
        auto it = erasures_.find(name);
        return it != erasures_.end() ? it->second.get() : nullptr;
    }
    
    // Выполнение стирания типа
    bool executeTypeErasure(const std::string& name) {
        auto erasure = getTypeErasure(name);
        if (erasure) {
            erasure->execute();
            return true;
        }
        return false;
    }
    
    // Выполнение всех стираний типов
    void executeAll() {
        std::cout << "\n=== Executing All Type Erasures ===" << std::endl;
        for (auto& pair : erasures_) {
            std::cout << "Executing " << pair.first << "..." << std::endl;
            pair.second->execute();
        }
    }
    
    // Получение значения стирания типа
    std::optional<std::any> getValue(const std::string& name) const {
        auto erasure = getTypeErasure(name);
        if (erasure) {
            return erasure->getValue();
        }
        return std::nullopt;
    }
    
    // Установка значения стирания типа
    bool setValue(const std::string& name, const std::any& value) {
        auto erasure = getTypeErasure(name);
        if (erasure) {
            erasure->setValue(value);
            return true;
        }
        return false;
    }
    
    // Получение информации о стираниях типов
    std::string getTypeErasuresInfo() const {
        std::string info = "Type Erasures:\n";
        for (const auto& pair : erasures_) {
            info += "  " + pair.first + ": " + pair.second->getName() + 
                   " - " + pair.second->getDescription() + "\n";
        }
        return info;
    }
    
    // Получение количества стираний типов
    size_t getTypeErasureCount() const {
        return erasures_.size();
    }
};

// Фабрика стираний типов
class TypeErasureFactory {
private:
    using TypeErasureVariant = std::variant<
        std::unique_ptr<IntTypeErasure>,
        std::unique_ptr<StringTypeErasure>,
        std::unique_ptr<DoubleTypeErasure>
    >;
    
    using CreatorFunction = std::function<TypeErasureVariant(const std::string&, const std::any&)>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerTypeErasure(const std::string& type) {
        if (type == "int") {
            creators_[type] = [](const std::string& name, const std::any& value) -> TypeErasureVariant {
                return std::make_unique<IntTypeErasure>(name, std::any_cast<int>(value));
            };
        } else if (type == "string") {
            creators_[type] = [](const std::string& name, const std::any& value) -> TypeErasureVariant {
                return std::make_unique<StringTypeErasure>(name, std::any_cast<std::string>(value));
            };
        } else if (type == "double") {
            creators_[type] = [](const std::string& name, const std::any& value) -> TypeErasureVariant {
                return std::make_unique<DoubleTypeErasure>(name, std::any_cast<double>(value));
            };
        }
    }
    
    std::optional<TypeErasureVariant> createTypeErasure(const std::string& type, const std::string& name, const std::any& value) {
        auto it = creators_.find(type);
        if (it != creators_.end()) {
            return it->second(name, value);
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
    TypeErasureFactory factory;
    
    // Регистрируем стирания типов
    factory.registerTypeErasure("int");
    factory.registerTypeErasure("string");
    factory.registerTypeErasure("double");
    
    // Создаем менеджер стирания типов
    ModernTypeErasureManager manager;
    
    // Добавляем стирания типов
    manager.addTypeErasure<IntTypeErasure>("int1", 42);
    manager.addTypeErasure<StringTypeErasure>("string1", "Hello, World!");
    manager.addTypeErasure<DoubleTypeErasure>("double1", 3.14159);
    
    std::cout << "Initial type erasures:" << std::endl;
    std::cout << manager.getTypeErasuresInfo() << std::endl;
    
    // Выполняем все стирания типов
    manager.executeAll();
    
    // Получаем значения
    std::cout << "\n=== Getting Values ===" << std::endl;
    if (auto value = manager.getValue("int1")) {
        std::cout << "int1 value: " << std::any_cast<int>(*value) << std::endl;
    }
    
    if (auto value = manager.getValue("string1")) {
        std::cout << "string1 value: " << std::any_cast<std::string>(*value) << std::endl;
    }
    
    if (auto value = manager.getValue("double1")) {
        std::cout << "double1 value: " << std::any_cast<double>(*value) << std::endl;
    }
    
    // Устанавливаем новые значения
    std::cout << "\n=== Setting New Values ===" << std::endl;
    manager.setValue("int1", std::any(100));
    manager.setValue("string1", std::any(std::string("New String")));
    manager.setValue("double1", std::any(2.71828));
    
    return 0;
}
```

---

## 📋 Задание 2: Multi-Level Type Erasure System

### Описание
Создайте систему многоуровневых Type Erasure для работы с разными типами объектов:

### Требования
1. **Multiple Levels**: Поддержка множественных уровней стирания типов
2. **Type Safety**: Проверка типов на этапе компиляции
3. **Error Handling**: Обработка ошибок стирания типов
4. **Validation**: Валидация стирания типов

### Реализация
```cpp
#include <any>
#include <type_traits>
#include <stdexcept>

// Базовый интерфейс для многоуровневого стирания типов
class IMultiLevelTypeErasure {
public:
    virtual ~IMultiLevelTypeErasure() = default;
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual bool isValid() const = 0;
    virtual void process() = 0;
    virtual std::any getData() const = 0;
    virtual void setData(const std::any& data) = 0;
    virtual std::string getTypeName() const = 0;
};

// Конкретные классы для многоуровневого стирания типов
class FileTypeErasure : public IMultiLevelTypeErasure {
private:
    std::string name_;
    std::string filePath_;
    std::any data_;
    
public:
    FileTypeErasure(const std::string& name, const std::string& filePath) 
        : name_(name), filePath_(filePath) {
        std::cout << "FileTypeErasure " << name_ << " created for " << filePath_ << std::endl;
    }
    
    std::string getName() const override {
        return name_;
    }
    
    std::string getDescription() const override {
        return "File type erasure for " + filePath_;
    }
    
    bool isValid() const override {
        return !filePath_.empty() && !name_.empty();
    }
    
    void process() override {
        if (!isValid()) {
            throw std::runtime_error("FileTypeErasure is not valid");
        }
        
        std::cout << "FileTypeErasure " << name_ << " processing file " << filePath_ << std::endl;
    }
    
    std::any getData() const override {
        return data_;
    }
    
    void setData(const std::any& data) override {
        data_ = data;
        std::cout << "FileTypeErasure " << name_ << " data set" << std::endl;
    }
    
    std::string getTypeName() const override {
        return "file";
    }
    
    std::string getFilePath() const {
        return filePath_;
    }
};

class DatabaseTypeErasure : public IMultiLevelTypeErasure {
private:
    std::string name_;
    std::string tableName_;
    std::any data_;
    
public:
    DatabaseTypeErasure(const std::string& name, const std::string& tableName) 
        : name_(name), tableName_(tableName) {
        std::cout << "DatabaseTypeErasure " << name_ << " created for table " << tableName_ << std::endl;
    }
    
    std::string getName() const override {
        return name_;
    }
    
    std::string getDescription() const override {
        return "Database type erasure for table " + tableName_;
    }
    
    bool isValid() const override {
        return !tableName_.empty() && !name_.empty();
    }
    
    void process() override {
        if (!isValid()) {
            throw std::runtime_error("DatabaseTypeErasure is not valid");
        }
        
        std::cout << "DatabaseTypeErasure " << name_ << " processing table " << tableName_ << std::endl;
    }
    
    std::any getData() const override {
        return data_;
    }
    
    void setData(const std::any& data) override {
        data_ = data;
        std::cout << "DatabaseTypeErasure " << name_ << " data set" << std::endl;
    }
    
    std::string getTypeName() const override {
        return "database";
    }
    
    std::string getTableName() const {
        return tableName_;
    }
};

class NetworkTypeErasure : public IMultiLevelTypeErasure {
private:
    std::string name_;
    std::string url_;
    std::any data_;
    
public:
    NetworkTypeErasure(const std::string& name, const std::string& url) 
        : name_(name), url_(url) {
        std::cout << "NetworkTypeErasure " << name_ << " created for " << url_ << std::endl;
    }
    
    std::string getName() const override {
        return name_;
    }
    
    std::string getDescription() const override {
        return "Network type erasure for " + url_;
    }
    
    bool isValid() const override {
        return !url_.empty() && !name_.empty();
    }
    
    void process() override {
        if (!isValid()) {
            throw std::runtime_error("NetworkTypeErasure is not valid");
        }
        
        std::cout << "NetworkTypeErasure " << name_ << " processing " << url_ << std::endl;
    }
    
    std::any getData() const override {
        return data_;
    }
    
    void setData(const std::any& data) override {
        data_ = data;
        std::cout << "NetworkTypeErasure " << name_ << " data set" << std::endl;
    }
    
    std::string getTypeName() const override {
        return "network";
    }
    
    std::string getUrl() const {
        return url_;
    }
};

// Многоуровневый менеджер стирания типов
class MultiLevelTypeErasureManager {
private:
    std::vector<std::unique_ptr<IMultiLevelTypeErasure>> erasures_;
    
public:
    MultiLevelTypeErasureManager() = default;
    
    // Добавление стирания типа
    void addTypeErasure(std::unique_ptr<IMultiLevelTypeErasure> erasure) {
        if (!erasure) {
            std::cerr << "Cannot add null type erasure" << std::endl;
            return;
        }
        
        if (!erasure->isValid()) {
            std::cerr << "Type erasure is not valid" << std::endl;
            return;
        }
        
        erasures_.push_back(std::move(erasure));
    }
    
    // Обработка стирания типа по индексу
    bool processTypeErasure(size_t index) {
        if (index >= erasures_.size()) {
            std::cerr << "Type erasure index out of range" << std::endl;
            return false;
        }
        
        try {
            erasures_[index]->process();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error processing type erasure: " << e.what() << std::endl;
            return false;
        }
    }
    
    // Обработка всех стираний типов
    bool processAll() {
        bool allSuccessful = true;
        
        for (size_t i = 0; i < erasures_.size(); ++i) {
            if (!processTypeErasure(i)) {
                allSuccessful = false;
            }
        }
        
        return allSuccessful;
    }
    
    // Получение данных стирания типа
    std::optional<std::any> getData(size_t index) const {
        if (index >= erasures_.size()) {
            return std::nullopt;
        }
        
        return erasures_[index]->getData();
    }
    
    // Установка данных стирания типа
    bool setData(size_t index, const std::any& data) {
        if (index >= erasures_.size()) {
            return false;
        }
        
        erasures_[index]->setData(data);
        return true;
    }
    
    // Получение информации о стираниях типов
    std::string getTypeErasuresInfo() const {
        std::string info = "Type Erasures:\n";
        for (size_t i = 0; i < erasures_.size(); ++i) {
            info += "  " + std::to_string(i) + ": " + erasures_[i]->getName() + 
                   " (" + erasures_[i]->getTypeName() + ") - " + erasures_[i]->getDescription() + "\n";
        }
        return info;
    }
    
    // Получение количества стираний типов
    size_t getTypeErasureCount() const {
        return erasures_.size();
    }
    
    // Очистка стираний типов
    void clearTypeErasures() {
        erasures_.clear();
    }
};

// Фабрика многоуровневых стираний типов
class MultiLevelTypeErasureFactory {
private:
    using TypeErasureVariant = std::variant<
        std::unique_ptr<FileTypeErasure>,
        std::unique_ptr<DatabaseTypeErasure>,
        std::unique_ptr<NetworkTypeErasure>
    >;
    
    using CreatorFunction = std::function<TypeErasureVariant(const std::string&, const std::string&)>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerTypeErasure(const std::string& type) {
        if (type == "file") {
            creators_[type] = [](const std::string& name, const std::string& param) -> TypeErasureVariant {
                return std::make_unique<FileTypeErasure>(name, param);
            };
        } else if (type == "database") {
            creators_[type] = [](const std::string& name, const std::string& param) -> TypeErasureVariant {
                return std::make_unique<DatabaseTypeErasure>(name, param);
            };
        } else if (type == "network") {
            creators_[type] = [](const std::string& name, const std::string& param) -> TypeErasureVariant {
                return std::make_unique<NetworkTypeErasure>(name, param);
            };
        }
    }
    
    std::optional<TypeErasureVariant> createTypeErasure(const std::string& type, const std::string& name, const std::string& param) {
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
    MultiLevelTypeErasureFactory factory;
    
    // Регистрируем стирания типов
    factory.registerTypeErasure("file");
    factory.registerTypeErasure("database");
    factory.registerTypeErasure("network");
    
    // Создаем многоуровневый менеджер стирания типов
    MultiLevelTypeErasureManager manager;
    
    // Добавляем стирания типов
    if (auto erasure1 = factory.createTypeErasure("file", "file1", "/tmp/file1.txt")) {
        std::visit([&manager](auto& e) {
            manager.addTypeErasure(std::move(e));
        }, *erasure1);
    }
    
    if (auto erasure2 = factory.createTypeErasure("database", "db1", "users")) {
        std::visit([&manager](auto& e) {
            manager.addTypeErasure(std::move(e));
        }, *erasure2);
    }
    
    if (auto erasure3 = factory.createTypeErasure("network", "net1", "https://api.example.com")) {
        std::visit([&manager](auto& e) {
            manager.addTypeErasure(std::move(e));
        }, *erasure3);
    }
    
    // Показываем информацию о стираниях типов
    std::cout << manager.getTypeErasuresInfo() << std::endl;
    
    // Обрабатываем стирания типов по одному
    std::cout << "=== Processing Type Erasures One by One ===" << std::endl;
    for (size_t i = 0; i < manager.getTypeErasureCount(); ++i) {
        std::cout << "Processing type erasure " << i << "..." << std::endl;
        if (manager.processTypeErasure(i)) {
            std::cout << "Type erasure " << i << " processed successfully" << std::endl;
        } else {
            std::cout << "Type erasure " << i << " processing failed" << std::endl;
        }
    }
    
    // Устанавливаем данные
    std::cout << "\n=== Setting Data ===" << std::endl;
    manager.setData(0, std::any(std::string("File data")));
    manager.setData(1, std::any(std::vector<int>{1, 2, 3}));
    manager.setData(2, std::any(std::string("Network data")));
    
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

1. **Type Erasure**: Используйте type erasure для скрытия конкретных типов
2. **std::any**: Применяйте std::any для хранения любых типов
3. **Error Handling**: Обрабатывайте ошибки стирания типов
4. **Validation**: Валидируйте стирания типов
5. **Testing**: Создавайте тесты для проверки стирания типов

---

*Помните: Type Erasure позволяет скрыть конкретные типы за единым интерфейсом!*
