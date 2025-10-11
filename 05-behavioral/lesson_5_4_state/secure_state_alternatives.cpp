#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <map>
#include <set>
#include <stdexcept>

/**
 * @file secure_state_alternatives.cpp
 * @brief Безопасные реализации паттерна State
 */

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 1: THREAD-SAFE STATE С МЬЮТЕКСОМ
// Решает: Race Conditions
// ============================================================================

class IState {
public:
    virtual ~IState() = default;
    virtual void handle() = 0;
    virtual std::string getName() const = 0;
};

class SafeLockedState : public IState {
public:
    void handle() override {
        std::cout << "🔒 Дверь заблокирована (thread-safe)\n";
    }
    
    std::string getName() const override { return "Locked"; }
};

class SafeUnlockedState : public IState {
public:
    void handle() override {
        std::cout << "🔓 Дверь разблокирована (thread-safe)\n";
    }
    
    std::string getName() const override { return "Unlocked"; }
};

// Безопасный Context с мьютексом
class ThreadSafeContext {
private:
    std::unique_ptr<IState> state_;
    mutable std::mutex mutex_;
    
public:
    ThreadSafeContext() : state_(std::make_unique<SafeLockedState>()) {}
    
    void setState(std::unique_ptr<IState> new_state) {
        std::lock_guard<std::mutex> lock(mutex_);
        state_ = std::move(new_state);
    }
    
    void request() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (state_) {
            state_->handle();
        }
    }
    
    std::string getStateName() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return state_ ? state_->getName() : "None";
    }
};

void demonstrateThreadSafeState() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 1: Thread-Safe State ===\n";
    
    ThreadSafeContext context;
    
    std::vector<std::thread> threads;
    
    // Поток переключения состояний
    threads.emplace_back([&context]() {
        for (int i = 0; i < 50; ++i) {
            context.setState(std::make_unique<SafeUnlockedState>());
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            context.setState(std::make_unique<SafeLockedState>());
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    
    // Потоки использования
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&context, i]() {
            for (int j = 0; j < 30; ++j) {
                std::cout << "Thread " << i << ": ";
                context.request();
                std::this_thread::sleep_for(std::chrono::milliseconds(15));
            }
        });
    }
    
    for (auto& t : threads) t.join();
    
    std::cout << "✅ Нет race conditions - все операции атомарны\n";
}

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 2: STATE MACHINE С ВАЛИДАЦИЕЙ ПЕРЕХОДОВ
// Решает: Invalid State Transitions
// ============================================================================

enum class OrderStatus {
    CREATED,
    PAID,
    SHIPPED,
    DELIVERED,
    CANCELLED
};

class SecureOrder {
private:
    OrderStatus state_ = OrderStatus::CREATED;
    double amount_ = 0.0;
    bool refunded_ = false;
    std::mutex mutex_;
    
    // Определяем допустимые переходы
    static const std::map<OrderStatus, std::set<OrderStatus>>& getAllowedTransitions() {
        static std::map<OrderStatus, std::set<OrderStatus>> transitions = {
            {OrderStatus::CREATED,   {OrderStatus::PAID, OrderStatus::CANCELLED}},
            {OrderStatus::PAID,      {OrderStatus::SHIPPED, OrderStatus::CANCELLED}},
            {OrderStatus::SHIPPED,   {OrderStatus::DELIVERED}},
            {OrderStatus::DELIVERED, {}},  // Финальное состояние
            {OrderStatus::CANCELLED, {}}   // Финальное состояние
        };
        return transitions;
    }
    
    bool isTransitionAllowed(OrderStatus from, OrderStatus to) const {
        const auto& transitions = getAllowedTransitions();
        auto it = transitions.find(from);
        if (it == transitions.end()) return false;
        return it->second.find(to) != it->second.end();
    }
    
    void transition(OrderStatus new_state) {
        if (!isTransitionAllowed(state_, new_state)) {
            throw std::runtime_error(
                "Invalid transition from " + std::to_string(static_cast<int>(state_)) +
                " to " + std::to_string(static_cast<int>(new_state))
            );
        }
        state_ = new_state;
    }
    
public:
    void pay(double amount) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        transition(OrderStatus::PAID);
        amount_ = amount;
        std::cout << "💳 Оплачено: $" << amount << "\n";
    }
    
    void ship() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        transition(OrderStatus::SHIPPED);
        std::cout << "📦 Отправлено\n";
    }
    
    void deliver() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        transition(OrderStatus::DELIVERED);
        std::cout << "✅ Доставлено\n";
    }
    
    void cancel() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        transition(OrderStatus::CANCELLED);
        
        if (!refunded_ && amount_ > 0) {
            std::cout << "💰 Возврат: $" << amount_ << "\n";
            refunded_ = true;
        }
        
        std::cout << "❌ Отменено\n";
    }
    
    OrderStatus getState() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return state_;
    }
};

