// ============================================================================
// СРАВНЕНИЕ: Factory Method в C++17/20 vs C++23
// ============================================================================
// Новые возможности C++23:
// - std::expected для factory error handling
// - std::print для вывода
// - std::flat_map для product registry
// ============================================================================

#include <iostream>
#include <memory>
#include <map>
#include <string>

#if __cplusplus >= 202302L
    #include <expected>
    #include <print>
    #define HAS_CPP23 1
    using std::expected;
    using std::unexpected;
#else
    #define HAS_CPP23 0
    template<typename T, typename E> struct expected { T value; bool has_value() const { return true; } T& operator*() { return value; } };
    template<typename E> struct unexpected { E error; };
#endif

// ----------------------------------------------------------------------------
// Product интерфейс
// ----------------------------------------------------------------------------
class Product {
public:
    virtual ~Product() = default;
    virtual void use() = 0;
    virtual std::string getName() const = 0;
};

class ConcreteProductA : public Product {
public:
    void use() override { 
#if HAS_CPP23
        std::println("Using Product A"); 
#else
        std::cout << "Using Product A\n";
#endif
    }
    std::string getName() const override { return "ProductA"; }
};

class ConcreteProductB : public Product {
public:
    void use() override { 
#if HAS_CPP23
        std::println("Using Product B"); 
#else
        std::cout << "Using Product B\n";
#endif
    }
    std::string getName() const override { return "ProductB"; }
};

// ============================================================================
// C++17/20 VERSION
// ============================================================================

namespace cpp17 {

class ProductFactory {
private:
    std::map<std::string, std::function<std::unique_ptr<Product>()>> registry_;  // ⚠️ Red-black tree
    
public:
    void registerProduct(const std::string& type, 
                        std::function<std::unique_ptr<Product>()> creator) {
        registry_[type] = creator;
    }
    
    // ❌ Exception на ошибку
    std::unique_ptr<Product> create(const std::string& type) {
        auto it = registry_.find(type);
        if (it == registry_.end()) {
            throw std::runtime_error("Unknown product type: " + type);
        }
        
        std::cout << "Creating product: " << type << "\n";
        return it->second();
    }
};

void demonstrate() {
    std::cout << "\n=== C++17/20 Factory Method ===\n";
    
    ProductFactory factory;
    factory.registerProduct("A", []() { return std::make_unique<ConcreteProductA>(); });
    factory.registerProduct("B", []() { return std::make_unique<ConcreteProductB>(); });
    
    try {
        auto product = factory.create("A");
        product->use();
        
        auto invalid = factory.create("C");  // ❌ Throws
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

} // namespace cpp17

// ============================================================================
// C++23 VERSION
// ============================================================================

namespace cpp23 {

enum class FactoryError {
    UnknownType,
    CreationFailed,
    InvalidConfiguration
};

class ProductFactory {
private:
#if HAS_CPP23
    // ✅ flat_map - лучшая cache locality для небольших registries
    std::flat_map<std::string, std::function<std::unique_ptr<Product>()>> registry_;
#else
    std::map<std::string, std::function<std::unique_ptr<Product>()>> registry_;
#endif
    
public:
    void registerProduct(const std::string& type,
                        std::function<std::unique_ptr<Product>()> creator) {
        registry_[type] = creator;
    }
    
    // ✅ expected вместо exception
    expected<std::unique_ptr<Product>, FactoryError> create(const std::string& type) {
        auto it = registry_.find(type);
        if (it == registry_.end()) {
            return unexpected(FactoryError::UnknownType);  // ✅ Error без exception
        }
        
#if HAS_CPP23
        std::print("Creating product: {}\n", type);
#else
        std::cout << "Creating product: " << type << "\n";
#endif
        
        try {
            return it->second();  // Success
        } catch (...) {
            return unexpected(FactoryError::CreationFailed);
        }
    }
    
    // ✅ Monadic operations с expected
    expected<std::string, FactoryError> createAndGetName(const std::string& type) {
        // ✅ Chainable error handling
        auto result = create(type);
        if (!result.has_value()) {
            return unexpected(result.error());
        }
        
        return (*result)->getName();
    }
};

void demonstrate() {
#if HAS_CPP23
    std::println("\n=== C++23 Factory Method ===");
#else
    std::cout << "\n=== C++23 Factory Method ===\n";
#endif
    
    ProductFactory factory;
    factory.registerProduct("A", []() { return std::make_unique<ConcreteProductA>(); });
    factory.registerProduct("B", []() { return std::make_unique<ConcreteProductB>(); });
    
    // ✅ Explicit error handling без try-catch
    auto result = factory.create("A");
    if (result.has_value()) {
        (*result)->use();
    }
    
    auto invalid = factory.create("C");  // ✅ Возвращает error
    if (!invalid.has_value()) {
#if HAS_CPP23
        std::print("Expected error: Unknown product type\n");
#else
        std::cout << "Expected error: Unknown product type\n";
#endif
    }
    
    // ✅ Monadic chainable operations
    auto name_result = factory.createAndGetName("B");
    if (name_result.has_value()) {
#if HAS_CPP23
        std::print("Product name: {}\n", *name_result);
#else
        std::cout << "Product name: " << *name_result << "\n";
#endif
    }
}

} // namespace cpp23

// ============================================================================
// MAIN
// ============================================================================

int main() {
    cpp17::demonstrate();
    cpp23::demonstrate();
    
    std::cout << "\n=== ПРЕИМУЩЕСТВА C++23 ===\n";
    std::cout << "✅ std::expected: Error handling без overhead exceptions\n";
    std::cout << "✅ std::flat_map: ~2-3x faster lookup для небольших registries\n";
    std::cout << "✅ std::print: Type-safe форматированный вывод\n";
    std::cout << "✅ Monadic operations: Chainable error handling\n";
    
    return 0;
}

