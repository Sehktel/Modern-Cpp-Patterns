# 🏋️ Упражнения: Factory Method Pattern

## 📋 Задание 1: Modern Factory Method с Templates

### Описание
Создайте современную реализацию Factory Method паттерна, используя C++17 возможности:

### Требования
1. ✅ Используйте `std::variant` для разных типов продуктов
2. ✅ Примените `std::optional` для обработки ошибок
3. ✅ Используйте `auto` и `decltype` для вывода типов
4. ✅ Добавьте поддержку variadic templates
5. ✅ Реализуйте type-safe создание объектов

### Современная архитектура
```cpp
#include <memory>
#include <variant>
#include <optional>
#include <unordered_map>
#include <functional>
#include <string>

// Базовый класс продукта
class Product {
public:
    virtual ~Product() = default;
    virtual void use() = 0;
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
};

// Конкретные продукты
class ConcreteProductA : public Product {
public:
    void use() override {
        std::cout << "Using ConcreteProductA" << std::endl;
    }
    
    std::string getName() const override {
        return "ProductA";
    }
    
    std::string getDescription() const override {
        return "High-quality product A";
    }
};

class ConcreteProductB : public Product {
public:
    void use() override {
        std::cout << "Using ConcreteProductB" << std::endl;
    }
    
    std::string getName() const override {
        return "ProductB";
    }
    
    std::string getDescription() const override {
        return "Premium product B";
    }
};

// Современная фабрика с использованием variant
class ModernFactory {
private:
    using ProductVariant = std::variant<
        std::unique_ptr<ConcreteProductA>,
        std::unique_ptr<ConcreteProductB>
    >;
    
    using CreatorFunction = std::function<ProductVariant()>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    // Регистрация создателя
    template<typename T>
    void registerCreator(const std::string& type) {
        creators_[type] = []() -> ProductVariant {
            return std::make_unique<T>();
        };
    }
    
    // Создание продукта
    std::optional<ProductVariant> createProduct(const std::string& type) {
        auto it = creators_.find(type);
        if (it != creators_.end()) {
            return it->second();
        }
        return std::nullopt;
    }
    
    // Получение списка доступных типов
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
    ModernFactory factory;
    
    // Регистрируем создателей
    factory.registerCreator<ConcreteProductA>("type_a");
    factory.registerCreator<ConcreteProductB>("type_b");
    
    // Создаем продукты
    if (auto product = factory.createProduct("type_a")) {
        std::visit([](auto& p) {
            p->use();
            std::cout << "Name: " << p->getName() << std::endl;
            std::cout << "Description: " << p->getDescription() << std::endl;
        }, *product);
    }
    
    // Показываем доступные типы
    auto types = factory.getAvailableTypes();
    std::cout << "Available types: ";
    for (const auto& type : types) {
        std::cout << type << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

---

## 📋 Задание 2: Parameterized Factory Method

### Описание
Создайте фабрику, которая может создавать объекты с различными параметрами инициализации:

### Требования
1. **Flexible Parameters**: Поддержка разных типов параметров
2. **Type Safety**: Проверка типов на этапе компиляции
3. **Error Handling**: Обработка ошибок создания
4. **Validation**: Валидация параметров перед созданием

### Реализация
```cpp
#include <any>
#include <type_traits>
#include <stdexcept>

// Параметры для создания объектов
struct ProductParams {
    std::string name;
    int quality;
    double price;
    bool isPremium;
    
    ProductParams(const std::string& n, int q, double p, bool premium)
        : name(n), quality(q), price(p), isPremium(premium) {}
};

// Базовый класс с параметрами
class ParameterizedProduct : public Product {
protected:
    ProductParams params_;
    
public:
    ParameterizedProduct(const ProductParams& params) : params_(params) {}
    
