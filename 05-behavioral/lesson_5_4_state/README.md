# Урок 5.4: State Pattern (Состояние)

## 🎯 Цель урока
Изучить паттерн State - поведенческий паттерн для изменения поведения объекта в зависимости от его внутреннего состояния. Понять, как инкапсулировать поведение, зависящее от состояния, и как изменять его динамически.

## 📚 Что изучаем

### 1. Паттерн State
- **Определение**: Позволяет объекту изменять поведение в зависимости от внутреннего состояния
- **Назначение**: Устранение больших switch/if-else конструкций для состояний
- **Применение**: Конечные автоматы, протоколы, UI состояния, игровые механики
- **Принцип**: Инкапсуляция поведения каждого состояния в отдельном классе

### 2. Структура паттерна
- **Context**: Объект, поведение которого зависит от состояния
- **State**: Интерфейс для инкапсуляции поведения состояния
- **ConcreteState**: Конкретные состояния с реализацией поведения
- **Transitions**: Переходы между состояниями

### 3. Отличия от других паттернов
- **vs Strategy**: State управляет переходами сам, Strategy выбирается извне
- **vs Command**: State инкапсулирует поведение, Command - запросы
- **vs Finite State Machine**: State - реализация FSM в ООП стиле

## 🔍 Ключевые концепции

### Проблема: Большие switch/if-else конструкции
```cpp
// ❌ ПРОБЛЕМА: Сложная логика состояний в одном классе

class DocumentBad {
private:
    enum class State { DRAFT, MODERATION, PUBLISHED };
    State currentState_;
    
public:
    void publish() {
        switch (currentState_) {
            case State::DRAFT:
                // Отправить на модерацию
                std::cout << "Отправлено на модерацию" << std::endl;
                currentState_ = State::MODERATION;
                break;
                
            case State::MODERATION:
                // Публикация после модерации
                std::cout << "Документ опубликован" << std::endl;
                currentState_ = State::PUBLISHED;
                break;
                
            case State::PUBLISHED:
                std::cout << "Документ уже опубликован" << std::endl;
                break;
        }
    }
    
    void reject() {
        switch (currentState_) {
            case State::DRAFT:
                std::cout << "Нельзя отклонить черновик" << std::endl;
                break;
                
            case State::MODERATION:
                std::cout << "Документ отклонен, возврат в черновик" << std::endl;
                currentState_ = State::DRAFT;
                break;
                
            case State::PUBLISHED:
                std::cout << "Нельзя отклонить опубликованный документ" << std::endl;
                break;
        }
    }
    
    // Проблемы:
    // - Каждый метод содержит switch по всем состояниям
    // - Добавление нового состояния требует изменения всех методов
    // - Сложно понять логику конкретного состояния
    // - Нарушение Open/Closed Principle
};
```

**Теоретическая база**: Проблема state explosion: для N состояний и M действий получаем N×M ветвей в коде. State Pattern уменьшает сложность до O(N + M) через полиморфизм: N классов состояний, каждый с M методами.

