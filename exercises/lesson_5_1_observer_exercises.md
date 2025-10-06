# 🏋️ Упражнения: Observer Pattern

## 📋 Задание 1: Modern Observer с Templates

### Описание
Создайте современную реализацию Observer паттерна с использованием C++17:

### Требования
1. ✅ Используйте `std::variant` для разных типов наблюдателей
2. ✅ Примените `std::optional` для обработки ошибок
3. ✅ Используйте `auto` и `decltype` для вывода типов
4. ✅ Добавьте поддержку variadic templates
5. ✅ Реализуйте type-safe уведомления

### Современная архитектура
```cpp
#include <memory>
#include <variant>
#include <optional>
#include <unordered_map>
#include <functional>
#include <string>
#include <vector>
#include <mutex>

// Базовый интерфейс наблюдателя
class Observer {
public:
    virtual ~Observer() = default;
    virtual void update(const std::string& message) = 0;
    virtual std::string getName() const = 0;
};

// Конкретные наблюдатели
class ConcreteObserverA : public Observer {
private:
    std::string name_;
    
public:
    ConcreteObserverA(const std::string& name) : name_(name) {}
    
    void update(const std::string& message) override {
        std::cout << "ObserverA " << name_ << " received: " << message << std::endl;
    }
    
    std::string getName() const override {
        return name_;
    }
};

class ConcreteObserverB : public Observer {
private:
    std::string name_;
    
public:
    ConcreteObserverB(const std::string& name) : name_(name) {}
    
    void update(const std::string& message) override {
        std::cout << "ObserverB " << name_ << " received: " << message << std::endl;
    }
    
    std::string getName() const override {
        return name_;
    }
};

// Современный субъект
class ModernSubject {
private:
    std::vector<std::weak_ptr<Observer>> observers_;
    std::mutex observersMutex_;
    std::string state_;
    
public:
    ModernSubject() : state_("Initial") {}
    
    // Добавление наблюдателя
    void attach(std::shared_ptr<Observer> observer) {
        std::lock_guard<std::mutex> lock(observersMutex_);
        observers_.push_back(observer);
    }
    
    // Удаление наблюдателя
    void detach(std::shared_ptr<Observer> observer) {
        std::lock_guard<std::mutex> lock(observersMutex_);
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [&observer](const std::weak_ptr<Observer>& weakObs) {
                    return weakObs.lock() == observer;
                }),
            observers_.end()
        );
    }
    
    // Уведомление всех наблюдателей
    void notify() {
        std::lock_guard<std::mutex> lock(observersMutex_);
        
        // Очищаем недействительные weak_ptr
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [](const std::weak_ptr<Observer>& weakObs) {
                    return weakObs.expired();
                }),
            observers_.end()
        );
        
        // Уведомляем активных наблюдателей
        for (auto& weakObs : observers_) {
            if (auto observer = weakObs.lock()) {
                observer->update(state_);
            }
        }
    }
    
    // Изменение состояния
    void setState(const std::string& newState) {
        state_ = newState;
        notify();
    }
    
    // Получение состояния
    std::string getState() const {
        return state_;
    }
    
    // Получение количества наблюдателей
    size_t getObserverCount() const {
        std::lock_guard<std::mutex> lock(observersMutex_);
        return observers_.size();
    }
    
    // Получение списка имен наблюдателей
    std::vector<std::string> getObserverNames() const {
        std::lock_guard<std::mutex> lock(observersMutex_);
        std::vector<std::string> names;
        
        for (const auto& weakObs : observers_) {
            if (auto observer = weakObs.lock()) {
                names.push_back(observer->getName());
            }
        }
        
        return names;
    }
};

// Фабрика наблюдателей
class ObserverFactory {
private:
    using ObserverVariant = std::variant<
        std::shared_ptr<ConcreteObserverA>,
        std::shared_ptr<ConcreteObserverB>
    >;
    
    using CreatorFunction = std::function<ObserverVariant(const std::string&)>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerObserver(const std::string& type) {
        if (type == "type_a") {
            creators_[type] = [](const std::string& name) -> ObserverVariant {
                return std::make_shared<ConcreteObserverA>(name);
            };
        } else if (type == "type_b") {
            creators_[type] = [](const std::string& name) -> ObserverVariant {
                return std::make_shared<ConcreteObserverB>(name);
            };
        }
    }
    
    std::optional<ObserverVariant> createObserver(const std::string& type, const std::string& name) {
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
    ObserverFactory factory;
    
    // Регистрируем наблюдателей
    factory.registerObserver("type_a");
    factory.registerObserver("type_b");
    
    // Создаем субъект
    ModernSubject subject;
    
    // Создаем наблюдателей
    if (auto observer1 = factory.createObserver("type_a", "Observer1")) {
        std::visit([&subject](auto& obs) {
            subject.attach(obs);
        }, *observer1);
    }
    
    if (auto observer2 = factory.createObserver("type_b", "Observer2")) {
        std::visit([&subject](auto& obs) {
            subject.attach(obs);
        }, *observer2);
    }
    
    // Проверяем количество наблюдателей
    std::cout << "Observer count: " << subject.getObserverCount() << std::endl;
    
    // Получаем имена наблюдателей
    auto names = subject.getObserverNames();
    std::cout << "Observer names: ";
    for (const auto& name : names) {
        std::cout << name << " ";
    }
    std::cout << std::endl;
    
    // Изменяем состояние и уведомляем наблюдателей
    std::cout << "\n=== State Change 1 ===" << std::endl;
    subject.setState("New State 1");
    
    std::cout << "\n=== State Change 2 ===" << std::endl;
    subject.setState("New State 2");
    
    return 0;
}
```

