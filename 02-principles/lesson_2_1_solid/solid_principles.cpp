/**
 * @file solid_principles.cpp
 * @brief Демонстрация SOLID принципов в современном C++
 * @author Senior C++ Developer
 * @date 2024
 * 
 * Этот файл демонстрирует все пять принципов SOLID через практические примеры.
 * Каждый принцип рассматривается с точки зрения теории и практики.
 */

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <chrono>
#include <fstream>
#include <sstream>

// ============================================================================
// SINGLE RESPONSIBILITY PRINCIPLE (SRP)
// ============================================================================

/**
 * @brief Нарушение SRP: Класс с множественными ответственностями
 * 
 * Проблема: UserManager отвечает за:
 * 1. Хранение данных пользователя
 * 2. Валидацию данных
 * 3. Отправку email
 * 4. Логирование
 * 5. Работу с базой данных
 * 
 * Это нарушает принцип единственной ответственности.
 */
class BadUserManager {
private:
    std::string name;
    std::string email;
    std::string password;
    
public:
    void setUserData(const std::string& n, const std::string& e, const std::string& p) {
        name = n;
        email = e;
        password = p;
    }
    
    // Нарушение SRP: валидация
    bool validateUser() {
        if (name.empty() || email.empty() || password.empty()) {
            return false;
        }
        if (email.find('@') == std::string::npos) {
            return false;
        }
        return true;
    }
    
    // Нарушение SRP: отправка email
    void sendWelcomeEmail() {
        std::cout << "Отправка email на " << email << ": Добро пожаловать, " << name << "!\n";
    }
    
    // Нарушение SRP: логирование
    void logUserCreation() {
        std::ofstream log("user_creation.log", std::ios::app);
        log << "Пользователь создан: " << name << " (" << email << ")\n";
    }
    
    // Нарушение SRP: работа с БД
    void saveToDatabase() {
        std::cout << "Сохранение пользователя в БД: " << name << "\n";
    }
};

/**
 * @brief Соблюдение SRP: Разделение ответственностей
 * 
 * Каждый класс имеет единственную ответственность:
 * - User: хранение данных
 * - UserValidator: валидация
 * - EmailService: отправка email
 * - Logger: логирование
 * - UserRepository: работа с БД
 */

// Класс для хранения данных пользователя
class User {
private:
    std::string name;
    std::string email;
    std::string password;
    
public:
    User(const std::string& n, const std::string& e, const std::string& p)
        : name(n), email(e), password(p) {}
    
    // Геттеры
    const std::string& getName() const { return name; }
    const std::string& getEmail() const { return email; }
    const std::string& getPassword() const { return password; }
    
    // Сеттеры
    void setName(const std::string& n) { name = n; }
    void setEmail(const std::string& e) { email = e; }
    void setPassword(const std::string& p) { password = p; }
};

// Класс для валидации пользователя
class UserValidator {
public:
    static bool isValid(const User& user) {
        const std::string& name = user.getName();
        const std::string& email = user.getEmail();
        const std::string& password = user.getPassword();
        
        if (name.empty() || email.empty() || password.empty()) {
            return false;
        }
        
        if (email.find('@') == std::string::npos) {
            return false;
        }
        
        if (password.length() < 6) {
            return false;
        }
        
        return true;
    }
};

// Класс для отправки email
class EmailService {
public:
    static void sendWelcomeEmail(const User& user) {
        std::cout << "📧 Отправка email на " << user.getEmail() 
                  << ": Добро пожаловать, " << user.getName() << "!\n";
    }
    
    static void sendPasswordReset(const User& user) {
        std::cout << "📧 Отправка сброса пароля на " << user.getEmail() << "\n";
    }
};

// Класс для логирования
class Logger {
public:
    static void logUserCreation(const User& user) {
        std::ofstream log("user_creation.log", std::ios::app);
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        log << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") 
            << "] Пользователь создан: " << user.getName() 
            << " (" << user.getEmail() << ")\n";
    }
    
