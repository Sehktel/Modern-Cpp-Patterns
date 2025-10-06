# 📊 Плакат: Mediator Pattern (Посредник)

## 🎯 Назначение паттерна Mediator

```
┌─────────────────────────────────────────────────────────────────┐
│                        MEDIATOR PATTERN                        │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│  🤝 ИНКАПСУЛИРУЕТ ВЗАИМОДЕЙСТВИЕ МЕЖДУ ОБЪЕКТАМИ              │
│  🔄 УПРОЩАЕТ СВЯЗИ МЕЖДУ КОМПОНЕНТАМИ СИСТЕМЫ                 │
│  🎯 ИЗБЕГАЕТ ПРЯМЫХ СВЯЗЕЙ МЕЖДУ ОБЪЕКТАМИ                     │
└─────────────────────────────────────────────────────────────────┘

Применение:
┌─────────────────┬───────────────────────────────────────────────┐
│   Chat System   │ • Координация сообщений между пользователями  │
├─────────────────┼───────────────────────────────────────────────┤
│   UI Framework  │ • Управление взаимодействием между виджетами   │
├─────────────────┼───────────────────────────────────────────────┤
│   Game Engine   │ • Координация действий между игровыми объектами│
├─────────────────┼───────────────────────────────────────────────┤
│   Workflow      │ • Управление бизнес-процессами                │
├─────────────────┼───────────────────────────────────────────────┤
│   Event System  │ • Централизованная обработка событий          │
└─────────────────┴───────────────────────────────────────────────┘
```

## 🔄 Проблема без Mediator

```
┌─────────────────────────────────────────────────────────────────┐
│                    ПРОБЛЕМА: СЛОЖНЫЕ СВЯЗИ                    │
└─────────────────────────────────────────────────────────────────┘

❌ ПЛОХО - Прямые связи между объектами:
┌─────────────────────────────────────────────────────────────────┐
│ class Button {                                                  │
│ private:                                                        │
│     std::string text_;                                          │
│     TextBox* textBox_;                                         │
│     Label* label_;                                              │
│     Dialog* dialog_;                                           │
│                                                                 │
│ public:                                                         │
│     Button(const std::string& text, TextBox* textBox,          │
│            Label* label, Dialog* dialog)                       │
│         : text_(text), textBox_(textBox), label_(label),       │
│           dialog_(dialog) {}                                   │
│                                                                 │
│     void onClick() {                                            │
│         if (textBox_->isEmpty()) {                             │
│             label_->setText("Please enter text");               │
│             label_->setColor("red");                            │
│         } else {                                               │
│             label_->setText("Text entered: " + textBox_->getText()); │
│             label_->setColor("green");                          │
│             dialog_->close();                                  │
│         }                                                       │
│     }                                                           │
│ };                                                              │
│                                                                 │
│ class TextBox {                                                 │
│ private:                                                        │
│     std::string text_;                                          │
│     Button* button_;                                            │
│     Label* label_;                                              │
│                                                                 │
│ public:                                                         │
│     TextBox(Button* button, Label* label)                      │
│         : button_(button), label_(label) {}                    │
│                                                                 │
│     void onTextChanged() {                                      │
│         if (isEmpty()) {                                        │
│             button_->setEnabled(false);                         │
│             label_->setText("Enter some text");                 │
│         } else {                                               │
│             button_->setEnabled(true);                         │
│             label_->setText("");                                │
│         }                                                       │
│     }                                                           │
│                                                                 │
│     bool isEmpty() const { return text_.empty(); }              │
│     const std::string& getText() const { return text_; }       │
│     void setText(const std::string& text) { text_ = text; }     │
│ };                                                              │
│                                                                 │
│ // Проблемы:                                                    │
│ // • Сложные зависимости между объектами                        │
│ // • Нарушение принципа единственной ответственности           │
│ // • Сложность тестирования и сопровождения                   │
│ // • Нарушение принципа открытости/закрытости                  │
└─────────────────────────────────────────────────────────────────┘
```

## ✅ Решение с Mediator