---

## 📋 Задание 2: Multi-Level Observer System

### Описание
Создайте систему многоуровневых наблюдателей для работы с разными типами событий:

### Требования
1. **Multiple Levels**: Поддержка множественных уровней наблюдателей
2. **Type Safety**: Проверка типов на этапе компиляции
3. **Error Handling**: Обработка ошибок уведомлений
4. **Validation**: Валидация наблюдателей

### Реализация
```cpp
#include <any>
#include <type_traits>
#include <stdexcept>

// Базовый интерфейс для событий
class Event {
public:
    virtual ~Event() = default;
    virtual std::string getType() const = 0;
    virtual std::string getMessage() const = 0;
    virtual std::chrono::system_clock::time_point getTimestamp() const = 0;
};

// Конкретные события
class StateChangeEvent : public Event {
private:
    std::string oldState_;
    std::string newState_;
    std::chrono::system_clock::time_point timestamp_;
    
public:
    StateChangeEvent(const std::string& oldState, const std::string& newState)
        : oldState_(oldState), newState_(newState), 
          timestamp_(std::chrono::system_clock::now()) {}
    
    std::string getType() const override {
        return "StateChange";
    }
    
    std::string getMessage() const override {
        return "State changed from " + oldState_ + " to " + newState_;
    }
    
    std::chrono::system_clock::time_point getTimestamp() const override {
        return timestamp_;
    }
    
    std::string getOldState() const {
        return oldState_;
    }
    
    std::string getNewState() const {
        return newState_;
    }
};

class ErrorEvent : public Event {
private:
    std::string errorMessage_;
    int errorCode_;
    std::chrono::system_clock::time_point timestamp_;
    
public:
    ErrorEvent(const std::string& message, int code)
        : errorMessage_(message), errorCode_(code),
          timestamp_(std::chrono::system_clock::now()) {}
    
    std::string getType() const override {
        return "Error";
    }
    
    std::string getMessage() const override {
        return "Error " + std::to_string(errorCode_) + ": " + errorMessage_;
    }
    
    std::chrono::system_clock::time_point getTimestamp() const override {
        return timestamp_;
    }
    
    int getErrorCode() const {
        return errorCode_;
    }
};

class InfoEvent : public Event {
private:
    std::string infoMessage_;
    std::string source_;
    std::chrono::system_clock::time_point timestamp_;
    
public:
    InfoEvent(const std::string& message, const std::string& source)
        : infoMessage_(message), source_(source),
          timestamp_(std::chrono::system_clock::now()) {}
    
    std::string getType() const override {
        return "Info";
    }
    
    std::string getMessage() const override {
        return "[" + source_ + "] " + infoMessage_;
    }
    
    std::chrono::system_clock::time_point getTimestamp() const override {
        return timestamp_;
    }
    
    std::string getSource() const {
        return source_;
    }
};

// Базовый интерфейс для наблюдателей событий
class EventObserver {
public:
    virtual ~EventObserver() = default;
    virtual void handleEvent(const Event& event) = 0;
    virtual std::string getName() const = 0;
    virtual std::vector<std::string> getSupportedEventTypes() const = 0;
};

// Конкретные наблюдатели событий
class StateChangeObserver : public EventObserver {
private:
    std::string name_;
    
public:
    StateChangeObserver(const std::string& name) : name_(name) {}
    
    void handleEvent(const Event& event) override {
        if (event.getType() == "StateChange") {
            const auto& stateEvent = static_cast<const StateChangeEvent&>(event);
            std::cout << "StateChangeObserver " << name_ 
                      << " handled: " << stateEvent.getMessage() << std::endl;
        }
    }
    
    std::string getName() const override {
        return name_;
    }
    
    std::vector<std::string> getSupportedEventTypes() const override {
        return {"StateChange"};
    }
};

class ErrorObserver : public EventObserver {
private:
    std::string name_;
    
public:
    ErrorObserver(const std::string& name) : name_(name) {}
    
    void handleEvent(const Event& event) override {
        if (event.getType() == "Error") {
            const auto& errorEvent = static_cast<const ErrorEvent&>(event);
            std::cout << "ErrorObserver " << name_ 
                      << " handled: " << errorEvent.getMessage() << std::endl;
        }
    }
    
    std::string getName() const override {
        return name_;
    }
    
    std::vector<std::string> getSupportedEventTypes() const override {
        return {"Error"};
    }
};

class UniversalObserver : public EventObserver {
private:
    std::string name_;
    
public:
    UniversalObserver(const std::string& name) : name_(name) {}
    
    void handleEvent(const Event& event) override {
        std::cout << "UniversalObserver " << name_ 
                  << " handled " << event.getType() 
                  << ": " << event.getMessage() << std::endl;
    }
    
    std::string getName() const override {
        return name_;
    }
    
    std::vector<std::string> getSupportedEventTypes() const override {
        return {"StateChange", "Error", "Info"};
    }
};

// Многоуровневый субъект событий
class MultiLevelEventSubject {
private:
    std::vector<std::weak_ptr<EventObserver>> observers_;
    std::mutex observersMutex_;
    std::string state_;
    
public:
    MultiLevelEventSubject() : state_("Initial") {}
    
    // Добавление наблюдателя
    void attach(std::shared_ptr<EventObserver> observer) {
        std::lock_guard<std::mutex> lock(observersMutex_);
        observers_.push_back(observer);
    }
    
    // Удаление наблюдателя
    void detach(std::shared_ptr<EventObserver> observer) {
        std::lock_guard<std::mutex> lock(observersMutex_);
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [&observer](const std::weak_ptr<EventObserver>& weakObs) {
                    return weakObs.lock() == observer;
                }),
            observers_.end()
        );
    }
    
    // Уведомление наблюдателей о событии
    void notify(const Event& event) {
        std::lock_guard<std::mutex> lock(observersMutex_);
        
        // Очищаем недействительные weak_ptr
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [](const std::weak_ptr<EventObserver>& weakObs) {
                    return weakObs.expired();
                }),
            observers_.end()
        );
        
        // Уведомляем активных наблюдателей
        for (auto& weakObs : observers_) {
            if (auto observer = weakObs.lock()) {
                // Проверяем, поддерживает ли наблюдатель этот тип события
                auto supportedTypes = observer->getSupportedEventTypes();
                if (std::find(supportedTypes.begin(), supportedTypes.end(), 
                             event.getType()) != supportedTypes.end()) {
                    observer->handleEvent(event);
                }
            }
        }
    }
    
    // Изменение состояния
    void setState(const std::string& newState) {
        std::string oldState = state_;
        state_ = newState;
        
        StateChangeEvent event(oldState, newState);
        notify(event);
    }
    
    // Генерация ошибки
    void generateError(const std::string& message, int code) {
        ErrorEvent event(message, code);
        notify(event);
    }
    
    // Генерация информационного события
    void generateInfo(const std::string& message, const std::string& source) {
        InfoEvent event(message, source);
        notify(event);
    }
    
    // Получение состояния
    std::string getState() const {
        return state_;
    }
    
    // Получение количества наблюдателей
    size_t getObserverCount() const {
        std::lock_guard<std::mutex> lock(observersMutex_);
        return observers_.size();
    }
    
    // Получение списка имен наблюдателей
    std::vector<std::string> getObserverNames() const {
        std::lock_guard<std::mutex> lock(observersMutex_);
        std::vector<std::string> names;
        
        for (const auto& weakObs : observers_) {
            if (auto observer = weakObs.lock()) {
                names.push_back(observer->getName());
            }
        }
        
        return names;
    }
};

// Фабрика наблюдателей событий
class EventObserverFactory {
private:
    using ObserverVariant = std::variant<
        std::shared_ptr<StateChangeObserver>,
        std::shared_ptr<ErrorObserver>,
        std::shared_ptr<UniversalObserver>
    >;
    
    using CreatorFunction = std::function<ObserverVariant(const std::string&)>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerObserver(const std::string& type) {
        if (type == "state_change") {
            creators_[type] = [](const std::string& name) -> ObserverVariant {
                return std::make_shared<StateChangeObserver>(name);
            };
        } else if (type == "error") {
            creators_[type] = [](const std::string& name) -> ObserverVariant {
                return std::make_shared<ErrorObserver>(name);
            };
        } else if (type == "universal") {
            creators_[type] = [](const std::string& name) -> ObserverVariant {
                return std::make_shared<UniversalObserver>(name);
            };
        }
    }
    
    std::optional<ObserverVariant> createObserver(const std::string& type, const std::string& name) {
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
    EventObserverFactory factory;
    
    // Регистрируем наблюдателей
    factory.registerObserver("state_change");
    factory.registerObserver("error");
    factory.registerObserver("universal");
    
    // Создаем субъект событий
    MultiLevelEventSubject subject;
    
    // Создаем наблюдателей
    if (auto observer1 = factory.createObserver("state_change", "StateObserver1")) {
        std::visit([&subject](auto& obs) {
            subject.attach(obs);
        }, *observer1);
    }
    
    if (auto observer2 = factory.createObserver("error", "ErrorObserver1")) {
        std::visit([&subject](auto& obs) {
            subject.attach(obs);
        }, *observer2);
    }
    
    if (auto observer3 = factory.createObserver("universal", "UniversalObserver1")) {
        std::visit([&subject](auto& obs) {
            subject.attach(obs);
        }, *observer3);
    }
    
    // Проверяем количество наблюдателей
    std::cout << "Observer count: " << subject.getObserverCount() << std::endl;
    
    // Получаем имена наблюдателей
    auto names = subject.getObserverNames();
    std::cout << "Observer names: ";
    for (const auto& name : names) {
        std::cout << name << " ";
    }
    std::cout << std::endl;
    
    // Генерируем различные события
    std::cout << "\n=== State Change Event ===" << std::endl;
    subject.setState("New State 1");
    
    std::cout << "\n=== Error Event ===" << std::endl;
    subject.generateError("Database connection failed", 1001);
    
    std::cout << "\n=== Info Event ===" << std::endl;
    subject.generateInfo("System started successfully", "System");
    
    std::cout << "\n=== Another State Change ===" << std::endl;
    subject.setState("New State 2");
    
    return 0;
}
```

