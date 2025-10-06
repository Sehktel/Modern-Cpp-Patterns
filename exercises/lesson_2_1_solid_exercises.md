# 🏋️ Упражнения: SOLID принципы

## 📋 Задание 1: Рефакторинг нарушающего SOLID код

### Описание
Перепишите следующий код, нарушающий все принципы SOLID, следуя каждому принципу отдельно.

### Исходный код (нарушает все SOLID принципы)
```cpp
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

// Нарушает все принципы SOLID!
class UserManager {
private:
    std::vector<std::string> users;
    std::string logFile;
    
public:
    UserManager() : logFile("users.log") {}
    
    // Нарушает SRP: управляет пользователями И логированием И валидацией
    void addUser(const std::string& username, const std::string& email) {
        // Валидация (должна быть отдельно)
        if (username.empty() || email.empty()) {
            std::cout << "Ошибка: пустые поля" << std::endl;
            return;
        }
        
        if (email.find("@") == std::string::npos) {
            std::cout << "Ошибка: неверный email" << std::endl;
            return;
        }
        
        // Проверка дубликатов
        for (const auto& user : users) {
            if (user == username) {
                std::cout << "Ошибка: пользователь уже существует" << std::endl;
                return;
            }
        }
        
        // Добавление пользователя
        users.push_back(username);
        
        // Логирование (должно быть отдельно)
        std::ofstream log(logFile, std::ios::app);
        log << "Добавлен пользователь: " << username << std::endl;
        log.close();
        
        std::cout << "Пользователь " << username << " добавлен" << std::endl;
    }
    
    // Нарушает OCP: жестко привязан к конкретному типу хранения
    void saveToFile(const std::string& filename) {
        std::ofstream file(filename);
        for (const auto& user : users) {
            file << user << std::endl;
        }
        file.close();
    }
    
    // Нарушает LSP: не может быть заменен базовым классом
    void deleteUser(const std::string& username) {
        for (auto it = users.begin(); it != users.end(); ++it) {
            if (*it == username) {
                users.erase(it);
                
                // Логирование
                std::ofstream log(logFile, std::ios::app);
                log << "Удален пользователь: " << username << std::endl;
                log.close();
                
                std::cout << "Пользователь " << username << " удален" << std::endl;
                return;
            }
        }
        std::cout << "Пользователь не найден" << std::endl;
    }
    
    // Нарушает ISP: клиенты зависят от методов, которые не используют
    void sendEmail(const std::string& username, const std::string& message) {
        std::cout << "Отправка email пользователю " << username << ": " << message << std::endl;
    }
    
    void generateReport() {
        std::cout << "Генерация отчета..." << std::endl;
        std::cout << "Всего пользователей: " << users.size() << std::endl;
    }
    
    // Нарушает DIP: зависит от конкретных реализаций
    void backupToDatabase() {
        std::cout << "Сохранение в MySQL базу данных..." << std::endl;
    }
    
    void printUsers() const {
        for (const auto& user : users) {
            std::cout << user << std::endl;
        }
    }
};
```

### Требования к решению

#### 1. Single Responsibility Principle (SRP)
- ✅ Разделите класс на отдельные классы по ответственности
- ✅ Создайте отдельные классы для: управления пользователями, валидации, логирования

#### 2. Open/Closed Principle (OCP)
- ✅ Сделайте систему расширяемой без изменения существующего кода
- ✅ Используйте абстракции для различных типов хранилищ

#### 3. Liskov Substitution Principle (LSP)
- ✅ Создайте иерархию классов, где подклассы могут заменять базовые
- ✅ Обеспечьте корректное поведение при подстановке

#### 4. Interface Segregation Principle (ISP)
- ✅ Разделите интерфейсы на более мелкие и специфичные
- ✅ Клиенты не должны зависеть от неиспользуемых методов

#### 5. Dependency Inversion Principle (DIP)
- ✅ Зависите от абстракций, а не от конкретных реализаций
- ✅ Используйте dependency injection

### Ожидаемый результат
```cpp
// Пример структуры после рефакторинга
class UserValidator {
    // Валидация пользователей
};

class Logger {
    // Логирование
};

class UserRepository {
    // Хранение пользователей
};

class UserService {
    // Бизнес-логика пользователей
};

// И так далее...
```

