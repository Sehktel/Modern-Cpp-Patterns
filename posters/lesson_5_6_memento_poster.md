# 📊 Плакат: Memento Pattern (Хранитель)

## 🎯 Назначение паттерна Memento

```
┌─────────────────────────────────────────────────────────────────┐
│                        MEMENTO PATTERN                         │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│  💾 СОХРАНЯЕТ И ВОССТАНАВЛИВАЕТ ВНУТРЕННЕЕ СОСТОЯНИЕ ОБЪЕКТА  │
│  🔄 ПОЗВОЛЯЕТ ОТКАТЫВАТЬ ИЗМЕНЕНИЯ БЕЗ НАРУШЕНИЯ ИНКАПСУЛЯЦИИ   │
│  🎯 ОБЕСПЕЧИВАЕТ UNDO/REDO ФУНКЦИОНАЛЬНОСТЬ                    │
└─────────────────────────────────────────────────────────────────┘

Применение:
┌─────────────────┬───────────────────────────────────────────────┐
│   Text Editor   │ • Сохранение и восстановление текста          │
├─────────────────┼───────────────────────────────────────────────┤
│   Game Engine   │ • Сохранение состояния игры                  │
├─────────────────┼───────────────────────────────────────────────┤
│   Graphics      │ • Отмена/повтор операций рисования           │
│   Editor        │                                               │
├─────────────────┼───────────────────────────────────────────────┤
│   Database      │ • Транзакции и откат изменений               │
├─────────────────┼───────────────────────────────────────────────┤
│   Configuration │ • Сохранение и восстановление настроек       │
└─────────────────┴───────────────────────────────────────────────┘
```

## 🔄 Проблема без Memento

```
┌─────────────────────────────────────────────────────────────────┐
│                    ПРОБЛЕМА: НАРУШЕНИЕ ИНКАПСУЛЯЦИИ            │
└─────────────────────────────────────────────────────────────────┘

❌ ПЛОХО - Прямой доступ к внутреннему состоянию:
┌─────────────────────────────────────────────────────────────────┐
│ class TextEditor {                                              │
│ private:                                                        │
│     std::string content_;                                       │
│     int cursorPosition_;                                        │
│     std::string font_;                                          │
│     int fontSize_;                                              │
│                                                                 │
│ public:                                                         │
│     void setContent(const std::string& content) {               │
│         content_ = content;                                    │
│     }                                                           │
│                                                                 │
│     void setCursorPosition(int position) {                     │
│         cursorPosition_ = position;                             │
│     }                                                           │
│                                                                 │
│     void setFont(const std::string& font) {                     │
│         font_ = font;                                           │
│     }                                                           │
│                                                                 │
│     void setFontSize(int size) {                                │
│         fontSize_ = size;                                       │
│     }                                                           │
│                                                                 │
│     // Проблема: нарушение инкапсуляции                        │
│     std::string getContent() const { return content_; }         │
│     int getCursorPosition() const { return cursorPosition_; }   │
│     std::string getFont() const { return font_; }               │
│     int getFontSize() const { return fontSize_; }               │
│                                                                 │
│     // Проблемы:                                               │
│     // • Нарушение инкапсуляции                                │
│     // • Сложность сохранения состояния                        │
│     // • Зависимость от внутренней структуры                   │
│     // • Сложность реализации undo/redo                        │
│ };                                                              │
└─────────────────────────────────────────────────────────────────┘
```

## ✅ Решение с Memento

