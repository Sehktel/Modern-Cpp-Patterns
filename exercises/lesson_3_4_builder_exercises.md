# 🏋️ Упражнения: Builder Pattern

## 📋 Задание 1: Modern Builder с Fluent Interface

### Описание
Создайте современную реализацию Builder паттерна с fluent interface, используя C++17:

### Требования
1. ✅ Используйте `std::optional` для опциональных параметров
2. ✅ Примените `std::variant` для разных типов конфигурации
3. ✅ Используйте `auto` и `decltype` для вывода типов
4. ✅ Добавьте поддержку variadic templates
5. ✅ Реализуйте type-safe построение объектов

### Современная архитектура
```cpp
#include <memory>
#include <optional>
#include <variant>
#include <string>
#include <vector>
#include <unordered_map>

// Продукт для построения
class Computer {
private:
    std::string cpu_;
    std::string memory_;
    std::string storage_;
    std::string graphics_;
    std::vector<std::string> peripherals_;
    std::unordered_map<std::string, std::string> specifications_;
    
public:
    Computer() = default;
    
    // Setters
    void setCPU(const std::string& cpu) { cpu_ = cpu; }
    void setMemory(const std::string& memory) { memory_ = memory; }
    void setStorage(const std::string& storage) { storage_ = storage; }
    void setGraphics(const std::string& graphics) { graphics_ = graphics; }
    void addPeripheral(const std::string& peripheral) { peripherals_.push_back(peripheral); }
    void setSpecification(const std::string& key, const std::string& value) {
        specifications_[key] = value;
    }
    
    // Getters
    std::string getCPU() const { return cpu_; }
    std::string getMemory() const { return memory_; }
    std::string getStorage() const { return storage_; }
    std::string getGraphics() const { return graphics_; }
    const std::vector<std::string>& getPeripherals() const { return peripherals_; }
    std::string getSpecification(const std::string& key) const {
        auto it = specifications_.find(key);
        return it != specifications_.end() ? it->second : "";
    }
    
    void display() const {
        std::cout << "Computer Configuration:" << std::endl;
        std::cout << "  CPU: " << cpu_ << std::endl;
        std::cout << "  Memory: " << memory_ << std::endl;
        std::cout << "  Storage: " << storage_ << std::endl;
        std::cout << "  Graphics: " << graphics_ << std::endl;
        std::cout << "  Peripherals: ";
        for (const auto& peripheral : peripherals_) {
            std::cout << peripheral << " ";
        }
        std::cout << std::endl;
        std::cout << "  Specifications:" << std::endl;
        for (const auto& spec : specifications_) {
            std::cout << "    " << spec.first << ": " << spec.second << std::endl;
        }
    }
};

// Абстрактный строитель
class ComputerBuilder {
public:
    virtual ~ComputerBuilder() = default;
    virtual ComputerBuilder& setCPU(const std::string& cpu) = 0;
    virtual ComputerBuilder& setMemory(const std::string& memory) = 0;
    virtual ComputerBuilder& setStorage(const std::string& storage) = 0;
    virtual ComputerBuilder& setGraphics(const std::string& graphics) = 0;
    virtual ComputerBuilder& addPeripheral(const std::string& peripheral) = 0;
    virtual ComputerBuilder& setSpecification(const std::string& key, const std::string& value) = 0;
    virtual std::unique_ptr<Computer> build() = 0;
};

// Конкретный строитель
class ModernComputerBuilder : public ComputerBuilder {
private:
    std::unique_ptr<Computer> computer_;
    
public:
    ModernComputerBuilder() : computer_(std::make_unique<Computer>()) {}
    
    ComputerBuilder& setCPU(const std::string& cpu) override {
        computer_->setCPU(cpu);
        return *this;
    }
    
    ComputerBuilder& setMemory(const std::string& memory) override {
        computer_->setMemory(memory);
        return *this;
    }
    
    ComputerBuilder& setStorage(const std::string& storage) override {
        computer_->setStorage(storage);
        return *this;
    }
    
    ComputerBuilder& setGraphics(const std::string& graphics) override {
        computer_->setGraphics(graphics);
        return *this;
    }
    
    ComputerBuilder& addPeripheral(const std::string& peripheral) override {
        computer_->addPeripheral(peripheral);
        return *this;
    }
    
    ComputerBuilder& setSpecification(const std::string& key, const std::string& value) override {
        computer_->setSpecification(key, value);
        return *this;
    }
    
    std::unique_ptr<Computer> build() override {
        return std::move(computer_);
    }
};

// Директор
class ComputerDirector {
public:
    static std::unique_ptr<Computer> buildGamingPC(ComputerBuilder& builder) {
        return builder
            .setCPU("Intel Core i9-12900K")
            .setMemory("32GB DDR5")
            .setStorage("1TB NVMe SSD")
            .setGraphics("RTX 4080")
            .addPeripheral("Gaming Mouse")
            .addPeripheral("Mechanical Keyboard")
            .addPeripheral("144Hz Monitor")
            .setSpecification("Power Supply", "850W")
            .setSpecification("Cooling", "Liquid Cooling")
            .build();
    }
    
    static std::unique_ptr<Computer> buildOfficePC(ComputerBuilder& builder) {
        return builder
            .setCPU("Intel Core i5-12400")
            .setMemory("16GB DDR4")
            .setStorage("512GB SSD")
            .setGraphics("Integrated")
            .addPeripheral("Standard Mouse")
            .addPeripheral("Standard Keyboard")
            .setSpecification("Power Supply", "500W")
            .build();
    }
    
    static std::unique_ptr<Computer> buildWorkstation(ComputerBuilder& builder) {
        return builder
            .setCPU("AMD Ryzen 9 7950X")
            .setMemory("64GB DDR5")
            .setStorage("2TB NVMe SSD")
            .setGraphics("RTX A6000")
            .addPeripheral("Professional Mouse")
            .addPeripheral("Ergonomic Keyboard")
            .addPeripheral("4K Monitor")
            .setSpecification("Power Supply", "1000W")
            .setSpecification("Cooling", "Air Cooling")
            .setSpecification("Case", "Full Tower")
            .build();
    }
};
```

