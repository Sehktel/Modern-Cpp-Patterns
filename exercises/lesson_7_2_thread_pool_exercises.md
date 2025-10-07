# 🏋️ Упражнения: Thread Pool Pattern

## 📋 Задание 1: Базовый Thread Pool с std::future

### Описание
Реализуйте базовый Thread Pool с поддержкой `std::future` для возврата результатов и обработки исключений.

### Требования
1. ✅ Фиксированное количество worker threads
2. ✅ Thread-safe очередь задач
3. ✅ Поддержка `std::future` для получения результатов
4. ✅ Graceful shutdown
5. ✅ Exception handling в задачах

### Шаблон для реализации
```cpp
#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <memory>

template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    
public:
    // TODO: Реализуйте push
    void push(T value) {
        // Ваш код здесь
    }
    
    // TODO: Реализуйте pop с ожиданием
    bool pop(T& value) {
        // Ваш код здесь
        return false;
    }
    
    // TODO: Реализуйте проверку пустоты
    bool empty() const {
        // Ваш код здесь
        return true;
    }
};

class ThreadPool {
private:
    std::vector<std::thread> workers_;
    ThreadSafeQueue<std::function<void()>> tasks_;
    std::atomic<bool> stop_{false};
    
public:
    // TODO: Реализуйте конструктор
    explicit ThreadPool(size_t num_threads) {
        // Ваш код здесь
    }
    
    // TODO: Реализуйте деструктор
    ~ThreadPool() {
        // Ваш код здесь
    }
    
    // TODO: Реализуйте enqueue с std::future
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type> {
        // Ваш код здесь
    }
    
private:
    // TODO: Реализуйте worker thread loop
    void workerThread() {
        // Ваш код здесь
    }
};
```

### Тесты
```cpp
void testBasicThreadPool() {
    std::cout << "\n=== Test 1: Basic Thread Pool ===" << std::endl;
    
    ThreadPool pool(4);
    
    // Тест 1: Простые задачи
    std::vector<std::future<int>> futures;
    for (int i = 0; i < 10; ++i) {
        futures.push_back(pool.enqueue([i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return i * i;
        }));
    }
    
    // Проверка результатов
    for (int i = 0; i < 10; ++i) {
        int result = futures[i].get();
        assert(result == i * i);
        std::cout << "Task " << i << " result: " << result << std::endl;
    }
    
    std::cout << "Test 1 PASSED ✅" << std::endl;
}

void testExceptionHandling() {
    std::cout << "\n=== Test 2: Exception Handling ===" << std::endl;
    
    ThreadPool pool(2);
    
    // Задача с исключением
    auto future = pool.enqueue([]() -> int {
        throw std::runtime_error("Test exception");
        return 42;
    });
    
    try {
        int result = future.get();
        assert(false && "Should throw exception");
    } catch (const std::runtime_error& e) {
        std::cout << "Exception caught: " << e.what() << std::endl;
        std::cout << "Test 2 PASSED ✅" << std::endl;
    }
}

int main() {
    testBasicThreadPool();
    testExceptionHandling();
    return 0;
}
```

### Решение
<details>
<summary>Показать решение</summary>

```cpp
template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_ = false;
    
public:
    void push(T value) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(value));
        }
        cv_.notify_one();
    }
    
    bool pop(T& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !queue_.empty() || stopped_; });
        
        if (stopped_ && queue_.empty()) {
            return false;
        }
        
        value = std::move(queue_.front());
        queue_.pop();
        return true;
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
    
    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopped_ = true;
        }
        cv_.notify_all();
    }
};

class ThreadPool {
private:
    std::vector<std::thread> workers_;
    ThreadSafeQueue<std::function<void()>> tasks_;
    std::atomic<bool> stop_{false};
    
public:
    explicit ThreadPool(size_t num_threads) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] { workerThread(); });
        }
    }
    
    ~ThreadPool() {
        stop_.store(true);
        tasks_.stop();
        
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
    
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type> {
        
        using return_type = typename std::result_of<F(Args...)>::type;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> result = task->get_future();
        
        tasks_.push([task]() { (*task)(); });
        
        return result;
    }
    
private:
    void workerThread() {
        while (!stop_.load()) {
            std::function<void()> task;
            
            if (tasks_.pop(task)) {
                try {
                    task();
                } catch (const std::exception& e) {
                    std::cerr << "Exception in task: " << e.what() << std::endl;
                }
            }
        }
    }
};
```