```
┌─────────────────────────────────────────────────────────────────┐
│                    РЕШЕНИЕ: ИНКАПСУЛЯЦИЯ СОСТОЯНИЯ             │
└─────────────────────────────────────────────────────────────────┘

✅ ХОРОШО - Инкапсуляция состояния:
┌─────────────────────────────────────────────────────────────────┐
│ class Memento {                                                 │
│ private:                                                        │
│     std::string content_;                                       │
│     int cursorPosition_;                                        │
│     std::string font_;                                          │
│     int fontSize_;                                              │
│     std::chrono::system_clock::time_point timestamp_;          │
│                                                                 │
│     // Только TextEditor может создавать Memento               │
│     friend class TextEditor;                                    │
│                                                                 │
│     Memento(const std::string& content, int cursorPosition,     │
│             const std::string& font, int fontSize)               │
│         : content_(content), cursorPosition_(cursorPosition),   │
│           font_(font), fontSize_(fontSize),                      │
│           timestamp_(std::chrono::system_clock::now()) {}       │
│                                                                 │
│ public:                                                         │
│     std::chrono::system_clock::time_point getTimestamp() const { │
│         return timestamp_;                                      │
│     }                                                           │
│                                                                 │
│     std::string getDescription() const {                       │
│         return "Content: " + content_.substr(0, 20) + "...";    │
│     }                                                           │
│ };                                                              │
│                                                                 │
│ class TextEditor {                                              │
│ private:                                                        │
│     std::string content_;                                       │
│     int cursorPosition_;                                        │
│     std::string font_;                                          │
│     int fontSize_;                                              │
│                                                                 │
│ public:                                                         │
│     TextEditor() : cursorPosition_(0), fontSize_(12) {}         │
│                                                                 │
│     void setContent(const std::string& content) {               │
│         content_ = content;                                     │
│     }                                                           │
│                                                                 │
│     void setCursorPosition(int position) {                     │
│         cursorPosition_ = position;                             │
│     }                                                           │
│                                                                 │
│     void setFont(const std::string& font) {                     │
│         font_ = font;                                           │
│     }                                                           │
│                                                                 │
│     void setFontSize(int size) {                                │
│         fontSize_ = size;                                      │
│     }                                                           │
│                                                                 │
│     // Создание снимка состояния                               │
│     std::unique_ptr<Memento> createMemento() const {            │
│         return std::make_unique<Memento>(content_, cursorPosition_, │
│                                          font_, fontSize_);      │
│     }                                                           │
│                                                                 │
│     // Восстановление состояния                                 │
│     void restoreFromMemento(const Memento& memento) {          │
│         content_ = memento.content_;                            │
│         cursorPosition_ = memento.cursorPosition_;              │
│         font_ = memento.font_;                                  │
│         fontSize_ = memento.fontSize_;                          │
│     }                                                           │
│                                                                 │
│     void display() const {                                      │
│         std::cout << "Content: " << content_ << std::endl;      │
│         std::cout << "Cursor: " << cursorPosition_ << std::endl; │
│         std::cout << "Font: " << font_ << " " << fontSize_      │
│                   << "px" << std::endl;                        │
│     }                                                           │
│ };                                                              │
│                                                                 │
│ class Caretaker {                                               │
│ private:                                                        │
│     std::vector<std::unique_ptr<Memento>> mementos_;           │
│     size_t currentIndex_;                                       │
│                                                                 │
│ public:                                                         │
│     Caretaker() : currentIndex_(0) {}                           │
│                                                                 │
│     void saveMemento(std::unique_ptr<Memento> memento) {        │
│         // Удаляем все снимки после текущего индекса           │
│         mementos_.erase(mementos_.begin() + currentIndex_ + 1,  │
│                         mementos_.end());                      │
│                                                                 │
│         mementos_.push_back(std::move(memento));                │
│         currentIndex_ = mementos_.size() - 1;                   │
│     }                                                           │
│                                                                 │
│     std::unique_ptr<Memento> undo() {                           │
│         if (currentIndex_ > 0) {                               │
│             currentIndex_--;                                    │
│             return std::make_unique<Memento>(*mementos_[currentIndex_]); │
│         }                                                       │
│         return nullptr;                                          │
│     }                                                           │
│                                                                 │
│     std::unique_ptr<Memento> redo() {                           │
│         if (currentIndex_ < mementos_.size() - 1) {            │
│             currentIndex_++;                                    │
│             return std::make_unique<Memento>(*mementos_[currentIndex_]); │
│         }                                                       │
│         return nullptr;                                          │
│     }                                                           │
│                                                                 │
│     bool canUndo() const { return currentIndex_ > 0; }          │
│     bool canRedo() const { return currentIndex_ < mementos_.size() - 1; } │
│                                                                 │
│     void showHistory() const {                                  │
│         std::cout << "History:" << std::endl;                  │
│         for (size_t i = 0; i < mementos_.size(); ++i) {         │
│             std::cout << (i == currentIndex_ ? "> " : "  ")     │
│                       << i << ": " << mementos_[i]->getDescription() │
│                       << std::endl;                            │
│         }                                                       │
│     }                                                           │
│ };                                                              │
└─────────────────────────────────────────────────────────────────┘

Использование:
┌─────────────────────────────────────────────────────────────────┐
│ TextEditor editor;                                              │
│ Caretaker caretaker;                                            │
│                                                                 │
│ // Состояние 1                                                  │
│ editor.setContent("Hello");                                     │
│ editor.setFont("Arial");                                        │
│ caretaker.saveMemento(editor.createMemento());                  │
│                                                                 │
│ // Состояние 2                                                  │
│ editor.setContent("Hello World");                               │
│ editor.setFontSize(16);                                         │
│ caretaker.saveMemento(editor.createMemento());                  │
│                                                                 │
│ // Undo                                                         │
│ auto undoMemento = caretaker.undo();                            │
│ if (undoMemento) {                                              │
│     editor.restoreFromMemento(*undoMemento);                    │
│ }                                                               │
│                                                                 │
│ editor.display(); // Показывает "Hello" с Arial 12px           │
└─────────────────────────────────────────────────────────────────┘

Преимущества:
✅ Инкапсуляция внутреннего состояния
✅ Легкая реализация undo/redo
✅ Сохранение истории изменений
✅ Соблюдение принципов SOLID
```

