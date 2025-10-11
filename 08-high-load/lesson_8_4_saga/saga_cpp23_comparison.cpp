// C++17/20 vs C++23: Saga Pattern
// C++23: std::generator для compensation steps, std::expected

#include <iostream>
#include <vector>
#include <functional>
#include <stack>

#if __cplusplus >= 202302L
    #include <expected>
    #include <print>
    #include <generator>
    #define HAS_CPP23 1
    using std::expected; using std::unexpected;
#else
    #define HAS_CPP23 0
    template<typename T, typename E> struct expected { T v; bool has_value() const { return true; } T& operator*() { return v; } };
#endif

namespace cpp17 {
    class Saga {
        std::stack<std::function<void()>> compensations_;
    public:
        void execute() {
            try {
                step1();
                compensations_.push([]() { compensate1(); });
                
                step2();
                compensations_.push([]() { compensate2(); });
                
            } catch (...) {
                while (!compensations_.empty()) {  // ❌ Manual rollback
                    compensations_.top()();
                    compensations_.pop();
                }
                throw;
            }
        }
        
    private:
        void step1() { std::cout << "Step 1\n"; }
        void step2() { throw std::runtime_error("Fail"); }
        static void compensate1() { std::cout << "Compensate 1\n"; }
        static void compensate2() { std::cout << "Compensate 2\n"; }
    };
}

namespace cpp23 {
    enum class SagaError { Step1Failed, Step2Failed };
    
    class Saga {
        std::stack<std::function<void()>> compensations_;
        
    public:
        // ✅ expected вместо exceptions
        expected<void, SagaError> execute() {
            auto r1 = step1();
            if (!r1.has_value()) return unexpected(SagaError::Step1Failed);
            compensations_.push([]() { compensate1(); });
            
            auto r2 = step2();
            if (!r2.has_value()) {
                rollback();
                return unexpected(SagaError::Step2Failed);
            }
            compensations_.push([]() { compensate2(); });
            
            return {};
        }
        
#if HAS_CPP23
        // ✅ Generator для compensation steps
        std::generator<std::function<void()>> getCompensations() {
            while (!compensations_.empty()) {
                co_yield compensations_.top();
                compensations_.pop();
            }
        }
#endif
        
    private:
        void rollback() {
            while (!compensations_.empty()) {
                compensations_.top()();
                compensations_.pop();
            }
        }
        
        expected<void, SagaError> step1() { std::cout << "Step 1\n"; return {}; }
        expected<void, SagaError> step2() { return unexpected(SagaError::Step2Failed); }
        static void compensate1() { std::cout << "Compensate 1\n"; }
        static void compensate2() { std::cout << "Compensate 2\n"; }
    };
}

int main() {
    cpp23::Saga saga;
    
    auto result = saga.execute();
    if (!result.has_value()) {
        std::cout << "✅ Saga failed and rolled back\n";
    }
    
    std::cout << "✅ C++23: generator для compensation iteration\n";
    std::cout << "✅ C++23: expected для saga step results\n";
    return 0;
}

