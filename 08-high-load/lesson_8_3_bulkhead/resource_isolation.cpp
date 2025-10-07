/**
 * @file resource_isolation.cpp
 * @brief Изоляция ресурсов для Bulkhead Pattern
 * 
 * Реализована изоляция ресурсов с поддержкой:
 * - Изоляция CPU ресурсов
 * - Изоляция памяти
 * - Изоляция I/O ресурсов
 * - Мониторинг изоляции
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <queue>
#include <condition_variable>
#include <functional>
#include <unordered_map>
#include <optional>

// Лимиты ресурсов для изолированного сервиса
struct ResourceLimits {
    size_t max_memory_bytes;      // Максимальная память
    size_t max_threads;            // Максимум потоков
    size_t max_connections;        // Максимум соединений
    size_t max_file_descriptors;  // Максимум файловых дескрипторов
    
    ResourceLimits(size_t memory_mb = 100, 
                  size_t threads = 4,
                  size_t connections = 10,
                  size_t fds = 100)
        : max_memory_bytes(memory_mb * 1024 * 1024),
          max_threads(threads),
          max_connections(connections),
          max_file_descriptors(fds) {}
};

// Счетчики использования ресурсов
class ResourceUsageTracker {
private:
    std::atomic<size_t> memory_used_{0};
    std::atomic<size_t> threads_active_{0};
    std::atomic<size_t> connections_active_{0};
    std::atomic<size_t> file_descriptors_used_{0};
    
    // Пиковые значения
    std::atomic<size_t> memory_peak_{0};
    std::atomic<size_t> threads_peak_{0};
    std::atomic<size_t> connections_peak_{0};
    
    ResourceLimits limits_;
    std::string service_name_;
    
public:
    ResourceUsageTracker(const std::string& name, const ResourceLimits& limits)
        : limits_(limits), service_name_(name) {}
    
    // Аллокация памяти
    bool allocateMemory(size_t bytes) {
        size_t current = memory_used_.load();
        size_t new_value = current + bytes;
        
        if (new_value > limits_.max_memory_bytes) {
            std::cerr << "[" << service_name_ << "] Превышен лимит памяти: " 
                      << new_value << " > " << limits_.max_memory_bytes << std::endl;
            return false;
        }
        
        memory_used_.fetch_add(bytes);
        
        // Обновляем пик
        size_t peak = memory_peak_.load();
        while (new_value > peak && 
               !memory_peak_.compare_exchange_weak(peak, new_value)) {}
        
        return true;
    }
    
    // Освобождение памяти
    void releaseMemory(size_t bytes) {
        memory_used_.fetch_sub(bytes);
    }
    
    // Создание потока
    bool createThread() {
        size_t current = threads_active_.fetch_add(1);
        
        if (current >= limits_.max_threads) {
            threads_active_.fetch_sub(1);
            std::cerr << "[" << service_name_ << "] Превышен лимит потоков: " 
                      << current << " >= " << limits_.max_threads << std::endl;
            return false;
        }
        
        // Обновляем пик
        size_t new_value = current + 1;
        size_t peak = threads_peak_.load();
        while (new_value > peak && 
               !threads_peak_.compare_exchange_weak(peak, new_value)) {}
        
        return true;
    }
    
    // Уничтожение потока
    void destroyThread() {
        threads_active_.fetch_sub(1);
    }
    
    // Создание соединения
    bool createConnection() {
        size_t current = connections_active_.fetch_add(1);
        
        if (current >= limits_.max_connections) {
            connections_active_.fetch_sub(1);
            std::cerr << "[" << service_name_ << "] Превышен лимит соединений: " 
                      << current << " >= " << limits_.max_connections << std::endl;
            return false;
        }
        
        // Обновляем пик
        size_t new_value = current + 1;
        size_t peak = connections_peak_.load();
        while (new_value > peak && 
               !connections_peak_.compare_exchange_weak(peak, new_value)) {}
        
        return true;
    }
    
    // Закрытие соединения
    void closeConnection() {
        connections_active_.fetch_sub(1);
    }
    
    // Открытие файлового дескриптора
    bool openFileDescriptor() {
        size_t current = file_descriptors_used_.fetch_add(1);
        
        if (current >= limits_.max_file_descriptors) {
            file_descriptors_used_.fetch_sub(1);
            std::cerr << "[" << service_name_ << "] Превышен лимит файловых дескрипторов: " 
                      << current << " >= " << limits_.max_file_descriptors << std::endl;
            return false;
        }
        
        return true;
    }
    
    // Закрытие файлового дескриптора
    void closeFileDescriptor() {
        file_descriptors_used_.fetch_sub(1);
    }
    
    // Текущее использование
    size_t getMemoryUsed() const { return memory_used_.load(); }
    size_t getThreadsActive() const { return threads_active_.load(); }
    size_t getConnectionsActive() const { return connections_active_.load(); }
    size_t getFileDescriptorsUsed() const { return file_descriptors_used_.load(); }
    
    // Пиковые значения
    size_t getMemoryPeak() const { return memory_peak_.load(); }
    size_t getThreadsPeak() const { return threads_peak_.load(); }
    size_t getConnectionsPeak() const { return connections_peak_.load(); }
    
    // Статистика
    void printStats() const {
        double memory_usage_pct = 100.0 * memory_used_.load() / limits_.max_memory_bytes;
        double threads_usage_pct = 100.0 * threads_active_.load() / limits_.max_threads;
        double connections_usage_pct = 100.0 * connections_active_.load() / limits_.max_connections;
        double fds_usage_pct = 100.0 * file_descriptors_used_.load() / limits_.max_file_descriptors;
        
        std::cout << "\n=== Resource Tracker '" << service_name_ << "' ===" << std::endl;
        std::cout << "Память: " << (memory_used_.load() / 1024) << " KB / " 
                  << (limits_.max_memory_bytes / 1024) << " KB (" 
                  << memory_usage_pct << "%, пик: " 
                  << (memory_peak_.load() / 1024) << " KB)" << std::endl;
        std::cout << "Потоки: " << threads_active_.load() << " / " 
                  << limits_.max_threads << " (" << threads_usage_pct 
                  << "%, пик: " << threads_peak_.load() << ")" << std::endl;
        std::cout << "Соединения: " << connections_active_.load() << " / " 
                  << limits_.max_connections << " (" << connections_usage_pct 
                  << "%, пик: " << connections_peak_.load() << ")" << std::endl;
        std::cout << "Файловые дескрипторы: " << file_descriptors_used_.load() << " / " 
                  << limits_.max_file_descriptors << " (" << fds_usage_pct << "%)" << std::endl;
        std::cout << "============================================" << std::endl;
    }
};

// RAII обертка для управления ресурсами
template<typename T>
class ResourceGuard {
private:
    T* resource_;
    std::function<void(T*)> release_func_;
    
public:
    ResourceGuard(T* res, std::function<void(T*)> release)
        : resource_(res), release_func_(std::move(release)) {}
    
    ~ResourceGuard() {
        if (resource_ && release_func_) {
            release_func_(resource_);
        }
    }
    
    ResourceGuard(const ResourceGuard&) = delete;
    ResourceGuard& operator=(const ResourceGuard&) = delete;
    
    ResourceGuard(ResourceGuard&& other) noexcept 
        : resource_(other.resource_), 
          release_func_(std::move(other.release_func_)) {
        other.resource_ = nullptr;
    }
    
    T* get() { return resource_; }
    const T* get() const { return resource_; }
};

// Изолированный сервис с лимитами ресурсов
class IsolatedService {
private:
    std::string name_;
    std::shared_ptr<ResourceUsageTracker> tracker_;
    std::atomic<bool> running_{true};
    
    // Рабочие потоки
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> task_queue_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    
    std::atomic<size_t> tasks_executed_{0};
    std::atomic<size_t> tasks_failed_{0};
    
public:
    IsolatedService(const std::string& name, const ResourceLimits& limits)
        : name_(name),
          tracker_(std::make_shared<ResourceUsageTracker>(name, limits)) {
        
        std::cout << "Изолированный сервис '" << name_ << "' создан" << std::endl;
    }
    
    ~IsolatedService() {
        shutdown();
    }
    
    // Запуск сервиса
    bool start(size_t num_threads) {
        std::cout << "[" << name_ << "] Запуск с " << num_threads << " потоками..." << std::endl;
        
        for (size_t i = 0; i < num_threads; ++i) {
            if (!tracker_->createThread()) {
                std::cerr << "[" << name_ << "] Не удалось создать поток " << i << std::endl;
                return false;
            }
            
            workers_.emplace_back([this, i]() {
                workerThread(i);
            });
        }
        
        return true;
    }
    
    // Остановка сервиса
    void shutdown() {
        if (!running_.load()) return;
        
        std::cout << "[" << name_ << "] Остановка сервиса..." << std::endl;
        
        running_.store(false);
        condition_.notify_all();
        
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
                tracker_->destroyThread();
            }
        }
        
        std::cout << "[" << name_ << "] Сервис остановлен" << std::endl;
    }
    
    // Выполнение задачи
    bool execute(std::function<void()> task, size_t memory_required = 1024) {
        // Проверяем, можем ли аллоцировать память
        if (!tracker_->allocateMemory(memory_required)) {
            tasks_failed_.fetch_add(1);
            return false;
        }
        
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            task_queue_.push([this, task, memory_required]() {
                try {
                    task();
                    tasks_executed_.fetch_add(1);
                } catch (const std::exception& e) {
                    std::cerr << "[" << name_ << "] Ошибка в задаче: " << e.what() << std::endl;
                    tasks_failed_.fetch_add(1);
                }
                
                // Освобождаем память
                tracker_->releaseMemory(memory_required);
            });
        }
        
        condition_.notify_one();
        return true;
    }
    
    // Создание соединения с лимитами
    bool createConnection() {
        return tracker_->createConnection();
    }
    
    void closeConnection() {
        tracker_->closeConnection();
    }
    
    // Открытие файла с лимитами
    bool openFile() {
        return tracker_->openFileDescriptor();
    }
    
    void closeFile() {
        tracker_->closeFileDescriptor();
    }
    
    // Статистика
    void printStats() const {
        std::cout << "\n=== Isolated Service '" << name_ << "' Statistics ===" << std::endl;
        std::cout << "Задач выполнено: " << tasks_executed_.load() << std::endl;
        std::cout << "Задач не удалось: " << tasks_failed_.load() << std::endl;
        
        tracker_->printStats();
    }
    
    std::shared_ptr<ResourceUsageTracker> getTracker() {
        return tracker_;
    }
    
private:
    void workerThread(size_t thread_id) {
        std::cout << "[" << name_ << "] Worker " << thread_id << " запущен" << std::endl;
        
        while (running_.load()) {
            std::function<void()> task;
            
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                
                condition_.wait(lock, [this] {
                    return !task_queue_.empty() || !running_.load();
                });
                
                if (!running_.load() && task_queue_.empty()) {
                    break;
                }
                
                if (!task_queue_.empty()) {
                    task = std::move(task_queue_.front());
                    task_queue_.pop();
                }
            }
            
            if (task) {
                task();
            }
        }
        
        std::cout << "[" << name_ << "] Worker " << thread_id << " завершен" << std::endl;
    }
};

// Менеджер изолированных сервисов
class ResourceIsolationManager {
private:
    std::unordered_map<std::string, std::shared_ptr<IsolatedService>> services_;
    std::mutex mutex_;
    
public:
    ResourceIsolationManager() {
        std::cout << "Resource Isolation Manager создан" << std::endl;
    }
    
    // Создание изолированного сервиса
    bool createService(const std::string& name, 
                      const ResourceLimits& limits,
                      size_t num_threads) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto service = std::make_shared<IsolatedService>(name, limits);
        
        if (!service->start(num_threads)) {
            std::cerr << "Не удалось запустить сервис " << name << std::endl;
            return false;
        }
        
        services_[name] = service;
        return true;
    }
    
    // Получение сервиса
    std::shared_ptr<IsolatedService> getService(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = services_.find(name);
        if (it != services_.end()) {
            return it->second;
        }
        
        return nullptr;
    }
    
    // Статистика всех сервисов
    void printAllStats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::cout << "\n========== Resource Isolation Manager Statistics ==========" << std::endl;
        std::cout << "Всего изолированных сервисов: " << services_.size() << std::endl;
        
        for (const auto& pair : services_) {
            pair.second->printStats();
        }
        
        std::cout << "===========================================================" << std::endl;
    }
    
    // Остановка всех сервисов
    void shutdownAll() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (auto& pair : services_) {
            pair.second->shutdown();
        }
    }
};

// Демонстрация изоляции CPU/памяти
void demonstrateCPUMemoryIsolation() {
    std::cout << "\n=== Демонстрация изоляции CPU/памяти ===" << std::endl;
    
    ResourceIsolationManager manager;
    
    // Создаем изолированные сервисы с разными лимитами
    manager.createService("HighPriorityService", 
                         ResourceLimits(50, 4, 10, 100), 4);
    
    manager.createService("LowPriorityService", 
                         ResourceLimits(20, 2, 5, 50), 2);
    
    // Отправляем задачи в разные сервисы
    auto high_priority = manager.getService("HighPriorityService");
    auto low_priority = manager.getService("LowPriorityService");
    
    std::cout << "\n--- Отправка задач в HighPriorityService ---" << std::endl;
    for (int i = 0; i < 20; ++i) {
        high_priority->execute([i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << "HighPriority задача " << i << " завершена" << std::endl;
        }, 1024 * 100);  // 100 KB на задачу
    }
    
    std::cout << "\n--- Отправка задач в LowPriorityService ---" << std::endl;
    for (int i = 0; i < 20; ++i) {
        low_priority->execute([i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            std::cout << "LowPriority задача " << i << " завершена" << std::endl;
        }, 1024 * 100);  // 100 KB на задачу
    }
    
    // Ждем выполнения
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    manager.printAllStats();
    manager.shutdownAll();
}

// Демонстрация изоляции соединений
void demonstrateConnectionIsolation() {
    std::cout << "\n=== Демонстрация изоляции соединений ===" << std::endl;
    
    ResourceIsolationManager manager;
    
    // Создаем сервисы с лимитами соединений
    manager.createService("WebService", 
                         ResourceLimits(100, 8, 20, 200), 4);
    
    manager.createService("APIService", 
                         ResourceLimits(50, 4, 10, 100), 2);
    
    auto web_service = manager.getService("WebService");
    auto api_service = manager.getService("APIService");
    
    std::vector<std::thread> threads;
    
    std::cout << "\n--- WebService соединения ---" << std::endl;
    for (int i = 0; i < 30; ++i) {
        threads.emplace_back([&web_service, i]() {
            if (web_service->createConnection()) {
                std::cout << "WebService соединение " << i << " создано" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                web_service->closeConnection();
            }
        });
    }
    
    std::cout << "\n--- APIService соединения ---" << std::endl;
    for (int i = 0; i < 20; ++i) {
        threads.emplace_back([&api_service, i]() {
            if (api_service->createConnection()) {
                std::cout << "APIService соединение " << i << " создано" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                api_service->closeConnection();
            }
        });
    }
    
    // Ждем завершения
    for (auto& thread : threads) {
        thread.join();
    }
    
    manager.printAllStats();
    manager.shutdownAll();
}

// Демонстрация изоляции файловых дескрипторов
void demonstrateFileDescriptorIsolation() {
    std::cout << "\n=== Демонстрация изоляции файловых дескрипторов ===" << std::endl;
    
    ResourceIsolationManager manager;
    
    // Создаем сервис с лимитом файловых дескрипторов
    manager.createService("FileService", 
                         ResourceLimits(100, 4, 10, 50), 2);
    
    auto file_service = manager.getService("FileService");
    
    std::vector<std::thread> threads;
    
    std::cout << "\n--- Открытие файлов ---" << std::endl;
    for (int i = 0; i < 100; ++i) {
        threads.emplace_back([&file_service, i]() {
            if (file_service->openFile()) {
                std::cout << "Файл " << i << " открыт" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                file_service->closeFile();
            } else {
                std::cerr << "Не удалось открыть файл " << i << std::endl;
            }
        });
    }
    
    // Ждем завершения
    for (auto& thread : threads) {
        thread.join();
    }
    
    manager.printAllStats();
    manager.shutdownAll();
}

int main() {
    std::cout << "=== Resource Isolation Pattern ===" << std::endl;
    
    try {
        demonstrateCPUMemoryIsolation();
        demonstrateConnectionIsolation();
        demonstrateFileDescriptorIsolation();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Демонстрация завершена ===" << std::endl;
    return 0;
}
