/**
 * @file srp_example.cpp
 * @brief Детальная демонстрация Single Responsibility Principle (SRP)
 * @author Senior C++ Developer
 * @date 2024
 * 
 * Этот файл содержит углубленный анализ SRP с математическим обоснованием
 * и практическими примерами из реальной разработки.
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <chrono>
#include <iomanip>

/**
 * @brief Математическая формализация SRP
 * 
 * SRP можно формализовать следующим образом:
 * ∀C ∈ Classes: |Reasons(C)| = 1
 * где Reasons(C) = {r | изменение r требует модификации C}
 * 
 * Другими словами: каждый класс должен иметь ровно одну причину для изменения.
 */

// ============================================================================
// ПРИМЕР 1: СИСТЕМА УПРАВЛЕНИЯ ЗАКАЗАМИ
// ============================================================================

/**
 * @brief Нарушение SRP: Монолитный класс OrderManager
 * 
 * Этот класс нарушает SRP, так как отвечает за:
 * 1. Хранение данных заказа
 * 2. Валидацию данных
 * 3. Расчет стоимости
 * 4. Применение скидок
 * 5. Отправку уведомлений
 * 6. Работу с базой данных
 * 7. Генерацию отчетов
 */
class BadOrderManager {
private:
    struct Order {
        int id;
        std::string customerName;
        std::string customerEmail;
        std::vector<std::string> items;
        double totalAmount;
        std::string status;
        std::chrono::system_clock::time_point createdAt;
    };
    
    Order order;
    
public:
    BadOrderManager(int orderId, const std::string& customer, const std::string& email) {
        order.id = orderId;
        order.customerName = customer;
        order.customerEmail = email;
        order.totalAmount = 0.0;
        order.status = "pending";
        order.createdAt = std::chrono::system_clock::now();
    }
    
    // Нарушение SRP: добавление товаров
    void addItem(const std::string& item) {
        order.items.push_back(item);
        std::cout << "📦 Добавлен товар: " << item << "\n";
    }
    
    // Нарушение SRP: валидация
    bool validateOrder() {
        if (order.customerName.empty()) {
            std::cout << "❌ Имя клиента не может быть пустым\n";
            return false;
        }
        
        if (order.customerEmail.empty() || order.customerEmail.find('@') == std::string::npos) {
            std::cout << "❌ Неверный email адрес\n";
            return false;
        }
        
        if (order.items.empty()) {
            std::cout << "❌ Заказ не может быть пустым\n";
            return false;
        }
        
        return true;
    }
    
    // Нарушение SRP: расчет стоимости
    void calculateTotal() {
        order.totalAmount = 0.0;
        for (const auto& item : order.items) {
            // Упрощенная логика расчета (в реальности была бы более сложной)
            if (item == "laptop") order.totalAmount += 1000.0;
            else if (item == "mouse") order.totalAmount += 25.0;
            else if (item == "keyboard") order.totalAmount += 75.0;
            else order.totalAmount += 50.0; // цена по умолчанию
        }
        std::cout << "💰 Общая стоимость: $" << order.totalAmount << "\n";
    }
    
    // Нарушение SRP: применение скидок
    void applyDiscount(double discountPercent) {
        if (discountPercent > 0 && discountPercent <= 100) {
            double discountAmount = order.totalAmount * (discountPercent / 100.0);
            order.totalAmount -= discountAmount;
            std::cout << "🎯 Скидка " << discountPercent << "% применена. "
                      << "Экономия: $" << discountAmount << "\n";
        }
    }
    
    // Нарушение SRP: отправка уведомлений
    void sendConfirmationEmail() {
        std::cout << "📧 Отправка подтверждения на " << order.customerEmail << "\n";
        std::cout << "Текст: Здравствуйте, " << order.customerName 
                  << "! Ваш заказ #" << order.id << " на сумму $" 
                  << order.totalAmount << " подтвержден.\n";
    }
    
    // Нарушение SRP: работа с БД
    void saveToDatabase() {
        std::cout << "💾 Сохранение заказа #" << order.id << " в базу данных\n";
        // Здесь была бы реальная логика работы с БД
    }
    
