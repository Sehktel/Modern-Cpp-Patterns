/**
 * @file resilient_client.cpp
 * @brief Устойчивый клиент с Circuit Breaker
 * 
 * Реализован устойчивый клиент с поддержкой:
 * - HTTP клиент с Circuit Breaker
 * - Database клиент с Circuit Breaker
 * - Retry логика
 * - Fallback механизмы
 */

#include <iostream>
#include <string>
#include <memory>
#include <mutex>
#include <chrono>
#include <atomic>
#include <thread>
#include <queue>
#include <functional>
#include <random>
#include <optional>

// Состояния Circuit Breaker
enum class CircuitState {
    CLOSED,       // Нормальная работа
    OPEN,         // Отказ, запросы блокируются
    HALF_OPEN     // Восстановление, пробные запросы
};

// Конфигурация Circuit Breaker
struct CircuitBreakerConfig {
    size_t failure_threshold;           // Порог ошибок для открытия
    size_t success_threshold;           // Порог успехов для закрытия
    std::chrono::milliseconds timeout;  // Таймаут в открытом состоянии
    std::chrono::milliseconds request_timeout;  // Таймаут запроса
    
    CircuitBreakerConfig(
        size_t fail_threshold = 5,
        size_t success_threshold = 2,
        std::chrono::milliseconds circuit_timeout = std::chrono::seconds(10),
        std::chrono::milliseconds req_timeout = std::chrono::seconds(5))
        : failure_threshold(fail_threshold),
          success_threshold(success_threshold),
          timeout(circuit_timeout),
          request_timeout(req_timeout) {}
};

// Circuit Breaker с метриками
class CircuitBreaker {
private:
    std::string name_;
    CircuitBreakerConfig config_;
    
    std::atomic<CircuitState> state_{CircuitState::CLOSED};
    std::atomic<size_t> failure_count_{0};
    std::atomic<size_t> success_count_{0};
    std::chrono::system_clock::time_point last_failure_time_;
    
    mutable std::mutex mutex_;
    
    // Статистика
    std::atomic<size_t> total_requests_{0};
    std::atomic<size_t> successful_requests_{0};
    std::atomic<size_t> failed_requests_{0};
    std::atomic<size_t> rejected_requests_{0};
    
public:
    explicit CircuitBreaker(const std::string& name, 
                           const CircuitBreakerConfig& config = CircuitBreakerConfig())
        : name_(name), config_(config) {
        last_failure_time_ = std::chrono::system_clock::now();
        std::cout << "Circuit Breaker '" << name_ << "' создан" << std::endl;
    }
    
    // Выполнение операции через Circuit Breaker
    template<typename F>
    auto execute(F&& func) -> decltype(func()) {
        total_requests_.fetch_add(1);
        
        // Проверяем состояние
        if (state_.load() == CircuitState::OPEN) {
            // Проверяем, не пора ли перейти в HALF_OPEN
            if (shouldAttemptReset()) {
                std::cout << "[" << name_ << "] Переход OPEN -> HALF_OPEN" << std::endl;
                state_.store(CircuitState::HALF_OPEN);
            } else {
                rejected_requests_.fetch_add(1);
                throw std::runtime_error("Circuit Breaker OPEN: запрос отклонен");
            }
        }
        
        try {
            // Выполняем операцию
            auto result = func();
            
            // Успех
            onSuccess();
            return result;
            
        } catch (const std::exception& e) {
            // Неудача
            onFailure();
            throw;
        }
    }
    
    // Получение текущего состояния
    CircuitState getState() const {
        return state_.load();
    }
    
    std::string getStateName() const {
        switch (state_.load()) {
            case CircuitState::CLOSED: return "CLOSED";
            case CircuitState::OPEN: return "OPEN";
            case CircuitState::HALF_OPEN: return "HALF_OPEN";
            default: return "UNKNOWN";
        }
    }
    