void demonstrateValidatedStateMachine() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 2: Validated State Machine ===\n";
    
    SecureOrder order;
    
    // Легитимный flow
    order.pay(100.0);
    order.ship();
    order.deliver();
    
    std::cout << "\nПопытка недопустимых переходов:\n";
    
    // Попытка cancel после delivery
    try {
        order.cancel();
    } catch (const std::runtime_error& e) {
        std::cout << "✅ Блокировано: " << e.what() << "\n";
    }
    
    // Попытка повторной доставки
    try {
        order.deliver();
    } catch (const std::runtime_error& e) {
        std::cout << "✅ Блокировано: " << e.what() << "\n";
    }
    
    std::cout << "✅ Все недопустимые переходы заблокированы\n";
}

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 3: ATOMIC STATE С ENUM
// Решает: Race Conditions без блокировок
// ============================================================================

enum class ConnectionState : uint8_t {
    DISCONNECTED = 0,
    CONNECTING = 1,
    CONNECTED = 2,
    DISCONNECTING = 3
};

class AtomicConnection {
private:
    std::atomic<ConnectionState> state_{ConnectionState::DISCONNECTED};
    
public:
    bool connect() {
        // Атомарное изменение состояния
        ConnectionState expected = ConnectionState::DISCONNECTED;
        if (state_.compare_exchange_strong(expected, ConnectionState::CONNECTING)) {
            std::cout << "🔌 Подключение...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            state_.store(ConnectionState::CONNECTED, std::memory_order_release);
            std::cout << "✅ Подключено\n";
            return true;
        }
        
        std::cout << "❌ Уже подключаемся/подключены\n";
        return false;
    }
    
    bool disconnect() {
        ConnectionState expected = ConnectionState::CONNECTED;
        if (state_.compare_exchange_strong(expected, ConnectionState::DISCONNECTING)) {
            std::cout << "🔌 Отключение...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            
            state_.store(ConnectionState::DISCONNECTED, std::memory_order_release);
            std::cout << "✅ Отключено\n";
            return true;
        }
        
        std::cout << "❌ Не подключены\n";
        return false;
    }
    
    ConnectionState getState() const {
        return state_.load(std::memory_order_acquire);
    }
};

void demonstrateAtomicState() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 3: Atomic State ===\n";
    
    AtomicConnection conn;
    
    // Несколько потоков пытаются подключиться
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&conn, i]() {
            std::cout << "Thread " << i << " пытается подключиться\n";
            conn.connect();  // Только один успешно подключится
        });
    }
    
    for (auto& t : threads) t.join();
    
    conn.disconnect();
    
    std::cout << "✅ Atomic CAS гарантирует корректные переходы\n";
}

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 4: STATE С RAII TRANSITIONS
// Решает: Memory Leaks
// ============================================================================

class State {
public:
    virtual ~State() = default;
    virtual void enter() = 0;
    virtual void exit() = 0;
    virtual std::string getName() const = 0;
};

class IdleState : public State {
public:
    void enter() override {
        std::cout << "[Idle] Вход в состояние\n";
    }
    
    void exit() override {
        std::cout << "[Idle] Выход из состояния\n";
    }
    
    std::string getName() const override { return "Idle"; }
    
    ~IdleState() {
        std::cout << "[Idle] Удален\n";
    }
};

class WorkingState : public State {
public:
    void enter() override {
        std::cout << "[Working] Вход в состояние\n";
    }
    
    void exit() override {
        std::cout << "[Working] Выход из состояния\n";
    }
    
    std::string getName() const override { return "Working"; }
    
    ~WorkingState() {
        std::cout << "[Working] Удален\n";
    }
};

class RAIIStateContext {
private:
    std::unique_ptr<State> current_state_;
    
public:
    RAIIStateContext() : current_state_(std::make_unique<IdleState>()) {
        current_state_->enter();
    }
    
    void transitionTo(std::unique_ptr<State> new_state) {
        if (current_state_) {
            current_state_->exit();
        }
        
        current_state_ = std::move(new_state);
        
        if (current_state_) {
            current_state_->enter();
        }
    }
    
    ~RAIIStateContext() {
        if (current_state_) {
            current_state_->exit();
        }
    }
};

void demonstrateRAIIState() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 4: RAII State Transitions ===\n";
    
    RAIIStateContext context;
    
    std::cout << "\nПереход в Working:\n";
    context.transitionTo(std::make_unique<WorkingState>());
    
    std::cout << "\nПереход в Idle:\n";
    context.transitionTo(std::make_unique<IdleState>());
    
    std::cout << "\nВыход из scope (автоматический exit):\n";
    
    std::cout << "✅ RAII гарантирует вызов enter/exit и удаление\n";
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    std::cout << "=== БЕЗОПАСНЫЕ РЕАЛИЗАЦИИ STATE PATTERN ===\n";
    
    demonstrateThreadSafeState();
    demonstrateValidatedStateMachine();
    demonstrateAtomicState();
    demonstrateRAIIState();
    
    std::cout << "\n=== РЕКОМЕНДАЦИИ ===\n";
    std::cout << "✅ Используйте мьютексы для защиты state\n";
    std::cout << "✅ Валидируйте переходы через transition table\n";
    std::cout << "✅ Используйте std::atomic для простых состояний\n";
    std::cout << "✅ Применяйте RAII для гарантии enter/exit\n";
    std::cout << "✅ Используйте unique_ptr для владения state\n";
    std::cout << "✅ Проверяйте допустимость переходов\n";
    std::cout << "✅ Тестируйте с ThreadSanitizer\n";
    
    return 0;
}
