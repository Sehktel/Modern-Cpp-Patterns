// C++17/20 vs C++23: Decorator Pattern
// C++23: std::generator для lazy decorator chains

#include <iostream>
#include <memory>
#include <vector>

#if __cplusplus >= 202302L
    #include <expected>
    #include <print>
    #include <generator>
    #define HAS_CPP23 1
#else
    #define HAS_CPP23 0
#endif

class Component { public: virtual ~Component() = default; virtual void operation() = 0; };
class ConcreteComponent : public Component { public: void operation() override { std::cout << "Base\n"; } };

class Decorator : public Component {
protected:
    std::unique_ptr<Component> component_;
public:
    Decorator(std::unique_ptr<Component> c) : component_(std::move(c)) {}
};

class LogDecorator : public Decorator {
public:
    using Decorator::Decorator;
    void operation() override {
        std::cout << "[LOG] ";
        component_->operation();
    }
};

namespace cpp17 {
    // ❌ Eager decorator chain construction
    std::unique_ptr<Component> buildChain(int depth) {
        auto component = std::make_unique<ConcreteComponent>();
        for (int i = 0; i < depth; ++i) {
            component = std::make_unique<LogDecorator>(std::move(component));  // Все сразу
        }
        return component;
    }
}

namespace cpp23 {
#if HAS_CPP23
    // ✅ Lazy decorator chain с generator
    std::generator<std::unique_ptr<Component>> buildChainLazy(int depth) {
        auto component = std::make_unique<ConcreteComponent>();
        
        for (int i = 0; i < depth; ++i) {
            std::print("  Building decorator layer {}\n", i);
            component = std::make_unique<LogDecorator>(std::move(component));
            co_yield component;  // Возвращаем промежуточные состояния
        }
    }
#endif
}

int main() {
#if HAS_CPP23
    std::println("C++23: Lazy decorator construction");
    
    for (auto comp : cpp23::buildChainLazy(3)) {
        // Можем inspect каждый уровень
    }
#else
    std::cout << "C++17 version\n";
    auto chain = cpp17::buildChain(3);
    chain->operation();
#endif
    
    std::cout << "✅ C++23: generator для lazy decorator chains\n";
    return 0;
}

