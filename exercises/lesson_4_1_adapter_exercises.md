# 🏋️ Упражнения: Adapter Pattern

## 📋 Задание 1: Modern Adapter с Templates

### Описание
Создайте современную реализацию Adapter паттерна с использованием C++17:

### Требования
1. ✅ Используйте `std::variant` для разных типов адаптеров
2. ✅ Примените `std::optional` для обработки ошибок
3. ✅ Используйте `auto` и `decltype` для вывода типов
4. ✅ Добавьте поддержку variadic templates
5. ✅ Реализуйте type-safe адаптацию

### Современная архитектура
```cpp
#include <memory>
#include <variant>
#include <optional>
#include <unordered_map>
#include <functional>
#include <string>

// Целевой интерфейс
class Target {
public:
    virtual ~Target() = default;
    virtual std::string request() const = 0;
    virtual void process(const std::string& data) = 0;
};

// Адаптируемый класс
class Adaptee {
public:
    std::string specificRequest() const {
        return "Specific request from Adaptee";
    }
    
    void specificProcess(const std::string& data) {
        std::cout << "Adaptee processing: " << data << std::endl;
    }
    
    std::string getInfo() const {
        return "Adaptee information";
    }
};

// Современный адаптер
class ModernAdapter : public Target {
private:
    std::unique_ptr<Adaptee> adaptee_;
    
public:
    ModernAdapter(std::unique_ptr<Adaptee> adaptee) 
        : adaptee_(std::move(adaptee)) {}
    
    std::string request() const override {
        if (adaptee_) {
            return adaptee_->specificRequest();
        }
        return "No adaptee available";
    }
    
    void process(const std::string& data) override {
        if (adaptee_) {
            adaptee_->specificProcess(data);
        }
    }
    
    // Дополнительные методы
    std::string getAdapteeInfo() const {
        return adaptee_ ? adaptee_->getInfo() : "No adaptee";
    }
    
    bool hasAdaptee() const {
        return adaptee_ != nullptr;
    }
};

// Фабрика адаптеров
class AdapterFactory {
private:
    using AdapterVariant = std::variant<
        std::unique_ptr<ModernAdapter>
    >;
    
    using CreatorFunction = std::function<AdapterVariant()>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerAdapter(const std::string& type) {
        creators_[type] = []() -> AdapterVariant {
            return std::make_unique<ModernAdapter>(std::make_unique<Adaptee>());
        };
    }
    
    std::optional<AdapterVariant> createAdapter(const std::string& type) {
        auto it = creators_.find(type);
        if (it != creators_.end()) {
            return it->second();
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
    AdapterFactory factory;
    
    // Регистрируем адаптер
    factory.registerAdapter("modern");
    
    // Создаем адаптер
    if (auto adapter = factory.createAdapter("modern")) {
        std::visit([](auto& a) {
            std::cout << "Request: " << a->request() << std::endl;
            a->process("Test data");
            std::cout << "Info: " << a->getAdapteeInfo() << std::endl;
            std::cout << "Has adaptee: " << (a->hasAdaptee() ? "Yes" : "No") << std::endl;
        }, *adapter);
    }
    
    return 0;
}
```

---

## 📋 Задание 2: Multi-Adapter System

### Описание
Создайте систему множественных адаптеров для работы с разными типами данных:

### Требования
1. **Multiple Adapters**: Поддержка разных типов адаптеров
2. **Type Safety**: Проверка типов на этапе компиляции
3. **Error Handling**: Обработка ошибок адаптации
4. **Validation**: Валидация данных перед адаптацией

