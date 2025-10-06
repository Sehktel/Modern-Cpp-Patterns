# 🏋️ Упражнения: Strategy Pattern

## 📋 Задание 1: Modern Strategy с Templates

### Описание
Создайте современную реализацию Strategy паттерна с использованием C++17:

### Требования
1. ✅ Используйте `std::variant` для разных типов стратегий
2. ✅ Примените `std::optional` для обработки ошибок
3. ✅ Используйте `auto` и `decltype` для вывода типов
4. ✅ Добавьте поддержку variadic templates
5. ✅ Реализуйте type-safe выполнение стратегий

### Современная архитектура
```cpp
#include <memory>
#include <variant>
#include <optional>
#include <unordered_map>
#include <functional>
#include <string>

// Базовый интерфейс стратегии
class Strategy {
public:
    virtual ~Strategy() = default;
    virtual std::string execute(const std::string& data) = 0;
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
};

// Конкретные стратегии
class QuickSortStrategy : public Strategy {
public:
    std::string execute(const std::string& data) override {
        std::cout << "QuickSortStrategy: Sorting data using Quick Sort algorithm" << std::endl;
        return "Data sorted with Quick Sort: " + data;
    }
    
    std::string getName() const override {
        return "QuickSort";
    }
    
    std::string getDescription() const override {
        return "Efficient sorting algorithm with O(n log n) average complexity";
    }
};

class MergeSortStrategy : public Strategy {
public:
    std::string execute(const std::string& data) override {
        std::cout << "MergeSortStrategy: Sorting data using Merge Sort algorithm" << std::endl;
        return "Data sorted with Merge Sort: " + data;
    }
    
    std::string getName() const override {
        return "MergeSort";
    }
    
    std::string getDescription() const override {
        return "Stable sorting algorithm with O(n log n) guaranteed complexity";
    }
};

class BubbleSortStrategy : public Strategy {
public:
    std::string execute(const std::string& data) override {
        std::cout << "BubbleSortStrategy: Sorting data using Bubble Sort algorithm" << std::endl;
        return "Data sorted with Bubble Sort: " + data;
    }
    
    std::string getName() const override {
        return "BubbleSort";
    }
    
    std::string getDescription() const override {
        return "Simple sorting algorithm with O(n²) complexity";
    }
};

// Современный контекст
class ModernContext {
private:
    std::unique_ptr<Strategy> strategy_;
    std::string data_;
    
public:
    ModernContext() : data_("") {}
    
    // Установка стратегии
    void setStrategy(std::unique_ptr<Strategy> strategy) {
        strategy_ = std::move(strategy);
    }
    
    // Выполнение стратегии
    std::string executeStrategy() {
        if (!strategy_) {
            return "No strategy set";
        }
        
        return strategy_->execute(data_);
    }
    
    // Выполнение стратегии с данными
    std::string executeStrategy(const std::string& data) {
        if (!strategy_) {
            return "No strategy set";
        }
        
        return strategy_->execute(data);
    }
    
    // Установка данных
    void setData(const std::string& data) {
        data_ = data;
    }
    
    // Получение данных
    std::string getData() const {
        return data_;
    }
    
    // Получение информации о текущей стратегии
    std::string getStrategyInfo() const {
        if (!strategy_) {
            return "No strategy set";
        }
        
        return "Strategy: " + strategy_->getName() + 
               " - " + strategy_->getDescription();
    }
    
    // Проверка наличия стратегии
    bool hasStrategy() const {
        return strategy_ != nullptr;
    }
};

// Фабрика стратегий
class StrategyFactory {
private:
    using StrategyVariant = std::variant<
        std::unique_ptr<QuickSortStrategy>,
        std::unique_ptr<MergeSortStrategy>,
        std::unique_ptr<BubbleSortStrategy>
    >;
    
    using CreatorFunction = std::function<StrategyVariant()>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerStrategy(const std::string& type) {
        if (type == "quicksort") {
            creators_[type] = []() -> StrategyVariant {
                return std::make_unique<QuickSortStrategy>();
            };
        } else if (type == "mergesort") {
            creators_[type] = []() -> StrategyVariant {
                return std::make_unique<MergeSortStrategy>();
            };
        } else if (type == "bubblesort") {
            creators_[type] = []() -> StrategyVariant {
                return std::make_unique<BubbleSortStrategy>();
            };
        }
    }
    
    std::optional<StrategyVariant> createStrategy(const std::string& type) {
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
    StrategyFactory factory;
    
    // Регистрируем стратегии
    factory.registerStrategy("quicksort");
    factory.registerStrategy("mergesort");
    factory.registerStrategy("bubblesort");
    
    // Создаем контекст
    ModernContext context;
    context.setData("5,3,8,1,9,2,7,4,6");
    
    // Тестируем разные стратегии
    std::vector<std::string> strategies = {"quicksort", "mergesort", "bubblesort"};
    
    for (const auto& strategyType : strategies) {
        std::cout << "\n=== Testing " << strategyType << " ===" << std::endl;
        
        if (auto strategy = factory.createStrategy(strategyType)) {
            std::visit([&context](auto& s) {
                context.setStrategy(std::move(s));
                std::cout << context.getStrategyInfo() << std::endl;
                std::cout << "Result: " << context.executeStrategy() << std::endl;
            }, *strategy);
        }
    }
    
    // Показываем доступные стратегии
    auto types = factory.getAvailableTypes();
    std::cout << "\nAvailable strategies: ";
    for (const auto& type : types) {
        std::cout << type << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

---

## 📋 Задание 2: Multi-Level Strategy System

### Описание
Создайте систему многоуровневых стратегий для работы с разными типами алгоритмов:

### Требования
1. **Multiple Levels**: Поддержка множественных уровней стратегий
2. **Type Safety**: Проверка типов на этапе компиляции
3. **Error Handling**: Обработка ошибок выполнения стратегий
4. **Validation**: Валидация стратегий

### Реализация
```cpp
#include <any>
#include <type_traits>
#include <stdexcept>