    // Статистика
    void printStats() const {
        double success_rate = total_requests_.load() > 0 
            ? (100.0 * successful_requests_.load() / total_requests_.load()) 
            : 0.0;
        
        std::cout << "\n=== Circuit Breaker '" << name_ << "' Statistics ===" << std::endl;
        std::cout << "Состояние: " << getStateName() << std::endl;
        std::cout << "Всего запросов: " << total_requests_.load() << std::endl;
        std::cout << "Успешных: " << successful_requests_.load() << std::endl;
        std::cout << "Неудачных: " << failed_requests_.load() << std::endl;
        std::cout << "Отклоненных: " << rejected_requests_.load() << std::endl;
        std::cout << "Success Rate: " << success_rate << "%" << std::endl;
        std::cout << "Текущий счетчик ошибок: " << failure_count_.load() << std::endl;
        std::cout << "Текущий счетчик успехов: " << success_count_.load() << std::endl;
        std::cout << "================================================" << std::endl;
    }
    
    // Ручной сброс
    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        state_.store(CircuitState::CLOSED);
        failure_count_.store(0);
        success_count_.store(0);
        std::cout << "[" << name_ << "] Ручной сброс Circuit Breaker" << std::endl;
    }
    
private:
    void onSuccess() {
        successful_requests_.fetch_add(1);
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        CircuitState current_state = state_.load();
        
        if (current_state == CircuitState::HALF_OPEN) {
            size_t successes = success_count_.fetch_add(1) + 1;
            
            std::cout << "[" << name_ << "] HALF_OPEN успех " << successes 
                      << "/" << config_.success_threshold << std::endl;
            
            if (successes >= config_.success_threshold) {
                std::cout << "[" << name_ << "] Переход HALF_OPEN -> CLOSED" << std::endl;
                state_.store(CircuitState::CLOSED);
                failure_count_.store(0);
                success_count_.store(0);
            }
        } else if (current_state == CircuitState::CLOSED) {
            // В закрытом состоянии сбрасываем счетчик ошибок при успехе
            failure_count_.store(0);
        }
    }
    
    void onFailure() {
        failed_requests_.fetch_add(1);
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        last_failure_time_ = std::chrono::system_clock::now();
        
        CircuitState current_state = state_.load();
        
        if (current_state == CircuitState::HALF_OPEN) {
            std::cout << "[" << name_ << "] HALF_OPEN неудача, переход -> OPEN" << std::endl;
            state_.store(CircuitState::OPEN);
            success_count_.store(0);
        } else if (current_state == CircuitState::CLOSED) {
            size_t failures = failure_count_.fetch_add(1) + 1;
            
            std::cout << "[" << name_ << "] CLOSED неудача " << failures 
                      << "/" << config_.failure_threshold << std::endl;
            
            if (failures >= config_.failure_threshold) {
                std::cout << "[" << name_ << "] Переход CLOSED -> OPEN" << std::endl;
                state_.store(CircuitState::OPEN);
            }
        }
    }
    
    bool shouldAttemptReset() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto now = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - last_failure_time_);
        
        return elapsed >= config_.timeout;
    }
};

// Retry политика
struct RetryPolicy {
    size_t max_attempts;
    std::chrono::milliseconds initial_delay;
    double backoff_multiplier;
    
    RetryPolicy(size_t attempts = 3, 
                std::chrono::milliseconds delay = std::chrono::milliseconds(100),
                double multiplier = 2.0)
        : max_attempts(attempts), 
          initial_delay(delay), 
          backoff_multiplier(multiplier) {}
};

// Утилита для Retry с экспоненциальной задержкой
template<typename F>
auto retryWithBackoff(F&& func, const RetryPolicy& policy) -> decltype(func()) {
    std::chrono::milliseconds delay = policy.initial_delay;
    
    for (size_t attempt = 1; attempt <= policy.max_attempts; ++attempt) {
        try {
            return func();
        } catch (const std::exception& e) {
            if (attempt == policy.max_attempts) {
                std::cerr << "Все попытки retry исчерпаны: " << e.what() << std::endl;
                throw;
            }
            
            std::cout << "Попытка " << attempt << " неудачна, retry через " 
                      << delay.count() << " ms" << std::endl;
            
            std::this_thread::sleep_for(delay);
            delay = std::chrono::milliseconds(
                static_cast<long>(delay.count() * policy.backoff_multiplier));
        }
    }
    
    throw std::runtime_error("Retry logic failed");
}