    static void logError(const std::string& error) {
        std::ofstream log("error.log", std::ios::app);
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        log << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") 
            << "] ОШИБКА: " << error << "\n";
    }
};

// Класс для работы с базой данных
class UserRepository {
public:
    static void save(const User& user) {
        std::cout << "💾 Сохранение пользователя в БД: " << user.getName() << "\n";
        // Здесь была бы реальная логика работы с БД
    }
    
    static User findById(int id) {
        // Заглушка для демонстрации
        return User("Test User", "test@example.com", "password");
    }
    
    static void deleteUser(int id) {
        std::cout << "🗑️ Удаление пользователя с ID: " << id << "\n";
    }
};

// ============================================================================
// OPEN/CLOSED PRINCIPLE (OCP)
// ============================================================================

/**
 * @brief Нарушение OCP: Модификация для добавления функциональности
 * 
 * Проблема: При добавлении нового типа платежа нужно модифицировать
 * существующий класс PaymentProcessor.
 */
class BadPaymentProcessor {
public:
    enum PaymentType {
        CREDIT_CARD,
        PAYPAL,
        BANK_TRANSFER
    };
    
    void processPayment(PaymentType type, double amount) {
        switch (type) {
            case CREDIT_CARD:
                std::cout << "Обработка кредитной карты: $" << amount << "\n";
                break;
            case PAYPAL:
                std::cout << "Обработка PayPal: $" << amount << "\n";
                break;
            case BANK_TRANSFER:
                std::cout << "Обработка банковского перевода: $" << amount << "\n";
                break;
            // При добавлении нового типа нужно модифицировать этот switch!
        }
    }
};

/**
 * @brief Соблюдение OCP: Расширение без модификации
 * 
 * Новые типы платежей добавляются через наследование от абстрактного класса,
 * без изменения существующего кода.
 */

// Абстрактный класс для стратегии платежей
class PaymentStrategy {
public:
    virtual ~PaymentStrategy() = default;
    virtual void processPayment(double amount) = 0;
    virtual std::string getPaymentType() const = 0;
};

// Конкретные реализации для разных типов платежей
class CreditCardPayment : public PaymentStrategy {
public:
    void processPayment(double amount) override {
        std::cout << "💳 Обработка кредитной карты: $" << amount << "\n";
        // Логика обработки кредитной карты
    }
    
    std::string getPaymentType() const override {
        return "Credit Card";
    }
};

class PayPalPayment : public PaymentStrategy {
public:
    void processPayment(double amount) override {
        std::cout << "💰 Обработка PayPal: $" << amount << "\n";
        // Логика обработки PayPal
    }
    
    std::string getPaymentType() const override {
        return "PayPal";
    }
};

class BankTransferPayment : public PaymentStrategy {
public:
    void processPayment(double amount) override {
        std::cout << "🏦 Обработка банковского перевода: $" << amount << "\n";
        // Логика обработки банковского перевода
    }
    
    std::string getPaymentType() const override {
        return "Bank Transfer";
    }
};

// Новый тип платежа - добавляется без изменения существующего кода!
class CryptoPayment : public PaymentStrategy {
public:
    void processPayment(double amount) override {
        std::cout << "₿ Обработка криптовалютного платежа: $" << amount << "\n";
        // Логика обработки криптовалюты
    }
    
    std::string getPaymentType() const override {
        return "Cryptocurrency";
    }
};

// Класс для обработки платежей (открыт для расширения, закрыт для модификации)
class PaymentProcessor {
private:
    std::unique_ptr<PaymentStrategy> strategy;
    
public:
    void setPaymentStrategy(std::unique_ptr<PaymentStrategy> paymentStrategy) {
        strategy = std::move(paymentStrategy);
    }
    
    void processPayment(double amount) {
        if (strategy) {
            std::cout << "Выбранный тип платежа: " << strategy->getPaymentType() << "\n";
            strategy->processPayment(amount);
        } else {
            std::cout << "❌ Стратегия платежа не установлена!\n";
        }
    }
};

