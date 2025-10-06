#include <iostream>
#include <memory>
#include <mutex>
#include <functional>
#include <map>
#include <string>
#include <any>

/**
 * @file modern_singleton.cpp
 * @brief Современные подходы к реализации Singleton в C++17/20
 * 
 * Этот файл демонстрирует продвинутые техники и альтернативы
 * классическому паттерну Singleton.
 */

// ============================================================================
// TEMPLATE SINGLETON
// ============================================================================

/**
 * @brief Шаблонный Singleton для переиспользования
 * 
 * Преимущества:
 * - Переиспользуемый код
 * - Type-safe
 * - Современный C++ подход
 */
template<typename T>
class Singleton {
protected:
    Singleton() = default;
    
public:
    static T& getInstance() {
        static T instance;
        return instance;
    }
    
    // Запрещаем копирование и присваивание
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;
};

// ============================================================================
// RAII SINGLETON
// ============================================================================

/**
 * @brief RAII Singleton с явным управлением жизненным циклом
 * 
 * Преимущества:
 * - Явный контроль над созданием/уничтожением
 * - Возможность переинициализации
 * - Thread-safe
 */
class RAIISingleton {
private:
    static std::unique_ptr<RAIISingleton> instance_;
    static std::mutex mutex_;
    std::string data_;
    
    RAIISingleton(const std::string& data) : data_(data) {
        std::cout << "RAIISingleton: Создан с данными: " << data_ << std::endl;
    }
    
public:
    static void initialize(const std::string& data) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_) {
            std::cout << "RAIISingleton: Переинициализация" << std::endl;
        }
        instance_ = std::make_unique<RAIISingleton>(data);
    }
    
    static RAIISingleton& getInstance() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
            throw std::runtime_error("RAIISingleton не инициализирован!");
        }
        return *instance_;
    }
    
    static void destroy() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_) {
            std::cout << "RAIISingleton: Уничтожение" << std::endl;
            instance_.reset();
        }
    }
    
    const std::string& getData() const { return data_; }
    void setData(const std::string& data) { data_ = data; }
    
    // Запрещаем копирование и присваивание
    RAIISingleton(const RAIISingleton&) = delete;
    RAIISingleton& operator=(const RAIISingleton&) = delete;
};

// Статические переменные
std::unique_ptr<RAIISingleton> RAIISingleton::instance_ = nullptr;
std::mutex RAIISingleton::mutex_;

// ============================================================================
// SERVICE LOCATOR (АЛЬТЕРНАТИВА SINGLETON)
// ============================================================================

/**
 * @brief Service Locator - альтернатива Singleton
 * 
 * Преимущества:
 * - Явное управление зависимостями
 * - Легче тестировать
 * - Возможность замены сервисов
 * - Не нарушает принципы SOLID
 */
class ServiceLocator {
private:
    static std::map<std::string, std::any> services_;
    static std::mutex mutex_;
    
public:
    template<typename T>
    static void registerService(const std::string& name, std::unique_ptr<T> service) {
        std::lock_guard<std::mutex> lock(mutex_);
        services_[name] = std::move(service);
        std::cout << "ServiceLocator: Зарегистрирован сервис '" << name << "'" << std::endl;
    }
    
    template<typename T>
    static T& getService(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = services_.find(name);
        if (it == services_.end()) {
            throw std::runtime_error("Сервис '" + name + "' не найден!");
        }
        
        try {
            return *std::any_cast<std::unique_ptr<T>&>(it->second);
        } catch (const std::bad_any_cast&) {
            throw std::runtime_error("Неверный тип сервиса '" + name + "'!");
        }
    }
    
    static void unregisterService(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        services_.erase(name);
        std::cout << "ServiceLocator: Удален сервис '" << name << "'" << std::endl;
    }
    
    static void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        services_.clear();
        std::cout << "ServiceLocator: Очищены все сервисы" << std::endl;
    }
};

// Статические переменные
std::map<std::string, std::any> ServiceLocator::services_;
std::mutex ServiceLocator::mutex_;

// ============================================================================
// DEPENDENCY INJECTION CONTAINER
// ============================================================================

