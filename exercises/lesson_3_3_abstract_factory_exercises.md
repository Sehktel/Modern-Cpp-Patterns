# 🏋️ Упражнения: Abstract Factory Pattern

## 📋 Задание 1: Modern Abstract Factory с Templates

### Описание
Создайте современную реализацию Abstract Factory паттерна с использованием C++17:

### Требования
1. ✅ Используйте `std::variant` для разных семейств продуктов
2. ✅ Примените `std::optional` для обработки ошибок
3. ✅ Используйте `auto` и `decltype` для вывода типов
4. ✅ Добавьте поддержку variadic templates
5. ✅ Реализуйте type-safe создание семейств

### Современная архитектура
```cpp
#include <memory>
#include <variant>
#include <optional>
#include <unordered_map>
#include <functional>
#include <string>

// Абстрактные продукты
class Button {
public:
    virtual ~Button() = default;
    virtual void render() = 0;
    virtual void onClick() = 0;
    virtual std::string getStyle() const = 0;
};

class TextBox {
public:
    virtual ~TextBox() = default;
    virtual void render() = 0;
    virtual void setText(const std::string& text) = 0;
    virtual std::string getText() const = 0;
    virtual std::string getStyle() const = 0;
};

// Абстрактная фабрика
class UIFactory {
public:
    virtual ~UIFactory() = default;
    virtual std::unique_ptr<Button> createButton() = 0;
    virtual std::unique_ptr<TextBox> createTextBox() = 0;
    virtual std::string getTheme() const = 0;
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
    
    std::string getStyle() const override {
        return "Windows";
    }
};

class WindowsTextBox : public TextBox {
private:
    std::string text_;
    
public:
    void render() override {
        std::cout << "Rendering Windows textbox: " << text_ << std::endl;
    }
    
    void setText(const std::string& text) override {
        text_ = text;
    }
    
    std::string getText() const override {
        return text_;
    }
    
    std::string getStyle() const override {
        return "Windows";
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
    
    std::string getTheme() const override {
        return "Windows Theme";
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
    
    std::string getStyle() const override {
        return "macOS";
    }
};

class MacOSTextBox : public TextBox {
private:
    std::string text_;
    
public:
    void render() override {
        std::cout << "Rendering macOS textbox: " << text_ << std::endl;
    }
    
    void setText(const std::string& text) override {
        text_ = text;
    }
    
    std::string getText() const override {
        return text_;
    }
    
    std::string getStyle() const override {
        return "macOS";
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
    
    std::string getTheme() const override {
        return "macOS Theme";
    }
};

// Фабрика фабрик с использованием variant
class UIFactoryFactory {
private:
    using FactoryVariant = std::variant<
        std::unique_ptr<WindowsUIFactory>,
        std::unique_ptr<MacOSUIFactory>
    >;
    
    using CreatorFunction = std::function<FactoryVariant()>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerFactory(const std::string& platform) {
        if (platform == "windows") {
            creators_[platform] = []() -> FactoryVariant {
                return std::make_unique<WindowsUIFactory>();
            };
        } else if (platform == "macos") {
            creators_[platform] = []() -> FactoryVariant {
                return std::make_unique<MacOSUIFactory>();
            };
        }
    }
    
    std::optional<FactoryVariant> createFactory(const std::string& platform) {
        auto it = creators_.find(platform);
        if (it != creators_.end()) {
            return it->second();
        }
        return std::nullopt;
    }
    
    std::vector<std::string> getSupportedPlatforms() const {
        std::vector<std::string> platforms;
        for (const auto& pair : creators_) {
            platforms.push_back(pair.first);
        }
        return platforms;
    }
};
```

