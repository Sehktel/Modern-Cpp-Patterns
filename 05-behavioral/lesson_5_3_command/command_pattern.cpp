#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <stack>
#include <queue>
#include <chrono>
#include <thread>

/**
 * @file command_pattern.cpp
 * @brief Демонстрация паттерна Command
 * 
 * Этот файл показывает различные реализации паттерна Command,
 * от базовых до продвинутых с системами отмены и макрокомандами.
 */

// ============================================================================
// БАЗОВЫЙ COMMAND PATTERN
// ============================================================================

/**
 * @brief Интерфейс команды
 */
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string getDescription() const = 0;
};

/**
 * @brief Получатель - Свет
 */
class Light {
private:
    bool isOn_;
    int brightness_;
    
public:
    Light() : isOn_(false), brightness_(0) {}
    
    void turnOn() {
        isOn_ = true;
        brightness_ = 100;
        std::cout << "💡 Свет включен (яркость: " << brightness_ << "%)" << std::endl;
    }
    
    void turnOff() {
        isOn_ = false;
        brightness_ = 0;
        std::cout << "💡 Свет выключен" << std::endl;
    }
    
    void setBrightness(int brightness) {
        if (isOn_) {
            brightness_ = brightness;
            std::cout << "💡 Яркость изменена на " << brightness_ << "%" << std::endl;
        }
    }
    
    bool isOn() const { return isOn_; }
    int getBrightness() const { return brightness_; }
};

/**
 * @brief Команда включения света
 */
class LightOnCommand : public Command {
private:
    Light* light_;
    
public:
    explicit LightOnCommand(Light* light) : light_(light) {}
    
    void execute() override {
        light_->turnOn();
    }
    
    void undo() override {
        light_->turnOff();
    }
    
    std::string getDescription() const override {
        return "Включить свет";
    }
};

/**
 * @brief Команда выключения света
 */
class LightOffCommand : public Command {
private:
    Light* light_;
    
public:
    explicit LightOffCommand(Light* light) : light_(light) {}
    
    void execute() override {
        light_->turnOff();
    }
    
    void undo() override {
        light_->turnOn();
    }
    
    std::string getDescription() const override {
        return "Выключить свет";
    }
};

/**
 * @brief Команда изменения яркости
 */
class BrightnessCommand : public Command {
private:
    Light* light_;
    int newBrightness_;
    int oldBrightness_;
    
public:
    BrightnessCommand(Light* light, int brightness) 
        : light_(light), newBrightness_(brightness), oldBrightness_(light->getBrightness()) {}
    
    void execute() override {
        light_->setBrightness(newBrightness_);
    }
    
    void undo() override {
        light_->setBrightness(oldBrightness_);
    }
    
    std::string getDescription() const override {
        return "Изменить яркость на " + std::to_string(newBrightness_) + "%";
    }
};

/**
 * @brief Вызывающий - Пульт управления
 */
class RemoteControl {
private:
    std::unique_ptr<Command> command_;
    std::string name_;
    
public:
    explicit RemoteControl(const std::string& name) : name_(name) {}
    
    void setCommand(std::unique_ptr<Command> command) {
        command_ = std::move(command);
    }
    
    void pressButton() {
        if (command_) {
            std::cout << "🎮 " << name_ << " нажимает кнопку: " << command_->getDescription() << std::endl;
            command_->execute();
        } else {
            std::cout << "🎮 " << name_ << ": Команда не установлена!" << std::endl;
        }
    }
    
    void pressUndoButton() {
        if (command_) {
            std::cout << "↩️ " << name_ << " отменяет: " << command_->getDescription() << std::endl;
            command_->undo();
        } else {
            std::cout << "↩️ " << name_ << ": Команда не установлена!" << std::endl;
        }
    }
};

// ============================================================================
// СИСТЕМА ОТМЕНЫ С ИСТОРИЕЙ
// ============================================================================

/**
 * @brief Менеджер команд с историей
 */
class CommandManager {
private:
    std::stack<std::unique_ptr<Command>> history_;
    std::stack<std::unique_ptr<Command>> redoStack_;
    
public:
    void executeCommand(std::unique_ptr<Command> command) {
        command->execute();
        history_.push(std::move(command));
        
        // Очищаем стек повтора при выполнении новой команды
        while (!redoStack_.empty()) {
            redoStack_.pop();
        }
    }
    
