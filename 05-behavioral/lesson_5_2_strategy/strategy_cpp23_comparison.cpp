// C++17/20 vs C++23: Strategy Pattern  
// C++23: std::flat_map для strategy registry, std::expected для validation

#include <iostream>
#include <memory>
#include <map>
#include <functional>

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

class Strategy { public: virtual ~Strategy() = default; virtual int execute(int x) = 0; };
class AddStrategy : public Strategy { public: int execute(int x) override { return x + 10; } };
class MulStrategy : public Strategy { public: int execute(int x) override { return x * 2; } };

namespace cpp17 {
    class Context {
        std::map<std::string, std::unique_ptr<Strategy>> strategies_;  // ⚠️ Red-black tree
    public:
        void setStrategy(const std::string& name, std::unique_ptr<Strategy> s) {
            strategies_[name] = std::move(s);
        }
        
        int execute(const std::string& name, int x) {
            if (strategies_.find(name) == strategies_.end()) {
                throw std::runtime_error("Strategy not found");  // ❌ Exception
            }
            return strategies_[name]->execute(x);
        }
    };
}

namespace cpp23 {
    enum class StrategyError { NotFound, ExecutionFailed };
    
    class Context {
#if HAS_CPP23
        // ✅ flat_map - better performance для небольших registries
        std::flat_map<std::string, std::unique_ptr<Strategy>> strategies_;
#else
        std::map<std::string, std::unique_ptr<Strategy>> strategies_;
#endif
    public:
        void setStrategy(const std::string& name, std::unique_ptr<Strategy> s) {
            strategies_[name] = std::move(s);
        }
        
        // ✅ expected вместо exception
        expected<int, StrategyError> execute(const std::string& name, int x) {
            auto it = strategies_.find(name);
            if (it == strategies_.end()) {
                return unexpected(StrategyError::NotFound);
            }
            
            try {
                return it->second->execute(x);
            } catch (...) {
                return unexpected(StrategyError::ExecutionFailed);
            }
        }
    };
}

int main() {
    cpp23::Context ctx;
    ctx.setStrategy("add", std::make_unique<AddStrategy>());
    
    auto result = ctx.execute("add", 5);
    if (result.has_value()) {
        std::cout << "Result: " << *result << "\n";
    }
    
    std::cout << "✅ C++23: flat_map для strategy registry\n";
    std::cout << "✅ C++23: expected для strategy errors\n";
    return 0;
}

