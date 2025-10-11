#include <iostream>
#include <queue>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <cstring>

/**
 * @file producer_consumer_vulnerabilities.cpp
 * @brief Уязвимые реализации паттерна Producer-Consumer
 * 
 * ⚠️  ТОЛЬКО ДЛЯ ОБРАЗОВАТЕЛЬНЫХ ЦЕЛЕЙ!
 * Демонстрирует опасные практики в многопоточном программировании.
 */

// ============================================================================
// УЯЗВИМОСТЬ 1: RACE CONDITION - НЕТ СИНХРОНИЗАЦИИ
// Проблема: Доступ к shared queue без мьютекса
// ============================================================================

template<typename T>
class UnsafeQueue {
private:
    std::queue<T> queue_;  // НЕТ МЬЮТЕКСА!
    
public:
    void push(const T& item) {
        queue_.push(item);  // RACE CONDITION!
    }
    
    bool pop(T& item) {
        if (queue_.empty()) return false;
        item = queue_.front();  // RACE CONDITION!
        queue_.pop();          // RACE CONDITION!
        return true;
    }
    
    size_t size() const {
        return queue_.size();  // RACE CONDITION!
    }
};

void demonstrateRaceCondition() {
    std::cout << "\n=== УЯЗВИМОСТЬ 1: Race Condition ===\n";
    
    UnsafeQueue<int> queue;
    std::atomic<int> produced{0};
    std::atomic<int> consumed{0};
    
    // Producer: добавляет 10000 элементов
    std::thread producer([&queue, &produced]() {
        for (int i = 0; i < 10000; ++i) {
            queue.push(i);  // RACE CONDITION
            produced++;
        }
    });
    
    // Consumer: извлекает элементы
    std::thread consumer([&queue, &consumed]() {
        int item;
        for (int i = 0; i < 10000; ++i) {
            while (!queue.pop(item)) {  // RACE CONDITION
                std::this_thread::yield();
            }
            consumed++;
        }
    });
    
    producer.join();
    consumer.join();
    
    std::cout << "Произведено: " << produced << "\n";
    std::cout << "Потреблено: " << consumed << "\n";
    std::cout << "Осталось в очереди: " << queue.size() << "\n";
    std::cout << "⚠️  Результаты могут быть некорректны из-за data race!\n";
}

// ============================================================================
// УЯЗВИМОСТЬ 2: BUFFER OVERFLOW В ФИКСИРОВАННОМ БУФЕРЕ
// Проблема: Нет проверки границ при записи в циклический буфер
// ============================================================================

template<typename T, size_t N>
class VulnerableRingBuffer {
private:
    T buffer_[N];
    size_t head_ = 0;  // Индекс для записи
    size_t tail_ = 0;  // Индекс для чтения
    // НЕТ МЬЮТЕКСА и НЕТ ПРОВЕРКИ ПЕРЕПОЛНЕНИЯ!
    
public:
    void push(const T& item) {
        buffer_[head_] = item;
        head_ = (head_ + 1) % N;
        // ОПАСНО: может перезаписать непрочитанные данные!
    }
    
    bool pop(T& item) {
        if (head_ == tail_) return false;  // Пустая очередь
        
        item = buffer_[tail_];
        tail_ = (tail_ + 1) % N;
        return true;
    }
};

