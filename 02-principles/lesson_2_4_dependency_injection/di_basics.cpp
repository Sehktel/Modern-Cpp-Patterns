/**
 * @file di_basics.cpp
 * @brief Основы Dependency Injection
 * @author Senior C++ Developer
 * @date 2024
 * 
 * Этот файл демонстрирует основы dependency injection:
 * - Проблемы без DI
 * - Constructor Injection
 * - Setter Injection
 * - Interface Injection
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <iomanip>

/**
 * @brief Математическая формализация DI
 * 
 * DI можно формализовать следующим образом:
 * ∀O ∈ Objects, ∀D ∈ Dependencies: 
 * DI(O, D) ⟹ D ∈ injected(O) ∧ D ∉ created(O)
 * где injected(O) = {d | d внедрена в объект O}
 *     created(O) = {d | d создана внутри объекта O}
 */

// ============================================================================
// ПРИМЕР 1: ПРОБЛЕМЫ БЕЗ DEPENDENCY INJECTION
// ============================================================================

/**
 * @brief Проблема: Жестко связанные зависимости
 * 
 * Этот код демонстрирует проблемы без использования DI:
 * - Tight coupling
 * - Сложность тестирования
 * - Нарушение принципа единственной ответственности
 */
class BadEmailService {
public:
    void sendEmail(const std::string& to, const std::string& subject, const std::string& body) {
        std::cout << "📧 Отправка email на " << to << ": " << subject << "\n";
        std::cout << "Содержание: " << body << "\n";
        // Жестко связанная логика отправки email
    }
};

class BadSMSService {
public:
    void sendSMS(const std::string& phone, const std::string& message) {
        std::cout << "📱 Отправка SMS на " << phone << ": " << message << "\n";
        // Жестко связанная логика отправки SMS
    }
};

class BadDatabaseService {
public:
    void saveUser(const std::string& userData) {
        std::cout << "💾 Сохранение пользователя в БД: " << userData << "\n";
        // Жестко связанная логика работы с БД
    }
    
    std::string getUserData(int userId) {
        std::cout << "📖 Получение данных пользователя " << userId << " из БД\n";
        return "User data for " + std::to_string(userId);
    }
};

class BadLogger {
public:
    void log(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::cout << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") 
                  << "] " << message << "\n";
        // Жестко связанная логика логирования
    }
};

class BadUserService {
private:
    // Проблема: Жестко связанные зависимости
    BadEmailService emailService;
    BadSMSService smsService;
    BadDatabaseService databaseService;
    BadLogger logger;
    
public:
    BadUserService() {
        // Проблема: Создание зависимостей внутри класса
        std::cout << "🏗️ UserService создан с жестко связанными зависимостями\n";
    }
    
    void registerUser(const std::string& username, const std::string& email, const std::string& phone) {
        logger.log("Начало регистрации пользователя: " + username);
        
        // Бизнес-логика
        std::string userData = "User: " + username + ", Email: " + email + ", Phone: " + phone;
        
        // Сохранение в БД
        databaseService.saveUser(userData);
        
        // Отправка уведомлений
        emailService.sendEmail(email, "Добро пожаловать!", "Вы успешно зарегистрированы!");
        smsService.sendSMS(phone, "Регистрация завершена успешно!");
        
        logger.log("Пользователь зарегистрирован: " + username);
    }
    
    void updateUser(int userId, const std::string& newData) {
        logger.log("Обновление пользователя: " + std::to_string(userId));
        
        // Получение данных из БД
        std::string currentData = databaseService.getUserData(userId);
        
        // Обновление данных
        std::string updatedData = currentData + " -> " + newData;
        databaseService.saveUser(updatedData);
        
        logger.log("Пользователь обновлен: " + std::to_string(userId));
    }
};

// ============================================================================
// ПРИМЕР 2: CONSTRUCTOR INJECTION
// ============================================================================

/**
 * @brief Решение: Dependency Injection через конструктор
 * 
 * Этот код демонстрирует правильное использование DI:
 * - Loose coupling
 * - Улучшенная тестируемость
 * - Соблюдение принципа единственной ответственности
 */

// Абстракции (интерфейсы)
class IEmailService {
public:
    virtual ~IEmailService() = default;
    virtual void sendEmail(const std::string& to, const std::string& subject, const std::string& body) = 0;
    virtual std::string getServiceName() const = 0;
};