/**
 * @brief Простой контейнер для Dependency Injection
 * 
 * Это более современная альтернатива Singleton,
 * которая следует принципам SOLID.
 */
class DIContainer {
private:
    static std::map<std::string, std::function<std::any()>> factories_;
    static std::map<std::string, std::any> instances_;
    static std::mutex mutex_;
    
public:
    template<typename T, typename... Args>
    static void registerSingleton(const std::string& name, Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex_);
        factories_[name] = [args...]() -> std::any {
            return std::make_unique<T>(args...);
        };
        std::cout << "DIContainer: Зарегистрирован singleton '" << name << "'" << std::endl;
    }
    
    template<typename T>
    static T& getSingleton(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Если экземпляр уже создан, возвращаем его
        auto instanceIt = instances_.find(name);
        if (instanceIt != instances_.end()) {
            return *std::any_cast<std::unique_ptr<T>&>(instanceIt->second);
        }
        
        // Создаем новый экземпляр
        auto factoryIt = factories_.find(name);
        if (factoryIt == factories_.end()) {
            throw std::runtime_error("Singleton '" + name + "' не зарегистрирован!");
        }
        
        auto instance = factoryIt->second();
        instances_[name] = instance;
        
        return *std::any_cast<std::unique_ptr<T>&>(instances_[name]);
    }
    
    template<typename T>
    static std::unique_ptr<T> create(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = factories_.find(name);
        if (it == factories_.end()) {
            throw std::runtime_error("Фабрика '" + name + "' не зарегистрирована!");
        }
        
        auto instance = it->second();
        return std::unique_ptr<T>(std::any_cast<std::unique_ptr<T>>(instance).release());
    }
};

// Статические переменные
std::map<std::string, std::function<std::any()>> DIContainer::factories_;
std::map<std::string, std::any> DIContainer::instances_;
std::mutex DIContainer::mutex_;

// ============================================================================
// ПРИМЕРЫ СЕРВИСОВ
// ============================================================================

/**
 * @brief Пример сервиса - логгер
 */
class LoggerService {
private:
    std::string logFile_;
    
public:
    explicit LoggerService(const std::string& logFile = "app.log") 
        : logFile_(logFile) {
        std::cout << "LoggerService: Создан с файлом " << logFile_ << std::endl;
    }
    
    void log(const std::string& message) {
        std::cout << "[LOG] " << message << std::endl;
    }
    
    void info(const std::string& message) {
        std::cout << "[INFO] " << message << std::endl;
    }
    
    void error(const std::string& message) {
        std::cout << "[ERROR] " << message << std::endl;
    }
};

/**
 * @brief Пример сервиса - конфигурация
 */
class ConfigService {
private:
    std::map<std::string, std::string> config_;
    
public:
    ConfigService() {
        config_["app.name"] = "MyApp";
        config_["app.version"] = "1.0.0";
        config_["db.host"] = "localhost";
        config_["db.port"] = "5432";
        std::cout << "ConfigService: Создан" << std::endl;
    }
    
    std::string get(const std::string& key) const {
        auto it = config_.find(key);
        return (it != config_.end()) ? it->second : "";
    }
    
    void set(const std::string& key, const std::string& value) {
        config_[key] = value;
    }
};

/**
 * @brief Пример класса, использующего сервисы через DI
 */
class UserService {
private:
    LoggerService& logger_;
    ConfigService& config_;
    
public:
    UserService(LoggerService& logger, ConfigService& config)
        : logger_(logger), config_(config) {
        logger_.info("UserService создан");
    }
    
    void createUser(const std::string& username) {
        logger_.info("Создание пользователя: " + username);
        std::cout << "Пользователь '" << username << "' создан" << std::endl;
    }
    
    void deleteUser(const std::string& username) {
        logger_.info("Удаление пользователя: " + username);
        std::cout << "Пользователь '" << username << "' удален" << std::endl;
    }
};

// ============================================================================
// ДЕМОНСТРАЦИОННЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Демонстрация Template Singleton
 */