### Тестовый случай
```cpp
int main() {
    ModernComputerBuilder builder;
    
    // Строим игровой ПК
    auto gamingPC = ComputerDirector::buildGamingPC(builder);
    std::cout << "=== Gaming PC ===" << std::endl;
    gamingPC->display();
    
    std::cout << "\n=== Office PC ===" << std::endl;
    ModernComputerBuilder officeBuilder;
    auto officePC = ComputerDirector::buildOfficePC(officeBuilder);
    officePC->display();
    
    std::cout << "\n=== Workstation ===" << std::endl;
    ModernComputerBuilder workstationBuilder;
    auto workstation = ComputerDirector::buildWorkstation(workstationBuilder);
    workstation->display();
    
    // Кастомная сборка
    std::cout << "\n=== Custom Build ===" << std::endl;
    ModernComputerBuilder customBuilder;
    auto customPC = customBuilder
        .setCPU("AMD Ryzen 7 7700X")
        .setMemory("32GB DDR5")
        .setStorage("1TB NVMe SSD")
        .setGraphics("RTX 4070")
        .addPeripheral("Wireless Mouse")
        .addPeripheral("Wireless Keyboard")
        .setSpecification("Power Supply", "750W")
        .setSpecification("Cooling", "Air Cooling")
        .build();
    
    customPC->display();
    
    return 0;
}
```

---

## 📋 Задание 2: Parameterized Builder

### Описание
Создайте строитель, который может создавать объекты с различными параметрами и валидацией:

### Требования
1. **Flexible Parameters**: Поддержка разных типов параметров
2. **Type Safety**: Проверка типов на этапе компиляции
3. **Error Handling**: Обработка ошибок построения
4. **Validation**: Валидация параметров перед построением

