/**
 * @file dip_example.cpp
 * @brief Детальная демонстрация Dependency Inversion Principle (DIP)
 * @author Senior C++ Developer
 * @date 2024
 * 
 * Этот файл содержит углубленный анализ DIP с математическим обоснованием
 * и практическими примерами из реальной разработки.
 */

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

/**
 * @brief Математическая формализация DIP
 * 
 * DIP можно формализовать следующим образом:
 * ∀M₁ ∈ HighLevel, ∀M₂ ∈ LowLevel:
 * Dependency(M₁, M₂) ⟹ ∃A ∈ Abstractions: 
 * Dependency(M₁, A) ∧ Dependency(M₂, A)
 * 
 * Другими словами: модули высокого уровня не должны зависеть от модулей
 * низкого уровня. Оба должны зависеть от абстракций.
 */

// ============================================================================
// ПРИМЕР 1: СИСТЕМА ЛОГИРОВАНИЯ
// ============================================================================

/**
 * @brief Нарушение DIP: Зависимость от конкретных реализаций
 * 
 * Этот код нарушает DIP, так как BusinessLogic напрямую зависит от
 * конкретных классов FileLogger и EmailNotification.
 */
class BadFileLogger {
public:
    void log(const std::string& message) {
        std::ofstream logFile("application.log", std::ios::app);
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        logFile << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") 
                << "] [FILE] " << message << "\n";
        logFile.close();
        std::cout << "📝 [FILE] " << message << "\n";
    }
};

class BadConsoleLogger {
public:
    void log(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::cout << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") 
                  << "] [CONSOLE] " << message << "\n";
    }
};

class BadEmailNotification {
public:
    void send(const std::string& message) {
        std::cout << "📧 [EMAIL] Отправка уведомления: " << message << "\n";
        // Здесь была бы реальная логика отправки email
    }
};

class BadSMSNotification {
public:
    void send(const std::string& message) {
        std::cout << "📱 [SMS] Отправка SMS: " << message << "\n";
        // Здесь была бы реальная логика отправки SMS
    }
};

class BadBusinessLogic {
private:
    BadFileLogger fileLogger;
    BadConsoleLogger consoleLogger;
    BadEmailNotification emailNotification;
    BadSMSNotification smsNotification;
    
public:
    void processOrder(const std::string& orderId) {
        // Нарушение DIP: прямая зависимость от конкретных классов
        fileLogger.log("Начало обработки заказа: " + orderId);
        consoleLogger.log("Обработка заказа: " + orderId);
        
        // Бизнес-логика
        std::cout << "🛒 Обработка заказа " << orderId << "\n";
        
        // Нарушение DIP: прямая зависимость от конкретных классов
        emailNotification.send("Заказ " + orderId + " обработан");
        smsNotification.send("Заказ " + orderId + " готов к отправке");
        
        fileLogger.log("Заказ обработан: " + orderId);
    }
    
    void processPayment(const std::string& paymentId) {
        fileLogger.log("Начало обработки платежа: " + paymentId);
        consoleLogger.log("Обработка платежа: " + paymentId);
        
        // Бизнес-логика
        std::cout << "💳 Обработка платежа " << paymentId << "\n";
        
        emailNotification.send("Платеж " + paymentId + " успешно обработан");
        smsNotification.send("Платеж " + paymentId + " завершен");
        
        fileLogger.log("Платеж обработан: " + paymentId);
    }
};

/**
 * @brief Соблюдение DIP: Зависимость от абстракций
 */

// Абстракции (интерфейсы)
class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(const std::string& message) = 0;
    virtual void logError(const std::string& error) = 0;
    virtual void logWarning(const std::string& warning) = 0;
    virtual void logInfo(const std::string& info) = 0;
    virtual std::string getLoggerType() const = 0;
};

class INotificationService {
public:
    virtual ~INotificationService() = default;
    virtual bool send(const std::string& message) = 0;
    virtual bool sendUrgent(const std::string& message) = 0;
    virtual std::string getServiceType() const = 0;
    virtual bool isAvailable() const = 0;
};

// Конкретные реализации
class FileLogger : public ILogger {
private:
    std::string logFilePath;
    
public:
    FileLogger(const std::string& path = "application.log") : logFilePath(path) {}
    
    void log(const std::string& message) override {
        writeToFile("INFO", message);
        std::cout << "📝 [FILE] " << message << "\n";
    }
    