class ISMSService {
public:
    virtual ~ISMSService() = default;
    virtual void sendSMS(const std::string& phone, const std::string& message) = 0;
    virtual std::string getServiceName() const = 0;
};

class IDatabaseService {
public:
    virtual ~IDatabaseService() = default;
    virtual void saveUser(const std::string& userData) = 0;
    virtual std::string getUserData(int userId) = 0;
    virtual std::string getServiceName() const = 0;
};

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(const std::string& message) = 0;
    virtual void logError(const std::string& error) = 0;
    virtual std::string getLoggerName() const = 0;
};

// Конкретные реализации
class EmailService : public IEmailService {
private:
    std::string smtpServer;
    int port;
    
public:
    EmailService(const std::string& server = "smtp.gmail.com", int serverPort = 587) 
        : smtpServer(server), port(serverPort) {
        std::cout << "📧 EmailService создан (сервер: " << smtpServer << ":" << port << ")\n";
    }
    
    void sendEmail(const std::string& to, const std::string& subject, const std::string& body) override {
        std::cout << "📧 [EmailService] Отправка email на " << to << " через " << smtpServer << "\n";
        std::cout << "Тема: " << subject << "\n";
        std::cout << "Содержание: " << body << "\n";
    }
    
    std::string getServiceName() const override {
        return "EmailService";
    }
};

class SMSService : public ISMSService {
private:
    std::string provider;
    std::string apiKey;
    
public:
    SMSService(const std::string& smsProvider = "Twilio", const std::string& key = "api_key_123") 
        : provider(smsProvider), apiKey(key) {
        std::cout << "📱 SMSService создан (провайдер: " << provider << ")\n";
    }
    
    void sendSMS(const std::string& phone, const std::string& message) override {
        std::cout << "📱 [SMSService] Отправка SMS на " << phone << " через " << provider << "\n";
        std::cout << "Сообщение: " << message << "\n";
    }
    
    std::string getServiceName() const override {
        return "SMSService";
    }
};

class DatabaseService : public IDatabaseService {
private:
    std::string connectionString;
    std::string dbType;
    
public:
    DatabaseService(const std::string& connStr = "localhost:5432/mydb", const std::string& type = "PostgreSQL") 
        : connectionString(connStr), dbType(type) {
        std::cout << "💾 DatabaseService создан (" << dbType << "): " << connectionString << "\n";
    }
    
    void saveUser(const std::string& userData) override {
        std::cout << "💾 [DatabaseService] Сохранение в " << dbType << ": " << userData << "\n";
    }
    
    std::string getUserData(int userId) override {
        std::cout << "📖 [DatabaseService] Получение данных пользователя " << userId << " из " << dbType << "\n";
        return "User data for " + std::to_string(userId) + " from " + dbType;
    }
    
    std::string getServiceName() const override {
        return "DatabaseService";
    }
};

class Logger : public ILogger {
private:
    std::string logLevel;
    std::string outputFormat;
    
public:
    Logger(const std::string& level = "INFO", const std::string& format = "TIMESTAMP") 
        : logLevel(level), outputFormat(format) {
        std::cout << "📝 Logger создан (уровень: " << logLevel << ", формат: " << outputFormat << ")\n";
    }
    
    void log(const std::string& message) override {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::cout << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") 
                  << "] [" << logLevel << "] " << message << "\n";
    }
    
    void logError(const std::string& error) override {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::cout << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") 
                  << "] [ERROR] " << error << "\n";
    }
    
    std::string getLoggerName() const override {
        return "Logger";
    }
};

// Основной сервис с DI
class GoodUserService {
private:
    // Зависимости внедрены через конструктор
    std::unique_ptr<IEmailService> emailService;
    std::unique_ptr<ISMSService> smsService;
    std::unique_ptr<IDatabaseService> databaseService;
    std::unique_ptr<ILogger> logger;
    
public:
    // Constructor Injection
    GoodUserService(std::unique_ptr<IEmailService> email,
                   std::unique_ptr<ISMSService> sms,
                   std::unique_ptr<IDatabaseService> db,
                   std::unique_ptr<ILogger> log)
        : emailService(std::move(email)),
          smsService(std::move(sms)),
          databaseService(std::move(db)),
          logger(std::move(log)) {
        std::cout << "🏗️ GoodUserService создан с внедренными зависимостями\n";
        logger->log("UserService инициализирован");
    }
    
