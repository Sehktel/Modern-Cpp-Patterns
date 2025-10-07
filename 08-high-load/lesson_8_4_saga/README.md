# Урок 8.4: Saga Pattern (Сага)

## 🎯 Цель урока
Изучить паттерн Saga - управление распределенными транзакциями без 2PC (Two-Phase Commit). Понять, как обеспечить консистентность в микросервисной архитектуре через компенсирующие операции.

## 📚 Что изучаем

### 1. Паттерн Saga
- **Определение**: Последовательность локальных транзакций с компенсирующими операциями
- **Назначение**: Распределенные транзакции, eventual consistency, долгоживущие процессы
- **Применение**: E-commerce, booking systems, финансовые операции, microservices

### 2. Два подхода
- **Orchestration**: Центральный координатор управляет saga
- **Choreography**: Сервисы реагируют на события (event-driven)

### 3. Ключевые компоненты
- **Saga Steps**: Шаги с action и compensation
- **Saga Orchestrator**: Координатор выполнения (для orchestration)
- **Event Bus**: Шина событий (для choreography)
- **State Machine**: Управление состоянием saga

## 🔍 Ключевые концепции

### Saga Transaction

```cpp
#include <vector>
#include <functional>
#include <memory>
#include <string>

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
    std::function<bool()> action;        // Основная операция
    std::function<bool()> compensation;  // Компенсация
    StepState state;
    std::string error_message;
    
    SagaStep(const std::string& n,
            std::function<bool()> act,
            std::function<bool()> comp)
        : name(n), 
          action(std::move(act)), 
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
    
    void addStep(const std::string& name,
                std::function<bool()> action,
                std::function<bool()> compensation) {
        steps_.push_back(
            std::make_shared<SagaStep>(name, action, compensation)
        );
    }
    
    bool execute() {
        state_ = SagaState::RUNNING;
        
        // Выполняем шаги последовательно
        for (size_t i = 0; i < steps_.size(); ++i) {
            auto& step = steps_[i];
            step->state = StepState::RUNNING;
            
            try {
                bool success = step->action();
                
                if (!success) {
                    // Отказ - запускаем компенсацию
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
    void compensate(size_t failed_step_index) {
        std::cout << "Starting compensation..." << std::endl;
        
        state_ = SagaState::COMPENSATING;
        
        // Компенсируем в ОБРАТНОМ порядке (LIFO)
        for (int i = static_cast<int>(failed_step_index); i >= 0; --i) {
            auto& step = steps_[i];
            
            if (step->state != StepState::COMPLETED) {
                continue;  // Пропускаем не завершенные
            }
            
            step->state = StepState::COMPENSATING;
            
            try {
                bool success = step->compensation();
                
                if (success) {
                    step->state = StepState::COMPENSATED;
                } else {
                    std::cerr << "Compensation failed for: " 
                              << step->name << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Compensation exception: " 
                          << e.what() << std::endl;
            }
        }
    }
};
```

### E-commerce Order Saga

```cpp
class OrderSaga {
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
        
        // Step 2: Process Payment
        saga->addStep("ProcessPayment",
            [&payments, order_id]() {
                return payments.processPayment(order_id, 99.99);
            },
            [&payments, order_id]() {
                return payments.refundPayment(order_id);
            }
        );
        
        // Step 3: Reserve Inventory
        saga->addStep("ReserveInventory",
            [&inventory, order_id]() {
                return inventory.reserveItems(order_id);
            },
            [&inventory, order_id]() {
                return inventory.releaseItems(order_id);
            }
        );
        
        // Step 4: Schedule Shipping
        saga->addStep("ScheduleShipping",
            [&shipping, order_id]() {
                return shipping.scheduleShipment(order_id);
            },
            [&shipping, order_id]() {
                return shipping.cancelShipment(order_id);
            }
        );
        
        return saga;
    }
};
```

### Saga Orchestrator

