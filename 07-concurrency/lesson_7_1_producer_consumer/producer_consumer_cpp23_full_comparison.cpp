/**
 * @file producer_consumer_cpp23_full_comparison.cpp
 * @brief ПОЛНОЕ СРАВНЕНИЕ Producer-Consumer Pattern: C++17/20 vs C++23
 * 
 * Этот файл содержит детальное сравнение реализаций Producer-Consumer
 * с полными примерами кода, различными стратегиями и статистикой.
 * 
 * СОДЕРЖАНИЕ:
 * - Полная реализация C++17/20 Producer-Consumer Queue
 * - Полная реализация C++23 Producer-Consumer Queue с std::expected
 * - SmartProducer с 4 стратегиями: LINEAR, RANDOM, FIBONACCI, PRIME
 * - SmartConsumer с 4 стратегиями: SIMPLE, STATISTICS, FILTER, TRANSFORM
 * - Детальная статистика и мониторинг
 * - Демонстрация всех возможностей side-by-side
 * 
 * Компиляция:
 * - C++17/20: g++ -std=c++20 producer_consumer_cpp23_full_comparison.cpp
 * - C++23: g++ -std=c++23 producer_consumer_cpp23_full_comparison.cpp
 * 
 * РАЗМЕР: ~820 строк (для глубокого изучения и практики)
 */

#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <vector>
#include <random>
#include <memory>
#include <optional>
#include <variant>
#include <any>
#include <functional>
#include <atomic>

// ============================================================================
// C++17/20 РЕАЛИЗАЦИЯ (ТЕКУЩАЯ)
// ============================================================================

/**
 * @brief Текущая реализация Producer-Consumer (C++17/20)
 */
template<typename T>
class ProducerConsumerQueueCpp20 {
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    bool finished_ = false;
    size_t maxSize_ = 0; // 0 = без ограничений
    
public:
    explicit ProducerConsumerQueueCpp20(size_t maxSize = 0) : maxSize_(maxSize) {}
    
    // ❌ C++17/20: Простое добавление без детальной информации об ошибках
    bool push(T item) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Ждем, если очередь полная
        if (maxSize_ > 0 && queue_.size() >= maxSize_) {
            condition_.wait(lock, [this] { 
                return queue_.size() < maxSize_ || finished_; 
            });
        }
        
        if (finished_) return false;
        
        queue_.push(std::move(item));
        condition_.notify_one(); // Уведомляем consumer
        return true;
    }
    
    // ❌ C++17/20: Простое извлечение без timeout
    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Ждем, пока не появится элемент или не завершится работа
        condition_.wait(lock, [this] { return !queue_.empty() || finished_; });
        
        if (queue_.empty()) return false;
        
        item = std::move(queue_.front());
        queue_.pop();
        condition_.notify_one(); // Уведомляем producer о свободном месте
        return true;
    }
    
    void finish() {
        std::lock_guard<std::mutex> lock(mutex_);
        finished_ = true;
        condition_.notify_all(); // Уведомляем все ожидающие потоки
    }
    
    bool isFinished() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return finished_;
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
};

/**
 * @brief Простой Producer для C++17/20
 */
class ProducerCpp20 {
private:
    ProducerConsumerQueueCpp20<int>& queue_;
    int startValue_;
    int count_;
    std::chrono::milliseconds delay_;
    std::string name_;
    
public:
    ProducerCpp20(ProducerConsumerQueueCpp20<int>& queue, 
                  const std::string& name = "Producer",
                  int startValue = 1, 
                  int count = 10,
                  std::chrono::milliseconds delay = std::chrono::milliseconds(100))
        : queue_(queue), name_(name), startValue_(startValue), count_(count), delay_(delay) {}
    
    void produce() {
        std::cout << "[" << name_ << "] Начинаю производство " << count_ << " элементов..." << std::endl;
        
        for (int i = 0; i < count_; ++i) {
            int value = startValue_ + i;
            
            // Имитация времени на производство
            std::this_thread::sleep_for(delay_);
            
            if (queue_.push(value)) {
                std::cout << "[" << name_ << "] Произвел: " << value << std::endl;
            } else {
                std::cout << "[" << name_ << "] Не удалось произвести: " << value << " (очередь завершена)" << std::endl;
                break;
            }
        }
        
        std::cout << "[" << name_ << "] Завершил производство" << std::endl;
    }
};