    void registerUser(const std::string& username, const std::string& email, const std::string& phone) {
        logger->log("Начало регистрации пользователя: " + username);
        
        try {
            // Бизнес-логика
            std::string userData = "User: " + username + ", Email: " + email + ", Phone: " + phone;
            
            // Сохранение в БД
            databaseService->saveUser(userData);
            
            // Отправка уведомлений
            emailService->sendEmail(email, "Добро пожаловать!", "Вы успешно зарегистрированы!");
            smsService->sendSMS(phone, "Регистрация завершена успешно!");
            
            logger->log("Пользователь зарегистрирован: " + username);
            
        } catch (const std::exception& e) {
            logger->logError("Ошибка регистрации пользователя " + username + ": " + e.what());
        }
    }
    
    void updateUser(int userId, const std::string& newData) {
        logger->log("Обновление пользователя: " + std::to_string(userId));
        
        try {
            // Получение данных из БД
            std::string currentData = databaseService->getUserData(userId);
            
            // Обновление данных
            std::string updatedData = currentData + " -> " + newData;
            databaseService->saveUser(updatedData);
            
            logger->log("Пользователь обновлен: " + std::to_string(userId));
            
        } catch (const std::exception& e) {
            logger->logError("Ошибка обновления пользователя " + std::to_string(userId) + ": " + e.what());
        }
    }
    
    // Геттеры для информации о зависимостях
    void printDependencies() {
        std::cout << "\n📋 Зависимости UserService:\n";
        std::cout << "  Email Service: " << emailService->getServiceName() << "\n";
        std::cout << "  SMS Service: " << smsService->getServiceName() << "\n";
        std::cout << "  Database Service: " << databaseService->getServiceName() << "\n";
        std::cout << "  Logger: " << logger->getLoggerName() << "\n";
    }
};

// ============================================================================
// ПРИМЕР 3: SETTER INJECTION
// ============================================================================

/**
 * @brief Демонстрация Setter Injection
 */
class FlexibleUserService {
private:
    std::unique_ptr<IEmailService> emailService;
    std::unique_ptr<ISMSService> smsService;
    std::unique_ptr<IDatabaseService> databaseService;
    std::unique_ptr<ILogger> logger;
    
public:
    FlexibleUserService() {
        std::cout << "🏗️ FlexibleUserService создан (зависимости будут внедрены через сеттеры)\n";
    }
    
    // Setter Injection
    void setEmailService(std::unique_ptr<IEmailService> email) {
        emailService = std::move(email);
        std::cout << "📧 Email service установлен: " << emailService->getServiceName() << "\n";
    }
    
    void setSMSService(std::unique_ptr<ISMSService> sms) {
        smsService = std::move(sms);
        std::cout << "📱 SMS service установлен: " << smsService->getServiceName() << "\n";
    }
    
    void setDatabaseService(std::unique_ptr<IDatabaseService> db) {
        databaseService = std::move(db);
        std::cout << "💾 Database service установлен: " << databaseService->getServiceName() << "\n";
    }
    
    void setLogger(std::unique_ptr<ILogger> log) {
        logger = std::move(log);
        std::cout << "📝 Logger установлен: " << logger->getLoggerName() << "\n";
    }
    
    bool isConfigured() const {
        return emailService && smsService && databaseService && logger;
    }
    
    void registerUser(const std::string& username, const std::string& email, const std::string& phone) {
        if (!isConfigured()) {
            throw std::runtime_error("UserService не полностью настроен");
        }
        
        logger->log("Начало регистрации пользователя: " + username);
        
        std::string userData = "User: " + username + ", Email: " + email + ", Phone: " + phone;
        databaseService->saveUser(userData);
        emailService->sendEmail(email, "Добро пожаловать!", "Вы успешно зарегистрированы!");
        smsService->sendSMS(phone, "Регистрация завершена успешно!");
        
        logger->log("Пользователь зарегистрирован: " + username);
    }
};

// ============================================================================
// ПРИМЕР 4: INTERFACE INJECTION
// ============================================================================

/**
 * @brief Демонстрация Interface Injection
 */
class IUserServiceContainer {
public:
    virtual ~IUserServiceContainer() = default;
    virtual void setEmailService(std::unique_ptr<IEmailService> email) = 0;
    virtual void setSMSService(std::unique_ptr<ISMSService> sms) = 0;
    virtual void setDatabaseService(std::unique_ptr<IDatabaseService> db) = 0;
    virtual void setLogger(std::unique_ptr<ILogger> logger) = 0;
};

