/**
 * @file producer_consumer_cpp23_quick_comparison.cpp
 * @brief КРАТКОЕ СРАВНЕНИЕ Producer-Consumer Pattern: C++17/20 vs C++23
 * 
 * Минималистичный файл для быстрого понимания ключевых отличий C++23.
 * Показывает только самые важные улучшения без лишних деталей.
 * 
 * КЛЮЧЕВЫЕ ОТЛИЧИЯ:
 * - ❌ std::thread (C++17/20) → ✅ std::jthread (C++23) - auto-join + stop_token
 * - ❌ bool return (C++17/20) → ✅ std::expected (C++23) - детальные ошибки
 * 
 * Компиляция:
 * - C++17/20: g++ -std=c++20 producer_consumer_cpp23_quick_comparison.cpp
 * - C++23: g++ -std=c++23 producer_consumer_cpp23_quick_comparison.cpp
 * 
 * РАЗМЕР: ~110 строк (для быстрого ознакомления)
 * 
 * Для полного изучения см.: producer_consumer_cpp23_full_comparison.cpp
 */

// C++17/20 vs C++23: Producer-Consumer
// C++23: std::jthread для auto-join, std::expected для queue operations

#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

#if __cplusplus >= 202302L
    #include <expected>
    #include <print>
    #define HAS_CPP23 1
    using std::expected; using std::unexpected;
#else
    #define HAS_CPP23 0
    template<typename T, typename E> struct expected { T v; bool has_value() const { return true; } T& operator*() { return v; } };
#endif

namespace cpp17 {
    class Queue {
        std::queue<int> queue_;
        std::mutex mtx_;
        std::condition_variable cv_;
    public:
        void push(int value) {
            std::lock_guard lock(mtx_);
            queue_.push(value);
            cv_.notify_one();
        }
        
        int pop() {
            std::unique_lock lock(mtx_);
            cv_.wait(lock, [this] { return !queue_.empty(); });
            int value = queue_.front();
            queue_.pop();
            return value;
        }
        
        // ❌ std::thread - ручной join
        void runProducer() {
            std::thread t([this]() {
                for (int i = 0; i < 10; ++i) push(i);
            });
            t.join();
        }
    };
}

namespace cpp23 {
    enum class QueueError { Empty, Full, Timeout };
    
    class Queue {
        std::queue<int> queue_;
        std::mutex mtx_;
        std::condition_variable cv_;
        const size_t MAX_SIZE = 100;
    public:
        // ✅ expected для bounded queue
        expected<void, QueueError> push(int value) {
            std::lock_guard lock(mtx_);
            if (queue_.size() >= MAX_SIZE) {
                return unexpected(QueueError::Full);
            }
            queue_.push(value);
            cv_.notify_one();
            return {};
        }
        
        expected<int, QueueError> pop() {
            std::unique_lock lock(mtx_);
            cv_.wait(lock, [this] { return !queue_.empty(); });
            int value = queue_.front();
            queue_.pop();
            return value;
        }
        
#if HAS_CPP23
        // ✅ jthread - auto-join + stop_token
        void runProducer() {
            std::jthread t([this](std::stop_token stoken) {
                for (int i = 0; i < 10 && !stoken.stop_requested(); ++i) {
                    push(i);
                }
            });
            // ✅ Auto-join при destruction
        }
#endif
    };
}

int main() {
    cpp23::Queue queue;
    
    auto result = queue.push(42);
    if (result.has_value()) {
        std::cout << "✅ Pushed successfully\n";
    }
    
    auto value = queue.pop();
    if (value.has_value()) {
        std::cout << "✅ Popped: " << *value << "\n";
    }
    
    std::cout << "✅ C++23: jthread для producer/consumer threads\n";
    std::cout << "✅ C++23: expected для queue overflow handling\n";
    return 0;
}
