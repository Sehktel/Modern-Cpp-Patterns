// ============================================================================
// СРАВНЕНИЕ: Builder в C++17/20 vs C++23
// ============================================================================
// C++23: std::expected для validation, std::print для вывода
// ============================================================================

#include <iostream>
#include <string>
#include <sstream>

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

class Product {
public:
    std::string header;
    std::string body;
    std::string footer;
    
    void display() const {
#if HAS_CPP23
        std::print("Product:\n  Header: {}\n  Body: {}\n  Footer: {}\n", 
                   header, body, footer);
#else
        std::cout << "Product:\n  Header: " << header 
                  << "\n  Body: " << body 
                  << "\n  Footer: " << footer << "\n";
#endif
    }
};

// ============================================================================
// C++17/20 VERSION
// ============================================================================

namespace cpp17 {

class Builder {
private:
    Product product_;
    
public:
    // ❌ Throwing на ошибки
    Builder& setHeader(const std::string& h) {
        if (h.empty()) {
            throw std::invalid_argument("Header cannot be empty");
        }
        product_.header = h;
        return *this;
    }
    
    Builder& setBody(const std::string& b) {
        if (b.length() > 1000) {
            throw std::invalid_argument("Body too large");
        }
        product_.body = b;
        return *this;
    }
    
    Builder& setFooter(const std::string& f) {
        product_.footer = f;
        return *this;
    }
    
    Product build() {
        if (product_.header.empty() || product_.body.empty()) {
            throw std::runtime_error("Incomplete product");
        }
        return product_;
    }
};

} // namespace cpp17

// ============================================================================
// C++23 VERSION
// ============================================================================

namespace cpp23 {

enum class BuildError {
    EmptyHeader,
    EmptyBody,
    BodyTooLarge,
    IncompleteProduct
};

class Builder {
private:
    Product product_;
    
public:
    // ✅ expected вместо exceptions
    expected<Builder&, BuildError> setHeader(const std::string& h) {
        if (h.empty()) {
            return unexpected(BuildError::EmptyHeader);
        }
        product_.header = h;
        return *this;
    }
    
    expected<Builder&, BuildError> setBody(const std::string& b) {
        if (b.length() > 1000) {
            return unexpected(BuildError::BodyTooLarge);
        }
        product_.body = b;
        return *this;
    }
    
    Builder& setFooter(const std::string& f) {
        product_.footer = f;
        return *this;
    }
    
    expected<Product, BuildError> build() {
        if (product_.header.empty() || product_.body.empty()) {
            return unexpected(BuildError::IncompleteProduct);
        }
        return product_;
    }
};

void demonstrate() {
#if HAS_CPP23
    std::println("\n=== C++23 Builder ===");
#else
    std::cout << "\n=== C++23 Builder ===\n";
#endif
    
    Builder builder;
    
    // ✅ Chainable error handling
    auto header_result = builder.setHeader("Title");
    if (!header_result.has_value()) {
#if HAS_CPP23
        std::print("Error setting header\n");
#else
        std::cout << "Error setting header\n";
#endif
        return;
    }
    
    auto body_result = (*header_result).setBody("Content");
    if (!body_result.has_value()) {
#if HAS_CPP23
        std::print("Error setting body\n");
#else
        std::cout << "Error setting body\n";
#endif
        return;
    }
    
    (*body_result).setFooter("End");
    
    auto product_result = builder.build();
    if (product_result.has_value()) {
        (*product_result).display();
    }
}

} // namespace cpp23

int main() {
    cpp23::demonstrate();
    
    std::cout << "\n=== C++23 IMPROVEMENTS ===\n";
    std::cout << "✅ std::expected: Fluent error handling\n";
    std::cout << "✅ std::generator: Lazy building для больших объектов\n";
    std::cout << "✅ No exception overhead\n";
    
    return 0;
}