---

## 📋 Задание 2: Система уведомлений с SOLID принципами

### Описание
Создайте систему уведомлений, строго следующую всем принципам SOLID.

### Требования

#### SRP: Разделение ответственности
1. **NotificationService**: Управляет отправкой уведомлений
2. **UserRepository**: Управляет пользователями
3. **MessageFormatter**: Форматирует сообщения
4. **NotificationLogger**: Логирует уведомления

#### OCP: Расширяемость
1. **NotificationChannel**: Базовый класс для каналов уведомлений
2. **EmailChannel, SMSChannel, PushChannel**: Конкретные реализации
3. Возможность добавления новых каналов без изменения существующего кода

#### LSP: Заменяемость
1. Все каналы уведомлений должны быть взаимозаменяемы
2. Подклассы должны корректно работать вместо базового класса

#### ISP: Разделение интерфейсов
1. **Sendable**: Интерфейс для отправки
2. **Receivable**: Интерфейс для получения
3. **Trackable**: Интерфейс для отслеживания статуса

#### DIP: Инверсия зависимостей
1. Зависимость от абстракций, а не от конкретных реализаций
2. Использование dependency injection

### Интерфейсы
```cpp
// ISP: Разделенные интерфейсы
class Sendable {
public:
    virtual ~Sendable() = default;
    virtual bool send(const std::string& message, const std::string& recipient) = 0;
};

class Receivable {
public:
    virtual ~Receivable() = default;
    virtual std::string receive() = 0;
};

class Trackable {
public:
    virtual ~Trackable() = default;
    virtual bool isDelivered() const = 0;
    virtual std::string getStatus() const = 0;
};

// OCP: Базовый класс для каналов
class NotificationChannel : public Sendable, public Trackable {
public:
    virtual ~NotificationChannel() = default;
    virtual std::string getChannelName() const = 0;
};

// SRP: Отдельные классы по ответственности
class User {
public:
    User(const std::string& id, const std::string& name, const std::string& email);
    const std::string& getId() const;
    const std::string& getName() const;
    const std::string& getEmail() const;
private:
    std::string id_;
    std::string name_;
    std::string email_;
};

class UserRepository {
public:
    virtual ~UserRepository() = default;
    virtual std::vector<User> getAllUsers() const = 0;
    virtual User getUserById(const std::string& id) const = 0;
    virtual void addUser(const User& user) = 0;
};

class MessageFormatter {
public:
    virtual ~MessageFormatter() = default;
    virtual std::string format(const std::string& template_, const User& user) = 0;
};

class NotificationLogger {
public:
    virtual ~NotificationLogger() = default;
    virtual void logNotification(const std::string& channel, const std::string& recipient, bool success) = 0;
};

// DIP: Сервис зависит от абстракций
class NotificationService {
public:
    NotificationService(std::unique_ptr<UserRepository> userRepo,
                      std::unique_ptr<MessageFormatter> formatter,
                      std::unique_ptr<NotificationLogger> logger);
    
    void addChannel(std::unique_ptr<NotificationChannel> channel);
    void sendToAllUsers(const std::string& messageTemplate);
    void sendToUser(const std::string& userId, const std::string& messageTemplate);
    
private:
    std::unique_ptr<UserRepository> userRepository_;
    std::unique_ptr<MessageFormatter> messageFormatter_;
    std::unique_ptr<NotificationLogger> logger_;
    std::vector<std::unique_ptr<NotificationChannel>> channels_;
};
```