/**
 * @brief Простой Consumer для C++17/20
 */
class ConsumerCpp20 {
private:
    ProducerConsumerQueueCpp20<int>& queue_;
    std::chrono::milliseconds delay_;
    std::string name_;
    
public:
    ConsumerCpp20(ProducerConsumerQueueCpp20<int>& queue,
                  const std::string& name = "Consumer",
                  std::chrono::milliseconds delay = std::chrono::milliseconds(150))
        : queue_(queue), name_(name), delay_(delay) {}
    
    void consume() {
        std::cout << "[" << name_ << "] Начинаю потребление..." << std::endl;
        
        int item;
        int processedCount = 0;
        
        while (queue_.pop(item)) {
            // Имитация времени на обработку
            std::this_thread::sleep_for(delay_);
            
            // Обработка элемента
            int result = processItem(item);
            
            std::cout << "[" << name_ << "] Обработал: " << item << " -> " << result << std::endl;
            processedCount++;
        }
        
        std::cout << "[" << name_ << "] Завершил потребление. Обработано: " << processedCount << " элементов" << std::endl;
    }
    
private:
    int processItem(int item) {
        // Простая обработка - возведение в квадрат
        return item * item;
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

#ifdef __cpp_lib_generator
#include <generator>
#endif

#ifdef __cpp_lib_flat_map
#include <flat_map>
#endif

#ifdef __cpp_lib_stacktrace
#include <stacktrace>
#endif

/**
 * @brief Типы сообщений для продвинутой очереди
 */
struct DataMessage {
    int id;
    std::string content;
    std::chrono::system_clock::time_point timestamp;
};

struct ControlMessage {
    enum Type { START, STOP, PAUSE, RESUME } type;
    std::string reason;
};

using Message = std::variant<DataMessage, ControlMessage>;

/**
 * @brief Улучшенная реализация Producer-Consumer (C++23)
 */
template<typename T>
class ProducerConsumerQueueCpp23 {
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic<bool> finished_{false};
    std::atomic<size_t> maxSize_{0};
    std::atomic<size_t> currentSize_{0};
    
    // ✅ C++23: Расширенная статистика
    mutable std::mutex statsMutex_;
    std::atomic<size_t> totalProduced_{0};
    std::atomic<size_t> totalConsumed_{0};
    std::atomic<size_t> totalBlocked_{0};
    std::atomic<size_t> totalTimeouts_{0};
    
public:
    explicit ProducerConsumerQueueCpp23(size_t maxSize = 0) : maxSize_(maxSize) {}
    
    // ✅ C++23: Добавление с детальной информацией об ошибках
    std::expected<void, std::string> push(T item) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Проверяем ограничение размера
        if (maxSize_.load() > 0 && currentSize_.load() >= maxSize_.load()) {
            totalBlocked_.fetch_add(1);
            condition_.wait(lock, [this] { 
                return currentSize_.load() < maxSize_.load() || finished_.load(); 
            });
        }
        
        if (finished_.load()) {
            return std::unexpected("Queue is finished");
        }
        
        queue_.push(std::move(item));
        currentSize_.fetch_add(1);
        totalProduced_.fetch_add(1);
        
        condition_.notify_one();
        return {};
    }
    
    // ✅ C++23: Извлечение с timeout и optional возвратом
    std::expected<T, std::string> pop(std::chrono::milliseconds timeout = std::chrono::milliseconds::max()) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        bool success = condition_.wait_for(lock, timeout, [this] { 
            return !queue_.empty() || finished_.load(); 
        });
        
        if (!success) {
            totalTimeouts_.fetch_add(1);
            return std::unexpected("Timeout waiting for item");
        }
        
        if (queue_.empty()) {
            return std::unexpected("Queue is finished and empty");
        }
        
        T item = std::move(queue_.front());
        queue_.pop();
        currentSize_.fetch_sub(1);
        totalConsumed_.fetch_add(1);
        
        condition_.notify_one();
        return std::move(item);
    }
    
    // ✅ C++23: Попытка извлечения без блокировки
    std::optional<T> tryPop() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (queue_.empty()) {
            return std::nullopt;
        }
        
