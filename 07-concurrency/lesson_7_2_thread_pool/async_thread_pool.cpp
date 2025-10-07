/**
 * @file async_thread_pool.cpp
 * @brief Асинхронный Thread Pool с современными C++ возможностями
 * 
 * Реализован асинхронный Thread Pool с поддержкой:
 * - std::async интеграция
 * - std::future/std::promise
 * - Work stealing
 * - Динамическое масштабирование
 */

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <atomic>
#include <chrono>
#include <memory>
#include <algorithm>
#include <random>

// Приоритеты задач
enum class TaskPriority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    CRITICAL = 3
};

// Структура задачи с приоритетом
struct Task {
    std::function<void()> function;
    TaskPriority priority;
    std::chrono::system_clock::time_point created_at;
    
    Task(std::function<void()> func, TaskPriority prio = TaskPriority::NORMAL)
        : function(std::move(func)), priority(prio), 
          created_at(std::chrono::system_clock::now()) {}
    
    // Компаратор для приоритетной очереди (высший приоритет первым)
    bool operator<(const Task& other) const {
        if (priority != other.priority) {
            return priority < other.priority; // Обратный порядок для приоритета
        }
        return created_at > other.created_at; // FIFO для одинакового приоритета
    }
};

// Статистика выполнения
struct ThreadPoolStats {
    std::atomic<size_t> tasks_completed{0};
    std::atomic<size_t> tasks_failed{0};
    std::atomic<size_t> tasks_pending{0};
    std::atomic<size_t> active_threads{0};
    std::chrono::system_clock::time_point start_time;
    
    ThreadPoolStats() : start_time(std::chrono::system_clock::now()) {}
    
    void printStats() const {
        auto now = std::chrono::system_clock::now();
        auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - start_time);
        
        std::cout << "\n=== Thread Pool Statistics ===" << std::endl;
        std::cout << "Uptime: " << uptime.count() << " seconds" << std::endl;
        std::cout << "Tasks completed: " << tasks_completed.load() << std::endl;
        std::cout << "Tasks failed: " << tasks_failed.load() << std::endl;
        std::cout << "Tasks pending: " << tasks_pending.load() << std::endl;
        std::cout << "Active threads: " << active_threads.load() << std::endl;
        
        if (tasks_completed.load() > 0) {
            double throughput = static_cast<double>(tasks_completed.load()) / uptime.count();
            std::cout << "Throughput: " << throughput << " tasks/second" << std::endl;
        }
        std::cout << "===============================" << std::endl;
    }
};

// Асинхронный Thread Pool с Work Stealing
class AsyncThreadPool {
private:
    std::vector<std::thread> workers_;
    std::vector<std::priority_queue<Task>> local_queues_; // Локальные очереди для work stealing
    std::priority_queue<Task> global_queue_; // Глобальная очередь
    std::mutex global_queue_mutex_;
    std::vector<std::mutex> local_queue_mutexes_;
    std::condition_variable condition_;
    std::atomic<bool> stop_{false};
    std::atomic<bool> shutdown_{false};
    ThreadPoolStats stats_;
    std::atomic<size_t> next_thread_{0};
    
public:
    explicit AsyncThreadPool(size_t num_threads = std::thread::hardware_concurrency()) 
        : local_queues_(num_threads), local_queue_mutexes_(num_threads) {
        
        // Создаем рабочие потоки
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this, i]() {
                workerLoop(i);
            });
        }
        
        std::cout << "Async Thread Pool создан с " << num_threads << " потоками" << std::endl;
    }
    
    ~AsyncThreadPool() {
        shutdown();
    }
    
    // Добавление задачи с возвратом future
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type> {
        
        using return_type = typename std::result_of<F(Args...)>::type;
        
        // Создаем packaged_task
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        // Получаем future
        std::future<return_type> result = task->get_future();
        
        // Выбираем случайный поток для локальной очереди
        size_t thread_id = next_thread_.fetch_add(1) % workers_.size();
        
        {
            std::unique_lock<std::mutex> lock(local_queue_mutexes_[thread_id]);
            
            if (shutdown_.load()) {
                throw std::runtime_error("Thread Pool остановлен");
            }
            
            // Добавляем задачу в локальную очередь
            local_queues_[thread_id].emplace([task]() { (*task)(); }, TaskPriority::NORMAL);
            stats_.tasks_pending.fetch_add(1);
        }
        
        // Уведомляем поток
        condition_.notify_one();
        
        return result;
    }
    
    // Добавление задачи с приоритетом
    template<typename F, typename... Args>
    auto enqueueWithPriority(TaskPriority priority, F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type> {
        
        using return_type = typename std::result_of<F(Args...)>::type;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> result = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(global_queue_mutex_);
            
            if (shutdown_.load()) {
                throw std::runtime_error("Thread Pool остановлен");
            }
            
            // Высокоприоритетные задачи идут в глобальную очередь
            global_queue_.emplace([task]() { (*task)(); }, priority);
            stats_.tasks_pending.fetch_add(1);
        }
        
        condition_.notify_all();
        return result;
    }
    
    // Graceful shutdown
    void shutdown() {
        if (shutdown_.load()) return;
        
        std::cout << "Начинаем graceful shutdown..." << std::endl;
        
        shutdown_.store(true);
        condition_.notify_all();
        
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        
        std::cout << "Async Thread Pool остановлен" << std::endl;
        stats_.printStats();
    }
    
    // Получение статистики
    const ThreadPoolStats& getStats() const {
        return stats_;
    }
    
    // Динамическое масштабирование (упрощенная версия)
    void scaleUp(size_t additional_threads) {
        std::cout << "Масштабирование вверх: добавляем " << additional_threads << " потоков" << std::endl;
        
        for (size_t i = 0; i < additional_threads; ++i) {
            size_t thread_id = workers_.size();
            local_queue_mutexes_.push_back(std::mutex{});
            local_queues_.push_back(std::priority_queue<Task>{});
            
            workers_.emplace_back([this, thread_id]() {
                workerLoop(thread_id);
            });
        }
    }
    