class InjectableUserService : public IUserServiceContainer {
private:
    std::unique_ptr<IEmailService> emailService;
    std::unique_ptr<ISMSService> smsService;
    std::unique_ptr<IDatabaseService> databaseService;
    std::unique_ptr<ILogger> logger;
    
public:
    InjectableUserService() {
        std::cout << "🏗️ InjectableUserService создан (Interface Injection)\n";
    }
    
    // Interface Injection через интерфейс
    void setEmailService(std::unique_ptr<IEmailService> email) override {
        emailService = std::move(email);
        std::cout << "📧 Email service внедрен через интерфейс: " << emailService->getServiceName() << "\n";
    }
    
    void setSMSService(std::unique_ptr<ISMSService> sms) override {
        smsService = std::move(sms);
        std::cout << "📱 SMS service внедрен через интерфейс: " << smsService->getServiceName() << "\n";
    }
    
    void setDatabaseService(std::unique_ptr<IDatabaseService> db) override {
        databaseService = std::move(db);
        std::cout << "💾 Database service внедрен через интерфейс: " << databaseService->getServiceName() << "\n";
    }
    
    void setLogger(std::unique_ptr<ILogger> log) override {
        logger = std::move(log);
        std::cout << "📝 Logger внедрен через интерфейс: " << logger->getLoggerName() << "\n";
    }
    
    void registerUser(const std::string& username, const std::string& email, const std::string& phone) {
        if (!emailService || !smsService || !databaseService || !logger) {
            throw std::runtime_error("Не все зависимости внедрены");
        }
        
        logger->log("Начало регистрации пользователя: " + username);
        
        std::string userData = "User: " + username + ", Email: " + email + ", Phone: " + phone;
        databaseService->saveUser(userData);
        emailService->sendEmail(email, "Добро пожаловать!", "Вы успешно зарегистрированы!");
        smsService->sendSMS(phone, "Регистрация завершена успешно!");
        
        logger->log("Пользователь зарегистрирован: " + username);
    }
};

// ============================================================================
// ДЕМОНСТРАЦИЯ ПРИНЦИПА
// ============================================================================

void demonstrateProblemsWithoutDI() {
    std::cout << "❌ ПРОБЛЕМЫ БЕЗ DEPENDENCY INJECTION:\n";
    std::cout << std::string(50, '-') << "\n";
    
    BadUserService badService;
    badService.registerUser("Иван", "ivan@example.com", "+1234567890");
    badService.updateUser(1, "Новые данные");
    
    std::cout << "\n⚠️ ПРОБЛЕМЫ:\n";
    std::cout << "• Жестко связанные зависимости\n";
    std::cout << "• Сложность тестирования\n";
    std::cout << "• Нарушение принципа единственной ответственности\n";
    std::cout << "• Сложность замены реализаций\n\n";
}

void demonstrateConstructorInjection() {
    std::cout << "✅ CONSTRUCTOR INJECTION:\n";
    std::cout << std::string(50, '-') << "\n";
    
    // Создание зависимостей
    auto emailService = std::make_unique<EmailService>("smtp.company.com", 587);
    auto smsService = std::make_unique<SMSService>("Twilio", "api_key_456");
    auto databaseService = std::make_unique<DatabaseService>("postgresql://localhost:5432/app", "PostgreSQL");
    auto logger = std::make_unique<Logger>("DEBUG", "TIMESTAMP");
    
    // Внедрение через конструктор
    GoodUserService userService(std::move(emailService), std::move(smsService), 
                               std::move(databaseService), std::move(logger));
    
    // Использование сервиса
    userService.registerUser("Петр", "petr@example.com", "+0987654321");
    userService.updateUser(2, "Обновленные данные");
    
    userService.printDependencies();
    
    std::cout << "\n✅ ПРЕИМУЩЕСТВА:\n";
    std::cout << "• Слабая связанность\n";
    std::cout << "• Улучшенная тестируемость\n";
    std::cout << "• Легкость замены реализаций\n";
    std::cout << "• Соблюдение принципов SOLID\n\n";
}