### Тестовый случай
```cpp
int main() {
    UIFactoryFactory factoryFactory;
    
    // Регистрируем фабрики
    factoryFactory.registerFactory("windows");
    factoryFactory.registerFactory("macos");
    
    // Создаем фабрику для Windows
    if (auto factory = factoryFactory.createFactory("windows")) {
        std::visit([](auto& f) {
            std::cout << "Theme: " << f->getTheme() << std::endl;
            
            auto button = f->createButton();
            auto textBox = f->createTextBox();
            
            button->render();
            button->onClick();
            
            textBox->setText("Hello, Windows!");
            textBox->render();
            
            std::cout << "Button style: " << button->getStyle() << std::endl;
            std::cout << "TextBox style: " << textBox->getStyle() << std::endl;
        }, *factory);
    }
    
    return 0;
}
```

---

## 📋 Задание 2: Configurable Abstract Factory

### Описание
Создайте конфигурируемую Abstract Factory с поддержкой различных тем и стилей:

### Требования
1. **Theme Support**: Поддержка различных тем оформления
2. **Style Configuration**: Настройка стилей через конфигурацию
3. **Dynamic Loading**: Динамическая загрузка фабрик
4. **Validation**: Валидация конфигурации

### Реализация
```cpp
#include <any>
#include <type_traits>
#include <stdexcept>

// Конфигурация темы
struct ThemeConfig {
    std::string name;
    std::string primaryColor;
    std::string secondaryColor;
    std::string fontFamily;
    int fontSize;
    bool darkMode;
    
    ThemeConfig(const std::string& n, const std::string& primary, 
                const std::string& secondary, const std::string& font, 
                int size, bool dark)
        : name(n), primaryColor(primary), secondaryColor(secondary), 
          fontFamily(font), fontSize(size), darkMode(dark) {}
};

// Базовый класс с конфигурацией
class ConfigurableComponent {
protected:
    ThemeConfig config_;
    
public:
    ConfigurableComponent(const ThemeConfig& config) : config_(config) {}
    
    virtual void validateConfig() const {
        if (config_.name.empty()) {
            throw std::invalid_argument("Theme name cannot be empty");
        }
        if (config_.fontSize < 8 || config_.fontSize > 72) {
            throw std::invalid_argument("Font size must be between 8 and 72");
        }
    }
    
    std::string getThemeName() const {
        return config_.name;
    }
};

// Конфигурируемые компоненты
class ConfigurableButton : public Button, public ConfigurableComponent {
public:
    ConfigurableButton(const ThemeConfig& config) : ConfigurableComponent(config) {
        validateConfig();
    }
    
    void render() override {
        std::cout << "Rendering " << config_.name << " button" << std::endl;
        std::cout << "  Primary color: " << config_.primaryColor << std::endl;
        std::cout << "  Font: " << config_.fontFamily << " " << config_.fontSize << "px" << std::endl;
        std::cout << "  Dark mode: " << (config_.darkMode ? "Yes" : "No") << std::endl;
    }
    
    void onClick() override {
        std::cout << config_.name << " button clicked" << std::endl;
    }
    
    std::string getStyle() const override {
        return config_.name + " Style";
    }
};

class ConfigurableTextBox : public TextBox, public ConfigurableComponent {
private:
    std::string text_;
    
public:
    ConfigurableTextBox(const ThemeConfig& config) : ConfigurableComponent(config) {
        validateConfig();
    }
    
    void render() override {
        std::cout << "Rendering " << config_.name << " textbox: " << text_ << std::endl;
        std::cout << "  Primary color: " << config_.primaryColor << std::endl;
        std::cout << "  Font: " << config_.fontFamily << " " << config_.fontSize << "px" << std::endl;
    }
    
    void setText(const std::string& text) override {
        text_ = text;
    }
    
    std::string getText() const override {
        return text_;
    }
    
    std::string getStyle() const override {
        return config_.name + " Style";
    }
};

// Конфигурируемая фабрика
class ConfigurableUIFactory : public UIFactory {
private:
    ThemeConfig config_;
    
public:
    ConfigurableUIFactory(const ThemeConfig& config) : config_(config) {}
    
    std::unique_ptr<Button> createButton() override {
        return std::make_unique<ConfigurableButton>(config_);
    }
    
    std::unique_ptr<TextBox> createTextBox() override {
        return std::make_unique<ConfigurableTextBox>(config_);
    }
    
    std::string getTheme() const override {
        return config_.name + " Theme";
    }
    
    void updateConfig(const ThemeConfig& newConfig) {
        config_ = newConfig;
    }
    
    ThemeConfig getConfig() const {
        return config_;
    }
};

// Менеджер конфигураций
class ThemeManager {
private:
    std::unordered_map<std::string, ThemeConfig> themes_;
    
public:
    void registerTheme(const std::string& name, const ThemeConfig& config) {
        themes_[name] = config;
    }
    
    std::optional<ThemeConfig> getTheme(const std::string& name) const {
        auto it = themes_.find(name);
        if (it != themes_.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    
    std::vector<std::string> getAvailableThemes() const {
        std::vector<std::string> themeNames;
        for (const auto& pair : themes_) {
            themeNames.push_back(pair.first);
        }
        return themeNames;
    }
    
    std::unique_ptr<ConfigurableUIFactory> createFactory(const std::string& themeName) {
        auto config = getTheme(themeName);
        if (config) {
            return std::make_unique<ConfigurableUIFactory>(*config);
        }
        return nullptr;
    }
};
```

