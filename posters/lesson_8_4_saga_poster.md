# 📊 Плакат: Saga Pattern

## 🎯 Концепция Saga

```
┌─────────────────────────────────────────────────────────────────┐
│                       SAGA PATTERN                             │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│  🔄 РАСПРЕДЕЛЕННЫЕ ТРАНЗАКЦИИ БЕЗ 2PC                          │
│  ↩️ КОМПЕНСИРУЮЩИЕ ОПЕРАЦИИ ПРИ ОТКАЗЕ                        │
│  📊 УПРАВЛЕНИЕ ДОЛГОЖИВУЩИМИ БИЗНЕС-ПРОЦЕССАМИ                │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────┬───────────────────────────────────────────────┐
│   ПРОБЛЕМА      │  • Распределенная транзакция через микросервисы│
│                 │  • 2PC (Two-Phase Commit) не масштабируется   │
│                 │  • Блокировки ресурсов на долгое время        │
│                 │  • Нет ACID в распределенных системах         │
├─────────────────┼───────────────────────────────────────────────┤
│   РЕШЕНИЕ       │  • Последовательность локальных транзакций    │
│                 │  • Компенсирующие операции (rollback)         │
│                 │  • Eventual consistency                       │
│                 │  • Saga Orchestration или Choreography        │
└─────────────────┴───────────────────────────────────────────────┘
```

## 🔄 Жизненный цикл Saga

```
┌─────────────────────────────────────────────────────────────────┐
│                  УСПЕШНАЯ SAGA EXECUTION                       │
└─────────────────────────────────────────────────────────────────┘

ORDER SAGA (E-commerce заказ):

Step 1: Create Order              → ✅ SUCCESS
          ↓
Step 2: Process Payment            → ✅ SUCCESS  
          ↓
Step 3: Reserve Inventory          → ✅ SUCCESS
          ↓
Step 4: Schedule Shipping          → ✅ SUCCESS
          ↓
     [SAGA COMPLETED] 🎉


┌─────────────────────────────────────────────────────────────────┐
│              НЕУДАЧНАЯ SAGA С КОМПЕНСАЦИЕЙ                     │
└─────────────────────────────────────────────────────────────────┘

ORDER SAGA (Отказ на шаге 3):

Step 1: Create Order              → ✅ SUCCESS
          ↓
Step 2: Process Payment            → ✅ SUCCESS
          ↓
Step 3: Reserve Inventory          → ❌ FAILED (нет товара)
          ↓
     [START COMPENSATION] ⚠️
          ↓
Compensate Step 2: Refund Payment  → ✅ COMPENSATED
          ↓
Compensate Step 1: Cancel Order    → ✅ COMPENSATED
          ↓
     [SAGA ROLLED BACK] ↩️
```

## 📊 Два подхода: Orchestration vs Choreography

```
┌─────────────────────────────────────────────────────────────────┐
│              ORCHESTRATION (Оркестрация)                       │
└─────────────────────────────────────────────────────────────────┘

Центральный координатор (Saga Orchestrator) управляет процессом:

                    ┌──────────────────┐
                    │  Saga Orchestr.  │
                    │  (Coordinator)   │
                    └──────────────────┘
                      │    │    │    │
        ┌─────────────┘    │    │    └─────────────┐
        ↓                  ↓    ↓                  ↓
  ┌──────────┐     ┌──────────┐     ┌──────────┐ ┌──────────┐
  │  Order   │     │ Payment  │     │Inventory │ │ Shipping │
  │ Service  │     │ Service  │     │ Service  │ │ Service  │
  └──────────┘     └──────────┘     └──────────┘ └──────────┘

ПЛЮСЫ:
✅ Централизованная логика
✅ Легко понять flow
✅ Простая отладка
✅ Контроль над порядком выполнения

МИНУСЫ:
❌ Single point of failure (orchestrator)
❌ Tight coupling к orchestrator
❌ Orchestrator может стать bottleneck


┌─────────────────────────────────────────────────────────────────┐
│              CHOREOGRAPHY (Хореография)                        │
└─────────────────────────────────────────────────────────────────┘

Сервисы реагируют на события (Event-Driven):

┌──────────┐   OrderCreated   ┌──────────┐
│  Order   │──────────────────>│ Payment  │
│ Service  │                   │ Service  │
└──────────┘                   └──────────┘
                                    │ PaymentProcessed
                                    ↓
                               ┌──────────┐
                               │Inventory │
                               │ Service  │
                               └──────────┘
                                    │ InventoryReserved
                                    ↓
                               ┌──────────┐
                               │ Shipping │
                               │ Service  │
                               └──────────┘

ПЛЮСЫ:
✅ Нет single point of failure
✅ Loose coupling
✅ Лучшая масштабируемость
✅ Каждый сервис независим

МИНУСЫ:
❌ Сложно понять flow
❌ Трудная отладка
❌ Cyclic dependencies риск
❌ Сложнее обработка ошибок
```