void demonstrateSetterInjection() {
    std::cout << "✅ SETTER INJECTION:\n";
    std::cout << std::string(50, '-') << "\n";
    
    FlexibleUserService flexibleService;
    
    // Внедрение через сеттеры
    flexibleService.setEmailService(std::make_unique<EmailService>("smtp.gmail.com", 465));
    flexibleService.setSMSService(std::make_unique<SMSService>("SendGrid", "sg_api_key"));
    flexibleService.setDatabaseService(std::make_unique<DatabaseService>("mysql://localhost:3306/app", "MySQL"));
    flexibleService.setLogger(std::make_unique<Logger>("INFO", "SIMPLE"));
    
    // Проверка конфигурации
    if (flexibleService.isConfigured()) {
        flexibleService.registerUser("Мария", "maria@example.com", "+1122334455");
    }
    
    std::cout << "\n✅ ПРЕИМУЩЕСТВА SETTER INJECTION:\n";
    std::cout << "• Гибкость в настройке\n";
    std::cout << "• Возможность частичной конфигурации\n";
    std::cout << "• Подходит для опциональных зависимостей\n\n";
}

void demonstrateInterfaceInjection() {
    std::cout << "✅ INTERFACE INJECTION:\n";
    std::cout << std::string(50, '-') << "\n";
    
    auto injectableService = std::make_unique<InjectableUserService>();
    
    // Внедрение через интерфейс
    injectableService->setEmailService(std::make_unique<EmailService>("outlook.com", 587));
    injectableService->setSMSService(std::make_unique<SMSService>("AWS SNS", "aws_key"));
    injectableService->setDatabaseService(std::make_unique<DatabaseService>("mongodb://localhost:27017", "MongoDB"));
    injectableService->setLogger(std::make_unique<Logger>("WARN", "JSON"));
    
    injectableService->registerUser("Анна", "anna@example.com", "+9988776655");
    
    std::cout << "\n✅ ПРЕИМУЩЕСТВА INTERFACE INJECTION:\n";
    std::cout << "• Четкий контракт внедрения\n";
    std::cout << "• Поддержка множественных реализаций\n";
    std::cout << "• Легкость создания моков для тестирования\n\n";
}

void analyzeDIAdvantages() {
    std::cout << "🔬 АНАЛИЗ ПРЕИМУЩЕСТВ DEPENDENCY INJECTION:\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::cout << "📊 ОСНОВНЫЕ ПРЕИМУЩЕСТВА:\n";
    std::cout << "• Loose Coupling - слабая связанность между компонентами\n";
    std::cout << "• Testability - улучшенная возможность тестирования\n";
    std::cout << "• Flexibility - гибкость в замене реализаций\n";
    std::cout << "• Single Responsibility - соблюдение принципа единственной ответственности\n";
    std::cout << "• Dependency Inversion - соблюдение принципа инверсии зависимостей\n\n";
    
    std::cout << "⚠️ НЕДОСТАТКИ:\n";
    std::cout << "• Complexity - усложнение архитектуры\n";
    std::cout << "• Learning Curve - кривая обучения\n";
    std::cout << "• Over-engineering - риск переусложнения\n";
    std::cout << "• Performance - потенциальный overhead\n\n";
    
    std::cout << "🎯 РЕКОМЕНДАЦИИ:\n";
    std::cout << "• Используйте Constructor Injection для обязательных зависимостей\n";
    std::cout << "• Используйте Setter Injection для опциональных зависимостей\n";
    std::cout << "• Используйте Interface Injection для сложных сценариев\n";
    std::cout << "• Избегайте DI для простых случаев\n";
}

int main() {
    std::cout << "🎯 ОСНОВЫ DEPENDENCY INJECTION\n";
    std::cout << "Автор: Senior C++ Developer\n";
    std::cout << "Цель: Понимание принципов и методов внедрения зависимостей\n\n";
    
    demonstrateProblemsWithoutDI();
    demonstrateConstructorInjection();
    demonstrateSetterInjection();
    demonstrateInterfaceInjection();
    analyzeDIAdvantages();
    
    std::cout << "\n📚 МАТЕМАТИЧЕСКОЕ ОБОСНОВАНИЕ:\n";
    std::cout << "DI: ∀O ∈ Objects, ∀D ∈ Dependencies: \n";
    std::cout << "DI(O, D) ⟹ D ∈ injected(O) ∧ D ∉ created(O)\n\n";
    
    std::cout << "💡 КЛЮЧЕВЫЕ ВЫВОДЫ:\n";
    std::cout << "1. DI улучшает тестируемость и гибкость\n";
    std::cout << "2. Constructor Injection предпочтительнее для обязательных зависимостей\n";
    std::cout << "3. Setter Injection подходит для опциональных зависимостей\n";
    std::cout << "4. Interface Injection обеспечивает четкий контракт\n\n";
    
    std::cout << "🔬 Dependency Injection - основа современной архитектуры!\n";
    
    return 0;
}
