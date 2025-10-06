#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <stack>
#include <map>
#include <chrono>
#include <thread>

/**
 * @file undo_redo.cpp
 * @brief Продвинутая система отмены и повтора операций
 * 
 * Этот файл демонстрирует сложную систему отмены операций
 * с поддержкой группировки, временных меток и ограничений памяти.
 */

// ============================================================================
// ПРОДВИНУТАЯ СИСТЕМА ОТМЕНЫ
// ============================================================================

/**
 * @brief Расширенная команда с метаданными
 */
class AdvancedCommand {
public:
    virtual ~AdvancedCommand() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string getDescription() const = 0;
    virtual std::chrono::system_clock::time_point getTimestamp() const = 0;
    virtual bool canGroupWith(const AdvancedCommand& other) const = 0;
    virtual std::unique_ptr<AdvancedCommand> clone() const = 0;
};

/**
 * @brief Менеджер команд с группировкой
 */
class AdvancedCommandManager {
private:
    struct CommandGroup {
        std::vector<std::unique_ptr<AdvancedCommand>> commands_;
        std::string groupName_;
        std::chrono::system_clock::time_point timestamp_;
        
        CommandGroup(const std::string& name) 
            : groupName_(name), timestamp_(std::chrono::system_clock::now()) {}
    };
    
    std::stack<std::unique_ptr<CommandGroup>> history_;
    std::stack<std::unique_ptr<CommandGroup>> redoStack_;
    std::unique_ptr<CommandGroup> currentGroup_;
    size_t maxHistorySize_;
    
public:
    explicit AdvancedCommandManager(size_t maxSize = 50) 
        : maxHistorySize_(maxSize) {}
    
    void startGroup(const std::string& groupName) {
        if (currentGroup_) {
            endGroup();
        }
        currentGroup_ = std::make_unique<CommandGroup>(groupName);
        std::cout << "📁 Начата группа команд: " << groupName << std::endl;
    }
    
    void endGroup() {
        if (currentGroup_ && !currentGroup_->commands_.empty()) {
            history_.push(std::move(currentGroup_));
            currentGroup_ = nullptr;
            
            // Очищаем стек повтора при выполнении новой группы
            while (!redoStack_.empty()) {
                redoStack_.pop();
            }
            
            // Ограничиваем размер истории
            while (history_.size() > maxHistorySize_) {
                history_.pop();
            }
        }
    }
    
    void executeCommand(std::unique_ptr<AdvancedCommand> command) {
        if (currentGroup_) {
            // Добавляем в текущую группу
            currentGroup_->commands_.push_back(std::move(command));
            currentGroup_->commands_.back()->execute();
        } else {
            // Создаем группу из одной команды
            auto group = std::make_unique<CommandGroup>("Одиночная команда");
            group->commands_.push_back(std::move(command));
            group->commands_.back()->execute();
            history_.push(std::move(group));
            
            // Очищаем стек повтора
            while (!redoStack_.empty()) {
                redoStack_.pop();
            }
            
            // Ограничиваем размер истории
            while (history_.size() > maxHistorySize_) {
                history_.pop();
            }
        }
    }
    
    void undo() {
        if (!history_.empty()) {
            auto group = std::move(history_.top());
            history_.pop();
            
            std::cout << "↩️ Отмена группы: " << group->groupName_ 
                      << " (" << group->commands_.size() << " команд)" << std::endl;
            
            // Отменяем команды в обратном порядке
            for (auto it = group->commands_.rbegin(); it != group->commands_.rend(); ++it) {
                (*it)->undo();
            }
            
            redoStack_.push(std::move(group));
        } else {
            std::cout << "❌ Нет команд для отмены" << std::endl;
        }
    }
    
    void redo() {
        if (!redoStack_.empty()) {
            auto group = std::move(redoStack_.top());
            redoStack_.pop();
            
            std::cout << "🔄 Повтор группы: " << group->groupName_ 
                      << " (" << group->commands_.size() << " команд)" << std::endl;
            
            // Выполняем команды в прямом порядке
            for (auto& command : group->commands_) {
                command->execute();
            }
            
            history_.push(std::move(group));
        } else {
            std::cout << "❌ Нет команд для повтора" << std::endl;
        }
    }
    
    void undoGroup(const std::string& groupName) {
        std::stack<std::unique_ptr<CommandGroup>> tempStack;
        
        // Ищем группу с нужным именем
        while (!history_.empty()) {
            auto group = std::move(history_.top());
            history_.pop();
            
            if (group->groupName_ == groupName) {
                std::cout << "↩️ Отмена конкретной группы: " << groupName << std::endl;
                
                // Отменяем команды
                for (auto it = group->commands_.rbegin(); it != group->commands_.rend(); ++it) {
                    (*it)->undo();
                }
                
                redoStack_.push(std::move(group));
                break;
            } else {
                tempStack.push(std::move(group));
            }
        }
        
        // Восстанавливаем стек
        while (!tempStack.empty()) {
            history_.push(std::move(tempStack.top()));
            tempStack.pop();
        }
    }
    