    virtual void validateParams() const {
        if (params_.name.empty()) {
            throw std::invalid_argument("Product name cannot be empty");
        }
        if (params_.quality < 1 || params_.quality > 10) {
            throw std::invalid_argument("Quality must be between 1 and 10");
        }
        if (params_.price < 0) {
            throw std::invalid_argument("Price cannot be negative");
        }
    }
    
    std::string getName() const override {
        return params_.name;
    }
};

// Конкретные продукты с параметрами
class QualityProduct : public ParameterizedProduct {
public:
    QualityProduct(const ProductParams& params) : ParameterizedProduct(params) {
        validateParams();
    }
    
    void use() override {
        std::cout << "Using " << params_.name 
                  << " (Quality: " << params_.quality 
                  << ", Price: $" << params_.price << ")" << std::endl;
    }
    
    std::string getDescription() const override {
        return "Quality product with parameters";
    }
};

class PremiumProduct : public ParameterizedProduct {
public:
    PremiumProduct(const ProductParams& params) : ParameterizedProduct(params) {
        validateParams();
        if (!params_.isPremium) {
            throw std::invalid_argument("Premium product must have premium flag set");
        }
    }
    
    void use() override {
        std::cout << "Using PREMIUM " << params_.name 
                  << " (Quality: " << params_.quality 
                  << ", Price: $" << params_.price << ")" << std::endl;
    }
    
    std::string getDescription() const override {
        return "Premium product with enhanced features";
    }
};

// Параметризованная фабрика
class ParameterizedFactory {
private:
    using CreatorFunction = std::function<std::unique_ptr<Product>(const ProductParams&)>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    // Регистрация создателя
    template<typename T>
    void registerCreator(const std::string& type) {
        creators_[type] = [](const ProductParams& params) -> std::unique_ptr<Product> {
            return std::make_unique<T>(params);
        };
    }
    
    // Создание продукта с параметрами
    std::optional<std::unique_ptr<Product>> createProduct(
        const std::string& type, 
        const ProductParams& params
    ) {
        try {
            auto it = creators_.find(type);
            if (it != creators_.end()) {
                return it->second(params);
            }
            return std::nullopt;
        } catch (const std::exception& e) {
            std::cerr << "Error creating product: " << e.what() << std::endl;
            return std::nullopt;
        }
    }
    
    // Создание продукта с валидацией
    std::optional<std::unique_ptr<Product>> createProductSafe(
        const std::string& type,
        const ProductParams& params
    ) {
        // Предварительная валидация
        if (type.empty()) {
            std::cerr << "Product type cannot be empty" << std::endl;
            return std::nullopt;
        }
        
        if (creators_.find(type) == creators_.end()) {
            std::cerr << "Unknown product type: " << type << std::endl;
            return std::nullopt;
        }
        
        return createProduct(type, params);
    }
};
```

### Пример использования
```cpp
int main() {
    ParameterizedFactory factory;
    
    // Регистрируем создателей
    factory.registerCreator<QualityProduct>("quality");
    factory.registerCreator<PremiumProduct>("premium");
    
    // Создаем продукты с параметрами
    ProductParams params1("Widget A", 8, 99.99, false);
    ProductParams params2("Premium Widget", 10, 199.99, true);
    
    // Создание обычного продукта
    if (auto product = factory.createProductSafe("quality", params1)) {
        (*product)->use();
        std::cout << "Description: " << (*product)->getDescription() << std::endl;
    }
    
    // Создание премиум продукта
    if (auto product = factory.createProductSafe("premium", params2)) {
        (*product)->use();
        std::cout << "Description: " << (*product)->getDescription() << std::endl;
    }
    
    // Тест с неверными параметрами
    ProductParams invalidParams("", -1, -50.0, false);
    if (auto product = factory.createProductSafe("quality", invalidParams)) {
        // Не должно выполниться
    } else {
        std::cout << "Invalid parameters rejected" << std::endl;
    }
    
    return 0;
}
```

---

## 📋 Задание 3: Abstract Factory Method

### Описание
Создайте систему Abstract Factory для создания семейств связанных объектов:

### Требования
1. **Family of Products**: Создание связанных продуктов
2. **Multiple Factories**: Разные фабрики для разных семейств
3. **Consistency**: Гарантия совместимости продуктов в семействе
4. **Extensibility**: Легкое добавление новых семейств

### Архитектура
```cpp
// Абстрактные продукты
class Button {
public:
    virtual ~Button() = default;
    virtual void render() = 0;
    virtual void onClick() = 0;
};