## 🔧 Реализация Saga Orchestrator

```
┌─────────────────────────────────────────────────────────────────┐
│              SAGA ORCHESTRATOR IMPLEMENTATION                  │
└─────────────────────────────────────────────────────────────────┘

class SagaTransaction {
private:
    std::string transaction_id_;
    std::vector<std::shared_ptr<SagaStep>> steps_;
    TransactionState state_;
    
public:
    // Добавление шага с компенсацией
    void addStep(std::string name,
                std::function<bool()> action,
                std::function<bool()> compensation) {
        steps_.push_back(
            std::make_shared<SagaStep>(name, action, compensation)
        );
    }
    
    // Выполнение Saga
    bool execute() {
        state_ = TransactionState::RUNNING;
        
        // Выполняем шаги последовательно
        for (size_t i = 0; i < steps_.size(); ++i) {
            auto& step = steps_[i];
            
            try {
                bool success = step->action();
                
                if (!success) {
                    // Отказ → компенсация
                    compensate(i);
                    state_ = TransactionState::COMPENSATED;
                    return false;
                }
                
                step->state = StepState::COMPLETED;
                
            } catch (const std::exception& e) {
                // Исключение → компенсация
                compensate(i);
                state_ = TransactionState::COMPENSATED;
                return false;
            }
        }
        
        state_ = TransactionState::COMPLETED;
        return true;
    }
    
private:
    // Компенсация в обратном порядке
    void compensate(size_t failed_step_index) {
        for (int i = failed_step_index; i >= 0; --i) {
            auto& step = steps_[i];
            
            if (step->state == StepState::COMPLETED) {
                try {
                    step->compensation();
                    step->state = StepState::COMPENSATED;
                } catch (...) {
                    // Логируем ошибку компенсации
                }
            }
        }
    }
};

ИСПОЛЬЗОВАНИЕ:
┌─────────────────────────────────────────────────────────────────┐
│ auto saga = orchestrator.createTransaction("ORDER_12345");     │
│                                                                 │
│ saga->addStep("CreateOrder",                                    │
│     []() { return orderService.createOrder("ORDER_12345"); },  │
│     []() { return orderService.cancelOrder("ORDER_12345"); }   │
│ );                                                              │
│                                                                 │
│ saga->addStep("ProcessPayment",                                 │
│     []() { return paymentService.process(99.99); },            │
│     []() { return paymentService.refund("ORDER_12345"); }      │
│ );                                                              │
│                                                                 │
│ bool success = saga->execute();                                 │
└─────────────────────────────────────────────────────────────────┘
```

## 💡 Примеры компенсирующих операций