    void showHistory() const {
        std::cout << "\n📚 История команд:" << std::endl;
        
        std::stack<std::unique_ptr<CommandGroup>> tempStack = history_;
        int index = 1;
        
        while (!tempStack.empty()) {
            const auto& group = tempStack.top();
            auto time_t = std::chrono::system_clock::to_time_t(group->timestamp_);
            
            std::cout << "  " << index << ". " << group->groupName_ 
                      << " (" << group->commands_.size() << " команд) - " 
                      << std::ctime(&time_t);
            
            tempStack.pop();
            ++index;
        }
        
        std::cout << "Всего групп: " << history_.size() << std::endl;
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
    
    void clearHistory() {
        while (!history_.empty()) {
            history_.pop();
        }
        while (!redoStack_.empty()) {
            redoStack_.pop();
        }
        std::cout << "🗑️ История команд очищена" << std::endl;
    }
};

// ============================================================================
// КОНКРЕТНЫЕ КОМАНДЫ
// ============================================================================

/**
 * @brief Команда изменения текста
 */
class TextChangeCommand : public AdvancedCommand {
private:
    std::string* text_;
    std::string oldText_;
    std::string newText_;
    std::string description_;
    std::chrono::system_clock::time_point timestamp_;
    
public:
    TextChangeCommand(std::string* text, const std::string& newText, const std::string& description)
        : text_(text), newText_(newText), description_(description), 
          timestamp_(std::chrono::system_clock::now()) {
        oldText_ = *text_;
    }
    
    void execute() override {
        *text_ = newText_;
        std::cout << "📝 " << description_ << ": \"" << newText_ << "\"" << std::endl;
    }
    
    void undo() override {
        *text_ = oldText_;
        std::cout << "↩️ Отмена: \"" << oldText_ << "\"" << std::endl;
    }
    
    std::string getDescription() const override {
        return description_;
    }
    
    std::chrono::system_clock::time_point getTimestamp() const override {
        return timestamp_;
    }
    
    bool canGroupWith(const AdvancedCommand& other) const override {
        const auto* otherTextCmd = dynamic_cast<const TextChangeCommand*>(&other);
        return otherTextCmd && otherTextCmd->text_ == text_;
    }
    
    std::unique_ptr<AdvancedCommand> clone() const override {
        return std::make_unique<TextChangeCommand>(text_, newText_, description_);
    }
};

/**
 * @brief Команда изменения числового значения
 */
class NumberChangeCommand : public AdvancedCommand {
private:
    int* value_;
    int oldValue_;
    int newValue_;
    std::string description_;
    std::chrono::system_clock::time_point timestamp_;
    
public:
    NumberChangeCommand(int* value, int newValue, const std::string& description)
        : value_(value), newValue_(newValue), description_(description),
          timestamp_(std::chrono::system_clock::now()) {
        oldValue_ = *value_;
    }
    
    void execute() override {
        *value_ = newValue_;
        std::cout << "🔢 " << description_ << ": " << newValue_ << std::endl;
    }
    
    void undo() override {
        *value_ = oldValue_;
        std::cout << "↩️ Отмена: " << oldValue_ << std::endl;
    }
    
    std::string getDescription() const override {
        return description_;
    }
    
    std::chrono::system_clock::time_point getTimestamp() const override {
        return timestamp_;
    }
    
    bool canGroupWith(const AdvancedCommand& other) const override {
        const auto* otherNumCmd = dynamic_cast<const NumberChangeCommand*>(&other);
        return otherNumCmd && otherNumCmd->value_ == value_;
    }
    
    std::unique_ptr<AdvancedCommand> clone() const override {
        return std::make_unique<NumberChangeCommand>(value_, newValue_, description_);
    }
};

// ============================================================================
// СИСТЕМА СОХРАНЕНИЯ И ЗАГРУЗКИ СОСТОЯНИЯ
// ============================================================================

/**
 * @brief Интерфейс для сохранения состояния
 */
class Stateful {
public:
    virtual ~Stateful() = default;
    virtual std::string serialize() const = 0;
    virtual void deserialize(const std::string& data) = 0;
    virtual std::string getStateDescription() const = 0;
};

/**
 * @brief Команда сохранения состояния
 */
class SaveStateCommand : public AdvancedCommand {
private:
    Stateful* object_;
    std::string savedState_;
    std::string description_;
    std::chrono::system_clock::time_point timestamp_;
    
public:
    SaveStateCommand(Stateful* object, const std::string& description)
        : object_(object), description_(description),
          timestamp_(std::chrono::system_clock::now()) {
        savedState_ = object_->serialize();
    }
    