class TextBox {
public:
    virtual ~TextBox() = default;
    virtual void render() = 0;
    virtual void setText(const std::string& text) = 0;
    virtual std::string getText() const = 0;
};

class Window {
public:
    virtual ~Window() = default;
    virtual void render() = 0;
    virtual void addComponent(std::unique_ptr<Button> button) = 0;
    virtual void addComponent(std::unique_ptr<TextBox> textBox) = 0;
};

// Абстрактная фабрика
class UIFactory {
public:
    virtual ~UIFactory() = default;
    virtual std::unique_ptr<Button> createButton() = 0;
    virtual std::unique_ptr<TextBox> createTextBox() = 0;
    virtual std::unique_ptr<Window> createWindow() = 0;
};

// Windows семейство
class WindowsButton : public Button {
public:
    void render() override {
        std::cout << "Rendering Windows button" << std::endl;
    }
    
    void onClick() override {
        std::cout << "Windows button clicked" << std::endl;
    }
};

class WindowsTextBox : public TextBox {
private:
    std::string text_;
    
public:
    void render() override {
        std::cout << "Rendering Windows textbox with text: " << text_ << std::endl;
    }
    
    void setText(const std::string& text) override {
        text_ = text;
    }
    
    std::string getText() const override {
        return text_;
    }
};

class WindowsWindow : public Window {
private:
    std::vector<std::unique_ptr<Button>> buttons_;
    std::vector<std::unique_ptr<TextBox>> textBoxes_;
    
public:
    void render() override {
        std::cout << "Rendering Windows window" << std::endl;
        for (auto& button : buttons_) {
            button->render();
        }
        for (auto& textBox : textBoxes_) {
            textBox->render();
        }
    }
    
    void addComponent(std::unique_ptr<Button> button) override {
        buttons_.push_back(std::move(button));
    }
    
    void addComponent(std::unique_ptr<TextBox> textBox) override {
        textBoxes_.push_back(std::move(textBox));
    }
};

class WindowsUIFactory : public UIFactory {
public:
    std::unique_ptr<Button> createButton() override {
        return std::make_unique<WindowsButton>();
    }
    
    std::unique_ptr<TextBox> createTextBox() override {
        return std::make_unique<WindowsTextBox>();
    }
    
    std::unique_ptr<Window> createWindow() override {
        return std::make_unique<WindowsWindow>();
    }
};

// macOS семейство
class MacOSButton : public Button {
public:
    void render() override {
        std::cout << "Rendering macOS button" << std::endl;
    }
    
    void onClick() override {
        std::cout << "macOS button clicked" << std::endl;
    }
};

class MacOSTextBox : public TextBox {
private:
    std::string text_;
    
public:
    void render() override {
        std::cout << "Rendering macOS textbox with text: " << text_ << std::endl;
    }
    
    void setText(const std::string& text) override {
        text_ = text;
    }
    
    std::string getText() const override {
        return text_;
    }
};

class MacOSWindow : public Window {
private:
    std::vector<std::unique_ptr<Button>> buttons_;
    std::vector<std::unique_ptr<TextBox>> textBoxes_;
    
public:
    void render() override {
        std::cout << "Rendering macOS window" << std::endl;
        for (auto& button : buttons_) {
            button->render();
        }
        for (auto& textBox : textBoxes_) {
            textBox->render();
        }
    }
    
    void addComponent(std::unique_ptr<Button> button) override {
        buttons_.push_back(std::move(button));
    }
    