// ============================================================================
// LISKOV SUBSTITUTION PRINCIPLE (LSP)
// ============================================================================

/**
 * @brief Нарушение LSP: Классический пример с Rectangle и Square
 * 
 * Проблема: Square не может быть заменен на Rectangle без нарушения
 * ожидаемого поведения.
 */
class Rectangle {
protected:
    int width;
    int height;
    
public:
    Rectangle(int w, int h) : width(w), height(h) {}
    
    virtual void setWidth(int w) { width = w; }
    virtual void setHeight(int h) { height = h; }
    virtual int getWidth() const { return width; }
    virtual int getHeight() const { return height; }
    virtual int getArea() const { return width * height; }
};

class BadSquare : public Rectangle {
public:
    BadSquare(int side) : Rectangle(side, side) {}
    
    void setWidth(int w) override {
        width = w;
        height = w; // Нарушение LSP: изменяет поведение базового класса
    }
    
    void setHeight(int h) override {
        width = h;  // Нарушение LSP: изменяет поведение базового класса
        height = h;
    }
};

/**
 * @brief Соблюдение LSP: Правильная иерархия геометрических фигур
 */

// Абстрактный базовый класс для всех фигур
class Shape {
public:
    virtual ~Shape() = default;
    virtual double getArea() const = 0;
    virtual double getPerimeter() const = 0;
    virtual std::string getType() const = 0;
};

// Правильная реализация прямоугольника
class GoodRectangle : public Shape {
private:
    double width;
    double height;
    
public:
    GoodRectangle(double w, double h) : width(w), height(h) {}
    
    double getArea() const override {
        return width * height;
    }
    
    double getPerimeter() const override {
        return 2 * (width + height);
    }
    
    std::string getType() const override {
        return "Rectangle";
    }
    
    void setWidth(double w) { width = w; }
    void setHeight(double h) { height = h; }
    double getWidth() const { return width; }
    double getHeight() const { return height; }
};

// Правильная реализация квадрата (частный случай прямоугольника)
class GoodSquare : public Shape {
private:
    double side;
    
public:
    GoodSquare(double s) : side(s) {}
    
    double getArea() const override {
        return side * side;
    }
    
    double getPerimeter() const override {
        return 4 * side;
    }
    
    std::string getType() const override {
        return "Square";
    }
    
    void setSide(double s) { side = s; }
    double getSide() const { return side; }
};

// Функция для демонстрации LSP
void demonstrateLSP(const Shape& shape) {
    std::cout << "📐 Фигура: " << shape.getType() 
              << ", Площадь: " << shape.getArea()
              << ", Периметр: " << shape.getPerimeter() << "\n";
}

// ============================================================================
// INTERFACE SEGREGATION PRINCIPLE (ISP)
// ============================================================================

/**
 * @brief Нарушение ISP: Слишком большой интерфейс
 * 
 * Проблема: Интерфейс IDevice содержит методы, которые не все устройства
 * должны поддерживать.
 */
class BadIDevice {
public:
    virtual ~BadIDevice() = default;
    virtual void read() = 0;
    virtual void write() = 0;
    virtual void scan() = 0;
    virtual void print() = 0;
    virtual void fax() = 0;
    virtual void copy() = 0;
};

// Принтер вынужден реализовывать методы, которые ему не нужны
class BadPrinter : public BadIDevice {
public:
    void read() override {
        throw std::runtime_error("Принтер не может читать!");
    }
    
    void write() override {
        throw std::runtime_error("Принтер не может писать!");
    }
    
    void scan() override {
        throw std::runtime_error("Принтер не может сканировать!");
    }
    
    void print() override {
        std::cout << "🖨️ Печать документа\n";
    }
    
    void fax() override {
        throw std::runtime_error("Принтер не может отправлять факсы!");
    }
    
    void copy() override {
        throw std::runtime_error("Принтер не может копировать!");
    }
};