```
┌─────────────────────────────────────────────────────────────────┐
│              COMPENSATING TRANSACTIONS                         │
└─────────────────────────────────────────────────────────────────┘

┌──────────────────┬────────────────────┬──────────────────────────┐
│   ОПЕРАЦИЯ       │     ACTION         │     COMPENSATION         │
├──────────────────┼────────────────────┼──────────────────────────┤
│ Create Order     │ INSERT INTO orders │ UPDATE orders            │
│                  │ status='PENDING'   │ SET status='CANCELLED'   │
├──────────────────┼────────────────────┼──────────────────────────┤
│ Process Payment  │ Charge card        │ Refund payment           │
│                  │ amount=100         │ amount=100               │
├──────────────────┼────────────────────┼──────────────────────────┤
│ Reserve          │ UPDATE inventory   │ UPDATE inventory         │
│ Inventory        │ quantity -= 1      │ quantity += 1            │
├──────────────────┼────────────────────┼──────────────────────────┤
│ Send Email       │ sendEmail()        │ sendCancellationEmail()  │
│                  │ "Order confirmed"  │ "Order cancelled"        │
├──────────────────┼────────────────────┼──────────────────────────┤
│ Create Shipment  │ createShipment()   │ cancelShipment()         │
│                  │                    │                          │
└──────────────────┴────────────────────┴──────────────────────────┘

⚠️ ВАЖНО: Компенсация НЕ всегда возвращает к исходному состоянию!
Пример: Отправленный email нельзя "отменить", только отправить новый.
```

## ⚠️ Проблемы и решения

```
┌─────────────────────────────────────────────────────────────────┐
│                  ТИПИЧНЫЕ ПРОБЛЕМЫ И РЕШЕНИЯ                   │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────┬───────────────────────────────────────────────┐
│   COMPENSATION  │  РЕШЕНИЕ:                                     │
│   FAILURE       │  • Retry с exponential backoff                │
│                 │  • Manual intervention (admin console)        │
│   Компенсация   │  • Dead letter queue для failed compensations│
│   не удалась    │  • Idempotent compensation operations         │
├─────────────────┼───────────────────────────────────────────────┤
│   ORDERING      │  РЕШЕНИЕ:                                     │
│   ISSUES        │  • Строгий порядок компенсации (LIFO)        │
│                 │  • Dependency graph для параллельных шагов    │
│   Неправильный  │  • Versioning для saga steps                  │
│   порядок       │  • Distributed tracing                        │
│   компенсации   │                                               │
├─────────────────┼───────────────────────────────────────────────┤
│   ISOLATION     │  РЕШЕНИЕ:                                     │
│   PROBLEMS      │  • Semantic locks (не DB locks!)              │
│                 │  • Pessimistic view (show as "pending")       │
│   Другая saga   │  • Optimistic concurrency control             │
│   видит         │  • Saga correlation ID                        │
│   inconsistent  │                                               │
│   state         │                                               │
├─────────────────┼───────────────────────────────────────────────┤
│   ORCHESTRATOR  │  РЕШЕНИЕ:                                     │
│   FAILURE       │  • State persistence (DB, event store)        │
│                 │  • Saga recovery mechanism                    │
│   Orchestrator  │  • Multiple orchestrator instances            │
│   упал mid-saga │  • Checkpointing после каждого шага           │
└─────────────────┴───────────────────────────────────────────────┘
```

## 🔍 Saga State Machine

```
┌─────────────────────────────────────────────────────────────────┐
│                  SAGA STATE MACHINE                            │
└─────────────────────────────────────────────────────────────────┘

           ┌────────────┐
           │  PENDING   │
           └────────────┘
                 │
            start saga
                 ↓
           ┌────────────┐
     ┌─────│  RUNNING   │─────┐
     │     └────────────┘     │
     │                        │
all steps           step failed
succeed                 │
     │                  ↓
     │            ┌─────────────┐
     │            │COMPENSATING │
     │            └─────────────┘
     │                  │
     │         all compensations
     │            completed
     ↓                  ↓
┌────────────┐    ┌────────────┐
│ COMPLETED  │    │COMPENSATED │
└────────────┘    └────────────┘
     │                  │
     └─────────┬────────┘
               ↓
          ┌────────────┐
          │  ARCHIVED  │
          └────────────┘

ПЕРЕХОДЫ:
• PENDING → RUNNING: Начало выполнения
• RUNNING → COMPLETED: Все шаги успешны
• RUNNING → COMPENSATING: Отказ любого шага
• COMPENSATING → COMPENSATED: Компенсация завершена
• COMPLETED/COMPENSATED → ARCHIVED: Архивация после TTL
```