### Пример использования
```cpp
int main() {
    ThemeManager themeManager;
    
    // Регистрируем темы
    themeManager.registerTheme("dark", ThemeConfig(
        "Dark", "#2d3748", "#4a5568", "Inter", 14, true
    ));
    
    themeManager.registerTheme("light", ThemeConfig(
        "Light", "#ffffff", "#f7fafc", "Inter", 14, false
    ));
    
    themeManager.registerTheme("high-contrast", ThemeConfig(
        "High Contrast", "#000000", "#ffffff", "Arial", 16, true
    ));
    
    // Создаем фабрику для темной темы
    auto darkFactory = themeManager.createFactory("dark");
    if (darkFactory) {
        std::cout << "Theme: " << darkFactory->getTheme() << std::endl;
        
        auto button = darkFactory->createButton();
        auto textBox = darkFactory->createTextBox();
        
        button->render();
        textBox->setText("Dark theme text");
        textBox->render();
    }
    
    // Создаем фабрику для светлой темы
    auto lightFactory = themeManager.createFactory("light");
    if (lightFactory) {
        std::cout << "\nTheme: " << lightFactory->getTheme() << std::endl;
        
        auto button = lightFactory->createButton();
        auto textBox = lightFactory->createTextBox();
        
        button->render();
        textBox->setText("Light theme text");
        textBox->render();
    }
    
    // Показываем доступные темы
    auto themes = themeManager.getAvailableThemes();
    std::cout << "\nAvailable themes: ";
    for (const auto& theme : themes) {
        std::cout << theme << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

---

## 📋 Задание 3: Abstract Factory с Dependency Injection

### Описание
Создайте Abstract Factory с поддержкой dependency injection для тестируемости:

### Требования
1. **Dependency Injection**: Внедрение зависимостей через конструктор
2. **Testability**: Возможность замены зависимостей для тестов
3. **Configuration**: Настройка фабрики через конфигурацию
4. **Lifecycle Management**: Управление жизненным циклом

### Реализация
```cpp
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

class IResourceManager {
public:
    virtual ~IResourceManager() = default;
    virtual void loadResource(const std::string& name) = 0;
    virtual bool isResourceLoaded(const std::string& name) const = 0;
};

// Компоненты с зависимостями
class DIAwareButton : public Button {
private:
    std::shared_ptr<ILogger> logger_;
    std::shared_ptr<IConfiguration> config_;
    std::string name_;
    
public:
    DIAwareButton(
        const std::string& name,
        std::shared_ptr<ILogger> logger,
        std::shared_ptr<IConfiguration> config
    ) : name_(name), logger_(logger), config_(config) {
        logger_->log("Button " + name_ + " created");
    }
    
    void render() override {
        logger_->log("Rendering button " + name_);
        std::cout << "Rendering " << name_ << " button" << std::endl;
    }
    
    void onClick() override {
        logger_->log("Button " + name_ + " clicked");
        std::cout << name_ << " button clicked" << std::endl;
    }
    
