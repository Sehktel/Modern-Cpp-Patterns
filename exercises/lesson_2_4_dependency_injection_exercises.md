# 🏋️ Упражнения: Dependency Injection (Внедрение зависимостей)

## 📋 Задание 1: Рефакторинг жестко связанного кода

### Описание
Перепишите следующий код, заменив жесткие зависимости на dependency injection.

### Исходный код (жестко связанный)
```cpp
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

// Нарушает принцип инверсии зависимостей
class EmailService {
public:
    void sendEmail(const std::string& to, const std::string& subject, const std::string& body) {
        std::cout << "Отправка email на " << to << ": " << subject << std::endl;
        std::cout << "Содержимое: " << body << std::endl;
    }
};

class DatabaseService {
public:
    void saveUser(const std::string& name, const std::string& email) {
        std::cout << "Сохранение пользователя в базе данных: " << name << ", " << email << std::endl;
    }
    
    std::vector<std::string> getAllUsers() {
        return {"Иван", "Петр", "Мария"};
    }
};

class FileService {
public:
    void saveToFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        file << content;
        file.close();
        std::cout << "Сохранено в файл: " << filename << std::endl;
    }
};

// Проблемный класс с жесткими зависимостями
class UserService {
private:
    EmailService emailService;      // Жесткая зависимость!
    DatabaseService databaseService; // Жесткая зависимость!
    FileService fileService;        // Жесткая зависимость!
    
public:
    UserService() {
        // Зависимости создаются внутри класса
        emailService = EmailService();
        databaseService = DatabaseService();
        fileService = FileService();
    }
    
    void registerUser(const std::string& name, const std::string& email) {
        // Сохраняем в базу данных
        databaseService.saveUser(name, email);
        
        // Отправляем приветственное письмо
        emailService.sendEmail(email, "Добро пожаловать!", "Спасибо за регистрацию!");
        
        // Сохраняем в файл для резервной копии
        fileService.saveToFile("backup.txt", name + "," + email);
        
        std::cout << "Пользователь " << name << " зарегистрирован" << std::endl;
    }
    
    void sendNewsletter(const std::string& subject, const std::string& body) {
        auto users = databaseService.getAllUsers();
        for (const auto& user : users) {
            emailService.sendEmail(user + "@example.com", subject, body);
        }
    }
};
```

### Требования к решению
1. ✅ Создайте интерфейсы для всех сервисов
2. ✅ Используйте dependency injection в конструкторе
3. ✅ Сделайте классы тестируемыми
4. ✅ Уберите жесткие зависимости
5. ✅ Примените принцип инверсии зависимостей

### Ожидаемый результат
```cpp
// Интерфейсы
class IEmailService {
    // Интерфейс для отправки email
};

class IDatabaseService {
    // Интерфейс для работы с базой данных
};

class IFileService {
    // Интерфейс для работы с файлами
};

// Рефакторированный UserService с DI
class UserService {
    // Использует dependency injection
};
```

---

## 📋 Задание 2: Система уведомлений с DI

### Описание
Создайте систему уведомлений, используя dependency injection для всех зависимостей.

### Требования
1. **Интерфейсы**: Создайте интерфейсы для всех сервисов
2. **DI**: Используйте dependency injection
3. **Тестируемость**: Классы должны быть легко тестируемы
4. **Гибкость**: Легко заменять реализации

### Интерфейсы
```cpp
// Интерфейс для отправки уведомлений
class INotificationService {
public:
    virtual ~INotificationService() = default;
    virtual bool sendNotification(const std::string& recipient, const std::string& message) = 0;
};

// Интерфейс для логирования
class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(const std::string& message) = 0;
    virtual void logError(const std::string& error) = 0;
};

// Интерфейс для хранения пользователей
class IUserRepository {
public:
    virtual ~IUserRepository() = default;
    virtual std::vector<std::string> getAllUsers() = 0;
    virtual bool addUser(const std::string& user) = 0;
    virtual bool removeUser(const std::string& user) = 0;
};

// Интерфейс для конфигурации
class IConfiguration {
public:
    virtual ~IConfiguration() = default;
    virtual std::string getValue(const std::string& key) = 0;
    virtual void setValue(const std::string& key, const std::string& value) = 0;
};
```