### Реализация
```cpp
#include <any>
#include <type_traits>
#include <stdexcept>

// Параметры для построения
struct BuildParams {
    std::string name;
    int priority;
    double cost;
    bool isUrgent;
    
    BuildParams(const std::string& n, int p, double c, bool urgent)
        : name(n), priority(p), cost(c), isUrgent(urgent) {}
};

// Базовый класс с параметрами
class ParameterizedProduct {
protected:
    BuildParams params_;
    
public:
    ParameterizedProduct(const BuildParams& params) : params_(params) {}
    
    virtual void validateParams() const {
        if (params_.name.empty()) {
            throw std::invalid_argument("Product name cannot be empty");
        }
        if (params_.priority < 1 || params_.priority > 10) {
            throw std::invalid_argument("Priority must be between 1 and 10");
        }
        if (params_.cost < 0) {
            throw std::invalid_argument("Cost cannot be negative");
        }
    }
    
    std::string getName() const {
        return params_.name;
    }
    
    int getPriority() const {
        return params_.priority;
    }
    
    double getCost() const {
        return params_.cost;
    }
    
    bool isUrgent() const {
        return params_.isUrgent;
    }
};

// Конкретные продукты с параметрами
class QualityProduct : public ParameterizedProduct {
private:
    std::string quality_;
    std::vector<std::string> features_;
    
public:
    QualityProduct(const BuildParams& params) : ParameterizedProduct(params) {
        validateParams();
        quality_ = "High";
    }
    
    void setQuality(const std::string& quality) {
        quality_ = quality;
    }
    
    void addFeature(const std::string& feature) {
        features_.push_back(feature);
    }
    
    void display() const {
        std::cout << "Quality Product: " << params_.name << std::endl;
        std::cout << "  Priority: " << params_.priority << std::endl;
        std::cout << "  Cost: $" << params_.cost << std::endl;
        std::cout << "  Urgent: " << (params_.isUrgent ? "Yes" : "No") << std::endl;
        std::cout << "  Quality: " << quality_ << std::endl;
        std::cout << "  Features: ";
        for (const auto& feature : features_) {
            std::cout << feature << " ";
        }
        std::cout << std::endl;
    }
};

class PremiumProduct : public ParameterizedProduct {
private:
    std::string premiumLevel_;
    std::unordered_map<std::string, std::string> premiumFeatures_;
    
public:
    PremiumProduct(const BuildParams& params) : ParameterizedProduct(params) {
        validateParams();
        if (!params_.isUrgent) {
            throw std::invalid_argument("Premium product must be urgent");
        }
        premiumLevel_ = "Ultra";
    }
    
    void setPremiumLevel(const std::string& level) {
        premiumLevel_ = level;
    }
    
    void addPremiumFeature(const std::string& key, const std::string& value) {
        premiumFeatures_[key] = value;
    }
    
    void display() const {
        std::cout << "Premium Product: " << params_.name << std::endl;
        std::cout << "  Priority: " << params_.priority << std::endl;
        std::cout << "  Cost: $" << params_.cost << std::endl;
        std::cout << "  Urgent: " << (params_.isUrgent ? "Yes" : "No") << std::endl;
        std::cout << "  Premium Level: " << premiumLevel_ << std::endl;
        std::cout << "  Premium Features:" << std::endl;
        for (const auto& feature : premiumFeatures_) {
            std::cout << "    " << feature.first << ": " << feature.second << std::endl;
        }
    }
};

// Параметризованный строитель
class ParameterizedBuilder {
private:
    BuildParams params_;
    std::string productType_;
    
public:
    ParameterizedBuilder(const BuildParams& params, const std::string& type)
        : params_(params), productType_(type) {}
    
    // Методы для настройки параметров
    ParameterizedBuilder& setName(const std::string& name) {
        params_.name = name;
        return *this;
    }
    
    ParameterizedBuilder& setPriority(int priority) {
        params_.priority = priority;
        return *this;
    }
    
    ParameterizedBuilder& setCost(double cost) {
        params_.cost = cost;
        return *this;
    }
    
    ParameterizedBuilder& setUrgent(bool urgent) {
        params_.isUrgent = urgent;
        return *this;
    }
    
    // Создание продукта
    std::unique_ptr<ParameterizedProduct> build() {
        if (productType_ == "quality") {
            return std::make_unique<QualityProduct>(params_);
        } else if (productType_ == "premium") {
            return std::make_unique<PremiumProduct>(params_);
        }
        throw std::invalid_argument("Unknown product type: " + productType_);
    }
    
    // Создание продукта с валидацией
    std::unique_ptr<ParameterizedProduct> buildSafe() {
        try {
            return build();
        } catch (const std::exception& e) {
            std::cerr << "Error building product: " << e.what() << std::endl;
            return nullptr;
        }
    }
    
    // Получение текущих параметров
    BuildParams getParams() const {
        return params_;
    }
    
    std::string getProductType() const {
        return productType_;
    }
};
```

