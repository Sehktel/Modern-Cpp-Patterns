# 🏋️ Упражнения: Proxy Pattern

## 📋 Задание 1: Modern Proxy с Templates

### Описание
Создайте современную реализацию Proxy паттерна с использованием C++17:

### Требования
1. ✅ Используйте `std::variant` для разных типов прокси
2. ✅ Примените `std::optional` для обработки ошибок
3. ✅ Используйте `auto` и `decltype` для вывода типов
4. ✅ Добавьте поддержку variadic templates
5. ✅ Реализуйте type-safe проксирование

### Современная архитектура
```cpp
#include <memory>
#include <variant>
#include <optional>
#include <unordered_map>
#include <functional>
#include <string>

// Базовый интерфейс
class Subject {
public:
    virtual ~Subject() = default;
    virtual std::string request() const = 0;
    virtual void process(const std::string& data) = 0;
    virtual bool isAvailable() const = 0;
};

// Реальный субъект
class RealSubject : public Subject {
private:
    std::string name_;
    bool available_;
    
public:
    RealSubject(const std::string& name) : name_(name), available_(true) {}
    
    std::string request() const override {
        if (!available_) {
            throw std::runtime_error("RealSubject is not available");
        }
        return "RealSubject " + name_ + " processed request";
    }
    
    void process(const std::string& data) override {
        if (!available_) {
            throw std::runtime_error("RealSubject is not available");
        }
        std::cout << "RealSubject " << name_ << " processing: " << data << std::endl;
    }
    
    bool isAvailable() const override {
        return available_;
    }
    
    void setAvailable(bool available) {
        available_ = available;
    }
    
    std::string getName() const {
        return name_;
    }
};

// Современный прокси
class ModernProxy : public Subject {
private:
    std::unique_ptr<RealSubject> realSubject_;
    std::string name_;
    bool cached_;
    std::string cachedResult_;
    
public:
    ModernProxy(const std::string& name) 
        : name_(name), cached_(false) {
        realSubject_ = std::make_unique<RealSubject>(name);
    }
    
    std::string request() const override {
        if (!realSubject_->isAvailable()) {
            return "Proxy " + name_ + ": RealSubject not available";
        }
        
        if (cached_) {
            return "Proxy " + name_ + ": Cached result - " + cachedResult_;
        }
        
        try {
            std::string result = realSubject_->request();
            cachedResult_ = result;
            cached_ = true;
            return "Proxy " + name_ + ": " + result;
        } catch (const std::exception& e) {
            return "Proxy " + name_ + ": Error - " + std::string(e.what());
        }
    }
    
    void process(const std::string& data) override {
        if (!realSubject_->isAvailable()) {
            std::cout << "Proxy " << name_ << ": RealSubject not available" << std::endl;
            return;
        }
        
        try {
            realSubject_->process(data);
            std::cout << "Proxy " << name_ << ": Processed successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Proxy " << name_ << ": Error - " << e.what() << std::endl;
        }
    }
    
    bool isAvailable() const override {
        return realSubject_->isAvailable();
    }
    
    // Дополнительные методы прокси
    void clearCache() {
        cached_ = false;
        cachedResult_.clear();
    }
    
    bool isCached() const {
        return cached_;
    }
    
    void setRealSubjectAvailable(bool available) {
        realSubject_->setAvailable(available);
    }
};

// Фабрика прокси
class ProxyFactory {
private:
    using ProxyVariant = std::variant<
        std::unique_ptr<ModernProxy>
    >;
    
    using CreatorFunction = std::function<ProxyVariant(const std::string&)>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerProxy(const std::string& type) {
        creators_[type] = [](const std::string& name) -> ProxyVariant {
            return std::make_unique<ModernProxy>(name);
        };
    }
    
    std::optional<ProxyVariant> createProxy(const std::string& type, const std::string& name) {
        auto it = creators_.find(type);
        if (it != creators_.end()) {
            return it->second(name);
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
    ProxyFactory factory;
    
    // Регистрируем прокси
    factory.registerProxy("modern");
    
    // Создаем прокси
    if (auto proxy = factory.createProxy("modern", "TestProxy")) {
        std::visit([](auto& p) {
            std::cout << "=== Proxy Test ===" << std::endl;
            
            // Тест доступности
            std::cout << "Available: " << (p->isAvailable() ? "Yes" : "No") << std::endl;
            
            // Тест запроса
            std::cout << "Request: " << p->request() << std::endl;
            
            // Тест обработки
            p->process("Test data");
            
            // Тест кэширования
            std::cout << "Cached: " << (p->isCached() ? "Yes" : "No") << std::endl;
            
            // Повторный запрос (должен использовать кэш)
            std::cout << "Request (cached): " << p->request() << std::endl;
            
            // Очистка кэша
            p->clearCache();
            std::cout << "Cache cleared" << std::endl;
            
            // Тест недоступности
            p->setRealSubjectAvailable(false);
            std::cout << "Available: " << (p->isAvailable() ? "Yes" : "No") << std::endl;
            std::cout << "Request (unavailable): " << p->request() << std::endl;
        }, *proxy);
    }
    
    return 0;
}
```

