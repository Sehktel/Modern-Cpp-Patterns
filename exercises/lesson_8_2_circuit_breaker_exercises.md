# 🏋️ Упражнения: Circuit Breaker Pattern

## 📋 Задание 1: Базовый Circuit Breaker

### Описание
Реализуйте Circuit Breaker с тремя состояниями: CLOSED, OPEN, HALF_OPEN.

### Требования
1. ✅ Три состояния с правильными переходами
2. ✅ Подсчет ошибок (failure threshold)
3. ✅ Timeout для перехода OPEN → HALF_OPEN
4. ✅ Success threshold для HALF_OPEN → CLOSED
5. ✅ Метрики: успехи, ошибки, отклонения

### Шаблон для реализации
```cpp
#include <iostream>
#include <atomic>
#include <mutex>
#include <chrono>
#include <functional>
#include <stdexcept>

enum class CircuitState {
    CLOSED,
    OPEN,
    HALF_OPEN
};

struct CircuitBreakerConfig {
    size_t failure_threshold;           // 5 failures → OPEN
    size_t success_threshold;           // 2 successes → CLOSED
    std::chrono::milliseconds timeout;  // 10s timeout для reset
};

class CircuitBreaker {
private:
    std::string name_;
    CircuitBreakerConfig config_;
    
    std::atomic<CircuitState> state_{CircuitState::CLOSED};
    std::atomic<size_t> failure_count_{0};
    std::atomic<size_t> success_count_{0};
    std::chrono::system_clock::time_point last_failure_time_;
    
    mutable std::mutex mutex_;
    
    // Метрики
    std::atomic<size_t> total_requests_{0};
    std::atomic<size_t> successful_requests_{0};
    std::atomic<size_t> failed_requests_{0};
    std::atomic<size_t> rejected_requests_{0};
    
public:
    CircuitBreaker(const std::string& name, const CircuitBreakerConfig& config)
        : name_(name), config_(config) {
        last_failure_time_ = std::chrono::system_clock::now();
    }
    
    // TODO: Реализуйте execute
    template<typename F>
    auto execute(F&& func) -> decltype(func()) {
        total_requests_.fetch_add(1);
        
        // TODO: 1. Проверить состояние
        // TODO: 2. Если OPEN, проверить shouldAttemptReset()
        // TODO: 3. Выполнить функцию
        // TODO: 4. Обработать успех/неудачу
    }
    
    CircuitState getState() const {
        return state_.load();
    }
    
    void printStats() const {
        // TODO: Вывод статистики
    }
    
private:
    // TODO: Реализуйте onSuccess
    void onSuccess() {
        // Ваш код здесь
    }
    
    // TODO: Реализуйте onFailure
    void onFailure() {
        // Ваш код здесь
    }
    
    // TODO: Проверка timeout для перехода в HALF_OPEN
    bool shouldAttemptReset() {
        // Ваш код здесь
        return false;
    }
};
```

### Тесты
```cpp
void testCircuitBreakerStates() {
    CircuitBreakerConfig config{
        .failure_threshold = 3,
        .success_threshold = 2,
        .timeout = std::chrono::seconds(2)
    };
    
    CircuitBreaker cb("TestService", config);
    
    // Тест 1: Нормальная работа (CLOSED)
    assert(cb.getState() == CircuitState::CLOSED);
    
    // Выполняем успешные операции
    for (int i = 0; i < 5; ++i) {
        cb.execute([]() { return true; });
    }
    
    assert(cb.getState() == CircuitState::CLOSED);
    
    // Тест 2: Переход в OPEN
    for (int i = 0; i < 3; ++i) {
        try {
            cb.execute([]() -> bool { 
                throw std::runtime_error("Failure"); 
            });
        } catch (...) {}
    }
    
    assert(cb.getState() == CircuitState::OPEN);
    
    // Тест 3: Отклонение в OPEN
    try {
        cb.execute([]() { return true; });
        assert(false && "Should reject in OPEN state");
    } catch (const std::runtime_error& e) {
        std::cout << "Correctly rejected: " << e.what() << std::endl;
    }
    
    // Тест 4: Переход в HALF_OPEN после timeout
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    cb.execute([]() { return true; });
    assert(cb.getState() == CircuitState::HALF_OPEN);
    
    // Тест 5: Переход обратно в CLOSED
    cb.execute([]() { return true; });
    assert(cb.getState() == CircuitState::CLOSED);
    
    cb.printStats();
    std::cout << "Circuit Breaker test PASSED ✅" << std::endl;
}
```

---

## 📋 Задание 2: Circuit Breaker + Retry Logic