    // Нарушение SRP: генерация отчетов
    void generateReport() {
        std::ofstream report("order_report.txt");
        report << "=== ОТЧЕТ ПО ЗАКАЗУ ===\n";
        report << "ID: " << order.id << "\n";
        report << "Клиент: " << order.customerName << "\n";
        report << "Email: " << order.customerEmail << "\n";
        report << "Товары:\n";
        for (const auto& item : order.items) {
            report << "  - " << item << "\n";
        }
        report << "Общая сумма: $" << order.totalAmount << "\n";
        report << "Статус: " << order.status << "\n";
        report.close();
        std::cout << "📊 Отчет сохранен в order_report.txt\n";
    }
    
    // Нарушение SRP: изменение статуса
    void updateStatus(const std::string& newStatus) {
        order.status = newStatus;
        std::cout << "🔄 Статус заказа изменен на: " << newStatus << "\n";
    }
};

/**
 * @brief Соблюдение SRP: Разделение ответственностей
 * 
 * Каждый класс имеет единственную ответственность:
 * - Order: хранение данных заказа
 * - OrderValidator: валидация данных
 * - PriceCalculator: расчет стоимости
 * - DiscountManager: применение скидок
 * - NotificationService: отправка уведомлений
 * - OrderRepository: работа с БД
 * - ReportGenerator: генерация отчетов
 * - OrderStatusManager: управление статусами
 */

// Класс для хранения данных заказа
class Order {
private:
    int id;
    std::string customerName;
    std::string customerEmail;
    std::vector<std::string> items;
    double totalAmount;
    std::string status;
    std::chrono::system_clock::time_point createdAt;
    
public:
    Order(int orderId, const std::string& customer, const std::string& email)
        : id(orderId), customerName(customer), customerEmail(email), 
          totalAmount(0.0), status("pending"), createdAt(std::chrono::system_clock::now()) {}
    
    // Геттеры
    int getId() const { return id; }
    const std::string& getCustomerName() const { return customerName; }
    const std::string& getCustomerEmail() const { return customerEmail; }
    const std::vector<std::string>& getItems() const { return items; }
    double getTotalAmount() const { return totalAmount; }
    const std::string& getStatus() const { return status; }
    auto getCreatedAt() const { return createdAt; }
    
    // Сеттеры
    void addItem(const std::string& item) {
        items.push_back(item);
        std::cout << "📦 Добавлен товар: " << item << "\n";
    }
    
    void setTotalAmount(double amount) { totalAmount = amount; }
    void setStatus(const std::string& newStatus) { status = newStatus; }
};

// Класс для валидации заказов
class OrderValidator {
public:
    static bool isValid(const Order& order) {
        return validateCustomerName(order) && 
               validateCustomerEmail(order) && 
               validateItems(order);
    }
    
    static std::vector<std::string> getValidationErrors(const Order& order) {
        std::vector<std::string> errors;
        
        if (!validateCustomerName(order)) {
            errors.push_back("Имя клиента не может быть пустым");
        }
        
        if (!validateCustomerEmail(order)) {
            errors.push_back("Неверный email адрес");
        }
        
        if (!validateItems(order)) {
            errors.push_back("Заказ не может быть пустым");
        }
        
        return errors;
    }
    
private:
    static bool validateCustomerName(const Order& order) {
        return !order.getCustomerName().empty();
    }
    
    static bool validateCustomerEmail(const Order& order) {
        const std::string& email = order.getCustomerEmail();
        return !email.empty() && email.find('@') != std::string::npos;
    }
    
    static bool validateItems(const Order& order) {
        return !order.getItems().empty();
    }
};

