# 🏋️ Упражнения: State Pattern

## 📋 Задание 1: Modern State с Templates

### Описание
Создайте современную реализацию State паттерна с использованием C++17:

### Требования
1. ✅ Используйте `std::variant` для разных типов состояний
2. ✅ Примените `std::optional` для обработки ошибок
3. ✅ Используйте `auto` и `decltype` для вывода типов
4. ✅ Добавьте поддержку variadic templates
5. ✅ Реализуйте type-safe переходы между состояниями

### Современная архитектура
```cpp
#include <memory>
#include <variant>
#include <optional>
#include <unordered_map>
#include <functional>
#include <string>
#include <vector>

// Базовый интерфейс состояния
class State {
public:
    virtual ~State() = default;
    virtual void handle() = 0;
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual std::vector<std::string> getPossibleTransitions() const = 0;
    virtual bool canTransitionTo(const std::string& stateName) const = 0;
};

// Конкретные состояния
class IdleState : public State {
public:
    void handle() override {
        std::cout << "IdleState: System is idle, waiting for input" << std::endl;
    }
    
    std::string getName() const override {
        return "Idle";
    }
    
    std::string getDescription() const override {
        return "System is in idle state";
    }
    
    std::vector<std::string> getPossibleTransitions() const override {
        return {"Running", "Paused", "Error"};
    }
    
    bool canTransitionTo(const std::string& stateName) const override {
        auto transitions = getPossibleTransitions();
        return std::find(transitions.begin(), transitions.end(), stateName) != transitions.end();
    }
};

class RunningState : public State {
public:
    void handle() override {
        std::cout << "RunningState: System is running, processing data" << std::endl;
    }
    
    std::string getName() const override {
        return "Running";
    }
    
    std::string getDescription() const override {
        return "System is running and processing";
    }
    
    std::vector<std::string> getPossibleTransitions() const override {
        return {"Paused", "Idle", "Error"};
    }
    
    bool canTransitionTo(const std::string& stateName) const override {
        auto transitions = getPossibleTransitions();
        return std::find(transitions.begin(), transitions.end(), stateName) != transitions.end();
    }
};

class PausedState : public State {
public:
    void handle() override {
        std::cout << "PausedState: System is paused, ready to resume" << std::endl;
    }
    
    std::string getName() const override {
        return "Paused";
    }
    
    std::string getDescription() const override {
        return "System is paused";
    }
    
    std::vector<std::string> getPossibleTransitions() const override {
        return {"Running", "Idle", "Error"};
    }
    
    bool canTransitionTo(const std::string& stateName) const override {
        auto transitions = getPossibleTransitions();
        return std::find(transitions.begin(), transitions.end(), stateName) != transitions.end();
    }
};

class ErrorState : public State {
public:
    void handle() override {
        std::cout << "ErrorState: System encountered an error" << std::endl;
    }
    
    std::string getName() const override {
        return "Error";
    }
    
    std::string getDescription() const override {
        return "System is in error state";
    }
    
    std::vector<std::string> getPossibleTransitions() const override {
        return {"Idle"};
    }
    
    bool canTransitionTo(const std::string& stateName) const override {
        auto transitions = getPossibleTransitions();
        return std::find(transitions.begin(), transitions.end(), stateName) != transitions.end();
    }
};

// Современный контекст
class ModernContext {
private:
    std::unique_ptr<State> currentState_;
    std::string contextName_;
    std::vector<std::string> stateHistory_;
    
public:
    ModernContext(const std::string& name) : contextName_(name) {
        // Устанавливаем начальное состояние
        currentState_ = std::make_unique<IdleState>();
        stateHistory_.push_back(currentState_->getName());
    }
    
    // Установка состояния
    bool setState(std::unique_ptr<State> newState) {
        if (!newState) {
            std::cerr << "Cannot set null state" << std::endl;
            return false;
        }
        
        if (!currentState_->canTransitionTo(newState->getName())) {
            std::cerr << "Cannot transition from " << currentState_->getName() 
                      << " to " << newState->getName() << std::endl;
            return false;
        }
        
        std::cout << "Transitioning from " << currentState_->getName() 
                  << " to " << newState->getName() << std::endl;
        
        currentState_ = std::move(newState);
        stateHistory_.push_back(currentState_->getName());
        
        return true;
    }
    
    // Выполнение текущего состояния
    void handle() {
        if (currentState_) {
            currentState_->handle();
        }
    }
    
    // Получение текущего состояния
    std::string getCurrentStateName() const {
        return currentState_ ? currentState_->getName() : "No State";
    }
    
    // Получение описания текущего состояния
    std::string getCurrentStateDescription() const {
        return currentState_ ? currentState_->getDescription() : "No State";
    }
    
    // Получение возможных переходов
    std::vector<std::string> getPossibleTransitions() const {
        return currentState_ ? currentState_->getPossibleTransitions() : std::vector<std::string>();
    }
    
    // Получение истории состояний
    std::vector<std::string> getStateHistory() const {
        return stateHistory_;
    }
    
    // Получение имени контекста
    std::string getContextName() const {
        return contextName_;
    }
    
    // Проверка возможности перехода
    bool canTransitionTo(const std::string& stateName) const {
        return currentState_ ? currentState_->canTransitionTo(stateName) : false;
    }
    
    // Получение информации о контексте
    std::string getContextInfo() const {
        std::string info = "Context: " + contextName_ + "\n";
        info += "Current State: " + getCurrentStateName() + "\n";
        info += "Description: " + getCurrentStateDescription() + "\n";
        info += "Possible Transitions: ";
        
        auto transitions = getPossibleTransitions();
        for (size_t i = 0; i < transitions.size(); ++i) {
            info += transitions[i];
            if (i < transitions.size() - 1) {
                info += ", ";
            }
        }
        
        return info;
    }
};

// Фабрика состояний
class StateFactory {
private:
    using StateVariant = std::variant<
        std::unique_ptr<IdleState>,
        std::unique_ptr<RunningState>,
        std::unique_ptr<PausedState>,
        std::unique_ptr<ErrorState>
    >;
    
    using CreatorFunction = std::function<StateVariant()>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerState(const std::string& type) {
        if (type == "idle") {
            creators_[type] = []() -> StateVariant {
                return std::make_unique<IdleState>();
            };
        } else if (type == "running") {
            creators_[type] = []() -> StateVariant {
                return std::make_unique<RunningState>();
            };
        } else if (type == "paused") {
            creators_[type] = []() -> StateVariant {
                return std::make_unique<PausedState>();
            };
        } else if (type == "error") {
            creators_[type] = []() -> StateVariant {
                return std::make_unique<ErrorState>();
            };
        }
    }
    
    std::optional<StateVariant> createState(const std::string& type) {
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
    StateFactory factory;
    
    // Регистрируем состояния
    factory.registerState("idle");
    factory.registerState("running");
    factory.registerState("paused");
    factory.registerState("error");
    
    // Создаем контекст
    ModernContext context("TestContext");
    
    std::cout << "Initial state:" << std::endl;
    std::cout << context.getContextInfo() << std::endl;
    
    // Тестируем переходы состояний
    std::vector<std::string> stateSequence = {"running", "paused", "running", "idle", "error", "idle"};
    
    for (const auto& stateType : stateSequence) {
        std::cout << "\n=== Transitioning to " << stateType << " ===" << std::endl;
        
        if (auto state = factory.createState(stateType)) {
            std::visit([&context](auto& s) {
                if (context.setState(std::move(s))) {
                    context.handle();
                }
            }, *state);
        }
        
        std::cout << "Current state: " << context.getCurrentStateName() << std::endl;
    }
    
    // Показываем историю состояний
    auto history = context.getStateHistory();
    std::cout << "\nState history: ";
    for (const auto& state : history) {
        std::cout << state << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

---

## 📋 Задание 2: Multi-Level State System

### Описание
Создайте систему многоуровневых состояний для работы с разными типами объектов:

### Требования
1. **Multiple Levels**: Поддержка множественных уровней состояний
2. **Type Safety**: Проверка типов на этапе компиляции
3. **Error Handling**: Обработка ошибок переходов состояний
4. **Validation**: Валидация состояний

### Реализация
```cpp
#include <any>
#include <type_traits>
#include <stdexcept>