---

## 📋 Задание 2: Multi-Type Proxy System

### Описание
Создайте систему множественных типов прокси для работы с разными типами объектов:

### Требования
1. **Multiple Types**: Поддержка разных типов прокси
2. **Type Safety**: Проверка типов на этапе компиляции
3. **Error Handling**: Обработка ошибок проксирования
4. **Validation**: Валидация прокси

### Реализация
```cpp
#include <any>
#include <type_traits>
#include <stdexcept>

// Базовый интерфейс для проксируемого объекта
class IProxiable {
public:
    virtual ~IProxiable() = default;
    virtual std::string getName() const = 0;
    virtual bool isReady() const = 0;
    virtual void initialize() = 0;
    virtual void shutdown() = 0;
};

// Конкретные проксируемые объекты
class DatabaseService : public IProxiable {
private:
    std::string name_;
    bool ready_;
    
public:
    DatabaseService(const std::string& name) : name_(name), ready_(false) {}
    
    std::string getName() const override {
        return name_;
    }
    
    bool isReady() const override {
        return ready_;
    }
    
    void initialize() override {
        std::cout << "DatabaseService " << name_ << " initializing..." << std::endl;
        ready_ = true;
        std::cout << "DatabaseService " << name_ << " ready" << std::endl;
    }
    
    void shutdown() override {
        std::cout << "DatabaseService " << name_ << " shutting down..." << std::endl;
        ready_ = false;
        std::cout << "DatabaseService " << name_ << " stopped" << std::endl;
    }
    
    std::string query(const std::string& sql) {
        if (!isReady()) {
            throw std::runtime_error("DatabaseService not ready");
        }
        return "Query result: " + sql;
    }
};

class CacheService : public IProxiable {
private:
    std::string name_;
    bool ready_;
    std::unordered_map<std::string, std::string> cache_;
    
public:
    CacheService(const std::string& name) : name_(name), ready_(false) {}
    
    std::string getName() const override {
        return name_;
    }
    
    bool isReady() const override {
        return ready_;
    }
    
    void initialize() override {
        std::cout << "CacheService " << name_ << " initializing..." << std::endl;
        ready_ = true;
        std::cout << "CacheService " << name_ << " ready" << std::endl;
    }
    
    void shutdown() override {
        std::cout << "CacheService " << name_ << " shutting down..." << std::endl;
        cache_.clear();
        ready_ = false;
        std::cout << "CacheService " << name_ << " stopped" << std::endl;
    }
    
    void put(const std::string& key, const std::string& value) {
        if (!isReady()) {
            throw std::runtime_error("CacheService not ready");
        }
        cache_[key] = value;
    }
    
    std::optional<std::string> get(const std::string& key) const {
        if (!isReady()) {
            throw std::runtime_error("CacheService not ready");
        }
        auto it = cache_.find(key);
        return it != cache_.end() ? std::optional<std::string>(it->second) : std::nullopt;
    }
};

class LoggingService : public IProxiable {
private:
    std::string name_;
    bool ready_;
    
public:
    LoggingService(const std::string& name) : name_(name), ready_(false) {}
    
    std::string getName() const override {
        return name_;
    }
    
    bool isReady() const override {
        return ready_;
    }
    
    void initialize() override {
        std::cout << "LoggingService " << name_ << " initializing..." << std::endl;
        ready_ = true;
        std::cout << "LoggingService " << name_ << " ready" << std::endl;
    }
    
    void shutdown() override {
        std::cout << "LoggingService " << name_ << " shutting down..." << std::endl;
        ready_ = false;
        std::cout << "LoggingService " << name_ << " stopped" << std::endl;
    }
    
    void log(const std::string& message) {
        if (!isReady()) {
            throw std::runtime_error("LoggingService not ready");
        }
        std::cout << "[LOG] " << message << std::endl;
    }
};

// Базовый прокси
class BaseProxy : public IProxiable {
protected:
    std::unique_ptr<IProxiable> service_;
    
public:
    BaseProxy(std::unique_ptr<IProxiable> service) : service_(std::move(service)) {}
    
    std::string getName() const override {
        return service_ ? service_->getName() : "No service";
    }
    
    bool isReady() const override {
        return service_ ? service_->isReady() : false;
    }
    
    void initialize() override {
        if (service_) {
            service_->initialize();
        }
    }
    
    void shutdown() override {
        if (service_) {
            service_->shutdown();
        }
    }
};

// Конкретные прокси
class DatabaseProxy : public BaseProxy {
private:
    std::string cachedQuery_;
    std::string cachedResult_;
    bool hasCachedResult_;
    
public:
    DatabaseProxy(std::unique_ptr<DatabaseService> service) 
        : BaseProxy(std::move(service)), hasCachedResult_(false) {}
    
    std::string query(const std::string& sql) {
        if (!isReady()) {
            throw std::runtime_error("DatabaseProxy not ready");
        }
        
        // Проверяем кэш
        if (hasCachedResult_ && cachedQuery_ == sql) {
            return "Cached: " + cachedResult_;
        }
        
        // Выполняем запрос
        auto dbService = static_cast<DatabaseService*>(service_.get());
        std::string result = dbService->query(sql);
        
        // Сохраняем в кэш
        cachedQuery_ = sql;
        cachedResult_ = result;
        hasCachedResult_ = true;
        
        return result;
    }
    
    void clearCache() {
        hasCachedResult_ = false;
        cachedQuery_.clear();
        cachedResult_.clear();
    }
    
    bool hasCachedResult() const {
        return hasCachedResult_;
    }
};

class CacheProxy : public BaseProxy {
private:
    int accessCount_;
    int maxAccesses_;
    
public:
    CacheProxy(std::unique_ptr<CacheService> service, int maxAccesses = 100)
        : BaseProxy(std::move(service)), accessCount_(0), maxAccesses_(maxAccesses) {}
    
    void put(const std::string& key, const std::string& value) {
        if (!isReady()) {
            throw std::runtime_error("CacheProxy not ready");
        }
        
        if (accessCount_ >= maxAccesses_) {
            throw std::runtime_error("CacheProxy: Maximum accesses exceeded");
        }
        
        auto cacheService = static_cast<CacheService*>(service_.get());
        cacheService->put(key, value);
        accessCount_++;
    }
    
    std::optional<std::string> get(const std::string& key) {
        if (!isReady()) {
            throw std::runtime_error("CacheProxy not ready");
        }
        
        if (accessCount_ >= maxAccesses_) {
            throw std::runtime_error("CacheProxy: Maximum accesses exceeded");
        }
        
        auto cacheService = static_cast<CacheService*>(service_.get());
        auto result = cacheService->get(key);
        accessCount_++;
        return result;
    }
    
    int getAccessCount() const {
        return accessCount_;
    }
    
    int getMaxAccesses() const {
        return maxAccesses_;
    }
    
    void resetAccessCount() {
        accessCount_ = 0;
    }
};

class LoggingProxy : public BaseProxy {
private:
    std::vector<std::string> logBuffer_;
    size_t maxBufferSize_;
    
public:
    LoggingProxy(std::unique_ptr<LoggingService> service, size_t maxBufferSize = 10)
        : BaseProxy(std::move(service)), maxBufferSize_(maxBufferSize) {}
    
    void log(const std::string& message) {
        if (!isReady()) {
            throw std::runtime_error("LoggingProxy not ready");
        }
        
        // Добавляем в буфер
        logBuffer_.push_back(message);
        
        // Если буфер заполнен, сбрасываем его
        if (logBuffer_.size() >= maxBufferSize_) {
            flushLogs();
        }
    }
    
    void flushLogs() {
        if (!isReady()) {
            throw std::runtime_error("LoggingProxy not ready");
        }
        
        auto loggingService = static_cast<LoggingService*>(service_.get());
        for (const auto& log : logBuffer_) {
            loggingService->log(log);
        }
        logBuffer_.clear();
    }
    
    size_t getBufferSize() const {
        return logBuffer_.size();
    }
    
    size_t getMaxBufferSize() const {
        return maxBufferSize_;
    }
    
    void setMaxBufferSize(size_t size) {
        maxBufferSize_ = size;
    }
};

// Фабрика прокси
class MultiTypeProxyFactory {
private:
    using ProxyVariant = std::variant<
        std::unique_ptr<DatabaseProxy>,
        std::unique_ptr<CacheProxy>,
        std::unique_ptr<LoggingProxy>
    >;
    
    using CreatorFunction = std::function<ProxyVariant(const std::string&)>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerProxy(const std::string& type) {
        if (type == "database") {
            creators_[type] = [](const std::string& name) -> ProxyVariant {
                return std::make_unique<DatabaseProxy>(
                    std::make_unique<DatabaseService>(name)
                );
            };
        } else if (type == "cache") {
            creators_[type] = [](const std::string& name) -> ProxyVariant {
                return std::make_unique<CacheProxy>(
                    std::make_unique<CacheService>(name)
                );
            };
        } else if (type == "logging") {
            creators_[type] = [](const std::string& name) -> ProxyVariant {
                return std::make_unique<LoggingProxy>(
                    std::make_unique<LoggingService>(name)
                );
            };
        }
    }
    
    std::optional<ProxyVariant> createProxy(const std::string& type, const std::string& name) {
        auto it = creators_.find(type);
        if (it != creators_.end()) {
            return it->second(name);
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
    MultiTypeProxyFactory factory;
    
    // Регистрируем прокси
    factory.registerProxy("database");
    factory.registerProxy("cache");
    factory.registerProxy("logging");
    
    // Создаем прокси для базы данных
    if (auto proxy = factory.createProxy("database", "MainDB")) {
        std::visit([](auto& p) {
            std::cout << "=== Database Proxy Test ===" << std::endl;
            
            p->initialize();
            
            // Тест запроса
            std::cout << "Query 1: " << p->query("SELECT * FROM users") << std::endl;
            
            // Повторный запрос (должен использовать кэш)
            std::cout << "Query 2 (cached): " << p->query("SELECT * FROM users") << std::endl;
            
            // Очистка кэша
            p->clearCache();
            std::cout << "Cache cleared" << std::endl;
            
            // Новый запрос
            std::cout << "Query 3: " << p->query("SELECT * FROM products") << std::endl;
            
            p->shutdown();
        }, *proxy);
    }
    
    // Создаем прокси для кэша
    if (auto proxy = factory.createProxy("cache", "MainCache")) {
        std::visit([](auto& p) {
            std::cout << "\n=== Cache Proxy Test ===" << std::endl;
            
            p->initialize();
            
            // Тест записи
            p->put("key1", "value1");
            p->put("key2", "value2");
            
            // Тест чтения
            if (auto value = p->get("key1")) {
                std::cout << "Retrieved: " << *value << std::endl;
            }
            
            std::cout << "Access count: " << p->getAccessCount() << std::endl;
            std::cout << "Max accesses: " << p->getMaxAccesses() << std::endl;
            
            p->shutdown();
        }, *proxy);
    }
    
    // Создаем прокси для логирования
    if (auto proxy = factory.createProxy("logging", "MainLogger")) {
        std::visit([](auto& p) {
            std::cout << "\n=== Logging Proxy Test ===" << std::endl;
            
            p->initialize();
            
            // Тест логирования
            p->log("Message 1");
            p->log("Message 2");
            p->log("Message 3");
            
            std::cout << "Buffer size: " << p->getBufferSize() << std::endl;
            std::cout << "Max buffer size: " << p->getMaxBufferSize() << std::endl;
            
            // Принудительный сброс буфера
            p->flushLogs();
            
            std::cout << "Buffer size after flush: " << p->getBufferSize() << std::endl;
            
            p->shutdown();
        }, *proxy);
    }
    
    return 0;
}
```