</details>

---

## 📋 Задание 2: Work Stealing Thread Pool

### Описание
Реализуйте продвинутый Thread Pool с алгоритмом Work Stealing для балансировки нагрузки.

### Требования
1. ✅ Локальные очереди для каждого worker thread
2. ✅ Work stealing от других потоков при простое
3. ✅ Глобальная очередь для новых задач
4. ✅ Метрики: steal count, utilization
5. ✅ Приоритетная очередь задач

### Шаблон для реализации
```cpp
enum class TaskPriority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    CRITICAL = 3
};

struct Task {
    std::function<void()> work;
    TaskPriority priority;
    
    // TODO: Реализуйте компаратор для приоритетной очереди
    bool operator<(const Task& other) const {
        // Ваш код здесь
        return false;
    }
};

class WorkStealingThreadPool {
private:
    std::vector<std::thread> workers_;
    std::vector<std::priority_queue<Task>> local_queues_;
    std::priority_queue<Task> global_queue_;
    
    std::vector<std::mutex> local_mutexes_;
    std::mutex global_mutex_;
    std::condition_variable cv_;
    
    std::atomic<bool> stop_{false};
    std::atomic<size_t> steal_count_{0};
    
public:
    // TODO: Реализуйте конструктор
    explicit WorkStealingThreadPool(size_t num_threads) {
        // Ваш код здесь
    }
    
    // TODO: Реализуйте добавление задачи
    void enqueue(std::function<void()> task, 
                TaskPriority priority = TaskPriority::NORMAL) {
        // Ваш код здесь
    }
    
private:
    // TODO: Реализуйте worker loop с work stealing
    void workerLoop(size_t worker_id) {
        // Ваш код здесь
    }
    
    // TODO: Реализуйте алгоритм кражи работы
    bool tryStealWork(size_t worker_id, Task& task) {
        // Ваш код здесь
        return false;
    }
};
```

### Тесты
```cpp
void testWorkStealing() {
    std::cout << "\n=== Test: Work Stealing ===" << std::endl;
    
    WorkStealingThreadPool pool(4);
    
    // Создаем дисбаланс нагрузки
    for (int i = 0; i < 100; ++i) {
        pool.enqueue([i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::cout << "Task " << i << " completed" << std::endl;
        }, TaskPriority::NORMAL);
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Проверяем что все задачи выполнены
    std::cout << "Work stealing test completed" << std::endl;
}
```

### Решение
<details>
<summary>Показать решение</summary>

