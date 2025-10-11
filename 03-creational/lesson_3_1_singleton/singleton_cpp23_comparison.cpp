// ============================================================================
// СРАВНЕНИЕ: Singleton в C++17/20 vs C++23
// ============================================================================
//
// Демонстрирует улучшения C++23 для паттерна Singleton:
// 1. std::expected для error handling без exceptions
// 2. std::print для форматированного вывода
// 3. std::jthread для RAII-compliant threads
// 4. std::flat_map для thread-local storage (better cache locality)
//
// Компиляция:
//   C++23: g++ -std=c++23 singleton_cpp23_comparison.cpp
//   C++20: g++ -std=c++20 singleton_cpp23_comparison.cpp (fallback)
//
// ============================================================================

#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <map>
#include <string>
#include <chrono>

// C++23 features (with fallback для старых компиляторов)
#if __cplusplus >= 202302L
    #include <expected>
    #include <print>
    #define HAS_CPP23 1
    using std::expected;
    using std::unexpected;
#else
    #define HAS_CPP23 0
    // Fallback: простая реализация expected
    template<typename T, typename E>
    struct expected {
        T value;
        bool has_value() const { return true; }
        T& operator*() { return value; }
        const T& operator*() const { return value; }
    };
    template<typename E> struct unexpected { E error; };
#endif

// ============================================================================
// C++17/20 VERSION - Традиционный подход
// ============================================================================

namespace cpp17 {

// Ошибки через exceptions
class ConfigurationError : public std::runtime_error {
public:
    explicit ConfigurationError(const std::string& msg) 
        : std::runtime_error(msg) {}
};

class Singleton {
private:
    std::string config_path_;
    std::map<std::string, std::string> settings_;  // std::map - red-black tree
    mutable std::mutex mutex_;
    
    Singleton() {
        std::cout << "[C++17] Singleton created\n";  // Старый cout
    }
    
public:
    // Запрет копирования
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    
    static Singleton& getInstance() {
        static Singleton instance;  // Thread-safe C++11
        return instance;
    }
    
    // Инициализация с exception throwing
    void initialize(const std::string& config_path) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (config_path.empty()) {
            throw ConfigurationError("Config path cannot be empty");
        }
        
        config_path_ = config_path;
        settings_["path"] = config_path;
        
        std::cout << "Initialized with config: " << config_path << "\n";
    }
    
    // Получение значения с exception
    std::string getValue(const std::string& key) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = settings_.find(key);
        if (it == settings_.end()) {
            throw std::runtime_error("Key not found: " + key);
        }
        
        return it->second;
    }
    
    // Работа с потоками (старый std::thread)
    void processInBackground() {
        std::thread t([this]() {
            std::cout << "Background thread started\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << "Background thread finished\n";
        });
        
        t.join();  // Ручной join
    }
};

void demonstrate() {
    std::cout << "\n=== C++17/20 VERSION ===\n";
    
    auto& singleton = Singleton::getInstance();
    
    // Exception handling
    try {
        singleton.initialize("/etc/config.ini");
        
        auto value = singleton.getValue("path");
        std::cout << "Value: " << value << "\n";
        
        // Это выбросит exception
        auto missing = singleton.getValue("missing_key");
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    
    singleton.processInBackground();
}

} // namespace cpp17

// ============================================================================
// C++23 VERSION - Современный подход
// ============================================================================

namespace cpp23 {

// Ошибки через std::expected (нет exceptions!)
enum class ConfigError {
    EmptyPath,
    InvalidFormat,
    KeyNotFound,
    PermissionDenied
};

// Конвертация ошибки в строку
constexpr const char* errorToString(ConfigError err) {
    switch (err) {
        case ConfigError::EmptyPath: return "Config path cannot be empty";
        case ConfigError::InvalidFormat: return "Invalid config format";
        case ConfigError::KeyNotFound: return "Key not found";
        case ConfigError::PermissionDenied: return "Permission denied";
    }
    return "Unknown error";
}

class Singleton {
private:
    std::string config_path_;
    
#if HAS_CPP23
    // ✅ C++23: flat_map - лучшая cache locality
    std::flat_map<std::string, std::string> settings_;
#else
    std::map<std::string, std::string> settings_;
#endif
    
    mutable std::mutex mutex_;
    
    Singleton() {
#if HAS_CPP23
        std::print("[C++23] Singleton created\n");  // ✅ std::print
#else
        std::cout << "[C++23-fallback] Singleton created\n";
#endif
    }
    
public:
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    
    static Singleton& getInstance() {
        static Singleton instance;
        return instance;
    }
    
    // ✅ C++23: Возвращаем expected вместо throwing exception
    expected<void, ConfigError> initialize(const std::string& config_path) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (config_path.empty()) {
            return unexpected(ConfigError::EmptyPath);  // ✅ Error без exception
        }
        
        config_path_ = config_path;
        settings_["path"] = config_path;
        
#if HAS_CPP23
        std::print("Initialized with config: {}\n", config_path);  // ✅ Format string
#else
        std::cout << "Initialized with config: " << config_path << "\n";
#endif
        
        return {};  // Success
    }
    
    // ✅ C++23: Возвращаем expected<T, E>
    expected<std::string, ConfigError> getValue(const std::string& key) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = settings_.find(key);
        if (it == settings_.end()) {
            return unexpected(ConfigError::KeyNotFound);  // ✅ Error без exception
        }
        