## 📊 Метрики и мониторинг

```
┌─────────────────────────────────────────────────────────────────┐
│                   КЛЮЧЕВЫЕ МЕТРИКИ SAGA                        │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│  📈 ОПЕРАЦИОННЫЕ МЕТРИКИ                                        │
│  • Saga success rate: процент успешных saga                     │
│  • Saga compensation rate: процент компенсированных             │
│  • Saga duration p50/p95/p99: время выполнения                 │
│  • Active sagas: количество выполняющихся saga                  │
│  • Failed sagas: провалившиеся saga                             │
├─────────────────────────────────────────────────────────────────┤
│  ⏱️ ПРОИЗВОДИТЕЛЬНОСТЬ ШАГОВ                                    │
│  • Step execution time: время каждого шага                      │
│  • Step failure rate: процент отказов на каждом шаге           │
│  • Compensation duration: время компенсации                     │
│  • Bottleneck steps: самые медленные шаги                       │
├─────────────────────────────────────────────────────────────────┤
│  🔍 ДИАГНОСТИКА                                                 │
│  • Saga state distribution: распределение по состояниям         │
│  • Compensations by step: где чаще всего компенсация           │
│  • Retry count per saga: количество ретраев                     │
│  • Stuck sagas: зависшие saga (> threshold time)               │
└─────────────────────────────────────────────────────────────────┘

ALERTS:
⚠️  Saga success rate < 95%
⚠️  Compensation rate > 20%
⚠️  Saga duration p99 > 30 seconds
⚠️  Stuck sagas detected (> 5 minutes in RUNNING)
```

## 🎓 Best Practices

```
┌─────────────────────────────────────────────────────────────────┐
│                      ЛУЧШИЕ ПРАКТИКИ                           │
└─────────────────────────────────────────────────────────────────┘

✅ DO (Рекомендуется):
┌─────────────────────────────────────────────────────────────────┐
│ 1. Делайте операции ИДЕМПОТЕНТНЫМИ                             │
│    • Можно безопасно повторить                                  │
│    • Используйте idempotency keys                               │
│                                                                 │
│ 2. Персистируйте состояние Saga                                │
│    • После каждого шага сохраняйте в DB                        │
│    • Позволяет восстановить после сбоя                         │
│                                                                 │
│ 3. Логируйте все переходы состояний                            │
│    • Saga ID, Step ID, State transitions                        │
│    • Distributed tracing                                        │
│                                                                 │
│ 4. Используйте correlation IDs                                 │
│    • Прослеживание saga через сервисы                          │
│    • Упрощает debugging                                         │
│                                                                 │
│ 5. Тестируйте компенсацию                                      │
│    • Unit tests для compensation logic                          │
│    • Chaos engineering (forced failures)                        │
│                                                                 │
│ 6. Semantic Locking вместо DB locks                            │
│    • Не блокируйте ресурсы на долго                            │
│    • Используйте status flags (PENDING, RESERVED)              │
└─────────────────────────────────────────────────────────────────┘

❌ DON'T (Не рекомендуется):
┌─────────────────────────────────────────────────────────────────┐
│ 1. НЕ используйте для ACID-требуемых операций                  │
│    ❌ Financial transactions requiring strict consistency       │
│                                                                 │
│ 2. НЕ делайте слишком длинные Saga                             │
│    ❌ > 10 шагов сложно управлять                              │
│                                                                 │
│ 3. НЕ забывайте про timeout для шагов                          │
│    ❌ Бесконечное ожидание                                     │
│                                                                 │
│ 4. НЕ игнорируйте partial failures                             │
│    ❌ Некоторые компенсации могут провалиться                  │
│                                                                 │
│ 5. НЕ смешивайте синхронные и асинхронные шаги                │
│    ❌ Усложняет orchestration                                  │
└─────────────────────────────────────────────────────────────────┘
```