// Базовый интерфейс для объекта состояния
class StateObject {
public:
    virtual ~StateObject() = default;
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual bool isValid() const = 0;
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    virtual void onUpdate() = 0;
};

// Конкретные объекты состояний
class PlayerState : public StateObject {
private:
    std::string playerName_;
    int health_;
    int energy_;
    
public:
    PlayerState(const std::string& name) : playerName_(name), health_(100), energy_(100) {}
    
    std::string getName() const override {
        return "PlayerState";
    }
    
    std::string getDescription() const override {
        return "Player state for " + playerName_;
    }
    
    bool isValid() const override {
        return health_ > 0 && energy_ >= 0;
    }
    
    void onEnter() override {
        std::cout << "Player " << playerName_ << " entered state" << std::endl;
    }
    
    void onExit() override {
        std::cout << "Player " << playerName_ << " exited state" << std::endl;
    }
    
    void onUpdate() override {
        std::cout << "Player " << playerName_ << " updating state (Health: " 
                  << health_ << ", Energy: " << energy_ << ")" << std::endl;
    }
    
    void setHealth(int health) {
        health_ = health;
    }
    
    int getHealth() const {
        return health_;
    }
    
    void setEnergy(int energy) {
        energy_ = energy;
    }
    
    int getEnergy() const {
        return energy_;
    }
};

