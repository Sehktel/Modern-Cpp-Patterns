#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <vector>
#include <future>
#include <optional>
#include <variant>
#include <any>
#include <functional>
#include <atomic>
#include <memory>
#include <random>
#include <algorithm>

/**
 * @file modern_producer_consumer.cpp
 * @brief Современные реализации паттерна Producer-Consumer с использованием C++17/20
 * 
 * Этот файл демонстрирует продвинутые техники и современные подходы
 * к реализации Producer-Consumer паттерна.
 */

// ============================================================================
// СОВРЕМЕННАЯ ОЧЕРЕДЬ С VARIANT И OPTIONAL
// ============================================================================

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
 * @brief Современная thread-safe очередь с поддержкой различных типов сообщений
 */
template<typename T>
class ModernProducerConsumerQueue {
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic<bool> finished_{false};
    std::atomic<size_t> maxSize_{0};
    std::atomic<size_t> currentSize_{0};
    
    // Статистика
    mutable std::mutex statsMutex_;
    std::atomic<size_t> totalProduced_{0};
    std::atomic<size_t> totalConsumed_{0};
    std::atomic<size_t> totalBlocked_{0};
    
public:
    explicit ModernProducerConsumerQueue(size_t maxSize = 0) : maxSize_(maxSize) {}
    
    /**
     * @brief Добавляет элемент в очередь с move semantics
     */
    template<typename U>
    bool push(U&& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Проверяем ограничение размера
        if (maxSize_.load() > 0 && currentSize_.load() >= maxSize_.load()) {
            totalBlocked_.fetch_add(1);
            condition_.wait(lock, [this] { 
                return currentSize_.load() < maxSize_.load() || finished_.load(); 
            });
        }
        
        if (finished_.load()) return false;
        
        queue_.push(std::forward<U>(item));
        currentSize_.fetch_add(1);
        totalProduced_.fetch_add(1);
        
        condition_.notify_one();
        return true;
    }
    
    /**
     * @brief Извлекает элемент с optional возвратом
     */
    std::optional<T> pop(std::chrono::milliseconds timeout = std::chrono::milliseconds::max()) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        bool success = condition_.wait_for(lock, timeout, [this] { 
            return !queue_.empty() || finished_.load(); 
        });
        
        if (!success || queue_.empty()) {
            return std::nullopt;
        }
        
        T item = std::move(queue_.front());
        queue_.pop();
        currentSize_.fetch_sub(1);
        totalConsumed_.fetch_add(1);
        
        condition_.notify_one();
        return std::make_optional(std::move(item));
    }
    
    /**
     * @brief Попытка извлечения без блокировки
     */
    std::optional<T> tryPop() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (queue_.empty()) {
            return std::nullopt;
        }
        
        T item = std::move(queue_.front());
        queue_.pop();
        currentSize_.fetch_sub(1);
        totalConsumed_.fetch_add(1);
        
        return std::make_optional(std::move(item));
    }
    
    /**
     * @brief Завершает работу очереди
     */
    void finish() {
        finished_.store(true);
        condition_.notify_all();
    }
    
    /**
     * @brief Сбрасывает состояние очереди
     */
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
    
    /**
     * @brief Получает подробную статистику
     */
    struct Statistics {
        size_t currentSize;
        size_t totalProduced;
        size_t totalConsumed;
        size_t totalBlocked;
        double utilizationRate;
    };
    
    Statistics getStatistics() const {
        Statistics stats;
        stats.currentSize = currentSize_.load();
        stats.totalProduced = totalProduced_.load();
        stats.totalConsumed = totalConsumed_.load();
        stats.totalBlocked = totalBlocked_.load();
        
        if (stats.totalProduced > 0) {
            stats.utilizationRate = static_cast<double>(stats.totalConsumed) / stats.totalProduced;
        } else {
            stats.utilizationRate = 0.0;
        }
        
        return stats;
    }
};

