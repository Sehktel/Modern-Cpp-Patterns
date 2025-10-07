/**
 * @file circuit_breaker_cpp23_comparison.cpp
 * @brief Сравнение Circuit Breaker Pattern: C++17/20 vs C++23
 * 
 * Этот файл демонстрирует различия между текущей реализацией
 * и улучшенной версией с использованием возможностей C++23.
 */

#include <iostream>
#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <random>
#include <vector>
#include <unordered_map>
#include <memory>

// ============================================================================
// C++17/20 РЕАЛИЗАЦИЯ (ТЕКУЩАЯ)
// ============================================================================

/**
 * @brief Текущая реализация Circuit Breaker (C++17/20)
 */
enum class CircuitState {
    CLOSED,    // Нормальная работа
    OPEN,      // Блокировка вызовов
    HALF_OPEN  // Тестовый режим
};

template<typename T>
class CircuitBreakerCpp20 {
private:
    std::string name_;
    std::atomic<CircuitState> state_{CircuitState::CLOSED};
    std::atomic<int> failureCount_{0};
    std::atomic<int> successCount_{0};
    
    int failureThreshold_;
    int successThreshold_;
    std::chrono::milliseconds timeout_;
    
    std::chrono::system_clock::time_point lastFailureTime_;
    std::mutex mutex_;
    
    // Статистика
    std::atomic<size_t> totalRequests_{0};
    std::atomic<size_t> totalFailures_{0};
    std::atomic<size_t> totalSuccesses_{0};
    std::atomic<size_t> rejectedRequests_{0};
    
public:
    CircuitBreakerCpp20(const std::string& name, 
                        int failureThreshold = 5,
                        int successThreshold = 3,
                        std::chrono::milliseconds timeout = std::chrono::seconds(30))
        : name_(name), failureThreshold_(failureThreshold), 
          successThreshold_(successThreshold), timeout_(timeout) {
        
        // ❌ C++17/20: Многострочный вывод с std::cout
        std::cout << "[" << name_ << "] 🔧 Circuit Breaker создан: "
                  << "failureThreshold=" << failureThreshold_
                  << ", successThreshold=" << successThreshold_
                  << ", timeout=" << timeout_.count() << "ms" << std::endl;
    }
    
    // ❌ C++17/20: std::optional без информации об ошибке
    std::optional<T> execute(std::function<T()> operation) {
        totalRequests_.fetch_add(1);
        
        if (!canExecute()) {
            rejectedRequests_.fetch_add(1);
            std::cout << "[" << name_ << "] 🚫 Запрос отклонен (состояние: " 
                      << stateToString(state_.load()) << ")" << std::endl;
            return std::nullopt; // ❌ Нет информации об ошибке
        }
        
        std::cout << "[" << name_ << "] ✅ Выполняю операцию (состояние: " 
                  << stateToString(state_.load()) << ")" << std::endl;
        
        try {
            T result = operation();
            onSuccess();
            return result;
        } catch (const std::exception& e) {
            onFailure();
            std::cout << "[" << name_ << "] ❌ Ошибка выполнения: " << e.what() << std::endl;
            throw; // ❌ Исключение вместо возврата ошибки
        }
    }
    
