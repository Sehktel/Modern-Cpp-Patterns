# 🏋️ Упражнения: Bulkhead Pattern

## 📋 Задание 1: Thread Pool Bulkhead

### Описание
Реализуйте изолированные Thread Pools для разных типов сервисов с защитой от resource exhaustion.

### Требования
1. ✅ Отдельные Thread Pools для CRITICAL/NORMAL/BATCH
2. ✅ Bounded queues для каждого пула
3. ✅ Rejection policy при переполнении
4. ✅ Метрики утилизации каждого bulkhead
5. ✅ Graceful degradation

### Шаблон для реализации
```cpp
#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>

enum class ServiceType {
    CRITICAL,
    NORMAL,
    BATCH
};

class ThreadPoolBulkhead {
private:
    std::string name_;
    ServiceType type_;
    size_t num_threads_;
    size_t max_queue_size_;
    
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};
    
    // Метрики
    std::atomic<size_t> tasks_processed_{0};
    std::atomic<size_t> tasks_rejected_{0};
    
public:
    // TODO: Реализуйте конструктор
    ThreadPoolBulkhead(const std::string& name, ServiceType type,
                      size_t num_threads, size_t max_queue_size) {
        // Ваш код здесь
    }
    
    // TODO: Реализуйте enqueue с rejection
    bool enqueue(std::function<void()> task) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // TODO: Проверьте переполнение очереди
        if (tasks_.size() >= max_queue_size_) {
            tasks_rejected_.fetch_add(1);
            return false;
        }
        
        // TODO: Добавьте задачу
        return true;
    }
    
    void printStats() const {
        // TODO: Вывод метрик
    }
    
private:
    void workerThread() {
        // TODO: Worker loop
    }
};

class BulkheadManager {
private:
    std::unordered_map<ServiceType, std::shared_ptr<ThreadPoolBulkhead>> bulkheads_;
    
public:
    // TODO: Регистрация bulkhead
    void registerBulkhead(ServiceType type, const std::string& name,
                         size_t threads, size_t queue_size) {
        // Ваш код здесь
    }
    
    // TODO: Выполнение через соответствующий bulkhead
    bool execute(ServiceType type, std::function<void()> task) {
        // Ваш код здесь
        return false;
    }
};
```

### Тесты
```cpp
void testBulkheadIsolation() {
    BulkheadManager manager;
    
    // Настраиваем bulkheads
    manager.registerBulkhead(ServiceType::CRITICAL, "Critical", 4, 10);
    manager.registerBulkhead(ServiceType::NORMAL, "Normal", 2, 5);
    manager.registerBulkhead(ServiceType::BATCH, "Batch", 1, 20);
    
    // Перегружаем BATCH bulkhead
    for (int i = 0; i < 50; ++i) {
        manager.execute(ServiceType::BATCH, [i]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        });
    }
    
    // CRITICAL и NORMAL должны работать нормально
    bool critical_ok = manager.execute(ServiceType::CRITICAL, []() {
        std::cout << "Critical task executed" << std::endl;
    });
    
    assert(critical_ok && "Critical bulkhead should be available");
    std::cout << "Bulkhead isolation test PASSED ✅" << std::endl;
}
```

---

## 📋 Задание 2: Semaphore Bulkhead

### Описание
Реализуйте легковесный Bulkhead используя семафоры для ограничения конкурентного доступа.

### Требования
1. ✅ Counting semaphore для лимита
2. ✅ Timeout для acquire
3. ✅ RAII guard для auto-release
4. ✅ Метрики: acquisitions, rejections, wait times
5. ✅ Thread-safe

### Шаблон
```cpp
class SemaphoreBulkhead {
private:
    size_t limit_;
    std::atomic<size_t> available_{0};
    std::mutex mutex_;
    std::condition_variable cv_;
    
public:
    explicit SemaphoreBulkhead(size_t limit) : limit_(limit) {
        available_.store(limit);
    }
    
    // TODO: Реализуйте acquire с timeout
    bool acquire(std::chrono::milliseconds timeout) {
        // Ваш код здесь
        return false;
    }
    
    // TODO: Реализуйте release
    void release() {
        // Ваш код здесь
    }
    
    // TODO: RAII Guard
    class Guard {
    private:
        SemaphoreBulkhead* bulkhead_;
        bool acquired_;
        
    public:
        Guard(SemaphoreBulkhead* bh, bool acq) 
            : bulkhead_(bh), acquired_(acq) {}
        
        ~Guard() {
            if (acquired_ && bulkhead_) {
                bulkhead_->release();
            }
        }
    };
    
    Guard acquireGuard(std::chrono::milliseconds timeout) {
        bool acquired = acquire(timeout);
        return Guard(this, acquired);
    }
};
```

---

## 📋 Задание 3: Resource Isolation Manager

### Описание
Реализуйте менеджер для изоляции различных типов ресурсов: CPU, память, соединения, файлы.

### Требования
1. ✅ Лимиты для разных типов ресурсов
2. ✅ Tracking текущего использования
3. ✅ Soft/hard limits
4. ✅ Alerts при приближении к лимитам
5. ✅ RAII guards для ресурсов

### Шаблон
```cpp
struct ResourceLimits {
    size_t max_memory_bytes;
    size_t max_threads;
    size_t max_connections;
    size_t max_file_descriptors;
};

class ResourceTracker {
private:
    ResourceLimits limits_;
    std::atomic<size_t> memory_used_{0};
    std::atomic<size_t> threads_active_{0};
    std::atomic<size_t> connections_active_{0};
    std::atomic<size_t> fds_used_{0};
    
public:
    // TODO: Реализуйте аллокацию ресурсов с проверкой лимитов
    bool allocateMemory(size_t bytes) {
        // Ваш код здесь
        return false;
    }
    
    void releaseMemory(size_t bytes) {
        // Ваш код здесь
    }
    
    // TODO: Аналогично для других ресурсов
};
```

---

**Автор**: Senior C++ Developer  
**Дата**: 2025-10-07  
**Сложность**: ⭐⭐⭐⭐ (Продвинутый уровень)