// Имитация HTTP сервиса
class HTTPService {
private:
    std::string name_;
    std::atomic<bool> healthy_{true};
    double failure_rate_;
    std::atomic<size_t> request_count_{0};
    
public:
    HTTPService(const std::string& name, double failure_rate = 0.0) 
        : name_(name), failure_rate_(failure_rate) {
        std::cout << "HTTP Service '" << name_ << "' создан (failure rate: " 
                  << (failure_rate * 100) << "%)" << std::endl;
    }
    
    std::string request(const std::string& endpoint) {
        request_count_.fetch_add(1);
        
        // Имитация сетевой задержки
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        // Симуляция ошибок
        if (!healthy_.load()) {
            throw std::runtime_error("Service unavailable");
        }
        
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<> dis(0.0, 1.0);
        
        if (dis(gen) < failure_rate_) {
            throw std::runtime_error("Request failed (simulated)");
        }
        
        return "Response from " + name_ + " for " + endpoint;
    }
    
    void setHealthy(bool healthy) {
        healthy_.store(healthy);
        std::cout << "Service '" << name_ << "' установлен " 
                  << (healthy ? "healthy" : "unhealthy") << std::endl;
    }
    
    size_t getRequestCount() const {
        return request_count_.load();
    }
};

// Устойчивый HTTP клиент
class ResilientHTTPClient {
private:
    std::shared_ptr<HTTPService> service_;
    std::shared_ptr<CircuitBreaker> circuit_breaker_;
    RetryPolicy retry_policy_;
    
    // Fallback данные
    std::function<std::string(const std::string&)> fallback_;
    
public:
    ResilientHTTPClient(std::shared_ptr<HTTPService> service,
                       const std::string& cb_name = "HTTPClient",
                       RetryPolicy retry = RetryPolicy())
        : service_(std::move(service)),
          circuit_breaker_(std::make_shared<CircuitBreaker>(cb_name)),
          retry_policy_(retry) {}
    
    // Установка fallback функции
    void setFallback(std::function<std::string(const std::string&)> fallback) {
        fallback_ = std::move(fallback);
    }
    
    // Выполнение запроса с защитой
    std::string request(const std::string& endpoint) {
        try {
            // Пытаемся выполнить через Circuit Breaker
            return circuit_breaker_->execute([&]() {
                // С retry логикой
                return retryWithBackoff([&]() {
                    return service_->request(endpoint);
                }, retry_policy_);
            });
            
        } catch (const std::exception& e) {
            std::cerr << "Запрос не удался: " << e.what() << std::endl;
            
            // Fallback
            if (fallback_) {
                std::cout << "Использование fallback для " << endpoint << std::endl;
                return fallback_(endpoint);
            }
            
            throw;
        }
    }
    
    std::shared_ptr<CircuitBreaker> getCircuitBreaker() {
        return circuit_breaker_;
    }
};

// База данных
class Database {
private:
    std::string name_;
    std::atomic<bool> available_{true};
    std::atomic<size_t> query_count_{0};
    
public:
    explicit Database(const std::string& name) : name_(name) {
        std::cout << "Database '" << name_ << "' создана" << std::endl;
    }
    
    std::string query(const std::string& sql) {
        query_count_.fetch_add(1);
        
        // Имитация задержки БД
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        
        if (!available_.load()) {
            throw std::runtime_error("Database connection lost");
        }
        
        return "Result from " + name_ + " for: " + sql;
    }
    
    void setAvailable(bool available) {
        available_.store(available);
        std::cout << "Database '" << name_ << "' " 
                  << (available ? "доступна" : "недоступна") << std::endl;
    }
    
    size_t getQueryCount() const {
        return query_count_.load();
    }
};

// Устойчивый Database клиент
class ResilientDatabaseClient {
private:
    std::shared_ptr<Database> db_;
    std::shared_ptr<CircuitBreaker> circuit_breaker_;
    