    void execute() override {
        std::cout << "💾 Сохранение состояния: " << description_ << std::endl;
        std::cout << "   Состояние: " << object_->getStateDescription() << std::endl;
    }
    
    void undo() override {
        object_->deserialize(savedState_);
        std::cout << "↩️ Восстановление состояния: " << description_ << std::endl;
    }
    
    std::string getDescription() const override {
        return "Сохранение: " + description_;
    }
    
    std::chrono::system_clock::time_point getTimestamp() const override {
        return timestamp_;
    }
    
    bool canGroupWith(const AdvancedCommand& other) const override {
        return false; // Сохранения не группируются
    }
    
    std::unique_ptr<AdvancedCommand> clone() const override {
        return std::make_unique<SaveStateCommand>(object_, description_);
    }
};

/**
 * @brief Пример объекта с состоянием
 */
class Document : public Stateful {
private:
    std::string content_;
    int fontSize_;
    std::string fontFamily_;
    
public:
    Document() : fontSize_(12), fontFamily_("Arial") {}
    
    void setContent(const std::string& content) {
        content_ = content;
    }
    
    void setFontSize(int size) {
        fontSize_ = size;
    }
    
    void setFontFamily(const std::string& family) {
        fontFamily_ = family;
    }
    
    std::string serialize() const override {
        return content_ + "|" + std::to_string(fontSize_) + "|" + fontFamily_;
    }
    
    void deserialize(const std::string& data) override {
        size_t pos1 = data.find('|');
        size_t pos2 = data.find('|', pos1 + 1);
        
        if (pos1 != std::string::npos && pos2 != std::string::npos) {
            content_ = data.substr(0, pos1);
            fontSize_ = std::stoi(data.substr(pos1 + 1, pos2 - pos1 - 1));
            fontFamily_ = data.substr(pos2 + 1);
        }
    }
    
    std::string getStateDescription() const override {
        return "Документ: \"" + content_ + "\", размер: " + std::to_string(fontSize_) + 
               ", шрифт: " + fontFamily_;
    }
    