### Конкретные реализации
```cpp
// Реализация для отправки email
class EmailNotificationService : public INotificationService {
public:
    bool sendNotification(const std::string& recipient, const std::string& message) override {
        std::cout << "Отправка email на " << recipient << ": " << message << std::endl;
        return true;
    }
};

// Реализация для отправки SMS
class SMSNotificationService : public INotificationService {
public:
    bool sendNotification(const std::string& recipient, const std::string& message) override {
        std::cout << "Отправка SMS на " << recipient << ": " << message << std::endl;
        return true;
    }
};

// Реализация для консольного логирования
class ConsoleLogger : public ILogger {
public:
    void log(const std::string& message) override {
        std::cout << "[INFO] " << message << std::endl;
    }
    
    void logError(const std::string& error) override {
        std::cerr << "[ERROR] " << error << std::endl;
    }
};

// Реализация для файлового логирования
class FileLogger : public ILogger {
private:
    std::string filename_;
    
public:
    FileLogger(const std::string& filename) : filename_(filename) {}
    
    void log(const std::string& message) override {
        std::ofstream file(filename_, std::ios::app);
        file << "[INFO] " << message << std::endl;
    }
    
    void logError(const std::string& error) override {
        std::ofstream file(filename_, std::ios::app);
        file << "[ERROR] " << error << std::endl;
    }
};

// Реализация для хранения в памяти
class InMemoryUserRepository : public IUserRepository {
private:
    std::vector<std::string> users_;
    
public:
    std::vector<std::string> getAllUsers() override {
        return users_;
    }
    
    bool addUser(const std::string& user) override {
        users_.push_back(user);
        return true;
    }
    
    bool removeUser(const std::string& user) override {
        auto it = std::find(users_.begin(), users_.end(), user);
        if (it != users_.end()) {
            users_.erase(it);
            return true;
        }
        return false;
    }
};

// Реализация для конфигурации в памяти
class InMemoryConfiguration : public IConfiguration {
private:
    std::map<std::string, std::string> config_;
    
public:
    std::string getValue(const std::string& key) override {
        auto it = config_.find(key);
        return (it != config_.end()) ? it->second : "";
    }
    
    void setValue(const std::string& key, const std::string& value) override {
        config_[key] = value;
    }
};
```

### Основной сервис с DI
```cpp
class NotificationManager {
private:
    std::unique_ptr<INotificationService> notificationService_;
    std::unique_ptr<ILogger> logger_;
    std::unique_ptr<IUserRepository> userRepository_;
    std::unique_ptr<IConfiguration> configuration_;
    
public:
    // Конструктор с dependency injection
    NotificationManager(std::unique_ptr<INotificationService> notificationService,
                       std::unique_ptr<ILogger> logger,
                       std::unique_ptr<IUserRepository> userRepository,
                       std::unique_ptr<IConfiguration> configuration)
        : notificationService_(std::move(notificationService)),
          logger_(std::move(logger)),
          userRepository_(std::move(userRepository)),
          configuration_(std::move(configuration)) {}
    
    void sendNotificationToAll(const std::string& message) {
        logger_->log("Отправка уведомления всем пользователям");
        
        auto users = userRepository_->getAllUsers();
        for (const auto& user : users) {
            bool success = notificationService_->sendNotification(user, message);
            if (success) {
                logger_->log("Уведомление отправлено пользователю: " + user);
            } else {
                logger_->logError("Ошибка отправки уведомления пользователю: " + user);
            }
        }
    }
    
    void sendNotificationToUser(const std::string& user, const std::string& message) {
        logger_->log("Отправка уведомления пользователю: " + user);
        
        bool success = notificationService_->sendNotification(user, message);
        if (success) {
            logger_->log("Уведомление успешно отправлено");
        } else {
            logger_->logError("Ошибка отправки уведомления");
        }
    }
    
    void addUser(const std::string& user) {
        bool success = userRepository_->addUser(user);
        if (success) {
            logger_->log("Пользователь добавлен: " + user);
        } else {
            logger_->logError("Ошибка добавления пользователя: " + user);
        }
    }
    
    void removeUser(const std::string& user) {
        bool success = userRepository_->removeUser(user);
        if (success) {
            logger_->log("Пользователь удален: " + user);
        } else {
            logger_->logError("Ошибка удаления пользователя: " + user);
        }
    }
};
```

### Тестовый случай
```cpp
int main() {
    // Создаем зависимости
    auto emailService = std::make_unique<EmailNotificationService>();
    auto logger = std::make_unique<ConsoleLogger>();
    auto userRepository = std::make_unique<InMemoryUserRepository>();
    auto configuration = std::make_unique<InMemoryConfiguration>();
    
    // Создаем менеджер уведомлений с DI
    NotificationManager manager(std::move(emailService), std::move(logger), 
                               std::move(userRepository), std::move(configuration));
    
    // Добавляем пользователей
    manager.addUser("Иван");
    manager.addUser("Петр");
    manager.addUser("Мария");
    
    // Отправляем уведомления
    manager.sendNotificationToAll("Добро пожаловать в нашу систему!");
    manager.sendNotificationToUser("Иван", "Персональное сообщение");
    
    // Заменяем сервис уведомлений на SMS
    auto smsService = std::make_unique<SMSNotificationService>();
    auto fileLogger = std::make_unique<FileLogger>("notifications.log");
    auto newUserRepository = std::make_unique<InMemoryUserRepository>();
    auto newConfiguration = std::make_unique<InMemoryConfiguration>();
    
    NotificationManager smsManager(std::move(smsService), std::move(fileLogger),
                                   std::move(newUserRepository), std::move(newConfiguration));
    
    smsManager.addUser("Анна");
    smsManager.sendNotificationToAll("SMS уведомление");
    
    return 0;
}
```