        return it->second;  // Success
    }
    
    // ✅ C++23: jthread - автоматический join
    void processInBackground() {
#if HAS_CPP23
        std::jthread t([this](std::stop_token stoken) {
            std::print("Background jthread started\n");
            
            // ✅ Можем проверять stop_token
            if (!stoken.stop_requested()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
            std::print("Background jthread finished\n");
        });
        // ✅ Автоматический join при выходе из scope!
#else
        std::thread t([this]() {
            std::cout << "Background thread started\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << "Background thread finished\n";
        });
        t.join();
#endif
    }
};

void demonstrate() {
#if HAS_CPP23
    std::println("\n=== C++23 VERSION ===");
#else
    std::cout << "\n=== C++23 VERSION (with fallbacks) ===\n";
#endif
    
    auto& singleton = Singleton::getInstance();
    
    // ✅ Error handling без exceptions
    auto init_result = singleton.initialize("/etc/config.ini");
    if (!init_result.has_value()) {
#if HAS_CPP23
        std::print("Initialization failed: {}\n", 
                   errorToString(init_result.error()));
#else
        std::cout << "Initialization failed\n";
#endif
        return;
    }
    
    // ✅ Проверка результата без try-catch
    auto value_result = singleton.getValue("path");
    if (value_result.has_value()) {
#if HAS_CPP23
        std::print("Value: {}\n", *value_result);
#else
        std::cout << "Value: " << *value_result << "\n";
#endif
    }
    
    // ✅ Обработка ошибки без exception
    auto missing_result = singleton.getValue("missing_key");
    if (!missing_result.has_value()) {
#if HAS_CPP23
        std::print("Expected error: {}\n", 
                   errorToString(missing_result.error()));
#else
        std::cout << "Expected error: Key not found\n";
#endif
    }
    
    singleton.processInBackground();
}

} // namespace cpp23

// ============================================================================
// СРАВНЕНИЕ ПРОИЗВОДИТЕЛЬНОСТИ
// ============================================================================

void performanceComparison() {
    std::cout << "\n=== PERFORMANCE COMPARISON ===\n";
    
    const int iterations = 100000;
    
    // C++17: exception overhead
    {
        auto start = std::chrono::high_resolution_clock::now();
        
        auto& s = cpp17::Singleton::getInstance();
        int errors = 0;
        
        for (int i = 0; i < iterations; ++i) {
            try {
                s.getValue("nonexistent");
            } catch (...) {
                errors++;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "C++17 (exceptions): " << duration.count() << " µs\n";
        std::cout << "  Errors caught: " << errors << "\n";
    }
    
    // C++23: expected (no exceptions)
    {
        auto start = std::chrono::high_resolution_clock::now();
        
        auto& s = cpp23::Singleton::getInstance();
        int errors = 0;
        
        for (int i = 0; i < iterations; ++i) {
            auto result = s.getValue("nonexistent");
            if (!result.has_value()) {
                errors++;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "C++23 (expected):   " << duration.count() << " µs\n";
        std::cout << "  Errors handled: " << errors << "\n";
    }
    
    std::cout << "\n💡 std::expected обычно быстрее на 10-100x для error paths\n";
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    std::cout << "=== SINGLETON: C++17/20 vs C++23 COMPARISON ===\n";
    
#if HAS_CPP23
    std::println("✅ Compiling with C++23 features");
#else
    std::cout << "⚠️  C++23 features not available, using fallbacks\n";
#endif
    
    cpp17::demonstrate();
    cpp23::demonstrate();
    
    performanceComparison();
    
    std::cout << "\n=== KEY IMPROVEMENTS IN C++23 ===\n";
    std::cout << "1. std::expected<T,E> - Error handling без exceptions\n";
    std::cout << "   + Лучшая производительность (no stack unwinding)\n";
    std::cout << "   + Explicit error handling\n";
    std::cout << "   + Composable (monadic operations)\n";
    std::cout << "\n";
    std::cout << "2. std::print/println - Безопасный форматированный вывод\n";
    std::cout << "   + Type-safe (compile-time checking)\n";
    std::cout << "   + Удобный синтаксис (Python-like)\n";
    std::cout << "   + Лучше чем printf/cout\n";
    std::cout << "\n";
    std::cout << "3. std::jthread - RAII threads\n";
    std::cout << "   + Автоматический join при destruction\n";
    std::cout << "   + std::stop_token для graceful cancellation\n";
    std::cout << "   + Безопаснее std::thread\n";
    std::cout << "\n";
    std::cout << "4. std::flat_map - Плоский контейнер\n";
    std::cout << "   + Лучшая cache locality (~2-3x faster lookup)\n";
    std::cout << "   + Меньше аллокаций\n";
    std::cout << "   + Лучше для небольших maps\n";
    
    return 0;
}

// ============================================================================
// РЕЗЮМЕ МИГРАЦИИ
// ============================================================================
//
// БЫЛО (C++17/20):
// - try-catch для error handling
// - std::cout с operator<<
// - std::thread с ручным join()
// - std::map с red-black tree
//
// СТАЛО (C++23):
// - std::expected<T,E> без exceptions
// - std::print с format strings
// - std::jthread с автоматическим join
// - std::flat_map с better cache locality
//
// ПРЕИМУЩЕСТВА:
// ✅ Производительность: expected быстрее exceptions на порядки
// ✅ Безопасность: jthread не забудет join
// ✅ Читаемость: print лучше cout
// ✅ Cache-friendly: flat_map для небольших коллекций
//
// РЕКОМЕНДАЦИИ:
// - Используйте expected для recoverable errors
// - Используйте jthread вместо thread
// - Используйте print для всего вывода
// - Используйте flat_map для маленьких maps (<1000 элементов)
//
// ============================================================================

