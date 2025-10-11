// C++17/20 vs C++23: Type Erasure
// C++23: std::expected для type-safe casting, std::generator

#include <iostream>
#include <memory>
#include <typeinfo>

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
    class Any {
        void* data_;
        const std::type_info* type_;
    public:
        template<typename T>
        Any(T value) : data_(new T(std::move(value))), type_(&typeid(T)) {}
        
        template<typename T>
        T& get() {
            if (*type_ != typeid(T)) {
                throw std::bad_cast();  // ❌ Exception
            }
            return *static_cast<T*>(data_);
        }
    };
}

namespace cpp23 {
    enum class CastError { TypeMismatch, NullPointer };
    
    class Any {
        void* data_;
        const std::type_info* type_;
    public:
        template<typename T>
        Any(T value) : data_(new T(std::move(value))), type_(&typeid(T)) {}
        
        // ✅ expected для safe casting
        template<typename T>
        expected<T&, CastError> get() {
            if (!data_) {
                return unexpected(CastError::NullPointer);
            }
            
            if (*type_ != typeid(T)) {
                return unexpected(CastError::TypeMismatch);
            }
            
            return *static_cast<T*>(data_);
        }
    };
}

int main() {
    cpp23::Any any(42);
    
    auto result = any.get<int>();
    if (result.has_value()) {
#if HAS_CPP23
        std::print("Value: {}\n", *result);
#else
        std::cout << "Value: " << *result << "\n";
#endif
    }
    
    auto wrong = any.get<std::string>();
    if (!wrong.has_value()) {
        std::cout << "✅ Type mismatch detected без exception\n";
    }
    
    std::cout << "✅ C++23: expected для type-safe casting\n";
    return 0;
}

