# 🏋️ Упражнения: Singleton Pattern

## 📋 Задание 1: Thread-Safe Singleton с Modern C++

### Описание
Создайте thread-safe реализацию Singleton паттерна, используя современные возможности C++17:

### Требования
1. ✅ Используйте `std::once_flag` и `std::call_once` для thread-safety
2. ✅ Примените RAII принципы
3. ✅ Добавьте возможность инициализации с параметрами
4. ✅ Реализуйте правильное управление жизненным циклом
5. ✅ Добавьте логирование для отслеживания создания/уничтожения

### Шаблон для начала
```cpp
#include <mutex>
#include <once_flag>
#include <memory>
#include <string>

class Logger {
private:
    static std::unique_ptr<Logger> instance_;
    static std::once_flag init_flag_;
    
    std::string log_file_;
    bool is_initialized_;
    
    // Приватный конструктор
    Logger(const std::string& log_file = "default.log");
    
public:
    // Запрещаем копирование и присваивание
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    // Разрешаем перемещение
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;
    
    // Получение экземпляра
    static Logger& getInstance(const std::string& log_file = "default.log");
    
    // Методы логирования
    void log(const std::string& message);
    void setLogLevel(int level);
    
    // Деструктор
    ~Logger();
};
```

### Тестовый случай
```cpp
#include <thread>
#include <vector>

void workerThread(int id) {
    auto& logger = Logger::getInstance("thread_" + std::to_string(id) + ".log");
    logger.log("Thread " + std::to_string(id) + " started");
    
    // Имитация работы
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    logger.log("Thread " + std::to_string(id) + " finished");
}

int main() {
    // Тест многопоточности
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(workerThread, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // Проверяем, что все потоки используют один экземпляр
    auto& logger1 = Logger::getInstance();
    auto& logger2 = Logger::getInstance();
    
    std::cout << "Same instance: " << (&logger1 == &logger2) << std::endl;
    
    return 0;
}
```

---

## 📋 Задание 2: Registry Singleton Pattern

### Описание
Создайте систему регистрации компонентов с использованием Singleton паттерна:

### Требования
1. **ComponentRegistry**: Singleton для регистрации компонентов
2. **Component**: Базовый класс для всех компонентов
3. **Factory**: Создание компонентов по имени
4. **Lifecycle Management**: Управление жизненным циклом компонентов

### Архитектура
```cpp
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>

// Базовый класс компонента
class Component {
public:
    virtual ~Component() = default;
    virtual void initialize() = 0;
    virtual void shutdown() = 0;
    virtual std::string getName() const = 0;
};

// Фабрика компонентов
using ComponentFactory = std::function<std::unique_ptr<Component>()>;

class ComponentRegistry {
private:
    static std::unique_ptr<ComponentRegistry> instance_;
    static std::once_flag init_flag_;
    
    std::unordered_map<std::string, ComponentFactory> factories_;
    std::unordered_map<std::string, std::unique_ptr<Component>> instances_;
    
    ComponentRegistry() = default;
    
public:
    static ComponentRegistry& getInstance();
    
    // Регистрация фабрики
    void registerFactory(const std::string& name, ComponentFactory factory);
    
    // Создание компонента
    std::unique_ptr<Component> createComponent(const std::string& name);
    
    // Управление экземплярами
    void registerInstance(const std::string& name, std::unique_ptr<Component> instance);
    Component* getInstance(const std::string& name);
    
    // Инициализация всех компонентов
    void initializeAll();
    void shutdownAll();
    
    // Информация о зарегистрированных компонентах
    std::vector<std::string> getRegisteredComponents() const;
};
```

### Пример использования
```cpp
// Конкретные компоненты
class DatabaseComponent : public Component {
public:
    void initialize() override {
        std::cout << "Database component initialized" << std::endl;
    }
    
    void shutdown() override {
        std::cout << "Database component shutdown" << std::endl;
    }
    
    std::string getName() const override {
        return "Database";
    }
};

class CacheComponent : public Component {
public:
    void initialize() override {
        std::cout << "Cache component initialized" << std::endl;
    }
    
    void shutdown() override {
        std::cout << "Cache component shutdown" << std::endl;
    }
    
    std::string getName() const override {
        return "Cache";
    }
};

int main() {
    auto& registry = ComponentRegistry::getInstance();
    
    // Регистрируем фабрики
    registry.registerFactory("Database", []() {
        return std::make_unique<DatabaseComponent>();
    });
    
    registry.registerFactory("Cache", []() {
        return std::make_unique<CacheComponent>();
    });
    
    // Создаем и регистрируем экземпляры
    auto db = registry.createComponent("Database");
    registry.registerInstance("db", std::move(db));
    
    auto cache = registry.createComponent("Cache");
    registry.registerInstance("cache", std::move(cache));
    
    // Инициализируем все компоненты
    registry.initializeAll();
    
    // Получаем доступ к компонентам
    auto* dbInstance = registry.getInstance("db");
    auto* cacheInstance = registry.getInstance("cache");
    
    // Завершаем работу
    registry.shutdownAll();
    
    return 0;
}
```

