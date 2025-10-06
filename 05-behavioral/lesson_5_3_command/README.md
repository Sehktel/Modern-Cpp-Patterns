# Урок 5.3: Command Pattern (Команда)

## 🎯 Цель урока
Изучить паттерн Command - один из наиболее мощных поведенческих паттернов. Понять, как инкапсулировать запросы как объекты и как реализовать системы отмены операций, очереди команд и макросы.

## 📚 Что изучаем

### 1. Паттерн Command
- **Определение**: Инкапсулирует запрос как объект, позволяя параметризовать клиентов различными запросами
- **Назначение**: Отложенное выполнение, отмена операций, логирование, очереди команд
- **Применение**: GUI приложения, системы отмены, макросы, очереди задач, транзакции

### 2. Компоненты паттерна
- **Command (Команда)**: Интерфейс для выполнения операций
- **ConcreteCommand**: Конкретная реализация команды
- **Invoker (Вызывающий)**: Объект, который вызывает команду
- **Receiver (Получатель)**: Объект, который выполняет операцию
- **Client (Клиент)**: Создает команды и устанавливает получателей

### 3. Расширенные возможности
- **Undo/Redo**: Система отмены и повтора операций
- **Macro Commands**: Составные команды
- **Command Queue**: Очередь команд для отложенного выполнения
- **Command History**: История выполненных команд

## 🔍 Ключевые концепции

### Базовый Command
```cpp
// Интерфейс команды
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
};

// Получатель
class Light {
public:
    void turnOn() { std::cout << "Свет включен" << std::endl; }
    void turnOff() { std::cout << "Свет выключен" << std::endl; }
};

// Конкретная команда
class LightOnCommand : public Command {
private:
    Light* light_;
    
public:
    LightOnCommand(Light* light) : light_(light) {}
    
    void execute() override {
        light_->turnOn();
    }
    
    void undo() override {
        light_->turnOff();
    }
};

// Вызывающий
class RemoteControl {
private:
    std::unique_ptr<Command> command_;
    
public:
    void setCommand(std::unique_ptr<Command> command) {
        command_ = std::move(command);
    }
    
    void pressButton() {
        if (command_) {
            command_->execute();
        }
    }
};
```

### Command с параметрами
```cpp
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
};
```

## 🤔 Вопросы для размышления

### 1. Когда использовать Command?
**✅ Хорошие случаи:**
- Нужна система отмены операций
- Требуется отложенное выполнение команд
- Нужно логировать запросы
- Требуется поддержка макросов
- Нужна очередь команд
- Транзакционные операции

**❌ Плохие случаи:**
- Простые операции без необходимости отмены
- Когда команды не изменяются
- Когда добавляется ненужная сложность
- Простые вызовы функций

### 2. Преимущества Command
- **Инкапсуляция**: Запросы инкапсулированы как объекты
- **Отмена операций**: Легко реализовать undo/redo
- **Отложенное выполнение**: Команды можно выполнять позже
- **Логирование**: Легко логировать команды
- **Макросы**: Можно создавать составные команды

### 3. Недостатки Command
- **Сложность**: Добавляет дополнительные классы
- **Производительность**: Накладные расходы на объекты команд
- **Память**: Каждая команда требует памяти
- **Понимание**: Может усложнить понимание кода

## 🛠️ Практические примеры

### Система отмены в текстовом редакторе
```cpp
class TextEditor {
private:
    std::string content_;
    
public:
    void insertText(size_t position, const std::string& text) {
        content_.insert(position, text);
    }
    
    void deleteText(size_t position, size_t length) {
        content_.erase(position, length);
    }
    
    const std::string& getContent() const { return content_; }
    size_t getLength() const { return content_.length(); }
};

class InsertTextCommand : public Command {
private:
    TextEditor* editor_;
    size_t position_;
    std::string text_;
    
public:
    InsertTextCommand(TextEditor* editor, size_t position, const std::string& text)
        : editor_(editor), position_(position), text_(text) {}
    
    void execute() override {
        editor_->insertText(position_, text_);
    }
    
    void undo() override {
        editor_->deleteText(position_, text_.length());
    }
};
```

### Система управления файлами
```cpp
class FileManager {
public:
    void createFile(const std::string& filename) {
        std::cout << "Создан файл: " << filename << std::endl;
    }
    
    void deleteFile(const std::string& filename) {
        std::cout << "Удален файл: " << filename << std::endl;
    }
    
    void moveFile(const std::string& from, const std::string& to) {
        std::cout << "Файл перемещен: " << from << " -> " << to << std::endl;
    }
};

class CreateFileCommand : public Command {
private:
    FileManager* fileManager_;
    std::string filename_;
    
public:
    CreateFileCommand(FileManager* fm, const std::string& filename)
        : fileManager_(fm), filename_(filename) {}
    
    void execute() override {
        fileManager_->createFile(filename_);
    }
    
    void undo() override {
        fileManager_->deleteFile(filename_);
    }
};
```