### Решение: State Pattern
```cpp
// ============================================================================
// FORWARD DECLARATION
// ============================================================================

class Document;  // Forward declaration для Context

// ============================================================================
// STATE INTERFACE
// ============================================================================

// Абстрактное состояние
class DocumentState {
public:
    virtual ~DocumentState() = default;
    
    virtual void publish(Document& doc) = 0;
    virtual void reject(Document& doc) = 0;
    virtual void edit(Document& doc) = 0;
    
    virtual std::string getStateName() const = 0;
};

// ============================================================================
// CONTEXT
// ============================================================================

// Контекст - документ с изменяемым состоянием
class Document {
private:
    std::unique_ptr<DocumentState> state_;
    std::string content_;
    
public:
    explicit Document(std::unique_ptr<DocumentState> initialState)
        : state_(std::move(initialState)) {}
    
    // Изменение состояния
    void setState(std::unique_ptr<DocumentState> newState) {
        std::cout << "Переход из состояния '" << state_->getStateName() 
                  << "' в '" << newState->getStateName() << "'" << std::endl;
        state_ = std::move(newState);
    }
    
    // Делегирование действий текущему состоянию
    void publish() {
        state_->publish(*this);
    }
    
    void reject() {
        state_->reject(*this);
    }
    
    void edit() {
        state_->edit(*this);
    }
    
    void setContent(const std::string& content) {
        content_ = content;
    }
    
    std::string getContent() const {
        return content_;
    }
    
    std::string getCurrentState() const {
        return state_->getStateName();
    }
};

// ============================================================================
// CONCRETE STATES
// ============================================================================

// Состояние: Черновик
class DraftState : public DocumentState {
public:
    void publish(Document& doc) override {
        std::cout << "Отправка документа на модерацию..." << std::endl;
        doc.setState(std::make_unique<ModerationState>());
    }
    
    void reject(Document& doc) override {
        std::cout << "Нельзя отклонить черновик" << std::endl;
    }
    
    void edit(Document& doc) override {
        std::cout << "Редактирование черновика" << std::endl;
        // Редактирование возможно
    }
    
    std::string getStateName() const override {
        return "DRAFT";
    }
};

// Состояние: На модерации
class ModerationState : public DocumentState {
public:
    void publish(Document& doc) override {
        std::cout << "Документ одобрен и опубликован" << std::endl;
        doc.setState(std::make_unique<PublishedState>());
    }
    
    void reject(Document& doc) override {
        std::cout << "Документ отклонен, возврат в черновик" << std::endl;
        doc.setState(std::make_unique<DraftState>());
    }
    
    void edit(Document& doc) override {
        std::cout << "Нельзя редактировать документ на модерации" << std::endl;
    }
    
    std::string getStateName() const override {
        return "MODERATION";
    }
};

// Состояние: Опубликован
class PublishedState : public DocumentState {
public:
    void publish(Document& doc) override {
        std::cout << "Документ уже опубликован" << std::endl;
    }
    
    void reject(Document& doc) override {
        std::cout << "Нельзя отклонить опубликованный документ" << std::endl;
    }
    
    void edit(Document& doc) override {
        std::cout << "Нельзя редактировать опубликованный документ" << std::endl;
    }
    
    std::string getStateName() const override {
        return "PUBLISHED";
    }
};

// ============================================================================
// ИСПОЛЬЗОВАНИЕ
// ============================================================================

void demonstrateStatePattern() {
    std::cout << "=== State Pattern ===" << std::endl;
    
    // Создаем документ в состоянии Draft
    Document doc(std::make_unique<DraftState>());
    doc.setContent("Содержимое документа");
    
    std::cout << "Текущее состояние: " << doc.getCurrentState() << "\n" << std::endl;
    
    // Редактируем черновик
    doc.edit();
    std::cout << std::endl;
    
    // Отправляем на модерацию
    doc.publish();
    std::cout << "Текущее состояние: " << doc.getCurrentState() << "\n" << std::endl;
    
    // Пытаемся редактировать (нельзя)
    doc.edit();
    std::cout << std::endl;
    
    // Одобряем и публикуем
    doc.publish();
    std::cout << "Текущее состояние: " << doc.getCurrentState() << "\n" << std::endl;
    
    // Пытаемся опубликовать снова (нельзя)
    doc.publish();
}
```

**Теоретическая база**: State Pattern реализует конечный автомат (Finite State Machine) в объектно-ориентированном стиле. FSM формально определяется как M = (S, Σ, δ, s₀, F), где:
- S - множество состояний (DraftState, ModerationState, PublishedState)
- Σ - алфавит входных символов (publish, reject, edit)
- δ: S × Σ → S - функция переходов (реализована в методах состояний)
- s₀ - начальное состояние (DraftState)
- F ⊆ S - множество конечных состояний

## 🤔 Вопросы для размышления

### 1. Когда использовать State?
**✅ Хорошие случаи:**
- Объект имеет несколько состояний
- Поведение зависит от состояния
- Много условных операторов по состоянию
- Состояния имеют сложную логику переходов