---

## 📋 Задание 3: Singleton с Dependency Injection

### Описание
Создайте Singleton, который поддерживает dependency injection и может быть легко тестируемым:

### Требования
1. **Configurable Singleton**: Возможность настройки через интерфейс
2. **Testable**: Возможность замены реализации для тестов
3. **Lazy Initialization**: Инициализация по требованию
4. **Reset Capability**: Возможность сброса для тестов

### Интерфейс
```cpp
#include <memory>
#include <functional>

// Интерфейс для конфигурации
class IConfiguration {
public:
    virtual ~IConfiguration() = default;
    virtual std::string getValue(const std::string& key) const = 0;
    virtual void setValue(const std::string& key, const std::string& value) = 0;
};

// Интерфейс для сервиса
class IService {
public:
    virtual ~IService() = default;
    virtual void doWork() = 0;
    virtual std::string getStatus() const = 0;
};

// Конфигурируемый Singleton
class ServiceManager {
private:
    static std::unique_ptr<ServiceManager> instance_;
    static std::once_flag init_flag_;
    
    std::unique_ptr<IConfiguration> config_;
    std::unique_ptr<IService> service_;
    
    ServiceManager() = default;
    
public:
    static ServiceManager& getInstance();
    
    // Настройка для тестов
    static void setTestInstance(std::unique_ptr<ServiceManager> testInstance);
    static void resetInstance();
    
    // Конфигурация
    void setConfiguration(std::unique_ptr<IConfiguration> config);
    void setService(std::unique_ptr<IService> service);
    
    // Работа с сервисом
    void initialize();
    void doWork();
    std::string getStatus() const;
    
    // Получение конфигурации
    IConfiguration* getConfiguration() const;
    IService* getService() const;
};
```

### Тестовый случай
```cpp
// Mock объекты для тестов
class MockConfiguration : public IConfiguration {
private:
    std::unordered_map<std::string, std::string> values_;
    
public:
    std::string getValue(const std::string& key) const override {
        auto it = values_.find(key);
        return it != values_.end() ? it->second : "";
    }
    
    void setValue(const std::string& key, const std::string& value) override {
        values_[key] = value;
    }
};

class MockService : public IService {
public:
    void doWork() override {
        std::cout << "Mock service working" << std::endl;
    }
    
    std::string getStatus() const override {
        return "Mock service status";
    }
};

void testServiceManager() {
    // Создаем тестовый экземпляр
    auto testManager = std::make_unique<ServiceManager>();
    
    // Настраиваем mock объекты
    auto mockConfig = std::make_unique<MockConfiguration>();
    mockConfig->setValue("test.key", "test.value");
    
    auto mockService = std::make_unique<MockService>();
    
    testManager->setConfiguration(std::move(mockConfig));
    testManager->setService(std::move(mockService));
    
    // Устанавливаем тестовый экземпляр
    ServiceManager::setTestInstance(std::move(testManager));
    
    // Тестируем
    auto& manager = ServiceManager::getInstance();
    manager.initialize();
    manager.doWork();
    
    std::cout << "Status: " << manager.getStatus() << std::endl;
    
    // Сбрасываем для следующих тестов
    ServiceManager::resetInstance();
}

int main() {
    testServiceManager();
    return 0;
}
```

---

## 📋 Задание 4: Singleton Anti-Pattern Analysis

### Описание
Проанализируйте проблемы Singleton паттерна и создайте альтернативные решения:

### Требования
1. **Проблемы Singleton**: Выявите основные проблемы
2. **Альтернативы**: Предложите лучшие решения
3. **Сравнение**: Сравните производительность и тестируемость
4. **Рекомендации**: Когда использовать, когда избегать

### Анализ проблем
```cpp
// Проблема 1: Глобальное состояние
class GlobalStateSingleton {
    // Проблемы: сложно тестировать, скрытые зависимости
};

// Проблема 2: Нарушение SRP
class GodSingleton {
    // Проблемы: слишком много ответственности
    void doEverything();
};

// Проблема 3: Нарушение OCP
class InflexibleSingleton {
    // Проблемы: сложно расширять и модифицировать
};
```

