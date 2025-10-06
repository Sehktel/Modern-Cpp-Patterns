# 🏋️ Упражнения: Command Pattern

## 📋 Задание 1: Modern Command с Templates

### Описание
Создайте современную реализацию Command паттерна с использованием C++17:

### Требования
1. ✅ Используйте `std::variant` для разных типов команд
2. ✅ Примените `std::optional` для обработки ошибок
3. ✅ Используйте `auto` и `decltype` для вывода типов
4. ✅ Добавьте поддержку variadic templates
5. ✅ Реализуйте type-safe выполнение команд

### Современная архитектура
```cpp
#include <memory>
#include <variant>
#include <optional>
#include <unordered_map>
#include <functional>
#include <string>
#include <vector>
#include <stack>

// Базовый интерфейс команды
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual bool canUndo() const = 0;
};

// Получатель команды
class Receiver {
private:
    std::string state_;
    int value_;
    
public:
    Receiver() : state_("Initial"), value_(0) {}
    
    void setState(const std::string& state) {
        state_ = state;
    }
    
    std::string getState() const {
        return state_;
    }
    
    void setValue(int value) {
        value_ = value;
    }
    
    int getValue() const {
        return value_;
    }
    
    void increment() {
        value_++;
    }
    
    void decrement() {
        value_--;
    }
    
    void reset() {
        value_ = 0;
        state_ = "Reset";
    }
};

// Конкретные команды
class SetStateCommand : public Command {
private:
    std::shared_ptr<Receiver> receiver_;
    std::string newState_;
    std::string oldState_;
    bool executed_;
    
public:
    SetStateCommand(std::shared_ptr<Receiver> receiver, const std::string& newState)
        : receiver_(receiver), newState_(newState), executed_(false) {}
    
    void execute() override {
        if (executed_) {
            return;
        }
        
        oldState_ = receiver_->getState();
        receiver_->setState(newState_);
        executed_ = true;
        
        std::cout << "SetStateCommand executed: " << oldState_ << " -> " << newState_ << std::endl;
    }
    
    void undo() override {
        if (!executed_) {
            return;
        }
        
        receiver_->setState(oldState_);
        executed_ = false;
        
        std::cout << "SetStateCommand undone: " << newState_ << " -> " << oldState_ << std::endl;
    }
    
    std::string getName() const override {
        return "SetState";
    }
    
    std::string getDescription() const override {
        return "Sets the state of the receiver";
    }
    
    bool canUndo() const override {
        return executed_;
    }
};

class IncrementCommand : public Command {
private:
    std::shared_ptr<Receiver> receiver_;
    bool executed_;
    
public:
    IncrementCommand(std::shared_ptr<Receiver> receiver)
        : receiver_(receiver), executed_(false) {}
    
    void execute() override {
        if (executed_) {
            return;
        }
        
        receiver_->increment();
        executed_ = true;
        
        std::cout << "IncrementCommand executed. Value: " << receiver_->getValue() << std::endl;
    }
    
    void undo() override {
        if (!executed_) {
            return;
        }
        
        receiver_->decrement();
        executed_ = false;
        
        std::cout << "IncrementCommand undone. Value: " << receiver_->getValue() << std::endl;
    }
    
    std::string getName() const override {
        return "Increment";
    }
    
    std::string getDescription() const override {
        return "Increments the value of the receiver";
    }
    
    bool canUndo() const override {
        return executed_;
    }
};

class ResetCommand : public Command {
private:
    std::shared_ptr<Receiver> receiver_;
    int oldValue_;
    std::string oldState_;
    bool executed_;
    
public:
    ResetCommand(std::shared_ptr<Receiver> receiver)
        : receiver_(receiver), executed_(false) {}
    
    void execute() override {
        if (executed_) {
            return;
        }
        
        oldValue_ = receiver_->getValue();
        oldState_ = receiver_->getState();
        receiver_->reset();
        executed_ = true;
        
        std::cout << "ResetCommand executed. Value: " << receiver_->getValue() 
                  << ", State: " << receiver_->getState() << std::endl;
    }
    
    void undo() override {
        if (!executed_) {
            return;
        }
        
        receiver_->setValue(oldValue_);
        receiver_->setState(oldState_);
        executed_ = false;
        
        std::cout << "ResetCommand undone. Value: " << receiver_->getValue() 
                  << ", State: " << receiver_->getState() << std::endl;
    }
    
    std::string getName() const override {
        return "Reset";
    }
    
    std::string getDescription() const override {
        return "Resets the receiver to initial state";
    }
    
    bool canUndo() const override {
        return executed_;
    }
};

// Современный вызывающий
class ModernInvoker {
private:
    std::stack<std::unique_ptr<Command>> commandHistory_;
    std::shared_ptr<Receiver> receiver_;
    
public:
    ModernInvoker(std::shared_ptr<Receiver> receiver) : receiver_(receiver) {}
    
    // Выполнение команды
    void executeCommand(std::unique_ptr<Command> command) {
        if (!command) {
            std::cerr << "Cannot execute null command" << std::endl;
            return;
        }
        
        command->execute();
        
        if (command->canUndo()) {
            commandHistory_.push(std::move(command));
        }
    }
    
    // Отмена последней команды
    void undoLastCommand() {
        if (commandHistory_.empty()) {
            std::cout << "No commands to undo" << std::endl;
            return;
        }
        
        auto command = std::move(commandHistory_.top());
        commandHistory_.pop();
        
        command->undo();
    }
    
    // Отмена всех команд
    void undoAllCommands() {
        while (!commandHistory_.empty()) {
            undoLastCommand();
        }
    }
    
    // Получение истории команд
    std::vector<std::string> getCommandHistory() const {
        std::vector<std::string> history;
        std::stack<std::unique_ptr<Command>> tempStack = commandHistory_;
        
        while (!tempStack.empty()) {
            history.push_back(tempStack.top()->getName());
            tempStack.pop();
        }
        
        return history;
    }
    
    // Получение количества команд в истории
    size_t getCommandCount() const {
        return commandHistory_.size();
    }
    
    // Получение информации о получателе
    std::string getReceiverInfo() const {
        return "Receiver state: " + receiver_->getState() + 
               ", value: " + std::to_string(receiver_->getValue());
    }
};

// Фабрика команд
class CommandFactory {
private:
    using CommandVariant = std::variant<
        std::unique_ptr<SetStateCommand>,
        std::unique_ptr<IncrementCommand>,
        std::unique_ptr<ResetCommand>
    >;
    
    using CreatorFunction = std::function<CommandVariant(std::shared_ptr<Receiver>)>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerCommand(const std::string& type) {
        if (type == "set_state") {
            creators_[type] = [](std::shared_ptr<Receiver> receiver) -> CommandVariant {
                return std::make_unique<SetStateCommand>(receiver, "New State");
            };
        } else if (type == "increment") {
            creators_[type] = [](std::shared_ptr<Receiver> receiver) -> CommandVariant {
                return std::make_unique<IncrementCommand>(receiver);
            };
        } else if (type == "reset") {
            creators_[type] = [](std::shared_ptr<Receiver> receiver) -> CommandVariant {
                return std::make_unique<ResetCommand>(receiver);
            };
        }
    }
    
    std::optional<CommandVariant> createCommand(const std::string& type, std::shared_ptr<Receiver> receiver) {
        auto it = creators_.find(type);
        if (it != creators_.end()) {
            return it->second(receiver);
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
    CommandFactory factory;
    
    // Регистрируем команды
    factory.registerCommand("set_state");
    factory.registerCommand("increment");
    factory.registerCommand("reset");
    
    // Создаем получателя и вызывающего
    auto receiver = std::make_shared<Receiver>();
    ModernInvoker invoker(receiver);
    
    std::cout << "Initial: " << invoker.getReceiverInfo() << std::endl;
    
    // Выполняем команды
    std::vector<std::string> commands = {"increment", "increment", "set_state", "reset"};
    
    for (const auto& commandType : commands) {
        std::cout << "\n=== Executing " << commandType << " ===" << std::endl;
        
        if (auto command = factory.createCommand(commandType, receiver)) {
            std::visit([&invoker](auto& cmd) {
                invoker.executeCommand(std::move(cmd));
            }, *command);
        }
        
        std::cout << "Current: " << invoker.getReceiverInfo() << std::endl;
    }
    
    // Показываем историю команд
    auto history = invoker.getCommandHistory();
    std::cout << "\nCommand history: ";
    for (const auto& cmd : history) {
        std::cout << cmd << " ";
    }
    std::cout << std::endl;
    
    // Отменяем команды
    std::cout << "\n=== Undoing Commands ===" << std::endl;
    while (invoker.getCommandCount() > 0) {
        invoker.undoLastCommand();
        std::cout << "Current: " << invoker.getReceiverInfo() << std::endl;
    }
    
    return 0;
}
```