/**
 * @brief Соблюдение ISP: Разделение интерфейсов по функциональности
 */

// Разделенные интерфейсы
class IReadable {
public:
    virtual ~IReadable() = default;
    virtual void read() = 0;
};

class IWritable {
public:
    virtual ~IWritable() = default;
    virtual void write() = 0;
};

class IScannable {
public:
    virtual ~IScannable() = default;
    virtual void scan() = 0;
};

class IPrintable {
public:
    virtual ~IPrintable() = default;
    virtual void print() = 0;
};

class IFaxable {
public:
    virtual ~IFaxable() = default;
    virtual void fax() = 0;
};

class ICopyable {
public:
    virtual ~ICopyable() = default;
    virtual void copy() = 0;
};

// Конкретные устройства реализуют только нужные им интерфейсы
class Printer : public IPrintable, public ICopyable {
public:
    void print() override {
        std::cout << "🖨️ Печать документа\n";
    }
    
    void copy() override {
        std::cout << "📋 Копирование документа\n";
    }
};

class Scanner : public IReadable, public IScannable {
public:
    void read() override {
        std::cout << "📖 Чтение документа\n";
    }
    
    void scan() override {
        std::cout << "📄 Сканирование документа\n";
    }
};

class MultiFunctionDevice : public IPrintable, public IScannable, public IFaxable, public ICopyable {
public:
    void print() override {
        std::cout << "🖨️ Печать документа\n";
    }
    
    void scan() override {
        std::cout << "📄 Сканирование документа\n";
    }
    
    void fax() override {
        std::cout << "📠 Отправка факса\n";
    }
    
    void copy() override {
        std::cout << "📋 Копирование документа\n";
    }
};

// ============================================================================
// DEPENDENCY INVERSION PRINCIPLE (DIP)
// ============================================================================

/**
 * @brief Нарушение DIP: Зависимость от конкретных реализаций
 * 
 * Проблема: BusinessLogic напрямую зависит от конкретных классов
 * FileLogger и EmailNotification, что затрудняет тестирование и
 * изменение поведения.
 */
class BadFileLogger {
public:
    void log(const std::string& message) {
        std::cout << "📝 [FILE] " << message << "\n";
    }
};

class BadEmailNotification {
public:
    void send(const std::string& message) {
        std::cout << "📧 [EMAIL] " << message << "\n";
    }
};

class BadBusinessLogic {
private:
    BadFileLogger logger;
    BadEmailNotification notification;
    
public:
    void processOrder(const std::string& orderId) {
        logger.log("Обработка заказа: " + orderId);
        
        // Бизнес-логика
        std::cout << "🛒 Обработка заказа " << orderId << "\n";
        
        notification.send("Заказ " + orderId + " обработан");
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
};

class INotificationService {
public:
    virtual ~INotificationService() = default;
    virtual void send(const std::string& message) = 0;
};

// Конкретные реализации
class FileLogger : public ILogger {
public:
    void log(const std::string& message) override {
        std::cout << "📝 [FILE] " << message << "\n";
        // Реальная логика записи в файл
    }
};

class ConsoleLogger : public ILogger {
public:
    void log(const std::string& message) override {
        std::cout << "🖥️ [CONSOLE] " << message << "\n";
    }
};

class EmailNotificationService : public INotificationService {
public:
    void send(const std::string& message) override {
        std::cout << "📧 [EMAIL] " << message << "\n";
        // Реальная логика отправки email
    }
};

class SMSNotificationService : public INotificationService {
public:
    void send(const std::string& message) override {
        std::cout << "📱 [SMS] " << message << "\n";
        // Реальная логика отправки SMS
    }
};

// Бизнес-логика зависит только от абстракций
class BusinessLogic {
private:
    std::unique_ptr<ILogger> logger;
    std::unique_ptr<INotificationService> notificationService;
    
public:
    // Dependency Injection через конструктор
    BusinessLogic(std::unique_ptr<ILogger> loggerPtr, 
                  std::unique_ptr<INotificationService> notificationPtr)
        : logger(std::move(loggerPtr)), notificationService(std::move(notificationPtr)) {}
    