// ============================================================================
// ASYNC PRODUCER-CONSUMER
// ============================================================================

/**
 * @brief Асинхронный Producer-Consumer с использованием std::async
 */
template<typename T>
class AsyncProducerConsumer {
private:
    ModernProducerConsumerQueue<T> queue_;
    std::vector<std::future<void>> producerFutures_;
    std::vector<std::future<void>> consumerFutures_;
    
public:
    explicit AsyncProducerConsumer(size_t maxSize = 0) : queue_(maxSize) {}
    
    /**
     * @brief Запускает асинхронного producer'а
     */
    template<typename ProducerFunc>
    void startProducer(ProducerFunc&& producerFunc) {
        producerFutures_.emplace_back(
            std::async(std::launch::async, [this, func = std::forward<ProducerFunc>(producerFunc)]() {
                func(queue_);
            })
        );
    }
    
    /**
     * @brief Запускает асинхронного consumer'а
     */
    template<typename ConsumerFunc>
    void startConsumer(ConsumerFunc&& consumerFunc) {
        consumerFutures_.emplace_back(
            std::async(std::launch::async, [this, func = std::forward<ConsumerFunc>(consumerFunc)]() {
                func(queue_);
            })
        );
    }
    
    /**
     * @brief Ждет завершения всех потоков
     */
    void waitForCompletion() {
        // Завершаем очередь
        queue_.finish();
        
        // Ждем завершения всех producer'ов
        for (auto& future : producerFutures_) {
            if (future.valid()) {
                future.wait();
            }
        }
        
        // Ждем завершения всех consumer'ов
        for (auto& future : consumerFutures_) {
            if (future.valid()) {
                future.wait();
            }
        }
    }
    
    /**
     * @brief Получает статистику
     */
    auto getStatistics() const {
        return queue_.getStatistics();
    }
};

// ============================================================================
// ПРОДВИНУТЫЕ PRODUCER И CONSUMER
// ============================================================================

/**
 * @brief Умный Producer с различными стратегиями
 */
class SmartProducer {
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
    SmartProducer(Strategy strategy = Strategy::LINEAR, int count = 20, int minVal = 1, int maxVal = 100)
        : strategy_(strategy), count_(count),
          generator_(std::random_device{}()),
          distribution_(minVal, maxVal) {}
    
    template<typename QueueType>
    void operator()(QueueType& queue) {
        std::cout << "[SmartProducer] Запуск с стратегией: " << static_cast<int>(strategy_) << std::endl;
        
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
        
        std::cout << "[SmartProducer] Завершен" << std::endl;
    }
    
private:
    template<typename QueueType>
    void produceLinear(QueueType& queue) {
        for (int i = 1; i <= count_; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            queue.push(i);
            std::cout << "[SmartProducer] Произвел: " << i << std::endl;
        }
    }
    
    template<typename QueueType>
    void produceRandom(QueueType& queue) {
        for (int i = 0; i < count_; ++i) {
            int value = distribution_(generator_);
            std::this_thread::sleep_for(std::chrono::milliseconds(50 + (value % 100)));
            queue.push(value);
            std::cout << "[SmartProducer] Произвел: " << value << std::endl;
        }
    }
    
