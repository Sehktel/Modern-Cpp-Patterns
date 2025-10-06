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

/**
 * @file circuit_breaker_pattern.cpp
 * @brief Демонстрация Circuit Breaker Pattern
 * 
 * Этот файл показывает полную реализацию Circuit Breaker с различными
 * примерами использования для защиты от каскадных отказов.
 */

// ============================================================================
// CIRCUIT BREAKER IMPLEMENTATION
// ============================================================================

/**
 * @brief Состояния Circuit Breaker
 */
enum class CircuitState {
    CLOSED,    // Нормальная работа
    OPEN,      // Блокировка вызовов
    HALF_OPEN  // Тестовый режим
};

/**
 * @brief Универсальный Circuit Breaker
 */
template<typename T>
class CircuitBreaker {
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
    CircuitBreaker(const std::string& name, 
                   int failureThreshold = 5,
                   int successThreshold = 3,
                   std::chrono::milliseconds timeout = std::chrono::seconds(30))
        : name_(name), failureThreshold_(failureThreshold), 
          successThreshold_(successThreshold), timeout_(timeout) {
        
        std::cout << "[" << name_ << "] 🔧 Circuit Breaker создан: "
                  << "failureThreshold=" << failureThreshold_
                  << ", successThreshold=" << successThreshold_
                  << ", timeout=" << timeout_.count() << "ms" << std::endl;
    }
    
    // Выполнение операции через Circuit Breaker
    std::optional<T> execute(std::function<T()> operation) {
        totalRequests_.fetch_add(1);
        
        if (!canExecute()) {
            rejectedRequests_.fetch_add(1);
            std::cout << "[" << name_ << "] 🚫 Запрос отклонен (состояние: " 
                      << stateToString(state_.load()) << ")" << std::endl;
            return std::nullopt;
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
            throw;
        }
    }
    
    // Асинхронная версия
    template<typename Callback>
    void executeAsync(std::function<T()> operation, Callback callback) {
        std::thread([this, operation, callback]() {
            try {
                auto result = execute(operation);
                callback(result);
            } catch (const std::exception& e) {
                callback(std::nullopt);
            }
        }).detach();
    }
    
    // Проверка состояния
    CircuitState getState() const {
        return state_.load();
    }
    
    bool isClosed() const {
        return state_.load() == CircuitState::CLOSED;
    }
    
    bool isOpen() const {
        return state_.load() == CircuitState::OPEN;
    }
    
    bool isHalfOpen() const {
        return state_.load() == CircuitState::HALF_OPEN;
    }
    
    std::string getStateString() const {
        return stateToString(state_.load());
    }
    
    // Принудительное изменение состояния (для тестирования)
    void forceState(CircuitState newState) {
        state_.store(newState);
        std::cout << "[" << name_ << "] 🔄 Принудительно изменено состояние на: " 
                  << stateToString(newState) << std::endl;
    }
    
    // Сброс состояния
    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        state_.store(CircuitState::CLOSED);
        failureCount_.store(0);
        successCount_.store(0);
        std::cout << "[" << name_ << "] 🔄 Состояние сброшено" << std::endl;
    }
    
    // Статистика
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
    
    void printStatistics() const {
        auto stats = getStatistics();
        std::cout << "\n=== СТАТИСТИКА CIRCUIT BREAKER [" << name_ << "] ===" << std::endl;
        std::cout << "Состояние: " << stateToString(stats.state) << std::endl;
        std::cout << "Всего запросов: " << stats.totalRequests << std::endl;
        std::cout << "Успешных: " << stats.totalSuccesses << std::endl;
        std::cout << "Неудачных: " << stats.totalFailures << std::endl;
        std::cout << "Отклоненных: " << stats.rejectedRequests << std::endl;
        std::cout << "Текущие счетчики: failures=" << stats.currentFailureCount 
                  << ", successes=" << stats.currentSuccessCount << std::endl;
        std::cout << "Процент успеха: " << (stats.successRate * 100) << "%" << std::endl;
        std::cout << "Процент неудач: " << (stats.failureRate * 100) << "%" << std::endl;
        std::cout << "===============================================" << std::endl;
    }
    
private:
    bool canExecute() {
        CircuitState currentState = state_.load();
        
        switch (currentState) {
            case CircuitState::CLOSED:
                return true;
                
            case CircuitState::OPEN:
                // Проверяем, не истек ли timeout
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
                // Переходим в CLOSED состояние
                state_.store(CircuitState::CLOSED);
                failureCount_.store(0);
                successCount_.store(0);
                
                std::cout << "[" << name_ << "] ✅ Переход в состояние CLOSED (сервис восстановлен)" << std::endl;
            }
        } else if (currentState == CircuitState::CLOSED) {
            // Сбрасываем счетчик ошибок при успешном вызове
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
                // Переходим в OPEN состояние
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
};