    void logError(const std::string& error) override {
        writeToFile("ERROR", error);
        std::cout << "❌ [FILE ERROR] " << error << "\n";
    }
    
    void logWarning(const std::string& warning) override {
        writeToFile("WARNING", warning);
        std::cout << "⚠️ [FILE WARNING] " << warning << "\n";
    }
    
    void logInfo(const std::string& info) override {
        writeToFile("INFO", info);
        std::cout << "ℹ️ [FILE INFO] " << info << "\n";
    }
    
    std::string getLoggerType() const override {
        return "File Logger";
    }
    
private:
    void writeToFile(const std::string& level, const std::string& message) {
        std::ofstream logFile(logFilePath, std::ios::app);
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        logFile << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") 
                << "] [" << level << "] " << message << "\n";
        logFile.close();
    }
};

class ConsoleLogger : public ILogger {
public:
    void log(const std::string& message) override {
        logWithTimestamp("INFO", message);
    }
    
    void logError(const std::string& error) override {
        logWithTimestamp("ERROR", error);
        std::cout << "❌ [CONSOLE ERROR] " << error << "\n";
    }
    
    void logWarning(const std::string& warning) override {
        logWithTimestamp("WARNING", warning);
        std::cout << "⚠️ [CONSOLE WARNING] " << warning << "\n";
    }
    
    void logInfo(const std::string& info) override {
        logWithTimestamp("INFO", info);
        std::cout << "ℹ️ [CONSOLE INFO] " << info << "\n";
    }
    
    std::string getLoggerType() const override {
        return "Console Logger";
    }
    
private:
    void logWithTimestamp(const std::string& level, const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::cout << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") 
                  << "] [" << level << "] " << message << "\n";
    }
};

class DatabaseLogger : public ILogger {
private:
    std::string connectionString;
    bool connected = false;
    
public:
    DatabaseLogger(const std::string& connStr = "database://localhost:5432/logs") 
        : connectionString(connStr) {
        // Упрощенная логика подключения
        connected = true;
    }
    
    void log(const std::string& message) override {
        if (connected) {
            writeToDatabase("INFO", message);
            std::cout << "🗄️ [DATABASE] " << message << "\n";
        }
    }
    
    void logError(const std::string& error) override {
        if (connected) {
            writeToDatabase("ERROR", error);
            std::cout << "❌ [DATABASE ERROR] " << error << "\n";
        }
    }
    
    void logWarning(const std::string& warning) override {
        if (connected) {
            writeToDatabase("WARNING", warning);
            std::cout << "⚠️ [DATABASE WARNING] " << warning << "\n";
        }
    }
    
    void logInfo(const std::string& info) override {
        if (connected) {
            writeToDatabase("INFO", info);
            std::cout << "ℹ️ [DATABASE INFO] " << info << "\n";
        }
    }
    
    std::string getLoggerType() const override {
        return "Database Logger";
    }
    
private:
    void writeToDatabase(const std::string& level, const std::string& message) {
        // Упрощенная логика записи в БД
        // INSERT INTO logs (timestamp, level, message) VALUES (?, ?, ?)
    }
};

class EmailNotificationService : public INotificationService {
private:
    std::string smtpServer;
    std::string fromEmail;
    bool available = true;
    
public:
    EmailNotificationService(const std::string& server = "smtp.gmail.com", 
                           const std::string& from = "noreply@company.com")
        : smtpServer(server), fromEmail(from) {}
    
    bool send(const std::string& message) override {
        if (!isAvailable()) {
            return false;
        }
        
        std::cout << "📧 [EMAIL] Отправка уведомления через " << smtpServer << ": " << message << "\n";
        // Здесь была бы реальная логика отправки email
        return true;
    }
    
    bool sendUrgent(const std::string& message) override {
        if (!isAvailable()) {
            return false;
        }
        
        std::cout << "🚨 [URGENT EMAIL] Срочное уведомление: " << message << "\n";
        // Здесь была бы реальная логика отправки срочного email
        return true;
    }
    
    std::string getServiceType() const override {
        return "Email Notification Service";
    }
    
    bool isAvailable() const override {
        return available;
    }
    
    void setAvailable(bool avail) {
        available = avail;
    }
};

class SMSNotificationService : public INotificationService {
private:
    std::string provider;
    std::string apiKey;
    bool available = true;
    
public:
    SMSNotificationService(const std::string& smsProvider = "Twilio", 
                          const std::string& key = "api_key_123")
        : provider(smsProvider), apiKey(key) {}
    