private:
    void workerLoop(size_t worker_id) {
        std::cout << "Worker " << worker_id << " запущен" << std::endl;
        
        while (!stop_.load()) {
            Task task([](){}); // Пустая задача по умолчанию
            bool has_task = false;
            
            // 1. Проверяем локальную очередь
            {
                std::unique_lock<std::mutex> lock(local_queue_mutexes_[worker_id]);
                if (!local_queues_[worker_id].empty()) {
                    task = std::move(const_cast<Task&>(local_queues_[worker_id].top()));
                    local_queues_[worker_id].pop();
                    has_task = true;
                    stats_.tasks_pending.fetch_sub(1);
                    stats_.active_threads.fetch_add(1);
                }
            }
            
            // 2. Если локальная очередь пуста, проверяем глобальную
            if (!has_task) {
                std::unique_lock<std::mutex> lock(global_queue_mutex_);
                if (!global_queue_.empty()) {
                    task = std::move(const_cast<Task&>(global_queue_.top()));
                    global_queue_.pop();
                    has_task = true;
                    stats_.tasks_pending.fetch_sub(1);
                    stats_.active_threads.fetch_add(1);
                }
            }
            
            // 3. Work Stealing: если все очереди пусты, пытаемся украсть работу
            if (!has_task) {
                has_task = tryStealWork(worker_id, task);
            }
            
            // 4. Если все еще нет работы, ждем
            if (!has_task) {
                std::unique_lock<std::mutex> lock(global_queue_mutex_);
                condition_.wait(lock, [this] {
                    return !global_queue_.empty() || stop_.load() || shutdown_.load();
                });
                continue;
            }
            
            // Выполняем задачу
            if (has_task) {
                try {
                    task.function();
                    stats_.tasks_completed.fetch_add(1);
                } catch (const std::exception& e) {
                    std::cerr << "Ошибка в задаче: " << e.what() << std::endl;
                    stats_.tasks_failed.fetch_add(1);
                }
                
                stats_.active_threads.fetch_sub(1);
            }
            
            if (shutdown_.load() && !has_task) {
                break;
            }
        }
        
        std::cout << "Worker " << worker_id << " завершен" << std::endl;
    }
    
    // Work Stealing: попытка украсть работу у других потоков
    bool tryStealWork(size_t worker_id, Task& task) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, workers_.size() - 1);
        
        // Пытаемся украсть работу у случайного потока
        for (int attempt = 0; attempt < 3; ++attempt) {
            size_t victim_id = dis(gen);
            if (victim_id == worker_id) continue;
            
            std::unique_lock<std::mutex> lock(local_queue_mutexes_[victim_id], std::try_to_lock);
            if (lock.owns_lock() && !local_queues_[victim_id].empty()) {
                // Берем половину задач из жертвы
                size_t steal_count = local_queues_[victim_id].size() / 2;
                if (steal_count == 0) steal_count = 1;
                
                std::vector<Task> stolen_tasks;
                for (size_t i = 0; i < steal_count && !local_queues_[victim_id].empty(); ++i) {
                    stolen_tasks.push_back(std::move(const_cast<Task&>(local_queues_[victim_id].top())));
                    local_queues_[victim_id].pop();
                }
                
                // Берем первую задачу для выполнения
                if (!stolen_tasks.empty()) {
                    task = std::move(stolen_tasks[0]);
                    stats_.tasks_pending.fetch_sub(1);
                    stats_.active_threads.fetch_add(1);
                    
                    // Остальные задачи добавляем в свою локальную очередь
                    std::unique_lock<std::mutex> my_lock(local_queue_mutexes_[worker_id]);
                    for (size_t i = 1; i < stolen_tasks.size(); ++i) {
                        local_queues_[worker_id].push(std::move(stolen_tasks[i]));
                    }
                    
                    return true;
                }
            }
        }
        
        return false;
    }
};

