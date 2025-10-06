# 💉 Dependency Injection - Плакат

## 📋 Обзор паттерна

**Dependency Injection (DI)** - это паттерн проектирования, который реализует принцип инверсии зависимостей (Dependency Inversion Principle). Вместо создания зависимостей внутри класса, они передаются извне через конструктор, методы или свойства.

## 🎯 Назначение

- **Инверсия зависимостей**: Зависимости создаются вне класса и передаются в него
- **Слабая связанность**: Классы не зависят от конкретных реализаций
- **Тестируемость**: Легко заменять зависимости на моки для тестирования
- **Гибкость**: Возможность изменять поведение без изменения кода

## 🏗️ Структура паттерна

### Без DI (плохо):
```
┌─────────────────┐
│   Client        │
│   (клиент)      │
├─────────────────┤
│ - service       │
│ + doWork()      │
└─────────────────┘
         │
         ▼
┌─────────────────┐
│   Service       │
│   (сервис)      │
├─────────────────┤
│ + process()     │
└─────────────────┘
```

### С DI (хорошо):
```
┌─────────────────┐    ┌─────────────────┐
│   Client        │    │   Service      │
│   (клиент)      │◄───┤   (сервис)     │
├─────────────────┤    ├─────────────────┤
│ - service       │    │ + process()    │
│ + doWork()      │    └─────────────────┘
└─────────────────┘
         ▲
         │
┌─────────────────┐
│   Injector      │
│   (инжектор)    │
├─────────────────┤
│ + inject()      │
└─────────────────┘
```

## 💡 Ключевые компоненты

### 1. Интерфейс зависимости
```cpp
// Интерфейс для сервиса
class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(const std::string& message) = 0;
};

class IDatabase {
public:
    virtual ~IDatabase() = default;
    virtual void save(const std::string& data) = 0;
    virtual std::string load(int id) = 0;
};
```

### 2. Конкретные реализации
```cpp
// Конкретные реализации сервисов
class ConsoleLogger : public ILogger {
public:
    void log(const std::string& message) override {
        std::cout << "[CONSOLE] " << message << std::endl;
    }
};

class FileLogger : public ILogger {
private:
    std::string filename;
    
public:
    FileLogger(const std::string& file) : filename(file) {}
    
    void log(const std::string& message) override {
        std::cout << "[FILE:" << filename << "] " << message << std::endl;
    }
};

class SQLDatabase : public IDatabase {
public:
    void save(const std::string& data) override {
        std::cout << "SQL: Сохранение данных: " << data << std::endl;
    }
    
    std::string load(int id) override {
        return "SQL: Данные с ID " + std::to_string(id);
    }
};

class NoSQLDatabase : public IDatabase {
public:
    void save(const std::string& data) override {
        std::cout << "NoSQL: Сохранение документа: " << data << std::endl;
    }
    
    std::string load(int id) override {
        return "NoSQL: Документ с ID " + std::to_string(id);
    }
};
```

### 3. Класс с инжекцией зависимостей
```cpp
// Класс, который получает зависимости через конструктор
class UserService {
private:
    std::unique_ptr<ILogger> logger;
    std::unique_ptr<IDatabase> database;
    
public:
    // Constructor Injection
    UserService(std::unique_ptr<ILogger> log, std::unique_ptr<IDatabase> db)
        : logger(std::move(log)), database(std::move(db)) {}
    
    void createUser(const std::string& name, const std::string& email) {
        logger->log("Создание пользователя: " + name);
        
        std::string userData = "Имя: " + name + ", Email: " + email;
        database->save(userData);
        
        logger->log("Пользователь " + name + " успешно создан");
    }
    
    std::string getUser(int id) {
        logger->log("Получение пользователя с ID: " + std::to_string(id));
        return database->load(id);
    }
};
```

## 🔧 Практический пример: Система управления заказами