```
┌─────────────────────────────────────────────────────────────────┐
│                    РЕШЕНИЕ: ЦЕНТРАЛИЗОВАННОЕ УПРАВЛЕНИЕ       │
└─────────────────────────────────────────────────────────────────┘

✅ ХОРОШО - Централизованное управление взаимодействием:
┌─────────────────────────────────────────────────────────────────┐
│ class Mediator {                                               │
│ public:                                                         │
│     virtual ~Mediator() = default;                             │
│     virtual void notify(Component* sender, const std::string& event) = 0; │
│ };                                                              │
│                                                                 │
│ class Component {                                               │
│ protected:                                                      │
│     Mediator* mediator_;                                        │
│                                                                 │
│ public:                                                         │
│     Component(Mediator* mediator) : mediator_(mediator) {}     │
│     virtual ~Component() = default;                            │
│                                                                 │
│     void setMediator(Mediator* mediator) { mediator_ = mediator; } │
│                                                                 │
│     virtual void notify(const std::string& event) {            │
│         if (mediator_) {                                       │
│             mediator_->notify(this, event);                     │
│         }                                                       │
│     }                                                           │
│ };                                                              │
│                                                                 │
│ class Button : public Component {                               │
│ private:                                                        │
│     std::string text_;                                          │
│     bool enabled_;                                              │
│                                                                 │
│ public:                                                         │
│     Button(const std::string& text, Mediator* mediator)         │
│         : Component(mediator), text_(text), enabled_(false) {}  │
│                                                                 │
│     void onClick() {                                            │
│         notify("button_clicked");                               │
│     }                                                           │
│                                                                 │
│     void setEnabled(bool enabled) { enabled_ = enabled; }      │
│     bool isEnabled() const { return enabled_; }                │
│     const std::string& getText() const { return text_; }        │
│ };                                                              │
│                                                                 │
│ class TextBox : public Component {                              │
│ private:                                                        │
│     std::string text_;                                          │
│                                                                 │
│ public:                                                         │
│     TextBox(Mediator* mediator) : Component(mediator) {}        │
│                                                                 │
│     void onTextChanged() {                                      │
│         notify("text_changed");                                 │
│     }                                                           │
│                                                                 │
│     bool isEmpty() const { return text_.empty(); }              │
│     const std::string& getText() const { return text_; }        │
│     void setText(const std::string& text) {                     │
│         text_ = text;                                           │
│         onTextChanged();                                        │
│     }                                                           │
│ };                                                              │
│                                                                 │
│ class Label : public Component {                                │
│ private:                                                        │
│     std::string text_;                                          │
│     std::string color_;                                         │
│                                                                 │
│ public:                                                         │
│     Label(Mediator* mediator) : Component(mediator) {}          │
│                                                                 │
│     void setText(const std::string& text) { text_ = text; }     │
│     void setColor(const std::string& color) { color_ = color; } │
│     const std::string& getText() const { return text_; }        │
│     const std::string& getColor() const { return color_; }     │
│ };                                                              │
│                                                                 │
│ class DialogMediator : public Mediator {                       │
│ private:                                                        │
│     Button* button_;                                            │
│     TextBox* textBox_;                                          │
│     Label* label_;                                              │
│                                                                 │
│ public:                                                         │
│     DialogMediator(Button* button, TextBox* textBox, Label* label) │
│         : button_(button), textBox_(textBox), label_(label) {}  │
│                                                                 │
│     void notify(Component* sender, const std::string& event) override { │
│         if (event == "text_changed") {                         │
│             if (textBox_->isEmpty()) {                          │
│                 button_->setEnabled(false);                     │
│                 label_->setText("Enter some text");             │
│                 label_->setColor("gray");                       │
│             } else {                                           │
│                 button_->setEnabled(true);                     │
│                 label_->setText("");                            │
│             }                                                   │
│         } else if (event == "button_clicked") {                │
│             if (textBox_->isEmpty()) {                         │
│                 label_->setText("Please enter text");           │
│                 label_->setColor("red");                        │
│             } else {                                           │
│                 label_->setText("Text entered: " + textBox_->getText()); │
│                 label_->setColor("green");                      │
│                 // Dialog would close here                     │
│             }                                                   │
│         }                                                       │
│     }                                                           │
│ };                                                              │
└─────────────────────────────────────────────────────────────────┘

Использование:
┌─────────────────────────────────────────────────────────────────┐
│ // Создание компонентов                                        │
│ auto button = std::make_unique<Button>("Submit", nullptr);      │
│ auto textBox = std::make_unique<TextBox>(nullptr);             │
│ auto label = std::make_unique<Label>(nullptr);                 │
│                                                                 │
│ // Создание медиатора                                          │
│ auto mediator = std::make_unique<DialogMediator>(               │
│     button.get(), textBox.get(), label.get());                 │
│                                                                 │
│ // Установка медиатора для компонентов                         │
│ button->setMediator(mediator.get());                           │
│ textBox->setMediator(mediator.get());                          │
│ label->setMediator(mediator.get());                            │
│                                                                 │
│ // Взаимодействие                                              │
│ textBox->setText("Hello World");                               │
│ button->onClick();                                             │
└─────────────────────────────────────────────────────────────────┘

Преимущества:
✅ Упрощение связей между объектами
✅ Централизованное управление взаимодействием
✅ Легкость добавления новых компонентов
✅ Соблюдение принципов SOLID
```