// Базовый интерфейс для алгоритма
class Algorithm {
public:
    virtual ~Algorithm() = default;
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual std::string execute(const std::string& input) = 0;
    virtual bool validateInput(const std::string& input) const = 0;
};

// Конкретные алгоритмы
class CompressionAlgorithm : public Algorithm {
private:
    std::string algorithmName_;
    
public:
    CompressionAlgorithm(const std::string& name) : algorithmName_(name) {}
    
    std::string getName() const override {
        return algorithmName_;
    }
    
    std::string getDescription() const override {
        return "Compression algorithm for data reduction";
    }
    
    std::string execute(const std::string& input) override {
        if (!validateInput(input)) {
            throw std::invalid_argument("Invalid input for compression");
        }
        
        std::cout << algorithmName_ << ": Compressing data..." << std::endl;
        return "Compressed: " + input;
    }
    
    bool validateInput(const std::string& input) const override {
        return !input.empty() && input.length() > 10;
    }
};

class EncryptionAlgorithm : public Algorithm {
private:
    std::string algorithmName_;
    
public:
    EncryptionAlgorithm(const std::string& name) : algorithmName_(name) {}
    
    std::string getName() const override {
        return algorithmName_;
    }
    
    std::string getDescription() const override {
        return "Encryption algorithm for data security";
    }
    
    std::string execute(const std::string& input) override {
        if (!validateInput(input)) {
            throw std::invalid_argument("Invalid input for encryption");
        }
        
        std::cout << algorithmName_ << ": Encrypting data..." << std::endl;
        return "Encrypted: " + input;
    }
    
    bool validateInput(const std::string& input) const override {
        return !input.empty() && input.length() > 5;
    }
};

class ValidationAlgorithm : public Algorithm {
private:
    std::string algorithmName_;
    
public:
    ValidationAlgorithm(const std::string& name) : algorithmName_(name) {}
    
    std::string getName() const override {
        return algorithmName_;
    }
    
    std::string getDescription() const override {
        return "Validation algorithm for data integrity";
    }
    
    std::string execute(const std::string& input) override {
        if (!validateInput(input)) {
            throw std::invalid_argument("Invalid input for validation");
        }
        
        std::cout << algorithmName_ << ": Validating data..." << std::endl;
        return "Validated: " + input;
    }
    
    bool validateInput(const std::string& input) const override {
        return !input.empty();
    }
};

// Многоуровневый контекст
class MultiLevelContext {
private:
    std::vector<std::unique_ptr<Algorithm>> algorithms_;
    std::string data_;
    
public:
    MultiLevelContext() : data_("") {}
    
    // Добавление алгоритма
    void addAlgorithm(std::unique_ptr<Algorithm> algorithm) {
        algorithms_.push_back(std::move(algorithm));
    }
    