class GameState : public StateObject {
private:
    std::string gameName_;
    int level_;
    int score_;
    
public:
    GameState(const std::string& name) : gameName_(name), level_(1), score_(0) {}
    
    std::string getName() const override {
        return "GameState";
    }
    
    std::string getDescription() const override {
        return "Game state for " + gameName_;
    }
    
    bool isValid() const override {
        return level_ > 0 && score_ >= 0;
    }
    
    void onEnter() override {
        std::cout << "Game " << gameName_ << " entered state" << std::endl;
    }
    
    void onExit() override {
        std::cout << "Game " << gameName_ << " exited state" << std::endl;
    }
    
    void onUpdate() override {
        std::cout << "Game " << gameName_ << " updating state (Level: " 
                  << level_ << ", Score: " << score_ << ")" << std::endl;
    }
    
    void setLevel(int level) {
        level_ = level;
    }
    
    int getLevel() const {
        return level_;
    }
    
    void setScore(int score) {
        score_ = score;
    }
    
    int getScore() const {
        return score_;
    }
};

class SystemState : public StateObject {
private:
    std::string systemName_;
    bool isOnline_;
    int uptime_;
    
public:
    SystemState(const std::string& name) : systemName_(name), isOnline_(true), uptime_(0) {}
    
    std::string getName() const override {
        return "SystemState";
    }
    
    std::string getDescription() const override {
        return "System state for " + systemName_;
    }
    
    bool isValid() const override {
        return uptime_ >= 0;
    }
    
    void onEnter() override {
        std::cout << "System " << systemName_ << " entered state" << std::endl;
    }
    
    void onExit() override {
        std::cout << "System " << systemName_ << " exited state" << std::endl;
    }
    
    void onUpdate() override {
        uptime_++;
        std::cout << "System " << systemName_ << " updating state (Online: " 
                  << (isOnline_ ? "Yes" : "No") << ", Uptime: " << uptime_ << ")" << std::endl;
    }
    
    void setOnline(bool online) {
        isOnline_ = online;
    }
    
    bool isOnline() const {
        return isOnline_;
    }
    
    int getUptime() const {
        return uptime_;
    }
};

// Многоуровневый контекст состояний
class MultiLevelStateContext {
private:
    std::vector<std::unique_ptr<StateObject>> stateObjects_;
    std::vector<std::string> stateHistory_;
    
public:
    MultiLevelStateContext() = default;
    
    // Добавление объекта состояния
    void addStateObject(std::unique_ptr<StateObject> stateObject) {
        if (!stateObject) {
            std::cerr << "Cannot add null state object" << std::endl;
            return;
        }
        
        if (!stateObject->isValid()) {
            std::cerr << "State object is not valid" << std::endl;
            return;
        }
        
        stateObjects_.push_back(std::move(stateObject));
    }
    
    // Обновление всех объектов состояний
    void updateAll() {
        for (auto& stateObject : stateObjects_) {
            if (stateObject && stateObject->isValid()) {
                stateObject->onUpdate();
            }
        }
    }
    