## 🏗️ Структура Memento

```
┌─────────────────────────────────────────────────────────────────┐
│                    СТРУКТУРА ПАТТЕРНА                          │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                    Originator (Создатель)                      │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │ class TextEditor {                                          ││
│  │ private:                                                    ││
│  │     std::string content_;                                   ││
│  │     int cursorPosition_;                                    ││
│  │     std::string font_;                                      ││
│  │     int fontSize_;                                          ││
│  │                                                             ││
│  │ public:                                                     ││
│  │     void setContent(const std::string& content);           ││
│  │     void setCursorPosition(int position);                  ││
│  │     void setFont(const std::string& font);                 ││
│  │     void setFontSize(int size);                            ││
│  │     std::unique_ptr<Memento> createMemento() const;        ││
│  │     void restoreFromMemento(const Memento& memento);       ││
│  │ };                                                          ││
│  └─────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                    Memento (Хранитель)                         │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │ class Memento {                                             ││
│  │ private:                                                    ││
│  │     std::string content_;                                   ││
│  │     int cursorPosition_;                                    ││
│  │     std::string font_;                                      ││
│  │     int fontSize_;                                          ││
│  │     std::chrono::system_clock::time_point timestamp_;       ││
│  │                                                             ││
│  │     friend class TextEditor;                                ││
│  │     Memento(const std::string& content, int cursorPosition, ││
│  │             const std::string& font, int fontSize);         ││
│  │                                                             ││
│  │ public:                                                     ││
│  │     std::chrono::system_clock::time_point getTimestamp() const; ││
│  │     std::string getDescription() const;                    ││
│  │ };                                                          ││
│  └─────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                    Caretaker (Опекун)                          │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │ class Caretaker {                                           ││
│  │ private:                                                    ││
│  │     std::vector<std::unique_ptr<Memento>> mementos_;       ││
│  │     size_t currentIndex_;                                   ││
│  │                                                             ││
│  │ public:                                                     ││
│  │     Caretaker();                                            ││
│  │     void saveMemento(std::unique_ptr<Memento> memento);     ││
│  │     std::unique_ptr<Memento> undo();                       ││
│  │     std::unique_ptr<Memento> redo();                       ││
│  │     bool canUndo() const;                                  ││
│  │     bool canRedo() const;                                  ││
│  │     void showHistory() const;                               ││
│  │ };                                                          ││
│  └─────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────┘
```