### Конкретные реализации
```cpp
// OCP: Конкретные каналы уведомлений
class EmailChannel : public NotificationChannel {
public:
    bool send(const std::string& message, const std::string& recipient) override;
    bool isDelivered() const override;
    std::string getStatus() const override;
    std::string getChannelName() const override;
    
private:
    bool delivered_;
};

class SMSChannel : public NotificationChannel {
public:
    bool send(const std::string& message, const std::string& recipient) override;
    bool isDelivered() const override;
    std::string getStatus() const override;
    std::string getChannelName() const override;
    
private:
    bool delivered_;
};

class PushChannel : public NotificationChannel {
public:
    bool send(const std::string& message, const std::string& recipient) override;
    bool isDelivered() const override;
    std::string getStatus() const override;
    std::string getChannelName() const override;
    
private:
    bool delivered_;
};

// SRP: Конкретные реализации репозитория
class InMemoryUserRepository : public UserRepository {
public:
    std::vector<User> getAllUsers() const override;
    User getUserById(const std::string& id) const override;
    void addUser(const User& user) override;
    
private:
    std::vector<User> users_;
};

// SRP: Форматтер сообщений
class TemplateMessageFormatter : public MessageFormatter {
public:
    std::string format(const std::string& template_, const User& user) override;
};

// SRP: Логгер
class ConsoleNotificationLogger : public NotificationLogger {
public:
    void logNotification(const std::string& channel, const std::string& recipient, bool success) override;
};
```

### Тестовый случай
```cpp
int main() {
    // DIP: Создаем зависимости
    auto userRepo = std::make_unique<InMemoryUserRepository>();
    auto formatter = std::make_unique<TemplateMessageFormatter>();
    auto logger = std::make_unique<ConsoleNotificationLogger>();
    
    // Добавляем тестовых пользователей
    userRepo->addUser(User("1", "Иван", "ivan@example.com"));
    userRepo->addUser(User("2", "Петр", "petr@example.com"));
    
    // Создаем сервис уведомлений
    NotificationService service(std::move(userRepo), std::move(formatter), std::move(logger));
    
    // OCP: Добавляем каналы уведомлений
    service.addChannel(std::make_unique<EmailChannel>());
    service.addChannel(std::make_unique<SMSChannel>());
    service.addChannel(std::make_unique<PushChannel>());
    
    // Отправляем уведомления всем пользователям
    service.sendToAllUsers("Привет, {name}! Это уведомление для {email}");
    
    // Отправляем уведомление конкретному пользователю
    service.sendToUser("1", "Персональное сообщение для {name}");
    
    return 0;
}
```

---

## 📋 Задание 3: Система обработки платежей с SOLID

### Описание
Создайте систему обработки платежей, демонстрирующую все принципы SOLID.

### Требования

#### SRP: Разделение ответственности
1. **PaymentProcessor**: Обрабатывает платежи
2. **PaymentValidator**: Валидирует платежи
3. **PaymentLogger**: Логирует операции
4. **PaymentNotifier**: Уведомляет о статусе

#### OCP: Расширяемость
1. **PaymentMethod**: Базовый класс для методов оплаты
2. **CreditCardPayment, PayPalPayment, BankTransferPayment**: Конкретные реализации
3. Возможность добавления новых методов оплаты

#### LSP: Заменяемость
1. Все методы оплаты должны быть взаимозаменяемы
2. Корректная работа при подстановке

#### ISP: Разделение интерфейсов
1. **Payable**: Интерфейс для оплаты
2. **Refundable**: Интерфейс для возврата
3. **Trackable**: Интерфейс для отслеживания

#### DIP: Инверсия зависимостей
1. Зависимость от абстракций
2. Dependency injection