    void processOrder(const std::string& orderId) {
        logger->log("Обработка заказа: " + orderId);
        
        // Бизнес-логика
        std::cout << "🛒 Обработка заказа " << orderId << "\n";
        
        notificationService->send("Заказ " + orderId + " обработан");
    }
};

// ============================================================================
// ДЕМОНСТРАЦИЯ ВСЕХ ПРИНЦИПОВ
// ============================================================================

void demonstrateSRP() {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "🎯 ДЕМОНСТРАЦИЯ SINGLE RESPONSIBILITY PRINCIPLE (SRP)\n";
    std::cout << std::string(60, '=') << "\n\n";
    
    std::cout << "❌ Нарушение SRP:\n";
    BadUserManager badManager;
    badManager.setUserData("Иван Иванов", "ivan@example.com", "password123");
    if (badManager.validateUser()) {
        badManager.saveToDatabase();
        badManager.sendWelcomeEmail();
        badManager.logUserCreation();
    }
    
    std::cout << "\n✅ Соблюдение SRP:\n";
    User user("Петр Петров", "petr@example.com", "securepass");
    
    if (UserValidator::isValid(user)) {
        UserRepository::save(user);
        EmailService::sendWelcomeEmail(user);
        Logger::logUserCreation(user);
    } else {
        Logger::logError("Невалидные данные пользователя");
    }
}

void demonstrateOCP() {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "🎯 ДЕМОНСТРАЦИЯ OPEN/CLOSED PRINCIPLE (OCP)\n";
    std::cout << std::string(60, '=') << "\n\n";
    
    std::cout << "❌ Нарушение OCP:\n";
    BadPaymentProcessor badProcessor;
    badProcessor.processPayment(BadPaymentProcessor::CREDIT_CARD, 100.0);
    badProcessor.processPayment(BadPaymentProcessor::PAYPAL, 200.0);
    badProcessor.processPayment(BadPaymentProcessor::BANK_TRANSFER, 300.0);
    
    std::cout << "\n✅ Соблюдение OCP:\n";
    PaymentProcessor processor;
    
    // Можно легко добавлять новые типы платежей без изменения существующего кода
    processor.setPaymentStrategy(std::make_unique<CreditCardPayment>());
    processor.processPayment(100.0);
    
    processor.setPaymentStrategy(std::make_unique<PayPalPayment>());
    processor.processPayment(200.0);
    
    processor.setPaymentStrategy(std::make_unique<BankTransferPayment>());
    processor.processPayment(300.0);
    
    // Новый тип платежа - добавляется без изменения существующего кода!
    processor.setPaymentStrategy(std::make_unique<CryptoPayment>());
    processor.processPayment(500.0);
}

void demonstrateLSP() {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "🎯 ДЕМОНСТРАЦИЯ LISKOV SUBSTITUTION PRINCIPLE (LSP)\n";
    std::cout << std::string(60, '=') << "\n\n";
    
    std::cout << "❌ Нарушение LSP:\n";
    Rectangle rect(5, 10);
    std::cout << "Прямоугольник 5x10, площадь: " << rect.getArea() << "\n";
    
    BadSquare badSquare(5);
    std::cout << "Квадрат 5x5, площадь: " << badSquare.getArea() << "\n";
    
    // Нарушение LSP: изменение width влияет на height
    badSquare.setWidth(10);
    std::cout << "После setWidth(10): " << badSquare.getWidth() << "x" 
              << badSquare.getHeight() << ", площадь: " << badSquare.getArea() << "\n";
    
    std::cout << "\n✅ Соблюдение LSP:\n";
    GoodRectangle goodRect(5, 10);
    GoodSquare goodSquare(5);
    
    // Можно безопасно использовать любую фигуру через базовый интерфейс
    demonstrateLSP(goodRect);
    demonstrateLSP(goodSquare);
    
    // Изменение размеров не нарушает ожидаемого поведения
    goodRect.setWidth(8);
    goodRect.setHeight(12);
    demonstrateLSP(goodRect);
    
    goodSquare.setSide(7);
    demonstrateLSP(goodSquare);
}

void demonstrateISP() {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "🎯 ДЕМОНСТРАЦИЯ INTERFACE SEGREGATION PRINCIPLE (ISP)\n";
    std::cout << std::string(60, '=') << "\n\n";
    
    std::cout << "❌ Нарушение ISP:\n";
    BadPrinter badPrinter;
    try {
        badPrinter.print();
        badPrinter.read(); // Выбросит исключение!
    } catch (const std::exception& e) {
        std::cout << "Ошибка: " << e.what() << "\n";
    }
    
    std::cout << "\n✅ Соблюдение ISP:\n";
    
    // Каждое устройство реализует только нужные ему интерфейсы
    Printer printer;
    Scanner scanner;
    MultiFunctionDevice mfd;
    
    std::cout << "Принтер:\n";
    printer.print();
    printer.copy();
    
    std::cout << "Сканер:\n";
    scanner.read();
    scanner.scan();
    
    std::cout << "МФУ:\n";
    mfd.print();
    mfd.scan();
    mfd.fax();
    mfd.copy();
}

void demonstrateDIP() {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "🎯 ДЕМОНСТРАЦИЯ DEPENDENCY INVERSION PRINCIPLE (DIP)\n";
    std::cout << std::string(60, '=') << "\n\n";
    
    std::cout << "❌ Нарушение DIP:\n";
    BadBusinessLogic badLogic;
    badLogic.processOrder("ORDER-001");
    
    std::cout << "\n✅ Соблюдение DIP:\n";
    
    // Можно легко менять реализации без изменения бизнес-логики
    auto fileLogger = std::make_unique<FileLogger>();
    auto emailNotification = std::make_unique<EmailNotificationService>();
    
    BusinessLogic businessLogic1(std::move(fileLogger), std::move(emailNotification));
    businessLogic1.processOrder("ORDER-002");
    
    // Используем другие реализации
    auto consoleLogger = std::make_unique<ConsoleLogger>();
    auto smsNotification = std::make_unique<SMSNotificationService>();
    
    BusinessLogic businessLogic2(std::move(consoleLogger), std::move(smsNotification));
    businessLogic2.processOrder("ORDER-003");
}

int main() {
    std::cout << "🚀 ДЕМОНСТРАЦИЯ SOLID ПРИНЦИПОВ В СОВРЕМЕННОМ C++\n";
    std::cout << "Автор: Senior C++ Developer\n";
    std::cout << "Цель: Изучение пяти фундаментальных принципов ООП\n\n";
    
    demonstrateSRP();
    demonstrateOCP();
    demonstrateLSP();
    demonstrateISP();
    demonstrateDIP();
    
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "🎓 ЗАКЛЮЧЕНИЕ\n";
    std::cout << std::string(60, '=') << "\n";
    std::cout << "SOLID принципы обеспечивают:\n";
    std::cout << "• SRP: Единственную ответственность каждого класса\n";
    std::cout << "• OCP: Расширяемость без модификации\n";
    std::cout << "• LSP: Корректность наследования\n";
    std::cout << "• ISP: Разделение интерфейсов по функциональности\n";
    std::cout << "• DIP: Зависимость от абстракций, а не от конкретных реализаций\n\n";
    
    std::cout << "💡 Ключевые выводы:\n";
    std::cout << "1. Принципы взаимосвязаны и должны применяться комплексно\n";
    std::cout << "2. Каждый принцип имеет математическое обоснование\n";
    std::cout << "3. Современные языки предоставляют новые возможности\n";
    std::cout << "4. Эмпирические исследования подтверждают эффективность\n\n";
    
    std::cout << "🔬 Принципы - это не догмы, а инструменты для мышления о проблемах!\n";
    
    return 0;
}