    std::string getStyle() const override {
        return config_->getValue("button.style");
    }
};

class DIAwareTextBox : public TextBox {
private:
    std::shared_ptr<ILogger> logger_;
    std::shared_ptr<IConfiguration> config_;
    std::shared_ptr<IResourceManager> resourceManager_;
    std::string name_;
    std::string text_;
    
public:
    DIAwareTextBox(
        const std::string& name,
        std::shared_ptr<ILogger> logger,
        std::shared_ptr<IConfiguration> config,
        std::shared_ptr<IResourceManager> resourceManager
    ) : name_(name), logger_(logger), config_(config), resourceManager_(resourceManager) {
        logger_->log("TextBox " + name_ + " created");
        resourceManager_->loadResource("textbox_font");
    }
    
    void render() override {
        logger_->log("Rendering textbox " + name_);
        std::cout << "Rendering " << name_ << " textbox: " << text_ << std::endl;
    }
    
    void setText(const std::string& text) override {
        text_ = text;
        logger_->log("TextBox " + name_ + " text set to: " + text_);
    }
    
    std::string getText() const override {
        return text_;
    }
    
    std::string getStyle() const override {
        return config_->getValue("textbox.style");
    }
};

// Фабрика с dependency injection
class DIUIFactory : public UIFactory {
private:
    std::shared_ptr<ILogger> logger_;
    std::shared_ptr<IConfiguration> config_;
    std::shared_ptr<IResourceManager> resourceManager_;
    std::string theme_;
    
public:
    DIUIFactory(
        const std::string& theme,
        std::shared_ptr<ILogger> logger,
        std::shared_ptr<IConfiguration> config,
        std::shared_ptr<IResourceManager> resourceManager
    ) : theme_(theme), logger_(logger), config_(config), resourceManager_(resourceManager) {
        logger_->log("DIUIFactory created for theme: " + theme_);
    }
    
    std::unique_ptr<Button> createButton() override {
        return std::make_unique<DIAwareButton>(
            theme_ + "_button", logger_, config_
        );
    }
    
    std::unique_ptr<TextBox> createTextBox() override {
        return std::make_unique<DIAwareTextBox>(
            theme_ + "_textbox", logger_, config_, resourceManager_
        );
    }
    
    std::string getTheme() const override {
        return theme_ + " Theme";
    }
    
    void updateDependencies(
        std::shared_ptr<ILogger> newLogger,
        std::shared_ptr<IConfiguration> newConfig,
        std::shared_ptr<IResourceManager> newResourceManager
    ) {
        logger_ = newLogger;
        config_ = newConfig;
        resourceManager_ = newResourceManager;
        logger_->log("Dependencies updated for theme: " + theme_);
    }
};

// Фабрика фабрик с DI
class DIFactoryFactory {
private:
    std::shared_ptr<ILogger> logger_;
    std::shared_ptr<IConfiguration> config_;
    std::shared_ptr<IResourceManager> resourceManager_;
    
public:
    DIFactoryFactory(
        std::shared_ptr<ILogger> logger,
        std::shared_ptr<IConfiguration> config,
        std::shared_ptr<IResourceManager> resourceManager
    ) : logger_(logger), config_(config), resourceManager_(resourceManager) {}
    
    std::unique_ptr<DIUIFactory> createFactory(const std::string& theme) {
        return std::make_unique<DIUIFactory>(
            theme, logger_, config_, resourceManager_
        );
    }
    
    void updateDependencies(
        std::shared_ptr<ILogger> newLogger,
        std::shared_ptr<IConfiguration> newConfig,
        std::shared_ptr<IResourceManager> newResourceManager
    ) {
        logger_ = newLogger;
        config_ = newConfig;
        resourceManager_ = newResourceManager;
    }
};
```

### Конкретные реализации зависимостей
```cpp
// Продакшн реализации
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
        values_["button.style"] = "modern";
        values_["textbox.style"] = "rounded";
    }
    
    std::string getValue(const std::string& key) const override {
        auto it = values_.find(key);
        return it != values_.end() ? it->second : "";
    }
    
    void setValue(const std::string& key, const std::string& value) override {
        values_[key] = value;
    }
};

