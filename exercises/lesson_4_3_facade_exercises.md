# 🏋️ Упражнения: Facade Pattern

## 📋 Задание 1: Modern Facade с Templates

### Описание
Создайте современную реализацию Facade паттерна с использованием C++17:

### Требования
1. ✅ Используйте `std::variant` для разных типов подсистем
2. ✅ Примените `std::optional` для обработки ошибок
3. ✅ Используйте `auto` и `decltype` для вывода типов
4. ✅ Добавьте поддержку variadic templates
5. ✅ Реализуйте type-safe упрощение интерфейса

### Современная архитектура
```cpp
#include <memory>
#include <variant>
#include <optional>
#include <unordered_map>
#include <functional>
#include <string>

// Подсистемы
class SubsystemA {
public:
    std::string operationA1() const {
        return "SubsystemA: Operation A1";
    }
    
    std::string operationA2() const {
        return "SubsystemA: Operation A2";
    }
    
    void complexOperationA() {
        std::cout << "SubsystemA: Performing complex operation A" << std::endl;
    }
};

class SubsystemB {
public:
    std::string operationB1() const {
        return "SubsystemB: Operation B1";
    }
    
    std::string operationB2() const {
        return "SubsystemB: Operation B2";
    }
    
    void complexOperationB() {
        std::cout << "SubsystemB: Performing complex operation B" << std::endl;
    }
};

class SubsystemC {
public:
    std::string operationC1() const {
        return "SubsystemC: Operation C1";
    }
    
    std::string operationC2() const {
        return "SubsystemC: Operation C2";
    }
    
    void complexOperationC() {
        std::cout << "SubsystemC: Performing complex operation C" << std::endl;
    }
};

// Современный фасад
class ModernFacade {
private:
    std::unique_ptr<SubsystemA> subsystemA_;
    std::unique_ptr<SubsystemB> subsystemB_;
    std::unique_ptr<SubsystemC> subsystemC_;
    
public:
    ModernFacade() 
        : subsystemA_(std::make_unique<SubsystemA>()),
          subsystemB_(std::make_unique<SubsystemB>()),
          subsystemC_(std::make_unique<SubsystemC>()) {}
    
    // Упрощенные операции
    std::string simpleOperation() const {
        return subsystemA_->operationA1() + "\n" +
               subsystemB_->operationB1() + "\n" +
               subsystemC_->operationC1();
    }
    
    void complexOperation() {
        std::cout << "Facade: Starting complex operation" << std::endl;
        subsystemA_->complexOperationA();
        subsystemB_->complexOperationB();
        subsystemC_->complexOperationC();
        std::cout << "Facade: Complex operation completed" << std::endl;
    }
    
    // Операции с параметрами
    std::string operationWithParams(const std::string& param) const {
        return subsystemA_->operationA1() + " with " + param + "\n" +
               subsystemB_->operationB2() + " with " + param + "\n" +
               subsystemC_->operationC2() + " with " + param;
    }
    
    // Проверка состояния подсистем
    bool areSubsystemsReady() const {
        return subsystemA_ && subsystemB_ && subsystemC_;
    }
    
    // Получение информации о подсистемах
    std::string getSubsystemInfo() const {
        return "SubsystemA: " + subsystemA_->operationA1() + "\n" +
               "SubsystemB: " + subsystemB_->operationB1() + "\n" +
               "SubsystemC: " + subsystemC_->operationC1();
    }
};

// Фабрика фасадов
class FacadeFactory {
private:
    using FacadeVariant = std::variant<
        std::unique_ptr<ModernFacade>
    >;
    
    using CreatorFunction = std::function<FacadeVariant()>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerFacade(const std::string& type) {
        creators_[type] = []() -> FacadeVariant {
            return std::make_unique<ModernFacade>();
        };
    }
    
    std::optional<FacadeVariant> createFacade(const std::string& type) {
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
    FacadeFactory factory;
    
    // Регистрируем фасад
    factory.registerFacade("modern");
    
    // Создаем фасад
    if (auto facade = factory.createFacade("modern")) {
        std::visit([](auto& f) {
            std::cout << "=== Simple Operation ===" << std::endl;
            std::cout << f->simpleOperation() << std::endl;
            
            std::cout << "\n=== Complex Operation ===" << std::endl;
            f->complexOperation();
            
            std::cout << "\n=== Operation with Parameters ===" << std::endl;
            std::cout << f->operationWithParams("test parameter") << std::endl;
            
            std::cout << "\n=== Subsystem Info ===" << std::endl;
            std::cout << f->getSubsystemInfo() << std::endl;
            
            std::cout << "\nSubsystems ready: " << (f->areSubsystemsReady() ? "Yes" : "No") << std::endl;
        }, *facade);
    }
    
    return 0;
}
```