**Примеры применения:**
```cpp
// 1. TCP Connection States
class TCPConnection {
    // CLOSED → LISTEN → SYN_SENT → ESTABLISHED → CLOSE_WAIT → CLOSED
};

// 2. Vending Machine
class VendingMachine {
    // NO_COIN → HAS_COIN → DISPENDING → OUT_OF_STOCK
};

// 3. Media Player
class MediaPlayer {
    // STOPPED → PLAYING → PAUSED → STOPPED
};

// 4. Game Character
class Character {
    // IDLE → WALKING → RUNNING → JUMPING → ATTACKING → DEAD
};
```

**❌ Плохие случаи:**
- Мало состояний (2-3)
- Простая логика переходов
- Состояния не имеют различного поведения

### 2. Преимущества State
```cpp
// ✅ Single Responsibility
// Каждое состояние в отдельном классе
class PlayingState : public MediaPlayerState {
    // Только логика воспроизведения
};

// ✅ Open/Closed Principle
// Добавление нового состояния не требует изменения существующих
class BufferingState : public MediaPlayerState {
    // Новое состояние без изменения других
};

// ✅ Читаемость
// Логика каждого состояния локализована
// Легко понять что делает состояние

// ✅ Устранение условных операторов
// Нет больших switch/if-else
```

### 3. Недостатки State
```cpp
// ❌ Много классов
// Каждое состояние - отдельный класс

// ❌ Сложность для простых случаев
// Для 2-3 состояний может быть overkill

// ❌ Связность между состояниями
// Состояния должны знать о других состояниях для переходов
```

## 🛠️ Практические примеры

### Пример 1: TCP Connection States
```cpp
// TCP соединение с состояниями
class TCPConnection;

// Базовое состояние TCP
class TCPState {
public:
    virtual ~TCPState() = default;
    
    virtual void open(TCPConnection& conn) = 0;
    virtual void close(TCPConnection& conn) = 0;
    virtual void acknowledge(TCPConnection& conn) = 0;
    
    virtual std::string getName() const = 0;
};

// Контекст - TCP соединение
class TCPConnection {
private:
    std::unique_ptr<TCPState> state_;
    std::string address_;
    int port_;
    
public:
    TCPConnection(const std::string& address, int port)
        : address_(address), port_(port) {
        // Начальное состояние - закрыто
        state_ = std::make_unique<ClosedState>();
    }
    
    void setState(std::unique_ptr<TCPState> newState) {
        std::cout << "TCP: " << state_->getName() << " -> " << newState->getName() << std::endl;
        state_ = std::move(newState);
    }
    
    void open() { state_->open(*this); }
    void close() { state_->close(*this); }
    void acknowledge() { state_->acknowledge(*this); }
    
    std::string getAddress() const { return address_; }
    int getPort() const { return port_; }
    std::string getCurrentState() const { return state_->getName(); }
};

// Состояние: Закрыто
class ClosedState : public TCPState {
public:
    void open(TCPConnection& conn) override {
        std::cout << "TCP: Открытие соединения к " 
                  << conn.getAddress() << ":" << conn.getPort() << std::endl;
        conn.setState(std::make_unique<ListenState>());
    }
    
    void close(TCPConnection& conn) override {
        std::cout << "TCP: Соединение уже закрыто" << std::endl;
    }
    
    void acknowledge(TCPConnection& conn) override {
        std::cout << "TCP: Нельзя подтвердить закрытое соединение" << std::endl;
    }
    
    std::string getName() const override { return "CLOSED"; }
};

// Состояние: Прослушивание
class ListenState : public TCPState {
public:
    void open(TCPConnection& conn) override {
        std::cout << "TCP: Соединение устанавливается..." << std::endl;
        conn.setState(std::make_unique<EstablishedState>());
    }
    
    void close(TCPConnection& conn) override {
        std::cout << "TCP: Закрытие соединения" << std::endl;
        conn.setState(std::make_unique<ClosedState>());
    }
    
    void acknowledge(TCPConnection& conn) override {
        std::cout << "TCP: ACK получен в состоянии LISTEN" << std::endl;
    }
    
    std::string getName() const override { return "LISTEN"; }
};

// Состояние: Установлено
class EstablishedState : public TCPState {
public:
    void open(TCPConnection& conn) override {
        std::cout << "TCP: Соединение уже установлено" << std::endl;
    }
    
    void close(TCPConnection& conn) override {
        std::cout << "TCP: Закрытие установленного соединения" << std::endl;
        conn.setState(std::make_unique<CloseWaitState>());
    }
    
    void acknowledge(TCPConnection& conn) override {
        std::cout << "TCP: ACK получен, соединение активно" << std::endl;
    }
    
    std::string getName() const override { return "ESTABLISHED"; }
};

// Состояние: Ожидание закрытия
class CloseWaitState : public TCPState {
public:
    void open(TCPConnection& conn) override {
        std::cout << "TCP: Нельзя открыть соединение в CLOSE_WAIT" << std::endl;
    }
    
    void close(TCPConnection& conn) override {
        std::cout << "TCP: Соединение полностью закрыто" << std::endl;
        conn.setState(std::make_unique<ClosedState>());
    }
    
    void acknowledge(TCPConnection& conn) override {
        std::cout << "TCP: Завершение закрытия" << std::endl;
        conn.setState(std::make_unique<ClosedState>());
    }
    
    std::string getName() const override { return "CLOSE_WAIT"; }
};

// Использование
void demonstrateTCPConnection() {
    std::cout << "\n=== TCP Connection States ===" << std::endl;
    
    TCPConnection conn("192.168.1.100", 8080);
    
    std::cout << "Состояние: " << conn.getCurrentState() << "\n" << std::endl;
    
    conn.open();         // CLOSED -> LISTEN
    conn.open();         // LISTEN -> ESTABLISHED
    conn.acknowledge();  // Подтверждение
    conn.close();        // ESTABLISHED -> CLOSE_WAIT
    conn.acknowledge();  // CLOSE_WAIT -> CLOSED
    
    std::cout << "\nКонечное состояние: " << conn.getCurrentState() << std::endl;
}
```