## 🏗️ Структура Mediator

```
┌─────────────────────────────────────────────────────────────────┐
│                    СТРУКТУРА ПАТТЕРНА                          │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                    Mediator (Посредник)                        │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │ class Mediator {                                            ││
│  │ public:                                                     ││
│  │     virtual ~Mediator() = default;                         ││
│  │     virtual void notify(Component* sender,                  ││
│  │                        const std::string& event) = 0;       ││
│  │ };                                                          ││
│  └─────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                    Component (Компонент)                       │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │ class Component {                                            ││
│  │ protected:                                                   ││
│  │     Mediator* mediator_;                                    ││
│  │                                                             ││
│  │ public:                                                     ││
│  │     Component(Mediator* mediator);                          ││
│  │     virtual ~Component() = default;                        ││
│  │     void setMediator(Mediator* mediator);                   ││
│  │     virtual void notify(const std::string& event);          ││
│  │ };                                                          ││
│  └─────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                    ConcreteComponent (Конкретный компонент)    │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │ class Button : public Component {                           ││
│  │ private:                                                    ││
│  │     std::string text_;                                      ││
│  │     bool enabled_;                                          ││
│  │                                                             ││
│  │ public:                                                     ││
│  │     Button(const std::string& text, Mediator* mediator);   ││
│  │     void onClick();                                         ││
│  │     void setEnabled(bool enabled);                          ││
│  │     bool isEnabled() const;                                 ││
│  │ };                                                          ││
│  │                                                             ││
│  │ class TextBox : public Component {                          ││
│  │ private:                                                    ││
│  │     std::string text_;                                      ││
│  │                                                             ││
│  │ public:                                                     ││
│  │     TextBox(Mediator* mediator);                            ││
│  │     void onTextChanged();                                   ││
│  │     bool isEmpty() const;                                   ││
│  │     const std::string& getText() const;                     ││
│  │     void setText(const std::string& text);                  ││
│  │ };                                                          ││
│  └─────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                    ConcreteMediator (Конкретный посредник)     │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │ class DialogMediator : public Mediator {                   ││
│  │ private:                                                    ││
│  │     Button* button_;                                        ││
│  │     TextBox* textBox_;                                      ││
│  │     Label* label_;                                          ││
│  │                                                             ││
│  │ public:                                                     ││
│  │     DialogMediator(Button* button, TextBox* textBox,        ││
│  │                    Label* label);                           ││
│  │     void notify(Component* sender,                          ││
│  │                const std::string& event) override;          ││
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
│   1. Клиент     │ textBox->setText("Hello");                    │
│      изменяет    │                                               │
│      состояние  │                                               │
├─────────────────┼───────────────────────────────────────────────┤
│   2. Компонент  │ textBox->onTextChanged();                     │
│      уведомляет │                                               │
│      медиатора  │                                               │
├─────────────────┼───────────────────────────────────────────────┤
│   3. Медиатор   │ mediator->notify(textBox, "text_changed");    │
│      получает    │                                               │
│      уведомление│                                               │
├─────────────────┼───────────────────────────────────────────────┤
│   4. Медиатор   │ button->setEnabled(true);                     │
│      координирует│ label->setText("");                          │
│      действия   │                                               │
├─────────────────┼───────────────────────────────────────────────┤
│   5. Клиент     │ button->onClick();                           │
│      взаимодействует│                                               │
│      с системой │                                               │
└─────────────────┴───────────────────────────────────────────────┘

Диаграмма взаимодействий:
┌─────────────────────────────────────────────────────────────────┐
│  Client ──┐                                                    │
│           │                                                    │
│           ▼                                                    │
│  TextBox ──┐                                                   │
│             │                                                   │
│             ▼                                                   │
│  DialogMediator ──┐                                             │
│                    │                                             │
│                    ▼                                             │
│  Button ──┐                                                      │
│  Label ───┘                                                      │
└─────────────────────────────────────────────────────────────────┘
```