    void undo() {
        if (!history_.empty()) {
            auto command = std::move(history_.top());
            history_.pop();
            
            command->undo();
            redoStack_.push(std::move(command));
        } else {
            std::cout << "❌ Нет команд для отмены" << std::endl;
        }
    }
    
    void redo() {
        if (!redoStack_.empty()) {
            auto command = std::move(redoStack_.top());
            redoStack_.pop();
            
            command->execute();
            history_.push(std::move(command));
        } else {
            std::cout << "❌ Нет команд для повтора" << std::endl;
        }
    }
    
    bool canUndo() const {
        return !history_.empty();
    }
    
    bool canRedo() const {
        return !redoStack_.empty();
    }
    
    size_t getHistorySize() const {
        return history_.size();
    }
    
    size_t getRedoStackSize() const {
        return redoStack_.size();
    }
};

// ============================================================================
// МАКРОКОМАНДЫ
// ============================================================================

/**
 * @brief Макрокоманда - составная команда
 */
class MacroCommand : public Command {
private:
    std::vector<std::unique_ptr<Command>> commands_;
    std::string macroName_;
    
public:
    explicit MacroCommand(const std::string& name) : macroName_(name) {}
    
    void addCommand(std::unique_ptr<Command> command) {
        commands_.push_back(std::move(command));
    }
    
    void execute() override {
        std::cout << "🎬 Выполняется макрокоманда: " << macroName_ << std::endl;
        for (auto& command : commands_) {
            command->execute();
        }
    }
    
    void undo() override {
        std::cout << "↩️ Отмена макрокоманды: " << macroName_ << std::endl;
        // Отменяем команды в обратном порядке
        for (auto it = commands_.rbegin(); it != commands_.rend(); ++it) {
            (*it)->undo();
        }
    }
    
    std::string getDescription() const override {
        return "Макрокоманда: " + macroName_ + " (" + std::to_string(commands_.size()) + " команд)";
    }
    
    size_t getCommandCount() const {
        return commands_.size();
    }
};

// ============================================================================
// ОЧЕРЕДЬ КОМАНД
// ============================================================================

/**
 * @brief Очередь команд для отложенного выполнения
 */
class CommandQueue {
private:
    std::queue<std::unique_ptr<Command>> commandQueue_;
    std::string queueName_;
    
public:
    explicit CommandQueue(const std::string& name) : queueName_(name) {}
    
    void addCommand(std::unique_ptr<Command> command) {
        commandQueue_.push(std::move(command));
        std::cout << "📋 Команда добавлена в очередь '" << queueName_ << "'" << std::endl;
    }
    
    void processNext() {
        if (!commandQueue_.empty()) {
            auto command = std::move(commandQueue_.front());
            commandQueue_.pop();
            
            std::cout << "⚡ Обрабатывается команда из очереди '" << queueName_ << "': " 
                      << command->getDescription() << std::endl;
            command->execute();
        } else {
            std::cout << "📋 Очередь '" << queueName_ << "' пуста" << std::endl;
        }
    }
    
    void processAll() {
        std::cout << "🔄 Обработка всех команд в очереди '" << queueName_ << "'" << std::endl;
        while (!commandQueue_.empty()) {
            processNext();
        }
    }
    
    size_t getQueueSize() const {
        return commandQueue_.size();
    }
    
    bool isEmpty() const {
        return commandQueue_.empty();
    }
};

// ============================================================================
// ФУНКЦИОНАЛЬНЫЕ КОМАНДЫ
// ============================================================================

/**
 * @brief Функциональная команда с использованием std::function
 */
class FunctionalCommand : public Command {
private:
    std::function<void()> executeFunc_;
    std::function<void()> undoFunc_;
    std::string description_;
    
public:
    FunctionalCommand(std::function<void()> execute, 
                     std::function<void()> undo, 
                     const std::string& description)
        : executeFunc_(execute), undoFunc_(undo), description_(description) {}
    
    void execute() override {
        if (executeFunc_) {
            executeFunc_();
        }
    }
    
    void undo() override {
        if (undoFunc_) {
            undoFunc_();
        }
    }
    
    std::string getDescription() const override {
        return description_;
    }
};

// ============================================================================
// ИГРОВЫЕ КОМАНДЫ
// ============================================================================

/**
 * @brief Игровой объект
 */