    // ❌ C++17/20: Простая статистика без детализации
    void printStatistics() const {
        auto stats = getStatistics();
        std::cout << "\n=== СТАТИСТИКА CIRCUIT BREAKER [" << name_ << "] ===" << std::endl;
        std::cout << "Состояние: " << stateToString(stats.state) << std::endl;
        std::cout << "Всего запросов: " << stats.totalRequests << std::endl;
        std::cout << "Успешных: " << stats.totalSuccesses << std::endl;
        std::cout << "Неудачных: " << stats.totalFailures << std::endl;
        std::cout << "Отклоненных: " << stats.rejectedRequests << std::endl;
        std::cout << "Процент успеха: " << (stats.successRate * 100) << "%" << std::endl;
        std::cout << "===============================================" << std::endl;
    }
    
private:
    bool canExecute() {
        CircuitState currentState = state_.load();
        
        switch (currentState) {
            case CircuitState::CLOSED:
                return true;
                
            case CircuitState::OPEN:
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    if (std::chrono::system_clock::now() - lastFailureTime_ >= timeout_) {
                        state_.store(CircuitState::HALF_OPEN);
                        successCount_.store(0);
                        std::cout << "[" << name_ << "] 🔄 Переход в состояние HALF_OPEN (timeout истек)" << std::endl;
                        return true;
                    }
                }
                return false;
                
            case CircuitState::HALF_OPEN:
                return true;
                
            default:
                return false;
        }
    }
    
    void onSuccess() {
        totalSuccesses_.fetch_add(1);
        
        CircuitState currentState = state_.load();
        
        if (currentState == CircuitState::HALF_OPEN) {
            int currentSuccessCount = successCount_.fetch_add(1) + 1;
            
            std::cout << "[" << name_ << "] ✅ Успех в HALF_OPEN режиме (" 
                      << currentSuccessCount << "/" << successThreshold_ << ")" << std::endl;
            
            if (currentSuccessCount >= successThreshold_) {
                state_.store(CircuitState::CLOSED);
                failureCount_.store(0);
                successCount_.store(0);
                
                std::cout << "[" << name_ << "] ✅ Переход в состояние CLOSED (сервис восстановлен)" << std::endl;
            }
        } else if (currentState == CircuitState::CLOSED) {
            failureCount_.store(0);
        }
    }
    
    void onFailure() {
        totalFailures_.fetch_add(1);
        
        std::lock_guard<std::mutex> lock(mutex_);
        lastFailureTime_ = std::chrono::system_clock::now();
        
        CircuitState currentState = state_.load();
        
        if (currentState == CircuitState::CLOSED || currentState == CircuitState::HALF_OPEN) {
            int currentFailureCount = failureCount_.fetch_add(1) + 1;
            
            std::cout << "[" << name_ << "] ❌ Неудача (" 
                      << currentFailureCount << "/" << failureThreshold_ << ")" << std::endl;
            
            if (currentFailureCount >= failureThreshold_) {
                state_.store(CircuitState::OPEN);
                std::cout << "[" << name_ << "] ❌ Переход в состояние OPEN (сервис недоступен)" << std::endl;
            }
        }
    }
    
    std::string stateToString(CircuitState state) const {
        switch (state) {
            case CircuitState::CLOSED: return "CLOSED";
            case CircuitState::OPEN: return "OPEN";
            case CircuitState::HALF_OPEN: return "HALF_OPEN";
            default: return "UNKNOWN";
        }
    }
    
    struct Statistics {
        CircuitState state;
        size_t totalRequests;
        size_t totalFailures;
        size_t totalSuccesses;
        size_t rejectedRequests;
        double failureRate;
        double successRate;
        int currentFailureCount;
        int currentSuccessCount;
    };
    
    Statistics getStatistics() const {
        Statistics stats;
        stats.state = state_.load();
        stats.totalRequests = totalRequests_.load();
        stats.totalFailures = totalFailures_.load();
        stats.totalSuccesses = totalSuccesses_.load();
        stats.rejectedRequests = rejectedRequests_.load();
        stats.currentFailureCount = failureCount_.load();
        stats.currentSuccessCount = successCount_.load();
        
        if (stats.totalRequests > 0) {
            stats.failureRate = static_cast<double>(stats.totalFailures) / stats.totalRequests;
            stats.successRate = static_cast<double>(stats.totalSuccesses) / stats.totalRequests;
        } else {
            stats.failureRate = 0.0;
            stats.successRate = 0.0;
        }
        
        return stats;
    }
};

// ============================================================================
// C++23 РЕАЛИЗАЦИЯ (УЛУЧШЕННАЯ)
// ============================================================================

// ✅ C++23: Новые заголовки
#ifdef __cpp_lib_expected
#include <expected>
#endif

#ifdef __cpp_lib_print
#include <print>
#endif

#ifdef __cpp_lib_stacktrace
#include <stacktrace>
#endif

/**
 * @brief Улучшенная реализация Circuit Breaker (C++23)
 */
template<typename T, typename E = std::string>
class CircuitBreakerCpp23 {
private:
    std::string name_;
    std::atomic<CircuitState> state_{CircuitState::CLOSED};
    std::atomic<int> failureCount_{0};
    std::atomic<int> successCount_{0};
    
    int failureThreshold_;
    int successThreshold_;
    std::chrono::milliseconds timeout_;
    