    bool send(const std::string& message) override {
        if (!isAvailable()) {
            return false;
        }
        
        std::cout << "📱 [SMS] Отправка SMS через " << provider << ": " << message << "\n";
        // Здесь была бы реальная логика отправки SMS
        return true;
    }
    
    bool sendUrgent(const std::string& message) override {
        if (!isAvailable()) {
            return false;
        }
        
        std::cout << "🚨 [URGENT SMS] Срочное SMS: " << message << "\n";
        // Здесь была бы реальная логика отправки срочного SMS
        return true;
    }
    
    std::string getServiceType() const override {
        return "SMS Notification Service";
    }
    
    bool isAvailable() const override {
        return available;
    }
    
    void setAvailable(bool avail) {
        available = avail;
    }
};

class PushNotificationService : public INotificationService {
private:
    std::string platform;
    std::string apiEndpoint;
    bool available = true;
    
public:
    PushNotificationService(const std::string& pushPlatform = "Firebase", 
                           const std::string& endpoint = "https://fcm.googleapis.com/fcm/send")
        : platform(pushPlatform), apiEndpoint(endpoint) {}
    
    bool send(const std::string& message) override {
        if (!isAvailable()) {
            return false;
        }
        
        std::cout << "🔔 [PUSH] Отправка push-уведомления через " << platform << ": " << message << "\n";
        // Здесь была бы реальная логика отправки push-уведомления
        return true;
    }
    
    bool sendUrgent(const std::string& message) override {
        if (!isAvailable()) {
            return false;
        }
        
        std::cout << "🚨 [URGENT PUSH] Срочное push-уведомление: " << message << "\n";
        // Здесь была бы реальная логика отправки срочного push-уведомления
        return true;
    }
    
    std::string getServiceType() const override {
        return "Push Notification Service";
    }
    
    bool isAvailable() const override {
        return available;
    }
    
    void setAvailable(bool avail) {
        available = avail;
    }
};

// Бизнес-логика зависит только от абстракций
class BusinessLogic {
private:
    std::unique_ptr<ILogger> logger;
    std::vector<std::unique_ptr<INotificationService>> notificationServices;
    
public:
    // Dependency Injection через конструктор
    BusinessLogic(std::unique_ptr<ILogger> loggerPtr, 
                  std::vector<std::unique_ptr<INotificationService>> notificationPtrs)
        : logger(std::move(loggerPtr)), notificationServices(std::move(notificationPtrs)) {}
    
    void processOrder(const std::string& orderId) {
        logger->logInfo("Начало обработки заказа: " + orderId);
        
        try {
            // Бизнес-логика
            std::cout << "🛒 Обработка заказа " << orderId << "\n";
            
            // Симуляция обработки
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            logger->logInfo("Заказ успешно обработан: " + orderId);
            
            // Отправка уведомлений через все доступные сервисы
            sendNotifications("Заказ " + orderId + " обработан и готов к отправке");
            
        } catch (const std::exception& e) {
            logger->logError("Ошибка при обработке заказа " + orderId + ": " + e.what());
            sendUrgentNotifications("КРИТИЧЕСКАЯ ОШИБКА: Заказ " + orderId + " не может быть обработан");
        }
    }
    
    void processPayment(const std::string& paymentId) {
        logger->logInfo("Начало обработки платежа: " + paymentId);
        
        try {
            // Бизнес-логика
            std::cout << "💳 Обработка платежа " << paymentId << "\n";
            
            // Симуляция обработки
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            
            logger->logInfo("Платеж успешно обработан: " + paymentId);
            
            // Отправка уведомлений
            sendNotifications("Платеж " + paymentId + " успешно обработан");
            
        } catch (const std::exception& e) {
            logger->logError("Ошибка при обработке платежа " + paymentId + ": " + e.what());
            sendUrgentNotifications("ОШИБКА ПЛАТЕЖА: " + paymentId + " - требуется вмешательство");
        }
    }
    