class GameObject {
private:
    std::string name_;
    int x_, y_;
    
public:
    GameObject(const std::string& name, int x = 0, int y = 0) 
        : name_(name), x_(x), y_(y) {}
    
    void move(int deltaX, int deltaY) {
        x_ += deltaX;
        y_ += deltaY;
        std::cout << "🎮 " << name_ << " перемещен в (" << x_ << "," << y_ << ")" << std::endl;
    }
    
    void moveTo(int newX, int newY) {
        x_ = newX;
        y_ = newY;
        std::cout << "🎮 " << name_ << " перемещен в (" << x_ << "," << y_ << ")" << std::endl;
    }
    
    void attack() {
        std::cout << "⚔️ " << name_ << " атакует!" << std::endl;
    }
    
    void defend() {
        std::cout << "🛡️ " << name_ << " защищается!" << std::endl;
    }
    
    int getX() const { return x_; }
    int getY() const { return y_; }
    const std::string& getName() const { return name_; }
};

/**
 * @brief Команда движения
 */
class MoveCommand : public Command {
private:
    GameObject* object_;
    int deltaX_, deltaY_;
    int oldX_, oldY_;
    
public:
    MoveCommand(GameObject* object, int deltaX, int deltaY)
        : object_(object), deltaX_(deltaX), deltaY_(deltaY) {
        oldX_ = object->getX();
        oldY_ = object->getY();
    }
    
    void execute() override {
        object_->move(deltaX_, deltaY_);
    }
    
    void undo() override {
        object_->moveTo(oldX_, oldY_);
    }
    
    std::string getDescription() const override {
        return "Переместить " + object_->getName() + " на (" + 
               std::to_string(deltaX_) + "," + std::to_string(deltaY_) + ")";
    }
};

/**
 * @brief Команда атаки
 */
class AttackCommand : public Command {
private:
    GameObject* object_;
    
public:
    explicit AttackCommand(GameObject* object) : object_(object) {}
    
    void execute() override {
        object_->attack();
    }
    
    void undo() override {
        // Отмена атаки - защита
        object_->defend();
    }
    
    std::string getDescription() const override {
        return "Атака " + object_->getName();
    }
};

// ============================================================================
// ДЕМОНСТРАЦИОННЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Демонстрация базового Command
 */
void demonstrateBasicCommand() {
    std::cout << "\n=== Базовый Command Pattern ===" << std::endl;
    
    Light light;
    RemoteControl remote("Универсальный пульт");
    
    // Устанавливаем команду включения света
    remote.setCommand(std::make_unique<LightOnCommand>(&light));
    remote.pressButton();
    
    // Меняем команду на изменение яркости
    remote.setCommand(std::make_unique<BrightnessCommand>(&light, 50));
    remote.pressButton();
    
    // Отменяем последнюю команду
    remote.pressUndoButton();
    
    // Устанавливаем команду выключения
    remote.setCommand(std::make_unique<LightOffCommand>(&light));
    remote.pressButton();
}

/**
 * @brief Демонстрация системы отмены
 */
void demonstrateUndoRedo() {
    std::cout << "\n=== Система отмены и повтора ===" << std::endl;
    
    Light light;
    CommandManager manager;
    
    // Выполняем несколько команд
    manager.executeCommand(std::make_unique<LightOnCommand>(&light));
    manager.executeCommand(std::make_unique<BrightnessCommand>(&light, 75));
    manager.executeCommand(std::make_unique<BrightnessCommand>(&light, 25));
    
    std::cout << "\nИстория команд: " << manager.getHistorySize() << std::endl;
    
    // Отменяем команды
    std::cout << "\n--- Отмена команд ---" << std::endl;
    manager.undo();
    manager.undo();
    
    std::cout << "Можно отменить: " << (manager.canUndo() ? "Да" : "Нет") << std::endl;
    std::cout << "Можно повторить: " << (manager.canRedo() ? "Да" : "Нет") << std::endl;
    
    // Повторяем команды
    std::cout << "\n--- Повтор команд ---" << std::endl;
    manager.redo();
    manager.redo();
}

/**
 * @brief Демонстрация макрокоманд
 */