    // Установка данных
    void setData(const std::string& data) {
        data_ = data;
    }
    
    // Получение данных
    std::string getData() const {
        return data_;
    }
    
    // Выполнение всех алгоритмов последовательно
    std::string executeAll() {
        if (algorithms_.empty()) {
            return "No algorithms set";
        }
        
        std::string result = data_;
        
        for (size_t i = 0; i < algorithms_.size(); ++i) {
            try {
                result = algorithms_[i]->execute(result);
                std::cout << "Step " << (i + 1) << " completed" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error in step " << (i + 1) << ": " << e.what() << std::endl;
                return "Error: " + std::string(e.what());
            }
        }
        
        return result;
    }
    
    // Выполнение алгоритма по индексу
    std::string executeAt(size_t index) {
        if (index >= algorithms_.size()) {
            throw std::out_of_range("Algorithm index out of range");
        }
        
        try {
            return algorithms_[index]->execute(data_);
        } catch (const std::exception& e) {
            std::cerr << "Error executing algorithm at index " << index << ": " << e.what() << std::endl;
            throw;
        }
    }
    
    // Получение информации об алгоритмах
    std::string getAlgorithmsInfo() const {
        std::string info = "Algorithms:\n";
        for (size_t i = 0; i < algorithms_.size(); ++i) {
            info += "  " + std::to_string(i) + ": " + algorithms_[i]->getName() + 
                   " - " + algorithms_[i]->getDescription() + "\n";
        }
        return info;
    }
    
    // Получение количества алгоритмов
    size_t getAlgorithmCount() const {
        return algorithms_.size();
    }
    
    // Очистка алгоритмов
    void clearAlgorithms() {
        algorithms_.clear();
    }
};

// Фабрика алгоритмов
class AlgorithmFactory {
private:
    using AlgorithmVariant = std::variant<
        std::unique_ptr<CompressionAlgorithm>,
        std::unique_ptr<EncryptionAlgorithm>,
        std::unique_ptr<ValidationAlgorithm>
    >;
    
    using CreatorFunction = std::function<AlgorithmVariant(const std::string&)>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerAlgorithm(const std::string& type) {
        if (type == "compression") {
            creators_[type] = [](const std::string& name) -> AlgorithmVariant {
                return std::make_unique<CompressionAlgorithm>(name);
            };
        } else if (type == "encryption") {
            creators_[type] = [](const std::string& name) -> AlgorithmVariant {
                return std::make_unique<EncryptionAlgorithm>(name);
            };
        } else if (type == "validation") {
            creators_[type] = [](const std::string& name) -> AlgorithmVariant {
                return std::make_unique<ValidationAlgorithm>(name);
            };
        }
    }
    
