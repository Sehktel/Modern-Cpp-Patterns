// C++17/20 vs C++23: Object Pool
// C++23: std::jthread для pool management, std::expected

#include <iostream>
#include <vector>
#include <memory>

#if __cplusplus >= 202302L
    #include <expected>
    #include <print>
    #define HAS_CPP23 1
    using std::expected; using std::unexpected;
#else
    #define HAS_CPP23 0
    template<typename T, typename E> struct expected { T v; bool has_value() const { return true; } T& operator*() { return v; } };
#endif

class Resource {
public:
    void use() { std::cout << "Using resource\n"; }
    void reset() { /* Reset state */ }
};

namespace cpp17 {
    class ObjectPool {
        std::vector<std::unique_ptr<Resource>> pool_;
    public:
        Resource* acquire() {
            if (pool_.empty()) {
                throw std::runtime_error("Pool exhausted");  // ❌ Exception
            }
            auto resource = std::move(pool_.back());
            pool_.pop_back();
            return resource.release();
        }
        
        void release(Resource* res) {
            res->reset();
            pool_.push_back(std::unique_ptr<Resource>(res));
        }
    };
}

namespace cpp23 {
    enum class PoolError { Exhausted, InvalidResource };
    
    class ObjectPool {
        std::vector<std::unique_ptr<Resource>> pool_;
    public:
        // ✅ expected для pool operations
        expected<std::unique_ptr<Resource>, PoolError> acquire() {
            if (pool_.empty()) {
                return unexpected(PoolError::Exhausted);
            }
            
            auto resource = std::move(pool_.back());
            pool_.pop_back();
#if HAS_CPP23
            std::print("Acquired resource from pool\n");
#endif
            return resource;
        }
        
        expected<void, PoolError> release(std::unique_ptr<Resource> res) {
            if (!res) {
                return unexpected(PoolError::InvalidResource);
            }
            
            res->reset();
            pool_.push_back(std::move(res));
            return {};
        }
    };
}

int main() {
    cpp23::ObjectPool pool;
    
    auto result = pool.acquire();
    if (result.has_value()) {
        (*result)->use();
        pool.release(std::move(*result));
        std::cout << "✅ Resource acquired and returned\n";
    }
    
    std::cout << "✅ C++23: expected для pool exhaustion handling\n";
    return 0;
}