### Пример 2: Media Player
```cpp
// Медиа плеер с состояниями
class MediaPlayer;

class PlayerState {
public:
    virtual ~PlayerState() = default;
    
    virtual void play(MediaPlayer& player) = 0;
    virtual void pause(MediaPlayer& player) = 0;
    virtual void stop(MediaPlayer& player) = 0;
    virtual void next(MediaPlayer& player) = 0;
    
    virtual std::string getName() const = 0;
};

// Контекст - медиа плеер
class MediaPlayer {
private:
    std::unique_ptr<PlayerState> state_;
    std::string currentTrack_;
    int volume_;
    
public:
    MediaPlayer() : volume_(50) {
        state_ = std::make_unique<StoppedState>();
    }
    
    void setState(std::unique_ptr<PlayerState> newState) {
        std::cout << "Player: " << state_->getName() << " -> " << newState->getName() << std::endl;
        state_ = std::move(newState);
    }
    
    void play() { state_->play(*this); }
    void pause() { state_->pause(*this); }
    void stop() { state_->stop(*this); }
    void next() { state_->next(*this); }
    
    void setTrack(const std::string& track) { currentTrack_ = track; }
    std::string getCurrentTrack() const { return currentTrack_; }
    
    void setVolume(int vol) { volume_ = vol; }
    int getVolume() const { return volume_; }
    
    std::string getState() const { return state_->getName(); }
};

// Состояние: Остановлен
class StoppedState : public PlayerState {
public:
    void play(MediaPlayer& player) override {
        std::cout << "Начало воспроизведения: " << player.getCurrentTrack() << std::endl;
        player.setState(std::make_unique<PlayingState>());
    }
    
    void pause(MediaPlayer& player) override {
        std::cout << "Нельзя поставить на паузу остановленный плеер" << std::endl;
    }
    
    void stop(MediaPlayer& player) override {
        std::cout << "Плеер уже остановлен" << std::endl;
    }
    
    void next(MediaPlayer& player) override {
        std::cout << "Переход к следующему треку" << std::endl;
        player.setTrack("Next Track");
    }
    
    std::string getName() const override { return "STOPPED"; }
};

// Состояние: Воспроизведение
class PlayingState : public PlayerState {
public:
    void play(MediaPlayer& player) override {
        std::cout << "Уже воспроизводится" << std::endl;
    }
    
    void pause(MediaPlayer& player) override {
        std::cout << "Пауза" << std::endl;
        player.setState(std::make_unique<PausedState>());
    }
    
    void stop(MediaPlayer& player) override {
        std::cout << "Остановка воспроизведения" << std::endl;
        player.setState(std::make_unique<StoppedState>());
    }
    
    void next(MediaPlayer& player) override {
        std::cout << "Переход к следующему треку во время воспроизведения" << std::endl;
        player.setTrack("Next Track");
        // Остаемся в состоянии Playing
    }
    
    std::string getName() const override { return "PLAYING"; }
};

// Состояние: Пауза
class PausedState : public PlayerState {
public:
    void play(MediaPlayer& player) override {
        std::cout << "Продолжение воспроизведения" << std::endl;
        player.setState(std::make_unique<PlayingState>());
    }
    
    void pause(MediaPlayer& player) override {
        std::cout << "Уже на паузе" << std::endl;
    }
    
    void stop(MediaPlayer& player) override {
        std::cout << "Остановка с паузы" << std::endl;
        player.setState(std::make_unique<StoppedState>());
    }
    
    void next(MediaPlayer& player) override {
        std::cout << "Переход к следующему треку" << std::endl;
        player.setTrack("Next Track");
        player.setState(std::make_unique<PlayingState>());
    }
    
    std::string getName() const override { return "PAUSED"; }
};

// Использование
void demonstrateMediaPlayer() {
    std::cout << "\n=== Media Player States ===" << std::endl;
    
    MediaPlayer player;
    player.setTrack("Track 1");
    
    std::cout << "Состояние: " << player.getState() << "\n" << std::endl;
    
    player.play();   // STOPPED -> PLAYING
    player.pause();  // PLAYING -> PAUSED
    player.play();   // PAUSED -> PLAYING
    player.next();   // Следующий трек, остаемся в PLAYING
    player.stop();   // PLAYING -> STOPPED
}
```

