# 🏋️ Упражнения: Decorator Pattern

## 📋 Задание 1: Modern Decorator с Templates

### Описание
Создайте современную реализацию Decorator паттерна с использованием C++17:

### Требования
1. ✅ Используйте `std::variant` для разных типов декораторов
2. ✅ Примените `std::optional` для обработки ошибок
3. ✅ Используйте `auto` и `decltype` для вывода типов
4. ✅ Добавьте поддержку variadic templates
5. ✅ Реализуйте type-safe декорирование

### Современная архитектура
```cpp
#include <memory>
#include <variant>
#include <optional>
#include <unordered_map>
#include <functional>
#include <string>

// Базовый компонент
class Component {
public:
    virtual ~Component() = default;
    virtual std::string operation() const = 0;
    virtual double getCost() const = 0;
    virtual std::string getDescription() const = 0;
};

// Конкретный компонент
class ConcreteComponent : public Component {
private:
    std::string name_;
    double cost_;
    
public:
    ConcreteComponent(const std::string& name, double cost) 
        : name_(name), cost_(cost) {}
    
    std::string operation() const override {
        return "Basic " + name_;
    }
    
    double getCost() const override {
        return cost_;
    }
    
    std::string getDescription() const override {
        return name_;
    }
};

// Базовый декоратор
class Decorator : public Component {
protected:
    std::unique_ptr<Component> component_;
    
public:
    Decorator(std::unique_ptr<Component> component) 
        : component_(std::move(component)) {}
    
    std::string operation() const override {
        return component_ ? component_->operation() : "No component";
    }
    
    double getCost() const override {
        return component_ ? component_->getCost() : 0.0;
    }
    
    std::string getDescription() const override {
        return component_ ? component_->getDescription() : "No component";
    }
};

// Конкретные декораторы
class ConcreteDecoratorA : public Decorator {
private:
    std::string feature_;
    double additionalCost_;
    
public:
    ConcreteDecoratorA(std::unique_ptr<Component> component, 
                       const std::string& feature, double cost)
        : Decorator(std::move(component)), feature_(feature), additionalCost_(cost) {}
    
    std::string operation() const override {
        return Decorator::operation() + " with " + feature_;
    }
    
    double getCost() const override {
        return Decorator::getCost() + additionalCost_;
    }
    
    std::string getDescription() const override {
        return Decorator::getDescription() + " + " + feature_;
    }
};

class ConcreteDecoratorB : public Decorator {
private:
    std::string enhancement_;
    double additionalCost_;
    
public:
    ConcreteDecoratorB(std::unique_ptr<Component> component, 
                       const std::string& enhancement, double cost)
        : Decorator(std::move(component)), enhancement_(enhancement), additionalCost_(cost) {}
    
    std::string operation() const override {
        return Decorator::operation() + " enhanced with " + enhancement_;
    }
    
    double getCost() const override {
        return Decorator::getCost() + additionalCost_;
    }
    
    std::string getDescription() const override {
        return Decorator::getDescription() + " + " + enhancement_;
    }
};

// Фабрика декораторов
class DecoratorFactory {
private:
    using DecoratorVariant = std::variant<
        std::unique_ptr<ConcreteDecoratorA>,
        std::unique_ptr<ConcreteDecoratorB>
    >;
    
    using CreatorFunction = std::function<DecoratorVariant(std::unique_ptr<Component>)>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerDecorator(const std::string& type) {
        if (type == "feature") {
            creators_[type] = [](std::unique_ptr<Component> component) -> DecoratorVariant {
                return std::make_unique<ConcreteDecoratorA>(
                    std::move(component), "Premium Feature", 50.0
                );
            };
        } else if (type == "enhancement") {
            creators_[type] = [](std::unique_ptr<Component> component) -> DecoratorVariant {
                return std::make_unique<ConcreteDecoratorB>(
                    std::move(component), "Advanced Enhancement", 100.0
                );
            };
        }
    }
    
    std::optional<DecoratorVariant> createDecorator(
        const std::string& type, 
        std::unique_ptr<Component> component
    ) {
        auto it = creators_.find(type);
        if (it != creators_.end()) {
            return it->second(std::move(component));
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
    DecoratorFactory factory;
    
    // Регистрируем декораторы
    factory.registerDecorator("feature");
    factory.registerDecorator("enhancement");
    
    // Создаем базовый компонент
    auto component = std::make_unique<ConcreteComponent>("Basic Widget", 100.0);
    
    // Декорируем компонент
    if (auto decorator = factory.createDecorator("feature", std::move(component))) {
        std::visit([](auto& d) {
            std::cout << "Operation: " << d->operation() << std::endl;
            std::cout << "Cost: $" << d->getCost() << std::endl;
            std::cout << "Description: " << d->getDescription() << std::endl;
        }, *decorator);
    }
    
    return 0;
}
```