void demonstrateTemplateSingleton() {
    std::cout << "\n=== Template Singleton ===" << std::endl;
    
    // Используем template singleton
    class MyService : public Singleton<MyService> {
        friend class Singleton<MyService>;
    private:
        MyService() {
            std::cout << "MyService: Создан через template singleton" << std::endl;
        }
        
    public:
        void doSomething() {
            std::cout << "MyService: Выполняю работу" << std::endl;
        }
    };
    
    auto& service1 = MyService::getInstance();
    auto& service2 = MyService::getInstance();
    
    std::cout << "service1 == service2: " << (&service1 == &service2) << std::endl;
    service1.doSomething();
}

/**
 * @brief Демонстрация RAII Singleton
 */
void demonstrateRAIISingleton() {
    std::cout << "\n=== RAII Singleton ===" << std::endl;
    
    // Инициализируем singleton
    RAIISingleton::initialize("Начальные данные");
    
    auto& singleton = RAIISingleton::getInstance();
    std::cout << "Данные: " << singleton.getData() << std::endl;
    
    // Переинициализируем
    RAIISingleton::initialize("Новые данные");
    std::cout << "Новые данные: " << RAIISingleton::getInstance().getData() << std::endl;
    
    // Уничтожаем
    RAIISingleton::destroy();
    
    try {
        RAIISingleton::getInstance();
    } catch (const std::exception& e) {
        std::cout << "Ошибка: " << e.what() << std::endl;
    }
}

/**
 * @brief Демонстрация Service Locator
 */
void demonstrateServiceLocator() {
    std::cout << "\n=== Service Locator ===" << std::endl;
    
    // Регистрируем сервисы
    ServiceLocator::registerService<LoggerService>("logger", 
        std::make_unique<LoggerService>("service_locator.log"));
    ServiceLocator::registerService<ConfigService>("config", 
        std::make_unique<ConfigService>());
    
    // Используем сервисы
    auto& logger = ServiceLocator::getService<LoggerService>("logger");
    auto& config = ServiceLocator::getService<ConfigService>("config");
    
    logger.info("Сервис через Service Locator");
    std::cout << "App name: " << config.get("app.name") << std::endl;
    
    // Удаляем сервис
    ServiceLocator::unregisterService("config");
    
    try {
        ServiceLocator::getService<ConfigService>("config");
    } catch (const std::exception& e) {
        std::cout << "Ошибка: " << e.what() << std::endl;
    }
}

/**
 * @brief Демонстрация Dependency Injection
 */
void demonstrateDependencyInjection() {
    std::cout << "\n=== Dependency Injection ===" << std::endl;
    
    // Регистрируем сервисы в DI контейнере
    DIContainer::registerSingleton<LoggerService>("logger", "di.log");
    DIContainer::registerSingleton<ConfigService>("config");
    
    // Получаем сервисы
    auto& logger = DIContainer::getSingleton<LoggerService>("logger");
    auto& config = DIContainer::getSingleton<ConfigService>("config");
    
    logger.info("Сервис через Dependency Injection");
    std::cout << "App version: " << config.get("app.version") << std::endl;
    
    // Создаем UserService с зависимостями
    UserService userService(logger, config);
    userService.createUser("john_doe");
    userService.deleteUser("jane_smith");
    
    // Создаем новый экземпляр (не singleton)
    auto newLogger = DIContainer::create<LoggerService>("logger");
    newLogger->info("Новый экземпляр логгера");
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🚀 Современные подходы к Singleton в C++" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    demonstrateTemplateSingleton();
    demonstrateRAIISingleton();
    demonstrateServiceLocator();
    demonstrateDependencyInjection();
    
    std::cout << "\n✅ Демонстрация современных подходов завершена!" << std::endl;
    std::cout << "\n🎯 Рекомендации:" << std::endl;
    std::cout << "1. Используйте Meyers Singleton для простых случаев" << std::endl;
    std::cout << "2. Рассмотрите Service Locator для гибкости" << std::endl;
    std::cout << "3. Предпочитайте Dependency Injection для тестируемости" << std::endl;
    std::cout << "4. Избегайте Singleton когда это возможно" << std::endl;
    std::cout << "5. Документируйте причины использования Singleton" << std::endl;
    
    return 0;
}