### Пример использования
```cpp
int main() {
    // Создаем строитель для качественного продукта
    BuildParams params("Widget A", 8, 99.99, false);
    ParameterizedBuilder builder(params, "quality");
    
    // Настраиваем параметры
    builder.setPriority(9).setCost(149.99).setUrgent(true);
    
    // Создаем продукт
    if (auto product = builder.buildSafe()) {
        auto qualityProduct = static_cast<QualityProduct*>(product.get());
        qualityProduct->setQuality("Ultra High");
        qualityProduct->addFeature("Durable");
        qualityProduct->addFeature("Waterproof");
        qualityProduct->addFeature("Warranty");
        
        qualityProduct->display();
    }
    
    // Создаем строитель для премиум продукта
    BuildParams premiumParams("Premium Widget", 10, 299.99, true);
    ParameterizedBuilder premiumBuilder(premiumParams, "premium");
    
    // Настраиваем параметры
    premiumBuilder.setCost(399.99);
    
    // Создаем продукт
    if (auto product = premiumBuilder.buildSafe()) {
        auto premiumProduct = static_cast<PremiumProduct*>(product.get());
        premiumProduct->setPremiumLevel("Ultra Premium");
        premiumProduct->addPremiumFeature("24/7 Support", "Included");
        premiumProduct->addPremiumFeature("Express Delivery", "Same Day");
        premiumProduct->addPremiumFeature("Customization", "Full");
        
        premiumProduct->display();
    }
    
    // Тест с неверными параметрами
    BuildParams invalidParams("", -1, -50.0, false);
    ParameterizedBuilder invalidBuilder(invalidParams, "quality");
    
    if (auto product = invalidBuilder.buildSafe()) {
        // Не должно выполниться
    } else {
        std::cout << "Invalid parameters rejected" << std::endl;
    }
    
    return 0;
}
```

---

## 📋 Задание 3: Builder с Configuration

### Описание
Создайте строитель, который поддерживает конфигурацию и может быть легко настраиваемым:

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

// Конфигурация строителя
struct BuilderConfig {
    std::string name;
    std::string version;
    std::unordered_map<std::string, std::string> settings;
    std::unordered_map<std::string, std::any> customSettings;
    
    BuilderConfig(const std::string& n, const std::string& v)
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
class ConfigurableProduct {
protected:
    BuilderConfig config_;
    
public:
    ConfigurableProduct(const BuilderConfig& config) : config_(config) {}
    
