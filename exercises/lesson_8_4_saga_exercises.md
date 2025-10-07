# 🏋️ Упражнения: Saga Pattern

## 📋 Задание 1: Базовая Saga с компенсацией

### Описание
Реализуйте базовую Saga для распределенной транзакции с компенсирующими операциями.

### Требования
1. ✅ SagaStep с action и compensation
2. ✅ Последовательное выполнение шагов
3. ✅ Автоматическая компенсация при отказе
4. ✅ Компенсация в обратном порядке (LIFO)
5. ✅ Метрики: успехи, отказы, компенсации

### Шаблон для реализации
```cpp
#include <iostream>
#include <vector>
#include <functional>
#include <memory>
#include <string>
#include <chrono>

enum class StepState {
    PENDING,
    RUNNING,
    COMPLETED,
    FAILED,
    COMPENSATING,
    COMPENSATED
};

enum class SagaState {
    PENDING,
    RUNNING,
    COMPLETED,
    COMPENSATED,
    FAILED
};

struct SagaStep {
    std::string name;
    std::function<bool()> action;
    std::function<bool()> compensation;
    StepState state;
    std::string error_message;
    
    SagaStep(const std::string& n,
            std::function<bool()> act,
            std::function<bool()> comp)
        : name(n), action(std::move(act)), 
          compensation(std::move(comp)),
          state(StepState::PENDING) {}
};

class SagaTransaction {
private:
    std::string transaction_id_;
    std::vector<std::shared_ptr<SagaStep>> steps_;
    SagaState state_;
    
public:
    explicit SagaTransaction(const std::string& id)
        : transaction_id_(id), state_(SagaState::PENDING) {}
    
    // TODO: Реализуйте добавление шага
    void addStep(const std::string& name,
                std::function<bool()> action,
                std::function<bool()> compensation) {
        // Ваш код здесь
    }
    
    // TODO: Реализуйте выполнение saga
    bool execute() {
        state_ = SagaState::RUNNING;
        
        // TODO: Выполните шаги последовательно
        for (size_t i = 0; i < steps_.size(); ++i) {
            auto& step = steps_[i];
            step->state = StepState::RUNNING;
            
            try {
                bool success = step->action();
                
                if (!success) {
                    // TODO: Компенсация
                    compensate(i);
                    state_ = SagaState::COMPENSATED;
                    return false;
                }
                
                step->state = StepState::COMPLETED;
                
            } catch (const std::exception& e) {
                step->error_message = e.what();
                compensate(i);
                state_ = SagaState::COMPENSATED;
                return false;
            }
        }
        
        state_ = SagaState::COMPLETED;
        return true;
    }
    
private:
    // TODO: Реализуйте компенсацию
    void compensate(size_t failed_step_index) {
        // Ваш код здесь
        // Компенсируйте в обратном порядке
    }
    
public:
    void printStats() const {
        // TODO: Вывод состояния saga и всех шагов
    }
};
```

### Тесты
```cpp
// Имитация сервисов
class OrderService {
public:
    bool createOrder(const std::string& order_id) {
        std::cout << "Creating order " << order_id << std::endl;
        return true;
    }
    
    bool cancelOrder(const std::string& order_id) {
        std::cout << "Cancelling order " << order_id << std::endl;
        return true;
    }
};

class PaymentService {
private:
    bool should_fail_;
public:
    PaymentService(bool fail = false) : should_fail_(fail) {}
    
    bool processPayment(const std::string& order_id, double amount) {
        std::cout << "Processing payment $" << amount << std::endl;
        return !should_fail_;
    }
    
    bool refundPayment(const std::string& order_id) {
        std::cout << "Refunding payment" << std::endl;
        return true;
    }
};

void testSuccessfulSaga() {
    OrderService orders;
    PaymentService payments(false);  // Не отказывает
    
    SagaTransaction saga("ORDER_123");
    
    saga.addStep("CreateOrder",
        [&]() { return orders.createOrder("ORDER_123"); },
        [&]() { return orders.cancelOrder("ORDER_123"); }
    );
    
    saga.addStep("ProcessPayment",
        [&]() { return payments.processPayment("ORDER_123", 99.99); },
        [&]() { return payments.refundPayment("ORDER_123"); }
    );
    
    bool success = saga.execute();
    assert(success && "Saga should complete successfully");
    
    saga.printStats();
    std::cout << "Successful saga test PASSED ✅" << std::endl;
}

void testFailedSaga() {
    OrderService orders;
    PaymentService payments(true);  // Отказывает
    
    SagaTransaction saga("ORDER_456");
    
    saga.addStep("CreateOrder",
        [&]() { return orders.createOrder("ORDER_456"); },
        [&]() { return orders.cancelOrder("ORDER_456"); }
    );
    
    saga.addStep("ProcessPayment",
        [&]() { return payments.processPayment("ORDER_456", 99.99); },
        [&]() { return payments.refundPayment("ORDER_456"); }
    );
    
    bool success = saga.execute();
    assert(!success && "Saga should fail");
    
    saga.printStats();
    std::cout << "Failed saga with compensation test PASSED ✅" << std::endl;
}
```

---

## 📋 Задание 2: Saga Orchestrator

### Описание
Реализуйте Saga Orchestrator для управления множественными saga транзакциями.

### Требования
1. ✅ Управление несколькими sagas
2. ✅ State persistence (сохранение состояния)
3. ✅ Recovery после сбоя orchestrator
4. ✅ Correlation IDs для трейсинга
5. ✅ Saga timeout и автоматическая компенсация