// ============================================================================
// ПРИМЕРЫ ИСПОЛЬЗОВАНИЯ
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
    
    std::string callService(const std::string& request) {
        // Имитация времени обработки
        std::this_thread::sleep_for(std::chrono::milliseconds(100 + (generator_() % 200)));
        
        // Имитация случайных ошибок
        if (distribution_(generator_) < failureRate_) {
            throw std::runtime_error("Сервис недоступен: " + request);
        }
        
        return "Ответ от сервиса для запроса: " + request;
    }
    
    void setFailureRate(double rate) {
        failureRate_ = rate;
        std::cout << "[UnstableService] Установлен процент ошибок: " << (rate * 100) << "%" << std::endl;
    }
};

/**
 * @brief Сервис с Circuit Breaker
 */
class ResilientService {
private:
    CircuitBreaker<std::string> circuitBreaker_;
    UnstableService service_;
    
public:
    ResilientService(const std::string& name, double failureRate = 0.3)
        : circuitBreaker_(name, 3, 2, std::chrono::seconds(5)),
          service_(failureRate) {}
    
    std::optional<std::string> callService(const std::string& request) {
        return circuitBreaker_.execute([this, &request]() {
            return service_.callService(request);
        });
    }
    
    void setFailureRate(double rate) {
        service_.setFailureRate(rate);
    }
    
    void printStatistics() const {
        circuitBreaker_.printStatistics();
    }
    
    auto getStatistics() const {
        return circuitBreaker_.getStatistics();
    }
    
    void reset() {
        circuitBreaker_.reset();
    }
};

/**
 * @brief Система с множественными сервисами
 */
class ServiceOrchestrator {
private:
    std::unordered_map<std::string, std::unique_ptr<ResilientService>> services_;
    
public:
    void addService(const std::string& name, double failureRate = 0.3) {
        services_[name] = std::make_unique<ResilientService>(name, failureRate);
        std::cout << "[Orchestrator] ➕ Добавлен сервис: " << name << std::endl;
    }
    
    std::optional<std::string> callService(const std::string& serviceName, const std::string& request) {
        auto it = services_.find(serviceName);
        if (it != services_.end()) {
            return it->second->callService(request);
        } else {
            std::cout << "[Orchestrator] ❓ Сервис не найден: " << serviceName << std::endl;
            return std::nullopt;
        }
    }
    
    void setServiceFailureRate(const std::string& serviceName, double rate) {
        auto it = services_.find(serviceName);
        if (it != services_.end()) {
            it->second->setFailureRate(rate);
        }
    }
    
    void printAllStatistics() {
        std::cout << "\n=== СТАТИСТИКА ВСЕХ СЕРВИСОВ ===" << std::endl;
        for (const auto& pair : services_) {
            pair.second->printStatistics();
        }
    }
    
    void resetAllServices() {
        for (const auto& pair : services_) {
            pair.second->reset();
        }
        std::cout << "[Orchestrator] 🔄 Все сервисы сброшены" << std::endl;
    }
};

// ============================================================================
// ДЕМОНСТРАЦИОННЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Демонстрация базового Circuit Breaker
 */