```cpp
struct Task {
    std::function<void()> work;
    TaskPriority priority;
    std::chrono::system_clock::time_point created_at;
    
    Task(std::function<void()> w, TaskPriority p)
        : work(std::move(w)), priority(p),
          created_at(std::chrono::system_clock::now()) {}
    
    bool operator<(const Task& other) const {
        if (priority != other.priority) {
            return priority < other.priority;  // Обратный порядок для приоритета
        }
        return created_at > other.created_at;  // FIFO для одного приоритета
    }
};

class WorkStealingThreadPool {
private:
    std::vector<std::thread> workers_;
    std::vector<std::priority_queue<Task>> local_queues_;
    std::priority_queue<Task> global_queue_;
    
    std::vector<std::mutex> local_mutexes_;
    std::mutex global_mutex_;
    std::condition_variable cv_;
    
    std::atomic<bool> stop_{false};
    std::atomic<size_t> next_queue_{0};
    std::atomic<size_t> steal_count_{0};
    
public:
    explicit WorkStealingThreadPool(size_t num_threads) 
        : local_queues_(num_threads), local_mutexes_(num_threads) {
        
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this, i]() { workerLoop(i); });
        }
    }
    
    ~WorkStealingThreadPool() {
        stop_.store(true);
        cv_.notify_all();
        
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        
        std::cout << "Total work steals: " << steal_count_.load() << std::endl;
    }
    
    void enqueue(std::function<void()> task, TaskPriority priority = TaskPriority::NORMAL) {
        size_t queue_id = next_queue_.fetch_add(1) % local_queues_.size();
        
        {
            std::lock_guard<std::mutex> lock(local_mutexes_[queue_id]);
            local_queues_[queue_id].emplace(std::move(task), priority);
        }
        
        cv_.notify_one();
    }
    
private:
    void workerLoop(size_t worker_id) {
        while (!stop_.load()) {
            Task task([](){}, TaskPriority::NORMAL);
            bool has_task = false;
            
            // 1. Проверяем локальную очередь
            {
                std::lock_guard<std::mutex> lock(local_mutexes_[worker_id]);
                if (!local_queues_[worker_id].empty()) {
                    task = std::move(const_cast<Task&>(local_queues_[worker_id].top()));
                    local_queues_[worker_id].pop();
                    has_task = true;
                }
            }
            
            // 2. Если нет работы, пытаемся украсть
            if (!has_task) {
                has_task = tryStealWork(worker_id, task);
            }
            
            // 3. Если все еще нет, ждем
            if (!has_task) {
                std::unique_lock<std::mutex> lock(global_mutex_);
                cv_.wait_for(lock, std::chrono::milliseconds(100), [this] {
                    return stop_.load();
                });
                continue;
            }
            
            // Выполняем задачу
            if (has_task) {
                try {
                    task.work();
                } catch (const std::exception& e) {
                    std::cerr << "Task exception: " << e.what() << std::endl;
                }
            }
        }
    }
    
    bool tryStealWork(size_t worker_id, Task& task) {
        // Пытаемся украсть у случайного другого потока
        for (size_t i = 0; i < workers_.size(); ++i) {
            size_t victim_id = (worker_id + i + 1) % workers_.size();
            
            std::unique_lock<std::mutex> lock(local_mutexes_[victim_id], std::try_to_lock);
            if (lock.owns_lock() && !local_queues_[victim_id].empty()) {
                task = std::move(const_cast<Task&>(local_queues_[victim_id].top()));
                local_queues_[victim_id].pop();
                steal_count_.fetch_add(1);
                return true;
            }
        }
        
        return false;
    }
};
```

</details>

---

## 📋 Задание 3: Thread Pool с динамическим масштабированием

### Описание
Реализуйте Thread Pool с возможностью динамического добавления/удаления потоков в зависимости от нагрузки.

### Требования
1. ✅ Автоматическое scale up при высокой загрузке
2. ✅ Автоматическое scale down при низкой загрузке
3. ✅ Минимальное и максимальное количество потоков
4. ✅ Метрики утилизации
5. ✅ Thread-safe масштабирование

### Критерии масштабирования
```cpp
struct ScalingPolicy {
    size_t min_threads;
    size_t max_threads;
    double scale_up_threshold;   // 0.8 = 80% утилизация
    double scale_down_threshold; // 0.2 = 20% утилизация
    std::chrono::seconds check_interval;
};
```

