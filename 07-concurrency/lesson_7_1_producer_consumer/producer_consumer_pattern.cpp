#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <vector>
#include <random>
#include <memory>

/**
 * @file producer_consumer_pattern.cpp
 * @brief Демонстрация паттерна Producer-Consumer
 * 
 * Этот файл показывает различные реализации паттерна Producer-Consumer,
 * от базовой до продвинутых версий с множественными producer/consumer.
 */

// ============================================================================
// БАЗОВАЯ ОЧЕРЕДЬ PRODUCER-CONSUMER
// ============================================================================

/**
 * @brief Базовая thread-safe очередь для Producer-Consumer
 * 
 * Особенности:
 * - Thread-safe операции push/pop
 * - Условные переменные для эффективного ожидания
 * - Поддержка завершения работы
 */
template<typename T>
class ProducerConsumerQueue {
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    bool finished_ = false;
    size_t maxSize_ = 0; // 0 = без ограничений
    
public:
    explicit ProducerConsumerQueue(size_t maxSize = 0) : maxSize_(maxSize) {}
    
    /**
     * @brief Добавляет элемент в очередь
     * @param item Элемент для добавления
     * @return true если элемент добавлен, false если очередь полная
     */
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
    
    /**
     * @brief Извлекает элемент из очереди
     * @param item Ссылка для сохранения извлеченного элемента
     * @return true если элемент извлечен, false если очередь пустая и завершена
     */
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
    
    /**
     * @brief Завершает работу очереди
     */
    void finish() {
        std::lock_guard<std::mutex> lock(mutex_);
        finished_ = true;
        condition_.notify_all(); // Уведомляем все ожидающие потоки
    }
    
    /**
     * @brief Проверяет, завершена ли работа
     */
    bool isFinished() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return finished_;
    }
    
    /**
     * @brief Возвращает текущий размер очереди
     */
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
    
    /**
     * @brief Проверяет, пуста ли очередь
     */
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
};

// ============================================================================
// PRODUCER (ПРОИЗВОДИТЕЛЬ)
// ============================================================================

/**
 * @brief Производитель данных для демонстрации
 * 
 * Создает последовательность чисел с задержкой,
 * имитируя реальную работу по производству данных.
 */
class DataProducer {
private:
    ProducerConsumerQueue<int>& queue_;
    int startValue_;
    int count_;
    std::chrono::milliseconds delay_;
    std::string name_;
    
public:
    DataProducer(ProducerConsumerQueue<int>& queue, 
                 const std::string& name = "Producer",
                 int startValue = 1, 
                 int count = 10,
                 std::chrono::milliseconds delay = std::chrono::milliseconds(100))
        : queue_(queue), name_(name), startValue_(startValue), count_(count), delay_(delay) {}
    
    /**
     * @brief Запускает производство данных
     */
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

// ============================================================================
// CONSUMER (ПОТРЕБИТЕЛЬ)
// ============================================================================

/**
 * @brief Потребитель данных для демонстрации
 * 
 * Обрабатывает данные из очереди с задержкой,
 * имитируя реальную работу по обработке данных.
 */
class DataConsumer {
private:
    ProducerConsumerQueue<int>& queue_;
    std::chrono::milliseconds delay_;
    std::string name_;
    
public:
    DataConsumer(ProducerConsumerQueue<int>& queue,
                 const std::string& name = "Consumer",
                 std::chrono::milliseconds delay = std::chrono::milliseconds(150))
        : queue_(queue), name_(name), delay_(delay) {}
    
    /**
     * @brief Запускает потребление данных
     */
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
    /**
     * @brief Обрабатывает один элемент данных
     */
    int processItem(int item) {
        // Простая обработка - возведение в квадрат
        return item * item;
    }
};

// ============================================================================
// ПРОДВИНУТЫЕ ПРИМЕРЫ
// ============================================================================

/**
 * @brief Производитель случайных данных
 */
class RandomProducer {
private:
    ProducerConsumerQueue<int>& queue_;
    int count_;
    std::mt19937 generator_;
    std::uniform_int_distribution<int> distribution_;
    
public:
    RandomProducer(ProducerConsumerQueue<int>& queue, int count, int minVal = 1, int maxVal = 100)
        : queue_(queue), count_(count), 
          generator_(std::random_device{}()),
          distribution_(minVal, maxVal) {}
    
    void produce() {
        std::cout << "[RandomProducer] Производю " << count_ << " случайных чисел..." << std::endl;
        
        for (int i = 0; i < count_; ++i) {
            int value = distribution_(generator_);
            
            // Случайная задержка
            std::this_thread::sleep_for(std::chrono::milliseconds(50 + (value % 100)));
            
            if (queue_.push(value)) {
                std::cout << "[RandomProducer] Произвел: " << value << std::endl;
            } else {
                break;
            }
        }
        
        std::cout << "[RandomProducer] Завершил производство" << std::endl;
    }
};

/**
 * @brief Статистический потребитель
 */
class StatisticsConsumer {
private:
    ProducerConsumerQueue<int>& queue_;
    std::vector<int> processedData_;
    
public:
    StatisticsConsumer(ProducerConsumerQueue<int>& queue) : queue_(queue) {}
    
