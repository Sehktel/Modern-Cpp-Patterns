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

/**
 * @file thread_pool_pattern.cpp
 * @brief Демонстрация паттерна Thread Pool
 * 
 * Этот файл показывает различные реализации Thread Pool,
 * от базовой до продвинутых версий с мониторингом и балансировкой.
 */

// ============================================================================
// БАЗОВАЯ РЕАЛИЗАЦИЯ THREAD POOL
// ============================================================================

/**
 * @brief Базовая реализация Thread Pool
 * 
 * Особенности:
 * - Предварительно созданные рабочие потоки
 * - Thread-safe очередь задач
 * - Поддержка std::future для получения результатов
 * - Graceful shutdown
 */
class ThreadPool {
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queueMutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
    size_t numThreads_;
    
public:
    explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency()) 
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
    
    ~ThreadPool() {
        shutdown();
    }
    
    /**
     * @brief Добавляет задачу в очередь
     */
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
    
    /**
     * @brief Получает количество потоков
     */
    size_t size() const {
        return numThreads_;
    }
    
    /**
     * @brief Получает количество задач в очереди
     */
    size_t queueSize() const {
        std::lock_guard<std::mutex> lock(queueMutex_);
        return tasks_.size();
    }
    
    /**
     * @brief Останавливает Thread Pool
     */
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
// ПРОДВИНУТАЯ РЕАЛИЗАЦИЯ С МОНИТОРИНГОМ
// ============================================================================

/**
 * @brief Продвинутый Thread Pool с мониторингом
 */
class AdvancedThreadPool {
private:
    struct WorkerStats {
        std::atomic<size_t> tasksCompleted{0};
        std::atomic<size_t> totalExecutionTime{0};
        std::atomic<bool> isBusy{false};
    };
    
    std::vector<std::thread> workers_;
    std::vector<WorkerStats> workerStats_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queueMutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
    size_t numThreads_;
    
