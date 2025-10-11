# 🛡️ Security Poster: Saga Pattern

## 🎯 Критические угрозы

```
🔴 КРИТИЧНЫЕ:
├─ Compensation Ordering: Incorrect rollback sequence
├─ Race Conditions: Concurrent saga execution
├─ Partial Compensation: Some compensations fail
└─ Idempotency Violation: Non-idempotent operations

🟡 ВЫСОКИЕ:
├─ State Inconsistency: Saga state vs actual state diverge
├─ Timeout Issues: Compensation never executes
└─ Saga Injection: Malicious compensation steps
```

## 💣 Ключевые уязвимости

### 1. Compensation Ordering Bug
```cpp
// ❌ ОПАСНО - Wrong rollback order
class WrongOrderSaga {
public:
    void execute() {
        charge_payment();     // Step 1
        reserve_inventory();  // Step 2
        send_email();         // Step 3 FAILS
        
        // Rollback:
        refund_payment();         // 💥 WRONG! Should be last
        release_inventory();
    }
};

// Правильный порядок: LIFO (reverse order)
// Compensate: send_email → reserve_inventory → charge_payment

// ✅ БЕЗОПАСНО - LIFO compensation
class SafeSaga {
    std::stack<std::function<void()>> compensations_;
    
public:
    void execute() {
        try {
            charge_payment();
            compensations_.push([]{ refund_payment(); });
            
            reserve_inventory();
            compensations_.push([]{ release_inventory(); });
            
            send_email();
            compensations_.push([]{ /* no compensation */ });
            
        } catch (...) {
            // ✅ Compensate в reverse order (LIFO)
            while (!compensations_.empty()) {
                compensations_.top()();
                compensations_.pop();
            }
            throw;
        }
    }
};
```

### 2. Non-Idempotent Compensation
```cpp
// ❌ ОПАСНО - Double compensation
void compensate_payment() {
    refund(amount_);  // 💥 Called twice = double refund!
}

// ✅ БЕЗОПАСНО - Idempotent compensation
void compensate_payment() {
    if (already_compensated("payment")) {
        return;  // ✅ Skip if already done
    }
    
    refund(amount_);
    mark_compensated("payment");
}

// ИЛИ check state
void compensate_payment() {
    if (payment_status_ == REFUNDED) {
        return;  // ✅ Already refunded
    }
    
    refund(amount_);
    payment_status_ = REFUNDED;
}
```

### 3. Race Condition - Concurrent Sagas
```cpp
// ❌ УЯЗВИМО - No locking
void saga_checkout(User user, Item item) {
    if (user.balance >= item.price) {
        user.balance -= item.price;  // 💥 Race!
        inventory.reserve(item);     // 💥 Race!
    }
}

// Two concurrent sagas:
// T1: check balance ($100 >= $80) ✓
// T2: check balance ($100 >= $80) ✓
// T1: deduct $80 → balance = $20
// T2: deduct $80 → balance = -$60  💥 Negative!

// ✅ БЕЗОПАСНО - Distributed locking
void saga_checkout(User user, Item item) {
    auto lock_id = distributed_lock.acquire(
        {"user:" + user.id, "item:" + item.id}
    );
    
    try {
        if (user.balance >= item.price) {
            user.balance -= item.price;
            inventory.reserve(item);
        }
    } finally {
        distributed_lock.release(lock_id);
    }
}
```

### 4. Saga State Persistence Failure
```cpp
// ❌ УЯЗВИМО - State lost on crash
class VolatileSaga {
    std::vector<std::string> completed_steps_;  // ⚠️ In-memory!
    
public:
    void execute() {
        step1();
        completed_steps_.push_back("step1");
        
        step2();  // CRASH HERE
        // → Lost track of completed steps
        // → Can't compensate step1!
    }
};

// ✅ БЕЗОПАСНО - Persistent state
class PersistentSaga {
    std::string saga_id_;
    
    void save_state(const std::vector<std::string>& steps) {
        db_.write("saga:" + saga_id_, serialize(steps));
    }
    
public:
    void execute() {
        std::vector<std::string> completed;
        
        step1();
        completed.push_back("step1");
        save_state(completed);  // ✅ Persist after each step
        
        step2();
        completed.push_back("step2");
        save_state(completed);
        
        // On crash: recover from DB, compensate completed steps
    }
    
    void recover() {
        auto completed = load_state(saga_id_);
        
        // ✅ Compensate в reverse order
        for (auto it = completed.rbegin(); it != completed.rend(); ++it) {
            compensate(*it);
        }
    }
};
```

## 🛠️ Инструменты
```bash
# Saga flow visualization
./visualize_saga --saga-id abc123

# Compensation testing
./test_saga_compensation --inject-failure step2

# State consistency validation
./validate_saga_state --saga-id abc123
```

## 📋 Checklist
```
[ ] LIFO compensation order
[ ] Idempotent operations (steps + compensations)
[ ] Persistent saga state (DB, не memory)
[ ] Distributed locking для concurrent sagas
[ ] Timeout handling (partial execution)
[ ] Saga ID для tracking
[ ] Monitoring (success rate, compensation rate)
[ ] Recovery mechanism (crash recovery)
```

## 💡 Золотое правило
```
"Saga - это distributed transaction without 2PC.
 Compensate в REVERSE order (LIFO).
 ALL operations must be idempotent.
 Persist state after EACH step.
 Lock resources до завершения saga!"
```

---
**Версия**: 1.0 | **Дата**: 2025-10-11