    void processRefund(const std::string& refundId) {
        logger->logWarning("Начало обработки возврата: " + refundId);
        
        try {
            // Бизнес-логика
            std::cout << "💰 Обработка возврата " << refundId << "\n";
            
            // Симуляция обработки
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            
            logger->logInfo("Возврат успешно обработан: " + refundId);
            
            // Отправка уведомлений
            sendNotifications("Возврат " + refundId + " обработан, средства возвращены");
            
        } catch (const std::exception& e) {
            logger->logError("Ошибка при обработке возврата " + refundId + ": " + e.what());
            sendUrgentNotifications("ОШИБКА ВОЗВРАТА: " + refundId + " - требуется проверка");
        }
    }
    
private:
    void sendNotifications(const std::string& message) {
        for (const auto& service : notificationServices) {
            if (service->isAvailable()) {
                bool success = service->send(message);
                if (success) {
                    logger->logInfo("Уведомление отправлено через " + service->getServiceType());
                } else {
                    logger->logWarning("Не удалось отправить уведомление через " + service->getServiceType());
                }
            }
        }
    }
    
    void sendUrgentNotifications(const std::string& message) {
        for (const auto& service : notificationServices) {
            if (service->isAvailable()) {
                bool success = service->sendUrgent(message);
                if (success) {
                    logger->logInfo("Срочное уведомление отправлено через " + service->getServiceType());
                } else {
                    logger->logError("Не удалось отправить срочное уведомление через " + service->getServiceType());
                }
            }
        }
    }
};

// ============================================================================
// ПРИМЕР 2: СИСТЕМА КЭШИРОВАНИЯ
// ============================================================================

/**
 * @brief Правильная реализация DIP для системы кэширования
 */
class ICache {
public:
    virtual ~ICache() = default;
    virtual bool set(const std::string& key, const std::string& value, int ttl = 3600) = 0;
    virtual std::string get(const std::string& key) = 0;
    virtual bool deleteKey(const std::string& key) = 0;
    virtual bool exists(const std::string& key) = 0;
    virtual void clear() = 0;
    virtual std::string getCacheType() const = 0;
};

class MemoryCache : public ICache {
private:
    std::map<std::string, std::string> cache;
    std::map<std::string, std::chrono::system_clock::time_point> expiration;
    
public:
    bool set(const std::string& key, const std::string& value, int ttl = 3600) override {
        cache[key] = value;
        expiration[key] = std::chrono::system_clock::now() + std::chrono::seconds(ttl);
        std::cout << "🧠 [MEMORY CACHE] Установлен ключ: " << key << "\n";
        return true;
    }
    
    std::string get(const std::string& key) override {
        if (exists(key)) {
            std::cout << "🧠 [MEMORY CACHE] Получен ключ: " << key << "\n";
            return cache[key];
        }
        return "";
    }
    
    bool deleteKey(const std::string& key) override {
        if (cache.find(key) != cache.end()) {
            cache.erase(key);
            expiration.erase(key);
            std::cout << "🧠 [MEMORY CACHE] Удален ключ: " << key << "\n";
            return true;
        }
        return false;
    }
    
    bool exists(const std::string& key) override {
        auto it = cache.find(key);
        if (it == cache.end()) {
            return false;
        }
        
        // Проверка истечения срока действия
        if (std::chrono::system_clock::now() > expiration[key]) {
            cache.erase(it);
            expiration.erase(key);
            return false;
        }
        
        return true;
    }
    
    void clear() override {
        cache.clear();
        expiration.clear();
        std::cout << "🧠 [MEMORY CACHE] Кэш очищен\n";
    }
    
    std::string getCacheType() const override {
        return "Memory Cache";
    }
};

class RedisCache : public ICache {
private:
    std::string host;
    int port;
    bool connected = true;
    
public:
    RedisCache(const std::string& redisHost = "localhost", int redisPort = 6379)
        : host(redisHost), port(redisPort) {}
    
    bool set(const std::string& key, const std::string& value, int ttl = 3600) override {
        if (!connected) return false;
        
        std::cout << "🔴 [REDIS CACHE] Установлен ключ: " << key << " (TTL: " << ttl << "s)\n";
        // Здесь была бы реальная логика работы с Redis
        return true;
    }
    
    std::string get(const std::string& key) override {
        if (!connected) return "";
        
        std::cout << "🔴 [REDIS CACHE] Получен ключ: " << key << "\n";
        // Здесь была бы реальная логика работы с Redis
        return "cached_value_from_redis";
    }
    
    bool deleteKey(const std::string& key) override {
        if (!connected) return false;
        
        std::cout << "🔴 [REDIS CACHE] Удален ключ: " << key << "\n";
        // Здесь была бы реальная логика работы с Redis
        return true;
    }
    