---

## 📋 Задание 2: Multi-Level Command System

### Описание
Создайте систему многоуровневых команд для работы с разными типами операций:

### Требования
1. **Multiple Levels**: Поддержка множественных уровней команд
2. **Type Safety**: Проверка типов на этапе компиляции
3. **Error Handling**: Обработка ошибок выполнения команд
4. **Validation**: Валидация команд

### Реализация
```cpp
#include <any>
#include <type_traits>
#include <stdexcept>

// Базовый интерфейс для операции
class Operation {
public:
    virtual ~Operation() = default;
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual bool execute() = 0;
    virtual bool undo() = 0;
    virtual bool canUndo() const = 0;
    virtual bool validate() const = 0;
};

// Конкретные операции
class FileOperation : public Operation {
private:
    std::string fileName_;
    std::string content_;
    bool executed_;
    
public:
    FileOperation(const std::string& fileName, const std::string& content)
        : fileName_(fileName), content_(content), executed_(false) {}
    
    std::string getName() const override {
        return "FileOperation";
    }
    
    std::string getDescription() const override {
        return "File operation on " + fileName_;
    }
    
    bool execute() override {
        if (executed_) {
            return false;
        }
        
        if (!validate()) {
            return false;
        }
        
        std::cout << "FileOperation: Writing to " << fileName_ << std::endl;
        executed_ = true;
        return true;
    }
    
    bool undo() override {
        if (!executed_) {
            return false;
        }
        
        std::cout << "FileOperation: Deleting " << fileName_ << std::endl;
        executed_ = false;
        return true;
    }
    
    bool canUndo() const override {
        return executed_;
    }
    
    bool validate() const override {
        return !fileName_.empty() && !content_.empty();
    }
    
    std::string getFileName() const {
        return fileName_;
    }
    
    std::string getContent() const {
        return content_;
    }
};

class DatabaseOperation : public Operation {
private:
    std::string tableName_;
    std::string query_;
    bool executed_;
    
public:
    DatabaseOperation(const std::string& tableName, const std::string& query)
        : tableName_(tableName), query_(query), executed_(false) {}
    
    std::string getName() const override {
        return "DatabaseOperation";
    }
    
    std::string getDescription() const override {
        return "Database operation on table " + tableName_;
    }
    
    bool execute() override {
        if (executed_) {
            return false;
        }
        
        if (!validate()) {
            return false;
        }
        
        std::cout << "DatabaseOperation: Executing query on " << tableName_ << std::endl;
        executed_ = true;
        return true;
    }
    
    bool undo() override {
        if (!executed_) {
            return false;
        }
        
        std::cout << "DatabaseOperation: Rolling back query on " << tableName_ << std::endl;
        executed_ = false;
        return true;
    }
    
    bool canUndo() const override {
        return executed_;
    }
    
    bool validate() const override {
        return !tableName_.empty() && !query_.empty();
    }
    
    std::string getTableName() const {
        return tableName_;
    }
    
    std::string getQuery() const {
        return query_;
    }
};

class NetworkOperation : public Operation {
private:
    std::string url_;
    std::string method_;
    bool executed_;
    
public:
    NetworkOperation(const std::string& url, const std::string& method)
        : url_(url), method_(method), executed_(false) {}
    
    std::string getName() const override {
        return "NetworkOperation";
    }
    
    std::string getDescription() const override {
        return "Network operation: " + method_ + " " + url_;
    }
    
    bool execute() override {
        if (executed_) {
            return false;
        }
        
        if (!validate()) {
            return false;
        }
        
        std::cout << "NetworkOperation: Sending " << method_ << " request to " << url_ << std::endl;
        executed_ = true;
        return true;
    }
    
    bool undo() override {
        if (!executed_) {
            return false;
        }
        
        std::cout << "NetworkOperation: Cancelling request to " << url_ << std::endl;
        executed_ = false;
        return true;
    }
    
    bool canUndo() const override {
        return executed_;
    }
    
    bool validate() const override {
        return !url_.empty() && !method_.empty();
    }
    
    std::string getUrl() const {
        return url_;
    }
    
    std::string getMethod() const {
        return method_;
    }
};

// Многоуровневый вызывающий
class MultiLevelInvoker {
private:
    std::vector<std::unique_ptr<Operation>> operations_;
    std::vector<size_t> executedOperations_;
    
public:
    MultiLevelInvoker() = default;
    
    // Добавление операции
    void addOperation(std::unique_ptr<Operation> operation) {
        operations_.push_back(std::move(operation));
    }
    
    // Выполнение операции по индексу
    bool executeOperation(size_t index) {
        if (index >= operations_.size()) {
            std::cerr << "Operation index out of range" << std::endl;
            return false;
        }
        
        if (!operations_[index]->validate()) {
            std::cerr << "Operation validation failed" << std::endl;
            return false;
        }
        
        if (operations_[index]->execute()) {
            executedOperations_.push_back(index);
            return true;
        }
        
        return false;
    }
    
    // Выполнение всех операций
    bool executeAll() {
        bool allSuccessful = true;
        
        for (size_t i = 0; i < operations_.size(); ++i) {
            if (!executeOperation(i)) {
                allSuccessful = false;
            }
        }
        
        return allSuccessful;
    }
    
    // Отмена операции по индексу
    bool undoOperation(size_t index) {
        if (index >= operations_.size()) {
            std::cerr << "Operation index out of range" << std::endl;
            return false;
        }
        
        if (!operations_[index]->canUndo()) {
            std::cerr << "Operation cannot be undone" << std::endl;
            return false;
        }
        
        if (operations_[index]->undo()) {
            // Удаляем из списка выполненных операций
            executedOperations_.erase(
                std::remove(executedOperations_.begin(), executedOperations_.end(), index),
                executedOperations_.end()
            );
            return true;
        }
        
        return false;
    }
    
    // Отмена всех операций
    bool undoAll() {
        bool allSuccessful = true;
        
        // Отменяем в обратном порядке
        for (auto it = executedOperations_.rbegin(); it != executedOperations_.rend(); ++it) {
            if (!undoOperation(*it)) {
                allSuccessful = false;
            }
        }
        
        return allSuccessful;
    }
    
    // Получение информации об операциях
    std::string getOperationsInfo() const {
        std::string info = "Operations:\n";
        for (size_t i = 0; i < operations_.size(); ++i) {
            info += "  " + std::to_string(i) + ": " + operations_[i]->getName() + 
                   " - " + operations_[i]->getDescription() + "\n";
        }
        return info;
    }
    
    // Получение количества операций
    size_t getOperationCount() const {
        return operations_.size();
    }
    
    // Получение количества выполненных операций
    size_t getExecutedOperationCount() const {
        return executedOperations_.size();
    }
    
    // Очистка операций
    void clearOperations() {
        operations_.clear();
        executedOperations_.clear();
    }
};

// Фабрика операций
class OperationFactory {
private:
    using OperationVariant = std::variant<
        std::unique_ptr<FileOperation>,
        std::unique_ptr<DatabaseOperation>,
        std::unique_ptr<NetworkOperation>
    >;
    
    using CreatorFunction = std::function<OperationVariant(const std::string&, const std::string&)>;
    std::unordered_map<std::string, CreatorFunction> creators_;
    
public:
    void registerOperation(const std::string& type) {
        if (type == "file") {
            creators_[type] = [](const std::string& param1, const std::string& param2) -> OperationVariant {
                return std::make_unique<FileOperation>(param1, param2);
            };
        } else if (type == "database") {
            creators_[type] = [](const std::string& param1, const std::string& param2) -> OperationVariant {
                return std::make_unique<DatabaseOperation>(param1, param2);
            };
        } else if (type == "network") {
            creators_[type] = [](const std::string& param1, const std::string& param2) -> OperationVariant {
                return std::make_unique<NetworkOperation>(param1, param2);
            };
        }
    }
    
    std::optional<OperationVariant> createOperation(
        const std::string& type, 
        const std::string& param1, 
        const std::string& param2
    ) {
        auto it = creators_.find(type);
        if (it != creators_.end()) {
            return it->second(param1, param2);
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
    OperationFactory factory;
    
    // Регистрируем операции
    factory.registerOperation("file");
    factory.registerOperation("database");
    factory.registerOperation("network");
    
    // Создаем многоуровневый вызывающий
    MultiLevelInvoker invoker;
    
    // Добавляем операции
    if (auto operation1 = factory.createOperation("file", "test.txt", "Hello, World!")) {
        std::visit([&invoker](auto& op) {
            invoker.addOperation(std::move(op));
        }, *operation1);
    }
    
    if (auto operation2 = factory.createOperation("database", "users", "SELECT * FROM users")) {
        std::visit([&invoker](auto& op) {
            invoker.addOperation(std::move(op));
        }, *operation2);
    }
    
    if (auto operation3 = factory.createOperation("network", "https://api.example.com", "GET")) {
        std::visit([&invoker](auto& op) {
            invoker.addOperation(std::move(op));
        }, *operation3);
    }
    
    // Показываем информацию об операциях
    std::cout << invoker.getOperationsInfo() << std::endl;
    
    // Выполняем операции по одной
    std::cout << "=== Executing Operations One by One ===" << std::endl;
    for (size_t i = 0; i < invoker.getOperationCount(); ++i) {
        std::cout << "Executing operation " << i << "..." << std::endl;
        if (invoker.executeOperation(i)) {
            std::cout << "Operation " << i << " executed successfully" << std::endl;
        } else {
            std::cout << "Operation " << i << " failed" << std::endl;
        }
    }
    
    std::cout << "\nExecuted operations: " << invoker.getExecutedOperationCount() << std::endl;
    
    // Отменяем операции
    std::cout << "\n=== Undoing Operations ===" << std::endl;
    if (invoker.undoAll()) {
        std::cout << "All operations undone successfully" << std::endl;
    } else {
        std::cout << "Some operations could not be undone" << std::endl;
    }
    
    return 0;
}
```

