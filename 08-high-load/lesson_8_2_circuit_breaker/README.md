# Урок 8.2: Circuit Breaker Pattern (Автоматический выключатель)

## 🎯 Цель урока
Изучить паттерн Circuit Breaker - один из самых важных паттернов для создания устойчивых высоконагруженных систем. Понять, как защитить приложение от каскадных отказов и обеспечить graceful degradation.

## 📚 Что изучаем

### 1. Паттерн Circuit Breaker
- **Определение**: Автоматический выключатель, который предотвращает вызовы неработающих сервисов
- **Назначение**: Защита от каскадных отказов, быстрое обнаружение проблем, автоматическое восстановление
- **Применение**: Микросервисы, внешние API, сетевые вызовы, базы данных

### 2. Состояния Circuit Breaker
- **CLOSED**: Нормальная работа, вызовы проходят через
- **OPEN**: Выключатель открыт, вызовы блокируются
- **HALF_OPEN**: Тестовый режим, ограниченные вызовы для проверки восстановления

### 3. Ключевые параметры
- **Failure Threshold**: Количество ошибок для перехода в OPEN
- **Timeout**: Время ожидания перед переходом в HALF_OPEN
- **Success Threshold**: Количество успешных вызовов для перехода в CLOSED

## 🔍 Ключевые концепции

### Базовая реализация Circuit Breaker
```cpp
#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <optional>

enum class CircuitState {
    CLOSED,    // Нормальная работа
    OPEN,      // Блокировка вызовов
    HALF_OPEN  // Тестовый режим
};

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
          successThreshold_(successThreshold), timeout_(timeout) {}
    
    // Выполнение операции через Circuit Breaker
    std::optional<T> execute(std::function<T()> operation) {
        totalRequests_.fetch_add(1);
        
        if (!canExecute()) {
            rejectedRequests_.fetch_add(1);
            return std::nullopt;
        }
        
        try {
            T result = operation();
            onSuccess();
            return result;
        } catch (const std::exception& e) {
            onFailure();
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
    
    // Статистика
    struct Statistics {
        CircuitState state;
        size_t totalRequests;
        size_t totalFailures;
        size_t totalSuccesses;
        size_t rejectedRequests;
        double failureRate;
        double successRate;
    };
    
    Statistics getStatistics() const {
        Statistics stats;
        stats.state = state_.load();
        stats.totalRequests = totalRequests_.load();
        stats.totalFailures = totalFailures_.load();
        stats.totalSuccesses = totalSuccesses_.load();
        stats.rejectedRequests = rejectedRequests_.load();
        
        if (stats.totalRequests > 0) {
            stats.failureRate = static_cast<double>(stats.totalFailures) / stats.totalRequests;
            stats.successRate = static_cast<double>(stats.totalSuccesses) / stats.totalRequests;
        } else {
            stats.failureRate = 0.0;
            stats.successRate = 0.0;
        }
        
        return stats;
    }
    
private:
    bool canExecute() {
        CircuitState currentState = state_.load();
        
        switch (currentState) {
            case CircuitState::CLOSED:
                return true;
                
            case CircuitState::OPEN:
                // Проверяем, не истек ли timeout
                if (std::chrono::system_clock::now() - lastFailureTime_ >= timeout_) {
                    state_.store(CircuitState::HALF_OPEN);
                    successCount_.store(0);
                    return true;
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
            
            if (currentSuccessCount >= successThreshold_) {
                // Переходим в CLOSED состояние
                state_.store(CircuitState::CLOSED);
                failureCount_.store(0);
                successCount_.store(0);
                
                std::cout << "[" << name_ << "] ✅ Circuit Breaker перешел в состояние CLOSED" << std::endl;
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
            
            if (currentFailureCount >= failureThreshold_) {
                // Переходим в OPEN состояние
                state_.store(CircuitState::OPEN);
                std::cout << "[" << name_ << "] ❌ Circuit Breaker перешел в состояние OPEN" << std::endl;
            }
        }
    }
};
```

## 🛠️ Практические примеры

### HTTP клиент с Circuit Breaker
```cpp
#include <curl/curl.h>
#include <string>
#include <memory>

class HttpClient {
private:
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl_;
    
public:
    HttpClient() : curl_(curl_easy_init(), &curl_easy_cleanup) {
        if (!curl_) {
            throw std::runtime_error("Failed to initialize CURL");
        }
    }
    
    std::string get(const std::string& url) {
        curl_easy_setopt(curl_.get(), CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_.get(), CURLOPT_TIMEOUT, 5L); // 5 секунд timeout
        
        std::string response;
        curl_easy_setopt(curl_.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl_.get(), CURLOPT_WRITEDATA, &response);
        
        CURLcode res = curl_easy_perform(curl_.get());
        
        if (res != CURLE_OK) {
            throw std::runtime_error("HTTP request failed: " + std::string(curl_easy_strerror(res)));
        }
        
        long httpCode = 0;
        curl_easy_getinfo(curl_.get(), CURLINFO_RESPONSE_CODE, &httpCode);
        
        if (httpCode >= 400) {
            throw std::runtime_error("HTTP error: " + std::to_string(httpCode));
        }
        
        return response;
    }
    
private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
        size_t totalSize = size * nmemb;
        data->append(static_cast<char*>(contents), totalSize);
        return totalSize;
    }
};

class ResilientHttpClient {
private:
    CircuitBreaker<std::string> circuitBreaker_;
    HttpClient httpClient_;
    
public:
    ResilientHttpClient(const std::string& serviceName) 
        : circuitBreaker_(serviceName, 5, 3, std::chrono::seconds(30)) {}
    
    std::optional<std::string> get(const std::string& url) {
        return circuitBreaker_.execute([this, &url]() {
            return httpClient_.get(url);
        });
    }
    
    void getAsync(const std::string& url, std::function<void(std::optional<std::string>)> callback) {
        circuitBreaker_.executeAsync([this, &url]() {
            return httpClient_.get(url);
        }, callback);
    }
    
    auto getStatistics() const {
        return circuitBreaker_.getStatistics();
    }
};
```

