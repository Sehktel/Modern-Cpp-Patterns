// C++17/20 vs C++23: Adapter Pattern
// C++23: std::expected для conversion errors, std::print

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

class LegacyAPI { public: void legacyMethod(int x) { std::cout << "Legacy: " << x << "\n"; } };
class ModernAPI { public: virtual void modernMethod(const std::string& s) = 0; };

namespace cpp17 {
    class Adapter : public ModernAPI {
        LegacyAPI legacy_;
    public:
        void modernMethod(const std::string& s) override {
            int x = std::stoi(s);  // ❌ May throw
            legacy_.legacyMethod(x);
        }
    };
}

namespace cpp23 {
    enum class AdapterError { InvalidConversion, OutOfRange };
    
    class Adapter : public ModernAPI {
        LegacyAPI legacy_;
    public:
        // ✅ expected для conversion errors
        expected<void, AdapterError> modernMethodSafe(const std::string& s) {
            try {
                int x = std::stoi(s);
                legacy_.legacyMethod(x);
                return {};
            } catch (...) {
                return unexpected(AdapterError::InvalidConversion);
            }
        }
        
        void modernMethod(const std::string& s) override {
            auto result = modernMethodSafe(s);
            if (!result.has_value()) {
#if HAS_CPP23
                std::print("Conversion failed\n");
#else
                std::cout << "Conversion failed\n";
#endif
            }
        }
    };
}

int main() {
    cpp23::Adapter adapter;
    adapter.modernMethod("42");
    
    std::cout << "✅ C++23: expected для type conversion errors\n";
    return 0;
}