### Реализация
```cpp
#include <any>
#include <type_traits>
#include <stdexcept>

// Базовый интерфейс для данных
class DataInterface {
public:
    virtual ~DataInterface() = default;
    virtual std::string getData() const = 0;
    virtual void setData(const std::string& data) = 0;
    virtual std::string getType() const = 0;
};

// Конкретные типы данных
class XMLData : public DataInterface {
private:
    std::string data_;
    
public:
    XMLData(const std::string& data) : data_(data) {}
    
    std::string getData() const override {
        return data_;
    }
    
    void setData(const std::string& data) override {
        data_ = data;
    }
    
    std::string getType() const override {
        return "XML";
    }
    
    void validateXML() const {
        if (data_.find("<") == std::string::npos) {
            throw std::invalid_argument("Invalid XML data");
        }
    }
};

class JSONData : public DataInterface {
private:
    std::string data_;
    
public:
    JSONData(const std::string& data) : data_(data) {}
    
    std::string getData() const override {
        return data_;
    }
    
    void setData(const std::string& data) override {
        data_ = data;
    }
    
    std::string getType() const override {
        return "JSON";
    }
    
    void validateJSON() const {
        if (data_.find("{") == std::string::npos) {
            throw std::invalid_argument("Invalid JSON data");
        }
    }
};

class CSVData : public DataInterface {
private:
    std::string data_;
    
public:
    CSVData(const std::string& data) : data_(data) {}
    
    std::string getData() const override {
        return data_;
    }
    
    void setData(const std::string& data) override {
        data_ = data;
    }
    
    std::string getType() const override {
        return "CSV";
    }
    
    void validateCSV() const {
        if (data_.find(",") == std::string::npos) {
            throw std::invalid_argument("Invalid CSV data");
        }
    }
};

// Адаптеры для конвертации
class XMLToJSONAdapter : public DataInterface {
private:
    std::unique_ptr<XMLData> xmlData_;
    
public:
    XMLToJSONAdapter(std::unique_ptr<XMLData> xmlData) 
        : xmlData_(std::move(xmlData)) {}
    
    std::string getData() const override {
        if (xmlData_) {
            // Простая конвертация XML в JSON
            std::string xml = xmlData_->getData();
            std::string json = xml;
            
            // Заменяем XML теги на JSON структуру
            size_t pos = 0;
            while ((pos = json.find("<", pos)) != std::string::npos) {
                json.replace(pos, 1, "\"");
                pos += 1;
            }
            
            pos = 0;
            while ((pos = json.find(">", pos)) != std::string::npos) {
                json.replace(pos, 1, "\":");
                pos += 2;
            }
            
            return "{" + json + "}";
        }
        return "{}";
    }
    
    void setData(const std::string& data) override {
        if (xmlData_) {
            xmlData_->setData(data);
        }
    }
    
    std::string getType() const override {
        return "JSON";
    }
    
    void validate() const {
        if (xmlData_) {
            xmlData_->validateXML();
        }
    }
};

class JSONToCSVAdapter : public DataInterface {
private:
    std::unique_ptr<JSONData> jsonData_;
    
public:
    JSONToCSVAdapter(std::unique_ptr<JSONData> jsonData) 
        : jsonData_(std::move(jsonData)) {}
    
    std::string getData() const override {
        if (jsonData_) {
            // Простая конвертация JSON в CSV
            std::string json = jsonData_->getData();
            std::string csv = json;
            
            // Заменяем JSON структуру на CSV
            size_t pos = 0;
            while ((pos = csv.find("\"", pos)) != std::string::npos) {
                csv.replace(pos, 1, "");
                pos += 1;
            }
            
            pos = 0;
            while ((pos = csv.find(":", pos)) != std::string::npos) {
                csv.replace(pos, 1, ",");
                pos += 1;
            }
            
            return csv;
        }
        return "";
    }
    
    void setData(const std::string& data) override {
        if (jsonData_) {
            jsonData_->setData(data);
        }
    }
    
    std::string getType() const override {
        return "CSV";
    }
    
    void validate() const {
        if (jsonData_) {
            jsonData_->validateJSON();
        }
    }
};

// Фабрика адаптеров
class MultiAdapterFactory {
private:
    using AdapterVariant = std::variant<
        std::unique_ptr<XMLToJSONAdapter>,
        std::unique_ptr<JSONToCSVAdapter>
    >;
    
    using CreatorFunction = std::function<AdapterVariant()>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerAdapter(const std::string& type) {
        if (type == "xml_to_json") {
            creators_[type] = []() -> AdapterVariant {
                return std::make_unique<XMLToJSONAdapter>(
                    std::make_unique<XMLData>("<root>data</root>")
                );
            };
        } else if (type == "json_to_csv") {
            creators_[type] = []() -> AdapterVariant {
                return std::make_unique<JSONToCSVAdapter>(
                    std::make_unique<JSONData>("{\"key\":\"value\"}")
                );
            };
        }
    }
    
    std::optional<AdapterVariant> createAdapter(const std::string& type) {
        auto it = creators_.find(type);
        if (it != creators_.end()) {
            return it->second();
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
    MultiAdapterFactory factory;
    
    // Регистрируем адаптеры
    factory.registerAdapter("xml_to_json");
    factory.registerAdapter("json_to_csv");
    
    // Создаем XML to JSON адаптер
    if (auto adapter = factory.createAdapter("xml_to_json")) {
        std::visit([](auto& a) {
            std::cout << "Original type: XML" << std::endl;
            std::cout << "Adapted type: " << a->getType() << std::endl;
            std::cout << "Adapted data: " << a->getData() << std::endl;
            
            try {
                a->validate();
                std::cout << "Validation passed" << std::endl;
            } catch (const std::exception& e) {
                std::cout << "Validation failed: " << e.what() << std::endl;
            }
        }, *adapter);
    }
    
    // Создаем JSON to CSV адаптер
    if (auto adapter = factory.createAdapter("json_to_csv")) {
        std::visit([](auto& a) {
            std::cout << "\nOriginal type: JSON" << std::endl;
            std::cout << "Adapted type: " << a->getType() << std::endl;
            std::cout << "Adapted data: " << a->getData() << std::endl;
            
            try {
                a->validate();
                std::cout << "Validation passed" << std::endl;
            } catch (const std::exception& e) {
                std::cout << "Validation failed: " << e.what() << std::endl;
            }
        }, *adapter);
    }
    
    return 0;
}
```