// Класс для расчета стоимости
class PriceCalculator {
private:
    static double getItemPrice(const std::string& item) {
        // В реальной системе это была бы база данных или конфигурационный файл
        if (item == "laptop") return 1000.0;
        else if (item == "mouse") return 25.0;
        else if (item == "keyboard") return 75.0;
        else if (item == "monitor") return 300.0;
        else if (item == "headphones") return 150.0;
        else return 50.0; // цена по умолчанию
    }
    
public:
    static double calculateTotal(const Order& order) {
        double total = 0.0;
        for (const auto& item : order.getItems()) {
            total += getItemPrice(item);
        }
        return total;
    }
    
    static void updateOrderTotal(Order& order) {
        double total = calculateTotal(order);
        order.setTotalAmount(total);
        std::cout << "💰 Общая стоимость: $" << total << "\n";
    }
};

// Класс для управления скидками
class DiscountManager {
public:
    enum class DiscountType {
        PERCENTAGE,
        FIXED_AMOUNT,
        BUY_ONE_GET_ONE
    };
    
    static void applyDiscount(Order& order, DiscountType type, double value) {
        double originalAmount = order.getTotalAmount();
        double discountAmount = 0.0;
        
        switch (type) {
            case DiscountType::PERCENTAGE:
                discountAmount = originalAmount * (value / 100.0);
                break;
            case DiscountType::FIXED_AMOUNT:
                discountAmount = std::min(value, originalAmount);
                break;
            case DiscountType::BUY_ONE_GET_ONE:
                // Упрощенная логика BOGO
                discountAmount = originalAmount * 0.5;
                break;
        }
        
        order.setTotalAmount(originalAmount - discountAmount);
        std::cout << "🎯 Скидка применена. Экономия: $" << discountAmount 
                  << ", новая сумма: $" << order.getTotalAmount() << "\n";
    }
    
    static bool isEligibleForDiscount(const Order& order) {
        return order.getTotalAmount() > 100.0; // минимальная сумма для скидки
    }
};

// Класс для отправки уведомлений
class NotificationService {
public:
    static void sendConfirmationEmail(const Order& order) {
        std::cout << "📧 Отправка подтверждения на " << order.getCustomerEmail() << "\n";
        std::cout << "Текст: Здравствуйте, " << order.getCustomerName() 
                  << "! Ваш заказ #" << order.getId() << " на сумму $" 
                  << order.getTotalAmount() << " подтвержден.\n";
    }
    
    static void sendStatusUpdate(const Order& order) {
        std::cout << "📱 Отправка обновления статуса на " << order.getCustomerEmail() << "\n";
        std::cout << "Текст: Статус вашего заказа #" << order.getId() 
                  << " изменен на: " << order.getStatus() << "\n";
    }
    
    static void sendShippingNotification(const Order& order) {
        std::cout << "🚚 Отправка уведомления об отправке на " << order.getCustomerEmail() << "\n";
        std::cout << "Текст: Ваш заказ #" << order.getId() << " отправлен!\n";
    }
};

// Класс для работы с базой данных
class OrderRepository {
public:
    static void save(const Order& order) {
        std::cout << "💾 Сохранение заказа #" << order.getId() << " в базу данных\n";
        // Здесь была бы реальная логика работы с БД
        // INSERT INTO orders (id, customer_name, customer_email, total_amount, status, created_at)
        // VALUES (?, ?, ?, ?, ?, ?)
    }
    
    static Order findById(int id) {
        std::cout << "🔍 Поиск заказа #" << id << " в базе данных\n";
        // Заглушка для демонстрации
        return Order(id, "Test Customer", "test@example.com");
    }
    
    static void updateStatus(int orderId, const std::string& status) {
        std::cout << "🔄 Обновление статуса заказа #" << orderId << " на " << status << "\n";
        // UPDATE orders SET status = ? WHERE id = ?
    }
    
    static std::vector<Order> findByCustomer(const std::string& customerEmail) {
        std::cout << "🔍 Поиск заказов клиента " << customerEmail << "\n";
        // SELECT * FROM orders WHERE customer_email = ?
        return {}; // заглушка
    }
};