## 🚀 Когда использовать Saga

```
┌─────────────────────────────────────────────────────────────────┐
│                    КОГДА ИСПОЛЬЗОВАТЬ                          │
└─────────────────────────────────────────────────────────────────┘

✅ ПОДХОДИТ ДЛЯ:
  • E-commerce orders (заказ → оплата → доставка)
  • Travel booking (рейс + отель + аренда авто)
  • Long-running business processes
  • Microservices с распределенными транзакциями
  • Системы где eventual consistency приемлема
  • Процессы с компенсируемыми операциями

❌ НЕ ПОДХОДИТ ДЛЯ:
  • ACID-требуемые операции (банковские переводы)
  • Короткие транзакции в одной БД (используйте DB transactions)
  • Операции без компенсации (отправка email)
  • Real-time системы с strict consistency
  • Системы где rollback невозможен
```

## 📚 Связанные паттерны

```
┌─────────────────────────────────────────────────────────────────┐
│                   СВЯЗАННЫЕ ПАТТЕРНЫ                           │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────┬───────────────────────────────────────────────┐
│  Event Sourcing │  • Хранение всех событий saga                 │
│                 │  • Replay для восстановления состояния        │
│                 │  • Audit trail                                │
├─────────────────┼───────────────────────────────────────────────┤
│  CQRS           │  • Разделение команд и запросов               │
│                 │  • Saga для write operations                  │
│                 │  • Query для read operations                  │
├─────────────────┼───────────────────────────────────────────────┤
│  Outbox Pattern │  • Гарантия доставки событий                  │
│                 │  • Атомарность бизнес-операции и события      │
│                 │  • Используется в Choreography saga           │
├─────────────────┼───────────────────────────────────────────────┤
│  Circuit        │  • Защита от медленных/недоступных сервисов   │
│  Breaker        │  • Fast fail для saga steps                   │
│                 │  • Комбинируется с Saga                       │
└─────────────────┴───────────────────────────────────────────────┘
```

## 💼 Реальные примеры

```
┌─────────────────────────────────────────────────────────────────┐
│                  ПРИМЕРЫ ИЗ РЕАЛЬНОГО МИРА                     │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│  E-COMMERCE ORDER SAGA                                          │
│  1. Create Order         → Compensation: Cancel Order           │
│  2. Validate Inventory   → Compensation: Release Reservation    │
│  3. Process Payment      → Compensation: Refund                 │
│  4. Reserve Inventory    → Compensation: Return to Stock        │
│  5. Send Confirmation    → Compensation: Send Cancellation      │
│  6. Schedule Shipment    → Compensation: Cancel Shipment        │
├─────────────────────────────────────────────────────────────────┤
│  TRAVEL BOOKING SAGA                                            │
│  1. Reserve Flight       → Compensation: Cancel Flight          │
│  2. Reserve Hotel        → Compensation: Cancel Hotel           │
│  3. Reserve Car          → Compensation: Cancel Car             │
│  4. Process Payment      → Compensation: Refund                 │
│  5. Send Confirmation    → Compensation: Send Cancellation      │
├─────────────────────────────────────────────────────────────────┤
│  MONEY TRANSFER SAGA (Inter-bank)                               │
│  1. Validate Account     → Compensation: None                   │
│  2. Debit Source Account → Compensation: Credit Source          │
│  3. Credit Target Account→ Compensation: Debit Target           │
│  4. Record Transaction   → Compensation: Mark as Reversed       │
│  5. Send Notification    → Compensation: Send Reversal Notice   │
└─────────────────────────────────────────────────────────────────┘
```

---

**Автор**: Senior C++ Developer  
**Дата**: 2025-10-07  
**Версия**: 1.0