---

## 📋 Задание 3: Adapter с Configuration

### Описание
Создайте адаптер, который поддерживает конфигурацию и может быть легко настраиваемым:

### Требования
1. **Configuration Support**: Поддержка различных конфигураций
2. **Style Configuration**: Настройка стилей через конфигурацию
3. **Dynamic Loading**: Динамическая загрузка конфигураций
4. **Validation**: Валидация конфигурации

### Реализация
```cpp
#include <any>
#include <type_traits>
#include <stdexcept>

// Конфигурация адаптера
struct AdapterConfig {
    std::string name;
    std::string version;
    std::unordered_map<std::string, std::string> settings;
    std::unordered_map<std::string, std::any> customSettings;
    
    AdapterConfig(const std::string& n, const std::string& v)
        : name(n), version(v) {}
    
    void setSetting(const std::string& key, const std::string& value) {
        settings[key] = value;
    }
    
    std::string getSetting(const std::string& key) const {
        auto it = settings.find(key);
        return it != settings.end() ? it->second : "";
    }
    
    template<typename T>
    void setCustomSetting(const std::string& key, const T& value) {
        customSettings[key] = value;
    }
    
    template<typename T>
    std::optional<T> getCustomSetting(const std::string& key) const {
        auto it = customSettings.find(key);
        if (it != customSettings.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                return std::nullopt;
            }
        }
        return std::nullopt;
    }
};

// Базовый класс с конфигурацией
class ConfigurableAdapter {
protected:
    AdapterConfig config_;
    
public:
    ConfigurableAdapter(const AdapterConfig& config) : config_(config) {}
    
    virtual void validateConfig() const {
        if (config_.name.empty()) {
            throw std::invalid_argument("Adapter name cannot be empty");
        }
        if (config_.version.empty()) {
            throw std::invalid_argument("Adapter version cannot be empty");
        }
    }
    
    std::string getName() const {
        return config_.name;
    }
    
    std::string getVersion() const {
        return config_.version;
    }
    
    std::string getSetting(const std::string& key) const {
        return config_.getSetting(key);
    }
    
    template<typename T>
    std::optional<T> getCustomSetting(const std::string& key) const {
        return config_.getCustomSetting<T>(key);
    }
};

// Конфигурируемые адаптеры
class ConfigurableDataAdapter : public DataInterface, public ConfigurableAdapter {
private:
    std::string sourceType_;
    std::string targetType_;
    std::string data_;
    
public:
    ConfigurableDataAdapter(const AdapterConfig& config) 
        : ConfigurableAdapter(config) {
        validateConfig();
        sourceType_ = config.getSetting("source.type");
        targetType_ = config.getSetting("target.type");
    }
    
    std::string getData() const override {
        return data_;
    }
    
    void setData(const std::string& data) override {
        data_ = data;
    }
    
    std::string getType() const override {
        return targetType_;
    }
    
    void adapt() {
        if (sourceType_ == "XML" && targetType_ == "JSON") {
            // Адаптация XML в JSON
            std::string adapted = data_;
            size_t pos = 0;
            while ((pos = adapted.find("<", pos)) != std::string::npos) {
                adapted.replace(pos, 1, "\"");
                pos += 1;
            }
            
            pos = 0;
            while ((pos = adapted.find(">", pos)) != std::string::npos) {
                adapted.replace(pos, 1, "\":");
                pos += 2;
            }
            
            data_ = "{" + adapted + "}";
        } else if (sourceType_ == "JSON" && targetType_ == "CSV") {
            // Адаптация JSON в CSV
            std::string adapted = data_;
            size_t pos = 0;
            while ((pos = adapted.find("\"", pos)) != std::string::npos) {
                adapted.replace(pos, 1, "");
                pos += 1;
            }
            
            pos = 0;
            while ((pos = adapted.find(":", pos)) != std::string::npos) {
                adapted.replace(pos, 1, ",");
                pos += 1;
            }
            
            data_ = adapted;
        }
    }
    
    void display() const {
        std::cout << "Configurable Data Adapter: " << config_.name << " v" << config_.version << std::endl;
        std::cout << "  Source type: " << sourceType_ << std::endl;
        std::cout << "  Target type: " << targetType_ << std::endl;
        std::cout << "  Data: " << data_ << std::endl;
        std::cout << "  Settings:" << std::endl;
        for (const auto& setting : config_.settings) {
            std::cout << "    " << setting.first << ": " << setting.second << std::endl;
        }
    }
};

// Конфигурируемая фабрика адаптеров
class ConfigurableAdapterFactory {
private:
    AdapterConfig config_;
    std::string adapterType_;
    
public:
    ConfigurableAdapterFactory(const AdapterConfig& config, const std::string& type)
        : config_(config), adapterType_(type) {}
    
    // Методы для настройки конфигурации
    ConfigurableAdapterFactory& setSetting(const std::string& key, const std::string& value) {
        config_.setSetting(key, value);
        return *this;
    }
    
    template<typename T>
    ConfigurableAdapterFactory& setCustomSetting(const std::string& key, const T& value) {
        config_.setCustomSetting(key, value);
        return *this;
    }
    
    // Создание адаптера
    std::unique_ptr<ConfigurableAdapter> build() {
        if (adapterType_ == "data") {
            return std::make_unique<ConfigurableDataAdapter>(config_);
        }
        throw std::invalid_argument("Unknown adapter type: " + adapterType_);
    }
    
    // Создание адаптера с валидацией
    std::unique_ptr<ConfigurableAdapter> buildSafe() {
        try {
            return build();
        } catch (const std::exception& e) {
            std::cerr << "Error building adapter: " << e.what() << std::endl;
            return nullptr;
        }
    }
    
    // Получение текущей конфигурации
    AdapterConfig getConfig() const {
        return config_;
    }
    
    std::string getAdapterType() const {
        return adapterType_;
    }
};

// Менеджер конфигураций
class AdapterConfigManager {
private:
    std::unordered_map<std::string, AdapterConfig> configs_;
    
public:
    void registerConfig(const std::string& name, const AdapterConfig& config) {
        configs_[name] = config;
    }
    
    std::optional<AdapterConfig> getConfig(const std::string& name) const {
        auto it = configs_.find(name);
        if (it != configs_.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    
    std::vector<std::string> getAvailableConfigs() const {
        std::vector<std::string> configNames;
        for (const auto& pair : configs_) {
            configNames.push_back(pair.first);
        }
        return configNames;
    }
    
    std::unique_ptr<ConfigurableAdapterFactory> createFactory(
        const std::string& configName, 
        const std::string& adapterType
    ) {
        auto config = getConfig(configName);
        if (config) {
            return std::make_unique<ConfigurableAdapterFactory>(*config, adapterType);
        }
        return nullptr;
    }
};
```