## 🎨 Варианты реализации

### 1. Classic Mediator (Классический)
```
┌─────────────────────────────────────────────────────────────────┐
│                    CLASSIC MEDIATOR                           │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│ class ChatMediator {                                           │
│ private:                                                        │
│     std::vector<User*> users_;                                  │
│                                                                 │
│ public:                                                         │
│     void addUser(User* user) {                                 │
│         users_.push_back(user);                                 │
│     }                                                           │
│                                                                 │
│     void sendMessage(const std::string& message, User* sender) { │
│         for (User* user : users_) {                             │
│             if (user != sender) {                               │
│                 user->receiveMessage(message, sender->getName()); │
│             }                                                   │
│         }                                                       │
│     }                                                           │
│                                                                 │
│     void sendPrivateMessage(const std::string& message,         │
│                             User* sender, User* receiver) {     │
│         receiver->receiveMessage(message, sender->getName());   │
│     }                                                           │
│ };                                                              │
│                                                                 │
│ class User {                                                    │
│ private:                                                        │
│     std::string name_;                                          │
│     ChatMediator* mediator_;                                    │
│                                                                 │
│ public:                                                         │
│     User(const std::string& name, ChatMediator* mediator)       │
│         : name_(name), mediator_(mediator) {}                  │
│                                                                 │
│     void sendMessage(const std::string& message) {             │
│         mediator_->sendMessage(message, this);                  │
│     }                                                           │
│                                                                 │
│     void sendPrivateMessage(const std::string& message,         │
│                             User* receiver) {                   │
│         mediator_->sendPrivateMessage(message, this, receiver); │
│     }                                                           │
│                                                                 │
│     void receiveMessage(const std::string& message,             │
│                         const std::string& senderName) {        │
│         std::cout << name_ << " received from " << senderName  │
│                   << ": " << message << std::endl;             │
│     }                                                           │
│                                                                 │
│     const std::string& getName() const { return name_; }        │
│ };                                                              │
└─────────────────────────────────────────────────────────────────┘
```