```cpp
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

// Интерфейсы для различных сервисов
class IPaymentProcessor {
public:
    virtual ~IPaymentProcessor() = default;
    virtual bool processPayment(double amount, const std::string& cardNumber) = 0;
    virtual std::string getProcessorName() const = 0;
};

class IEmailService {
public:
    virtual ~IEmailService() = default;
    virtual void sendEmail(const std::string& to, const std::string& subject, const std::string& body) = 0;
    virtual std::string getServiceName() const = 0;
};

class IInventoryService {
public:
    virtual ~IInventoryService() = default;
    virtual bool checkAvailability(const std::string& productId, int quantity) = 0;
    virtual void reserveProduct(const std::string& productId, int quantity) = 0;
    virtual void releaseReservation(const std::string& productId, int quantity) = 0;
};

class IOrderRepository {
public:
    virtual ~IOrderRepository() = default;
    virtual void saveOrder(const std::string& orderId, const std::string& orderData) = 0;
    virtual std::string getOrder(const std::string& orderId) = 0;
    virtual void updateOrderStatus(const std::string& orderId, const std::string& status) = 0;
};

// Конкретные реализации сервисов
class StripePaymentProcessor : public IPaymentProcessor {
public:
    bool processPayment(double amount, const std::string& cardNumber) override {
        std::cout << "💳 Stripe: Обработка платежа $" << amount << " для карты " << cardNumber << std::endl;
        // Симуляция обработки платежа
        return cardNumber.length() >= 16; // Простая валидация
    }
    
    std::string getProcessorName() const override {
        return "Stripe";
    }
};

class PayPalPaymentProcessor : public IPaymentProcessor {
public:
    bool processPayment(double amount, const std::string& cardNumber) override {
        std::cout << "💰 PayPal: Обработка платежа $" << amount << " для карты " << cardNumber << std::endl;
        return cardNumber.length() >= 13; // PayPal принимает более короткие номера
    }
    
    std::string getProcessorName() const override {
        return "PayPal";
    }
};

class SMTPEmailService : public IEmailService {
private:
    std::string smtpServer;
    
public:
    SMTPEmailService(const std::string& server) : smtpServer(server) {}
    
    void sendEmail(const std::string& to, const std::string& subject, const std::string& body) override {
        std::cout << "📧 SMTP (" << smtpServer << "): Отправка письма" << std::endl;
        std::cout << "   Кому: " << to << std::endl;
        std::cout << "   Тема: " << subject << std::endl;
        std::cout << "   Содержание: " << body << std::endl;
    }
    
    std::string getServiceName() const override {
        return "SMTP (" + smtpServer + ")";
    }
};

class SendGridEmailService : public IEmailService {
public:
    void sendEmail(const std::string& to, const std::string& subject, const std::string& body) override {
        std::cout << "📧 SendGrid: Отправка письма" << std::endl;
        std::cout << "   Кому: " << to << std::endl;
        std::cout << "   Тема: " << subject << std::endl;
        std::cout << "   Содержание: " << body << std::endl;
    }
    
    std::string getServiceName() const override {
        return "SendGrid";
    }
};

class InMemoryInventoryService : public IInventoryService {
private:
    std::unordered_map<std::string, int> inventory;
    
public:
    InMemoryInventoryService() {
        // Инициализация тестового инвентаря
        inventory["LAPTOP001"] = 10;
        inventory["MOUSE001"] = 50;
        inventory["KEYBOARD001"] = 25;
    }
    
    bool checkAvailability(const std::string& productId, int quantity) override {
        auto it = inventory.find(productId);
        if (it != inventory.end()) {
            bool available = it->second >= quantity;
            std::cout << "📦 Проверка наличия: " << productId << " - " << quantity 
                      << " (доступно: " << it->second << ")" << std::endl;
            return available;
        }
        std::cout << "❌ Товар " << productId << " не найден" << std::endl;
        return false;
    }
    
    void reserveProduct(const std::string& productId, int quantity) override {
        auto it = inventory.find(productId);
        if (it != inventory.end()) {
            it->second -= quantity;
            std::cout << "🔒 Резервирование: " << productId << " - " << quantity 
                      << " (осталось: " << it->second << ")" << std::endl;
        }
    }
    
    void releaseReservation(const std::string& productId, int quantity) override {
        auto it = inventory.find(productId);
        if (it != inventory.end()) {
            it->second += quantity;
            std::cout << "🔓 Освобождение резерва: " << productId << " - " << quantity 
                      << " (теперь: " << it->second << ")" << std::endl;
        }
    }
};

class SQLOrderRepository : public IOrderRepository {
private:
    std::string connectionString;
    
public:
    SQLOrderRepository(const std::string& connStr) : connectionString(connStr) {}
    
    void saveOrder(const std::string& orderId, const std::string& orderData) override {
        std::cout << "💾 SQL (" << connectionString << "): Сохранение заказа " << orderId << std::endl;
        std::cout << "   Данные: " << orderData << std::endl;
    }
    
    std::string getOrder(const std::string& orderId) override {
        std::cout << "📖 SQL: Получение заказа " << orderId << std::endl;
        return "Заказ " + orderId + " - данные из SQL";
    }
    
    void updateOrderStatus(const std::string& orderId, const std::string& status) override {
        std::cout << "🔄 SQL: Обновление статуса заказа " << orderId << " на " << status << std::endl;
    }
};

// Основной сервис с инжекцией зависимостей
class OrderService {
private:
    std::unique_ptr<IPaymentProcessor> paymentProcessor;
    std::unique_ptr<IEmailService> emailService;
    std::unique_ptr<IInventoryService> inventoryService;
    std::unique_ptr<IOrderRepository> orderRepository;
    
public:
    // Constructor Injection - все зависимости передаются через конструктор
    OrderService(
        std::unique_ptr<IPaymentProcessor> payment,
        std::unique_ptr<IEmailService> email,
        std::unique_ptr<IInventoryService> inventory,
        std::unique_ptr<IOrderRepository> repository
    ) : paymentProcessor(std::move(payment)),
        emailService(std::move(email)),
        inventoryService(std::move(inventory)),
        orderRepository(std::move(repository)) {}
    
    struct OrderItem {
        std::string productId;
        int quantity;
        double price;
    };
    
    struct Order {
        std::string orderId;
        std::string customerEmail;
        std::vector<OrderItem> items;
        double totalAmount;
        std::string status;
    };
    
    bool processOrder(const Order& order) {
        std::cout << "\n🛒 Обработка заказа " << order.orderId << std::endl;
        std::cout << "   Клиент: " << order.customerEmail << std::endl;
        std::cout << "   Сумма: $" << order.totalAmount << std::endl;
        
        // 1. Проверка наличия товаров
        for (const auto& item : order.items) {
            if (!inventoryService->checkAvailability(item.productId, item.quantity)) {
                std::cout << "❌ Заказ отменен: недостаточно товара " << item.productId << std::endl;
                return false;
            }
        }
        
        // 2. Резервирование товаров
        for (const auto& item : order.items) {
            inventoryService->reserveProduct(item.productId, item.quantity);
        }
        
        // 3. Обработка платежа
        std::string cardNumber = "1234567890123456"; // В реальности получается из формы
        if (!paymentProcessor->processPayment(order.totalAmount, cardNumber)) {
            std::cout << "❌ Заказ отменен: ошибка обработки платежа" << std::endl;
            
            // Освобождение резерва при ошибке платежа
            for (const auto& item : order.items) {
                inventoryService->releaseReservation(item.productId, item.quantity);
            }
            return false;
        }
        
        // 4. Сохранение заказа
        std::string orderData = "Заказ " + order.orderId + " на сумму $" + std::to_string(order.totalAmount);
        orderRepository->saveOrder(order.orderId, orderData);
        orderRepository->updateOrderStatus(order.orderId, "Оплачен");
        
        // 5. Отправка подтверждения
        std::string subject = "Подтверждение заказа " + order.orderId;
        std::string body = "Ваш заказ на сумму $" + std::to_string(order.totalAmount) + " успешно обработан!";
        emailService->sendEmail(order.customerEmail, subject, body);
        
        std::cout << "✅ Заказ " << order.orderId << " успешно обработан!" << std::endl;
        return true;
    }
    
    void getOrderStatus(const std::string& orderId) {
        std::string orderData = orderRepository->getOrder(orderId);
        std::cout << "📋 Статус заказа: " << orderData << std::endl;
    }
    
    // Методы для получения информации о сервисах
    void printServiceInfo() {
        std::cout << "\n🔧 Информация о сервисах:" << std::endl;
        std::cout << "   Платежная система: " << paymentProcessor->getProcessorName() << std::endl;
        std::cout << "   Email сервис: " << emailService->getServiceName() << std::endl;
        std::cout << "   Инвентарь: In-Memory" << std::endl;
        std::cout << "   Репозиторий: SQL" << std::endl;
    }
};

// Контейнер зависимостей (простая реализация)
class DIContainer {
public:
    static std::unique_ptr<OrderService> createOrderService() {
        // Создание всех зависимостей
        auto paymentProcessor = std::make_unique<StripePaymentProcessor>();
        auto emailService = std::make_unique<SMTPEmailService>("smtp.company.com");
        auto inventoryService = std::make_unique<InMemoryInventoryService>();
        auto orderRepository = std::make_unique<SQLOrderRepository>("Server=localhost;Database=Orders");
        
        // Инжекция зависимостей через конструктор
        return std::make_unique<OrderService>(
            std::move(paymentProcessor),
            std::move(emailService),
            std::move(inventoryService),
            std::move(orderRepository)
        );
    }
    
    static std::unique_ptr<OrderService> createOrderServiceWithPayPal() {
        // Альтернативная конфигурация с PayPal
        auto paymentProcessor = std::make_unique<PayPalPaymentProcessor>();
        auto emailService = std::make_unique<SendGridEmailService>();
        auto inventoryService = std::make_unique<InMemoryInventoryService>();
        auto orderRepository = std::make_unique<SQLOrderRepository>("Server=prod;Database=Orders");
        
        return std::make_unique<OrderService>(
            std::move(paymentProcessor),
            std::move(emailService),
            std::move(inventoryService),
            std::move(orderRepository)
        );
    }
};

// Демонстрация использования
int main() {
    std::cout << "=== Демонстрация Dependency Injection ===" << std::endl;
    
    // Создание сервиса через DI контейнер
    auto orderService = DIContainer::createOrderService();
    orderService->printServiceInfo();
    
    // Создание тестового заказа
    OrderService::Order testOrder;
    testOrder.orderId = "ORD-001";
    testOrder.customerEmail = "customer@example.com";
    testOrder.items = {
        {"LAPTOP001", 1, 999.99},
        {"MOUSE001", 2, 29.99}
    };
    testOrder.totalAmount = 1059.97;
    
    // Обработка заказа
    bool success = orderService->processOrder(testOrder);
    
    if (success) {
        std::cout << "\n📊 Проверка статуса заказа:" << std::endl;
        orderService->getOrderStatus("ORD-001");
    }
    
    // Демонстрация альтернативной конфигурации
    std::cout << "\n🔄 Переключение на PayPal конфигурацию:" << std::endl;
    auto paypalOrderService = DIContainer::createOrderServiceWithPayPal();
    paypalOrderService->printServiceInfo();
    
    // Обработка заказа с PayPal
    OrderService::Order paypalOrder;
    paypalOrder.orderId = "ORD-002";
    paypalOrder.customerEmail = "paypal@example.com";
    paypalOrder.items = {{"KEYBOARD001", 1, 79.99}};
    paypalOrder.totalAmount = 79.99;
    
    paypalOrderService->processOrder(paypalOrder);
    
    return 0;
}
```