---

## 📋 Задание 3: Система обработки платежей с DI

### Описание
Создайте систему обработки платежей, используя dependency injection для всех зависимостей.

### Требования
1. **Интерфейсы**: Создайте интерфейсы для всех сервисов
2. **DI**: Используйте dependency injection
3. **Тестируемость**: Классы должны быть легко тестируемы
4. **Гибкость**: Легко заменять реализации

### Интерфейсы
```cpp
// Интерфейс для обработки платежей
class IPaymentProcessor {
public:
    virtual ~IPaymentProcessor() = default;
    virtual bool processPayment(double amount, const std::string& cardNumber) = 0;
    virtual bool refundPayment(double amount, const std::string& transactionId) = 0;
};

// Интерфейс для валидации платежей
class IPaymentValidator {
public:
    virtual ~IPaymentValidator() = default;
    virtual bool validatePayment(double amount, const std::string& cardNumber) = 0;
    virtual std::string getValidationError() const = 0;
};

// Интерфейс для логирования платежей
class IPaymentLogger {
public:
    virtual ~IPaymentLogger() = default;
    virtual void logPayment(const std::string& transactionId, double amount, bool success) = 0;
    virtual void logRefund(const std::string& transactionId, double amount, bool success) = 0;
};

// Интерфейс для уведомлений
class IPaymentNotifier {
public:
    virtual ~IPaymentNotifier() = default;
    virtual void notifyPaymentSuccess(const std::string& transactionId, double amount) = 0;
    virtual void notifyPaymentFailure(const std::string& transactionId, const std::string& reason) = 0;
};
```

### Конкретные реализации
```cpp
// Реализация для обработки кредитных карт
class CreditCardProcessor : public IPaymentProcessor {
public:
    bool processPayment(double amount, const std::string& cardNumber) override {
        std::cout << "Обработка платежа по карте " << cardNumber << " на сумму " << amount << std::endl;
        return true;
    }
    
    bool refundPayment(double amount, const std::string& transactionId) override {
        std::cout << "Возврат платежа " << transactionId << " на сумму " << amount << std::endl;
        return true;
    }
};

// Реализация для обработки PayPal
class PayPalProcessor : public IPaymentProcessor {
public:
    bool processPayment(double amount, const std::string& cardNumber) override {
        std::cout << "Обработка платежа через PayPal на сумму " << amount << std::endl;
        return true;
    }
    
    bool refundPayment(double amount, const std::string& transactionId) override {
        std::cout << "Возврат платежа через PayPal " << transactionId << " на сумму " << amount << std::endl;
        return true;
    }
};

// Реализация для валидации платежей
class PaymentValidator : public IPaymentValidator {
private:
    std::string lastError_;
    
public:
    bool validatePayment(double amount, const std::string& cardNumber) override {
        if (amount <= 0) {
            lastError_ = "Сумма должна быть положительной";
            return false;
        }
        
        if (cardNumber.length() != 16) {
            lastError_ = "Номер карты должен содержать 16 цифр";
            return false;
        }
        
        return true;
    }
    
    std::string getValidationError() const override {
        return lastError_;
    }
};

// Реализация для логирования платежей
class PaymentLogger : public IPaymentLogger {
public:
    void logPayment(const std::string& transactionId, double amount, bool success) override {
        std::cout << "[PAYMENT] " << transactionId << " - " << amount << " - " 
                  << (success ? "SUCCESS" : "FAILURE") << std::endl;
    }
    
    void logRefund(const std::string& transactionId, double amount, bool success) override {
        std::cout << "[REFUND] " << transactionId << " - " << amount << " - " 
                  << (success ? "SUCCESS" : "FAILURE") << std::endl;
    }
};

// Реализация для уведомлений
class PaymentNotifier : public IPaymentNotifier {
public:
    void notifyPaymentSuccess(const std::string& transactionId, double amount) override {
        std::cout << "Уведомление: Платеж " << transactionId << " на сумму " << amount << " успешно обработан" << std::endl;
    }
    
    void notifyPaymentFailure(const std::string& transactionId, const std::string& reason) override {
        std::cout << "Уведомление: Платеж " << transactionId << " не обработан. Причина: " << reason << std::endl;
    }
};
```