    // Вход в состояние для всех объектов
    void enterAll() {
        for (auto& stateObject : stateObjects_) {
            if (stateObject && stateObject->isValid()) {
                stateObject->onEnter();
            }
        }
    }
    
    // Выход из состояния для всех объектов
    void exitAll() {
        for (auto& stateObject : stateObjects_) {
            if (stateObject && stateObject->isValid()) {
                stateObject->onExit();
            }
        }
    }
    
    // Получение информации об объектах состояний
    std::string getStateObjectsInfo() const {
        std::string info = "State Objects:\n";
        for (size_t i = 0; i < stateObjects_.size(); ++i) {
            if (stateObjects_[i]) {
                info += "  " + std::to_string(i) + ": " + stateObjects_[i]->getName() + 
                       " - " + stateObjects_[i]->getDescription() + "\n";
            }
        }
        return info;
    }
    
    // Получение количества объектов состояний
    size_t getStateObjectCount() const {
        return stateObjects_.size();
    }
    
    // Получение объекта состояния по индексу
    StateObject* getStateObject(size_t index) const {
        if (index >= stateObjects_.size()) {
            return nullptr;
        }
        return stateObjects_[index].get();
    }
    
    // Очистка объектов состояний
    void clearStateObjects() {
        stateObjects_.clear();
    }
};

// Фабрика объектов состояний
class StateObjectFactory {
private:
    using StateObjectVariant = std::variant<
        std::unique_ptr<PlayerState>,
        std::unique_ptr<GameState>,
        std::unique_ptr<SystemState>
    >;
    
    using CreatorFunction = std::function<StateObjectVariant(const std::string&)>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerStateObject(const std::string& type) {
        if (type == "player") {
            creators_[type] = [](const std::string& name) -> StateObjectVariant {
                return std::make_unique<PlayerState>(name);
            };
        } else if (type == "game") {
            creators_[type] = [](const std::string& name) -> StateObjectVariant {
                return std::make_unique<GameState>(name);
            };
        } else if (type == "system") {
            creators_[type] = [](const std::string& name) -> StateObjectVariant {
                return std::make_unique<SystemState>(name);
            };
        }
    }
    
