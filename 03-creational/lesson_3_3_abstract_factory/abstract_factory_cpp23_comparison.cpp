// ============================================================================
// СРАВНЕНИЕ: Abstract Factory в C++17/20 vs C++23
// ============================================================================
// C++23: std::generator для lazy product family creation
// ============================================================================

#include <iostream>
#include <memory>

#if __cplusplus >= 202302L
    #include <expected>
    #include <print>
    #include <generator>
    #define HAS_CPP23 1
#else
    #define HAS_CPP23 0
    template<typename T, typename E> struct expected { T value; bool has_value() const { return true; } T& operator*() { return value; } };
    template<typename E> struct unexpected { E error; };
#endif

class Widget { public: virtual ~Widget() = default; virtual void render() = 0; };
class WindowsWidget : public Widget { public: void render() override {} };

namespace cpp17 {
    // ❌ Eager creation всех products
    class Factory {
    public:
        std::vector<std::unique_ptr<Widget>> createWidgets(int count) {
            std::vector<std::unique_ptr<Widget>> widgets;
            for (int i = 0; i < count; ++i) {
                widgets.push_back(std::make_unique<WindowsWidget>());  // Все сразу
            }
            return widgets;
        }
    };
}

namespace cpp23 {
    class Factory {
    public:
#if HAS_CPP23
        // ✅ Lazy creation с generator
        std::generator<std::unique_ptr<Widget>> createWidgets(int count) {
            for (int i = 0; i < count; ++i) {
                std::print("  Creating widget {} on-demand\n", i);
                co_yield std::make_unique<WindowsWidget>();  // По требованию!
            }
        }
#endif
    };
}

int main() {
#if HAS_CPP23
    std::println("C++23: Lazy widget creation with std::generator");
    
    cpp23::Factory factory;
    auto gen = factory.createWidgets(100);
    
    int consumed = 0;
    for (auto widget : gen) {
        widget->render();
        if (++consumed >= 3) break;  // ✅ Создали только 3 из 100!
    }
    std::println("Created only {} widgets instead of 100", consumed);
#else
    std::cout << "C++23 features not available\n";
#endif
    
    return 0;
}
