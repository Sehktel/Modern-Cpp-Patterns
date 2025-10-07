/**
 * @file bulkhead_pattern.cpp
 * @brief Демонстрация Bulkhead Pattern
 * 
 * Реализован Bulkhead Pattern с поддержкой:
 * - Изоляция пулов соединений
 * - Изоляция thread pools
 * - Изоляция ресурсов памяти
 * - Мониторинг изолированных компонентов
 */

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <chrono>
#include <functional>
#include <unordered_map>

// Тип сервиса (для изоляции)
enum class ServiceType {
    CRITICAL,    // Критические сервисы (высокий приоритет)
    NORMAL,      // Обычные сервисы
    BATCH        // Фоновые/batch задачи
};

std::string serviceTypeToString(ServiceType type) {
    switch (type) {
        case ServiceType::CRITICAL: return "CRITICAL";
        case ServiceType::NORMAL: return "NORMAL";
        case ServiceType::BATCH: return "BATCH";
        default: return "UNKNOWN";
    }
}

// Задача для выполнения
struct Task {
    std::function<void()> work;
    ServiceType service_type;
    std::string description;
    
    Task(std::function<void()> w, ServiceType type, const std::string& desc)
        : work(std::move(w)), service_type(type), description(desc) {}
};

// Изолированный Thread Pool (Bulkhead)
class ThreadPoolBulkhead {
private:
    std::string name_;
    ServiceType service_type_;
    size_t num_threads_;
    size_t max_queue_size_;
    
    std::vector<std::thread> workers_;
    std::queue<Task> task_queue_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_{false};
    
    // Статистика
    std::atomic<size_t> tasks_processed_{0};
    std::atomic<size_t> tasks_queued_{0};
    std::atomic<size_t> tasks_rejected_{0};
    std::atomic<size_t> active_threads_{0};
    
public:
    ThreadPoolBulkhead(const std::string& name, 
                      ServiceType type,
                      size_t num_threads, 
                      size_t max_queue_size)
        : name_(name), 
          service_type_(type),
          num_threads_(num_threads),
          max_queue_size_(max_queue_size) {
        
        // Создаем рабочие потоки
        for (size_t i = 0; i < num_threads_; ++i) {
            workers_.emplace_back([this, i]() {
                workerThread(i);
            });
        }
        
        std::cout << "ThreadPool Bulkhead '" << name_ << "' создан ("
                  << serviceTypeToString(service_type_) << ", потоки: " 
                  << num_threads_ << ", макс. очередь: " << max_queue_size_ << ")" << std::endl;
    }
    
    ~ThreadPoolBulkhead() {
        shutdown();
    }
    
    // Добавление задачи
    bool enqueue(Task task) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        
        // Проверяем переполнение очереди (защита от DoS)
        if (task_queue_.size() >= max_queue_size_) {
            tasks_rejected_.fetch_add(1);
            std::cerr << "[" << name_ << "] Очередь переполнена, задача отклонена: " 
                      << task.description << std::endl;
            return false;
        }
        
        task_queue_.push(std::move(task));
        tasks_queued_.fetch_add(1);
        
        lock.unlock();
        condition_.notify_one();
        
        return true;
    }
    
    // Завершение работы
    void shutdown() {
        if (stop_.load()) return;
        
        std::cout << "[" << name_ << "] Остановка Thread Pool..." << std::endl;
        
        stop_.store(true);
        condition_.notify_all();
        
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        
        std::cout << "[" << name_ << "] Thread Pool остановлен" << std::endl;
    }
    
    // Статистика
    void printStats() const {
        std::cout << "\n=== Bulkhead '" << name_ << "' Statistics ===" << std::endl;
        std::cout << "Тип сервиса: " << serviceTypeToString(service_type_) << std::endl;
        std::cout << "Количество потоков: " << num_threads_ << std::endl;
        std::cout << "Макс. размер очереди: " << max_queue_size_ << std::endl;
        std::cout << "Задач обработано: " << tasks_processed_.load() << std::endl;
        std::cout << "Задач в очереди: " << tasks_queued_.load() << std::endl;
        std::cout << "Задач отклонено: " << tasks_rejected_.load() << std::endl;
        std::cout << "Активных потоков: " << active_threads_.load() << std::endl;
        std::cout << "==========================================" << std::endl;
    }
    
    size_t getActiveThreads() const {
        return active_threads_.load();
    }
    
    size_t getQueueSize() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return task_queue_.size();
    }
    