## 🎯 Применение в реальных проектах

### 1. **Веб-приложения**
```cpp
// Контроллер с инжекцией сервисов
class UserController {
private:
    std::unique_ptr<IUserService> userService;
    std::unique_ptr<IAuthService> authService;
    std::unique_ptr<ILogger> logger;
    
public:
    UserController(
        std::unique_ptr<IUserService> user,
        std::unique_ptr<IAuthService> auth,
        std::unique_ptr<ILogger> log
    ) : userService(std::move(user)),
        authService(std::move(auth)),
        logger(std::move(log)) {}
    
    void handleLogin(const std::string& username, const std::string& password) {
        logger->log("Попытка входа: " + username);
        if (authService->authenticate(username, password)) {
            logger->log("Успешный вход: " + username);
        }
    }
};
```

### 2. **Игровые движки**
```cpp
// Игровой объект с инжекцией компонентов
class GameObject {
private:
    std::unique_ptr<IRenderer> renderer;
    std::unique_ptr<IPhysics> physics;
    std::unique_ptr<IAudio> audio;
    
public:
    GameObject(
        std::unique_ptr<IRenderer> rend,
        std::unique_ptr<IPhysics> phys,
        std::unique_ptr<IAudio> aud
    ) : renderer(std::move(rend)),
        physics(std::move(phys)),
        audio(std::move(aud)) {}
    
    void update() {
        physics->update();
        renderer->render();
    }
};
```