### 2. Event-based Mediator
```
┌─────────────────────────────────────────────────────────────────┐
│                    EVENT-BASED MEDIATOR                       │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│ class EventMediator {                                          │
│ private:                                                        │
│     std::unordered_map<std::string,                            │
│                        std::vector<std::function<void(const std::string&)>>> │
│                        eventHandlers_;                          │
│                                                                 │
│ public:                                                         │
│     void subscribe(const std::string& event,                    │
│                    std::function<void(const std::string&)> handler) { │
│         eventHandlers_[event].push_back(handler);               │
│     }                                                           │
│                                                                 │
│     void publish(const std::string& event,                      │
│                  const std::string& data) {                    │
│         auto it = eventHandlers_.find(event);                  │
│         if (it != eventHandlers_.end()) {                      │
│             for (const auto& handler : it->second) {            │
│                 handler(data);                                  │
│             }                                                   │
│         }                                                       │
│     }                                                           │
│                                                                 │
│     void unsubscribe(const std::string& event,                  │
│                      std::function<void(const std::string&)> handler) { │
│         auto it = eventHandlers_.find(event);                  │
│         if (it != eventHandlers_.end()) {                      │
│             auto& handlers = it->second;                        │
│             handlers.erase(                                    │
│                 std::remove(handlers.begin(), handlers.end(), handler), │
│                 handlers.end());                                │
│         }                                                       │
│     }                                                           │
│ };                                                              │
│                                                                 │
│ class EventComponent {                                          │
│ protected:                                                      │
│     EventMediator* mediator_;                                   │
│     std::string componentId_;                                   │
│                                                                 │
│ public:                                                         │
│     EventComponent(const std::string& id, EventMediator* mediator) │
│         : componentId_(id), mediator_(mediator) {}             │
│                                                                 │
│     void publishEvent(const std::string& event,                  │
│                      const std::string& data) {                 │
│         mediator_->publish(event, data);                        │
│     }                                                           │
│                                                                 │
│     void subscribeToEvent(const std::string& event,             │
│                           std::function<void(const std::string&)> handler) { │
│         mediator_->subscribe(event, handler);                   │
│     }                                                           │
│                                                                 │
│     const std::string& getComponentId() const {                 │
│         return componentId_;                                    │
│     }                                                           │
│ };                                                              │
└─────────────────────────────────────────────────────────────────┘
```

### 3. Template Mediator
```
┌─────────────────────────────────────────────────────────────────┐
│                    TEMPLATE MEDIATOR                           │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│ template<typename EventType>                                   │
│ class TemplateMediator {                                       │
│ private:                                                        │
│     std::unordered_map<EventType,                              │
│                        std::vector<std::function<void(const EventType&)>>> │
│                        eventHandlers_;                          │
│                                                                 │
│ public:                                                         │
│     void subscribe(EventType event,                             │
│                    std::function<void(const EventType&)> handler) { │
│         eventHandlers_[event].push_back(handler);               │
│     }                                                           │
│                                                                 │
│     void publish(EventType event) {                            │
│         auto it = eventHandlers_.find(event);                  │
│         if (it != eventHandlers_.end()) {                      │
│             for (const auto& handler : it->second) {           │
│                 handler(event);                                 │
│             }                                                   │
│         }                                                       │
│     }                                                           │
│                                                                 │
│     void unsubscribe(EventType event,                          │
│                      std::function<void(const EventType&)> handler) { │
│         auto it = eventHandlers_.find(event);                  │
│         if (it != eventHandlers_.end()) {                      │
│             auto& handlers = it->second;                        │
│             handlers.erase(                                    │
│                 std::remove(handlers.begin(), handlers.end(), handler), │
│                 handlers.end());                                │
│         }                                                       │
│     }                                                           │
│ };                                                              │
│                                                                 │
│ // Использование с enum                                        │
│ enum class GameEvent {                                          │
│     PLAYER_MOVED,                                              │
│     PLAYER_ATTACKED,                                           │
│     PLAYER_DIED,                                               │
│     ITEM_PICKED_UP                                             │
│ };                                                              │
│                                                                 │
│ TemplateMediator<GameEvent> gameMediator;                      │
│                                                                 │
│ gameMediator.subscribe(GameEvent::PLAYER_MOVED,                │
│     [](const GameEvent& event) {                               │
│         std::cout << "Player moved!" << std::endl;             │
│     });                                                         │
│                                                                 │
│ gameMediator.publish(GameEvent::PLAYER_MOVED);                 │
└─────────────────────────────────────────────────────────────────┘
```

## 🔄 Сравнение подходов

```
┌─────────────────────────────────────────────────────────────────┐
│                    СРАВНЕНИЕ ПОДХОДОВ                          │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────┬─────────────────┬─────────────────┬─────────────────┐
│   Classic       │   Event-based   │   Template      │   Hybrid        │
│   Mediator      │   Mediator      │   Mediator      │   Mediator      │
├─────────────────┼─────────────────┼─────────────────┼─────────────────┤
│ Простота        │ Средняя          │ Высокая         │ Низкая          │
│ Гибкость        │ Высокая          │ Очень высокая   │ Высокая         │
│ Производительность│ Высокая        │ Средняя         │ Высокая         │
│ Тестируемость   │ Высокая          │ Высокая         │ Средняя         │
│ Типобезопасность│ Средняя          │ Низкая          │ Высокая         │
│ Применимость    │ Стандартные      │ Событийные      │ Типизированные  │
│                 │ случаи          │ системы         │ случаи          │
└─────────────────┴─────────────────┴─────────────────┴─────────────────┘
```