void demonstrateBufferOverflow() {
    std::cout << "\n=== УЯЗВИМОСТЬ 2: Buffer Overflow ===\n";
    
    VulnerableRingBuffer<int, 10> buffer;
    
    // Producer перезаписывает буфер
    std::thread producer([&buffer]() {
        for (int i = 0; i < 100; ++i) {  // Пишем 100 элементов в буфер на 10
            buffer.push(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    
    // Consumer читает медленнее
    std::thread consumer([&buffer]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));  // Задержка
        
        int item;
        int count = 0;
        while (buffer.pop(item)) {
            std::cout << "Получено: " << item << "\n";
            count++;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        std::cout << "Всего получено: " << count << "\n";
    });
    
    producer.join();
    consumer.join();
    
    std::cout << "⚠️  Данные потеряны из-за переполнения буфера!\n";
}

// ============================================================================
// УЯЗВИМОСТЬ 3: DEADLOCK ПРИ НЕПРАВИЛЬНОМ ИСПОЛЬЗОВАНИИ CONDITION VARIABLES
// Проблема: Producer и Consumer ждут друг друга
// ============================================================================

class DeadlockQueue {
private:
    std::queue<int> queue_;
    std::mutex mutex_;
    std::condition_variable cv_producer_;  // Для producer
    std::condition_variable cv_consumer_;  // Для consumer
    size_t maxSize_ = 10;
    bool finished_ = false;
    
public:
    void push(int item) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // ОПАСНО: Ждем, но можем пропустить notify
        while (queue_.size() >= maxSize_ && !finished_) {
            cv_producer_.wait(lock);
        }
        
        if (finished_) return;
        
        queue_.push(item);
        // ОШИБКА: Уведомляем неправильную condition variable!
        cv_producer_.notify_one();  // Должно быть cv_consumer_!
    }
    
    bool pop(int& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        while (queue_.empty() && !finished_) {
            cv_consumer_.wait(lock);
        }
        
        if (queue_.empty()) return false;
        
        item = queue_.front();
        queue_.pop();
        
        // ОШИБКА: Уведомляем неправильную condition variable!
        cv_consumer_.notify_one();  // Должно быть cv_producer_!
        return true;
    }
    
    void finish() {
        std::lock_guard<std::mutex> lock(mutex_);
        finished_ = true;
        cv_producer_.notify_all();
        cv_consumer_.notify_all();
    }
};

void demonstrateDeadlock() {
    std::cout << "\n=== УЯЗВИМОСТЬ 3: Potential Deadlock ===\n";
    
    DeadlockQueue queue;
    
    std::thread producer([&queue]() {
        for (int i = 0; i < 20; ++i) {
            queue.push(i);
            std::cout << "Произведено: " << i << "\n";
        }
        queue.finish();
    });
    
    std::thread consumer([&queue]() {
        int item;
        while (queue.pop(item)) {
            std::cout << "Потреблено: " << item << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    
    // Таймаут для предотвращения зависания
    if (producer.joinable()) producer.join();
    if (consumer.joinable()) consumer.join();
    
    std::cout << "⚠️  Может возникнуть deadlock из-за неправильных condition variables!\n";
}

// ============================================================================
// УЯЗВИМОСТЬ 4: USE-AFTER-FREE В ОЧЕРЕДИ УКАЗАТЕЛЕЙ
// Проблема: Хранение сырых указателей без владения
// ============================================================================

struct Message {
    int id;
    char data[256];
    
    Message(int i) : id(i) {
        snprintf(data, sizeof(data), "Message #%d", id);
    }
};

class PointerQueue {
private:
    std::queue<Message*> queue_;  // ОПАСНО: Сырые указатели!
    std::mutex mutex_;
    std::condition_variable cv_;
    
public:
    void push(Message* msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(msg);
        cv_.notify_one();
    }
    
    Message* pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !queue_.empty(); });
        
        Message* msg = queue_.front();
        queue_.pop();
        return msg;  // Кто владеет указателем?
    }
};

void demonstrateUseAfterFree() {
    std::cout << "\n=== УЯЗВИМОСТЬ 4: Use-After-Free ===\n";
    
    PointerQueue queue;
    
    // Producer создает сообщения на стеке
    std::thread producer([&queue]() {
        for (int i = 0; i < 5; ++i) {
            Message msg(i);  // Локальная переменная!
            queue.push(&msg);  // ОПАСНО: Передаем указатель на стек!
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            // msg удаляется здесь!
        }
    });
    
    // Consumer пытается использовать удаленные сообщения
    std::thread consumer([&queue]() {
        for (int i = 0; i < 5; ++i) {
            Message* msg = queue.pop();
            // ОПАСНО: msg может указывать на удаленную память!
            std::cout << "Получено: " << msg->data << "\n";  // USE-AFTER-FREE
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });
    
    producer.join();
    consumer.join();
    
    std::cout << "⚠️  Use-after-free при доступе к удаленным объектам!\n";
}

// ============================================================================
// УЯЗВИМОСТЬ 5: UNBOUNDED QUEUE - RESOURCE EXHAUSTION
// Проблема: Нет ограничения на размер очереди
// ============================================================================

class UnboundedQueue {
private:
    std::queue<std::vector<char>> queue_;  // Может расти бесконечно
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    
public:
    void push(std::vector<char> data) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(data));  // НЕТ ПРОВЕРКИ РАЗМЕРА!
        cv_.notify_one();
    }
    
    bool pop(std::vector<char>& data) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.empty()) return false;
        
        data = std::move(queue_.front());
        queue_.pop();
        return true;
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
};