### 3. **Микросервисы**
```cpp
// Сервис с инжекцией внешних зависимостей
class OrderMicroservice {
private:
    std::unique_ptr<IPaymentGateway> paymentGateway;
    std::unique_ptr<IInventoryAPI> inventoryAPI;
    std::unique_ptr<INotificationService> notificationService;
    
public:
    OrderMicroservice(
        std::unique_ptr<IPaymentGateway> payment,
        std::unique_ptr<IInventoryAPI> inventory,
        std::unique_ptr<INotificationService> notification
    ) : paymentGateway(std::move(payment)),
        inventoryAPI(std::move(inventory)),
        notificationService(std::move(notification)) {}
};
```

## ⚡ Преимущества паттерна

### ✅ **Плюсы:**
- **Слабая связанность**: Классы не зависят от конкретных реализаций
- **Тестируемость**: Легко заменять зависимости на моки
- **Гибкость**: Возможность изменять поведение без изменения кода
- **Переиспользование**: Компоненты можно использовать в разных контекстах
- **Принцип единственной ответственности**: Каждый класс отвечает за свою область

### ❌ **Минусы:**
- **Сложность**: Больше кода для настройки зависимостей
- **Производительность**: Дополнительные вызовы методов
- **Понимание**: Сложнее понять полную картину зависимостей
- **Отладка**: Сложнее отслеживать поток выполнения

