// C++17/20 vs C++23: CRTP Pattern
// C++23: std::expected для type-safe operations, std::print

#include <iostream>

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
    template<typename Derived>
    class Base {
    public:
        void interface() {
            static_cast<Derived*>(this)->implementation();  // ⚠️ No validation
        }
    };
    
    class Derived : public Base<Derived> {
    public:
        void implementation() { std::cout << "Derived impl\n"; }
    };
}

namespace cpp23 {
    enum class CRTPError { WrongDerivedType, NotInitialized };
    
    template<typename Derived>
    class Base {
    public:
        // ✅ expected для type-safe CRTP
        expected<void, CRTPError> interface() {
            if (typeid(*this) != typeid(Derived)) {
                return unexpected(CRTPError::WrongDerivedType);
            }
            
            static_cast<Derived*>(this)->implementation();
            return {};
        }
        
        void interfaceUnsafe() {
            static_cast<Derived*>(this)->implementation();
        }
    };
    
    class Derived : public Base<Derived> {
    public:
        void implementation() { 
#if HAS_CPP23
            std::println("Derived impl"); 
#else
            std::cout << "Derived impl\n";
#endif
        }
    };
}

int main() {
    cpp23::Derived d;
    
    auto result = d.interface();
    if (result.has_value()) {
        std::cout << "✅ CRTP call successful\n";
    }
    
    std::cout << "✅ C++23: expected для CRTP type safety\n";
    return 0;
}