private:
    void workerThread(size_t thread_id) {
        std::cout << "[" << name_ << "] Worker " << thread_id << " запущен" << std::endl;
        
        while (!stop_.load()) {
            Task task([](){}, ServiceType::NORMAL, "");
            bool has_task = false;
            
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                
                condition_.wait(lock, [this] {
                    return !task_queue_.empty() || stop_.load();
                });
                
                if (stop_.load() && task_queue_.empty()) {
                    break;
                }
                
                if (!task_queue_.empty()) {
                    task = std::move(task_queue_.front());
                    task_queue_.pop();
                    has_task = true;
                }
            }
            
            if (has_task) {
                active_threads_.fetch_add(1);
                
                try {
                    std::cout << "[" << name_ << "] Выполняется: " << task.description << std::endl;
                    task.work();
                    tasks_processed_.fetch_add(1);
                } catch (const std::exception& e) {
                    std::cerr << "[" << name_ << "] Ошибка в задаче: " << e.what() << std::endl;
                }
                
                active_threads_.fetch_sub(1);
            }
        }
        
        std::cout << "[" << name_ << "] Worker " << thread_id << " завершен" << std::endl;
    }
};

// Менеджер Bulkheads для управления несколькими изолированными пулами
class BulkheadManager {
private:
    std::unordered_map<ServiceType, std::shared_ptr<ThreadPoolBulkhead>> bulkheads_;
    std::mutex mutex_;
    
public:
    BulkheadManager() {
        std::cout << "Bulkhead Manager создан" << std::endl;
    }
    
    // Регистрация bulkhead для типа сервиса
    void registerBulkhead(ServiceType type, 
                         const std::string& name,
                         size_t num_threads, 
                         size_t max_queue_size) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto bulkhead = std::make_shared<ThreadPoolBulkhead>(
            name, type, num_threads, max_queue_size);
        
        bulkheads_[type] = bulkhead;
    }
    
    // Выполнение задачи через соответствующий bulkhead
    bool execute(ServiceType type, Task task) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = bulkheads_.find(type);
        if (it != bulkheads_.end()) {
            return it->second->enqueue(std::move(task));
        }
        
        std::cerr << "Bulkhead для типа " << serviceTypeToString(type) 
                  << " не найден" << std::endl;
        return false;
    }
    
    // Статистика всех bulkheads
    void printAllStats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::cout << "\n========== Bulkhead Manager Statistics ==========" << std::endl;
        std::cout << "Всего Bulkheads: " << bulkheads_.size() << std::endl;
        
        for (const auto& pair : bulkheads_) {
            pair.second->printStats();
        }
        
        std::cout << "==================================================" << std::endl;
    }
    
    // Завершение всех bulkheads
    void shutdownAll() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (auto& pair : bulkheads_) {
            pair.second->shutdown();
        }
    }
};

// Пул соединений с Bulkhead изоляцией
class ConnectionPoolBulkhead {
private:
    std::string name_;
    size_t max_connections_;
    std::vector<int> available_connections_;  // Имитация соединений
    std::queue<int> available_queue_;
    std::mutex mutex_;
    std::condition_variable condition_;
    
    std::atomic<size_t> active_connections_{0};
    std::atomic<size_t> connection_requests_{0};
    std::atomic<size_t> connection_timeouts_{0};
    
public:
    ConnectionPoolBulkhead(const std::string& name, size_t max_connections)
        : name_(name), max_connections_(max_connections) {
        
        // Инициализируем пул соединений
        for (size_t i = 0; i < max_connections_; ++i) {
            available_connections_.push_back(static_cast<int>(i));
            available_queue_.push(static_cast<int>(i));
        }
        
        std::cout << "Connection Pool Bulkhead '" << name_ << "' создан (макс. соединений: " 
                  << max_connections_ << ")" << std::endl;
    }
    
    // Получение соединения с таймаутом
    std::optional<int> acquireConnection(std::chrono::milliseconds timeout) {
        connection_requests_.fetch_add(1);
        
        std::unique_lock<std::mutex> lock(mutex_);
        
        if (!condition_.wait_for(lock, timeout, [this] { 
            return !available_queue_.empty(); 
        })) {
            connection_timeouts_.fetch_add(1);
            std::cerr << "[" << name_ << "] Timeout при получении соединения" << std::endl;
            return std::nullopt;
        }
        
        int conn_id = available_queue_.front();
        available_queue_.pop();
        active_connections_.fetch_add(1);
        
        std::cout << "[" << name_ << "] Соединение " << conn_id << " получено" << std::endl;
        return conn_id;
    }
    
