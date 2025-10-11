#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>

/**
 * @file state_vulnerabilities.cpp
 * @brief Уязвимые реализации паттерна State
 * 
 * ⚠️  ТОЛЬКО ДЛЯ ОБРАЗОВАТЕЛЬНЫХ ЦЕЛЕЙ!
 */

// ============================================================================
// УЯЗВИМОСТЬ 1: RACE CONDITION ПРИ СМЕНЕ СОСТОЯНИЯ
// Проблема: Переход состояния и проверка не атомарны
// ============================================================================

class IState {
public:
    virtual ~IState() = default;
    virtual void handle() = 0;
    virtual std::string getName() const = 0;
};

class Context;

class LockedState : public IState {
public:
    void handle() override {
        std::cout << "❌ Дверь заблокирована\n";
    }
    
    std::string getName() const override { return "Locked"; }
};

class UnlockedState : public IState {
public:
    void handle() override {
        std::cout << "✅ Дверь разблокирована\n";
    }
    
    std::string getName() const override { return "Unlocked"; }
};

// УЯЗВИМЫЙ CONTEXT без синхронизации
class VulnerableContext {
private:
    IState* state_;  // НЕТ МЬЮТЕКСА!
    
public:
    VulnerableContext() : state_(new LockedState()) {}
    
    ~VulnerableContext() {
        delete state_;
    }
    
    void setState(IState* new_state) {
        delete state_;
        state_ = new_state;  // RACE CONDITION!
    }
    
    void request() {
        state_->handle();  // RACE: state_ может измениться!
    }
    
    std::string getStateName() const {
        return state_->getName();  // RACE!
    }
};

void demonstrateRaceCondition() {
    std::cout << "\n=== УЯЗВИМОСТЬ 1: Race Condition при смене состояния ===\n";
    
    VulnerableContext context;
    
    // Поток 1: Постоянно меняет состояние
    std::thread changer([&context]() {
        for (int i = 0; i < 100; ++i) {
            context.setState(new UnlockedState());
            std::this_thread::sleep_for(std::chrono::microseconds(10));
            context.setState(new LockedState());
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    });
    
    // Поток 2: Пытается использовать состояние
    std::thread user([&context]() {
        for (int i = 0; i < 100; ++i) {
            context.request();  // RACE: может обратиться к удаленному state!
            std::this_thread::sleep_for(std::chrono::microseconds(15));
        }
    });
    
    changer.join();
    user.join();
    
    std::cout << "⚠️  Race condition может вызвать use-after-free!\n";
}

// ============================================================================
// УЯЗВИМОСТЬ 2: STATE CONFUSION - TOCTOU
// Проблема: Проверка состояния и действие не атомарны
// ============================================================================

enum class DoorState {
    LOCKED,
    UNLOCKED,
    BROKEN
};

class VulnerableDoor {
private:
    DoorState state_ = DoorState::LOCKED;
    // НЕТ МЬЮТЕКСА!
    
public:
    DoorState getState() const {
        return state_;  // Time of Check
    }
    
    void setState(DoorState new_state) {
        state_ = new_state;
    }
    
    void unlock() {
        state_ = DoorState::UNLOCKED;
        std::cout << "🔓 Дверь разблокирована\n";
    }
    
    void lock() {
        state_ = DoorState::LOCKED;
        std::cout << "🔒 Дверь заблокирована\n";
    }
    
    void enter() {
        // УЯЗВИМОСТЬ: TOCTOU
        if (getState() == DoorState::UNLOCKED) {  // Time of Check
            std::cout << "Проверка пройдена...\n";
            
            std::this_thread::sleep_for(std::chrono::milliseconds(50));  // Задержка
            
            // Time of Use - состояние могло измениться!
            std::cout << "🚪 Вход разрешен\n";
        } else {
            std::cout << "❌ Вход запрещен\n";
        }
    }
};

void demonstrateStateTOCTOU() {
    std::cout << "\n=== УЯЗВИМОСТЬ 2: State Confusion (TOCTOU) ===\n";
    
    VulnerableDoor door;
    door.unlock();
    
    // Поток 1: Пытается войти
    std::thread attacker([&door]() {
        std::cout << "[Атакующий] Попытка входа...\n";
        door.enter();  // Проверка: state = UNLOCKED
    });
    
    // Поток 2: Блокирует дверь во время проверки
    std::thread security([&door]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
        std::cout << "[Охрана] Блокировка двери!\n";
        door.lock();  // Меняем state во время TOCTOU window
    });
    
    attacker.join();
    security.join();
    
    std::cout << "⚠️  Вход разрешен даже после блокировки!\n";
}