        T item = std::move(queue_.front());
        queue_.pop();
        currentSize_.fetch_sub(1);
        totalConsumed_.fetch_add(1);
        
        condition_.notify_one();
        return std::make_optional(std::move(item));
    }
    
    void finish() {
        finished_.store(true);
        condition_.notify_all();
    }
    
    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_ = std::queue<T>{}; // Очищаем очередь
        finished_.store(false);
        currentSize_.store(0);
    }
    
    // Геттеры для статистики
    size_t size() const { return currentSize_.load(); }
    bool empty() const { return size() == 0; }
    bool isFinished() const { return finished_.load(); }
    size_t getTotalProduced() const { return totalProduced_.load(); }
    size_t getTotalConsumed() const { return totalConsumed_.load(); }
    size_t getTotalBlocked() const { return totalBlocked_.load(); }
    size_t getTotalTimeouts() const { return totalTimeouts_.load(); }
    
    // ✅ C++23: Детальная статистика
    struct DetailedStatistics {
        size_t currentSize;
        size_t totalProduced;
        size_t totalConsumed;
        size_t totalBlocked;
        size_t totalTimeouts;
        double utilizationRate;
        double blockingRate;
        double timeoutRate;
    };
    
    DetailedStatistics getDetailedStatistics() const {
        DetailedStatistics stats;
        stats.currentSize = currentSize_.load();
        stats.totalProduced = totalProduced_.load();
        stats.totalConsumed = totalConsumed_.load();
        stats.totalBlocked = totalBlocked_.load();
        stats.totalTimeouts = totalTimeouts_.load();
        
        if (stats.totalProduced > 0) {
            stats.utilizationRate = static_cast<double>(stats.totalConsumed) / stats.totalProduced;
            stats.blockingRate = static_cast<double>(stats.totalBlocked) / stats.totalProduced;
        } else {
            stats.utilizationRate = 0.0;
            stats.blockingRate = 0.0;
        }
        
        size_t totalAttempts = stats.totalProduced + stats.totalTimeouts;
        if (totalAttempts > 0) {
            stats.timeoutRate = static_cast<double>(stats.totalTimeouts) / totalAttempts;
        } else {
            stats.timeoutRate = 0.0;
        }
        
        return stats;
    }
    
    void printDetailedStatistics() const {
        auto stats = getDetailedStatistics();
        
#ifdef __cpp_lib_print
        std::print("\n=== ДЕТАЛЬНАЯ СТАТИСТИКА ОЧЕРЕДИ ===\n");
        std::print("Текущий размер: {}\n", stats.currentSize);
        std::print("Произведено: {}\n", stats.totalProduced);
        std::print("Потреблено: {}\n", stats.totalConsumed);
        std::print("Заблокировано: {}\n", stats.totalBlocked);
        std::print("Таймаутов: {}\n", stats.totalTimeouts);
        std::print("Коэффициент использования: {:.2f}%\n", stats.utilizationRate * 100);
        std::print("Коэффициент блокировки: {:.2f}%\n", stats.blockingRate * 100);
        std::print("Коэффициент таймаутов: {:.2f}%\n", stats.timeoutRate * 100);
        std::print("=====================================\n");
#else
        std::cout << "\n=== ДЕТАЛЬНАЯ СТАТИСТИКА ОЧЕРЕДИ ===" << std::endl;
        std::cout << "Текущий размер: " << stats.currentSize << std::endl;
        std::cout << "Произведено: " << stats.totalProduced << std::endl;
        std::cout << "Потреблено: " << stats.totalConsumed << std::endl;
        std::cout << "Заблокировано: " << stats.totalBlocked << std::endl;
        std::cout << "Таймаутов: " << stats.totalTimeouts << std::endl;
        std::cout << "Коэффициент использования: " << (stats.utilizationRate * 100) << "%" << std::endl;
        std::cout << "Коэффициент блокировки: " << (stats.blockingRate * 100) << "%" << std::endl;
        std::cout << "Коэффициент таймаутов: " << (stats.timeoutRate * 100) << "%" << std::endl;
        std::cout << "=====================================" << std::endl;
#endif
    }
};

/**
 * @brief Умный Producer для C++23
 */
