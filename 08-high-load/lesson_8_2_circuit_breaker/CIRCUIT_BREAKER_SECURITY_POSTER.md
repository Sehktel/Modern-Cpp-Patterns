# 🛡️ Security Poster: Circuit Breaker Pattern

## 🎯 Критические угрозы

```
🔴 КРИТИЧНЫЕ:
├─ State Desynchronization: Circuit state inconsistent
├─ Race Conditions: Concurrent state transitions
├─ Threshold Bypass: Attacker forces circuit open/closed
└─ Resource Exhaustion: Circuit never opens → cascading failure

🟡 ВЫСОКИЕ:
├─ False Positives: Circuit opens too eagerly
├─ False Negatives: Circuit doesn't open when should
└─ Timing Attacks: Circuit state reveals system health
```

## 💣 Ключевые уязвимости

### 1. Race Condition на State Transitions
```cpp
// ❌ УЯЗВИМО - Non-atomic state check
class RacyCircuitBreaker {
    State state_ = CLOSED;
    int failures_ = 0;
    
public:
    void call() {
        if (state_ == OPEN) {  // [1] Check
            throw CircuitOpenException();
        }
        
        try {
            service_->call();
            failures_ = 0;
        } catch (...) {
            failures_++;  // 💥 Data race!
            if (failures_ > threshold_) {
                state_ = OPEN;  // 💥 Data race!
            }
        }
    }
};

// ✅ БЕЗОПАСНО - Atomic operations
class SafeCircuitBreaker {
    std::atomic<State> state_{CLOSED};
    std::atomic<int> failures_{0};
    std::mutex transition_mutex_;
    
public:
    void call() {
        if (state_.load() == OPEN) {
            throw CircuitOpenException();
        }
        
        try {
            service_->call();
            failures_.store(0);
        } catch (...) {
            int current_failures = failures_.fetch_add(1) + 1;
            
            if (current_failures > threshold_) {
                std::lock_guard lock(transition_mutex_);
                // ✅ Double-check под lock
                if (state_.load() == CLOSED) {
                    state_.store(OPEN);
                    schedule_half_open_timer();
                }
            }
        }
    }
};
```

### 2. Threshold Bypass Attack
```cpp
// ❌ УЯЗВИМО - Attacker can manipulate state
class BypassableCircuitBreaker {
public:
    void reset() {  // ⚠️ Public!
        failures_ = 0;
        state_ = CLOSED;
    }
};

// Attacker continuously resets:
while (true) {
    breaker.call();  // Fails
    breaker.reset();  // 💥 Reset → never opens!
}

// ✅ БЕЗОПАСНО - No manual reset, time-based recovery
class SecureCircuitBreaker {
    std::chrono::time_point<std::chrono::steady_clock> open_time_;
    const std::chrono::seconds timeout_{60};
    
    void attempt_half_open() {
        if (state_.load() == OPEN) {
            auto now = std::chrono::steady_clock::now();
            if (now - open_time_ >= timeout_) {
                std::lock_guard lock(mutex_);
                if (state_.load() == OPEN) {  // Double-check
                    state_.store(HALF_OPEN);
                }
            }
        }
    }
    
    // ✅ No public reset method
};
```

### 3. Cascading Failure - Circuit Never Opens
```cpp
// ❌ ОПАСНО - High threshold
class DangerousCircuitBreaker {
    const int THRESHOLD = 1000;  // 💥 Too high!
};

// System overloaded → 1000 failures → cascading failure

// ✅ БЕЗОПАСНО - Dynamic threshold
class AdaptiveCircuitBreaker {
    int calculate_threshold() {
        // ✅ Adjust based on system load
        auto load = monitor_.get_cpu_usage();
        
        if (load > 0.8) {
            return 5;  // Fail fast under high load
        } else if (load > 0.5) {
            return 20;
        } else {
            return 50;  // Normal threshold
        }
    }
    
    void call() {
        int threshold = calculate_threshold();
        // Use dynamic threshold
    }
};
```

## 🛠️ Инструменты
```bash
# Race detection
g++ -fsanitize=thread -g circuit_breaker.cpp

# State transition monitoring
./monitor_circuit_breaker --log-transitions

# Load testing
./stress_test --rps 10000 --duration 60s
```

## 📋 Checklist
```
[ ] Atomic state transitions
[ ] Thread-safe failure counting
[ ] Time-based recovery (no manual reset)
[ ] Dynamic threshold adjustment
[ ] Half-open state testing
[ ] Monitoring (state changes, failure rate)
[ ] Fallback strategy
[ ] Cascading breaker hierarchy
```

## 💡 Золотое правило
```
"Circuit Breaker - это safety valve.
 Atomic state, no manual override.
 Dynamic thresholds based on load.
 Fast fail to prevent cascading!"
```

---
**Версия**: 1.0 | **Дата**: 2025-10-11