    const std::string& getContent() const { return content_; }
    int getFontSize() const { return fontSize_; }
    const std::string& getFontFamily() const { return fontFamily_; }
};

// ============================================================================
// ДЕМОНСТРАЦИОННЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Демонстрация группировки команд
 */
void demonstrateCommandGrouping() {
    std::cout << "\n=== Группировка команд ===" << std::endl;
    
    AdvancedCommandManager manager;
    Document doc;
    
    // Группа команд для форматирования текста
    manager.startGroup("Форматирование текста");
    manager.executeCommand(std::make_unique<TextChangeCommand>(&doc.content_, "Привет, мир!", "Изменение текста"));
    manager.executeCommand(std::make_unique<NumberChangeCommand>(&doc.fontSize_, 16, "Изменение размера шрифта"));
    manager.executeCommand(std::make_unique<TextChangeCommand>(&doc.fontFamily_, "Times New Roman", "Изменение шрифта"));
    manager.endGroup();
    
    // Группа команд для редактирования
    manager.startGroup("Редактирование");
    manager.executeCommand(std::make_unique<TextChangeCommand>(&doc.content_, "Привет, мир! Как дела?", "Добавление текста"));
    manager.executeCommand(std::make_unique<NumberChangeCommand>(&doc.fontSize_, 18, "Увеличение размера"));
    manager.endGroup();
    
    std::cout << "\nТекущее состояние документа:" << std::endl;
    std::cout << doc.getStateDescription() << std::endl;
    
    // Отменяем последнюю группу
    manager.undo();
    std::cout << "\nПосле отмены последней группы:" << std::endl;
    std::cout << doc.getStateDescription() << std::endl;
    
    // Отменяем еще одну группу
    manager.undo();
    std::cout << "\nПосле отмены группы форматирования:" << std::endl;
    std::cout << doc.getStateDescription() << std::endl;
}

/**
 * @brief Демонстрация сохранения состояния
 */
void demonstrateStateSaving() {
    std::cout << "\n=== Сохранение и восстановление состояния ===" << std::endl;
    
    AdvancedCommandManager manager;
    Document doc;
    
    // Настраиваем документ
    doc.setContent("Исходный текст");
    doc.setFontSize(14);
    doc.setFontFamily("Calibri");
    
    // Сохраняем состояние
    manager.executeCommand(std::make_unique<SaveStateCommand>(&doc, "Исходное состояние"));
    
    // Вносим изменения
    manager.startGroup("Изменения");
    manager.executeCommand(std::make_unique<TextChangeCommand>(&doc.content_, "Измененный текст", "Изменение текста"));
    manager.executeCommand(std::make_unique<NumberChangeCommand>(&doc.fontSize_, 20, "Изменение размера"));
    manager.endGroup();
    
    std::cout << "\nПосле изменений:" << std::endl;
    std::cout << doc.getStateDescription() << std::endl;
    
    // Отменяем изменения (восстанавливаем состояние)
    manager.undo();
    std::cout << "\nПосле отмены (восстановление состояния):" << std::endl;
    std::cout << doc.getStateDescription() << std::endl;
}

/**
 * @brief Демонстрация истории команд
 */
void demonstrateCommandHistory() {
    std::cout << "\n=== История команд ===" << std::endl;
    
    AdvancedCommandManager manager(5); // Ограничиваем историю 5 группами
    
    Document doc;
    
    // Выполняем несколько групп команд
    for (int i = 1; i <= 7; ++i) {
        manager.startGroup("Группа " + std::to_string(i));
        manager.executeCommand(std::make_unique<TextChangeCommand>(&doc.content_, "Текст " + std::to_string(i), "Изменение " + std::to_string(i)));
        manager.executeCommand(std::make_unique<NumberChangeCommand>(&doc.fontSize_, 10 + i, "Размер " + std::to_string(i)));
        manager.endGroup();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Небольшая задержка
    }
    
    // Показываем историю
    manager.showHistory();
    
    // Отменяем несколько команд
    std::cout << "\n--- Отмена команд ---" << std::endl;
    manager.undo();
    manager.undo();
    manager.undo();
    
    // Показываем историю после отмены
    manager.showHistory();
    
    // Повторяем команды
    std::cout << "\n--- Повтор команд ---" << std::endl;
    manager.redo();
    manager.redo();
    
    // Показываем финальную историю
    manager.showHistory();
}

/**
 * @brief Демонстрация отмены конкретной группы
 */
void demonstrateSelectiveUndo() {
    std::cout << "\n=== Выборочная отмена групп ===" << std::endl;
    
    AdvancedCommandManager manager;
    Document doc;
    
    // Создаем несколько групп
    manager.startGroup("Настройка документа");
    manager.executeCommand(std::make_unique<TextChangeCommand>(&doc.content_, "Заголовок", "Установка заголовка"));
    manager.executeCommand(std::make_unique<NumberChangeCommand>(&doc.fontSize_, 24, "Размер заголовка"));
    manager.endGroup();
    
    manager.startGroup("Основной текст");
    manager.executeCommand(std::make_unique<TextChangeCommand>(&doc.content_, "Заголовок\n\nОсновной текст документа", "Добавление текста"));
    manager.executeCommand(std::make_unique<NumberChangeCommand>(&doc.fontSize_, 14, "Размер основного текста"));
    manager.endGroup();
    
    manager.startGroup("Форматирование");
    manager.executeCommand(std::make_unique<TextChangeCommand>(&doc.fontFamily_, "Georgia", "Изменение шрифта"));
    manager.executeCommand(std::make_unique<NumberChangeCommand>(&doc.fontSize_, 16, "Финальный размер"));
    manager.endGroup();
    
    std::cout << "\nИсходное состояние:" << std::endl;
    std::cout << doc.getStateDescription() << std::endl;
    
    // Отменяем только группу "Основной текст"
    manager.undoGroup("Основной текст");
    
    std::cout << "\nПосле отмены группы 'Основной текст':" << std::endl;
    std::cout << doc.getStateDescription() << std::endl;
    
    // Показываем историю
    manager.showHistory();
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🔄 Продвинутая система отмены и повтора" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    demonstrateCommandGrouping();
    demonstrateStateSaving();
    demonstrateCommandHistory();
    demonstrateSelectiveUndo();
    
    std::cout << "\n✅ Демонстрация продвинутой системы отмены завершена!" << std::endl;
    std::cout << "\n🎯 Ключевые выводы:" << std::endl;
    std::cout << "• Группировка команд позволяет отменять связанные операции" << std::endl;
    std::cout << "• Сохранение состояния обеспечивает быструю отмену" << std::endl;
    std::cout << "• История команд помогает отслеживать изменения" << std::endl;
    std::cout << "• Выборочная отмена дает гибкость в управлении" << std::endl;
    std::cout << "• Ограничения памяти предотвращают утечки" << std::endl;
    
    return 0;
}