### Архитектура
```cpp
// ISP: Разделенные интерфейсы
class Payable {
public:
    virtual ~Payable() = default;
    virtual bool pay(double amount) = 0;
};

class Refundable {
public:
    virtual ~Refundable() = default;
    virtual bool refund(double amount) = 0;
};

class Trackable {
public:
    virtual ~Trackable() = default;
    virtual std::string getTransactionId() const = 0;
    virtual std::string getStatus() const = 0;
};

// OCP: Базовый класс для методов оплаты
class PaymentMethod : public Payable, public Trackable {
public:
    virtual ~PaymentMethod() = default;
    virtual std::string getMethodName() const = 0;
    virtual bool isAvailable() const = 0;
};

// SRP: Классы по ответственности
class Payment {
public:
    Payment(const std::string& id, double amount, const std::string& currency);
    const std::string& getId() const;
    double getAmount() const;
    const std::string& getCurrency() const;
    void setStatus(const std::string& status);
    std::string getStatus() const;
    
private:
    std::string id_;
    double amount_;
    std::string currency_;
    std::string status_;
};

class PaymentValidator {
public:
    virtual ~PaymentValidator() = default;
    virtual bool validate(const Payment& payment) = 0;
    virtual std::string getValidationError() const = 0;
};

class PaymentLogger {
public:
    virtual ~PaymentLogger() = default;
    virtual void logPayment(const Payment& payment, const std::string& method, bool success) = 0;
};

class PaymentNotifier {
public:
    virtual ~PaymentNotifier() = default;
    virtual void notifyPaymentSuccess(const Payment& payment) = 0;
    virtual void notifyPaymentFailure(const Payment& payment, const std::string& reason) = 0;
};

// DIP: Сервис зависит от абстракций
class PaymentService {
public:
    PaymentService(std::unique_ptr<PaymentValidator> validator,
                  std::unique_ptr<PaymentLogger> logger,
                  std::unique_ptr<PaymentNotifier> notifier);
    
    void addPaymentMethod(std::unique_ptr<PaymentMethod> method);
    bool processPayment(Payment& payment, const std::string& methodName);
    bool refundPayment(const std::string& paymentId, double amount);
    
private:
    std::unique_ptr<PaymentValidator> validator_;
    std::unique_ptr<PaymentLogger> logger_;
    std::unique_ptr<PaymentNotifier> notifier_;
    std::vector<std::unique_ptr<PaymentMethod>> methods_;
};
```

### Тестовый случай
```cpp
int main() {
    // DIP: Создаем зависимости
    auto validator = std::make_unique<AmountPaymentValidator>();
    auto logger = std::make_unique<FilePaymentLogger>("payments.log");
    auto notifier = std::make_unique<EmailPaymentNotifier>();
    
    // Создаем сервис платежей
    PaymentService service(std::move(validator), std::move(logger), std::move(notifier));
    
    // OCP: Добавляем методы оплаты
    service.addPaymentMethod(std::make_unique<CreditCardPayment>("1234-5678-9012-3456"));
    service.addPaymentMethod(std::make_unique<PayPalPayment>("user@example.com"));
    service.addPaymentMethod(std::make_unique<BankTransferPayment>("IBAN123456789"));
    
    // Создаем платеж
    Payment payment("PAY001", 100.50, "USD");
    
    // Обрабатываем платеж
    bool success = service.processPayment(payment, "CreditCard");
    
    if (success) {
        std::cout << "Платеж успешно обработан" << std::endl;
    } else {
        std::cout << "Ошибка обработки платежа" << std::endl;
    }
    
    // Возврат платежа
    service.refundPayment("PAY001", 50.25);
    
    return 0;
}
```

---

## 📋 Задание 4: Система управления заказами с SOLID

### Описание
Создайте систему управления заказами, строго следующую всем принципам SOLID.

### Требования
1. **Order**: Представляет заказ
2. **OrderProcessor**: Обрабатывает заказы
3. **InventoryManager**: Управляет складом
4. **ShippingCalculator**: Рассчитывает стоимость доставки
5. **OrderNotifier**: Уведомляет о статусе заказа