    std::optional<StateObjectVariant> createStateObject(const std::string& type, const std::string& name) {
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
    StateObjectFactory factory;
    
    // Регистрируем объекты состояний
    factory.registerStateObject("player");
    factory.registerStateObject("game");
    factory.registerStateObject("system");
    
    // Создаем многоуровневый контекст состояний
    MultiLevelStateContext context;
    
    // Добавляем объекты состояний
    if (auto stateObject1 = factory.createStateObject("player", "Player1")) {
        std::visit([&context](auto& obj) {
            context.addStateObject(std::move(obj));
        }, *stateObject1);
    }
    
    if (auto stateObject2 = factory.createStateObject("game", "Game1")) {
        std::visit([&context](auto& obj) {
            context.addStateObject(std::move(obj));
        }, *stateObject2);
    }
    
    if (auto stateObject3 = factory.createStateObject("system", "System1")) {
        std::visit([&context](auto& obj) {
            context.addStateObject(std::move(obj));
        }, *stateObject3);
    }
    
    // Показываем информацию об объектах состояний
    std::cout << context.getStateObjectsInfo() << std::endl;
    
    // Входим в состояние
    std::cout << "=== Entering State ===" << std::endl;
    context.enterAll();
    
    // Обновляем состояния
    std::cout << "\n=== Updating States ===" << std::endl;
    for (int i = 0; i < 3; ++i) {
        std::cout << "Update " << (i + 1) << ":" << std::endl;
        context.updateAll();
    }
    
    // Выходим из состояния
    std::cout << "\n=== Exiting State ===" << std::endl;
    context.exitAll();
    
    return 0;
}
```

---

## 📋 Задание 3: State с Configuration

### Описание
Создайте состояние, которое поддерживает конфигурацию и может быть легко настраиваемым:

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

// Конфигурация состояния
struct StateConfig {
    std::string name;
    std::string version;
    std::unordered_map<std::string, std::string> settings;
    std::unordered_map<std::string, std::any> customSettings;
    
    StateConfig(const std::string& n, const std::string& v)
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
class ConfigurableState {
protected:
    StateConfig config_;
    
public:
    ConfigurableState(const StateConfig& config) : config_(config) {}
    
    virtual void validateConfig() const {
        if (config_.name.empty()) {
            throw std::invalid_argument("State name cannot be empty");
        }
        if (config_.version.empty()) {
            throw std::invalid_argument("State version cannot be empty");
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

// Конфигурируемое состояние
class ConfigurableProcessingState : public State, public ConfigurableState {
private:
    std::string stateType_;
    bool enabled_;
    int executionCount_;
    
public:
    ConfigurableProcessingState(const StateConfig& config) 
        : ConfigurableState(config), enabled_(true), executionCount_(0) {
        validateConfig();
        
        // Загружаем тип состояния из конфигурации
        stateType_ = config_.getSetting("state.type");
        if (stateType_.empty()) {
            stateType_ = "default";
        }
        
        // Загружаем состояние включен/выключен
        if (auto enabled = config_.getCustomSetting<bool>("enabled")) {
            enabled_ = *enabled;
        }
    }
    
    void handle() override {
        if (!enabled_) {
            std::cout << "State is disabled" << std::endl;
            return;
        }
        
        executionCount_++;
        
        // Используем настройки из конфигурации
        std::string prefix = config_.getSetting("output.prefix");
        std::string suffix = config_.getSetting("output.suffix");
        
        std::string message = prefix + "Handling " + stateType_ + " state (execution #" + 
                            std::to_string(executionCount_) + ")" + suffix;
        
        if (auto timestamp = config_.getCustomSetting<bool>("logging.timestamp")) {
            if (*timestamp) {
                message = "[TIMESTAMP] " + message;
            }
        }
        
        if (auto level = config_.getSetting("logging.level")) {
            message = "[" + level + "] " + message;
        }
        
        std::cout << message << std::endl;
    }
    
    std::string getName() const override {
        return config_.name;
    }
    
    std::string getDescription() const override {
        return config_.getSetting("state.description");
    }
    
    std::vector<std::string> getPossibleTransitions() const override {
        std::vector<std::string> transitions;
        
        if (auto transitionsSetting = config_.getCustomSetting<std::vector<std::string>>("transitions")) {
            transitions = *transitionsSetting;
        } else {
            transitions = {"Idle", "Running", "Paused", "Error"};
        }
        
        return transitions;
    }
    
    bool canTransitionTo(const std::string& stateName) const override {
        auto transitions = getPossibleTransitions();
        return std::find(transitions.begin(), transitions.end(), stateName) != transitions.end();
    }
    
    // Дополнительные методы
    void setEnabled(bool enabled) {
        enabled_ = enabled;
    }
    
    bool isEnabled() const {
        return enabled_;
    }
    
    void setStateType(const std::string& type) {
        stateType_ = type;
    }
    
    std::string getStateType() const {
        return stateType_;
    }
    
    int getExecutionCount() const {
        return executionCount_;
    }
    
    void resetExecutionCount() {
        executionCount_ = 0;
    }
    
    void display() const {
        std::cout << "Configurable Processing State: " << config_.name << " v" << config_.version << std::endl;
        std::cout << "  State type: " << stateType_ << std::endl;
        std::cout << "  Enabled: " << (enabled_ ? "Yes" : "No") << std::endl;
        std::cout << "  Execution count: " << executionCount_ << std::endl;
        std::cout << "  Settings:" << std::endl;
        for (const auto& setting : config_.settings) {
            std::cout << "    " << setting.first << ": " << setting.second << std::endl;
        }
    }
};

// Конфигурируемая фабрика состояний
class ConfigurableStateFactory {
private:
    StateConfig config_;
    std::string stateType_;
    
public:
    ConfigurableStateFactory(const StateConfig& config, const std::string& type)
        : config_(config), stateType_(type) {}
    
    // Методы для настройки конфигурации
    ConfigurableStateFactory& setSetting(const std::string& key, const std::string& value) {
        config_.setSetting(key, value);
        return *this;
    }
    
    template<typename T>
    ConfigurableStateFactory& setCustomSetting(const std::string& key, const T& value) {
        config_.setCustomSetting(key, value);
        return *this;
    }
    
    // Создание состояния
    std::unique_ptr<ConfigurableState> build() {
        if (stateType_ == "processing") {
            return std::make_unique<ConfigurableProcessingState>(config_);
        }
        throw std::invalid_argument("Unknown state type: " + stateType_);
    }
    
    // Создание состояния с валидацией
    std::unique_ptr<ConfigurableState> buildSafe() {
        try {
            return build();
        } catch (const std::exception& e) {
            std::cerr << "Error building state: " << e.what() << std::endl;
            return nullptr;
        }
    }
    
    // Получение текущей конфигурации
    StateConfig getConfig() const {
        return config_;
    }
    
    std::string getStateType() const {
        return stateType_;
    }
};

// Менеджер конфигураций
class StateConfigManager {
private:
    std::unordered_map<std::string, StateConfig> configs_;
    
public:
    void registerConfig(const std::string& name, const StateConfig& config) {
        configs_[name] = config;
    }
    
    std::optional<StateConfig> getConfig(const std::string& name) const {
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
    
    std::unique_ptr<ConfigurableStateFactory> createFactory(
        const std::string& configName, 
        const std::string& stateType
    ) {
        auto config = getConfig(configName);
        if (config) {
            return std::make_unique<ConfigurableStateFactory>(*config, stateType);
        }
        return nullptr;
    }
};
```

### Пример использования
```cpp
int main() {
    StateConfigManager configManager;
    
    // Регистрируем конфигурации
    StateConfig activeStateConfig("Active State", "1.0");
    activeStateConfig.setSetting("state.type", "active");
    activeStateConfig.setSetting("state.description", "Active processing state");
    activeStateConfig.setSetting("output.prefix", "[ACTIVE] ");
    activeStateConfig.setSetting("output.suffix", " (completed)");
    activeStateConfig.setSetting("logging.level", "INFO");
    activeStateConfig.setCustomSetting("enabled", true);
    activeStateConfig.setCustomSetting("logging.timestamp", true);
    activeStateConfig.setCustomSetting("transitions", std::vector<std::string>{"Idle", "Paused", "Error"});
    
    StateConfig passiveStateConfig("Passive State", "2.0");
    passiveStateConfig.setSetting("state.type", "passive");
    passiveStateConfig.setSetting("state.description", "Passive processing state");
    passiveStateConfig.setSetting("output.prefix", "[PASSIVE] ");
    passiveStateConfig.setSetting("output.suffix", " (waiting)");
    passiveStateConfig.setSetting("logging.level", "DEBUG");
    passiveStateConfig.setCustomSetting("enabled", true);
    passiveStateConfig.setCustomSetting("logging.timestamp", false);
    passiveStateConfig.setCustomSetting("transitions", std::vector<std::string>{"Active", "Idle", "Error"});
    
    configManager.registerConfig("active", activeStateConfig);
    configManager.registerConfig("passive", passiveStateConfig);
    
    // Создаем контекст
    ModernContext context("TestContext");
    
    // Создаем фабрику для активного состояния
    auto activeStateFactory = configManager.createFactory("active", "processing");
    if (activeStateFactory) {
        activeStateFactory->setSetting("logging.level", "WARN");
        activeStateFactory->setCustomSetting("enabled", true);
        
        if (auto state = activeStateFactory->buildSafe()) {
            auto processingState = static_cast<ConfigurableProcessingState*>(state.get());
            processingState->display();
            
            context.setState(std::move(state));
            context.handle();
        }
    }
    
    // Создаем фабрику для пассивного состояния
    auto passiveStateFactory = configManager.createFactory("passive", "processing");
    if (passiveStateFactory) {
        passiveStateFactory->setSetting("logging.level", "TRACE");
        passiveStateFactory->setCustomSetting("enabled", true);
        
        if (auto state = passiveStateFactory->buildSafe()) {
            auto processingState = static_cast<ConfigurableProcessingState*>(state.get());
            processingState->display();
            
            context.setState(std::move(state));
            context.handle();
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

1. **State Transitions**: Управляйте переходами между состояниями
2. **State Validation**: Валидируйте состояния перед переходами
3. **Error Handling**: Обрабатывайте ошибки переходов состояний
4. **Configuration**: Используйте конфигурацию для настройки состояний
5. **Testing**: Создавайте тесты для проверки состояний

---

*Помните: State позволяет объекту изменять свое поведение при изменении внутреннего состояния!*