### Шаблон для реализации
```cpp
class DynamicThreadPool {
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    
    ScalingPolicy policy_;
    std::atomic<bool> stop_{false};
    std::atomic<size_t> active_threads_{0};
    std::thread monitor_thread_;
    
public:
    // TODO: Реализуйте конструктор
    explicit DynamicThreadPool(const ScalingPolicy& policy) {
        // Ваш код здесь
    }
    
    // TODO: Реализуйте enqueue
    void enqueue(std::function<void()> task) {
        // Ваш код здесь
    }
    
private:
    // TODO: Реализуйте monitor thread для масштабирования
    void monitorLoop() {
        // Ваш код здесь
    }
    
    // TODO: Реализуйте scale up
    void scaleUp(size_t count) {
        // Ваш код здесь
    }
    
    // TODO: Реализуйте scale down
    void scaleDown(size_t count) {
        // Ваш код здесь
    }
    
    // TODO: Вычисление утилизации
    double getUtilization() const {
        // Ваш код здесь
        return 0.0;
    }
};
```

### Тесты
```cpp
void testDynamicScaling() {
    ScalingPolicy policy{
        .min_threads = 2,
        .max_threads = 8,
        .scale_up_threshold = 0.8,
        .scale_down_threshold = 0.2,
        .check_interval = std::chrono::seconds(1)
    };
    
    DynamicThreadPool pool(policy);
    
    // Создаем высокую нагрузку
    for (int i = 0; i < 100; ++i) {
        pool.enqueue([i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        });
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(3));
    // Должен scale up
    
    // Ждем завершения
    std::this_thread::sleep_for(std::chrono::seconds(10));
    // Должен scale down
}
```

---

## 📋 Задание 4: Thread Pool с приоритетами и deadline

### Описание
Реализуйте Thread Pool с поддержкой приоритетов задач и deadline (крайний срок выполнения).

### Требования
1. ✅ Приоритетная очередь задач
2. ✅ Deadline для каждой задачи
3. ✅ Автоматический skip просроченных задач
4. ✅ Метрики: missed deadlines, average latency
5. ✅ Сортировка по приоритету и deadline

### Шаблон
```cpp
struct PrioritizedTask {
    std::function<void()> work;
    TaskPriority priority;
    std::chrono::system_clock::time_point deadline;
    std::string name;
    
    // TODO: Компаратор учитывающий приоритет И deadline
    bool operator<(const PrioritizedTask& other) const {
        // Ваш код здесь
        return false;
    }
    
    bool isExpired() const {
        return std::chrono::system_clock::now() > deadline;
    }
};
```

---

## 📋 Задание 5: Benchmarking Thread Pool

### Описание
Создайте benchmark для сравнения производительности Thread Pool vs создание потоков на каждую задачу.

### Требования
1. ✅ Измерение времени выполнения
2. ✅ Сравнение memory footprint
3. ✅ Анализ context switching overhead
4. ✅ Графики результатов (текстовые)
5. ✅ Рекомендации по использованию

### Шаблон
```cpp
struct BenchmarkResult {
    size_t num_tasks;
    std::chrono::milliseconds execution_time;
    size_t peak_threads;
    size_t context_switches;  // Приблизительно
};

BenchmarkResult benchmarkWithoutPool(size_t num_tasks) {
    // TODO: Реализуйте бенчмарк без пула
}

BenchmarkResult benchmarkWithPool(size_t num_tasks, size_t pool_size) {
    // TODO: Реализуйте бенчмарк с пулом
}

void compareBenchmarks() {
    std::vector<size_t> task_counts = {10, 100, 1000, 10000};
    
    for (size_t count : task_counts) {
        auto without_pool = benchmarkWithoutPool(count);
        auto with_pool = benchmarkWithPool(count, 4);
        
        // TODO: Вывод сравнения
    }
}
```

---

## 🎓 Дополнительные задания

### Задание 6: Thread Pool с timeout
Добавьте поддержку timeout для задач - автоматическая отмена если задача выполняется слишком долго.

### Задание 7: Thread Pool с callback
Реализуйте систему callback'ов для событий: task_started, task_completed, task_failed.

### Задание 8: Thread Pool с cancellation
Добавьте возможность отмены задач через `std::future::cancel()` или cancellation token.

---

**Автор**: Senior C++ Developer  
**Дата**: 2025-10-07  
**Сложность**: ⭐⭐⭐⭐ (Продвинутый уровень)