---

## 📋 Задание 2: Multi-Level Decorator System

### Описание
Создайте систему многоуровневых декораторов для создания сложных объектов:

### Требования
1. **Multiple Levels**: Поддержка множественных уровней декорирования
2. **Type Safety**: Проверка типов на этапе компиляции
3. **Error Handling**: Обработка ошибок декорирования
4. **Validation**: Валидация декораторов

### Реализация
```cpp
#include <any>
#include <type_traits>
#include <stdexcept>

// Базовый интерфейс для декорируемого объекта
class Decoratable {
public:
    virtual ~Decoratable() = default;
    virtual std::string getName() const = 0;
    virtual double getPrice() const = 0;
    virtual std::string getDescription() const = 0;
    virtual void addFeature(const std::string& feature) = 0;
    virtual std::vector<std::string> getFeatures() const = 0;
};

// Конкретный декорируемый объект
class Product : public Decoratable {
private:
    std::string name_;
    double basePrice_;
    std::vector<std::string> features_;
    
public:
    Product(const std::string& name, double price) 
        : name_(name), basePrice_(price) {}
    
    std::string getName() const override {
        return name_;
    }
    
    double getPrice() const override {
        return basePrice_;
    }
    
    std::string getDescription() const override {
        std::string desc = name_;
        if (!features_.empty()) {
            desc += " with features: ";
            for (size_t i = 0; i < features_.size(); ++i) {
                desc += features_[i];
                if (i < features_.size() - 1) desc += ", ";
            }
        }
        return desc;
    }
    
    void addFeature(const std::string& feature) override {
        features_.push_back(feature);
    }
    
    std::vector<std::string> getFeatures() const override {
        return features_;
    }
};

// Базовый декоратор
class ProductDecorator : public Decoratable {
protected:
    std::unique_ptr<Decoratable> product_;
    
public:
    ProductDecorator(std::unique_ptr<Decoratable> product) 
        : product_(std::move(product)) {}
    
    std::string getName() const override {
        return product_ ? product_->getName() : "No product";
    }
    
    double getPrice() const override {
        return product_ ? product_->getPrice() : 0.0;
    }
    
    std::string getDescription() const override {
        return product_ ? product_->getDescription() : "No product";
    }
    
    void addFeature(const std::string& feature) override {
        if (product_) {
            product_->addFeature(feature);
        }
    }
    
    std::vector<std::string> getFeatures() const override {
        return product_ ? product_->getFeatures() : std::vector<std::string>();
    }
};

// Конкретные декораторы
class WarrantyDecorator : public ProductDecorator {
private:
    int warrantyYears_;
    double warrantyCost_;
    
public:
    WarrantyDecorator(std::unique_ptr<Decoratable> product, int years, double cost)
        : ProductDecorator(std::move(product)), warrantyYears_(years), warrantyCost_(cost) {}
    
    std::string getName() const override {
        return ProductDecorator::getName() + " (Warranty)";
    }
    
    double getPrice() const override {
        return ProductDecorator::getPrice() + warrantyCost_;
    }
    
    std::string getDescription() const override {
        return ProductDecorator::getDescription() + 
               " + " + std::to_string(warrantyYears_) + " year warranty";
    }
    
    void addFeature(const std::string& feature) override {
        ProductDecorator::addFeature(feature);
    }
    
    std::vector<std::string> getFeatures() const override {
        auto features = ProductDecorator::getFeatures();
        features.push_back(std::to_string(warrantyYears_) + " year warranty");
        return features;
    }
};

class InstallationDecorator : public ProductDecorator {
private:
    std::string installationType_;
    double installationCost_;
    
public:
    InstallationDecorator(std::unique_ptr<Decoratable> product, 
                          const std::string& type, double cost)
        : ProductDecorator(std::move(product)), installationType_(type), installationCost_(cost) {}
    
    std::string getName() const override {
        return ProductDecorator::getName() + " (Installation)";
    }
    
    double getPrice() const override {
        return ProductDecorator::getPrice() + installationCost_;
    }
    
    std::string getDescription() const override {
        return ProductDecorator::getDescription() + 
               " + " + installationType_ + " installation";
    }
    
    void addFeature(const std::string& feature) override {
        ProductDecorator::addFeature(feature);
    }
    
    std::vector<std::string> getFeatures() const override {
        auto features = ProductDecorator::getFeatures();
        features.push_back(installationType_ + " installation");
        return features;
    }
};

class MaintenanceDecorator : public ProductDecorator {
private:
    std::string maintenancePlan_;
    double maintenanceCost_;
    
public:
    MaintenanceDecorator(std::unique_ptr<Decoratable> product, 
                        const std::string& plan, double cost)
        : ProductDecorator(std::move(product)), maintenancePlan_(plan), maintenanceCost_(cost) {}
    
    std::string getName() const override {
        return ProductDecorator::getName() + " (Maintenance)";
    }
    
    double getPrice() const override {
        return ProductDecorator::getPrice() + maintenanceCost_;
    }
    
    std::string getDescription() const override {
        return ProductDecorator::getDescription() + 
               " + " + maintenancePlan_ + " maintenance";
    }
    
    void addFeature(const std::string& feature) override {
        ProductDecorator::addFeature(feature);
    }
    
    std::vector<std::string> getFeatures() const override {
        auto features = ProductDecorator::getFeatures();
        features.push_back(maintenancePlan_ + " maintenance");
        return features;
    }
};

// Фабрика декораторов
class MultiLevelDecoratorFactory {
private:
    using DecoratorVariant = std::variant<
        std::unique_ptr<WarrantyDecorator>,
        std::unique_ptr<InstallationDecorator>,
        std::unique_ptr<MaintenanceDecorator>
    >;
    
    using CreatorFunction = std::function<DecoratorVariant(std::unique_ptr<Decoratable>)>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerDecorator(const std::string& type) {
        if (type == "warranty") {
            creators_[type] = [](std::unique_ptr<Decoratable> product) -> DecoratorVariant {
                return std::make_unique<WarrantyDecorator>(
                    std::move(product), 2, 50.0
                );
            };
        } else if (type == "installation") {
            creators_[type] = [](std::unique_ptr<Decoratable> product) -> DecoratorVariant {
                return std::make_unique<InstallationDecorator>(
                    std::move(product), "Professional", 100.0
                );
            };
        } else if (type == "maintenance") {
            creators_[type] = [](std::unique_ptr<Decoratable> product) -> DecoratorVariant {
                return std::make_unique<MaintenanceDecorator>(
                    std::move(product), "Annual", 200.0
                );
            };
        }
    }
    
    std::optional<DecoratorVariant> createDecorator(
        const std::string& type, 
        std::unique_ptr<Decoratable> product
    ) {
        auto it = creators_.find(type);
        if (it != creators_.end()) {
            return it->second(std::move(product));
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
    MultiLevelDecoratorFactory factory;
    
    // Регистрируем декораторы
    factory.registerDecorator("warranty");
    factory.registerDecorator("installation");
    factory.registerDecorator("maintenance");
    
    // Создаем базовый продукт
    auto product = std::make_unique<Product>("Smart TV", 500.0);
    product->addFeature("4K Resolution");
    product->addFeature("Smart Features");
    
    std::cout << "Original Product:" << std::endl;
    std::cout << "  Name: " << product->getName() << std::endl;
    std::cout << "  Price: $" << product->getPrice() << std::endl;
    std::cout << "  Description: " << product->getDescription() << std::endl;
    
    // Декорируем продукт гарантией
    if (auto decorator = factory.createDecorator("warranty", std::move(product))) {
        std::visit([](auto& d) {
            std::cout << "\nWith Warranty:" << std::endl;
            std::cout << "  Name: " << d->getName() << std::endl;
            std::cout << "  Price: $" << d->getPrice() << std::endl;
            std::cout << "  Description: " << d->getDescription() << std::endl;
        }, *decorator);
    }
    
    // Создаем новый продукт для многоуровневого декорирования
    auto product2 = std::make_unique<Product>("Gaming Console", 300.0);
    product2->addFeature("High Performance");
    product2->addFeature("VR Support");
    
    // Декорируем несколькими уровнями
    if (auto decorator1 = factory.createDecorator("warranty", std::move(product2))) {
        std::visit([&](auto& d1) {
            if (auto decorator2 = factory.createDecorator("installation", std::move(d1))) {
                std::visit([&](auto& d2) {
                    if (auto decorator3 = factory.createDecorator("maintenance", std::move(d2))) {
                        std::visit([](auto& d3) {
                            std::cout << "\nFully Decorated Product:" << std::endl;
                            std::cout << "  Name: " << d3->getName() << std::endl;
                            std::cout << "  Price: $" << d3->getPrice() << std::endl;
                            std::cout << "  Description: " << d3->getDescription() << std::endl;
                        }, *decorator3);
                    }
                }, *decorator2);
            }
        }, *decorator1);
    }
    
    return 0;
}
```