    void addComponent(std::unique_ptr<TextBox> textBox) override {
        textBoxes_.push_back(std::move(textBox));
    }
};

class MacOSUIFactory : public UIFactory {
public:
    std::unique_ptr<Button> createButton() override {
        return std::make_unique<MacOSButton>();
    }
    
    std::unique_ptr<TextBox> createTextBox() override {
        return std::make_unique<MacOSTextBox>();
    }
    
    std::unique_ptr<Window> createWindow() override {
        return std::make_unique<MacOSWindow>();
    }
};

// Фабрика фабрик
class UIFactoryFactory {
public:
    static std::unique_ptr<UIFactory> createFactory(const std::string& platform) {
        if (platform == "windows") {
            return std::make_unique<WindowsUIFactory>();
        } else if (platform == "macos") {
            return std::make_unique<MacOSUIFactory>();
        }
        return nullptr;
    }
    
    static std::vector<std::string> getSupportedPlatforms() {
        return {"windows", "macos"};
    }
};
```

### Пример использования
```cpp
class Application {
private:
    std::unique_ptr<UIFactory> factory_;
    
public:
    Application(const std::string& platform) {
        factory_ = UIFactoryFactory::createFactory(platform);
        if (!factory_) {
            throw std::invalid_argument("Unsupported platform: " + platform);
        }
    }
    
    void createUI() {
        // Создаем окно
        auto window = factory_->createWindow();
        
        // Создаем кнопку
        auto button = factory_->createButton();
        button->onClick();
        
        // Создаем текстовое поле
        auto textBox = factory_->createTextBox();
        textBox->setText("Hello, World!");
        
        // Добавляем компоненты в окно
        window->addComponent(std::move(button));
        window->addComponent(std::move(textBox));
        
        // Рендерим окно
        window->render();
    }
};