## 🔄 Типы инжекции зависимостей

### 1. **Constructor Injection (Инжекция через конструктор)**
```cpp
// ✅ Рекомендуемый способ
class Service {
private:
    std::unique_ptr<IDependency> dependency;
    
public:
    Service(std::unique_ptr<IDependency> dep) : dependency(std::move(dep)) {}
};
```

### 2. **Setter Injection (Инжекция через сеттер)**
```cpp
// ✅ Для опциональных зависимостей
class Service {
private:
    std::unique_ptr<IDependency> dependency;
    
public:
    void setDependency(std::unique_ptr<IDependency> dep) {
        dependency = std::move(dep);
    }
};
```

### 3. **Interface Injection (Инжекция через интерфейс)**
```cpp
// ✅ Для сложных случаев
class IInjectable {
public:
    virtual void inject(std::unique_ptr<IDependency> dep) = 0;
};

class Service : public IInjectable {
public:
    void inject(std::unique_ptr<IDependency> dep) override {
        // Установка зависимости
    }
};
```

## 🎨 Современные улучшения в C++

### 1. **Использование std::optional (C++17)**
```cpp
#include <optional>

// Опциональные зависимости
class FlexibleService {
private:
    std::optional<std::unique_ptr<ILogger>> logger;
    
public:
    void setLogger(std::unique_ptr<ILogger> log) {
        logger = std::move(log);
    }
    
    void doWork() {
        if (logger) {
            logger->value()->log("Работа выполнена");
        }
    }
};
```