---

## 📋 Задание 2: Multi-Level Facade System

### Описание
Создайте систему многоуровневых фасадов для работы с разными типами подсистем:

### Требования
1. **Multiple Levels**: Поддержка множественных уровней фасадов
2. **Type Safety**: Проверка типов на этапе компиляции
3. **Error Handling**: Обработка ошибок фасада
4. **Validation**: Валидация подсистем

### Реализация
```cpp
#include <any>
#include <type_traits>
#include <stdexcept>

// Базовый интерфейс для подсистем
class ISubsystem {
public:
    virtual ~ISubsystem() = default;
    virtual std::string getName() const = 0;
    virtual bool isReady() const = 0;
    virtual void initialize() = 0;
    virtual void shutdown() = 0;
};

// Конкретные подсистемы
class DatabaseSubsystem : public ISubsystem {
private:
    bool initialized_;
    
public:
    DatabaseSubsystem() : initialized_(false) {}
    
    std::string getName() const override {
        return "Database";
    }
    
    bool isReady() const override {
        return initialized_;
    }
    
    void initialize() override {
        std::cout << "Database subsystem initializing..." << std::endl;
        initialized_ = true;
        std::cout << "Database subsystem ready" << std::endl;
    }
    
    void shutdown() override {
        std::cout << "Database subsystem shutting down..." << std::endl;
        initialized_ = false;
        std::cout << "Database subsystem stopped" << std::endl;
    }
    
    std::string query(const std::string& sql) {
        if (!isReady()) {
            throw std::runtime_error("Database not ready");
        }
        return "Query result: " + sql;
    }
};

class CacheSubsystem : public ISubsystem {
private:
    bool initialized_;
    std::unordered_map<std::string, std::string> cache_;
    
public:
    CacheSubsystem() : initialized_(false) {}
    
    std::string getName() const override {
        return "Cache";
    }
    
    bool isReady() const override {
        return initialized_;
    }
    
    void initialize() override {
        std::cout << "Cache subsystem initializing..." << std::endl;
        initialized_ = true;
        std::cout << "Cache subsystem ready" << std::endl;
    }
    
    void shutdown() override {
        std::cout << "Cache subsystem shutting down..." << std::endl;
        cache_.clear();
        initialized_ = false;
        std::cout << "Cache subsystem stopped" << std::endl;
    }
    
    void put(const std::string& key, const std::string& value) {
        if (!isReady()) {
            throw std::runtime_error("Cache not ready");
        }
        cache_[key] = value;
    }
    
    std::optional<std::string> get(const std::string& key) const {
        if (!isReady()) {
            throw std::runtime_error("Cache not ready");
        }
        auto it = cache_.find(key);
        return it != cache_.end() ? std::optional<std::string>(it->second) : std::nullopt;
    }
};

class LoggingSubsystem : public ISubsystem {
private:
    bool initialized_;
    
public:
    LoggingSubsystem() : initialized_(false) {}
    
    std::string getName() const override {
        return "Logging";
    }
    
    bool isReady() const override {
        return initialized_;
    }
    
    void initialize() override {
        std::cout << "Logging subsystem initializing..." << std::endl;
        initialized_ = true;
        std::cout << "Logging subsystem ready" << std::endl;
    }
    
    void shutdown() override {
        std::cout << "Logging subsystem shutting down..." << std::endl;
        initialized_ = false;
        std::cout << "Logging subsystem stopped" << std::endl;
    }
    
    void log(const std::string& message) {
        if (!isReady()) {
            throw std::runtime_error("Logging not ready");
        }
        std::cout << "[LOG] " << message << std::endl;
    }
};

// Многоуровневый фасад
class MultiLevelFacade {
private:
    std::unique_ptr<DatabaseSubsystem> database_;
    std::unique_ptr<CacheSubsystem> cache_;
    std::unique_ptr<LoggingSubsystem> logging_;
    
public:
    MultiLevelFacade() 
        : database_(std::make_unique<DatabaseSubsystem>()),
          cache_(std::make_unique<CacheSubsystem>()),
          logging_(std::make_unique<LoggingSubsystem>()) {}
    
    // Инициализация всех подсистем
    void initializeAll() {
        std::cout << "=== Initializing All Subsystems ===" << std::endl;
        database_->initialize();
        cache_->initialize();
        logging_->initialize();
        std::cout << "=== All Subsystems Initialized ===" << std::endl;
    }
    
    // Завершение работы всех подсистем
    void shutdownAll() {
        std::cout << "=== Shutting Down All Subsystems ===" << std::endl;
        logging_->shutdown();
        cache_->shutdown();
        database_->shutdown();
        std::cout << "=== All Subsystems Shutdown ===" << std::endl;
    }
    
    // Проверка готовности всех подсистем
    bool areAllReady() const {
        return database_->isReady() && cache_->isReady() && logging_->isReady();
    }
    
    // Упрощенная операция с использованием всех подсистем
    std::string processRequest(const std::string& request) {
        if (!areAllReady()) {
            throw std::runtime_error("Not all subsystems are ready");
        }
        
        logging_->log("Processing request: " + request);
        
        // Проверяем кэш
        if (auto cached = cache_->get(request)) {
            logging_->log("Request found in cache");
            return *cached;
        }
        
        // Выполняем запрос к базе данных
        std::string result = database_->query("SELECT * FROM data WHERE key = '" + request + "'");
        
        // Сохраняем в кэш
        cache_->put(request, result);
        
        logging_->log("Request processed successfully");
        return result;
    }
    
    // Получение информации о подсистемах
    std::string getSubsystemStatus() const {
        std::string status = "Subsystem Status:\n";
        status += "  " + database_->getName() + ": " + (database_->isReady() ? "Ready" : "Not Ready") + "\n";
        status += "  " + cache_->getName() + ": " + (cache_->isReady() ? "Ready" : "Not Ready") + "\n";
        status += "  " + logging_->getName() + ": " + (logging_->isReady() ? "Ready" : "Not Ready");
        return status;
    }
    
    // Операции с отдельными подсистемами
    void logMessage(const std::string& message) {
        if (logging_->isReady()) {
            logging_->log(message);
        }
    }
    
    void cacheData(const std::string& key, const std::string& value) {
        if (cache_->isReady()) {
            cache_->put(key, value);
        }
    }
    
    std::optional<std::string> getCachedData(const std::string& key) {
        if (cache_->isReady()) {
            return cache_->get(key);
        }
        return std::nullopt;
    }
};

// Фабрика многоуровневых фасадов
class MultiLevelFacadeFactory {
private:
    using FacadeVariant = std::variant<
        std::unique_ptr<MultiLevelFacade>
    >;
    
    using CreatorFunction = std::function<FacadeVariant()>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerFacade(const std::string& type) {
        creators_[type] = []() -> FacadeVariant {
            return std::make_unique<MultiLevelFacade>();
        };
    }
    
    std::optional<FacadeVariant> createFacade(const std::string& type) {
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
    MultiLevelFacadeFactory factory;
    
    // Регистрируем фасад
    factory.registerFacade("multilevel");
    
    // Создаем фасад
    if (auto facade = factory.createFacade("multilevel")) {
        std::visit([](auto& f) {
            // Инициализируем все подсистемы
            f->initializeAll();
            
            std::cout << "\n" << f->getSubsystemStatus() << std::endl;
            
            // Проверяем готовность
            std::cout << "\nAll subsystems ready: " << (f->areAllReady() ? "Yes" : "No") << std::endl;
            
            // Используем упрощенную операцию
            try {
                std::cout << "\n=== Processing Request ===" << std::endl;
                std::string result = f->processRequest("user123");
                std::cout << "Result: " << result << std::endl;
                
                // Повторный запрос (должен быть из кэша)
                std::cout << "\n=== Processing Same Request (Should Use Cache) ===" << std::endl;
                result = f->processRequest("user123");
                std::cout << "Result: " << result << std::endl;
                
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << std::endl;
            }
            
            // Используем отдельные подсистемы
            std::cout << "\n=== Using Individual Subsystems ===" << std::endl;
            f->logMessage("Custom log message");
            f->cacheData("custom_key", "custom_value");
            
            if (auto cached = f->getCachedData("custom_key")) {
                std::cout << "Cached data: " << *cached << std::endl;
            }
            
            // Завершаем работу
            f->shutdownAll();
        }, *facade);
    }
    
    return 0;
}
```