    // Кэш для fallback
    std::unordered_map<std::string, std::string> cache_;
    std::mutex cache_mutex_;
    
public:
    ResilientDatabaseClient(std::shared_ptr<Database> db,
                           const std::string& cb_name = "DBClient")
        : db_(std::move(db)),
          circuit_breaker_(std::make_shared<CircuitBreaker>(cb_name)) {}
    
    std::string query(const std::string& sql) {
        try {
            // Пытаемся выполнить через Circuit Breaker
            std::string result = circuit_breaker_->execute([&]() {
                return db_->query(sql);
            });
            
            // Кэшируем успешный результат
            {
                std::lock_guard<std::mutex> lock(cache_mutex_);
                cache_[sql] = result;
            }
            
            return result;
            
        } catch (const std::exception& e) {
            std::cerr << "Query не удался: " << e.what() << std::endl;
            
            // Пытаемся взять из кэша
            {
                std::lock_guard<std::mutex> lock(cache_mutex_);
                auto it = cache_.find(sql);
                if (it != cache_.end()) {
                    std::cout << "Использование закэшированного результата" << std::endl;
                    return it->second + " (cached)";
                }
            }
            
            throw;
        }
    }
    
    std::shared_ptr<CircuitBreaker> getCircuitBreaker() {
        return circuit_breaker_;
    }
};

// Демонстрация HTTP клиента с Circuit Breaker
void demonstrateHTTPClient() {
    std::cout << "\n=== Демонстрация HTTP клиента с Circuit Breaker ===" << std::endl;
    
    // Создаем сервис с 30% отказами
    auto service = std::make_shared<HTTPService>("API Service", 0.3);
    
    // Создаем устойчивый клиент
    ResilientHTTPClient client(service, "API_CB", RetryPolicy(3));
    
    // Устанавливаем fallback
    client.setFallback([](const std::string& endpoint) {
        return "Fallback response for " + endpoint;
    });
    
    // Делаем много запросов
    std::cout << "\n--- Отправка запросов ---" << std::endl;
    for (int i = 0; i < 20; ++i) {
        try {
            std::string response = client.request("/api/endpoint");
            std::cout << "Запрос " << i << ": SUCCESS" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Запрос " << i << ": FAILED" << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    client.getCircuitBreaker()->printStats();
    std::cout << "Запросов к сервису: " << service->getRequestCount() << std::endl;
}

// Демонстрация Database клиента с Circuit Breaker
void demonstrateDatabaseClient() {
    std::cout << "\n=== Демонстрация Database клиента с Circuit Breaker ===" << std::endl;
    
    auto db = std::make_shared<Database>("MainDB");
    ResilientDatabaseClient client(db, "DB_CB");
    
    // Делаем успешные запросы
    std::cout << "\n--- Успешные запросы ---" << std::endl;
    for (int i = 0; i < 5; ++i) {
        try {
            std::string result = client.query("SELECT * FROM users WHERE id=" + std::to_string(i));
            std::cout << "Query " << i << ": SUCCESS" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Query " << i << ": FAILED" << std::endl;
        }
    }
    
    // Имитируем падение БД
    std::cout << "\n--- БД недоступна ---" << std::endl;
    db->setAvailable(false);
    
    for (int i = 0; i < 10; ++i) {
        try {
            std::string result = client.query("SELECT * FROM users WHERE id=" + std::to_string(i % 5));
            std::cout << "Query " << i << ": SUCCESS" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Query " << i << ": FAILED" << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Восстанавливаем БД
    std::cout << "\n--- БД восстановлена ---" << std::endl;
    db->setAvailable(true);
    
    std::this_thread::sleep_for(std::chrono::seconds(11)); // Ждем таймаут Circuit Breaker
    
    for (int i = 0; i < 5; ++i) {
        try {
            std::string result = client.query("SELECT * FROM users WHERE id=" + std::to_string(i));
            std::cout << "Query " << i << ": SUCCESS" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Query " << i << ": FAILED" << std::endl;
        }
    }
    
    client.getCircuitBreaker()->printStats();
}

int main() {
    std::cout << "=== Resilient Client Pattern ===" << std::endl;
    
    try {
        demonstrateHTTPClient();
        demonstrateDatabaseClient();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Демонстрация завершена ===" << std::endl;
    return 0;
}
