// C++17/20 vs C++23: Facade Pattern
// C++23: std::expected для subsystem errors, std::print

#include <iostream>
#include <string>

#if __cplusplus >= 202302L
    #include <expected>
    #include <print>
    #define HAS_CPP23 1
    using std::expected; using std::unexpected;
#else
    #define HAS_CPP23 0
    template<typename T, typename E> struct expected { T v; bool has_value() const { return true; } T& operator*() { return v; } };
    template<typename E> struct unexpected { E e; };
#endif

class SubsystemA { public: void operationA() { std::cout << "Op A\n"; } };
class SubsystemB { public: void operationB() { std::cout << "Op B\n"; } };

namespace cpp17 {
    class Facade {
        SubsystemA a_;
        SubsystemB b_;
    public:
        void operation() {
            a_.operationA();  // ❌ Может throw
            b_.operationB();
        }
    };
}

namespace cpp23 {
    enum class FacadeError { SubsystemAFailed, SubsystemBFailed };
    
    class Facade {
        SubsystemA a_;
        SubsystemB b_;
    public:
        // ✅ expected для aggregated errors
        expected<void, FacadeError> operation() {
            try {
                a_.operationA();
            } catch (...) {
                return unexpected(FacadeError::SubsystemAFailed);
            }
            
            try {
                b_.operationB();
            } catch (...) {
                return unexpected(FacadeError::SubsystemBFailed);
            }
            
            return {};
        }
    };
}

int main() {
    cpp23::Facade facade;
    auto result = facade.operation();
    
#if HAS_CPP23
    if (result.has_value()) {
        std::println("✅ Facade operation successful");
    }
#else
    std::cout << "✅ Facade operation successful\n";
#endif
    
    std::cout << "✅ C++23: expected для subsystem error aggregation\n";
    return 0;
}