### База данных с Circuit Breaker
```cpp
#include <sqlite3.h>
#include <memory>

class DatabaseClient {
private:
    std::unique_ptr<sqlite3, decltype(&sqlite3_close)> db_;
    
public:
    DatabaseClient(const std::string& dbPath) : db_(nullptr, &sqlite3_close) {
        int rc = sqlite3_open(dbPath.c_str(), &db_);
        if (rc != SQLITE_OK) {
            throw std::runtime_error("Failed to open database: " + std::string(sqlite3_errmsg(db_)));
        }
    }
    
    std::vector<std::string> executeQuery(const std::string& query) {
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db_.get(), query.c_str(), -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_.get())));
        }
        
        std::vector<std::string> results;
        
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            if (text) {
                results.push_back(text);
            }
        }
        
        sqlite3_finalize(stmt);
        
        if (rc != SQLITE_DONE) {
            throw std::runtime_error("Query execution failed: " + std::string(sqlite3_errmsg(db_.get())));
        }
        
        return results;
    }
};

class ResilientDatabaseClient {
private:
    CircuitBreaker<std::vector<std::string>> circuitBreaker_;
    DatabaseClient dbClient_;
    
public:
    ResilientDatabaseClient(const std::string& serviceName, const std::string& dbPath)
        : circuitBreaker_(serviceName, 3, 2, std::chrono::seconds(60)),
          dbClient_(dbPath) {}
    
    std::optional<std::vector<std::string>> executeQuery(const std::string& query) {
        return circuitBreaker_.execute([this, &query]() {
            return dbClient_.executeQuery(query);
        });
    }
    
    auto getStatistics() const {
        return circuitBreaker_.getStatistics();
    }
};
```

### Система с множественными Circuit Breakers
```cpp
class ServiceManager {
private:
    std::unordered_map<std::string, std::unique_ptr<ResilientHttpClient>> httpClients_;
    std::unordered_map<std::string, std::unique_ptr<ResilientDatabaseClient>> dbClients_;
    
public:
    void addHttpService(const std::string& name) {
        httpClients_[name] = std::make_unique<ResilientHttpClient>(name);
    }
    
    void addDatabaseService(const std::string& name, const std::string& dbPath) {
        dbClients_[name] = std::make_unique<ResilientDatabaseClient>(name, dbPath);
    }
    
    std::optional<std::string> callHttpService(const std::string& serviceName, const std::string& url) {
        auto it = httpClients_.find(serviceName);
        if (it != httpClients_.end()) {
            return it->second->get(url);
        }
        return std::nullopt;
    }
    
    std::optional<std::vector<std::string>> callDatabaseService(const std::string& serviceName, const std::string& query) {
        auto it = dbClients_.find(serviceName);
        if (it != dbClients_.end()) {
            return it->second->executeQuery(query);
        }
        return std::nullopt;
    }
    
    void printAllStatistics() {
        std::cout << "\n=== СТАТИСТИКА CIRCUIT BREAKERS ===" << std::endl;
        
        for (const auto& pair : httpClients_) {
            auto stats = pair.second->getStatistics();
            std::cout << "HTTP Service [" << pair.first << "]: "
                      << "State=" << static_cast<int>(stats.state)
                      << ", Requests=" << stats.totalRequests
                      << ", Failures=" << stats.totalFailures
                      << ", Success Rate=" << (stats.successRate * 100) << "%" << std::endl;
        }
        
        for (const auto& pair : dbClients_) {
            auto stats = pair.second->getStatistics();
            std::cout << "DB Service [" << pair.first << "]: "
                      << "State=" << static_cast<int>(stats.state)
                      << ", Requests=" << stats.totalRequests
                      << ", Failures=" << stats.totalFailures
                      << ", Success Rate=" << (stats.successRate * 100) << "%" << std::endl;
        }
        
        std::cout << "===================================" << std::endl;
    }
};
```

## 🎯 Практические упражнения

### Упражнение 1: Микросервис с Circuit Breaker
Создайте систему микросервисов с Circuit Breaker для каждого сервиса.

### Упражнение 2: Graceful Degradation
Реализуйте систему с fallback механизмами при открытом Circuit Breaker.

### Упражнение 3: Мониторинг Circuit Breakers
Создайте систему мониторинга состояния всех Circuit Breakers.

### Упражнение 4: Адаптивные пороги
Реализуйте Circuit Breaker с адаптивными порогами в зависимости от нагрузки.

## 💡 Важные принципы

1. **Быстрое обнаружение проблем**: Низкие пороги для перехода в OPEN состояние
2. **Автоматическое восстановление**: Переход в HALF_OPEN для тестирования
3. **Graceful degradation**: Fallback механизмы при недоступности сервисов
4. **Мониторинг**: Отслеживание состояния и статистики
5. **Настройка параметров**: Адаптация под конкретные сервисы и требования