    std::chrono::system_clock::time_point lastFailureTime_;
    std::mutex mutex_;
    
    // Статистика
    std::atomic<size_t> totalRequests_{0};
    std::atomic<size_t> totalFailures_{0};
    std::atomic<size_t> totalSuccesses_{0};
    std::atomic<size_t> rejectedRequests_{0};
    
public:
    CircuitBreakerCpp23(const std::string& name, 
                        int failureThreshold = 5,
                        int successThreshold = 3,
                        std::chrono::milliseconds timeout = std::chrono::seconds(30))
        : name_(name), failureThreshold_(failureThreshold), 
          successThreshold_(successThreshold), timeout_(timeout) {
        
        // ✅ C++23: Красивый вывод с std::print
#ifdef __cpp_lib_print
        std::print("[{}] 🔧 Circuit Breaker создан: failureThreshold={}, successThreshold={}, timeout={}ms\n",
                   name_, failureThreshold_, successThreshold_, timeout_.count());
#else
        std::cout << "[" << name_ << "] 🔧 Circuit Breaker создан: "
                  << "failureThreshold=" << failureThreshold_
                  << ", successThreshold=" << successThreshold_
                  << ", timeout=" << timeout_.count() << "ms" << std::endl;
#endif
    }
    
    // ✅ C++23: std::expected с информацией об ошибке
    std::expected<T, E> execute(std::function<std::expected<T, E>()> operation) {
        totalRequests_.fetch_add(1);
        
        if (!canExecute()) {
            rejectedRequests_.fetch_add(1);
            E error = "Circuit breaker is " + stateToString(state_.load());
            
#ifdef __cpp_lib_print
            std::print("[{}] 🚫 Запрос отклонен: {}\n", name_, error);
#else
            std::cout << "[" << name_ << "] 🚫 Запрос отклонен: " << error << std::endl;
#endif
            
            return std::unexpected(std::move(error)); // ✅ Четкая информация об ошибке
        }
        
#ifdef __cpp_lib_print
        std::print("[{}] ✅ Выполняю операцию (состояние: {})\n", name_, stateToString(state_.load()));
#else
        std::cout << "[" << name_ << "] ✅ Выполняю операцию (состояние: " 
                  << stateToString(state_.load()) << ")" << std::endl;
#endif
        
        auto result = operation();
        if (result) {
            onSuccess();
        } else {
            onFailure(result.error());
        }
        
        return result; // ✅ Возвращаем результат с ошибкой
    }
    
    // ✅ C++23: Улучшенная статистика с форматированием
    void printStatistics() const {
        auto stats = getStatistics();
        
#ifdef __cpp_lib_print
        std::print("\n=== СТАТИСТИКА CIRCUIT BREAKER [{}] ===\n", name_);
        std::print("Состояние: {}\n", stateToString(stats.state));
        std::print("Всего запросов: {}\n", stats.totalRequests);
        std::print("Успешных: {}\n", stats.totalSuccesses);
        std::print("Неудачных: {}\n", stats.totalFailures);
        std::print("Отклоненных: {}\n", stats.rejectedRequests);
        std::print("Процент успеха: {:.2f}%\n", stats.successRate * 100);
        std::print("Процент неудач: {:.2f}%\n", stats.failureRate * 100);
        std::print("===============================================\n");
#else
        std::cout << "\n=== СТАТИСТИКА CIRCUIT BREAKER [" << name_ << "] ===" << std::endl;
        std::cout << "Состояние: " << stateToString(stats.state) << std::endl;
        std::cout << "Всего запросов: " << stats.totalRequests << std::endl;
        std::cout << "Успешных: " << stats.totalSuccesses << std::endl;
        std::cout << "Неудачных: " << stats.totalFailures << std::endl;
        std::cout << "Отклоненных: " << stats.rejectedRequests << std::endl;
        std::cout << "Процент успеха: " << (stats.successRate * 100) << "%" << std::endl;
        std::cout << "===============================================" << std::endl;
#endif
    }
    
