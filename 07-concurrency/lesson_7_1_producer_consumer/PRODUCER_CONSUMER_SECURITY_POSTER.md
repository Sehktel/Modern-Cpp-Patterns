# 🛡️ Security Poster: Producer-Consumer Pattern

## 🎯 Критические угрозы

```
🔴 КРИТИЧНЫЕ:
├─ Race Conditions (CWE-362): Concurrent queue access без синхронизации
├─ Deadlock (CWE-833): Circular wait на queue empty/full conditions
├─ Buffer Overflow (CWE-120): Unbounded queue growth → memory exhaustion
└─ Data Corruption: Lost updates, torn reads/writes

🟡 ВЫСОКИЕ:
├─ Starvation: Producers/consumers blocked indefinitely  
├─ Priority Inversion: High-priority blocked by low-priority
└─ Thundering Herd: All consumers wake on notify_all
```

## 💣 Ключевые уязвимости

### 1. Race Condition на Queue
```cpp
// ❌ УЯЗВИМО - Нет синхронизации
std::queue<Task> tasks;

void producer() {
    tasks.push(task);  // 💥 Data race!
}

void consumer() {
    auto task = tasks.front();  // 💥 Data race!
    tasks.pop();
}

// ✅ БЕЗОПАСНО - Mutex protection
std::queue<Task> tasks;
std::mutex mtx;
std::condition_variable cv;

void producer() {
    std::lock_guard lock(mtx);
    tasks.push(task);
    cv.notify_one();
}

void consumer() {
    std::unique_lock lock(mtx);
    cv.wait(lock, []{ return !tasks.empty(); });
    auto task = tasks.front();
    tasks.pop();
}
```

### 2. Deadlock - Producer/Consumer
```cpp
// ❌ DEADLOCK
std::mutex mutex_a, mutex_b;

void producer() {
    std::lock_guard l1(mutex_a);  // Locks A
    std::lock_guard l2(mutex_b);  // Wants B
}

void consumer() {
    std::lock_guard l2(mutex_b);  // Locks B
    std::lock_guard l1(mutex_a);  // Wants A → DEADLOCK!
}

// ✅ БЕЗОПАСНО - Единый lock или порядок
std::mutex mtx;  // Single mutex

// ИЛИ std::lock для atomic multi-lock
std::lock(mutex_a, mutex_b);
std::lock_guard l1(mutex_a, std::adopt_lock);
std::lock_guard l2(mutex_b, std::adopt_lock);
```

### 3. Buffer Overflow - Unbounded Queue
```cpp
// ❌ DoS через memory exhaustion
while (true) {
    queue.push(generate_large_task());  // 💥 Unbounded!
}
// → OOM

// ✅ БЕЗОПАСНО - Bounded queue
class BoundedQueue {
    std::queue<Task> queue_;
    std::mutex mtx_;
    std::condition_variable cv_producer_, cv_consumer_;
    const size_t MAX_SIZE = 1000;
    
public:
    void push(Task task) {
        std::unique_lock lock(mtx_);
        cv_producer_.wait(lock, [this] { 
            return queue_.size() < MAX_SIZE;  // ✅ Backpressure
        });
        queue_.push(std::move(task));
        cv_consumer_.notify_one();
    }
};
```

## 🛠️ Инструменты
```bash
# Race detection
g++ -fsanitize=thread -g producer_consumer.cpp

# Deadlock detection
valgrind --tool=helgrind ./program

# Performance profiling
perf record -e cycles ./program
```

## 📋 Checklist
```
[ ] Mutex protection для shared queue
[ ] Condition variables для coordination
[ ] Bounded queue (backpressure)
[ ] Deadlock-free lock ordering
[ ] ThreadSanitizer testing
[ ] Graceful shutdown mechanism
[ ] Producer/consumer count monitoring
```

## 💡 Золотое правило
```
"Producer-Consumer - это communication channel.
 ALWAYS use mutex + condition_variable.
 ALWAYS bound the queue.
 NEVER lock in opposite order!"
```

---
**Версия**: 1.0 | **Дата**: 2025-10-11