---

## 📋 Задание 3: Decorator с Configuration

### Описание
Создайте декоратор, который поддерживает конфигурацию и может быть легко настраиваемым:

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

// Конфигурация декоратора
struct DecoratorConfig {
    std::string name;
    std::string version;
    std::unordered_map<std::string, std::string> settings;
    std::unordered_map<std::string, std::any> customSettings;
    
    DecoratorConfig(const std::string& n, const std::string& v)
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
class ConfigurableDecorator {
protected:
    DecoratorConfig config_;
    
public:
    ConfigurableDecorator(const DecoratorConfig& config) : config_(config) {}
    
    virtual void validateConfig() const {
        if (config_.name.empty()) {
            throw std::invalid_argument("Decorator name cannot be empty");
        }
        if (config_.version.empty()) {
            throw std::invalid_argument("Decorator version cannot be empty");
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

// Конфигурируемые декораторы
class ConfigurableProductDecorator : public Decoratable, public ConfigurableDecorator {
private:
    std::unique_ptr<Decoratable> product_;
    std::string decoratorType_;
    double additionalCost_;
    
public:
    ConfigurableProductDecorator(
        const DecoratorConfig& config,
        std::unique_ptr<Decoratable> product,
        const std::string& type,
        double cost
    ) : ConfigurableDecorator(config), product_(std::move(product)), 
        decoratorType_(type), additionalCost_(cost) {
        validateConfig();
    }
    
    std::string getName() const override {
        return product_ ? product_->getName() + " (" + decoratorType_ + ")" : "No product";
    }
    
    double getPrice() const override {
        return product_ ? product_->getPrice() + additionalCost_ : 0.0;
    }
    
    std::string getDescription() const override {
        if (product_) {
            std::string desc = product_->getDescription();
            desc += " + " + decoratorType_ + " (" + config_.name + ")";
            return desc;
        }
        return "No product";
    }
    
    void addFeature(const std::string& feature) override {
        if (product_) {
            product_->addFeature(feature);
        }
    }
    
    std::vector<std::string> getFeatures() const override {
        auto features = product_ ? product_->getFeatures() : std::vector<std::string>();
        features.push_back(decoratorType_ + " (" + config_.name + ")");
        return features;
    }
    
    void display() const {
        std::cout << "Configurable Product Decorator: " << config_.name << " v" << config_.version << std::endl;
        std::cout << "  Decorator type: " << decoratorType_ << std::endl;
        std::cout << "  Additional cost: $" << additionalCost_ << std::endl;
        std::cout << "  Settings:" << std::endl;
        for (const auto& setting : config_.settings) {
            std::cout << "    " << setting.first << ": " << setting.second << std::endl;
        }
    }
};

// Конфигурируемая фабрика декораторов
class ConfigurableDecoratorFactory {
private:
    DecoratorConfig config_;
    std::string decoratorType_;
    
public:
    ConfigurableDecoratorFactory(
        const DecoratorConfig& config, 
        const std::string& type
    ) : config_(config), decoratorType_(type) {}
    
    // Методы для настройки конфигурации
    ConfigurableDecoratorFactory& setSetting(const std::string& key, const std::string& value) {
        config_.setSetting(key, value);
        return *this;
    }
    
    template<typename T>
    ConfigurableDecoratorFactory& setCustomSetting(const std::string& key, const T& value) {
        config_.setCustomSetting(key, value);
        return *this;
    }
    
    // Создание декоратора
    std::unique_ptr<ConfigurableDecorator> build(std::unique_ptr<Decoratable> product) {
        double cost = 0.0;
        if (auto costSetting = config_.getCustomSetting<double>("cost")) {
            cost = *costSetting;
        }
        
        return std::make_unique<ConfigurableProductDecorator>(
            config_, std::move(product), decoratorType_, cost
        );
    }
    
    // Создание декоратора с валидацией
    std::unique_ptr<ConfigurableDecorator> buildSafe(std::unique_ptr<Decoratable> product) {
        try {
            return build(std::move(product));
        } catch (const std::exception& e) {
            std::cerr << "Error building decorator: " << e.what() << std::endl;
            return nullptr;
        }
    }
    
    // Получение текущей конфигурации
    DecoratorConfig getConfig() const {
        return config_;
    }
    
    std::string getDecoratorType() const {
        return decoratorType_;
    }
};

// Менеджер конфигураций
class DecoratorConfigManager {
private:
    std::unordered_map<std::string, DecoratorConfig> configs_;
    
public:
    void registerConfig(const std::string& name, const DecoratorConfig& config) {
        configs_[name] = config;
    }
    
    std::optional<DecoratorConfig> getConfig(const std::string& name) const {
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
    
    std::unique_ptr<ConfigurableDecoratorFactory> createFactory(
        const std::string& configName, 
        const std::string& decoratorType
    ) {
        auto config = getConfig(configName);
        if (config) {
            return std::make_unique<ConfigurableDecoratorFactory>(*config, decoratorType);
        }
        return nullptr;
    }
};
```

### Пример использования
```cpp
int main() {
    DecoratorConfigManager configManager;
    
    // Регистрируем конфигурации
    DecoratorConfig warrantyConfig("Warranty", "1.0");
    warrantyConfig.setSetting("type", "Extended");
    warrantyConfig.setSetting("coverage", "Full");
    warrantyConfig.setCustomSetting("cost", 75.0);
    warrantyConfig.setCustomSetting("years", 3);
    
    DecoratorConfig installationConfig("Installation", "2.0");
    installationConfig.setSetting("type", "Professional");
    installationConfig.setSetting("service", "Premium");
    installationConfig.setCustomSetting("cost", 150.0);
    installationConfig.setCustomSetting("hours", 4);
    
    configManager.registerConfig("warranty", warrantyConfig);
    configManager.registerConfig("installation", installationConfig);
    
    // Создаем базовый продукт
    auto product = std::make_unique<Product>("Smartphone", 800.0);
    product->addFeature("5G Support");
    product->addFeature("High Resolution Camera");
    
    // Создаем фабрику для гарантии
    auto warrantyFactory = configManager.createFactory("warranty", "Warranty");
    if (warrantyFactory) {
        warrantyFactory->setSetting("type", "Premium");
        warrantyFactory->setCustomSetting("cost", 100.0);
        
        if (auto decorator = warrantyFactory->buildSafe(std::move(product))) {
            auto productDecorator = static_cast<ConfigurableProductDecorator*>(decorator.get());
            productDecorator->display();
            
            std::cout << "\nDecorated Product:" << std::endl;
            std::cout << "  Name: " << productDecorator->getName() << std::endl;
            std::cout << "  Price: $" << productDecorator->getPrice() << std::endl;
            std::cout << "  Description: " << productDecorator->getDescription() << std::endl;
        }
    }
    
    // Создаем новый продукт для установки
    auto product2 = std::make_unique<Product>("Smart Home System", 1200.0);
    product2->addFeature("Voice Control");
    product2->addFeature("Energy Monitoring");
    
    // Создаем фабрику для установки
    auto installationFactory = configManager.createFactory("installation", "Installation");
    if (installationFactory) {
        installationFactory->setSetting("service", "Expert");
        installationFactory->setCustomSetting("cost", 200.0);
        
        if (auto decorator = installationFactory->buildSafe(std::move(product2))) {
            auto productDecorator = static_cast<ConfigurableProductDecorator*>(decorator.get());
            productDecorator->display();
            
            std::cout << "\nDecorated Product:" << std::endl;
            std::cout << "  Name: " << productDecorator->getName() << std::endl;
            std::cout << "  Price: $" << productDecorator->getPrice() << std::endl;
            std::cout << "  Description: " << productDecorator->getDescription() << std::endl;
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

1. **Composition**: Используйте композицию для добавления функциональности
2. **Flexibility**: Делайте декораторы гибкими и настраиваемыми
3. **Error Handling**: Обрабатывайте ошибки декорирования
4. **Configuration**: Используйте конфигурацию для настройки декораторов
5. **Testing**: Создавайте тесты для проверки декорирования

---

*Помните: Decorator обеспечивает динамическое добавление функциональности к объектам!*