// Класс для генерации отчетов
class ReportGenerator {
public:
    static void generateOrderReport(const Order& order, const std::string& filename = "order_report.txt") {
        std::ofstream report(filename);
        
        report << "=== ОТЧЕТ ПО ЗАКАЗУ ===\n";
        report << "ID: " << order.getId() << "\n";
        report << "Клиент: " << order.getCustomerName() << "\n";
        report << "Email: " << order.getCustomerEmail() << "\n";
        report << "Товары:\n";
        
        for (const auto& item : order.getItems()) {
            report << "  - " << item << "\n";
        }
        
        report << "Общая сумма: $" << order.getTotalAmount() << "\n";
        report << "Статус: " << order.getStatus() << "\n";
        
        auto time_t = std::chrono::system_clock::to_time_t(order.getCreatedAt());
        report << "Дата создания: " << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "\n";
        
        report.close();
        std::cout << "📊 Отчет сохранен в " << filename << "\n";
    }
    
    static void generateCustomerReport(const std::string& customerEmail) {
        auto orders = OrderRepository::findByCustomer(customerEmail);
        
        std::ofstream report("customer_report.txt");
        report << "=== ОТЧЕТ ПО КЛИЕНТУ ===\n";
        report << "Email: " << customerEmail << "\n";
        report << "Количество заказов: " << orders.size() << "\n";
        report.close();
        
        std::cout << "📊 Отчет по клиенту сохранен в customer_report.txt\n";
    }
};

// Класс для управления статусами заказов
class OrderStatusManager {
public:
    enum class Status {
        PENDING,
        CONFIRMED,
        PROCESSING,
        SHIPPED,
        DELIVERED,
        CANCELLED
    };
    
    static std::string statusToString(Status status) {
        switch (status) {
            case Status::PENDING: return "pending";
            case Status::CONFIRMED: return "confirmed";
            case Status::PROCESSING: return "processing";
            case Status::SHIPPED: return "shipped";
            case Status::DELIVERED: return "delivered";
            case Status::CANCELLED: return "cancelled";
            default: return "unknown";
        }
    }
    
    static bool canTransitionTo(const std::string& currentStatus, const std::string& newStatus) {
        // Упрощенная логика переходов состояний
        if (currentStatus == "pending") {
            return newStatus == "confirmed" || newStatus == "cancelled";
        } else if (currentStatus == "confirmed") {
            return newStatus == "processing" || newStatus == "cancelled";
        } else if (currentStatus == "processing") {
            return newStatus == "shipped" || newStatus == "cancelled";
        } else if (currentStatus == "shipped") {
            return newStatus == "delivered";
        }
        return false;
    }
    
    static void updateStatus(Order& order, const std::string& newStatus) {
        if (canTransitionTo(order.getStatus(), newStatus)) {
            order.setStatus(newStatus);
            OrderRepository::updateStatus(order.getId(), newStatus);
            std::cout << "🔄 Статус заказа #" << order.getId() 
                      << " изменен на: " << newStatus << "\n";
        } else {
            std::cout << "❌ Невозможно изменить статус с " << order.getStatus() 
                      << " на " << newStatus << "\n";
        }
    }
};

// ============================================================================
// ДЕМОНСТРАЦИЯ ПРИНЦИПА
// ============================================================================

void demonstrateBadSRP() {
    std::cout << "❌ НАРУШЕНИЕ SRP - Монолитный класс:\n";
    std::cout << std::string(50, '-') << "\n";
    
    BadOrderManager badOrder(1001, "Иван Иванов", "ivan@example.com");
    badOrder.addItem("laptop");
    badOrder.addItem("mouse");
    badOrder.addItem("keyboard");
    
    if (badOrder.validateOrder()) {
        badOrder.calculateTotal();
        badOrder.applyDiscount(10.0);
        badOrder.sendConfirmationEmail();
        badOrder.saveToDatabase();
        badOrder.generateReport();
        badOrder.updateStatus("confirmed");
    }
}