## 🎯 Когда использовать Mediator

```
┌─────────────────────────────────────────────────────────────────┐
│                КОГДА ИСПОЛЬЗОВАТЬ MEDIATOR                    │
└─────────────────────────────────────────────────────────────────┘

✅ ХОРОШИЕ СЛУЧАИ:
┌─────────────────┬───────────────────────────────────────────────┐
│ Сложные связи   │ • Много объектов с взаимными зависимостями   │
│                 │ • Сложная логика взаимодействия               │
├─────────────────┼───────────────────────────────────────────────┤
│ Централизация   │ • Нужно централизовать управление            │
│                 │ • Единая точка координации                    │
├─────────────────┼───────────────────────────────────────────────┤
│ Упрощение       │ • Нужно упростить связи между объектами      │
│                 │ • Избежать прямых зависимостей               │
├─────────────────┼───────────────────────────────────────────────┤
│ Расширяемость   │ • Система должна быть расширяемой            │
│                 │ • Легкое добавление новых компонентов        │
├─────────────────┼───────────────────────────────────────────────┤
│ Тестирование    │ • Нужно легко тестировать компоненты         │
│                 │ • Изоляция компонентов                        │
└─────────────────┴───────────────────────────────────────────────┘

❌ ПЛОХИЕ СЛУЧАИ:
┌─────────────────┬───────────────────────────────────────────────┐
│ Простые случаи  │ • Мало объектов или простые связи           │
├─────────────────┼───────────────────────────────────────────────┤
│ Производительность│ • Критична производительность             │
├─────────────────┼───────────────────────────────────────────────┤
│ Сложность       │ • Добавляется ненужная сложность             │
├─────────────────┼───────────────────────────────────────────────┤
│ Альтернативы    │ • Есть лучшие альтернативы (Observer)       │
└─────────────────┴───────────────────────────────────────────────┘
```

## 🧪 Тестирование Mediator

```
┌─────────────────────────────────────────────────────────────────┐
│                    ТЕСТИРОВАНИЕ MEDIATOR                      │
└─────────────────────────────────────────────────────────────────┘

Преимущества для тестирования:
┌─────────────────┬───────────────────────────────────────────────┐
│ Моки            │ ✅ Легко создавать моки для медиаторов       │
│                 │ class MockMediator : public Mediator {        │
│                 │     MOCK_METHOD(void, notify, (Component*, const std::string&)); │
│                 │ };                                            │
├─────────────────┼───────────────────────────────────────────────┤
│ Изоляция        │ ✅ Легко изолировать тесты                   │
│                 │ auto mockMediator = std::make_unique<MockMediator>(); │
│                 │ Button button("Test", mockMediator.get());    │
├─────────────────┼───────────────────────────────────────────────┤
│ Управление      │ ✅ Полный контроль над взаимодействием      │
│ зависимостями   │ EXPECT_CALL(*mockMediator, notify(_, "button_clicked")) │
│                 │     .Times(1);                               │
└─────────────────┴───────────────────────────────────────────────┘

Пример теста:
┌─────────────────────────────────────────────────────────────────┐
│ TEST(MediatorTest, ButtonClickNotifiesMediator) {               │
│     auto mockMediator = std::make_unique<MockMediator>();        │
│     Button button("Test", mockMediator.get());                  │
│                                                                 │
│     EXPECT_CALL(*mockMediator, notify(&button, "button_clicked")) │
│         .Times(1);                                              │
│                                                                 │
│     button.onClick();                                          │
│ }                                                               │
└─────────────────────────────────────────────────────────────────┘
```

## 🔗 Связь с другими паттернами