    virtual void validateConfig() const {
        if (config_.name.empty()) {
            throw std::invalid_argument("Product name cannot be empty");
        }
        if (config_.version.empty()) {
            throw std::invalid_argument("Product version cannot be empty");
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

// Конфигурируемые продукты
class ConfigurableWidget : public ConfigurableProduct {
private:
    std::string color_;
    std::string material_;
    std::vector<std::string> features_;
    
public:
    ConfigurableWidget(const BuilderConfig& config) : ConfigurableProduct(config) {
        validateConfig();
        color_ = config.getSetting("default.color");
        material_ = config.getSetting("default.material");
    }
    
    void setColor(const std::string& color) {
        color_ = color;
    }
    
    void setMaterial(const std::string& material) {
        material_ = material;
    }
    
    void addFeature(const std::string& feature) {
        features_.push_back(feature);
    }
    
    void display() const {
        std::cout << "Configurable Widget: " << config_.name << " v" << config_.version << std::endl;
        std::cout << "  Color: " << color_ << std::endl;
        std::cout << "  Material: " << material_ << std::endl;
        std::cout << "  Features: ";
        for (const auto& feature : features_) {
            std::cout << feature << " ";
        }
        std::cout << std::endl;
        std::cout << "  Settings:" << std::endl;
        for (const auto& setting : config_.settings) {
            std::cout << "    " << setting.first << ": " << setting.second << std::endl;
        }
    }
};

class ConfigurableGadget : public ConfigurableProduct {
private:
    std::string size_;
    std::string power_;
    std::unordered_map<std::string, std::string> specifications_;
    
public:
    ConfigurableGadget(const BuilderConfig& config) : ConfigurableProduct(config) {
        validateConfig();
        size_ = config.getSetting("default.size");
        power_ = config.getSetting("default.power");
    }
    
    void setSize(const std::string& size) {
        size_ = size;
    }
    
    void setPower(const std::string& power) {
        power_ = power;
    }
    
    void addSpecification(const std::string& key, const std::string& value) {
        specifications_[key] = value;
    }
    
    void display() const {
        std::cout << "Configurable Gadget: " << config_.name << " v" << config_.version << std::endl;
        std::cout << "  Size: " << size_ << std::endl;
        std::cout << "  Power: " << power_ << std::endl;
        std::cout << "  Specifications:" << std::endl;
        for (const auto& spec : specifications_) {
            std::cout << "    " << spec.first << ": " << spec.second << std::endl;
        }
    }
};

// Конфигурируемый строитель
class ConfigurableBuilder {
private:
    BuilderConfig config_;
    std::string productType_;
    
public:
    ConfigurableBuilder(const BuilderConfig& config, const std::string& type)
        : config_(config), productType_(type) {}
    
    // Методы для настройки конфигурации
    ConfigurableBuilder& setSetting(const std::string& key, const std::string& value) {
        config_.setSetting(key, value);
        return *this;
    }
    
    template<typename T>
    ConfigurableBuilder& setCustomSetting(const std::string& key, const T& value) {
        config_.setCustomSetting(key, value);
        return *this;
    }
    
    // Создание продукта
    std::unique_ptr<ConfigurableProduct> build() {
        if (productType_ == "widget") {
            return std::make_unique<ConfigurableWidget>(config_);
        } else if (productType_ == "gadget") {
            return std::make_unique<ConfigurableGadget>(config_);
        }
        throw std::invalid_argument("Unknown product type: " + productType_);
    }
    
    // Создание продукта с валидацией
    std::unique_ptr<ConfigurableProduct> buildSafe() {
        try {
            return build();
        } catch (const std::exception& e) {
            std::cerr << "Error building product: " << e.what() << std::endl;
            return nullptr;
        }
    }
    
    // Получение текущей конфигурации
    BuilderConfig getConfig() const {
        return config_;
    }
    
    std::string getProductType() const {
        return productType_;
    }
};

// Менеджер конфигураций
class ConfigManager {
private:
    std::unordered_map<std::string, BuilderConfig> configs_;
    
public:
    void registerConfig(const std::string& name, const BuilderConfig& config) {
        configs_[name] = config;
    }
    
    std::optional<BuilderConfig> getConfig(const std::string& name) const {
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
    
    std::unique_ptr<ConfigurableBuilder> createBuilder(
        const std::string& configName, 
        const std::string& productType
    ) {
        auto config = getConfig(configName);
        if (config) {
            return std::make_unique<ConfigurableBuilder>(*config, productType);
        }
        return nullptr;
    }
};
```

### Пример использования
```cpp
int main() {
    ConfigManager configManager;
    
    // Регистрируем конфигурации
    BuilderConfig widgetConfig("Widget", "1.0");
    widgetConfig.setSetting("default.color", "Blue");
    widgetConfig.setSetting("default.material", "Plastic");
    widgetConfig.setSetting("quality", "High");
    widgetConfig.setCustomSetting("warranty", 2);
    widgetConfig.setCustomSetting("price", 99.99);
    
    BuilderConfig gadgetConfig("Gadget", "2.0");
    gadgetConfig.setSetting("default.size", "Medium");
    gadgetConfig.setSetting("default.power", "100W");
    gadgetConfig.setSetting("quality", "Premium");
    gadgetConfig.setCustomSetting("warranty", 5);
    gadgetConfig.setCustomSetting("price", 299.99);
    
    configManager.registerConfig("widget", widgetConfig);
    configManager.registerConfig("gadget", gadgetConfig);
    
    // Создаем строитель для виджета
    auto widgetBuilder = configManager.createBuilder("widget", "widget");
    if (widgetBuilder) {
        widgetBuilder->setSetting("default.color", "Red");
        widgetBuilder->setCustomSetting("warranty", 3);
        
        if (auto product = widgetBuilder->buildSafe()) {
            auto widget = static_cast<ConfigurableWidget*>(product.get());
            widget->setColor("Green");
            widget->setMaterial("Metal");
            widget->addFeature("Waterproof");
            widget->addFeature("Shockproof");
            
            widget->display();
        }
    }
    
    // Создаем строитель для гаджета
    auto gadgetBuilder = configManager.createBuilder("gadget", "gadget");
    if (gadgetBuilder) {
        gadgetBuilder->setSetting("default.size", "Large");
        gadgetBuilder->setCustomSetting("warranty", 7);
        
        if (auto product = gadgetBuilder->buildSafe()) {
            auto gadget = static_cast<ConfigurableGadget*>(product.get());
            gadget->setSize("Extra Large");
            gadget->setPower("200W");
            gadget->addSpecification("Battery", "5000mAh");
            gadget->addSpecification("Connectivity", "WiFi + Bluetooth");
            
            gadget->display();
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
- ✅ Fluent interface реализован
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

1. **Fluent Interface**: Используйте возврат ссылки на себя для цепочки вызовов
2. **Validation**: Добавляйте валидацию на каждом этапе построения
3. **Configuration**: Используйте конфигурацию для настройки строителя
4. **Error Handling**: Обрабатывайте ошибки построения
5. **Flexibility**: Делайте строитель гибким и настраиваемым

---

*Помните: Builder обеспечивает пошаговое создание сложных объектов!*
