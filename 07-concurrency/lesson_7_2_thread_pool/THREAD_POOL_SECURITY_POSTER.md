# 🛡️ Security Poster: Thread Pool Pattern

## 🎯 Критические угрозы

```
🔴 КРИТИЧНЫЕ:
├─ Task Queue Overflow (CWE-770): Unbounded task submission
├─ Thread Starvation: All threads blocked indefinitely
├─ Resource Leak: Tasks hold resources without cleanup
└─ Race Conditions: Concurrent pool state access

🟡 ВЫСОКИЕ:
├─ Double Submit: Same task executed twice
├─ Exception Propagation: Uncaught task exceptions crash workers
└─ Shutdown Race: Tasks submitted during shutdown
```

## 💣 Ключевые уязвимости

### 1. Task Queue Overflow
```cpp
// ❌ DoS через queue flooding
ThreadPool pool(4);
while (true) {
    pool.submit([]{ heavy_task(); });  // 💥 Unbounded!
}

// ✅ БЕЗОПАСНО - Bounded + rejection
class BoundedThreadPool {
    const size_t MAX_QUEUE_SIZE = 10000;
    
public:
    bool submit(Task task, std::chrono::milliseconds timeout) {
        std::unique_lock lock(mutex_);
        
        if (!cv_.wait_for(lock, timeout, [this] {
            return queue_.size() < MAX_QUEUE_SIZE;
        })) {
            return false;  // ✅ Reject if full
        }
        
        queue_.push(std::move(task));
        cv_.notify_one();
        return true;
    }
};
```

### 2. Exception Safety
```cpp
// ❌ Uncaught exception crashes worker
void worker() {
    while (running_) {
        auto task = queue_.pop();
        task();  // 💥 Exception propagates → thread dies!
    }
}

// ✅ БЕЗОПАСНО - Catch all exceptions
void worker() {
    while (running_) {
        try {
            auto task = queue_.pop();
            task();
        } catch (const std::exception& e) {
            logger_.error("Task failed: " + std::string(e.what()));
        } catch (...) {
            logger_.error("Task failed: unknown exception");
        }
    }
}
```

### 3. Shutdown Race Condition
```cpp
// ❌ УЯЗВИМО - Tasks submitted during shutdown
void shutdown() {
    running_ = false;  // [1] Set flag
}

void submit(Task task) {
    if (running_) {  // [2] Check flag
        // RACE: shutdown() может выполниться здесь
        queue_.push(task);  // 💥 Lost task!
    }
}

// ✅ БЕЗОПАСНО - Atomic shutdown
void shutdown() {
    {
        std::lock_guard lock(mutex_);
        running_ = false;
    }
    cv_.notify_all();
    
    for (auto& thread : threads_) {
        thread.join();  // ✅ Wait for all
    }
}

void submit(Task task) {
    std::lock_guard lock(mutex_);
    if (!running_) {
        throw std::runtime_error("Pool shutdown");
    }
    queue_.push(std::move(task));
    cv_.notify_one();
}
```

## 🛠️ Инструменты
```bash
# Race/deadlock detection
g++ -fsanitize=thread -g thread_pool.cpp

# Load testing
./stress_test --threads 100 --tasks 100000

# Resource monitoring
watch -n 1 'ps aux | grep thread_pool'
```

## 📋 Checklist
```
[ ] Bounded task queue
[ ] Exception handling в workers
[ ] Graceful shutdown mechanism
[ ] Atomic state transitions
[ ] Resource cleanup on task completion
[ ] Thread-safe task submission
[ ] Monitoring (queue size, active threads)
[ ] Rejection policy для overload
```

## 💡 Золотое правило
```
"Thread pool - это shared resource.
 Bound the queue, catch ALL exceptions.
 Atomic shutdown, no lost tasks.
 Monitor queue size, reject на overload!"
```

---
**Версия**: 1.0 | **Дата**: 2025-10-11

