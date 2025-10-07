# Урок 8.3: Bulkhead Pattern (Переборка)

## 🎯 Цель урока
Изучить паттерн Bulkhead - стратегию изоляции ресурсов для предотвращения каскадных отказов. Понять, как защитить критичные части системы от перегрузки и отказов некритичных компонентов.

## 📚 Что изучаем

### 1. Паттерн Bulkhead
- **Определение**: Изоляция ресурсов в независимые пулы для предотвращения распространения отказов
- **Назначение**: Fault isolation, graceful degradation, защита критичных сервисов
- **Применение**: Microservices, высоконагруженные системы, multi-tenant приложения

### 2. Ключевые компоненты
- **Thread Pool Bulkhead**: Изолированные пулы потоков
- **Connection Pool Bulkhead**: Изолированные пулы соединений
- **Memory Bulkhead**: Лимиты памяти на сервис
- **Semaphore Bulkhead**: Лимиты конкурентного доступа

### 3. Аналогия с кораблем
- **Без Bulkheads**: Пробоина → вся вода → корабль тонет
- **С Bulkheads**: Пробоина → затопление одного отсека → корабль плывет

## 🔍 Ключевые концепции

### Thread Pool Bulkhead

```cpp
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

enum class ServiceType {
    CRITICAL,    // Критичные операции (платежи)
    NORMAL,      // Обычные операции (поиск)
    BATCH        // Фоновые операции (аналитика)
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
    
    std::atomic<size_t> tasks_processed_{0};
    std::atomic<size_t> tasks_rejected_{0};
    
public:
    ThreadPoolBulkhead(const std::string& name, ServiceType type,
                      size_t num_threads, size_t max_queue_size)
        : name_(name), type_(type), 
          num_threads_(num_threads), max_queue_size_(max_queue_size) {
        
        // Создаем worker threads
        for (size_t i = 0; i < num_threads_; ++i) {
            workers_.emplace_back([this]() { workerLoop(); });
        }
    }
    
    ~ThreadPoolBulkhead() {
        stop_.store(true);
        cv_.notify_all();
        
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
    
    bool enqueue(std::function<void()> task) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Защита от переполнения
        if (tasks_.size() >= max_queue_size_) {
            tasks_rejected_.fetch_add(1);
            return false;
        }
        
        tasks_.push(std::move(task));
        lock.unlock();
        cv_.notify_one();
        
        return true;
    }
    
private:
    void workerLoop() {
        while (!stop_.load()) {
            std::function<void()> task;
            
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this] {
                    return !tasks_.empty() || stop_.load();
                });
                
                if (stop_.load() && tasks_.empty()) break;
                
                if (!tasks_.empty()) {
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
            }
            
            if (task) {
                try {
                    task();
                    tasks_processed_.fetch_add(1);
                } catch (const std::exception& e) {
                    std::cerr << "Task error: " << e.what() << std::endl;
                }
            }
        }
    }
};
```

### Bulkhead Manager

```cpp
class BulkheadManager {
private:
    std::unordered_map<ServiceType, std::shared_ptr<ThreadPoolBulkhead>> bulkheads_;
    
public:
    void registerBulkhead(ServiceType type, const std::string& name,
                         size_t threads, size_t queue_size) {
        auto bulkhead = std::make_shared<ThreadPoolBulkhead>(
            name, type, threads, queue_size);
        bulkheads_[type] = bulkhead;
    }
    
    bool execute(ServiceType type, std::function<void()> task) {
        auto it = bulkheads_.find(type);
        if (it != bulkheads_.end()) {
            return it->second->enqueue(std::move(task));
        }
        return false;
    }
};

// Использование
BulkheadManager manager;

// Настраиваем изоляцию
manager.registerBulkhead(ServiceType::CRITICAL, "Payment", 8, 20);
manager.registerBulkhead(ServiceType::NORMAL, "Search", 4, 10);
manager.registerBulkhead(ServiceType::BATCH, "Analytics", 2, 100);

// Медленный analytics не блокирует критичные платежи
manager.execute(ServiceType::CRITICAL, []() { processPayment(); });
manager.execute(ServiceType::BATCH, []() { analyzeData(); });
```

### Resource Isolation

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
    
public:
    bool allocateMemory(size_t bytes) {
        size_t current = memory_used_.load();
        if (current + bytes > limits_.max_memory_bytes) {
            return false;  // Превышен лимит
        }
        memory_used_.fetch_add(bytes);
        return true;
    }
    
    void releaseMemory(size_t bytes) {
        memory_used_.fetch_sub(bytes);
    }
    
    // Аналогично для других ресурсов
};
```

## 🎓 Best Practices

### ✅ DO (Рекомендуется)

1. **Изолируйте критичные от некритичных сервисов**
   - Платежи должны иметь свой dedicated bulkhead
   - Не смешивайте user-facing и batch операции

2. **Используйте bounded queues**
   - Защита от memory exhaustion
   - Fast fail лучше чем медленный ответ

3. **Мониторьте утилизацию**
   - Alerts на saturation > 90%
   - Track rejected tasks

4. **Правильно sizing bulkheads**
   - Load testing для определения размеров
   - Критичные сервисы: больше ресурсов

### ❌ DON'T (Не рекомендуется)

1. **НЕ создавайте слишком много bulkheads**
   - Overhead управления
   - Resource waste

2. **НЕ используйте unbounded queues**
   - Может привести к OOM

3. **НЕ игнорируйте rejected tasks**
   - Логируйте и мониторьте

## 📊 Метрики

### Ключевые метрики
- **Utilization**: % занятости bulkhead
- **Queue depth**: размер очереди
- **Rejection rate**: % отклоненных задач
- **Throughput**: задач/сек
- **Active threads**: активных потоков

### Alerts
⚠️ Utilization > 90% for > 5 min  
⚠️ Rejection rate > 1%  
⚠️ Queue depth > 80% capacity

## 📁 Файлы урока

- `bulkhead_pattern.cpp` - Thread Pool и Connection Pool Bulkheads
- `resource_isolation.cpp` - Изоляция CPU/памяти/соединений
- `bulkhead_vulnerabilities.cpp` - Уязвимости паттерна
- `secure_bulkhead_alternatives.cpp` - Безопасные альтернативы
- `SECURITY_ANALYSIS.md` - Анализ безопасности

## 🔗 Связанные паттерны

- **Circuit Breaker**: Fast fail + Isolation = двойная защита
- **Rate Limiting**: Ограничение входящего трафика
- **Timeout**: Освобождение ресурсов
- **Retry**: Осторожность с retry + bulkhead

## 🚀 Практическое применение

### Sizing Guidelines

**Thread Pool Size:**
- CPU-bound: `num_cores + 1`
- I/O-bound: `num_cores * (1 + wait_time/compute_time)`
- Смешанные: начать с `num_cores * 2`

**Queue Size:**
- Минимум: `thread_pool_size`
- Максимум: не более 100-200 (memory overhead)
- Критичные: меньше (fast fail)
- Batch: больше (накопление)

### Примеры конфигурации

```cpp
// Payment Service (критичный)
manager.registerBulkhead(ServiceType::CRITICAL, "Payment", 8, 16);

// Search Service (обычный)  
manager.registerBulkhead(ServiceType::NORMAL, "Search", 4, 10);

// Analytics (фоновый)
manager.registerBulkhead(ServiceType::BATCH, "Analytics", 2, 100);
```

---

**Автор**: Senior C++ Developer  
**Дата**: 2025-10-07  
**Версия**: 1.0