### Пример 3: Vending Machine
```cpp
// Торговый автомат
class VendingMachine;

class VendingMachineState {
public:
    virtual ~VendingMachineState() = default;
    
    virtual void insertCoin(VendingMachine& machine, int amount) = 0;
    virtual void selectProduct(VendingMachine& machine, int productId) = 0;
    virtual void dispense(VendingMachine& machine) = 0;
    virtual void refund(VendingMachine& machine) = 0;
    
    virtual std::string getName() const = 0;
};

// Контекст - торговый автомат
class VendingMachine {
private:
    std::unique_ptr<VendingMachineState> state_;
    int balance_;
    int selectedProduct_;
    std::map<int, std::pair<std::string, int>> products_;  // id -> (name, price)
    
public:
    VendingMachine() : balance_(0), selectedProduct_(-1) {
        state_ = std::make_unique<NoCoinState>();
        
        // Инициализация товаров
        products_[1] = {"Кола", 50};
        products_[2] = {"Чипсы", 30};
        products_[3] = {"Шоколад", 40};
    }
    
    void setState(std::unique_ptr<VendingMachineState> newState) {
        std::cout << "Автомат: " << state_->getName() << " -> " << newState->getName() << std::endl;
        state_ = std::move(newState);
    }
    
    void insertCoin(int amount) { state_->insertCoin(*this, amount); }
    void selectProduct(int productId) { state_->selectProduct(*this, productId); }
    void dispense() { state_->dispense(*this); }
    void refund() { state_->refund(*this); }
    
    void addBalance(int amount) { balance_ += amount; }
    int getBalance() const { return balance_; }
    void setBalance(int balance) { balance_ = balance; }
    
    void setSelectedProduct(int id) { selectedProduct_ = id; }
    int getSelectedProduct() const { return selectedProduct_; }
    
    bool hasProduct(int id) const { return products_.count(id) > 0; }
    std::string getProductName(int id) const { return products_.at(id).first; }
    int getProductPrice(int id) const { return products_.at(id).second; }
};

// Состояние: Нет монет
class NoCoinState : public VendingMachineState {
public:
    void insertCoin(VendingMachine& machine, int amount) override {
        std::cout << "Монета внесена: " << amount << " руб." << std::endl;
        machine.addBalance(amount);
        machine.setState(std::make_unique<HasCoinState>());
    }
    
    void selectProduct(VendingMachine& machine, int productId) override {
        std::cout << "Сначала внесите монету" << std::endl;
    }
    
    void dispense(VendingMachine& machine) override {
        std::cout << "Нет товара для выдачи" << std::endl;
    }
    
    void refund(VendingMachine& machine) override {
        std::cout << "Нечего возвращать" << std::endl;
    }
    
    std::string getName() const override { return "NO_COIN"; }
};

// Состояние: Есть монеты
class HasCoinState : public VendingMachineState {
public:
    void insertCoin(VendingMachine& machine, int amount) override {
        std::cout << "Монета внесена: " << amount << " руб." << std::endl;
        machine.addBalance(amount);
        std::cout << "Баланс: " << machine.getBalance() << " руб." << std::endl;
    }
    
    void selectProduct(VendingMachine& machine, int productId) override {
        if (!machine.hasProduct(productId)) {
            std::cout << "Товар не найден" << std::endl;
            return;
        }
        
        int price = machine.getProductPrice(productId);
        
        if (machine.getBalance() < price) {
            std::cout << "Недостаточно средств. Нужно еще " 
                      << (price - machine.getBalance()) << " руб." << std::endl;
            return;
        }
        
        std::cout << "Товар выбран: " << machine.getProductName(productId) << std::endl;
        machine.setSelectedProduct(productId);
        machine.setState(std::make_unique<DispensingState>());
        machine.dispense();
    }
    
    void dispense(VendingMachine& machine) override {
        std::cout << "Сначала выберите товар" << std::endl;
    }
    
    void refund(VendingMachine& machine) override {
        std::cout << "Возврат: " << machine.getBalance() << " руб." << std::endl;
        machine.setBalance(0);
        machine.setState(std::make_unique<NoCoinState>());
    }
    
    std::string getName() const override { return "HAS_COIN"; }
};

// Состояние: Выдача товара
class DispensingState : public VendingMachineState {
public:
    void insertCoin(VendingMachine& machine, int amount) override {
        std::cout << "Подождите, идет выдача товара" << std::endl;
    }
    
    void selectProduct(VendingMachine& machine, int productId) override {
        std::cout << "Подождите, идет выдача товара" << std::endl;
    }
    
    void dispense(VendingMachine& machine) override {
        int productId = machine.getSelectedProduct();
        std::string name = machine.getProductName(productId);
        int price = machine.getProductPrice(productId);
        
        std::cout << "Выдача товара: " << name << std::endl;
        
        int balance = machine.getBalance();
        int change = balance - price;
        
        if (change > 0) {
            std::cout << "Сдача: " << change << " руб." << std::endl;
        }
        
        machine.setBalance(0);
        machine.setSelectedProduct(-1);
        machine.setState(std::make_unique<NoCoinState>());
    }
    
    void refund(VendingMachine& machine) override {
        std::cout << "Невозможно вернуть деньги во время выдачи" << std::endl;
    }
    
    std::string getName() const override { return "DISPENSING"; }
};

// Использование
void demonstrateVendingMachine() {
    std::cout << "\n=== Vending Machine States ===" << std::endl;
    
    VendingMachine machine;
    
    machine.selectProduct(1);   // Нужна монета
    machine.insertCoin(20);     // NO_COIN -> HAS_COIN
    machine.selectProduct(1);   // Недостаточно (нужно 50)
    machine.insertCoin(30);     // Добавляем еще
    machine.selectProduct(1);   // HAS_COIN -> DISPENSING -> NO_COIN
}
```