class SmartProducerCpp23 {
public:
    enum class Strategy {
        LINEAR,      // Линейная последовательность
        RANDOM,      // Случайные числа
        FIBONACCI,   // Числа Фибоначчи
        PRIME        // Простые числа
    };
    
private:
    Strategy strategy_;
    int count_;
    std::mt19937 generator_;
    std::uniform_int_distribution<int> distribution_;
    
public:
    SmartProducerCpp23(Strategy strategy = Strategy::LINEAR, int count = 20, int minVal = 1, int maxVal = 100)
        : strategy_(strategy), count_(count),
          generator_(std::random_device{}()),
          distribution_(minVal, maxVal) {}
    
    template<typename QueueType>
    void operator()(QueueType& queue) {
#ifdef __cpp_lib_print
        std::print("[SmartProducer] Запуск с стратегией: {}\n", static_cast<int>(strategy_));
#else
        std::cout << "[SmartProducer] Запуск с стратегией: " << static_cast<int>(strategy_) << std::endl;
#endif
        
        switch (strategy_) {
            case Strategy::LINEAR:
                produceLinear(queue);
                break;
            case Strategy::RANDOM:
                produceRandom(queue);
                break;
            case Strategy::FIBONACCI:
                produceFibonacci(queue);
                break;
            case Strategy::PRIME:
                producePrime(queue);
                break;
        }
        
#ifdef __cpp_lib_print
        std::print("[SmartProducer] Завершен\n");
#else
        std::cout << "[SmartProducer] Завершен" << std::endl;
#endif
    }
    
private:
    template<typename QueueType>
    void produceLinear(QueueType& queue) {
        for (int i = 1; i <= count_; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            auto result = queue.push(i);
            if (result) {
#ifdef __cpp_lib_print
                std::print("[SmartProducer] Произвел: {}\n", i);
#else
                std::cout << "[SmartProducer] Произвел: " << i << std::endl;
#endif
            } else {
#ifdef __cpp_lib_print
                std::print("[SmartProducer] Ошибка: {}\n", result.error());
#else
                std::cout << "[SmartProducer] Ошибка: " << result.error() << std::endl;
#endif
                break;
            }
        }
    }
    
    template<typename QueueType>
    void produceRandom(QueueType& queue) {
        for (int i = 0; i < count_; ++i) {
            int value = distribution_(generator_);
            std::this_thread::sleep_for(std::chrono::milliseconds(50 + (value % 100)));
            
            auto result = queue.push(value);
            if (result) {
#ifdef __cpp_lib_print
                std::print("[SmartProducer] Произвел: {}\n", value);
#else
                std::cout << "[SmartProducer] Произвел: " << value << std::endl;
#endif
            } else {
#ifdef __cpp_lib_print
                std::print("[SmartProducer] Ошибка: {}\n", result.error());
#else
                std::cout << "[SmartProducer] Ошибка: " << result.error() << std::endl;
#endif
                break;
            }
        }
    }
    
    template<typename QueueType>
    void produceFibonacci(QueueType& queue) {
        int a = 1, b = 1;
        
        auto result1 = queue.push(a);
        if (result1) {
#ifdef __cpp_lib_print
            std::print("[SmartProducer] Произвел: {}\n", a);
#else
            std::cout << "[SmartProducer] Произвел: " << a << std::endl;
#endif
        }
        
        auto result2 = queue.push(b);
        if (result2) {
#ifdef __cpp_lib_print
            std::print("[SmartProducer] Произвел: {}\n", b);
#else
            std::cout << "[SmartProducer] Произвел: " << b << std::endl;
#endif
        }
        
        for (int i = 2; i < count_; ++i) {
            int next = a + b;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            auto result = queue.push(next);
            if (result) {
#ifdef __cpp_lib_print
                std::print("[SmartProducer] Произвел: {}\n", next);
#else
                std::cout << "[SmartProducer] Произвел: " << next << std::endl;
#endif
            } else {
#ifdef __cpp_lib_print
                std::print("[SmartProducer] Ошибка: {}\n", result.error());
#else
                std::cout << "[SmartProducer] Ошибка: " << result.error() << std::endl;
#endif
                break;
            }
            
            a = b;
            b = next;
        }
    }
    
