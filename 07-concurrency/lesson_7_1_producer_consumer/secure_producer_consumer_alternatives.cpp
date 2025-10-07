#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <memory>
#include <atomic>
#include <optional>
#include <vector>
#include <array>

/**
 * @file secure_producer_consumer_alternatives.cpp
 * @brief Безопасные реализации паттерна Producer-Consumer
 */

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 1: THREAD-SAFE QUEUE С BOUNDED SIZE
// Решает: Race conditions, Resource exhaustion
// ============================================================================

template<typename T>
class SafeBoundedQueue {
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_not_full_;   // Для producers
    std::condition_variable cv_not_empty_;  // Для consumers
    const size_t max_size_;
    bool finished_ = false;
    
public:
    explicit SafeBoundedQueue(size_t max_size) : max_size_(max_size) {}
    
    // Запрещаем копирование
    SafeBoundedQueue(const SafeBoundedQueue&) = delete;
    SafeBoundedQueue& operator=(const SafeBoundedQueue&) = delete;
    
    bool push(T item, std::chrono::milliseconds timeout = std::chrono::milliseconds::max()) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Ждем, пока очередь не освободится
        if (!cv_not_full_.wait_for(lock, timeout, [this] { 
            return queue_.size() < max_size_ || finished_; 
        })) {
            return false;  // Timeout
        }
        
        if (finished_) return false;
        
        queue_.push(std::move(item));
        cv_not_empty_.notify_one();
        return true;
    }
    
    bool pop(T& item, std::chrono::milliseconds timeout = std::chrono::milliseconds::max()) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Ждем, пока не появится элемент
        if (!cv_not_empty_.wait_for(lock, timeout, [this] { 
            return !queue_.empty() || finished_; 
        })) {
            return false;  // Timeout
        }
        
        if (queue_.empty()) return false;
        
        item = std::move(queue_.front());
        queue_.pop();
        cv_not_full_.notify_one();
        return true;
    }
    
    void finish() {
        std::lock_guard<std::mutex> lock(mutex_);
        finished_ = true;
        cv_not_full_.notify_all();
        cv_not_empty_.notify_all();
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
};

void demonstrateSafeBoundedQueue() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 1: Bounded Queue ===\n";
    
    SafeBoundedQueue<int> queue(10);  // Максимум 10 элементов
    
    // Producer
    std::thread producer([&queue]() {
        for (int i = 0; i < 20; ++i) {
            if (queue.push(i, std::chrono::milliseconds(100))) {
                std::cout << "Произведено: " << i << ", Размер: " << queue.size() << "\n";
            } else {
                std::cout << "⏱️  Timeout при добавлении " << i << "\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        queue.finish();
    });
    
    // Consumer
    std::thread consumer([&queue]() {
        int item;
        while (queue.pop(item, std::chrono::milliseconds(200))) {
            std::cout << "Потреблено: " << item << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    
    producer.join();
    consumer.join();
    
    std::cout << "✅ Безопасно: размер ограничен, нет race conditions\n";
}

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 2: LOCK-FREE RING BUFFER (SPSC)
// Решает: Lock contention, высокая производительность
// Примечание: Single Producer, Single Consumer
// ============================================================================

template<typename T, size_t N>
class LockFreeRingBuffer {
private:
    std::array<T, N> buffer_;
    std::atomic<size_t> head_{0};  // Для producer
    std::atomic<size_t> tail_{0};  // Для consumer
    
    // Padding для избежания false sharing
    char padding_[64];
    
public:
    bool push(const T& item) {
        size_t current_head = head_.load(std::memory_order_relaxed);
        size_t next_head = (current_head + 1) % N;
        
        if (next_head == tail_.load(std::memory_order_acquire)) {
            return false;  // Очередь полная
        }
        
        buffer_[current_head] = item;
        head_.store(next_head, std::memory_order_release);
        return true;
    }
    
    bool pop(T& item) {
        size_t current_tail = tail_.load(std::memory_order_relaxed);
        
        if (current_tail == head_.load(std::memory_order_acquire)) {
            return false;  // Очередь пустая
        }
        
        item = buffer_[current_tail];
        tail_.store((current_tail + 1) % N, std::memory_order_release);
        return true;
    }
    
    bool empty() const {
        return head_.load(std::memory_order_acquire) == 
               tail_.load(std::memory_order_acquire);
    }
    
    size_t size() const {
        size_t h = head_.load(std::memory_order_acquire);
        size_t t = tail_.load(std::memory_order_acquire);
        return (h >= t) ? (h - t) : (N + h - t);
    }
};

void demonstrateLockFreeRingBuffer() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 2: Lock-Free Ring Buffer (SPSC) ===\n";
    
    LockFreeRingBuffer<int, 32> buffer;
    std::atomic<bool> done{false};
    
    // Single Producer
    std::thread producer([&buffer, &done]() {
        for (int i = 0; i < 1000; ++i) {
            while (!buffer.push(i)) {
                std::this_thread::yield();
            }
        }
        done = true;
    });
    
    // Single Consumer
    std::thread consumer([&buffer, &done]() {
        int item;
        int count = 0;
        while (!done || !buffer.empty()) {
            if (buffer.pop(item)) {
                count++;
            } else {
                std::this_thread::yield();
            }
        }
        std::cout << "Потреблено: " << count << " элементов\n";
    });
    
    producer.join();
    consumer.join();
    
    std::cout << "✅ Lock-free: высокая производительность без блокировок\n";
}

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 3: ОЧЕРЕДЬ С УМНЫМИ УКАЗАТЕЛЯМИ
// Решает: Memory leaks, Use-after-free
// ============================================================================

template<typename T>
class SmartPointerQueue {
private:
    std::queue<std::shared_ptr<T>> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    bool finished_ = false;
    
public:
    void push(std::shared_ptr<T> item) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(item));
        cv_.notify_one();
    }
    
    std::shared_ptr<T> pop(std::chrono::milliseconds timeout = std::chrono::milliseconds::max()) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        if (!cv_.wait_for(lock, timeout, [this] { 
            return !queue_.empty() || finished_; 
        })) {
            return nullptr;  // Timeout
        }
        
        if (queue_.empty()) return nullptr;
        
        auto item = queue_.front();
        queue_.pop();
        return item;
    }
    
    void finish() {
        std::lock_guard<std::mutex> lock(mutex_);
        finished_ = true;
        cv_.notify_all();
    }
};