---

## 📋 Задание 3: Proxy с Configuration

### Описание
Создайте прокси, который поддерживает конфигурацию и может быть легко настраиваемым:

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

// Конфигурация прокси
struct ProxyConfig {
    std::string name;
    std::string version;
    std::unordered_map<std::string, std::string> settings;
    std::unordered_map<std::string, std::any> customSettings;
    
    ProxyConfig(const std::string& n, const std::string& v)
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
class ConfigurableProxy {
protected:
    ProxyConfig config_;
    
public:
    ConfigurableProxy(const ProxyConfig& config) : config_(config) {}
    
    virtual void validateConfig() const {
        if (config_.name.empty()) {
            throw std::invalid_argument("Proxy name cannot be empty");
        }
        if (config_.version.empty()) {
            throw std::invalid_argument("Proxy version cannot be empty");
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

// Конфигурируемый прокси
class ConfigurableServiceProxy : public IProxiable, public ConfigurableProxy {
private:
    std::unique_ptr<IProxiable> service_;
    std::string serviceType_;
    bool initialized_;
    
public:
    ConfigurableServiceProxy(
        const ProxyConfig& config,
        std::unique_ptr<IProxiable> service,
        const std::string& type
    ) : ConfigurableProxy(config), service_(std::move(service)), 
        serviceType_(type), initialized_(false) {
        validateConfig();
    }
    
    std::string getName() const override {
        return service_ ? service_->getName() : "No service";
    }
    
    bool isReady() const override {
        return service_ ? service_->isReady() : false;
    }
    
    void initialize() override {
        if (service_) {
            service_->initialize();
            initialized_ = true;
        }
    }
    
    void shutdown() override {
        if (service_) {
            service_->shutdown();
            initialized_ = false;
        }
    }
    
    // Операции с конфигурацией
    std::string processWithConfig(const std::string& data) {
        if (!isReady()) {
            throw std::runtime_error("Service not ready");
        }
        
        // Используем настройки из конфигурации
        std::string processedData = data;
        
        if (auto prefix = config_.getSetting("data.prefix")) {
            processedData = prefix + "_" + data;
        }
        
        if (auto suffix = config_.getSetting("data.suffix")) {
            processedData = processedData + "_" + suffix;
        }
        
        return "Processed: " + processedData;
    }
    
    void logWithConfig(const std::string& message) {
        if (!isReady()) {
            throw std::runtime_error("Service not ready");
        }
        
        // Используем настройки логирования из конфигурации
        std::string logMessage = message;
        
        if (auto level = config_.getSetting("logging.level")) {
            logMessage = "[" + level + "] " + message;
        }
        
        if (auto timestamp = config_.getCustomSetting<bool>("logging.timestamp")) {
            if (*timestamp) {
                logMessage = "[TIMESTAMP] " + logMessage;
            }
        }
        
        std::cout << logMessage << std::endl;
    }
    
    void display() const {
        std::cout << "Configurable Service Proxy: " << config_.name << " v" << config_.version << std::endl;
        std::cout << "  Service type: " << serviceType_ << std::endl;
        std::cout << "  Initialized: " << (initialized_ ? "Yes" : "No") << std::endl;
        std::cout << "  Settings:" << std::endl;
        for (const auto& setting : config_.settings) {
            std::cout << "    " << setting.first << ": " << setting.second << std::endl;
        }
    }
};

// Конфигурируемая фабрика прокси
class ConfigurableProxyFactory {
private:
    ProxyConfig config_;
    std::string proxyType_;
    
public:
    ConfigurableProxyFactory(const ProxyConfig& config, const std::string& type)
        : config_(config), proxyType_(type) {}
    
    // Методы для настройки конфигурации
    ConfigurableProxyFactory& setSetting(const std::string& key, const std::string& value) {
        config_.setSetting(key, value);
        return *this;
    }
    
    template<typename T>
    ConfigurableProxyFactory& setCustomSetting(const std::string& key, const T& value) {
        config_.setCustomSetting(key, value);
        return *this;
    }
    
    // Создание прокси
    std::unique_ptr<ConfigurableProxy> build(std::unique_ptr<IProxiable> service) {
        return std::make_unique<ConfigurableServiceProxy>(
            config_, std::move(service), proxyType_
        );
    }
    
    // Создание прокси с валидацией
    std::unique_ptr<ConfigurableProxy> buildSafe(std::unique_ptr<IProxiable> service) {
        try {
            return build(std::move(service));
        } catch (const std::exception& e) {
            std::cerr << "Error building proxy: " << e.what() << std::endl;
            return nullptr;
        }
    }
    
    // Получение текущей конфигурации
    ProxyConfig getConfig() const {
        return config_;
    }
    
    std::string getProxyType() const {
        return proxyType_;
    }
};

// Менеджер конфигураций
class ProxyConfigManager {
private:
    std::unordered_map<std::string, ProxyConfig> configs_;
    
public:
    void registerConfig(const std::string& name, const ProxyConfig& config) {
        configs_[name] = config;
    }
    
    std::optional<ProxyConfig> getConfig(const std::string& name) const {
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
    
    std::unique_ptr<ConfigurableProxyFactory> createFactory(
        const std::string& configName, 
        const std::string& proxyType
    ) {
        auto config = getConfig(configName);
        if (config) {
            return std::make_unique<ConfigurableProxyFactory>(*config, proxyType);
        }
        return nullptr;
    }
};
```

### Пример использования
```cpp
int main() {
    ProxyConfigManager configManager;
    
    // Регистрируем конфигурации
    ProxyConfig databaseProxyConfig("Database Proxy", "1.0");
    databaseProxyConfig.setSetting("data.prefix", "db");
    databaseProxyConfig.setSetting("data.suffix", "processed");
    databaseProxyConfig.setSetting("logging.level", "INFO");
    databaseProxyConfig.setCustomSetting("logging.timestamp", true);
    databaseProxyConfig.setCustomSetting("cache.size", 1000);
    
    ProxyConfig cacheProxyConfig("Cache Proxy", "2.0");
    cacheProxyConfig.setSetting("data.prefix", "cache");
    cacheProxyConfig.setSetting("data.suffix", "cached");
    cacheProxyConfig.setSetting("logging.level", "DEBUG");
    cacheProxyConfig.setCustomSetting("logging.timestamp", false);
    cacheProxyConfig.setCustomSetting("max.accesses", 500);
    
    configManager.registerConfig("database", databaseProxyConfig);
    configManager.registerConfig("cache", cacheProxyConfig);
    
    // Создаем фабрику для прокси базы данных
    auto databaseProxyFactory = configManager.createFactory("database", "Database");
    if (databaseProxyFactory) {
        databaseProxyFactory->setSetting("logging.level", "WARN");
        databaseProxyFactory->setCustomSetting("cache.size", 2000);
        
        if (auto proxy = databaseProxyFactory->buildSafe(
            std::make_unique<DatabaseService>("MainDB")
        )) {
            auto serviceProxy = static_cast<ConfigurableServiceProxy*>(proxy.get());
            serviceProxy->display();
            
            serviceProxy->initialize();
            
            try {
                std::cout << "\n=== Processing with Config ===" << std::endl;
                std::string result = serviceProxy->processWithConfig("user_data");
                std::cout << "Result: " << result << std::endl;
                
                serviceProxy->logWithConfig("Processing completed");
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << std::endl;
            }
            
            serviceProxy->shutdown();
        }
    }
    
    // Создаем фабрику для прокси кэша
    auto cacheProxyFactory = configManager.createFactory("cache", "Cache");
    if (cacheProxyFactory) {
        cacheProxyFactory->setSetting("logging.level", "TRACE");
        cacheProxyFactory->setCustomSetting("max.accesses", 1000);
        
        if (auto proxy = cacheProxyFactory->buildSafe(
            std::make_unique<CacheService>("MainCache")
        )) {
            auto serviceProxy = static_cast<ConfigurableServiceProxy*>(proxy.get());
            serviceProxy->display();
            
            serviceProxy->initialize();
            
            try {
                std::cout << "\n=== Processing with Config ===" << std::endl;
                std::string result = serviceProxy->processWithConfig("cache_data");
                std::cout << "Result: " << result << std::endl;
                
                serviceProxy->logWithConfig("Cache processing completed");
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << std::endl;
            }
            
            serviceProxy->shutdown();
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

1. **Control Access**: Используйте прокси для контроля доступа к объектам
2. **Caching**: Реализуйте кэширование для улучшения производительности
3. **Error Handling**: Обрабатывайте ошибки проксирования
4. **Configuration**: Используйте конфигурацию для настройки прокси
5. **Testing**: Создавайте тесты для проверки прокси

---

*Помните: Proxy обеспечивает контроль доступа и дополнительную функциональность!*