### Основной сервис с DI
```cpp
class PaymentService {
private:
    std::unique_ptr<IPaymentProcessor> processor_;
    std::unique_ptr<IPaymentValidator> validator_;
    std::unique_ptr<IPaymentLogger> logger_;
    std::unique_ptr<IPaymentNotifier> notifier_;
    
public:
    // Конструктор с dependency injection
    PaymentService(std::unique_ptr<IPaymentProcessor> processor,
                   std::unique_ptr<IPaymentValidator> validator,
                   std::unique_ptr<IPaymentLogger> logger,
                   std::unique_ptr<IPaymentNotifier> notifier)
        : processor_(std::move(processor)),
          validator_(std::move(validator)),
          logger_(std::move(logger)),
          notifier_(std::move(notifier)) {}
    
    bool processPayment(double amount, const std::string& cardNumber) {
        // Валидация
        if (!validator_->validatePayment(amount, cardNumber)) {
            std::string error = validator_->getValidationError();
            logger_->logPayment("INVALID", amount, false);
            notifier_->notifyPaymentFailure("INVALID", error);
            return false;
        }
        
        // Обработка платежа
        std::string transactionId = "TXN" + std::to_string(rand() % 10000);
        bool success = processor_->processPayment(amount, cardNumber);
        
        // Логирование
        logger_->logPayment(transactionId, amount, success);
        
        // Уведомления
        if (success) {
            notifier_->notifyPaymentSuccess(transactionId, amount);
        } else {
            notifier_->notifyPaymentFailure(transactionId, "Ошибка обработки платежа");
        }
        
        return success;
    }
    
    bool refundPayment(double amount, const std::string& transactionId) {
        bool success = processor_->refundPayment(amount, transactionId);
        
        logger_->logRefund(transactionId, amount, success);
        
        if (success) {
            notifier_->notifyPaymentSuccess(transactionId + "_REFUND", amount);
        } else {
            notifier_->notifyPaymentFailure(transactionId + "_REFUND", "Ошибка возврата платежа");
        }
        
        return success;
    }
};
```

### Тестовый случай
```cpp
int main() {
    // Создаем зависимости для кредитных карт
    auto creditCardProcessor = std::make_unique<CreditCardProcessor>();
    auto validator = std::make_unique<PaymentValidator>();
    auto logger = std::make_unique<PaymentLogger>();
    auto notifier = std::make_unique<PaymentNotifier>();
    
    // Создаем сервис платежей с DI
    PaymentService creditCardService(std::move(creditCardProcessor), std::move(validator),
                                    std::move(logger), std::move(notifier));
    
    // Обрабатываем платежи
    creditCardService.processPayment(100.0, "1234567890123456");
    creditCardService.processPayment(-50.0, "1234567890123456"); // Ошибка валидации
    creditCardService.processPayment(200.0, "123"); // Ошибка валидации
    
    // Заменяем процессор на PayPal
    auto paypalProcessor = std::make_unique<PayPalProcessor>();
    auto newValidator = std::make_unique<PaymentValidator>();
    auto newLogger = std::make_unique<PaymentLogger>();
    auto newNotifier = std::make_unique<PaymentNotifier>();
    
    PaymentService paypalService(std::move(paypalProcessor), std::move(newValidator),
                               std::move(newLogger), std::move(newNotifier));
    
    paypalService.processPayment(150.0, "1234567890123456");
    
    return 0;
}
```

---

## 🎯 Критерии оценки

### Отлично (5 баллов)
- ✅ Все зависимости инвертированы
- ✅ Используется dependency injection
- ✅ Классы легко тестируемы
- ✅ Легко заменять реализации
- ✅ Код следует принципам SOLID

### Хорошо (4 балла)
- ✅ Большинство зависимостей инвертированы
- ✅ DI используется в основном правильно
- ✅ Есть небольшие проблемы с архитектурой
- ✅ Код работает корректно

### Удовлетворительно (3 балла)
- ✅ Базовая инверсия зависимостей реализована
- ✅ Код компилируется и работает
- ❌ Есть проблемы с тестируемостью
- ❌ Код сложно расширять

### Неудовлетворительно (2 балла)
- ❌ Зависимости не инвертированы
- ❌ DI не используется или используется неправильно
- ❌ Код не компилируется или работает неправильно

---

## 💡 Подсказки

1. **Инверсия зависимостей**: Зависите от абстракций, а не от конкретных реализаций
2. **Dependency Injection**: Передавайте зависимости через конструктор
3. **Интерфейсы**: Создавайте интерфейсы для всех зависимостей
4. **Тестируемость**: DI делает код легко тестируемым
5. **Гибкость**: Легко заменять реализации без изменения кода

---

*Dependency Injection - это ключ к созданию гибкого и тестируемого кода. Освойте этот принцип, и ваши системы станут намного лучше!*