    // ✅ C++23: Метод для получения детальной информации об ошибках
    std::vector<E> getRecentErrors() const {
        // В реальной реализации здесь бы хранились последние ошибки
        return {};
    }
    
private:
    bool canExecute() {
        CircuitState currentState = state_.load();
        
        switch (currentState) {
            case CircuitState::CLOSED:
                return true;
                
            case CircuitState::OPEN:
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    if (std::chrono::system_clock::now() - lastFailureTime_ >= timeout_) {
                        state_.store(CircuitState::HALF_OPEN);
                        successCount_.store(0);
                        
#ifdef __cpp_lib_print
                        std::print("[{}] 🔄 Переход в состояние HALF_OPEN (timeout истек)\n", name_);
#else
                        std::cout << "[" << name_ << "] 🔄 Переход в состояние HALF_OPEN (timeout истек)" << std::endl;
#endif
                        return true;
                    }
                }
                return false;
                
            case CircuitState::HALF_OPEN:
                return true;
                
            default:
                return false;
        }
    }
    
    void onSuccess() {
        totalSuccesses_.fetch_add(1);
        
        CircuitState currentState = state_.load();
        
        if (currentState == CircuitState::HALF_OPEN) {
            int currentSuccessCount = successCount_.fetch_add(1) + 1;
            
#ifdef __cpp_lib_print
            std::print("[{}] ✅ Успех в HALF_OPEN режиме ({}/{})\n", 
                       name_, currentSuccessCount, successThreshold_);
#else
            std::cout << "[" << name_ << "] ✅ Успех в HALF_OPEN режиме (" 
                      << currentSuccessCount << "/" << successThreshold_ << ")" << std::endl;
#endif
            
            if (currentSuccessCount >= successThreshold_) {
                state_.store(CircuitState::CLOSED);
                failureCount_.store(0);
                successCount_.store(0);
                
#ifdef __cpp_lib_print
                std::print("[{}] ✅ Переход в состояние CLOSED (сервис восстановлен)\n", name_);
#else
                std::cout << "[" << name_ << "] ✅ Переход в состояние CLOSED (сервис восстановлен)" << std::endl;
#endif
            }
        } else if (currentState == CircuitState::CLOSED) {
            failureCount_.store(0);
        }
    }
    
    void onFailure(const E& error) {
        totalFailures_.fetch_add(1);
        
        std::lock_guard<std::mutex> lock(mutex_);
        lastFailureTime_ = std::chrono::system_clock::now();
        
        // ✅ C++23: Детальная информация об ошибке
#ifdef __cpp_lib_stacktrace
        auto trace = std::stacktrace::current();
        std::print("❌ Circuit Breaker failure: {}\n", error);
        std::print("Stack trace:\n");
        for (const auto& frame : trace) {
            std::print("  {}\n", frame.description());
        }
#endif
        
        CircuitState currentState = state_.load();
        
        if (currentState == CircuitState::CLOSED || currentState == CircuitState::HALF_OPEN) {
            int currentFailureCount = failureCount_.fetch_add(1) + 1;
            
#ifdef __cpp_lib_print
            std::print("[{}] ❌ Неудача ({}/{}): {}\n", 
                       name_, currentFailureCount, failureThreshold_, error);
#else
            std::cout << "[" << name_ << "] ❌ Неудача (" 
                      << currentFailureCount << "/" << failureThreshold_ << "): " << error << std::endl;
#endif
            
            if (currentFailureCount >= failureThreshold_) {
                state_.store(CircuitState::OPEN);
                
#ifdef __cpp_lib_print
                std::print("[{}] ❌ Переход в состояние OPEN (сервис недоступен)\n", name_);
#else
                std::cout << "[" << name_ << "] ❌ Переход в состояние OPEN (сервис недоступен)" << std::endl;
#endif
            }
        }
    }
    
    std::string stateToString(CircuitState state) const {
        switch (state) {
            case CircuitState::CLOSED: return "CLOSED";
            case CircuitState::OPEN: return "OPEN";
            case CircuitState::HALF_OPEN: return "HALF_OPEN";
            default: return "UNKNOWN";
        }
    }
    
    struct Statistics {
        CircuitState state;
        size_t totalRequests;
        size_t totalFailures;
        size_t totalSuccesses;
        size_t rejectedRequests;
        double failureRate;
        double successRate;
        int currentFailureCount;
        int currentSuccessCount;
    };
    
    Statistics getStatistics() const {
        Statistics stats;
        stats.state = state_.load();
        stats.totalRequests = totalRequests_.load();
        stats.totalFailures = totalFailures_.load();
        stats.totalSuccesses = totalSuccesses_.load();
        stats.rejectedRequests = rejectedRequests_.load();
        stats.currentFailureCount = failureCount_.load();
        stats.currentSuccessCount = successCount_.load();
        
        if (stats.totalRequests > 0) {
            stats.failureRate = static_cast<double>(stats.totalFailures) / stats.totalRequests;
            stats.successRate = static_cast<double>(stats.totalSuccesses) / stats.totalRequests;
        } else {
            stats.failureRate = 0.0;
            stats.successRate = 0.0;
        }
        
        return stats;
    }
};