    template<typename QueueType>
    void produceFibonacci(QueueType& queue) {
        int a = 1, b = 1;
        queue.push(a);
        queue.push(b);
        
        for (int i = 2; i < count_; ++i) {
            int next = a + b;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            queue.push(next);
            std::cout << "[SmartProducer] Произвел: " << next << std::endl;
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
                queue.push(num);
                std::cout << "[SmartProducer] Произвел: " << num << std::endl;
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
 * @brief Умный Consumer с различными стратегиями обработки
 */
class SmartConsumer {
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
    SmartConsumer(Strategy strategy = Strategy::SIMPLE) : strategy_(strategy) {}
    
    template<typename QueueType>
    void operator()(QueueType& queue) {
        std::cout << "[SmartConsumer] Запуск с стратегией: " << static_cast<int>(strategy_) << std::endl;
        
        while (true) {
            auto item = queue.pop(std::chrono::milliseconds(100));
            if (!item) {
                break; // Очередь завершена
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
                std::cout << "[SmartConsumer] Обработал: " << item << std::endl;
                break;
            case Strategy::STATISTICS:
                std::cout << "[SmartConsumer] Добавил в статистику: " << item << std::endl;
                break;
            case Strategy::FILTER:
                if (item % 2 == 0) {
                    std::cout << "[SmartConsumer] Четное число: " << item << std::endl;
                } else {
                    std::cout << "[SmartConsumer] Нечетное число: " << item << std::endl;
                }
                break;
            case Strategy::TRANSFORM:
                int transformed = item * item;
                std::cout << "[SmartConsumer] " << item << " -> " << transformed << std::endl;
                break;
        }
    }
    
    void printResults() {
        std::lock_guard<std::mutex> lock(dataMutex_);
        
        if (processedData_.empty()) {
            std::cout << "[SmartConsumer] Нет данных для результатов" << std::endl;
            return;
        }
        
        std::cout << "\n=== РЕЗУЛЬТАТЫ ОБРАБОТКИ ===" << std::endl;
        std::cout << "Обработано элементов: " << processedData_.size() << std::endl;
        
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
            
            std::cout << "Минимум: " << min << std::endl;
            std::cout << "Максимум: " << max << std::endl;
            std::cout << "Среднее: " << mean << std::endl;
            std::cout << "Медиана: " << median << std::endl;
        }
        
        std::cout << "=============================" << std::endl;
    }
};

// ============================================================================
// ДЕМОНСТРАЦИОННЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Демонстрация современной очереди
 */
void demonstrateModernQueue() {
    std::cout << "\n=== СОВРЕМЕННАЯ ОЧЕРЕДЬ ===" << std::endl;
    
    ModernProducerConsumerQueue<int> queue(5);
    
    // Тестируем различные операции
    std::cout << "Тестируем push/pop..." << std::endl;
    
    for (int i = 1; i <= 10; ++i) {
        bool pushed = queue.push(i);
        std::cout << "Push " << i << ": " << (pushed ? "OK" : "FAILED") 
                  << " (размер: " << queue.size() << ")" << std::endl;
        
        if (i % 3 == 0) {
            auto item = queue.tryPop();
            if (item) {
                std::cout << "Pop: " << *item << " (размер: " << queue.size() << ")" << std::endl;
            }
        }
    }
    
    // Завершаем очередь
    queue.finish();
    
    // Извлекаем оставшиеся элементы
    std::cout << "Извлекаем оставшиеся элементы..." << std::endl;
    while (auto item = queue.pop(std::chrono::milliseconds(100))) {
        std::cout << "Pop: " << *item << std::endl;
    }
    
    // Выводим статистику
    auto stats = queue.getStatistics();
    std::cout << "\n=== СТАТИСТИКА ===" << std::endl;
    std::cout << "Произведено: " << stats.totalProduced << std::endl;
    std::cout << "Потреблено: " << stats.totalConsumed << std::endl;
    std::cout << "Заблокировано: " << stats.totalBlocked << std::endl;
    std::cout << "Коэффициент использования: " << stats.utilizationRate << std::endl;
    std::cout << "==================" << std::endl;
}

/**
 * @brief Демонстрация асинхронного Producer-Consumer
 */
void demonstrateAsyncProducerConsumer() {
    std::cout << "\n=== АСИНХРОННЫЙ PRODUCER-CONSUMER ===" << std::endl;
    
    AsyncProducerConsumer<int> asyncPC(10);
    
    // Запускаем несколько producer'ов с разными стратегиями
    asyncPC.startProducer(SmartProducer(SmartProducer::Strategy::LINEAR, 5));
    asyncPC.startProducer(SmartProducer(SmartProducer::Strategy::RANDOM, 5, 1, 50));
    asyncPC.startProducer(SmartProducer(SmartProducer::Strategy::FIBONACCI, 5));
    
    // Запускаем несколько consumer'ов с разными стратегиями
    asyncPC.startConsumer(SmartConsumer(SmartConsumer::Strategy::SIMPLE));
    asyncPC.startConsumer(SmartConsumer(SmartConsumer::Strategy::STATISTICS));
    asyncPC.startConsumer(SmartConsumer(SmartConsumer::Strategy::FILTER));
    
    // Ждем завершения
    asyncPC.waitForCompletion();
    
    // Выводим финальную статистику
    auto stats = asyncPC.getStatistics();
    std::cout << "\n=== ФИНАЛЬНАЯ СТАТИСТИКА ===" << std::endl;
    std::cout << "Произведено: " << stats.totalProduced << std::endl;
    std::cout << "Потреблено: " << stats.totalConsumed << std::endl;
    std::cout << "Заблокировано: " << stats.totalBlocked << std::endl;
    std::cout << "Коэффициент использования: " << stats.utilizationRate << std::endl;
    std::cout << "=============================" << std::endl;
}

/**
 * @brief Демонстрация работы с сообщениями
 */
void demonstrateMessageQueue() {
    std::cout << "\n=== ОЧЕРЕДЬ СООБЩЕНИЙ ===" << std::endl;
    
    ModernProducerConsumerQueue<Message> messageQueue(20);
    
    // Producer сообщений
    std::thread producer([&]() {
        for (int i = 0; i < 10; ++i) {
            if (i == 0) {
                // Отправляем управляющее сообщение
                ControlMessage startMsg{ControlMessage::Type::START, "Начало работы"};
                messageQueue.push(startMsg);
            }
            
            // Отправляем данные
            DataMessage dataMsg{
                i,
                "Сообщение " + std::to_string(i),
                std::chrono::system_clock::now()
            };
            messageQueue.push(dataMsg);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        
        // Завершаем
        ControlMessage stopMsg{ControlMessage::Type::STOP, "Завершение работы"};
        messageQueue.push(stopMsg);
        messageQueue.finish();
    });
    
    // Consumer сообщений
    std::thread consumer([&]() {
        while (true) {
            auto msg = messageQueue.pop(std::chrono::milliseconds(100));
            if (!msg) break;
            
            std::visit([](const auto& message) {
                using T = std::decay_t<decltype(message)>;
                if constexpr (std::is_same_v<T, DataMessage>) {
                    std::cout << "[MessageConsumer] Данные: ID=" << message.id 
                              << ", Content=" << message.content << std::endl;
                } else if constexpr (std::is_same_v<T, ControlMessage>) {
                    std::cout << "[MessageConsumer] Управление: " 
                              << static_cast<int>(message.type) << " - " << message.reason << std::endl;
                }
            }, *msg);
        }
    });
    
    producer.join();
    consumer.join();
    
    std::cout << "Очередь сообщений завершена" << std::endl;
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🚀 Современный Producer-Consumer Pattern" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    try {
        demonstrateModernQueue();
        demonstrateAsyncProducerConsumer();
        demonstrateMessageQueue();
        
        std::cout << "\n✅ Все современные демонстрации завершены!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n🎯 Современные возможности:" << std::endl;
    std::cout << "1. Используйте std::optional для безопасного извлечения" << std::endl;
    std::cout << "2. Применяйте std::variant для различных типов сообщений" << std::endl;
    std::cout << "3. Используйте std::async для асинхронных операций" << std::endl;
    std::cout << "4. Применяйте атомарные операции для статистики" << std::endl;
    std::cout << "5. Используйте move semantics для эффективности" << std::endl;
    
    return 0;
}
