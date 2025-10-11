// C++17/20 vs C++23: Smart Pointers
// C++23: std::expected для safe pointer operations, std::print

#include <iostream>
#include <memory>

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

namespace cpp17 {
    class ResourceManager {
    public:
        std::shared_ptr<int> acquire() {
            return std::make_shared<int>(42);
        }
        
        int* get(std::shared_ptr<int> ptr) {
            if (!ptr) {
                throw std::runtime_error("Null pointer");  // ❌ Exception
            }
            return ptr.get();
        }
    };
}

namespace cpp23 {
    enum class PointerError { Null, Expired };
    
    class ResourceManager {
    public:
        std::shared_ptr<int> acquire() {
            return std::make_shared<int>(42);
        }
        
        // ✅ expected для null-safety
        expected<int*, PointerError> get(std::shared_ptr<int> ptr) {
            if (!ptr) {
                return unexpected(PointerError::Null);
            }
            return ptr.get();
        }
        
        // ✅ expected для weak_ptr
        expected<int*, PointerError> get(std::weak_ptr<int> weak) {
            if (auto ptr = weak.lock()) {
                return ptr.get();
            }
            return unexpected(PointerError::Expired);
        }
    };
}

int main() {
    cpp23::ResourceManager mgr;
    auto ptr = mgr.acquire();
    
    auto result = mgr.get(ptr);
    if (result.has_value()) {
#if HAS_CPP23
        std::print("Value: {}\n", **result);
#else
        std::cout << "Value: " << **result << "\n";
#endif
    }
    
    std::cout << "✅ C++23: expected для null-safe smart pointer operations\n";
    return 0;
}