## 🎨 Современные подходы

### State с std::variant (Type-Safe)
```cpp
#include <variant>

// Состояния как типы
struct Stopped {};
struct Playing { std::string track; };
struct Paused { std::string track; int position; };

using PlayerState = std::variant<Stopped, Playing, Paused>;

class ModernPlayer {
private:
    PlayerState state_;
    
public:
    ModernPlayer() : state_(Stopped{}) {}
    
    void play(const std::string& track) {
        std::visit([&](auto&& state) {
            using T = std::decay_t<decltype(state)>;
            
            if constexpr (std::is_same_v<T, Stopped>) {
                state_ = Playing{track};
                std::cout << "Начало: " << track << std::endl;
            }
            else if constexpr (std::is_same_v<T, Paused>) {
                state_ = Playing{state.track};
                std::cout << "Продолжение: " << state.track << std::endl;
            }
            else {
                std::cout << "Уже играет" << std::endl;
            }
        }, state_);
    }
};
```

### Enum-based State Machine
```cpp
// Таблица переходов для простых FSM
enum class State { IDLE, PROCESSING, COMPLETE, ERROR };
enum class Event { START, SUCCESS, FAIL, RESET };

std::map<std::pair<State, Event>, State> transitions = {
    {{State::IDLE, Event::START}, State::PROCESSING},
    {{State::PROCESSING, Event::SUCCESS}, State::COMPLETE},
    {{State::PROCESSING, Event::FAIL}, State::ERROR},
    {{State::ERROR, Event::RESET}, State::IDLE},
    {{State::COMPLETE, Event::RESET}, State::IDLE}
};

State handleEvent(State current, Event event) {
    auto key = std::make_pair(current, event);
    if (transitions.count(key)) {
        return transitions[key];
    }
    return current;  // Недопустимый переход
}
```