    // Возврат соединения в пул
    void releaseConnection(int conn_id) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            available_queue_.push(conn_id);
        }
        
        active_connections_.fetch_sub(1);
        condition_.notify_one();
        
        std::cout << "[" << name_ << "] Соединение " << conn_id << " возвращено" << std::endl;
    }
    
    // Статистика
    void printStats() const {
        std::cout << "\n=== Connection Pool '" << name_ << "' Statistics ===" << std::endl;
        std::cout << "Макс. соединений: " << max_connections_ << std::endl;
        std::cout << "Активных соединений: " << active_connections_.load() << std::endl;
        std::cout << "Доступных соединений: " << available_queue_.size() << std::endl;
        std::cout << "Запросов соединений: " << connection_requests_.load() << std::endl;
        std::cout << "Таймаутов: " << connection_timeouts_.load() << std::endl;
        std::cout << "================================================" << std::endl;
    }
};

// Демонстрация Thread Pool Bulkheads
void demonstrateThreadPoolBulkheads() {
    std::cout << "\n=== Демонстрация Thread Pool Bulkheads ===" << std::endl;
    
    BulkheadManager manager;
    
    // Регистрируем bulkheads для разных типов сервисов
    manager.registerBulkhead(ServiceType::CRITICAL, "CriticalService", 4, 10);
    manager.registerBulkhead(ServiceType::NORMAL, "NormalService", 2, 5);
    manager.registerBulkhead(ServiceType::BATCH, "BatchService", 1, 20);
    
    // Отправляем задачи разных типов
    std::cout << "\n--- Отправка задач ---" << std::endl;
    
    // Критические задачи
    for (int i = 0; i < 5; ++i) {
        Task task(
            [i]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                std::cout << "Критическая задача " << i << " завершена" << std::endl;
            },
            ServiceType::CRITICAL,
            "Critical_" + std::to_string(i)
        );
        manager.execute(ServiceType::CRITICAL, std::move(task));
    }
    
    // Обычные задачи
    for (int i = 0; i < 10; ++i) {
        Task task(
            [i]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                std::cout << "Обычная задача " << i << " завершена" << std::endl;
            },
            ServiceType::NORMAL,
            "Normal_" + std::to_string(i)
        );
        manager.execute(ServiceType::NORMAL, std::move(task));
    }
    
    // Фоновые задачи (некоторые будут отклонены из-за переполнения)
    for (int i = 0; i < 30; ++i) {
        Task task(
            [i]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                std::cout << "Фоновая задача " << i << " завершена" << std::endl;
            },
            ServiceType::BATCH,
            "Batch_" + std::to_string(i)
        );
        manager.execute(ServiceType::BATCH, std::move(task));
    }
    
    // Ждем выполнения
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    manager.printAllStats();
    manager.shutdownAll();
}

// Демонстрация Connection Pool Bulkheads
void demonstrateConnectionPoolBulkheads() {
    std::cout << "\n=== Демонстрация Connection Pool Bulkheads ===" << std::endl;
    
    // Создаем отдельные пулы для разных сервисов
    ConnectionPoolBulkhead critical_pool("CriticalDB", 5);
    ConnectionPoolBulkhead normal_pool("NormalDB", 3);
    ConnectionPoolBulkhead batch_pool("BatchDB", 2);
    
    std::vector<std::thread> threads;
    
    // Имитируем одновременные запросы от разных сервисов
    std::cout << "\n--- Критические запросы ---" << std::endl;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&critical_pool, i]() {
            auto conn = critical_pool.acquireConnection(std::chrono::seconds(2));
            if (conn.has_value()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                critical_pool.releaseConnection(conn.value());
            }
        });
    }
    
    std::cout << "\n--- Обычные запросы ---" << std::endl;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&normal_pool, i]() {
            auto conn = normal_pool.acquireConnection(std::chrono::seconds(1));
            if (conn.has_value()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                normal_pool.releaseConnection(conn.value());
            }
        });
    }
    
    std::cout << "\n--- Фоновые запросы ---" << std::endl;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&batch_pool, i]() {
            auto conn = batch_pool.acquireConnection(std::chrono::milliseconds(500));
            if (conn.has_value()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                batch_pool.releaseConnection(conn.value());
            }
        });
    }
    
    // Ждем завершения всех потоков
    for (auto& thread : threads) {
        thread.join();
    }
    
    critical_pool.printStats();
    normal_pool.printStats();
    batch_pool.printStats();
}

int main() {
    std::cout << "=== Bulkhead Pattern ===" << std::endl;
    
    try {
        demonstrateThreadPoolBulkheads();
        demonstrateConnectionPoolBulkheads();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Демонстрация завершена ===" << std::endl;
    return 0;
}