    template<typename QueueType>
    void producePrime(QueueType& queue) {
        int num = 2;
        int found = 0;
        
        while (found < count_) {
            if (isPrime(num)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                
                auto result = queue.push(num);
                if (result) {
#ifdef __cpp_lib_print
                    std::print("[SmartProducer] Произвел: {}\n", num);
#else
                    std::cout << "[SmartProducer] Произвел: " << num << std::endl;
#endif
                } else {
#ifdef __cpp_lib_print
                    std::print("[SmartProducer] Ошибка: {}\n", result.error());
#else
                    std::cout << "[SmartProducer] Ошибка: " << result.error() << std::endl;
#endif
                    break;
                }
                found++;
            }
            num++;
        }
    }
    
    bool isPrime(int n) {
        if (n < 2) return false;
        for (int i = 2; i * i <= n; ++i) {
            if (n % i == 0) return false;
        }
        return true;
    }
};

/**
 * @brief Умный Consumer для C++23
 */
class SmartConsumerCpp23 {
public:
    enum class Strategy {
        SIMPLE,      // Простая обработка
        STATISTICS,  // Сбор статистики
        FILTER,      // Фильтрация данных
        TRANSFORM    // Преобразование данных
    };
    
private:
    Strategy strategy_;
    std::vector<int> processedData_;
    std::mutex dataMutex_;
    
public:
    SmartConsumerCpp23(Strategy strategy = Strategy::SIMPLE) : strategy_(strategy) {}
    