struct Message {
    int id;
    std::string data;
    
    Message(int i, std::string d) : id(i), data(std::move(d)) {
        std::cout << "  [Message " << id << "] Создано\n";
    }
    
    ~Message() {
        std::cout << "  [Message " << id << "] Удалено\n";
    }
};

void demonstrateSmartPointerQueue() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 3: Smart Pointer Queue ===\n";
    
    SmartPointerQueue<Message> queue;
    
    // Producer
    std::thread producer([&queue]() {
        for (int i = 0; i < 5; ++i) {
            auto msg = std::make_shared<Message>(i, "Data " + std::to_string(i));
            queue.push(msg);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        queue.finish();
    });
    
    // Consumer
    std::thread consumer([&queue]() {
        while (auto msg = queue.pop(std::chrono::milliseconds(500))) {
            std::cout << "Обработка: " << msg->data << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
    });
    
    producer.join();
    consumer.join();
    
    std::cout << "✅ Все Message автоматически удалены (RAII)\n";
}

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 4: MPMC QUEUE (Multiple Producers, Multiple Consumers)
// Решает: Масштабируемость, deadlocks
// ============================================================================

template<typename T>
class MPMCQueue {
private:
    struct Node {
        std::shared_ptr<T> data;
        std::unique_ptr<Node> next;
    };
    
    std::unique_ptr<Node> head_;
    Node* tail_;
    mutable std::mutex head_mutex_;
    mutable std::mutex tail_mutex_;
    std::condition_variable cv_;
    std::atomic<bool> finished_{false};
    
    Node* getTail() {
        std::lock_guard<std::mutex> lock(tail_mutex_);
        return tail_;
    }
    
public:
    MPMCQueue() : head_(std::make_unique<Node>()), tail_(head_.get()) {}
    
    void push(T value) {
        auto data = std::make_shared<T>(std::move(value));
        auto new_node = std::make_unique<Node>();
        
        {
            std::lock_guard<std::mutex> lock(tail_mutex_);
            tail_->data = data;
            Node* const new_tail = new_node.get();
            tail_->next = std::move(new_node);
            tail_ = new_tail;
        }
        
        cv_.notify_one();
    }
    
    std::shared_ptr<T> pop() {
        std::unique_lock<std::mutex> lock(head_mutex_);
        
        cv_.wait(lock, [this] { 
            return head_.get() != getTail() || finished_.load(); 
        });
        
        if (head_.get() == getTail()) {
            return nullptr;  // Finished and empty
        }
        
        auto old_head = std::move(head_);
        head_ = std::move(old_head->next);
        return old_head->data;
    }
    
    void finish() {
        finished_.store(true);
        cv_.notify_all();
    }
};

void demonstrateMPMCQueue() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 4: MPMC Queue ===\n";
    
    MPMCQueue<int> queue;
    std::atomic<int> total_produced{0};
    std::atomic<int> total_consumed{0};
    
    // Multiple producers
    std::vector<std::thread> producers;
    for (int p = 0; p < 3; ++p) {
        producers.emplace_back([&queue, &total_produced, p]() {
            for (int i = 0; i < 10; ++i) {
                int value = p * 100 + i;
                queue.push(value);
                total_produced++;
                std::cout << "Producer " << p << " → " << value << "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        });
    }
    
    // Multiple consumers
    std::vector<std::thread> consumers;
    for (int c = 0; c < 2; ++c) {
        consumers.emplace_back([&queue, &total_consumed, c]() {
            while (auto item = queue.pop()) {
                std::cout << "  Consumer " << c << " ← " << *item << "\n";
                total_consumed++;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
    }
    
    // Ждем producers
    for (auto& t : producers) t.join();
    queue.finish();
    
    // Ждем consumers
    for (auto& t : consumers) t.join();
    
    std::cout << "\n✅ Произведено: " << total_produced 
              << ", Потреблено: " << total_consumed << "\n";
    std::cout << "✅ MPMC: безопасная работа множества потоков\n";
}

// ============================================================================
// БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 5: PRIORITY QUEUE
// Решает: Приоритизация задач
// ============================================================================

template<typename T>
class PriorityProducerConsumerQueue {
private:
    struct Item {
        T data;
        int priority;
        
        bool operator<(const Item& other) const {
            return priority < other.priority;  // Высокий приоритет = большее число
        }
    };
    
    std::priority_queue<Item> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    bool finished_ = false;
    size_t max_size_;
    
public:
    explicit PriorityProducerConsumerQueue(size_t max_size) : max_size_(max_size) {}
    
    bool push(T data, int priority) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        if (queue_.size() >= max_size_) {
            return false;  // Очередь полная
        }
        
        queue_.push({std::move(data), priority});
        cv_.notify_one();
        return true;
    }
    
    bool pop(T& data, int& priority) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        cv_.wait(lock, [this] { return !queue_.empty() || finished_; });
        
        if (queue_.empty()) return false;
        
        auto item = queue_.top();
        queue_.pop();
        data = std::move(item.data);
        priority = item.priority;
        return true;
    }
    
    void finish() {
        std::lock_guard<std::mutex> lock(mutex_);
        finished_ = true;
        cv_.notify_all();
    }
};

void demonstratePriorityQueue() {
    std::cout << "\n=== БЕЗОПАСНАЯ РЕАЛИЗАЦИЯ 5: Priority Queue ===\n";
    
    PriorityProducerConsumerQueue<std::string> queue(20);
    
    // Producer с разными приоритетами
    std::thread producer([&queue]() {
        queue.push("Обычная задача 1", 1);
        queue.push("СРОЧНАЯ задача!", 10);
        queue.push("Обычная задача 2", 1);
        queue.push("Важная задача", 5);
        queue.push("Низкий приоритет", 0);
        queue.finish();
    });
    
    // Consumer
    std::thread consumer([&queue]() {
        std::string task;
        int priority;
        while (queue.pop(task, priority)) {
            std::cout << "Приоритет " << priority << ": " << task << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    
    producer.join();
    consumer.join();
    
    std::cout << "✅ Задачи обработаны в порядке приоритета\n";
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    std::cout << "=== БЕЗОПАСНЫЕ РЕАЛИЗАЦИИ PRODUCER-CONSUMER ===\n";
    
    demonstrateSafeBoundedQueue();
    demonstrateLockFreeRingBuffer();
    demonstrateSmartPointerQueue();
    demonstrateMPMCQueue();
    demonstratePriorityQueue();
    
    std::cout << "\n=== РЕКОМЕНДАЦИИ ===\n";
    std::cout << "✅ Используйте bounded queue для предотвращения OOM\n";
    std::cout << "✅ Применяйте умные указатели для управления памятью\n";
    std::cout << "✅ Используйте lock-free структуры для SPSC\n";
    std::cout << "✅ Применяйте timeout для предотвращения deadlocks\n";
    std::cout << "✅ Используйте condition variables корректно\n";
    std::cout << "✅ Тестируйте с ThreadSanitizer\n";
    
    return 0;
}