void demonstrateResourceExhaustion() {
    std::cout << "\n=== УЯЗВИМОСТЬ 5: Resource Exhaustion ===\n";
    
    UnboundedQueue queue;
    std::atomic<bool> stop{false};
    
    // Producer: быстро создает большие объекты
    std::thread producer([&queue, &stop]() {
        for (int i = 0; i < 1000 && !stop; ++i) {
            std::vector<char> large_data(1024 * 1024, 'X');  // 1 MB
            queue.push(std::move(large_data));
            
            if (i % 100 == 0) {
                std::cout << "Произведено: " << i << ", Размер очереди: " 
                          << queue.size() << "\n";
            }
        }
    });
    
    // Consumer: медленно обрабатывает
    std::thread consumer([&queue, &stop]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        std::vector<char> data;
        int consumed = 0;
        while (queue.pop(data)) {
            consumed++;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            
            if (consumed >= 50) {
                stop = true;
                break;
            }
        }
        std::cout << "Потреблено: " << consumed << "\n";
    });
    
    producer.join();
    consumer.join();
    
    std::cout << "⚠️  Очередь росла без ограничений → утечка памяти!\n";
    std::cout << "Финальный размер очереди: " << queue.size() << "\n";
}

// ============================================================================
// УЯЗВИМОСТЬ 6: TOCTOU - TIME-OF-CHECK TO TIME-OF-USE
// Проблема: Проверка и использование не атомарны
// ============================================================================

class TOCTOUQueue {
private:
    std::queue<int> queue_;
    std::mutex mutex_;
    
public:
    bool isEmpty() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
    
    int front() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.front();  // Может быть пустым!
    }
    
    void pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.pop();
    }
    
    void push(int item) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(item);
    }
};

void demonstrateTOCTOU() {
    std::cout << "\n=== УЯЗВИМОСТЬ 6: TOCTOU (Time-of-Check to Time-of-Use) ===\n";
    
    TOCTOUQueue queue;
    
    // Producer
    std::thread producer([&queue]() {
        for (int i = 0; i < 10; ++i) {
            queue.push(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });
    
    // Consumer с TOCTOU уязвимостью
    std::thread consumer([&queue]() {
        for (int i = 0; i < 10; ++i) {
            if (!queue.isEmpty()) {  // Time of Check
                // Другой поток может изменить состояние здесь!
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                
                try {
                    int item = queue.front();  // Time of Use - может быть пустым!
                    queue.pop();
                    std::cout << "Получено: " << item << "\n";
                } catch (...) {
                    std::cout << "⚠️  TOCTOU exception!\n";
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    });
    
    producer.join();
    consumer.join();
    
    std::cout << "⚠️  TOCTOU может вызвать undefined behavior!\n";
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    std::cout << "=== УЯЗВИМОСТИ PRODUCER-CONSUMER PATTERN ===\n";
    std::cout << "⚠️  ВНИМАНИЕ: Этот код содержит уязвимости для обучения!\n";
    
    try {
        demonstrateRaceCondition();
    } catch (...) {
        std::cout << "Exception in race condition demo\n";
    }
    
    try {
        demonstrateBufferOverflow();
    } catch (...) {
        std::cout << "Exception in buffer overflow demo\n";
    }
    
    try {
        demonstrateDeadlock();
    } catch (...) {
        std::cout << "Exception in deadlock demo\n";
    }
    
    try {
        demonstrateUseAfterFree();
    } catch (...) {
        std::cout << "Exception in use-after-free demo\n";
    }
    
    try {
        demonstrateResourceExhaustion();
    } catch (...) {
        std::cout << "Exception in resource exhaustion demo\n";
    }
    
    try {
        demonstrateTOCTOU();
    } catch (...) {
        std::cout << "Exception in TOCTOU demo\n";
    }
    
    std::cout << "\n=== ИНСТРУМЕНТЫ АНАЛИЗА ===\n";
    std::cout << "• ThreadSanitizer: g++ -fsanitize=thread -g producer_consumer_vulnerabilities.cpp\n";
    std::cout << "• AddressSanitizer: g++ -fsanitize=address -g producer_consumer_vulnerabilities.cpp\n";
    std::cout << "• Valgrind Helgrind: valgrind --tool=helgrind ./producer_consumer_vulnerabilities\n";
    
    return 0;
}