### Альтернативные решения
```cpp
// Решение 1: Dependency Injection
class ServiceContainer {
private:
    std::unordered_map<std::type_index, std::any> services_;
    
public:
    template<typename T>
    void registerService(std::unique_ptr<T> service) {
        services_[std::type_index(typeid(T))] = std::move(service);
    }
    
    template<typename T>
    T* getService() {
        auto it = services_.find(std::type_index(typeid(T)));
        return it != services_.end() ? std::any_cast<T*>(it->second) : nullptr;
    }
};

// Решение 2: Service Locator
class ServiceLocator {
private:
    static ServiceContainer container_;
    
public:
    template<typename T>
    static void registerService(std::unique_ptr<T> service) {
        container_.registerService(std::move(service));
    }
    
    template<typename T>
    static T* getService() {
        return container_.getService<T>();
    }
};

// Решение 3: Factory с кэшированием
template<typename T>
class CachedFactory {
private:
    static std::unique_ptr<T> instance_;
    static std::once_flag init_flag_;
    
public:
    static T& getInstance() {
        std::call_once(init_flag_, []() {
            instance_ = std::make_unique<T>();
        });
        return *instance_;
    }
    
    static void reset() {
        instance_.reset();
        init_flag_ = std::once_flag{};
    }
};
```

### Сравнительный анализ
```cpp
class PerformanceTest {
public:
    void testSingletonPerformance() {
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < 1000000; ++i) {
            auto& instance = Singleton::getInstance();
            instance.doSomething();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Singleton performance: " << duration.count() << " microseconds" << std::endl;
    }
    
    void testDIPerformance() {
        ServiceContainer container;
        container.registerService(std::make_unique<Service>());
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < 1000000; ++i) {
            auto* service = container.getService<Service>();
            service->doSomething();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "DI performance: " << duration.count() << " microseconds" << std::endl;
    }
};
```

---

## 📋 Задание 5: Modern Singleton Implementation

### Описание
Создайте современную реализацию Singleton с использованием C++17/20 возможностей:

### Требования
1. ✅ Используйте `std::optional` для lazy initialization
2. ✅ Примените `std::atomic` для thread-safety
3. ✅ Используйте `std::variant` для разных типов конфигурации
4. ✅ Добавьте поддержку `std::any` для гибкости
5. ✅ Реализуйте RAII с правильным порядком уничтожения

### Современная реализация
```cpp
#include <optional>
#include <atomic>
#include <variant>
#include <any>
#include <memory>
#include <mutex>

template<typename T>
class ModernSingleton {
private:
    static std::atomic<T*> instance_;
    static std::atomic<bool> destroyed_;
    static std::once_flag init_flag_;
    
    // Конфигурация с использованием variant
    using ConfigVariant = std::variant<int, double, std::string, bool>;
    static std::unordered_map<std::string, ConfigVariant> config_;
    static std::mutex config_mutex_;
    
    // Дополнительные данные с использованием any
    static std::unordered_map<std::string, std::any> metadata_;
    static std::mutex metadata_mutex_;
    
    ModernSingleton() = default;
    
public:
    // Получение экземпляра с конфигурацией
    template<typename... Args>
    static T& getInstance(Args&&... args) {
        T* instance = instance_.load(std::memory_order_acquire);
        
        if (instance == nullptr) {
            std::call_once(init_flag_, [&]() {
                instance = new T(std::forward<Args>(args)...);
                instance_.store(instance, std::memory_order_release);
                destroyed_.store(false, std::memory_order_release);
            });
        }
        
        return *instance;
    }
    
    // Конфигурация
    template<typename U>
    static void setConfig(const std::string& key, U&& value) {
        std::lock_guard<std::mutex> lock(config_mutex_);
        config_[key] = std::forward<U>(value);
    }
    
    template<typename U>
    static std::optional<U> getConfig(const std::string& key) {
        std::lock_guard<std::mutex> lock(config_mutex_);
        auto it = config_.find(key);
        if (it != config_.end()) {
            if (auto* value = std::get_if<U>(&it->second)) {
                return *value;
            }
        }
        return std::nullopt;
    }
    
    // Метаданные
    template<typename U>
    static void setMetadata(const std::string& key, U&& value) {
        std::lock_guard<std::mutex> lock(metadata_mutex_);
        metadata_[key] = std::forward<U>(value);
    }
    
    template<typename U>
    static std::optional<U> getMetadata(const std::string& key) {
        std::lock_guard<std::mutex> lock(metadata_mutex_);
        auto it = metadata_.find(key);
        if (it != metadata_.end()) {
            try {
                return std::any_cast<U>(it->second);
            } catch (const std::bad_any_cast&) {
                return std::nullopt;
            }
        }
        return std::nullopt;
    }
    
    // Сброс для тестов
    static void reset() {
        if (!destroyed_.load(std::memory_order_acquire)) {
            delete instance_.load(std::memory_order_acquire);
            instance_.store(nullptr, std::memory_order_release);
            destroyed_.store(true, std::memory_order_release);
            init_flag_ = std::once_flag{};
        }
    }
    
    // Деструктор
    ~ModernSingleton() {
        reset();
    }
};

// Статические переменные
template<typename T>
std::atomic<T*> ModernSingleton<T>::instance_{nullptr};

template<typename T>
std::atomic<bool> ModernSingleton<T>::destroyed_{false};

template<typename T>
std::once_flag ModernSingleton<T>::init_flag_;

template<typename T>
std::unordered_map<std::string, typename ModernSingleton<T>::ConfigVariant> ModernSingleton<T>::config_;

template<typename T>
std::mutex ModernSingleton<T>::config_mutex_;

template<typename T>
std::unordered_map<std::string, std::any> ModernSingleton<T>::metadata_;

template<typename T>
std::mutex ModernSingleton<T>::metadata_mutex_;
```