### Пример использования
```cpp
int main() {
    AdapterConfigManager configManager;
    
    // Регистрируем конфигурации
    AdapterConfig xmlToJsonConfig("XML to JSON", "1.0");
    xmlToJsonConfig.setSetting("source.type", "XML");
    xmlToJsonConfig.setSetting("target.type", "JSON");
    xmlToJsonConfig.setSetting("quality", "High");
    xmlToJsonConfig.setCustomSetting("timeout", 30);
    xmlToJsonConfig.setCustomSetting("retries", 3);
    
    AdapterConfig jsonToCsvConfig("JSON to CSV", "2.0");
    jsonToCsvConfig.setSetting("source.type", "JSON");
    jsonToCsvConfig.setSetting("target.type", "CSV");
    jsonToCsvConfig.setSetting("quality", "Premium");
    jsonToCsvConfig.setCustomSetting("timeout", 60);
    jsonToCsvConfig.setCustomSetting("retries", 5);
    
    configManager.registerConfig("xml_to_json", xmlToJsonConfig);
    configManager.registerConfig("json_to_csv", jsonToCsvConfig);
    
    // Создаем фабрику для XML to JSON адаптера
    auto xmlToJsonFactory = configManager.createFactory("xml_to_json", "data");
    if (xmlToJsonFactory) {
        xmlToJsonFactory->setSetting("source.type", "XML");
        xmlToJsonFactory->setCustomSetting("timeout", 45);
        
        if (auto adapter = xmlToJsonFactory->buildSafe()) {
            auto dataAdapter = static_cast<ConfigurableDataAdapter*>(adapter.get());
            dataAdapter->setData("<root>test data</root>");
            dataAdapter->adapt();
            dataAdapter->display();
        }
    }
    
    // Создаем фабрику для JSON to CSV адаптера
    auto jsonToCsvFactory = configManager.createFactory("json_to_csv", "data");
    if (jsonToCsvFactory) {
        jsonToCsvFactory->setSetting("target.type", "CSV");
        jsonToCsvFactory->setCustomSetting("retries", 7);
        
        if (auto adapter = jsonToCsvFactory->buildSafe()) {
            auto dataAdapter = static_cast<ConfigurableDataAdapter*>(adapter.get());
            dataAdapter->setData("{\"key\":\"value\"}");
            dataAdapter->adapt();
            dataAdapter->display();
        }
    }
    
    // Показываем доступные конфигурации
    auto configs = configManager.getAvailableConfigs();
    std::cout << "\nAvailable configurations: ";
    for (const auto& config : configs) {
        std::cout << config << " ";
    }
    std::cout << std::endl;
    
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

1. **Interface Compatibility**: Убедитесь, что адаптер совместим с целевым интерфейсом
2. **Data Conversion**: Реализуйте правильную конвертацию данных
3. **Error Handling**: Обрабатывайте ошибки адаптации
4. **Configuration**: Используйте конфигурацию для настройки адаптера
5. **Testing**: Создавайте тесты для проверки адаптации

---

*Помните: Adapter обеспечивает совместимость несовместимых интерфейсов!*