### Шаблон
```cpp
class SagaOrchestrator {
private:
    std::unordered_map<std::string, std::shared_ptr<SagaTransaction>> sagas_;
    std::mutex mutex_;
    
    // Метрики
    std::atomic<size_t> total_sagas_{0};
    std::atomic<size_t> completed_sagas_{0};
    std::atomic<size_t> failed_sagas_{0};
    std::atomic<size_t> compensated_sagas_{0};
    
public:
    // TODO: Создание новой saga
    std::shared_ptr<SagaTransaction> createSaga(const std::string& id) {
        // Ваш код здесь
    }
    
    // TODO: Выполнение saga
    bool executeSaga(const std::string& id) {
        // Ваш код здесь
        return false;
    }
    
    // TODO: State persistence
    void persistSagaState(const std::string& id) {
        // Сохранение в БД/файл
    }
    
    // TODO: Recovery
    void recoverSagas() {
        // Восстановление из БД/файла
    }
};
```

---

## 📋 Задание 3: Choreography-based Saga

### Описание
Реализуйте Saga используя Choreography подход (event-driven) вместо Orchestration.

### Требования
1. ✅ Event bus для публикации событий
2. ✅ Сервисы подписываются на события
3. ✅ Автоматическая цепочка событий
4. ✅ Compensation events
5. ✅ Distributed tracing

### Шаблон
```cpp
struct Event {
    std::string type;
    std::string saga_id;
    std::string payload;
    std::chrono::system_clock::time_point timestamp;
};

class EventBus {
private:
    std::unordered_map<std::string, std::vector<std::function<void(const Event&)>>> subscribers_;
    std::mutex mutex_;
    
public:
    // TODO: Подписка на тип события
    void subscribe(const std::string& event_type, 
                  std::function<void(const Event&)> handler) {
        // Ваш код здесь
    }
    
    // TODO: Публикация события
    void publish(const Event& event) {
        // Ваш код здесь
    }
};

class OrderServiceChoreography {
private:
    EventBus& event_bus_;
    
public:
    OrderServiceChoreography(EventBus& bus) : event_bus_(bus) {
        // TODO: Подпишитесь на события
        event_bus_.subscribe("StartOrder", [this](const Event& e) {
            handleStartOrder(e);
        });
        
        event_bus_.subscribe("CancelOrder", [this](const Event& e) {
            handleCancelOrder(e);
        });
    }
    
    void handleStartOrder(const Event& e) {
        // TODO: Создать заказ
        // TODO: Опубликовать OrderCreated событие
        event_bus_.publish(Event{"OrderCreated", e.saga_id, ""});
    }
    
    void handleCancelOrder(const Event& e) {
        // TODO: Компенсация - отменить заказ
    }
};
```

---

## 📋 Задание 4: Saga с Timeout и Retry

### Описание
Добавьте поддержку timeout для каждого шага и retry logic при временных сбоях.

### Требования
1. ✅ Timeout для каждого шага
2. ✅ Retry с exponential backoff
3. ✅ Различие между permanent и transient failures
4. ✅ Max retry count per step
5. ✅ Метрики retry attempts

### Шаблон
```cpp
struct SagaStepConfig {
    std::chrono::milliseconds timeout;
    size_t max_retries;
    std::chrono::milliseconds retry_delay;
    double backoff_multiplier;
};

struct SagaStepWithRetry : public SagaStep {
    SagaStepConfig config;
    std::atomic<size_t> retry_count_{0};
    
    bool executeWithRetry() {
        auto delay = config.retry_delay;
        
        for (size_t attempt = 0; attempt <= config.max_retries; ++attempt) {
            try {
                // TODO: Выполните с timeout
                auto future = std::async(std::launch::async, action);
                
                if (future.wait_for(config.timeout) == std::future_status::ready) {
                    return future.get();
                } else {
                    // Timeout
                    if (attempt == config.max_retries) {
                        return false;
                    }
                }
                
            } catch (...) {
                if (attempt == config.max_retries) {
                    throw;
                }
            }
            
            // TODO: Backoff
            std::this_thread::sleep_for(delay);
            delay = std::chrono::milliseconds(
                static_cast<long>(delay.count() * config.backoff_multiplier)
            );
            retry_count_.fetch_add(1);
        }
        
        return false;
    }
};
```

---

## 📋 Задание 5: E-commerce Order Saga

### Описание
Реализуйте полную Saga для обработки e-commerce заказа с реальными сервисами.

### Требования
1. ✅ Order Service: создание/отмена заказа
2. ✅ Payment Service: оплата/возврат
3. ✅ Inventory Service: резервирование/освобождение
4. ✅ Shipping Service: планирование/отмена доставки
5. ✅ Notification Service: уведомления

### Полный пример
```cpp
class EcommerceOrderSaga {
public:
    static std::shared_ptr<SagaTransaction> createOrderSaga(
        const std::string& order_id,
        OrderService& orders,
        PaymentService& payments,
        InventoryService& inventory,
        ShippingService& shipping) {
        
        auto saga = std::make_shared<SagaTransaction>(order_id);
        
        // Step 1: Create Order
        saga->addStep("CreateOrder",
            [&orders, order_id]() {
                return orders.createOrder(order_id);
            },
            [&orders, order_id]() {
                return orders.cancelOrder(order_id);
            }
        );
        
        // TODO: Добавьте остальные шаги
        
        return saga;
    }
};
```

---

**Автор**: Senior C++ Developer  
**Дата**: 2025-10-07  
**Сложность**: ⭐⭐⭐⭐⭐ (Экспертный уровень)