void demonstrateBasicCircuitBreaker() {
    std::cout << "\n=== БАЗОВЫЙ CIRCUIT BREAKER ===" << std::endl;
    
    ResilientService service("TestService", 0.4); // 40% ошибок
    
    // Выполняем множество запросов
    for (int i = 1; i <= 20; ++i) {
        std::string request = "Request_" + std::to_string(i);
        auto result = service.callService(request);
        
        if (result) {
            std::cout << "✅ Успех: " << *result << std::endl;
        } else {
            std::cout << "❌ Запрос отклонен Circuit Breaker" << std::endl;
        }
        
        // Небольшая задержка между запросами
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    service.printStatistics();
}

/**
 * @brief Демонстрация восстановления сервиса
 */
void demonstrateServiceRecovery() {
    std::cout << "\n=== ВОССТАНОВЛЕНИЕ СЕРВИСА ===" << std::endl;
    
    ResilientService service("RecoveryService", 0.8); // 80% ошибок
    
    // Фаза 1: Сервис падает
    std::cout << "Фаза 1: Сервис работает нестабильно (80% ошибок)" << std::endl;
    for (int i = 1; i <= 10; ++i) {
        auto result = service.callService("Request_" + std::to_string(i));
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    
    service.printStatistics();
    
    // Фаза 2: Ждем timeout
    std::cout << "\nФаза 2: Ждем timeout для перехода в HALF_OPEN..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(6));
    
    // Фаза 3: Сервис восстанавливается
    std::cout << "\nФаза 3: Сервис восстанавливается (0% ошибок)" << std::endl;
    service.setFailureRate(0.0);
    
    for (int i = 11; i <= 20; ++i) {
        auto result = service.callService("Request_" + std::to_string(i));
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    
    service.printStatistics();
}

/**
 * @brief Демонстрация множественных сервисов
 */
void demonstrateMultipleServices() {
    std::cout << "\n=== МНОЖЕСТВЕННЫЕ СЕРВИСЫ ===" << std::endl;
    
    ServiceOrchestrator orchestrator;
    
    // Добавляем сервисы с разными уровнями надежности
    orchestrator.addService("UserService", 0.1);    // 10% ошибок
    orchestrator.addService("OrderService", 0.3);   // 30% ошибок
    orchestrator.addService("PaymentService", 0.6); // 60% ошибок
    
    // Выполняем запросы к разным сервисам
    std::vector<std::string> services = {"UserService", "OrderService", "PaymentService"};
    
    for (int round = 1; round <= 3; ++round) {
        std::cout << "\n--- Раунд " << round << " ---" << std::endl;
        
        for (const auto& serviceName : services) {
            for (int i = 1; i <= 3; ++i) {
                std::string request = "Request_" + std::to_string(i);
                auto result = orchestrator.callService(serviceName, request);
                
                if (result) {
                    std::cout << "✅ " << serviceName << ": " << *result << std::endl;
                } else {
                    std::cout << "❌ " << serviceName << ": Запрос отклонен" << std::endl;
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }
    }
    
    orchestrator.printAllStatistics();
}

/**
 * @brief Демонстрация адаптивного поведения
 */
void demonstrateAdaptiveBehavior() {
    std::cout << "\n=== АДАПТИВНОЕ ПОВЕДЕНИЕ ===" << std::endl;
    
    ResilientService service("AdaptiveService", 0.2);
    
    // Фаза 1: Нормальная работа
    std::cout << "Фаза 1: Нормальная работа (20% ошибок)" << std::endl;
    for (int i = 1; i <= 5; ++i) {
        auto result = service.callService("Normal_" + std::to_string(i));
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    
    // Фаза 2: Увеличиваем нагрузку
    std::cout << "\nФаза 2: Увеличиваем процент ошибок (60% ошибок)" << std::endl;
    service.setFailureRate(0.6);
    
    for (int i = 6; i <= 10; ++i) {
        auto result = service.callService("HighLoad_" + std::to_string(i));
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    
    // Фаза 3: Критическая ситуация
    std::cout << "\nФаза 3: Критическая ситуация (90% ошибок)" << std::endl;
    service.setFailureRate(0.9);
    
    for (int i = 11; i <= 15; ++i) {
        auto result = service.callService("Critical_" + std::to_string(i));
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    
    // Фаза 4: Восстановление
    std::cout << "\nФаза 4: Восстановление после критической ситуации" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(6)); // Ждем timeout
    service.setFailureRate(0.1); // Восстанавливаемся
    
    for (int i = 16; i <= 20; ++i) {
        auto result = service.callService("Recovery_" + std::to_string(i));
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    
    service.printStatistics();
}

// ============================================================================
// ОСНОВНАЯ ФУНКЦИЯ
// ============================================================================

int main() {
    std::cout << "🔌 Демонстрация Circuit Breaker Pattern" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    try {
        demonstrateBasicCircuitBreaker();
        demonstrateServiceRecovery();
        demonstrateMultipleServices();
        demonstrateAdaptiveBehavior();
        
        std::cout << "\n✅ Все демонстрации завершены успешно!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n🎯 Рекомендации:" << std::endl;
    std::cout << "1. Используйте Circuit Breaker для защиты от каскадных отказов" << std::endl;
    std::cout << "2. Настройте пороги под конкретные сервисы и требования" << std::endl;
    std::cout << "3. Реализуйте fallback механизмы при открытом Circuit Breaker" << std::endl;
    std::cout << "4. Мониторьте состояние и статистику Circuit Breakers" << std::endl;
    std::cout << "5. Тестируйте поведение при различных сценариях отказов" << std::endl;
    
    return 0;
}