---

## 📋 Задание 3: Command с Configuration

### Описание
Создайте команду, которая поддерживает конфигурацию и может быть легко настраиваемой:

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

// Конфигурация команды
struct CommandConfig {
    std::string name;
    std::string version;
    std::unordered_map<std::string, std::string> settings;
    std::unordered_map<std::string, std::any> customSettings;
    
    CommandConfig(const std::string& n, const std::string& v)
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
class ConfigurableCommand {
protected:
    CommandConfig config_;
    
public:
    ConfigurableCommand(const CommandConfig& config) : config_(config) {}
    
    virtual void validateConfig() const {
        if (config_.name.empty()) {
            throw std::invalid_argument("Command name cannot be empty");
        }
        if (config_.version.empty()) {
            throw std::invalid_argument("Command version cannot be empty");
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

// Конфигурируемая команда
class ConfigurableProcessingCommand : public Command, public ConfigurableCommand {
private:
    std::string commandType_;
    bool executed_;
    bool enabled_;
    
public:
    ConfigurableProcessingCommand(const CommandConfig& config) 
        : ConfigurableCommand(config), executed_(false), enabled_(true) {
        validateConfig();
        
        // Загружаем тип команды из конфигурации
        commandType_ = config_.getSetting("command.type");
        if (commandType_.empty()) {
            commandType_ = "default";
        }
        
        // Загружаем состояние включен/выключен
        if (auto enabled = config_.getCustomSetting<bool>("enabled")) {
            enabled_ = *enabled;
        }
    }
    
    void execute() override {
        if (!enabled_) {
            std::cout << "Command is disabled" << std::endl;
            return;
        }
        
        if (executed_) {
            std::cout << "Command already executed" << std::endl;
            return;
        }
        
        // Используем настройки из конфигурации
        std::string prefix = config_.getSetting("output.prefix");
        std::string suffix = config_.getSetting("output.suffix");
        
        std::string message = prefix + "Executing " + commandType_ + " command" + suffix;
        
        if (auto timestamp = config_.getCustomSetting<bool>("logging.timestamp")) {
            if (*timestamp) {
                message = "[TIMESTAMP] " + message;
            }
        }
        
        if (auto level = config_.getSetting("logging.level")) {
            message = "[" + level + "] " + message;
        }
        
        std::cout << message << std::endl;
        executed_ = true;
    }
    
    void undo() override {
        if (!enabled_) {
            std::cout << "Command is disabled" << std::endl;
            return;
        }
        
        if (!executed_) {
            std::cout << "Command not executed" << std::endl;
            return;
        }
        
        // Используем настройки из конфигурации
        std::string prefix = config_.getSetting("output.prefix");
        std::string suffix = config_.getSetting("output.suffix");
        
        std::string message = prefix + "Undoing " + commandType_ + " command" + suffix;
        
        if (auto timestamp = config_.getCustomSetting<bool>("logging.timestamp")) {
            if (*timestamp) {
                message = "[TIMESTAMP] " + message;
            }
        }
        
        if (auto level = config_.getSetting("logging.level")) {
            message = "[" + level + "] " + message;
        }
        
        std::cout << message << std::endl;
        executed_ = false;
    }
    
    std::string getName() const override {
        return config_.name;
    }
    
    std::string getDescription() const override {
        return config_.getSetting("command.description");
    }
    
    bool canUndo() const override {
        return executed_;
    }
    
    // Дополнительные методы
    void setEnabled(bool enabled) {
        enabled_ = enabled;
    }
    
    bool isEnabled() const {
        return enabled_;
    }
    
    void setCommandType(const std::string& type) {
        commandType_ = type;
    }
    
    std::string getCommandType() const {
        return commandType_;
    }
    
    void display() const {
        std::cout << "Configurable Processing Command: " << config_.name << " v" << config_.version << std::endl;
        std::cout << "  Command type: " << commandType_ << std::endl;
        std::cout << "  Enabled: " << (enabled_ ? "Yes" : "No") << std::endl;
        std::cout << "  Executed: " << (executed_ ? "Yes" : "No") << std::endl;
        std::cout << "  Settings:" << std::endl;
        for (const auto& setting : config_.settings) {
            std::cout << "    " << setting.first << ": " << setting.second << std::endl;
        }
    }
};

// Конфигурируемая фабрика команд
class ConfigurableCommandFactory {
private:
    CommandConfig config_;
    std::string commandType_;
    
public:
    ConfigurableCommandFactory(const CommandConfig& config, const std::string& type)
        : config_(config), commandType_(type) {}
    
    // Методы для настройки конфигурации
    ConfigurableCommandFactory& setSetting(const std::string& key, const std::string& value) {
        config_.setSetting(key, value);
        return *this;
    }
    
    template<typename T>
    ConfigurableCommandFactory& setCustomSetting(const std::string& key, const T& value) {
        config_.setCustomSetting(key, value);
        return *this;
    }
    
    // Создание команды
    std::unique_ptr<ConfigurableCommand> build() {
        if (commandType_ == "processing") {
            return std::make_unique<ConfigurableProcessingCommand>(config_);
        }
        throw std::invalid_argument("Unknown command type: " + commandType_);
    }
    
    // Создание команды с валидацией
    std::unique_ptr<ConfigurableCommand> buildSafe() {
        try {
            return build();
        } catch (const std::exception& e) {
            std::cerr << "Error building command: " << e.what() << std::endl;
            return nullptr;
        }
    }
    
    // Получение текущей конфигурации
    CommandConfig getConfig() const {
        return config_;
    }
    
    std::string getCommandType() const {
        return commandType_;
    }
};

// Менеджер конфигураций
class CommandConfigManager {
private:
    std::unordered_map<std::string, CommandConfig> configs_;
    
public:
    void registerConfig(const std::string& name, const CommandConfig& config) {
        configs_[name] = config;
    }
    
    std::optional<CommandConfig> getConfig(const std::string& name) const {
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
    
    std::unique_ptr<ConfigurableCommandFactory> createFactory(
        const std::string& configName, 
        const std::string& commandType
    ) {
        auto config = getConfig(configName);
        if (config) {
            return std::make_unique<ConfigurableCommandFactory>(*config, commandType);
        }
        return nullptr;
    }
};
```

### Пример использования
```cpp
int main() {
    CommandConfigManager configManager;
    
    // Регистрируем конфигурации
    CommandConfig fastCommandConfig("Fast Command", "1.0");
    fastCommandConfig.setSetting("command.type", "fast");
    fastCommandConfig.setSetting("command.description", "Fast processing command");
    fastCommandConfig.setSetting("output.prefix", "[FAST] ");
    fastCommandConfig.setSetting("output.suffix", " (completed)");
    fastCommandConfig.setSetting("logging.level", "INFO");
    fastCommandConfig.setCustomSetting("enabled", true);
    fastCommandConfig.setCustomSetting("logging.timestamp", true);
    
    CommandConfig slowCommandConfig("Slow Command", "2.0");
    slowCommandConfig.setSetting("command.type", "slow");
    slowCommandConfig.setSetting("command.description", "Slow but thorough processing command");
    slowCommandConfig.setSetting("output.prefix", "[SLOW] ");
    slowCommandConfig.setSetting("output.suffix", " (thoroughly completed)");
    slowCommandConfig.setSetting("logging.level", "DEBUG");
    slowCommandConfig.setCustomSetting("enabled", true);
    slowCommandConfig.setCustomSetting("logging.timestamp", false);
    
    configManager.registerConfig("fast", fastCommandConfig);
    configManager.registerConfig("slow", slowCommandConfig);
    
    // Создаем вызывающего
    auto receiver = std::make_shared<Receiver>();
    ModernInvoker invoker(receiver);
    
    // Создаем фабрику для быстрой команды
    auto fastCommandFactory = configManager.createFactory("fast", "processing");
    if (fastCommandFactory) {
        fastCommandFactory->setSetting("logging.level", "WARN");
        fastCommandFactory->setCustomSetting("enabled", true);
        
        if (auto command = fastCommandFactory->buildSafe()) {
            auto processingCommand = static_cast<ConfigurableProcessingCommand*>(command.get());
            processingCommand->display();
            
            invoker.executeCommand(std::move(command));
        }
    }
    
    // Создаем фабрику для медленной команды
    auto slowCommandFactory = configManager.createFactory("slow", "processing");
    if (slowCommandFactory) {
        slowCommandFactory->setSetting("logging.level", "TRACE");
        slowCommandFactory->setCustomSetting("enabled", true);
        
        if (auto command = slowCommandFactory->buildSafe()) {
            auto processingCommand = static_cast<ConfigurableProcessingCommand*>(command.get());
            processingCommand->display();
            
            invoker.executeCommand(std::move(command));
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

1. **Encapsulation**: Инкапсулируйте запросы как объекты
2. **Undo/Redo**: Реализуйте возможность отмены и повтора операций
3. **Error Handling**: Обрабатывайте ошибки выполнения команд
4. **Configuration**: Используйте конфигурацию для настройки команд
5. **Testing**: Создавайте тесты для проверки команд

---

*Помните: Command инкапсулирует запросы как объекты для их выполнения!*