## 🔄 Последовательность взаимодействий

```
┌─────────────────────────────────────────────────────────────────┐
│                ПОСЛЕДОВАТЕЛЬНОСТЬ ВЗАИМОДЕЙСТВИЙ               │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────┬───────────────────────────────────────────────┐
│   1. Клиент     │ editor.setContent("Hello");                   │
│      изменяет    │                                               │
│      состояние  │                                               │
├─────────────────┼───────────────────────────────────────────────┤
│   2. Клиент     │ auto memento = editor.createMemento();        │
│      создает     │                                               │
│      снимок     │                                               │
├─────────────────┼───────────────────────────────────────────────┤
│   3. Клиент     │ caretaker.saveMemento(std::move(memento));    │
│      сохраняет   │                                               │
│      снимок     │                                               │
├─────────────────┼───────────────────────────────────────────────┤
│   4. Клиент     │ editor.setContent("Hello World");             │
│      изменяет    │                                               │
│      состояние  │                                               │
├─────────────────┼───────────────────────────────────────────────┤
│   5. Клиент     │ auto undoMemento = caretaker.undo();          │
│      откатывает  │                                               │
│      изменения  │                                               │
├─────────────────┼───────────────────────────────────────────────┤
│   6. Клиент     │ editor.restoreFromMemento(*undoMemento);      │
│      восстанавливает│                                               │
│      состояние  │                                               │
└─────────────────┴───────────────────────────────────────────────┘

Диаграмма взаимодействий:
┌─────────────────────────────────────────────────────────────────┐
│  Client ──┐                                                    │
│           │                                                    │
│           ▼                                                    │
│  TextEditor ──┐                                                │
│                │                                                │
│                ▼                                                │
│  Memento ──┐                                                    │
│             │                                                    │
│             ▼                                                    │
│  Caretaker ──┐                                                   │
│               │                                                   │
│               ▼                                                   │
│  History Management                                               │
└─────────────────────────────────────────────────────────────────┘
```

## 🎨 Варианты реализации

### 1. Classic Memento (Классический)
```
┌─────────────────────────────────────────────────────────────────┐
│                    CLASSIC MEMENTO                             │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│ class GameState {                                               │
│ private:                                                        │
│     int playerHealth_;                                          │
│     int playerLevel_;                                           │
│     std::string currentLocation_;                              │
│     std::vector<std::string> inventory_;                       │
│                                                                 │
│     friend class GameMemento;                                   │
│                                                                 │
│ public:                                                         │
│     GameState() : playerHealth_(100), playerLevel_(1),          │
│                    currentLocation_("Start") {}                 │
│                                                                 │
│     void takeDamage(int damage) {                               │
│         playerHealth_ -= damage;                                │
│     }                                                           │
│                                                                 │
│     void levelUp() {                                            │
│         playerLevel_++;                                         │
│         playerHealth_ = 100; // Full health on level up         │
│     }                                                           │
│                                                                 │
│     void moveTo(const std::string& location) {                  │
│         currentLocation_ = location;                            │
│     }                                                           │
│                                                                 │
│     void addItem(const std::string& item) {                      │
│         inventory_.push_back(item);                             │
│     }                                                           │
│                                                                 │
│     void display() const {                                      │
│         std::cout << "Health: " << playerHealth_                │
│                   << ", Level: " << playerLevel_               │
│                   << ", Location: " << currentLocation_        │
│                   << std::endl;                                │
│     }                                                           │
│                                                                 │
│     std::unique_ptr<GameMemento> createMemento() const;         │
│     void restoreFromMemento(const GameMemento& memento);        │
│ };                                                              │
│                                                                 │
│ class GameMemento {                                             │
│ private:                                                        │
│     int playerHealth_;                                          │
│     int playerLevel_;                                           │
│     std::string currentLocation_;                              │
│     std::vector<std::string> inventory_;                       │
│     std::chrono::system_clock::time_point timestamp_;           │
│                                                                 │
│     friend class GameState;                                     │
│                                                                 │
│     GameMemento(int health, int level, const std::string& location, │
│                 const std::vector<std::string>& inventory)      │
│         : playerHealth_(health), playerLevel_(level),           │
│           currentLocation_(location), inventory_(inventory),    │
│           timestamp_(std::chrono::system_clock::now()) {}       │
│                                                                 │
│ public:                                                         │
│     std::chrono::system_clock::time_point getTimestamp() const { │
│         return timestamp_;                                      │
│     }                                                           │
│                                                                 │
│     std::string getDescription() const {                        │
│         return "Level " + std::to_string(playerLevel_) +       │
│                " at " + currentLocation_;                       │
│     }                                                           │
│ };                                                              │
└─────────────────────────────────────────────────────────────────┘
```