// Примеры использования
void demonstrateAsyncThreadPool() {
    std::cout << "\n=== Демонстрация Async Thread Pool ===" << std::endl;
    
    AsyncThreadPool pool(4);
    
    // Простые задачи
    std::vector<std::future<int>> futures;
    
    for (int i = 0; i < 10; ++i) {
        auto future = pool.enqueue([i]() -> int {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << "Задача " << i << " выполнена в потоке " 
                      << std::this_thread::get_id() << std::endl;
            return i * i;
        });
        futures.push_back(std::move(future));
    }
    
    // Задачи с приоритетами
    auto high_priority_future = pool.enqueueWithPriority(TaskPriority::HIGH, []() -> int {
        std::cout << "ВЫСОКОПРИОРИТЕТНАЯ задача выполнена!" << std::endl;
        return 999;
    });
    
    auto low_priority_future = pool.enqueueWithPriority(TaskPriority::LOW, []() -> int {
        std::cout << "НИЗКОПРИОРИТЕТНАЯ задача выполнена!" << std::endl;
        return 1;
    });
    
    // Ждем завершения всех задач
    std::cout << "\nОжидание завершения задач..." << std::endl;
    
    for (auto& future : futures) {
        try {
            int result = future.get();
            std::cout << "Результат: " << result << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Ошибка: " << e.what() << std::endl;
        }
    }
    
    std::cout << "Высокоприоритетный результат: " << high_priority_future.get() << std::endl;
    std::cout << "Низкоприоритетный результат: " << low_priority_future.get() << std::endl;
    
    pool.getStats().printStats();
    pool.shutdown();
}

// Демонстрация Work Stealing
void demonstrateWorkStealing() {
    std::cout << "\n=== Демонстрация Work Stealing ===" << std::endl;
    
    AsyncThreadPool pool(2);
    
    // Создаем много задач для демонстрации work stealing
    std::vector<std::future<int>> futures;
    
    for (int i = 0; i < 20; ++i) {
        auto future = pool.enqueue([i]() -> int {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            return i;
        });
        futures.push_back(std::move(future));
    }
    
    // Ждем завершения
    int sum = 0;
    for (auto& future : futures) {
        sum += future.get();
    }
    
    std::cout << "Сумма результатов: " << sum << std::endl;
    pool.getStats().printStats();
    pool.shutdown();
}

// Демонстрация динамического масштабирования
void demonstrateScaling() {
    std::cout << "\n=== Демонстрация динамического масштабирования ===" << std::endl;
    
    AsyncThreadPool pool(2);
    
    // Добавляем задачи
    std::vector<std::future<int>> futures;
    for (int i = 0; i < 10; ++i) {
        auto future = pool.enqueue([i]() -> int {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            return i;
        });
        futures.push_back(std::move(future));
    }
    
    // Масштабируем вверх
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pool.scaleUp(2);
    
    // Добавляем еще задач
    for (int i = 10; i < 20; ++i) {
        auto future = pool.enqueue([i]() -> int {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            return i;
        });
        futures.push_back(std::move(future));
    }
    
    // Ждем завершения
    int sum = 0;
    for (auto& future : futures) {
        sum += future.get();
    }
    
    std::cout << "Сумма результатов: " << sum << std::endl;
    pool.getStats().printStats();
    pool.shutdown();
}

int main() {
    std::cout << "=== Async Thread Pool Pattern ===" << std::endl;
    
    try {
        demonstrateAsyncThreadPool();
        demonstrateWorkStealing();
        demonstrateScaling();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Демонстрация завершена ===" << std::endl;
    return 0;
}