    bool exists(const std::string& key) override {
        if (!connected) return false;
        
        std::cout << "🔴 [REDIS CACHE] Проверка существования ключа: " << key << "\n";
        // Здесь была бы реальная логика работы с Redis
        return true;
    }
    
    void clear() override {
        if (connected) {
            std::cout << "🔴 [REDIS CACHE] Кэш очищен\n";
            // Здесь была бы реальная логика работы с Redis
        }
    }
    
    std::string getCacheType() const override {
        return "Redis Cache";
    }
    
    void setConnected(bool conn) {
        connected = conn;
    }
};

class DataService {
private:
    std::unique_ptr<ICache> cache;
    
public:
    DataService(std::unique_ptr<ICache> cachePtr) : cache(std::move(cachePtr)) {}
    
    std::string getUserData(const std::string& userId) {
        std::string cacheKey = "user_" + userId;
        
        // Попытка получить данные из кэша
        if (cache->exists(cacheKey)) {
            std::cout << "📊 [DATA SERVICE] Данные пользователя получены из кэша\n";
            return cache->get(cacheKey);
        }
        
        // Загрузка данных из базы данных (симуляция)
        std::cout << "📊 [DATA SERVICE] Загрузка данных пользователя из БД\n";
        std::string userData = "User data for " + userId;
        
        // Сохранение в кэш
        cache->set(cacheKey, userData, 1800); // TTL 30 минут
        
        return userData;
    }
    
    void updateUserData(const std::string& userId, const std::string& newData) {
        std::string cacheKey = "user_" + userId;
        
        // Обновление в базе данных (симуляция)
        std::cout << "📊 [DATA SERVICE] Обновление данных пользователя в БД\n";
        
        // Обновление кэша
        cache->set(cacheKey, newData, 1800);
        
        std::cout << "📊 [DATA SERVICE] Кэш обновлен для пользователя " << userId << "\n";
    }
    
    void deleteUser(const std::string& userId) {
        std::string cacheKey = "user_" + userId;
        
        // Удаление из базы данных (симуляция)
        std::cout << "📊 [DATA SERVICE] Удаление пользователя из БД\n";
        
        // Удаление из кэша
        cache->deleteKey(cacheKey);
        
        std::cout << "📊 [DATA SERVICE] Пользователь удален из кэша\n";
    }
};

// ============================================================================
// ДЕМОНСТРАЦИЯ ПРИНЦИПА
// ============================================================================

void demonstrateBadDIP() {
    std::cout << "❌ НАРУШЕНИЕ DIP - Зависимость от конкретных реализаций:\n";
    std::cout << std::string(60, '-') << "\n";
    
    BadBusinessLogic badLogic;
    badLogic.processOrder("ORDER-001");
    badLogic.processPayment("PAYMENT-001");
}

void demonstrateGoodDIP() {
    std::cout << "\n✅ СОБЛЮДЕНИЕ DIP - Зависимость от абстракций:\n";
    std::cout << std::string(60, '-') << "\n";
    
    // Создание зависимостей
    auto fileLogger = std::make_unique<FileLogger>("business.log");
    auto consoleLogger = std::make_unique<ConsoleLogger>();
    auto databaseLogger = std::make_unique<DatabaseLogger>();
    
    auto emailService = std::make_unique<EmailNotificationService>();
    auto smsService = std::make_unique<SMSNotificationService>();
    auto pushService = std::make_unique<PushNotificationService>();
    
    // Можно легко менять реализации без изменения бизнес-логики
    std::vector<std::unique_ptr<INotificationService>> notificationServices;
    notificationServices.push_back(std::move(emailService));
    notificationServices.push_back(std::move(smsService));
    notificationServices.push_back(std::move(pushService));
    
    // Бизнес-логика зависит только от абстракций
    BusinessLogic businessLogic(std::move(fileLogger), std::move(notificationServices));
    
    // Тестирование
    businessLogic.processOrder("ORDER-002");
    businessLogic.processPayment("PAYMENT-002");
    businessLogic.processRefund("REFUND-001");
    
    std::cout << "\n🔄 Тестирование с другой реализацией логгера:\n";
    std::vector<std::unique_ptr<INotificationService>> notificationServices2;
    notificationServices2.push_back(std::make_unique<EmailNotificationService>());
    
    BusinessLogic businessLogic2(std::make_unique<ConsoleLogger>(), std::move(notificationServices2));
    businessLogic2.processOrder("ORDER-003");
}