### Описание
Реализуйте интеграцию Circuit Breaker с Retry Pattern и exponential backoff.

### Требования
1. ✅ Retry с exponential backoff
2. ✅ Максимальное количество попыток
3. ✅ Jitter для предотвращения thundering herd
4. ✅ Интеграция с Circuit Breaker
5. ✅ Metrics: retry count, backoff delays

### Шаблон
```cpp
struct RetryPolicy {
    size_t max_attempts;
    std::chrono::milliseconds initial_delay;
    double backoff_multiplier;
    double jitter_factor;  // 0.1 = ±10% случайности
};

template<typename F>
auto retryWithBackoff(F&& func, const RetryPolicy& policy) -> decltype(func()) {
    std::chrono::milliseconds delay = policy.initial_delay;
    
    for (size_t attempt = 1; attempt <= policy.max_attempts; ++attempt) {
        try {
            return func();
        } catch (const std::exception& e) {
            if (attempt == policy.max_attempts) {
                throw;
            }
            
            // TODO: Добавьте jitter
            // TODO: Sleep с backoff
            // TODO: Увеличьте delay
        }
    }
}

// Комбинация с Circuit Breaker
template<typename F>
auto executeWithRetryAndCircuitBreaker(
    CircuitBreaker& cb, F&& func, const RetryPolicy& policy) {
    
    return cb.execute([&]() {
        return retryWithBackoff(std::forward<F>(func), policy);
    });
}
```

---

## 📋 Задание 3: Fallback Механизмы

### Описание
Реализуйте различные fallback стратегии для Circuit Breaker.

### Требования
1. ✅ Cache fallback
2. ✅ Default value fallback
3. ✅ Secondary service fallback
4. ✅ Degraded functionality fallback
5. ✅ Composable fallbacks (цепочка)

### Шаблон
```cpp
template<typename T>
class FallbackChain {
private:
    std::vector<std::function<std::optional<T>()>> fallbacks_;
    
public:
    void addFallback(std::function<std::optional<T>()> fallback) {
        fallbacks_.push_back(std::move(fallback));
    }
    
    T execute() {
        // TODO: Пробуйте каждый fallback по очереди
        for (auto& fallback : fallbacks_) {
            auto result = fallback();
            if (result.has_value()) {
                return result.value();
            }
        }
        
        throw std::runtime_error("All fallbacks failed");
    }
};

// Пример использования
std::string getUserData(const std::string& user_id) {
    FallbackChain<std::string> chain;
    
    // Primary: API call через Circuit Breaker
    chain.addFallback([&]() -> std::optional<std::string> {
        try {
            return cb.execute([&]() { return api.getUser(user_id); });
        } catch (...) {
            return std::nullopt;
        }
    });
    
    // Fallback 1: Cache
    chain.addFallback([&]() {
        return cache.get(user_id);
    });
    
    // Fallback 2: Default
    chain.addFallback([&]() {
        return std::optional<std::string>("Default User");
    });
    
    return chain.execute();
}
```

---

## 📋 Задание 4: Distributed Circuit Breaker

### Описание
Реализуйте Circuit Breaker с разделяемым состоянием через Redis или shared memory.

### Требования
1. ✅ Shared state между инстансами приложения
2. ✅ Atomic операции для state transitions
3. ✅ Pub/Sub для уведомлений о state changes
4. ✅ Consistency гарантии
5. ✅ Fault tolerance самого CB

---

## 📋 Задание 5: Circuit Breaker Metrics Dashboard

### Описание
Создайте систему сбора и визуализации метрик Circuit Breaker.

### Требования
1. ✅ Time-series метрики
2. ✅ Sliding window для расчета rates
3. ✅ Percentiles (p50, p95, p99)
4. ✅ Alerting на критичные состояния
5. ✅ Текстовый dashboard

### Пример метрик
```cpp
struct CircuitBreakerMetrics {
    std::atomic<size_t> total_calls;
    std::atomic<size_t> successful_calls;
    std::atomic<size_t> failed_calls;
    std::atomic<size_t> rejected_calls;
    
    // Sliding window для последних N секунд
    std::deque<std::pair<std::chrono::system_clock::time_point, bool>> recent_calls;
    
    double getSuccessRate(std::chrono::seconds window) const {
        // TODO: Вычислить success rate за последние N секунд
    }
    
    void recordCall(bool success) {
        // TODO: Записать вызов
    }
};
```

---

**Автор**: Senior C++ Developer  
**Дата**: 2025-10-07  
**Сложность**: ⭐⭐⭐⭐ (Продвинутый уровень)