// ============================================================================
// УЯЗВИМОСТЬ 3: INVALID STATE TRANSITIONS
// Проблема: Нет проверки допустимых переходов между состояниями
// ============================================================================

enum class OrderState {
    CREATED,
    PAID,
    SHIPPED,
    DELIVERED,
    CANCELLED
};

class VulnerableOrder {
private:
    OrderState state_ = OrderState::CREATED;
    double amount_ = 0.0;
    
public:
    void setState(OrderState new_state) {
        // УЯЗВИМОСТЬ: Нет проверки допустимости перехода!
        state_ = new_state;
    }
    
    void pay(double amount) {
        state_ = OrderState::PAID;
        amount_ = amount;
        std::cout << "💳 Оплачено: $" << amount << "\n";
    }
    
    void ship() {
        state_ = OrderState::SHIPPED;
        std::cout << "📦 Отправлено\n";
    }
    
    void deliver() {
        state_ = OrderState::DELIVERED;
        std::cout << "✅ Доставлено\n";
    }
    
    void cancel() {
        state_ = OrderState::CANCELLED;
        std::cout << "❌ Отменено\n";
    }
    
    void refund() {
        if (state_ == OrderState::CANCELLED) {
            std::cout << "💰 Возврат средств: $" << amount_ << "\n";
            amount_ = 0.0;
        } else {
            std::cout << "❌ Возврат невозможен в текущем состоянии\n";
        }
    }
    
    OrderState getState() const { return state_; }
    double getAmount() const { return amount_; }
};

void demonstrateInvalidStateTransition() {
    std::cout << "\n=== УЯЗВИМОСТЬ 3: Invalid State Transitions ===\n";
    
    VulnerableOrder order;
    
    // Легитимный flow: CREATED → PAID → SHIPPED → DELIVERED
    order.pay(100.0);
    order.ship();
    order.deliver();
    
    std::cout << "\nАТАКА: Недопустимые переходы:\n";
    
    // АТАКА 1: Cancel после delivery (не должно быть возможно)
    order.cancel();
    
    // АТАКА 2: Refund после delivery (double refund!)
    order.refund();
    
    // АТАКА 3: Снова ship (дубликат отправки)
    order.ship();
    
    // АТАКА 4: Прямой переход в любое состояние
    order.setState(OrderState::PAID);  // Вернули в PAID после DELIVERED!
    order.refund();  // Еще один возврат!
    
    std::cout << "\n⚠️  Финансовое мошенничество через invalid transitions!\n";
}

// ============================================================================
// УЯЗВИМОСТЬ 4: STATE POLLUTION В SHARED CONTEXT
// Проблема: Несколько потоков используют один Context
// ============================================================================

class TrafficLightState {
public:
    virtual ~TrafficLightState() = default;
    virtual void next(class TrafficLight* light) = 0;
    virtual std::string getColor() const = 0;
};

class TrafficLight {
private:
    TrafficLightState* state_;  // Shared между потоками!
    
public:
    TrafficLight();
    
    void setState(TrafficLightState* new_state) {
        delete state_;
        state_ = new_state;  // НЕТ СИНХРОНИЗАЦИИ!
    }
    
    void next() {
        state_->next(this);  // RACE CONDITION!
    }
    
    std::string getColor() const {
        return state_->getColor();  // RACE CONDITION!
    }
    
    ~TrafficLight() {
        delete state_;
    }
};

class RedState : public TrafficLightState {
public:
    void next(TrafficLight* light) override {
        light->setState(new class GreenState());
    }
    