void demonstrateCacheDIP() {
    std::cout << "\n✅ СОБЛЮДЕНИЕ DIP - Система кэширования:\n";
    std::cout << std::string(50, '-') << "\n";
    
    // Можно легко менять реализацию кэша
    auto memoryCache = std::make_unique<MemoryCache>();
    DataService dataService1(std::move(memoryCache));
    
    std::cout << "Тестирование с Memory Cache:\n";
    dataService1.getUserData("user123");
    dataService1.getUserData("user123"); // Должно быть из кэша
    dataService1.updateUserData("user123", "Updated user data");
    dataService1.deleteUser("user123");
    
    std::cout << "\nТестирование с Redis Cache:\n";
    auto redisCache = std::make_unique<RedisCache>("redis.example.com", 6379);
    DataService dataService2(std::move(redisCache));
    
    dataService2.getUserData("user456");
    dataService2.getUserData("user456"); // Должно быть из кэша
    dataService2.updateUserData("user456", "Updated user data");
    
    std::cout << "\nТестирование с недоступным Redis:\n";
    auto unavailableRedis = std::make_unique<RedisCache>("unavailable.redis.com", 6379);
    unavailableRedis->setConnected(false);
    DataService dataService3(std::move(unavailableRedis));
    
    dataService3.getUserData("user789"); // Должно работать без кэша
}

void analyzeTradeOffs() {
    std::cout << "\n🔬 АНАЛИЗ КОМПРОМИССОВ DIP:\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::cout << "📊 ПРЕИМУЩЕСТВА:\n";
    std::cout << "• Модули высокого уровня не зависят от модулей низкого уровня\n";
    std::cout << "• Оба типа модулей зависят от абстракций\n";
    std::cout << "• Улучшенная тестируемость (легко создавать моки)\n";
    std::cout << "• Возможность легкой замены реализаций\n";
    std::cout << "• Снижение coupling между модулями\n";
    std::cout << "• Поддержка принципа инверсии управления (IoC)\n\n";
    
    std::cout << "⚠️ НЕДОСТАТКИ:\n";
    std::cout << "• Усложнение архитектуры (больше абстракций)\n";
    std::cout << "• Необходимость настройки dependency injection\n";
    std::cout << "• Потенциальный overhead от виртуальных вызовов\n";
    std::cout << "• Возможное over-engineering для простых случаев\n";
    std::cout << "• Необходимость понимания IoC контейнеров\n\n";
    
    std::cout << "🎯 РЕКОМЕНДАЦИИ:\n";
    std::cout << "• Используйте DIP для часто изменяющихся зависимостей\n";
    std::cout << "• Применяйте dependency injection через конструктор\n";
    std::cout << "• Рассмотрите использование IoC контейнеров\n";
    std::cout << "• Избегайте преждевременной абстракции\n";
    std::cout << "• Используйте factory pattern для создания сложных объектов\n";
}

int main() {
    std::cout << "🎯 ДЕТАЛЬНАЯ ДЕМОНСТРАЦИЯ DEPENDENCY INVERSION PRINCIPLE (DIP)\n";
    std::cout << "Автор: Senior C++ Developer\n";
    std::cout << "Цель: Углубленное изучение принципа инверсии зависимостей\n\n";
    
    demonstrateBadDIP();
    demonstrateGoodDIP();
    demonstrateCacheDIP();
    analyzeTradeOffs();
    
    std::cout << "\n📚 МАТЕМАТИЧЕСКОЕ ОБОСНОВАНИЕ:\n";
    std::cout << "DIP: ∀M₁ ∈ HighLevel, ∀M₂ ∈ LowLevel:\n";
    std::cout << "Dependency(M₁, M₂) ⟹ ∃A ∈ Abstractions: \n";
    std::cout << "Dependency(M₁, A) ∧ Dependency(M₂, A)\n\n";
    
    std::cout << "💡 КЛЮЧЕВЫЕ ВЫВОДЫ:\n";
    std::cout << "1. Модули высокого уровня не должны зависеть от модулей низкого уровня\n";
    std::cout << "2. Оба типа модулей должны зависеть от абстракций\n";
    std::cout << "3. DIP обеспечивает гибкость и тестируемость\n";
    std::cout << "4. Применение требует баланса между простотой и гибкостью\n\n";
    
    std::cout << "🔬 Принципы - это инструменты для мышления о проблемах!\n";
    
    return 0;
}