void demonstrateGoodSRP() {
    std::cout << "\n✅ СОБЛЮДЕНИЕ SRP - Разделение ответственностей:\n";
    std::cout << std::string(50, '-') << "\n";
    
    // Создание заказа
    Order order(1002, "Петр Петров", "petr@example.com");
    order.addItem("laptop");
    order.addItem("monitor");
    order.addItem("headphones");
    
    // Валидация
    if (OrderValidator::isValid(order)) {
        std::cout << "✅ Заказ валиден\n";
        
        // Расчет стоимости
        PriceCalculator::updateOrderTotal(order);
        
        // Применение скидки (если применимо)
        if (DiscountManager::isEligibleForDiscount(order)) {
            DiscountManager::applyDiscount(order, DiscountManager::DiscountType::PERCENTAGE, 15.0);
        }
        
        // Сохранение в БД
        OrderRepository::save(order);
        
        // Отправка уведомления
        NotificationService::sendConfirmationEmail(order);
        
        // Генерация отчета
        ReportGenerator::generateOrderReport(order);
        
        // Обновление статуса
        OrderStatusManager::updateStatus(order, "confirmed");
        
        // Дополнительные уведомления при изменении статуса
        if (order.getStatus() == "confirmed") {
            NotificationService::sendStatusUpdate(order);
        }
        
    } else {
        std::cout << "❌ Заказ невалиден. Ошибки:\n";
        auto errors = OrderValidator::getValidationErrors(order);
        for (const auto& error : errors) {
            std::cout << "  - " << error << "\n";
        }
    }
}

// ============================================================================
// АНАЛИЗ КОМПРОМИССОВ
// ============================================================================

void analyzeTradeOffs() {
    std::cout << "\n🔬 АНАЛИЗ КОМПРОМИССОВ SRP:\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::cout << "📊 ПРЕИМУЩЕСТВА:\n";
    std::cout << "• Единственная ответственность - легче понимать и тестировать\n";
    std::cout << "• Меньше coupling между модулями\n";
    std::cout << "• Возможность независимого изменения компонентов\n";
    std::cout << "• Улучшенная переиспользуемость кода\n";
    std::cout << "• Более четкое разделение обязанностей в команде\n\n";
    
    std::cout << "⚠️ НЕДОСТАТКИ:\n";
    std::cout << "• Увеличение количества классов и файлов\n";
    std::cout << "• Потенциальное усложнение архитектуры\n";
    std::cout << "• Необходимость координации между компонентами\n";
    std::cout << "• Возможный overhead от создания объектов\n\n";
    
    std::cout << "🎯 РЕКОМЕНДАЦИИ:\n";
    std::cout << "• Применяйте SRP для бизнес-логики\n";
    std::cout << "• Не переусложняйте простые задачи\n";
    std::cout << "• Используйте композицию для объединения компонентов\n";
    std::cout << "• Рассмотрите использование фасадов для упрощения интерфейса\n";
}

int main() {
    std::cout << "🎯 ДЕТАЛЬНАЯ ДЕМОНСТРАЦИЯ SINGLE RESPONSIBILITY PRINCIPLE (SRP)\n";
    std::cout << "Автор: Senior C++ Developer\n";
    std::cout << "Цель: Углубленное изучение принципа единственной ответственности\n\n";
    
    demonstrateBadSRP();
    demonstrateGoodSRP();
    analyzeTradeOffs();
    
    std::cout << "\n📚 МАТЕМАТИЧЕСКОЕ ОБОСНОВАНИЕ:\n";
    std::cout << "SRP: ∀C ∈ Classes: |Reasons(C)| = 1\n";
    std::cout << "где Reasons(C) = {r | изменение r требует модификации C}\n\n";
    
    std::cout << "💡 КЛЮЧЕВЫЕ ВЫВОДЫ:\n";
    std::cout << "1. Каждый класс должен иметь ровно одну причину для изменения\n";
    std::cout << "2. Разделение ответственностей улучшает maintainability\n";
    std::cout << "3. SRP является основой для других принципов SOLID\n";
    std::cout << "4. Применение требует баланса между простотой и гибкостью\n\n";
    
    std::cout << "🔬 Принципы - это инструменты для мышления о проблемах!\n";
    
    return 0;
}