## 🎨 Современные подходы в C++

### Functional Command
```cpp
class FunctionalCommand {
private:
    std::function<void()> executeFunc_;
    std::function<void()> undoFunc_;
    
public:
    FunctionalCommand(std::function<void()> execute, std::function<void()> undo)
        : executeFunc_(execute), undoFunc_(undo) {}
    
    void execute() {
        if (executeFunc_) {
            executeFunc_();
        }
    }
    
    void undo() {
        if (undoFunc_) {
            undoFunc_();
        }
    }
};

// Использование
auto command = FunctionalCommand(
    []() { std::cout << "Выполняем операцию" << std::endl; },
    []() { std::cout << "Отменяем операцию" << std::endl; }
);
```

### Template Command
```cpp
template<typename Receiver, typename ExecuteFunc, typename UndoFunc>
class TemplateCommand {
private:
    Receiver* receiver_;
    ExecuteFunc executeFunc_;
    UndoFunc undoFunc_;
    
public:
    TemplateCommand(Receiver* receiver, ExecuteFunc execute, UndoFunc undo)
        : receiver_(receiver), executeFunc_(execute), undoFunc_(undo) {}
    
    void execute() {
        executeFunc_(receiver_);
    }
    
    void undo() {
        undoFunc_(receiver_);
    }
};
```

### Macro Command
```cpp
class MacroCommand : public Command {
private:
    std::vector<std::unique_ptr<Command>> commands_;
    
public:
    void addCommand(std::unique_ptr<Command> command) {
        commands_.push_back(std::move(command));
    }
    
    void execute() override {
        for (auto& command : commands_) {
            command->execute();
        }
    }
    
    void undo() override {
        // Отменяем в обратном порядке
        for (auto it = commands_.rbegin(); it != commands_.rend(); ++it) {
            (*it)->undo();
        }
    }
};
```

## 🧪 Тестирование Command

### Создание моков для тестирования
```cpp
class MockReceiver {
public:
    MOCK_METHOD(void, performAction, (), ());
    MOCK_METHOD(void, undoAction, (), ());
};

class MockCommand : public Command {
private:
    MockReceiver* receiver_;
    
public:
    explicit MockCommand(MockReceiver* receiver) : receiver_(receiver) {}
    
    void execute() override {
        receiver_->performAction();
    }
    
    void undo() override {
        receiver_->undoAction();
    }
};

// Тест
TEST(CommandTest, ExecuteAndUndo) {
    MockReceiver mockReceiver;
    MockCommand command(&mockReceiver);
    
    EXPECT_CALL(mockReceiver, performAction()).Times(1);
    EXPECT_CALL(mockReceiver, undoAction()).Times(1);
    
    command.execute();
    command.undo();
}
```

## 🎯 Практические упражнения

### Упражнение 1: Текстовый редактор
Создайте систему отмены для текстового редактора с командами вставки, удаления и форматирования.

### Упражнение 2: Графический редактор
Реализуйте команды для рисования фигур с возможностью отмены и повтора.

### Упражнение 3: Система управления задачами
Создайте систему команд для управления задачами с историей операций.

### Упражнение 4: Игровые команды
Реализуйте систему команд для игрового персонажа с возможностью отмены действий.

## 📈 Связь с другими паттернами

### Command vs Strategy
- **Command**: Инкапсулирует запрос как объект
- **Strategy**: Инкапсулирует алгоритм

### Command vs Observer
- **Command**: Отложенное выполнение операций
- **Observer**: Реакция на изменения состояния

### Command vs Memento
- **Command**: Отмена операций через выполнение обратных действий
- **Memento**: Отмена операций через восстановление состояния

## 📈 Следующие шаги
После изучения Command вы будете готовы к:
- Уроку 5.4: State Pattern
- Пониманию поведенческих паттернов
- Созданию систем отмены
- Разработке пользовательских интерфейсов

## 💡 Важные принципы

1. **Инкапсуляция запросов**: Каждая команда должна быть самодостаточной
2. **Обратимость**: Всегда предусматривайте возможность отмены
3. **Логирование**: Команды легко логировать и воспроизводить
4. **Композиция**: Используйте макрокоманды для сложных операций
5. **Производительность**: Учитывайте накладные расходы на объекты команд
