/**
 * @file thread_pool_cpp23_comparison.cpp
 * @brief Сравнение Thread Pool Pattern: C++17/20 vs C++23
 * 
 * Этот файл демонстрирует различия между текущей реализацией
 * и улучшенной версией с использованием возможностей C++23.
 */

#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <functional>
#include <future>
#include <chrono>
#include <atomic>
#include <random>
#include <memory>

// ============================================================================
// C++17/20 РЕАЛИЗАЦИЯ (ТЕКУЩАЯ)
// ============================================================================

/**
 * @brief Текущая реализация Thread Pool (C++17/20)
 */
class ThreadPoolCpp20 {
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queueMutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
    size_t numThreads_;
    
public:
    explicit ThreadPoolCpp20(size_t numThreads = std::thread::hardware_concurrency()) 
        : stop_(false), numThreads_(numThreads) {
        
        std::cout << "Создаю Thread Pool с " << numThreads_ << " потоками..." << std::endl;
        
        // Создаем рабочие потоки
        for (size_t i = 0; i < numThreads_; ++i) {
            workers_.emplace_back([this, i] {
                std::cout << "Рабочий поток " << i << " запущен (ID: " 
                          << std::this_thread::get_id() << ")" << std::endl;
                
                while (true) {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(queueMutex_);
                        condition_.wait(lock, [this] { return stop_.load() || !tasks_.empty(); });
                        
                        if (stop_.load() && tasks_.empty()) {
                            break;
                        }
                        
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    
                    // Выполняем задачу
                    try {
                        task();
                    } catch (const std::exception& e) {
                        std::cerr << "Ошибка в рабочем потоке: " << e.what() << std::endl;
                    }
                }
                
                std::cout << "Рабочий поток " << i << " завершен" << std::endl;
            });
        }
    }
    
    ~ThreadPoolCpp20() {
        shutdown();
    }
    
    // ❌ C++17/20: Простое добавление задач без проверки ошибок
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type> {
        
        using return_type = typename std::result_of<F(Args...)>::type;
        
        if (stop_.load()) {
            throw std::runtime_error("enqueue на остановленном ThreadPool");
        }
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> result = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            tasks_.emplace([task]() { (*task)(); });
        }
        
        condition_.notify_one();
        return result;
    }
    
    size_t size() const {
        return numThreads_;
    }
    
    size_t queueSize() const {
        std::lock_guard<std::mutex> lock(queueMutex_);
        return tasks_.size();
    }
    
    void shutdown() {
        if (stop_.load()) {
            return;
        }
        
        std::cout << "Останавливаю Thread Pool..." << std::endl;
        
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            stop_.store(true);
        }
        
        condition_.notify_all();
        
        for (std::thread& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        
        std::cout << "Thread Pool остановлен" << std::endl;
    }
};

// ============================================================================
// C++23 РЕАЛИЗАЦИЯ (УЛУЧШЕННАЯ)
// ============================================================================

// ✅ C++23: Новые заголовки
#ifdef __cpp_lib_expected
#include <expected>
#endif

#ifdef __cpp_lib_print
#include <print>
#endif

#ifdef __cpp_lib_jthread
#include <jthread>
#endif

#ifdef __cpp_lib_generator
#include <generator>
#endif

#ifdef __cpp_lib_stacktrace
#include <stacktrace>
#endif

/**
 * @brief Улучшенная реализация Thread Pool (C++23)
 */
class ThreadPoolCpp23 {
private:
    // ✅ C++23: std::jthread с автоматическим join
#ifdef __cpp_lib_jthread
    std::vector<std::jthread> workers_;
#else
    std::vector<std::thread> workers_;
#endif
    
    std::queue<std::function<void()>> tasks_;
    std::mutex queueMutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
    size_t numThreads_;
    
    // ✅ C++23: Расширенная статистика
    std::atomic<size_t> totalTasksSubmitted_{0};
    std::atomic<size_t> totalTasksCompleted_{0};
    std::atomic<size_t> totalTasksFailed_{0};
    std::atomic<size_t> totalExecutionTime_{0};
    
    // Статистика по потокам
    struct WorkerStats {
        std::atomic<size_t> tasksCompleted{0};
        std::atomic<size_t> totalExecutionTime{0};
        std::atomic<bool> isBusy{false};
        std::string name;
    };
    