## 🚀 Современный C++

### State Machine Libraries
```cpp
// Использование библиотек для FSM
// Например: Boost.MSM, Boost.Statechart

// Простой пример с концепцией
template<typename StateType>
concept StateLike = requires(StateType s) {
    { s.onEnter() } -> std::same_as<void>;
    { s.onExit() } -> std::same_as<void>;
};
```

## 🎯 Практические упражнения

### Упражнение 1: Traffic Light
Создайте светофор с состояниями (Red, Yellow, Green).

### Упражнение 2: Game Character
Реализуйте игрового персонажа с состояниями (Idle, Walk, Run, Jump, Attack).

### Упражнение 3: Order Processing
Создайте систему обработки заказов с состояниями (New, Paid, Shipped, Delivered).

### Упражнение 4: Authentication
Реализуйте систему аутентификации с состояниями (Unauthenticated, Authenticated, Expired).

## 📈 Связь с другими паттернами

### State + Singleton
```cpp
// Состояния как синглтоны
class IdleState : public State {
public:
    static IdleState& getInstance() {
        static IdleState instance;
        return instance;
    }
private:
    IdleState() = default;
};
```

### State + Strategy
```cpp
// Комбинирование State и Strategy
class Context {
    std::unique_ptr<State> state_;
    std::unique_ptr<Strategy> strategy_;
};
```

## 📈 Следующие шаги
После изучения State вы будете готовы к:
- Модулю 6: Современный C++
- Пониманию конечных автоматов
- Проектированию сложных состояний системы
- Реализации протоколов и игровой логики

## ⚠️ Важные предупреждения

1. **Не переусложняйте**: Для простых случаев достаточно enum
2. **Документируйте переходы**: Создайте диаграмму состояний
3. **Избегайте циклических зависимостей**: Между состояниями
4. **Thread-safety**: Учитывайте многопоточность при переходах

---

*"The State pattern allows an object to alter its behavior when its internal state changes."* - Gang of Four