### 2. Command-based Memento
```
┌─────────────────────────────────────────────────────────────────┐
│                    COMMAND-BASED MEMENTO                       │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│ class Command {                                                 │
│ public:                                                         │
│     virtual ~Command() = default;                              │
│     virtual void execute() = 0;                                │
│     virtual void undo() = 0;                                   │
│     virtual std::string getDescription() const = 0;            │
│ };                                                              │
│                                                                 │
│ class TextCommand : public Command {                            │
│ private:                                                        │
│     std::string& text_;                                         │
│     std::string previousText_;                                 │
│     std::string newText_;                                      │
│                                                                 │
│ public:                                                         │
│     TextCommand(std::string& text, const std::string& newText)  │
│         : text_(text), previousText_(text), newText_(newText) {} │
│                                                                 │
│     void execute() override {                                   │
│         text_ = newText_;                                       │
│     }                                                           │
│                                                                 │
│     void undo() override {                                      │
│         text_ = previousText_;                                  │
│     }                                                           │
│                                                                 │
│     std::string getDescription() const override {               │
│         return "Text: " + newText_.substr(0, 20) + "...";      │
│     }                                                           │
│ };                                                              │
│                                                                 │
│ class CommandHistory {                                          │
│ private:                                                        │
│     std::vector<std::unique_ptr<Command>> commands_;            │
│     size_t currentIndex_;                                       │
│                                                                 │
│ public:                                                         │
│     CommandHistory() : currentIndex_(0) {}                       │
│                                                                 │
│     void executeCommand(std::unique_ptr<Command> command) {     │
│         command->execute();                                     │
│                                                                 │
│         // Удаляем команды после текущего индекса              │
│         commands_.erase(commands_.begin() + currentIndex_ + 1,  │
│                         commands_.end());                       │
│                                                                 │
│         commands_.push_back(std::move(command));                │
│         currentIndex_ = commands_.size() - 1;                   │
│     }                                                           │
│                                                                 │
│     void undo() {                                               │
│         if (currentIndex_ < commands_.size()) {                │
│             commands_[currentIndex_]->undo();                   │
             currentIndex_--;                                    │
│         }                                                       │
│     }                                                           │
│                                                                 │
│     void redo() {                                               │
│         if (currentIndex_ + 1 < commands_.size()) {            │
│             currentIndex_++;                                    │
│             commands_[currentIndex_]->execute();                │
│         }                                                       │
│     }                                                           │
│                                                                 │
│     bool canUndo() const { return currentIndex_ < commands_.size(); } │
│     bool canRedo() const { return currentIndex_ + 1 < commands_.size(); } │
│                                                                 │
│     void showHistory() const {                                  │
│         std::cout << "Command History:" << std::endl;           │
│         for (size_t i = 0; i < commands_.size(); ++i) {         │
│             std::cout << (i == currentIndex_ ? "> " : "  ")     │
│                       << i << ": " << commands_[i]->getDescription() │
│                       << std::endl;                            │
│         }                                                       │
│     }                                                           │
│ };                                                              │
└─────────────────────────────────────────────────────────────────┘
```