void demonstrateMacroCommands() {
    std::cout << "\n=== Макрокоманды ===" << std::endl;
    
    Light light1, light2;
    
    // Создаем макрокоманду "Вечерний режим"
    auto eveningMode = std::make_unique<MacroCommand>("Вечерний режим");
    eveningMode->addCommand(std::make_unique<LightOnCommand>(&light1));
    eveningMode->addCommand(std::make_unique<BrightnessCommand>(&light1, 30));
    eveningMode->addCommand(std::make_unique<LightOnCommand>(&light2));
    eveningMode->addCommand(std::make_unique<BrightnessCommand>(&light2, 20));
    
    std::cout << "Создана макрокоманда: " << eveningMode->getDescription() << std::endl;
    
    // Выполняем макрокоманду
    eveningMode->execute();
    
    // Отменяем макрокоманду
    eveningMode->undo();
}

/**
 * @brief Демонстрация очереди команд
 */
void demonstrateCommandQueue() {
    std::cout << "\n=== Очередь команд ===" << std::endl;
    
    Light light;
    CommandQueue queue("Очередь освещения");
    
    // Добавляем команды в очередь
    queue.addCommand(std::make_unique<LightOnCommand>(&light));
    queue.addCommand(std::make_unique<BrightnessCommand>(&light, 60));
    queue.addCommand(std::make_unique<BrightnessCommand>(&light, 80));
    queue.addCommand(std::make_unique<BrightnessCommand>(&light, 100));
    
    std::cout << "Размер очереди: " << queue.getQueueSize() << std::endl;
    
    // Обрабатываем команды по одной
    std::cout << "\n--- Обработка по одной команде ---" << std::endl;
    queue.processNext();
    queue.processNext();
    
    std::cout << "Осталось в очереди: " << queue.getQueueSize() << std::endl;
    
    // Обрабатываем все оставшиеся команды
    std::cout << "\n--- Обработка всех оставшихся команд ---" << std::endl;
    queue.processAll();
}

/**
 * @brief Демонстрация функциональных команд
 */
void demonstrateFunctionalCommands() {
    std::cout << "\n=== Функциональные команды ===" << std::endl;
    
    int counter = 0;
    
    // Создаем функциональную команду
    auto incrementCommand = std::make_unique<FunctionalCommand>(
        [&counter]() {
            ++counter;
            std::cout << "Счетчик увеличен до: " << counter << std::endl;
        },
        [&counter]() {
            --counter;
            std::cout << "Счетчик уменьшен до: " << counter << std::endl;
        },
        "Увеличить счетчик"
    );
    
    // Выполняем команду
    incrementCommand->execute();
    incrementCommand->execute();
    
    // Отменяем команду
    incrementCommand->undo();
}

/**
 * @brief Демонстрация игровых команд
 */
void demonstrateGameCommands() {
    std::cout << "\n=== Игровые команды ===" << std::endl;
    
    GameObject player("Игрок", 0, 0);
    GameObject enemy("Враг", 5, 5);
    
    CommandManager gameManager;
    
    // Выполняем игровые команды
    gameManager.executeCommand(std::make_unique<MoveCommand>(&player, 2, 3));
    gameManager.executeCommand(std::make_unique<AttackCommand>(&player));
    gameManager.executeCommand(std::make_unique<MoveCommand>(&player, 1, 1));
    
    std::cout << "\nПозиция игрока: (" << player.getX() << "," << player.getY() << ")" << std::endl;
    
    // Отменяем последние действия
    std::cout << "\n--- Отмена действий ---" << std::endl;
    gameManager.undo();
    gameManager.undo();
    
    std::cout << "Позиция игрока после отмены: (" << player.getX() << "," << player.getY() << ")" << std::endl;
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🎮 Демонстрация паттерна Command" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    demonstrateBasicCommand();
    demonstrateUndoRedo();
    demonstrateMacroCommands();
    demonstrateCommandQueue();
    demonstrateFunctionalCommands();
    demonstrateGameCommands();
    
    std::cout << "\n✅ Демонстрация Command Pattern завершена!" << std::endl;
    std::cout << "\n🎯 Ключевые выводы:" << std::endl;
    std::cout << "• Command инкапсулирует запросы как объекты" << std::endl;
    std::cout << "• Система отмены легко реализуется через undo операции" << std::endl;
    std::cout << "• Макрокоманды позволяют создавать сложные составные операции" << std::endl;
    std::cout << "• Очереди команд поддерживают отложенное выполнение" << std::endl;
    std::cout << "• Функциональные команды упрощают создание простых операций" << std::endl;
    
    return 0;
}