```
┌─────────────────────────────────────────────────────────────────┐
│              СВЯЗЬ С ДРУГИМИ ПАТТЕРНАМИ                        │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────┬───────────────────────────────────────────────┐
│ Observer        │ • Mediator может использовать Observer      │
│                 │ • Observer для уведомления компонентов       │
│                 │ • Mediator - координация                    │
│                 │ • Observer - уведомления                    │
├─────────────────┼───────────────────────────────────────────────┤
│ Command         │ • Mediator может использовать Command       │
│                 │ • Command для инкапсуляции действий         │
│                 │ • Mediator - координация                    │
│                 │ • Command - действия                        │
├─────────────────┼───────────────────────────────────────────────┤
│ Facade          │ • Mediator может быть частью Facade        │
│                 │ • Facade упрощает работу с Mediator         │
│                 │ • Mediator - координация                    │
│                 │ • Facade - упрощение интерфейса             │
├─────────────────┼───────────────────────────────────────────────┤
│ Chain of        │ • Mediator может использовать Chain of      │
│ Responsibility  │   Responsibility                             │
│                 │ • Chain of Responsibility для обработки событий│
│                 │ • Mediator - координация                    │
│                 │ • Chain of Responsibility - обработка      │
├─────────────────┼───────────────────────────────────────────────┤
│ Strategy        │ • Mediator может использовать Strategy      │
│                 │ • Strategy для выбора алгоритма координации │
│                 │ • Mediator - координация                    │
│                 │ • Strategy - выбор алгоритма                │
└─────────────────┴───────────────────────────────────────────────┘
```

## 💡 Практические рекомендации

```
┌─────────────────────────────────────────────────────────────────┐
│                  ПРАКТИЧЕСКИЕ РЕКОМЕНДАЦИИ                     │
└─────────────────────────────────────────────────────────────────┘

1. 🎯 Выбор реализации:
   • Classic Mediator - для большинства случаев
   • Event-based Mediator - для событийных систем
   • Template Mediator - для типизированных случаев
   • Hybrid Mediator - для сложных систем

2. 🔧 Принципы проектирования:
   • Соблюдайте Single Responsibility Principle
   • Используйте централизованное управление
   • Инкапсулируйте логику взаимодействия
   • Обеспечьте слабую связанность компонентов

3. 🧪 Тестирование:
   • Создавайте интерфейсы для медиаторов
   • Используйте моки в тестах
   • Тестируйте координацию компонентов
   • Проверяйте корректность уведомлений

4. ⚡ Производительность:
   • Рассмотрите кэширование событий
   • Используйте асинхронную обработку
   • Избегайте ненужных уведомлений
   • Профилируйте критические участки

5. 📚 Документация:
   • Документируйте события и их обработку
   • Описывайте координацию компонентов
   • Приводите примеры использования
   • Объясняйте причины выбора паттерна

6. 🔄 Рефакторинг:
   • Начинайте с простых медиаторов
   • Постепенно добавляйте сложность
   • Используйте интерфейсы для абстракции
   • Планируйте расширяемость
```

## 🎓 Ключевые выводы

```
┌─────────────────────────────────────────────────────────────────┐
│                      КЛЮЧЕВЫЕ ВЫВОДЫ                          │
└─────────────────────────────────────────────────────────────────┘

✅ Mediator - мощный инструмент:
   • Упрощает связи между объектами
   • Централизует управление взаимодействием
   • Обеспечивает слабую связанность
   • Следует принципам SOLID

✅ Выбирайте подходящую реализацию:
   • Classic Mediator - для большинства случаев
   • Event-based Mediator - для событийных систем
   • Template Mediator - для типизированных случаев
   • Hybrid Mediator - для сложных систем

✅ Используйте правильно:
   • Когда много объектов с взаимными зависимостями
   • Когда нужна централизация управления
   • Когда важно упростить связи
   • Когда важна расширяемость

✅ Избегайте злоупотребления:
   • Не используйте для простых случаев
   • Не добавляйте сложность без необходимости
   • Не забывайте об обработке ошибок
   • Не нарушайте принципы SOLID

✅ Помните о компромиссах:
   • Централизация vs распределенность
   • Упрощение vs сложность
   • Гибкость vs производительность
   • Тестируемость vs производительность
```

---
*Mediator - это посредник между объектами. 
Используйте его для упрощения сложных взаимодействий!*