int main() {
    // Создаем приложение для Windows
    Application windowsApp("windows");
    windowsApp.createUI();
    
    std::cout << "\n---\n" << std::endl;
    
    // Создаем приложение для macOS
    Application macApp("macos");
    macApp.createUI();
    
    // Показываем поддерживаемые платформы
    auto platforms = UIFactoryFactory::getSupportedPlatforms();
    std::cout << "\nSupported platforms: ";
    for (const auto& platform : platforms) {
        std::cout << platform << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

---

## 📋 Задание 4: Factory Method с Dependency Injection

### Описание
Создайте фабрику, которая поддерживает dependency injection и может быть легко тестируемой:

### Требования
1. **Dependency Injection**: Внедрение зависимостей через конструктор
2. **Testability**: Возможность замены зависимостей для тестов
3. **Configuration**: Настройка фабрики через конфигурацию
4. **Lifecycle Management**: Управление жизненным циклом создаваемых объектов

### Реализация
```cpp
#include <memory>
#include <functional>
#include <unordered_map>

// Интерфейсы для зависимостей
class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(const std::string& message) = 0;
};

class IConfiguration {
public:
    virtual ~IConfiguration() = default;
    virtual std::string getValue(const std::string& key) const = 0;
    virtual void setValue(const std::string& key, const std::string& value) = 0;
};

class ICache {
public:
    virtual ~ICache() = default;
    virtual void put(const std::string& key, const std::string& value) = 0;
    virtual std::optional<std::string> get(const std::string& key) const = 0;
};

// Продукт с зависимостями
class Service {
private:
    std::shared_ptr<ILogger> logger_;
    std::shared_ptr<IConfiguration> config_;
    std::shared_ptr<ICache> cache_;
    std::string name_;
    
public:
    Service(
        const std::string& name,
        std::shared_ptr<ILogger> logger,
        std::shared_ptr<IConfiguration> config,
        std::shared_ptr<ICache> cache
    ) : name_(name), logger_(logger), config_(config), cache_(cache) {
        logger_->log("Service " + name_ + " created");
    }
    
    void doWork() {
        logger_->log("Service " + name_ + " doing work");
        
        // Используем конфигурацию
        auto timeout = config_->getValue("timeout");
        logger_->log("Timeout: " + timeout);
        
        // Используем кэш
        cache_->put("last_work", "completed");
    }
    
    std::string getName() const {
        return name_;
    }
};

// Конфигурация фабрики
struct FactoryConfig {
    std::string defaultLoggerType;
    std::string defaultConfigType;
    std::string defaultCacheType;
    std::unordered_map<std::string, std::string> serviceConfigs;
};

// Фабрика с dependency injection
class DIFactory {
private:
    FactoryConfig config_;
    
    // Реестр создателей зависимостей
    std::unordered_map<std::string, std::function<std::shared_ptr<ILogger>()>> loggerCreators_;
    std::unordered_map<std::string, std::function<std::shared_ptr<IConfiguration>()>> configCreators_;
    std::unordered_map<std::string, std::function<std::shared_ptr<ICache>()>> cacheCreators_;
    
    // Кэш созданных зависимостей
    std::shared_ptr<ILogger> logger_;
    std::shared_ptr<IConfiguration> config_;
    std::shared_ptr<ICache> cache_;
    
public:
    DIFactory(const FactoryConfig& config) : config_(config) {}
    
    // Регистрация создателей зависимостей
    void registerLoggerCreator(const std::string& type, std::function<std::shared_ptr<ILogger>()> creator) {
        loggerCreators_[type] = creator;
    }
    
    void registerConfigCreator(const std::string& type, std::function<std::shared_ptr<IConfiguration>()> creator) {
        configCreators_[type] = creator;
    }
    
    void registerCacheCreator(const std::string& type, std::function<std::shared_ptr<ICache>()> creator) {
        cacheCreators_[type] = creator;
    }
    
    // Создание зависимостей
    std::shared_ptr<ILogger> createLogger(const std::string& type = "") {
        std::string loggerType = type.empty() ? config_.defaultLoggerType : type;
        auto it = loggerCreators_.find(loggerType);
        if (it != loggerCreators_.end()) {
            return it->second();
        }
        throw std::runtime_error("Unknown logger type: " + loggerType);
    }
    
    std::shared_ptr<IConfiguration> createConfiguration(const std::string& type = "") {
        std::string configType = type.empty() ? config_.defaultConfigType : type;
        auto it = configCreators_.find(configType);
        if (it != configCreators_.end()) {
            return it->second();
        }
        throw std::runtime_error("Unknown config type: " + configType);
    }
    
    std::shared_ptr<ICache> createCache(const std::string& type = "") {
        std::string cacheType = type.empty() ? config_.defaultCacheType : type;
        auto it = cacheCreators_.find(cacheType);
        if (it != cacheCreators_.end()) {
            return it->second();
        }
        throw std::runtime_error("Unknown cache type: " + cacheType);
    }
    
    // Создание сервиса с автоматическим внедрением зависимостей
    std::unique_ptr<Service> createService(const std::string& name) {
        // Получаем или создаем зависимости
        if (!logger_) {
            logger_ = createLogger();
        }
        if (!config_) {
            config_ = createConfiguration();
        }
        if (!cache_) {
            cache_ = createCache();
        }
        
        // Настраиваем конфигурацию для сервиса
        auto serviceConfigIt = config_.serviceConfigs.find(name);
        if (serviceConfigIt != config_.serviceConfigs.end()) {
            config_->setValue("service.name", name);
            config_->setValue("service.type", serviceConfigIt->second);
        }
        
        return std::make_unique<Service>(name, logger_, config_, cache_);
    }
    
    // Создание сервиса с кастомными зависимостями
    std::unique_ptr<Service> createService(
        const std::string& name,
        std::shared_ptr<ILogger> logger,
        std::shared_ptr<IConfiguration> config,
        std::shared_ptr<ICache> cache
    ) {
        return std::make_unique<Service>(name, logger, config, cache);
    }
    
    // Сброс для тестов
    void reset() {
        logger_.reset();
        config_.reset();
        cache_.reset();
    }
};
```

### Конкретные реализации зависимостей
```cpp
// Реализации для продакшена
class ConsoleLogger : public ILogger {
public:
    void log(const std::string& message) override {
        std::cout << "[LOG] " << message << std::endl;
    }
};

class FileConfiguration : public IConfiguration {
private:
    std::unordered_map<std::string, std::string> values_;
    
public:
    FileConfiguration() {
        // Загружаем конфигурацию из файла
        values_["timeout"] = "30";
        values_["retries"] = "3";
    }
    
    std::string getValue(const std::string& key) const override {
        auto it = values_.find(key);
        return it != values_.end() ? it->second : "";
    }
    
    void setValue(const std::string& key, const std::string& value) override {
        values_[key] = value;
    }
};

class MemoryCache : public ICache {
private:
    std::unordered_map<std::string, std::string> cache_;
    
public:
    void put(const std::string& key, const std::string& value) override {
        cache_[key] = value;
    }
    
    std::optional<std::string> get(const std::string& key) const override {
        auto it = cache_.find(key);
        return it != cache_.end() ? std::optional<std::string>(it->second) : std::nullopt;
    }
};

// Mock реализации для тестов
class MockLogger : public ILogger {
private:
    std::vector<std::string> logs_;
    
public:
    void log(const std::string& message) override {
        logs_.push_back(message);
    }
    
    const std::vector<std::string>& getLogs() const {
        return logs_;
    }
    
    void clearLogs() {
        logs_.clear();
    }
};

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
    
    void setTestValue(const std::string& key, const std::string& value) {
        values_[key] = value;
    }
};

class MockCache : public ICache {
private:
    std::unordered_map<std::string, std::string> cache_;
    
public:
    void put(const std::string& key, const std::string& value) override {
        cache_[key] = value;
    }
    
    std::optional<std::string> get(const std::string& key) const override {
        auto it = cache_.find(key);
        return it != cache_.end() ? std::optional<std::string>(it->second) : std::nullopt;
    }
    
    bool hasKey(const std::string& key) const {
        return cache_.find(key) != cache_.end();
    }
};
```

### Пример использования
```cpp
void setupProductionFactory(DIFactory& factory) {
    // Регистрируем продакшн реализации
    factory.registerLoggerCreator("console", []() {
        return std::make_shared<ConsoleLogger>();
    });
    
    factory.registerConfigCreator("file", []() {
        return std::make_shared<FileConfiguration>();
    });
    
    factory.registerCacheCreator("memory", []() {
        return std::make_shared<MemoryCache>();
    });
}

void setupTestFactory(DIFactory& factory) {
    // Регистрируем mock реализации
    factory.registerLoggerCreator("mock", []() {
        return std::make_shared<MockLogger>();
    });
    
    factory.registerConfigCreator("mock", []() {
        return std::make_shared<MockConfiguration>();
    });
    
    factory.registerCacheCreator("mock", []() {
        return std::make_shared<MockCache>();
    });
}

int main() {
    // Настройка конфигурации
    FactoryConfig config;
    config.defaultLoggerType = "console";
    config.defaultConfigType = "file";
    config.defaultCacheType = "memory";
    config.serviceConfigs["user_service"] = "user";
    config.serviceConfigs["order_service"] = "order";
    
    DIFactory factory(config);
    setupProductionFactory(factory);
    
    // Создаем сервисы
    auto userService = factory.createService("user_service");
    auto orderService = factory.createService("order_service");
    
    // Используем сервисы
    userService->doWork();
    orderService->doWork();
    
    // Тестирование с mock объектами
    factory.reset();
    setupTestFactory(factory);
    
    auto mockLogger = std::make_shared<MockLogger>();
    auto mockConfig = std::make_shared<MockConfiguration>();
    auto mockCache = std::make_shared<MockCache>();
    
    mockConfig->setTestValue("timeout", "60");
    
    auto testService = factory.createService("test_service", mockLogger, mockConfig, mockCache);
    testService->doWork();
    
    // Проверяем логи
    const auto& logs = mockLogger->getLogs();
    for (const auto& log : logs) {
        std::cout << "Test log: " << log << std::endl;
    }
    
    return 0;
}
```

---

## 📋 Задание 5: Factory Method Performance Analysis

### Описание
Проанализируйте производительность различных реализаций Factory Method и оптимизируйте их:

### Требования
1. **Performance Measurement**: Измерение времени создания объектов
2. **Memory Usage**: Анализ использования памяти
3. **Optimization**: Оптимизация критических путей
4. **Comparison**: Сравнение разных подходов

### Бенчмарки
```cpp
#include <chrono>
#include <vector>
#include <random>

class PerformanceTest {
private:
    std::mt19937 rng_;
    
public:
    PerformanceTest() : rng_(std::random_device{}()) {}
    
    // Тест производительности создания объектов
    void testObjectCreationPerformance() {
        const int iterations = 1000000;
        
        // Тест 1: Прямое создание
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            auto obj = std::make_unique<ConcreteProductA>();
            (void)obj; // Подавляем предупреждение о неиспользуемой переменной
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto directTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Тест 2: Создание через фабрику
        ModernFactory factory;
        factory.registerCreator<ConcreteProductA>("type_a");
        
        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            auto obj = factory.createProduct("type_a");
            (void)obj;
        }
        end = std::chrono::high_resolution_clock::now();
        auto factoryTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Direct creation: " << directTime.count() << " microseconds" << std::endl;
        std::cout << "Factory creation: " << factoryTime.count() << " microseconds" << std::endl;
        std::cout << "Overhead: " << (factoryTime.count() - directTime.count()) << " microseconds" << std::endl;
    }
    
    // Тест производительности с разными типами
    void testMultiTypePerformance() {
        const int iterations = 100000;
        
        ModernFactory factory;
        factory.registerCreator<ConcreteProductA>("type_a");
        factory.registerCreator<ConcreteProductB>("type_b");
        
        std::vector<std::string> types = {"type_a", "type_b"};
        std::uniform_int_distribution<int> dist(0, types.size() - 1);
        
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            std::string type = types[dist(rng_)];
            auto obj = factory.createProduct(type);
            (void)obj;
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Multi-type creation: " << time.count() << " microseconds" << std::endl;
    }
    
    // Тест производительности с параметрами
    void testParameterizedPerformance() {
        const int iterations = 100000;
        
        ParameterizedFactory factory;
        factory.registerCreator<QualityProduct>("quality");
        
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            ProductParams params("Product " + std::to_string(i), 5, 99.99, false);
            auto obj = factory.createProduct("quality", params);
            (void)obj;
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Parameterized creation: " << time.count() << " microseconds" << std::endl;
    }
    
    // Тест производительности Abstract Factory
    void testAbstractFactoryPerformance() {
        const int iterations = 100000;
        
        auto windowsFactory = std::make_unique<WindowsUIFactory>();
        
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            auto button = windowsFactory->createButton();
            auto textBox = windowsFactory->createTextBox();
            auto window = windowsFactory->createWindow();
            (void)button; (void)textBox; (void)window;
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Abstract factory creation: " << time.count() << " microseconds" << std::endl;
    }
    
    // Тест производительности DI Factory
    void testDIFactoryPerformance() {
        const int iterations = 10000; // Меньше итераций из-за сложности
        
        FactoryConfig config;
        config.defaultLoggerType = "console";
        config.defaultConfigType = "file";
        config.defaultCacheType = "memory";
        
        DIFactory factory(config);
        setupProductionFactory(factory);
        
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            auto service = factory.createService("service_" + std::to_string(i));
            (void)service;
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "DI factory creation: " << time.count() << " microseconds" << std::endl;
    }
    
    void runAllTests() {
        std::cout << "=== Factory Method Performance Tests ===" << std::endl;
        
        std::cout << "\n1. Object Creation Performance:" << std::endl;
        testObjectCreationPerformance();
        
        std::cout << "\n2. Multi-Type Performance:" << std::endl;
        testMultiTypePerformance();
        
        std::cout << "\n3. Parameterized Performance:" << std::endl;
        testParameterizedPerformance();
        
        std::cout << "\n4. Abstract Factory Performance:" << std::endl;
        testAbstractFactoryPerformance();
        
        std::cout << "\n5. DI Factory Performance:" << std::endl;
        testDIFactoryPerformance();
    }
};
```

### Оптимизированная фабрика
```cpp
// Оптимизированная фабрика с кэшированием
template<typename T>
class OptimizedFactory {
private:
    using CreatorFunction = std::function<std::unique_ptr<T>()>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
    // Кэш для часто создаваемых объектов
    std::unordered_map<std::string, std::vector<std::unique_ptr<T>>> objectPool_;
    static constexpr size_t POOL_SIZE = 100;
    
public:
    void registerCreator(const std::string& type, CreatorFunction creator) {
        creators_[type] = creator;
        // Инициализируем пул объектов
        objectPool_[type].reserve(POOL_SIZE);
    }
    
    std::unique_ptr<T> createProduct(const std::string& type) {
        auto& pool = objectPool_[type];
        
        // Если есть объект в пуле, используем его
        if (!pool.empty()) {
            auto obj = std::move(pool.back());
            pool.pop_back();
            return obj;
        }
        
        // Иначе создаем новый
        auto it = creators_.find(type);
        if (it != creators_.end()) {
            return it->second();
        }
        return nullptr;
    }
    
    // Возвращаем объект в пул
    void returnToPool(const std::string& type, std::unique_ptr<T> obj) {
        auto& pool = objectPool_[type];
        if (pool.size() < POOL_SIZE) {
            pool.push_back(std::move(obj));
        }
    }
    
    // Статистика пула
    void printPoolStats() const {
        for (const auto& pair : objectPool_) {
            std::cout << "Pool " << pair.first << ": " << pair.second.size() << " objects" << std::endl;
        }
    }
};
```

---

## 🎯 Критерии оценки

### Отлично (5 баллов)
- ✅ Все требования выполнены
- ✅ Современные C++17/20 возможности
- ✅ Thread-safe реализация
- ✅ Хорошая производительность
- ✅ Полная документация и тесты
- ✅ Анализ производительности

### Хорошо (4 балла)
- ✅ Большинство требований выполнено
- ✅ Современные возможности использованы
- ✅ Код работает корректно
- ✅ Есть тесты

### Удовлетворительно (3 балла)
- ✅ Основные требования выполнены
- ✅ Код компилируется и работает
- ❌ Не все современные возможности использованы
- ❌ Ограниченное тестирование

### Неудовлетворительно (2 балла)
- ❌ Код не компилируется или работает неправильно
- ❌ Не выполнены основные требования
- ❌ Использованы устаревшие подходы

---

## 💡 Подсказки

1. **Templates**: Используйте templates для type-safe создания объектов
2. **Variadic Templates**: Применяйте для гибкости параметров
3. **std::variant**: Используйте для type-safe хранения разных типов
4. **Performance**: Измеряйте производительность разных подходов
5. **Testing**: Создавайте mock объекты для тестирования

---

## 🚨 Важные замечания

### Преимущества Factory Method:
- ✅ Инкапсуляция логики создания
- ✅ Легкость добавления новых типов
- ✅ Централизованное управление созданием
- ✅ Возможность кэширования и оптимизации

### Недостатки Factory Method:
- ❌ Дополнительная сложность кода
- ❌ Overhead на создание объектов
- ❌ Необходимость регистрации создателей
- ❌ Сложность отладки

---

*Помните: Factory Method - это мощный паттерн для создания объектов, но используйте его там, где это действительно необходимо!*