### Архитектура
```cpp
// ISP: Разделенные интерфейсы
class Processable {
public:
    virtual ~Processable() = default;
    virtual bool process() = 0;
};

class Shippable {
public:
    virtual ~Shippable() = default;
    virtual double calculateShippingCost() = 0;
    virtual bool canShip() = 0;
};

class Notifiable {
public:
    virtual ~Notifiable() = default;
    virtual void notify(const std::string& message) = 0;
};

// OCP: Базовый класс для заказов
class Order : public Processable, public Shippable, public Notifiable {
public:
    Order(const std::string& id, const std::string& customerId);
    virtual ~Order() = default;
    
    const std::string& getId() const;
    const std::string& getCustomerId() const;
    void addItem(const std::string& itemId, int quantity);
    void setStatus(const std::string& status);
    std::string getStatus() const;
    
    // Абстрактные методы
    virtual std::string getOrderType() const = 0;
    virtual double calculateTotal() = 0;
    
private:
    std::string id_;
    std::string customerId_;
    std::string status_;
    std::map<std::string, int> items_;
};

// SRP: Классы по ответственности
class InventoryManager {
public:
    virtual ~InventoryManager() = default;
    virtual bool hasItem(const std::string& itemId, int quantity) = 0;
    virtual void reserveItem(const std::string& itemId, int quantity) = 0;
    virtual void releaseItem(const std::string& itemId, int quantity) = 0;
};

class ShippingCalculator {
public:
    virtual ~ShippingCalculator() = default;
    virtual double calculateCost(const Order& order) = 0;
    virtual std::string getShippingMethod() = 0;
};

class OrderNotifier {
public:
    virtual ~OrderNotifier() = default;
    virtual void notifyOrderCreated(const Order& order) = 0;
    virtual void notifyOrderProcessed(const Order& order) = 0;
    virtual void notifyOrderShipped(const Order& order) = 0;
};

// DIP: Сервис зависит от абстракций
class OrderService {
public:
    OrderService(std::unique_ptr<InventoryManager> inventory,
                std::unique_ptr<ShippingCalculator> shipping,
                std::unique_ptr<OrderNotifier> notifier);
    
    void addOrder(std::unique_ptr<Order> order);
    bool processOrder(const std::string& orderId);
    bool shipOrder(const std::string& orderId);
    
private:
    std::unique_ptr<InventoryManager> inventoryManager_;
    std::unique_ptr<ShippingCalculator> shippingCalculator_;
    std::unique_ptr<OrderNotifier> orderNotifier_;
    std::map<std::string, std::unique_ptr<Order>> orders_;
};
```

### Тестовый случай
```cpp
int main() {
    // DIP: Создаем зависимости
    auto inventory = std::make_unique<InMemoryInventoryManager>();
    auto shipping = std::make_unique<StandardShippingCalculator>();
    auto notifier = std::make_unique<EmailOrderNotifier>();
    
    // Создаем сервис заказов
    OrderService service(std::move(inventory), std::move(shipping), std::move(notifier));
    
    // Создаем заказы
    auto order1 = std::make_unique<OnlineOrder>("ORD001", "CUST001");
    order1->addItem("ITEM001", 2);
    order1->addItem("ITEM002", 1);
    
    auto order2 = std::make_unique<StoreOrder>("ORD002", "CUST002");
    order2->addItem("ITEM003", 1);
    
    // Добавляем заказы
    service.addOrder(std::move(order1));
    service.addOrder(std::move(order2));
    
    // Обрабатываем заказы
    service.processOrder("ORD001");
    service.processOrder("ORD002");
    
    // Отправляем заказы
    service.shipOrder("ORD001");
    service.shipOrder("ORD002");
    
    return 0;
}
```

---

## 📋 Задание 5: Комплексная система с SOLID принципами

### Описание
Создайте систему управления библиотекой, демонстрирующую все принципы SOLID.

### Требования
1. **Book**: Представляет книгу
2. **Library**: Управляет книгами
3. **User**: Представляет пользователя
4. **Loan**: Представляет выдачу книги
5. **NotificationService**: Уведомляет о событиях