// ============================================================================
// ДЕМОНСТРАЦИЯ РАЗЛИЧИЙ
// ============================================================================

/**
 * @brief Имитация нестабильного сервиса
 */
class UnstableService {
private:
    std::mt19937 generator_;
    std::uniform_real_distribution<double> distribution_;
    double failureRate_;
    
public:
    UnstableService(double failureRate = 0.3) 
        : generator_(std::random_device{}()),
          distribution_(0.0, 1.0),
          failureRate_(failureRate) {}
    
    // C++17/20 версия
    std::string callServiceCpp20(const std::string& request) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        if (distribution_(generator_) < failureRate_) {
            throw std::runtime_error("Сервис недоступен: " + request);
        }
        
        return "Ответ от сервиса для запроса: " + request;
    }
    
    // C++23 версия
    std::expected<std::string, std::string> callServiceCpp23(const std::string& request) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        if (distribution_(generator_) < failureRate_) {
            return std::unexpected("Сервис недоступен: " + request);
        }
        
        return "Ответ от сервиса для запроса: " + request;
    }
};

/**
 * @brief Демонстрация различий между версиями
 */
void demonstrateDifferences() {
    std::cout << "\n=== СРАВНЕНИЕ C++17/20 vs C++23 ===" << std::endl;
    
    UnstableService service(0.4); // 40% ошибок
    
    // ============================================================================
    // C++17/20 ДЕМОНСТРАЦИЯ
    // ============================================================================
    std::cout << "\n--- C++17/20 Circuit Breaker ---" << std::endl;
    
    CircuitBreakerCpp20<std::string> cb20("TestService20");
    
    for (int i = 1; i <= 5; ++i) {
        std::string request = "Request_" + std::to_string(i);
        
        // ❌ C++17/20: Неудобная обработка ошибок
        auto result = cb20.execute([&service, &request]() {
            return service.callServiceCpp20(request);
        });
        
        if (result) {
            std::cout << "✅ Успех: " << *result << std::endl;
        } else {
            std::cout << "❌ Запрос отклонен Circuit Breaker" << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    cb20.printStatistics();
    
    // ============================================================================
    // C++23 ДЕМОНСТРАЦИЯ
    // ============================================================================
    std::cout << "\n--- C++23 Circuit Breaker ---" << std::endl;
    
    CircuitBreakerCpp23<std::string, std::string> cb23("TestService23");
    
    for (int i = 1; i <= 5; ++i) {
        std::string request = "Request_" + std::to_string(i);
        
        // ✅ C++23: Удобная обработка ошибок
        auto result = cb23.execute([&service, &request]() {
            return service.callServiceCpp23(request);
        });
        
        if (result) {
            std::cout << "✅ Успех: " << *result << std::endl;
        } else {
            std::cout << "❌ Ошибка: " << result.error() << std::endl; // ✅ Четкая информация об ошибке
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    cb23.printStatistics();
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🔌 Сравнение Circuit Breaker Pattern: C++17/20 vs C++23" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    
    try {
        demonstrateDifferences();
        
        std::cout << "\n✅ Сравнение завершено!" << std::endl;
        
        std::cout << "\n🎯 Ключевые улучшения C++23:" << std::endl;
        std::cout << "1. std::expected вместо std::optional - четкая информация об ошибках" << std::endl;
        std::cout << "2. std::print - более читаемый и эффективный вывод" << std::endl;
        std::cout << "3. std::stacktrace - детальная отладка ошибок" << std::endl;
        std::cout << "4. Улучшенное форматирование с std::format" << std::endl;
        std::cout << "5. Более безопасная обработка ошибок" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