    template<typename QueueType>
    void operator()(QueueType& queue) {
#ifdef __cpp_lib_print
        std::print("[SmartConsumer] Запуск с стратегией: {}\n", static_cast<int>(strategy_));
#else
        std::cout << "[SmartConsumer] Запуск с стратегией: " << static_cast<int>(strategy_) << std::endl;
#endif
        
        while (true) {
            auto item = queue.pop(std::chrono::milliseconds(100));
            if (!item) {
                break; // Очередь завершена или таймаут
            }
            
            processItem(*item);
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
        
        printResults();
    }
    
private:
    void processItem(int item) {
        std::lock_guard<std::mutex> lock(dataMutex_);
        processedData_.push_back(item);
        
        switch (strategy_) {
            case Strategy::SIMPLE:
#ifdef __cpp_lib_print
                std::print("[SmartConsumer] Обработал: {}\n", item);
#else
                std::cout << "[SmartConsumer] Обработал: " << item << std::endl;
#endif
                break;
            case Strategy::STATISTICS:
#ifdef __cpp_lib_print
                std::print("[SmartConsumer] Добавил в статистику: {}\n", item);
#else
                std::cout << "[SmartConsumer] Добавил в статистику: " << item << std::endl;
#endif
                break;
            case Strategy::FILTER:
                if (item % 2 == 0) {
#ifdef __cpp_lib_print
                    std::print("[SmartConsumer] Четное число: {}\n", item);
#else
                    std::cout << "[SmartConsumer] Четное число: " << item << std::endl;
#endif
                } else {
#ifdef __cpp_lib_print
                    std::print("[SmartConsumer] Нечетное число: {}\n", item);
#else
                    std::cout << "[SmartConsumer] Нечетное число: " << item << std::endl;
#endif
                }
                break;
            case Strategy::TRANSFORM:
                int transformed = item * item;
#ifdef __cpp_lib_print
                std::print("[SmartConsumer] {} -> {}\n", item, transformed);
#else
                std::cout << "[SmartConsumer] " << item << " -> " << transformed << std::endl;
#endif
                break;
        }
    }
    
    void printResults() {
        std::lock_guard<std::mutex> lock(dataMutex_);
        
        if (processedData_.empty()) {
#ifdef __cpp_lib_print
            std::print("[SmartConsumer] Нет данных для результатов\n");
#else
            std::cout << "[SmartConsumer] Нет данных для результатов" << std::endl;
#endif
            return;
        }
        
#ifdef __cpp_lib_print
        std::print("\n=== РЕЗУЛЬТАТЫ ОБРАБОТКИ ===\n");
        std::print("Обработано элементов: {}\n", processedData_.size());
#else
        std::cout << "\n=== РЕЗУЛЬТАТЫ ОБРАБОТКИ ===" << std::endl;
        std::cout << "Обработано элементов: " << processedData_.size() << std::endl;
#endif
        
        if (strategy_ == Strategy::STATISTICS) {
            std::sort(processedData_.begin(), processedData_.end());
            
            int sum = 0;
            for (int value : processedData_) {
                sum += value;
            }
            
            double mean = static_cast<double>(sum) / processedData_.size();
            int median = processedData_[processedData_.size() / 2];
            int min = processedData_.front();
            int max = processedData_.back();
            
#ifdef __cpp_lib_print
            std::print("Минимум: {}\n", min);
            std::print("Максимум: {}\n", max);
            std::print("Среднее: {:.2f}\n", mean);
            std::print("Медиана: {}\n", median);
#else
            std::cout << "Минимум: " << min << std::endl;
            std::cout << "Максимум: " << max << std::endl;
            std::cout << "Среднее: " << mean << std::endl;
            std::cout << "Медиана: " << median << std::endl;
#endif
        }
        
#ifdef __cpp_lib_print
        std::print("=============================\n");
#else
        std::cout << "=============================" << std::endl;
#endif
    }
};

// ============================================================================
// ДЕМОНСТРАЦИЯ РАЗЛИЧИЙ
// ============================================================================

void demonstrateDifferences() {
    std::cout << "\n=== СРАВНЕНИЕ PRODUCER-CONSUMER PATTERN: C++17/20 vs C++23 ===" << std::endl;
    
    // ============================================================================
    // C++17/20 ДЕМОНСТРАЦИЯ
    // ============================================================================
    std::cout << "\n--- C++17/20 Producer-Consumer ---" << std::endl;
    
    ProducerConsumerQueueCpp20<int> queue20(5);
    
    ProducerCpp20 producer20(queue20, "BasicProducer", 1, 8);
    ConsumerCpp20 consumer20(queue20, "BasicConsumer");
    
    // Запускаем потоки
    std::thread producerThread20(&ProducerCpp20::produce, &producer20);
    std::thread consumerThread20(&ConsumerCpp20::consume, &consumer20);
    
    // Ждем завершения producer
    producerThread20.join();
    
    // Завершаем очередь
    queue20.finish();
    
    // Ждем завершения consumer
    consumerThread20.join();
    
    std::cout << "C++17/20 Producer-Consumer завершен" << std::endl;
    
    // ============================================================================
    // C++23 ДЕМОНСТРАЦИЯ
    // ============================================================================
    std::cout << "\n--- C++23 Producer-Consumer ---" << std::endl;
    
    ProducerConsumerQueueCpp23<int> queue23(5);
    
    // ✅ C++23: Умные Producer и Consumer с различными стратегиями
    SmartProducerCpp23 producer23(SmartProducerCpp23::Strategy::FIBONACCI, 8);
    SmartConsumerCpp23 consumer23(SmartConsumerCpp23::Strategy::STATISTICS);
    
    // Запускаем потоки
    std::thread producerThread23([&]() { producer23(queue23); });
    std::thread consumerThread23([&]() { consumer23(queue23); });
    
    // Ждем завершения producer
    producerThread23.join();
    
    // Завершаем очередь
    queue23.finish();
    
    // Ждем завершения consumer
    consumerThread23.join();
    
    // ✅ C++23: Детальная статистика
    queue23.printDetailedStatistics();
    
    std::cout << "C++23 Producer-Consumer завершен" << std::endl;
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🏭 Сравнение Producer-Consumer Pattern: C++17/20 vs C++23" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    
    try {
        demonstrateDifferences();
        
        std::cout << "\n✅ Сравнение завершено!" << std::endl;
        
        std::cout << "\n🎯 Ключевые улучшения C++23:" << std::endl;
        std::cout << "1. std::expected - четкая обработка ошибок при push/pop" << std::endl;
        std::cout << "2. std::optional - безопасное извлечение с tryPop" << std::endl;
        std::cout << "3. std::print - более читаемый вывод" << std::endl;
        std::cout << "4. std::generator - ленивая генерация данных" << std::endl;
        std::cout << "5. Расширенная статистика и мониторинг" << std::endl;
        std::cout << "6. Умные стратегии для Producer и Consumer" << std::endl;
        std::cout << "7. Детальная статистика с коэффициентами" << std::endl;
        std::cout << "8. Timeout поддержка для pop операций" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}