### 3. Serialization-based Memento
```
┌─────────────────────────────────────────────────────────────────┐
│                    SERIALIZATION-BASED MEMENTO                │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│ class SerializableMemento {                                    │
│ private:                                                        │
│     std::string serializedData_;                                │
│     std::chrono::system_clock::time_point timestamp_;          │
│                                                                 │
│ public:                                                         │
│     SerializableMemento(const std::string& data)                │
│         : serializedData_(data),                                │
│           timestamp_(std::chrono::system_clock::now()) {}       │
│                                                                 │
│     const std::string& getSerializedData() const {              │
│         return serializedData_;                                 │
│     }                                                           │
│                                                                 │
│     std::chrono::system_clock::time_point getTimestamp() const { │
│         return timestamp_;                                      │
│     }                                                           │
│                                                                 │
│     std::string getDescription() const {                        │
│         return "Serialized data: " + serializedData_.substr(0, 20) + "..."; │
│     }                                                           │
│ };                                                              │
│                                                                 │
│ class SerializableObject {                                      │
│ private:                                                        │
│     std::string data_;                                          │
│     int value_;                                                 │
│                                                                 │
│ public:                                                         │
│     SerializableObject() : data_(""), value_(0) {}             │
│                                                                 │
│     void setData(const std::string& data) { data_ = data; }     │
│     void setValue(int value) { value_ = value; }                │
│                                                                 │
│     const std::string& getData() const { return data_; }        │
│     int getValue() const { return value_; }                     │
│                                                                 │
│     std::unique_ptr<SerializableMemento> createMemento() const { │
│         std::ostringstream oss;                                │
│         oss << data_ << "|" << value_;                          │
│         return std::make_unique<SerializableMemento>(oss.str()); │
│     }                                                           │
│                                                                 │
│     void restoreFromMemento(const SerializableMemento& memento) { │
│         std::istringstream iss(memento.getSerializedData());    │
│         std::string data;                                       │
│         std::getline(iss, data, '|');                           │
│         iss >> value_;                                          │
│         data_ = data;                                           │
│     }                                                           │
│                                                                 │
│     void display() const {                                      │
│         std::cout << "Data: " << data_ << ", Value: " << value_ │
│                   << std::endl;                                │
│     }                                                           │
│ };                                                              │
└─────────────────────────────────────────────────────────────────┘
```

## 🔄 Сравнение подходов

```
┌─────────────────────────────────────────────────────────────────┐
│                    СРАВНЕНИЕ ПОДХОДОВ                          │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────┬─────────────────┬─────────────────┬─────────────────┐
│   Classic       │   Command-based  │   Serialization  │   Hybrid        │
│   Memento       │   Memento        │   Memento        │   Memento       │
├─────────────────┼─────────────────┼─────────────────┼─────────────────┤
│ Простота        │ Средняя          │ Высокая         │ Низкая          │
│ Гибкость        │ Высокая          │ Очень высокая   │ Средняя         │
│ Производительность│ Высокая        │ Средняя         │ Низкая          │
│ Тестируемость   │ Высокая          │ Высокая         │ Средняя         │
│ Память          │ Средняя          │ Высокая         │ Низкая          │
│ Применимость    │ Стандартные      │ Команды         │ Сериализация    │
│                 │ случаи          │                 │                 │
└─────────────────┴─────────────────┴─────────────────┴─────────────────┘
```

## 🎯 Когда использовать Memento