```cpp
class SagaOrchestrator {
private:
    std::unordered_map<std::string, std::shared_ptr<SagaTransaction>> sagas_;
    std::mutex mutex_;
    
    std::atomic<size_t> total_sagas_{0};
    std::atomic<size_t> completed_sagas_{0};
    std::atomic<size_t> compensated_sagas_{0};
    
public:
    std::shared_ptr<SagaTransaction> createSaga(const std::string& id) {
        auto saga = std::make_shared<SagaTransaction>(id);
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            sagas_[id] = saga;
        }
        
        total_sagas_.fetch_add(1);
        return saga;
    }
    
    bool executeSaga(const std::string& id) {
        std::shared_ptr<SagaTransaction> saga;
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = sagas_.find(id);
            if (it == sagas_.end()) return false;
            saga = it->second;
        }
        
        bool success = saga->execute();
        
        if (success) {
            completed_sagas_.fetch_add(1);
        } else {
            compensated_sagas_.fetch_add(1);
        }
        
        return success;
    }
};
```

## 🎓 Best Practices

### ✅ DO (Рекомендуется)

1. **Делайте операции идемпотентными**
   - Безопасно повторять
   - Используйте idempotency keys
   - Проверка перед выполнением

2. **Персистируйте состояние saga**
   - Сохранение после каждого шага
   - Восстановление после сбоя
   - Event sourcing для audit trail

3. **Используйте correlation IDs**
   - Трейсинг через distributed систему
   - Debugging и monitoring
   - Связывание логов

4. **Тестируйте компенсацию**
   - Unit tests для compensation logic
   - Chaos engineering
   - Forced failures в тестах

5. **Semantic Locking**
   - НЕ DB locks на долгое время
   - Status flags (PENDING, RESERVED)
   - Optimistic concurrency

### ❌ DON'T (Не рекомендуется)

1. **НЕ используйте для ACID-требуемых операций**
   - Строгая consistency → DB transactions
   - Saga = eventual consistency

2. **НЕ делайте слишком длинные saga**
   - > 10 шагов сложно управлять
   - Разбивайте на подсаги

3. **НЕ забывайте про timeout**
   - Зависшие saga
   - Автоматическая компенсация

4. **НЕ игнорируйте partial failures**
   - Некоторые компенсации могут провалиться
   - Manual intervention механизм

## 📊 Компенсирующие операции

| Операция | Action | Compensation |
|----------|--------|--------------|
| Create Order | INSERT order | UPDATE status='CANCELLED' |
| Process Payment | Charge card | Refund |
| Reserve Inventory | quantity -= 1 | quantity += 1 |
| Send Email | sendEmail() | sendCancellationEmail() |
| Create Shipment | createShipment() | cancelShipment() |

⚠️ **Важно**: Компенсация НЕ всегда возвращает к исходному состоянию!

## 📁 Файлы урока

- `distributed_transactions.cpp` - Полная реализация Saga
- `saga_pattern.cpp` - Базовая реализация
- `saga_vulnerabilities.cpp` - Уязвимости
- `secure_saga_alternatives.cpp` - Безопасные альтернативы
- `SECURITY_ANALYSIS.md` - Анализ безопасности

## 🔗 Связанные паттерны

- **Event Sourcing**: Хранение всех событий saga
- **CQRS**: Разделение команд и запросов
- **Outbox Pattern**: Гарантия доставки событий
- **Circuit Breaker**: Защита saga steps

## 🚀 Практическое применение

### Используется в
- **Uber**: Ride booking saga
- **Amazon**: Order processing
- **Netflix**: Payment processing
- **Booking.com**: Travel reservation

### Когда использовать
✅ Распределенные транзакции в microservices  
✅ Долгоживущие бизнес-процессы  
✅ Eventual consistency приемлема  
✅ Компенсируемые операции

### Когда НЕ использовать
❌ ACID-требуемые операции  
❌ Короткие транзакции в одной БД  
❌ Операции без компенсации  
❌ Strict consistency требования

---

**Автор**: Senior C++ Developer  
**Дата**: 2025-10-07  
**Версия**: 1.0
