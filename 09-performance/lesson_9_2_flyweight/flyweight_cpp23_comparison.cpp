// C++17/20 vs C++23: Flyweight Pattern
// C++23: std::flat_map для flyweight factory

#include <iostream>
#include <map>
#include <memory>
#include <string>

#if __cplusplus >= 202302L
    #include <expected>
    #include <print>
    #include <flat_map>
    #define HAS_CPP23 1
    using std::expected; using std::unexpected;
#else
    #define HAS_CPP23 0
    template<typename T, typename E> struct expected { T v; bool has_value() const { return true; } T& operator*() { return v; } };
#endif

class Flyweight {
    const std::string intrinsic_;  // Shared immutable state
public:
    Flyweight(std::string s) : intrinsic_(std::move(s)) {}
    void operation(const std::string& extrinsic) {
#if HAS_CPP23
        std::print("Intrinsic: {}, Extrinsic: {}\n", intrinsic_, extrinsic);
#else
        std::cout << "Intrinsic: " << intrinsic_ << ", Extrinsic: " << extrinsic << "\n";
#endif
    }
};

namespace cpp17 {
    class FlyweightFactory {
        std::map<std::string, std::shared_ptr<Flyweight>> cache_;  // ⚠️ Red-black tree
    public:
        std::shared_ptr<Flyweight> get(const std::string& key) {
            auto it = cache_.find(key);
            if (it != cache_.end()) {
                return it->second;
            }
            
            auto flyweight = std::make_shared<Flyweight>(key);
            cache_[key] = flyweight;
            return flyweight;
        }
    };
}

namespace cpp23 {
    enum class FactoryError { AllocationFailed };
    
    class FlyweightFactory {
#if HAS_CPP23
        // ✅ flat_map - faster для small/medium caches
        std::flat_map<std::string, std::shared_ptr<Flyweight>> cache_;
#else
        std::map<std::string, std::shared_ptr<Flyweight>> cache_;
#endif
    public:
        expected<std::shared_ptr<Flyweight>, FactoryError> get(const std::string& key) {
            auto it = cache_.find(key);
            if (it != cache_.end()) {
                return it->second;
            }
            
            try {
                auto flyweight = std::make_shared<Flyweight>(key);
                cache_[key] = flyweight;
                return flyweight;
            } catch (...) {
                return unexpected(FactoryError::AllocationFailed);
            }
        }
    };
}

int main() {
    cpp23::FlyweightFactory factory;
    
    auto result = factory.get("shared_state");
    if (result.has_value()) {
        (*result)->operation("extrinsic_data");
        std::cout << "✅ Flyweight retrieved\n";
    }
    
    std::cout << "✅ C++23: flat_map для flyweight cache (~2x faster)\n";
    return 0;
}