    std::string getColor() const override { return "RED"; }
};

class GreenState : public TrafficLightState {
public:
    void next(TrafficLight* light) override {
        light->setState(new class YellowState());
    }
    
    std::string getColor() const override { return "GREEN"; }
};

class YellowState : public TrafficLightState {
public:
    void next(TrafficLight* light) override {
        light->setState(new RedState());
    }
    
    std::string getColor() const override { return "YELLOW"; }
};

TrafficLight::TrafficLight() : state_(new RedState()) {}

void demonstrateStatePollution() {
    std::cout << "\n=== УЯЗВИМОСТЬ 4: State Pollution ===\n";
    
    TrafficLight light;
    
    // Несколько потоков меняют состояние
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&light, i]() {
            for (int j = 0; j < 10; ++j) {
                std::cout << "Thread " << i << ": " << light.getColor() << " → ";
                light.next();
                std::cout << light.getColor() << "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }
    
    for (auto& t : threads) t.join();
    
    std::cout << "⚠️  State может быть corruption из-за race conditions!\n";
}

// ============================================================================
// УЯЗВИМОСТЬ 5: MEMORY LEAK ПРИ ПЕРЕХОДАХ
// Проблема: State не удаляется при переходах
// ============================================================================

class LeakyState {
public:
    virtual ~LeakyState() = default;
    virtual void process() = 0;
};

class StateA : public LeakyState {
private:
    std::vector<char> data_;  // Занимает память
    
public:
    StateA() : data_(1024 * 1024, 'A') {  // 1 MB
        std::cout << "StateA создан (1 MB)\n";
    }
    
    ~StateA() {
        std::cout << "StateA удален\n";
    }
    
    void process() override {
        std::cout << "Processing in StateA\n";
    }
};

class StateB : public LeakyState {
private:
    std::vector<char> data_;
    
public:
    StateB() : data_(1024 * 1024, 'B') {  // 1 MB
        std::cout << "StateB создан (1 MB)\n";
    }
    
    ~StateB() {
        std::cout << "StateB удален\n";
    }
    
    void process() override {
        std::cout << "Processing in StateB\n";
    }
};

class LeakyContext {
private:
    LeakyState* state_;
    
public:
    LeakyContext() : state_(new StateA()) {}
    
    void transitionTo(LeakyState* new_state) {
        // УЯЗВИМОСТЬ: Не удаляем старый state!
        state_ = new_state;  // MEMORY LEAK!
    }
    
    void process() {
        state_->process();
    }
};

void demonstrateMemoryLeak() {
    std::cout << "\n=== УЯЗВИМОСТЬ 5: Memory Leak при переходах ===\n";
    
    LeakyContext context;
    
    // Постоянно меняем состояния
    for (int i = 0; i < 10; ++i) {
        context.transitionTo(new StateB());  // StateA не удален!
        context.transitionTo(new StateA());  // StateB не удален!
    }
    
    std::cout << "⚠️  20 объектов создано, но не удалено → 20 MB утечки!\n";
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    std::cout << "=== УЯЗВИМОСТИ STATE PATTERN ===\n";
    std::cout << "⚠️  ВНИМАНИЕ: Этот код содержит уязвимости для обучения!\n";
    
    try {
        demonstrateRaceCondition();
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
    
    try {
        demonstrateStateTOCTOU();
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
    
    try {
        demonstrateInvalidStateTransition();
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
    
    try {
        demonstrateStatePollution();
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
    
    try {
        demonstrateMemoryLeak();
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
    
    std::cout << "\n=== ИНСТРУМЕНТЫ АНАЛИЗА ===\n";
    std::cout << "• ThreadSanitizer: g++ -fsanitize=thread state_vulnerabilities.cpp\n";
    std::cout << "• AddressSanitizer: g++ -fsanitize=address state_vulnerabilities.cpp\n";
    std::cout << "• Valgrind: valgrind --tool=memcheck ./state_vulnerabilities\n";
    std::cout << "• Helgrind: valgrind --tool=helgrind ./state_vulnerabilities\n";
    
    return 0;
}