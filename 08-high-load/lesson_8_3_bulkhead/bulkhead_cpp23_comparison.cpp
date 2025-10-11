// C++17/20 vs C++23: Bulkhead Pattern
// C++23: std::jthread для isolated thread pools, std::expected

#include <iostream>
#include <thread>
#include <vector>
#include <functional>

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
    class Bulkhead {
        std::vector<std::thread> threads_;
    public:
        void submit(std::function<void()> task) {
            threads_.emplace_back(task);
        }
        
        ~Bulkhead() {
            for (auto& t : threads_) {
                t.join();  // ❌ Manual join
            }
        }
    };
}

namespace cpp23 {
    enum class BulkheadError { PartitionFull, TaskFailed };
    
    class Bulkhead {
#if HAS_CPP23
        std::vector<std::jthread> threads_;  // ✅ Auto-join
#else
        std::vector<std::thread> threads_;
#endif
        const size_t MAX_THREADS = 10;
        
    public:
        // ✅ expected для capacity checking
        expected<void, BulkheadError> submit(std::function<void()> task) {
            if (threads_.size() >= MAX_THREADS) {
                return unexpected(BulkheadError::PartitionFull);
            }
            
#if HAS_CPP23
            threads_.emplace_back(task);  // ✅ jthread auto-joins
#else
            threads_.emplace_back(task);
#endif
            return {};
        }
        
        // ✅ Destructor: jthreads auto-join
    };
}

int main() {
    cpp23::Bulkhead bulkhead;
    
    auto result = bulkhead.submit([]() {
        std::cout << "Task executing\n";
    });
    
    if (result.has_value()) {
        std::cout << "✅ Task submitted\n";
    }
    
    std::cout << "✅ C++23: jthread для isolated partitions (auto-join)\n";
    std::cout << "✅ C++23: expected для partition overflow\n";
    return 0;
}