### Пример использования
```cpp
class DatabaseManager : public ModernSingleton<DatabaseManager> {
private:
    std::string connection_string_;
    bool is_connected_;
    
public:
    DatabaseManager() : is_connected_(false) {
        // Инициализация из конфигурации
        if (auto host = getConfig<std::string>("db.host")) {
            connection_string_ = *host;
        }
    }
    
    void connect() {
        if (!is_connected_) {
            std::cout << "Connecting to: " << connection_string_ << std::endl;
            is_connected_ = true;
        }
    }
    
    void disconnect() {
        if (is_connected_) {
            std::cout << "Disconnecting from database" << std::endl;
            is_connected_ = false;
        }
    }
    
    bool isConnected() const {
        return is_connected_;
    }
};

int main() {
    // Настройка конфигурации
    DatabaseManager::setConfig("db.host", std::string("localhost:5432"));
    DatabaseManager::setConfig("db.timeout", 30);
    DatabaseManager::setConfig("db.ssl", true);
    
    // Настройка метаданных
    DatabaseManager::setMetadata("version", std::string("1.0.0"));
    DatabaseManager::setMetadata("created_at", std::chrono::system_clock::now());
    
    // Использование
    auto& db = DatabaseManager::getInstance();
    db.connect();
    
    // Получение конфигурации
    if (auto timeout = DatabaseManager::getConfig<int>("db.timeout")) {
        std::cout << "Timeout: " << *timeout << " seconds" << std::endl;
    }
    
    // Получение метаданных
    if (auto version = DatabaseManager::getMetadata<std::string>("version")) {
        std::cout << "Version: " << *version << std::endl;
    }
    
    return 0;
}
```

---

## 🎯 Критерии оценки

### Отлично (5 баллов)
- ✅ Все требования выполнены
- ✅ Thread-safe реализация
- ✅ Современные C++17/20 возможности
- ✅ Правильное управление ресурсами
- ✅ Хорошая тестируемость
- ✅ Документация и комментарии

### Хорошо (4 балла)
- ✅ Большинство требований выполнено
- ✅ Thread-safe реализация
- ✅ Использованы современные возможности
- ✅ Код работает корректно

### Удовлетворительно (3 балла)
- ✅ Основные требования выполнены
- ✅ Код компилируется и работает
- ❌ Не все современные возможности использованы
- ❌ Возможны проблемы с thread-safety

### Неудовлетворительно (2 балла)
- ❌ Код не компилируется или работает неправильно
- ❌ Не выполнены основные требования
- ❌ Проблемы с управлением памятью

---

## 💡 Подсказки

1. **Thread Safety**: Используйте `std::once_flag` и `std::call_once` для безопасной инициализации
2. **RAII**: Помните о правильном порядке уничтожения объектов
3. **Тестирование**: Создавайте возможность сброса состояния для тестов
4. **Альтернативы**: Рассмотрите dependency injection как альтернативу Singleton
5. **Производительность**: Измеряйте производительность разных подходов

---

## 🚨 Важные замечания

### Когда НЕ использовать Singleton:
- ❌ Когда нужна гибкость и тестируемость
- ❌ Когда объект должен иметь состояние
- ❌ Когда нужна возможность замены реализации
- ❌ В многопоточных приложениях без тщательного планирования

### Когда использовать Singleton:
- ✅ Для логгеров и утилит
- ✅ Для кэшей и пулов ресурсов
- ✅ Для глобальных настроек приложения
- ✅ Когда нужен единственный экземпляр на всю программу

---

*Помните: Singleton - это мощный паттерн, но используйте его осторожно и всегда рассматривайте альтернативы!*