### 2. **Концепты и шаблоны (C++20)**
```cpp
#include <concepts>

// Концепт для зависимостей
template<typename T>
concept Service = requires(T t) {
    t.initialize();
    t.process();
    t.cleanup();
};

// Шаблонный DI контейнер
template<Service... Services>
class DIContainer {
private:
    std::tuple<Services...> services;
    
public:
    template<typename T>
    T& get() {
        return std::get<T>(services);
    }
};
```

## 🚀 Лучшие практики

### 1. **Правильная инжекция**
```cpp
// ✅ Хорошо: Инжекция через конструктор
class UserService {
private:
    std::unique_ptr<IUserRepository> repository;
    std::unique_ptr<ILogger> logger;
    
public:
    UserService(
        std::unique_ptr<IUserRepository> repo,
        std::unique_ptr<ILogger> log
    ) : repository(std::move(repo)), logger(std::move(log)) {}
};

// ❌ Плохо: Создание зависимостей внутри класса
class BadUserService {
private:
    std::unique_ptr<IUserRepository> repository;
    
public:
    BadUserService() {
        repository = std::make_unique<SQLUserRepository>(); // Жесткая связанность!
    }
};
```

### 2. **Управление жизненным циклом**
```cpp
// ✅ Хорошо: RAII для управления зависимостями
class ServiceManager {
private:
    std::vector<std::unique_ptr<IService>> services;
    
public:
    template<typename T, typename... Args>
    void registerService(Args&&... args) {
        services.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }
    
    ~ServiceManager() {
        // Автоматическая очистка при уничтожении
        services.clear();
    }
};
```

### 3. **Конфигурация зависимостей**
```cpp
// ✅ Хорошо: Централизованная конфигурация
class DIConfiguration {
public:
    static std::unique_ptr<OrderService> createOrderService() {
        // Чтение конфигурации из файла или переменных окружения
        std::string paymentProvider = getConfig("payment.provider");
        std::string emailProvider = getConfig("email.provider");
        
        // Создание соответствующих зависимостей
        auto payment = createPaymentProcessor(paymentProvider);
        auto email = createEmailService(emailProvider);
        
        return std::make_unique<OrderService>(std::move(payment), std::move(email));
    }
    
private:
    static std::string getConfig(const std::string& key) {
        // Реализация чтения конфигурации
        return "stripe"; // Упрощенная реализация
    }
};
```

## 📊 Сравнение с другими паттернами

| Аспект | Dependency Injection | Service Locator | Singleton |
|--------|---------------------|-----------------|-----------|
| **Слабая связанность** | Высокая | Средняя | Низкая |
| **Тестируемость** | Высокая | Средняя | Низкая |
| **Гибкость** | Высокая | Средняя | Низкая |
| **Сложность** | Средняя | Низкая | Низкая |
| **Производительность** | Средняя | Высокая | Высокая |
| **Применение** | Крупные системы | Средние системы | Простые случаи |

## 🎯 Заключение

**Dependency Injection** - это мощный паттерн для создания слабо связанных, тестируемых и гибких систем. Он является основой современных архитектур и фреймворков.

### **Когда использовать:**
- ✅ Нужна слабая связанность между компонентами
- ✅ Требуется высокая тестируемость
- ✅ Система должна быть гибкой и расширяемой
- ✅ Есть внешние зависимости (БД, API, сервисы)

### **Когда НЕ использовать:**
- ❌ Простые системы без внешних зависимостей
- ❌ Производительность критична
- ❌ Команда не готова к сложности DI
- ❌ Нет времени на настройку DI контейнера

**Dependency Injection** - это основа современной архитектуры программного обеспечения, которая обеспечивает качество, гибкость и поддерживаемость кода! 💉✨