class MemoryResourceManager : public IResourceManager {
private:
    std::unordered_set<std::string> loadedResources_;
    
public:
    void loadResource(const std::string& name) override {
        loadedResources_.insert(name);
        std::cout << "Resource loaded: " << name << std::endl;
    }
    
    bool isResourceLoaded(const std::string& name) const override {
        return loadedResources_.find(name) != loadedResources_.end();
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

class MockResourceManager : public IResourceManager {
private:
    std::unordered_set<std::string> loadedResources_;
    
public:
    void loadResource(const std::string& name) override {
        loadedResources_.insert(name);
    }
    
    bool isResourceLoaded(const std::string& name) const override {
        return loadedResources_.find(name) != loadedResources_.end();
    }
    
    const std::unordered_set<std::string>& getLoadedResources() const {
        return loadedResources_;
    }
};
```

### Пример использования
```cpp
void setupProductionDependencies(
    std::shared_ptr<ILogger>& logger,
    std::shared_ptr<IConfiguration>& config,
    std::shared_ptr<IResourceManager>& resourceManager
) {
    logger = std::make_shared<ConsoleLogger>();
    config = std::make_shared<FileConfiguration>();
    resourceManager = std::make_shared<MemoryResourceManager>();
}

void setupTestDependencies(
    std::shared_ptr<ILogger>& logger,
    std::shared_ptr<IConfiguration>& config,
    std::shared_ptr<IResourceManager>& resourceManager
) {
    logger = std::make_shared<MockLogger>();
    config = std::make_shared<MockConfiguration>();
    resourceManager = std::make_shared<MockResourceManager>();
}

int main() {
    // Настройка продакшн зависимостей
    std::shared_ptr<ILogger> logger;
    std::shared_ptr<IConfiguration> config;
    std::shared_ptr<IResourceManager> resourceManager;
    
    setupProductionDependencies(logger, config, resourceManager);
    
    // Создаем фабрику фабрик
    DIFactoryFactory factoryFactory(logger, config, resourceManager);
    
    // Создаем фабрики для разных тем
    auto darkFactory = factoryFactory.createFactory("dark");
    auto lightFactory = factoryFactory.createFactory("light");
    
    // Используем фабрики
    auto darkButton = darkFactory->createButton();
    auto darkTextBox = darkFactory->createTextBox();
    
    darkButton->render();
    darkButton->onClick();
    
    darkTextBox->setText("Dark theme text");
    darkTextBox->render();
    
    // Тестирование с mock объектами
    setupTestDependencies(logger, config, resourceManager);
    factoryFactory.updateDependencies(logger, config, resourceManager);
    
    auto testFactory = factoryFactory.createFactory("test");
    auto testButton = testFactory->createButton();
    auto testTextBox = testFactory->createTextBox();
    
    testButton->render();
    testTextBox->setText("Test text");
    testTextBox->render();
    
    // Проверяем логи
    auto mockLogger = std::static_pointer_cast<MockLogger>(logger);
    const auto& logs = mockLogger->getLogs();
    std::cout << "\nTest logs:" << std::endl;
    for (const auto& log : logs) {
        std::cout << "  " << log << std::endl;
    }
    
    return 0;
}
```

---

## 🎯 Критерии оценки

### Отлично (5 баллов)
- ✅ Все требования выполнены
- ✅ Современные C++17 возможности
- ✅ Правильная архитектура
- ✅ Хорошая тестируемость
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

1. **Family Consistency**: Убедитесь, что все продукты в семействе совместимы
2. **Configuration**: Используйте конфигурацию для настройки фабрик
3. **Testing**: Создавайте mock объекты для тестирования
4. **Performance**: Рассмотрите кэширование созданных объектов
5. **Extensibility**: Сделайте систему легко расширяемой

---

*Помните: Abstract Factory обеспечивает создание совместимых семейств объектов!*