```
┌─────────────────────────────────────────────────────────────────┐
│                КОГДА ИСПОЛЬЗОВАТЬ MEMENTO                     │
└─────────────────────────────────────────────────────────────────┘

✅ ХОРОШИЕ СЛУЧАИ:
┌─────────────────┬───────────────────────────────────────────────┐
│ Undo/Redo       │ • Нужна функциональность отмены/повтора      │
│                 │ • История изменений                           │
├─────────────────┼───────────────────────────────────────────────┤
│ Сохранение      │ • Нужно сохранять состояние объектов         │
│ состояния       │ • Checkpoint'ы в играх                       │
├─────────────────┼───────────────────────────────────────────────┤
│ Инкапсуляция    │ • Важна инкапсуляция внутреннего состояния   │
│                 │ • Защита от прямого доступа                  │
├─────────────────┼───────────────────────────────────────────────┤
│ Транзакции      │ • Нужны транзакции с возможностью отката     │
│                 │ • Атомарные операции                         │
├─────────────────┼───────────────────────────────────────────────┤
│ Конфигурация    │ • Сохранение и восстановление настроек       │
│                 │ • Откат к предыдущим версиям                 │
└─────────────────┴───────────────────────────────────────────────┘

❌ ПЛОХИЕ СЛУЧАИ:
┌─────────────────┬───────────────────────────────────────────────┐
│ Простые случаи  │ • Простые объекты без сложного состояния     │
├─────────────────┼───────────────────────────────────────────────┤
│ Производительность│ • Критична производительность             │
├─────────────────┼───────────────────────────────────────────────┤
│ Память          │ • Критична экономия памяти                   │
├─────────────────┼───────────────────────────────────────────────┤
│ Альтернативы    │ • Есть лучшие альтернативы (Command)        │
└─────────────────┴───────────────────────────────────────────────┘
```

## 🧪 Тестирование Memento

```
┌─────────────────────────────────────────────────────────────────┐
│                    ТЕСТИРОВАНИЕ MEMENTO                      │
└─────────────────────────────────────────────────────────────────┘

Преимущества для тестирования:
┌─────────────────┬───────────────────────────────────────────────┐
│ Моки            │ ✅ Легко создавать моки для memento'ов       │
│                 │ class MockMemento : public Memento {          │
│                 │     MOCK_METHOD(std::string, getDescription, (), (const)); │
│                 │     MOCK_METHOD(std::chrono::system_clock::time_point, │
│                 │                  getTimestamp, (), (const));   │
│                 │ };                                            │
├─────────────────┼───────────────────────────────────────────────┤
│ Изоляция        │ ✅ Легко изолировать тесты                   │
│                 │ auto mockMemento = std::make_unique<MockMemento>(); │
│                 │ TextEditor editor;                            │
│                 │ editor.restoreFromMemento(*mockMemento);       │
├─────────────────┼───────────────────────────────────────────────┤
│ Управление      │ ✅ Полный контроль над состоянием            │
│ зависимостями   │ EXPECT_CALL(*mockMemento, getDescription())    │
│                 │     .WillOnce(Return("Test description"));     │
└─────────────────┴───────────────────────────────────────────────┘

Пример теста:
┌─────────────────────────────────────────────────────────────────┐
│ TEST(MementoTest, SaveAndRestoreState) {                       │
│     TextEditor editor;                                          │
│     Caretaker caretaker;                                        │
│                                                                 │
│     // Установка начального состояния                          │
│     editor.setContent("Initial");                               │
│     editor.setFont("Arial");                                    │
│                                                                 │
│     // Сохранение состояния                                    │
│     caretaker.saveMemento(editor.createMemento());              │
│                                                                 │
│     // Изменение состояния                                     │
│     editor.setContent("Modified");                              │
│     editor.setFont("Times");                                    │
│                                                                 │
│     // Восстановление состояния                                │
│     auto memento = caretaker.undo();                           │
│     editor.restoreFromMemento(*memento);                       │
│                                                                 │
│     // Проверка восстановления                                 │
│     EXPECT_EQ(editor.getContent(), "Initial");                  │
│     EXPECT_EQ(editor.getFont(), "Arial");                       │
│ }                                                               │
└─────────────────────────────────────────────────────────────────┘
```

## 🔗 Связь с другими паттернами

