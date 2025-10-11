// C++17/20 vs C++23: Cache-Aside Pattern
// C++23: std::flat_map для cache, std::expected для cache operations

#include <iostream>
#include <map>
#include <string>
#include <optional>

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

class Database { public: std::string query(const std::string& key) { return "value_" + key; } };

namespace cpp17 {
    class Cache {
        std::map<std::string, std::string> cache_;  // ⚠️ Red-black tree
        Database db_;
    public:
        std::optional<std::string> get(const std::string& key) {
            auto it = cache_.find(key);
            if (it != cache_.end()) {
                return it->second;  // Cache hit
            }
            
            // Cache miss
            auto value = db_.query(key);
            cache_[key] = value;
            return value;
        }
    };
}

namespace cpp23 {
    enum class CacheError { Timeout, DatabaseError, CacheFull };
    
    class Cache {
#if HAS_CPP23
        // ✅ flat_map - лучшая cache locality (~2-3x faster)
        std::flat_map<std::string, std::string> cache_;
#else
        std::map<std::string, std::string> cache_;
#endif
        Database db_;
        const size_t MAX_SIZE = 10000;
        
    public:
        // ✅ expected для error handling
        expected<std::string, CacheError> get(const std::string& key) {
            auto it = cache_.find(key);
            if (it != cache_.end()) {
#if HAS_CPP23
                std::print("Cache hit: {}\n", key);
#endif
                return it->second;
            }
            
            // Check size limit
            if (cache_.size() >= MAX_SIZE) {
                return unexpected(CacheError::CacheFull);
            }
            
            try {
                auto value = db_.query(key);
                cache_[key] = value;
                return value;
            } catch (...) {
                return unexpected(CacheError::DatabaseError);
            }
        }
    };
}

int main() {
    cpp23::Cache cache;
    
    auto result = cache.get("key1");
    if (result.has_value()) {
        std::cout << "✅ Value: " << *result << "\n";
    }
    
    std::cout << "✅ C++23: flat_map для cache (faster lookups)\n";
    std::cout << "✅ C++23: expected для cache error handling\n";
    return 0;
}