### Архитектура
```cpp
// ISP: Разделенные интерфейсы
class Borrowable {
public:
    virtual ~Borrowable() = default;
    virtual bool canBorrow() = 0;
    virtual void borrow() = 0;
    virtual void returnItem() = 0;
};

class Reservable {
public:
    virtual ~Reservable() = default;
    virtual bool canReserve() = 0;
    virtual void reserve() = 0;
    virtual void cancelReservation() = 0;
};

class Searchable {
public:
    virtual ~Searchable() = 0;
    virtual std::vector<std::string> search(const std::string& query) = 0;
};

// OCP: Базовый класс для библиотечных предметов
class LibraryItem : public Borrowable, public Reservable {
public:
    LibraryItem(const std::string& id, const std::string& title);
    virtual ~LibraryItem() = default;
    
    const std::string& getId() const;
    const std::string& getTitle() const;
    virtual std::string getType() const = 0;
    
protected:
    std::string id_;
    std::string title_;
    bool borrowed_;
    bool reserved_;
};

// SRP: Классы по ответственности
class Book : public LibraryItem {
public:
    Book(const std::string& id, const std::string& title, const std::string& author);
    std::string getType() const override;
    const std::string& getAuthor() const;
    
private:
    std::string author_;
};

class Magazine : public LibraryItem {
public:
    Magazine(const std::string& id, const std::string& title, int issueNumber);
    std::string getType() const override;
    int getIssueNumber() const;
    
private:
    int issueNumber_;
};

class User {
public:
    User(const std::string& id, const std::string& name, const std::string& email);
    const std::string& getId() const;
    const std::string& getName() const;
    const std::string& getEmail() const;
    
private:
    std::string id_;
    std::string name_;
    std::string email_;
};

class Loan {
public:
    Loan(const std::string& id, const std::string& userId, const std::string& itemId);
    const std::string& getId() const;
    const std::string& getUserId() const;
    const std::string& getItemId() const;
    void setReturnDate(const std::string& date);
    std::string getReturnDate() const;
    
private:
    std::string id_;
    std::string userId_;
    std::string itemId_;
    std::string returnDate_;
};

class LibraryService {
public:
    LibraryService(std::unique_ptr<Searchable> searchEngine,
                  std::unique_ptr<NotificationService> notifier);
    
    void addItem(std::unique_ptr<LibraryItem> item);
    void addUser(const User& user);
    bool borrowItem(const std::string& userId, const std::string& itemId);
    bool returnItem(const std::string& userId, const std::string& itemId);
    bool reserveItem(const std::string& userId, const std::string& itemId);
    
private:
    std::unique_ptr<Searchable> searchEngine_;
    std::unique_ptr<NotificationService> notifier_;
    std::vector<std::unique_ptr<LibraryItem>> items_;
    std::vector<User> users_;
    std::vector<Loan> loans_;
};
```

### Тестовый случай
```cpp
int main() {
    // DIP: Создаем зависимости
    auto searchEngine = std::make_unique<FullTextSearchEngine>();
    auto notifier = std::make_unique<EmailNotificationService>();
    
    // Создаем сервис библиотеки
    LibraryService library(std::move(searchEngine), std::move(notifier));
    
    // Добавляем книги
    library.addItem(std::make_unique<Book>("BK001", "Война и мир", "Лев Толстой"));
    library.addItem(std::make_unique<Book>("BK002", "1984", "Джордж Оруэлл"));
    library.addItem(std::make_unique<Magazine>("MG001", "Наука и жизнь", 1));
    
    // Добавляем пользователей
    library.addUser(User("USR001", "Иван Иванов", "ivan@example.com"));
    library.addUser(User("USR002", "Петр Петров", "petr@example.com"));
    
    // Выдаем книги
    library.borrowItem("USR001", "BK001");
    library.borrowItem("USR002", "BK002");
    
    // Резервируем книгу
    library.reserveItem("USR001", "MG001");
    
    // Возвращаем книгу
    library.returnItem("USR001", "BK001");
    
    return 0;
}
```

---

## 🎯 Критерии оценки

### Отлично (5 баллов)
- ✅ Все принципы SOLID соблюдены
- ✅ Код хорошо структурирован и расширяем
- ✅ Интерфейсы правильно разделены
- ✅ Зависимости инвертированы
- ✅ Код следует современным стандартам C++

### Хорошо (4 балла)
- ✅ Большинство принципов SOLID соблюдены
- ✅ Код в целом хорошо структурирован
- ✅ Есть небольшие нарушения принципов
- ✅ Код работает корректно

### Удовлетворительно (3 балла)
- ✅ Основные принципы SOLID соблюдены
- ✅ Код компилируется и работает
- ❌ Есть значительные нарушения принципов
- ❌ Код сложно расширять

### Неудовлетворительно (2 балла)
- ❌ Принципы SOLID не соблюдены
- ❌ Код плохо структурирован
- ❌ Код не компилируется или работает неправильно

---

## 💡 Подсказки

1. **SRP**: Каждый класс должен иметь только одну причину для изменения
2. **OCP**: Классы должны быть открыты для расширения, но закрыты для модификации
3. **LSP**: Подклассы должны быть заменяемы базовыми классами
4. **ISP**: Клиенты не должны зависеть от интерфейсов, которые они не используют
5. **DIP**: Зависите от абстракций, а не от конкретных реализаций

---

*SOLID принципы - это основа качественного объектно-ориентированного дизайна. Освойте их, и ваши системы станут намного лучше!*