    // Статистика
    std::atomic<size_t> totalTasksSubmitted_{0};
    std::atomic<size_t> totalTasksCompleted_{0};
    std::atomic<size_t> totalExecutionTime_{0};
    
public:
    explicit AdvancedThreadPool(size_t numThreads = std::thread::hardware_concurrency()) 
        : stop_(false), numThreads_(numThreads), workerStats_(numThreads) {
        
        std::cout << "Создаю Advanced Thread Pool с " << numThreads_ << " потоками..." << std::endl;
        
        for (size_t i = 0; i < numThreads_; ++i) {
            workers_.emplace_back([this, i] {
                std::cout << "Advanced Worker " << i << " запущен" << std::endl;
                
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
                    
                    // Выполняем задачу с мониторингом
                    workerStats_[i].isBusy.store(true);
                    auto start = std::chrono::high_resolution_clock::now();
                    
                    try {
                        task();
                    } catch (const std::exception& e) {
                        std::cerr << "Ошибка в Advanced Worker " << i << ": " << e.what() << std::endl;
                    }
                    
                    auto end = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                    
                    // Обновляем статистику
                    workerStats_[i].tasksCompleted.fetch_add(1);
                    workerStats_[i].totalExecutionTime.fetch_add(duration.count());
                    workerStats_[i].isBusy.store(false);
                    
                    totalTasksCompleted_.fetch_add(1);
                    totalExecutionTime_.fetch_add(duration.count());
                }
                
                std::cout << "Advanced Worker " << i << " завершен" << std::endl;
            });
        }
    }
    
    ~AdvancedThreadPool() {
        shutdown();
    }
    
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type> {
        
        using return_type = typename std::result_of<F(Args...)>::type;
        
        if (stop_.load()) {
            throw std::runtime_error("enqueue на остановленном AdvancedThreadPool");
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
    
    void printStatistics() const {
        std::cout << "\n=== СТАТИСТИКА THREAD POOL ===" << std::endl;
        std::cout << "Всего потоков: " << numThreads_ << std::endl;
        std::cout << "Задач в очереди: " << queueSize() << std::endl;
        std::cout << "Задач отправлено: " << totalTasksSubmitted_.load() << std::endl;
        std::cout << "Задач выполнено: " << totalTasksCompleted_.load() << std::endl;
        std::cout << "Общее время выполнения: " << totalExecutionTime_.load() << " мс" << std::endl;
        
        if (totalTasksCompleted_.load() > 0) {
            double avgTime = static_cast<double>(totalExecutionTime_.load()) / totalTasksCompleted_.load();
            std::cout << "Среднее время выполнения: " << avgTime << " мс" << std::endl;
        }
        
        std::cout << "\n=== СТАТИСТИКА ПО ПОТОКАМ ===" << std::endl;
        for (size_t i = 0; i < workerStats_.size(); ++i) {
            const auto& stats = workerStats_[i];
            std::cout << "Worker " << i << ": "
                      << "задач=" << stats.tasksCompleted.load()
                      << ", время=" << stats.totalExecutionTime.load() << " мс"
                      << ", занят=" << (stats.isBusy.load() ? "да" : "нет") << std::endl;
        }
        std::cout << "==============================" << std::endl;
    }
    
    size_t size() const { return numThreads_; }
    
    size_t queueSize() const {
        std::lock_guard<std::mutex> lock(queueMutex_);
        return tasks_.size();
    }
    
    void shutdown() {
        if (stop_.load()) return;
        
        std::cout << "Останавливаю Advanced Thread Pool..." << std::endl;
        
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
        
        printStatistics();
    }
};

// ============================================================================
// ПРИМЕРЫ ИСПОЛЬЗОВАНИЯ
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
 * @brief Сложная задача с исключениями
 */
void complexTask(int id) {
    std::cout << "Начинаю сложную задачу " << id << std::endl;
    
    // Имитация работы
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Иногда генерируем исключение
    if (id % 7 == 0) {
        throw std::runtime_error("Ошибка в задаче " + std::to_string(id));
    }
    
    std::cout << "Сложная задача " << id << " завершена успешно" << std::endl;
}

/**
 * @brief Задача для вычисления чисел Фибоначчи
 */
long long fibonacci(int n) {
    if (n <= 1) return n;
    
    long long a = 0, b = 1;
    for (int i = 2; i <= n; ++i) {
        long long temp = a + b;
        a = b;
        b = temp;
    }
    
    return b;
}

// ============================================================================
// ДЕМОНСТРАЦИОННЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Демонстрация базового Thread Pool
 */
void demonstrateBasicThreadPool() {
    std::cout << "\n=== БАЗОВЫЙ THREAD POOL ===" << std::endl;
    
    ThreadPool pool(4);
    
    std::cout << "Размер пула: " << pool.size() << std::endl;
    
    // Добавляем несколько задач
    std::vector<std::future<int>> futures;
    
    for (int i = 1; i <= 10; ++i) {
        auto future = pool.enqueue(simpleTask, i, 100 + i * 10);
        futures.push_back(std::move(future));
        
        std::cout << "Отправлена задача " << i << ", очередь: " << pool.queueSize() << std::endl;
    }
    
    // Ждем завершения всех задач
    std::cout << "Жду завершения задач..." << std::endl;
    for (size_t i = 0; i < futures.size(); ++i) {
        try {
            int result = futures[i].get();
            std::cout << "Получен результат задачи " << (i + 1) << ": " << result << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Ошибка в задаче " << (i + 1) << ": " << e.what() << std::endl;
        }
    }
    
    std::cout << "Все задачи завершены" << std::endl;
}

/**
 * @brief Демонстрация продвинутого Thread Pool
 */
void demonstrateAdvancedThreadPool() {
    std::cout << "\n=== ПРОДВИНУТЫЙ THREAD POOL ===" << std::endl;
    
    AdvancedThreadPool pool(3);
    
    // Добавляем задачи с разной сложностью
    std::vector<std::future<void>> futures;
    
    for (int i = 1; i <= 15; ++i) {
        auto future = pool.enqueue(complexTask, i);
        futures.push_back(std::move(future));
        
        // Небольшая задержка между отправкой задач
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    // Ждем завершения всех задач
    for (size_t i = 0; i < futures.size(); ++i) {
        try {
            futures[i].get();
        } catch (const std::exception& e) {
            std::cout << "Задача " << (i + 1) << " завершилась с ошибкой: " << e.what() << std::endl;
        }
    }
    
    std::cout << "Все задачи завершены" << std::endl;
}

/**
 * @brief Демонстрация параллельных вычислений
 */
void demonstrateParallelComputations() {
    std::cout << "\n=== ПАРАЛЛЕЛЬНЫЕ ВЫЧИСЛЕНИЯ ===" << std::endl;
    
    ThreadPool pool(std::thread::hardware_concurrency());
    
    // Вычисляем числа Фибоначчи параллельно
    std::vector<int> fibonacciNumbers = {30, 35, 40, 45, 50};
    std::vector<std::future<long long>> futures;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int n : fibonacciNumbers) {
        auto future = pool.enqueue(fibonacci, n);
        futures.push_back(std::move(future));
        std::cout << "Запущено вычисление Fibonacci(" << n << ")" << std::endl;
    }
    
    // Собираем результаты
    for (size_t i = 0; i < futures.size(); ++i) {
        long long result = futures[i].get();
        std::cout << "Fibonacci(" << fibonacciNumbers[i] << ") = " << result << std::endl;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Все вычисления завершены за " << duration.count() << " мс" << std::endl;
    
    // Сравним с последовательным выполнением
    start = std::chrono::high_resolution_clock::now();
    
    for (int n : fibonacciNumbers) {
        long long result = fibonacci(n);
        std::cout << "Sequential Fibonacci(" << n << ") = " << result << std::endl;
    }
    
    end = std::chrono::high_resolution_clock::now();
    auto sequentialDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Последовательное выполнение заняло " << sequentialDuration.count() << " мс" << std::endl;
    std::cout << "Ускорение: " << (double)sequentialDuration.count() / duration.count() << "x" << std::endl;
}

/**
 * @brief Демонстрация производительности
 */
void demonstratePerformance() {
    std::cout << "\n=== ТЕСТ ПРОИЗВОДИТЕЛЬНОСТИ ===" << std::endl;
    
    const int NUM_TASKS = 1000;
    const int TASK_DURATION = 10; // мс
    
    // Тест с Thread Pool
    {
        std::cout << "Тестирую Thread Pool..." << std::endl;
        ThreadPool pool(4);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::future<int>> futures;
        for (int i = 0; i < NUM_TASKS; ++i) {
            futures.push_back(pool.enqueue(simpleTask, i, TASK_DURATION));
        }
        
        // Ждем завершения всех задач
        for (auto& future : futures) {
            future.get();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Thread Pool: " << NUM_TASKS << " задач за " 
                  << duration.count() << " мс" << std::endl;
        std::cout << "Производительность: " 
                  << (NUM_TASKS * 1000.0 / duration.count()) << " задач/сек" << std::endl;
    }
    
    // Тест последовательного выполнения
    {
        std::cout << "Тестирую последовательное выполнение..." << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < NUM_TASKS; ++i) {
            simpleTask(i, TASK_DURATION);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Последовательное: " << NUM_TASKS << " задач за " 
                  << duration.count() << " мс" << std::endl;
        std::cout << "Производительность: " 
                  << (NUM_TASKS * 1000.0 / duration.count()) << " задач/сек" << std::endl;
    }
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🏊 Демонстрация паттерна Thread Pool" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    try {
        demonstrateBasicThreadPool();
        demonstrateAdvancedThreadPool();
        demonstrateParallelComputations();
        demonstratePerformance();
        
        std::cout << "\n✅ Все демонстрации завершены успешно!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n🎯 Рекомендации:" << std::endl;
    std::cout << "1. Используйте Thread Pool для избежания накладных расходов" << std::endl;
    std::cout << "2. Размер пула обычно равен количеству CPU ядер" << std::endl;
    std::cout << "3. Мониторьте производительность и загрузку потоков" << std::endl;
    std::cout << "4. Правильно обрабатывайте исключения в задачах" << std::endl;
    std::cout << "5. Используйте std::future для получения результатов" << std::endl;
    
    return 0;
}