```
┌─────────────────────────────────────────────────────────────────┐
│              СВЯЗЬ С ДРУГИМИ ПАТТЕРНАМИ                        │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────┬───────────────────────────────────────────────┐
│ Command         │ • Memento может использоваться с Command     │
│                 │ • Command для инкапсуляции действий          │
│                 │ • Memento для сохранения состояния           │
│                 │ • Command - действия                        │
│                 │ • Memento - состояние                        │
├─────────────────┼───────────────────────────────────────────────┤
│ State           │ • Memento может сохранять State              │
│                 │ • State для изменения поведения              │
│                 │ • Memento для сохранения состояния           │
│                 │ • State - поведение                          │
│                 │ • Memento - состояние                        │
├─────────────────┼───────────────────────────────────────────────┤
│ Prototype       │ • Memento может использовать Prototype       │
│                 │ • Prototype для клонирования объектов       │
│                 │ • Memento для сохранения состояния           │
│                 │ • Prototype - клонирование                  │
│                 │ • Memento - состояние                        │
├─────────────────┼───────────────────────────────────────────────┤
│ Iterator        │ • Memento может использоваться с Iterator    │
│                 │ • Iterator для обхода истории                │
│                 │ • Memento для сохранения состояния           │
│                 │ • Iterator - обход                          │
│                 │ • Memento - состояние                        │
├─────────────────┼───────────────────────────────────────────────┤
│ Observer        │ • Memento может уведомлять Observer         │
│                 │ • Observer для уведомления об изменениях     │
│                 │ • Memento для сохранения состояния           │
│                 │ • Observer - уведомления                     │
│                 │ • Memento - состояние                        │
└─────────────────┴───────────────────────────────────────────────┘
```

## 💡 Практические рекомендации

```
┌─────────────────────────────────────────────────────────────────┐
│                  ПРАКТИЧЕСКИЕ РЕКОМЕНДАЦИИ                     │
└─────────────────────────────────────────────────────────────────┘

1. 🎯 Выбор реализации:
   • Classic Memento - для большинства случаев
   • Command-based Memento - для командных систем
   • Serialization-based Memento - для сериализации
   • Hybrid Memento - для сложных систем

2. 🔧 Принципы проектирования:
   • Соблюдайте инкапсуляцию состояния
   • Используйте friend классы осторожно
   • Инкапсулируйте логику сохранения
   • Обеспечьте неизменяемость memento

3. 🧪 Тестирование:
   • Создавайте интерфейсы для memento'ов
   • Используйте моки в тестах
   • Тестируйте сохранение и восстановление
   • Проверяйте корректность истории

4. ⚡ Производительность:
   • Рассмотрите ленивое создание memento'ов
   • Используйте move semantics
   • Избегайте ненужных копирований
   • Профилируйте критические участки

5. 📚 Документация:
   • Документируйте состояние объектов
   • Описывайте критерии сохранения
   • Приводите примеры использования
   • Объясняйте причины выбора паттерна

6. 🔄 Рефакторинг:
   • Начинайте с простых memento'ов
   • Постепенно добавляйте сложность
   • Используйте интерфейсы для абстракции
   • Планируйте расширяемость
```

## 🎓 Ключевые выводы

```
┌─────────────────────────────────────────────────────────────────┐
│                      КЛЮЧЕВЫЕ ВЫВОДЫ                          │
└─────────────────────────────────────────────────────────────────┘

✅ Memento - мощный инструмент:
   • Сохраняет и восстанавливает состояние
   • Обеспечивает инкапсуляцию
   • Поддерживает undo/redo функциональность
   • Следует принципам SOLID

✅ Выбирайте подходящую реализацию:
   • Classic Memento - для большинства случаев
   • Command-based Memento - для командных систем
   • Serialization-based Memento - для сериализации
   • Hybrid Memento - для сложных систем

✅ Используйте правильно:
   • Когда нужна функциональность undo/redo
   • Когда важно сохранять состояние
   • Когда важна инкапсуляция
   • Когда нужны транзакции

✅ Избегайте злоупотребления:
   • Не используйте для простых случаев
   • Не добавляйте сложность без необходимости
   • Не забывайте об обработке ошибок
   • Не нарушайте принципы SOLID

✅ Помните о компромиссах:
   • Инкапсуляция vs производительность
   • Память vs функциональность
   • Простота vs гибкость
   • Тестируемость vs производительность
```

---
*Memento - это хранитель состояния. 
Используйте его для реализации undo/redo функциональности!*