---

## 📋 Задание 3: Facade с Configuration

### Описание
Создайте фасад, который поддерживает конфигурацию и может быть легко настраиваемым:

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

// Конфигурация фасада
struct FacadeConfig {
    std::string name;
    std::string version;
    std::unordered_map<std::string, std::string> settings;
    std::unordered_map<std::string, std::any> customSettings;
    
    FacadeConfig(const std::string& n, const std::string& v)
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
class ConfigurableFacade {
protected:
    FacadeConfig config_;
    
public:
    ConfigurableFacade(const FacadeConfig& config) : config_(config) {}
    
    virtual void validateConfig() const {
        if (config_.name.empty()) {
            throw std::invalid_argument("Facade name cannot be empty");
        }
        if (config_.version.empty()) {
            throw std::invalid_argument("Facade version cannot be empty");
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

// Конфигурируемый фасад
class ConfigurableSystemFacade : public ConfigurableFacade {
private:
    std::unique_ptr<DatabaseSubsystem> database_;
    std::unique_ptr<CacheSubsystem> cache_;
    std::unique_ptr<LoggingSubsystem> logging_;
    
public:
    ConfigurableSystemFacade(const FacadeConfig& config) 
        : ConfigurableFacade(config) {
        validateConfig();
        
        // Инициализируем подсистемы на основе конфигурации
        database_ = std::make_unique<DatabaseSubsystem>();
        cache_ = std::make_unique<CacheSubsystem>();
        logging_ = std::make_unique<LoggingSubsystem>();
    }
    
    // Инициализация с конфигурацией
    void initializeWithConfig() {
        std::cout << "Initializing " << config_.name << " v" << config_.version << std::endl;
        
        // Настраиваем подсистемы на основе конфигурации
        if (auto dbConfig = config_.getCustomSetting<std::string>("database.config")) {
            std::cout << "Database config: " << *dbConfig << std::endl;
        }
        
        if (auto cacheConfig = config_.getCustomSetting<std::string>("cache.config")) {
            std::cout << "Cache config: " << *cacheConfig << std::endl;
        }
        
        if (auto logConfig = config_.getCustomSetting<std::string>("logging.config")) {
            std::cout << "Logging config: " << *logConfig << std::endl;
        }
        
        database_->initialize();
        cache_->initialize();
        logging_->initialize();
        
        std::cout << "All subsystems initialized with configuration" << std::endl;
    }
    
    // Упрощенная операция с конфигурацией
    std::string processWithConfig(const std::string& request) {
        if (!areAllReady()) {
            throw std::runtime_error("Not all subsystems are ready");
        }
        
        logging_->log("Processing request with config: " + request);
        
        // Используем настройки из конфигурации
        std::string cacheKey = request;
        if (auto prefix = config_.getSetting("cache.prefix")) {
            cacheKey = prefix + "_" + request;
        }
        
        // Проверяем кэш
        if (auto cached = cache_->get(cacheKey)) {
            logging_->log("Request found in cache with key: " + cacheKey);
            return *cached;
        }
        
        // Выполняем запрос к базе данных
        std::string query = "SELECT * FROM data WHERE key = '" + request + "'";
        if (auto table = config_.getSetting("database.table")) {
            query = "SELECT * FROM " + table + " WHERE key = '" + request + "'";
        }
        
        std::string result = database_->query(query);
        
        // Сохраняем в кэш
        cache_->put(cacheKey, result);
        
        logging_->log("Request processed successfully with config");
        return result;
    }
    
    // Проверка готовности всех подсистем
    bool areAllReady() const {
        return database_->isReady() && cache_->isReady() && logging_->isReady();
    }
    
    // Получение информации о конфигурации
    std::string getConfigInfo() const {
        std::string info = "Facade Configuration:\n";
        info += "  Name: " + config_.name + "\n";
        info += "  Version: " + config_.version + "\n";
        info += "  Settings:\n";
        for (const auto& setting : config_.settings) {
            info += "    " + setting.first + ": " + setting.second + "\n";
        }
        return info;
    }
    
    void display() const {
        std::cout << "Configurable System Facade: " << config_.name << " v" << config_.version << std::endl;
        std::cout << "  Settings:" << std::endl;
        for (const auto& setting : config_.settings) {
            std::cout << "    " << setting.first << ": " << setting.second << std::endl;
        }
    }
};

// Конфигурируемая фабрика фасадов
class ConfigurableFacadeFactory {
private:
    FacadeConfig config_;
    std::string facadeType_;
    
public:
    ConfigurableFacadeFactory(const FacadeConfig& config, const std::string& type)
        : config_(config), facadeType_(type) {}
    
    // Методы для настройки конфигурации
    ConfigurableFacadeFactory& setSetting(const std::string& key, const std::string& value) {
        config_.setSetting(key, value);
        return *this;
    }
    
    template<typename T>
    ConfigurableFacadeFactory& setCustomSetting(const std::string& key, const T& value) {
        config_.setCustomSetting(key, value);
        return *this;
    }
    
    // Создание фасада
    std::unique_ptr<ConfigurableFacade> build() {
        if (facadeType_ == "system") {
            return std::make_unique<ConfigurableSystemFacade>(config_);
        }
        throw std::invalid_argument("Unknown facade type: " + facadeType_);
    }
    
    // Создание фасада с валидацией
    std::unique_ptr<ConfigurableFacade> buildSafe() {
        try {
            return build();
        } catch (const std::exception& e) {
            std::cerr << "Error building facade: " << e.what() << std::endl;
            return nullptr;
        }
    }
    
    // Получение текущей конфигурации
    FacadeConfig getConfig() const {
        return config_;
    }
    
    std::string getFacadeType() const {
        return facadeType_;
    }
};

// Менеджер конфигураций
class FacadeConfigManager {
private:
    std::unordered_map<std::string, FacadeConfig> configs_;
    
public:
    void registerConfig(const std::string& name, const FacadeConfig& config) {
        configs_[name] = config;
    }
    
    std::optional<FacadeConfig> getConfig(const std::string& name) const {
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
    
    std::unique_ptr<ConfigurableFacadeFactory> createFactory(
        const std::string& configName, 
        const std::string& facadeType
    ) {
        auto config = getConfig(configName);
        if (config) {
            return std::make_unique<ConfigurableFacadeFactory>(*config, facadeType);
        }
        return nullptr;
    }
};
```

### Пример использования
```cpp
int main() {
    FacadeConfigManager configManager;
    
    // Регистрируем конфигурации
    FacadeConfig productionConfig("Production System", "1.0");
    productionConfig.setSetting("database.table", "production_data");
    productionConfig.setSetting("cache.prefix", "prod");
    productionConfig.setSetting("logging.level", "INFO");
    productionConfig.setCustomSetting("database.config", "production_db.conf");
    productionConfig.setCustomSetting("cache.config", "production_cache.conf");
    productionConfig.setCustomSetting("logging.config", "production_log.conf");
    
    FacadeConfig developmentConfig("Development System", "2.0");
    developmentConfig.setSetting("database.table", "dev_data");
    developmentConfig.setSetting("cache.prefix", "dev");
    developmentConfig.setSetting("logging.level", "DEBUG");
    developmentConfig.setCustomSetting("database.config", "dev_db.conf");
    developmentConfig.setCustomSetting("cache.config", "dev_cache.conf");
    developmentConfig.setCustomSetting("logging.config", "dev_log.conf");
    
    configManager.registerConfig("production", productionConfig);
    configManager.registerConfig("development", developmentConfig);
    
    // Создаем фабрику для продакшн системы
    auto productionFactory = configManager.createFactory("production", "system");
    if (productionFactory) {
        productionFactory->setSetting("logging.level", "WARN");
        productionFactory->setCustomSetting("database.config", "prod_db.conf");
        
        if (auto facade = productionFactory->buildSafe()) {
            auto systemFacade = static_cast<ConfigurableSystemFacade*>(facade.get());
            systemFacade->display();
            
            std::cout << "\n" << systemFacade->getConfigInfo() << std::endl;
            
            systemFacade->initializeWithConfig();
            
            try {
                std::cout << "\n=== Processing Request ===" << std::endl;
                std::string result = systemFacade->processWithConfig("user123");
                std::cout << "Result: " << result << std::endl;
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << std::endl;
            }
        }
    }
    
    // Создаем фабрику для dev системы
    auto developmentFactory = configManager.createFactory("development", "system");
    if (developmentFactory) {
        developmentFactory->setSetting("logging.level", "TRACE");
        developmentFactory->setCustomSetting("cache.config", "dev_cache.conf");
        
        if (auto facade = developmentFactory->buildSafe()) {
            auto systemFacade = static_cast<ConfigurableSystemFacade*>(facade.get());
            systemFacade->display();
            
            std::cout << "\n" << systemFacade->getConfigInfo() << std::endl;
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

1. **Simplification**: Упрощайте сложные интерфейсы подсистем
2. **Abstraction**: Скрывайте сложность за простым интерфейсом
3. **Error Handling**: Обрабатывайте ошибки подсистем
4. **Configuration**: Используйте конфигурацию для настройки фасада
5. **Testing**: Создавайте тесты для проверки фасада

---

*Помните: Facade обеспечивает упрощенный интерфейс для сложных подсистем!*