    std::optional<AlgorithmVariant> createAlgorithm(const std::string& type, const std::string& name) {
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
    AlgorithmFactory factory;
    
    // Регистрируем алгоритмы
    factory.registerAlgorithm("compression");
    factory.registerAlgorithm("encryption");
    factory.registerAlgorithm("validation");
    
    // Создаем многоуровневый контекст
    MultiLevelContext context;
    context.setData("This is a test data for processing");
    
    // Добавляем алгоритмы
    if (auto algorithm1 = factory.createAlgorithm("validation", "DataValidator")) {
        std::visit([&context](auto& alg) {
            context.addAlgorithm(std::move(alg));
        }, *algorithm1);
    }
    
    if (auto algorithm2 = factory.createAlgorithm("compression", "DataCompressor")) {
        std::visit([&context](auto& alg) {
            context.addAlgorithm(std::move(alg));
        }, *algorithm2);
    }
    
    if (auto algorithm3 = factory.createAlgorithm("encryption", "DataEncryptor")) {
        std::visit([&context](auto& alg) {
            context.addAlgorithm(std::move(alg));
        }, *algorithm3);
    }
    
    // Показываем информацию об алгоритмах
    std::cout << context.getAlgorithmsInfo() << std::endl;
    
    // Выполняем все алгоритмы последовательно
    std::cout << "\n=== Executing All Algorithms ===" << std::endl;
    std::string result = context.executeAll();
    std::cout << "Final result: " << result << std::endl;
    
    // Выполняем алгоритм по индексу
    std::cout << "\n=== Executing Algorithm at Index 1 ===" << std::endl;
    try {
        std::string result2 = context.executeAt(1);
        std::cout << "Result: " << result2 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
```

---

## 📋 Задание 3: Strategy с Configuration

### Описание
Создайте стратегию, которая поддерживает конфигурацию и может быть легко настраиваемой:

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

// Конфигурация стратегии
struct StrategyConfig {
    std::string name;
    std::string version;
    std::unordered_map<std::string, std::string> settings;
    std::unordered_map<std::string, std::any> customSettings;
    
    StrategyConfig(const std::string& n, const std::string& v)
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
class ConfigurableStrategy {
protected:
    StrategyConfig config_;
    
public:
    ConfigurableStrategy(const StrategyConfig& config) : config_(config) {}
    
    virtual void validateConfig() const {
        if (config_.name.empty()) {
            throw std::invalid_argument("Strategy name cannot be empty");
        }
        if (config_.version.empty()) {
            throw std::invalid_argument("Strategy version cannot be empty");
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

// Конфигурируемая стратегия
class ConfigurableProcessingStrategy : public Strategy, public ConfigurableStrategy {
private:
    std::string strategyType_;
    bool enabled_;
    
public:
    ConfigurableProcessingStrategy(const StrategyConfig& config) 
        : ConfigurableStrategy(config), enabled_(true) {
        validateConfig();
        
        // Загружаем тип стратегии из конфигурации
        strategyType_ = config_.getSetting("strategy.type");
        if (strategyType_.empty()) {
            strategyType_ = "default";
        }
        
        // Загружаем состояние включен/выключен
        if (auto enabled = config_.getCustomSetting<bool>("enabled")) {
            enabled_ = *enabled;
        }
    }
    
    std::string execute(const std::string& data) override {
        if (!enabled_) {
            return "Strategy is disabled";
        }
        
        // Используем настройки из конфигурации
        std::string prefix = config_.getSetting("output.prefix");
        std::string suffix = config_.getSetting("output.suffix");
        
        std::string result = prefix + "Processed with " + strategyType_ + ": " + data + suffix;
        
        if (auto timestamp = config_.getCustomSetting<bool>("logging.timestamp")) {
            if (*timestamp) {
                result = "[TIMESTAMP] " + result;
            }
        }
        
        if (auto level = config_.getSetting("logging.level")) {
            result = "[" + level + "] " + result;
        }
        
        return result;
    }
    
    std::string getName() const override {
        return config_.name;
    }
    
    std::string getDescription() const override {
        return config_.getSetting("strategy.description");
    }
    
    // Дополнительные методы
    void setEnabled(bool enabled) {
        enabled_ = enabled;
    }
    
    bool isEnabled() const {
        return enabled_;
    }
    
    void setStrategyType(const std::string& type) {
        strategyType_ = type;
    }
    
    std::string getStrategyType() const {
        return strategyType_;
    }
    
    void display() const {
        std::cout << "Configurable Processing Strategy: " << config_.name << " v" << config_.version << std::endl;
        std::cout << "  Strategy type: " << strategyType_ << std::endl;
        std::cout << "  Enabled: " << (enabled_ ? "Yes" : "No") << std::endl;
        std::cout << "  Settings:" << std::endl;
        for (const auto& setting : config_.settings) {
            std::cout << "    " << setting.first << ": " << setting.second << std::endl;
        }
    }
};

// Конфигурируемая фабрика стратегий
class ConfigurableStrategyFactory {
private:
    StrategyConfig config_;
    std::string strategyType_;
    
public:
    ConfigurableStrategyFactory(const StrategyConfig& config, const std::string& type)
        : config_(config), strategyType_(type) {}
    
    // Методы для настройки конфигурации
    ConfigurableStrategyFactory& setSetting(const std::string& key, const std::string& value) {
        config_.setSetting(key, value);
        return *this;
    }
    
    template<typename T>
    ConfigurableStrategyFactory& setCustomSetting(const std::string& key, const T& value) {
        config_.setCustomSetting(key, value);
        return *this;
    }
    
    // Создание стратегии
    std::unique_ptr<ConfigurableStrategy> build() {
        if (strategyType_ == "processing") {
            return std::make_unique<ConfigurableProcessingStrategy>(config_);
        }
        throw std::invalid_argument("Unknown strategy type: " + strategyType_);
    }
    
    // Создание стратегии с валидацией
    std::unique_ptr<ConfigurableStrategy> buildSafe() {
        try {
            return build();
        } catch (const std::exception& e) {
            std::cerr << "Error building strategy: " << e.what() << std::endl;
            return nullptr;
        }
    }
    
    // Получение текущей конфигурации
    StrategyConfig getConfig() const {
        return config_;
    }
    
    std::string getStrategyType() const {
        return strategyType_;
    }
};

// Менеджер конфигураций
class StrategyConfigManager {
private:
    std::unordered_map<std::string, StrategyConfig> configs_;
    
public:
    void registerConfig(const std::string& name, const StrategyConfig& config) {
        configs_[name] = config;
    }
    
    std::optional<StrategyConfig> getConfig(const std::string& name) const {
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
    
    std::unique_ptr<ConfigurableStrategyFactory> createFactory(
        const std::string& configName, 
        const std::string& strategyType
    ) {
        auto config = getConfig(configName);
        if (config) {
            return std::make_unique<ConfigurableStrategyFactory>(*config, strategyType);
        }
        return nullptr;
    }
};
```

### Пример использования
```cpp
int main() {
    StrategyConfigManager configManager;
    
    // Регистрируем конфигурации
    StrategyConfig fastStrategyConfig("Fast Strategy", "1.0");
    fastStrategyConfig.setSetting("strategy.type", "fast");
    fastStrategyConfig.setSetting("strategy.description", "Fast processing strategy");
    fastStrategyConfig.setSetting("output.prefix", "[FAST] ");
    fastStrategyConfig.setSetting("output.suffix", " (completed)");
    fastStrategyConfig.setSetting("logging.level", "INFO");
    fastStrategyConfig.setCustomSetting("enabled", true);
    fastStrategyConfig.setCustomSetting("logging.timestamp", true);
    
    StrategyConfig slowStrategyConfig("Slow Strategy", "2.0");
    slowStrategyConfig.setSetting("strategy.type", "slow");
    slowStrategyConfig.setSetting("strategy.description", "Slow but thorough processing strategy");
    slowStrategyConfig.setSetting("output.prefix", "[SLOW] ");
    slowStrategyConfig.setSetting("output.suffix", " (thoroughly completed)");
    slowStrategyConfig.setSetting("logging.level", "DEBUG");
    slowStrategyConfig.setCustomSetting("enabled", true);
    slowStrategyConfig.setCustomSetting("logging.timestamp", false);
    
    configManager.registerConfig("fast", fastStrategyConfig);
    configManager.registerConfig("slow", slowStrategyConfig);
    
    // Создаем контекст
    ModernContext context;
    context.setData("Test data for processing");
    
    // Создаем фабрику для быстрой стратегии
    auto fastStrategyFactory = configManager.createFactory("fast", "processing");
    if (fastStrategyFactory) {
        fastStrategyFactory->setSetting("logging.level", "WARN");
        fastStrategyFactory->setCustomSetting("enabled", true);
        
        if (auto strategy = fastStrategyFactory->buildSafe()) {
            auto processingStrategy = static_cast<ConfigurableProcessingStrategy*>(strategy.get());
            processingStrategy->display();
            
            context.setStrategy(std::move(strategy));
            std::cout << "\nResult: " << context.executeStrategy() << std::endl;
        }
    }
    
    // Создаем фабрику для медленной стратегии
    auto slowStrategyFactory = configManager.createFactory("slow", "processing");
    if (slowStrategyFactory) {
        slowStrategyFactory->setSetting("logging.level", "TRACE");
        slowStrategyFactory->setCustomSetting("enabled", true);
        
        if (auto strategy = slowStrategyFactory->buildSafe()) {
            auto processingStrategy = static_cast<ConfigurableProcessingStrategy*>(strategy.get());
            processingStrategy->display();
            
            context.setStrategy(std::move(strategy));
            std::cout << "\nResult: " << context.executeStrategy() << std::endl;
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

1. **Algorithm Selection**: Используйте стратегии для выбора алгоритмов во время выполнения
2. **Flexibility**: Делайте стратегии гибкими и настраиваемыми
3. **Error Handling**: Обрабатывайте ошибки выполнения стратегий
4. **Configuration**: Используйте конфигурацию для настройки стратегий
5. **Testing**: Создавайте тесты для проверки стратегий

---

*Помните: Strategy обеспечивает выбор алгоритма во время выполнения!*