    void consume() {
        std::cout << "[StatisticsConsumer] Начинаю сбор статистики..." << std::endl;
        
        int item;
        while (queue_.pop(item)) {
            processedData_.push_back(item);
            
            // Имитация обработки
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            std::cout << "[StatisticsConsumer] Получил: " << item << std::endl;
        }
        
        printStatistics();
    }
    
private:
    void printStatistics() {
        if (processedData_.empty()) {
            std::cout << "[StatisticsConsumer] Нет данных для статистики" << std::endl;
            return;
        }
        
        // Вычисляем статистику
        std::sort(processedData_.begin(), processedData_.end());
        
        int sum = 0;
        for (int value : processedData_) {
            sum += value;
        }
        
        double mean = static_cast<double>(sum) / processedData_.size();
        int median = processedData_[processedData_.size() / 2];
        int min = processedData_.front();
        int max = processedData_.back();
        
        std::cout << "\n=== СТАТИСТИКА ===" << std::endl;
        std::cout << "Количество элементов: " << processedData_.size() << std::endl;
        std::cout << "Минимум: " << min << std::endl;
        std::cout << "Максимум: " << max << std::endl;
        std::cout << "Среднее: " << mean << std::endl;
        std::cout << "Медиана: " << median << std::endl;
        std::cout << "==================" << std::endl;
    }
};

// ============================================================================
// ДЕМОНСТРАЦИОННЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Демонстрация базового Producer-Consumer
 */
void demonstrateBasicProducerConsumer() {
    std::cout << "\n=== БАЗОВЫЙ PRODUCER-CONSUMER ===" << std::endl;
    
    ProducerConsumerQueue<int> queue(5); // Максимум 5 элементов
    
    DataProducer producer(queue, "BasicProducer", 1, 8);
    DataConsumer consumer(queue, "BasicConsumer");
    
    // Запускаем потоки
    std::thread producerThread(&DataProducer::produce, &producer);
    std::thread consumerThread(&DataConsumer::consume, &consumer);
    
    // Ждем завершения producer
    producerThread.join();
    
    // Завершаем очередь
    queue.finish();
    
    // Ждем завершения consumer
    consumerThread.join();
    
    std::cout << "Базовый Producer-Consumer завершен" << std::endl;
}

/**
 * @brief Демонстрация множественных Producer-Consumer
 */
void demonstrateMultipleProducerConsumer() {
    std::cout << "\n=== МНОЖЕСТВЕННЫЕ PRODUCER-CONSUMER ===" << std::endl;
    
    ProducerConsumerQueue<int> queue(10);
    
    // Создаем несколько producer'ов
    DataProducer producer1(queue, "Producer1", 1, 5);
    DataProducer producer2(queue, "Producer2", 100, 5);
    DataProducer producer3(queue, "Producer3", 200, 5);
    
    // Создаем несколько consumer'ов
    DataConsumer consumer1(queue, "Consumer1");
    DataConsumer consumer2(queue, "Consumer2");
    
    // Запускаем все потоки
    std::thread t1(&DataProducer::produce, &producer1);
    std::thread t2(&DataProducer::produce, &producer2);
    std::thread t3(&DataProducer::produce, &producer3);
    std::thread t4(&DataConsumer::consume, &consumer1);
    std::thread t5(&DataConsumer::consume, &consumer2);
    
    // Ждем завершения всех producer'ов
    t1.join();
    t2.join();
    t3.join();
    
    // Завершаем очередь
    queue.finish();
    
    // Ждем завершения всех consumer'ов
    t4.join();
    t5.join();
    
    std::cout << "Множественные Producer-Consumer завершены" << std::endl;
}

/**
 * @brief Демонстрация Producer-Consumer со статистикой
 */
void demonstrateStatisticsProducerConsumer() {
    std::cout << "\n=== PRODUCER-CONSUMER СО СТАТИСТИКОЙ ===" << std::endl;
    
    ProducerConsumerQueue<int> queue;
    
    RandomProducer producer(queue, 20, 1, 50);
    StatisticsConsumer consumer(queue);
    
    // Запускаем потоки
    std::thread producerThread(&RandomProducer::produce, &producer);
    std::thread consumerThread(&StatisticsConsumer::consume, &consumer);
    
    // Ждем завершения producer
    producerThread.join();
    
    // Завершаем очередь
    queue.finish();
    
    // Ждем завершения consumer
    consumerThread.join();
    
    std::cout << "Producer-Consumer со статистикой завершен" << std::endl;
}

/**
 * @brief Демонстрация производительности
 */
void demonstratePerformance() {
    std::cout << "\n=== ТЕСТ ПРОИЗВОДИТЕЛЬНОСТИ ===" << std::endl;
    
    const int NUM_ITEMS = 1000;
    ProducerConsumerQueue<int> queue(100); // Буфер на 100 элементов
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Быстрый producer
    std::thread producer([&]() {
        for (int i = 0; i < NUM_ITEMS; ++i) {
            queue.push(i);
        }
        queue.finish();
    });
    
    // Быстрый consumer
    std::thread consumer([&]() {
        int item;
        int count = 0;
        while (queue.pop(item)) {
            count++;
        }
        std::cout << "Обработано элементов: " << count << std::endl;
    });
    
    producer.join();
    consumer.join();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Обработано " << NUM_ITEMS << " элементов за " 
              << duration.count() << " мс" << std::endl;
    std::cout << "Производительность: " 
              << (NUM_ITEMS * 1000.0 / duration.count()) << " элементов/сек" << std::endl;
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🏭 Демонстрация паттерна Producer-Consumer" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    try {
        demonstrateBasicProducerConsumer();
        demonstrateMultipleProducerConsumer();
        demonstrateStatisticsProducerConsumer();
        demonstratePerformance();
        
        std::cout << "\n✅ Все демонстрации завершены успешно!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n🎯 Рекомендации:" << std::endl;
    std::cout << "1. Используйте ограниченные очереди для контроля памяти" << std::endl;
    std::cout << "2. Правильно завершайте работу очереди" << std::endl;
    std::cout << "3. Мониторьте производительность и размер очереди" << std::endl;
    std::cout << "4. Рассмотрите lock-free реализации для критичных участков" << std::endl;
    std::cout << "5. Тестируйте многопоточность тщательно" << std::endl;
    
    return 0;
}