---

## 📋 Задание 3: Observer с Configuration

### Описание
Создайте наблюдатель, который поддерживает конфигурацию и может быть легко настраиваемым:

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

// Конфигурация наблюдателя
struct ObserverConfig {
    std::string name;
    std::string version;
    std::unordered_map<std::string, std::string> settings;
    std::unordered_map<std::string, std::any> customSettings;
    
    ObserverConfig(const std::string& n, const std::string& v)
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
class ConfigurableObserver {
protected:
    ObserverConfig config_;
    
public:
    ConfigurableObserver(const ObserverConfig& config) : config_(config) {}
    
    virtual void validateConfig() const {
        if (config_.name.empty()) {
            throw std::invalid_argument("Observer name cannot be empty");
        }
        if (config_.version.empty()) {
            throw std::invalid_argument("Observer version cannot be empty");
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

// Конфигурируемый наблюдатель
class ConfigurableEventObserver : public EventObserver, public ConfigurableObserver {
private:
    std::vector<std::string> supportedEventTypes_;
    bool enabled_;
    
public:
    ConfigurableEventObserver(const ObserverConfig& config) 
        : ConfigurableObserver(config), enabled_(true) {
        validateConfig();
        
        // Загружаем поддерживаемые типы событий из конфигурации
        if (auto types = config_.getCustomSetting<std::vector<std::string>>("supported_types")) {
            supportedEventTypes_ = *types;
        } else {
            supportedEventTypes_ = {"StateChange", "Error", "Info"};
        }
        
        // Загружаем состояние включен/выключен
        if (auto enabled = config_.getCustomSetting<bool>("enabled")) {
            enabled_ = *enabled;
        }
    }
    
    void handleEvent(const Event& event) override {
        if (!enabled_) {
            return;
        }
        
        // Проверяем, поддерживается ли тип события
        if (std::find(supportedEventTypes_.begin(), supportedEventTypes_.end(), 
                     event.getType()) == supportedEventTypes_.end()) {
            return;
        }
        
        // Используем настройки из конфигурации
        std::string prefix = config_.getSetting("message.prefix");
        std::string suffix = config_.getSetting("message.suffix");
        
        std::string message = prefix + event.getMessage() + suffix;
        
        if (auto timestamp = config_.getCustomSetting<bool>("logging.timestamp")) {
            if (*timestamp) {
                message = "[TIMESTAMP] " + message;
            }
        }
        
        if (auto level = config_.getSetting("logging.level")) {
            message = "[" + level + "] " + message;
        }
        
        std::cout << "ConfigurableObserver " << config_.name 
                  << " handled: " << message << std::endl;
    }
    
    std::string getName() const override {
        return config_.name;
    }
    
    std::vector<std::string> getSupportedEventTypes() const override {
        return supportedEventTypes_;
    }
    
    // Дополнительные методы
    void setEnabled(bool enabled) {
        enabled_ = enabled;
    }
    
    bool isEnabled() const {
        return enabled_;
    }
    
    void addSupportedEventType(const std::string& type) {
        if (std::find(supportedEventTypes_.begin(), supportedEventTypes_.end(), 
                     type) == supportedEventTypes_.end()) {
            supportedEventTypes_.push_back(type);
        }
    }
    
    void removeSupportedEventType(const std::string& type) {
        supportedEventTypes_.erase(
            std::remove(supportedEventTypes_.begin(), supportedEventTypes_.end(), type),
            supportedEventTypes_.end()
        );
    }
    
    void display() const {
        std::cout << "Configurable Event Observer: " << config_.name << " v" << config_.version << std::endl;
        std::cout << "  Enabled: " << (enabled_ ? "Yes" : "No") << std::endl;
        std::cout << "  Supported event types: ";
        for (const auto& type : supportedEventTypes_) {
            std::cout << type << " ";
        }
        std::cout << std::endl;
        std::cout << "  Settings:" << std::endl;
        for (const auto& setting : config_.settings) {
            std::cout << "    " << setting.first << ": " << setting.second << std::endl;
        }
    }
};

// Конфигурируемая фабрика наблюдателей
class ConfigurableObserverFactory {
private:
    ObserverConfig config_;
    std::string observerType_;
    
public:
    ConfigurableObserverFactory(const ObserverConfig& config, const std::string& type)
        : config_(config), observerType_(type) {}
    
    // Методы для настройки конфигурации
    ConfigurableObserverFactory& setSetting(const std::string& key, const std::string& value) {
        config_.setSetting(key, value);
        return *this;
    }
    
    template<typename T>
    ConfigurableObserverFactory& setCustomSetting(const std::string& key, const T& value) {
        config_.setCustomSetting(key, value);
        return *this;
    }
    
    // Создание наблюдателя
    std::unique_ptr<ConfigurableObserver> build() {
        if (observerType_ == "event") {
            return std::make_unique<ConfigurableEventObserver>(config_);
        }
        throw std::invalid_argument("Unknown observer type: " + observerType_);
    }
    
    // Создание наблюдателя с валидацией
    std::unique_ptr<ConfigurableObserver> buildSafe() {
        try {
            return build();
        } catch (const std::exception& e) {
            std::cerr << "Error building observer: " << e.what() << std::endl;
            return nullptr;
        }
    }
    
    // Получение текущей конфигурации
    ObserverConfig getConfig() const {
        return config_;
    }
    
    std::string getObserverType() const {
        return observerType_;
    }
};

// Менеджер конфигураций
class ObserverConfigManager {
private:
    std::unordered_map<std::string, ObserverConfig> configs_;
    
public:
    void registerConfig(const std::string& name, const ObserverConfig& config) {
        configs_[name] = config;
    }
    
    std::optional<ObserverConfig> getConfig(const std::string& name) const {
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
    
    std::unique_ptr<ConfigurableObserverFactory> createFactory(
        const std::string& configName, 
        const std::string& observerType
    ) {
        auto config = getConfig(configName);
        if (config) {
            return std::make_unique<ConfigurableObserverFactory>(*config, observerType);
        }
        return nullptr;
    }
};
```

### Пример использования
```cpp
int main() {
    ObserverConfigManager configManager;
    
    // Регистрируем конфигурации
    ObserverConfig stateObserverConfig("State Observer", "1.0");
    stateObserverConfig.setSetting("message.prefix", "[STATE] ");
    stateObserverConfig.setSetting("message.suffix", " (processed)");
    stateObserverConfig.setSetting("logging.level", "INFO");
    stateObserverConfig.setCustomSetting("supported_types", std::vector<std::string>{"StateChange"});
    stateObserverConfig.setCustomSetting("enabled", true);
    stateObserverConfig.setCustomSetting("logging.timestamp", true);
    
    ObserverConfig errorObserverConfig("Error Observer", "2.0");
    errorObserverConfig.setSetting("message.prefix", "[ERROR] ");
    errorObserverConfig.setSetting("message.suffix", " (handled)");
    errorObserverConfig.setSetting("logging.level", "ERROR");
    errorObserverConfig.setCustomSetting("supported_types", std::vector<std::string>{"Error"});
    errorObserverConfig.setCustomSetting("enabled", true);
    errorObserverConfig.setCustomSetting("logging.timestamp", false);
    
    ObserverConfig universalObserverConfig("Universal Observer", "3.0");
    universalObserverConfig.setSetting("message.prefix", "[UNIVERSAL] ");
    universalObserverConfig.setSetting("message.suffix", " (universal)");
    universalObserverConfig.setSetting("logging.level", "DEBUG");
    universalObserverConfig.setCustomSetting("supported_types", std::vector<std::string>{"StateChange", "Error", "Info"});
    universalObserverConfig.setCustomSetting("enabled", true);
    universalObserverConfig.setCustomSetting("logging.timestamp", true);
    
    configManager.registerConfig("state", stateObserverConfig);
    configManager.registerConfig("error", errorObserverConfig);
    configManager.registerConfig("universal", universalObserverConfig);
    
    // Создаем субъект событий
    MultiLevelEventSubject subject;
    
    // Создаем фабрику для наблюдателя состояний
    auto stateObserverFactory = configManager.createFactory("state", "event");
    if (stateObserverFactory) {
        stateObserverFactory->setSetting("logging.level", "WARN");
        stateObserverFactory->setCustomSetting("enabled", true);
        
        if (auto observer = stateObserverFactory->buildSafe()) {
            auto eventObserver = static_cast<ConfigurableEventObserver*>(observer.get());
            eventObserver->display();
            
            subject.attach(std::shared_ptr<EventObserver>(std::move(observer)));
        }
    }
    
    // Создаем фабрику для наблюдателя ошибок
    auto errorObserverFactory = configManager.createFactory("error", "event");
    if (errorObserverFactory) {
        errorObserverFactory->setSetting("logging.level", "CRITICAL");
        errorObserverFactory->setCustomSetting("enabled", true);
        
        if (auto observer = errorObserverFactory->buildSafe()) {
            auto eventObserver = static_cast<ConfigurableEventObserver*>(observer.get());
            eventObserver->display();
            
            subject.attach(std::shared_ptr<EventObserver>(std::move(observer)));
        }
    }
    
    // Создаем фабрику для универсального наблюдателя
    auto universalObserverFactory = configManager.createFactory("universal", "event");
    if (universalObserverFactory) {
        universalObserverFactory->setSetting("logging.level", "TRACE");
        universalObserverFactory->setCustomSetting("enabled", true);
        
        if (auto observer = universalObserverFactory->buildSafe()) {
            auto eventObserver = static_cast<ConfigurableEventObserver*>(observer.get());
            eventObserver->display();
            
            subject.attach(std::shared_ptr<EventObserver>(std::move(observer)));
        }
    }
    
    // Генерируем различные события
    std::cout << "\n=== State Change Event ===" << std::endl;
    subject.setState("New State 1");
    
    std::cout << "\n=== Error Event ===" << std::endl;
    subject.generateError("Database connection failed", 1001);
    
    std::cout << "\n=== Info Event ===" << std::endl;
    subject.generateInfo("System started successfully", "System");
    
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

1. **Loose Coupling**: Используйте слабую связанность между субъектом и наблюдателями
2. **Event Handling**: Обрабатывайте различные типы событий
3. **Error Handling**: Обрабатывайте ошибки уведомлений
4. **Configuration**: Используйте конфигурацию для настройки наблюдателей
5. **Testing**: Создавайте тесты для проверки уведомлений

---

*Помните: Observer обеспечивает слабую связанность и динамическое уведомление!*