    std::vector<WorkerStats> workerStats_;
    
public:
    explicit ThreadPoolCpp23(size_t numThreads = std::thread::hardware_concurrency()) 
        : stop_(false), numThreads_(numThreads), workerStats_(numThreads) {
        
#ifdef __cpp_lib_print
        std::print("Создаю Thread Pool с {} потоками...\n", numThreads_);
#else
        std::cout << "Создаю Thread Pool с " << numThreads_ << " потоками..." << std::endl;
#endif
        
        // Создаем рабочие потоки
        for (size_t i = 0; i < numThreads_; ++i) {
            workerStats_[i].name = "Worker_" + std::to_string(i);
            
#ifdef __cpp_lib_jthread
            workers_.emplace_back([this, i](std::stop_token stopToken) {
#else
            workers_.emplace_back([this, i]() {
#endif
                
#ifdef __cpp_lib_print
                std::print("Рабочий поток {} запущен (ID: {})\n", i, std::this_thread::get_id());
#else
                std::cout << "Рабочий поток " << i << " запущен (ID: " 
                          << std::this_thread::get_id() << ")" << std::endl;
#endif
                
                while (true) {
#ifdef __cpp_lib_jthread
                    if (stopToken.stop_requested()) {
                        break;
                    }
#endif
                    
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(queueMutex_);
                        condition_.wait(lock, [this] { return stop_.load() || !tasks_.empty(); });
                        
                        if (stop_.load() && tasks_.empty()) {
                            break;
                        }
                        
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    
                    // ✅ C++23: Выполнение с детальной статистикой
                    workerStats_[i].isBusy.store(true);
                    auto start = std::chrono::high_resolution_clock::now();
                    
                    try {
                        task();
                        totalTasksCompleted_.fetch_add(1);
                        workerStats_[i].tasksCompleted.fetch_add(1);
                    } catch (const std::exception& e) {
                        totalTasksFailed_.fetch_add(1);
                        
#ifdef __cpp_lib_stacktrace
                        auto trace = std::stacktrace::current();
                        std::print("❌ Ошибка в рабочем потоке {}: {}\n", i, e.what());
                        std::print("Stack trace:\n");
                        for (const auto& frame : trace) {
                            std::print("  {}\n", frame.description());
                        }
#else
                        std::cerr << "❌ Ошибка в рабочем потоке " << i << ": " << e.what() << std::endl;
#endif
                    }
                    
                    auto end = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                    
                    workerStats_[i].totalExecutionTime.fetch_add(duration.count());
                    workerStats_[i].isBusy.store(false);
                    totalExecutionTime_.fetch_add(duration.count());
                }
                
#ifdef __cpp_lib_print
                std::print("Рабочий поток {} завершен\n", i);
#else
                std::cout << "Рабочий поток " << i << " завершен" << std::endl;
#endif
            });
        }
    }
    
    ~ThreadPoolCpp23() {
        shutdown();
    }
    
    // ✅ C++23: Добавление задач с проверкой ошибок
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::expected<std::future<typename std::result_of<F(Args...)>::type>, std::string> {
        
        using return_type = typename std::result_of<F(Args...)>::type;
        
        if (stop_.load()) {
            return std::unexpected("Thread pool is stopped");
        }
        
        totalTasksSubmitted_.fetch_add(1);
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> result = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            tasks_.emplace([task]() { (*task)(); });
        }
        
        condition_.notify_one();
        return result;
    }
    
    // ✅ C++23: Генератор задач для ленивого выполнения
#ifdef __cpp_lib_generator
    std::generator<std::function<void()>> getTasks() {
        while (!stop_.load()) {
            std::function<void()> task;
            
            {
                std::unique_lock<std::mutex> lock(queueMutex_);
                condition_.wait(lock, [this] { return stop_.load() || !tasks_.empty(); });
                
                if (stop_.load() && tasks_.empty()) {
                    break;
                }
                
                task = std::move(tasks_.front());
                tasks_.pop();
            }
            
            co_yield task;
        }
    }
#endif
    
    size_t size() const {
        return numThreads_;
    }
    
    size_t queueSize() const {
        std::lock_guard<std::mutex> lock(queueMutex_);
        return tasks_.size();
    }
    
    // ✅ C++23: Детальная статистика
    struct DetailedStatistics {
        size_t totalTasksSubmitted;
        size_t totalTasksCompleted;
        size_t totalTasksFailed;
        size_t totalExecutionTime;
        double averageExecutionTime;
        double successRate;
        double failureRate;
        std::vector<WorkerStats> workerStats;
    };
    
    DetailedStatistics getDetailedStatistics() const {
        DetailedStatistics stats;
        stats.totalTasksSubmitted = totalTasksSubmitted_.load();
        stats.totalTasksCompleted = totalTasksCompleted_.load();
        stats.totalTasksFailed = totalTasksFailed_.load();
        stats.totalExecutionTime = totalExecutionTime_.load();
        stats.workerStats = workerStats_;
        
        if (stats.totalTasksCompleted > 0) {
            stats.averageExecutionTime = static_cast<double>(stats.totalExecutionTime) / stats.totalTasksCompleted;
        } else {
            stats.averageExecutionTime = 0.0;
        }
        
        if (stats.totalTasksSubmitted > 0) {
            stats.successRate = static_cast<double>(stats.totalTasksCompleted) / stats.totalTasksSubmitted;
            stats.failureRate = static_cast<double>(stats.totalTasksFailed) / stats.totalTasksSubmitted;
        } else {
            stats.successRate = 0.0;
            stats.failureRate = 0.0;
        }
        
        return stats;
    }
    
    void printDetailedStatistics() const {
        auto stats = getDetailedStatistics();
        
#ifdef __cpp_lib_print
        std::print("\n=== ДЕТАЛЬНАЯ СТАТИСТИКА THREAD POOL ===\n");
        std::print("Всего потоков: {}\n", numThreads_);
        std::print("Задач в очереди: {}\n", queueSize());
        std::print("Задач отправлено: {}\n", stats.totalTasksSubmitted);
        std::print("Задач выполнено: {}\n", stats.totalTasksCompleted);
        std::print("Задач неудачных: {}\n", stats.totalTasksFailed);
        std::print("Общее время выполнения: {} мс\n", stats.totalExecutionTime);
        std::print("Среднее время выполнения: {:.2f} мс\n", stats.averageExecutionTime);
        std::print("Процент успеха: {:.2f}%\n", stats.successRate * 100);
        std::print("Процент неудач: {:.2f}%\n", stats.failureRate * 100);
        
        std::print("\n=== СТАТИСТИКА ПО ПОТОКАМ ===\n");
        for (size_t i = 0; i < workerStats_.size(); ++i) {
            const auto& workerStats = workerStats_[i];
            std::print("Worker {}: задач={}, время={} мс, занят={}\n",
                       i, workerStats.tasksCompleted.load(), 
                       workerStats.totalExecutionTime.load(),
                       (workerStats.isBusy.load() ? "да" : "нет"));
        }
        std::print("=====================================\n");
#else
        std::cout << "\n=== ДЕТАЛЬНАЯ СТАТИСТИКА THREAD POOL ===" << std::endl;
        std::cout << "Всего потоков: " << numThreads_ << std::endl;
        std::cout << "Задач в очереди: " << queueSize() << std::endl;
        std::cout << "Задач отправлено: " << stats.totalTasksSubmitted << std::endl;
        std::cout << "Задач выполнено: " << stats.totalTasksCompleted << std::endl;
        std::cout << "Задач неудачных: " << stats.totalTasksFailed << std::endl;
        std::cout << "Общее время выполнения: " << stats.totalExecutionTime << " мс" << std::endl;
        std::cout << "Среднее время выполнения: " << stats.averageExecutionTime << " мс" << std::endl;
        std::cout << "Процент успеха: " << (stats.successRate * 100) << "%" << std::endl;
        std::cout << "Процент неудач: " << (stats.failureRate * 100) << "%" << std::endl;
        
        std::cout << "\n=== СТАТИСТИКА ПО ПОТОКАМ ===" << std::endl;
        for (size_t i = 0; i < workerStats_.size(); ++i) {
            const auto& workerStats = workerStats_[i];
            std::cout << "Worker " << i << ": задач=" << workerStats.tasksCompleted.load()
                      << ", время=" << workerStats.totalExecutionTime.load() << " мс"
                      << ", занят=" << (workerStats.isBusy.load() ? "да" : "нет") << std::endl;
        }
        std::cout << "=====================================" << std::endl;
#endif
    }
    
    void shutdown() {
        if (stop_.load()) return;
        
#ifdef __cpp_lib_print
        std::print("Останавливаю Thread Pool...\n");
#else
        std::cout << "Останавливаю Thread Pool..." << std::endl;
#endif
        
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            stop_.store(true);
        }
        
        condition_.notify_all();
        
#ifdef __cpp_lib_jthread
        // ✅ C++23: std::jthread автоматически join в деструкторе
        workers_.clear();
#else
        for (std::thread& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
#endif
        
        printDetailedStatistics();
        
#ifdef __cpp_lib_print
        std::print("Thread Pool остановлен\n");
#else
        std::cout << "Thread Pool остановлен" << std::endl;
#endif
    }
};

// ============================================================================
// ПРИМЕРЫ ЗАДАЧ ДЛЯ ДЕМОНСТРАЦИИ
// ============================================================================

/**
 * @brief Простая задача для демонстрации
 */
int simpleTask(int id, int duration) {
    std::cout << "Выполняю задачу " << id << " в потоке " 
              << std::this_thread::get_id() << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    
    int result = id * id;
    std::cout << "Задача " << id << " завершена, результат: " << result << std::endl;
    
    return result;
}

/**
 * @brief Задача с исключением для демонстрации обработки ошибок
 */
void taskWithException(int id) {
    std::cout << "Начинаю задачу с исключением " << id << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    if (id % 3 == 0) {
        throw std::runtime_error("Ошибка в задаче " + std::to_string(id));
    }
    
    std::cout << "Задача " << id << " завершена успешно" << std::endl;
}

// ============================================================================
// ДЕМОНСТРАЦИЯ РАЗЛИЧИЙ
// ============================================================================

void demonstrateDifferences() {
    std::cout << "\n=== СРАВНЕНИЕ THREAD POOL PATTERN: C++17/20 vs C++23 ===" << std::endl;
    
    // ============================================================================
    // C++17/20 ДЕМОНСТРАЦИЯ
    // ============================================================================
    std::cout << "\n--- C++17/20 Thread Pool ---" << std::endl;
    
    ThreadPoolCpp20 pool20(3);
    
    // Добавляем задачи
    std::vector<std::future<int>> futures20;
    
    for (int i = 1; i <= 6; ++i) {
        try {
            auto future = pool20.enqueue(simpleTask, i, 100 + i * 10);
            futures20.push_back(std::move(future));
            std::cout << "Отправлена задача " << i << ", очередь: " << pool20.queueSize() << std::endl;
        } catch (const std::exception& e) {
            std::cout << "❌ Ошибка отправки задачи " << i << ": " << e.what() << std::endl;
        }
    }
    
    // Ждем завершения всех задач
    for (size_t i = 0; i < futures20.size(); ++i) {
        try {
            int result = futures20[i].get();
            std::cout << "✅ Получен результат задачи " << (i + 1) << ": " << result << std::endl;
        } catch (const std::exception& e) {
            std::cout << "❌ Ошибка в задаче " << (i + 1) << ": " << e.what() << std::endl;
        }
    }
    
    pool20.shutdown();
    
    // ============================================================================
    // C++23 ДЕМОНСТРАЦИЯ
    // ============================================================================
    std::cout << "\n--- C++23 Thread Pool ---" << std::endl;
    
    ThreadPoolCpp23 pool23(3);
    
    // ✅ C++23: Добавление задач с проверкой ошибок
    std::vector<std::future<int>> futures23;
    
    for (int i = 1; i <= 6; ++i) {
        auto result = pool23.enqueue(simpleTask, i, 100 + i * 10);
        
        if (result) {
            futures23.push_back(std::move(*result));
            std::cout << "✅ Отправлена задача " << i << ", очередь: " << pool23.queueSize() << std::endl;
        } else {
            std::cout << "❌ Ошибка отправки задачи " << i << ": " << result.error() << std::endl;
        }
    }
    
    // Добавляем задачи с исключениями для демонстрации обработки ошибок
    for (int i = 7; i <= 9; ++i) {
        auto result = pool23.enqueue(taskWithException, i);
        
        if (result) {
            std::cout << "✅ Отправлена задача с исключением " << i << std::endl;
        } else {
            std::cout << "❌ Ошибка отправки задачи " << i << ": " << result.error() << std::endl;
        }
    }
    
    // Ждем завершения всех задач
    for (size_t i = 0; i < futures23.size(); ++i) {
        try {
            int result = futures23[i].get();
            std::cout << "✅ Получен результат задачи " << (i + 1) << ": " << result << std::endl;
        } catch (const std::exception& e) {
            std::cout << "❌ Ошибка в задаче " << (i + 1) << ": " << e.what() << std::endl;
        }
    }
    
    // ✅ C++23: Детальная статистика
    pool23.printDetailedStatistics();
    
    pool23.shutdown();
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🏊 Сравнение Thread Pool Pattern: C++17/20 vs C++23" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    
    try {
        demonstrateDifferences();
        
        std::cout << "\n✅ Сравнение завершено!" << std::endl;
        
        std::cout << "\n🎯 Ключевые улучшения C++23:" << std::endl;
        std::cout << "1. std::jthread - автоматический join в деструкторе" << std::endl;
        std::cout << "2. std::expected - четкая обработка ошибок при добавлении задач" << std::endl;
        std::cout << "3. std::generator - ленивая генерация задач" << std::endl;
        std::cout << "4. std::print - более читаемый вывод" << std::endl;
        std::cout << "5. std::stacktrace - детальная отладка ошибок" << std::endl;
        std::cout << "6. Расширенная статистика и мониторинг" << std::endl;
        std::cout << "7. Детальная статистика по каждому потоку" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

